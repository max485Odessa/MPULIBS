#ifndef _h_pump_station_defines_h_
#define _h_pump_station_defines_h_



#include "rfbasecmd.h"



class TRFPUMP: public TRFSLAVE {
	public:
		TRFPUMP ();
		virtual bool user_call_event (local_rf_id_t dvid, ERESPSTATE rslt, uint32_t event_code);
		virtual bool user_get_event (local_rf_id_t dvid, ERESPSTATE rslt, uint16_t ix, S_EVENT_ITEM_T *evnt);
		virtual bool user_get_param (local_rf_id_t dvid, ERESPSTATE rslt, uint16_t ix, const char *name, S_RFPARAMVALUE_T *src);
		virtual bool user_set_param (local_rf_id_t dvid, ERESPSTATE rslt, uint16_t ix, const char *name, S_RFPARAMVALUE_T *src);
		virtual bool user_get_state (local_rf_id_t dvid, ERESPSTATE rslt, S_DEVSTATE_T *src);
};



#endif
