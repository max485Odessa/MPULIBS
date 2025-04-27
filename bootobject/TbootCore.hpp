/*
 * TbootCore.h
 *
 *  Created on: Apr 1, 2025
 *      Author: Maxim
 */

#ifndef SRC_TBOOTCORE_HPP_
#define SRC_TBOOTCORE_HPP_

#include <stdint.h>
#include "..\imemif.hpp"
#include "stm32f4xx_hal.h"


typedef void (*cb_firmware_progress) (bool f_ok, uint8_t procnt);

/*
 * 1. Проверяется запрос на прошивку. Если запрос есть, проверить целостность прошивки во внешней флеш памяти.
 * 2. Записать прошивку (в stm32), записать параметры прошивки (в flash), стереть запрос записи (из flash).
 * 3. Стартануть пользовательскую программу.
 *
 * 4. Запроса на прошивку не было. Проверяем наличие параметров, параметров нет - стартуем прошивку (это значит что никогда еще не обновлялась система)
 * 5. Если параметры обнаружены, проверяем целостность stm32 памяти, если все Нормально - стартуем программу.
 * 6. Если параметры обнаружены и по этим параметрам программа повреждена, проверить целостность прошивки во внешней флеш памяти. Если все нормально - восстановить/записать из внешней памяти прошивку в STM. Стартануть пользовательскую программу.
 * 7. Бут не передает управление программе в случае: когда обнаружен тег параметров и программа не соответствует записям этого тега и отсутствуют firmware образы во внешней памяти. Или при отсутствии тега параметров, адрес перехода находится не в пределах диапазона адресов (base adress user program + maximal firmware size)
 * */

#pragma pack (push, 1)



#define C_SIGNATURE_BINFILE 0xA3A53A5A			// magic field
#define C_CHANK_SIZE 512		// размер секций в firmware файле


typedef struct {
	unsigned long Signature;
	unsigned long DestAdr;	// используется как флаг запроса на обновление, после обновления его надо сбросить
} UPDATEREQTAG;



typedef struct {
	unsigned long Signature;			// magic field
	unsigned long Version;				// версия firmware
	unsigned long sizes;				// размер бинарника в исполнимом байт коде
	unsigned long CRC32_CODE;			// контрольная сумма, контрольных сумм секций firmware файла
	unsigned long CRC32_ENCODE;			// контрольная сумма firmware бинарника в исполнимом байт коде
	uint8_t devname[32];				// device name
} USERDATA;



// с этой структуры начинаются сектора(CHANKCONTROL) в firmware файле
typedef struct {
	USERDATA Data;
	char FreeTxt[1024 - sizeof(USERDATA)];
} FMWHEADER;									// префикс файла firmware формата (без изменения)



typedef struct {
	unsigned long HiAdress;			// всегда 0 (наследуется с 16-ти битного расширения Intel Hex формата)
	unsigned long LoAdress;			// текущий адрес размещения секции в памяти
	unsigned long SizeData;			// размер секции
	unsigned long CRC32_CODE;		// контрольная сумма секции
} CHANKCONTROL;


typedef struct {
	CHANKCONTROL inf;
	unsigned char raw[C_CHANK_SIZE];
} FULLCHANK;									// полная секция (информация и данные)


typedef struct {
	USERDATA USER;
} INIS;

#pragma pack (pop)


const unsigned char C_8BIT_POL = 0x41;
const unsigned long C_BITMASK_HB = 0x80000000;
/*
 * 	enum ERSLT {\
		ERSLT_OK = 0, \
		ERSLT_NOTAG, \
		ERSLT_BADCRC, \
		ERSLT_NOPROGRAM, \
		ERSLT_ENDENUM};
 * */

extern const char *ltxt_firmwarename;
extern const char *ltxt_lansetsfilename;

class TbootCore {
public:
	enum ERSLT {\
		ERSLT_OK = 0, /*  */ \
		ERSLT_NOPROGRAM, /*  */ \
		ERSLT_ENDENUM};

	enum EWCH {EWCH_CHECK = 0, EWCH_WRITE = 1};

	TbootCore (TEEPROMIF *eprm, uint32_t bt_txt, uint32_t a_user_str, uint32_t a_prmini, uint32_t a_reqwr, uint32_t a_frm, uint32_t c_frcnt, const char *lpass, const char *ldn, uint32_t mxfz);


	uint32_t get_firmware_version ();

	bool check_write_req ();
	ERSLT check_req_write_firmware (cb_firmware_progress cb);
	void exec_user_prog ();
	void restart_sys ();	// переход на boot через рестарт

	void enable_irq ();
	void disable_irq ();

	bool get_boot_message (char *dst, uint32_t maxsz);
	void set_boot_message (char *src_txt);
	void clear_boot_message ();

	void write_firmware_req_event ();

	bool write_firmware_to_flash_sect (uint8_t idix, uint32_t offs_dst, void *src, uint16_t sz_wr);

	const uint32_t firmware_size_control_max ();
	const uint32_t firmware_sect_size_max ();
	const char *device_name ();

private:
	uint8_t *bufer;
	TEEPROMIF *flashobj;
	cb_firmware_progress cb_write;

	const uint32_t c_adr_boot_msg;
	const uint32_t c_adr_user_start;	// stm32 adress
	const uint32_t c_adr_req;			// ext flash adr
	const uint32_t c_adr_paramini;		// ext flash adr
	const uint32_t c_adr_firmsrc;		// ext flash adr
	const uint32_t c_firm_cntr;			// firmware count in the ext flash
	const uint32_t c_bufsize;
	const char *lpasswrd;
	const char *lpdevname;
	const uint32_t c_control_firm_size;	// firmware size
	const uint32_t c_sect_cnt_control;


	uint32_t RamCodingPink (uint8_t *lpRamInput, uint8_t *lpRamOutput, uint32_t sizesdata, uint8_t *lpPassCode);
	uint32_t CalcCRC32Data (uint8_t *lSrc, uint32_t siz);
	uint32_t GetSector (uint32_t Address);
	uint32_t GetSectorSize (uint32_t Sector);

	// находит во внешней флеш памяти целостную копию прошивки
	bool firmware_find (uint32_t &adr, USERDATA *udt);
	ERSLT check_user_flash ();
	void clear_wr_req ();		// ++
	uint32_t find_firmware_count ();
	bool ext_check_firmware (uint32_t adr, USERDATA *udt);
	bool ExternalFirmware_CheckOrWrite (uint32_t Adr, EWCH mod, USERDATA *udt);

	//bool read_firmw_header (uint32_t adr_ext_flash, USERDATA *h);
	bool write_firmware (uint32_t adr_ext_src);

	//bool write_firmware_params (const USERDATA *src);

	bool EraseSTMSectors (uint32_t AdrStart, uint32_t sizes);
	bool WriteSTM (uint32_t Dst, void *lSrc, uint32_t sizes);
	bool WriteUser_Info (USERDATA *src);
	bool ReadUser_Info (USERDATA *dst);

};

#endif /* SRC_TBOOTCORE_HPP_ */
