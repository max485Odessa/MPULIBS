#include "T595LED8.h"


IDIG8::IDIG8 (uint8_t dgcnt)
{
	strdata = new uint8_t[dgcnt + 5];
	dig_cnt = dgcnt;
	str = new TSTMSTRING (strdata, dig_cnt);
	*str = "";
}



void IDIG8::draw_raw (uint8_t ix, uint8_t dat)
{
	if (ix < dig_cnt)
		{
		write_data (ix, dat);
		}
}


static const char maskled_09[10] = { \
	SG_A | SG_B | SG_C | SG_D | SG_E | SG_F, /* 0 */ \
	SG_B | SG_C , /* 1 */ \
	SG_A | SG_B | SG_G | SG_E | SG_D, /* 2 */ \
	SG_A | SG_B | SG_G | SG_C | SG_D, /* 3 */ \
	SG_F | SG_C | SG_G | SG_B, /* 4 */ \
	SG_A | SG_F | SG_G | SG_C | SG_D, /* 5 */ \
	SG_A | SG_F | SG_G | SG_E | SG_C | SG_D, /* 6 */ \
	SG_A | SG_B | SG_C , /* 7 */ \
	SG_A | SG_B | SG_C | SG_D | SG_E | SG_F | SG_G, /* 8 */ \
	SG_A | SG_B | SG_C | SG_D | SG_F | SG_G, /* 9 */ \
};

static const char maskled_ab[6] = { \
	SG_A | SG_B | SG_C  | SG_E | SG_F | SG_G, /* A */ \
	SG_C | SG_D | SG_E | SG_F | SG_G, /* B */ \
	SG_D | SG_E | SG_G, /* C */ \
	SG_B | SG_C | SG_D | SG_E | SG_G, /* D */ \
	SG_A | SG_D | SG_E | SG_F | SG_G, /* E */ \
	SG_A | SG_E | SG_F | SG_G, /* F */ \
};


void IDIG8::str_to_ledbuf (uint8_t ix_dst)
{
	char ix_src = 0, cnt_str = str->Length (), dat;
	while (ix_src < cnt_str && ix_dst < dig_cnt)
		{
		dat = strdata[ix_src];
		if (dat == '.')
			{
			if (ix_dst) write_data (ix_dst-1, read_data (ix_dst-1) | SG_P);
			ix_src++;
			}
		else
			{
			if (dat >= '0' && dat <= '9')
				{
				dat = maskled_09[dat - '0'];
				}
			else
				{
				if (dat >= 'a' && dat <= 'f')
					{
					dat = maskled_ab[dat - 'a'];
					}
				else
					{
					if (dat >= 'A' && dat <= 'B')
						{
						dat = maskled_ab[dat - 'A'];
						}
					else
						{
						dat = 0;
						}
					}
				}
			draw_raw (ix_dst, dat);
			ix_src++;
			ix_dst++;
			}
		}
}



void IDIG8::draw_raw (char *str)
{
}



void IDIG8::draw_long (long val)
{
}



void IDIG8::mark_big ()
{
uint8_t ix = 0;
while (ix < dig_cnt)
	{
	write_data (ix++, 1);
	ix++;
	}
}



typedef struct {
	long val;
	uint8_t cnt;
} s_strcel_t;

static const s_strcel_t ssarr[9] = {100000000, 9, 10000000, 8, 1000000, 7, 100000, 6, 10000, 5, 1000, 4, 100, 3, 10 , 2, 1, 1};

uint8_t IDIG8::get_celie_cnt (float val)
{
uint8_t ix = 0, rv = 0;
while (ix < 9)
	{
	if (ssarr[ix].val <= val)
		{
		rv = ssarr[ix].cnt;
		break;
		}
	ix++;
	}
return rv;
}



