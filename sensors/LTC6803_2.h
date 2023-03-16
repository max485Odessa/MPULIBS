#ifndef _H_STM32F103_LTC6803_2_H_
#define _H_STM32F103_LTC6803_2_H_


//#include "spidispatch_class.h"
#include "sysbios.h"
#include "stdint.h"
#include "spiobj_class.h"

// ������ ��������� task
enum ESWLTC {ESWLTC_INIT = 0, ESWLTC_START = 1, ESWLTC_READ = 2, ESWLTC_COMPLETE = 3};
const unsigned char C_STCVAD = 0x10;
const unsigned char C_STCVAD_PEC = 0xB0;
const unsigned char WRCFG = 0x01;
const unsigned char WRCFG_PEC = 0xC7;


/* ----
���������� �� ����� ����� � ���������� 1 ��. ��� �������� �� ����� ���� � ������ Broadcast (��� ����� ��������� ���������� �� ����).
����������: ������� �����, ���� ���������� �� �����  = 0;
��� ������������� ���������� ���������, ����� ����� ������ ���


LTC ���������� ��������� ���������� - ������������ ������� CLK = 1 ���, (master mode baud rate prescalers (fPCLK/2 max.))
�� 72 ��� - ��� ����� 36 ���, ���� 36 ��� �������� �� ��������� �������� � ������� �� 32 - �� ����� ������� ������� ����� 1.125 ���, �� ��� ����
��� �� ������� �������� ����������, ������� SPI �������� ������ 64. ������ ������� CLKI - 560 ���.
*/

typedef struct {
	float volt_cells[12];
} SBMSVOLTAGE;



class TBMSMEASURE {
	private:
		const unsigned long C_MAXPERMITREAD;
		const unsigned long C_WAITCONV_TIME;
		uint8_t pec8_calc(uint8_t len, uint8_t *data);
		const unsigned char PEC_POLY;
		uint8_t configReg[6];			// ������� ������������ ��������� CFGR 0-5
		SBMSVOLTAGE bmsdata;			// ������� ���������� �� 12 ������
		TSPIOBJ *Spi_obj;
	
	protected:
		utimer_t Timer_release;
		unsigned long conversionStart;
		void ConfigUpdate ();
		void WriteConfig ();
		void ltc6803_stcvad(void);
		bool ltc6803_rdcv();
	
		ESWLTC Sw;
		unsigned long PermitCount;
		unsigned long Errors;
		bool f_parse_new_complete;
	
		bool ParseVoltData (char *lram);

	public:
		TBMSMEASURE ();
		void Init (TSPIOBJ *bus_obj);		// ��������� ������� SPI ������ ������������������ �� 560 ���.
		virtual void Task ();
		virtual void Start ();
	
		bool CheckNewData ();
		SBMSVOLTAGE *GetNewData ();
		void ClearNewData ();
	
};


#endif

