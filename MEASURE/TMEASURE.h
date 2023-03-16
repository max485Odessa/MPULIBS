#ifndef _H_MEASURE_ADC_STM32F303_H_
#define _H_MEASURE_ADC_STM32F303_H_


#include "stm32f30x_adc.h"
#include "stm32f30x_rcc.h"
#include "stm32f30x_gpio.h"
#include "hard_rut.h"
#include "TFTASKIF.h"
#include "stdint.h"
#include "sysbios.h"


enum EAIN {EAIN_VREF = 0, EAIN_TEMP = 1, EAIN_CUR_ROLL = 2, EAIN_CUR_PITCH = 3, EAIN_CUR_YAW = 4, EAIN_ENDENUM = 5};

typedef struct {
	ADC_TypeDef* ADCx;
	uint8_t ADC_Channel;
	uint8_t cicles;
} S_ADCINTMEA_T;


class TMEAS: public TFFC {
		virtual void Task ();
		void init_adcs ();
		float vcc_voltage;
		float ion_quant;
		uint16_t rawadc[EAIN_ENDENUM];
		float voltages[EAIN_ENDENUM];
		EAIN meas_sw;

		ADC_TypeDef *get_adc_ic (EAIN ix);
		uint8_t get_mux (EAIN ix);
		uint8_t get_cicles (EAIN ix);
		void sw_next ();
		void start_measure ();
		bool f_new_cicle;
		utimer_t meas_timeout;
		utimer_t meas_block_time;
		bool f_meas_started;
		void calculate_voltages ();
	
		void init_adc (ADC_TypeDef *adc);
		TMEAS ();
		static TMEAS *meas_obj;
		
	public:
		static TMEAS *get_obj ();

		bool check_cicle_f ();
		void clear_cicle_f ();
	
		float get_vcc ();
		float *get_vcc_adr ();
		float get_voltage (EAIN ix);
		float *get_adr (EAIN ix);
	
		void meas_blocked (uint32_t tmm);
		float get_voltage_sync (EAIN ix);
};


#endif
