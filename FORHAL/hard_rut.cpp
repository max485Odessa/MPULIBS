#include "hard_rut.h"



enum EGPIOIX {EGPIOIX_A = 0, EGPIOIX_B, EGPIOIX_C, EGPIOIX_D, EGPIOIX_E, EGPIOIX_F, EGPIOIX_ENDENUM};
static bool cur_clock_state[EGPIOIX_ENDENUM] = {0,0,0,0,0,0};
static bool cur_clock_state_tim[ESYSTIM_ENDENUM] = {0,0,0,0,0,0,0,0,0,0,0,0};
static bool cur_clock_state_spi[ESYSSPI_ENDENUM] = {0,0,0,0,0,0};
static bool cur_clock_state_uart[ESYSUSART_ENDENUM] = {0,0,0,0,0,0};
static const TIM_TypeDef *cur_port_tim[ESYSTIM_ENDENUM] = {TIM1,TIM2,TIM3,TIM4,TIM5,0,0,0,TIM9,TIM10,TIM11,0};
static const SPI_TypeDef *cur_port_spi[ESYSSPI_ENDENUM] = {SPI1,SPI2,SPI3,SPI4,SPI5,0};
static const bool period_tim[ESYSTIM_ENDENUM] = {0,true,0,0,true,0,0,0,0,0,0,0};
static const IRQn_Type spiirq_type[ESYSSPI_ENDENUM] = {SPI1_IRQn, SPI2_IRQn, SPI3_IRQn, SPI4_IRQn, SPI5_IRQn};



IRQn_Type hard_spi_irq_type (ESYSSPI p)
{
	IRQn_Type rv = (IRQn_Type)0xFF;
	if (p < ESYSSPI_ENDENUM) rv = spiirq_type[p];
	return rv;
}


TIM_TypeDef *hard_get_tim (ESYSTIM tn, bool *f_wdth_32)
{
	TIM_TypeDef *rv = 0;
	if (tn < ESYSTIM_ENDENUM) {
		rv = const_cast<TIM_TypeDef*>(cur_port_tim[tn]);
		if (f_wdth_32) *f_wdth_32 = period_tim[tn];
		}
	return rv;
}


SPI_TypeDef *hard_get_spi (ESYSSPI tn)
{
	SPI_TypeDef *rv = 0;
	if (tn < ESYSSPI_ENDENUM) {
		rv = const_cast<SPI_TypeDef*>(cur_port_spi[tn]);
		}
	return rv;
}



bool hard_tim_is_32bit (ESYSTIM tn)
{
bool rv = false;
	if (tn < ESYSTIM_ENDENUM) {
		rv = period_tim[tn];
		}
	return rv;
}




void hard_spi_clock_enable (ESYSSPI tn)
{
	bool f_sets = false;
	if (cur_clock_state_spi[tn]) return;
	do	{
			#ifdef SPI1
			if (tn == ESYSSPI_1) {
				__HAL_RCC_SPI1_CLK_ENABLE();
				f_sets = true;
				break;
				}
			#endif
				
			#ifdef SPI2
			if (tn == ESYSSPI_2) {
				__HAL_RCC_SPI2_CLK_ENABLE();
				f_sets = true;
				break;
				}
			#endif
				
			#ifdef SPI3
			if (tn == ESYSSPI_3) {
				__HAL_RCC_SPI3_CLK_ENABLE();
				f_sets = true;
				break;
				}
			#endif
				
			#ifdef SPI4
			if (tn == ESYSSPI_4) {
				__HAL_RCC_SPI4_CLK_ENABLE();
				f_sets = true;
				break;
				}
			#endif
				
				
			#ifdef SPI5
			if (tn == ESYSSPI_5) {
				__HAL_RCC_SPI5_CLK_ENABLE();
				f_sets = true;
				break;
				}
			#endif
				
			#ifdef SPI6
			if (tn == ESYSSPI_6) {
				__HAL_RCC_SPI6_CLK_ENABLE();
				f_sets = true;
				break;
				}
			#endif
				
			} while (false);
	
	if (f_sets) cur_clock_state_spi[tn] = true;	
}



