#ifndef _H_APP_BOOT_SECTORS_H_
#define _H_APP_BOOT_SECTORS_H_


#include "winb25x16.h"



extern const char *lPassSectors; 
#define STM103_SECTOR_SIZE 1024
#define C_FIRMSIZEMAX 0x14000

// W25QxxAdreses 
// ������ ���� �������� ������ ���� �������� 4096 ���� �������, ��� ������ ������� w25xx ��� �������� SectorErase
#define C_ADREXT_UPDATE 			0x2000				// ����� ���������� ��������� ������� �� ����������
#define C_ADREXT_UAVPARAMS_A 	0x3000				// ��������� ����� ������ ����� ����������
#define C_ADREXT_UAVPARAMS_B 	0x4000				// ��������� ����� ������ �����
#define C_ADREXT_UAVPARAMS_C 	0x5000				// ��������� ����� ������� �����
#define C_ADREXT_FIRMWARE_A 	0x6000																			// ����� ���������� �������� �������� �� uavcan ����� 1
#define C_ADREXT_FIRMWARE_B 	(C_ADREXT_FIRMWARE_A + C_FIRMSIZEMAX)				// ����� ���������� �������� �������� �� uavcan ����� 2
#define C_ADREXT_FIRMWARE_T 	(C_ADREXT_FIRMWARE_B + C_FIRMSIZEMAX) 			// temporary ����� ���� �������� ����� ����������� ��� �������� � uavcan
#define C_ADREXT_FREE					(C_ADREXT_FIRMWARE_T + C_FIRMSIZEMAX)				// �� ������������



/* ������������� �������� � �����������
��� ������ � stm32f103C8 - ������� �� 1 �� �������...
0x08000000 - sector 1		- boot start
0x08001C00 - sector 7   - user informations (crc, size � �.�.)
0x08002000 - sector 8   - user start
*/

#define BOOT_ADDRESS ((unsigned long)0x08000000)	
#define C_ADR_USERINFO ((unsigned long)0x08001C00)					// ���������� � ���������������� �������� (��� �������� ����������� �� �� ������)		
#define APPLICATION_ADDRESS ((unsigned long)0x08002000)			// ����� ���������


#define C_SIGNATURE_BINFILE 0xA3A53A5A			// magic field
#define C_MAXPROGRAM_SIZE ((unsigned long)(65536-8192))		// ���� ������ ���� ��������� ���������������� ����� 8192
#define C_CHANK_SIZE 512		// ������ ������ � firmware �����
#define C_MPRGSECTOR_SIZE (C_MAXPROGRAM_SIZE/C_CHANK_SIZE)


typedef struct {
	unsigned long Signature;
	unsigned long DestAdr;	// ������������ ��� ���� ������� �� ����������, ����� ���������� ��� ���� ��������
} UPDATEREQTAG;



typedef struct {
	unsigned long Signature;			// magic field
	unsigned long Version;				// ������ firmware (������������ � uavcan)
	unsigned long sizes;					// ������ ��������� � ���������� ���� ����
	unsigned long CRC32_CODE;			// ����������� �����, ����������� ���� ������ firmware �����
	unsigned long CRC32_ENCODE;		// ����������� ����� firmware ��������� � ���������� ���� ����
} USERDATA;



// � ���� ��������� ���������� �������(CHANKCONTROL) � firmware �����
typedef struct {
	USERDATA Data;
	char FreeTxt[1024 - sizeof(USERDATA)];
} FMWHEADER;									// ������� ����� firmware ������� (��� ���������)



typedef struct {
	unsigned long HiAdress;			// ������ 0 (����������� � 16-�� ������� ���������� Intel Hex �������)
	unsigned long LoAdress;			// ������� ����� ���������� ������ � ������
	unsigned long SizeData;			// ������ ������
	unsigned long CRC32_CODE;		// ����������� ����� ������
} CHANKCONTROL;


typedef struct {
	CHANKCONTROL inf;
	unsigned char raw[C_CHANK_SIZE];
} FULLCHANK;									// ������ ������ (���������� � ������)


typedef struct {
	USERDATA USER;
} INIS;												

enum E_INIERR {E_INIERR_NONE = 0, E_INIERR_BADCRC = 1};
//bool CheckUserFlash ();		// ��������� ����������� ���������������� ��������
bool CheckExtFlash ();		// ��������� ������� ���������� �������� �� Flash
bool WriteUserFlash (unsigned long ExtSrcAdr, unsigned long IntDstAdr, unsigned long sizes);	// �������� ������ 
bool WriteIniSector (INIS *ldata);

void CodingData ();
void DecodeData ();
extern unsigned int GetLenStr (char *lpAdr);
unsigned long CalcCRC32Data (unsigned char *lSrc, unsigned long siz);
unsigned long RamCodingPink (unsigned char *lpRamInput, unsigned char *lpRamOutput,unsigned long sizesdata,unsigned char *lpPassCode);
bool ExternalFirmware_Check (TWINBOND25X16 &EFlsh, unsigned long Adr);


#endif

