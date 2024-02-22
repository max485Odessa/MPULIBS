#include "TCOMIFS.h"



TTRANSPPARSE::TTRANSPPARSE (uint32_t rxsize, uint8_t pon) : c_rx_alloc_size (rxsize), c_protocol_obj (pon)
{
//fifo_tx = new TTFIFO<uint8_t>(txsize);
//fifo_tx->clear();
c_rx_buffer = new uint8_t[rxsize];
ix_add_rx = 0;
protocol_obj_n = 0;
parsobj_arr = new S_PODETECT_T[pon];
}



bool TTRANSPPARSE::add_parser (TPARSEOBJ *obj)
{
bool rv = false;
if (obj && protocol_obj_n < c_protocol_obj) {
    parsobj_arr[protocol_obj_n].po = obj;
    parsobj_arr[protocol_obj_n].finded = false;
    protocol_obj_n++;
    rv = true;
    }
return rv;
}


/*
uint16_t TTRANSPPARSE::calculate_crc16 (void *src, uint16_t sz)
{
uint16_t rv = 0;
if (src)
    {
    uint8_t *s = (uint8_t*)src;
    while (sz)
        {
        rv += *s++;
        sz--;
        }
    }
return rv;
}
*/




bool TTRANSPPARSE::detect_protocols (void *src, uint32_t sz)
{
bool rv = false;
if (src && sz) {
    uint32_t ix = 0;
    while (ix < protocol_obj_n) {
        parsobj_arr[ix].finded = parsobj_arr[ix].po->find_protocol(src, sz, parsobj_arr[ix].find_ix, parsobj_arr[ix].find_sz);
        ix++;
        }
    }
return rv;
}


// определяет какой протокол обнаружен первым
S_PODETECT_T *TTRANSPPARSE::getfist_frame ()
{
S_PODETECT_T *rv = 0;
long findix = -1, ix = 0;
uint32_t find_start_ix = c_rx_alloc_size - 1;
uint32_t find_sz = 0;
bool f_copy;
while (ix < protocol_obj_n)
    {
    f_copy = false;
    if (parsobj_arr[ix].finded)
        {
        if (find_sz)
            {
            if (find_start_ix > parsobj_arr[ix].find_ix) f_copy = true;
            }
        else
            {
            f_copy = true;
            }
        }
    if (f_copy)
        {
        find_sz = parsobj_arr[ix].find_sz;
        find_start_ix = parsobj_arr[ix].find_ix;
        findix = ix;
        }
    ix++;
    }
if (findix != -1) rv = &parsobj_arr[findix];
return rv;
}



void TTRANSPPARSE::parse_start (S_PODETECT_T *p)
{
if (p) {
    p->po->parse (c_rx_buffer + p->find_ix, p->find_sz);
    }
}



void TTRANSPPARSE::parse ()
{
if (ix_add_rx)
    {
    if (detect_protocols (c_rx_buffer, ix_add_rx))
        {
        S_PODETECT_T *po = getfist_frame ();
        if (po)
            {
            parse_start (po);
            // last data moved to left
            uint32_t last_ix = po->find_ix + po->find_sz;
            if (last_ix < ix_add_rx)
                {
                uint32_t tailsize = ix_add_rx - last_ix;
                copymem_sdc ((char*)&c_rx_buffer[last_ix], (char*)c_rx_buffer, tailsize);
                ix_add_rx = tailsize;
                }
            else
                {
                ix_add_rx = 0;
                }
            }
        }
    }
}



uint32_t TTRANSPPARSE::is_free_space ()
{
uint32_t rv = 0;
if (c_rx_alloc_size > ix_add_rx) rv = c_rx_alloc_size - ix_add_rx;
return rv;
}



void TTRANSPPARSE::in (TRXIF *sobj)
{
uint32_t szrx = sobj->rxif_is_rx();
if (szrx)
    {
    uint32_t szfree = is_free_space ();
    if (szfree)
        {
        if (szrx > szfree) szrx = szfree;
        uint8_t *dst_copy = &c_rx_buffer[ix_add_rx];
        sobj->rxif_in (dst_copy, szrx);
        ix_add_rx += szrx;
        }
    }
}



void TTRANSPPARSE::copymem_sdc (uint8_t *s, uint8_t *d, uint32_t sz)
{
while (sz)
    {
    *d++ = *s++;
    sz--;
    }
}



TIOTCMD::TIOTCMD (TTXIF *t)
{
tx_obj = t;
}



bool TIOTCMD::find_protocol (void *s, uint32_t szsrc, uint32_t &find_ix, uint32_t &findsz)
{
bool rv = false;
uint32_t ix = 0, datasize;
uint8_t *src = (uint8_t*)s;
S_CHANHDR_T *lhdr;
while (ix < szsrc)
    {
    datasize = szsrc - ix;
    lhdr = (S_CHANHDR_T*)&src[ix];
    if (datasize > (sizeof(S_CHANHDR_T)))
        {
        if (lhdr->preamble_a == C_PREAMBLE_P1_A && lhdr->preamble_b == C_PREAMBLE_P1_B)
            {
            unsigned short full_len = sizeof(S_CHANHDR_T) + lhdr->size;
            if (datasize >= full_len)
                {
                uint16_t svcrc = lhdr->crc16;
                lhdr->crc16 = 0;
                uint16_t clccrc = calculate_crc (src, full_len);
                lhdr->crc16 = svcrc;
                if (clccrc == svcrc)
                    {
                    find_ix = ix;
                    findsz = full_len;
                    rv = true;
                    break;
                    }
                }
            }
        }
    ix++;
    }
return rv;
}



uint16_t TIOTCMD::calculate_crc (uint8_t *src, uint32_t sz)
{
uint16_t crc = 0;
uint8_t dat;
while (sz)
    {
    dat = *src++;
    if (!dat)
        {
        crc += 0x0201;
        }
    else
        {
        if (dat == 0xFF)
            {
            crc += 0x1014;
            }
        else
            {
            if (dat & 0x41)
                {
                crc += 0x25;
                }
            else
                {
                crc += dat;
                }
            }
        }
    sz--;
    }
return crc;
}




bool TIOTCMD::parse (void *src, uint32_t szsrc)
{
}



/*
void TTRANSPPARSE::send (uint8_t system, uint8_t trid, void *src, uint32_t src_sz)
{
S_CHANHDR_T chhdr;
chhdr.preamble_a = C_TRLAYERPREAMBLE_A;
chhdr.preamble_b = C_TRLAYERPREAMBLE_B;
chhdr.chan = system;
chhdr.trid = trid;
chhdr.size = src_sz;
chhdr.crc16 = 0;
chhdr.crc16 = calculate_crc16 (&chhdr, sizeof(S_CHANHDR_T));
chhdr.crc16 += calculate_crc16 (src, src_sz);
fifo_tx->push ((uint8_t*)&chhdr, sizeof(S_CHANHDR_T));
fifo_tx->push ((uint8_t*)src, src_sz);
}
*/



 