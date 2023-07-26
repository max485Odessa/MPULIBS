#ifndef _H_STM32_PRESSURE_DPS310_H_
#define _H_STM32_PRESSURE_DPS310_H_



//#include "DISPATCHIFACE.H"
#include "SYSBIOS.h"
#include "PRSIFACE.H"


const unsigned char DPS310DEVADR = 0x76;	// текущий адресс устройства 76

// 1000 hpa = 1 Bar.

#define C_DPS310_PSR_B2 0
#define C_DPS310_PSR_B1 1
#define C_DPS310_PSR_B0 2
#define C_DPS310_TMP_B2 3
#define C_DPS310_TMP_B1 4
#define C_DPS310_TMP_B0 5
#define C_DPS310_PRS_CFG 6
#define C_DPS310_TMP_CFG 7
#define C_DPS310_MEAS_CFG 8
#define C_DPS310_CFG_REG 9
#define C_DPS310_INT_STS 0x0A
#define C_DPS310_FIFO_STS 0x0B
#define C_DPS310_RESET 0x0C
#define C_DPS310_ID 0x0D
#define C_DPS310_COEF 0x10
#define C_DPS310_COEF_SRCE 0x28

#define C_STATE_TMP_RDY 0x20
#define C_STATE_PRS_RDY 0x10
#define C_DPS_READYMASK 0xC0


#define DPS310_PRSB2 0x00       ///< Highest byte of pressure data
#define DPS310_TMPB2 0x03       ///< Highest byte of temperature data
#define DPS310_PRSCFG 0x06      ///< Pressure configuration
#define DPS310_TMPCFG 0x07      ///< Temperature configuration
#define DPS310_MEASCFG 0x08     ///< Sensor configuration
#define DPS310_CFGREG 0x09      ///< Interrupt/FIFO configuration
#define DPS310_RESET 0x0C       ///< Soft reset
#define DPS310_PRODREVID 0x0D   ///< Register that contains the part ID
#define DPS310_TMPCOEFSRCE 0x28 ///< Temperature calibration src
/** The measurement rate ranges */
typedef enum {
  DPS310_1HZ,   ///< 1 Hz
  DPS310_2HZ,   ///< 2 Hz
  DPS310_4HZ,   ///< 4 Hz
  DPS310_8HZ,   ///< 8 Hz
  DPS310_16HZ,  ///< 16 Hz
  DPS310_32HZ,  ///< 32 Hz
  DPS310_64HZ,  ///< 64 Hz
  DPS310_128HZ, ///< 128 Hz
} dps310_rate_t;

/** The  oversample rate ranges */
typedef enum {
  DPS310_1SAMPLE,    ///< 1 Hz
  DPS310_2SAMPLES,   ///< 2 Hz
  DPS310_4SAMPLES,   ///< 4 Hz
  DPS310_8SAMPLES,   ///< 8 Hz
  DPS310_16SAMPLES,  ///< 16 Hz
  DPS310_32SAMPLES,  ///< 32 Hz
  DPS310_64SAMPLES,  ///< 64 Hz
  DPS310_128SAMPLES, ///< 128 Hz
} dps310_oversample_t;


typedef enum {
  DPS310_IDLE = 0,            ///< Stopped/idle
  DPS310_ONE_PRESSURE = 1,    ///< Take single pressure measurement
  DPS310_ONE_TEMPERATURE = 2, ///< Take single temperature measurement
  DPS310_CONT_PRESSURE = 5,   ///< Continuous pressure measurements
  DPS310_CONT_TEMP = 6,       ///< Continuous pressure measurements
  DPS310_CONT_PRESTEMP = 7,   ///< Continuous temp+pressure measurements
} dps310_mode_t;


#define DPS__MEASUREMENT_RATE_1 0
#define DPS__MEASUREMENT_RATE_2 1
#define DPS__MEASUREMENT_RATE_4 2
#define DPS__MEASUREMENT_RATE_8 3
#define DPS__MEASUREMENT_RATE_16 4
#define DPS__MEASUREMENT_RATE_32 5
#define DPS__MEASUREMENT_RATE_64 6
#define DPS__MEASUREMENT_RATE_128 7

#define DPS__OVERSAMPLING_RATE_1 DPS__MEASUREMENT_RATE_1
#define DPS__OVERSAMPLING_RATE_2 DPS__MEASUREMENT_RATE_2
#define DPS__OVERSAMPLING_RATE_4 DPS__MEASUREMENT_RATE_4
#define DPS__OVERSAMPLING_RATE_8 DPS__MEASUREMENT_RATE_8
#define DPS__OVERSAMPLING_RATE_16 DPS__MEASUREMENT_RATE_16
#define DPS__OVERSAMPLING_RATE_32 DPS__MEASUREMENT_RATE_32
#define DPS__OVERSAMPLING_RATE_64 DPS__MEASUREMENT_RATE_64
#define DPS__OVERSAMPLING_RATE_128 DPS__MEASUREMENT_RATE_128




