#include "TMPU6050.h"
#include "SYSBIOS.H"
#include <math.h>



//#define C_MPUOFFSET_VAL 2047



TI2CIFACE *TMPU6050::i2_object = 0;
TMPU6050 *TMPU6050::singltn = 0;



TMPU6050::TMPU6050 ()
{
		smooth_acc_angles[X_AXIS] = 0; smooth_acc_angles[Y_AXIS] = 0; smooth_acc_angles[Z_AXIS] = 0;
		c_filter_level = 0;
}



void TMPU6050::set_i2c (TI2CIFACE *ob)
{
	i2_object = ob;
	i2_object->Hard_Init ();
	obj()->mpuInit (i2_object);
}



/*
void TMPU6050::getorient_offset (S_MPUOFFSET_T *dst)
{
char ix = 0;
short dat;
while (ix <= 2)
	{
	dat = accel_data.u.array[ix];
	// тут можно прибавить калибровочное значения для осей, а так же сделать привязку осей X,Y,Z к ROLL, PITCH, YAW ориентациям.
	dst->u.array[ix] = dat;
	ix++;
	}
}
*/

//static float AccAngle[EULAR];
//static float heading_x;
//static float heading_y;
//static float heading_z;


// переводит значения ориентации акселерометра в величины угла 0-360 
bool TMPU6050::get_angles (float * dst)
{
	bool rv = false;
	float accrawsmooth[ENDENUM_AXIS];
	float anglarr[ENDENUM_AXIS];
	float tmp_angl, dlt_angl;

	char ix = 0;
	if (GetRawAccel_sync (accrawsmooth))
		{
    anglarr[X_AXIS] = (-atan2((double)accrawsmooth[Y_AXIS], (double)accrawsmooth[X_AXIS])) * 180.0F/M_PI + 180;
		anglarr[Y_AXIS] = (atan2((double)accrawsmooth[Z_AXIS], (double)accrawsmooth[X_AXIS])) * 180.0F/M_PI + 180;
		anglarr[Z_AXIS] = (atan2((double)accrawsmooth[X_AXIS], (double)accrawsmooth[Z_AXIS])) * 180.0F/M_PI + 180;
		while (ix < ENDENUM_AXIS)
			{
			tmp_angl = anglarr[ix];
			while (tmp_angl < 0) tmp_angl += 360;
			while (tmp_angl > 360) tmp_angl -= 360;
			dlt_angl = tmp_angl - smooth_acc_angles[ix];
			if (dlt_angl < 0)	dlt_angl *= -1.0F;
			if (dlt_angl < c_filter_level) 
				{
				tmp_angl = (tmp_angl + smooth_acc_angles[ix]) / 2;
				}
			smooth_acc_angles[ix] = tmp_angl;
			*dst++ = tmp_angl;
			ix++;
			}
		rv = true;
		}
	
/* переводит значения ориентации акселерометра в величины угла -90:0:90   (180)
//	AccAngle[ROLL]  = (-(atan(AccData.x / (sqrtf(AccData.z * AccData.z + AccData.y * AccData.y))))) * 180.0F/M_PI;   //Calculating roll ACC angle
//	AccAngle[PITCH] = (+(atan(AccData.y / (sqrtf(AccData.x * AccData.x + AccData.z * AccData.z))))) * 180.0F/M_PI;   //Calculating pitch ACC angle
//	AccAngle[YAW] = (+(atan(AccData.z / (sqrtf(AccData.x * AccData.x + AccData.y * AccData.y))))) * 180.0F/M_PI;   //Calculating yaw ACC angle
	
    // AccAngle[ROLL]  = -(atan2f(AccData.x, sqrtf(AccData.z * AccData.z + AccData.y * AccData.y))) * 180.0F/M_PI;;   //Calculating roll ACC angle
    // AccAngle[PITCH] = +(atan2f(AccData.y, sqrtf(AccData.x * AccData.x + AccData.z * AccData.z))) * 180.0F/M_PI;;   //Calculating pitch ACC angle
		// AccAngle[YAW] = +(atan2f(AccData.z, sqrtf(AccData.x * AccData.x + AccData.y * AccData.y))) * 180.0F/M_PI;;   //Calculating yaw ACC angle
	*/

return rv;
}


