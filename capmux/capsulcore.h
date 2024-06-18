#ifndef _H_CAPSULE_CORE_H_
#define _H_CAPSULE_CORE_H_

#include "stdint.h"

//typedef enum {EMUCCHAN_TLV_TRAFIC = 0, EMUCCHAN_MAV_TRAFIC, EMUCCHAN_ENDENUM} EMUCCHAN;

#define C_MUXPREAMBLE_A 0x32
#define C_MUXPREAMBLE_B 0x33
#define C_MUXDATASIZE_MAX 2048
#define C_TXRXBUF_SIZE (C_MUXDATASIZE_MAX * 2)

#pragma pack (push, 1)

typedef struct {
    uint8_t Preamble_A;
    uint8_t Preamble_B;
    uint8_t Mux;
    uint16_t size;
    uint16_t crc;
} S_MUXCAPSULE_HDR_T;

#pragma pack (pop)


#ifdef SBCPP
class TCPMXRXCB {
	public:
		virtual void capmux_rx_cb (uint8_t mux, uint8_t *buf, uint32_t len);
};


typedef struct {
	TCPMXRXCB *cb;
	uint32_t muxid;
} S_CPMXRXDATA_T;




class TCAPMUX {

		uint8_t ifcadd_ix;
		const uint8_t c_muxifcrx_cnt;
		S_CPMXRXDATA_T *listifc;
		long findaifc (uint32_t mxid);

		char *copysdc (char *s, char *d, uint32_t sz);
		uint16_t muxprotocol_crc (void *data, uint16_t ctrl_size);
		bool check_muxcapsule_linedata (void *basebuf, uint32_t *ix_start, uint32_t *mess_size, uint32_t start_ix, uint32_t last_psh_ix);
		uint32_t detect_find_ix, detect_find_size;
		bool detect_protocols (void *base, uint32_t start_ix, uint32_t stop_ix);
		void cb_execute_mux_frame (uint8_t *s, uint16_t sz);
		void push_tx_data (char *s, uint32_t sz);

   protected:
		uint8_t *raw_rx;
		uint8_t *raw_tx;
		const uint32_t c_raw_rx_alloc_size;
		const uint32_t c_raw_tx_alloc_size;
		uint32_t push_rx_ix;
		uint32_t push_tx_ix;
		uint32_t pop_tx_ix;
		uint32_t size_tx_ix;

	public:
		TCAPMUX (uint32_t rxsz, uint32_t tx, uint8_t micnt);
		TCAPMUX (void *s_rx, uint32_t rxsz, void *s_tx, uint32_t tx, uint8_t micnt);
	
		bool AddOrReplace_rx_ifc (uint32_t mxid, TCPMXRXCB *mcb);
	
		void rx_push_raw (uint8_t *s, uint32_t sz);
		void mux_rx_data ();
	
		bool tx_free_space_check (uint32_t sz);                 // проверяет возможность принять данные
		bool mux_tx_push (uint16_t mux, void *s, uint32_t sz);  // кодирует/пакетирует данные на нужный mux канал
	
		uint32_t tx_pushed_size ();
		uint32_t tx_pop_raw (void *dst, uint32_t masize);
	
};
#endif


typedef void (*_capsule_rxcb_t) (uint16_t mux_id, char *buf, uint32_t len);

// common
void capsule_protocol_init ();
// rx function
void capsule_rx_push_raw (uint8_t *s, uint32_t sz);         // все поступающие данные с usart передавать сюда
void capsule_parse (_capsule_rxcb_t cb);                    // после загрузки данных методом capsule_rx_push_raw периодически запускать этот метод, в случае декодирования фрейма запустится callback передаваемый в качестве параметра
// tx function
bool capsule_tx_free_space_check (uint32_t sz);                 // проверяет возможность принять данные
bool capsule_mux_tx_push (uint16_t mux, void *s, uint32_t sz);  // кодирует/пакетирует данные на нужный mux канал
// tx bufer function
uint32_t capsule_push_size ();          // функция узнать есть ли данные на передачу
uint32_t capsule_tx_pop_raw (void *dst, uint32_t masize);   // извлечь данные из fifo буфера

#endif

 