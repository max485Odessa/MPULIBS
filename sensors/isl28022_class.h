#ifndef _H_STM32_ISL28022_H_
#define _H_STM32_ISL28022_H_

#include "sysbios.h"
#include "dispatchiface.h"
#include "rutine.h"


typedef struct {
	float shunt_voltage;
	float bus_voltage;
	float power;
	float Current;
	bool f_error_ovf;
} TSCURRENT;


typedef __packed struct {
	short ShuntVoltage_raw;
	short BusVoltage_raw;
	short Power_raw;
	short Current_raw;
} rawfield;

// отключить возможное выравнивание
typedef __packed struct {
	__packed union {
				rawfield fld;
				unsigned char datline[8];
				};
} TISLRAWDATA;


enum EISLSW {EISLSW_INIT = 0, EISLSW_WAITCONV = 1, EISLSW_COMPLETE = 2};

const unsigned char ISLREG_CONFG = 0;
const unsigned char ISLREG_SVOLT = 1;
const unsigned char ISLREG_BVOLT = 2;


class TCURRENTMEAS : public TI2CTRDIFACE {
	private:
		utimer_t ConvTimeLimit;									// лимит ожидания готовности данных
		const unsigned short C_PERMIT_READ;
		const unsigned short C_TIMEOUT_CONV;		// аварийный таймаут ожидания данных
		const unsigned char C_SLAVEADRESS;
		TSCURRENT Parse_data;
		TISLRAWDATA rawdata;

		bool ReadRaw_ShuntVoltage ();
		bool ReadRaw_BusVolatage ();
		bool WriteConfig (unsigned short dat);
		//bool ReadRawData (TISLRAWDATA *lDst);
		void SwapRawData ();
		bool CheckConversionComplete ();		// проверяет установленный бит CNVR
		bool CheckOverflovValue ();					// проверяет установленный бит OVF
	
		bool ParseData ();
		
	protected:
		bool f_new_parse_data;
		EISLSW Sw;
		unsigned short PermitCount;
		unsigned short Errors;
	
	public:
		TCURRENTMEAS ();

		virtual void Task ();	
		virtual void Start ();
	
		void ClearFlagNewData ();							
		bool CheckNewData ();									
		TSCURRENT *GetCurrent ();	

};


#endif


