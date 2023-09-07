#include "winb25x16_stml4xx.h"
#include "stm32l4xx_hal.h"
#include "app.h"
#include "TWatchDog.h"

__STATIC_INLINE void SPI_FLASH_CS_LOW () {SPI_FLASH_CS_GPIO_PORT->BRR = SPI_FLASH_CS_PIN;}  //  GPIO_ResetBits (SPI_FLASH_CS_GPIO_PORT, SPI_FLASH_CS_PIN);}
__STATIC_INLINE void SPI_FLASH_CS_HIGH () {SPI_FLASH_CS_GPIO_PORT->BSRR = SPI_FLASH_CS_PIN;}			// GPIO_SetBits (SPI_FLASH_CS_GPIO_PORT, SPI_FLASH_CS_PIN);

static const S_GPIOPIN flpinsarr[4] = {{SPI_FLASH_SPI_MOSI_GPIO_PORT, SPI_FLASH_SPI_MOSI_PIN}, {SPI_FLASH_SPI_MOSI_GPIO_PORT, SPI_FLASH_SPI_SCK_PIN}, {SPI_FLASH_SPI_MISO_GPIO_PORT, SPI_FLASH_SPI_MISO_PIN}, {SPI_FLASH_CS_GPIO_PORT, SPI_FLASH_CS_PIN}};


TWINBOND25X16::TWINBOND25X16 ()
{

}


/*
void TWINBOND25X16::SPI_Enabled (bool state)
{
	const unsigned short val = 0x40;
  if (state)
		{
    SPI_FLASH_SPI->CR1 |= val;
		}
  else
		{
    SPI_FLASH_SPI->CR1 &= ~val;
		}
}
*/



/*******************************************************************************
* Function Name  : SPI_FLASH_Init
* Description    : Initializes the peripherals used by the SPI FLASH driver.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TWINBOND25X16::Init (void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

	__HAL_RCC_GPIOB_CLK_ENABLE();

	GPIO_InitStruct.Pin = SPI_FLASH_SPI_MOSI_PIN | SPI_FLASH_SPI_SCK_PIN;		// mosi , sck
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(SPI_FLASH_SPI_MOSI_GPIO_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = SPI_FLASH_SPI_MISO_PIN;		// miso
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init (SPI_FLASH_SPI_MISO_GPIO_PORT, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = SPI_FLASH_CS_PIN;					// cs
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init (SPI_FLASH_CS_GPIO_PORT, &GPIO_InitStruct);
	
  SPI_FLASH_CS_HIGH();

/*
	HAL_SPI_DeInit(&hspi1);

  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;// SPI_POLARITY_HIGH;//SPI_POLARITY_HIGH;//SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;//SPI_TIMODE_DISABLE;//SPI_TIMODE_ENABLE;//SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_8BIT;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;//SPI_NSS_PULSE_DISABLE;//SPI_NSS_PULSE_DISABLE;

  if (HAL_SPI_Init(&hspi1) != HAL_OK)
		{
			Error_Handler();
		}

		__HAL_SPI_ENABLE(&hspi1);
*/
		
	WAKEUP(); 
}



void TWINBOND25X16::PinsPowdown ()
{
	_pin_low_init_in (const_cast<S_GPIOPIN*>(flpinsarr), 4);
}



/*******************************************************************************
* Function Name  : SPI_FLASH_SectorErase
* Description    : Erases the specified FLASH sector.
* Input          : SectorAddr: address of the sector to erase.
* Output         : None
* Return         : None
*******************************************************************************/
void TWINBOND25X16::SectorErase(u32 SectorAddr)
{
  /* Send write enable instruction */
	SectorAddr = SectorAddr & (0xFFFFFFFF - (W25_SECTOR_SIZE - 1));
  SPI_FLASH_WriteEnable();
  SPI_FLASH_WaitForWriteEnd();
  /* Sector Erase */
  /* Select the FLASH: Chip Select low */
  SPI_FLASH_CS_LOW();
  /* Send Sector Erase instruction */
  SPI_FLASH_SendByte(W25X_SectorErase);
  /* Send SectorAddr high nibble address byte */
  SPI_FLASH_SendByte((SectorAddr & 0xFF0000) >> 16);
  /* Send SectorAddr medium nibble address byte */
  SPI_FLASH_SendByte((SectorAddr & 0xFF00) >> 8);
  /* Send SectorAddr low nibble address byte */
  SPI_FLASH_SendByte(SectorAddr & 0xFF);
  /* Deselect the FLASH: Chip Select high */
  SPI_FLASH_CS_HIGH();
  /* Wait the end of Flash writing */
  SPI_FLASH_WaitForWriteEnd();

}



