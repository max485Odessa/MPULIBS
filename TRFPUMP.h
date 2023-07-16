#ifndef _h_pump_station_defines_h_
#define _h_pump_station_defines_h_



#include "rfbasecmd.h"
#include "TPARAMS.h"
#include "TFTASKIF.h"
#include "rutine.h"
#include "SYSBIOS.H"
#include "TSIGNALS.H"
#include "tpressure5vsensor.h"



enum EPUMPEVENT {EPUMPEVENT_NONE = 0, EPUMPEVENT_MANUAL_ON = 1, EPUMPEVENT_MANUAL_OFF = 2, EPUMPEVENT_ENDENUM = 3};
// 0 - off, 1 - auto, 2 - auto/manual, 3 - manual
enum EPUMPMODE {EPUMPMODE_NONE = 0, EPUMPMODE_AUTO = 1, EPUMPMODE_AUTO_MANUAL = 2, EPUMPMODE_MANUAL = 3, EPUMPMODE_ENDENUM = 4};

class TRFPUMP: public IUSERRFCB, public TFFC {
		static const char *evnt_txt[EPUMPEVENT_ENDENUM];
		void copy_eventname_str (S_PARAM_CAPTION_T *dst, EPUMPEVENT ix);

		SYSBIOS::Timer relax_timer;
		SYSBIOS::Timer manual_timer;
		virtual void Task ();
		IRFPARAMS *params;
		IPINS *pins;
		TPRESSURE *pressure;
	
		EPUMPMODE workmode;
		bool getpressure_on (float &dst);
		bool getpressure_off (float &dst);
		
		bool get_pressure_sensors (float &dst);
		void motor_onoff (bool val);
	
		virtual bool user_call_event_req_cb (local_rf_id_t dvid, uint32_t event_code, uint32_t ev_time);
		virtual bool user_get_event_req_cb (local_rf_id_t dvid, uint16_t ix, S_EVENT_ITEM_T *evnt);
		virtual bool user_get_param_req_cb (local_rf_id_t dvid, uint16_t ix, const S_PARAM_CAPTION_T *name, S_RFPARAMVALUE_T &src);	// пользователь должен изьять параметр и передать его по ссылке
		virtual bool user_set_param_req_cb (local_rf_id_t dvid, uint16_t ix, const S_PARAM_CAPTION_T *name, S_RFPARAMVALUE_T *src);
		virtual bool user_get_state_req_cb (local_rf_id_t dvid, S_DEVSTATE_T *src);	
	
	public:
		TRFPUMP (IPINS *sg, TPRESSURE *p);

};



#endif
