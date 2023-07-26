#ifndef _H_KEY_CLASS_WINDER_ONE_STM32_H_
#define _H_KEY_CLASS_WINDER_ONE_STM32_H_

#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "SYSBIOS.H"
#include "TFTASKIF.h"


#define PIN_KEY GPIO_Pin_0
#define PORT_KEYS GPIOA

enum EKEYCODE {EKEYCODE_NONE = 0, EKEYCODE_KEY = 1, EKEYCODE_ENDENUM = 2};	
const unsigned short C_KEYPIN_MASKS[EKEYCODE_ENDENUM] = {0, PIN_KEY};

#define C_KEYTIME_TO_MENU 2000
#define C_KEYTIME_ESC 2000
#define C_SKIP_TIME_500MS  500
#define C_MINPUSH_TIME 2

class TKEY: public TFFC {
	private:
		bool LastKeyState[EKEYCODE_ENDENUM];					// текущее состояние
		bool BlockToKeyPop[EKEYCODE_ENDENUM];					// блокировка клавиши до ее отжатия
		EKEYCODE LastKeyCode;													// текущий код клавиши
		unsigned long last_time_push;
		unsigned long skip_time;
		unsigned long PushTimers[EKEYCODE_ENDENUM];
		virtual void Task ();
	
	public:
		TKEY ();
		void Init ();
		
		void Task_1ms_timers ();
	
		EKEYCODE GetKey (unsigned long *time_push);
		unsigned long GetPushTimer (EKEYCODE k_itm);
		void ClearPushTimer (EKEYCODE k_itm);					
		void SkipKey (EKEYCODE k_itm);								// если клавиша нажата и надо заблокировать формирование кода клавиши по отпусканию единоразово
		void SkipTimer (unsigned long val_ms);				// блокирует формирование клавиши
		void Clear_Key ();
};

#endif


