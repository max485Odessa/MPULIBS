#include "THBRIDGE.h"



THBRIDGE::THBRIDGE (ESYSTIM t, uint32_t period, uint32_t hz_clk, S_HBRIDGE_INIT_LIST_T *ls)
{
	hbrpins = *ls;
	hbrpins.p[EHBRPINS_PWM].f_inverse = false;
	hbrpins.p[EHBRPINS_PHASE].f_inverse = true;
	pwm_base = new TPWM (t, period, hz_clk, hbrpins.p, EHBRPINS_ENDENUM);

	set_hbr_period (period);
}



uint32_t THBRIDGE::max_width ()
{
	return (period_hbr_mks > C_HBR_NEXTPH_PULSE)?period_hbr_mks - C_HBR_NEXTPH_PULSE:0;
}



void THBRIDGE::set_hbr_period (uint32_t mks)
{
	period_hbr_mks = mks;
	pwm_base->set_period (period_hbr_mks);
	set_hbr_pwm (pwm_hbr_mks);
}



void THBRIDGE::set_hbr_pwm (uint32_t mks)
{
	uint32_t mxwdth = max_width ();
	if (mxwdth)
		{
		if (mks > mxwdth) mks = mxwdth;
		pwm_hbr_mks = mks;
		pwm_base->set_pwm (EHBRPINS_PWM, pwm_hbr_mks);
		pwm_base->set_pwm (EHBRPINS_PHASE, max_width ());	
		}
}



void THBRIDGE::hbr_enable (bool val)
{
	pwm_base->enable (EHBRPINS_PWM, val);
	pwm_base->enable (EHBRPINS_PHASE, val);
}


