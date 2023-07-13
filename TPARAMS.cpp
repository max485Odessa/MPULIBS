#include "TPARAMS.h"

static S_CONTROL_FLOAT_T param_press_profile = {EPARAMTYPE_FLOAT, "profile", 1, 6, 3};
static S_CONTROL_FLOAT_T param_press_on_a = {EPARAMTYPE_FLOAT, "presure_a.min", 1, 6, 3};
static S_CONTROL_FLOAT_T param_press_on_b = {EPARAMTYPE_FLOAT, "presure_b.min", 1, 6, 3};
static S_CONTROL_FLOAT_T param_press_off_a = {EPARAMTYPE_FLOAT, "presure_a.max", 1, 6, 3};
static S_CONTROL_FLOAT_T param_press_off_b = {EPARAMTYPE_FLOAT, "presure_b.max", 1, 6, 3};
static S_CONTROL_UINT32_T param_time_relax = {EPARAMTYPE_U32, "time.relax", 1000, 1000, 1000};
static S_CONTROL_UINT32_T param_station_en = {EPARAMTYPE_U32, "station.en", 0, 1, 1};

S_HDRPARAM_T *IRFPARAMS::list[EPRMIX_ENDENUM] = {(S_HDRPARAM_T*)&param_press_profile, (S_HDRPARAM_T*)&param_press_on_a, (S_HDRPARAM_T*)&param_press_off_a, \
(S_HDRPARAM_T*)&param_press_on_b, (S_HDRPARAM_T*)&param_press_off_b, (S_HDRPARAM_T*)&param_time_relax, (S_HDRPARAM_T*)&param_station_en, \
};



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



long IRFPARAMS::get_papam_l (EPRMIX ix)
{
}



void IRFPARAMS::set_papam_l (EPRMIX ix, long prm)
{
}



float IRFPARAMS::get_papam_f (EPRMIX ix)
{
}



void IRFPARAMS::set_papam_f (EPRMIX ix, float prm)
{
}



bool IRFPARAMS::get_papam_b (EPRMIX ix)
{
}



void IRFPARAMS::set_papam_b (EPRMIX ix, bool prm)
{
}

