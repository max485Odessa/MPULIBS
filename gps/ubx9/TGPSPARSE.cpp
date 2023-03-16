#include "TGPSPARSE.h"
#include "stdint.h"


S_FLOAT_T TGPSPCOMN::lon_static;
S_FLOAT_T TGPSPCOMN::lat_static;
S_FLOAT_T TGPSPCOMN::alt_static;
UTCTIME TGPSPCOMN::utc_static;
UTDDATE TGPSPCOMN::utd_static;
S_UDIG32_T TGPSPCOMN::satel_cnt_static_all;
S_UDIG32_T TGPSPCOMN::satel_cnt_static_active;
S_POSFIX_MODE_T TGPSPCOMN::posfixmod;
uint32_t TGPSPCOMN::Timer_RTK_implement;
S_DIG8_T TGPSPCOMN::lon_EW;
S_DIG8_T TGPSPCOMN::lat_NS;
S_DIG8_T TGPSPCOMN::magnet_EW;
S_FLOAT_T TGPSPCOMN::magnet_var;
S_FLOAT_T TGPSPCOMN::vdop;
S_FLOAT_T TGPSPCOMN::hdop;
S_FLOAT_T TGPSPCOMN::pdop;

const uint32_t TGPSPCOMN::tmpMultData[10] = {1,10,100,1000,10000,100000,1000000,10000000,100000000,1000000000};
const uint8_t TGPSPCOMN::C_NMEASIZELONG_DEG = 3;
const uint8_t TGPSPCOMN::C_NMEASIZELAT_DEG = 2;


TGPSPCOMN::TGPSPCOMN (uint16_t rxbufsize, TGPSSTATS *gpsstat) : TUBXNMRTCMUX (rxbufsize, gpsstat)
{
//statistics = gpsstat;
Timer_RTK_implement = 0;
lon_static.f_actual = false;
lat_static.f_actual = false;
alt_static.f_actual = false;
utc_static.f_actual = false;
utd_static.f_actual = false;
satel_cnt_static_all.f_actual = false;
satel_cnt_static_active.f_actual = false;
posfixmod.f_actual = false;
Timer_RTK_implement = 0;
lon_EW.f_actual = false;
lat_NS.f_actual = false;
magnet_EW.f_actual = false;
magnet_var.f_actual = false;
vdop.f_actual = false;
hdop.f_actual = false;
pdop.f_actual = false;

}






void TGPSPCOMN::cb_ubx_data (HDRUBX *inp, uint32_t size)
{
}



void TGPSPCOMN::cb_nmea_data (uint8_t *inp, uint32_t size)
{
}



void TGPSPCOMN::cb_rtcm3_data (uint8_t *inp, uint32_t size)
{

}





uint32_t TGPSPCOMN::str_len (char *lsrt)
{
	uint32_t sz = 0;
	char dat;
	while (true)
		{
		dat = lsrt[0];
		if (!dat) break;
		sz++;	
		lsrt++;
		}
return sz;
}


uint32_t TGPSPCOMN::CheckDecimal (char *lTxt, uint32_t sz)
{
uint32_t rv = 0;
if (lTxt && sz)
	{
	uint8_t datt;
	if (!sz) sz = GetLenStr (lTxt);
	while (sz)
		{
		datt = lTxt[0];
		if (!datt) break;
		if (datt < '0' || datt > '9')
			{
			rv = 0;
			break;
			}
		rv++;
		lTxt++;
		sz--;
		}
	}
return rv;
}



bool TGPSPCOMN::TxtToULong (char *lpRamData, unsigned char sz, uint32_t *lpDataOut)
{
bool rv = false;
if (lpRamData)
	{
	uint32_t Sizes = sz;
	if (!Sizes) Sizes = GetLenStr ((char*)lpRamData);
	if (CheckDecimal ((char*)lpRamData, Sizes) == Sizes)
		{
		if (Sizes < 11)
			{
			uint32_t Multer;
			uint32_t DigOut = 0;
			while (Sizes)
				{
				Multer = tmpMultData[Sizes-1];
				DigOut = DigOut + (Multer * (lpRamData[0] - '0'));
				lpRamData++;
				Sizes--;
				}
			if (lpDataOut) *lpDataOut = DigOut;
			rv = true;
			}
		}
	}
return rv;
}



