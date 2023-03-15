#include "rutine.h"
#include "stddef.h"
#include "stdint.h"

const static unsigned long tmpMultData[10] = {1,10,100,1000,10000,100000,1000000,10000000,100000000,1000000000};
extern uint32_t SystemCoreClock; 


void HardwareReset ()
{
typedef void (*cb_reset) (void);	
	cb_reset reset = 0;
	reset ();
}


unsigned short SwapShort (unsigned short dat)
{
	unsigned char hibyte = (dat & 0xff00) >> 8;
	unsigned char lobyte = (dat & 0xff);
	unsigned short rv = lobyte;	rv = rv << 8; rv |= hibyte;
	return rv;
}



char *CopyMemorySDC (char *lpSour, char *lpDest, unsigned int sizes)
{
if (lpSour && lpDest && sizes)
    {
    while (sizes)
        {
        lpDest[0]=lpSour[0];
        lpDest++;
        lpSour++;
        sizes--;
        }
    }
return lpDest;
}



unsigned long lenstr (const char *lsrt)
{
	unsigned long sz = 0;
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



void *memset (void *lDst, unsigned char dat, long sizes)
{
char *lRam = (char*)lDst;
while (sizes)
	{
	*lRam = dat;
	lRam++;
	sizes--;
	}
return lRam;
}



long strncmp( const char * string1, const char * string2, long num)
{
	long rv = 0;
	char d1, d2;
	if (num > 0)
		{
		while (num)
			{
				d1 = string1[0];
				d2 = string2[0];
			if (d1 != d2)
				{
				if (d1 > d2)
					{
					rv = 1;
					}
				else
					{
					rv = -1;
					}
				break;
				}
			string1++;
			string2++;
			num--;
			}
		}
	return rv;
}


unsigned short abs16 (short datas)
{
unsigned short rv=(unsigned short)datas;
if (rv>=32768)
	{
	rv=(rv ^ 0xFFFF)+1;
	}
return rv;
}



unsigned long abs32 (long datas)
{
unsigned long rv=(unsigned long)datas;
if (rv>=0x80000000)
	{
	rv=(rv ^ 0xFFFFFFFF)+1;
	}
return rv;
}



bool str_compare (char *lStr1, char *lStr2, unsigned long size)
{
	bool rv = false;
	if (size)
		{
		rv = true;
		while (size)
			{
			if (lStr1[0] != lStr2[0])
				{
				rv = false;
				break;
				}
			lStr1++;
			lStr2++;
			size--;
			}
		}
	return rv;
}



unsigned long str_size (const char *lsrt)
{
	unsigned long sz = 0;
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


/*
void *memcpy ( void * destination, const void * source, long num )
{
	char *lDst = (char*)destination, *lSrc = (char*)source;
while (num)
	{
	*lDst++ = lSrc[0];
	lSrc++;
	num--;
	}
return lDst;
}
*/



unsigned long HAL_RCC_GetPCLK1Freq ()
{
	return SystemCoreClock / 2;
}



unsigned long GetCountStringElementDelim ( char *lpAdrStr, char DelimCode )
{
unsigned long rv=0;
if (lpAdrStr)
	{
	char datas;
	bool F_dat = false;
  while (true)
		{
    datas = lpAdrStr[0];
		if (!datas)
			{
      if (F_dat) rv++;
      break;
      }
    else
			{
			if (datas == 13 || datas == 10)
				{
				rv++;
				break;
				}
			else
				{
				if (datas == DelimCode)
					{
					rv++;
					F_dat = false;
					}
				else
					{
					F_dat = true;
					}
				}
			}
		lpAdrStr++;
		}
	}
return rv;
}



// Возвращает указатель вхождения на нужный стринг по указаному индексу, возвращает размер текстового поля по этому индексу.
// Можно вычислить количество полей записи указав большой индекс поиска, количество полей возвращается по указателю lPCountField,
unsigned long GetTagStringDelimIndx (char *lpLinetxt, unsigned long Indxx, char delimc, char **lDest, unsigned long *lPCountField)
{
unsigned long rv = 0;
if (lpLinetxt)
		{
		unsigned long size = 0;
		unsigned long lcinx = 0;
		unsigned long countfield = 0;
		char *lFistInp = 0;
		char tmpb;
		bool f_fist_input = false;
		while (true)
				{
				tmpb = lpLinetxt[0];
				if (tmpb == 9) tmpb = 32;
				if (!tmpb || tmpb == 13 || tmpb == 10)
					{
					if (Indxx != lcinx) f_fist_input = false;
					countfield++;
					break;
					}
				else
					{
					if (tmpb == delimc)
						{
						countfield++;
						if (Indxx == lcinx) 
							{
							break;
							}
						lcinx++;
						f_fist_input = false;
						lFistInp = 0;
						size = 0;
						}
					else
						{
						if (f_fist_input == false)
							{
							lFistInp = lpLinetxt;
							f_fist_input = true;
							}
						size++;
						}
					}
				lpLinetxt++;
				}
		if (!f_fist_input)
			{
			size = 0;
			lFistInp = 0;
			}
		if (lDest) *lDest = lFistInp;
		if (lPCountField) *lPCountField = countfield;
		rv = size;     
		}
return rv;
}





unsigned long CheckDecimal (char *lTxt, unsigned long sz)
{
unsigned long rv = 0;
if (lTxt && sz)
	{
	unsigned char datt;
		if (!sz) sz = str_size (lTxt);
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



unsigned long GetSizeToDelim (char *lTxt, unsigned long sz, char delim)
{
unsigned long rv = 0;
if (lTxt)
	{
	char dat;
	while (sz)
		{
		dat = *lTxt;
		if (!dat || dat == delim) break;
		rv++;
		lTxt++;
		sz--;
		}
	}
return rv;
}




bool TxtToULong (char *lpRamData, unsigned char sz, unsigned long *lpDataOut)
{
bool rv = false;
if (lpRamData)
	{
	unsigned long Sizes = sz;
	if (!Sizes) Sizes = str_size ((char*)lpRamData);
	if (CheckDecimal ((char*)lpRamData, Sizes) == Sizes)
		{
		if (Sizes < 11)
			{
			unsigned long Multer;
			unsigned long DigOut = 0;
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



bool TxtToFloat (float *lpDest, char *lpTxtIn, unsigned long Sizes)
{
bool rv = false;
if (lpDest && lpTxtIn)
	{
	if (!Sizes) Sizes = str_size (lpTxtIn);
	if (Sizes && Sizes < 32)
		{
		float Datf = 0;
		unsigned long NLongDroba = 0;
		unsigned long NLongCel = 0;
		unsigned long len_drb = 0,len_cel = 0;
		bool f_sig = false;
		if (lpTxtIn[0] == '-')
			{
			f_sig = true;
			lpTxtIn++;
			Sizes--;
			}
		if (Sizes)
			{
			len_cel = GetSizeToDelim (lpTxtIn,Sizes,'.');
			if (len_cel)
					{
					len_drb = Sizes - len_cel;
					if (len_drb)
							{
							len_drb--;
							if (!len_drb) return rv;
							if (!TxtToULong ((lpTxtIn + len_cel + sizeof('.')), len_drb, &NLongDroba)) return rv;
							}
					if (!TxtToULong (lpTxtIn, len_cel, &NLongCel)) return rv;
					Datf = NLongCel;
					if (len_drb)
							{
							float mult = ((float)(1.0)) / tmpMultData[len_drb];
							float drba = NLongDroba;
							Datf = Datf + (drba * mult) + 0.0000005;
							}
					if (f_sig) Datf *= (-1.0);
					lpDest[0] = Datf;
					rv = true;
					}
			 }
		}
	}
return rv;
}



unsigned char *UlongToStr (unsigned char *lpDest, unsigned long datas)
{
unsigned char flagnz=0, resd;
unsigned long delmt = 1000000000;
while (1)
	{
	if (datas>=delmt)
		{
		flagnz=1;
		resd=(datas/delmt);
		datas=datas % delmt;
		}
	else
		{
		resd=0;
		}
	if (flagnz)
		{
		lpDest[0]=resd+48;
		lpDest++;
		}
	delmt=delmt/10;

	if (delmt<=1) 
		{
		lpDest[0]=datas+48;
		lpDest++;
        lpDest[0] = 0;
		break;
		}
	}
return lpDest;
}



unsigned char *LongToStr (unsigned char *lpDest, long datas)
{
if (lpDest)
	{
	bool m_state = false;
	unsigned long val;
	if (datas < 0)
		{
		m_state = true;
		val = abs32 (datas);
		}
	else
		{
		val = datas;
		}
	if (m_state)
		{
		lpDest[0] = '-'; 
		lpDest++;
		}
	lpDest = UlongToStr (lpDest, val);
	}
return lpDest;
}



static unsigned char *FloatToStrDroba (unsigned char *lpDest,float datas,unsigned char Cntr)
{
unsigned char resd;
float delmt=0.1;
while (Cntr)
	{
	if (datas>=delmt)
		{
		resd=(datas/delmt);
		datas = datas - (delmt*resd);
		}
	else
		{
		resd=0;
		}

	lpDest[0]=resd+48;
	lpDest++;

	delmt = delmt/10;

	Cntr--;
	}
lpDest[0] = 0;
return lpDest;
}




char *FloatToStr (char *lDst, float val, unsigned char dig_n)
{
char *lrv = lDst;
if (lDst)
	{
	long Celie = val;
	lrv = (char *)LongToStr ((unsigned char*)lDst, Celie);
	
	if (dig_n)
		{
		if (dig_n > 6) dig_n = 6;
		float DrobOstat = val - Celie;

		lrv[0] = '.';
		lrv++;
		lrv = (char *)FloatToStrDroba ((unsigned char *)lrv, DrobOstat, dig_n);
		}
	}
return lrv;
}



unsigned long CreateValueFromBitMassive (unsigned char *lInp, unsigned long CurOffset, unsigned char databitsize)
{
unsigned long rv = 0;
if (lInp  && databitsize && databitsize <= 32)
	{
	unsigned long byte_ofs = CurOffset / 8;
	unsigned char bit_msk_input = 128;
	unsigned long bit_msk_output = 1;
	bit_msk_output = bit_msk_output << (databitsize - 1);
		
  bit_msk_input = bit_msk_input >> (CurOffset % 8);
  lInp += byte_ofs;

	while (databitsize)
		{
		if (lInp[0] & bit_msk_input) rv |= bit_msk_output;
		bit_msk_input = bit_msk_input >> 1;
		if (!bit_msk_input)
			{
			bit_msk_input = 128;
			lInp++;
			}
		bit_msk_output = bit_msk_output >> 1;
		if (!bit_msk_output) bit_msk_output = 0x80000000;
		databitsize--;
		}
	}
return rv;
}



unsigned long SwapBitsLong (unsigned long val, unsigned char bitsize)
{
unsigned long rv = 0;
if (bitsize)
	{
	if (bitsize == 1)
		{
		rv = val;
		}
	else
		{
		unsigned long dst_mask = 1;
		unsigned long src_mask = 1;
		src_mask = src_mask << (bitsize - 1);
		while (bitsize)
			{
			if (val & src_mask) rv |= dst_mask;
			dst_mask <<= 1;
			src_mask >>= 1;
			bitsize--;
			}
		}
	}
return rv;
}



unsigned char SWAPBits (unsigned char datas)
{
unsigned char rv=0,cnt7=7;
while (cnt7)
        {
        if (datas & 1) rv=rv | 1;
        datas=datas >> 1;
        rv=rv << 1;
        cnt7--;
        }
if (datas & 1) rv=rv | 1;
return rv;
}



