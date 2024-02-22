#ifndef _h_usart_stream_interface_h_
#define _h_usart_stream_interface_h_


#include <stdint.h>


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
 