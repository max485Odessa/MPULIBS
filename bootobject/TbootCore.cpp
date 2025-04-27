/*
 * TbootCore.cpp
 *
 *  Created on: Apr 1, 2025
 *      Author: Maxim
 */

#include "TbootCore.hpp"
#include "..\TWatchDog.hpp"
#include "..\rutine.hpp"

#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base @ of Sector 7, 128 Kbytes */
#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) /* Base @ of Sector 8, 128 Kbytes */
#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) /* Base @ of Sector 9, 128 Kbytes */
#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) /* Base @ of Sector 10, 128 Kbytes */
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbytes */

/* Base address of the Flash sectors Bank 2 */
#define ADDR_FLASH_SECTOR_12     ((uint32_t)0x08100000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_13     ((uint32_t)0x08104000) /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_14     ((uint32_t)0x08108000) /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_15     ((uint32_t)0x0810C000) /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_16     ((uint32_t)0x08110000) /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_17     ((uint32_t)0x08120000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_18     ((uint32_t)0x08140000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_19     ((uint32_t)0x08160000) /* Base @ of Sector 7, 128 Kbytes */
#define ADDR_FLASH_SECTOR_20     ((uint32_t)0x08180000) /* Base @ of Sector 8, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_21     ((uint32_t)0x081A0000) /* Base @ of Sector 9, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_22     ((uint32_t)0x081C0000) /* Base @ of Sector 10, 128 Kbytes */
#define ADDR_FLASH_SECTOR_23     ((uint32_t)0x081E0000) /* Base @ of Sector 11, 128 Kbytes */


const char *ltxt_firmwarename = "firmware.bin";
const char *ltxt_lansetsfilename = "setting.ini";


TbootCore::TbootCore (TEEPROMIF *eprm, uint32_t bt_txt, uint32_t a_user_str, uint32_t a_prmini, uint32_t a_reqwr, uint32_t a_frm, uint32_t c_frcnt, const char *lpass, const char *ldn, uint32_t mxfz): \
c_adr_boot_msg (bt_txt), c_adr_user_start(a_user_str), c_adr_req (a_reqwr), c_adr_paramini (a_prmini), c_adr_firmsrc (a_frm), c_firm_cntr (c_frcnt), c_bufsize (1024), lpasswrd (lpass), lpdevname (ldn), \
c_control_firm_size (mxfz), c_sect_cnt_control (mxfz/C_CHANK_SIZE + ((mxfz % C_CHANK_SIZE)?1:0))
{
	flashobj = eprm;
	cb_write = 0;
	bufer = new uint8_t[c_bufsize];
}



const char *TbootCore::device_name ()
{
	return lpdevname;
}


bool TbootCore::get_boot_message (char *dst, uint32_t maxsz)
{
	bool rv = false;
	uint32_t no_zero = 0;

	if (flashobj->read (c_adr_boot_msg, (uint8_t*)&no_zero, sizeof(no_zero))) {
		if (no_zero) rv = flashobj->read (c_adr_boot_msg, (uint8_t*)dst, maxsz);
		}
	return rv;
}



void TbootCore::set_boot_message (char *src_txt)
{
	if (src_txt) {
		uint32_t ln = lenstr (src_txt);
		if (ln)
			{
			ln++;	// zero add
			if (ln > 512) ln = 512;
			flashobj->write (c_adr_boot_msg, (uint8_t*)src_txt, ln);
			}
		}
}



void TbootCore::clear_boot_message ()
{
	uint32_t zero = 0;
	flashobj->write (c_adr_boot_msg, (uint8_t*)&zero, sizeof(zero));
}



