#include "TWATCHDOGEXT.h"

S_GPIOPIN *TWATCHDOGEXT::pinwdi = 0;

TWATCHDOGEXT::TWATCHDOGEXT (S_GPIOPIN *p, uint32_t ms_p) : c_period_pulse (ms_p/2)
{
	pinwdi = p;
	init ();
	_pin_low_init_out_pp (p, 1);
	relax_tim.set (0);
	f_enable_auto_wdi = true;
}



void TWATCHDOGEXT::pin_front ()
{
	_pin_output((S_GPIOPIN*)pinwdi,true);
}



void TWATCHDOGEXT::pin_fall ()
{
	_pin_output((S_GPIOPIN*)pinwdi,false);
}



void TWATCHDOGEXT::init ()
{
	
}



void TWATCHDOGEXT::Task ()
{
	if (!f_enable_auto_wdi) return;
	if (relax_tim.get()) return;
	
	switch (sw)
		{
		case EWDGSW_FRONT:
			{
			pin_front ();
			relax_tim.set (c_period_pulse);	
			sw = EWDGSW_FALL;
			break;
			}
		case EWDGSW_FALL:
			{
			pin_fall ();
			relax_tim.set (c_period_pulse);	
			sw = EWDGSW_FRONT;
			break;
			}
		default: break;
		}
}



void TWATCHDOGEXT::enable_wdi_signal (bool v)
{
	f_enable_auto_wdi = v;
}



void TWATCHDOGEXT::wdr_now ()
{
	pin_front ();
	pin_fall ();
}