bool TGPSPCOMN::get_latitude (float &val)
{
if (lat_static.f_actual) val = lat_static.data;
return lat_static.f_actual;
}



bool TGPSPCOMN::get_longitude (float &val)
{
if (lon_static.f_actual) val = lon_static.data;
return lon_static.f_actual;
}



bool TGPSPCOMN::get_altitude (float &val)
{
if (alt_static.f_actual) val = alt_static.data;
return alt_static.f_actual;
}



bool TGPSPCOMN::get_utc_hour (uint32_t &val)
{
if (utc_static.f_actual) val = utc_static.Hour;
return utc_static.f_actual;
}



bool TGPSPCOMN::get_utc_minute (uint32_t &val)
{
if (utc_static.f_actual) val = utc_static.Minute;
return utc_static.f_actual;
}



bool TGPSPCOMN::get_utc_second (float &val)
{
if (utc_static.f_actual) val = utc_static.seconds;
return utc_static.f_actual;
}



bool TGPSPCOMN::get_utd_day (uint32_t &val)
{
if (utd_static.f_actual) val = utd_static.Day;
return utd_static.f_actual;
}




bool TGPSPCOMN::get_utd_month (uint32_t &val)
{
if (utd_static.f_actual) val = utd_static.Month;
return utd_static.f_actual;
}



bool TGPSPCOMN::get_utd_year (uint32_t &val)
{
if (utd_static.f_actual) val = utd_static.Year;
return utd_static.f_actual;
}



bool TGPSPCOMN::get_satel_cnt_all (uint32_t &val)
{
if (satel_cnt_static_all.f_actual) val = satel_cnt_static_all.data;
return satel_cnt_static_all.f_actual;
}



bool TGPSPCOMN::get_satel_cnt_active (uint32_t &val)
{
if (satel_cnt_static_active.f_actual) val = satel_cnt_static_active.data;
return satel_cnt_static_active.f_actual;
}




