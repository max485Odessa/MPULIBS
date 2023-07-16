#include "TRFPUMP.h"



const char *TRFPUMP::evnt_txt[EPUMPEVENT_ENDENUM] = {"none", "pump on", "pump off"};



TRFPUMP::TRFPUMP (IPINS *sg, TPRESSURE *p)
{
	pins = sg;
	params = IRFPARAMS::obj ();
	workmode = (EPUMPMODE)params->get_papam_u32 (EPRMIX_MODE);
}



void TRFPUMP::copy_eventname_str (S_PARAM_CAPTION_T *dst, EPUMPEVENT ix)
{
	if (ix < EPUMPEVENT_ENDENUM)
		{
		char *intxt = (char*)evnt_txt[ix];
		uint32_t sz_name = lenstr (intxt);
		if (sz_name > sizeof(dst->txt)) sz_name = sizeof(dst->txt);
		CopySDC_Data ((void*)evnt_txt[ix], dst->txt, sz_name);
		dst->txt[sizeof(dst->txt)-1] = 0;
		}
}



bool TRFPUMP::user_call_event_req_cb (local_rf_id_t dvid, uint32_t event_code, uint32_t ev_time)
{
	bool rv = false;
	if (event_code < EPUMPEVENT_ENDENUM)		// event_code совпадает с индексацией
		{
		rv = true;
		}
	return rv;
}



bool TRFPUMP::user_get_event_req_cb (local_rf_id_t dvid, uint16_t ix, S_EVENT_ITEM_T *evnt)
{
	bool rv = false;
	if (ix < EPUMPEVENT_ENDENUM)
		{
		copy_eventname_str (&evnt->caption, (EPUMPEVENT)ix);
		evnt->event_code = ix;	// совпадает с индексом
		rv = true;
		}
	return rv;
}



bool TRFPUMP::user_get_param_req_cb (local_rf_id_t dvid, uint16_t ix, const S_PARAM_CAPTION_T *name, S_RFPARAMVALUE_T &src)
{
	
}



bool TRFPUMP::user_set_param_req_cb (local_rf_id_t dvid, uint16_t ix, const S_PARAM_CAPTION_T *name, S_RFPARAMVALUE_T *src)
{
}



bool TRFPUMP::user_get_state_req_cb (local_rf_id_t dvid, S_DEVSTATE_T *src)	
{
}



bool TRFPUMP::getpressure_on (float &dst)
{
	bool rv = false;
	float val;
	switch (params->get_papam_u32 (EPRMIX_PRESS_PROFILE))
		{
		case 0:
			{
			if (params->get_papam_f (EPRMIX_PRESS_ON_A, val))
				{
				dst = val;
				rv = true;
				}
			break;
			}
		case 1:
			{
			if (params->get_papam_f (EPRMIX_PRESS_ON_B, val))
				{
				dst = val;
				rv = true;
				}
			break;
			}
		}
	return rv;
}



bool TRFPUMP::getpressure_off (float &dst)
{
	bool rv = false;
	float val;
	switch (params->get_papam_u32 (EPRMIX_PRESS_PROFILE))
		{
		case 0:
			{
			if (params->get_papam_f (EPRMIX_PRESS_OFF_A, val))
				{
				dst = val;
				rv = true;
				}
			break;
			}
		case 1:
			{
			if (params->get_papam_f (EPRMIX_PRESS_OFF_B, val))
				{
				dst = val;
				rv = true;
				}
			break;
			}
		}
	return rv;
}



bool TRFPUMP::get_pressure_sensors (float &dst)
{
	return pressure->pressure (dst);
}



void TRFPUMP::motor_onoff (bool val)
{
	pins->Pin_to (ESPINS_RELAY, val);
	relax_timer.set (params->get_papam_u32 (EPRMIX_PRESS_OFF_B));
}	



void TRFPUMP::Task ()
{
	bool f_param_ok = false;
	float press_on;
	float press_off;
	float press_sensor;
	uint32_t time_tt;
	do	{
			if (!getpressure_on (press_on)) break;
			if (!getpressure_off (press_off)) break;
			
			if (!get_pressure_sensors (press_sensor)) break;
		
			f_param_ok = true;
			} while (false);
	
	if (relax_timer.get ())	return;
			
	switch (workmode)
		{
		case EPUMPMODE_AUTO:
			{
			if (!f_param_ok) break;
			if (press_sensor >= press_off)
				{
				motor_onoff (false);
				}
			else
				{
				if (press_sensor <= press_on)
					{
					motor_onoff (true);
					}
				}
			break;
			}
		case EPUMPMODE_AUTO_MANUAL:
			{
			break;
			}
		case EPUMPMODE_MANUAL:
			{
			break;
			}
		default: break;
		}
}


