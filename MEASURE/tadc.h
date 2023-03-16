#ifndef _H_ADC_OBJECT_STM32L432_H_
#define _H_ADC_OBJECT_STM32L432_H_


#include "stm32l4xx_hal.h"
#include "SYSBIOS.H"
#include "TFTASKIF.h"

#define ADC_CONVERTED_DATA_BUFFER_SIZE 2
enum EADCCH {EADCCH_5V = 0, EADCCH_VREF_EXT = 1, EADCCH_ENDENUM = ADC_CONVERTED_DATA_BUFFER_SIZE};

#define C_5_VOLT_DIVIDER_COEF ((float)3.00003)

class TAINCONTROL:  public TFFC {
	private:
		float vref_internal;
		ADC_ChannelConfTypeDef sConfig;
		void set_chan_mux (EADCCH emux);
		ADC_HandleTypeDef AdcHandle;
		uint32_t curadc_chan;
		uint16_t   aADCxConvertedData[ADC_CONVERTED_DATA_BUFFER_SIZE];
		volatile utimer_t meas_timer;

		float curval_5v;
		float curval_vcc;
	
		float curval_quantum;
		float calc_quantum ();
		EADCCH lastchenum;
	
		bool f_actual_measure;
		
	public:
		TAINCONTROL ();
		void Init (float cvref);	// параметр - внутрений опорный источник напряжения
		virtual void Task ();
	
		// user functions
		bool is_actual_vals ();
		float get_5volt ();
		float get_vcc ();
};


#endif
