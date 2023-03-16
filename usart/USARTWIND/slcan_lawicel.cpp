#include "slcan_lawicel.h"
#include "textrut.hpp"


TLAWICELL::TLAWICELL ()
{
push_rx_ix = 0;
pop_rx_ix = 0;
size_rx = 0;
}



// перекодирует миникадры с uavcan в lawicel формат  
bool TLAWICELL::UavToLawicel (CanardCANFrame *inp_src, TSLTTAGWR *sl_dst)
{
bool rv = false;
if (inp_src && sl_dst)
    {
    char *lDst = sl_dst->data;
    char *lSrc = inp_src->data;
    unsigned char sz_dst = inp_src->data_len;
    do  {
        if (!sz_dst || sz_dst > sizeof(inp_src->data)) break;
        // сигнатура Data кадра slcan
        *lDst++ = 'T';
        // кодируем адресную секцию slcan
        unsigned char ix = 0, dat;
        unsigned long dat32 = inp_src->id | 0x80000000;     // & 0x7FFFFFFF) 
        while (ix < sizeof(long))
            {
            dat = dat32 >> ((sizeof(long) - 1 - ix) * 8);
            ByteToHEX (lDst, dat); lDst += 2;
            ix++;
            }
        // кодируем 1 байтный размер slcan
        *lDst++ = ConvBinToASCIIHex (sz_dst);
        // кодируем секцию данных slcan
        ix = 0;
        dat32 = inp_src->data_len;
        while (ix < dat32)
            {
            ByteToHEX (lDst, *lSrc++); lDst += 2;
            ix++;
            }
        // общий размер пакета (информация для передачи по usart)
        sl_dst->sz = 11 + sz_dst * 2;       // с учетом закрывающего символа 0x0D
        // добавляем закрывающего тег
        *lDst = 0x0D;
        rv = true;
        } while (false);
    }
return rv;
}



bool TLAWICELL::push (TEX::BUFPAR *Inp)
{
 bool rv = false;
 static unsigned long size_rx_debug = 0;
 static unsigned long size_freespace_debug = C_SLCANRXBUF_SIZE;
 if (Inp && Inp->lpRam)
    {
    unsigned long free_sp = C_SLCANRXBUF_SIZE - size_rx;
    if (free_sp < size_freespace_debug)
        {
        // для отладки
        size_freespace_debug = free_sp;
        }
    if (Inp->sizes <= free_sp)
        {
        unsigned long sz = Inp->sizes;
        unsigned char *lSrc =(unsigned char*) Inp->lpRam;
        if (sz)
            {
            while (sz)
                {
                if (push_rx_ix >= C_SLCANRXBUF_SIZE) push_rx_ix = 0;
                buf_rx[push_rx_ix++] = *lSrc++;
                size_rx++;
                sz--;
                }
            if (size_rx > size_rx_debug) size_rx_debug = size_rx;       // для отладки
            rv = true;
            }
        }
    }
 return rv;
}



bool TLAWICELL::In (TEX::BUFPAR *InRaw)
{
   return push (InRaw);
}


