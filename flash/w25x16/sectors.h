#ifndef _H_APP_BOOT_SECTORS_H_
#define _H_APP_BOOT_SECTORS_H_


#include "winb25x16.h"



extern const char *lPassSectors; 
#define STM103_SECTOR_SIZE 1024
#define C_FIRMSIZEMAX 0x14000

// W25QxxAdreses 
// начала этих адрессов должны быть кратными 4096 байт размеру, это размер сектора w25xx для операций SectorErase
#define C_ADREXT_UPDATE 			0x2000				// адрес размещения структуры запроса на обновление
#define C_ADREXT_UAVPARAMS_A 	0x3000				// начальный адрес первой копии параметров
#define C_ADREXT_UAVPARAMS_B 	0x4000				// начальный адрес второй копии
#define C_ADREXT_UAVPARAMS_C 	0x5000				// начальный адрес третьей копии
#define C_ADREXT_FIRMWARE_A 	0x6000																			// адрес размещения прошивки принятой по uavcan копия 1
#define C_ADREXT_FIRMWARE_B 	(C_ADREXT_FIRMWARE_A + C_FIRMSIZEMAX)				// адрес размещения прошивки принятой по uavcan копия 2
#define C_ADREXT_FIRMWARE_T 	(C_ADREXT_FIRMWARE_B + C_FIRMSIZEMAX) 			// temporary адрес куда временно может приниматься вся прошивка с uavcan
#define C_ADREXT_FREE					(C_ADREXT_FIRMWARE_T + C_FIRMSIZEMAX)				// не используется



/* Распределение секторов в контроллере
Вся память в stm32f103C8 - разбита на 1 кб сектора...
0x08000000 - sector 1		- boot start
0x08001C00 - sector 7   - user informations (crc, size и т.д.)
0x08002000 - sector 8   - user start
*/

#define BOOT_ADDRESS ((unsigned long)0x08000000)	
#define C_ADR_USERINFO ((unsigned long)0x08001C00)					// информация о пользовательской прошивке (для контроля целостности до ее старта)		
#define APPLICATION_ADDRESS ((unsigned long)0x08002000)			// старт программы


#define C_SIGNATURE_BINFILE 0xA3A53A5A			// magic field
#define C_MAXPROGRAM_SIZE ((unsigned long)(65536-8192))		// если размер бута поменялся подкоректировать число 8192
#define C_CHANK_SIZE 512		// размер секций в firmware файле
#define C_MPRGSECTOR_SIZE (C_MAXPROGRAM_SIZE/C_CHANK_SIZE)


typedef struct {
	unsigned long Signature;
	unsigned long DestAdr;	// используется как флаг запроса на обновление, после обновления его надо сбросить
} UPDATEREQTAG;



typedef struct {
	unsigned long Signature;			// magic field
	unsigned long Version;				// версия firmware (отображается в uavcan)
	unsigned long sizes;					// размер бинарника в исполнимом байт коде
	unsigned long CRC32_CODE;			// контрольная сумма, контрольных сумм секций firmware файла
	unsigned long CRC32_ENCODE;		// контрольная сумма firmware бинарника в исполнимом байт коде
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

enum E_INIERR {E_INIERR_NONE = 0, E_INIERR_BADCRC = 1};
//bool CheckUserFlash ();		// проверить целостность пользовательской прошивки
bool CheckExtFlash ();		// проверить наличие правильной прошивки во Flash
bool WriteUserFlash (unsigned long ExtSrcAdr, unsigned long IntDstAdr, unsigned long sizes);	// записать данные 
bool WriteIniSector (INIS *ldata);

void CodingData ();
void DecodeData ();
extern unsigned int GetLenStr (char *lpAdr);
unsigned long CalcCRC32Data (unsigned char *lSrc, unsigned long siz);
unsigned long RamCodingPink (unsigned char *lpRamInput, unsigned char *lpRamOutput,unsigned long sizesdata,unsigned char *lpPassCode);
bool ExternalFirmware_Check (TWINBOND25X16 &EFlsh, unsigned long Adr);


#endif

