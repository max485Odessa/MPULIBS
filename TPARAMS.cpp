#include "TPARAMS.h"

static S_CONTROL_FLOAT_T param_press_profile = {EPARAMTYPE_FLOAT, "profile", 1, 6, 3};
static S_CONTROL_FLOAT_T param_press_on_a = {EPARAMTYPE_FLOAT, "presure_a.on", 1, 6, 3};
static S_CONTROL_FLOAT_T param_press_on_b = {EPARAMTYPE_FLOAT, "presure_b.on", 1, 6, 3};
static S_CONTROL_FLOAT_T param_press_off_a = {EPARAMTYPE_FLOAT, "presure_a.off", 1, 6, 3};
static S_CONTROL_FLOAT_T param_press_off_b = {EPARAMTYPE_FLOAT, "presure_b.off", 1, 6, 3};
static S_CONTROL_UINT32_T param_time_relax = {EPARAMTYPE_U32, "time.relax", 1000, 1000, 1000};
static S_CONTROL_UINT32_T param_station_mode = {EPARAMTYPE_U32, "station.mode", 2, 3, 2};		// 2 - auto/manual
static S_CONTROL_FLOAT_T param_press_clbr_zero = {EPARAMTYPE_FLOAT, "pres.calibr.zero", 1, 6, 3};
static S_CONTROL_FLOAT_T param_press_clbr_max = {EPARAMTYPE_FLOAT, "pres.calibr.max", 1, 6, 3};
static S_CONTROL_FLOAT_T param_press_bar = {EPARAMTYPE_FLOAT, "pres.bar", 1, 6, 3};

S_HDRPARAM_T *IRFPARAMS::list[EPRMIX_ENDENUM] = {(S_HDRPARAM_T*)&param_press_profile, (S_HDRPARAM_T*)&param_press_on_a, (S_HDRPARAM_T*)&param_press_off_a, \
(S_HDRPARAM_T*)&param_press_on_b, (S_HDRPARAM_T*)&param_press_off_b, (S_HDRPARAM_T*)&param_time_relax, (S_HDRPARAM_T*)&param_station_mode, \
(S_HDRPARAM_T*)&param_press_clbr_zero, (S_HDRPARAM_T*)&param_press_clbr_max, (S_HDRPARAM_T*)&param_press_bar,\
};

IRFPARAMS *IRFPARAMS::singlobj = 0;

IRFPARAMS *IRFPARAMS::obj ()
{
	if (!singlobj) singlobj = new IRFPARAMS ();
	return singlobj;
}



EPARAMTYPE IRFPARAMS::gettype (EPRMIX ix)
{
	EPARAMTYPE rv = EPARAMTYPE_ENDENUM;
	if (ix < EPRMIX_ENDENUM)
		{
		S_HDRPARAM_T *hdr = (S_HDRPARAM_T*)list[ix];
		rv = hdr->type;
		}
	return rv;
}



void IRFPARAMS::correct_param (EPRMIX ix)
{
if (ix < EPRMIX_ENDENUM)
	{
	S_HDRPARAM_T *hdr = (S_HDRPARAM_T*)list[ix];
	switch (hdr->type)
		{
		case EPARAMTYPE_FLOAT:
			{
			S_CONTROL_FLOAT_T *dt = (S_CONTROL_FLOAT_T*)hdr;
			if (data[ix].type == EPARAMTYPE_FLOAT)
				{
				float val = data[ix].data.u.v_f;
				
				if (val > dt->max) val = dt->max;
				if (val < dt->min) val = dt->min;
				data[ix].data.u.v_f = val;
				}
			else
				{
				data[ix].type = EPARAMTYPE_FLOAT;
				data[ix].data.u.v_f = dt->def;
				}
			break;
			}
		case EPARAMTYPE_U32:
			{
			S_CONTROL_UINT32_T *dt = (S_CONTROL_UINT32_T*)hdr;
			if (data[ix].type == EPARAMTYPE_U32)
				{
				float val = data[ix].data.u.v_u32;
				if (val > dt->max) val = dt->max;
				if (val < dt->min) val = dt->min;
				data[ix].data.u.v_u32 = val;
				}
			else
				{
				data[ix].type = EPARAMTYPE_U32;
				data[ix].data.u.v_u32 = dt->def;
				}
			break;
			}
		default: break;
		}
	}
}



void IRFPARAMS::load ()
{
	// физическая загрузка с места хранения
	// проверка и воостановление
	correct_all ();
}



void IRFPARAMS::save ()
{
}



