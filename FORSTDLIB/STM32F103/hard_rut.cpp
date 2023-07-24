#include "hard_rut.h"



enum EGPIOIX {EGPIOIX_A = 0, EGPIOIX_B = 1, EGPIOIX_C = 2, EGPIOIX_D = 3, EGPIOIX_E = 4, EGPIOIX_F = 5, EGPIOIX_G = 6, EGPIOIX_H = 7, EGPIOIX_ENDENUM = 8};
static bool cur_clock_state[EGPIOIX_ENDENUM] = {0,0,0,0,0,0,0,0};


void hard_gpio_clock_enable (GPIO_TypeDef *port)
{
	do {
			if (port == GPIOA) 
				{
				if (!cur_clock_state[EGPIOIX_A]) 
					{
					RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOA, ENABLE);
					cur_clock_state[EGPIOIX_A] = true;
					}
				break;
				}
			if (port == GPIOB) 
				{
				if (!cur_clock_state[EGPIOIX_B]) 
					{
					RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
					cur_clock_state[EGPIOIX_B] = true;
					}
				break;
				}
			if (port == GPIOC) 
				{
				if (!cur_clock_state[EGPIOIX_C]) 
					{
					RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
					cur_clock_state[EGPIOIX_C] = true;
					}
				break;
				}
			if (port == GPIOD) 
				{
				if (!cur_clock_state[EGPIOIX_D]) 
					{
					RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
					cur_clock_state[EGPIOIX_D] = true;
					}
				break;
				}
			if (port == GPIOE) 
				{
				if (!cur_clock_state[EGPIOIX_E]) 
					{
					RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
					cur_clock_state[EGPIOIX_E] = true;
					}
				break;
				}
			if (port == GPIOF) 
				{
				if (!cur_clock_state[EGPIOIX_F]) 
					{
					RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);
					cur_clock_state[EGPIOIX_F] = true;
					}
				break;
				}
			if (port == GPIOG) 
				{
				if (!cur_clock_state[EGPIOIX_G]) 
					{
					RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);
					cur_clock_state[EGPIOIX_G] = true;
					}
				break;
				}
			} while (false);
}




static void hard_gpio_init_raw (GPIO_InitTypeDef *gpio, S_GPIOPIN *inarr, unsigned short cnt)
{
	while (cnt)
		{
		hard_gpio_clock_enable (inarr->port);
		gpio->GPIO_Pin = inarr->pin;
		GPIO_Init (inarr->port, gpio);
		inarr++;
		cnt--;
		}
}


void _pin_pp_to (S_GPIOPIN *lp_pin, bool val)
{
	if (val)
		{
    lp_pin->port->BSRR = lp_pin->pin;
		}
  else
		{
    lp_pin->port->BRR = lp_pin->pin;
		}
}



void _pin_low_init_out_od (S_GPIOPIN *lp_pin, unsigned char cnt)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	while (cnt)
		{
		hard_gpio_clock_enable (lp_pin->port);
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;//GPIO_Speed_Level_3;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
		GPIO_InitStructure.GPIO_Pin = lp_pin->pin;
		GPIO_Init (lp_pin->port, &GPIO_InitStructure);
		_pin_pp_to (lp_pin, lp_pin->def_val);
		lp_pin++;
		cnt--;
		}
}



void _pin_low_init_out_pp (S_GPIOPIN *lp_pin, unsigned char cnt)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	while (cnt)
		{
		hard_gpio_clock_enable (lp_pin->port);
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//GPIO_Speed_Level_3;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Pin = lp_pin->pin;
		GPIO_Init (lp_pin->port, &GPIO_InitStructure);
		_pin_pp_to (lp_pin, lp_pin->def_val);
		lp_pin++;
		cnt--;
		}
}



void _pin_low_init_in (S_GPIOPIN *lp_pin, unsigned char cnt)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	while (cnt)
		{
		hard_gpio_clock_enable (lp_pin->port);
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//GPIO_Speed_Level_3;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_InitStructure.GPIO_Pin = lp_pin->pin;
		GPIO_Init (lp_pin->port, &GPIO_InitStructure);
		lp_pin++;
		cnt--;
		}
}



void _pin_low_init_adc (S_GPIOPIN *lp_pin, unsigned char cnt)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Pin = lp_pin->pin;
	hard_gpio_init_raw (&GPIO_InitStructure, lp_pin, cnt);
}



void _pin_low_init_af_o_pp (S_GPIOPIN *lp_pin, unsigned char cnt)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = lp_pin->pin;
	hard_gpio_init_raw (&GPIO_InitStructure, lp_pin, cnt);	
}


