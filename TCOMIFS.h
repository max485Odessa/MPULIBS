#ifndef _h_communication_interfaces_h_
#define _h_communication_interfaces_h_



#include <stdint.h>
#include "TTFIFO.h"
#include "TFTASKIF.h"
#include "TUSARTSTREAMDEF.H"

#define C_TRLAYERPREAMBLE_A 0x3D
#define C_TRLAYERPREAMBLE_B 0xBC

/*
uint8_t system - реализация механизма назначения (что-то типа порта). В каждом назначении(порту) работает свой протокол.
uint8_t trid - индификатор транзакции (локальный для каждой системы, это поле используется для контроля целостности передач больших блоков данных при приеме, иногда как привязку request к responce).
*/


#define C_PREAMBLE_P1_A 0x55
#define C_PREAMBLE_P1_B 0x12


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






class TPARSEOBJ {
      public:
        // src - input raw data,
        // szsrc - size input data
        // if return value true:
        // find_ix - index fist data frame protocol
        // findsz - size protocol data
        virtual bool find_protocol (void *src, uint32_t szsrc, uint32_t &find_ix, uint32_t &findsz) = 0;

        // src - fist data
        // szsrc - size data
        virtual bool parse (void *src, uint32_t szsrc) = 0;
};



typedef struct {
    TPARSEOBJ *po;
    uint32_t find_ix;
    uint32_t find_sz;
    bool finded;
} S_PODETECT_T;



/*  RX
 RX raw hard driver ->bridge mechanism-> (TTRANSPPARSE::TRXIF)-> TPARSEOBJ(base class)->other cb ifaces

    TX
 TPARSEOBJ-> TTXIF (hard driver interface)

*/



// Обьект транспортного уровня (функция add_parser реализует добавление нескольких протоколов транспортного уровня).
// Пакует данные предназначенные для передачи в транспортный пакет.
// При приеме извлекает транспортные данные и передает обьекту обработки TPARSEOBJ
class TTRANSPPARSE {
        //TTFIFO<uint8_t> *fifo_tx;
        uint8_t *c_rx_buffer;
        const uint32_t c_rx_alloc_size;
        uint32_t ix_add_rx;
        //uint16_t calculate_crc16 (void *src, uint16_t sz);
        //virtual void rxif_in (void *src, uint32_t sz);

        const uint8_t c_protocol_obj;
        uint8_t protocol_obj_n;
        S_PODETECT_T *parsobj_arr;
        bool detect_protocols (void *src, uint32_t sz);
        S_PODETECT_T *getfist_frame ();
        void parse_start (S_PODETECT_T *po);

        void copymem_sdc (uint8_t *s, uint8_t *d, uint32_t sz);
        uint32_t is_free_space ();

    protected:


        
    public:
        TTRANSPPARSE (uint32_t rxsize, uint8_t pon);
        bool add_parser (TPARSEOBJ *obj);
        void in (TRXIF *sobj);
        void parse ();
        //void send (uint8_t system, uint8_t trid, void *src, uint32_t src_sz);
};


/*
enum ETXERROR {ETXERROR_NONE = 0, ETXERROR_ENDENUM};
class TIOTCMD: public TPARSEOBJ {
        virtual bool find_protocol (void *src, uint32_t szsrc, uint32_t &find_ix, uint32_t &findsz);    // detect капсулы S_CHANHDR_T
        virtual bool parse (void *src, uint32_t szsrc);
        TTXIF *tx_obj;

        uint16_t calculate_crc (uint8_t *src, uint32_t sz);
      protected:

      public:
        TIOTCMD (TTXIF *t);



};
*/



#endif

 