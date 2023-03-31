#ifndef _H_TANALOG_STM32_SIGNALS_H_
#define _H_TANALOG_STM32_SIGNALS_H_

//#include "main.h"
#include "TFTASKIF.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "rutine.h"
#include "SYSBIOS.H"

#define C_ADCPIN_AMOUNT 12
#define ADC1_DR_Address    ((uint32_t)0x4001244C)
//#define C_EXT_VREF_VALUE ((float)2.997)
#define C_ASCSENS_STEP ((float)0.0264)	// ампер на шаг в миливольтах
#define C_BATTERY_WATS_NOMINAL (16.8 * 50 * 3600)
#define C_ISRTOSECOND 1000	// количество ADC выборок в секунду тока и напряжения батареи, для вычисления ват


enum EASIG {EASIG_VBUS_1 = 0, EASIG_VBUS_2 = 1, EASIG_V12 = 2, EASIG_VSYS = 3, EASIG_VBAT_1 = 4, EASIG_VBAT_3 = 5, EASIG_VBAT_4 = 6, EASIG_VBAT_2 = 7, EASIG_VREF = 8, EASIG_NONE1 = 9, EASIG_TEMP = 10, EASIG_NONE2 = 11, EASIG_ENDENUM = C_ADCPIN_AMOUNT};

class TASIGNALS : public TFFC {
	private:
		float c_vrefint;
		//float c_ext_voltage;
		static uint16_t ADC_DualConvertedValueTab[C_ADCPIN_AMOUNT];
		float voltage[C_ADCPIN_AMOUNT];
		unsigned short *RAWARRAY;
		float cur_bitquant;
		float calculateQuant (unsigned short raw);
		void calculateVoltages ();
		float rawToVoltages (unsigned long raw);
	
		void ClearDMABarray ();
		static float VCC_value;
		virtual void Task ();
		static float cur_quant_vref;
		
	protected:
	
	public:
		TASIGNALS (float vint);
		void Init ();
		void Deinit ();
		void ISR_1ms_Task ();
	
		float GetVoltage (EASIG ix);
		unsigned short GetRaw (EASIG ix);
	
		static float GetVCC ();
		static float GetQuant ();
	
		float GetJoule ();

};


#endif

