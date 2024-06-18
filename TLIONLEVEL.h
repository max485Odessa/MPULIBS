#ifndef _H_BATTERY_LEVEL_H_
#define _H_BATTERY_LEVEL_H_



#include "TMAVPARAMS.h"
#include "app.h"
#include "SYSBIOS.H"
#include "TFTASKIF.h"



class TLIONLEVEL: public TFFC {
		const float *volt_ain;
		virtual void Task () override;
	
		SYSBIOS::Timer relax_tim;
	
		const float c_voltage_max;
		const float c_voltage_min;
		float c_max_joule;
		float c_min_jopule;
		float curent_joule (float volt);
		void set_full_charge_level (float v);
		void set_min_charge_level (float v);
		float capacity_joule ();
	
		uint8_t proc_result;
	
	public:
		TLIONLEVEL (const float *l, float v_min, float v_max);

		void update_now ();
		uint8_t level ();
};



#endif
