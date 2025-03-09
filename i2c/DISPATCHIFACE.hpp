#ifndef _H_I2CMULTI_DISPATCH_H_
#define _H_I2CMULTI_DISPATCH_H_


#include <stdint.h>
#include "I2CSOFTWARE.hpp"
#include "SYSBIOS.hpp"
#include "STMSTRING.hpp"



//const unsigned char C_MAXI2CDEV_AMOUNT = 3;
extern bool check_firmware_status ();



class TI2CTRDIFACE {
	private:
		//bool F_timer_registered;
		bool f_need_ic_soft_reset;
	
	protected:
		const uint8_t C_SLAVEADRESS;
		const char *debug_chip_name;
		utimer_t ReleaseTimeout;
		bool F_complete;
		void SetBussError_i2c ();
		void SetBussError_chip ();
		void SetBussError_meas ();
	
		unsigned long PermitCount;
		unsigned long BusErrorCount_i2c;
		unsigned long BusErrorCount_chip;
		unsigned long BusErrorCount_measure;
	
		bool f_buss_error;	
		bool f_chip_error;
		bool f_meass_error;
		bool is_soft_reset ();
		TI2CTRDIFACE (uint8_t adr);

	public:

		//void Init ();
		bool CheckComplete ();
		bool CheckError ();
		void ClearError ();
		unsigned long get_i2c_error_cnt ();
		unsigned long get_chip_error_cnt ();
		unsigned long get_measure_error_cnt ();
	
		const char *get_device_name ();
		virtual void Task (TI2CBUS *i2cobj) = 0;
		virtual void Start () = 0;
		virtual const char *GetChipName () = 0;
};




class TDISPATCHI2C {
	TI2CBUS *I2C_device;
	const uint8_t c_max_i2cdev;

	TI2CTRDIFACE *debug_object;
	utimer_t ReleaseTimeout;
	TI2CTRDIFACE **DevArray;
	unsigned long DevCount;
	unsigned long CurDevIndx;
	void CircleIndexControl ();
	
	public:
		TDISPATCHI2C (TI2CBUS *I2C_dv, uint8_t c_maxi2c);
		bool Add (TI2CTRDIFACE *dev_iface);				// ����� ��� ���������� � ��������� ���������� � TI2CTRDIFACE �����������
		void Task ();															// task ����������
};



#endif
