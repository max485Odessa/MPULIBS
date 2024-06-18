#ifndef _H_STM32HALL_CP2102_RESET_CLASS_H_
#define _H_STM32HALL_CP2102_RESET_CLASS_H_


#include "hard_rut.h"
#include "SYSBIOS.H"
#include "TFTASKIF.h"
#include "THIZIF.h"

enum ECPRESETSW {ECPRESETSW_NONE = 0, ECPRESETSW_WAIT_ACT, ECPRESETSW_ACTIVE, ECPRESETSW_WAIT_AFTER_RS, ECPRESETSW_ACT_DP, ECPRESETSW_DP_WAIT, ECPRESETSW_ENDENUM};


class TCPRESET: public TFFC, public THIZIF {
		virtual void Task () override;
		S_GPIOPIN *pin_reset;
		//S_GPIOPIN *pin_dp;
		SYSBIOS::Timer relax_tim;
	
		const bool f_passive_pin_state;
		ECPRESETSW sw;
		const uint32_t c_timereset;
		utimer_t lasttick;
		uint32_t reset_cnt;
	
		virtual void thizif_hiz_outputs (bool f_act_hiz) override;
		
	public:
		TCPRESET (S_GPIOPIN *gch, uint32_t p_time, bool f_pasv_st); // , S_GPIOPIN *dp
		void reset ();
		uint32_t counter ();
		
};


#endif
