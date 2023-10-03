#include "led_W2812.h"

//extern void disable_irq ();
//extern void enable_irq ();

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
	__HAL_RCC_GPIOA_CLK_ENABLE();

  GPIO_InitStructure.Pin = C_LED_PIN;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;//GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;  //GPIO_MODE_OUTPUT_PP;//GPIO_Mode_Out_PP;
	GPIO_InitStructure.Pull = GPIO_PULLUP; 
  HAL_GPIO_Init (C_LED_PORT, &GPIO_InitStructure);
	C_LED_PORT->BSRR = C_LED_PIN << 16;	// линию в 0
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

		__disable_irq ();
		ByteColor_Tx (RGB_dat[1]);
		__enable_irq ();
		S_NOP();
		__disable_irq ();
		ByteColor_Tx (RGB_dat[0]);
		__enable_irq ();
		S_NOP();
		__disable_irq ();
		ByteColor_Tx (RGB_dat[2]);
		__enable_irq ();
	
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
		C_LED_PORT->BSRR = C_LED_PIN;
		S_NOP();
		if (val)
			{
			S_NOP();
			S_NOP();
			S_NOP();
			C_LED_PORT->BSRR = C_LED_PIN << 16;
			}
		else
			{
			C_LED_PORT->BSRR = C_LED_PIN << 16;
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
	bright_mult = 1.0F;
}



void TLED::set_bright (uint8_t val)
{
	bright_mult = val / 255.0;
}



void TLED::color_rgb (uint8_t r, uint8_t g, uint8_t b)
{
	RGB_dat[0] = bright_mult * r;
	RGB_dat[1] = bright_mult * g;
	RGB_dat[2] = bright_mult * b;
}



static const s_rgb_t arrdefcolor[ECOLR_ENDENUM] = {{0,0,0},{0xFF,0,0},{0,0xFF,0},{0,0,0xFF}, {0xFF,0xFF,0}};
void TLED::color (EBASECOLOR eclr)
{
	if (eclr < ECOLR_ENDENUM)
		{
		s_rgb_t *sc = const_cast<s_rgb_t*>(&arrdefcolor[eclr]);
		RGB_dat[0] = bright_mult * sc->r;
		RGB_dat[1] = bright_mult * sc->g;
		RGB_dat[2] = bright_mult * sc->b;
		}
}



TLEDS::TLEDS (const S_GPIOPIN *pn, uint8_t cnt, uint8_t *imx) : gp(pn), c_ar_cnt (cnt), reixmx(imx)
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
		S_NOP();
		//S_NOP();
		if (val)
			{
			S_NOP();
			S_NOP();
			S_NOP();
			S_NOP();
			gp->port->BSRR = ((uint32_t)gp->pin) << 16;
			S_NOP();
			}
		else
			{
			gp->port->BSRR = ((uint32_t)gp->pin) << 16;
			S_NOP();
			S_NOP();
			S_NOP();
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
		__disable_irq ();
		Tx8bit (ld->RGB_dat[1]);
		__enable_irq ();
		S_NOP();
		__disable_irq ();
		Tx8bit (ld->RGB_dat[0]);
		__enable_irq ();
		S_NOP();
		__disable_irq ();
		Tx8bit (ld->RGB_dat[2]);
		__enable_irq ();
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



void TLEDS::all_bright (uint8_t val)
{
	uint8_t ix = 0;
	while (ix < c_ar_cnt)
		{
		array[ix].set_bright (val);
		ix++;
		}
	f_need_update = true;
}




void TLEDS::color (uint8_t ix, uint8_t r, uint8_t g, uint8_t b)
{
	if (ix < c_ar_cnt) 
		{
		array[reixmx[ix]].color_rgb (r,g,b);
		}
	f_need_update = true;
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



// --------------------
TLEDEFFCT::TLEDEFFCT (const S_GPIOPIN *pn, uint8_t cnt, uint8_t *imx) : TLEDS (pn, cnt, imx)
{
	
}



void TLEDEFFCT::powdown_mode ()
{
	_pin_low_init_in (const_cast<S_GPIOPIN*>(gp), 1);
}



void TLEDEFFCT::gen_progress (uint8_t proc, uint8_t lightmaxlevel, EBASECOLOR colb)
{
	float procled = 100.0F / c_ar_cnt;		// процентов отображени€ на светодиод
	float lighttoproc = (float)lightmaxlevel / procled;	// €ркосной шаг на 1 процент
	
	float maxlighrled_cnt = proc / procled;	// количество максимально €рких светодиодов
	uint8_t level = proc % (char)procled;
	
	uint8_t ix = 0;
	while (ix < (uint8_t)maxlighrled_cnt && ix < c_ar_cnt)
		{
		array[reixmx[ix]].set_bright (lightmaxlevel);
		array[reixmx[ix]].color (colb);
		ix++;
		}
	if (ix < c_ar_cnt)
		{
		array[reixmx[ix]].set_bright (level);
		array[reixmx[ix]].color (colb);
		ix++;
		}
	while (ix < c_ar_cnt)
		{
		array[reixmx[ix]].color (ECOLR_BLACK);
		ix++;
		}
	f_need_update = true;
}



void TLEDEFFCT::Task ()
{
	TLEDS::Task ();
	
}


