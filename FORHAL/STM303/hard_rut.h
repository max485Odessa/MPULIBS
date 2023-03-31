#ifndef _H_HARDWARE_RUTINES_H_
#define _H_HARDWARE_RUTINES_H_

#include "stm32f30x.h"
#include "stm32f30x_gpio.h"
#include "stm32f30x_tim.h"
#include "stm32f30x_rcc.h"



typedef struct {
	GPIO_TypeDef *port;
	uint16_t pin;
} S_GPIOPIN;


void hard_gpio_clock_enable (GPIO_TypeDef *port);
void _pin_low_init_out_op ( S_GPIOPIN *lp_pin, unsigned char cnt);
void _pin_low_init_out_pp ( S_GPIOPIN *lp_pin, unsigned char cnt);
void _pin_low_init_in ( S_GPIOPIN *lp_pin, unsigned char cnt);
void _pin_low_init_adc ( S_GPIOPIN *lp_pin, unsigned char cnt);
void _pin_low_init_af_o_pp (S_GPIOPIN *lp_pin, unsigned char cnt);
//IRQn_Type _hard_get_irq_vector_usart (USART_TypeDef *prt);
//uint8_t _hard_get_index_vector_usart (USART_TypeDef *prt);

#endif

