#ifndef _H_STM32F103_LTC6803_2_H_
#define _H_STM32F103_LTC6803_2_H_


//#include "spidispatch_class.h"
#include "sysbios.h"
#include "stdint.h"
#include "spiobj_class.h"

// машина состояния task
enum ESWLTC {ESWLTC_INIT = 0, ESWLTC_START = 1, ESWLTC_READ = 2, ESWLTC_COMPLETE = 3};
const unsigned char C_STCVAD = 0x10;
const unsigned char C_STCVAD_PEC = 0xB0;
const unsigned char WRCFG = 0x01;
const unsigned char WRCFG_PEC = 0xC7;


/* ----
Микросхема на плате стоит в количестве 1 шт. Все комманды на линию идут в режиме Broadcast (без байта адресации микросхемы на шине).
Примечание: текущий адрес, этой микросхемы на плате  = 0;
При использовании нескольких микросхем, нужно будет менять код


LTC достаточно медленная микросхема - максимальная частота CLK = 1 Мгц, (master mode baud rate prescalers (fPCLK/2 max.))
от 72 Мгц - это сразу 36 мгц, если 36 Мгц поделить на возможные делители к примеру на 32 - то очень блиская частота будет 1.125 Мгц, но она выше
чем по мануалу работает микросхема, поэтому SPI делитель выбран 64. Теперь чачтота CLKI - 560 Кгц.
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
		uint8_t configReg[6];			// текущая конфигурация регистров CFGR 0-5
		SBMSVOLTAGE bmsdata;			// готовые напряжения по 12 банкам
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
		void Init (TSPIOBJ *bus_obj);		// принимает внешний SPI обьект инициализированный на 560 Кгц.
		virtual void Task ();
		virtual void Start ();
	
		bool CheckNewData ();
		SBMSVOLTAGE *GetNewData ();
		void ClearNewData ();
	
};


#endif