uint32_t TbootCore::GetSector(uint32_t Address)
{
  uint32_t sector = 0;

  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_SECTOR_0;
  }
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_SECTOR_1;
  }
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_SECTOR_2;
  }
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_SECTOR_3;
  }
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_SECTOR_4;
  }
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_SECTOR_5;
  }
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_SECTOR_6;
  }
  else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
  {
    sector = FLASH_SECTOR_7;
  }
  else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
  {
    sector = FLASH_SECTOR_8;
  }
  else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
  {
    sector = FLASH_SECTOR_9;
  }
  else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
  {
    sector = FLASH_SECTOR_10;
  }
  else if((Address < ADDR_FLASH_SECTOR_12) && (Address >= ADDR_FLASH_SECTOR_11))
  {
    sector = FLASH_SECTOR_11;
  }
  else if((Address < ADDR_FLASH_SECTOR_13) && (Address >= ADDR_FLASH_SECTOR_12))
  {
    sector = FLASH_SECTOR_12;
  }
  else if((Address < ADDR_FLASH_SECTOR_14) && (Address >= ADDR_FLASH_SECTOR_13))
  {
    sector = FLASH_SECTOR_13;
  }
  else if((Address < ADDR_FLASH_SECTOR_15) && (Address >= ADDR_FLASH_SECTOR_14))
  {
    sector = FLASH_SECTOR_14;
  }
  else if((Address < ADDR_FLASH_SECTOR_16) && (Address >= ADDR_FLASH_SECTOR_15))
  {
    sector = FLASH_SECTOR_15;
  }
  else if((Address < ADDR_FLASH_SECTOR_17) && (Address >= ADDR_FLASH_SECTOR_16))
  {
    sector = FLASH_SECTOR_16;
  }
  else if((Address < ADDR_FLASH_SECTOR_18) && (Address >= ADDR_FLASH_SECTOR_17))
  {
    sector = FLASH_SECTOR_17;
  }
  else if((Address < ADDR_FLASH_SECTOR_19) && (Address >= ADDR_FLASH_SECTOR_18))
  {
    sector = FLASH_SECTOR_18;
  }
  else if((Address < ADDR_FLASH_SECTOR_20) && (Address >= ADDR_FLASH_SECTOR_19))
  {
    sector = FLASH_SECTOR_19;
  }
  else if((Address < ADDR_FLASH_SECTOR_21) && (Address >= ADDR_FLASH_SECTOR_20))
  {
    sector = FLASH_SECTOR_20;
  }
  else if((Address < ADDR_FLASH_SECTOR_22) && (Address >= ADDR_FLASH_SECTOR_21))
  {
    sector = FLASH_SECTOR_21;
  }
  else if((Address < ADDR_FLASH_SECTOR_23) && (Address >= ADDR_FLASH_SECTOR_22))
  {
    sector = FLASH_SECTOR_22;
  }
  else/*(Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_23))*/
  {
    sector = FLASH_SECTOR_23;
  }

  return sector;
}



uint32_t TbootCore::GetSectorSize(uint32_t Sector)
{
  uint32_t sectorsize = 0x00;
  if((Sector == FLASH_SECTOR_0) || (Sector == FLASH_SECTOR_1) || (Sector == FLASH_SECTOR_2) ||\
     (Sector == FLASH_SECTOR_3) || (Sector == FLASH_SECTOR_12) || (Sector == FLASH_SECTOR_13) ||\
     (Sector == FLASH_SECTOR_14) || (Sector == FLASH_SECTOR_15))
  {
    sectorsize = 16 * 1024;
  }
  else if((Sector == FLASH_SECTOR_4) || (Sector == FLASH_SECTOR_16))
  {
    sectorsize = 64 * 1024;
  }
  else
  {
    sectorsize = 128 * 1024;
  }
  return sectorsize;
}





