#include "TLIONLEVEL.h"



TLIONLEVEL::TLIONLEVEL (const float *vp, float v_min, float v_max) : volt_ain (vp), c_voltage_max (v_max), c_voltage_min (v_min)
{
	set_min_charge_level (c_voltage_min);
	set_full_charge_level (c_voltage_max);
	relax_tim.set (0);
}



float TLIONLEVEL::capacity_joule ()
{
	return c_max_joule - c_min_jopule; 
}



void TLIONLEVEL::set_full_charge_level (float v)
{
	c_max_joule = v * v;
}



void TLIONLEVEL::set_min_charge_level (float v)
{
	c_min_jopule = v * v;
}



void TLIONLEVEL::update_now ()
{
	float volt = *volt_ain;
	//volt = 4.3F;
	if (volt > c_voltage_max) volt = c_voltage_max;
	if (volt < c_voltage_min) volt = c_voltage_min;
	float lvjl = curent_joule (volt) - c_min_jopule;
	float proc1jl = capacity_joule () / 100;
	lvjl /= proc1jl;
	proc_result = lvjl;
	if (proc_result > 97) proc_result = 100;
}



void TLIONLEVEL::Task ()
{
	if (!relax_tim.get ())
		{
		update_now ();
		relax_tim.set (300);
		}
}



float TLIONLEVEL::curent_joule (float volt)
{
	if (volt > c_voltage_max) volt = c_voltage_max;
	if (volt < c_voltage_min) volt = c_voltage_min;
	volt = volt * volt;
	return volt;
}



uint8_t TLIONLEVEL::level ()
{
	return proc_result;
}


