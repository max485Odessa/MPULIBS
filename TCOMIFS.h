#ifndef _h_communication_interfaces_h_
#define _h_communication_interfaces_h_



#include <stdint.h>
#include "TTFIFO.h"

#define C_TRLAYERPREAMBLE_A 0x3D
#define C_TRLAYERPREAMBLE_B 0xBC

/*
uint8_t system - реализация механизма назначения (что-то типа порта). В каждом назначении(порту) работает свой протокол.
uint8_t trid - индификатор транзакции (локальный для каждой системы, это поле используется для контроля целостности передач больших блоков данных при приеме, иногда как привязку request к responce).
*/

#pragma pack (push,1)

typedef struct {
    uint8_t preamble_a;
    uint8_t preamble_b;
    uint8_t chan;
    uint8_t trid;
    uint16_t size;
    uint16_t crc16;
} S_CHANHDR_T;

#pragma pack (pop)



// интерфейс физического уровня приема
// его имплементирует код декодировки протокола/ов канального уровня
// для гибкости его не должен вызывать низкоуровневый драйвер из своего контекста задачи
class TRXIF {
    public:
        virtual void rxif_in (void *src, uint32_t sz) = 0;
};



// интерфейс физического уровня передачи данных
// его имплементирует низкоуровневый драйвер работающий на отправку данных в физическое устройство
// драйвер должен реализовать FIFO
class TTXIF {
    public:
        virtual uint32_t txif_out (void *dst, uint32_t sz) = 0;
        virtual uint32_t txif_free_size () = 0;
};



class TPARSEOBJ {
      public:
        virtual bool find_protocol (void *src, uint32_t szsrc, uint32_t &find_ix, uint32_t &findsz) = 0;
        virtual bool parse (void *src, uint32_t szsrc) = 0;
};



typedef struct {
    TPARSEOBJ *po;
    uint32_t find_ix;
    uint32_t find_sz;
    bool finded;
} S_PODETECT_T;



// Обьект транспортного уровня (функция add_parser реализует добавление нескольких протоколов транспортного уровня).
// Пакует данные предназначенные для передачи в транспортный пакет.
// При приеме извлекает транспортные данные и передает обьекту обработка TPARSEOBJ
class TTRANSPPARSE: protected TRXIF {
        //TTFIFO<uint8_t> *fifo_tx;
        uint8_t *c_rx_buffer;
        const uint32_t c_rx_alloc_size;
        uint32_t ix_add_rx;
        //uint16_t calculate_crc16 (void *src, uint16_t sz);
        virtual void rxif_in (void *src, uint32_t sz);

        const uint8_t c_protocol_obj;
        uint8_t protocol_obj_n;
        S_PODETECT_T *parsobj_arr;
        bool detect_protocols (void *src, uint32_t sz);
        S_PODETECT_T *getfist_frame ();
        void parse_start (S_PODETECT_T *po);

        void copymem_sdc (uint8_t *s, uint8_t *d, uint32_t sz);

    protected:


        
    public:
        TTRANSPPARSE (uint32_t rxsize, uint8_t pon);
        bool add_parser (TPARSEOBJ *obj);
        void parse ();
        //void send (uint8_t system, uint8_t trid, void *src, uint32_t src_sz);
};








#endif

 