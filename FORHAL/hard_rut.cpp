#include "hard_rut.h"



enum EGPIOIX {EGPIOIX_A = 0, EGPIOIX_B = 1, EGPIOIX_C = 2, EGPIOIX_ENDENUM = 3};
static bool cur_clock_state[EGPIOIX_ENDENUM] = {0,0,0};


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
	

	//hard_gpio_init_raw (&GPIO_InitStructure, lp_pin, cnt);
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


#ifdef STM32F446xx
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

