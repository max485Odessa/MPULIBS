#include "key_class.h"



TKEY::TKEY ()
{
	LastKeyCode = EKEYCODE_NONE;
	AddObjectToExecuteManager ();
}



void TKEY::Init ()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Pin = PIN_KEY;
	GPIO_Init (PORT_KEYS, &GPIO_InitStructure);
}



void TKEY::Task_1ms_timers ()
{
unsigned char start_inx = EKEYCODE_KEY;
while (start_inx < EKEYCODE_ENDENUM) {
	if (LastKeyState[start_inx]) {
		if (PushTimers[start_inx] < 0xFFFFFFFF) PushTimers[start_inx]++;
		}
	start_inx++;
	}
if (skip_time) skip_time--;
}



unsigned long TKEY::GetPushTimer (EKEYCODE k_itm)
{
unsigned long rv = 0;
if (k_itm < EKEYCODE_ENDENUM) rv = PushTimers[k_itm];
return rv;
}



void TKEY::ClearPushTimer (EKEYCODE k_itm)
{
if (k_itm < EKEYCODE_ENDENUM) PushTimers[k_itm] = 0;
}



void TKEY::SkipKey (EKEYCODE k_itm)
{
if (k_itm < EKEYCODE_ENDENUM) 
	{
	LastKeyState[k_itm] = false;
	PushTimers[k_itm] = 0;
	BlockToKeyPop[k_itm] = true;
	SkipTimer (100);
	}
}

	

EKEYCODE TKEY::GetKey (unsigned long *time_push)
{
EKEYCODE rv = EKEYCODE_NONE;
if (LastKeyCode != EKEYCODE_NONE && time_push) *time_push = last_time_push;
rv = LastKeyCode;
LastKeyCode = EKEYCODE_NONE;
return rv;
}



void TKEY::Clear_Key ()
{
LastKeyCode = EKEYCODE_NONE;
last_time_push = 0;
}



void TKEY::SkipTimer (unsigned long val_ms)
{
	skip_time = val_ms;
}



void TKEY::Task ()
{
	unsigned char ix = EKEYCODE_KEY;
	uint16_t raw_pin = GPIO_ReadInputData (PORT_KEYS);
	while (ix < EKEYCODE_ENDENUM)
		{
		if (!(raw_pin & C_KEYPIN_MASKS[ix]))
			{
			if (!BlockToKeyPop[ix]) LastKeyState[ix] = true;
			}
		else
			{
			BlockToKeyPop[ix] = false;
			if (LastKeyState[ix])
				{
				LastKeyState[ix] = false;
				if (PushTimers[ix] > C_MINPUSH_TIME)
					{
					LastKeyCode = (EKEYCODE)ix;
					last_time_push = PushTimers[ix];
					//break;
					}
				}
			PushTimers[ix] = 0;
			}
		ix++;
		}

	if (LastKeyCode != EKEYCODE_NONE) {
		if (skip_time) LastKeyCode = EKEYCODE_NONE;
		}
}


		