void IDIG8::draw_float (float val, uint8_t dr_cnt)
{
//TSTMSTRING s = *str;
uint8_t cel_cnt =  get_celie_cnt (val);
if (!cel_cnt) cel_cnt = 1;
uint8_t drob_curent_n = 0;
if (cel_cnt > dig_cnt)
	{
	mark_big ();	// целое число больше чем дисплей
	return;
	}
else
	{
	uint8_t full_d_sizes = 0;
	if (cel_cnt == dig_cnt)
		{
		drob_curent_n = 0;
		}
	else
		{
		uint8_t free_to_droba = dig_cnt - cel_cnt;
		if (free_to_droba >= dr_cnt) free_to_droba = dr_cnt;
		drob_curent_n = free_to_droba;
		}
	full_d_sizes += cel_cnt;
	full_d_sizes += drob_curent_n;
	
	uint8_t start_dst_ix = 0;
	if (full_d_sizes < dig_cnt) start_dst_ix = dig_cnt - full_d_sizes;
		
	*str = ""; str->Insert_Float (val, drob_curent_n);
	str_to_ledbuf (start_dst_ix);
	
	}
}



void IDIG8::set_align (EALIGNDIG a_md)
{
	align_mode = a_md;
}



TLED595::TLED595 (S_GPIOPIN *pns, uint8_t dgcnt) : IDIG8 (dgcnt)
{
	_pin_low_init_out_pp (pns, ELEDPIN_ENDENUM);
	rawdata = new uint8_t[dgcnt];
	pins = pns;
	update_timer = 0;
	regen_ix = 0;
	f_need_strobe = false;
	SYSBIOS::ADD_TIMER_SYS (&update_timer);
	SYSBIOS::AddPeriodicThread_ISR_A (this, (void*)regen_task_1ms_hard, 1);
}



void TLED595::write_data (uint8_t ix, uint8_t dat)
{
if (ix < dig_cnt) rawdata[dig_cnt - ix - 1] = dat;
}



uint8_t TLED595::read_data (uint8_t ix)
{
uint8_t rv = 0;
if (ix < dig_cnt) rv = rawdata[dig_cnt - ix - 1];
return rv;
}



void TLED595::clear_ledbuf ()
{
	char ix = 0;
	while (ix < dig_cnt)
		{
		rawdata[ix] = 0;
		ix++;
		}
}



void TLED595::regen_task_1ms_hard (void *obj)
{
	((TLED595*)obj)->regen_task_1ms ();
}



void TLED595::regen_task_1ms ()
{
	if (delay_ms) delay_ms--;
	if (!delay_ms) {
		delay_ms = 3;
		if (f_need_strobe) {
			strobe_rclk ();
			//regen_ix++;
			f_need_strobe = false;
			}
		}
}



uint8_t TLED595::dign_to_bit (uint8_t n)
{
	return 1 << n;
}



void TLED595::strobe_rclk ()
{
	pins[ELEDPIN_RCK].port->BSRR = pins[ELEDPIN_RCK].pin;	// set bit
	pins[ELEDPIN_RCK].port->BRR = pins[ELEDPIN_RCK].pin;	// clr bit
}



void TLED595::strobe_sclk ()
{
	pins[ELEDPIN_SCK].port->BSRR = pins[ELEDPIN_SCK].pin;	// set bit
	pins[ELEDPIN_SCK].port->BRR = pins[ELEDPIN_SCK].pin;	// clr bit	
}



void TLED595::send_byte (uint8_t data)
{
	char cnt = 8;
	while (cnt)
		{
		if (data & 128)
			{
			pins[ELEDPIN_DIO].port->BSRR = pins[ELEDPIN_DIO].pin;	// set bit
			}
		else
			{
			pins[ELEDPIN_DIO].port->BRR = pins[ELEDPIN_DIO].pin;	// clr bit	
			}
		data = data << 1;
		strobe_sclk ();
		cnt--;
		}
}



void TLED595::Task ()
{
if (!f_need_strobe)
	{
	send_byte (0xFF ^ rawdata[regen_ix]);
	send_byte (dign_to_bit(regen_ix));
	
	regen_ix++;
	if (regen_ix >= dig_cnt) regen_ix = 0;		// dig_cnt
	f_need_strobe = true;
	}
}


