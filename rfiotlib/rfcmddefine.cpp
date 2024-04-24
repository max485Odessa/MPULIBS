#include "rfcmddefine.h"


void decode_trid (trid_t d,  ERFDTYPE *tp, ERESPSTATE *srslt, uint8_t *trid )
{
if (tp) *tp = (d & 128)?ERFDTYPE_REQ:ERFDTYPE_RESP;
if (srslt) *srslt = (ERESPSTATE)((d >> 5) & 3);
if (trid) *trid = d & 0x1F;
}



trid_t encode_trid (ERFDTYPE tp, ERESPSTATE srs, uint8_t trid )
{
uint8_t rv = 0;
if (tp == ERFDTYPE_REQ) rv |= 128;
rv |= (srs << 5) & 6;
rv |= (trid & 0x1F);
return rv;
}



uint8_t calculate_crc8rf (void *s, uint32_t sz)
{
uint8_t crc = 0;
uint8_t dat;
uint8_t *src = (uint8_t*)s;
while (sz)
    {
    dat = *src++;
    if (!dat)
        {
        crc += (sz | 1);
        }
    else
        {
        if (dat == 0xFF)
            {
            crc += (sz | 2);
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

