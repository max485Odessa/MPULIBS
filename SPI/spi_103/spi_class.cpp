#include "spi_class.h"



	
TSPI1HARD::TSPI1HARD (S_SPIGPIO_T *g): spigpio (g)
{
SpiHandle.Instance = SPI1;


F_Transaction = false;
Tech_wait_s = 0; 
Tech_wait_p = 0;
}


/*
GPIO_InitTypeDef  GPIO_InitStruct;

  if(hspi->Instance == SPIx)
  {     

    SPIx_SCK_GPIO_CLK_ENABLE();
    SPIx_MISO_GPIO_CLK_ENABLE();
    SPIx_MOSI_GPIO_CLK_ENABLE();

    SPIx_CLK_ENABLE(); __HAL_RCC_SPI1_CLK_ENABLE
    

    GPIO_InitStruct.Pin       = SPIx_SCK_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed     = GPIO_SPEED_LOW;
    HAL_GPIO_Init(SPIx_SCK_GPIO_PORT, &GPIO_InitStruct);


    GPIO_InitStruct.Pin = SPIx_MISO_PIN;
    HAL_GPIO_Init(SPIx_MISO_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = SPIx_MOSI_PIN;
    HAL_GPIO_Init(SPIx_MOSI_GPIO_PORT, &GPIO_InitStruct);
  }
*/


void TSPI1HARD::init ()
{
	SPI_InitTypeDef SPI_InitStructure;
	
		_pin_low_init_out_pp_af (const_cast<S_GPIOPIN*>(&spigpio->mosi), EHRTGPIOSPEED_MID);		// mosi
		_pin_low_init_out_pp_af (const_cast<S_GPIOPIN*>(&spigpio->sck), EHRTGPIOSPEED_MID);		// sck
		_pin_low_init_out_pp_af (const_cast<S_GPIOPIN*>(&spigpio->miso), EHRTGPIOSPEED_MID);		// sck
		_pin_low_init_out_pp (const_cast<S_GPIOPIN*>(&spigpio->cs),1, EHRTGPIOSPEED_MID);		// cs

	
	__HAL_RCC_SPI1_CLK_ENABLE ();
  SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
  SpiHandle.Init.Direction         = SPI_DIRECTION_2LINES;
  SpiHandle.Init.CLKPhase          = SPI_PHASE_1EDGE;
  SpiHandle.Init.CLKPolarity       = SPI_POLARITY_LOW;
  SpiHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
  SpiHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
  SpiHandle.Init.TIMode            = SPI_TIMODE_DISABLE;
  SpiHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
  SpiHandle.Init.CRCPolynomial     = 7;
  SpiHandle.Init.NSS               = SPI_NSS_SOFT;
	SpiHandle.Init.Mode = SPI_MODE_MASTER;
	HAL_SPI_Init(&SpiHandle);
	__HAL_SPI_ENABLE(&SpiHandle);
	

}



void TSPI1HARD::deinit ()
{

}



void TSPI1HARD::cs_0 ()
{
_pin_output (const_cast<S_GPIOPIN*>(&spigpio->cs), false);
}



void TSPI1HARD::cs_1 ()
{
_pin_output (const_cast<S_GPIOPIN*>(&spigpio->cs), true);
}


uint8_t TSPI1HARD::txrx (uint8_t byte)
{
while (__HAL_SPI_GET_FLAG(&SpiHandle, SPI_FLAG_TXE) == RESET);
SpiHandle.Instance->DR = byte; 
while (__HAL_SPI_GET_FLAG(&SpiHandle, SPI_FLAG_RXNE) == RESET);
return SpiHandle.Instance->DR;
}



void TSPI1HARD::Wait_cs (unsigned char wt_time)
{
while (wt_time)
	{
	wt_time--;
	}
}



void TSPI1HARD::csp_spi_read_fast (uint8_t reg_addr_len, uint8_t *reg_addr, uint16_t data_len, uint8_t *data)
{
//	uint8_t data_tmp;
	uint16_t i;
	
	//TransmitSPIData (*reg_addr++);
	for (i = 0; i < reg_addr_len; i++)
		{
		//data_tmp = *reg_addr++;
		txrx (*reg_addr++);
		}
	//TransmitSPIData (0);	//dummy write to start read
	for (i = 0; i < data_len; i++)
		{
		*data++ = txrx (0);
		}
}



void TSPI1HARD::csp_spi_write_fast (uint8_t reg_addr_len, uint8_t *reg_addr, uint16_t data_len, uint8_t *data)
{
	uint8_t data_tmp;
	uint16_t i;

	txrx ( *reg_addr++);
	for (i = 1; i < reg_addr_len; i++)
		{
		data_tmp = *reg_addr++;
		txrx (data_tmp);
		}
	for (i = 0; i < data_len; i++)
		{
		data_tmp = *data++;
		txrx (data_tmp);
		}
}


/*
TSPI::TSPI (const S_SPIPINS_T *spipns)
{
	_pin_low_init_out_pp (const_cast<S_GPIOPIN*>(&spipns->cs), 1);
	_pin_low_init_out_pp (const_cast<S_GPIOPIN*>(&spipns->mosi), 1);
	_pin_low_init_out_pp (const_cast<S_GPIOPIN*>(&spipns->sck), 1);
	_pin_low_init_in (const_cast<S_GPIOPIN*>(&spipns->miso), 1);
	
	mosi_port = spipns->mosi.port;
	mosi_pin = spipns->mosi.pin;
	
	miso_port = spipns->miso.port;
	miso_pin = spipns->miso.pin;
	
	sck_port = spipns->sck.port;
	sck_pin = spipns->sck.pin;	
	
	cs_port = spipns->cs.port;
	cs_pin = spipns->cs.pin;	
	
	cs_1 ();
}



void TSPI::cs_0 ()
{
cs_port->BRR = cs_pin;
}



void TSPI::cs_1 ()
{
cs_port->BSRR = cs_pin;	
}



uint8_t TSPI::transact (uint8_t byte)
{
unsigned char rv = 0, cnt = 8;
	while (cnt)
		{
		rv <<= 1;	
		// MOSI DATA SET
		if (byte & 128)
			{
			mosi_port->BSRR = mosi_pin;
			}
		else
			{
			mosi_port->BRR = mosi_pin;
			}
		// SCK 
		sck_port->BSRR = sck_pin;
		if (miso_port->IDR & miso_pin) rv |= 1;	// MISO
		sck_port->BRR = sck_pin;
		byte <<= 1;
		cnt--;
		}
return rv;
}
*/

