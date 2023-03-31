#include "hard_rut.h"



enum EGPIOIX {EGPIOIX_A = 0, EGPIOIX_B = 1, EGPIOIX_C = 2, EGPIOIX_D = 3, EGPIOIX_E = 4, EGPIOIX_F = 5, EGPIOIX_G = 6, EGPIOIX_H = 7, EGPIOIX_ENDENUM = 8};
static bool cur_clock_state[EGPIOIX_ENDENUM] = {0,0,0,0,0,0,0,0};

/*
#define C_USART_AMOUNTS 3
static const IRQn_Type israrrusart[C_USART_AMOUNTS] = {USART1_IRQn, USART2_IRQn, USART3_IRQn};
static const USART_TypeDef *israrrusart_name[C_USART_AMOUNTS] = {USART1, USART2, USART3};
static IRQn_Type _hard_irq_vector_usart (uint8_t ix)
{
IRQn_Type rv = (IRQn_Type)0;
if (ix < C_USART_AMOUNTS) rv = israrrusart[ix];
return rv;
}



// порядковый номер для массива
uint8_t _hard_get_index_vector_usart (USART_TypeDef *prt)
{
uint8_t rv = 0, ix = 0;
while (ix < C_USART_AMOUNTS)
	{
	if (israrrusart_name[ix] == prt) {
		rv = ix;
		break;
		}
	ix++;
	}	
return rv;
}



IRQn_Type _hard_get_irq_vector_usart (USART_TypeDef *prt)
{
return _hard_irq_vector_usart (_hard_get_index_vector_usart (prt));
}
*/


void hard_gpio_clock_enable (GPIO_TypeDef *port)
{
	do {
			if (port == GPIOA) 
				{
				if (!cur_clock_state[EGPIOIX_A]) 
					{
					RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
					cur_clock_state[EGPIOIX_A] = true;
					}
				break;
				}
			if (port == GPIOB) 
				{
				if (!cur_clock_state[EGPIOIX_B]) 
					{
					RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
					cur_clock_state[EGPIOIX_B] = true;
					}
				break;
				}
			if (port == GPIOC) 
				{
				if (!cur_clock_state[EGPIOIX_C]) 
					{
					RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
					cur_clock_state[EGPIOIX_C] = true;
					}
				break;
				}
			if (port == GPIOD) 
				{
				if (!cur_clock_state[EGPIOIX_D]) 
					{
					RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOD, ENABLE);
					cur_clock_state[EGPIOIX_D] = true;
					}
				break;
				}
			if (port == GPIOE) 
				{
				if (!cur_clock_state[EGPIOIX_E]) 
					{
					RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE, ENABLE);
					cur_clock_state[EGPIOIX_E] = true;
					}
				break;
				}
			if (port == GPIOF) 
				{
				if (!cur_clock_state[EGPIOIX_F]) 
					{
					RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOF, ENABLE);
					cur_clock_state[EGPIOIX_F] = true;
					}
				break;
				}
			if (port == GPIOG) 
				{
				if (!cur_clock_state[EGPIOIX_G]) 
					{
					RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOG, ENABLE);
					cur_clock_state[EGPIOIX_G] = true;
					}
				break;
				}
			if (port == GPIOH) 
				{
				if (!cur_clock_state[EGPIOIX_H]) 
					{
					RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOH, ENABLE);
					cur_clock_state[EGPIOIX_H] = true;
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



void _pin_low_init_out_op (S_GPIOPIN *lp_pin, unsigned char cnt)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	while (cnt)
		{
		hard_gpio_clock_enable (lp_pin->port);
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_3;//GPIO_Speed_Level_3;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_Pin = lp_pin->pin;
		GPIO_Init (lp_pin->port, &GPIO_InitStructure);
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
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_3;//GPIO_Speed_Level_3;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Pin = lp_pin->pin;
		GPIO_Init (lp_pin->port, &GPIO_InitStructure);
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
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_1;//GPIO_Speed_Level_3;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//GPIO_PuPd_UP;//GPIO_PuPd_NOPULL;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//GPIO_OType_OD;
		GPIO_InitStructure.GPIO_Pin = lp_pin->pin;
		GPIO_Init (lp_pin->port, &GPIO_InitStructure);
		lp_pin++;
		cnt--;
		}
}



void _pin_low_init_adc (S_GPIOPIN *lp_pin, unsigned char cnt)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_Pin = lp_pin->pin;
	hard_gpio_init_raw (&GPIO_InitStructure, lp_pin, cnt);
}

//  GPIOOType_TypeDef o_type, GPIOPuPd_TypeDef plr_type

void _pin_low_init_af_o_pp (S_GPIOPIN *lp_pin, unsigned char cnt)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//plr_type; // GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; // GPIO_OType_OD;
	GPIO_InitStructure.GPIO_Pin = lp_pin->pin;
	hard_gpio_init_raw (&GPIO_InitStructure, lp_pin, cnt);	
}