static const unsigned long crc24q[256] = {
    0x00000000, 0x01864CFB, 0x028AD50D, 0x030C99F6,
    0x0493E6E1, 0x0515AA1A, 0x061933EC, 0x079F7F17,
    0x08A18139, 0x0927CDC2, 0x0A2B5434, 0x0BAD18CF,
    0x0C3267D8, 0x0DB42B23, 0x0EB8B2D5, 0x0F3EFE2E,
    0x10C54E89, 0x11430272, 0x124F9B84, 0x13C9D77F,
    0x1456A868, 0x15D0E493, 0x16DC7D65, 0x175A319E,
    0x1864CFB0, 0x19E2834B, 0x1AEE1ABD, 0x1B685646,
    0x1CF72951, 0x1D7165AA, 0x1E7DFC5C, 0x1FFBB0A7,
    0x200CD1E9, 0x218A9D12, 0x228604E4, 0x2300481F,
    0x249F3708, 0x25197BF3, 0x2615E205, 0x2793AEFE,
    0x28AD50D0, 0x292B1C2B, 0x2A2785DD, 0x2BA1C926,
    0x2C3EB631, 0x2DB8FACA, 0x2EB4633C, 0x2F322FC7,
    0x30C99F60, 0x314FD39B, 0x32434A6D, 0x33C50696,
    0x345A7981, 0x35DC357A, 0x36D0AC8C, 0x3756E077,
    0x38681E59, 0x39EE52A2, 0x3AE2CB54, 0x3B6487AF,
    0x3CFBF8B8, 0x3D7DB443, 0x3E712DB5, 0x3FF7614E,
    0x4019A3D2, 0x419FEF29, 0x429376DF, 0x43153A24,
    0x448A4533, 0x450C09C8, 0x4600903E, 0x4786DCC5,
    0x48B822EB, 0x493E6E10, 0x4A32F7E6, 0x4BB4BB1D,
    0x4C2BC40A, 0x4DAD88F1, 0x4EA11107, 0x4F275DFC,
    0x50DCED5B, 0x515AA1A0, 0x52563856, 0x53D074AD,
    0x544F0BBA, 0x55C94741, 0x56C5DEB7, 0x5743924C,
    0x587D6C62, 0x59FB2099, 0x5AF7B96F, 0x5B71F594,
    0x5CEE8A83, 0x5D68C678, 0x5E645F8E, 0x5FE21375,
    0x6015723B, 0x61933EC0, 0x629FA736, 0x6319EBCD,
    0x648694DA, 0x6500D821, 0x660C41D7, 0x678A0D2C,
    0x68B4F302, 0x6932BFF9, 0x6A3E260F, 0x6BB86AF4,
    0x6C2715E3, 0x6DA15918, 0x6EADC0EE, 0x6F2B8C15,
    0x70D03CB2, 0x71567049, 0x725AE9BF, 0x73DCA544,
    0x7443DA53, 0x75C596A8, 0x76C90F5E, 0x774F43A5,
    0x7871BD8B, 0x79F7F170, 0x7AFB6886, 0x7B7D247D,
    0x7CE25B6A, 0x7D641791, 0x7E688E67, 0x7FEEC29C,
    0x803347A4, 0x81B50B5F, 0x82B992A9, 0x833FDE52,
    0x84A0A145, 0x8526EDBE, 0x862A7448, 0x87AC38B3,
    0x8892C69D, 0x89148A66, 0x8A181390, 0x8B9E5F6B,
    0x8C01207C, 0x8D876C87, 0x8E8BF571, 0x8F0DB98A,
    0x90F6092D, 0x917045D6, 0x927CDC20, 0x93FA90DB,
    0x9465EFCC, 0x95E3A337, 0x96EF3AC1, 0x9769763A,
    0x98578814, 0x99D1C4EF, 0x9ADD5D19, 0x9B5B11E2,
    0x9CC46EF5, 0x9D42220E, 0x9E4EBBF8, 0x9FC8F703,
    0xA03F964D, 0xA1B9DAB6, 0xA2B54340, 0xA3330FBB,
    0xA4AC70AC, 0xA52A3C57, 0xA626A5A1, 0xA7A0E95A,
    0xA89E1774, 0xA9185B8F, 0xAA14C279, 0xAB928E82,
    0xAC0DF195, 0xAD8BBD6E, 0xAE872498, 0xAF016863,
    0xB0FAD8C4, 0xB17C943F, 0xB2700DC9, 0xB3F64132,
    0xB4693E25, 0xB5EF72DE, 0xB6E3EB28, 0xB765A7D3,
    0xB85B59FD, 0xB9DD1506, 0xBAD18CF0, 0xBB57C00B,
    0xBCC8BF1C, 0xBD4EF3E7, 0xBE426A11, 0xBFC426EA,
    0xC02AE476, 0xC1ACA88D, 0xC2A0317B, 0xC3267D80,
    0xC4B90297, 0xC53F4E6C, 0xC633D79A, 0xC7B59B61,
    0xC88B654F, 0xC90D29B4, 0xCA01B042, 0xCB87FCB9,
    0xCC1883AE, 0xCD9ECF55, 0xCE9256A3, 0xCF141A58,
    0xD0EFAAFF, 0xD169E604, 0xD2657FF2, 0xD3E33309,
    0xD47C4C1E, 0xD5FA00E5, 0xD6F69913, 0xD770D5E8,
    0xD84E2BC6, 0xD9C8673D, 0xDAC4FECB, 0xDB42B230,
    0xDCDDCD27, 0xDD5B81DC, 0xDE57182A, 0xDFD154D1,
    0xE026359F, 0xE1A07964, 0xE2ACE092, 0xE32AAC69,
    0xE4B5D37E, 0xE5339F85, 0xE63F0673, 0xE7B94A88,
    0xE887B4A6, 0xE901F85D, 0xEA0D61AB, 0xEB8B2D50,
    0xEC145247, 0xED921EBC, 0xEE9E874A, 0xEF18CBB1,
    0xF0E37B16, 0xF16537ED, 0xF269AE1B, 0xF3EFE2E0,
    0xF4709DF7, 0xF5F6D10C, 0xF6FA48FA, 0xF77C0401,
    0xF842FA2F, 0xF9C4B6D4, 0xFAC82F22, 0xFB4E63D9,
    0xFCD11CCE, 0xFD575035, 0xFE5BC9C3, 0xFFDD8538,
};




