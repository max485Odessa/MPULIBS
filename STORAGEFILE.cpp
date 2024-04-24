#include "STORAGEFILE.h"

TSTORAGEFILE::TSTORAGEFILE (char *filename, uint32_t size) : c_file_size (size), c_filename(filename)
{
wrrd = new TMCreateWrireStream  ();
f_objok = false;
cur_file_size = 0;
if (wrrd && c_file_size && c_filename)
    {
    bool f_need_blank = false;
    if (wrrd->OpenStream (c_filename))
        {
        cur_file_size = wrrd->GlobalFileSize;
        if (cur_file_size != c_file_size) f_need_blank = true;
        wrrd->CloseStream();
        }
    else
        {
        f_need_blank = true;
        }
    f_objok = true;
    if (f_need_blank) create_file_blank ();
    }
}



void TSTORAGEFILE::create_file_blank ()
{
if (f_objok && wrrd->CreateStreamLen (const_cast<char*>(c_filename), c_file_size))
    {
    cur_file_size = wrrd->GlobalFileSize;
    wrrd->CloseStream();
    }
}



TSTORAGEFILE::~TSTORAGEFILE ()
{
if (wrrd) delete wrrd;
}



bool TSTORAGEFILE::write (uint32_t adrix, uint8_t *src, uint32_t wr_size)
{
bool rv = false;
if (f_objok && wrrd->OpenStream (c_filename)) {
    uint32_t sz = wrrd->GlobalFileSize;
    if (sz == c_file_size) {
        if ((adrix + wr_size) <= c_file_size) {
            if (wrrd->SetOffsetBegin (adrix)) {
                if (wr_size == wrrd->Write (src, wr_size)) rv = true;
                }
            }
        }
    wrrd->CloseStream();
    }
return rv;
}



bool TSTORAGEFILE::read (uint32_t adrix, uint8_t *dst, uint32_t rd_size)
{
bool rv = false;
if (f_objok && wrrd->OpenStream (c_filename)) {
    uint32_t sz = wrrd->GlobalFileSize;
    if (sz == c_file_size) {
        if ((adrix + rd_size) <= c_file_size) {
            if (wrrd->SetOffsetBegin (adrix)) {
                if (rd_size == wrrd->ReadData (dst, rd_size)) rv = true;
                }
            }
        }
    wrrd->CloseStream();
    }
return rv;
}



uint32_t TSTORAGEFILE::file_size ()
{
uint32_t sz = 0;
if (f_objok) sz = cur_file_size;
return sz;
}


