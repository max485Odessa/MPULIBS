#include "SPL06-001.hpp"
#include <math.h>



static const char *chipname = "spl06";



SPL06::SPL06 (uint8_t slv_adr) : TI2CTRDIFACE (slv_adr)
{
    sw_state = EALTIMSW_INIT;
    relax_tim.set(0);
}




// after begin need 100 ms waits
bool SPL06::begin ()
{
	bool rv = false;
	uint8_t data;
	do	{
		if (!readSensorID (data)) break;
		if (data != 0x01) break;

		if (!readCoefficients ()) break;
		if (!setSampling ()) break;

		rv = true;
		} while (false);

return rv;
}



bool SPL06::setSampling(sensor_mode mode,
                        sensor_sampling tempSampling,
                        sensor_sampling pressSampling,
                        sensor_rate tempRate,
                        sensor_rate pressRate)
{

bool rv = false;
do	{

    _measReg.mode = mode;

    if (pressRate > RATE_X8)
        _configReg.prs_shift_en = 1;
    else
        _configReg.prs_shift_en = 0;

    if (tempRate > RATE_X8)
        _configReg.tmp_shift_en = 1;
    else
        _configReg.tmp_shift_en = 0;

    _pressReg.pm_rate = pressRate;
    _pressReg.pm_prc = pressSampling;

    _tempReg.tmp_rate = tempRate;
    _tempReg.tmp_prc = tempSampling;


    if (!write8(SPL06_CFG_REG, _configReg.get())) break;
    if (!write8(SPL06_MEAS_CFG, _measReg.get())) break;
    if (!write8(SPL06_PSR_CFG, _pressReg.get())) break;
    if (!write8(SPL06_TMP_CFG, _tempReg.get())) break;

	rv = true;
	} while (false);
return rv;
}



void SPL06::reset(void) {
  write8(SPL06_SOFT_RESET, MODE_SOFT_RESET_CODE);
}




bool SPL06::getStatusFIFO (uint8_t &dst)
{
    return read8(SPL06_FIFO_STS, dst);
}




bool SPL06::write8 (byte reg, byte value)
{
    return i2c_bus->WriteFrame_i2c(C_SLAVEADRESS, reg, &value, 1);
}



bool SPL06::read8(byte reg, uint8_t &dst)
{
return i2c_bus->ReadFrame_i2c(C_SLAVEADRESS, reg, &dst, 1);
}



bool SPL06::readCoefficients()
{
	bool rv = false;
	do	{
		if (!get_c0 (_spl06_calib.c0)) break;
		if (!get_c1 (_spl06_calib.c1)) break;
		if (!get_coef(SPL06_COEF_C01a, SPL06_COEF_C01b, _spl06_calib.c01)) break;
		if (!get_coef(SPL06_COEF_C11a, SPL06_COEF_C11b, _spl06_calib.c11)) break;
		if (!get_coef(SPL06_COEF_C20a, SPL06_COEF_C20b, _spl06_calib.c20)) break;
		if (!get_coef(SPL06_COEF_C21a, SPL06_COEF_C21b, _spl06_calib.c21)) break;
		if (!get_coef(SPL06_COEF_C30a, SPL06_COEF_C30b, _spl06_calib.c30)) break;
		if (!get_c00(_spl06_calib.c00)) break;
		if (!get_c10(_spl06_calib.c10)) break;

		rv = true;
		} while (false);
	return rv;
}



//bool readSensorID(byte reg, int8_t &dat);

bool SPL06::readSensorID (uint8_t &dat)
{
bool rv = false;
uint8_t tmp;
do	{
	if (!read8(SPL06_DEVICE_ID, tmp)) break;
    dat = (tmp & 0xF0) >> 4;
	rv = true;
	} while (false);
return rv;
}



bool SPL06::get_c0 (int16_t &dst)
{
	bool rv = false;
	uint8_t tmp_MSB, tmp_LSB;
	int16_t tmp;
	do	{
	    //datasheet table 10 page 28
	    if (!read8(SPL06_COEF_C0, tmp_MSB)) break;
	    if (!read8(SPL06_COEF_C0C1, tmp_LSB)) break;

	    tmp_LSB = tmp_LSB >> 4;
	    tmp = (tmp_MSB << 4) | tmp_LSB;

	    if(tmp & (1 << 11)) // Check for 2's complement negative number
	        tmp = tmp | 0XF000; // Set left bits to one for 2's complement conversion of negative number
	    dst = tmp;
		rv = true;
		} while (false);
	return rv;
}



