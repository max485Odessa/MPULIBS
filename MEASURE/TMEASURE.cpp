#include "TMEASURE.h"



TMEAS *TMEAS::meas_obj = 0;


TMEAS::TMEAS ()
{
	f_new_cicle = false;
	meas_timeout = 0;
	SYSBIOS::ADD_TIMER_SYS (&meas_timeout);
	meas_block_time = 0;
	SYSBIOS::ADD_TIMER_SYS (&meas_block_time);
	init_adcs ();
}



TMEAS *TMEAS::get_obj ()
{
	if (!meas_obj) meas_obj = new TMEAS ();
	return meas_obj;
}




float *TMEAS::get_vcc_adr ()
{
	return &vcc_voltage;
}



float *TMEAS::get_adr (EAIN ix)
{
	float *rv = &vcc_voltage;
	if (ix < EAIN_ENDENUM) rv = &voltages[ix];
	return rv;
}


// останавливает текущие преобразования и блокирует на время автоматические преобразования
void TMEAS::meas_blocked (uint32_t tmm)
{
	meas_block_time = tmm;
	if (meas_block_time)
		{
		ADC_TypeDef *adcc;
		uint8_t ix = EAIN_CUR_ROLL;
		while (ix < EAIN_ENDENUM)
			{
			adcc = get_adc_ic ((EAIN)ix);
			ADC_StopConversion (adcc);
			ADC_ClearFlag (adcc, ADC_FLAG_EOC);
			ix++;
			}
		}
}



float TMEAS::get_voltage_sync (EAIN ix)
{
	float rv;
	ADC_TypeDef *adcc = get_adc_ic (ix);
	ADC_RegularChannelConfig (adcc, get_mux (ix), 1, get_cicles (ix));
	ADC_StartConversion (adcc);
	while (ADC_GetFlagStatus (adcc, ADC_FLAG_EOC) != SET) {};
	rv = ion_quant * ADC_GetConversionValue (adcc);
	return rv;
}



void TMEAS::Task ()
{
	if (meas_block_time)
		{
		if (meas_timeout)
			{
			ADC_TypeDef *adcc = get_adc_ic (meas_sw);
			if (ADC_GetFlagStatus (adcc, ADC_FLAG_EOC) == SET) 
				{
				ADC_ClearFlag (adcc, ADC_FLAG_EOC);
				meas_timeout = 0;
				}
			}
		}
	else
		{
		if (meas_timeout)
			{
			ADC_TypeDef *adcc = get_adc_ic (meas_sw);
			if (ADC_GetFlagStatus (adcc, ADC_FLAG_EOC) == SET)
				{
				meas_timeout = 0;
				rawadc[meas_sw] = ADC_GetConversionValue (adcc);
				sw_next ();
				if (meas_sw == EAIN_VREF) calculate_voltages ();
				start_measure ();		
				}
			}
		else
			{
			start_measure ();
			}
		}
}



void TMEAS::calculate_voltages ()
{
ion_quant = 1.2 / rawadc[EAIN_VREF];
vcc_voltage = ion_quant * 4095;
char ix = 0;
while (ix < EAIN_ENDENUM)
	{
	voltages[ix] = ion_quant * rawadc[ix];
	ix++;
	}
}



void TMEAS::init_adc (ADC_TypeDef *adc)
{
	__IO uint32_t TimingDelay = 0, calibration_value = 0;
	ADC_InitTypeDef       ADC_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	ADC_StructInit (&ADC_InitStructure);
	ADC_VoltageRegulatorCmd (adc, ENABLE);
	SYSBIOS::Wait (2);
	
  ADC_SelectCalibrationMode (adc, ADC_CalibrationMode_Single);
  ADC_StartCalibration (adc);

  while(ADC_GetCalibrationStatus (adc) != RESET );
  calibration_value = ADC_GetCalibrationValue (adc);
     
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;                                                                    
  ADC_CommonInitStructure.ADC_Clock = ADC_Clock_AsynClkMode;                    
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;             
  ADC_CommonInitStructure.ADC_DMAMode = ADC_DMAMode_OneShot;                  
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = 0;          
  ADC_CommonInit (adc, &ADC_CommonInitStructure);
	
  ADC_InitStructure.ADC_ContinuousConvMode = ADC_ContinuousConvMode_Disable;//ADC_ContinuousConvMode_Enable;
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b; 
  ADC_InitStructure.ADC_ExternalTrigConvEvent = ADC_ExternalTrigConvEvent_0;         
  ADC_InitStructure.ADC_ExternalTrigEventEdge = ADC_ExternalTrigEventEdge_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_OverrunMode = ADC_OverrunMode_Disable;   
  ADC_InitStructure.ADC_AutoInjMode = ADC_AutoInjec_Disable;  
  ADC_InitStructure.ADC_NbrOfRegChannel = 1;
  ADC_Init (adc, &ADC_InitStructure);
  
  ADC_Cmd (adc, ENABLE);
  while (!ADC_GetFlagStatus (adc, ADC_FLAG_RDY));
}



