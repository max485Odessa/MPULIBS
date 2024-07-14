#ifndef _H_STM32HALL_WATCHDOG_EXTERNAL_PIN_CLASS_H_
#define _H_STM32HALL_WATCHDOG_EXTERNAL_PIN_CLASS_H_

#include "hard_rut.h"
#include "SYSBIOS.H"
#include "TFTASKIF.h"


class TWATCHDOGEXT: public TFFC  {
		enum EWDGSW {EWDGSW_FRONT, EWDGSW_FALL, EWDGSW_OFF, EWDGSW_ENDENUM};
		EWDGSW sw;
		
		const uint32_t c_period_pulse;
		SYSBIOS::Timer relax_tim;
		static S_GPIOPIN *pinwdi;
		virtual void Task ();
		void init ();
		
		bool f_enable_auto_wdi;
		static void pin_front ();
		static void pin_fall ();
	
	public:
		TWATCHDOGEXT (S_GPIOPIN *p, uint32_t ms_p);
		void enable_wdi_signal (bool v);
		static void wdr_now ();
};


#endif
