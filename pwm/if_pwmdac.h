#ifndef _H_INTERFACE_PWM_DAC_H_
#define _H_INTERFACE_PWM_DAC_H_


#include "userdef.h"
#include <stdint.h>
#include "tevents.h"


typedef struct {
	float value;
	uint16_t raw;
} S_PWMVPAIR_T;



// need defined: enum EDACCH
// interface pwm for dac emulations
class IFPWM16DAC {
		
	public:
		virtual bool SetVoltage (EDACCH ch, float val, TEVENT *ev_cb) = 0;
};





class IFPWM16DACCORE: public IFPWM16DAC {

	protected:
		typedef struct {
			TEVENT *event_cb;
			uint32_t delay;
		} S_DACEVNT_T;
		
		S_DACEVNT_T evnts[EDACCH_ENDENUM];
		
		S_PWMVPAIR_T c_min;
		S_PWMVPAIR_T c_max;
		float c_raw_volt_quant_lmb;
		uint16_t GetRaw (float val);
		IFPWM16DACCORE ();
	
	public:
		virtual bool SetVoltage (EDACCH ch, float val, TEVENT *ev_cb) = 0;
		void Calibrate (float val_min, uint16_t raw_min, float val_max, uint16_t raw_max);
};


#endif