enum E_DPS310WORKSW {E_DPS310WORKSW_RESET = 0, E_DPS310WORKSW_WAITREADY, E_DPS310WORKSW_READCOEF, E_DPS310WORKSW_RDSRC_TMP, E_DPS310WORKSW_CONFIG, \
E_DPS310WORKSW_START_TEMP, E_DPS310WORKSW_WAIT_TEMP, E_DPS310WORKSW_READ_TEMP, \
E_DPS310WORKSW_START_PRESS, E_DPS310WORKSW_WAIT_PRESS, E_DPS310WORKSW_READ_PRESS, \
E_DPS310WORKSW_ERROR,  E_DPS310WORKSW_RELEASE};
const unsigned short C_DPS310_READ_AMOUNT = 1000;			// количество свободных чтений до полной переинициализации микросхемы
const unsigned short C_DPS310_TIMEOUT_COPYMEAS = 1000;	// аварийный таймаут преобразования

enum EMDPS310 {EMDPS310_PRESSURE = 1, EMDPS310_TEMPERATURE = 2};

#define C_DPS310_COEF_AMOUNT 18
#define DPS310__OSR_SE 3U


class TDPS310SENS : public TPRSIFACE{		//  : public TI2CTRDIFACE
	private:
		static const long scaling_facts[8];
		
		utimer_t RDYTimeout;									// таймер-таймаут ожидания преобразования и внутреннего техн. копирования данных в самой микросхеме
		
		unsigned char frm_ix;
		unsigned char coefdata[C_DPS310_COEF_AMOUNT];
		unsigned char temp_raw[3];
		unsigned char presr_raw[3];
		bool Write_PRSCFG (unsigned char dat);
		bool Write_TMPCFG (unsigned char dat);
		bool Write_CFGREG (unsigned char dat);
		bool Write_MEASCONTROL (EMDPS310 cmd);
		//bool GetMeasStatus (unsigned char &st);
		bool GetReadyStatus (unsigned char &st);
		bool ResetCmd ();
		bool ReadID (unsigned char &id_rd);
	
		void configTemp (unsigned char tempMr, unsigned char  tempOsr);
		void configPressure (unsigned char prsMr, unsigned char prsOsr);
		void CalculateCoeficient ();
		void getTwosComplement (long *raw, unsigned char length);
		void getRawResult (long *dst_raw, void *mem);
	
		long raw_pressure;
		long raw_temperature;
		
		void CalculateTmpPrs ();
		
		unsigned char curset_tempSrc;
		unsigned char curset_tempRate;
		unsigned char curset_tempPRC;
		unsigned char curset_prsRate;
		unsigned char curset_prsPRC;
		unsigned char curset_cfg;
	
		unsigned char GetReg_TMP_CFG ();
		unsigned char GetReg_PRS_CFG ();
		unsigned char GetReg_GFG_REG ();
	
		unsigned char m_tempOsr;
		unsigned char m_prsOsr;
	
		long m_c00;
		long m_c10;
		long m_c01;
		long m_c11;
		long m_c20;
		long m_c21;
		long m_c30;
		long m_c0Half;
		long m_c1;
		
		float m_lastTempScal;
		
		float calcTemp(long raw);
		float calcPressure (long raw);
		
		float cur_temperature;

		//bool ReadData (char *lDest, unsigned char DevRegAdr, unsigned char size);		// +
		//bool WriteData (char *lSources, unsigned char DevRegAdr, unsigned char size);	// +
		void WriteByte (unsigned char dst_adr, unsigned char dat);
	
		E_DPS310WORKSW DevSw;
		unsigned short ReadNormalCounter;
		unsigned short BusErrorCount;
		long t_fine;
	
		bool f_temperature_valid;			// предотвращает выдачу данных до первого преобразования
		bool f_pressure_valid;				// предотвращает выдачу данных до первого преобразования
		bool NewDataIsParsed;				
		float Cur_Pressure;
		float Cur_Temp;
	
	public:
		TDPS310SENS ();
	
		virtual void Task ();
		virtual void Start ();
		virtual void Config ();
		bool GetPressAndTemper (float *Tempr, float *Press);			// считывает и сбрасывает флаг что данные новые
		virtual bool GetPressure (float &DataOut);
		virtual bool GetTemperature (float &DataOut);
		virtual void ClearFlagNewData ();
		virtual bool CheckNewData ();
};




#endif
