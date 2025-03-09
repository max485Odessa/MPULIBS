#include "ICM42688.hpp"
#include "registers_ICM42688.hpp"
#include "rutine.hpp"
#include <math.h>


using namespace ICM42688reg;

static const char *chipname = "icm42688";


ICM42688::ICM42688(uint8_t slv_adr): TI2CTRDIFACE (slv_adr) {
	i2c_bus = 0;
	sw_state = EIMUSW_NONE;
}


const char *ICM42688::GetChipName ()
{
	return chipname;
}



/* starts communication with the ICM42688 */
bool ICM42688::begin() {

bool rv = false;
do	{
	reset();
	if (whoAmI() != WHO_AM_I) break;
	// turn on accel and gyro in Low Noise (LN) Mode
	if (!writeRegister(UB0_REG_PWR_MGMT0, 0x0F)) break;

	// 16G is default -- do this to set up accel resolution scaling
	if (!setAccelFS(gpm16)) break;

	// 2000DPS is default -- do this to set up gyro resolution scaling
	if (!setGyroFS(dps2000)) break;


	// disable inner filters (Notch filter, Anti-alias filter, UI filter block)
	if (!setFilters(false, false)) break;

	// estimate gyro bias
	if (!calibrateGyro()) break;

	rv = true;

	} while (false);


return rv;
}

/* sets the accelerometer full scale range to values other than default */
bool ICM42688::setAccelFS(AccelFS fssel) {

bool rv = false;
uint8_t reg;

do	{

	setBank(0);

	// read current register value

	if (!readRegisters(UB0_REG_ACCEL_CONFIG0, 1, &reg)) break;

	// only change FS_SEL in reg
	reg = (fssel << 5) | (reg & 0x1F);

	if (!writeRegister(UB0_REG_ACCEL_CONFIG0, reg)) break;

	_accelScale = static_cast<float>(1 << (4 - fssel)) / 32768.0f;
	_accelFS    = fssel;

	rv = true;

	} while (false);

return rv;
}

/* get the accelerometer full scale range return the ACCEL_FS_SEL value*/
int ICM42688::getAccelFS() {
	int rv = 0;
	uint8_t reg;

	do	{
		setBank(0);
		// read current register value

		if (!readRegisters(UB0_REG_ACCEL_CONFIG0, 1, &reg)) break;
		rv = (reg & 0xE0) >> 5;

		} while (false);
	return rv;

}



/* sets the gyro full scale range to values other than default */
bool ICM42688::setGyroFS(GyroFS fssel) {

	bool rv = false;
	uint8_t reg;

	do	{

		setBank(0);

		// read current register value

		if (!readRegisters(UB0_REG_GYRO_CONFIG0, 1, &reg)) break;
		// only change FS_SEL in reg
		reg = (fssel << 5) | (reg & 0x1F);

		if (!writeRegister(UB0_REG_GYRO_CONFIG0, reg)) break;

		_gyroScale = (2000.0f / static_cast<float>(1 << fssel)) / 32768.0f;
		_gyroFS    = fssel;

		rv = true;
		} while (false);
	return rv;
}



bool ICM42688::setAccelODR(ODR odr) {

	bool rv = false;
	uint8_t reg;

	do	{
		setBank(0);

		if (!readRegisters(UB0_REG_ACCEL_CONFIG0, 1, &reg)) break;

		// only change ODR in reg
		reg = odr | (reg & 0xF0);

		if (!writeRegister(UB0_REG_ACCEL_CONFIG0, reg)) break;
		rv = true;
	} while (false);

	return rv;
}



bool ICM42688::setGyroODR(ODR odr) {

	bool rv = false;
	uint8_t reg;

	do	{

		setBank(0);

		// read current register value
		if (!readRegisters(UB0_REG_GYRO_CONFIG0, 1, &reg)) break;
		// only change ODR in reg
		reg = odr | (reg & 0xF0);

		if (!writeRegister(UB0_REG_GYRO_CONFIG0, reg)) break;


		rv = true;
		} while (false);
	return rv;
}



bool ICM42688::setFilters(bool gyroFilters, bool accFilters) {
	bool rv = false;
	do	{
		if (!setBank(1)) break;

		if (gyroFilters == true) {
			if (!writeRegister(UB1_REG_GYRO_CONFIG_STATIC2, GYRO_NF_ENABLE | GYRO_AAF_ENABLE)) break;
		} else {
			if (!writeRegister(UB1_REG_GYRO_CONFIG_STATIC2, GYRO_NF_DISABLE | GYRO_AAF_DISABLE)) break;
		}

		if (!setBank(2)) break;

		if (accFilters == true) {
			if (!writeRegister(UB2_REG_ACCEL_CONFIG_STATIC2, ACCEL_AAF_ENABLE)) break;
		} else {
			if (!writeRegister(UB2_REG_ACCEL_CONFIG_STATIC2, ACCEL_AAF_DISABLE)) break;
		}

		if (!setBank(0)) break;
		rv = true;
		} while (false);
	return rv;
}



