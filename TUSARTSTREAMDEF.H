#ifndef _h_usart_stream_interface_h_
#define _h_usart_stream_interface_h_


#include <stdint.h>



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


// интерфейс для обьекта который буферезирует приемные данные
class TRXIF {
    public:
        virtual uint32_t rxif_in (void *dst, uint32_t max_sz) = 0;
        virtual uint32_t rxif_is_rx () = 0;
};


class TTXIF {
    public:
        virtual bool txif_out (void *src, uint32_t sz) = 0;
        virtual uint32_t txif_free_space () = 0;
};



#endif
 