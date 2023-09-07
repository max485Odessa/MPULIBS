#ifndef __SPI_FLASH_STM32L4XX_LL_H_
#define __SPI_FLASH_STM32L4XX_LL_H_


#include "stm32l4xx_hal.h"
#include "hard_rut.h"
#include "SYSBIOS.H"



#define SPI_FLASH_SPI_SCK_PIN                   GPIO_PIN_3
#define SPI_FLASH_SPI_SCK_GPIO_PORT             GPIOB
#define SPI_FLASH_SPI_MISO_PIN                  GPIO_PIN_4  
#define SPI_FLASH_SPI_MISO_GPIO_PORT            GPIOB   
#define SPI_FLASH_SPI_MOSI_PIN                  GPIO_PIN_5  
#define SPI_FLASH_SPI_MOSI_GPIO_PORT            GPIOB     
#define SPI_FLASH_CS_PIN                        GPIO_PIN_7        
#define SPI_FLASH_CS_GPIO_PORT                  GPIOB                   



/* Private typedef -----------------------------------------------------------*/
//#define SPI_FLASH_PageSize      4096
#define SPI_FLASH_PageSize      256
#define SPI_FLASH_PerWritePageSize      256

/* Private define ------------------------------------------------------------*/
#define W25X_WriteEnable		      0x06 
#define W25X_WriteDisable		      0x04 
#define W25X_ReadStatusReg		    0x05 
#define W25X_WriteStatusReg		    0x01 
#define W25X_ReadData			        0x03 
#define W25X_FastReadData		      0x0B 
#define W25X_FastReadDual		      0x3B 
#define W25X_PageProgram		      0x02 
#define W25X_BlockErase			      0xD8 
#define W25X_SectorErase		      0x20 
#define W25X_ChipErase			      0xC7 
#define W25X_PowerDown			      0xB9 
#define W25X_ReleasePowerDown	    0xAB 
#define W25X_DeviceID			        0xAB 
#define W25X_ManufactDeviceID   	0x90 
#define W25X_JedecDeviceID		    0x9F 

#define WIP_Flag                  0x01  /* Write In Progress (WIP) flag */

#define Dummy_Byte                0xFF


#define W25_SECTOR_SIZE 4096

class TWINBOND25X16 {
	protected:
		
		static u8 SPI_FLASH_ReadByte(void);
		static u8 SPI_FLASH_SendByte(u8 byte);

		void SPI_FLASH_WriteEnable(void);
		void SPI_FLASH_WaitForWriteEnd(void);
		void SPI_FLASH_StartReadSequence(u32 ReadAddr);

	public:
		TWINBOND25X16 ();
		void Init(void);
		u32 ReadDeviceID(void);
		u32 ReadID(void);
		void SectorErase (u32 SectorAddr);		// принимает адрес памяти (не номер сектора) 
		void SectorErase (u32 SectorAddr, unsigned short stop_n);
		
		void BulkErase(void);
		void PageWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite);
		void BufferWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite);
		void BufferRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead);
		void Copy (u32 Src, u32 Dst, u32 sz);
		
		static void PowerDown(void);
		static void PinsPowdown ();
		void WAKEUP(void);
		void StatusRegUnprotect ();

};


extern TWINBOND25X16 W25X;


#endif /* __SPI_FLASH_H */
