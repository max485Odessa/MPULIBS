#ifndef ___SYSBIOS_H_
#define ___SYSBIOS_H_

#include "config_sysbios.h"



typedef unsigned long utimer_t;



typedef struct {
	void (*lpStaticRutine)(void *param);
	void *lpThisObject;
} CALLBK_CLASS_STATIC;



namespace SYSBIOS {


extern  unsigned char LastDeltaTicks;
unsigned long GetTickCountLong();
unsigned short GetTickCountShort();
unsigned char GetTickCountChar();


typedef struct {
	unsigned char Status;			
	CALLBK_CLASS_STATIC ParamStart;
	unsigned long DelayTime;
	unsigned long tmpDT;
} PPTHREAD;



typedef struct {
	CALLBK_CLASS_STATIC ParamStart;
	utimer_t *TimerCallBack;
} CALLBACKTIMER;



char AddPeriodicThread_ISR (CALLBK_CLASS_STATIC *lpParam, utimer_t DelayTime);
char AddPeriodicThread_ISR_A (void *lpThis, void *lpStaticProc, utimer_t DelayTime);
char DelPeriodicThread_ISR (CALLBK_CLASS_STATIC *lpParam);
char DelPeriodicThread_ISR_A (void *lpThis, void *lpStaticProc);
char ADD_CB_TIMER_ISR (void *lpThis, void *lpStaticProc, utimer_t *lpSTim);	// по истечении таймера 1 раз запускает указанную процедуру
char DEL_CB_TIMER_ISR (utimer_t *lpSTim);
char ADD_TIMER_ISR (utimer_t *lpSTim);
char DEL_TIMER_ISR (utimer_t *lpSTim);
char SetThreadStatus_ISR (void *lpStaticProc, char Stats);
char AddPeriodicThread (CALLBK_CLASS_STATIC *lpParam, utimer_t DelayTime);
char AddPeriodicThread_A (void *lpThis, void *lpStaticProc, utimer_t DelayTime);
char DelPeriodicThread (CALLBK_CLASS_STATIC *lpParam);
char DelPeriodicThread_A (void *lpThis, void *lpStaticProc);
char ADD_CB_TIMER (void *lpThis, void *lpStaticProc, utimer_t *lpSTim);
char DEL_CB_TIMER (utimer_t *lpSTim);
char ADD_TIMER_SYS (utimer_t *lpSTim);
char DEL_TIMER_SYS (utimer_t *lpSTim);
void Wait (utimer_t timee);
void ExecuteClassStaticProc (CALLBK_CLASS_STATIC *lpSTparam);

void EXECUTE_PERIODIC_ISR ();   // нужно поместить в 1 мс прерывания
void EXECUTE_PERIODIC_SYS ();   // нужно поместить в системный поток для периодического вызова не реже пару милисекунд
}


#endif

