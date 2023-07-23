#ifndef _H_PWM_DAC_STM32_H_
#define _H_PWM_DAC_STM32_H_


#include "rutine.h"
#include "hard_rut.h"
#include "if_pwmdac.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"
#include "TANALOGS.h"
#include "TFTASKIF.h"
#include "userdef.h"
//#include "SYSBIOS.H"


#define C_PWMDACPERIOD 1000
enum EPWMDACSTART {EPWMDACSTART_0, EPWMDACSTART_1, EPWMDACSTART_ENDENUM};


// need defined: enum EDACCH


class TPWMDACIMPL : public IFPWM16DACCORE, public TFFC  {
		static const uint32_t delay_arrays[EDACCH_ENDENUM];

		uint32_t last_ticks;
		virtual void Task ();
		virtual bool SetVoltage (EDACCH ch, float val, TEVENT *ev_cb);
		void init (EPWMDACSTART mod);
		bool f_no_init;
		void pwm_level_set (EDACCH ch, float val);
		
	public:
		TPWMDACIMPL (EPWMDACSTART mod);
	
};



#endif
