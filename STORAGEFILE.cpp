#include "STORAGEFILE.h"

TSTORAGEFILE::TSTORAGEFILE (char *filename, uint32_t size) : c_file_size (size), c_filename(filename)
{
wrrd = new TMCreateWrireStream  ();
f_objok = false;
if (wrrd && size && c_filename)
    {
    bool f_need_blank = false;
    if (wrrd->OpenStream (const_cast<char*>(filename)))
        {
        uint32_t sz = wrrd->GlobalFileSize;
        if (sz != c_file_size) f_need_blank = true;
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
if (f_objok && wrrd->CreateStreamLen (const_cast<char*>(c_filename), c_file_size)) wrrd->CloseStream();
}



TSTORAGEFILE::~TSTORAGEFILE ()
{
if (wrrd) delete wrrd;
}



bool TSTORAGEFILE::Write (uint32_t adrix, uint8_t *src, uint32_t wr_size)
{
bool rv = false;
if (f_objok && wrrd->OpenStream (c_filename)) {
    uint32_t sz = wrrd->GlobalFileSize;
    if (sz == c_file_size) {
        if ((adrix + wr_size) <= c_file_size) {
            if (wrrd->SetOffsetBegin (adrix)) {
                if (sz == wrrd->Write (src, wr_size)) rv = true;
                }
            }
        }
    wrrd->CloseStream();
    }
return rv;
}



bool TSTORAGEFILE::Read (uint32_t adrix, uint8_t *dst, uint32_t wr_size)
{
bool rv = false;
if (f_objok && wrrd->OpenStream (c_filename)) {
    uint32_t sz = wrrd->GlobalFileSize;
    if (sz == c_file_size) {
        if ((adrix + wr_size) <= c_file_size) {
            if (wrrd->SetOffsetBegin (adrix)) {
                if (sz == wrrd->ReadData (dst, wr_size)) rv = true;
                }
            }
        }
    wrrd->CloseStream();
    }
return rv;
}
