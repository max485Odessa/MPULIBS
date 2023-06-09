#ifndef _H_I2CMULTI_DISPATCH_H_
#define _H_I2CMULTI_DISPATCH_H_

#include "I2CSOFTWARE.H"
#include "SYSBIOS.H"
#include "STMSTRING.h"



const unsigned char C_MAXI2CDEV_AMOUNT = 3;
extern bool check_firmware_status ();



// ��������� I2C ������� ��� ���������� (��� ������: CheckComplete, Task, Start)
class TI2CTRDIFACE {
	private:
		//bool F_timer_registered;
		bool f_need_ic_soft_reset;							// ���� ���������� ��� �� ���������� ������� start - ����� ���������� soft_reset � �������������
	
	protected:
		const char *debug_chip_name;
		utimer_t ReleaseTimeout;
		bool F_complete;
		void SetBussError_i2c ();								// ���� ���������� ���������� ������ � ������� �� ������� I2C - ���������� ���� �����
		void SetBussError_chip ();							// ���� ���������� ���������� ������ � ������� �� ������� ������ ������������ ���������� - ���������� ���� �����
		void SetBussError_meas ();							// ���� ���������� ���������� ������ � ������� �� ������� ��������� - ���������� ���� �����
	
		unsigned long PermitCount;
		unsigned long BusErrorCount_i2c;
		unsigned long BusErrorCount_chip;
		unsigned long BusErrorCount_measure;
	
		bool f_buss_error;	
		bool f_chip_error;
		bool f_meass_error;
		bool is_soft_reset ();
	
	public:
		TI2CTRDIFACE ();
		//void Init ();
		bool CheckComplete ();
		bool CheckError ();
		void ClearError ();
		unsigned long get_i2c_error_cnt ();
		unsigned long get_chip_error_cnt ();
		unsigned long get_measure_error_cnt ();
	
		const char *get_device_name ();
		virtual void Task (TI2CIFACE *i2cobj) = 0;				// ��������� ��������� �������� ���� ����� ��� ���������� ������ �������� �����������
		virtual void Start () = 0;			// ��������� �������� ���������� �� ������ ������
};



// ��� ���������
class TDISPATCHI2C {
	TI2CIFACE *I2C_device;
	
	TI2CTRDIFACE *debug_object;
	utimer_t ReleaseTimeout;
	TI2CTRDIFACE *DevArray[C_MAXI2CDEV_AMOUNT];
	unsigned long DevCount;
	unsigned long CurDevIndx;
	void CircleIndexControl ();
	
	public:
		TDISPATCHI2C (TI2CIFACE *I2C_dv);
		bool Add (TI2CTRDIFACE *dev_iface);				// ����� ��� ���������� � ��������� ���������� � TI2CTRDIFACE �����������
		void Task ();															// task ����������
};



#endif
