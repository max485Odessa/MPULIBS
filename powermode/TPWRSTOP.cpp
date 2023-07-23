#include "TPWRSTOP.h"
#include "stm32f10x_iwdg.h"
#include "stm32f10x_gpio.h"



#ifdef __cplusplus
extern "C" {
#endif

void RTCAlarm_IRQHandler(void)
{
  if (RTC_GetITStatus (RTC_IT_ALR) != RESET)
		{
    /* Clear EXTI line17 pending bit */
    EXTI_ClearITPendingBit(EXTI_Line17);
    /* Check if the Wake-Up flag is set */
    if (PWR_GetFlagStatus(PWR_FLAG_WU) != RESET)
			{
      /* Clear Wake Up flag */
      PWR_ClearFlag(PWR_FLAG_WU);
			}
    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();   
    /* Clear RTC Alarm interrupt pending bit */
    RTC_ClearITPendingBit(RTC_IT_ALR);
    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();
		TPWR::SYSCLKConfig_STOP ();
		}
}



#ifdef __cplusplus
}
#endif



void TPWR::Init ()
{
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	
  /* Enable PWR and BKP clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
  /* Configure EXTI Line to generate an interrupt on falling edge */

  /* Configure EXTI Line17(RTC Alarm) to generate an interrupt on rising edge */
  EXTI_ClearITPendingBit(EXTI_Line17);
  EXTI_InitStructure.EXTI_Line = EXTI_Line17;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  /* Configure RTC clock source and prescaler */
  /* RTC clock source configuration ------------------------------------------*/
  /* Allow access to BKP Domain */
  PWR_BackupAccessCmd(ENABLE);

  /* Reset Backup Domain */
  BKP_DeInit();
  
  /* Enable the LSE OSC */
  RCC_LSEConfig(RCC_LSE_ON);
  /* Wait till LSE is ready */
  while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
  {
  }

  /* Select the RTC Clock Source */
  RCC_RTCCLKConfig (RCC_RTCCLKSource_LSE);

  /* Enable the RTC Clock */
  RCC_RTCCLKCmd (ENABLE);

  /* RTC configuration -------------------------------------------------------*/
  /* Wait for RTC APB registers synchronisation */
  RTC_WaitForSynchro();

  /* Set the RTC time base to 1ms or 1s */
  RTC_SetPrescaler (C_RTC_PRESCALER);  
  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();

  /* Enable the RTC Alarm interrupt */
  RTC_ITConfig(RTC_IT_ALR, ENABLE);
  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();
  /* NVIC configuration */
  /* 2 bits for Preemption Priority and 2 bits for Sub Priority */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

  NVIC_InitStructure.NVIC_IRQChannel = RTCAlarm_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}



void TPWR::SYSCLKConfig_STOP (void)
{
	ErrorStatus HSEStartUpStatus;
  /* Enable HSE */
  RCC_HSEConfig (RCC_HSE_ON);
  /* Wait till HSE is ready */
  HSEStartUpStatus = RCC_WaitForHSEStartUp();

  if (HSEStartUpStatus == SUCCESS)
		{
    /* Enable PLL */ 
    RCC_PLLCmd (ENABLE);
    /* Wait till PLL is ready */
    while (RCC_GetFlagStatus (RCC_FLAG_PLLRDY) == RESET)
			{
			}
    /* Select PLL as system clock source */
    RCC_SYSCLKConfig (RCC_SYSCLKSource_PLLCLK);
    /* Wait till PLL is used as system clock source */
    while (RCC_GetSYSCLKSource () != 0x08)
			{
			}
		}
}



void TPWR::Sync ()
{
	RTC_SetCounter (C_RTC_BEACON_START_VALUE);
}


// ¬ход в sleep режим
void TPWR::ToSleep (unsigned short ms)
{
	RTC_ClearFlag(RTC_FLAG_SEC);
	while (RTC_GetFlagStatus(RTC_FLAG_SEC) == RESET);
	RTC_SetAlarm(RTC_GetCounter()+ ms);
	RTC_WaitForLastTask();
	PWR_EnterSTOPMode (PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
}



// вход в standby режим и выход с него по Reset от IWDT
// врем€ standby в милисекундах
void TPWR::StandbyWitch_IWDG (unsigned short Time_sleep)
{
	// программруем ватчдог
  IWDG_WriteAccessCmd (IWDG_WriteAccess_Enable);		
  IWDG_SetPrescaler (IWDG_Prescaler_64);		// устанавливаем период 0.0016
	float val = Time_sleep;
	val /= 1.6;										// теперь имеем значение дл€ RLR
	if (val > 4095) val = 4095;		// 12 бит максимум
	IWDG_SetReload (val);
  IWDG_ReloadCounter();
  IWDG_Enable();
	// программируем режим и входим в standby
	RCC_APB1PeriphClockCmd (RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	//PWR_WakeUpPinCmd (FunctionalState NewState);
	PWR_EnterSTANDBYMode ();
}


// сброс счетчика ватчдога
void TPWR::WDG_Clear ()
{
	IWDG_ReloadCounter();
}


// вход в standby режим и выход с него по Reset от IWDT или по кнопке WKUP
// врем€ standby в милисекундах
void TPWR::StandbyWitch_IWDG_and_WkupPin (unsigned short Time_sleep)
{
	// wkup init
	/*
	RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
  GPIO_Init (GPIOA, &GPIO_InitStructure);
	*/
	
	PWR_WakeUpPinCmd (ENABLE);
	StandbyWitch_IWDG (Time_sleep);
}
	

