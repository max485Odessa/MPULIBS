#ifndef __H_STM32F103PWR_STOP_MODE_H__
#define __H_STM32F103PWR_STOP_MODE_H__


#include "stm32f10x_pwr.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_bkp.h"
#include "stm32f10x_rtc.h"
#include "misc.h"


// „астота часового кварца 32768 гц, длительность тайслота в биконе 4 мс.
// ѕерепрограмируем делитель RTC тактировани€ на необходимый интервал.
// ƒл€ гибкости даем возможность установки интервалов сп€чки
// кратное 1 мс, дл€ этого предделитель будет равен 32

#define C_RTC_BEACON_START_VALUE 0xFFFFFFFF
#define C_RTC_PRESCALER 32 // 32768
#define C_LSI_FREQ 40000


class TPWR {
	private:
	
	protected:
	
	public:

		static void Init ();
		static void Sync ();	// устанавливает RTC_CNT таким, чтобы синхронизироватьс€ с координатором (запускать нужно в момент приема beacon пакета)
		static void ToSleep (unsigned short ms);
		static void SYSCLKConfig_STOP (void);
	
		static void StandbyWitch_IWDG (unsigned short Time_sleep);
		static void StandbyWitch_IWDG_and_WkupPin (unsigned short Time_sleep);
		static void WDG_Clear ();
		
};


#endif


