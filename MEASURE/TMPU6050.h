#ifndef _H_I2C_MPU_6050_H_
#define _H_I2C_MPU_6050_H_


#include "DISPATCHIFACE.H"
#include <stdint.h>
#include "definitions.h"

// MPU6050 I2C slave address
//#define MPU_SLAVE_ADDR (0xD2 >> 1)
#define MPU_SLAVE_ADDR 0x68
// MPU6050 register addresses
#define MPU_REG_SMPLRT_DIV  0x19
#define MPU_REG_CONFIG  0x1A
#define MPU_REG_GYRO_CONFIG  0x1B
#define MPU_REG_ACCEL_CONFIG  0x1C
#define MPU_REG_INT_PIN_CFG  0x37
#define MPU_REG_INT_ENABLE  0x38
#define MPU_REG_USER_CTRL  0x6A
#define MPU_REG_SIGNAL_PATH_RESET  0x68
#define MPU_REG_PWR_MGMT_1  0x6B
#define MPU_REG_ACCEL_XOUT_H  0x3B
#define MPU_REG_GYRO_XOUT_H  0x43
#define MPU_REG_TEMP_OUT_H  0x41
// MPU6050 configuration parameters
// sample rate divider, to set the sample rate of the sensor
#define MPU_SAMPLE_RATE_DIV  0x07 // to generate the desired Sample Rate for MPU
// external FSYNC pin sampling
#define MPU_EXT_SYNC  0
// digital low pass filter bandwidth
#define MPU_DLP_BW  0
// gyroscope full scale range
#define MPU_GYRO_FS_RANGE  0x18 // full scale range = ± 1000 °/s
// accelerometer full scale range
#define MPU_ACC_FS_RANGE  0x18 // full Scale Range = ± 16 °/s
// interrupt status bit clear by read operation
#define MPU_INT_STAT_CLEAR  0x10 // enable
// set FSYNC pin active logic level
#define MPU_FSYNC_LOGIC_LEVEL  0x80 // active low
// set aux I2C bus access for host
#define MPU_I2C_BYPASS  0x20 // enable
// enable interrupts
#define MPU_INT_ENABLE  0x59 // enabled interrupts: motion detection,
// FIFO overflow,
// I2C master,
// data ready
// clock selection
#define MPU_CLK_SEL  0 // internal 8MHz oscillator
// gyroscope scaling factor. This depends on MPU_GYRO_FS_RANGE
#define MPU_GYRO_SCALE_FACTOR  0.060975
// accelerometer scaling factor. This depends on MPU_ACC_FS_RANGE
#define MPU_ACC_SCALE_FACTOR  0.488281
#define MPU6050_RA_WHO_AM_I         0x75

enum E_MPUWSW {E_MPUWSW_INIT = 0, E_MPUWSW_STATUS = 1, E_MPUWSW_READACCEL = 2, E_MPUWSW_READGYRO = 3, E_MPUWSW_BUSSERROR = 4, E_MPUWSW_RELEASE = 5};


typedef struct {
	uint8_t buf[6];
} S_MPUDATA_T;


typedef struct {
	union {
				short array[3];
				struct {
								short x;
								short y;
								short z;
							 } xyz;
				} u;
} S_IMURAW_T;


typedef struct {
	union {
				short array[3];
				struct {
								short x;
								short y;
								short z;
							 } xyz;
				} u;
} S_MPUOFFSET_T;


typedef struct {
	union {
				short array[3];
				struct {
								short x;
								short y;
								short z;
							 } xyz;
				} u;
} S_MPUGYRO_T;



typedef struct {
	float x;
	float y;
	float z;
} S_IMUORIENT_T;		




typedef struct {
	float x;
	float y;
	float z;
} S_POSANGLS_T;		



typedef struct {
	float x;
	float y;
	float z;
} S_GYRACCDATA_T;	



class TSENIMU_IF {
	protected:
		//bool f_is_gyro_calibrated;
		//E_MPUWSW DevSw;
		//float gyroADC_ROLL_offset, gyroADC_PITCH_offset, gyroADC_YAW_offset;
		//short int gyroADC_PITCH, gyroADC_ROLL, gyroADC_YAW;
	
	public:

		//virtual bool GetGyro (float *dst) = 0;
		//virtual bool GetAccel (float *dst) = 0;
		virtual bool get_angles (float * dst) = 0;
		virtual void filter_level (float val) = 0;

	
		//void Get_Orientation(float *SmoothAcc, float *Orient, float *AccData, float *GyroData, float dt);
		//virtual bool GetOrientData (S_IMUORIENT_T * dst) = 0;
		//virtual bool GetAngle (S_POSANGLS_T * dst) = 0;
};



class TMPU6050: public TSENIMU_IF {		// public TI2CTRDIFACE,
	

		//unsigned long ReadNormalCounter;

		bool WhoIam (TI2CIFACE *i2cobj, unsigned char &val);
		//bool ReadStatus (TI2CIFACE *i2cobj, unsigned char &dat);
		//bool ReadGyroFrame (TI2CIFACE *i2cobj, S_MPUDATA_T *dst);
		//bool ReadAccelFrame (TI2CIFACE *i2cobj, S_MPUDATA_T *dst);
		
		//utimer_t RDYTimeout;
	
		bool mpuRegRead (TI2CIFACE *i2cobj, uint8_t regAddr, uint8_t *dstbyte);
		bool mpuRegWrite (TI2CIFACE *i2cobj, uint8_t regAddr, uint8_t data);
		
		S_IMURAW_T accel_data;		// S_GYRACCDATA_T
		//S_IMURAW_T gyro_data;		// S_GYRACCDATA_T
		//bool f_actual_data;
		//virtual void Start () ;
		//virtual void Task (TI2CIFACE *i2cobj) ;
	
		//void getorient_offset (S_MPUOFFSET_T *dst);
		
		// read, smooth and filtering
		float smooth_acc_angles[ENDENUM_AXIS];
		float c_filter_level;
		/*
		bool read_acc_smooth_filtering (float *dst);
		*/
		

		TMPU6050 ();
		static TMPU6050 *singltn;
		//virtual bool GetGyro (float *dst);
		//virtual bool GetAccel (float *dst) ;
		//virtual bool GetOrientData (S_IMUORIENT_T * dst);
		virtual bool get_angles (float *dst);
		
		
		virtual bool GetRawAccel_sync (float *dst);
		//virtual bool GetRawGyro_sync (float *dst);
		static TI2CIFACE *i2_object;
			

			bool mpuInit (TI2CIFACE *i2cobj);

			
			virtual void filter_level (float val);

	public:
		static void set_i2c (TI2CIFACE *obj);
		static TMPU6050 *obj ();
		static TSENIMU_IF *imuobj ();
};


#endif
