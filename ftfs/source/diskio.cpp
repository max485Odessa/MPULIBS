/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#include "..\TSDCARD.hpp"

/* Definitions of physical drive number for each drive */
#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */


extern TFFSIF *sdffobj;


extern DRESULT disk_read (
	BYTE pdrv,	
	BYTE *buff,		
	LBA_t sector,	
	UINT count		
);

/*
static DRESULT MMC_disk_read (BYTE *buff, LBA_t sector, UINT count)
{
DRESULT rv = RES_OK;
	while (count)
		{
		if (TSDCARD::SD_ReadSector (sector, buff))
			{
			rv = RES_ERROR;
			break;
			}
		buff += 512;
		sector++;
		count--;
		}
	
	return rv;
}


	
static DRESULT MMC_disk_write (const BYTE *buff, LBA_t sector, UINT count)
{
	DRESULT rv = RES_OK;
	while (count)
		{
		if (TSDCARD::SD_WriteSector (sector, (BYTE*) buff))
			{
			rv = RES_ERROR;
			break;
			}
		buff += 512;
		sector++;
		count--;
		}
	return rv;
}
*/

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/


DSTATUS disk_status (
	BYTE pdrv	
)
{
	return sdffobj->disk_status (pdrv);
}




/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv			
)
{
	return sdffobj->disk_initialize (pdrv);
}




/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,	
	BYTE *buff,		
	LBA_t sector,	
	UINT count		
)
{
	return sdffobj->disk_read (pdrv, buff, sector, count);
}




/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,		
	const BYTE *buff,
	LBA_t sector,		
	UINT count		
)
{
	return sdffobj->disk_write (pdrv, buff, sector, count);
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/


DRESULT disk_ioctl (
	BYTE pdrv,	
	BYTE cmd,	
	void *buff	
)
{

	return sdffobj->disk_ioctl(pdrv, cmd, buff);
}

