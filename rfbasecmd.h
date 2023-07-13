#ifndef _h_rfcmd_layer_h_
#define _h_rfcmd_layer_h_



#include "tftaskif.h"
//#include "RFM69.h"
#include "SYSBIOS.H"
#include "rutine.h"
#include "rfcmddefine.h"

enum ERFMODE {ERFMODE_SLEEP = 0, ERFMODE_STANDBY = 1, ERFMODE_SYNT = 2, ERFMODE_RX = 3, ERFMODE_TX = 4};
enum ETRSSTAT {ETRS_SUCCESS = 0, ETRS_TIMEOUT = 1, ETRS_ERROR = 2};



class IFCRFRX {
	public:
		virtual void RF_recv_cb (uint8_t *data, uint16_t sz, uint16_t rssi) = 0;
		virtual void RF_txend_cb (bool f_ok) = 0;
};

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



class TRFMASTER: protected TRFBASECMD, public TFFC, public IFCRFRX {
		virtual void Task ();
		virtual void RF_recv_cb (uint8_t *data, uint16_t sz, uint16_t rssi);
		virtual void RF_txend_cb (bool f_ok);
	protected:
		virtual void get_param_resp_cb (local_rf_id_t svid, uint16_t ix, const S_PARAM_CAPTION_T *name, S_RFPARAMVALUE_T &src, ERESPSTATE rx_state) = 0;
		virtual void set_param_resp_cb (local_rf_id_t svid, uint16_t ix, const S_PARAM_CAPTION_T *name, S_RFPARAMVALUE_T &src, ERESPSTATE rx_state) = 0;
		virtual void get_state_resp_cb (local_rf_id_t svid, S_DEVSTATE_T *src, ERESPSTATE rx_state) = 0;
		virtual void get_event_resp_cb (local_rf_id_t svid, uint16_t ix, S_EVENT_ITEM_T *evnt, ERESPSTATE rx_state) = 0;
		virtual void call_event_resp_cb (local_rf_id_t svid, uint32_t event_code, ERESPSTATE rx_state) = 0;
	
	public:
		TRFMASTER (IFCRFTX *objc, uint16_t abcnt, uint32_t rxalcsz);
		void get_param_req (local_rf_id_t dvid, uint16_t ix, uint32_t tmot);
		void get_param_req (local_rf_id_t dvid, char *name, uint32_t tmot);
		void set_param_req (local_rf_id_t dvid, uint16_t ix, S_RFPARAMVALUE_T prm, uint32_t tmot);
		void set_param_req (local_rf_id_t dvid, char *name, S_RFPARAMVALUE_T prm, uint32_t tmot);
		void get_state_req (local_rf_id_t dvid, uint32_t tmot);
		void get_event_req (local_rf_id_t dvid, uint16_t ix, uint32_t tmot);
		void call_event_req (local_rf_id_t dvid, uint32_t event_code, uint32_t calltime, uint32_t tmot );		// тип event_code знает gui обработчик
};



class TRFSLAVE: protected TRFBASECMD, public TFFC, public IFCRFRX {
		virtual void Task ();
		virtual void RF_recv_cb (uint8_t *data, uint16_t sz, uint16_t rssi);
		virtual void RF_txend_cb (bool f_ok);
	protected:
		virtual bool user_call_event_req_cb (local_rf_id_t dvid, uint32_t event_code, uint32_t ev_time) = 0;
		virtual bool user_get_event_req_cb (local_rf_id_t dvid, uint16_t ix, S_EVENT_ITEM_T *evnt) = 0;
		virtual bool user_get_param_req_cb (local_rf_id_t dvid, uint16_t ix, const S_PARAM_CAPTION_T *name, S_RFPARAMVALUE_T &src) = 0;	// пользователь должен изьять параметр и передать его по ссылке
		virtual bool user_set_param_req_cb (local_rf_id_t dvid, uint16_t ix, const S_PARAM_CAPTION_T *name, S_RFPARAMVALUE_T *src) = 0;
		virtual bool user_get_state_req_cb (local_rf_id_t dvid, S_DEVSTATE_T *src) = 0;	
	public:
		TRFSLAVE (IFCRFTX *objc, uint16_t abcnt, uint32_t rxalcsz);

};


#endif
