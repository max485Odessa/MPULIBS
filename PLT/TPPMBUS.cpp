#include "TPPMBUS.H"



unsigned long TEXTIIFACE::proc_cnt = 0;
TEXTIIFACE *TEXTIIFACE::ifacearray[C_EXTI915AMOUNT] = {0,0,0,0,0,0,0};


void TEXTIIFACE::isr ()
{
	if (HAL_EXTI_GetPending(&hexti_hall, exti_conf.Mode))
		{
		Pending_cb ();
		HAL_EXTI_ClearPending(&hexti_hall, exti_conf.Mode);
		}
}



void TEXTIIFACE::NVIC_ENABLE (bool stat)
{
	nvic_irq = EXTI15_10_IRQn;
	if (stat)
		{
		NVIC_EnableIRQ (nvic_irq);
		}
	else
		{
		NVIC_DisableIRQ (nvic_irq);
		}
}



void TEXTIIFACE::Pending_cb ()
{
	
}



long TEXTIIFACE::find_cb (TEXTIIFACE *obj)
{
	long rv = -1;
	long ix = 0;
	while (ix < proc_cnt)
		{
		if (ifacearray[ix] == obj)
			{
			rv = ix;
			break;
			}
		ix++;
		}
	return rv;
}



bool TEXTIIFACE::add_cb (TEXTIIFACE *obj)
{
bool rv = false;
if (obj && proc_cnt < C_EXTI915AMOUNT)
	{
	if (find_cb (obj)== -1)
		{
		ifacearray[proc_cnt] = obj;
		proc_cnt++;
		rv = true;
		}
	}
return rv;
}



// запускает цепочку зарегистрированных обработчиков
void TEXTIMANAGER::ISRs ()
{
	unsigned long ix = 0;
	TEXTIIFACE *face;
	while (ix < proc_cnt)
		{
		face = ifacearray[ix];
		if (face) face->isr ();
		ix++;
		}
}



#ifdef __cplusplus
 extern "C" {
#endif 

	 

void EXTI15_10_IRQHandler ()
{
	TEXTIMANAGER::ISRs ();
}



#ifdef __cplusplus
}
#endif



TPPMBUS::TPPMBUS ()
{
	f_meas_complete = false;
	add_chan_ix = 0;
}



void TPPMBUS::Task ()
{
}



void TPPMBUS::Init ()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_TIM2_CLK_ENABLE ();

	// частота тактирования - 1 Мгц
  uint32_t uwPrescalerValue = (uint32_t)(SystemCoreClock / 1000000) - 1;
  TimHandle.Instance = TIM2;
  TimHandle.Init.Period            = 0xFFFFFFFF;
  TimHandle.Init.Prescaler         = uwPrescalerValue;
  TimHandle.Init.ClockDivision     = 0;
  TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
  TimHandle.Init.RepetitionCounter = 0;
	HAL_TIM_Base_Init (&TimHandle);
	__HAL_TIM_ENABLE (&TimHandle);
	
	GPIO_InitStructure.Pin = C_PPM_PIN_RX;
	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init (C_PPM_PORT, &GPIO_InitStructure);
	
	exti_conf.Line = EXTI_LINE_15;
	exti_conf.GPIOSel = C_PPM_PIN_RX;
	exti_conf.Mode = EXTI_MODE_INTERRUPT;
	exti_conf.Trigger = EXTI_TRIGGER_FALLING;
	
	hexti_hall.Line = EXTI_LINE_15;
	HAL_StatusTypeDef rslt = HAL_EXTI_SetConfigLine (&hexti_hall, &exti_conf);
	NVIC_ENABLE (true);
	
	AddObjectToExecuteManager ();
	add_cb (this);
}



void TPPMBUS::Pending_cb ()
{
unsigned long delt;
unsigned long ticks = TimHandle.Instance->CNT;
delt = ticks - prevval_tim;
prevval_tim = ticks;

if (delt > C_PPMINTERVALBETW_MIN && delt < C_PPMINTERVALBETW_MAX)		// обязательное наличие интервала > 5000 мс, формирует сброс индексного указателя
	{
	add_chan_ix = 0;
	}
else
	{
	if (add_chan_ix < C_PPMCHANEL_AMOUNT) 
		{
		if (delt < C_PPMVALUE_MAX && delt > C_PPMVALUE_MIN)
			{
			raw_array.array[add_chan_ix++] = delt;
			if (add_chan_ix >= C_PPMCHANEL_AMOUNT)
				{
				if (!f_meas_complete)
					{
					results_array = raw_array;	// фиксируем принятые данные в буфере
					f_meas_complete = true;
					}
				}
			}
		else
			{
			add_chan_ix = C_PPMCHANEL_AMOUNT;	// блокируем формирование кадра до следующего межпакетного интервала
			}
		}
	}	
}




TSBUSDATA *TPPMBUS::GetData ()
{
	TSBUSDATA *rv = 0;
	if (f_meas_complete)
		{
		unsigned long ix = 0;
		while (ix < 8) {
			sbus_data.channels[ix] = results_array.array[ix];
			ix++;
			}
		sbus_data.flags = 0;
		rv = &sbus_data;		
		f_meas_complete = false;
		}

	return rv;
}



