#ifndef _h_easy_storage_interface_h_
#define _h_easy_storage_interface_h_

#include "stdint.h"

class IFSTORAGE {
	public:
        virtual bool Write (uint32_t adrix, uint8_t *src, uint32_t wr_size) = 0;
        virtual bool Read (uint32_t adrix, uint8_t *dst, uint32_t wr_size) = 0;
};


#endif
