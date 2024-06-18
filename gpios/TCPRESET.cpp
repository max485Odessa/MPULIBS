#include "TCPRESET.h"



TCPRESET::TCPRESET (S_GPIOPIN *p, uint32_t p_time, bool f_pasv_st) : f_passive_pin_state (f_pasv_st), c_timereset (p_time)
{
	pin_reset = p;

	//_pin_low_init_out_pp (pin_reset, 1);
	thizif_hiz_outputs (false);
	_pin_output(pin_reset, f_passive_pin_state);

	sw = ECPRESETSW_NONE;
}



void TCPRESET::thizif_hiz_outputs (bool f_act_hiz)
{
	if (!f_act_hiz)
		{
		_pin_low_init_out_pp (pin_reset, 1);
		}
	else
		{
		_pin_low_init_in (pin_reset, 1);
		}
}



void TCPRESET::reset ()
{
	//relax_tim.set (c_timereset);
	lasttick = SYSBIOS::GetTickCountLong ();
	sw = ECPRESETSW_WAIT_ACT;
}



void TCPRESET::Task ()
{
if (relax_tim.get ()) return;
	switch (sw)
		{
		case ECPRESETSW_WAIT_ACT:
			{
			if (SYSBIOS::GetTickCountLong () != lasttick) 
				{
				// reset active level
				relax_tim.set (c_timereset);
				//relax_tim.set (10000);
				_pin_output(pin_reset, !f_passive_pin_state);
				//_pin_output((S_GPIOPIN*)pin_dp, 0);
				sw = ECPRESETSW_ACTIVE;
				}
			break;
			}
		case ECPRESETSW_ACTIVE:
			{
			// reset pasive level
				_pin_output(pin_reset, f_passive_pin_state);
				relax_tim.set (2000);
				sw = ECPRESETSW_ACT_DP;
				reset_cnt++;
			break;
			}
		case ECPRESETSW_ACT_DP:
			{
			
			//_pin_output(pin_dp, true);	// active DP
				relax_tim.set (500);
				sw = ECPRESETSW_DP_WAIT;

			break;
			}
		case ECPRESETSW_DP_WAIT:
			{
			sw = ECPRESETSW_NONE;
			// not break - normal
			}
		default: break;
		}
}



uint32_t TCPRESET::counter ()
{
	return reset_cnt;
}