uint32_t TbootCore::CalcCRC32Data (uint8_t *lSrc, uint32_t siz)
{
	uint32_t rv = 0, z_cnt = 0;
	uint8_t dat;
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



uint32_t TbootCore::RamCodingPink (uint8_t *lpRamInput, uint8_t *lpRamOutput, uint32_t sizesdata, uint8_t *lpPassCode)
{
	uint32_t rv = 0;
if (lpRamInput && lpRamOutput && sizesdata)
	{
	uint8_t *lpCurPassdata=lpPassCode;
	uint32_t LenPassword;
	uint8_t curbyted;
	uint8_t maskleft=1;
	uint8_t maskright=128;
	if (!lpPassCode)
					{
					LenPassword = 0;
					}
	else
					{
					LenPassword = lenstr ((const char*)lpPassCode);
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



// очищает сектора в STM32.
// AdrStart - начальный адрес флеш, sizes - количество байт
bool TbootCore::EraseSTMSectors (uint32_t AdrStart, uint32_t sizes)
{
	bool rv = true;
	FLASH_EraseInitTypeDef EraseInitStruct;

	uint32_t FirstSector = 0, NbOfSectors = 0;
	uint32_t SectorError = 0;
	/* Get the 1st sector to erase */
	FirstSector = GetSector(c_adr_user_start);
	/* Get the number of sector to erase from 1st sector*/
	NbOfSectors = GetSector(c_adr_user_start + sizes - 1) - FirstSector + 1;

	HAL_FLASH_Unlock();
	  /* Fill EraseInit structure*/
	  EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
	  EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
	  EraseInitStruct.Sector = FirstSector;
	  EraseInitStruct.NbSectors = NbOfSectors;
	  if(HAL_FLASHEx_Erase (&EraseInitStruct, &SectorError) == HAL_OK) rv = true;

	  /* Note: If an erase operation in Flash memory also concerns data in the data or instruction cache,
	     you have to make sure that these data are rewritten before they are accessed during code
	     execution. If this cannot be done safely, it is recommended to flush the caches by setting the
	     DCRST and ICRST bits in the FLASH_CR register. */
	  __HAL_FLASH_DATA_CACHE_DISABLE();
	  __HAL_FLASH_INSTRUCTION_CACHE_DISABLE();

	  __HAL_FLASH_DATA_CACHE_RESET();
	  __HAL_FLASH_INSTRUCTION_CACHE_RESET();

	  __HAL_FLASH_INSTRUCTION_CACHE_ENABLE();
	  __HAL_FLASH_DATA_CACHE_ENABLE();

	  return rv;
return rv;
}



// програмирует память STM32
// FlashAddress - начальный адрес флеш, Data - буфер источник, DataLength - размер блока в байтах
bool TbootCore::WriteSTM (uint32_t Dst, void *lSrc, uint32_t sizes)
{
	bool rv = false;
	uint32_t last_adr = Dst + sizes;
	uint32_t *DATA_32 = (uint32_t*)lSrc;

	HAL_FLASH_Unlock();

	while (Dst < last_adr)
		{
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Dst, *DATA_32) != HAL_OK) break;
		DATA_32++;
		Dst = Dst + 4;
		}
	 if (Dst == last_adr) rv = true;
	 return rv;
}



// записывает в STM32 флеш, информационный сектор (данные о firmware файле)
bool TbootCore::WriteUser_Info (USERDATA *lfrm)
{
return flashobj->write (c_adr_paramini, (uint8_t*)lfrm, sizeof(USERDATA));
}




bool TbootCore::ReadUser_Info (USERDATA *dst)
{
	USERDATA ud;
	bool rv = flashobj->read (c_adr_paramini, (uint8_t*)&ud, sizeof(USERDATA));
	if (rv) {
		rv = false;
		do	{
			if (ud.Signature != C_SIGNATURE_BINFILE) break;	// наличие сигнатуры
			if (!ud.sizes || ud.sizes > c_control_firm_size) break;	// корректность размера

			uint32_t len_dname = str_size (lpdevname);
			if (!str_compare ((char*)lpdevname, (char*)ud.devname, len_dname)) break;	// совпадение имени устройства

			if (dst) *dst = ud;
			rv = true;
			} while (false);
		}
	return rv;
}



bool TbootCore::write_firmware_to_flash_sect (uint8_t idix, uint32_t offs_dst, void *src, uint16_t sz_wr)
{
	bool rv = false;
	if (idix < c_firm_cntr && offs_dst < c_control_firm_size)
		{
		uint32_t base_dst = c_adr_firmsrc + (idix * c_control_firm_size) + offs_dst;
		rv = flashobj->write (base_dst, (uint8_t*)src, sz_wr);
		}
	return rv;
}



const uint32_t TbootCore::firmware_size_control_max ()
{
	return c_control_firm_size;
}



const uint32_t TbootCore::firmware_sect_size_max ()
{
	return c_sect_cnt_control;
}



// наличие firmware во флеш с коректной контрольной суммой
bool TbootCore::ExternalFirmware_CheckOrWrite (uint32_t Adr, EWCH mod, USERDATA *udt)
{
	bool rv = false;
	uint32_t CurExAdr = Adr;
	USERDATA usdat;
	FULLCHANK chank;
	// считываем и проверяем начальный заголовок
	flashobj->read (CurExAdr, (uint8_t*)&usdat, sizeof(usdat));

	if (usdat.Signature == C_SIGNATURE_BINFILE)
		{
		if (usdat.sizes && usdat.sizes <= c_control_firm_size)
			{
			uint32_t calc_crc_code = 0, tmp_crc;
			CurExAdr += sizeof(FMWHEADER);	// за USERDATA, следует область произвольного текста
			uint32_t indx = 0;
			bool f_error = false;

			if (udt) *udt = usdat;

			if (mod == EWCH_WRITE)
				{
				// подсчитываем количество секторов по размеру прошивки и стираем
				rv = EraseSTMSectors (c_adr_user_start, usdat.sizes);
				if (!rv) return rv;
				}
			uint32_t ContrSize = 0;
			// читаем и проверяем все секторные записи, подсчитываем crc
			while (indx < c_sect_cnt_control && ContrSize < usdat.sizes && !f_error)
				{
				flashobj->read (CurExAdr, (uint8_t*)&chank, sizeof(chank));
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
				if (mod == EWCH_WRITE)
					{
					RamCodingPink (chank.raw, chank.raw, chank.inf.SizeData, (unsigned char *)lpasswrd);	// декодируем секцию данных
					rv = WriteSTM (chank.inf.LoAdress, chank.raw, chank.inf.SizeData);			// записываем данные в STM по адресу указаному в секции

					if (cb_write)
						{
						// запуск процеса индикации на callback функции
						float curproc = ContrSize, quant_proc = usdat.sizes; quant_proc /= 100.0F;
						curproc /= quant_proc;
						cb_write (rv, curproc);
						}

					if (!rv)
						{
						f_error = true;
						break;
						}
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



bool TbootCore::write_firmware (uint32_t adr_ext_src)
{
	return ExternalFirmware_CheckOrWrite (adr_ext_src, EWCH_WRITE, 0);
}



bool TbootCore::ext_check_firmware (uint32_t Adr, USERDATA *udt)
{
	return ExternalFirmware_CheckOrWrite (Adr, EWCH_CHECK, udt);
}


// просматривает область памяти на внешней флеш на наличие целой записи прошивки
// возвращает адрес внешней памяти где хранится прошивка
bool TbootCore::firmware_find (uint32_t &adr_dst, USERDATA *udt)
{
	bool rv = false;
	uint8_t ix = 0;
	uint32_t Adr = c_adr_firmsrc;

	while (ix < c_firm_cntr)
		{
		if (ext_check_firmware (Adr, udt))
			{
			adr_dst = Adr;
			rv = true;
			break;
			}
		Adr += c_control_firm_size;//C_FIRMWARE_ARRTAG_SIZE;
		ix++;
		}
	return rv;
}



// ++
bool TbootCore::check_write_req ()
{
bool rv = false;
UPDATEREQTAG tag;
if (flashobj->read (c_adr_req, (uint8_t*)&tag, sizeof(UPDATEREQTAG))) {
	if (tag.Signature == C_SIGNATURE_BINFILE && tag.DestAdr == c_adr_user_start) rv = true;
	}
return rv;
}



void TbootCore::write_firmware_req_event ()
{
	UPDATEREQTAG tag;
	tag.Signature = C_SIGNATURE_BINFILE;
	tag.DestAdr = c_adr_user_start;
	flashobj->write (c_adr_req, (uint8_t*)&tag, sizeof(UPDATEREQTAG));
}



// ++
void TbootCore::clear_wr_req ()
{
	UPDATEREQTAG tag = {0,0};
	flashobj->write (c_adr_req, (uint8_t*)&tag, sizeof(UPDATEREQTAG));
}



TbootCore::ERSLT TbootCore::check_user_flash ()
{
	ERSLT rv = ERSLT_OK;
	USERDATA usdt;
	if (ReadUser_Info (&usdt))
		{
		if (usdt.CRC32_ENCODE != CalcCRC32Data ((unsigned char*)c_adr_user_start, usdt.sizes)) rv = ERSLT_NOPROGRAM;
		}
	else
		{
		uint32_t VAdr = ((uint32_t*)c_adr_user_start)[1];
		if ((VAdr < c_adr_user_start) || (VAdr > (c_adr_user_start + c_control_firm_size))) rv = ERSLT_NOPROGRAM;
		}
	return rv;
}



uint32_t TbootCore::get_firmware_version ()
{
	uint32_t rv = 0;
	USERDATA usdt;
	if (ReadUser_Info (&usdt)) {
		if (usdt.CRC32_ENCODE == CalcCRC32Data ((unsigned char*)c_adr_user_start, usdt.sizes)) rv = usdt.Version;
		}
	return rv;
}



TbootCore::ERSLT TbootCore::check_req_write_firmware (cb_firmware_progress cb)
{
	ERSLT rslt;

	cb_write = cb;
	if (check_write_req ())	// проверяет запись запроса на прошивку (хранится во внешней памяти)
		{
		// запрос на запись найден
		USERDATA usdat;
		uint32_t adr_ext_firmware = 0;
		if (firmware_find (adr_ext_firmware, &usdat))	// находим первую рабочую копию и дополнительно сохраняем параметры этой копии
			{
			uint8_t cnt = 3;
			// так как бут стартует с самого начала с минимальными задержками по включению питания
			// небольшой цикл из 3 попыток записи может устранить возможные проблемы с дребезгом питания устройства в момент его включения
			while (cnt)
				{
				if (write_firmware (adr_ext_firmware) && WriteUser_Info (&usdat))	// записываем прошивку и параметры прошивки
					{
					break;
					}
				cnt--;
				}
			}
		clear_wr_req ();	// удаляем запрос на запись в любом случае
		}

	rslt = check_user_flash ();

	return rslt;
}



void TbootCore::enable_irq ()
{
	__enable_irq ();
}



void TbootCore::disable_irq ()
{
	__disable_irq ();
}



void TbootCore::exec_user_prog ()
{
	void (*user_code_entry)(void);
	// запускает пользовательскую программу
	disable_irq ();
	uint32_t *lpVct = (uint32_t*)c_adr_user_start;
	SCB->VTOR = c_adr_user_start;
	user_code_entry = (void (*)(void))(lpVct[1]);
	user_code_entry();
}




void TbootCore::restart_sys ()
{
	TWATCH::Init (100);
	while (true) {};
}





