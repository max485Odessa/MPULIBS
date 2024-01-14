#ifndef _h_file_storage_object_h_
#define _h_file_storage_object_h_



#include "STORAGEIF.h"
#include "TMReadStream.hpp"
#include "TMWriteStream.cpp"


class TSTORAGEFILE: public IFSTORAGE {
        TMCreateWrireStream *wrrd;
        const uint32_t c_file_size;
        const char *c_filename;
        void create_file_blank ();
        bool f_objok;
        
    public:
        TSTORAGEFILE (char *filename, const uint32_t size);
        ~TSTORAGEFILE ();
        virtual bool Write (uint32_t adrix, uint8_t *src, uint32_t wr_size);
        virtual bool Read (uint32_t adrix, uint8_t *dst, uint32_t wr_size);
};


#endif
