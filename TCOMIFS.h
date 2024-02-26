#ifndef _h_communication_interfaces_h_
#define _h_communication_interfaces_h_



#include <stdint.h>
#include "TTFIFO.h"
#include "TFTASKIF.h"
#include "TUSARTSTREAMDEF.H"

#define C_TRLAYERPREAMBLE_A 0x3D
#define C_TRLAYERPREAMBLE_B 0xBC

/*
uint8_t system - ���������� ��������� ���������� (���-�� ���� �����). � ������ ����������(�����) �������� ���� ��������.
uint8_t trid - ����������� ���������� (��������� ��� ������ �������, ��� ���� ������������ ��� �������� ����������� ������� ������� ������ ������ ��� ������, ������ ��� �������� request � responce).
*/








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



// ������ ������������� ������ (������� add_parser ��������� ���������� ���������� ���������� ������������� ������).
// ������ ������ ��������������� ��� �������� � ������������ �����.
// ��� ������ ��������� ������������ ������ � �������� ������� ��������� TPARSEOBJ
class TTRANSPPARSE {

        uint8_t *c_rx_buffer;
        const uint32_t c_rx_alloc_size;
        uint32_t ix_add_rx;

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

};



#endif

 