void TWINBOND25X16::SectorErase (u32 start_n, unsigned short stop_n)
{
while (stop_n)
	{
	SectorErase(start_n);
	start_n += W25_SECTOR_SIZE;
	stop_n--;
	}
}

/*******************************************************************************
* Function Name  : SPI_FLASH_BulkErase
* Description    : Erases the entire FLASH.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TWINBOND25X16::BulkErase(void)
{
  /* Send write enable instruction */
  SPI_FLASH_WriteEnable();

  /* Bulk Erase */
  /* Select the FLASH: Chip Select low */
  SPI_FLASH_CS_LOW();
  /* Send Bulk Erase instruction  */
  SPI_FLASH_SendByte(W25X_ChipErase);
  /* Deselect the FLASH: Chip Select high */
  SPI_FLASH_CS_HIGH();

  /* Wait the end of Flash writing */
  SPI_FLASH_WaitForWriteEnd();
}

/*******************************************************************************
* Function Name  : SPI_FLASH_PageWrite
* Description    : Writes more than one byte to the FLASH with a single WRITE
*                  cycle(Page WRITE sequence). The number of byte can't exceed
*                  the FLASH page size.
* Input          : - pBuffer : pointer to the buffer  containing the data to be
*                    written to the FLASH.
*                  - WriteAddr : FLASH's internal address to write to.
*                  - NumByteToWrite : number of bytes to write to the FLASH,
*                    must be equal or less than "SPI_FLASH_PageSize" value.
* Output         : None
* Return         : None
*******************************************************************************/
void TWINBOND25X16::PageWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
  /* Enable the write access to the FLASH */
	if (!NumByteToWrite) return;
	TWATCH::Wdr ();
  SPI_FLASH_WriteEnable();

  /* Select the FLASH: Chip Select low */
  SPI_FLASH_CS_LOW();
  /* Send "Write to Memory " instruction */
  SPI_FLASH_SendByte(W25X_PageProgram);
  /* Send WriteAddr high nibble address byte to write to */
  SPI_FLASH_SendByte((WriteAddr & 0xFF0000) >> 16);
  /* Send WriteAddr medium nibble address byte to write to */
  SPI_FLASH_SendByte((WriteAddr & 0xFF00) >> 8);
  /* Send WriteAddr low nibble address byte to write to */
  SPI_FLASH_SendByte(WriteAddr & 0xFF);

  if(NumByteToWrite > SPI_FLASH_PerWritePageSize)
  {
     NumByteToWrite = SPI_FLASH_PerWritePageSize;
     //printf("\n\r Err: SPI_FLASH_PageWrite too large!");
  }

  /* while there is data to be written on the FLASH */
  while (NumByteToWrite--)
  {
    /* Send the current byte */
    SPI_FLASH_SendByte(*pBuffer);
    /* Point on the next byte to be written */
    pBuffer++;
  }

  /* Deselect the FLASH: Chip Select high */
  SPI_FLASH_CS_HIGH();

  /* Wait the end of Flash writing */
  SPI_FLASH_WaitForWriteEnd();
	//SYSBIOS::Wait (4);
}

/*******************************************************************************
* Function Name  : SPI_FLASH_BufferWrite
* Description    : Writes block of data to the FLASH. In this function, the
*                  number of WRITE cycles are reduced, using Page WRITE sequence.
* Input          : - pBuffer : pointer to the buffer  containing the data to be
*                    written to the FLASH.
*                  - WriteAddr : FLASH's internal address to write to.
*                  - NumByteToWrite : number of bytes to write to the FLASH.
* Output         : None
* Return         : None
*******************************************************************************/
void TWINBOND25X16::BufferWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
  u8 NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

  Addr = WriteAddr % SPI_FLASH_PageSize;
  count = SPI_FLASH_PageSize - Addr;
  NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
  NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

  if (Addr == 0) /* WriteAddr is SPI_FLASH_PageSize aligned  */
  {
    if (NumOfPage == 0) /* NumByteToWrite < SPI_FLASH_PageSize */
    {
      PageWrite(pBuffer, WriteAddr, NumByteToWrite);
    }
    else /* NumByteToWrite > SPI_FLASH_PageSize */
    {
      while (NumOfPage--)
      {
        PageWrite(pBuffer, WriteAddr, SPI_FLASH_PageSize);
        WriteAddr +=  SPI_FLASH_PageSize;
        pBuffer += SPI_FLASH_PageSize;
      }

      PageWrite(pBuffer, WriteAddr, NumOfSingle);
    }
  }
  else /* WriteAddr is not SPI_FLASH_PageSize aligned  */
  {
    if (NumOfPage == 0) /* NumByteToWrite < SPI_FLASH_PageSize */
    {
      if (NumOfSingle > count) /* (NumByteToWrite + WriteAddr) > SPI_FLASH_PageSize */
      {
        temp = NumOfSingle - count;

        PageWrite(pBuffer, WriteAddr, count);
        WriteAddr +=  count;
        pBuffer += count;

        PageWrite(pBuffer, WriteAddr, temp);
      }
      else
      {
        PageWrite(pBuffer, WriteAddr, NumByteToWrite);
      }
    }
    else /* NumByteToWrite > SPI_FLASH_PageSize */
    {
      NumByteToWrite -= count;
      NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
      NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

      PageWrite(pBuffer, WriteAddr, count);
      WriteAddr +=  count;
      pBuffer += count;

      while (NumOfPage--)
      {
        PageWrite(pBuffer, WriteAddr, SPI_FLASH_PageSize);
        WriteAddr +=  SPI_FLASH_PageSize;
        pBuffer += SPI_FLASH_PageSize;
      }

      if (NumOfSingle != 0)
      {
        PageWrite(pBuffer, WriteAddr, NumOfSingle);
      }
    }
  }
}