TUBXNMRTCMUX::TUBXNMRTCMUX (uint16_t ff_rx_cnt, TGPSSTATS *gpsstat): C_GPS_LINEBUF_SIZE (ff_rx_cnt)
{
statistics = gpsstat;
linebuf = new uint8_t[ff_rx_cnt];
push_ix = 0;
locfpscnt_lasttick = GetTickCount ();
rsltfps_nmea = 0;
rsltfps_ubx = 0;
rsltfps_rtcm3 = 0;

locfpscnt_nmea = 0;
locfpscnt_ubx = 0;
locfpscnt_rtcm3 = 0;

clear_peack_buf_statistics ();
}



void TUBXNMRTCMUX::clear_peack_buf_statistics ()
{
cur_peack_rx_buf = 0;
}


uint32_t TUBXNMRTCMUX::get_peack_buf_size ()
{
return cur_peack_rx_buf;
}




bool TUBXNMRTCMUX::push_raw (uint8_t dat)
{
bool rv = false;
if (push_ix < C_GPS_LINEBUF_SIZE)
    {
    linebuf[push_ix++] = dat;
    rv = true;
    }
return rv;
}



bool TUBXNMRTCMUX::CalculateUBX_CRC (unsigned char *lSrc, unsigned short sz, TUBXCRC *lDstCrc)
{
	bool rv = false;
	unsigned char CK_A = 0;
	unsigned char CK_B = 0;
	if (lSrc)
		{
		while (sz )
			{
			CK_A = CK_A + lSrc[0];
			CK_B = CK_B + CK_A;
			lSrc++;
			sz--;
			}
		rv = true;
		}
		
	if (lDstCrc)
		{
		lDstCrc->CK_A = CK_A;
		lDstCrc->CK_B = CK_B;
		}
return rv;		
}



char *TUBXNMRTCMUX::FindFistDelimitter (char *lInpInBuf, uint32_t size, char delimm)
{
char *lpRv = 0;

char *lEnd = (char*)linebuf + C_GPS_LINEBUF_SIZE;
char  dat;
while ((lEnd > lInpInBuf) && size)
    {
    dat = lInpInBuf[0];
    if (!dat) break;
    if (dat == delimm)
        {
        lpRv = lInpInBuf;
        break;
        }
    lInpInBuf++;
    size--;
    }

return lpRv;
}




static unsigned char HexCharToBin (unsigned char datas)
{
unsigned char rv=0;
while (1)
        {
        if (datas>=97 && datas<=102)
                {
                rv=10+(datas-97);
                break;
                }
        if (datas>=65 && datas<=70)
                {
                rv=10+(datas-65);
                break;
                }
        if (datas>=48 && datas<=57)
                {
                rv=datas-48;
                break;
                }
        break;
        }
return rv;
}



static unsigned char CHex2Val (unsigned char *lpRams)
{
unsigned char rv = (HexCharToBin(lpRams[0]) << 4);
rv = rv | HexCharToBin(lpRams[1]);
return rv;
}






bool TUBXNMRTCMUX::check_nmea_linedata (uint32_t &ix_start, uint32_t &mess_size)
{
bool rv = false;
	// находим разделители MNEA сообщений
    if (push_ix)
        {
        unsigned char *lStart = FindFistDelimitter (linebuf, push_ix, '$');

        if (lStart)
            {
            uint32_t fsize = lStart - linebuf;
            if (push_ix >= fsize)
                {
                unsigned char *lStop = FindFistDelimitter (lStart, push_ix - fsize , '*');       // fsize
                if (lStop)
                    {

                    unsigned short full_mess_size = lStop - lStart + 3;  // + 1 + 2byte crc
                    if ((lStop + 2) <= &linebuf[push_ix])  // crc in bufer ?
                        {
                        unsigned char CRCX = 0;
                        unsigned char *lCur = (unsigned char *)lStart + 1;
                        unsigned short crcsz = lStop - lStart - 1;// full_mess_size - 4;
                        // считаем контрольную сумму NMEA
                        while (crcsz)
                            {
                            CRCX = CRCX ^ lCur[0];
                            lCur++;
                            crcsz--;
                            }
                        unsigned char dat = CHex2Val ((unsigned char *)lStop + 1);
                        if (dat == CRCX)
                            {
                            mess_size = full_mess_size;
                            ix_start = lStart - linebuf;
                            rv = true;
                            }
                        }
                    }
                }
            }
        }
return rv;
}




