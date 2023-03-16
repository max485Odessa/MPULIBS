#include "TProfilerSTM.h"
#include <stdint.h>
#include "resources.h"

static bool f_interupt_overflow = false;
static unsigned long overflow_timer = 0;

#ifdef PROFILER_HAL_STM32F432
	#include "stm32l4xx_hal.h"
	#include "stm32l4xx_hal_tim.h"
	
	static TIM_HandleTypeDef    TimHandle;
	//#define TIMPROFILER_IRQn    TIM7_IRQn
	#define TIMPROFILER_TIMER   TIM7
	#define TIMPROFILER_QUANT 60000
	
#endif



#ifdef __cplusplus
 extern "C" {
#endif 

	 
#ifdef PROFILER_HAL_STM32F432
void TIM7_IRQHandler ()
{
  if (__HAL_TIM_GET_FLAG (&TimHandle, TIM_FLAG_UPDATE) != RESET)
		{
    if (__HAL_TIM_GET_IT_SOURCE (&TimHandle, TIM_FLAG_UPDATE) != RESET)
			{
			overflow_timer++;
			f_interupt_overflow = true;
      __HAL_TIM_CLEAR_IT (&TimHandle, TIM_FLAG_UPDATE);
			}
		}
}
#endif


#ifdef __cplusplus
}
#endif





TPROFILER::TPROFILER ()
{

}



void TPROFILER::Init ()
{
	uint32_t uwPrescalerValue = 0;
	
#ifdef PROFILER_HAL_STM32F432

	GPIO_InitTypeDef GPIO_InitStructure;
	
	__HAL_RCC_TIM7_CLK_ENABLE ();

	// частота тактирования - 1 Мгц
  uwPrescalerValue = (uint32_t)(SystemCoreClock / 1000000) - 1;
  TimHandle.Instance = TIMPROFILER_TIMER;
  TimHandle.Init.Period            = TIMPROFILER_QUANT - 1;
  TimHandle.Init.Prescaler         = uwPrescalerValue;
  TimHandle.Init.ClockDivision     = 0;
  TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
  TimHandle.Init.RepetitionCounter = 0;
	HAL_TIM_Base_Init (&TimHandle);
	__HAL_TIM_ENABLE_IT (&TimHandle, TIM_IT_UPDATE);
	__HAL_TIM_ENABLE (&TimHandle);
  HAL_NVIC_SetPriority (TIMPROFILER_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ (TIMPROFILER_IRQn);

	
	
	
	/*
  __HAL_RCC_TIM2_CLK_ENABLE();
  uwPrescalerValue = (uint32_t)(SystemCoreClock / 1000000) - 1;
  TimHandle.Instance = TIMPROFILER_TIMER;
  TimHandle.Init.Period            = TIMPROFILER_QUANT - 1;
  TimHandle.Init.Prescaler         = uwPrescalerValue;
  TimHandle.Init.ClockDivision     = 0;
  TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
  TimHandle.Init.RepetitionCounter = 0;
	HAL_TIM_Base_Init (&TimHandle);
	__HAL_TIM_ENABLE_IT (&TimHandle, TIM_IT_UPDATE);
	__HAL_TIM_ENABLE (&TimHandle);
	
  HAL_NVIC_SetPriority (TIMPROFILER_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ (TIMPROFILER_IRQn);
	*/
	
Clear ();
#endif
	
	
}


#ifdef PROFILER_HAL_STM32F432
unsigned long TPROFILER::get_rawticks ()
{
	return  (overflow_timer * TIMPROFILER_QUANT) + TIMPROFILER_TIMER->CNT;
}



unsigned long TPROFILER::get_proticker ()
{
unsigned long rv = 0;
while (true)
	{
	f_interupt_overflow = false;
	rv = get_rawticks ();
	if (!f_interupt_overflow) break;
	}
return rv;
}



void TPROFILER::Clear ()
{
	unsigned long ix = 0;
	while (ix < C_PROFLR_POINT_AMOUNT)
		{
		Clear_Statistics (ix);
		ix++;
		}
}



void TPROFILER::Clear_Statistics (long pnt_ix)
{
	if (pnt_ix < C_PROFLR_POINT_AMOUNT)
		{
		point[pnt_ix].min_time = 0xFFFFFFFF;//  = get_proticker ();
		point[pnt_ix].max_time = 0;
		}
}




void TPROFILER::Start_Point (long pnt_ix)
{
	if (pnt_ix < C_PROFLR_POINT_AMOUNT)
		{
		point[pnt_ix].cur_ticks = get_proticker ();
		}
}



void TPROFILER::Stop_Point (long pnt_ix)
{
	if (pnt_ix < C_PROFLR_POINT_AMOUNT)
		{
		unsigned long cticks = get_proticker ();
		unsigned long delt = cticks - point[pnt_ix].cur_ticks;
		if (delt > point[pnt_ix].max_time) point[pnt_ix].max_time = delt;
		if (delt < point[pnt_ix].min_time) point[pnt_ix].min_time = delt;
		}
}
#endif