/*******************************************************************************
* Function Name  : SPI_FLASH_BufferRead
* Description    : Reads a block of data from the FLASH.
* Input          : - pBuffer : pointer to the buffer that receives the data read
*                    from the FLASH.
*                  - ReadAddr : FLASH's internal address to read from.
*                  - NumByteToRead : number of bytes to read from the FLASH.
* Output         : None
* Return         : None
*******************************************************************************/
void TWINBOND25X16::BufferRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
  /* Select the FLASH: Chip Select low */
  SPI_FLASH_CS_LOW();

  /* Send "Read from Memory " instruction */
  SPI_FLASH_SendByte(W25X_ReadData);

  /* Send ReadAddr high nibble address byte to read from */
  SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
  /* Send ReadAddr medium nibble address byte to read from */
  SPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
  /* Send ReadAddr low nibble address byte to read from */
  SPI_FLASH_SendByte(ReadAddr & 0xFF);

  while (NumByteToRead--) /* while there is data to be read */
  {
    /* Read a byte from the FLASH */
    *pBuffer = SPI_FLASH_SendByte(Dummy_Byte);
    /* Point to the next location where the byte read will be saved */
    pBuffer++;
  }

  /* Deselect the FLASH: Chip Select high */
  SPI_FLASH_CS_HIGH();
}

/*******************************************************************************
* Function Name  : SPI_FLASH_ReadID
* Description    : Reads FLASH identification.
* Input          : None
* Output         : None
* Return         : FLASH identification
*******************************************************************************/
u32 TWINBOND25X16::ReadID(void)
{
  u32 Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;

  /* Select the FLASH: Chip Select low */
  SPI_FLASH_CS_LOW();

  /* Send "RDID " instruction */
  SPI_FLASH_SendByte(W25X_JedecDeviceID);

  /* Read a byte from the FLASH */
  Temp0 = SPI_FLASH_SendByte(Dummy_Byte);

  /* Read a byte from the FLASH */
  Temp1 = SPI_FLASH_SendByte(Dummy_Byte);

  /* Read a byte from the FLASH */
  Temp2 = SPI_FLASH_SendByte(Dummy_Byte);

  /* Deselect the FLASH: Chip Select high */
  SPI_FLASH_CS_HIGH();

  Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;

  return Temp;
}
/*******************************************************************************
* Function Name  : SPI_FLASH_ReadID
* Description    : Reads FLASH identification.
* Input          : None
* Output         : None
* Return         : FLASH identification
*******************************************************************************/
u32 TWINBOND25X16::ReadDeviceID(void)
{
  u32 Temp = 0;

  /* Select the FLASH: Chip Select low */
  SPI_FLASH_CS_LOW();

  /* Send "RDID " instruction */
  SPI_FLASH_SendByte(W25X_DeviceID);
  SPI_FLASH_SendByte(Dummy_Byte);
  SPI_FLASH_SendByte(Dummy_Byte);
  SPI_FLASH_SendByte(Dummy_Byte);
  
  /* Read a byte from the FLASH */
  Temp = SPI_FLASH_SendByte(Dummy_Byte);

  /* Deselect the FLASH: Chip Select high */
  SPI_FLASH_CS_HIGH();

  return Temp;
}
/*******************************************************************************
* Function Name  : SPI_FLASH_StartReadSequence
* Description    : Initiates a read data byte (READ) sequence from the Flash.
*                  This is done by driving the /CS line low to select the device,
*                  then the READ instruction is transmitted followed by 3 bytes
*                  address. This function exit and keep the /CS line low, so the
*                  Flash still being selected. With this technique the whole
*                  content of the Flash is read with a single READ instruction.
* Input          : - ReadAddr : FLASH's internal address to read from.
* Output         : None
* Return         : None
*******************************************************************************/
void TWINBOND25X16::SPI_FLASH_StartReadSequence(u32 ReadAddr)
{
  /* Select the FLASH: Chip Select low */
  SPI_FLASH_CS_LOW();

  /* Send "Read from Memory " instruction */
  SPI_FLASH_SendByte(W25X_ReadData);

  /* Send the 24-bit address of the address to read from -----------------------*/
  /* Send ReadAddr high nibble address byte */
  SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
  /* Send ReadAddr medium nibble address byte */
  SPI_FLASH_SendByte((ReadAddr& 0xFF00) >> 8);
  /* Send ReadAddr low nibble address byte */
  SPI_FLASH_SendByte(ReadAddr & 0xFF);
}

