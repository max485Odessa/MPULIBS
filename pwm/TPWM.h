#ifndef _H_STM32HALL_AIN_CLASS_H_
#define _H_STM32HALL_AIN_CLASS_H_


#include "SYSBIOS.H"
#include "TFTASKIF.h"
#include "stm32f4xx_hal.h"
#include "hard_rut.h"
#include "ITIMINT.h"



enum EPWMCH {EPWMCH_1 = 0, EPWMCH_2 = 1, EPWMCH_3 = 2, EPWMCH_4 = 3, EPWMCH_ENDENUM = 4};
class TPWMIFC {
	public:
		virtual void enable (bool v) = 0;
		virtual void set_pwm (float val) = 0;
};



typedef struct {
	uint32_t period;
	uint32_t freq_clk;
	ESYSTIM e_tim;
	TIM_HandleTypeDef    *TimHandle;
	bool f_is_32Bit;
} S_BASEPWM_INF_T;



class TPWMSCHAN: public TPWMIFC {
		virtual void enable (bool v) override;
		virtual void set_pwm (float val) override;
	
		uint32_t pwm;
		bool f_output_active;

		TIM_OC_InitTypeDef sConfig;
		const uint32_t C_CHAN_ID;
		S_BASEPWM_INF_T const &tim;
	
		uint32_t calculate_pwm (float val);
	
	public:
		TPWMSCHAN (S_BASEPWM_INF_T &t, uint32_t ch);
		
};



class TPWM  {
		static const uint32_t chanpwmlist[EPWMCH_ENDENUM];
		void init_base ();

		const S_BASEPWM_INF_T info;
		TIM_HandleTypeDef    TimHandle;
	
		TPWMSCHAN *ch_array[EPWMCH_ENDENUM];
	
	public:
		TPWM (ESYSTIM t, uint32_t period, uint32_t hz_clk);
		TPWMIFC *getChanel (EPWMCH c);
		void set_period (uint32_t mks);
};





#endif
