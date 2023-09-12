#include "TJOYSTICK.h"
#include "rutine.h"



TJOYSTIC::TJOYSTIC (S_GPIOPIN *p, const uint8_t pc, const float *x, const float *y) : volt_x (x), volt_y (y), c_pins_cnt (pc)
{
	uint8_t ix = 0;
	pins = new S_KEYSETS_T[c_pins_cnt];
	while (ix < EJSTCPINS_ENDENUM) 
		{
		pins[ix].keypin = p[ix];
		_pin_low_init_in (&pins[ix].keypin, 1);
		clear_key (pins[ix]);
		ix++;
		}
	gmsg_ix = 0;
	last_ticks = SYSBIOS::GetTickCountLong ();
}



void TJOYSTIC::clear_key (S_KEYSETS_T &kp)
{
	kp.block_time = 0;
	kp.f_block_next_msg = false;
	kp.pop_time = 0;
	kp.pushstate_cur = false;
	kp.pushstate_prev = false;
	kp.push_time = 0;
	kp.messg = EJSTMSG_NONE;
}



void TJOYSTIC::update_push_state_all ()
{
	uint8_t ix = 0;
	while (ix < EJSTCPINS_ENDENUM)  update_push_state (pins[ix++]);
}



void TJOYSTIC::update_push_state (S_KEYSETS_T &ps)
{
	ps.pushstate_cur = _pin_get(&ps.keypin);
}



uint32_t TJOYSTIC::get_pushtime_cur (EJSTCPINS p)
{
	uint32_t rv = 0;
	if (p < EJSTCPINS_ENDENUM) rv = pins[p].push_time;
	return rv;
}



uint32_t TJOYSTIC::get_pushtime_last (EJSTCPINS p)
{
	uint32_t rv = 0;
	if (p < EJSTCPINS_ENDENUM) rv = pins[p].last_push_time;
	return rv;
}



EJSTMSG TJOYSTIC::get_message (EJSTCPINS &kn)
{
	EJSTMSG rv = EJSTMSG_NONE;
	uint8_t cntkeys = EJSTCPINS_ENDENUM;
	while (cntkeys)
		{
		if (gmsg_ix >= EJSTCPINS_ENDENUM) gmsg_ix = 0;
		if (pins[gmsg_ix].messg != EJSTMSG_NONE)
			{
			rv = pins[gmsg_ix].messg;
			pins[gmsg_ix].messg = EJSTMSG_NONE;
			break;
			}
		gmsg_ix++;
		cntkeys--;
		}
	return rv;
}



void TJOYSTIC::block_next_msg (EJSTCPINS p)
{
	if (p < EJSTCPINS_ENDENUM) pins[p].f_block_next_msg = true;
}



void TJOYSTIC::block_time (EJSTCPINS p, uint32_t tbl)
{
	if (p < EJSTCPINS_ENDENUM) pins[p].block_time = tbl;
}



float TJOYSTIC::axis_value (EJSTCA ax)
{
	float rv = 0;
	if (ax < EJSTCA_ENDENUM) rv = 0;
	return rv;
}



void TJOYSTIC::Task ()
{
	uint32_t curticks = SYSBIOS::GetTickCountLong (), c_dlt, dlt;
	c_dlt = curticks - last_ticks;
	if (c_dlt)
		{
		uint8_t ix = 0;
		dlt = c_dlt;
		update_push_state_all ();
		while (ix < EJSTCPINS_ENDENUM)
			{
			subval_u32 (pins[ix].block_time, c_dlt);
			if (pins[ix].pushstate_cur != pins[ix].pushstate_prev)
				{
				EJSTMSG messg = EJSTMSG_NONE;
				if (pins[ix].pushstate_cur)
					{
					if (pins[ix].pop_time < 200) messg = EJSTMSG_DBLCLICK;
					}
				else
					{
					messg = EJSTMSG_CLICK;
					pins[ix].last_push_time = pins[ix].push_time;
					}
				if (pins[ix].f_block_next_msg)
					{
					messg = EJSTMSG_NONE;
					pins[ix].f_block_next_msg = false;
					}
				else
					{
					if (pins[ix].block_time) messg = EJSTMSG_NONE;
					}
				pins[ix].messg = messg;
				pins[ix].pop_time = 0;
				pins[ix].push_time = 0;
				pins[ix].pushstate_prev = pins[ix].pushstate_cur;
				}
			else
				{
				// push/pop timers update
				if (pins[ix].pushstate_cur)
					{
					addval_u32 (pins[ix].push_time, c_dlt);
					pins[ix].pop_time = 0;
					}
				else
					{
					addval_u32 (pins[ix].pop_time, c_dlt);
					pins[ix].push_time = 0;
					}
				}

			ix++;
			}
		last_ticks = curticks;
		}
}


