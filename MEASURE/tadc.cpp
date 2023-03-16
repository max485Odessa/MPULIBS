#include "tadc.h"

//ADC_HandleTypeDef TAINCONTROL::AdcHandle;
//ADC_ChannelConfTypeDef TAINCONTROL::sConfig;
//EADCCH TAINCONTROL::lastchenum;

/*
#ifdef __cplusplus
 extern "C" {
#endif 



void ADC1_IRQHandler(void)
{
  HAL_ADC_IRQHandler(&TAINCONTROL::AdcHandle);
}

#ifdef __cplusplus
}
#endif
*/



TAINCONTROL::TAINCONTROL () : vref_internal (1.212)
{
	AddObjectToExecuteManager ();
	meas_timer = 3000;
	SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&meas_timer);
	curval_quantum = 0;
	
  sConfig.Channel      = ADC_CHANNEL_16;                /* Sampled channel number */
  sConfig.Rank         = ADC_REGULAR_RANK_1;          /* Rank of sampled channel number ADCx_CHANNEL */
  sConfig.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;    /* Sampling time (number of clock cycles unit) */
  sConfig.SingleDiff   = ADC_SINGLE_ENDED;            /* Single-ended input channel */
  sConfig.OffsetNumber = ADC_OFFSET_NONE;             /* No offset subtraction */ 
  sConfig.Offset = 0;                                 /* Parameter discarded because offset correction is disabled */
	
  //HAL_NVIC_SetPriority (ADC1_IRQn, 0, 0);
  //HAL_NVIC_EnableIRQ (ADC1_IRQn);
	
	f_actual_measure = false;
}



void TAINCONTROL::set_chan_mux (EADCCH emux)
{
	if (emux < EADCCH_ENDENUM)
		{
		switch ((char)emux)
			{
			case EADCCH_5V:
				{
				sConfig.Channel = ADC_CHANNEL_16;  
				break;
				}
			case EADCCH_VREF_EXT:
				{
				sConfig.Channel = ADC_CHANNEL_VREFINT;//ADC_CHANNEL_0;  
				break;
				}
			}
		}
	HAL_ADC_ConfigChannel (&AdcHandle, &sConfig);
	//lastchenum = emux;
}



void TAINCONTROL::Init (float cvref)
{
	vref_internal = cvref;
	
  GPIO_InitTypeDef GPIO_InitStruct;

	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_ADC_CLK_ENABLE ();

	GPIO_InitStruct.Pin = GPIO_PIN_1;		
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init (GPIOB, &GPIO_InitStruct);

  __HAL_RCC_ADC_CONFIG(RCC_ADCCLKSOURCE_SYSCLK);
	//__HAL_RCC_ADC_CONFIG(RCC_ADCCLKSOURCE_SYSCLK);
	
  AdcHandle.Instance          = ADC1;
  HAL_ADC_DeInit (&AdcHandle);

  AdcHandle.Init.ClockPrescaler        = ADC_CLOCK_ASYNC_DIV1;          /* Asynchronous clock mode, input ADC clock not divided */
  AdcHandle.Init.Resolution            = ADC_RESOLUTION_12B;            /* 12-bit resolution for converted data */
  AdcHandle.Init.DataAlign             = ADC_DATAALIGN_RIGHT;           /* Right-alignment for converted data */
  AdcHandle.Init.ScanConvMode          = DISABLE;                       /* Sequencer disabled (ADC conversion on only 1 channel: channel set on rank 1) */
  AdcHandle.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;           /* EOC flag picked-up to indicate conversion end */
  AdcHandle.Init.LowPowerAutoWait      = DISABLE;                       /* Auto-delayed conversion feature disabled */
  AdcHandle.Init.ContinuousConvMode    = DISABLE;                       /* Continuous mode disabled to have only 1 conversion at each conversion trig */
  AdcHandle.Init.NbrOfConversion       = 1;                             /* Parameter discarded because sequencer is disabled */
  AdcHandle.Init.DiscontinuousConvMode = DISABLE;                       /* Parameter discarded because sequencer is disabled */
  AdcHandle.Init.NbrOfDiscConversion   = 1;                             /* Parameter discarded because sequencer is disabled */
  AdcHandle.Init.ExternalTrigConv      = ADC_SOFTWARE_START;            /* Software start to trig the 1st conversion manually, without external event */
  AdcHandle.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE; /* Parameter discarded because software trigger chosen */
  AdcHandle.Init.DMAContinuousRequests = DISABLE;                       /* DMA one-shot mode selected (not applied to this example) */
  AdcHandle.Init.Overrun               = ADC_OVR_DATA_OVERWRITTEN;      /* DR register is overwritten with the last conversion result in case of overrun */
  AdcHandle.Init.OversamplingMode      = DISABLE;                       /* No oversampling */

  HAL_ADC_Init(&AdcHandle);

	lastchenum = EADCCH_VREF_EXT;
	set_chan_mux (lastchenum);
	
	HAL_ADCEx_Calibration_Start(&AdcHandle, ADC_SINGLE_ENDED);

	HAL_ADC_Start (&AdcHandle);
  
}
	


float TAINCONTROL::calc_quantum ()
{
	return vref_internal / (aADCxConvertedData[EADCCH_VREF_EXT] & 0x0FFF);
}



float TAINCONTROL::get_vcc ()
{
	return curval_vcc;
}



float TAINCONTROL::get_5volt ()
{
	return curval_5v;
}



bool TAINCONTROL::is_actual_vals ()
{
	return f_actual_measure;
}



void TAINCONTROL::Task ()
{
if (HAL_ADC_PollForConversion (&AdcHandle, 0) == HAL_OK)
	{
	bool f_circl = false;
	uint32_t val = HAL_ADC_GetValue(&AdcHandle);
	aADCxConvertedData[lastchenum] = val & 0x0FFF;
	uint32_t cenum = lastchenum;
	cenum++;
	if (cenum >= EADCCH_ENDENUM) 
		{
		f_circl = true;
		
		cenum = EADCCH_5V;
		}
	lastchenum = (EADCCH)cenum;
		
	set_chan_mux (lastchenum);
	HAL_ADC_Start (&AdcHandle);
	if (!meas_timer)
		{
		curval_quantum = calc_quantum ();			// вычисляем квантование по внутренему vref 1.212
		curval_vcc = curval_quantum * 4096;		// вычисляем vcc, vcc = ext_vref.
			
		float vlt = curval_quantum * (aADCxConvertedData[EADCCH_5V] & 0x0FFF);
		vlt *= C_5_VOLT_DIVIDER_COEF;
		float delt = vlt - curval_5v;
		if (delt > 0.5)
			{
			curval_5v = vlt; 
			}
		else
			{
			curval_5v = (curval_5v + vlt ) / 2;
			}
		
		f_actual_measure = true;
		meas_timer = 200;
		}	
	}

}
		
		
		