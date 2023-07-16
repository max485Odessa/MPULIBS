#include "TANALOGS.h"



uint16_t TASIGNALS::ADC_DMA_arr[EASIG_ENDENUM];


TASIGNALS::TASIGNALS (uint16_t smcl) : c_smooth_cicle_n (smcl)
{
	c_vref_voltage = 1.2F;
	ClearDMABarray ();
}



void TASIGNALS::ClearDMABarray ()
{
	memset (&ADC_DMA_arr, 0, sizeof(ADC_DMA_arr));
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
  RCC_APB2PeriphClockCmd (RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2 | RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	Deinit ();
	
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC1_DR_Address;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ADC_DMA_arr;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = EASIG_ENDENUM / 2;
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
  ADC_InitStructure.ADC_NbrOfChannel = EASIG_ENDENUM / 2;
  ADC_Init(ADC1, &ADC_InitStructure);

  ADC_RegularChannelConfig(ADC1, ADC_Channel_17, 1, ADC_SampleTime_239Cycles5);    // vrefint
  ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 2, ADC_SampleTime_239Cycles5);			// vddhi
	ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 3, ADC_SampleTime_239Cycles5);			// v12sw

  /* Enable ADC1 DMA */
  ADC_DMACmd(ADC1, ENABLE);
  /* ADC2 configuration ------------------------------------------------------*/
  ADC_InitStructure.ADC_Mode = ADC_Mode_RegSimult;
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfChannel = EASIG_ENDENUM / 2;
  ADC_Init(ADC2, &ADC_InitStructure);
  /* ADC2 regular channels configuration */ 
  ADC_RegularChannelConfig(ADC2, ADC_Channel_1, 1, ADC_SampleTime_239Cycles5);		// vbat
  ADC_RegularChannelConfig(ADC2, ADC_Channel_3, 2, ADC_SampleTime_239Cycles5);		// ntc
	ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 3, ADC_SampleTime_239Cycles5);		// vset
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
}




float TASIGNALS::rawToVoltages (unsigned long raw)
{
float rv = bitquant_val;
return rv * raw;
}



void TASIGNALS::calculateVoltages ()
{
unsigned char indx = 0;
bitquant_val = c_vref_voltage / ADC_DMA_arr[EASIG_VREFINT];

while (indx < EASIG_ENDENUM)
	{
	voltage[indx] = rawToVoltages (ADC_DMA_arr[indx]);
	smooth_acc_arr[indx] += voltage[indx];
	indx++;
	}
smooth_cicle_n++;
if (smooth_cicle_n >= c_smooth_cicle_n)
	{
	indx = 0;
	while (indx < EASIG_ENDENUM)
		{
		smooth_voltage[indx] = smooth_acc_arr[indx] / c_smooth_cicle_n;
		smooth_acc_arr[indx] = 0;
		indx++;
		}
	smooth_cicle_n = 0;
	}
VCC_value = rawToVoltages (4095);		// узнаем напряжение питания
}



float TASIGNALS::GetVCC ()
{
return VCC_value;
}



float TASIGNALS::GetValue (EASIG ix)
{
float rv = 0;
if (ix < EASIG_ENDENUM) rv = voltage[ix];
return rv;
}



float *TASIGNALS::GetVoltCell (EASIG ix)
{
	float *rv = 0;
	if (ix < EASIG_ENDENUM) rv = &voltage[ix];
	return rv;
}
	



uint16_t TASIGNALS::GetRaw (EASIG ix)
{
unsigned short rv = 0;
if (ix < EASIG_ENDENUM) rv = ADC_DMA_arr[ix];
return rv;
}



float TASIGNALS::GetSmoothValue (EASIG ix)
{
float rv = 0;
if (ix < EASIG_ENDENUM) rv = smooth_voltage[ix];
return rv;
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


