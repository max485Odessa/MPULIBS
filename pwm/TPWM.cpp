#include "TPWM.h"

const uint32_t TPWMSCHAN::chanpwmlist[EPWMCH_ENDENUM] = {TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3, TIM_CHANNEL_4};

TPWMSCHAN::TPWMSCHAN (S_BASEPWM_INF_T &t, uint32_t ch) : tim(t), C_CHAN_ID (ch), c_f_inverse (false)
{
	default_init ();
}



TPWMSCHAN::TPWMSCHAN (S_BASEPWM_INF_T &t, uint32_t ch, bool inv) : tim(t), C_CHAN_ID (ch), c_f_inverse (inv)
{
	default_init ();
}



void TPWMSCHAN::default_init ()
{
	f_output_active = false;
	pwm = 0;
  sConfig.OCMode       = TIM_OCMODE_PWM1;//(c_f_inverse)?TIM_OCMODE_PWM1:TIM_OCMODE_PWM2; 
  sConfig.OCPolarity   = c_f_inverse ? TIM_OCPOLARITY_LOW : TIM_OCPOLARITY_HIGH;
  sConfig.OCFastMode   = TIM_OCFAST_DISABLE;
  sConfig.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
  sConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  sConfig.OCIdleState  = TIM_OCIDLESTATE_RESET;
	sConfig.Pulse = pwm;
	
	HAL_TIM_PWM_ConfigChannel (tim.TimHandle, &sConfig, chanpwmlist[C_CHAN_ID]);
}




void TPWMSCHAN::enable (bool v)
{
	if (f_output_active != v)
		{
		if (v)
			{
			HAL_TIM_PWM_Start (tim.TimHandle, chanpwmlist[C_CHAN_ID]);
			}
		else
			{
			HAL_TIM_PWM_Stop (tim.TimHandle, chanpwmlist[C_CHAN_ID]);
			}
		f_output_active = v;
		}
}


// in 0 - 1
uint32_t TPWMSCHAN::calculate_pwm (float val)
{
if (val > 1.0F) val = 1.0F;
if (val < -1.0F) val = 0;//-1.0F;
//if (val < 0) val *= -1;
float data = tim.period - 1;
data *= val;
return data;
}



void TPWMSCHAN::Set_CCR (uint32_t v)
{
	if (C_CHAN_ID < EPWMCH_ENDENUM) {
		__IO uint32_t *dst = &tim.TimHandle->Instance->CCR1;
		dst[C_CHAN_ID] = v;
		}
}



void TPWMSCHAN::set_pwm_f (float val)
{
	pwm = calculate_pwm (val);
	enable (pwm);
	if (pwm > period_pwm_control) pwm = period_pwm_control;
	
	Set_CCR (pwm);
}



void TPWMSCHAN::set_control_period (uint32_t val)
{
	period_pwm_control = val;
}



void TPWMSCHAN::set_pwm (uint32_t val)
{
	if (val > period_pwm_control) val = period_pwm_control;
	pwm = val;
	Set_CCR (pwm);
	//tim.TimHandle->Instance->CCR1 = pwm;
}



TPWM::TPWM (ESYSTIM t, uint32_t period, uint32_t hz_clk, S_PWM_INIT_LIST_T *ls, uint8_t pn) : info {period, hz_clk, t, &TimHandle, hard_tim_is_32bit (t)}
{
	hard_tim_clock_enable (info.e_tim);
	tim_gpio_init (ls, pn);
	
	HAL_TIM_Base_DeInit(&TimHandle);
	
	uint32_t uhPrescalerValue = (uint32_t)(SystemCoreClock / info.freq_clk) - 1;
	TimHandle.Instance = get_hard_tim (info.e_tim);

	TimHandle.Init.Prescaler         = uhPrescalerValue;
	TimHandle.Init.Period            = info.period;
	TimHandle.Init.ClockDivision     = 0;
	TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
	TimHandle.Init.RepetitionCounter = 0;

	uint8_t ix = ETIMCH_1;
	while (ix < EPWMCH_ENDENUM) {
		ch_array[ix] = new TPWMSCHAN (const_cast<S_BASEPWM_INF_T&>(info), ix /*chanpwmlist[ix]*/, ls[ix].f_inverse);
		ix++;
		}
	init_base ();
}



void TPWM::set_pwm (uint8_t ix, uint32_t mks)
{
	if (ix < EPWMCH_ENDENUM && ch_array[ix]) ch_array[ix]->set_pwm (mks);
}



void TPWM::enable (uint8_t ix, bool v)
{
	if (ix < EPWMCH_ENDENUM) ch_array[ix]->enable (v);
}



void TPWM::tim_gpio_init (S_PWM_INIT_LIST_T *ls, const uint8_t c_pn)
{
	uint8_t ix = 0;
while (ix < c_pn) {
	_pin_low_init_out_pp_af (ls[ix].af, &ls[ix].port, EHRTGPIOSPEED_HI);
	ix++;
	}
}



void TPWM::init_base ()
{
	HAL_TIM_PWM_Init(&TimHandle);
}



TPWMIFC *TPWM::getChanel (ETIMCH c)
{
	TPWMIFC *rv = 0;
	if (c < EPWMCH_ENDENUM) rv = ch_array[c];
	return rv;
}



void TPWM::set_period (uint32_t mks)
{
info.period = mks;
	// update width pwm controls
	uint8_t ix = ETIMCH_1;
	while (ix < EPWMCH_ENDENUM) {
		if (ch_array[ix]) ch_array[ix]->set_control_period (mks);
		ix++;
		}
TimHandle.Instance->ARR = info.period;
}


