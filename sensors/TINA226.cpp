#include "TINA226.h"
#include <math.h>
#include <stdio.h>


TTINA226::TTINA226 (uint8_t adr, TI2CIFACE *cc_i2c)
{
	i2c_adress = adr;
	i2c = cc_i2c;
	configure (INA226_AVERAGES_1, INA226_BUS_CONV_TIME_1100US, INA226_SHUNT_CONV_TIME_1100US, INA226_MODE_SHUNT_BUS_CONT);		// INA226_MODE_SHUNT_BUS_CONT
	relax_time = 0;
	SYSBIOS::ADD_TIMER_SYS (&relax_time);
}





bool TTINA226::writeRegister16 (uint8_t reg, uint16_t val)
{
	uint8_t dat_dst[2];
	dat_dst[0] = val >> 8;
	dat_dst[1] = (uint8_t)val;

	return i2c->WriteFrame (i2c_adress, reg, dat_dst, 2);
}



bool TTINA226::readRegister(uint8_t SrcReg, uint16_t &dst)
{
	uint8_t dat_dst[2];
	bool rv = i2c->ReadFrame (i2c_adress, SrcReg, dat_dst, 2);
	if (rv) {
		uint16_t value = dat_dst[0]; value <<= 8;
		value |= dat_dst[1];
		dst = value;
		}
	return rv;
}



bool TTINA226::getManufacturerID (uint16_t &id)
{
	bool rv = false;
	uint16_t data = 0;
	if (readRegister (INA226_MANUFACTURER, data)) 
		{
		id = data;
		rv = true;
		}
  return rv;
}



bool TTINA226::getDieID (uint16_t &id)
{
	bool rv = false;
	uint16_t data = 0;
	if (readRegister (INA226_DIE_ID, data)) 
		{
		id = data;
		rv = true;
		}
  return rv;
}



bool TTINA226::configure (ina226_averages_t avg, ina226_busConvTime_t busConvTime, ina226_shuntConvTime_t shuntConvTime, ina226_mode_t mode)
{
    uint16_t config = 0;
    config |= (avg << 9 | busConvTime << 6 | shuntConvTime << 3 | mode);
    //vBusMax = 36;
    //vShuntMax = 0.08192f;
    writeRegister16(INA226_REG_CONFIG, config);

    return true;
}


/*
bool TTINA226::readShuntCurrent (float &dst)
{
	bool rv = false;
	uint16_t dat = 0;
	if (readRegister(INA226_REG_CURRENT, dat)) {
		dst = (_current_LSB * dat);
		rv = true;
		}
	return rv;
}
*/

bool TTINA226::readShuntVoltage(float &dst)
{
	bool rv = false;
	uint16_t dat = 0;
	if (readRegister(INA226_REG_SHUNTVOLTAGE, dat)) {
		dst = (dat * 2.5e-6);
		rv = true;
		}
	return rv;
}



bool TTINA226::readBusVoltage(float &dst)
{
	bool rv = false;
	uint16_t dat = 0;
	if (readRegister(INA226_REG_BUSVOLTAGE, dat)) {
		dst = 0.00125 * dat;
		rv = true;
		}
	return rv;
}


/*
bool TTINA226::calibrate(float rShuntValue, float iMaxExpected)
{
    uint16_t calibrationValue;
    rShunt = rShuntValue;

    float iMaxPossible, minimumLSB;

    iMaxPossible = vShuntMax / rShunt;

    minimumLSB = iMaxExpected / 32767;

    currentLSB = (uint16_t)(minimumLSB * 100000000);
    currentLSB /= 100000000;
    currentLSB /= 0.0001;
    currentLSB = ceil(currentLSB);
    currentLSB *= 0.0001;

    powerLSB = currentLSB * 25;

    calibrationValue = (uint16_t)((0.00512) / (currentLSB * rShunt));

    writeRegister16(INA226_REG_CALIBRATION, calibrationValue);

    return true;
}
*/

static float dbg_vbus;
static float dbg_vshunt;
static float dbg_current;

float TTINA226::current ()
{
	return c_shunt;
}



void TTINA226::Task ()
{
if (relax_time) return;
	switch (sw)
		{
		case EINAMOD_VSHUNT:
			{
			readShuntVoltage (v_shunt);
			c_shunt = v_shunt / 0.02;
			//dst /= 0.02;
			
			dbg_vshunt = v_shunt;
			sw = EINAMOD_VBUS;
			relax_time = C_INARELAX_TIME;
			break;
			}
		case EINAMOD_VBUS:
			{
			readBusVoltage (v_bus);
			dbg_vbus = v_bus;
			sw = EINAMOD_VSHUNT;
			relax_time = C_INARELAX_TIME;
			break;
			}
		/*
		case EINAMOD_CSHUNT:
			{
			readShuntCurrent (c_shunt);
			dbg_current = c_shunt;
			sw = EINAMOD_VSHUNT;
			relax_time = C_INARELAX_TIME;
			break;
			}
		*/
		default:
			{
			sw = EINAMOD_VSHUNT;
			break;
			}
		}
}


