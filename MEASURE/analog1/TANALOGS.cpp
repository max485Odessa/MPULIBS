#include "TANALOGS.h"



uint16_t TASIGNALS::ADC_DualConvertedValueTab[C_ADCPIN_AMOUNT];
float TASIGNALS::VCC_value = 0;
float TASIGNALS::cur_quant_vref = 0;

TASIGNALS::TASIGNALS (float vint)
{
	c_vrefint = vint;
	Init ();
	ClearDMABarray ();
}



void TASIGNALS::ClearDMABarray ()
{
	memset (&ADC_DualConvertedValueTab, 0, sizeof(ADC_DualConvertedValueTab));
}



void TASIGNALS::Deinit ()
{
  DMA_DeInit(DMA1_Channel1);
	ADC_DeInit(ADC1);
	ADC_DeInit(ADC2);
}



void TASIGNALS::Init ()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef           ADC_InitStructure;
	DMA_InitTypeDef           DMA_InitStructure;
  RCC_ADCCLKConfig (RCC_PCLK2_Div4);
	
  RCC_AHBPeriphClockCmd (RCC_AHBPeriph_DMA1, ENABLE);
  RCC_APB2PeriphClockCmd (RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_5 | GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	Deinit ();
	
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC1_DR_Address;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ADC_DualConvertedValueTab;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = C_ADCPIN_AMOUNT / 2;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;//DMA_PeripheralDataSize_HalfWord;//DMA_PeripheralDataSize_Word;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;// DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);

  DMA_Cmd(DMA1_Channel1, ENABLE);	

  ADC_InitStructure.ADC_Mode = ADC_Mode_RegSimult;
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfChannel = C_ADCPIN_AMOUNT / 2;
  ADC_Init(ADC1, &ADC_InitStructure);

  ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_239Cycles5);    // EASIG_VBUS_1
  ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 2, ADC_SampleTime_239Cycles5);		// EASIG_V12
	ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 3, ADC_SampleTime_239Cycles5);		// EASIG_VBAT_1
	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 4, ADC_SampleTime_239Cycles5);		// EASIG_VBAT_4
	ADC_RegularChannelConfig(ADC1, ADC_Channel_17, 5, ADC_SampleTime_239Cycles5);		// VREFINT 1.2V 
	ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 6, ADC_SampleTime_239Cycles5);		// TEMP
  /* Enable ADC1 DMA */
  ADC_DMACmd(ADC1, ENABLE);
  /* ADC2 configuration ------------------------------------------------------*/
  ADC_InitStructure.ADC_Mode = ADC_Mode_RegSimult;
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfChannel = C_ADCPIN_AMOUNT / 2;
  ADC_Init(ADC2, &ADC_InitStructure);
  /* ADC2 regular channels configuration */ 
  ADC_RegularChannelConfig(ADC2, ADC_Channel_2, 1, ADC_SampleTime_239Cycles5);		// EASIG_VBUS_2
  ADC_RegularChannelConfig(ADC2, ADC_Channel_6, 2, ADC_SampleTime_239Cycles5);		// EASIG_VSYS
	ADC_RegularChannelConfig(ADC2, ADC_Channel_15, 3, ADC_SampleTime_239Cycles5);		// EASIG_VBAT_3
	ADC_RegularChannelConfig(ADC2, ADC_Channel_9, 4, ADC_SampleTime_239Cycles5);		// EASIG_VBAT_2
	ADC_RegularChannelConfig(ADC2, ADC_Channel_1, 5, ADC_SampleTime_239Cycles5);		// ADC_Channel_1 - none1
	ADC_RegularChannelConfig(ADC2, ADC_Channel_1, 6, ADC_SampleTime_239Cycles5);		// ADC_Channel_1 - none2
  /* Enable ADC2 external trigger conversion */
  ADC_ExternalTrigConvCmd(ADC2, ENABLE);
  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);
  ADC_TempSensorVrefintCmd(ENABLE);
  /* Enable ADC1 reset calibration register */   
  ADC_ResetCalibration(ADC1);
  /* Check the end of ADC1 reset calibration register */
  while(ADC_GetResetCalibrationStatus(ADC1));
  /* Start ADC1 calibration */
  ADC_StartCalibration(ADC1);
  /* Check the end of ADC1 calibration */
  while(ADC_GetCalibrationStatus(ADC1));
  /* Enable ADC2 */
  ADC_Cmd(ADC2, ENABLE);
  /* Enable ADC2 reset calibration register */   
  ADC_ResetCalibration(ADC2);
  /* Check the end of ADC2 reset calibration register */
  while(ADC_GetResetCalibrationStatus(ADC2));
  /* Start ADC2 calibration */
  ADC_StartCalibration(ADC2);
  /* Check the end of ADC2 calibration */
  while(ADC_GetCalibrationStatus(ADC2));
  /* Start ADC1 Software Conversion */ 
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);
  /* Test on DMA1 channel1 transfer complete flag */
  while(!DMA_GetFlagStatus(DMA1_FLAG_TC1));
  /* Clear DMA1 channel1 transfer complete flag */
  DMA_ClearFlag(DMA1_FLAG_TC1);
	
	//zof_cur.Init ();		// загружаем калибровки температурного дрейфа токового датчика
}





float TASIGNALS::calculateQuant (unsigned short raw)
{
cur_quant_vref = c_vrefint / raw;
return cur_quant_vref;
}



float TASIGNALS::GetQuant ()
{
return cur_quant_vref;
}
	


float TASIGNALS::rawToVoltages (unsigned long raw)
{
float rv = cur_bitquant;
return rv * raw;
}


//static uint16_t ADC_TAB[C_ADCPIN_AMOUNT];


static const float multarr[EASIG_ENDENUM] = {7.6666F, 7.6666F, 7.6666F, 7.6666F, 7.6666F, 7.6666F, 7.6666F, 7.6666F, 1, 1, 1, 1};

void TASIGNALS::calculateVoltages ()
{
unsigned char indx = 0;
cur_bitquant = calculateQuant (ADC_DualConvertedValueTab[EASIG_VREF]);
while (indx < C_ADCPIN_AMOUNT)
	{
	voltage[indx] = multarr[indx] * rawToVoltages (ADC_DualConvertedValueTab[indx]);
	indx++;
	}
VCC_value = rawToVoltages (4095);		// узнаем напряжение питания
}



float TASIGNALS::GetVCC ()
{
return VCC_value;
}



float TASIGNALS::GetVoltage (EASIG ix)
{
float rv = 0;
if (ix < EASIG_ENDENUM) rv = voltage[ix];
return rv;
}



unsigned short TASIGNALS::GetRaw (EASIG ix)
{
unsigned short rv = 0;//
switch (ix)
	{
	default:
		{
		rv = ADC_DualConvertedValueTab[ix];
		break;
		}
	}
return rv;
}




void TASIGNALS::ISR_1ms_Task ()
{

}



void TASIGNALS::Task ()
{
static unsigned long last_ticknt = 0;
unsigned long cur_ticknt = SYSBIOS::GetTickCountLong();
	if (cur_ticknt != last_ticknt)
		{
		last_ticknt = cur_ticknt;
		calculateVoltages ();
		}	

}