bool ICM42688::enableDataReadyInterrupt() {

	bool rv = false;
	uint8_t reg;

	do	{
		// push-pull, pulsed, active HIGH interrupts
		if (!writeRegister(UB0_REG_INT_CONFIG, 0x18 | 0x03)) break;

		// need to clear bit 4 to allow proper INT1 and INT2 operation

		if (!readRegisters(UB0_REG_INT_CONFIG1, 1, &reg)) break;

		reg &= ~0x10;
		if (!writeRegister(UB0_REG_INT_CONFIG1, reg)) break;


		// route UI data ready interrupt to INT1
		if (!writeRegister(UB0_REG_INT_SOURCE0, 0x18)) break;
		rv = true;
		} while (false);
	return rv;
}



bool ICM42688::disableDataReadyInterrupt() {

	bool rv = false;
	uint8_t reg;
	do	{


		if (!readRegisters(UB0_REG_INT_CONFIG1, 1, &reg)) break;

		reg |= 0x10;
		if (!writeRegister(UB0_REG_INT_CONFIG1, reg)) break;


		// return reg to reset value
		if (!writeRegister(UB0_REG_INT_SOURCE0, 0x10)) break;


		rv = true;
		} while (false);
	return rv;
}

/* reads the most current data from ICM42688 and stores in buffer */
bool ICM42688::getAGT() {  // modified to use getRawAGT()
	bool rv = getRawAGT();

	if (rv)
		{
		_t = (static_cast<float>(_rawT) / TEMP_DATA_REG_SCALE) + TEMP_OFFSET;

		_acc[0] = ((_rawAcc[0] * _accelScale) - _accB[0]) * _accS[0];
		_acc[1] = ((_rawAcc[1] * _accelScale) - _accB[1]) * _accS[1];
		_acc[2] = ((_rawAcc[2] * _accelScale) - _accB[2]) * _accS[2];

		_gyr[0] = (_rawGyr[0] * _gyroScale) - _gyrB[0];
		_gyr[1] = (_rawGyr[1] * _gyroScale) - _gyrB[1];
		_gyr[2] = (_rawGyr[2] * _gyroScale) - _gyrB[2];
		}

	return rv;
}

/* reads the most current data from ICM42688 and stores in buffer */
bool ICM42688::getRawAGT() {  // Added to return raw data only
	// grab the data from the ICM42688
	bool rv = false;
	// combine bytes into 16 bit values
	int16_t rawMeas[7];  // temp, accel xyz, gyro xyz

	do	{

		if (!readRegisters(UB0_REG_TEMP_DATA1, 14, _buffer)) break;

		for (size_t i = 0; i < 7; i++) {
			rawMeas[i] = ((int16_t)_buffer[i * 2] << 8) | _buffer[i * 2 + 1];
		}

		_rawT      = rawMeas[0];
		_rawAcc[0] = rawMeas[1];
		_rawAcc[1] = rawMeas[2];
		_rawAcc[2] = rawMeas[3];
		_rawGyr[0] = rawMeas[4];
		_rawGyr[1] = rawMeas[5];
		_rawGyr[2] = rawMeas[6];

		rv = true;
		} while (false);

	return rv;
}

/* configures and enables the FIFO buffer
  Enforces best-fitting structure (1, 2, or 3) but cannot replicate Packet 4 as choosing between 3/4 requires
  additional argument for high-resolution

  See https://invensense.tdk.com/wp-content/uploads/2020/04/ds-000347_icm-42688-p-datasheet.pdf, 6.1 for details
*/
bool ICM42688_FIFO::enableFifo(bool accel, bool gyro, bool temp) {

	_enFifoAccel = accel;
	_enFifoGyro  = gyro;
	_enFifoTemp  = true;  // all structures have 1-byte temp, didn't return error to maintain backwards compatibility
	_enFifoTimestamp =
	  accel && gyro;      // can only read both accel and gyro in Structure 3 or 4, both have 2-byte timestamp
	_enFifoHeader  = accel || gyro;  // if neither sensor requested, FIFO will not send any more packets
	_fifoFrameSize = _enFifoHeader * 1 + _enFifoAccel * 6 + _enFifoGyro * 6 + _enFifoTemp + _enFifoTimestamp * 2;


	return writeRegister(FIFO_EN, (_enFifoAccel * FIFO_ACCEL) | (_enFifoGyro * FIFO_GYRO) | (_enFifoTemp * FIFO_TEMP_EN));

}

/* Start streaming, required to read after enableFifo() under most sensor configurations */
bool ICM42688_FIFO::streamToFifo() {
	return writeRegister(ICM42688reg::UB0_REG_FIFO_CONFIG, 1 << 6);
}

/* reads data from the ICM42688 FIFO and stores in buffer
  High-resolution mode not yet supported */
