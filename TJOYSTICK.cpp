#include "TJOYSTICK.h"
#include "rutine.h"



TJOYSTIC::TJOYSTIC (S_GPIOPIN *p, const uint8_t pc, const float *x, const float *y) : volt_x (x), volt_y (y), c_pins_cnt (pc)
{
	uint8_t ix = 0;
	pins = new S_KEYSETS_T[c_pins_cnt];
	last_pushed_mask = 0;
	axiscalibr[EJSTCA_X].f_result_ok = false;
	axiscalibr[EJSTCA_Y].f_result_ok = false;
	c_volt_move_quant = 0.03F;		// величина напряжения для устранения джитера центрального положения стика
	while (ix < EJSTCPINS_ENDENUM) 
		{
		pins[ix].keypin = p[ix];
		HAL_GPIO_DeInit(p[ix].port, p[ix].pin);
		_pin_low_init_in_pullup (&pins[ix].keypin, 1, true);
		clear_key (pins[ix]);
		ix++;
		}
	gmsg_ix = 0;
	last_ticks = SYSBIOS::GetTickCountLong ();
	relax_jstc_tim.set (1000);		// задержка работы джойстика после старта системы
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



void TJOYSTIC::set_calibrate_axis_band (EJSTCA ax, S_JOYCALIBR_T *v)
{
	if (ax < EJSTCA_ENDENUM)  {
		axiscalibr[ax].calibset = *v;
		axiscalibr[ax].mult_band_plus = 1.0F / (axiscalibr[ax].calibset.max_value - axiscalibr[ax].calibset.midl_value);	// max - midl
		axiscalibr[ax].mult_band_minus = 1.0F / (axiscalibr[ax].calibset.midl_value - axiscalibr[ax].calibset.min_value);	// midl - min
		}
}



bool TJOYSTIC::get_axis (float &val_x, float &val_y)
{
	bool rv = false;
	if (axiscalibr[EJSTCA_X].f_result_ok && axiscalibr[EJSTCA_Y].f_result_ok)
		{
		val_x = axiscalibr[EJSTCA_X].cur_axis_value;
		val_y = axiscalibr[EJSTCA_Y].cur_axis_value;
		rv = true;
		}
	return rv;
}



void TJOYSTIC::update_push_state_all ()
{
	uint8_t ix = 0;
	bool stt;
	while (ix < EJSTCPINS_ENDENUM)  
		{
		stt = update_push_state (pins[ix]);
		if (stt)
			{
			last_pushed_mask |= (1UL << ix);
			}
		else
			{
			last_pushed_mask &= (0xFFFFFFFFUL ^ (1UL << ix));
			}
		ix++;
		}
}



bool TJOYSTIC::update_push_state (S_KEYSETS_T &ps)
{
	bool rv;
	ps.pushstate_cur = !_pin_get(&ps.keypin);
	rv = ps.pushstate_cur;
	return rv;
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
			kn = (EJSTCPINS)gmsg_ix;
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



void TJOYSTIC::joyst_subtask ()
{
	if (!relax_jstc_tim.get())
		{
		float axis_volts[EJSTCA_ENDENUM];
		//float axis_delt_v[EJSTCA_ENDENUM];
		float tmpf;
		axis_volts[EJSTCA_X] = *volt_x;
		axis_volts[EJSTCA_Y] = *volt_y;

		
		uint8_t ix = EJSTCA_X;
		while (ix < EJSTCA_ENDENUM)
			{
			if (axis_volts[ix] > axiscalibr[ix].calibset.max_value) axis_volts[ix] = axiscalibr[ix].calibset.max_value;
			if (axis_volts[ix] < axiscalibr[ix].calibset.min_value) axis_volts[ix] = axiscalibr[ix].calibset.min_value;
			tmpf = axis_volts[ix] - axiscalibr[ix].calibset.midl_value;
			if (tmpf >= 0)
				{
				if (tmpf < c_volt_move_quant) 
					{
					tmpf = 0;
					}
				else
					{
					tmpf *= axiscalibr[ix].mult_band_plus;
					if (tmpf > 1.0F) tmpf = 1.0F;
					}
				}
			else
				{
				if (tmpf > -c_volt_move_quant) 
					{
					tmpf = 0;
					}
				else
					{
					tmpf *= axiscalibr[ix].mult_band_minus;
					if (tmpf < -1.0F) tmpf = -1.0F;
					}
				}
			axiscalibr[ix].cur_axis_value = tmpf;
			axiscalibr[ix].f_result_ok = true;
			ix++;
			}
		relax_jstc_tim.set (5);		// 20 ms
		}
		
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
					if (pins[ix].pop_time && pins[ix].pop_time < 200) messg = EJSTMSG_DBLCLICK;
					pins[ix].pop_time = 0;
					pins[ix].push_time = 1;
					}
				else
					{
					messg = EJSTMSG_CLICK;
					pins[ix].last_push_time = pins[ix].push_time;
					pins[ix].push_time = 0;
					pins[ix].pop_time = 1;
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
				pins[ix].pushstate_prev = pins[ix].pushstate_cur;
				}
			else
				{
				// push/pop timers update
				if (pins[ix].pushstate_cur)
					{
					addval_u32 (pins[ix].push_time, c_dlt);
					}
				else
					{
					addval_u32 (pins[ix].pop_time, c_dlt);
					}
				}

			ix++;
			}
		last_ticks = curticks;
		}
		
	joyst_subtask ();
}



uint32_t TJOYSTIC::get_pushed_mask ()
{
	return last_pushed_mask;
}


