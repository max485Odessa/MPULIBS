#include "TENCODERS.h"



uint16_t IISRENCODER::hard_arr_vals[NUMAXIS];
volatile static uint16_t result_roll = 0;
TENCODERS *TENCODERS::singlobj = 0;

#ifdef __cplusplus
 extern "C" {
#endif 


volatile static uint16_t pitch_a = 0;
volatile static uint16_t roll_a = 0;
	 
	 
void EXTI2_TS_IRQHandler ()
{
//__disable_irq();
uint16_t valtim = TIM16->CNT;
//uint16_t valtim = TIM_GetCounter (TIM16);
  if (EXTI_GetITStatus (EXTI_Line2) != RESET)
  {
	IISRENCODER::hard_capt_roll (0xFFFF - (valtim - roll_a));
    EXTI_ClearITPendingBit(EXTI_Line2);
  }
//	__enable_irq();
}



void EXTI9_5_IRQHandler ()
{
//__disable_irq();
uint16_t valtim = TIM16->CNT;
//uint16_t valtim = TIM_GetCounter (TIM16);
	
	if (EXTI_GetITStatus (EXTI_Line6) != RESET)
		{
		pitch_a = valtim;
    EXTI_ClearITPendingBit(EXTI_Line6);
  }
		
	if (EXTI_GetITStatus (EXTI_Line7) != RESET)
		{
		roll_a = valtim;
    EXTI_ClearITPendingBit(EXTI_Line7);
  }
		

	//__enable_irq();
}



void EXTI15_10_IRQHandler ()
{
//	__disable_irq();
uint16_t valtim = TIM16->CNT;
  if (EXTI_GetITStatus(EXTI_Line13) != RESET)
  {
	IISRENCODER::hard_capt_pitch (0xFFFF - (valtim - pitch_a));
    EXTI_ClearITPendingBit(EXTI_Line13);
  }
	
//__enable_irq();
}


	 
/*
void TIM1_UP_TIM16_IRQHandler ()
{
	if (TIM_GetITStatus (TIM_PITCH, TIM_IT_CC1)==SET)
		{
		TRCHARDIF::hard_capt_pitch (TIM_GetCapture1 (TIM_PITCH));
		TIM_ClearITPendingBit (TIM_PITCH, TIM_IT_CC1);
		}
}



void TIM1_TRG_COM_TIM17_IRQHandler ()
{
	if (TIM_GetITStatus (TIM_ROLL, TIM_IT_CC1)==SET)
		{
		TRCHARDIF::hard_capt_roll (TIM_GetCapture1 (TIM_ROLL));
		TIM_ClearITPendingBit (TIM_ROLL, TIM_IT_CC1);
		}
}
*/



void TIM1_BRK_TIM15_IRQHandler ()
{
	if (TIM_GetITStatus (TIM_RCIN_YAW, TIM_IT_CC2)==SET)
		{
		IISRENCODER::hard_capt_yaw (TIM_GetCapture1 (TIM_RCIN_YAW));
		TIM_ClearITPendingBit (TIM_RCIN_YAW, TIM_IT_CC2);
		}
}

	 
	 
#ifdef __cplusplus
}
#endif






void IISRENCODER::hard_capt_pitch (uint16_t val)
{
	hard_arr_vals[PITCH] = val;
}



void IISRENCODER::hard_capt_roll (uint16_t val)
{
	hard_arr_vals[ROLL] = val;
	result_roll = val;
}



void IISRENCODER::hard_capt_yaw (uint16_t val)
{
	hard_arr_vals[YAW] = val;
}



// tim16
void TENCODERS::pitch_init ()
{
EXTI_InitTypeDef   EXTI_InitStructure;
GPIO_InitTypeDef   GPIO_InitStructure;
NVIC_InitTypeDef   NVIC_InitStructure;
	
	

  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	RCC_APB2PeriphClockCmd (RCC_APB2Periph_TIM16, ENABLE);
	uint16_t PrescalerValue = 0;
  /* Compute the prescaler value */
  PrescalerValue = (uint16_t) ((SystemCoreClock) / 72000000) - 1;

  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 65535;
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit (TIM_RCIN_PITCH, &TIM_TimeBaseStructure);
  TIM_PrescalerConfig (TIM_RCIN_PITCH, PrescalerValue, TIM_PSCReloadMode_Immediate);
	TIM_Cmd (TIM_RCIN_PITCH, ENABLE);
	
  RCC_AHBPeriphClockCmd (RCC_AHBPeriph_GPIOB, ENABLE);
  RCC_APB2PeriphClockCmd (RCC_APB2Periph_SYSCFG, ENABLE);
  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource13);
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource6);
  
  EXTI_InitStructure.EXTI_Line = EXTI_Line13;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
  EXTI_InitStructure.EXTI_Line = EXTI_Line6;
  EXTI_Init(&EXTI_InitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure); 
	
  NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure); 

}



