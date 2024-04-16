#ifndef _h_rfcmd_layer_h_
#define _h_rfcmd_layer_h_



#include "tftaskif.h"
#include "TCOMIFS.h"
#include "SYSBIOS.H"
#include "rutine.h"
#include "rfcmddefine.h"

enum ERFMODE {ERFMODE_SLEEP = 0, ERFMODE_STANDBY = 1, ERFMODE_SYNT = 2, ERFMODE_RX = 3, ERFMODE_TX = 4};
enum ETRSSTAT {ETRS_SUCCESS = 0, ETRS_TIMEOUT = 1, ETRS_ERROR = 2};
enum EEVENTRSLT {EEVENTRSLT_OK = 0, EEVENTRSLT_ERROR = 1, EEVENTRSLT_TIMEOUT = 2, EEVENTRSLT_ENDENUM = 3};

class IFEVENT {
	public:
		virtual void event_cb (uint32_t evcode, EEVENTRSLT ecod);
};



class TCBEVENT {
		IFEVENT *evlp;
		uint32_t evcode;
	public:
		TCBEVENT (uint32_t evcode, IFEVENT *evcb);
		void execute (EEVENTRSLT rslt);
};



// от этого инфтерфейса должен наследоваться обьект анализирующий входной трафик
class IFCRFRX {
	public:
		virtual void RF_recv_cb (uint8_t *data, uint16_t sz, uint16_t rssi) = 0;
		virtual void RF_txend_cb (bool f_ok) = 0;
};


// интерфейс методов передачи
class IFCRFTX {
	protected:
		IFCRFRX *eventsobj_cb;
		
	public:
		void setrx_cb (IFCRFRX *obj);
	
		virtual bool GetIndicatorRx () = 0;
		virtual bool GetIndicatorTx () = 0;
		virtual bool send (uint8_t *src, uint16_t sz) = 0;
};



class TRFBASECMD {
	protected:
		uint8_t txbufer[2048];
		uint8_t *rxbufer;
		const uint32_t c_rxbuf_size;
		IFCRFTX *txobj;
		bool f_tx_state;
		local_rf_id_t self_id;
		S_ABONENT_ITEM_T *abitm;
		const uint16_t c_abon_max;
		uint16_t abon_list_amount;
		long abnt_find (const S_RFSERIAL_T *sn);
		long abnt_find (local_rf_id_t snl);
		void rawsend (void *src, uint16_t sz, uint16_t timout);
		utimer_t tx_timeout;
		TRFBASECMD (IFCRFTX *objc, uint16_t abcnt, uint32_t rxalcsz);
	
	public:
		bool is_free ();
		void set_selfid (local_rf_id_t sid);
		bool add_abonent (S_ABONENT_ITEM_T *abnt);
		void clear_abonent_list ();

};






class IUSERRFCB {
	public:
		virtual bool user_call_event_req_cb (local_rf_id_t dvid, uint32_t event_code, uint32_t ev_time) = 0;
		virtual bool user_get_event_req_cb (local_rf_id_t dvid, uint16_t ix, S_EVENT_ITEM_T *evnt) = 0;
		virtual bool user_get_param_req_cb (local_rf_id_t dvid, uint16_t ix, const S_PRMF_CAPTION_T *name, S_RFPARAMVALUE_T &src) = 0;	// пользователь должен изьять параметр и передать его по ссылке
		virtual bool user_set_param_req_cb (local_rf_id_t dvid, uint16_t ix, const S_PRMF_CAPTION_T *name, S_RFPARAMVALUE_T *src) = 0;
		virtual bool user_get_state_req_cb (local_rf_id_t dvid, S_DEVSTATE_T *src) = 0;
};



class TRFSLAVE: protected TRFBASECMD, public TFFC, public IFCRFRX {
		virtual void Task ();
		virtual void RF_recv_cb (uint8_t *data, uint16_t sz, uint16_t rssi);
		virtual void RF_txend_cb (bool f_ok);
		IUSERRFCB *user_cb;
	protected:
		//virtual bool user_call_event_req_cb (local_rf_id_t dvid, uint32_t event_code, uint32_t ev_time);
		//virtual bool user_get_event_req_cb (local_rf_id_t dvid, uint16_t ix, S_EVENT_ITEM_T *evnt);
		//virtual bool user_get_param_req_cb (local_rf_id_t dvid, uint16_t ix, const S_PARAM_CAPTION_T *name, S_RFPARAMVALUE_T &src);	// пользователь должен изьять параметр и передать его по ссылке
		//virtual bool user_set_param_req_cb (local_rf_id_t dvid, uint16_t ix, const S_PARAM_CAPTION_T *name, S_RFPARAMVALUE_T *src);
		//virtual bool user_get_state_req_cb (local_rf_id_t dvid, S_DEVSTATE_T *src) = 0;	
	public:
		TRFSLAVE (IFCRFTX *objc, uint16_t abcnt, uint32_t rxalcsz, IUSERRFCB *us);

};


#endif