void hard_tim_clock_enable (ESYSTIM tn)
{
	bool f_sets = false;
	if (cur_clock_state_tim[tn]) return;
	do	{
			#ifdef TIM1
			if (tn == ESYSTIM_1) {
				__HAL_RCC_TIM1_CLK_ENABLE();
				f_sets = true;
				break;
				}
			#endif
				
			#ifdef TIM2
			if (tn == ESYSTIM_2) {
				__HAL_RCC_TIM2_CLK_ENABLE();
				f_sets = true;
				break;
				}
			#endif
				
			#ifdef TIM3
			if (tn == ESYSTIM_3) {
				__HAL_RCC_TIM3_CLK_ENABLE();
				f_sets = true;
				break;
				}
			#endif
				
			#ifdef TIM4
			if (tn == ESYSTIM_4) {
				__HAL_RCC_TIM4_CLK_ENABLE();
				f_sets = true;
				break;
				}
			#endif
				
				
			#ifdef TIM5
			if (tn == ESYSTIM_5) {
				__HAL_RCC_TIM5_CLK_ENABLE();
				f_sets = true;
				break;
				}
			#endif
				
			#ifdef TIM6
			if (tn == ESYSTIM_TIM6) {
				__HAL_RCC_TIM6_CLK_ENABLE();
				f_sets = true;
				break;
				}
			#endif
				
			#ifdef TIM7
			if (tn == ESYSTIM_TIM7) {
				__HAL_RCC_TIM7_CLK_ENABLE();
				f_sets = true;
				break;
				}
			#endif
				
			#ifdef TIM8
			if (tn == ESYSTIM_TIM8) {
				__HAL_RCC_TIM8_CLK_ENABLE();
				f_sets = true;
				break;
				}
			#endif
			
			#ifdef TIM9
			if (tn == ESYSTIM_9) {
				__HAL_RCC_TIM9_CLK_ENABLE();
				f_sets = true;
				break;
				}
			#endif
				
			#ifdef TIM10
			if (tn == ESYSTIM_10) {
				__HAL_RCC_TIM10_CLK_ENABLE();
				f_sets = true;
				break;
				}
			#endif
				
			#ifdef TIM11
			if (tn == ESYSTIM_11) {
				__HAL_RCC_TIM11_CLK_ENABLE();
				f_sets = true;
				break;
				}
			#endif
				
			#ifdef TIM12
			if (tn == ESYSTIM_TIM12) {
				__HAL_RCC_TIM12_CLK_ENABLE();
				f_sets = true;
				break;
				}
			#endif
			} while (false);
	
	if (f_sets) cur_clock_state_tim[tn] = true;
}




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
				if (!cur_clock_state_uart[ESYSUSART_1])
					{
					__HAL_RCC_USART1_CLK_ENABLE ();
					cur_clock_state_uart[ESYSUSART_1] = true;
					}
				break;
				}
			if (p == USART2)
				{
				if (!cur_clock_state_uart[ESYSUSART_2])
					{
					__HAL_RCC_USART2_CLK_ENABLE ();
					cur_clock_state_uart[ESYSUSART_2] = true;
					}
				break;
				}
			#ifdef USART3
			if (p == USART3)
				{
				if (!cur_clock_state_uart[ESYSUSART_3])
					{
					__HAL_RCC_USART3_CLK_ENABLE ();
					cur_clock_state_uart[ESYSUSART_3] = true;
					}
				break;
				}
			#endif
			#ifdef USART4
			if (p == UART4)
				{
				if (!cur_clock_state_uart[ESYSUSART_4])
					{
					__HAL_RCC_UART4_CLK_ENABLE ();
					cur_clock_state_uart[ESYSUSART_4] = true;
					}
				break;
				}
			#endif
			#ifdef UART5
			if (p == UART5)
				{
				if (!cur_clock_state_uart[ESYSUSART_5])
					{
					__HAL_RCC_UART5_CLK_ENABLE ();
					cur_clock_state_uart[ESYSUSART_5] = true;
					}
				break;
				}
			#endif
			#ifdef UART6
			if (p == USART6)
				{
				if (!cur_clock_state_uart[ESYSUSART_6])
					{
					__HAL_RCC_USART6_CLK_ENABLE ();
					cur_clock_state_uart[ESYSUSART_6] = true;
					}
				break;
				}
			#endif
			} while (false);
}



