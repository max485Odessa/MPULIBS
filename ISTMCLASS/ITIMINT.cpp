#include "ITIMINT.h"



static const uint8_t isrnumbarr[ESYSTIM_ENDENUM] = {TIM1_CC_IRQn, TIM2_IRQn, TIM3_IRQn, TIM4_IRQn, TIM5_IRQn, 0, 0, 0, 0, 0, 0, 0};
//static const TIM_TypeDef *artims[ESYSTIM_ENDENUM] = {TIM2, TIM5};
static ITIM_MKS_ISR *isr_this[ESYSTIM_ENDENUM] = {0,0,0,0,0,0,0,0,0,0,0,0};
static const uint32_t chanarr[4] = {TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3, TIM_CHANNEL_4};
static const uint32_t ocmode[2] = {TIM_OCMODE_PWM1, TIM_OCMODE_PWM2};


#ifdef __cplusplus
 extern "C" {
#endif 

	 

static void TimIsr (ITIM_MKS_ISR *o)
{
	ITIM_MKS_ISR &obj = *o;
	obj.isr_tim ();
}	


	 
void TIM2_IRQHandler ()
{
	TimIsr (isr_this[ESYSTIM_TIM2]);
}



void TIM3_IRQHandler ()
{
	TimIsr (isr_this[ESYSTIM_TIM3]);
}



void TIM4_IRQHandler ()
{
	TimIsr (isr_this[ESYSTIM_TIM4]);
}



void TIM5_IRQHandler ()
{
	TimIsr (isr_this[ESYSTIM_TIM5]);
}



#ifdef __cplusplus
}
#endif


ITIM_MKS_USER_A::ITIM_MKS_USER_A (ESYSTIM t, uint32_t prd, uint32_t fr) : c_value_period (prd), c_value_freq (fr)
{
TimHandle.Instance = hard_get_tim (t, 0);
timer_init (c_value_period, c_value_freq);
}



void ITIM_MKS_USER_A::timer_init (uint32_t period, uint32_t hz)
{
TIM_OC_InitTypeDef   sConfig;
uint32_t uwPrescalerValue = (uint32_t) ((SystemCoreClock / hz) - 1);
hard_tim_clock_enable (tim_ix);
  
  TimHandle.Init.Period = period - 1;
  TimHandle.Init.Prescaler = uwPrescalerValue;
  TimHandle.Init.ClockDivision = 0;
  TimHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
	HAL_TIM_OC_Init (&TimHandle);
	//HAL_TIM_Base_Start (&TimHandle);
}



uint32_t ITIM_MKS_USER_A::get_timer_counter ()
{
	return TimHandle.Instance->CNT;
}




uint32_t ITIM_MKS_USER_A::get_delta (uint32_t prv, uint32_t cur)
{
uint32_t rv;
if (prv <= cur)
	{
	rv = cur - prv;
	}
else
	{
	rv = c_value_period - prv + cur;
	}
return rv;
}



uint32_t ITIM_MKS_USER_A::get_period ()
{
	return c_value_period;
}



uint32_t ITIM_MKS_USER_A::get_freq ()
{
	return c_value_freq;
}



void ITIM_MKS_USER_A::clr_tim ()
{
	TimHandle.Instance->CNT = 0;
}



ITIM_MKS_ISR::ITIM_MKS_ISR (ESYSTIM t, uint32_t prd, uint32_t fr, ITIMCB *cb) : ITIM_MKS_USER_A (t, prd, fr)
{
	callback_user = cb;
	
	isr_this[t] = this;
	tim_ix = t;
	uint8_t ix = 0;
	while (ix < EPWMCHNL_ENDENUM)
		{
		f_active_isr[ix] = false;
		a_pwmvalue[ix] = 0;
		ix++;
		}
}