void TENCODERS::roll_init ()
{
	
EXTI_InitTypeDef   EXTI_InitStructure;
GPIO_InitTypeDef   GPIO_InitStructure;
NVIC_InitTypeDef   NVIC_InitStructure;
	
  RCC_AHBPeriphClockCmd (RCC_AHBPeriph_GPIOB, ENABLE);
  RCC_APB2PeriphClockCmd (RCC_APB2Periph_SYSCFG, ENABLE);
  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource2);
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource7);
  
  EXTI_InitStructure.EXTI_Line = EXTI_Line2;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
  EXTI_InitStructure.EXTI_Line = EXTI_Line7;
  EXTI_Init(&EXTI_InitStructure);
  
	
  NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure); 
	
  NVIC_InitStructure.NVIC_IRQChannel = EXTI2_TS_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure); 
	
}



void TENCODERS::yaw_init ()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  TIM_ICInitTypeDef  TIM_ICInitStructure;

  RCC_APB2PeriphClockCmd (RCC_APB2Periph_TIM15, ENABLE);
  RCC_AHBPeriphClockCmd (RCC_AHBPeriph_GPIOB, ENABLE);
  
	// pb15
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;// GPIO_OType_OD;//GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP ;
  GPIO_Init (GPIOB, &GPIO_InitStructure);

  GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_1);

  /* Enable the TIM15 global Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM1_BRK_TIM15_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;//0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;//0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init (&NVIC_InitStructure);
  /* --------------------------------------------------------------------------- 
    TIM configuration: PWM Input mode
    In this example TIM input clock (TIMCLK) is set to 2*APB1 clock (PCLK1)   
      TIMCLK = 2*PCLK1 
      => TIMCLK = HCLK = SystemCoreClock
    External Signal Frequency = TIM counter clock / TIM_CCR2 in Hz. 
    External Signal DutyCycle = (TIM_CCR1*100)/(TIM_CCR2) in %.
  --------------------------------------------------------------------------- */
  
  TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
  TIM_ICInitStructure.TIM_ICFilter = 0x0;

  TIM_PWMIConfig (TIM_RCIN_YAW, &TIM_ICInitStructure);
  TIM_SelectInputTrigger (TIM_RCIN_YAW, TIM_TS_TI2FP2);
  TIM_SelectSlaveMode (TIM_RCIN_YAW, TIM_SlaveMode_Reset);
  TIM_SelectMasterSlaveMode (TIM_RCIN_YAW, TIM_MasterSlaveMode_Enable);
  TIM_Cmd (TIM_RCIN_YAW, ENABLE);
  TIM_ITConfig (TIM_RCIN_YAW, TIM_IT_CC2, ENABLE);
}




float TENCODERS::get_encoder (tAxis ax)
{
	float rv = 0;
	if (ax < NUMAXIS) rv = RCResults[ax];
	return rv;
}



void TENCODERS::get_encodes (float *dst)
{
char ix = 0;
while (ix < NUMAXIS) {
	*dst++ = RCResults[ix++];
	}
}



TENCODERS::TENCODERS ()
{
	c_filter_level = 0;
	smoothfiltdata[X_AXIS] = 0; smoothfiltdata[Y_AXIS] = 0; smoothfiltdata[Z_AXIS] = 0;
	ReleaseTimer = 0;
	SYSBIOS::ADD_TIMER_SYS (&ReleaseTimer);	
	pitch_init ();
	roll_init ();
	yaw_init ();
}



TENCODERS *TENCODERS::obj_create ()
{
	if (!singlobj) singlobj = new TENCODERS ();
	return singlobj;
}




IISRENCODER *TENCODERS::obj_isr ()
{
	return obj_create ();
}



IUSRENCODER *TENCODERS::obj_user ()
{
	return obj_create ();
}



void TENCODERS::Task ()
{

}



void TENCODERS::init ()
{
}



void TENCODERS::filter_level (float val)
{
	c_filter_level = val;
}



void TENCODERS::get_angles (float *dst)
{
	float mult = (1.0F / 65535) * 359.999F;
	float dat, delt;
	char ix = 0;
	while (ix < ENDENUM_AXIS)
		{
		dat = mult * hard_arr_vals[ix];		// raw to angle 0-360
		delt = smoothfiltdata[ix] - dat;	// delt
		if (delt < 0) delt *= -1.0F;
		if (delt < c_filter_level) 
			{
			dat = (dat + smoothfiltdata[ix]) / 2;	// diff range between cur and prev value is low - goto smooth procedure
			}
		smoothfiltdata[ix] = dat;
		*dst++ = dat;
		ix++;
		}

}



