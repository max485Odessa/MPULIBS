#include "tevents.h"



TEVENT::TEVENT ()
{
	f_event_progress = false;
	obj_cb = 0;
}



void TEVENT::call_bc (EEVSTATUS st)
{
	if (obj_cb && f_event_progress) obj_cb->events_cb (ev_code, st, ev_param);
	f_event_progress = false;
}



TEVENTCREATE::TEVENTCREATE ()
{
}


		
void TEVENTCREATE::setevent_cb (IEVCB *obj)
{
	obj_cb = obj;
}



void TEVENTCREATE::setevent_code (uint32_t ev_cd)
{
	ev_code = ev_cd;
}



void TEVENTCREATE::setevent_param (uint32_t val)
{
	ev_param = val;
}



void TEVENTCREATE::activate ()
{
	f_event_progress = true;
}



bool TEVENTCREATE::is_progress ()
{
	return f_event_progress;
}


