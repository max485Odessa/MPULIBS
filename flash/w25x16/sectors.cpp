#include "sectors.h"



const char *lPassSectors = "lidar_shasi_pow";

static const unsigned char C_8BIT_POL = 0x41;
static const unsigned long C_BITMASK_HB = 0x80000000;


unsigned int GetLenStr (char *lpAdr)
{
unsigned int rv = 0;
if (lpAdr)
	{
	while (true)
			{
			if (!lpAdr[0]) break;
			rv++;
			lpAdr++;
			}
	}
return rv;
}



unsigned long CalcCRC32Data (unsigned char *lSrc, unsigned long siz)
{
	unsigned long rv = 0;
	unsigned char dat;
	unsigned long z_cnt = 0;
	while (siz)
		{
		dat = *lSrc;
		if (!dat)
			{
			rv += siz;
			z_cnt++;
			}
		else
			{
			if (dat & C_8BIT_POL)
				{
				rv += 0x24;
				}
			else
				{
				rv += 0x33;
				}
			}
		if (rv & C_BITMASK_HB)
			{
			rv <<= 1;
			rv |= 1;
			}
		else
			{
			rv <<= 1;
			}
		rv += dat;
		lSrc++;
		siz--;
		}
	rv += z_cnt;
return rv;
}


unsigned long RamCodingPink (unsigned char *lpRamInput, unsigned char *lpRamOutput,unsigned long sizesdata,unsigned char *lpPassCode)
{
unsigned long rv = 0;
if (lpRamInput && lpRamOutput && sizesdata)
	{
	unsigned char *lpCurPassdata=lpPassCode;
	unsigned long LenPassword;
	unsigned char curbyted;
	unsigned char maskleft=1;
	unsigned char maskright=128;
	if (!lpPassCode)
					{
					LenPassword = 0;
					}
	else
					{
					LenPassword = GetLenStr ((char *)lpPassCode);
					}
	while (sizesdata)
		{
		curbyted=lpRamInput[0];

		if (LenPassword)
			{
			curbyted=curbyted ^ lpCurPassdata[0];
			lpCurPassdata++;
			if (!lpCurPassdata[0]) lpCurPassdata=lpPassCode;
			}

		curbyted=curbyted ^ maskright;
		curbyted=curbyted ^ maskleft;

		maskright=maskright >> 1;
		if (!maskright) maskright=128;
		maskleft=maskleft <<1;
		if (!maskleft) maskleft=1;

		curbyted=curbyted ^ (unsigned char)rv;

		lpRamOutput[0] = curbyted;
		lpRamOutput++;
		lpRamInput++;

		sizesdata--;
		rv++;
		}
	}
return rv;
}



// наличие firmware во флеш с коректной контрольной суммой
bool ExternalFirmware_Check (TWINBOND25X16 &EFlsh, unsigned long Adr)
{
	bool rv = false;
	unsigned long CurExAdr = Adr;
	static FULLCHANK chank;		// сделал статическими
	USERDATA usdat;
	// считываем и проверяем начальный заголовок firmware
	EFlsh.BufferRead ((u8*)&usdat, CurExAdr, sizeof(usdat));	// загружает только полезные данные для экономии RAM (без текста)
	if (usdat.Signature == C_SIGNATURE_BINFILE)
		{
		if (usdat.sizes && usdat.sizes <= C_MAXPROGRAM_SIZE)
			{
			unsigned long calc_crc_code = 0, tmp_crc;
			CurExAdr += sizeof(FMWHEADER);	// за USERDATA, следует область произвольного текста
			unsigned char indx = 0;
			bool f_error = false;
			
			unsigned long ContrSize = 0;
			// читаем и проверяем все секторные записи, подсчитываем crc
			while (indx < C_MPRGSECTOR_SIZE && ContrSize < usdat.sizes)
				{
				EFlsh.BufferRead ((u8*)&chank, CurExAdr, sizeof(chank));
				if (chank.inf.SizeData > C_CHANK_SIZE)
					{
					// указанный размер больше размера сектора
					f_error = true;
					break;
					}
				tmp_crc = CalcCRC32Data ((unsigned char*)chank.raw, chank.inf.SizeData);
				if (tmp_crc != chank.inf.CRC32_CODE)
					{
					// контрольная сумма данных не совпадает
					f_error = true;
					break;
					}
				calc_crc_code += tmp_crc;
				ContrSize += chank.inf.SizeData;
				CurExAdr += sizeof(chank);
				indx++;
				}
			if (!f_error)
				{
				if (usdat.CRC32_CODE == calc_crc_code) rv = true;	// общая контрольная сумма блоков - совпала
				}
			}
		}
	return rv;
}






