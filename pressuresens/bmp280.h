#ifndef _H_GPS_STM32_PRESSURE_BMP280_H_
#define _H_GPS_STM32_PRESSURE_BMP280_H_



#include "SYSBIOS.h"
#include "PRSIFACE.H"



const unsigned char REG_CALIBRATION = 0x88;
const unsigned char REG_ID = 0xd0;
const unsigned char REG_STATUS = 0xf3;
const unsigned char REG_CTRL_MEAS = 0xf4;
const unsigned char REG_CONFIG = 0xf5;
const unsigned char REG_PRESS_MSB = 0xf7;
const unsigned char REG_PRESS_LSB = 0xf8;
const unsigned char REG_PRESS_XLSB = 0xf9;
const unsigned char REG_TEMP_MSB = 0xfa;
const unsigned char REG_TEMP_LSB = 0xfb;
const unsigned char REG_TEMP_XLSB = 0xfc;



// 0x76 0x77
const unsigned char BM280DEVADR = 0x76;	// текущий адресс устройства 76
//const unsigned char BM280DEVADR = 0x22;	// текущий адресс устройства 76

typedef struct {
	char data[6];
} SBM280RAWDAT;


const unsigned char BMP280CALIBRREGAMOUNT = 12;




typedef struct {
	unsigned short dig_T1;
	short dig_T2;
	short dig_T3;
	unsigned short dig_P1;
	short dig_P2;
	short dig_P3;
	short dig_P4;
	short dig_P5;
	short dig_P6;
	short dig_P7;
	short dig_P8;
	short dig_P9;
} BMP280CALIBRATE;



enum E_B280WORKSW {E_B280WORKSW_RESET = 0, E_B280WORKSW_CBRREAD = 1, E_B280WORKSW_INIT = 2, E_B280WORKSW_WAIT_MEASURE = 3, E_B280WORKSW_READDATA = 4, E_B280WORKSW_ERROR = 5,  E_B280WORKSW_RELEASE = 6};
const unsigned short C_BM280_READ_AMOUNT = 1000;			// количество свободных чтений до полной переинициализации микросхемы
const unsigned short C_B280_TIMEOUT_COPYMEAS = 1000;	// аварийный таймаут преобразования



class TBMP280 : public TPRSIFACE {
	private:
		utimer_t RDYTimeout;									// таймер-таймаут ожидания преобразования и внутреннего техн. копирования данных в самой микросхеме
		SBM280RAWDAT bmraw;
		BMP280CALIBRATE calibrate;
		//bool ReadData (char *lDest, unsigned char DevRegAdr, unsigned char size);		// +
		//bool WriteData (char *lSources, unsigned char DevRegAdr, unsigned char size);	// +
		bool SaveControlMeasur (unsigned char dat);		
		bool SaveConfig (unsigned char dat);
		bool ReadRawStatus (unsigned char &stat_raw);
		bool ReadCalibrationTable ();
		bool ReadRawDataFrame ();
		bool ResetCmd ();
		bool InitRegs ();
		bool CheckCopyTableAndMeasure (bool &f_state);
	
		unsigned long GetRawTemperature ();
		unsigned long GetRawPressure ();
		unsigned long Get24Bit (char *lram);
	
		E_B280WORKSW DevSw;
		unsigned short ReadNormalCounter;
		unsigned short BusErrorCount;
		long t_fine;
	
		bool CalculatePressure ();
		bool CalculateTemperature ();
		bool f_temperature_valid;
		bool f_pressure_valid;
		bool NewDataIsParsed;
		float Cur_Pressure;
		float Cur_Temp;
	
	public:
		TBMP280 ();
		virtual void Task ();
		virtual void Start ();
	
		virtual void Config ();
		bool GetPressAndTemper (float *Tempr, float *Press);
		virtual bool GetPressure (float &DataOut);
		virtual bool GetTemperature (float &DataOut);
		virtual void ClearFlagNewData ();
		virtual bool CheckNewData ();
		
};



#endif


