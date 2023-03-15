#include "SYSBIOS.H"



namespace SYSBIOS {
volatile unsigned long TickCount_Timer = 0;
CALLBACKTIMER CBTIMERS_ISR[C_MAXTIMCB_ISR];
volatile long INX_CBTIMER_ISR = 0;
CALLBACKTIMER CBTIMERS[C_MAXTIMCB];
volatile long INX_CBTIMER = 0;
PPTHREAD THREADsPeriodics_ISR[C_MAXPPROC_ISR];
PPTHREAD THREADsPeriodics[C_MAXPPROC];
volatile unsigned long INX_PPROC_ISR = 0;
volatile unsigned long INX_PPROC = 0;
volatile char F_BlockPerThreCalls = 0;
utimer_t *TIMER_S[C_MAXTIMSYS];
volatile unsigned char STMR_INDX_S = 0;
utimer_t *TIMER_I[C_MAXTIMINT];
volatile unsigned char STMR_INDX_I = 0;
unsigned char LastDeltaTicks = 0;






unsigned long GetTickCountLong()
{
return TickCount_Timer;
}



unsigned short GetTickCountShort()
{
return TickCount_Timer;
}



unsigned char GetTickCountChar()
{
return TickCount_Timer;
}



unsigned long FindIndxTimer_S (utimer_t *lpSTim)
{
unsigned long rv = 0;
if (STMR_INDX_S)
	{
	unsigned long Indx = 0;
	while (Indx < STMR_INDX_S)
		{
		if (TIMER_S[Indx] == lpSTim)
			{
			rv = Indx + 1;
			break;
			}
		Indx++;
		}
	}
return rv;
}



unsigned long FindIndxTimer_ISR (utimer_t *lpSTim)
{
unsigned long rv = 0;
if (STMR_INDX_I)
	{
	unsigned long Indx = 0;
	while (Indx < STMR_INDX_I)
		{
		if (TIMER_I[Indx] == lpSTim)
			{
			rv = Indx + 1;
			break;
			}
		Indx++;
		}
	}
return rv;
}






char ADD_TIMER_SYS (utimer_t *lpSTim)
{
char rv =0;
if (STMR_INDX_S < C_MAXTIMSYS)
	{
	unsigned long Ifnd = FindIndxTimer_S (lpSTim);
	if (!Ifnd)
		{
		TIMER_S[STMR_INDX_S] = lpSTim;
		STMR_INDX_S++;
		}
	rv = 1;
	}
return rv;
}



char ADD_TIMER_ISR (utimer_t *lpSTim)
{
char rv =0;
if (STMR_INDX_I < C_MAXTIMINT)
	{
	unsigned long Ifnd = FindIndxTimer_ISR (lpSTim);
	if (!Ifnd)
		{
		TIMER_I[STMR_INDX_I] = lpSTim;
		STMR_INDX_I++;
		}
	rv = 1;
	}
return rv;
}



char DEL_TIMER_SYS (utimer_t *lpSTim)
{
char rv =0;
unsigned long Ifnd = FindIndxTimer_S (lpSTim);
if (Ifnd)
	{
	unsigned long IndSrc = STMR_INDX_S - 1;
	Ifnd--;
	unsigned long IndDst = Ifnd;
	if (IndSrc > IndDst)
		{
		unsigned long CntMov = IndSrc - IndDst;
		IndSrc = IndDst + 1;
		while (CntMov)
			{
			TIMER_S[IndDst] = TIMER_S[IndSrc];
			IndSrc++;
			IndDst++;
			CntMov--;
			}
		}
	STMR_INDX_S--;
	}
return rv;
}



char DEL_TIMER_ISR (utimer_t *lpSTim)
{
char rv =0;
unsigned long Ifnd = FindIndxTimer_ISR (lpSTim);
if (Ifnd)
	{
	unsigned long IndSrc = STMR_INDX_I - 1;
	Ifnd--;
	unsigned long IndDst = Ifnd;
	if (IndSrc > IndDst)
		{
		unsigned long CntMov = IndSrc - IndDst;
		IndSrc = IndDst + 1;
		while (CntMov)
			{
			TIMER_I[IndDst] = TIMER_I[IndSrc];
			IndSrc++;
			IndDst++;
			CntMov--;
			}
		}
	STMR_INDX_I--;
	}
return rv;
}




void TIMER_RUTINE_S (utimer_t DPTime)
{
unsigned char Indxs = 0;
utimer_t *lpLNG;
utimer_t DT;
while (Indxs < STMR_INDX_S)
		{
		lpLNG = TIMER_S[Indxs];
		DT = lpLNG[0];
		if (DT >= DPTime)
				{
				DT = DT - DPTime;
				}
		else
				{
				DT = 0;
				}
		lpLNG[0] = DT;
		Indxs++;
		}
}




void TIMER_RUTINE_I ()
{
unsigned char Indxs = 0;
utimer_t *lpLNG;
utimer_t DT;
utimer_t DPTime = 1;
while (Indxs < STMR_INDX_I)
		{
		lpLNG = TIMER_I[Indxs];
		DT = lpLNG[0];
		if (DT >= DPTime)
			{
			DT = DT - DPTime;
			}
		else
			{
			DT = 0;
			}
		lpLNG[0] = DT;
		Indxs++;
		}
}




unsigned long FICBP_tmp (unsigned long *MaxRegs, PPTHREAD *lTabl, CALLBK_CLASS_STATIC *lpParam)
{
unsigned long rv = 0;
if (MaxRegs[0])
	{
	unsigned long Indx = 0;
	while (Indx < MaxRegs[0])
		{
		PPTHREAD *lpCTH = (PPTHREAD*) &lTabl[Indx];
		if (lpCTH->ParamStart.lpStaticRutine == lpParam->lpStaticRutine && lpCTH->ParamStart.lpThisObject == lpParam->lpThisObject)
			{
			rv = Indx + 1;
			break;
			}
		Indx++;
		}
	}
return rv;
}



unsigned long FindIndxCallBackPeriodic_ISR (CALLBK_CLASS_STATIC *lpParam)
{
return FICBP_tmp ((unsigned long*)&INX_PPROC_ISR, THREADsPeriodics_ISR, lpParam);
}



unsigned long FindIndxCallBackPeriodic (CALLBK_CLASS_STATIC *lpParam)
{
return FICBP_tmp ((unsigned long*)&INX_PPROC, THREADsPeriodics, lpParam);
}






char AddPeriodicThread_ISR (CALLBK_CLASS_STATIC *lpParam, utimer_t DelayTime)
{
char rv = 0;
if (INX_PPROC_ISR < C_MAXPPROC_ISR && lpParam)
        {
        unsigned long IndxC = FindIndxCallBackPeriodic_ISR (lpParam);   // проверить и не добавлять если уже присутствует
        if (!IndxC)
            {
            PPTHREAD *lpCTH = (PPTHREAD*) &THREADsPeriodics_ISR[INX_PPROC_ISR];
            lpCTH->DelayTime = DelayTime;
            lpCTH->tmpDT  = 0;
            lpCTH->ParamStart.lpStaticRutine = lpParam->lpStaticRutine;
            lpCTH->ParamStart.lpThisObject = lpParam->lpThisObject;
            lpCTH->Status = 1;
            INX_PPROC_ISR++;
            }
        rv = 1;
        }
return rv;
}




char AddPeriodicThread (CALLBK_CLASS_STATIC *lpParam, utimer_t DelayTime)
{
char rv = 0;
if (INX_PPROC < C_MAXPPROC && lpParam)
        {
        unsigned long IndxC = FindIndxCallBackPeriodic (lpParam);   // проверить и не добавлять если уже присутствует
        if (!IndxC)
            {
            PPTHREAD *lpCTH = (PPTHREAD*) &THREADsPeriodics[INX_PPROC];
            lpCTH->DelayTime = DelayTime;
            lpCTH->tmpDT  = 0;
            lpCTH->ParamStart.lpStaticRutine = lpParam->lpStaticRutine;
            lpCTH->ParamStart.lpThisObject = lpParam->lpThisObject;
            lpCTH->Status = 1;
            INX_PPROC++;
            }
        rv = 1;
        }
return rv;
}



char DPTh_tmp (unsigned long *lIndxMax, unsigned long *Ifnd, PPTHREAD *ltable)
{
char rv = 0;
if (lIndxMax[0])
    {
	unsigned long IndSrc = lIndxMax[0] - 1;
	Ifnd[0]--;
	unsigned long IndDst = Ifnd[0];
	if (IndSrc > IndDst)
		{
		unsigned long CntMov = IndSrc - IndDst;
		IndSrc = IndDst + 1;
		while (CntMov)
			{
			ltable[IndDst] = ltable[IndSrc];
			IndSrc++;
			IndDst++;
			CntMov--;
			}
		}
	lIndxMax[0]--;
    rv = 1;
    }
return rv;
}




char DelPeriodicThread_ISR (CALLBK_CLASS_STATIC *lpParam)  // utimer_t *lpSTim
{
char rv =0;
unsigned long Ifnd = FindIndxCallBackPeriodic_ISR (lpParam);
if (Ifnd) rv = DPTh_tmp ((unsigned long *)&INX_PPROC_ISR, &Ifnd, THREADsPeriodics_ISR);
return rv;
}




char DelPeriodicThread (CALLBK_CLASS_STATIC *lpParam)  // utimer_t *lpSTim
{
char rv =0;
unsigned long Ifnd = FindIndxCallBackPeriodic (lpParam);
if (Ifnd) rv = DPTh_tmp ((unsigned long *)&INX_PPROC, &Ifnd, THREADsPeriodics);
return rv;
}



char DelPeriodicThread_ISR_A (void *lpThis, void *lpStaticProc)
{
CALLBK_CLASS_STATIC tmpP;
tmpP.lpStaticRutine = (void(*)(void *param))lpStaticProc;
tmpP.lpThisObject = lpThis;
return DelPeriodicThread_ISR (&tmpP);
}


char DelPeriodicThread_A (void *lpThis, void *lpStaticProc)
{
CALLBK_CLASS_STATIC tmpP;
tmpP.lpStaticRutine = (void(*)(void *param))lpStaticProc;
tmpP.lpThisObject = lpThis;
return DelPeriodicThread (&tmpP);
}





char AddPeriodicThread_ISR_A (void *lpThis, void *lpStaticProc, utimer_t DelayTime)
{
CALLBK_CLASS_STATIC tmpP;
tmpP.lpStaticRutine = (void(*)(void *param))lpStaticProc;
tmpP.lpThisObject = lpThis;
return SYSBIOS::AddPeriodicThread_ISR (&tmpP,DelayTime);
}



char AddPeriodicThread_A (void *lpThis, void *lpStaticProc, utimer_t DelayTime)
{
CALLBK_CLASS_STATIC tmpP;
tmpP.lpStaticRutine = (void(*)(void *param))lpStaticProc;
tmpP.lpThisObject = lpThis;
return SYSBIOS::AddPeriodicThread (&tmpP,DelayTime);
}



unsigned long FICBT_tmp (unsigned long *lIndxCnt, CALLBACKTIMER *lCbTm, utimer_t *lpSTim)
{
unsigned long rv = 0;
if (lIndxCnt[0])
	{
	unsigned long Indx = 0;
	CALLBACKTIMER *lpCBT;
	while (Indx < lIndxCnt[0])
		{
		lpCBT = &lCbTm[Indx];
		if (lpCBT->TimerCallBack && lpCBT->TimerCallBack == lpSTim)
			{
			rv = Indx + 1;
			break;
			}
		Indx++;
		}
	}
return rv;
}



unsigned long FindIndxCBck_Timer_ISR (utimer_t *lpSTim)
{
return FICBT_tmp ((unsigned long*)&INX_CBTIMER_ISR, CBTIMERS_ISR, lpSTim);
}



unsigned long FindIndxCBck_Timer (utimer_t *lpSTim)
{
return FICBT_tmp ((unsigned long*)&INX_CBTIMER, CBTIMERS, lpSTim);
}



char DLCBT_tmp (unsigned long *lInxCnt, unsigned long *Ifnd, CALLBACKTIMER *lTabl)
{
char rv = 0;
if (Ifnd[0])
	{
	unsigned long IndSrc = lInxCnt[0] - 1;
	Ifnd[0]--;
	unsigned long IndDst = Ifnd[0];
	if (IndSrc > IndDst)
		{
		unsigned long CntMov = IndSrc - IndDst;
		IndSrc = IndDst + 1;
		while (CntMov)
			{
			lTabl[IndDst] = lTabl[IndSrc];
			IndSrc++;
			IndDst++;
			CntMov--;
			}
		}
	lInxCnt[0]--;
    rv = true;
	}
return rv;
}



char DEL_CB_TIMER_ISR (utimer_t *lpSTim)
{
char rv =0;
unsigned long Ifnd = FindIndxCBck_Timer_ISR (lpSTim);
if (Ifnd) rv = DLCBT_tmp ((unsigned long*)&INX_CBTIMER_ISR, &Ifnd, CBTIMERS_ISR);
return rv;
}



char DEL_CB_TIMER (utimer_t *lpSTim)
{
char rv =0;
unsigned long Ifnd = FindIndxCBck_Timer (lpSTim);
if (Ifnd) rv = DLCBT_tmp ((unsigned long*)&INX_CBTIMER, &Ifnd, CBTIMERS);
return rv;
}




char ADD_CB_TIMER_ISR (void *lpThis, void *lpStaticProc, utimer_t *lpSTim)
{
char rv = 0;
if (INX_CBTIMER_ISR < C_MAXTIMCB_ISR && lpStaticProc)
        {
        unsigned long Ifnd = FindIndxCBck_Timer_ISR (lpSTim);
        if (!Ifnd)
            {
            CALLBACKTIMER *lpCBT = &CBTIMERS_ISR[INX_CBTIMER_ISR];
            lpCBT->ParamStart.lpStaticRutine = (void (*)(void *))lpStaticProc;
            lpCBT->ParamStart.lpThisObject = lpThis;
            lpCBT->TimerCallBack = lpSTim;
            INX_CBTIMER_ISR++;
            }
		rv = 1;
        }
return rv;
}




char ADD_CB_TIMER (void *lpThis, void *lpStaticProc, utimer_t *lpSTim)
{
char rv = 0;
if (INX_CBTIMER < C_MAXTIMCB && lpStaticProc)
        {
        unsigned long Ifnd = FindIndxCBck_Timer (lpSTim);
        if (!Ifnd)
            {
            CALLBACKTIMER *lpCBT = &CBTIMERS[INX_CBTIMER];
            lpCBT->ParamStart.lpStaticRutine = (void (*)(void *))lpStaticProc;
            lpCBT->ParamStart.lpThisObject = lpThis;
            lpCBT->TimerCallBack = lpSTim;
            INX_CBTIMER++;
            }
		rv = 1;
        }
return rv;
}



void ExecutePeriodic_CBTimers_ISR ()
{
if (INX_CBTIMER_ISR)
    {
    unsigned char CINDX = 0;
    while (CINDX < INX_CBTIMER_ISR)
        {
        CALLBACKTIMER *lpCTH = &CBTIMERS_ISR[CINDX];
        if (lpCTH->TimerCallBack)
            {
            utimer_t tmr = lpCTH->TimerCallBack[0];
            if (tmr)
                {
                tmr--;
								lpCTH->TimerCallBack[0] = tmr;
                if (!tmr)
                    {
                    ExecuteClassStaticProc (&lpCTH->ParamStart);
                    }
                }
            }
        CINDX++;
        }
    }
}





char SetThreadStatus_ISR (void *lpStaticProc, char Stats)
{
char rv = 0;
    if (INX_PPROC_ISR)
        {
        unsigned char CINDX = 0;
        while (CINDX < INX_PPROC_ISR)
            {
            PPTHREAD *lpCTH = (PPTHREAD*)&THREADsPeriodics_ISR[CINDX];
            if ((void*)lpCTH->ParamStart.lpStaticRutine == lpStaticProc)
                {
                lpCTH->Status = Stats;
                rv = 1;
                break;
                }
            CINDX++;
            }
        }
return rv;
}



void ExecuteClassStaticProc (CALLBK_CLASS_STATIC *lpSTparam)
{
if (!F_BlockPerThreCalls)
    {
    if (lpSTparam->lpStaticRutine)
      {
      if (lpSTparam->lpThisObject)
        {
        void (*SPCBk) (void *lpObj);
        SPCBk = lpSTparam->lpStaticRutine;
        SPCBk (lpSTparam->lpThisObject);
        }
      else
        {
        void (*SPCBk) (void);
        SPCBk = (void (*)())lpSTparam->lpStaticRutine;
        SPCBk ();	
        }
      }
    }
}



void EXECUTE_PERIODIC_ISR ()
{
if (INX_PPROC_ISR)
		{
		unsigned char CINDX = 0;
		while (CINDX < INX_PPROC_ISR)
				{
				PPTHREAD *lpCTH = (PPTHREAD*)&THREADsPeriodics_ISR[CINDX];
				if (lpCTH->Status == 1)
						{

						if (lpCTH->tmpDT) lpCTH->tmpDT--;

						if (!lpCTH->tmpDT)
								{
								ExecuteClassStaticProc (&lpCTH->ParamStart);
								lpCTH->tmpDT = lpCTH->DelayTime;
								}
						}
				CINDX++;
				}
		}
ExecutePeriodic_CBTimers_ISR ();
TIMER_RUTINE_I ();
TickCount_Timer++;
}



void ExecutePeriodic_CBTimers ()
{
if (INX_CBTIMER)
	{
	unsigned char CINDX = 0;
	while (CINDX < INX_CBTIMER)
		{
		CALLBACKTIMER *lpCTH = &CBTIMERS[CINDX];
		if (lpCTH->TimerCallBack)
			{
			utimer_t tmr = lpCTH->TimerCallBack[0];
			if (tmr)
				{
				tmr--;
				lpCTH->TimerCallBack[0] = tmr;
				if (!tmr)
					{
					ExecuteClassStaticProc (&lpCTH->ParamStart);
					}
				}
			}
		CINDX++;
		}
	}
}



void Wait (utimer_t timee)
{
utimer_t cr_tm = GetTickCountLong(), dlt;
while (true)
	{
	dlt = GetTickCountLong() - cr_tm;
	if (dlt > timee) break;
	}
}



void EXECUTE_PERIODIC_SYS ()
{
static unsigned long LastTicks = 0;
unsigned long DltT = TickCount_Timer - LastTicks;
if (DltT)
	{
	if (INX_PPROC)
		{
		unsigned char CINDX = 0;
		while (CINDX < INX_PPROC)
			{
			PPTHREAD *lpCTH = (PPTHREAD*)&THREADsPeriodics[CINDX];
			if (lpCTH->Status == 1 && DltT)
				{
				if (DltT >= lpCTH->tmpDT)
					{
					ExecuteClassStaticProc (&lpCTH->ParamStart);
					lpCTH->tmpDT = lpCTH->DelayTime;
					}
				else
					{
					lpCTH->tmpDT -= DltT;
					}
				}
			CINDX++;
			}
		}
	LastTicks = TickCount_Timer;
	TIMER_RUTINE_S (DltT);
	ExecutePeriodic_CBTimers ();
	}
}



};