bool ICM42688_FIFO::readFifo() {
	bool rv = false;

	do	{
		// get the fifo size
		if (!readRegisters(UB0_REG_FIFO_COUNTH, 2, _buffer)) break;
		_fifoSize = (((uint16_t)(_buffer[0] & 0x0F)) << 8) + ((uint16_t)_buffer[1]);

		// precalculate packet structure as per-packet recalculation based on headers isn't reliable
		// header does not confirm whether packet is sized for high-resolution (20-bit) data
		size_t numFrames = _fifoSize / _fifoFrameSize;
		size_t accIndex  = 1;
		size_t gyroIndex = accIndex + _enFifoAccel * 6;
		size_t tempIndex = gyroIndex + _enFifoGyro * 6;
		// read and parse the buffer
		for (size_t i = 0; i < _fifoSize / _fifoFrameSize; i++) {
			// grab the data from the ICM42688
			if (!readRegisters(UB0_REG_FIFO_DATA, _fifoFrameSize, _buffer)) return rv;

			if (_enFifoAccel) {
				// combine into 16 bit values
				int16_t rawMeas[3];
				rawMeas[0] = (((int16_t)_buffer[0 + accIndex]) << 8) | _buffer[1 + accIndex];
				rawMeas[1] = (((int16_t)_buffer[2 + accIndex]) << 8) | _buffer[3 + accIndex];
				rawMeas[2] = (((int16_t)_buffer[4 + accIndex]) << 8) | _buffer[5 + accIndex];
				// transform and convert to float values
				_axFifo[i] = ((rawMeas[0] * _accelScale) - _accB[0]) * _accS[0];
				_ayFifo[i] = ((rawMeas[1] * _accelScale) - _accB[1]) * _accS[1];
				_azFifo[i] = ((rawMeas[2] * _accelScale) - _accB[2]) * _accS[2];
				_aSize     = numFrames;
				}
			if (_enFifoTemp) {
				int8_t rawMeas = _buffer[tempIndex + 0];
				// transform and convert to float values
				_tFifo[i] = (static_cast<float>(rawMeas) / TEMP_DATA_REG_SCALE) + TEMP_OFFSET;
				_tSize    = numFrames;
				}
			if (_enFifoGyro) {
				// combine into 16 bit values
				int16_t rawMeas[3];
				rawMeas[0] = (((int16_t)_buffer[0 + gyroIndex]) << 8) | _buffer[1 + gyroIndex];
				rawMeas[1] = (((int16_t)_buffer[2 + gyroIndex]) << 8) | _buffer[3 + gyroIndex];
				rawMeas[2] = (((int16_t)_buffer[4 + gyroIndex]) << 8) | _buffer[5 + gyroIndex];
				// transform and convert to float values
				_gxFifo[i] = (rawMeas[0] * _gyroScale) - _gyrB[0];
				_gyFifo[i] = (rawMeas[1] * _gyroScale) - _gyrB[1];
				_gzFifo[i] = (rawMeas[2] * _gyroScale) - _gyrB[2];
				_gSize     = numFrames;
				}
			}

		rv = true;
		} while (false);

	return rv;
}

/* returns the accelerometer FIFO size and data in the x direction, m/s/s */
void ICM42688_FIFO::getFifoAccelX_mss(size_t* size, float* data) {
	*size = _aSize;
	CopyMemorySDC (_axFifo, data, _aSize * sizeof(float));
	//memcpy(data, _axFifo, _aSize * sizeof(float));
}

/* returns the accelerometer FIFO size and data in the y direction, m/s/s */
void ICM42688_FIFO::getFifoAccelY_mss(size_t* size, float* data) {
	*size = _aSize;
	CopyMemorySDC (_ayFifo, data, _aSize * sizeof(float));
	//memcpy(data, _ayFifo, _aSize * sizeof(float));
}

/* returns the accelerometer FIFO size and data in the z direction, m/s/s */
void ICM42688_FIFO::getFifoAccelZ_mss(size_t* size, float* data) {
	*size = _aSize;
	CopyMemorySDC (_azFifo, data, _aSize * sizeof(float));
	//memcpy(data, _azFifo, _aSize * sizeof(float));
}

/* returns the gyroscope FIFO size and data in the x direction, dps */
void ICM42688_FIFO::getFifoGyroX(size_t* size, float* data) {
	*size = _gSize;
	CopyMemorySDC (_gxFifo, data, _gSize * sizeof(float));
	//memcpy(data, _gxFifo, _gSize * sizeof(float));
}

/* returns the gyroscope FIFO size and data in the y direction, dps */
void ICM42688_FIFO::getFifoGyroY(size_t* size, float* data) {
	*size = _gSize;
	CopyMemorySDC (_gyFifo, data, _gSize * sizeof(float));
	//memcpy(data, _gyFifo, _gSize * sizeof(float));
}

/* returns the gyroscope FIFO size and data in the z direction, dps */
void ICM42688_FIFO::getFifoGyroZ(size_t* size, float* data) {
	*size = _gSize;
	CopyMemorySDC (_gzFifo, data, _gSize * sizeof(float));
	//memcpy(data, _gzFifo, _gSize * sizeof(float));
}

/* returns the die temperature FIFO size and data, C */
void ICM42688_FIFO::getFifoTemperature_C(size_t* size, float* data) {
	*size = _tSize;
	CopyMemorySDC (_tFifo, data, _tSize * sizeof(float));
	//memcpy(data, _tFifo, _tSize * sizeof(float));
}

/* estimates the gyro biases */
bool ICM42688::calibrateGyro() {
	// set at a lower range (more resolution) since IMU not moving

	bool rv = false;
	do	{

		const GyroFS current_fssel = _gyroFS;
		if (!setGyroFS(dps250)) break;

		// take samples and find bias
		_gyroBD[0] = 0;
		_gyroBD[1] = 0;
		_gyroBD[2] = 0;
		for (size_t i = 0; i < NUM_CALIB_SAMPLES; i++) {
			getAGT();
			_gyroBD[0] += (gyrX() + _gyrB[0]) / NUM_CALIB_SAMPLES;
			_gyroBD[1] += (gyrY() + _gyrB[1]) / NUM_CALIB_SAMPLES;
			_gyroBD[2] += (gyrZ() + _gyrB[2]) / NUM_CALIB_SAMPLES;
			SYSBIOS::Wait(1);
			//delay(1);
		}
		_gyrB[0] = _gyroBD[0];
		_gyrB[1] = _gyroBD[1];
		_gyrB[2] = _gyroBD[2];

		// recover the full scale setting
		if (!setGyroFS(current_fssel)) break;

		rv = true;
		} while (false);
	return rv;
}

