#ifndef _H_RF_MASTER_H_
#define _H_RF_MASTER_H_



#include "TFTASKIF.h"
#include "rfbasecmd.h"
#include "RFM69.h"
#include "SYSBIOS.H"

/*
���� �������� ������ ������ �� ��������� ��������
1. �������� ������� ������ ������ (��������� � �������������)  (10)
2. ����� ������������� ������� ������.
3. �������� ���� ������.				(00)
4. �������� ���������� ������		(01)
5. ����� ACK � �������� ������� ��������.
*/



enum ESWTXA {\
// ������������������ ��� �������� ���������������� ������ (��������)
ESWTXA_TX_FIST /* �������� ������� ������� � ������������������ ��������*/, \
ESWTXA_RESP_FIST /* �������� ������������� ������ ������� ������ */, \
ESWTXA_TX_MIDLE /* �������� ����� �������� ��� ������������� */, \
ESWTXA_TX_LAST /* �������� ���������� ������� � ����� ������� */, \
ESWTXA_RESP_LAST /* �������� ������������� ������ */, \
// ������������������ ��� �������� ���������� ������ (���������)
ESWTXA_TX_FIST_LAST /* �������� ���������� �����-�������*/, \
ESWTXA_RESP_FIST_LAST /* ������������� ���������� �����-������� */, \
// ������������������ �������� ������� �� ������� ACK �������
ESWTXA_TX_SECTOR /* �������� ������� �� ACK ������� ������� */, \
ESWTXA_RESP_SECTOR /* ������������� ������� */, \
// ��������� ����������
ESWTXA_COMPLETE /* ���������� ����������� */, \
ESWTXA_ERROR /* ���������� ����������� � ������� */, \
ESWTXA_TIMEOUT /* ���������� ����������� � ��������� � ��������� */, \
ESWTXA_ENDENUM, \
};


#define C_RF_REPEATE 3


class TRFMASTER {
		virtual void Task ();
		local_rf_id_t self_id;
		local_rf_id_t dest_id;
		bool f_tx_hard_error;
		bool f_tx_req_type;
		ERFSDEVICE tx_device_status;
		//IFCRFTX *txobj;
		//IFEVENT *evcb_tx;
		TRADIOIF *radio;
		const uint16_t c_datapayload_size;
		ESWTXA sw_tx;

		uint8_t *acksectortab;
		uint8_t *rxsector;
		
	
		
		uint16_t tx_midl_sector_count;
		uint16_t tx_byte_cnt;
		uint8_t *tx_src;
		uint8_t c_set_repeate;
		uint8_t repeate_cnt;
		//uint8_t tx_seq_code;
	
		uint16_t cur_txsize;
		ERFFMARK cur_txmark;
		uint8_t cur_txsect;
		uint8_t cur_maxtxsectors;
		SYSBIOS::Timer cur_swtimeout;
		ERFTACK need_ack;
		//SYSBIOS::Timer cur_acktimeout;
		
		
		bool is_tx_processed ();
		void clear_rx_src ();
		bool check_rx_rf_sector ();
		bool read_rx_rf_sector (void *src);
		bool check_crc (S_RFMARKTAG_T *f);
		bool check_selfid (S_RFMARKTAG_T *f);
		void send_sector (void *scr, uint8_t sz, ERFFMARK m, ERFTACK ackt, uint8_t sect);
		//void set_destid (local_rf_id_t dstid);
		
	public:
		TRFMASTER (TRADIOIF *r, local_rf_id_t slf);
		static uint8_t tagbytegen (bool f_hrder, bool f_req, ERFFMARK m, ERFTACK ak, ERFSDEVICE dvs);
		void set_harderror_bit (bool v);

		void set_device_stat_bit (ERFSDEVICE dv);
		void tx_request_data (local_rf_id_t dstid, void *src, uint32_t sz);
		void tx_responce_data (local_rf_id_t dstid, void *src, uint32_t sz);
		void tx_repeate_set (uint8_t rcnt);
	
};





#endif
