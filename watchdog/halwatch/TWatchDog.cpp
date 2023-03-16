#include "TWatchDog.h"


extern IWDG_HandleTypeDef hiwdg;
bool TWATCH::f_prev_reset = false;


void TWATCH::Init (unsigned short mstime)
{
	if (mstime >= 2)
		{
		if (mstime > 4095) mstime = 4095;		// максимальная величина wdr счетчика 12 бит
			
		hiwdg.Instance = IWDG;
		hiwdg.Init.Reload = mstime;
		hiwdg.Init.Prescaler = IWDG_PRESCALER_256;
		hiwdg.Init.Window = IWDG_WINDOW_DISABLE;//IWDG_WINDOW_DISABLE;
		HAL_IWDG_Init (&hiwdg);

		Wdr ();
		}
}




void TWATCH::Wdr ()
{
	__HAL_IWDG_RELOAD_COUNTER (&hiwdg);
}



