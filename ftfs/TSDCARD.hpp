#ifndef _H_SDCARD_STM32F103_H_
#define _H_SDCARD_STM32F103_H_



#include ".\source\ff.h"			/* Declarations of FatFs API */
#include ".\source\diskio.h"		/* Declarations of device I/O functions */
#include "stm32f4xx_hal.h"
#include "ispi.hpp"
#include "..\diagnosticif.hpp"


#define GO_IDLE_STATE 0 								
#define SEND_IF_COND 8 											
#define READ_SINGLE_BLOCK 17 								
#define WRITE_SINGLE_BLOCK 24 								
#define SD_SEND_OP_COND 41 										
#define APP_CMD 55 														
#define READ_OCR 58

/*
#define PIN_SD_MOSI GPIO_PIN_15
#define PIN_SD_MISO GPIO_PIN_14
#define PIN_SD_SCK GPIO_PIN_13
#define PIN_SD_CS GPIO_PIN_12
#define CS_SD_ENABLE     GPIOB->BRR = PIN_SD_CS;          
#define CS_SD_DISABLE    GPIOB->BSRR = PIN_SD_CS;

#define spi_cs_low() do { GPIOB->BRR = PIN_SD_CS; } while (0)
#define spi_cs_high() do { GPIOB->BSRR = PIN_SD_CS; } while (0)
*/

enum sd_speed { SD_SPEED_INVALID, SD_SPEED_400KHZ, SD_SPEED_25MHZ };

#define CAP_VER2_00	(1<<0)
#define CAP_SDHC	(1<<1)
#define SPI_SD SPI2

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;

typedef struct {
	int initialized;
	int sectors;
	int erase_sectors;
	int capabilities;
} hwif;


class TFFSIF {

	public:
		virtual int hwif_init(hwif* hw) = 0;
		virtual int sd_read(hwif* hw, u32 address, u8 *buf) = 0;
		virtual int sd_write(hwif* hw, u32 address,const u8 *buf) = 0;
		virtual DSTATUS disk_initialize(BYTE drv) = 0;
		virtual DRESULT disk_read(BYTE drv, BYTE *buff, LBA_t sector, UINT count) = 0;
		virtual DSTATUS disk_status(BYTE drv) = 0;
		virtual DRESULT disk_ioctl(BYTE drv, BYTE ctrl, void *buff) = 0;
	#if _READONLY == 0
		virtual DRESULT disk_write(BYTE drv, const BYTE *buff, DWORD sector, UINT count) = 0;
	#endif
};



class TSDCARD: public TFFSIF,  public TDIAGIF {
		 S_GPIOPIN *sd_pin;

		 void diagnostick_start (uint32_t time_max) override;
		 ESTATE diagnostick_process (char **txtout) override;


		 ISPI *spi_obj;
		 hwif hw;

		 int hwif_init(hwif* hw) override;
		 int sd_read(hwif* hw, u32 address, u8 *buf) override;
		 int sd_write(hwif* hw, u32 address,const u8 *buf) override;
		 DSTATUS disk_initialize(BYTE drv) override;
		 DRESULT disk_read(BYTE drv, BYTE *buff, LBA_t sector, UINT count) override;
		 DSTATUS disk_status(BYTE drv) override;
		 DRESULT disk_ioctl(BYTE drv, BYTE ctrl, void *buff) override;
	#if _READONLY == 0
		 DRESULT disk_write(BYTE drv, const BYTE *buff, DWORD sector, UINT count) override;
	#endif

		 void spi_cs_low ();
		 void spi_cs_high ();
		 void sd_nec();
		 u8 spi_txrx(u8 data);

	protected:
		u16 crc16_ccitt(u16 crc, u8 ser_data);
		u16 crc16(const u8 *p, int len);
		u8 crc7_one(u8 t, u8 data);
		u8 crc7(const u8 *p, int len);

		u8 sd_get_r1();
		u16 sd_get_r2();
		u8 sd_get_r7(u32 *r7);

		int sd_get_data(hwif *hw, u8 *buf, int len);
		int sd_put_data(hwif *hw, const u8 *buf, int len);
		int sd_readsector(hwif *hw, u32 address, u8 *buf);
		int sd_writesector(hwif *hw, u32 address, const u8 *buf);

	public:
		TSDCARD (ISPI *s, const S_GPIOPIN *pin);
		void sd_cmd(u8 cmd, u32 arg);
		int sd_init(hwif *hw);
		int sd_read_status(hwif *hw);
		int sd_read_csd(hwif *hw);
		int sd_read_cid(hwif *hw);

		bool is_sdcard ();	// мем стик вставлен


};




#endif

