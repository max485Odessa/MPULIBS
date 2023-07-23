#include "TRCPWM.h"



unsigned short TRCPWM::pwm_min_raw = 0;
unsigned short TRCPWM::pwm_min_raw_matrix = 0;
unsigned short TRCPWM::pwm_max_raw = 65534;
unsigned short TRCPWM::pwm_max_raw_matrix = 65534;
unsigned short TRCPWM::pwm_period_raw = 65535;

unsigned short TRCPWM::pwm_last_raw = 0;
float TRCPWM::pwm_period_sek;
float TRCPWM::pwm_min_sek;
float TRCPWM::pwm_max_sek;
//unsigned short TRCPWM::last_uav_value;
unsigned char TRCPWM::cur_rc_indx = 1;	
unsigned char TRCPWM::src_esc_command = 0;	
bool TRCPWM::f_inverse_esc = false;
bool TRCPWM::f_inverse_act = false;
bool TRCPWM::f_enable_normal_out = false;
unsigned short TRCPWM::c_actuator_matrix_pwm_min = C_DEFAULT_ACT_MATRIX_MIN;
unsigned short TRCPWM::c_actuator_matrix_pwm_max = C_DEFAULT_ACT_MATRIX_MAX;

TRCPWM::TRCPWM ()
{
	AddObjectToExecuteManager ();
	pwm_period_raw = 0;
}


void TRCPWM::Task ()
{
}



void TRCPWM::Start ()
{
	f_enable_normal_out = true;
}



void TRCPWM::SetRC_indx (unsigned char ix)
{
	cur_rc_indx = ix;
}



void TRCPWM::SetESC_src_ID (unsigned char iddd)
{
	src_esc_command = iddd;
}



unsigned char TRCPWM::GetESC_src_ID ()
{
	return src_esc_command;
}



unsigned char TRCPWM::GetRC_indx ()
{
	return cur_rc_indx;
}


void TRCPWM::Init ()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	uint16_t PrescalerValue = 0;
	// rc init
  RCC_APB1PeriphClockCmd (RCC_APB1Periph_TIM3, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
	
	//GPIO_PinRemapConfig (GPIO_Remap_SWJ_NoJTRST, ENABLE);
	GPIO_PinRemapConfig (GPIO_PartialRemap_TIM3, ENABLE);		// GPIO_FullRemap_TIM3  GPIO_PartialRemap_TIM3
	
	// pin init
  GPIO_InitStructure.GPIO_Pin = C_PIN_SERV_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init (C_PWM_PORT, &GPIO_InitStructure);
	
  GPIO_InitStructure.GPIO_Pin = C_PIN_SERV_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init (C_PWM_PORT, &GPIO_InitStructure);
	
  PrescalerValue = (uint16_t) (SystemCoreClock / C_INTERNALPWM_CLOCK) - 1;
	
	TIM_DeInit(C_PWM_TIMER);
	
  // Time base configuration
  TIM_TimeBaseStructure.TIM_Period = pwm_period_raw;
  TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(C_PWM_TIMER, &TIM_TimeBaseStructure);

  // PWM1 Mode configuration
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = pwm_last_raw;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OC1Init(C_PWM_TIMER, &TIM_OCInitStructure);
	TIM_OC2Init(C_PWM_TIMER, &TIM_OCInitStructure);
	
  TIM_OC1PreloadConfig (C_PWM_TIMER, TIM_OCPreload_Enable);
	TIM_OC2PreloadConfig (C_PWM_TIMER, TIM_OCPreload_Enable);
  TIM_ARRPreloadConfig (C_PWM_TIMER, ENABLE);
  TIM_Cmd (C_PWM_TIMER, ENABLE);
}



void TRCPWM::SetPeriod_Freq (float val)
{
	if (val < 30) val = 30;		// меньше 15 герц нельзя
	pwm_period_sek = (1.0 / val);//  / C_INTERNALPWM_CLOCK;
	SetPeriod_Ms (pwm_period_sek);
}



// преобразование секундных величин в эквивалентные единицы PWM таймера
void TRCPWM::calculate_min_max_cur_uav_rc_values ()
{
// границы физического диапазона стандарта управления
pwm_max_raw = (pwm_max_sek / C_PWM_PERIOD_QUANT);
pwm_min_raw = (pwm_min_sek / C_PWM_PERIOD_QUANT);
	
// границы логического диапазона (в основном не будут совпадать с физическими)
// результат полученный при их использовании должен быть усечен физическим диапазоном pwm_max_raw-pwm_min_raw
pwm_max_raw_matrix = ((0.000001*c_actuator_matrix_pwm_max) / C_PWM_PERIOD_QUANT);		// расчитываем величины управления
pwm_min_raw_matrix  = ((0.000001*c_actuator_matrix_pwm_min) / C_PWM_PERIOD_QUANT);
}



