#include "spi_class.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"


	
TSPIHARD::TSPIHARD (ESPISEL s)
{
e_spi = s;
SPI_p = 0;
initpinsar = 0;

F_Transaction = false;
Tech_wait_s = 0; 
Tech_wait_p = 0;
}



void TSPIHARD::init ()
{
	SPI_InitTypeDef SPI_InitStructure;

	RCC_PCLK2Config(RCC_HCLK_Div1);
	SPI_p = 0;
	switch (e_spi)
		{
		case ESPISEL_SPI1:
			{
			SPI_p = SPI1;
			initpinsar = const_cast<S_GPIOPIN*>(spi_1_initarr);
			RCC_APB2PeriphClockCmd (RCC_APB2Periph_SPI1, ENABLE);
			break;
			}
		case ESPISEL_SPI2:
			{
			SPI_p = SPI2;
			initpinsar = const_cast<S_GPIOPIN*>(spi_2_initarr);
			RCC_APB1PeriphClockCmd (RCC_APB1Periph_SPI2, ENABLE);
			break;
			}
			default: break;
		}

	if (SPI_p)
		{
		_pin_low_init_af_o_pp (&initpinsar[ESPIPIN_MOSI], 2);		// mosi + sck
		_pin_low_init_out_pp (&initpinsar[ESPIPIN_CS], 1);		// cs
		_pin_low_init_in (&initpinsar[ESPIPIN_MISO], 1);		// miso
			
		SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
		SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
		SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
		SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low; // SPI_CPOL_Low; // SPI_CPOL_Low; // SPI_CPOL_Low; //SPI_CPOL_High; // SPI_CPOL_Low;
		SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge ; // SPI_CPHA_2Edge;  // SPI_CPHA_2Edge
		SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
		SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;//SPI_FirstBit_LSB;
		SPI_InitStructure.SPI_CRCPolynomial = 7;
		SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;		// 4
		SPI_Init(SPI_p, &SPI_InitStructure);
		SPI_Cmd(SPI_p, ENABLE);
		}
	
}



void TSPIHARD::deinit ()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_15 | GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//GPIO_Mode_AF_PP;// GPIO_Mode_IN_FLOATING;
	GPIO_Init (GPIOB, &GPIO_InitStructure);
	SPI_Cmd (SPI2, DISABLE);
	RCC_APB1PeriphClockCmd (RCC_APB1Periph_SPI2, DISABLE);

}



void TSPIHARD::cs_0 ()
{
_pin_pp_to (&initpinsar[ESPIPIN_CS], false);
//GPIO_ResetBits (GPIOB, GPIO_Pin_12);
}



void TSPIHARD::cs_1 ()
{
_pin_pp_to (&initpinsar[ESPIPIN_CS], true);
}


uint8_t TSPIHARD::txrx (uint8_t byte)
{
while (SPI_I2S_GetFlagStatus(SPI_p, SPI_I2S_FLAG_TXE) == RESET);
SPI_I2S_SendData(SPI_p, byte);
while (SPI_I2S_GetFlagStatus(SPI_p, SPI_I2S_FLAG_RXNE) == RESET);
return SPI_I2S_ReceiveData(SPI_p);
}



void TSPIHARD::Wait_cs (unsigned char wt_time)
{
while (wt_time)
	{
	wt_time--;
	}
}



void TSPIHARD::csp_spi_read_fast (uint8_t reg_addr_len, uint8_t *reg_addr, uint16_t data_len, uint8_t *data)
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



void TSPIHARD::csp_spi_write_fast(uint8_t reg_addr_len, uint8_t *reg_addr, uint16_t data_len, uint8_t *data)
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

