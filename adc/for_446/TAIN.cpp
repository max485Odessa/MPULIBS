#include "TAIN.h"



static const uint32_t adc_chan[EAINCH_ENDENUM] = {ADC_CHANNEL_10, ADC_CHANNEL_11, ADC_CHANNEL_12, ADC_CHANNEL_1, ADC_CHANNEL_2, ADC_CHANNEL_17, ADC_CHANNEL_18, ADC_CHANNEL_17};
static const S_GPIOPIN adcpinsarr[EAINPIN_ENDENUM] = {{GPIOC, GPIO_PIN_0}, {GPIOC, GPIO_PIN_1}, {GPIOC, GPIO_PIN_2}, {GPIOA, GPIO_PIN_1}, {GPIOA, GPIO_PIN_2}};
static ADC_HandleTypeDef AdcHandle;
static __IO uint16_t uhADCxConvertedValue[EAINCH_ENDENUM];


TAIN::TAIN ()
{
	_pin_low_init_adc (const_cast<S_GPIOPIN*>(adcpinsarr), EAINPIN_ENDENUM);
	ADC_ChannelConfTypeDef sConfig;
  GPIO_InitTypeDef          GPIO_InitStruct;
  static DMA_HandleTypeDef  hdma_adc;

	__HAL_RCC_ADC1_CLK_ENABLE ();
	__HAL_RCC_DMA2_CLK_ENABLE();
	
  hdma_adc.Instance = DMA2_Stream0;
  hdma_adc.Init.Channel  = DMA_CHANNEL_0;
  hdma_adc.Init.Direction = DMA_PERIPH_TO_MEMORY;
  hdma_adc.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_adc.Init.MemInc = DMA_MINC_ENABLE;
  hdma_adc.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  hdma_adc.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
  hdma_adc.Init.Mode = DMA_CIRCULAR;
  hdma_adc.Init.Priority = DMA_PRIORITY_HIGH;
  hdma_adc.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
  hdma_adc.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
  hdma_adc.Init.MemBurst = DMA_MBURST_SINGLE;
  hdma_adc.Init.PeriphBurst = DMA_PBURST_SINGLE;

  HAL_DMA_Init(&hdma_adc);

  __HAL_LINKDMA (&AdcHandle, DMA_Handle, hdma_adc);
	
  AdcHandle.Instance                   = ADC1;
  AdcHandle.Init.ClockPrescaler        = ADC_CLOCKPRESCALER_PCLK_DIV4;
  AdcHandle.Init.Resolution            = ADC_RESOLUTION_12B;
  AdcHandle.Init.ScanConvMode          = DISABLE;                       /* Sequencer disabled (ADC conversion on only 1 channel: channel set on rank 1) */
  AdcHandle.Init.ContinuousConvMode    = ENABLE;                        /* Continuous mode disabled to have only 1 conversion at each conversion trig */
  AdcHandle.Init.DiscontinuousConvMode = DISABLE;                       /* Parameter discarded because sequencer is disabled */
  AdcHandle.Init.NbrOfDiscConversion   = 0;
  AdcHandle.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;        /* Conversion start trigged at each external event */
  AdcHandle.Init.ExternalTrigConv      = ADC_EXTERNALTRIGCONV_T1_CC1;
  AdcHandle.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
  AdcHandle.Init.NbrOfConversion       = EAINCH_ENDENUM;
  AdcHandle.Init.DMAContinuousRequests = ENABLE;
  AdcHandle.Init.EOCSelection          = DISABLE;
	
	HAL_ADC_Init (&AdcHandle);
	
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  sConfig.Offset       = 0;
	
	uint8_t ix = 0;
	while (ix < EAINCH_ENDENUM)
		{
		sConfig.Channel      = adc_chan[ix];
		sConfig.Rank         = ix+1;
		HAL_ADC_ConfigChannel (&AdcHandle, &sConfig);	
		ix++;
		}
	HAL_ADC_Start_DMA(&AdcHandle, (uint32_t*)&uhADCxConvertedValue, EAINCH_ENDENUM);
}



float *TAIN::adr_voltage (EAINPIN ch)
{
	float *rv = 0;
	if (ch < EAINPIN_ENDENUM) rv = &voltage[ch];
	return rv;
}




void TAIN::Task ()
{
}