/*
bool TMPU6050::read_acc_smooth_filtering (float *dst)
{
	bool rv = false;
	float databuf[ENDENUM_AXIS];
	float dat, delt;
	if (GetRawAccel_sync (databuf))
		{
		char ix = 0;
		while (ix < ENDENUM_AXIS)
			{
			dat = databuf[ix];
			delt = smooth_acc_arr[ix] - dat;
			if (delt < 0) delt *= -1.0F;
			if (delt < c_filter_level) 
				{
				dat = (dat + smooth_acc_arr[ix]) / 2;	// diff range between cur and prev value is low - goto smooth procedure
				}
			smooth_acc_arr[ix] = dat;
			*dst++ = dat;
			ix++;
			}
		rv = true;
		}
	return rv;
}	
*/





TMPU6050 *TMPU6050::obj ()
{
	if (!singltn) singltn = new TMPU6050 ();
	return singltn;
}



TSENIMU_IF *TMPU6050::imuobj ()
{
	return obj ();
}




bool TMPU6050::WhoIam (TI2CIFACE *i2cobj, unsigned char &val)
{
	return i2cobj->ReadFrame (MPU_SLAVE_ADDR, MPU6050_RA_WHO_AM_I, &val, 1);
}



bool TMPU6050::mpuRegRead (TI2CIFACE *i2cobj, uint8_t regAddr, uint8_t *dstbyte)
{
return i2cobj->ReadFrame (MPU_SLAVE_ADDR, regAddr, dstbyte, 1);
}



bool TMPU6050::mpuRegWrite (TI2CIFACE *i2cobj, uint8_t regAddr, uint8_t data)
{
return i2cobj->WriteFrame (MPU_SLAVE_ADDR, regAddr, &data, 1);
}



bool TMPU6050::mpuInit (TI2CIFACE *i2cobj)
{
bool rv = false;
do	{
		// reset 
		if (!mpuRegWrite (i2cobj, MPU_REG_PWR_MGMT_1, 0x80)) break;
		SYSBIOS::Wait (150);
	
		if (!mpuRegWrite (i2cobj, MPU_REG_PWR_MGMT_1, MPU_CLK_SEL )) break;		// MPU_CLK_SEL 0x00 - alternate val = 3
		SYSBIOS::Wait (5);
		if (!mpuRegWrite (i2cobj, 0x6C, 0)) break;
		SYSBIOS::Wait (5);

		// set sampling rate
		if (!mpuRegWrite (i2cobj, MPU_REG_SMPLRT_DIV, MPU_SAMPLE_RATE_DIV )) break;		// MPU_SAMPLE_RATE_DIV 0x00
		SYSBIOS::Wait (5);
		// FSYNC and digital low pass filter settings
		if (!mpuRegWrite (i2cobj, MPU_REG_CONFIG,(MPU_EXT_SYNC | MPU_DLP_BW) )) break;
		SYSBIOS::Wait (5);
		// set gyroscope full scale range
		if (!mpuRegWrite (i2cobj, MPU_REG_GYRO_CONFIG, 0x00 )) break;		// MPU_GYRO_FS_RANGE 0x00
		SYSBIOS::Wait (5);
		// set accelerometer full scale range
		if (!mpuRegWrite (i2cobj, MPU_REG_ACCEL_CONFIG, 0x00)) break;		// MPU_ACC_FS_RANGE 0x00
		SYSBIOS::Wait (5);
		// MPU control functions
		// set interrupt clear option, FSYNC logic level, aux bus access
		if (!mpuRegWrite (i2cobj, MPU_REG_INT_PIN_CFG, 0x00)) break;		// (MPU_INT_STAT_CLEAR | MPU_FSYNC_LOGIC_LEVEL | MPU_I2C_BYPASS)
		SYSBIOS::Wait (5);
		// enable interrupts
		if (!mpuRegWrite (i2cobj, MPU_REG_INT_ENABLE, 0x00)) break;			// MPU_INT_ENABLE
		SYSBIOS::Wait (5);
		// configure MPU hardware FIFO
		//if (!mpuRegWrite (i2cobj, MPU_REG_USER_CTRL, 0x40)) break;
		//SYSBIOS::Wait (5);
		rv = true;
		} while (false);
return rv;
}