/* returns the gyro bias in the X direction, dps */
float ICM42688::getGyroBiasX() {
	return _gyrB[0];
}

/* returns the gyro bias in the Y direction, dps */
float ICM42688::getGyroBiasY() {
	return _gyrB[1];
}

/* returns the gyro bias in the Z direction, dps */
float ICM42688::getGyroBiasZ() {
	return _gyrB[2];
}

/* sets the gyro bias in the X direction to bias, dps */
void ICM42688::setGyroBiasX(float bias) {
	_gyrB[0] = bias;
}

/* sets the gyro bias in the Y direction to bias, dps */
void ICM42688::setGyroBiasY(float bias) {
	_gyrB[1] = bias;
}

/* sets the gyro bias in the Z direction to bias, dps */
void ICM42688::setGyroBiasZ(float bias) {
	_gyrB[2] = bias;
}

/* finds bias and scale factor calibration for the accelerometer,
this should be run for each axis in each direction (6 total) to find
the min and max values along each */
bool ICM42688::calibrateAccel() {
	// set at a lower range (more resolution) since IMU not moving

	bool rv = false;

	do	{


		const AccelFS current_fssel = _accelFS;
		if (!setAccelFS(gpm2)) break;

		// take samples and find min / max
		_accBD[0] = 0;
		_accBD[1] = 0;
		_accBD[2] = 0;
		for (size_t i = 0; i < NUM_CALIB_SAMPLES; i++) {
			getAGT();
			_accBD[0] += (accX() / _accS[0] + _accB[0]) / NUM_CALIB_SAMPLES;
			_accBD[1] += (accY() / _accS[1] + _accB[1]) / NUM_CALIB_SAMPLES;
			_accBD[2] += (accZ() / _accS[2] + _accB[2]) / NUM_CALIB_SAMPLES;
			SYSBIOS::Wait(1);
			}
		if (_accBD[0] > 0.9f) {
			_accMax[0] = _accBD[0];
			}
		if (_accBD[1] > 0.9f) {
			_accMax[1] = _accBD[1];
			}
		if (_accBD[2] > 0.9f) {
			_accMax[2] = _accBD[2];
			}
		if (_accBD[0] < -0.9f) {
			_accMin[0] = _accBD[0];
			}
		if (_accBD[1] < -0.9f) {
			_accMin[1] = _accBD[1];
			}
		if (_accBD[2] < -0.9f) {
			_accMin[2] = _accBD[2];
			}

		// find bias and scale factor
		if ((abs_f(_accMin[0]) > 0.9f) && (abs_f(_accMax[0]) > 0.9f)) {
			_accB[0] = (_accMin[0] + _accMax[0]) / 2.0f;
			_accS[0] = 1 / ((abs_f(_accMin[0]) + abs_f(_accMax[0])) / 2.0f);
		}
		if ((abs_f(_accMin[1]) > 0.9f) && (abs_f(_accMax[1]) > 0.9f)) {
			_accB[1] = (_accMin[1] + _accMax[1]) / 2.0f;
			_accS[1] = 1 / ((abs_f(_accMin[1]) + abs_f(_accMax[1])) / 2.0f);
		}
		if ((abs_f(_accMin[2]) > 0.9f) && (abs_f(_accMax[2]) > 0.9f)) {
			_accB[2] = (_accMin[2] + _accMax[2]) / 2.0f;
			_accS[2] = 1 / ((abs_f(_accMin[2]) + abs_f(_accMax[2])) / 2.0f);
		}

		// recover the full scale setting
		if (!setAccelFS(current_fssel)) break;

		rv = true;
		} while (false);

	return rv;
}

/* returns the accelerometer bias in the X direction, m/s/s */
float ICM42688::getAccelBiasX_mss() {
	return _accB[0];
}

/* returns the accelerometer scale factor in the X direction */
float ICM42688::getAccelScaleFactorX() {
	return _accS[0];
}

/* returns the accelerometer bias in the Y direction, m/s/s */
float ICM42688::getAccelBiasY_mss() {
	return _accB[1];
}

/* returns the accelerometer scale factor in the Y direction */
float ICM42688::getAccelScaleFactorY() {
	return _accS[1];
}

/* returns the accelerometer bias in the Z direction, m/s/s */
float ICM42688::getAccelBiasZ_mss() {
	return _accB[2];
}

/* returns the accelerometer scale factor in the Z direction */
float ICM42688::getAccelScaleFactorZ() {
	return _accS[2];
}

/* sets the accelerometer bias (m/s/s) and scale factor in the X direction */
void ICM42688::setAccelCalX(float bias, float scaleFactor) {
	_accB[0] = bias;
	_accS[0] = scaleFactor;
}

/* sets the accelerometer bias (m/s/s) and scale factor in the Y direction */
void ICM42688::setAccelCalY(float bias, float scaleFactor) {
	_accB[1] = bias;
	_accS[1] = scaleFactor;
}

