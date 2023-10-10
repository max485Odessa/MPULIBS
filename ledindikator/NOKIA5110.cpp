#include "NOKIA5110.hpp"
#include "stm32f10x.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "LCD132x64.h"


extern TLCDCANVABW LCD1;

namespace N5110 {

unsigned char LineGr[LCD_WIDTH];
static SPI_InitTypeDef   SPI_InitStructure_rf;


void LightSetTo (BitAction BitVal)
{
GPIO_WriteBit (GPIOA, GPIO_Pin_9, BitVal);
}


static void ResetPinTo (BitAction BitVal)
{
GPIO_WriteBit (GPIOA, GPIO_Pin_8, BitVal);
}



static void DCPinTo (BitAction BitVal)
{
GPIO_WriteBit (GPIOB, GPIO_Pin_14, BitVal);
}



static void CEPinTo (BitAction BitVal)
{
GPIO_WriteBit (GPIOB, GPIO_Pin_12, BitVal);
}




// Helpful function to directly command the LCD to go to a 
// specific x,y coordinate.
void gotoXY(int x, int y)
{
  LCDWrite(0, 0x80 | x);  // Column.
  LCDWrite(0, 0x40 | y);  // Row.  ?
}



uint8_t spi_send(uint8_t bSend)
{
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
  SPI_I2S_SendData(SPI2, bSend);
  //while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
  return 0;//SPI_I2S_ReceiveData(SPI2);
}




void digitalWrite (ENOKIA Pinr, unsigned char val)
{
BitAction BitVal = (BitAction)val;
switch ((unsigned char)Pinr)
	{
	case blPin:
		{
		LightSetTo (BitVal);
		break;
		}
	case rstPin:
		{
		ResetPinTo (BitVal);
		break;
		}
	case dcPin:
		{
		DCPinTo (BitVal);
		break;
		}
	case scePin:
		{
		CEPinTo (BitVal);
		break;
		}
	}
}




void LCDWrite(unsigned char data_or_command, unsigned char data) 
{
  //Tell the LCD that we are writing either to data or a command
  digitalWrite(dcPin, data_or_command); 

  //Send the data
  digitalWrite(scePin, LOW);
  spi_send(data); //shiftOut(sdinPin, sclkPin, MSBFIRST, data);
  digitalWrite(scePin, HIGH);
}





void updateDisplay()
{
BUFPAR Bpr;
Bpr.lpRam = LineGr;
Bpr.Sizes = sizeof(LineGr);
unsigned char szlen;
unsigned char IndxR = 0;
unsigned char *lsrc;
while (IndxR < (LCD_HEIGHT/8))
	{
	if (LCD1.CopyCanva_BLine (&Bpr, IndxR))
		{
		gotoXY(0, IndxR);
		szlen = LCD_WIDTH;
		lsrc = (unsigned char*)Bpr.lpRam;
		while (szlen)
			{
			LCDWrite(LCD_DATA, lsrc[0]);
			lsrc++;
			szlen--;
			}
		}
	IndxR++;
	}
}



void Nokia5110_Init ()
{
  //Configure control pins
	GPIO_InitTypeDef GPIO_InitStructure;
  RCC_PCLK2Config(RCC_HCLK_Div2); 
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15;	// hardware: mosi, sck
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
  /* SPIy Config -------------------------------------------------------------*/
  SPI_InitStructure_rf.SPI_Direction = SPI_Direction_1Line_Tx;//SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure_rf.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure_rf.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure_rf.SPI_CPOL = SPI_CPOL_Low;//SPI_CPOL_High;// SPI_CPOL_Low;
  SPI_InitStructure_rf.SPI_CPHA = SPI_CPHA_2Edge;//SPI_CPHA_1Edge;//SPI_CPHA_2Edge;
  SPI_InitStructure_rf.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure_rf.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
  SPI_InitStructure_rf.SPI_FirstBit = SPI_FirstBit_MSB;//SPI_FirstBit_LSB;
  SPI_InitStructure_rf.SPI_CRCPolynomial = 7;
  SPI_Init(SPI2, &SPI_InitStructure_rf);

  SPI_Cmd(SPI2, ENABLE);

  //Reset the LCD to a known state
	//digitalWrite (scePin, HIGH);
	//CEPinTo (Bit_SET);
	//
	ResetPinTo (Bit_RESET);
	ResetPinTo (Bit_SET);
	
  LCDWrite(LCD_COMMAND, 0x21); //Tell LCD extended commands follow
  LCDWrite(LCD_COMMAND, 0xB0); //Set LCD Vop (Contrast)
  LCDWrite(LCD_COMMAND, 0x04); //Set Temp coefficent
  LCDWrite(LCD_COMMAND, 0x14); //LCD bias mode 1:48 (try 0x13)
  //We must send 0x20 before modifying the display control mode
  LCDWrite(LCD_COMMAND, 0x20); 
  LCDWrite(LCD_COMMAND, 0x0C); //Set display control, normal mode.
	
	
}

};