/*
bool TMPU6050::ReadGyroFrame (TI2CIFACE *i2cobj, S_MPUDATA_T *dst)
{
return i2cobj->ReadFrame (MPU_SLAVE_ADDR, MPU_REG_GYRO_XOUT_H, (uint8_t*)dst, sizeof(S_MPUDATA_T));
}



bool TMPU6050::ReadAccelFrame (TI2CIFACE *i2cobj, S_MPUDATA_T *dst)
{
return i2cobj->ReadFrame (MPU_SLAVE_ADDR, MPU_REG_ACCEL_XOUT_H, (uint8_t*)dst, sizeof(S_MPUDATA_T));
}
*/


bool TMPU6050::GetRawAccel_sync (float *dst)
{
	bool rv = false;
	uint8_t read[6];
	if (i2_object->ReadFrame (MPU_SLAVE_ADDR, MPU_REG_ACCEL_XOUT_H, read, sizeof(read)))
		{
		dst[X_AXIS] = (short)((read[0] << 8) | read[1]);
		dst[Y_AXIS] = (short)((read[2] << 8) | read[3]);
		dst[Z_AXIS] = (short)((read[4] << 8) | read[5]);
		rv = true;
		}
return rv;
}



void TMPU6050::filter_level (float val)
{
	c_filter_level = val;
}





/*
bool TMPU6050::GetRawGyro_sync (float *dst)
{
	bool rv = false;
	if (i2_object && dst) {
		uint8_t read[6];
		if (i2_object->ReadFrame (MPU_SLAVE_ADDR, MPU_REG_GYRO_XOUT_H, read, sizeof(read)))
			{
			float gyroScaleFactor = 7505.747116f;// 8000.0f;//     2.0F/131.0F * M_PI/180.0F;

			gyroADC_ROLL  = (short)((read[0] << 8) | read[1]);
			dst[X_AXIS] = ((float)gyroADC_ROLL - gyroADC_ROLL_offset) / gyroScaleFactor;
			// GyroData[X_AXIS] = ((float)gyroADC_ROLL  - gyroADC_ROLL_offset)  * gyroScaleFactor;

			gyroADC_PITCH = (short)((read[2] << 8) | read[3]);
			dst[Y_AXIS] = ((float)gyroADC_PITCH - gyroADC_PITCH_offset) / gyroScaleFactor;
			// GyroData[Y_AXIS] = ((float)gyroADC_PITCH - gyroADC_PITCH_offset) * gyroScaleFactor;

			gyroADC_YAW   = (short)((read[4] << 8) | read[5]);
			dst[Z_AXIS] = ((float)gyroADC_YAW - gyroADC_YAW_offset) / gyroScaleFactor;
			// GyroData[Z_AXIS] = ((float)gyroADC_YAW   - gyroADC_YAW_offset)   * gyroScaleFactor;
			rv = true;
			}
		}
return rv;
}
*/


