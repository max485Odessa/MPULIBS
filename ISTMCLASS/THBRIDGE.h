#ifndef _H_HALF_BRIDGE_POWER_SUPLY_H_
#define _H_HALF_BRIDGE_POWER_SUPLY_H_


#include "TPWM.h"
#include "hard_rut.h"

enum EHBRPINS {EHBRPINS_PWM = 0, EHBRPINS_PHASE, EHBRPINS_ENDENUM};

typedef struct {
	S_PWM_INIT_LIST_T p[EHBRPINS_ENDENUM];
} S_HBRIDGE_INIT_LIST_T;


#define C_HBR_NEXTPH_PULSE 2

// логика работы предназначена под специфичную внешнюю схемотехнику, выполненную на
// 2 елементах логического 'И' и на одном элементе CLK/D тригера с выходом инверсии.
// Позволяет использовать любые типы микроконтроллеров с 2-мя выходами PWM.
// Один PWM используется для задания частоты и длительности импульса, второй для переключения очередности фазы.
class THBRIDGE {
		TPWM *pwm_base;
		uint32_t period_hbr_mks;
		uint32_t pwm_hbr_mks;
		bool f_hbr_enable;
		S_HBRIDGE_INIT_LIST_T hbrpins;
	
		uint32_t max_width ();
	
	public:
		THBRIDGE (ESYSTIM t, uint32_t period, uint32_t hz_clk, S_HBRIDGE_INIT_LIST_T *ls);
		void set_hbr_period (uint32_t mks);
		void set_hbr_pwm (uint32_t mks);
		void hbr_enable (bool val);
};


#endif

