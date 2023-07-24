#ifndef _H_UAVCAN_PWM_RC_STM32_H_
#define _H_UAVCAN_PWM_RC_STM32_H_

#include "TFTASKIF.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "memstorage_class.h"

const uint16_t C_PIN_SERV_1 = GPIO_Pin_4;
const uint16_t C_PIN_SERV_2 = GPIO_Pin_5;
#define C_PWM_PORT GPIOB
#define C_PWM_TIMER TIM3
#define C_INTERNALPWM_CLOCK 2000000

const float C_PWM_PERIOD_QUANT = 1.0 / C_INTERNALPWM_CLOCK;

class TRCPWM : public TFFC {
		
	protected:
		virtual void Task ();
		TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
		unsigned char cur_mode;
		static unsigned char src_esc_command;		// адрес прослушиваемого источника
		static unsigned char cur_rc_indx;
		static unsigned short pwm_last_raw;
		// минимальное физическое значение PWM при котором актуатор гарантированно работает (из спецификации стандарта). (выраженная в своих еквивалентных единицах управления таймера)
		static unsigned short pwm_min_raw;
		static unsigned short pwm_min_raw_matrix;
		// максимальное физическое значение PWM при котором актуатор гарантированно работает (из спецификации стандарта). (выраженная в своих еквивалентных единицах управления таймера)
		static unsigned short pwm_max_raw;		
		static unsigned short pwm_max_raw_matrix;	
		// период PWM цикла актуатора (из спецификации стандарта). (выраженная в своих еквивалентных единицах управления таймера)
		static unsigned short pwm_period_raw;	
		// pwm_min_raw и pwm_max_raw используются для контроля рабочих границ значений, при превышении результата вычисления
	
		static bool f_inverse_esc;
		static bool f_inverse_act;
	
		static float pwm_period_sek;
		static float pwm_min_sek;
		static float pwm_max_sek;
		
		//static unsigned short last_uav_value;
		void calculate_min_max_cur_uav_rc_values ();
		void SetPeriod_Freq (float val);
		static void set_raw_pwm (unsigned short val);
		static bool f_enable_normal_out;
		
		// c_actuator_matrix_pwm_min и c_actuator_matrix_pwm_max - используется для коррекции физических величин PWM  при преобразовании из логической системы управления (1: -1).
		// результат после коррекции входных логических данных может превышать рабочий физический диапазон ограниченный рамками pwm_min_raw-pwm_man_raw, поэтому должен быть отсечен
		// по максимальным границам этого диапазона
		static unsigned short c_actuator_matrix_pwm_min;	
		static unsigned short c_actuator_matrix_pwm_max;	
	
	public:
		TRCPWM ();
		void Init ();
	
		void SetPeriod_Ms (float val);
		void SetMin_Ms (float val);
		void SetMax_Ms (float val);
	
		static void SetRC_indx (unsigned char ix);
		static void SetESC_src_ID (unsigned char iddd);
	
		static unsigned char GetRC_indx ();
		static unsigned char GetESC_src_ID ();
	
		static void SetUAV_PWM_esc (short val);			// ~ 1000-2000
		static void SetUAV_PWM_actuator (float val);			// -1.0 to 1.0
	
		void ActMatrixPwm_min (unsigned short val);
		void ActMatrixPwm_max (unsigned short val);
	
		//static void SetUAV_inverse_esc (bool val);
		//static void SetUAV_inverse_act (bool val);
	
		void SetMode (unsigned char val);
	
		static void Start ();

};



#endif

