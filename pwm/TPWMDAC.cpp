#include "TPWMDAC.h"


const uint32_t TPWMDACIMPL::delay_arrays[EDACCH_ENDENUM] = {2,2};


TPWMDACIMPL::TPWMDACIMPL (EPWMDACSTART mod)
{
	last_ticks = 0;
	init (mod);
}



void TPWMDACIMPL::pwm_level_set (EDACCH ch, float val)
{
	uint16_t val16 = GetRaw (val);
	switch (ch)
		{
		case EDACCH_SETCUR:
			{
			TIM3->CCR3 = val16;
			break;
			}
		case EDACCH_SETVOLT:
			{
			TIM3->CCR4 = val16;
			break;
			}
		default: break;
		}
}



bool TPWMDACIMPL::SetVoltage (EDACCH ch, float val, TEVENT *ev_cb)
{
	bool rv = false;
	if (ch < EDACCH_ENDENUM)
		{
		evnts[ch].event_cb = ev_cb;
		evnts[ch].delay = delay_arrays[ch];
		pwm_level_set (ch, val);
		rv = true;
		}
	return rv;
}



void TPWMDACIMPL::Task ()
{
	uint32_t curtick = SYSBIOS::GetTickCountLong (), delt;
	delt = curtick - last_ticks;
	if (!delt)
		{
		uint32_t ix = 0;
		while (ix < EDACCH_ENDENUM)
			{
			if (evnts[ix].event_cb)
				{
				if (evnts[ix].delay)
					{
					if (dec_value (&evnts[ix].delay, delt))
						{
						evnts[ix].event_cb->call_bc (EEVSTATUS_OK);
						evnts[ix].event_cb = 0;
						}
					else
						{
						
						}
					}
				else
					{
					evnts[ix].event_cb->call_bc (EEVSTATUS_OK);
					evnts[ix].event_cb = 0;
					}
				}
			ix++;
			}
		}
}



void TPWMDACIMPL::init (EPWMDACSTART mod)
{
	const S_GPIOPIN pout = {GPIOA, GPIO_Pin_6};
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
		
  RCC_APB1PeriphClockCmd (RCC_APB1Periph_TIM3, ENABLE);
  RCC_APB2PeriphClockCmd (RCC_APB2Periph_AFIO, ENABLE);
		
  uint32_t PrescalerValue = (uint16_t) (SystemCoreClock / 24000000) - 1;

  TIM_TimeBaseStructure.TIM_Period = C_PWMDACPERIOD;
  TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 0;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

  TIM_OC1Init(TIM3, &TIM_OCInitStructure);
  TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
  TIM_ARRPreloadConfig(TIM3, ENABLE);
  TIM_Cmd(TIM3, ENABLE);
	
	switch (mod)
		{
		default:
		case EPWMDACSTART_0:
			{
			TIM3->CCR1 = 0;
			break;
			}
		case EPWMDACSTART_1:
			{
			TIM3->CCR1 = C_PWMDACPERIOD;
			break;
			}
		}
	_pin_low_init_af_o_pp (const_cast<S_GPIOPIN*>(&pout), 1);
}