void hard_usart_clock_disable (USART_TypeDef *p)
{
	do	{
			#ifdef USART1
			if (p == USART1)
				{
				if (cur_clock_state_uart[ESYSUSART_1])
					{
					__HAL_RCC_USART1_CLK_DISABLE (); 
					cur_clock_state_uart[ESYSUSART_1] = false;
					}
				break;
				}
			#endif

			#ifdef USART2
			if (p == USART2)
				{
				if (cur_clock_state_uart[ESYSUSART_2])
					{
					__HAL_RCC_USART2_CLK_DISABLE ();
					cur_clock_state_uart[ESYSUSART_2] = false;
					}
				break;
				}
			#endif

			#ifdef USART3
			if (p == USART3)
				{
				if (cur_clock_state_uart[ESYSUSART_3])
					{
					__HAL_RCC_USART3_CLK_DISABLE ();
					cur_clock_state_uart[ESYSUSART_3] = false;
					}
				break;
				}
			#endif

			#ifdef UART4
			if (p == UART4)
				{
				if (cur_clock_state_uart[ESYSUSART_4])
					{
					__HAL_RCC_UART4_CLK_DISABLE ();
					cur_clock_state_uart[ESYSUSART_4] = false;
					}
				break;
				}
			#endif

			#ifdef UART5
			if (p == UART5)
				{
				if (cur_clock_state_uart[ESYSUSART_5])
					{
					__HAL_RCC_UART5_CLK_DISABLE ();
					cur_clock_state_uart[ESYSUSART_5] = false;
					}
				break;
				}
			#endif

			#ifdef USART6
			if (p == USART6)
				{
				if (cur_clock_state_uart[ESYSUSART_6])
					{
					__HAL_RCC_USART6_CLK_DISABLE ();
					cur_clock_state_uart[ESYSUSART_6] = false;
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




void _pin_low_init_out_pp (S_GPIOPIN *lp_pin, unsigned char cnt, EHRTGPIOSPEED sp)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	while (cnt)
		{
		hard_gpio_clock_enable (lp_pin->port);
		GPIO_InitStructure.Speed = sp;

		GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStructure.Pull = GPIO_NOPULL;
	#if (HRDCPU == 2 || HRDCPU == 3 || HRDCPU == 4)
		GPIO_InitStructure.Alternate = 0;
	#endif
		GPIO_InitStructure.Pin = lp_pin->pin;
		HAL_GPIO_Init (lp_pin->port, &GPIO_InitStructure);
		lp_pin++;
		cnt--;
		}		
}


#if (HRDCPU == 1)
void _pin_low_init_out_pp_af (S_GPIOPIN *lp_pin, EHRTGPIOSPEED sp )
#else
void _pin_low_init_out_pp_af ( uint8_t af_codemux, S_GPIOPIN *lp_pin, EHRTGPIOSPEED sp )
#endif
{
	GPIO_InitTypeDef GPIO_InitStructure;
		hard_gpio_clock_enable (lp_pin->port);
		GPIO_InitStructure.Speed = sp;
		GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStructure.Pull = GPIO_PULLDOWN;//GPIO_NOPULL;
	#if (HRDCPU == 2 || HRDCPU == 3 || HRDCPU == 4)
		GPIO_InitStructure.Alternate = af_codemux;
	#endif
		GPIO_InitStructure.Pin = lp_pin->pin;
		HAL_GPIO_Init (lp_pin->port, &GPIO_InitStructure);
}



#if (HRDCPU == 1)
void _pin_low_init_out_od_af (S_GPIOPIN *lp_pin, EHRTGPIOSPEED sp )
#else
void _pin_low_init_out_od_af ( uint8_t af_codemux, S_GPIOPIN *lp_pin, EHRTGPIOSPEED sp )
#endif
{
	GPIO_InitTypeDef GPIO_InitStructure;
		hard_gpio_clock_enable (lp_pin->port);
		GPIO_InitStructure.Speed = sp;
		GPIO_InitStructure.Mode = GPIO_MODE_AF_OD;
		GPIO_InitStructure.Pull = GPIO_PULLUP;
	#if (HRDCPU == 2 || HRDCPU == 3 || HRDCPU == 4)
		GPIO_InitStructure.Alternate = af_codemux;
	#endif
		GPIO_InitStructure.Pin = lp_pin->pin;
		HAL_GPIO_Init (lp_pin->port, &GPIO_InitStructure);
}



void _pin_low_init_in_af ( uint8_t af_codemux, S_GPIOPIN *lp_pin )
{
	GPIO_InitTypeDef GPIO_InitStructure;
		hard_gpio_clock_enable (lp_pin->port);
		GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;//GPIO_SPEED_FREQ_LOW;// GPIO_SPEED_FREQ_MEDIUM;
		GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;//GPIO_MODE_INPUT;//GPIO_MODE_AF_PP;
		GPIO_InitStructure.Pull = GPIO_PULLUP;
		GPIO_InitStructure.Alternate = af_codemux;
		GPIO_InitStructure.Pin = lp_pin->pin;
		HAL_GPIO_Init (lp_pin->port, &GPIO_InitStructure);
}



void _pin_low_init_out_od (S_GPIOPIN *lp_pin, unsigned char cnt, EHRTGPIOSPEED sp)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	while (cnt)
		{
		hard_gpio_clock_enable (lp_pin->port);
		GPIO_InitStructure.Speed = sp;
		GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD;
		GPIO_InitStructure.Pull = GPIO_NOPULL;
	#if (HRDCPU == 2 || HRDCPU == 3 || HRDCPU == 4)
		GPIO_InitStructure.Alternate = 0;
	#endif
		GPIO_InitStructure.Pin = lp_pin->pin;
		HAL_GPIO_Init (lp_pin->port, &GPIO_InitStructure);
		lp_pin++;
		cnt--;
		}	
}



void _pin_low_init_in (S_GPIOPIN *lp_pin, uint8_t cnt, EHRTGPIOSPEED sp, EHRTGPIOPULL pl)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	while (cnt)
		{
		hard_gpio_clock_enable (lp_pin->port);
		GPIO_InitStructure.Speed = sp;
		GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
		GPIO_InitStructure.Pull = pl;
	#if (HRDCPU == 2 || HRDCPU == 3 || HRDCPU == 4)
		GPIO_InitStructure.Alternate = 0;
	#endif
		GPIO_InitStructure.Pin = lp_pin->pin;
		HAL_GPIO_Init (lp_pin->port, &GPIO_InitStructure);
		lp_pin++;
		cnt--;
		}
}



/*
void _pin_low_init_in (S_GPIOPIN *lp_pin, unsigned char cnt, EHRTGPIOSPEED sp)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	while (cnt)
		{
		hard_gpio_clock_enable (lp_pin->port);
		GPIO_InitStructure.Speed = sp;
		GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
		GPIO_InitStructure.Pull = GPIO_NOPULL;
	#if (HRDCPU == 2 || HRDCPU == 3 || HRDCPU == 4)
		GPIO_InitStructure.Alternate = 0;
	#endif
		GPIO_InitStructure.Pin = lp_pin->pin;
		HAL_GPIO_Init (lp_pin->port, &GPIO_InitStructure);
		lp_pin++;
		cnt--;
		}
}
*/


static const uint32_t modintarr[EGPINTMOD_ENDENUM] = {GPIO_MODE_IT_RISING, GPIO_MODE_IT_FALLING, GPIO_MODE_IT_RISING_FALLING};

void _pin_low_init_int (S_GPIOPIN *lp_pin, unsigned char cnt, EGPINTMOD md, EHRTGPIOSPEED sp)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	while (cnt)
		{
		hard_gpio_clock_enable (lp_pin->port);
		GPIO_InitStructure.Speed = sp;
		GPIO_InitStructure.Mode = modintarr[md];
		GPIO_InitStructure.Pull = C_HRDGPIO_NOPULL;
	#if (HRDCPU == 2 || HRDCPU == 3 || HRDCPU == 4)
		GPIO_InitStructure.Alternate = 0;
	#endif
		GPIO_InitStructure.Pin = lp_pin->pin;
		HAL_GPIO_Init (lp_pin->port, &GPIO_InitStructure);
		lp_pin++;
		cnt--;
		}
}