bool TUBXNMRTCMUX::check_ubx_linedata (uint32_t &ix_start, uint32_t &mess_size)
{
bool rv = false;
uint32_t ix = 0, datasize;
HDRUBX *lUbx;

while (ix < push_ix)
    {
    datasize = push_ix - ix;
    lUbx = (HDRUBX*)&linebuf[ix];
    if (datasize > (sizeof(HDRUBX) + sizeof(TUBXCRC)))
        {
        if (lUbx->Preamble_A == CUBXPREX_A && lUbx->Preamble_B == CUBXPREX_B)
            {
            unsigned short full_len = sizeof(HDRUBX) + lUbx->Len + sizeof(TUBXCRC);   // + 2 byte crc
            if (datasize >= full_len)
                {
                // проверка контрольной суммы
                TUBXCRC ubx_Crc;
                CalculateUBX_CRC ((uint8_t*)&lUbx->Class, full_len - 4, &ubx_Crc);  // - 2 bytes preamble, -2 bytes crc
                TUBXCRC *lfnd_Crc = (TUBXCRC*)(((uint8_t*)lUbx) + sizeof(HDRUBX) + lUbx->Len);
                if (lfnd_Crc->CK_A == ubx_Crc.CK_A && lfnd_Crc->CK_B == ubx_Crc.CK_B)
                    {
                    ix_start = ix;
                    mess_size = full_len;
                    rv = true;
                    break;
                    }
                }
            else
                {
                // малый размер данных
                break;
                }
            }
        }
    else
        {
        // малый размер данных
        break;
        }
    ix++;
    }
return rv;
}



bool TUBXNMRTCMUX::check_rtcm_linedata (uint32_t &ix_start, uint32_t &mess_size)
{
bool rv = false;
HDRTRANPLAYRTCM3 *lframe;
uint32_t ix = 0, datasize;
while (ix < push_ix)
    {
    datasize = push_ix - ix;
    lframe = (HDRTRANPLAYRTCM3*)&linebuf[ix];

    // предварительная проверка наличия целого пакета в буфере
    if (datasize > (sizeof(HDRTRANPLAYRTCM3) + C_RTCM3_CRC24_BSIZE))  // 3 byte header + 3 byte qcrc
        {
        if (lframe->Preamble_D3 == 0xD3)
            {
            unsigned short len_raw = SwapShort (lframe->len_raw);
            if (len_raw && len_raw < 1023)	// 6 reserved + 10 bits size
                {
                unsigned short len = sizeof(HDRTRANPLAYRTCM3) + len_raw;
                unsigned short full_len = len + C_RTCM3_CRC24_BSIZE;      // + 3 byte qcrc
                // финишная проверка наличия целого пакета в буфере
                if (datasize >= full_len)
                    {
                    unsigned long cur_crc = crc24q_hash ((unsigned char*)lframe, len);
                    if (compareLong24AndBuf (cur_crc, ((unsigned char*)lframe) + len))
                        {
                        mess_size = full_len; 
                        ix_start = ix;
                        rv = true;
                        break;
                        }
                    }
                else
                    {
                    // size is small
                    break;
                    }
                }
            }
        }
    else
        {
        // size is small
        break;
        }
    ix++;
    }
return rv;
}



void TUBXNMRTCMUX::In_raw (uint8_t *data, uint16_t sizes)
{
	while (sizes)
		{
        if (!push_raw (*data++))
            {
            push_ix = 0;
            break;
            }
		//if (!fifo_RX->push (data)) break;
		//data++;
		sizes--;
		}
}



