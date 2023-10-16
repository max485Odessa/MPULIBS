#include "hard_rut.h"



enum EGPIOIX {EGPIOIX_A = 0, EGPIOIX_B, EGPIOIX_C, EGPIOIX_D, EGPIOIX_E, EGPIOIX_F, EGPIOIX_ENDENUM};
static bool cur_clock_state[EGPIOIX_ENDENUM] = {0,0,0,0,0,0};


void hard_gpio_clock_enable (GPIO_TypeDef *port)
{
	do {
			if (port == GPIOA) 
				{
				if (!cur_clock_state[EGPIOIX_A]) 
					{
					__HAL_RCC_GPIOA_CLK_ENABLE();
					cur_clock_state[EGPIOIX_A] = true;
					}
				break;
				}
			if (port == GPIOB) 
				{
				if (!cur_clock_state[EGPIOIX_B]) 
					{
					__HAL_RCC_GPIOB_CLK_ENABLE();
					cur_clock_state[EGPIOIX_B] = true;
					}
				break;
				}
			if (port == GPIOC) 
				{
				if (!cur_clock_state[EGPIOIX_C]) 
					{
					__HAL_RCC_GPIOC_CLK_ENABLE();
					cur_clock_state[EGPIOIX_C] = true;
					}
				break;
				}
			#ifdef GPIOD
			if (port == GPIOD) 
				{
				if (!cur_clock_state[EGPIOIX_D]) 
					{
					__HAL_RCC_GPIOD_CLK_ENABLE();
					cur_clock_state[EGPIOIX_D] = true;
					}
				break;
				}
			#endif
			#ifdef GPIOE
			if (port == GPIOE) 
				{
				if (!cur_clock_state[EGPIOIX_E]) 
					{
					__HAL_RCC_GPIOE_CLK_ENABLE();
					cur_clock_state[EGPIOIX_E] = true;
					}
				break;
				}
			#endif
			#ifdef GPIOF
			if (port == GPIOF) 
				{
				if (!cur_clock_state[EGPIOIX_F]) 
					{
					__HAL_RCC_GPIOF_CLK_ENABLE();
					cur_clock_state[EGPIOIX_F] = true;
					}
				break;
				}
			#endif
			} while (false);
}



void hard_usart_clock_enable (USART_TypeDef *p)
{
	do	{
			if (p == USART1)
				{
				static bool f_active = false;
				if (!f_active)
					{
					__HAL_RCC_USART1_CLK_ENABLE ();
					f_active = true;
					}
				break;
				}
			if (p == USART2)
				{
				static bool f_active = false;
				if (!f_active)
					{
					__HAL_RCC_USART2_CLK_ENABLE ();
					f_active = true;
					}
				break;
				}
			#ifdef USART3
			if (p == USART3)
				{
				static bool f_active = false;
				if (!f_active)
					{
					__HAL_RCC_USART3_CLK_ENABLE ();
					f_active = true;
					}
				break;
				}
			#endif
			#ifdef USART4
			if (p == UART4)
				{
				static bool f_active = false;
				if (!f_active)
					{
					__HAL_RCC_UART4_CLK_ENABLE ();
					f_active = true;
					}
				break;
				}
			#endif
			#ifdef UART5
			if (p == UART5)
				{
				static bool f_active = false;
				if (!f_active)
					{
					__HAL_RCC_UART5_CLK_ENABLE ();
					f_active = true;
					}
				break;
				}
			#endif
			#ifdef UART6
			if (p == USART6)
				{
				static bool f_active = false;
				if (!f_active)
					{
					__HAL_RCC_USART6_CLK_ENABLE ();
					f_active = true;
					}
				break;
				}
			#endif
			} while (false);
}



static void hard_gpio_init_raw (GPIO_InitTypeDef *gpio, S_GPIOPIN *inarr, unsigned short cnt)
{
	while (cnt)
		{
		hard_gpio_clock_enable (inarr->port);
		gpio->Pin = inarr->pin;
		HAL_GPIO_Init (inarr->port, gpio);
		inarr++;
		cnt--;
		}
}




void _pin_low_init_out_pp (S_GPIOPIN *lp_pin, unsigned char cnt)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	while (cnt)
		{
		hard_gpio_clock_enable (lp_pin->port);
		GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;// GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStructure.Pull = GPIO_NOPULL;
		GPIO_InitStructure.Alternate = 0;
		GPIO_InitStructure.Pin = lp_pin->pin;
		HAL_GPIO_Init (lp_pin->port, &GPIO_InitStructure);
		lp_pin++;
		cnt--;
		}		
}