// isr context executed
void ITIM_MKS_ISR::isr_tim ()
{
	if(__HAL_TIM_GET_FLAG (&TimHandle, TIM_IT_CC1) !=RESET)
    {
			if (f_one_short[EPWMCHNL_PWM1]) enable_timer_oc (EPWMCHNL_PWM1, false);
			callback_user->tim_comp_cb_user_isr (tim_ix, EPWMCHNL_PWM1);
		 __HAL_TIM_CLEAR_IT(&TimHandle, TIM_IT_CC1);
		}
	if(__HAL_TIM_GET_FLAG (&TimHandle, TIM_IT_CC2) !=RESET)
    {
			if (f_one_short[EPWMCHNL_PWM2]) enable_timer_oc (EPWMCHNL_PWM2, false);
			callback_user->tim_comp_cb_user_isr (tim_ix, EPWMCHNL_PWM2);
		 __HAL_TIM_CLEAR_IT(&TimHandle, TIM_IT_CC2);
		}
	if(__HAL_TIM_GET_FLAG (&TimHandle, TIM_IT_CC3) !=RESET)
    {
			if (f_one_short[EPWMCHNL_PWM3]) enable_timer_oc (EPWMCHNL_PWM3, false);
			callback_user->tim_comp_cb_user_isr (tim_ix, EPWMCHNL_PWM3);
		 __HAL_TIM_CLEAR_IT(&TimHandle, TIM_IT_CC3);
		}
	if(__HAL_TIM_GET_FLAG (&TimHandle, TIM_IT_CC4) !=RESET)
    {
			if (f_one_short[EPWMCHNL_PWM4]) enable_timer_oc (EPWMCHNL_PWM4, false);
			callback_user->tim_comp_cb_user_isr (tim_ix, EPWMCHNL_PWM4);
		 __HAL_TIM_CLEAR_IT(&TimHandle, TIM_IT_CC4);
		}
	if(__HAL_TIM_GET_FLAG (&TimHandle, TIM_IT_UPDATE) !=RESET)
    {
			callback_user->tim_comp_cb_user_isr (tim_ix, EPWMCHNL_UPDATE);
		 __HAL_TIM_CLEAR_IT(&TimHandle, TIM_IT_UPDATE);
		}
}




void ITIM_MKS_ISR::enable_timer_isr (bool st)
{
	IRQn_Type tp = (IRQn_Type)(isrnumbarr[tim_ix]);
	if (st)
		{
		HAL_NVIC_EnableIRQ (tp);
		}
	else
		{
		HAL_NVIC_DisableIRQ (tp);
		}
}



void ITIM_MKS_ISR::set_timer_oc_value (EPWMCHNL c, uint32_t v)
{
	if (c < EPWMCHNL_ENDENUM) 
		{
		a_pwmvalue[c] = v;
		__HAL_TIM_SET_COMPARE (&TimHandle, chanarr[c], v);
		}
}



void ITIM_MKS_ISR::start_one_short (EPWMCHNL c, uint32_t dly_mks)
{
	if (c < EPWMCHNL_ENDENUM)
		{
		f_one_short[c] = true;
		uint32_t nw = get_timer_counter () + dly_mks;
		set_timer_oc_value (c, nw);
		enable_timer_oc (c, true);
		}
}



void ITIM_MKS_ISR::enable_timer_oc (EPWMCHNL c, bool state)
{
	if (c < EPWMCHNL_ENDENUM)
		{
		TIM_OC_InitTypeDef sConfig;

		if (state)
			{

			sConfig.OCMode  = TIM_OCMODE_ACTIVE;//TIM_OCMODE_FORCED_ACTIVE;//TIM_OCMODE_ACTIVE;//ocmode[c]; //TIM_OCMODE_PWM1;//TIM_OCMODE_ACTIVE;
			sConfig.OCPolarity = TIM_OCPOLARITY_HIGH;

			sConfig.Pulse = a_pwmvalue[c];  
			HAL_TIM_OC_ConfigChannel (&TimHandle, &sConfig, chanarr[c]);
			HAL_TIM_OC_Start_IT (&TimHandle, chanarr[c]);
			}
		else
			{
			HAL_TIM_OC_Stop_IT (&TimHandle, chanarr[c]);
			}
		f_active_isr[c] = state;
		}
}