/*
void TMPU6050::Task (TI2CIFACE *i2cobj)
{
	if (ReleaseTimeout) return;
	
	switch (DevSw)
		{
		case E_MPUWSW_INIT:
			{
			i2cobj->Hard_Init ();
			if (!mpuInit (i2cobj)) 
				{
				BusErrorCount_i2c++; 
				DevSw = E_MPUWSW_BUSSERROR;	
				}
			else
				{
				unsigned char val;
				if (WhoIam (i2cobj, val))
					{
					//if (!f_is_calibrated) gyro_calibrate (i2cobj);
					ReleaseTimeout = 100;					
					DevSw = E_MPUWSW_STATUS;			
					RDYTimeout = 100;								// 100 ms RDY					
					ReadNormalCounter = 100000;
					}
				}
			break;
			}
		case E_MPUWSW_STATUS:
			{
			unsigned char val;
			if (WhoIam (i2cobj, val))
				{
				if (val == 0x68)
					{
					DevSw = E_MPUWSW_READACCEL;
					}
				else
					{
					BusErrorCount_i2c++; 
					DevSw = E_MPUWSW_BUSSERROR;	
					}
				}
			else
				{
				BusErrorCount_i2c++;
				DevSw = E_MPUWSW_BUSSERROR;		
				}
			break;
			}
		case E_MPUWSW_READACCEL:
			{
			S_MPUDATA_T raw;
			if (!ReadAccelFrame (i2cobj, &raw)) 
				{
				BusErrorCount_i2c++; 
				DevSw = E_MPUWSW_BUSSERROR;	
				}
			else
				{
				accel_data.u.xyz.x = (short)((uint16_t)raw.buf[0] << 8) | raw.buf[1];
				accel_data.u.xyz.y = (short)((uint16_t)raw.buf[2] << 8) | raw.buf[3];
				accel_data.u.xyz.z = (short)((uint16_t)raw.buf[4] << 8) | raw.buf[5];
				//accraw = accel_data;
				DevSw = E_MPUWSW_READGYRO;
				}
			break;
			}
		case E_MPUWSW_READGYRO:
			{
			S_MPUDATA_T rawgyro;
			if (!ReadGyroFrame (i2cobj, &rawgyro)) 
				{
				BusErrorCount_i2c++; 
				DevSw = E_MPUWSW_BUSSERROR;	
				}
			else
				{
				if (ReadNormalCounter) ReadNormalCounter--;					
				short tmp;
				tmp = (short)((uint16_t)rawgyro.buf[0] << 8) | rawgyro.buf[1];
				gyro_data.u.xyz.x = tmp;					
				tmp = (short)((uint16_t)rawgyro.buf[2] << 8) | rawgyro.buf[3];
				gyro_data.u.xyz.y = tmp;					
				tmp = (short)((uint16_t)rawgyro.buf[4] << 8) | rawgyro.buf[5];
				gyro_data.u.xyz.z = tmp;
					

					
				//gyroraw = gyro_data;
				f_actual_data = true;
				DevSw = E_MPUWSW_RELEASE;
				}
			break;
			}
		case E_MPUWSW_BUSSERROR:
			{
			f_actual_data = false;
			//if (BusErrorCount_i2c) SetBussError ();	// ???????? ?????????? ?? ??????
			DevSw = E_MPUWSW_RELEASE;
			// ??? break ?? ?????
			}
		case E_MPUWSW_RELEASE:
			{
			F_complete = true;
			break;
			}
		default:
			{
			DevSw = E_MPUWSW_INIT;
			break;
			}
		}
}
*/


/*
void TMPU6050::gyro_calibrate (TI2CIFACE *i2cobj)
{
			S_MPUDATA_T rawgyro;
			long loops = 150, cnt = 0, seq_err = 0;
			gyroADC_ROLL_offset = 0;
			gyroADC_PITCH_offset = 0;
			gyroADC_YAW_offset = 0;
			SYSBIOS::Wait (2);
			while (cnt < loops)
				{
				if (ReadGyroFrame (i2cobj, &rawgyro)) 
					{
					seq_err = 0;
					gyroADC_ROLL = (short)((uint16_t)rawgyro.buf[0] << 8) | rawgyro.buf[1];
					gyroADC_PITCH = (short)((uint16_t)rawgyro.buf[2] << 8) | rawgyro.buf[3];
					gyroADC_YAW = (short)((uint16_t)rawgyro.buf[4] << 8) | rawgyro.buf[5];
					gyroADC_ROLL_offset += gyroADC_ROLL;
					gyroADC_PITCH_offset += gyroADC_PITCH;
					gyroADC_YAW_offset += gyroADC_YAW;
					}
				else
					{
					seq_err++;
					if (seq_err >= 3) break;
					}
				SYSBIOS::Wait (2);
				cnt++;
				}
			if (seq_err < 3)
				{
				gyroADC_ROLL_offset  /= loops;
				gyroADC_PITCH_offset /= loops;
				gyroADC_YAW_offset   /= loops;
				f_is_calibrated = true;
				}
	SYSBIOS::Wait (5);
}
*/





/*
void TMPU6050::Start ()
{
	if (!BusErrorCount_i2c)
		{
		if (ReadNormalCounter)
			{
			RDYTimeout = 5;	
			DevSw = E_MPUWSW_STATUS;
			}
		else
			{
			// counter normal read is out
			DevSw = E_MPUWSW_INIT;	
			}
		}
	else
		{
		// i2c errors detected
		DevSw = E_MPUWSW_INIT;
		BusErrorCount_i2c = 0;
		}
F_complete = false;
}
*/
		

