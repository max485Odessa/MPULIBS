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
конструкция капсулы: в начале фрейма идет фиксированная структура S_MUXCAPSULE_HDR_T, далее идут
данные, размер данных ограничен установкой константы C_MUXDATASIZE_MAX, контрольная сумма подсчитывается
только для секции данных.
*/


typedef void (*_capsule_rxcb_t) (EMUCCHAN mux_id, char *buf, uint32_t len);


// common
void capsule_protocol_init ();
// rx function
void capsule_rx_push_raw (uint8_t *s, uint32_t sz);         // все поступающие данные с usart передавать сюда
void capsule_parse (_capsule_rxcb_t cb);                    // после загрузки данных методом capsule_rx_push_raw периодически запускать этот метод, в случае декодирования фрейма запустится callback передаваемый в качестве параметра
// tx function
bool capsule_tx_free_space_check (uint32_t sz);                 // проверяет возможность принять данные
bool capsule_mux_tx_push (EMUCCHAN mux, void *s, uint32_t sz);  // кодирует/пакетирует данные на нужный mux канал
// tx bufer function
uint32_t capsule_push_size ();          // функция узнать есть ли данные на передачу
uint32_t capsule_tx_pop_raw (void *dst, uint32_t masize);   // извлечь данные из fifo буфера

#endif

 