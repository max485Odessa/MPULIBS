#ifndef __SPI_FLASH_H_
#define __SPI_FLASH_H_



#include "stm32f10x.h"
#include "stm32f10x_spi.h"
#include "SYSBIOS.H"
#include "hard_rut.h"

/*
#define SPI_FLASH_SPI_SCK_PIN                   GPIO_Pin_13
#define SPI_FLASH_SPI_SCK_GPIO_PORT             GPIOB                      
#define SPI_FLASH_SPI_MISO_PIN                  GPIO_Pin_14  
#define SPI_FLASH_SPI_MISO_GPIO_PORT            GPIOB                     
#define SPI_FLASH_SPI_MOSI_PIN                  GPIO_Pin_15  
#define SPI_FLASH_SPI_MOSI_GPIO_PORT            GPIOB                 
#define SPI_FLASH_CS_PIN                        GPIO_Pin_12             
#define SPI_FLASH_CS_GPIO_PORT                  GPIOB                   
*/


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

/*
typedef struct {
    u8 busy : 1;
    u8 write_enable : 1;
    u8 block_protect : 3;
    u8 top_bot_ptotect : 1;
    u8 sector_protect : 1;
    u8 status_reg_protect0 : 1;
} STATUS_REG1_STRUCT_t;
 
typedef union {
    u8 all;
    STATUS_REG1_STRUCT_t bit;
} Status_reg_1_t;
*/

#define W25_SECTOR_SIZE 4096

enum ESSPIPIN {ESSPIPIN_MISO = 0, ESSPIPIN_MOSI = 1, ESSPIPIN_SCK = 2, ESSPIPIN_CS = 3, ESSPIPIN_ENDENUM = 4};

typedef struct {
	uint8_t sn[8];
} S_W25QSERIALN_T;

class TWINBOND25X16 {
		const S_GPIOPIN *pins;
		void CS_LOW ();
		void CS_HIGH ();
		void Init ();
	
	protected:
		u8 SPI_FLASH_ReadByte(void);
		u8 SPI_FLASH_SendByte(u8 byte);
		u16 SPI_FLASH_SendHalfWord(u16 HalfWord);
		void SPI_FLASH_WriteEnable(void);
		void SPI_FLASH_WaitForWriteEnd(void);
		void SPI_FLASH_StartReadSequence(u32 ReadAddr);

	public:
		TWINBOND25X16 (const S_GPIOPIN *mempins);
		
		bool GetSerial (S_W25QSERIALN_T *dst);
		u32 ReadDeviceID(void);
		u32 ReadID(void);
		void SectorErase (u32 SectorAddr);		// ïðèíèìàåò àäðåñ ïàìÿòè (íå íîìåð ñåêòîðà) 
		void SectorErase (u32 SectorAddr, unsigned short stop_n);
		
		void BulkErase(void);
		void PageWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite);
		void BufferWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite);
		void BufferRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead);
		void Copy (u32 Src, u32 Dst, u32 sz);
		
		void PowerDown(void);
		void WAKEUP(void);
		void StatusRegUnprotect ();

};


extern TWINBOND25X16 W25X;


#endif /* __SPI_FLASH_H */