void _pin_low_init_out_pp_af ( uint8_t af_codemux, S_GPIOPIN *lp_pin )
{
	GPIO_InitTypeDef GPIO_InitStructure;
		hard_gpio_clock_enable (lp_pin->port);
		GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStructure.Pull = GPIO_NOPULL;
		GPIO_InitStructure.Alternate = af_codemux;
		GPIO_InitStructure.Pin = lp_pin->pin;
		HAL_GPIO_Init (lp_pin->port, &GPIO_InitStructure);
}



void _pin_low_init_out_od (S_GPIOPIN *lp_pin, unsigned char cnt)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	while (cnt)
		{
		hard_gpio_clock_enable (lp_pin->port);
		GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;//GPIO_SPEED_FREQ_HIGH;// GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD;
		GPIO_InitStructure.Pull = GPIO_NOPULL;
		GPIO_InitStructure.Alternate = 0;
		GPIO_InitStructure.Pin = lp_pin->pin;
		HAL_GPIO_Init (lp_pin->port, &GPIO_InitStructure);
		lp_pin++;
		cnt--;
		}	
}



void _pin_low_init_in_pull (S_GPIOPIN *lp_pin, uint8_t cnt, bool vl)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	while (cnt)
		{
		hard_gpio_clock_enable (lp_pin->port);
		GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
		GPIO_InitStructure.Pull = (vl)?GPIO_PULLUP:GPIO_PULLDOWN;
		GPIO_InitStructure.Alternate = 0;
		GPIO_InitStructure.Pin = lp_pin->pin;
		HAL_GPIO_Init (lp_pin->port, &GPIO_InitStructure);
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
		GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;// GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
		GPIO_InitStructure.Pull = GPIO_NOPULL;//GPIO_NOPULL;
		GPIO_InitStructure.Alternate = 0;
		GPIO_InitStructure.Pin = lp_pin->pin;
		HAL_GPIO_Init (lp_pin->port, &GPIO_InitStructure);
		lp_pin++;
		cnt--;
		}
}


static const uint32_t modintarr[EGPINTMOD_ENDENUM] = {GPIO_MODE_IT_RISING, GPIO_MODE_IT_FALLING, GPIO_MODE_IT_RISING_FALLING};

void _pin_low_init_int (S_GPIOPIN *lp_pin, unsigned char cnt, EGPINTMOD md)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	while (cnt)
		{
		hard_gpio_clock_enable (lp_pin->port);
		GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;// GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStructure.Mode = modintarr[md];
		GPIO_InitStructure.Pull = GPIO_NOPULL;//GPIO_NOPULL;
		GPIO_InitStructure.Alternate = 0;
		GPIO_InitStructure.Pin = lp_pin->pin;
		HAL_GPIO_Init (lp_pin->port, &GPIO_InitStructure);
		lp_pin++;
		cnt--;
		}
}



void _pin_low_init_adc (S_GPIOPIN *lp_pin, unsigned char cnt)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Alternate = 0;
	
	hard_gpio_init_raw (&GPIO_InitStructure, lp_pin, cnt);
}



bool _pin_input (S_GPIOPIN *lp_pin)
{
	bool rv;
  if ((lp_pin->port->IDR & lp_pin->pin) != 0x00u)
  {
    rv = true;
  }
  else
  {
    rv = false;
  }
return rv;
}



void _pin_low_init (S_GPMD_PIN_T *lp_pin, unsigned char cnt)
{
if (lp_pin) {
	while (cnt)
		{
		if (lp_pin->pin.port)
			{
			switch (lp_pin->mod)
				{
				case EGPMD_OD:
					{
					_pin_low_init_out_od (&lp_pin->pin, 1);
					_pin_set_to (&lp_pin->pin, lp_pin->out_mod_def_set);
					break;
					}
				case EGPMD_PP:
					{
					_pin_low_init_out_pp (&lp_pin->pin, 1);
					_pin_set_to (&lp_pin->pin, lp_pin->out_mod_def_set);
					break;
					}
				case EGPMD_AIN:
					{
					_pin_low_init_adc (&lp_pin->pin, 1);
					break;
					}
				case EGPMD_IN:
				default:
					{
					_pin_low_init_in (&lp_pin->pin, 1);
					break;
					}
				}
			}
		lp_pin++;
		cnt--;
		}
	}
}

#if defined(STM32F446xx) || defined(STM32F401xC)
void _pin_output (S_GPIOPIN *lp_pin, bool val)
{
  if (val)
		{
    lp_pin->port->BSRR = (uint32_t)lp_pin->pin;
		}
  else
  {
		lp_pin->port->BSRR = (uint32_t)lp_pin->pin << 16;
  }

}
#endif