/*******************************************************************************
* Function Name  : SPI_FLASH_ReadByte
* Description    : Reads a byte from the SPI Flash.
*                  This function must be used only if the Start_Read_Sequence
*                  function has been previously called.
* Input          : None
* Output         : None
* Return         : Byte Read from the SPI Flash.
*******************************************************************************/
u8 TWINBOND25X16::SPI_FLASH_ReadByte(void)
{
  return (SPI_FLASH_SendByte(Dummy_Byte));
}

/*******************************************************************************
* Function Name  : SPI_FLASH_SendByte
* Description    : Sends a byte through the SPI interface and return the byte
*                  received from the SPI bus.
* Input          : byte : byte to send.
* Output         : None
* Return         : The value of the received byte.
*******************************************************************************/
u8 TWINBOND25X16::SPI_FLASH_SendByte(u8 byte)
{
unsigned char rv = 0, cnt = 8;
	while (cnt)
		{
		rv <<= 1;	
		// MOSI DATA SET
		if (byte & 128)
			{
			SPI_FLASH_SPI_MOSI_GPIO_PORT->BSRR = SPI_FLASH_SPI_MOSI_PIN;
			}
		else
			{
			SPI_FLASH_SPI_MOSI_GPIO_PORT->BRR = SPI_FLASH_SPI_MOSI_PIN;
			}
		// SCK 
		SPI_FLASH_SPI_SCK_GPIO_PORT->BSRR = SPI_FLASH_SPI_SCK_PIN;
		if (SPI_FLASH_SPI_MISO_GPIO_PORT->IDR & SPI_FLASH_SPI_MISO_PIN) rv |= 1;	// MISO
		SPI_FLASH_SPI_SCK_GPIO_PORT->BRR = SPI_FLASH_SPI_SCK_PIN;
		byte <<= 1;
		cnt--;
		}
return rv;
	/*
	while (SPI_I2S_GetFlagStatus(SPI_FLASH_SPI, SPI_I2S_FLAG_BSY) == SET) {};
  while (SPI_I2S_GetFlagStatus(SPI_FLASH_SPI, SPI_I2S_FLAG_TXE) == RESET) {};


  SPI_I2S_SendData(SPI_FLASH_SPI, byte);

	while (SPI_I2S_GetFlagStatus(SPI_FLASH_SPI, SPI_I2S_FLAG_BSY) == SET) {};
  while (SPI_I2S_GetFlagStatus(SPI_FLASH_SPI, SPI_I2S_FLAG_RXNE) == RESET) {};

  return SPI_I2S_ReceiveData(SPI_FLASH_SPI);
		*/
	
	
	/*

	while (__HAL_SPI_GET_FLAG(&hspi1, SPI_FLAG_BSY) == SET) {};
  while (__HAL_SPI_GET_FLAG(&hspi1, SPI_FLAG_TXE) == RESET) {};

	SPI_FLASH_SPI->DR = byte;
  //SPI_SendData(SPI_FLASH_SPI, byte);
	while (__HAL_SPI_GET_FLAG(&hspi1, SPI_FLAG_BSY) == SET) {};
	while (__HAL_SPI_GET_FLAG(&hspi1, SPI_FLAG_TXE) == RESET) {};
	while (__HAL_SPI_GET_FLAG(&hspi1, SPI_FLAG_RXNE) == RESET) {};
	*/
	
	/*
        HAL_SPI_Transmit(&hspi1,(uint8_t *)OPCODEW,sizeof(uint8_t),SPI_TRANSFER_TIMEOUT); // Send the MCP23S09 opcode, and write bit
        while(HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY);

        HAL_SPI_Transmit(&hspi1,(uint8_t *)&reg,sizeof(uint8_t),SPI_TRANSFER_TIMEOUT); // Send the register we want to write
        while(HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY);

        HAL_SPI_Transmit(&hspi1,(uint8_t *)&value,sizeof(uint8_t),SPI_TRANSFER_TIMEOUT); // Send the byte
        while(HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY);
				
				*/
/*
	while (__HAL_SPI_GET_FLAG(&hspi1, SPI_FLAG_BSY) == SET) {};
  while (__HAL_SPI_GET_FLAG(&hspi1, SPI_FLAG_TXE) == RESET) {};

	SPI_FLASH_SPI->DR = byte;
  //SPI_SendData(SPI_FLASH_SPI, byte);
	while (__HAL_SPI_GET_FLAG(&hspi1, SPI_FLAG_BSY) == SET) {};
	while (__HAL_SPI_GET_FLAG(&hspi1, SPI_FLAG_TXE) == RESET) {};
	while (__HAL_SPI_GET_FLAG(&hspi1, SPI_FLAG_RXNE) == RESET) {};


  return SPI_FLASH_SPI->DR;//SPI_ReceiveData(SPI_FLASH_SPI);
	*/	
		
}