// проверяет наличие строки и выдает ее когда она сформирована
// проверяет входные ошибки
/*
bool TLAWICELL::GetNext_tag (TSLTTAG *dst)
{
 bool rv = false;
 if (dst)
    {
    TSLTTAG inp_tag;
    char *lDst = inp_tag.data;
    unsigned long dsc_cnt = 0;
    unsigned long cnt = 0;
    char dat;
    unsigned long search_ix = pop_rx_ix;
    bool f_checked = false;
    bool f_save = false;
    ELWSTATE sw = ELWSTATE_T_WAIT;
    unsigned char lc_cntr;
    bool f_error = false;
    if (size_rx >= 12)   // T + 8 + 1 + 2
        {
        while (cnt < size_rx && !f_checked)
            {
            if (search_ix >= C_SLCANRXBUF_SIZE) search_ix = 0;
            dat = buf_rx[search_ix];

            switch (sw)
                {
                case ELWSTATE_T_WAIT:
                    {
                    if (dat == 'T')
                        {
                        sw = ELWSTATE_ADR;
                        lc_cntr = 8;
                        f_save = true;
                        }
                    break;
                    }
                case ELWSTATE_ADR:
                    {
                    if (CheckHexVal (dat))
                        {
                        lc_cntr--;
                        if (!lc_cntr) sw = ELWSTATE_SIZE;
                        }
                    else
                        {
                        f_error = true;
                        }
                    break;
                    }
                case ELWSTATE_SIZE:
                    {
                    if (CheckHexVal (dat))
                        {
                        // проверка количества байт в пакете
                        // не должно быть больше 8 байт (это 16 символов)
                        lc_cntr = ConvertHexVal(dat) * 2;
                        if (lc_cntr && lc_cntr <= 16)
                            {
                            sw = ELWSTATE_BODY;
                            }
                        else
                            {
                            f_error = true;
                            }
                        }
                    else
                        {
                        f_error = true;
                        }
                    break;
                    }
                case ELWSTATE_BODY:
                    {
                    if (CheckHexVal (dat))
                        {
                        lc_cntr--;
                        if (!lc_cntr) f_checked = true;
                        }
                    else
                        {
                        f_error = true;
                        }
                    break;
                    }
                case ELWSTATE_DELIM:
                    {
                    if (dat == 10)
                        {
                        f_checked = true;
                        }
                    else
                        {
                        f_error = true;
                        }
                    break;
                    }
                default:
                    {
                    }
                }
            search_ix++;
            cnt++;
            if (f_error) break;

            if (f_save)
                {
                *lDst++ = dat;
                dsc_cnt++;
                }
            }
        }
    if (search_ix >= C_SLCANRXBUF_SIZE) search_ix = 0;
    if (f_error)
        {
        // удаляем данные из буфера от точки поступления данных до байта приведшего к ошибке, включительно.
        pop_rx_ix = search_ix;
        size_rx -= cnt;
        }
    else
        {
        if (f_checked)
            {
            // корректируем указатель изьятия и размер данных
            pop_rx_ix = search_ix;
            size_rx -= cnt;
            // копируем найденные данные
            inp_tag.sz = dsc_cnt;
            *dst = inp_tag;
            rv = true;
            }
        }
    }
 return rv;
}
*/



