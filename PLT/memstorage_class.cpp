#include "memstorage_class.h"


TPARAMSAVE TEASYMEMSTORAGE::data;
TWINBOND25X16 *TEASYMEMSTORAGE::lFlash = 0;
bool TEASYMEMSTORAGE::f_bad_setings = false;
bool TEASYMEMSTORAGE::f_need_save = false;



TEASYMEMSTORAGE::TEASYMEMSTORAGE ()
{
}



bool TEASYMEMSTORAGE::BadCRC_check ()
{
	return f_bad_setings;
}




void TEASYMEMSTORAGE::Init (TWINBOND25X16 *m)
{
	lFlash = m;
	Load ();
}



void TEASYMEMSTORAGE::Default_sets ()
{
data.params.Uav_self_id = C_DEF_NAVINID_ADR;
data.params.ADSB_on = 1;
data.params.ADSB_rate = C_ADSB_RATE_DEF;
data.params.GPS_aux_rate = C_AUX_RATE_DEF;
data.params.GPS_fix_rate = C_FIX_RATE_DEF;
data.params.GPS_on = 1;
data.params.SBUS_PPM_on = 1;		// 0 - off, 1 - sbus, 2 - ppm
data.params.SBUS_rate = C_SBUS_RATE_DEF;


data.params.link_control_adr = 10;	
data.params.adsb_test_time = C_ADSB_TIME_TEST_DEF;
	
}



void TEASYMEMSTORAGE::Task ()
{

}



uint32_t TEASYMEMSTORAGE::GetArrAdr (uint8_t arr_indx)
{
	uint32_t Base_f = C_FLASH_INI_ADRESS_A;
	if (arr_indx < C_UAVPARAM_COPY_AMOUNT) Base_f += (0x1000 * arr_indx);
	return Base_f;
}



bool TEASYMEMSTORAGE::SaveToExtFlash (uint8_t inx_copy)
{
	bool rv = false;
	if (lFlash)
		{
		uint32_t BaseDst = GetArrAdr (inx_copy);
		lFlash->SectorErase (BaseDst, 1);
		// запись размера
		S_UAVPAR hdr;
		hdr.arr_item = ESAVEPARIX_ENDENUM;
			
		lFlash->BufferWrite ((u8*)&hdr, BaseDst, sizeof(hdr));
		BaseDst += sizeof(hdr);
		lFlash->BufferWrite ((u8*)&data, BaseDst, sizeof(data));
		rv = true;
		}
	return rv;
}



uint32_t TEASYMEMSTORAGE::CalculateCRC (uint8_t *lSrc, uint32_t sz)
{
	uint32_t rv = 0;
	uint8_t dat, mask_b;
	
	while (sz)
		{
		mask_b = !!(rv & 0x80000000);
		rv <<= 1; rv |= mask_b;
		dat = *lSrc;
		if (!dat)
			{
			if (sz & 1)
				{
				rv += 0x26;
				}
			else
				{
				rv += 0x82;
				}
			}
		else
			{
			rv += dat;
			}
		lSrc++;
		sz--;
		}
	if (!rv) rv = 0x12345678;
	return rv;
}



bool TEASYMEMSTORAGE::Save ()
{
bool rv = false;
	if (lFlash && f_need_save)
		{
		data.CRC32 = CalculateCRC ((uint8_t*)&data, sizeof(data) - sizeof(long));
		uint8_t indx = 0;
		while (indx < C_UAVPARAM_COPY_AMOUNT)
			{
			SaveToExtFlash (indx);
			indx++;
			}
		f_need_save = false;
		rv = true;
		}
return rv;
}



// проверка целостности записи в одной из 3x копий
bool TEASYMEMSTORAGE::CheckCRCExtFlashCopy (unsigned char arr_indx)
{
bool rv = false;
	if (arr_indx < C_UAVPARAM_COPY_AMOUNT && lFlash)
		{
		S_UAVPAR par_c;					// количество параметров

		uint32_t Base_f = GetArrAdr (arr_indx);
			
		lFlash->BufferRead ((u8*)&par_c, Base_f, sizeof(par_c));				// загружаетм из внешней памяти
		Base_f += sizeof(par_c);
		if (par_c.arr_item == ESAVEPARIX_ENDENUM)
			{
			//uint8_t inx = 0;
			//bool rslt = true;
			TPARAMSAVE bufdata;
			lFlash->BufferRead ((u8*)&bufdata, Base_f, sizeof(bufdata));
			if (bufdata.CRC32 == CalculateCRC ((uint8_t*)&bufdata, sizeof(bufdata) - sizeof(long))) rv = true;
			}
		}
return rv;
}



bool TEASYMEMSTORAGE::LoadFromExtFlash (uint8_t inx_copy)
{
	bool rv = false;
	if (lFlash)
		{
		uint32_t BaseDst = GetArrAdr (inx_copy);

		S_UAVPAR hdr;
		lFlash->BufferRead ((u8*)&hdr, BaseDst, sizeof(hdr));
		if (hdr.arr_item == ESAVEPARIX_ENDENUM)
			{
			BaseDst += sizeof(hdr);
			lFlash->BufferRead ((u8*)&data, BaseDst, sizeof(data));
			// перенести указатели params.name из дефолтной записи
			if (data.CRC32 == CalculateCRC ((uint8_t*)&data, sizeof(data)- sizeof(long))) rv = true;
			}
		}
	return rv;
}



bool TEASYMEMSTORAGE::Load ()
{
	bool rv = false;
	if (lFlash)
		{
		unsigned char indx = 0;
		bool f_need_def = true;
		while (indx < C_UAVPARAM_COPY_AMOUNT)
			{
			if (CheckCRCExtFlashCopy (indx))
				{
				f_need_def = !LoadFromExtFlash (indx);
				break;
				}
			indx++;
			}
		if (f_need_def) 
			{
			Default_sets ();
			f_need_save = true;
			}
		else
			{
			rv = true;
			}
		}
	return rv;
}



void TEASYMEMSTORAGE::SaveParam (ESAVEPARIX ix, TSPDATA src_dat, bool f_phisical_save)
{
	if (ix < ESAVEPARIX_ENDENUM && src_dat)
		{
		CopyMemorySDC ((char*)src_dat, (char*)&((unsigned long *)&data.params)[ix], sizeof(float));

		f_need_save = true;
		if (f_phisical_save) Save ();
		}
}





unsigned char TEASYMEMSTORAGE::GetNodeID ()
{
return data.params.Uav_self_id;
}



unsigned long TEASYMEMSTORAGE::GetRawParam (ESAVEPARIX ix)
{
unsigned long val = 0;
if (ix < ESAVEPARIX_ENDENUM) val = ((unsigned long *)&data.params)[ix];
return val;
}



void TEASYMEMSTORAGE::GetParam (ESAVEPARIX ix, TSPDATA dest_dat)
{
	if (ix < ESAVEPARIX_ENDENUM && dest_dat)
		{
		CopyMemorySDC ((char*)&((unsigned long *)&data.params)[ix], (char*)dest_dat, sizeof(float));
		}
}