bool SPL06::get_c1 (int16_t &dst)
{
	bool rv = false;
	uint8_t tmp_MSB, tmp_LSB;
	int16_t tmp;
	do	{
	    //datasheet table 10 page 28
	    if (!read8(SPL06_COEF_C0C1, tmp_MSB)) break;
	    if (!read8(SPL06_COEF_C1, tmp_LSB)) break;

	    tmp_MSB = tmp_MSB & 0XF;

	    tmp = (tmp_MSB << 8) | tmp_LSB;

	    if(tmp & (1 << 11)) // Check for 2's complement negative number
	        tmp = tmp | 0XF000; // Set left bits to one for 2's complement conversion of negative number
	    dst = tmp;
		rv = true;
		} while (false);
	return rv;
}



bool SPL06::get_c00 (int32_t &dst)
{
	bool rv = false;
	uint8_t tmp_MSB, tmp_LSB, tmp_XLSB;
	int32_t tmp;
	do	{
	    //datasheet table 10 page 28
	    if (!read8(SPL06_COEF_C00a, tmp_MSB)) break;
	    if (!read8(SPL06_COEF_C00b, tmp_LSB)) break;
	    if (!read8(SPL06_COEF_C00C10, tmp_XLSB)) break;

	    tmp_XLSB = tmp_XLSB >> 4; //only keep the first 4 bits

	    tmp = (tmp_MSB << 8) | tmp_LSB;
	    tmp = (tmp << 4) | tmp_XLSB;

	    tmp = (uint32_t)tmp_MSB << 12 | (uint32_t)tmp_LSB << 4 | (uint32_t)tmp_XLSB >> 4;

	    if (tmp & (1 << 19))
	        tmp = tmp | 0xFFF00000; // Set left bits to one for 2's complement conversion of negative number
	    dst = tmp;
		rv = true;
		} while (false);
	return rv;
}




bool  SPL06::get_c10 (int32_t &dst)
{
	bool rv = false;
	uint8_t tmp_MSB, tmp_LSB, tmp_XLSB;
	int32_t tmp;
	do	{
	    //datasheet table 10 page 28
	    if (!read8(SPL06_COEF_C00C10, tmp_MSB)) break;
	    if (!read8(SPL06_COEF_C10a, tmp_LSB)) break;
	    if (!read8(SPL06_COEF_C10b, tmp_XLSB)) break;

	    tmp_MSB = tmp_MSB & 0B00001111; //only read the last 4 bits using a bit mask

	    tmp = (tmp_MSB << 4) | tmp_LSB;
	    tmp = (tmp << 8) | tmp_XLSB;

	    tmp = (uint32_t)tmp_MSB << 16 | (uint32_t)tmp_LSB << 8 | (uint32_t)tmp_XLSB;

	    if(tmp & (1 << 19))
	        tmp = tmp | 0XFFF00000; // Set left bits to one for 2's complement conversion of negative number
	    dst = tmp;
		rv = true;
		} while (false);
	return rv;
}


bool SPL06::get_coef(unsigned int line1, unsigned int line2, int16_t &dst)
{
	bool rv = false;
	uint8_t tmp_MSB, tmp_LSB;
	do	{
	    //datasheet table 10 page 28
	    if (!read8(line1, tmp_MSB)) break;
	    if (!read8(line2, tmp_LSB)) break;

	    dst = (tmp_MSB << 8) | tmp_LSB;
		rv = true;
		} while (false);
	return rv;

}



static const float factorarr[8] = {524288.0F, 1572864.0F, 3670016.0F, 7864320.0F, 253952.0F, 516096.0F, 1040384.0F, 2088960.0F};
//scaling factor depends on sampling rate that it is set at
bool SPL06::get_scale_factor(const uint8_t _regToRead, byte _bitToKeep, double &dst)
{
bool rv = false;
uint8_t tmp_Byte;
do	{
    //scale factor datasheet table 4 page 13
    if (!read8(_regToRead, tmp_Byte)) break;
    tmp_Byte = tmp_Byte & _bitToKeep; //filter out which bits to keep using provided bit mask
    if (tmp_Byte < 8)
    	{
    	dst = factorarr[tmp_Byte];
    	}
    else
    	{
    	dst = 0;
    	}

	rv = true;
	} while (false);
return rv;
}



bool SPL06::get_traw (int32_t &dst)
{
	bool rv = false;
	uint8_t tmp_MSB, tmp_LSB, tmp_XLSB;
	int32_t tmp;
	do	{
	    //raw temperature from the 24bit register; datasheet table 7 page 17

	    if (!read8(SPL06_TMP_B2, tmp_MSB)) break;
	    if (!read8(SPL06_TMP_B1, tmp_LSB)) break;
	    if (!read8(SPL06_TMP_B0, tmp_XLSB)) break;

	    tmp = (tmp_MSB << 8) | tmp_LSB;
	    tmp = (tmp << 8) | tmp_XLSB;

	    if(tmp & (1 << 23))
	        tmp = tmp | 0XFF000000; // Set left bits to one for 2's complement conversion of negative number

	    dst = tmp;
		rv = true;
		} while (false);
	return rv;
}



