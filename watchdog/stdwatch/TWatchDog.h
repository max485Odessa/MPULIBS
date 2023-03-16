#ifndef _H_TWATCH_STM32F103_H_
#define _H_TWATCH_STM32F103_H_



#include "stm32f10x.h"
#include "stm32f10x_iwdg.h"
#include "stm32f10x_rcc.h"

const float C_WDTI_CLOCK_PERIOD64 = 0.0016;

const unsigned short C_WATCH_TIME_4S = 4 / C_WDTI_CLOCK_PERIOD64;
const unsigned short C_WATCH_TIME_2S = 2 / C_WDTI_CLOCK_PERIOD64;
const unsigned short C_WATCH_TIME_1S = 1 / C_WDTI_CLOCK_PERIOD64;
const unsigned short C_WATCH_TIME_05S = 0.5 / C_WDTI_CLOCK_PERIOD64;


class TWATCH {
		static bool f_prev_reset;
		static uint32_t LsiFreq;
	public:
		static void Init (unsigned short times);
		static void Wdr ();
};


#endif