/* sets the accelerometer bias (m/s/s) and scale factor in the Z direction */
void ICM42688::setAccelCalZ(float bias, float scaleFactor) {
	_accB[2] = bias;
	_accS[2] = scaleFactor;
}

/* writes a byte to ICM42688 register given a register address and data */
bool ICM42688::writeRegister(uint8_t subAddress, uint8_t data) {
	bool rv = false;
	do	{
		if (!i2c_bus) break;
		if (!i2c_bus->WriteFrame_i2c (C_SLAVEADRESS, subAddress, &data, sizeof(data))) break;
		SYSBIOS::Wait(10);
		if (!readRegisters(subAddress, 1, _buffer)) break;
		if (_buffer[0] != data) break;
		rv = true;
		} while (false);
return rv;
}

/* reads registers from ICM42688 given a starting register address, number of bytes, and a pointer to store data */
bool ICM42688::readRegisters(uint8_t subAddress, uint8_t count, uint8_t* dest) {
	bool rv = false;
	do	{
		if (!i2c_bus) break;
		rv = i2c_bus->ReadFrame_i2c (C_SLAVEADRESS, subAddress, dest, count);
		} while (false);
return rv;
}


bool ICM42688::setBank(uint8_t bank) {
	// if we are already on this bank, bail
	if (_bank == bank) return true;
	_bank = bank;

	return writeRegister(REG_BANK_SEL, bank);
}

void ICM42688::reset() {
	setBank(0);

	writeRegister(UB0_REG_DEVICE_CONFIG, 0x01);

	// wait for ICM42688 to come back up
	SYSBIOS::Wait(1);
}

/* gets the ICM42688 WHO_AM_I register value */
uint8_t ICM42688::whoAmI() {

	uint8_t rv = 0;

	do	{
		setBank(0);

		// read the WHO AM I register
		if (!readRegisters(UB0_REG_WHO_AM_I, 1, _buffer)) break;
		// return the register value
		rv = _buffer[0];
		} while (false);

	return rv;
}



