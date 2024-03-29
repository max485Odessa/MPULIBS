#ifndef _h_rfcmd_implement_usart_h_
#define _h_rfcmd_implement_usart_h_


#include "tcomifs.h"
#include "rfcmddefine.h"
#include "TFTASKIF.h"
#include "TUSARTSTREAMDEF.H"

typedef struct {
    local_rf_id_t dst;
    uint32_t timeout;
    bool f_timeout;
    bool f_active;
} S_ACTIVE_TRANSACTION_SLOT_T;



#define C_CHANID_RF_BRIDGE 1
#define C_CHANID_DONGLE 2


// for host
class TUSARTCMDMASTER: public TFFC, public TPARSEOBJ  {
		virtual void Task ();
		virtual bool find_protocol (void *src, uint32_t szsrc, uint32_t &find_ix, uint32_t &findsz);    // detect ������� S_CHANHDR_T
		virtual bool parse (void *src, uint32_t szsrc);

		void framesend (void *src, uint32_t sz, uint8_t prt, uint32_t tmot);
		uint8_t cur_trid;
		uint8_t *txbufer;
		uint32_t c_txbufer_size;

		uint32_t lenstr (void *s);
		void CopyMemorySDC (void *s, void *d, uint32_t sz);

		S_ACTIVE_TRANSACTION_SLOT_T *reqslots;
		void clear_actslots ();

		S_RFHEADER_T *capsule_open (void *s, uint32_t srcsz, uint32_t &capsz);
		bool check_rf_frame (S_RFHEADER_T *s, uint32_t srcsz);
		//bool check_capsule (S_CHANHDR_T *s, uint32_t srcsz);

	protected:
		virtual void get_param_resp_cb (local_rf_id_t svid, uint16_t ix, const S_PRMF_CAPTION_T *name, S_RFPARAMVALUE_T &src, ERESPSTATE rx_state);
		virtual void set_param_resp_cb (local_rf_id_t svid, uint16_t ix, const S_PRMF_CAPTION_T *name, S_RFPARAMVALUE_T &src, ERESPSTATE rx_state);
		virtual void get_state_resp_cb (local_rf_id_t svid, S_DEVSTATE_T *src, ERESPSTATE rx_state);
		virtual void get_event_resp_cb (local_rf_id_t svid, uint16_t ix, S_EVENT_ITEM_T *evnt, ERESPSTATE rx_state);
		virtual void call_event_resp_cb (local_rf_id_t svid, uint32_t event_code, ERESPSTATE rx_state);


        TTXIF *tx_obj;
        uint16_t calculate_crc16cap (uint8_t *src, uint32_t sz);
        
        local_rf_id_t self_id;

	public:
        TUSARTCMDMASTER (TTXIF *objc, local_rf_id_t sa);
        ~TUSARTCMDMASTER ();
		void get_param_req (local_rf_id_t dvid, uint16_t ix, uint32_t tmot);
		void get_param_req (local_rf_id_t dvid, char *name, uint32_t tmot);
		void set_param_req (local_rf_id_t dvid, uint16_t ix, S_RFPARAMVALUE_T prm, uint32_t tmot);
		void set_param_req (local_rf_id_t dvid, char *name, S_RFPARAMVALUE_T prm, uint32_t tmot);
		void get_state_req (local_rf_id_t dvid, uint32_t tmot);
		void get_event_req (local_rf_id_t dvid, uint16_t ix, uint32_t tmot);
		void call_event_req (local_rf_id_t dvid, uint32_t event_code, uint32_t calltime, uint32_t tmot );		// ��� event_code ����� gui ����������
};



// for mcu
class TUSARTCMDSLAVE: public TFFC, public TPARSEOBJ  {
		virtual void Task ();
		virtual bool find_protocol (void *src, uint32_t szsrc, uint32_t &find_ix, uint32_t &findsz);    // detect ������� S_CHANHDR_T
		virtual bool parse (void *src, uint32_t szsrc);
		void framesend (void *src, uint32_t sz, uint8_t prt);
		uint8_t cur_trid;
		uint8_t cur_trid_rx;
		uint8_t *txbufer;
		uint32_t c_txbufer_size;
		uint32_t lenstr_sz (void *s, uint32_t sz);
		void CopyMemorySDC (void *s, void *d, uint32_t sz);

		S_ACTIVE_TRANSACTION_SLOT_T *reqslots;
		void clear_actslots ();

		S_RFHEADER_T *capsule_open (void *s, uint32_t srcsz, uint32_t &capsz);
		bool check_rf_frame (S_RFHEADER_T *s, uint32_t srcsz);
	
		uint8_t last_rx_trid;

	protected:
		 void send_get_param_resp (local_rf_id_t dvid, uint16_t ix, const S_PRMF_CAPTION_T *name, const S_RFPARAMVALUE_T &src, ERESPSTATE rx_state);
		 void send_set_param_resp (local_rf_id_t dvid, uint16_t ix, const S_PRMF_CAPTION_T *name, const S_RFPARAMVALUE_T &src, ERESPSTATE rx_state);
		 void send_get_state_resp (local_rf_id_t dvid, const S_DEVSTATE_T &src, ERESPSTATE rx_state);
		 void send_get_event_resp (local_rf_id_t dvid, uint16_t ix, const S_EVENT_ITEM_T &evnt, ERESPSTATE rx_state);
		 void send_call_event_resp (local_rf_id_t dvid, uint32_t event_code, ERESPSTATE rx_state);

		TTXIF *tx_obj;
		uint16_t calculate_crc16cap (uint8_t *src, uint32_t sz);
		
		local_rf_id_t self_id;
		TUSARTCMDSLAVE (TTXIF *objc, local_rf_id_t sa);
		~TUSARTCMDSLAVE ();
		virtual ERESPSTATE get_param_req_cb (local_rf_id_t svid, uint16_t ix, const S_PRMF_CAPTION_T *name, S_RFPARAMVALUE_T &dst) = 0;
		virtual ERESPSTATE set_param_req_cb (local_rf_id_t svid, uint16_t ix, const S_PRMF_CAPTION_T *name, S_RFPARAMVALUE_T &prm) = 0;
		virtual ERESPSTATE get_state_req_cb (local_rf_id_t svid, S_DEVSTATE_T &prm) = 0;
		virtual ERESPSTATE get_event_req_cb (local_rf_id_t svid, uint16_t ix, S_EVENT_ITEM_T &evnt) = 0;
		virtual ERESPSTATE call_event_req_cb (local_rf_id_t svid, uint32_t event_code) = 0;		// ��� event_code ����� gui ����������

};


#endif
 