static const S_GPIOPIN arrpins[3] = {{GPIOB, GPIO_Pin_11/*roll*/}, {GPIOB, GPIO_Pin_0/*pitch*/}, {GPIOB, GPIO_Pin_14/*yaw*/}};
static const S_ADCINTMEA_T admuxarr[EAIN_ENDENUM] = {{ADC1, ADC_Channel_18, ADC_SampleTime_601Cycles5/*vref_int*/}, {ADC1, ADC_Channel_16, ADC_SampleTime_601Cycles5/*temp_int*/}, {ADC1, ADC_Channel_14, ADC_SampleTime_601Cycles5/*roll*/}, {ADC3, ADC_Channel_12, ADC_SampleTime_601Cycles5/*pitch*/}, {ADC4, ADC_Channel_4, ADC_SampleTime_601Cycles5/*yaw*/}};
//static const uint8_t ads_muxs[EAIN_ENDENUM] = {ADC_Channel_18, ADC_Channel_16, ADC_Channel_11, ADC_Channel_12, ADC_Channel_4};



void TMEAS::init_adcs ()
{

//GPIO_InitTypeDef    GPIO_InitStructure;
	
	RCC_ADCCLKConfig (RCC_ADC12PLLCLK_Div2);
	RCC_ADCCLKConfig (RCC_ADC34PLLCLK_Div2);
	RCC_AHBPeriphClockCmd (RCC_AHBPeriph_ADC12, ENABLE);
	RCC_AHBPeriphClockCmd (RCC_AHBPeriph_ADC34, ENABLE);
	ADC_TempSensorCmd (admuxarr[EAIN_TEMP].ADCx, ENABLE);
	ADC_VrefintCmd (admuxarr[EAIN_VREF].ADCx, ENABLE);
	
	_pin_low_init_adc ((S_GPIOPIN*)arrpins, 3);
	
	init_adc (ADC1);
	//init_adc (ADC2);
	init_adc (ADC3);
	init_adc (ADC4);
	
}



ADC_TypeDef *TMEAS::get_adc_ic (EAIN ix)
{
	ADC_TypeDef *rv = ADC1;
	if (ix < EAIN_ENDENUM) rv = admuxarr[ix].ADCx;
	return rv;
}



uint8_t TMEAS::get_mux (EAIN ix)
{
	uint8_t rv = ADC_Channel_18;
	if (ix < EAIN_ENDENUM) rv = admuxarr[ix].ADC_Channel;
	return rv;
}



uint8_t TMEAS::get_cicles (EAIN ix)
{
	uint8_t rv = ADC_SampleTime_601Cycles5;
	if (ix < EAIN_ENDENUM) rv = admuxarr[ix].cicles;
	return rv;
}



void TMEAS::sw_next ()
{
	uint8_t dat = meas_sw;
	dat++;
	if (dat >= EAIN_ENDENUM) dat = EAIN_VREF;
	meas_sw = (EAIN)dat;
}



void TMEAS::start_measure ()
{
	ADC_TypeDef *adcc = get_adc_ic (meas_sw);
	ADC_RegularChannelConfig (adcc, get_mux (meas_sw), 1, get_cicles (meas_sw));
	ADC_StartConversion (adcc);
	meas_timeout = 2;	// 1-2 ms
}



bool TMEAS::check_cicle_f ()
{
	return f_new_cicle;
}



void TMEAS::clear_cicle_f ()
{
	f_new_cicle = false;
}



float TMEAS::get_vcc ()
{
	return vcc_voltage;
}



float TMEAS::get_voltage (EAIN ix)
{
	float rv = 0;
	if (ix < EAIN_ENDENUM) rv = voltages[ix];
	return rv;
}


