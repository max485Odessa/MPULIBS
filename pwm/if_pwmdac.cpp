#include "if_pwmdac.h"



IFPWM16DACCORE::IFPWM16DACCORE ()
{
	uint32_t ix = 0;
	while (ix < EDACCH_ENDENUM)
		{
		evnts[ix].event_cb = 0;
		evnts[ix].delay = 0;
		ix++;
		}
}



void IFPWM16DACCORE::Calibrate (float val_min, uint16_t raw_min, float val_max, uint16_t raw_max)
{
	float delt_v = val_max - val_min;
	uint16_t delt_raw = raw_max - raw_min;
	c_min.raw = raw_min;
	c_min.value = val_min;
	c_max.raw = raw_max;
	c_max.value = val_max;
	c_raw_volt_quant_lmb = delt_v / delt_raw;
}



uint16_t IFPWM16DACCORE::GetRaw (float val)
{
	uint16_t raw = c_min.raw;
	if (val > c_min.value)
		{
		if (val < c_max.value)
			{
			val -= c_min.value;
			val /= c_raw_volt_quant_lmb;
			raw = val;
			}
		else
			{
			raw = c_max.raw;
			}
		}
	return raw;
}


