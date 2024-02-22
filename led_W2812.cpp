#include "led_W2812.h"
#include "TGlobalISR.h"





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



void TLED::color_rgb ( s_rgb_t &c)
{
	
	RGB_dat[0] = bright_mult * c.r;
	RGB_dat[1] = bright_mult * c.g;
	RGB_dat[2] = bright_mult * c.b;
	
}



void TLED::color_rgb (s_rgb_t *lc)
{
	RGB_dat[0] = bright_mult * lc->r;
	RGB_dat[1] = bright_mult * lc->g;
	RGB_dat[2] = bright_mult * lc->b;
}




static const s_rgb_t arrdefcolor[ECOLR_ENDENUM] = {{0,0,0},{0xFF,0,0},{0,0xFF,0},{0,0,0xFF}, {0xFF,0xFF,0}, {0xFF,0xFF,0xFF}};
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
	relax_time = 0;
	SYSBIOS::ADD_TIMER_SYS (&relax_time);
	uint8_t ix = 0;
	array = new TLED*[c_ar_cnt];
	while (ix < c_ar_cnt)
		{
		array[ix] = new TLED ();
		ix++;
		}
	//sw_isr = ELEDSWISR_SYNC;
	f_need_update = true;
}



uint8_t TLEDS::leds_cnt ()
{
	return c_ar_cnt;
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
uint32_t cntr8 = 8;
TGLOBISR::disable ();
//__HAL_FLASH_INSTRUCTION_CACHE_DISABLE ();	
while (cntr8)
	{
	Bit_Tx (colr & 0x80);
	colr <<=1;
	cntr8--;	
	}
TGLOBISR::enable ();
//__HAL_FLASH_INSTRUCTION_CACHE_ENABLE ();
}



void TLEDS::update (TLED *ld)
{
//__HAL_FLASH_INSTRUCTION_CACHE_DISABLE ();	
		//TGLOBISR::disable ();
		Tx8bit (ld->RGB_dat[1]);
		//TGLOBISR::enable ();
		//S_NOP();
		//TGLOBISR::disable ();
		Tx8bit (ld->RGB_dat[0]);
		//TGLOBISR::enable ();
		//S_NOP();
		//TGLOBISR::disable ();
		Tx8bit (ld->RGB_dat[2]);
		//TGLOBISR::enable ();
//__HAL_FLASH_INSTRUCTION_CACHE_ENABLE ();
}







void TLEDS::update ()
{
	uint8_t ix = 0;
	//__HAL_FLASH_INSTRUCTION_CACHE_DISABLE ();	
	//TGLOBISR::disable ();
	while (ix < c_ar_cnt)
		{
		update (array[reixmx[ix]]);
		ix++;
		}
	//TGLOBISR::enable ();
	//__HAL_FLASH_INSTRUCTION_CACHE_ENABLE ();
}


/*
bool TLEDS::update_isr (TLED *ld)
{
static uint8_t ix = 0;
	bool rv = false;
		
__HAL_FLASH_INSTRUCTION_CACHE_DISABLE ();	
		switch (ix)
			{
			case 0:
				Tx8bit (ld->RGB_dat[1]);
				ix++;
				break;
			case 1:
				Tx8bit (ld->RGB_dat[0]);
				ix++;
				break;
			case 2:
				Tx8bit (ld->RGB_dat[2]);
				ix = 0;
				rv = true;
				break;
			}
__HAL_FLASH_INSTRUCTION_CACHE_ENABLE ();
		return rv;
}



void TLEDS::isr_task (bool f_rst)
{
	switch (sw_isr)
		{
		case ELEDSWISR_SYNC:
			{
			if (f_rst && f_need_update) 
				{
				ix_isr = 0;
				sw_isr = ELEDSWISR_LEDS;
				}
			break;
			}
		case ELEDSWISR_LEDS:
			{
			if (update_isr (array[ix_isr])) ix_isr++;
			if (ix_isr >= c_ar_cnt)  {
				f_need_update = false;
				relax_time = C_LEDSTROBE_FRAME_TIME;
				sw_isr = ELEDSWISR_WAIT;
				ix_isr = 0;
				break;
				}
			break;
			}
		case ELEDSWISR_WAIT:
			{
			if (!relax_time) sw_isr = ELEDSWISR_SYNC;
			break;
			}
		default: break;
		}
}
*/



void TLEDS::all_color (uint8_t r, uint8_t g, uint8_t b)
{
	uint8_t ix = 0;
	while (ix < c_ar_cnt)
		{
		array[reixmx[ix]]->color_rgb (r,g,b);
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
		array[reixmx[ix]]->set_bright (val);
		ix++;
		}
	f_need_update = true;
}




TLED *TLEDS::l10 (uint8_t ix)
{
	uint16_t ixline = ix * 4;
	TLED *rv = array[0];
	if (ixline < c_ar_cnt) rv = array[reixmx[ixline]];
	return rv;
}




void TLEDS::bright (uint8_t ix, float val)
{
	if (ix < c_ar_cnt) 
		{
		array[reixmx[ix]]->set_bright (val);
		f_need_update = true;
		}
}



void TLEDS::color (uint8_t ix, const s_rgb_t &c)
{
	if (ix < c_ar_cnt)  {
		array[reixmx[ix]]->color_rgb (const_cast<s_rgb_t&>(c));
		f_need_update = true;
		}
}




void TLEDS::color (uint8_t ix, uint8_t r, uint8_t g, uint8_t b)
{
	if (ix < c_ar_cnt) {
		array[reixmx[ix]]->color_rgb (r,g,b);
		f_need_update = true;
		}
}



void TLEDS::sync_after_isr ()
{
	f_isr_sync = true;
}



void TLEDS::Task ()
{
	if (!relax_time) {
		if (f_need_update && f_isr_sync) {
			update ();
			f_need_update = false;
			f_isr_sync = false;
			}
		relax_time = C_LEDSTROBE_FRAME_TIME;
		}

}


