#include "TDRAWPARAM.h"



TGRAPHPARAM::TGRAPHPARAM (TPARAMCB *cb, MAV_PARAM_TYPE pt, uint32_t pix, uint8_t c)
{
	data.txt_post = 0;
	data.txt_pre = 0;
	data.cb = cb;
	data.cb_p_ix = pix;
	if (!c) c = 1;
	data.type = pt;
	data.u.i32 = 0;
	data.cnt = c;
	data.txt = new uint8_t[(c + 1)];
	data.str = new TSTMSTRING (data.txt, c + 1);
}



TGRAPHPARAM::TGRAPHPARAM (float *v, uint8_t c, uint8_t dfn)
{
	data.txt_post = 0;
	data.txt_pre = 0;
	data.cb = 0;
	if (!c) c = 1;
	data.type = MAV_PARAM_TYPE_REAL32;
	data.u.fl = v;
	data.dign_f = dfn;
	data.cnt = c;
	data.txt = new uint8_t[(c + 1)];
	data.str = new TSTMSTRING (data.txt, c + 1);
}



TGRAPHPARAM::TGRAPHPARAM (uint32_t *v, uint8_t c)
{
	data.txt_post = 0;
	data.txt_pre = 0;
	data.cb = 0;
	if (!c) c = 1;
	data.type = MAV_PARAM_TYPE_UINT32;
	data.u.u32 = v;
	data.cnt = c;
	data.txt = new uint8_t[(c + 1)];
	data.str = new TSTMSTRING (data.txt, c + 1);
}



TGRAPHPARAM::TGRAPHPARAM (int32_t *v, uint8_t c)
{
	data.txt_post = 0;
	data.txt_pre = 0;
	data.cb = 0;
	if (!c) c = 1;
	data.type = MAV_PARAM_TYPE_INT32;
	data.u.i32 = v;
	data.cnt = c;
	data.txt = new uint8_t[(c + 1)];
	data.str = new TSTMSTRING (data.txt, c + 1);
}



TGRAPHPARAM::TGRAPHPARAM (uint16_t *v, uint8_t c)
{
	data.txt_post = 0;
	data.txt_pre = 0;
	data.cb = 0;
	if (!c) c = 1;
	data.type = MAV_PARAM_TYPE_UINT16;
	data.u.u16 = v;
	data.cnt = c;
	data.txt = new uint8_t[(c + 1)];
	data.str = new TSTMSTRING (data.txt, c + 1);
}



TGRAPHPARAM::TGRAPHPARAM (int16_t *v, uint8_t c)
{
	data.txt_post = 0;
	data.txt_pre = 0;
	data.cb = 0;
	if (!c) c = 1;
	data.type = MAV_PARAM_TYPE_INT16;
	data.u.i16 = v;
	data.cnt = c;
	data.txt = new uint8_t[(c + 1)];
	data.str = new TSTMSTRING (data.txt, c + 1);
}



TGRAPHPARAM::TGRAPHPARAM (uint8_t *v, uint8_t c)
{
	data.txt_post = 0;
	data.txt_pre = 0;
	data.cb = 0;
	if (!c) c = 1;
	data.type = MAV_PARAM_TYPE_UINT8;
	data.u.u8 = v;
	data.cnt = c;
	data.txt = new uint8_t[(c + 1)];
	data.str = new TSTMSTRING (data.txt, c + 1);
}



TGRAPHPARAM::TGRAPHPARAM (int8_t *v, uint8_t c)
{
	data.txt_post = 0;
	data.txt_pre = 0;
	data.cb = 0;
	if (!c) c = 1;
	data.type = MAV_PARAM_TYPE_INT8;
	data.u.i8 = v;
	data.cnt = c;
	data.txt = new uint8_t[(c + 1)];
	data.str = new TSTMSTRING (data.txt, c + 1);
}


TGRAPHPARAM::TGRAPHPARAM (bool *v, uint8_t c, const char *t_on, const char *t_off)
{
	data.txt_post = 0;
	data.txt_pre = 0;
	data.cb = 0;
	data.txt_bool_on = (const_cast <char*>(t_on));
	data.txt_bool_off = (const_cast <char*>(t_off));
	if (!c) c = 1;
	data.type = MAV_PARAM_TYPE_BOOL;
	data.u.bl = v;
	data.cnt = c;
	data.txt = new uint8_t[(c + 1)];
	data.str = new TSTMSTRING (data.txt, c + 1);
}



void TGRAPHPARAM::set_prephix (const char *txt)
{
	data.txt_pre = (uint8_t*)txt;
}



void TGRAPHPARAM::set_postfix (const char *txt)
{
	data.txt_post = (uint8_t*)txt;
}



const TSTMSTRING &TGRAPHPARAM::get_txt ()
{
	update ();
	return *data.str;
}



TSTMSTRING *TGRAPHPARAM::get_lp_txt ()
{
	update ();
	return data.str;
}



void TGRAPHPARAM::update ()
{
	*data.str = "" ;
	*data.str += data.txt_pre;
	switch (data.type)
		{
		case MAV_PARAM_TYPE_BOOL:
			{
			if (data.u.bl) {
				if (*data.u.bl)
					{
					if (data.txt_bool_on) *data.str += data.txt_bool_on;
					}
				else
					{
					if (data.txt_bool_off) *data.str += data.txt_bool_off;
					}
				}
			break;
			}
		case MAV_PARAM_TYPE_UINT8:
			{
			*data.str += data.u.u8;
			break;
			}
		case MAV_PARAM_TYPE_UINT16:
			{
			*data.str += *data.u.u16;
			break;
			}
		case MAV_PARAM_TYPE_UINT32:
			{
			if (data.cb)
				{
				*data.str += (data.cb->get_param_u32 (data.cb_p_ix));
				}
			else
				{
				*data.str += *data.u.u32;
				}
			break;
			}
		case MAV_PARAM_TYPE_INT8:
			{
			*data.str += *data.u.i8;
			break;
			}
		case MAV_PARAM_TYPE_INT16:
			{
			*data.str += *data.u.i16;
			break;
			}
		case MAV_PARAM_TYPE_INT32:
			{
			if (data.cb)
				{
				*data.str += (data.cb->get_param_i32 (data.cb_p_ix));
				}
			else
				{
				*data.str += *data.u.i32;
				}
			break;
			}
		case MAV_PARAM_TYPE_REAL32:
			{
			if (data.cb)
				{
				data.str->Insert_Float (data.cb->get_param_f (data.cb_p_ix), data.dign_f);
				}
			else
				{
				data.str->Insert_Float (*data.u.fl, data.dign_f);
				}
			break;
			}
		}
	
	*data.str += data.txt_post;
}