/*******************************************************************************
* Function Name  : SPI_FLASH_WriteEnable
* Description    : Enables the write access to the FLASH.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TWINBOND25X16::SPI_FLASH_WriteEnable(void)
{
  /* Select the FLASH: Chip Select low */
  SPI_FLASH_CS_LOW();

  /* Send "Write Enable" instruction */
  SPI_FLASH_SendByte(W25X_WriteEnable);

  /* Deselect the FLASH: Chip Select high */
  SPI_FLASH_CS_HIGH();
}


void TWINBOND25X16::StatusRegUnprotect ()
{
	SPI_FLASH_WriteEnable();
	SPI_FLASH_CS_LOW();
	SPI_FLASH_SendByte(W25X_WriteStatusReg);
	SPI_FLASH_SendByte(0);
	SPI_FLASH_SendByte(0);
	SPI_FLASH_CS_HIGH();
}


/*******************************************************************************
* Function Name  : SPI_FLASH_WaitForWriteEnd
* Description    : Polls the status of the Write In Progress (WIP) flag in the
*                  FLASH's status  register  and  loop  until write  opertaion
*                  has completed.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TWINBOND25X16::SPI_FLASH_WaitForWriteEnd(void)
{
  u8 FLASH_Status = 0;

  /* Select the FLASH: Chip Select low */
  SPI_FLASH_CS_LOW();

  /* Send "Read Status Register" instruction */
  SPI_FLASH_SendByte(W25X_ReadStatusReg);

  /* Loop as long as the memory is busy with a write cycle */
  do
  {
    /* Send a dummy byte to generate the clock needed by the FLASH
    and put the value of the status register in FLASH_Status variable */
		TWATCH::Wdr ();
    FLASH_Status = SPI_FLASH_SendByte(Dummy_Byte);	 
  }
  while (((FLASH_Status & WIP_Flag)) == SET); /* Write in progress */

  /* Deselect the FLASH: Chip Select high */
  SPI_FLASH_CS_HIGH();
}



void TWINBOND25X16::Copy (u32 Src, u32 Dst, u32 sz)
{
	u8 buf[SPI_FLASH_PerWritePageSize];
	u16 len;
	while (sz)
		{
		TWATCH::Wdr ();
		len = sizeof(buf);
		if (len > sz) len = sz; 
		BufferRead (buf, Src, len);
		BufferWrite (buf, Dst, len);
		Src += len; Dst += len;
		sz -= len;
		}
}



void TWINBOND25X16::PowerDown(void)   
{ 
  /* Select the FLASH: Chip Select low */
  SPI_FLASH_CS_LOW();

  /* Send "Power Down" instruction */
  SPI_FLASH_SendByte(W25X_PowerDown);

  /* Deselect the FLASH: Chip Select high */
  SPI_FLASH_CS_HIGH();
}   



//??
void TWINBOND25X16::WAKEUP(void)   
{
  /* Select the FLASH: Chip Select low */
  SPI_FLASH_CS_LOW();

  /* Send "Power Down" instruction */
  SPI_FLASH_SendByte(W25X_ReleasePowerDown);

  /* Deselect the FLASH: Chip Select high */
  SPI_FLASH_CS_HIGH();                   //??TRES1
}   







/******************************END OF FILE*****************************/
