#include "TANALOGS.h"



uint16_t TASIGNALS::ADC_DualConvertedValueTab[C_ADCPIN_AMOUNT];
float TASIGNALS::VCC_value = 0;
float TASIGNALS::cur_quant_vref = 0;

TASIGNALS::TASIGNALS () : c_ext_voltage (1.2)
{
	ClearDMABarray ();
	AddObjectToExecuteManager ();
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



void TASIGNALS::Init (bool f_d_mode)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef           ADC_InitStructure;
	DMA_InitTypeDef           DMA_InitStructure;
  RCC_ADCCLKConfig (RCC_PCLK2_Div4);
	f_dma_mode = f_d_mode;
  
  RCC_APB2PeriphClockCmd (RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2 | RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	Deinit ();
	
	if (f_dma_mode)
		{
		RCC_AHBPeriphClockCmd (RCC_AHBPeriph_DMA1, ENABLE);
		
		DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC1_DR_Address;
		DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ADC_DualConvertedValueTab;
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
		DMA_InitStructure.DMA_BufferSize = C_ADCPIN_AMOUNT / 2;
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;//DMA_PeripheralDataSize_HalfWord;//DMA_PeripheralDataSize_Word;
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
		DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;// DMA_Mode_Normal;
		DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
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

		ADC_RegularChannelConfig(ADC1, ADC_Channel_17, 1, ADC_SampleTime_239Cycles5);    

		ADC_DMACmd(ADC1, ENABLE);

		ADC_InitStructure.ADC_Mode = ADC_Mode_RegSimult;
		ADC_InitStructure.ADC_ScanConvMode = ENABLE;
		ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
		ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
		ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
		ADC_InitStructure.ADC_NbrOfChannel = C_ADCPIN_AMOUNT / 2;
		ADC_Init(ADC2, &ADC_InitStructure);

		ADC_RegularChannelConfig(ADC2, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5);

		ADC_ExternalTrigConvCmd(ADC2, ENABLE);

		ADC_Cmd(ADC1, ENABLE);
		ADC_TempSensorVrefintCmd(ENABLE);
		 
		ADC_ResetCalibration(ADC1);

		while(ADC_GetResetCalibrationStatus(ADC1));

		ADC_StartCalibration(ADC1);

		while(ADC_GetCalibrationStatus(ADC1));

		ADC_Cmd(ADC2, ENABLE);
		
		ADC_ResetCalibration(ADC2);

		while(ADC_GetResetCalibrationStatus(ADC2));

		ADC_StartCalibration(ADC2);

		while(ADC_GetCalibrationStatus(ADC2));

		ADC_SoftwareStartConvCmd(ADC1, ENABLE);

		while(!DMA_GetFlagStatus(DMA1_FLAG_TC1));

		DMA_ClearFlag(DMA1_FLAG_TC1);
		ADC_TempSensorVrefintCmd(ENABLE);
		}
	else
		{
		ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
		ADC_InitStructure.ADC_ScanConvMode = DISABLE;
		ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
		ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
		ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
		ADC_InitStructure.ADC_NbrOfChannel = 1;
		ADC_Init (ADC1, &ADC_InitStructure);
			
		soft_ix = 0;
		set_channel_mux (soft_ix);
		//ADC_RegularChannelConfig (ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5);

		ADC_Cmd(ADC1, ENABLE);
		ADC_TempSensorVrefintCmd(ENABLE);
		ADC_ResetCalibration(ADC1);
		while(ADC_GetResetCalibrationStatus(ADC1));
		ADC_StartCalibration(ADC1);
		while(ADC_GetCalibrationStatus(ADC1));
		ADC_SoftwareStartConvCmd(ADC1, ENABLE);
		}

}



uint16_t TASIGNALS::GetAdcChanelFrom_ix (uint16_t ix)
{
uint16_t rv = 0;
switch (ix)
	{
	case EASIG_VREF:
		{
		rv = ADC_Channel_17;
		break;
		}
	case EASIG_TROTLE:
		{
		rv = ADC_Channel_0;
		break;
		}
	}
return rv;
}


void TASIGNALS::set_channel_mux (uint16_t ix)
{
	ADC_RegularChannelConfig (ADC1, GetAdcChanelFrom_ix (ix), 1, ADC_SampleTime_239Cycles5);		// ADC_Channel_0
}



float TASIGNALS::calculateQuant (unsigned short raw)
{
cur_quant_vref = c_ext_voltage / raw;
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



void TASIGNALS::calculateVoltages ()
{
unsigned char indx = 0;
cur_bitquant = calculateQuant (ADC_DualConvertedValueTab[EASIG_VREF]);
while (indx < C_ADCPIN_AMOUNT)
	{
	voltage[indx] = rawToVoltages (ADC_DualConvertedValueTab[indx]);
	indx++;
	}
VCC_value = rawToVoltages (4095);		// узнаем напряжение питания
//BAT_OFFSET_ampere = (VCC_value / 2);		// корректируем среднюю точку по температурному дрейфу для тока
}



float TASIGNALS::GetTrotle ()
{
return voltage[EASIG_TROTLE] * C_RESISTORS_TROTL_MULT;	
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
return ADC_DualConvertedValueTab[ix];
}



void TASIGNALS::Task ()
{
static unsigned long last_ticknt = 0;
unsigned long cur_ticknt = SYSBIOS::GetTickCountLong();
	
if (!f_dma_mode)	
	{
	if (ADC_GetFlagStatus (ADC1, ADC_FLAG_EOC) == SET)
		{
		ADC_DualConvertedValueTab[soft_ix] = ADC_GetConversionValue (ADC1);
		soft_ix++;
		if (soft_ix >= EASIG_ENDENUM) soft_ix = 0;
		set_channel_mux (soft_ix);
		ADC_SoftwareStartConvCmd (ADC1, ENABLE);
		// soft_ix
		}
	}
	
	if (cur_ticknt != last_ticknt)
		{
		last_ticknt = cur_ticknt;
		calculateVoltages ();
		}	
}