/* get Raw Bias (Offsets)*/  //Added to use Offsets rather than compensating through additional code
bool ICM42688::computeOffsets() {
	const AccelFS current_Accelfssel = _accelFS;
	const GyroFS  current_Gyrofssel  = _gyroFS;

	bool rv = false;

	do	{

		// set the IMU at the correct resolution
		if (!setAccelFS(ICM42688::AccelFS::gpm2)) break;
		if (!setGyroFS(ICM42688::GyroFS::dps250)) break;
		int16_t FullScale_Acc = 2;
		int16_t FullScale_Gyr = 250;

		// reset the Offset_user
		setBank(4);
		if (!writeRegister(UB4_REG_OFFSET_USER5, 0)) break;
		if (!writeRegister(UB4_REG_OFFSET_USER6, 0)) break;
		if (!writeRegister(UB4_REG_OFFSET_USER8, 0)) break;
		if (!writeRegister(UB4_REG_OFFSET_USER2, 0)) break;
		if (!writeRegister(UB4_REG_OFFSET_USER3, 0)) break;
		if (!writeRegister(UB4_REG_OFFSET_USER0, 0)) break;
		if (!writeRegister(UB4_REG_OFFSET_USER4, 0)) break;
		if (!writeRegister(UB4_REG_OFFSET_USER7, 0)) break;
		if (!writeRegister(UB4_REG_OFFSET_USER1, 0)) break;

		setBank(0);
		// reinitialize the _rawAccBias and _rawGyrBias
		for (size_t ii = 1; ii < 3; ii++) {
			_rawAccBias[ii] = 0;
			_rawGyrBias[ii] = 0;
		}
		// record raw values and add samples
		for (size_t i = 0; i < NUM_CALIB_SAMPLES; i++) {
			getRawAGT();
			_rawAccBias[0] += _rawAcc[0];
			_rawAccBias[1] += _rawAcc[1];
			_rawAccBias[2] += _rawAcc[2];
			_rawGyrBias[0] += _rawGyr[0];
			_rawGyrBias[1] += _rawGyr[1];
			_rawGyrBias[2] += _rawGyr[2];
			SYSBIOS::Wait(1);
		}

		// Average
		_rawAccBias[0] = (int32_t)((double)_rawAccBias[0] / (double)NUM_CALIB_SAMPLES);
		_rawAccBias[1] = (int32_t)((double)_rawAccBias[1] / (double)NUM_CALIB_SAMPLES);
		_rawAccBias[2] = (int32_t)((double)_rawAccBias[2] / (double)NUM_CALIB_SAMPLES);
		_rawGyrBias[0] = (int32_t)((double)_rawGyrBias[0] / (double)NUM_CALIB_SAMPLES);
		_rawGyrBias[1] = (int32_t)((double)_rawGyrBias[1] / (double)NUM_CALIB_SAMPLES);
		_rawGyrBias[2] = (int32_t)((double)_rawGyrBias[2] / (double)NUM_CALIB_SAMPLES);

		//compensate gravity and compute the _AccOffset and _GyrOffset
		for (size_t ii = 0; ii < 3; ii++) {
			_AccOffset[ii] =
			  (int16_t)(-(_rawAccBias[ii]) * (FullScale_Acc / 32768.0f * 2048));  //*2048));  // 0.5 mg resolution
			if (_rawAccBias[ii] * FullScale_Acc > 26'000) {
				_AccOffset[ii] = (int16_t)(-(_rawAccBias[ii] - 32'768 / FullScale_Acc) * (FullScale_Acc / 32768.0f * 2048));
			}  //26000 ~80% of 32768
			if (_rawAccBias[ii] * FullScale_Acc < -26'000) {
				_AccOffset[ii] = (int16_t)(-(_rawAccBias[ii] + 32'768 / FullScale_Acc) * (FullScale_Acc / 32768.0f * 2048));
			}
			_GyrOffset[ii] = (int16_t)((-_rawGyrBias[ii]) * (FullScale_Gyr / 32768.0f * 32));  //1/32 dps resolution
		}

		// Serial.println("The new raw Bias are:");
		// for(size_t ii = 0; ii< 3; ii++){
		//   Serial.print(_rawAccBias[ii]);Serial.print("\t");
		// }
		//  for(size_t ii = 0; ii< 3; ii++){
		//   Serial.print(_rawGyrBias[ii]);Serial.print("\t");
		// }
		// Serial.println("");

		// Serial.println("The new Offsets are:");
		// for(size_t ii = 0; ii< 3; ii++){
		//   Serial.print(_AccOffset[ii]);Serial.print("\t");
		// }
		//  for(size_t ii = 0; ii< 3; ii++){
		//   Serial.print(_GyrOffset[ii]);Serial.print("\t");
		// }
		// Serial.println("");

		if (!setAccelFS(current_Accelfssel)) break;

		if (!setGyroFS(current_Gyrofssel)) break;

		rv = true;
		} while (false);

	return rv;
}

bool ICM42688::setAllOffsets() {
	bool rv = false;
	do	{

		setBank(4);
		uint8_t reg;

		// clear all offsets:
		if (!writeRegister(UB4_REG_OFFSET_USER0, 0)) break;
		if (!writeRegister(UB4_REG_OFFSET_USER1, 0)) break;
		if (!writeRegister(UB4_REG_OFFSET_USER2, 0)) break;
		if (!writeRegister(UB4_REG_OFFSET_USER3, 0)) break;
		if (!writeRegister(UB4_REG_OFFSET_USER4, 0)) break;
		if (!writeRegister(UB4_REG_OFFSET_USER5, 0)) break;
		if (!writeRegister(UB4_REG_OFFSET_USER6, 0)) break;
		if (!writeRegister(UB4_REG_OFFSET_USER7, 0)) break;

		reg = _AccOffset[0] & 0x00FF;  // lower Ax byte
		if (!writeRegister(UB4_REG_OFFSET_USER5, reg)) break;

		reg = _AccOffset[1] & 0x00FF;  // lower Ay byte
		if (!writeRegister(UB4_REG_OFFSET_USER6, reg)) break;

		reg = _AccOffset[2] & 0x00FF;  // lower Az byte
		if (!writeRegister(UB4_REG_OFFSET_USER8, reg)) break;

		reg = _GyrOffset[1] & 0x00FF;  // lower Gy byte
		if (!writeRegister(UB4_REG_OFFSET_USER2, reg)) break;

		reg = _GyrOffset[2] & 0x00FF;  // lower Gz byte
		if (!writeRegister(UB4_REG_OFFSET_USER3, reg)) break;

		reg = _GyrOffset[0] & 0x00FF;  // lower Gx byte
		if (!writeRegister(UB4_REG_OFFSET_USER0, reg)) break;


		reg = (_AccOffset[0] & 0x0F00) >> 4 | (_GyrOffset[2] & 0x0F00) >> 8;  // upper Ax and Gz bytes
		if (!writeRegister(UB4_REG_OFFSET_USER4, reg)) break;

		reg = (_AccOffset[2] & 0x0F00) >> 4 | (_AccOffset[1] & 0x0F00) >> 8;  // upper Az and Ay bytes
		if (!writeRegister(UB4_REG_OFFSET_USER7, reg)) break;

		reg = (_GyrOffset[1] & 0x0F00) >> 4 | (_GyrOffset[0] & 0x0F00) >> 8;  // upper Gy and Gx bytes
		if (!writeRegister(UB4_REG_OFFSET_USER1, reg)) break;

		setBank(0);
		rv = true;
		} while (false);

	return rv;
}

/* Set Gyro and Accel Offsets individually*/
bool ICM42688::setAccXOffset(int16_t accXoffset) {

	bool rv = false;
	do	{
		setBank(4);
		uint8_t reg1 = (accXoffset & 0x00FF);
		uint8_t reg2;
		if (!readRegisters(UB4_REG_OFFSET_USER4, 1, &reg2)) break;

		reg2 = (reg2 & 0x0F) | ((accXoffset & 0x0F00) >> 4);
		if (!writeRegister(UB4_REG_OFFSET_USER5, reg1)) break;

		if (!writeRegister(UB4_REG_OFFSET_USER4, reg2)) break;

		setBank(0);

		rv = true;
		} while (false);
	return rv;
}



bool ICM42688::setAccYOffset(int16_t accYoffset) {

	bool rv = false;
	do	{

		setBank(4);
		uint8_t reg1 = (accYoffset & 0x00FF);
		uint8_t reg2;
		if (!readRegisters(UB4_REG_OFFSET_USER7, 1, &reg2)) break;
		reg2 = (reg2 & 0xF0) | ((accYoffset & 0x0F00) >> 8);
		if (!writeRegister(UB4_REG_OFFSET_USER6, reg1)) break;
		if (!writeRegister(UB4_REG_OFFSET_USER7, reg2)) break;
		setBank(0);
		rv = true;
		} while (false);
	return rv;
}


bool ICM42688::setAccZOffset(int16_t accZoffset) {

	bool rv = false;
	do	{
		setBank(4);
		uint8_t reg1 = accZoffset & 0x00FF;
		uint8_t reg2;
		if (!readRegisters(UB4_REG_OFFSET_USER7, 1, &reg2)) break;
		reg2 = (reg2 & 0x0F) | ((accZoffset & 0x0F00) >> 4);
		if (!writeRegister(UB4_REG_OFFSET_USER8, reg1)) break;

		if (!writeRegister(UB4_REG_OFFSET_USER7, reg2)) break;

		setBank(0);
		rv = true;
		} while (false);

	return rv;
}

bool ICM42688::setGyrXOffset(int16_t gyrXoffset) {
	bool rv = false;

	do	{
		setBank(4);
		uint8_t reg1 = gyrXoffset & 0x00FF;
		uint8_t reg2;
		if (!readRegisters(UB4_REG_OFFSET_USER1, 1, &reg2)) break;

		reg2 = (reg2 & 0xF0) | ((gyrXoffset & 0x0F00) >> 8);
		if (!writeRegister(UB4_REG_OFFSET_USER0, reg1)) break;
		if (!writeRegister(UB4_REG_OFFSET_USER1, reg2)) break;

		setBank(0);
		rv = true;
		} while (false);

	return rv;
}

bool ICM42688::setGyrYOffset(int16_t gyrYoffset) {
	bool rv = false;

	do	{
		setBank(4);
		uint8_t reg1 = gyrYoffset & 0x00FF;
		uint8_t reg2;
		if (!readRegisters(UB4_REG_OFFSET_USER1, 1, &reg2)) break;

		reg2 = (reg2 & 0x0F) | ((gyrYoffset & 0x0F00) >> 4);
		reg2 = (gyrYoffset & 0x0F00) >> 4 | (reg2 & 0x0F00) >> 4;
		if (!writeRegister(UB4_REG_OFFSET_USER2, reg1)) break;

		if (!writeRegister(UB4_REG_OFFSET_USER1, reg2)) break;

		setBank(0);

		rv = true;
		} while (false);

	return rv;

}

bool ICM42688::setGyrZOffset(int16_t gyrZoffset) {

	bool rv = false;

	do	{
		setBank(4);
		uint8_t reg1 = gyrZoffset & 0x00FF;
		uint8_t reg2;
		if (!readRegisters(UB4_REG_OFFSET_USER4, 1, &reg2)) break;
		reg2 = (reg2 & 0xF0) | ((gyrZoffset & 0x0F00) >> 8);
		if (!writeRegister(UB4_REG_OFFSET_USER3, reg1)) break;
		if (!writeRegister(UB4_REG_OFFSET_USER4, reg2)) break;
		setBank(0);

		rv = true;
		} while (false);

	return rv;

}

int ICM42688::setUIFilterBlock(UIFiltOrd gyroUIFiltOrder, UIFiltOrd accelUIFiltOrder) {
	return 1;
}

bool ICM42688::setGyroNotchFilter(float gyroNFfreq_x, float gyroNFfreq_y, float gyroNFfreq_z, GyroNFBWsel gyro_nf_bw) {

	bool rv = false;


	do	{

		setBank(3);
		// get clock div
		uint8_t reg;
		if (!readRegisters(UB0_REG_GYRO_CONFIG0, 1, &reg)) break;

		uint8_t clkdiv = reg & 0x3F;
		setBank(1);
		uint16_t nf_coswz;
		//uint8_t nf_coswz_sel = 0;
		uint8_t     gyro_nf_coswz_low[3] = {0};
		uint8_t     buff                 = 0;
		float       Fdrv                 = 19'200 / (clkdiv * 10.0f);          // in kHz  (19.2MHz = 19200 kHz)
		const float fdesired[3] = {gyroNFfreq_x, gyroNFfreq_y, gyroNFfreq_z};  // in kHz - fesdeired between 1kz and 3 kHz
		// float coswz = 0;
		for (size_t ii = 0; ii < 3; ii++) {
			float coswz = cos(2 * M_PI * fdesired[ii] / Fdrv);
			if (coswz <= 0.875) {
				nf_coswz              = (uint16_t)round(coswz * 256);
				gyro_nf_coswz_low[ii] = (uint8_t)(nf_coswz & 0x00FF);    //take lower part
				buff = buff & (((nf_coswz & 0xFF00) >> 8) << ii);        //take upper part and concatenate in the buffer
			} else {
				buff = buff & (1 << (3 + ii));                           //nf_coswz_sel =  nf_coswz_sel & (1<<(3+ii));
				if (coswz > 0.875) {
					nf_coswz              = (uint16_t)round(8 * (1 - coswz) * 256);
					gyro_nf_coswz_low[ii] = (uint8_t)(nf_coswz & 0x00FF);  //take lower part
					buff = buff & (((nf_coswz & 0xFF00) >> 8) << ii);      //take upper part and concatenate in the buffer
				} else if (coswz < -0.875) {
					nf_coswz              = (uint16_t)round(-8 * (1 - coswz) * 256);
					gyro_nf_coswz_low[ii] = (uint8_t)(nf_coswz & 0x00FF);  //take lower part
					buff = buff & (((nf_coswz & 0xFF00) >> 8) << ii);      //take upper part and concatenate in the buffer}
				}
			}
		}
		// write to the Registers
		if (!writeRegister(UB1_REG_GYRO_CONFIG_STATIC6, gyro_nf_coswz_low[0])) break;
		if (!writeRegister(UB1_REG_GYRO_CONFIG_STATIC7, gyro_nf_coswz_low[1])) break;
		if (!writeRegister(UB1_REG_GYRO_CONFIG_STATIC8, gyro_nf_coswz_low[2])) break;
		if (!writeRegister(UB1_REG_GYRO_CONFIG_STATIC9, buff)) break;
		if (!writeRegister(UB1_REG_GYRO_CONFIG_STATIC10, gyro_nf_bw)) break;

		//Set Bank 0 to allow data measurements
		setBank(0);

		rv = true;
		} while (false);

	return rv;
}

/* for testing only*/
int ICM42688::testingFunction() {
	return 1;
}

/* Get Resolution FullScale */
float ICM42688::getAccelRes() {  // read  ACCEL_CONFIG0 and get ACCEL_FS_SEL value
	int   currentAccFS = getAccelFS();
	float accRes;
	switch (currentAccFS) {
	case ICM42688::AccelFS::gpm2:
		accRes = 16.0f / (32768.0f);
		break;
	case ICM42688::AccelFS::gpm4:
		accRes = 4.0f / (32768.0f);
		break;
	case ICM42688::AccelFS::gpm8:
		accRes = 8.0f / (32768.0f);
		break;
	case ICM42688::AccelFS::gpm16:
		accRes = 16.0f / (32768.0f);
		break;
	}
	return accRes;
}

/* Get Resolution FullScale */
float ICM42688::getGyroRes() {
	int   currentGyroFS = getGyroFS();
	float gyroRes;
	switch (currentGyroFS) {
	case ICM42688::GyroFS::dps2000:
		gyroRes = 2000.0f / 32768.0f;
		break;
	case ICM42688::GyroFS::dps1000:
		gyroRes = 1000.0f / 32768.0f;
		break;
	case ICM42688::GyroFS::dps500:
		gyroRes = 500.0f / 32768.0f;
		break;
	case ICM42688::GyroFS::dps250:
		gyroRes = 250.0f / 32768.0f;
		break;
	case ICM42688::GyroFS::dps125:
		gyroRes = 125.0f / 32768.0f;
		break;
	case ICM42688::GyroFS::dps62_5:
		gyroRes = 62.5f / 32768.0f;
		break;
	case ICM42688::GyroFS::dps31_25:
		gyroRes = 31.25f / 32768.0f;
		break;
	case ICM42688::GyroFS::dps15_625:
		gyroRes = 15.625f / 32768.0f;
		break;
	}
	return gyroRes;
}

/* Self Test*/
int ICM42688::selfTest() {
	return 1;
}


void ICM42688::Task (TI2CBUS *i2cobj)
{
	if (!i2cobj) return;
	i2c_bus = i2cobj;

	switch (sw_state)
		{
		case EIMUSW_INIT:
			{
			if (begin ())
				{
				PermitCount = 1000;
				sw_state = EIMUSW_READ;
				}
			else
				{
				SetBussError_i2c ();
				sw_state = EIMUSW_ERROR;
				}
			break;
			}
		case EIMUSW_READ:
			{
			if (!getAGT ())
				{
				SetBussError_i2c ();
				sw_state = EIMUSW_ERROR;
				}
			else
				{
				if (PermitCount) PermitCount--;
				}
			break;
			}
		case EIMUSW_ERROR:
			{
			// break not need !
			}
		case EIMUSW_COMPLETE:
			{
			F_complete = true;
			break;
			}
		default: sw_state = EIMUSW_INIT; break;
		}
}



void ICM42688::Start ()
{
	if (is_soft_reset ())
		{
		sw_state = EIMUSW_INIT;
		}
	else
		{
		if (!PermitCount)
			{
			sw_state = EIMUSW_INIT;
			}
		else
			{
			sw_state = EIMUSW_READ;
			}
		}
	F_complete = false;
}


/*//#TODO
//High priority
raw data reading          <-- Validated
Offset Bias compute       <-- Validated
Offset Bias set           <-- Validated
get Full scale resolution <-- To be tested
Notch Filter              <-- To be tested
AAF Filter                <-- To be developed
UI Filter Block           <-- To be developed
Self test                 <-- To be developed



//Low priority
Read INT_STATUS          <-- get info if data are available

ApexStatus   => INT_STATUS2 and INT_STATUS3
8.1 APEX ODR SUPPORT
8.2 DMP POWER SAVE MODE
8.3 PEDOMETER PROGRAMMING
8.4 TILT DETECTION PROGRAMMING
8.5 RAISE TO WAKE/SLEEP PROGRAMMING
8.6 TAP DETECTION PROGRAMMING
8.7 WAKE ON MOTION PROGRAMMING
8.8 SIGNIFICANT MOTION DETECTION PROGRAMMING  p47
*/