bool SPL06::get_traw_sc (double &dst)
{
	bool rv = false;
	double k;
	int32_t tt;
	do	{
		if (!get_scale_factor(SPL06_TMP_CFG, 0B00000111, k)) break;
		if (!get_traw (tt)) break;
		dst = (double)tt / k;
		rv = true;
		} while (false);
	return rv;
}



//value in Celcius
bool SPL06::readTemperature (double &dst)
{
	bool rv = false;
	double vald;
	do	{
	    if (!get_traw_sc (vald)) break;
	    dst = ( (double(_spl06_calib.c0) * 0.5d) + (double(_spl06_calib.c1) * vald) );
		rv = true;
		} while (false);
	return rv;
}



//value in Fahrenheit
bool SPL06::readTemperatureF (double &dst)
{
	bool rv = false;
	double vald;

	do	{
	    if (!get_traw_sc (vald)) break;
	    dst = (((double(_spl06_calib.c0) * 0.5d) + (double(_spl06_calib.c1) * vald)) * 9.0d/5.0d) + 32.0d;
		rv = true;
		} while (false);
	return rv;
}



bool SPL06::get_praw (int32_t &dst)
{
	bool rv = false;
	uint8_t tmp_MSB, tmp_LSB, tmp_XLSB;
	int32_t tmp;
	do	{
	    //raw pressure from the 24bit register; datasheet table 7 page 17

	    if (!read8(SPL06_PSR_B2, tmp_MSB)) break;
	    if (!read8(SPL06_PSR_B1, tmp_LSB)) break;
	    if (!read8(SPL06_PSR_B0, tmp_XLSB)) break;

	    tmp = (tmp_MSB << 8) | tmp_LSB;
	    tmp = (tmp << 8) | tmp_XLSB;

	    if(tmp & (1 << 23))
	        tmp = tmp | 0XFF000000; // Set left bits to one for 2's complement conversion of negative number

	    dst = tmp;
		rv = true;
		} while (false);
	return rv;
}



bool SPL06::get_praw_sc (double &dst)
{
	bool rv = false;
	double k;
	int32_t tt;
	do	{
		if (!get_scale_factor(SPL06_PSR_CFG, 0B00001111, k)) break;
		if (!get_praw (tt)) break;
		dst = (double)tt/k;
		rv = true;
		} while (false);
	return rv;
}


//in Pascal
bool SPL06::readPressure (double &dst)
{
	bool rv = false;
	double traw_sc, praw_sc;

	do	{
	    //compensated pressure, datasheet page 12
		int32_t c00 = _spl06_calib.c00;//notice the int type is 32
		int32_t c10 = _spl06_calib.c10;//notice the int type is 32
		int16_t c01 = _spl06_calib.c01;
		int16_t c11 = _spl06_calib.c11;
		int16_t c20 = _spl06_calib.c20;
		int16_t c21 = _spl06_calib.c21;
		int16_t c30 = _spl06_calib.c30;

		if (!get_traw_sc (traw_sc)) break;
		if (!get_praw_sc (praw_sc)) break;

		dst = ( double(c00) + praw_sc * (double(c10) + praw_sc * (double(c20) + praw_sc * double(c30))) + traw_sc * double(c01) + traw_sc * praw_sc * ( double(c11) + praw_sc * double(c21)) );

		rv = true;
		} while (false);
	return rv;
}



//unit is atmosphere
bool SPL06::readPressureATM (double &dst)
{
	double val;
	bool rv = readPressure (val);
	if (rv) dst = val / 101325.0d;
	return rv;
}



//unit is kiloPascal
bool SPL06::readPressureKPa(double &dst)
{
	double val;
	bool rv = readPressure (val);
	if (rv) dst = val / 1000.0d;
	return rv;
}




//unit is milli-bar
bool SPL06::readPressureMBar(double &dst)
{
	double val;
	bool rv = readPressure (val);
	if (rv) dst = val / 100.0d;
	return rv;
}


//unit is PSI
bool SPL06::readPressurePSI(double &dst)
{
	double val;
	bool rv = readPressure (val);
	if (rv) dst =  (val*0.0254d*0.0254d)/(0.45359237d * 9.80665d);
	return rv;
}
//mm of mercury
bool SPL06::readPressureMMHg(double &dst)
{
	double val;
	bool rv = readPressure (val);
	if (rv) dst = val / 133.322387415d;
	return rv;
}

//in of mercury
bool SPL06::readPressureINHg (double &dst)
{
	double val;
	bool rv = readPressure (val);
	if (rv) dst = (val/(25.4d*133.322387415d));
	return rv;
}