void IRFPARAMS::correct_all ()
{
	uint32_t ix = 0;
	while (ix < EPRMIX_ENDENUM)
		{
		correct_param ((EPRMIX)ix);
		ix++;
		}
}



void IRFPARAMS::param_todef (EPRMIX ix)
{
if (ix < EPRMIX_ENDENUM)
	{
	S_HDRPARAM_T *hdr = (S_HDRPARAM_T*)list[ix];
	switch (hdr->type)
		{
		case EPARAMTYPE_FLOAT:
			{
			S_CONTROL_FLOAT_T *dt = (S_CONTROL_FLOAT_T*)hdr;
			data[ix].type = EPARAMTYPE_FLOAT;
			data[ix].data.u.v_f = dt->def;
			break;
			}
		case EPARAMTYPE_U32:
			{
			S_CONTROL_UINT32_T *dt = (S_CONTROL_UINT32_T*)hdr;
			data[ix].type = EPARAMTYPE_U32;
			data[ix].data.u.v_u32 = dt->def;
			break;
			}
		default: break;
		}
	}
}



void IRFPARAMS::clear_data_todef ()
{
	uint32_t ix = 0;
	while (ix < EPRMIX_ENDENUM)
		{
		param_todef ((EPRMIX)ix);
		ix++;
		}
}



long IRFPARAMS::get_papam_i32 (EPRMIX ix)
{
	long rv = 0;
	get_papam_i32 (ix, rv);
	return rv;
}


bool IRFPARAMS::get_papam_i32 (EPRMIX ix, long &dst)
{
	bool rv = false;
	if (ix < EPRMIX_ENDENUM)
		{
		if (data[ix].type == EPARAMTYPE_I32)
			{
			dst = data[ix].data.u.v_i32;
			rv = true;
			}
		}
	return rv;
}



bool IRFPARAMS::get_papam_u32 (EPRMIX ix, uint32_t &dst)
{
	bool rv = false;
	if (ix < EPRMIX_ENDENUM)
		{
		if (data[ix].type == EPARAMTYPE_U32)
			{
			dst = data[ix].data.u.v_u32;
			rv = true;
			}
		}
	return rv;
}



uint32_t IRFPARAMS::get_papam_u32 (EPRMIX ix)
{
	uint32_t rv = 0;
	get_papam_u32 (ix, rv);
	return rv;
}



float IRFPARAMS::get_papam_f (EPRMIX ix)
{
	float rv = 0;
	get_papam_f (ix, rv);
	return rv;
}



bool IRFPARAMS::get_papam_f (EPRMIX ix, float &dst)
{
	bool rv = false;
	if (ix < EPRMIX_ENDENUM)
		{
		if (data[ix].type == EPARAMTYPE_FLOAT)
			{
			dst = data[ix].data.u.v_f;
			rv = true;
			}
		}
	return rv;
}



bool IRFPARAMS::get_papam_b (EPRMIX ix)
{
	bool rv = false;
	get_papam_b (ix, rv);
	return rv;
}



bool IRFPARAMS::get_papam_b (EPRMIX ix, bool &dst)
{
	bool rv = false;
	if (ix < EPRMIX_ENDENUM)
		{
		if (data[ix].type == EPARAMTYPE_BOOL)
			{
			dst = data[ix].data.u.v_b;
			rv = true;
			}
		}
	return rv;
}



void IRFPARAMS::set_papam_b (EPRMIX ix, bool prm)
{
	if (ix < EPRMIX_ENDENUM)
		{
		if (data[ix].type == EPARAMTYPE_BOOL)
			{
			data[ix].data.u.v_b = prm;
			}
		}
}



void IRFPARAMS::set_papam_u32 (EPRMIX ix, uint32_t prm)
{
	if (ix < EPRMIX_ENDENUM)
		{
		if (data[ix].type == EPARAMTYPE_U32)
			{
			data[ix].data.u.v_u32 = prm;
			}
		}
}



void IRFPARAMS::set_papam_i32 (EPRMIX ix, long prm)
{
	if (ix < EPRMIX_ENDENUM)
		{
		if (data[ix].type == EPARAMTYPE_I32)
			{
			data[ix].data.u.v_i32 = prm;
			}
		}
}



void IRFPARAMS::set_papam_f (EPRMIX ix, float prm)
{
	if (ix < EPRMIX_ENDENUM)
		{
		if (data[ix].type == EPARAMTYPE_FLOAT)
			{
			data[ix].data.u.v_f = prm;
			}
		}
}