void TUBXNMRTCMUX::Task ()
{
uint8_t dat;
uint32_t ticks = GetTickCount ();

    uint32_t ix_start, find_size;
    bool f_detect;
    if (push_ix >= C_GPS_LINEBUF_SIZE) push_ix  = 0;
    if (cur_peack_rx_buf < push_ix) cur_peack_rx_buf = push_ix;
    while (push_ix)
        {
            ix_start = 0, find_size = 0;
            f_detect = false;
            do  {
                if (check_nmea_linedata (ix_start, find_size))
                    {
                    locfpscnt_nmea++;   // fps counter

                        if (find_size > 4)  // +$ +* +crc
                            {
                            uint32_t l_size = find_size - 4;
                            uint8_t *adr = &linebuf[ix_start];
                            adr += 1;       // + 1byte '$'
                            cb_nmea_data (adr, l_size);
                            if (statistics) statistics->update_nmea (adr);
                            }

                    f_detect = true;
                    break;
                    }

                if (check_ubx_linedata (ix_start, find_size))
                    {
                    locfpscnt_ubx++;        // fps counter

                        if (find_size > (sizeof(HDRUBX) + sizeof(TUBXCRC)))
                            {
                            uint32_t l_size = find_size - sizeof(TUBXCRC);
                            HDRUBX *adr = (HDRUBX*)&linebuf[ix_start];
                            cb_ubx_data (adr, l_size);
                            if (statistics)
                                {
                                S_UBXTG param;
                                param.clas = adr->Class;
                                param.id = adr->Id;
                                statistics->update_ubx (&param);
                                }
                            }

                    f_detect = true;
                    break;
                    }

                if (check_rtcm_linedata (ix_start, find_size))
                    {
                    locfpscnt_rtcm3++;      // fps counter

                        if (find_size > (sizeof(HDRTRANPLAYRTCM3) + C_RTCM3_CRC24_BSIZE))  // + 1=d3, +2=len, +3=bytes crc
                            {
                            uint32_t l_size = find_size - (sizeof(HDRTRANPLAYRTCM3) + C_RTCM3_CRC24_BSIZE);    // +3 bytes header + 3bytes crc
                            uint8_t *adr = &linebuf[ix_start];
                            adr += sizeof(HDRTRANPLAYRTCM3);   // +3 bytes header
                            cb_rtcm3_data (adr, l_size);
                            if (statistics) statistics->update_rtcm3 (100);
                            }

                    f_detect = true;
                    break;
                    }

                } while (false);

          if (f_detect)
              {
              // проверяем нужно ли перемещение оставшихся данных в начало
              uint32_t last_ix = ix_start + find_size;

              if (last_ix <= push_ix)
                  {
                  uint32_t tailsize = push_ix - last_ix;//find_size;//last_ix;
                  CopyMemorySDC ((char*)&linebuf[last_ix], (char*)linebuf, tailsize);
                  push_ix = tailsize;
                  }
              else
                {
                // непредвиденная ошибка
                push_ix = 0;

                }
              }
          else
            {
            break;
            }


        }

uint32_t delt = ticks - locfpscnt_lasttick;
if (delt >= 1000)
    {
    float mult = 1000.0 / delt;

    float val = locfpscnt_nmea;
    rsltfps_nmea = (rsltfps_nmea + (val * mult)) / 2;

    val = locfpscnt_ubx;
    rsltfps_ubx = (rsltfps_ubx + (val * mult)) / 2;

    val = locfpscnt_rtcm3;
    rsltfps_rtcm3 = (rsltfps_rtcm3 + (val * mult)) / 2;

    locfpscnt_nmea = 0;
    locfpscnt_ubx = 0;
    locfpscnt_rtcm3 = 0;

    locfpscnt_lasttick = ticks;
    }

}




bool TUBXNMRTCMUX::compareLong24AndBuf (unsigned long curdata, unsigned char *lTxt)
{
bool rv = false;
if ((curdata & 0xFF) == lTxt[2] && ((curdata >> 8) & 0xFF) == lTxt[1] && ((curdata >> 16) & 0xFF) == lTxt[0]) rv = true;
return rv;	
}




unsigned long TUBXNMRTCMUX::crc24q_hash(unsigned char *data, unsigned short len)
{
    short i;
    unsigned long crc = 0;
    for (i = 0; i < len; i++) {
			crc = (crc << 8) ^ crc24q[data[i] ^ (unsigned char)(crc >> 16)];
			}
    crc = (crc & 0x00ffffff);
    return crc;
}


float TUBXNMRTCMUX::get_nmea_fps ()
{
return rsltfps_nmea;
}



float TUBXNMRTCMUX::get_ubx_fps ()
{
return rsltfps_ubx;
}



float TUBXNMRTCMUX::get_rtcm3_fps ()
{
return rsltfps_rtcm3;
}




void TUBXNMRTCMUX::cb_ubx_data (HDRUBX *inp, uint32_t size)
{
}



void TUBXNMRTCMUX::cb_nmea_data (uint8_t *inp, uint32_t size)
{
}



void TUBXNMRTCMUX::cb_rtcm3_data (uint8_t *inp, uint32_t size)
{
}






