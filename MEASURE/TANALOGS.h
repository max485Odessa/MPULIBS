#ifndef _H_TANALOG_STM32_SIGNALS_H_
#define _H_TANALOG_STM32_SIGNALS_H_


#include "TFTASKIF.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "rutine.h"
#include "SYSBIOS.H"

#define C_ADCPIN_AMOUNT 2
#define ADC1_DR_Address    ((uint32_t)0x4001244C)
//#define C_EXT_VREF_VALUE ((float)2.997)
#define C_ASCSENS_STEP ((float)0.0264)	// ампер на шаг в миливольтах
#define C_BATTERY_WATS_NOMINAL (16.8 * 50 * 3600)
#define C_ISRTOSECOND 1000	// количество ADC выборок в секунду тока и напряжения батареи, для вычисления ват
#define C_RESISTORS_TROTL_MULT 1.588F

enum EASIG { EASIG_VREF = 0, EASIG_TROTLE = 1, EASIG_ENDENUM};

class TASIGNALS : public TFFC {
	private:
		const float c_ext_voltage;
		static uint16_t ADC_DualConvertedValueTab[C_ADCPIN_AMOUNT];
		float voltage[C_ADCPIN_AMOUNT];
		float cur_bitquant;
		float calculateQuant (unsigned short raw);
		void calculateVoltages ();
		float rawToVoltages (unsigned long raw);
	
		void ClearDMABarray ();
	
		//float BAT_OFFSET_ampere;		// с учетом температурного дрейфа
	
		static float VCC_value;
		
		//float BATTERY_CUR_WATSECOND;	// текущая емкость батареи
		virtual void Task ();
		static float cur_quant_vref;
		unsigned short GetRaw (EASIG ix);
	
		bool f_dma_mode;
		uint16_t soft_ix;
		uint16_t GetAdcChanelFrom_ix (uint16_t ix);
		void set_channel_mux (uint16_t ch);
		
	protected:
	
	public:
		TASIGNALS ();
		void Init (bool f_dma_mode);
		void Deinit ();
	
		float GetVoltage (EASIG ix);
		float GetTrotle ();
		
		static float GetVCC ();
		static float GetQuant ();
};


#endif

