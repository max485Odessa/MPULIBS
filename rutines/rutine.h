#ifndef _h_rutine_lib_h_
#define _h_rutine_lib_h_

#include "stddef.h"
#include "stdint.h"

typedef struct {
	unsigned char *lRam;
	unsigned short sizes;
} TBUFPARAM;


typedef struct {
	unsigned char *lRam;
	unsigned short sizes;
} BUFPAR;

bool dec_value (uint32_t *lpval, uint32_t val_d);
unsigned long lenstr (const char *lsrt);
uint32_t lenstr_max (const char *lsrt, uint32_t maxsz);
unsigned short abs16 (short datas);
unsigned long abs32 (long datas);
void *memset (void *lDst, unsigned char dat, long sizes);
//void *memcpy ( void * destination, const void * source, long num );
unsigned long HAL_GetTick ();
unsigned long HAL_RCC_GetPCLK1Freq ();
unsigned long str_size (const char *lsrt);
//bool str_compare (char *lStr1, char *lStr2, unsigned long size);
long strncmp( const char * string1, const char * string2, long num );
unsigned long GetTagStringDelimIndx (char *lpLinetxt, unsigned long Indxx, char delimc, char **lDest, unsigned long *lPCountField);
bool str_compare (char *lStr1, char *lStr2, unsigned long size);
bool TxtToFloat (float *lpDest, char *lpTxtIn, unsigned long Sizes);
unsigned long CheckDecimal (char *lTxt, unsigned long sz);
bool TxtToULong (char *lpRamData, unsigned char sz, unsigned long *lpDataOut);
unsigned short SwapShort (unsigned short dat);
void HardwareReset (void);
unsigned char *UlongToStr (unsigned char *lpDest, unsigned long datas);
unsigned char *LongToStr (unsigned char *lpDest, long datas);
char *FloatToStr (char *lDst, float val, unsigned char dig_n);
//char *CopyMemorySDC (char *lpSour, char *lpDest, unsigned int sizes);
void *CopySDC_Data (void *s, void *d, uint32_t sz);
unsigned long CreateValueFromBitMassive (unsigned char *lInp, unsigned long CurOffset, unsigned char databitsize);
unsigned char SWAPBits (unsigned char datas);
unsigned long SwapBitsLong (unsigned long val, unsigned char bitsize);

/*
void swapByteOrder(void* data, size_t size);
uint16_t crcAddByte(uint16_t crc_val, uint8_t byte);
uint16_t crcAddSignature(uint16_t crc_val, uint64_t data_type_signature);
uint16_t crcAdd(uint16_t crc_val, const uint8_t* bytes, size_t len);
*/

#endif



