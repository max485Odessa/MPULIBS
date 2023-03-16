#include "TWatchDog.h"

uint32_t TWATCH::LsiFreq = 40000;
bool TWATCH::f_prev_reset = false;


void TWATCH::Init (unsigned short mstime)
{
		float clk_time = mstime;
		clk_time = clk_time / C_WDTI_CLOCK_PERIOD64;		// константа должна соответствовать временному интервалу тактирования IWDG_Prescaler_64
		if (clk_time > 4095) clk_time = 4095;		// максимальная величина wdr счетчика 12 бит
		if (RCC_GetFlagStatus (RCC_FLAG_IWDGRST) != RESET)
			{
			f_prev_reset = true;
			RCC_ClearFlag();
			}
		IWDG_WriteAccessCmd (IWDG_WriteAccess_Enable);
		IWDG_SetPrescaler (IWDG_Prescaler_64); 
		IWDG_SetReload (clk_time);
		if (mstime)
			{
			Wdr ();
			IWDG_Enable ();
			}
}



void TWATCH::Wdr ()
{
	IWDG_ReloadCounter();
}