void _pin_low_init_adc (S_GPIOPIN *lp_pin, unsigned char cnt, EHRTGPIOSPEED sp)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Speed = sp;
	GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStructure.Pull = C_HRDGPIO_NOPULL;
#if (HRDCPU == 2 || HRDCPU == 3 || HRDCPU == 4)
	GPIO_InitStructure.Alternate = 0;
#endif
	
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



void _pin_low_init (S_GPMD_PIN_T *lp_pin, unsigned char cnt, EHRTGPIOSPEED sp, EHRTGPIOPULL pl)
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
					_pin_low_init_out_od (&lp_pin->pin, 1, sp);
					_pin_set_to (&lp_pin->pin, lp_pin->out_mod_def_set);
					break;
					}
				case EGPMD_PP:
					{
					_pin_low_init_out_pp (&lp_pin->pin, 1, sp);
					_pin_set_to (&lp_pin->pin, lp_pin->out_mod_def_set);
					break;
					}
				case EGPMD_AIN:
					{
					_pin_low_init_adc (&lp_pin->pin, 1, sp);
					break;
					}
				case EGPMD_IN:
				default:
					{
					_pin_low_init_in (&lp_pin->pin, 1, sp, pl);
					break;
					}
				}
			}
		lp_pin++;
		cnt--;
		}
	}
}


void _pin_output (S_GPIOPIN *lp_pin, bool val)
{
  if (val)
		{
		#if (HRDCPU == 4 || HRDCPU == 1)
    lp_pin->port->BSRR = (uint32_t)lp_pin->pin;
		#else
			#error need cpu class deffine
		#endif
		}
  else
  {

		#if (HRDCPU == 4 || HRDCPU == 1)
		lp_pin->port->BSRR = (uint32_t)lp_pin->pin << 16;
		#else
			#error need cpu class deffine
		#endif
  }

}


