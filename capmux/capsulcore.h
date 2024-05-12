#ifndef _H_CAPSULE_CORE_H_
#define _H_CAPSULE_CORE_H_

#include "stdint.h"

typedef enum {EMUCCHAN_TLV_TRAFIC = 0, EMUCCHAN_MAV_TRAFIC, EMUCCHAN_ENDENUM} EMUCCHAN;

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

/*
����������� �������: � ������ ������ ���� ������������� ��������� S_MUXCAPSULE_HDR_T, ����� ����
������, ������ ������ ��������� ���������� ��������� C_MUXDATASIZE_MAX, ����������� ����� ��������������
������ ��� ������ ������.
*/


typedef void (*_capsule_rxcb_t) (EMUCCHAN mux_id, char *buf, uint32_t len);


// common
void capsule_protocol_init ();
// rx function
void capsule_rx_push_raw (uint8_t *s, uint32_t sz);         // ��� ����������� ������ � usart ���������� ����
void capsule_parse (_capsule_rxcb_t cb);                    // ����� �������� ������ ������� capsule_rx_push_raw ������������ ��������� ���� �����, � ������ ������������� ������ ���������� callback ������������ � �������� ���������
// tx function
bool capsule_tx_free_space_check (uint32_t sz);                 // ��������� ����������� ������� ������
bool capsule_mux_tx_push (EMUCCHAN mux, void *s, uint32_t sz);  // ��������/���������� ������ �� ������ mux �����
// tx bufer function
uint32_t capsule_push_size ();          // ������� ������ ���� �� ������ �� ��������
uint32_t capsule_tx_pop_raw (void *dst, uint32_t masize);   // ������� ������ �� fifo ������

#endif

 