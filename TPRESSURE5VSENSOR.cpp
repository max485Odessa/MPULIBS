#include "tpressure5vsensor.h"



TPRESSURE::TPRESSURE (IFAIN *ia, EASIG c, float b): chnl (c), c_model_bar (b)
{
	volt = ia;
	params = IRFPARAMS::obj ();
}



bool TPRESSURE::pressure (float &dst)
{
	bool rv = false;
	float cal_min;
	float cal_max;
	float bars;
	do		{
				if (!params->get_papam_f (EPRMIX_CALIBR_PRESS_ZERO)) break;
				if (!params->get_papam_f (EPRMIX_CALIBR_PRESS_MAX)) break;
				if (!params->get_papam_f (EPRMIX_PRESS_BAR)) break;
				if (cal_min >= cal_max) break;
				float dlt = cal_max - cal_min;
				float cur_volt = volt->GetValue (EASIG_NTC);
				if (cur_volt < (cal_min - dlt/10)) break;		// на 10% ниже нуля
				if (cur_volt > (cal_max + dlt/20)) break;		// на 5% больше верха
				if (cur_volt < cal_min)
					{
					dst = 0;
					}
				else
					{
					if (cur_volt > cal_max)
						{
						dst = bars;
						}
					else
						{
						float quant = bars / dlt;
						float clev = cur_volt - cal_min;
						dst = clev * quant;
						}
					}
				rv = true;
				} while (false);
	return rv;
}



void TPRESSURE::calibrate_pressure_zero (float volt_zr)
{
	params->set_papam_f (EPRMIX_CALIBR_PRESS_ZERO, volt_zr);
}



void TPRESSURE::calibrate_pressure_max (float volt_max)
{
	params->set_papam_f (EPRMIX_CALIBR_PRESS_MAX, volt_max);
}



