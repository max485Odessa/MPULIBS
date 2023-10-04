#include "HX1230_SPI.H"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "SYSBIOS.H"

//extern void Delay_Ms(unsigned short timss);

// din emosi  reset emiso  

THX1230SPI::THX1230SPI ()
{
}



void THX1230SPI::spi_send (unsigned char bSend)
{
unsigned char cnt = 8;
while (cnt)
	{
	BitStrobeTx (bSend & 128);
	bSend = bSend << 1;
	cnt--;
	}
}



void THX1230SPI::spi_send_cmd (unsigned char bSend)
{
CSSetTo (0);
BitStrobeTx (0);
spi_send (bSend);
CSSetTo (1);
}




void THX1230SPI::spi_send_data (unsigned char bSend)
{
CSSetTo (0);
BitStrobeTx (1);
spi_send (bSend);
CSSetTo (1);
}



void THX1230SPI::SetDataTo (bool val)		// +
{
if (val)
	{
	GPIO_WriteBit (GPIOB, GPIO_Pin_5, Bit_SET);
	}
else
	{
	GPIO_WriteBit (GPIOB, GPIO_Pin_5, Bit_RESET);
	}
}



void THX1230SPI::ResetTo (bool val)
{
if (val)
	{
	GPIO_WriteBit (GPIOB, GPIO_Pin_4, Bit_SET);
	}
else
	{
	GPIO_WriteBit (GPIOB, GPIO_Pin_4, Bit_RESET);
	}
}


void THX1230SPI::BitStrobeTx (bool val)
{
SetDataTo (val);
//Delay_us (1);
StrobeCLK ();
}



void THX1230SPI::StrobeCLK ()
{
GPIO_WriteBit (GPIOB, GPIO_Pin_3, Bit_SET);
//Delay_us (1);
GPIO_WriteBit (GPIOB, GPIO_Pin_3, Bit_RESET);
//Delay_us (1);
}




void THX1230SPI::CSSetTo (bool val)
{
if (val)
	{
	GPIO_WriteBit (GPIOA, GPIO_Pin_15, Bit_SET);
	}
else
	{
	GPIO_WriteBit (GPIOA, GPIO_Pin_15, Bit_RESET);
	}
}



void THX1230SPI::SpiModeInit ()
{
	GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_15;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_WriteBit (GPIOB, GPIO_Pin_3, Bit_RESET);		// clk
	ResetTo (0);
	CSSetTo (1);
	SYSBIOS::Wait (50);
	ResetTo (1);
	CSSetTo (0);
	SYSBIOS::Wait (1);
	CSSetTo (1);
	SYSBIOS::Wait (1);
}



void THX1230SPI::LCD_init(void)
{
	SpiModeInit ();
	PowerUp ();
}






void THX1230SPI::Contrast (unsigned char ucContrast)
{
  if (ucContrast > 31) ucContrast = 31;
  spi_send_cmd (0x80 + ucContrast); // set contrast
} 



void THX1230SPI::PowerUp ()
{
	spi_send_cmd (0x2F);	
	spi_send_cmd (0x91);		// напряжение  0x90
	spi_send_cmd (0xA6);		// инверсия
	spi_send_cmd (0xA4); 		// черный экран
	spi_send_cmd (0xAF);  	// белый экран
	spi_send_cmd (0x40);
	spi_send_cmd (0xB0);
	spi_send_cmd (0x10);
	spi_send_cmd (0x0);
}



void THX1230SPI::PowerDown()
{
    spi_send_cmd(0xae); // power down
}


void THX1230SPI::Write_Data (char *lSrc, unsigned char sizes)
{
if (lSrc && sizes)
	{
	//lSrc = lSrc + sizes - 1;
	while (sizes)
		{
		spi_send_data (lSrc[0]);
		lSrc++;
		sizes--;
		}
	}
}



void THX1230SPI::Fill_Data (unsigned char Fdat, unsigned char sizes)
{
if (sizes)
	{
	while (sizes)
		{
		spi_send_data (Fdat);
		sizes--;
		}
	}
}



void THX1230SPI::LCD_Clear (void)
{
	unsigned char Indx = 0;
	while (Indx < 4)
		{
		Set_Page (Indx);
		Fill_Data (234, SSD_WIDTH);
		Indx++;
		}
}



void THX1230SPI::Set_Page (unsigned char page)
{
	spi_send_cmd( 0xB0 + (page & 0x0F));
	spi_send_cmd( 0);			// 0
	spi_send_cmd( 0x10);	// 0
}



void THX1230SPI::RefreshAllDisplay (TLCDCANVABW *lCanv)
{
unsigned char Indx = 0;
unsigned long sz;
BUFPAR bpr;
bpr.lpRam = DataBuf;
bpr.Sizes = sizeof(DataBuf);
while (Indx < 9)
	{
	sz = lCanv->CopyCanva_BLine (&bpr, Indx);
	if (sz)
		{
		Set_Page (Indx);
		Write_Data (DataBuf, sizeof(DataBuf));
		}
	Indx++;
	}
}



