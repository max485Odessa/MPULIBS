#ifndef _H_HARDWARE_RUTINES_H_
#define _H_HARDWARE_RUTINES_H_

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"



typedef struct {
	GPIO_TypeDef *port;
	uint16_t pin;
	bool def_val;
} S_GPIOPIN;


void hard_gpio_clock_enable (GPIO_TypeDef *port);
void _pin_low_init_out_od ( S_GPIOPIN *lp_pin, unsigned char cnt);
void _pin_low_init_out_pp ( S_GPIOPIN *lp_pin, unsigned char cnt);
void _pin_low_init_in ( S_GPIOPIN *lp_pin, unsigned char cnt);
void _pin_low_init_adc ( S_GPIOPIN *lp_pin, unsigned char cnt);
void _pin_low_init_af_o_pp (S_GPIOPIN *lp_pin, unsigned char cnt);
void _pin_pp_to (S_GPIOPIN *lp_pin, bool val);
bool _pin_get (S_GPIOPIN *lp_pin);



#endif