bool TLAWICELL::GetNext_tag (TSLTTAG *dst)
{
 bool rv = false;
 if (dst)
    {
    TSLTTAG inp_tag;
    char *lDst = inp_tag.data;
    unsigned long dsc_cnt = 0;
    unsigned long cnt = 0;
    char dat;
    unsigned long search_ix = pop_rx_ix;
    bool f_checked = false;
    bool f_save = false;
    ELWSTATE sw = ELWSTATE_T_WAIT;
    unsigned char lc_cntr;
    bool f_error = false;
    if (size_rx >= 12)   // T + 8 + 1 + 2
        {
        while (cnt < size_rx && !f_checked)
            {
            if (search_ix >= C_SLCANRXBUF_SIZE) search_ix = 0;
            dat = buf_rx[search_ix];

            switch (sw)
                {
                case ELWSTATE_T_WAIT:
                    {
                    if (dat == 'T')
                        {
                        sw = ELWSTATE_ADR;
                        lc_cntr = 8;
                        f_save = true;
                        }
                    break;
                    }
                case ELWSTATE_ADR:
                    {
                    if (CheckHexVal (dat))
                        {
                        lc_cntr--;
                        if (!lc_cntr) sw = ELWSTATE_SIZE;
                        }
                    else
                        {
                        f_error = true;
                        }
                    break;
                    }
                case ELWSTATE_SIZE:
                    {
                    if (CheckHexVal (dat))
                        {
                        // проверка количества байт в пакете
                        // не должно быть больше 8 байт (это 16 символов)
                        lc_cntr = ConvertHexVal(dat) * 2;
                        if (lc_cntr && lc_cntr <= 16)
                            {
                            sw = ELWSTATE_BODY;
                            }
                        else
                            {
                            f_error = true;
                            }
                        }
                    else
                        {
                        f_error = true;
                        }
                    break;
                    }
                case ELWSTATE_BODY:
                    {
                    if (CheckHexVal (dat))
                        {
                        lc_cntr--;
                        if (!lc_cntr) f_checked = true;
                        /*
                            {
                            f_save = false;
                            sw = ELWSTATE_DELIM;
                            }
                        */
                        }
                    else
                        {
                        f_error = true;
                        }
                    break;
                    }
                case ELWSTATE_DELIM:
                    {
                    if (dat == 10)
                        {
                        f_checked = true;
                        }
                    else
                        {
                        f_error = true;
                        }
                    break;
                    }
                default:
                    {
                    }
                }
            search_ix++;
            cnt++;
            if (f_error) break;

            if (f_save)
                {
                *lDst++ = dat;
                dsc_cnt++;
                }
            }
        }
    if (search_ix >= C_SLCANRXBUF_SIZE) search_ix = 0;
    if (f_error)
        {
        // удаляем данные из буфера от точки поступления данных до байта приведшего к ошибке, включительно.
        pop_rx_ix = search_ix;
        size_rx -= cnt;
        }
    else
        {
        if (f_checked)
            {
            // корректируем указатель изьятия и размер данных
            pop_rx_ix = search_ix;
            size_rx -= cnt;
            // копируем найденные данные
            inp_tag.sz = dsc_cnt;
            *dst = inp_tag;
            rv = true;
            }
        }
    }
 return rv;
}




bool TLAWICELL::Get_RX (CanardCANFrame *dst)
{
bool rv = false;
if (dst && size_rx)
    {
    TSLTTAG tag;
    if (GetNext_tag (&tag))
        {
        TEX::BUFPAR inpp;
        inpp.lpRam = tag.data;
        inpp.sizes = tag.sz;
        return ExtractT_tag (&inpp, dst);
        }
    }
return rv;
}



// T1004243287292000000000084 - пример фрейма
bool TLAWICELL::ExtractT_tag (TEX::BUFPAR *linp, CanardCANFrame *dst)
{
bool rv = false;
static unsigned long error_count = 0;
if (linp && dst)
    {
    char *lSrc = (char*) linp->lpRam;
    unsigned long sz_string = linp->sizes;
    do  {
        if (sz_string <= 10) break;  // T+8(iiiiiiii)+1(size)
        if (*lSrc++ != 'T') break;
        sz_string--;
        if (!CheckHexFormats (lSrc, sz_string, false)) break;

        unsigned long adr29 = 0, dat;
        //unsigned char dat;

        dat = GetByteHexValue2 (lSrc);
        adr29 |= (dat << 24);
        lSrc += 2;

        dat = GetByteHexValue2 (lSrc);
        adr29 |= (dat << 16);
        lSrc += 2;

        dat = GetByteHexValue2 (lSrc);
        adr29 |= (dat << 8);
        lSrc += 2;

        dat = GetByteHexValue2 (lSrc);
        adr29 |= dat;
        lSrc += 2;

        sz_string -= 8;

        unsigned char data_sz = ConvertHexVal (*lSrc++);
        if (!data_sz || data_sz > 8) break;
        sz_string--;

        if (sz_string < (data_sz * 2)) break;  // !=
        char ix = 0;
        while (ix < data_sz)
            {
            dat = GetByteHexValue2 (lSrc);
            lSrc += 2;
            dst->data[ix] = dat;
            ix++;
            }

        dst->data_len = data_sz;
        dst->id = adr29;
        rv = true;
        } while (false);

    if (!rv)
        {
        error_count++;
        }
    }
return rv;
}



 