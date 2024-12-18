#include "TDRAWPARAM.h"



TGRAPHPARAM::TGRAPHPARAM (float *v, uint8_t c)
{
	data.txt_post = 0;
	data.txt_pre = 0;
	if (!c) c = 1;
	data.type = MAV_PARAM_TYPE_REAL32;
	data.u.fl = v;
	data.cnt = c;
	data.txt = new uint8_t[c];
	data.str = new TSTMSTRING (data.txt, c + 1);
}



TGRAPHPARAM::TGRAPHPARAM (uint32_t *v, uint8_t c)
{
	data.txt_post = 0;
	data.txt_pre = 0;
	if (!c) c = 1;
	data.type = MAV_PARAM_TYPE_UINT32;
	data.u.u32 = v;
	data.cnt = c;
	data.txt = new uint8_t[c];
	data.str = new TSTMSTRING (data.txt, c + 1);
}



TGRAPHPARAM::TGRAPHPARAM (int32_t *v, uint8_t c)
{
	data.txt_post = 0;
	data.txt_pre = 0;
	if (!c) c = 1;
	data.type = MAV_PARAM_TYPE_INT32;
	data.u.i32 = v;
	data.cnt = c;
	data.txt = new uint8_t[c];
	data.str = new TSTMSTRING (data.txt, c + 1);
}



TGRAPHPARAM::TGRAPHPARAM (uint16_t *v, uint8_t c)
{
	data.txt_post = 0;
	data.txt_pre = 0;
	if (!c) c = 1;
	data.type = MAV_PARAM_TYPE_UINT16;
	data.u.u16 = v;
	data.cnt = c;
	data.txt = new uint8_t[c];
	data.str = new TSTMSTRING (data.txt, c + 1);
}



TGRAPHPARAM::TGRAPHPARAM (int16_t *v, uint8_t c)
{
	data.txt_post = 0;
	data.txt_pre = 0;
	if (!c) c = 1;
	data.type = MAV_PARAM_TYPE_INT16;
	data.u.i16 = v;
	data.cnt = c;
	data.txt = new uint8_t[c];
	data.str = new TSTMSTRING (data.txt, c + 1);
}



TGRAPHPARAM::TGRAPHPARAM (uint8_t *v, uint8_t c)
{
	data.txt_post = 0;
	data.txt_pre = 0;
	if (!c) c = 1;
	data.type = MAV_PARAM_TYPE_UINT8;
	data.u.u8 = v;
	data.cnt = c;
	data.txt = new uint8_t[c];
	data.str = new TSTMSTRING (data.txt, c + 1);
}



TGRAPHPARAM::TGRAPHPARAM (int8_t *v, uint8_t c)
{
	data.txt_post = 0;
	data.txt_pre = 0;
	if (!c) c = 1;
	data.type = MAV_PARAM_TYPE_INT8;
	data.u.i8 = v;
	data.cnt = c;
	data.txt = new uint8_t[c];
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
	return *data.str;
}



TSTMSTRING *TGRAPHPARAM::get_lp_txt ()
{
	return data.str;
}



void TGRAPHPARAM::update ()
{
	*data.str = "" ;
	*data.str += data.txt_pre;
	switch (data.type)
		{
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
			*data.str += *data.u.u32;
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
			*data.str += *data.u.i32;
			break;
			}
		case MAV_PARAM_TYPE_REAL32:
			{
			data.str->Insert_Float (*data.u.fl, data.dign_f);
			break;
			}
		}
	
	*data.str += data.txt_post;
}


