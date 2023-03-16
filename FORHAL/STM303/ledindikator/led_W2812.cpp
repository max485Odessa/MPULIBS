#include "led_W2812.h"

extern void disable_irq ();
extern void enable_irq ();

TLEDWIF::TLEDWIF ()
{
	ResPeriodControl = 0;
	SYSBIOS::ADD_TIMER_SYS (&ResPeriodControl);
	f_update_need = false;
	AddObjectToExecuteManager ();
}



void TLEDWIF::Init ()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	__HAL_RCC_GPIOB_CLK_ENABLE();

  GPIO_InitStructure.Pin = C_PCBLED_PIN;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;//GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;  //GPIO_MODE_OUTPUT_PP;//GPIO_Mode_Out_PP;
	GPIO_InitStructure.Pull = GPIO_PULLUP; 
  HAL_GPIO_Init (C_PCBLED_PORT, &GPIO_InitStructure);
	C_PCBLED_PORT->BRR = C_PCBLED_PIN;	// линию в 0
}



void TLEDWIF::Set_RGB (uint8_t R, uint8_t G, uint8_t B)
{
	RGB_dat[0] = R; RGB_dat[1] = G; RGB_dat[2] = B;
	if (ResPeriodControl)
		{
		f_update_need = true;	
		}
	else
		{
		Update ();
		}
}



// no real update
void TLEDWIF::Set_Color (EBASECOLOR enm_c)
{
	switch (enm_c)
		{
		case ECOLR_RED:
			{
			Set_RGB (C_MAXCOLOR_LEVEL, 0, 0);
			break;
			}
		case ECOLR_GREEN:
			{
			Set_RGB (0, C_MAXCOLOR_LEVEL, 0);
			break;
			}
		case ECOLR_BLUE:
			{
			Set_RGB (0, 0, C_MAXCOLOR_LEVEL);
			break;
			}
		case ECOLR_BLACK:
		default:
			{
			Set_RGB (0, 0, 0);
			break;
			}
		}
}


// real update
void TLEDWIF::Update ()
{

		disable_irq ();
		ByteColor_Tx (RGB_dat[1]);
		enable_irq ();
		S_NOP();
		disable_irq ();
		ByteColor_Tx (RGB_dat[0]);
		enable_irq ();
		S_NOP();
		disable_irq ();
		ByteColor_Tx (RGB_dat[2]);
		enable_irq ();
	
		ResPeriodControl = C_W2818_TRESETPERIOD;
}



void TLEDWIF::Task ()
{
	if (f_update_need)
		{
		if (!ResPeriodControl)
			{
			Set_RGB (RGB_dat[0], RGB_dat[1], RGB_dat[2]);
			f_update_need = false;
			}
		}
}



void TLEDWIF::Wait_T (unsigned char val)
{
	while (val--)
		{
		S_NOP();
		}
}



void TLEDWIF::Bit_Tx (bool val)
{
		C_PCBLED_PORT->BSRR = C_PCBLED_PIN;
		S_NOP();
		if (val)
			{
			S_NOP();
			S_NOP();
			S_NOP();
			C_PCBLED_PORT->BRR = C_PCBLED_PIN;
			}
		else
			{
			C_PCBLED_PORT->BRR = C_PCBLED_PIN;
			S_NOP();
			}
}



void TLEDWIF::ByteColor_Tx (unsigned char colr)
{
unsigned char cntr = 0;
__HAL_FLASH_INSTRUCTION_CACHE_DISABLE ();
while (cntr < 8)
	{
	Bit_Tx (colr & 0x80);
	colr <<=1;
	cntr++;	
	}
__HAL_FLASH_INSTRUCTION_CACHE_ENABLE ();
}



TLED::TLED ()
{
}



void TLED::color_rgb (uint8_t r, uint8_t g, uint8_t b)
{
	RGB_dat[0] = r;
	RGB_dat[1] = g;
	RGB_dat[2] = b;
}



void TLED::color (EBASECOLOR eclr)
{
}



TLEDS::TLEDS (const S_GPIOPIN *pn, uint8_t cnt) : gp(pn), c_ar_cnt (cnt)
{
	_pin_low_init_out_pp ((S_GPIOPIN*)pn, 1);
	//c_ar_cnt = cnt;
	relax_time = 0;
	SYSBIOS::ADD_TIMER_SYS (&relax_time);
	array = new TLED[cnt];
	f_need_update = true;
}



void TLEDS::Bit_Tx (bool val)
{
		gp->port->BSRR = gp->pin;
		S_NOP();
		if (val)
			{
			S_NOP();
			S_NOP();
			S_NOP();
			gp->port->BRR = gp->pin;
			}
		else
			{
			gp->port->BRR = gp->pin;
			S_NOP();
			}
}



void TLEDS::Tx8bit (uint8_t colr)
{
uint8_t cntr = 0;
__HAL_FLASH_INSTRUCTION_CACHE_DISABLE ();
while (cntr < 8)
	{
	Bit_Tx (colr & 0x80);
	colr <<=1;
	cntr++;	
	}
__HAL_FLASH_INSTRUCTION_CACHE_ENABLE ();
}



void TLEDS::update (TLED *ld)
{
		disable_irq ();
		Tx8bit (ld->RGB_dat[1]);
		enable_irq ();
		S_NOP();
		disable_irq ();
		Tx8bit (ld->RGB_dat[0]);
		enable_irq ();
		S_NOP();
		disable_irq ();
		Tx8bit (ld->RGB_dat[2]);
		enable_irq ();
}



void TLEDS::update ()
{
	uint8_t ix = 0;
	while (ix < c_ar_cnt)
		{
		update (&array[ix]);
		ix++;
		}
}



void TLEDS::all_color (uint8_t r, uint8_t g, uint8_t b)
{
	uint8_t ix = 0;
	while (ix < c_ar_cnt)
		{
		array[ix].color_rgb (r,g,b);
		ix++;
		}
	f_need_update = true;
}



void TLEDS::color (uint8_t ix, uint8_t r, uint8_t g, uint8_t b)
{
	if (ix < c_ar_cnt) array[ix].color_rgb (r,g,b);
}



void TLEDS::Task ()
{
	if (!relax_time) {
		if (f_need_update) {
			update ();
			f_need_update = false;
			}
		relax_time = C_LEDSTROBE_FRAME_TIME;
		}
}


