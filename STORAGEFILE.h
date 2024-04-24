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
        uint32_t cur_file_size; 

    public:
        TSTORAGEFILE (char *filename, const uint32_t size);
        ~TSTORAGEFILE ();
        virtual uint32_t file_size ();
        virtual bool write (uint32_t adrix, uint8_t *src, uint32_t wr_size);
        virtual bool read (uint32_t adrix, uint8_t *dst, uint32_t wr_size);
};


#endif
