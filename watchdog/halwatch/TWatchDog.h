#ifndef _H_TWATCH_STM32L4XX_H_
#define _H_TWATCH_STM32L4XX_H_



#include "stm32l432xx.h"
#include "stm32l4xx_hal_iwdg.h"


const float C_WDTI_CLOCK_PERIOD = (float)0.008;		// 125 דצ
const float C_WATCH_TIME_4S = ((float)4.0 / C_WDTI_CLOCK_PERIOD);
const float C_WATCH_TIME_2S = ((float)2.0 / C_WDTI_CLOCK_PERIOD);
const float C_WATCH_TIME_1S = ((float)1.0 / C_WDTI_CLOCK_PERIOD);
const float C_WATCH_TIME_05S = ((float)0.5 / C_WDTI_CLOCK_PERIOD);
const float C_WATCH_TIME_03S = ((float)0.3 / C_WDTI_CLOCK_PERIOD);

class TWATCH {
		static IWDG_HandleTypeDef hndl;
		static bool f_prev_reset;
		static uint32_t LsiFreq;
	
	public:
		static void Init (unsigned short times);
		static void Wdr ();
};


#endif


