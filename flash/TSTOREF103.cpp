#include "TSTOREF103.h"


TSTORGF103::TSTORGF103 (uint32_t start_p, uint32_t stop_p) : c_page_start_adr (start_p), c_page_stop_adr (stop_p)
{
	
}



uint32_t TSTORGF103::file_size ()
{
	return c_page_stop_adr - c_page_start_adr;
}



uint32_t TSTORGF103::erase_pages (uint32_t start_adr, uint32_t stp_adr)
{
	uint32_t rv = 0;
	uint32_t PAGEError = 0;
	FLASH_EraseInitTypeDef EraseInitStruct;
  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.PageAddress = start_adr;
  EraseInitStruct.NbPages     = (stp_adr - start_adr) / FLASH_PAGE_SIZE;
	if (!EraseInitStruct.NbPages) EraseInitStruct.NbPages = 1;
	uint32_t rsltpgs = EraseInitStruct.NbPages;
	if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) == HAL_OK) rv  = rsltpgs;
	return rv;
}



void TSTORGF103::unlock_flash ()
{
	HAL_FLASH_Unlock ();
}


	
void TSTORGF103::lock_flash ()
{
	HAL_FLASH_Lock ();
}



bool TSTORGF103::get_word (uint8_t *src, uint32_t cursize, uint32_t &dstdata, uint8_t &lsize)
{
	bool rv = false;
	if (src && cursize)
		{
		if (cursize > 4) cursize = 4;
		lsize = cursize;
		uint8_t ardst[4];

		uint8_t *dst = (uint8_t*)ardst;
		while (cursize)
			{
			*dst++ = *src++;
			cursize--;
			}
		uint32_t *rsltdata = (uint32_t*)&ardst;
		dstdata = *rsltdata;
		rv = true;
		}
	return rv;	
}



bool TSTORGF103::get_word (uint32_t c_adr, uint32_t last_adr, uint32_t &dstdata, uint8_t &lsize)
{
	bool rv = false;
	if (c_adr < last_adr)
		{
		uint32_t len = last_adr - c_adr;
		if (len > 4) len = 4;
		lsize = len;
		uint8_t ardst[4];
		uint8_t *src = (uint8_t*)c_adr;
		uint8_t *dst = (uint8_t*)ardst;
		while (len)
			{
			*dst++ = *src++;
			len--;
			}
		uint32_t *rsltdata = (uint32_t*)&ardst;
		dstdata = *rsltdata;
		rv = true;
		}
	return rv;
}



bool TSTORGF103::write (uint32_t adrix, uint8_t *src, uint32_t wr_size)
{
	bool rv = false;
	adrix += c_page_start_adr;
	if (adrix < c_page_stop_adr && src)
		{
		unlock_flash ();

		if ((adrix + wr_size) > c_page_stop_adr) wr_size = c_page_stop_adr - adrix; 
		const uint32_t endadress = adrix + wr_size;
		if (erase_pages (adrix, endadress))
			{
			uint8_t inc_sz;
			uint32_t DATA_32;
			while (adrix < endadress && wr_size)
				{
				if (!get_word ((uint8_t *)adrix, wr_size, DATA_32, inc_sz)) break;
				if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, adrix, DATA_32) != HAL_OK) break;
				adrix += inc_sz;
				wr_size -= inc_sz;
				}
			if (!wr_size) rv = true;
			}
		lock_flash ();
		}
	return rv;
}



bool TSTORGF103::read (uint32_t adrix, uint8_t *dst, uint32_t wr_size)
{
	bool rv = false;
	if (dst)
		{
		uint8_t *src = (uint8_t*)(adrix + c_page_start_adr);
		while (wr_size)
			{
			*dst++ = *src++;
			wr_size--;
			}
		rv = true;
		}
	return rv;
}