void TRCPWM::SetMin_Ms (float val)
{
	pwm_min_sek = val;
	calculate_min_max_cur_uav_rc_values ();
}



void TRCPWM::SetMax_Ms (float val)
{
	pwm_max_sek = val;
	calculate_min_max_cur_uav_rc_values ();
}
			


void TRCPWM::SetPeriod_Ms (float val)
{
	if (val > 0.032) val = 0.032;
	pwm_period_sek = val;
	float p_ticks = pwm_period_sek * C_INTERNALPWM_CLOCK;
	pwm_period_raw = p_ticks;
	TIM_TimeBaseStructure.TIM_Period = p_ticks - 1;
	TIM_TimeBaseInit (C_PWM_TIMER, &TIM_TimeBaseStructure);
	calculate_min_max_cur_uav_rc_values ();	
}



void TRCPWM::set_raw_pwm (unsigned short val)
{
	if (!f_enable_normal_out) val = 0;
	pwm_last_raw = val;
	C_PWM_TIMER->CCR1 = val;
	C_PWM_TIMER->CCR2 = val;
}




void TRCPWM::ActMatrixPwm_min (unsigned short val)
{
	c_actuator_matrix_pwm_min = val;
	calculate_min_max_cur_uav_rc_values ();
}



void TRCPWM::ActMatrixPwm_max (unsigned short val)
{
	c_actuator_matrix_pwm_max = val;
	calculate_min_max_cur_uav_rc_values ();
}





/*
void TRCPWM::SetUAV_PWM_esc (unsigned short val)
{
	
	float quant_diff = pwm_max_raw - pwm_min_raw, val_out;		// val_out = pwm_min_raw;
	quant_diff /= 1100;		// c_margin_pwm_max-c_margin_pwm_min
	val_out = (quant_diff * (val - 900));
	
	set_raw_pwm (val_out + pwm_min_raw);
}
*/


/*
void TRCPWM::SetUAV_PWM_actuator (float val)
{
	if (val < 0.1 && val > -0.1) return;
	if (val > 1.0) val = 1.0;
	if (val < -1.0) val = -1.0;
	
	float quant_diff = (pwm_max_raw - pwm_min_raw) / 2, val_out;
	long midl = pwm_min_raw + quant_diff;
	val_out = (quant_diff * val);
	set_raw_pwm (midl + val_out);
	last_uav_value = val;
}
*/



void TRCPWM::SetUAV_PWM_esc (short val)
{
	// эта команда может принимать полярные величины, тоесть возможно в Кубе она может опять-таки, транслироваться в логическую величину
	// эти команды с Куба не приходили, поэтому предположительно ожидаются величины в диапазоне 1000-2000, тоесть однополярные.
	// Чтоб не дублировать код используем функцию SetUAV_PWM_actuator, которая учитывает матрицу преобразования
	// преобразуем физический шаг до коррекции, в логическую величину (1: -1) с которой работает SetUAV_PWM_actuator
	if (val > 8191) val = 8191;
	if (val < -8191) val = -8191;
	
	unsigned long microsek_min = ((float)pwm_min_sek / 0.000001);		// переводим в единицы микросекунд
	unsigned long microsek_max = ((float)pwm_max_sek / 0.000001);		// переводим в единицы микросекунд

	if (val > microsek_max) val = microsek_max;
	if (val < microsek_min) val = microsek_min;
	unsigned short phisical_diap = microsek_max - microsek_min;		
	unsigned short polus_diap = phisical_diap / 2;
	unsigned short midle_point = microsek_min + polus_diap;
	float quant = 1.0 / polus_diap;		// логический шаг на половину физического диапазона
	float logval = val;
	logval -= midle_point;		// знак получен
	logval *= quant;					// переводим в логическую величину (-1: 1)
	
	SetUAV_PWM_actuator (logval);
}




void TRCPWM::SetUAV_PWM_actuator (float val)
{
	if (val < 0.1 && val > -0.1) return;
	if (val > 1.0) val = 1.0;
	if (val < -1.0) val = -1.0;
	
	// матрица преобразования
	float quant_diff = (pwm_max_raw_matrix - pwm_min_raw_matrix) / 2, val_out;
	long midl = pwm_min_raw_matrix + quant_diff;
	val_out = (quant_diff * val);
	
	unsigned short val_pwm = ((long)midl + val_out);
	// контроль рабочего диапазона
	if (val_pwm > pwm_max_raw) val_pwm = pwm_max_raw;
	if (val_pwm < pwm_min_raw) val_pwm = pwm_min_raw;
	
	set_raw_pwm (val_pwm);
	//last_uav_value = val;
}





void TRCPWM::SetMode (unsigned char val)
{
	cur_mode = val;
}


