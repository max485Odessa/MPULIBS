#include "led_W2812.h"



TLED::TLED ()
{
	bright_mult = 1.0F;
}



void TLED::set_bright (float val)
{
	if (val > 1.0F) val = 1.0F;
	if (val < 0) val = 0;
	bright_mult = val;
}



void TLED::color_rgb (uint8_t r, uint8_t g, uint8_t b)
{
	RGB_dat[0] = bright_mult * r;
	RGB_dat[1] = bright_mult * g;
	RGB_dat[2] = bright_mult * b;
}



void TLED::color_rgb (const s_rgb_t &c)
{
	RGB_dat[0] = bright_mult * c.r;
	RGB_dat[1] = bright_mult * c.g;
	RGB_dat[2] = bright_mult * c.b;
}




static const s_rgb_t arrdefcolor[ECOLR_ENDENUM] = {{0,0,0},{0xFF,0,0},{0,0xFF,0},{0,0,0xFF}, {0xFF,0xFF,0}};
void TLED::color (EBASECOLOR eclr)
{
	s_rgb_t ccolr;
	color (eclr, ccolr, bright_mult);
	RGB_dat[0] = ccolr.r;
	RGB_dat[1] = ccolr.g;
	RGB_dat[2] = ccolr.b;
}



void TLED::color (EBASECOLOR eclr, s_rgb_t &dst, float brlv)
{
	if (eclr < ECOLR_ENDENUM)
		{
		s_rgb_t *sc = const_cast<s_rgb_t*>(&arrdefcolor[eclr]);
		dst.r = brlv * sc->r;
		dst.g = brlv * sc->g;
		dst.b = brlv * sc->b;
		}
	else
		{
		dst.r = 0;
		dst.g = 0;
		dst.b = 0;
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



void TLEDS::all_color (const s_rgb_t &c)
{
	all_color (c.r, c.g, c.b);
}



void TLEDS::all_bright (float val)
{
	uint8_t ix = 0;
	while (ix < c_ar_cnt)
		{
		array[ix].set_bright (val);
		ix++;
		}
	f_need_update = true;
}



void TLEDS::bright (uint8_t ix, float val)
{
	if (ix < c_ar_cnt) 
		{
		array[ix].set_bright (val);
		f_need_update = true;
		}
}



void TLEDS::color (uint8_t ix, const s_rgb_t &c)
{
	if (ix < c_ar_cnt)  {
		array[reixmx[ix]].color_rgb (c);
		f_need_update = true;
		}
}



void TLEDS::color (uint8_t ix, uint8_t r, uint8_t g, uint8_t b)
{
	if (ix < c_ar_cnt) {
		array[reixmx[ix]].color_rgb (r,g,b);
		f_need_update = true;
		}
	
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


