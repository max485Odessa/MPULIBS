#ifndef _H_SLCAN_LAWICEL_FORMAT_H_
#define _H_SLCAN_LAWICEL_FORMAT_H_


#include "textrut.hpp"
#include "canard.h"

#define C_SLCANRXBUF_SIZE 128000
#define C_SLCANTTAGMAX_SIZE 26

typedef struct {
    char data[C_SLCANTTAGMAX_SIZE];
    char sz;
} TSLTTAG;


typedef struct {
    char data[C_SLCANTTAGMAX_SIZE + 1];
    char sz;
} TSLTTAGWR;



enum ELWSTATE {ELWSTATE_T_WAIT = 0, ELWSTATE_ADR = 1, ELWSTATE_SIZE = 2, ELWSTATE_BODY= 3, ELWSTATE_DELIM = 4};


class TLAWICELL {
    private:
        unsigned char buf_rx[C_SLCANRXBUF_SIZE];
        
        unsigned long push_rx_ix;
        unsigned long pop_rx_ix;
        unsigned long size_rx;

        bool push (TEX::BUFPAR *Inp);
        bool ExtractT_tag (TEX::BUFPAR *linp, CanardCANFrame *dst);

        bool GetNext_tag (TSLTTAG *ldst);

    public:
        TLAWICELL ();
        bool In (TEX::BUFPAR *InRaw);           // добавляет сырые данные в буфер
        bool Get_RX (CanardCANFrame *dst);      // извлекает пакеты прошедшие проверку, удаляет битые

        bool UavToLawicel (CanardCANFrame *inp, TSLTTAGWR *ld);

        

};




#endif


 