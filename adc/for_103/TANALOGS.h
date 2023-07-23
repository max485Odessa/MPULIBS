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

//#define C_ADCCHAN_AMOUNT 4
#define ADC1_DR_Address    ((uint32_t)0x4001244C)



enum EASIG {EASIG_VREFINT = 0, EASIG_VBAT = 1, EASIG_VDDHI = 2, EASIG_NTC = 3,  EASIG_V12SW = 4, EASIG_VSET = 5, EASIG_ENDENUM = 6};
class IFAIN {
	public:
		virtual float GetSmoothValue (EASIG ix) = 0;
		virtual float GetValue (EASIG ix) = 0;
		virtual uint16_t GetRaw (EASIG ix) = 0;
		virtual float GetVCC () = 0;
		virtual float *GetVoltCell (EASIG ix) = 0;
};


class TASIGNALS : public TFFC, public IFAIN {
	private:
		//uint32_t last_tick;
		const uint16_t c_smooth_cicle_n;
		uint16_t smooth_cicle_n;
		
		float c_vref_voltage;
		float bitquant_val;
		static uint16_t ADC_DMA_arr[EASIG_ENDENUM];
		float voltage[EASIG_ENDENUM];
	
		float smooth_voltage[EASIG_ENDENUM];
		float smooth_acc_arr[EASIG_ENDENUM];

		void calculateVoltages ();
		float rawToVoltages (unsigned long raw);
	
		void ClearDMABarray ();
	
		float VCC_value;
		virtual void Task ();
		
	protected:
		void Init ();
		void Deinit ();
	public:
		TASIGNALS (uint16_t smcl);
		
		virtual float GetSmoothValue (EASIG ix);
		virtual float GetValue (EASIG ix);
		virtual uint16_t GetRaw (EASIG ix);
		virtual float GetVCC ();
		virtual float *GetVoltCell (EASIG ix);
};


#endif