//estimated altitude in meters
//reference: https://www.weather.gov/media/epz/wxcalc/pressureAltitude.pdf
bool SPL06::readPressureAltitudeMeter (float &dst)
{
	const float altimeterSetting_mbar=1013.25f;
    double pressure_mbar;
    if (!readPressureMBar(pressure_mbar)) return false;
    dst = (0.3048d*(1.0d-pow((pressure_mbar/altimeterSetting_mbar),(1.0d/5.255d) ))*145366.45d);
    return true;
}


//estimated altitude in feet
//reference: https://www.weather.gov/media/epz/wxcalc/pressureAltitude.pdf
bool SPL06::readPressureAltitudeFeet(float &dst)
{
	const float altimeterSetting_inHg=29.921255f;
    double pressure_inHg;
    if (!readPressureINHg(pressure_inHg)) return false;
    dst = ((1.0d-pow((pressure_inHg/altimeterSetting_inHg),(1.0d/5.255d) ))*145366.45d);
    return true;
}



bool SPL06::update ()
{
	bool rv = false;
	do	{
		if (!readPressureKPa (result_pressure)) break;
		rv = true;
		} while (false);
	return rv;
}



bool SPL06::get_pressure_pascal (float &dst)
{
	if (f_rslt_ok) dst = result_pressure;
	return f_rslt_ok;
}



void SPL06::Start ()
{
	if (is_soft_reset ()) {
		sw_state = EALTIMSW_INIT;
		}
	else
		{
		if (!PermitCount) {
			sw_state = EALTIMSW_INIT;
			}
		else
			{
			sw_state = EALTIMSW_READ;
			}
		}
	F_complete = false;
}



void SPL06::Task (TI2CBUS *i2cobj)
{
	i2c_bus = i2cobj;
	if (!i2c_bus || relax_tim.get()) return;

	switch (sw_state)
		{
		case EALTIMSW_INIT:
			{
			if (begin ())
				{
				PermitCount = 10000;
				sw_state = EALTIMSW_READ;
				relax_tim.set(100);
				}
			else
				{
				SetBussError_chip ();
				sw_state = EALTIMSW_ERROR;
				relax_tim.set (500);
				}
			break;
			}
		case EALTIMSW_READ:
			{
			if (update ())
				{
				f_rslt_ok = true;
				if (PermitCount) PermitCount--;
				sw_state = EALTIMSW_COMPLETE;
				}
			else
				{
				f_rslt_ok = false;
				SetBussError_chip ();
				sw_state = EALTIMSW_ERROR;
				relax_tim.set (500);
				}
			break;
			}
		case EALTIMSW_ERROR:
			{
			sw_state = EALTIMSW_COMPLETE;
			// break not need
			}
		case EALTIMSW_COMPLETE:
			{
			F_complete = true;
			break;
			}
		default: sw_state = EALTIMSW_INIT; break;
		}
}



const char *SPL06::GetChipName ()
{
	return chipname;
}



/*
void SPL06_Temp::getSensor(sensor_t *sensor) {

  memset(sensor, 0, sizeof(sensor_t));


  strncpy(sensor->name, "SPL06-001", sizeof(sensor->name) - 1);
  sensor->name[sizeof(sensor->name) - 1] = 0;
  sensor->version = 1;
  sensor->sensor_id = _sensorID;
  sensor->type = SENSOR_TYPE_AMBIENT_TEMPERATURE;
  sensor->min_delay = 0;
  sensor->min_value = -40.0;
  sensor->max_value = +85.0;
  sensor->resolution = 0.5;
}



bool SPL06_Temp::getEvent(sensors_event_t *event) {

  memset(event, 0, sizeof(sensors_event_t));

  event->version = sizeof(sensors_event_t);
  event->sensor_id = _sensorID;
  event->type = SENSOR_TYPE_AMBIENT_TEMPERATURE;
  event->timestamp = millis();
  event->temperature = _theSPL06->readTemperature();
  return true;
}



void SPL06_Pressure::getSensor(sensor_t *sensor) {

  memset(sensor, 0, sizeof(sensor_t));


  strncpy(sensor->name, "SPL06-001", sizeof(sensor->name) - 1);
  sensor->name[sizeof(sensor->name) - 1] = 0;
  sensor->version = 1;
  sensor->sensor_id = _sensorID;
  sensor->type = SENSOR_TYPE_PRESSURE;
  sensor->min_delay = 0;
  sensor->min_value = 300.0;
  sensor->max_value = 1100.0;
  sensor->resolution = 1.0;
}



bool SPL06_Pressure::getEvent(sensors_event_t *event) {
  memset(event, 0, sizeof(sensors_event_t));

  event->version = sizeof(sensors_event_t);
  event->sensor_id = _sensorID;
  event->type = SENSOR_TYPE_PRESSURE;
  event->timestamp = millis();
  event->pressure = _theSPL06->readPressure() / 100.0; // convert Pa to hPa
  return true;
}
*/







