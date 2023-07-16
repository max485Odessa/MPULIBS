#ifndef _H_SIGNALS_CANBMS_H_
#define _H_SIGNALS_CANBMS_H_



#include "TFTASKIF.h"
#include "hard_rut.h"
#include "tevents.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "SYSBIOS.H"



enum ESPINS {ESPINS_LIFE = 0, ESPINS_FAN = 1, ESPINS_LOAD = 2, ESPINS_BUZZ = 3, ESPINS_RELAY = 4, ESPINS_ENDENUM = 5};
class IPINS {
	public:
		virtual void Pin_to (uint8_t ix, bool val) = 0;
};



class TSIGNALS: public TFFC, public SYSBIOS::Periodic, public IPINS {
	private:
		
		const S_GPIOPIN *pins;
		uint8_t pins_n;
		virtual void Task ();
		bool f_life_enabled;
		bool f_life_pin_ph;
		virtual void periodic_cb (SYSBIOS::HPERIOD h);
		SYSBIOS::HPERIOD handl_life_exec;
		SYSBIOS::Timer life_strb_timer;
		TEVENT *live_event;
	
	public:
		TSIGNALS (const S_GPIOPIN *p, uint8_t np);

		virtual void Pin_to (uint8_t ix, bool val);
		void Life_enable (bool v, TEVENT *evlf);
};



#endif

