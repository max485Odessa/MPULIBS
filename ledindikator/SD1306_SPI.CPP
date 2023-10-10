#include "SD1306_SPI.H"



extern void Delay_Ms(unsigned short timss);



TSSD1306SPI::TSSD1306SPI ()
{
}






void TSSD1306SPI::spi_send (uint8_t bSend)
{
unsigned char cnt = 8;
while (cnt)
	{
	BitStrobeTx (bSend & 128);
	bSend = bSend << 1;
	cnt--;
	}
}



void TSSD1306SPI::spi_send_cmd (uint8_t bSend)
{
BitStrobeTx (0);
spi_send (bSend);
}




void TSSD1306SPI::spi_send_data (uint8_t bSend)
{
BitStrobeTx (1);
spi_send (bSend);
}



void TSSD1306SPI::SetDataTo (bool val)
{
if (val)
	{
	GPIO_WriteBit (GPIOB, GPIO_Pin_15, Bit_SET);
	}
else
	{
	GPIO_WriteBit (GPIOB, GPIO_Pin_15, Bit_RESET);
	}
}


void TSSD1306SPI::BitStrobeTx (bool val)
{
SetDataTo (val);
Delay_us (1);
StrobeCLK ();
}



void TSSD1306SPI::StrobeCLK ()
{
GPIO_WriteBit (GPIOB, GPIO_Pin_13, Bit_RESET);
GPIO_WriteBit (GPIOB, GPIO_Pin_13, Bit_SET);
Delay_us (1);
GPIO_WriteBit (GPIOB, GPIO_Pin_13, Bit_RESET);
Delay_us (1);
}




void TSSD1306SPI::DCSetTo (bool val)
{
if (val)
	{
	GPIO_WriteBit (GPIOB, GPIO_Pin_14, Bit_SET);
	}
else
	{
	GPIO_WriteBit (GPIOB, GPIO_Pin_14, Bit_RESET);
	}
Delay_us (2);
}







void TSSD1306SPI::SpiModeInit ()
{

	GPIO_InitTypeDef GPIO_InitStructure;
	//SPI_InitTypeDef   SPI_InitStructure_rf;
	//RCC_APB1PeriphClockCmd (RCC_APB1Periph_SPI2, ENABLE);
  RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOB, ENABLE);
	//SPI_I2S_DeInit(SPIMODULE);
	
	//GPIO_PinRemapConfig(GPIO_Remap_SPI2, ENABLE);
	
	// sck, mosi
	/*
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15  | GPIO_Pin_13;		// 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(SPICSPORT, &GPIO_InitStructure);
	*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;	// GPIO_Pin_15 | GPIO_Pin_14
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//GPIO_Mode_Out_OD;//GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	DCSetTo (1);
	GPIO_WriteBit (GPIOB, GPIO_Pin_13, Bit_RESET);
	
	/*
  SPI_InitStructure_rf.SPI_Direction = SPI_Direction_1Line_Tx;// SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure_rf.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure_rf.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure_rf.SPI_CPOL = SPI_CPOL_Low;//SPI_CPOL_Low;// SPI_CPOL_High; // SPI_CPOL_Low;
  SPI_InitStructure_rf.SPI_CPHA = SPI_CPHA_1Edge;//SPI_CPHA_1Edge;		// SPI_CPHA_2Edge
  SPI_InitStructure_rf.SPI_NSS = SPI_NSS_Soft;//SPI_NSS_Soft;
  SPI_InitStructure_rf.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;//SPI_BaudRatePrescaler_4;
  SPI_InitStructure_rf.SPI_FirstBit = SPI_FirstBit_LSB;//SPI_FirstBit_MSB;//SPI_FirstBit_MSB;//SPI_FirstBit_LSB;
  SPI_InitStructure_rf.SPI_CRCPolynomial = 7;
  SPI_Init(SPIMODULE, &SPI_InitStructure_rf);
  SPI_Cmd(SPIMODULE, ENABLE);
	*/
	
}


void TSSD1306SPI::LCD_init(void)
{
	SpiModeInit ();

  LCD_Sleep(0);
  Delay_Ms(10);
	LCD_Commmand2 (SSD1306_SETDISPLAYCLOCKDIV, 0x80);
	LCD_Commmand2 (SSD1306_SETMULTIPLEX, 0x3F);
	LCD_Commmand2 (SSD1306_SETDISPLAYOFFSET, 0x00);
  LCD_Commmand ( SSD1306_SETSTARTLINE | 0x00);
	LCD_Commmand2 (SSD1306_CHARGEPUMP, 0x14);
    // Horizontal memory mode
	LCD_Commmand2 (SSD1306_MEMORYMODE, 0x00);
  LCD_Commmand( SSD1306_SEGREMAP | 0x1);
  LCD_Commmand( SSD1306_COMSCANDEC);
	LCD_Commmand2 (SSD1306_SETCOMPINS, 0x12);
    // Max contrast
	LCD_Commmand2 (SSD1306_SETCONTRAST, 0xCF);
	LCD_Commmand2 (SSD1306_SETPRECHARGE, 0xF1);
	LCD_Commmand2 (SSD1306_SETVCOMDETECT, 0x40);

  LCD_Commmand( SSD1306_DISPLAYALLON_RESUME);
    // Non-inverted display
  LCD_Mode(0);
    // Turn display back on
  LCD_Sleep(1);
	LCD_Contrast(3);
  //LCD_Clear();

}



void TSSD1306SPI::LCD_Contrast(unsigned char set_contrast)
{
  LCD_Commmand(SSD1306_DISPLAYOFF);
  Delay_Ms(10);
  LCD_Commmand2(SSD1306_SETCONTRAST, set_contrast);
  LCD_Commmand(SSD1306_DISPLAYON);
}



void TSSD1306SPI::LCD_Mode (char mode)
{
if (mode > 1) mode = 1;
LCD_Commmand(SSD1306_NORMALDISPLAY | mode);
}



void TSSD1306SPI::LCD_Sleep(char set)
{
if (set > 1) set = 1;
LCD_Commmand ( SSD1306_DISPLAYOFF | set);
}



void TSSD1306SPI::LCD_Commmand2 (unsigned char CMDcode, unsigned char DataByte)
{
//LCD_Commmand(CMDcode);
//LCD_Commmand(DataByte);

	DCSetTo (0);
  spi_send_cmd (CMDcode);
	spi_send_data (DataByte);
	DCSetTo (1);
	
}



void TSSD1306SPI::LCD_Commmand(unsigned char ControByte)
{
	DCSetTo (0);
	//spi_send_cmd (SDS1306COMAND);
  spi_send_cmd (ControByte);
	DCSetTo (1);
}



void TSSD1306SPI::Write_Data (char *lSrc, unsigned char sizes)
{
if (sizes && lSrc)
	{
	//spi_send_cmd (SDS1306DATA);
	//spi_send_cmd (SDS1306COMAND);
	spi_send_cmd (SDS1306DATA);
	//LCD_Commmand (SDS1306DATA);
	DCSetTo (0);
  //spi_send_data (SDS1306DATA);
	while (sizes)
		{
		spi_send_data (lSrc[0]);
		lSrc++;
		sizes--;
		}
	DCSetTo (1);
	}
}



void TSSD1306SPI::Fill_Data (unsigned char Fdat, unsigned char sizes)
{
if (sizes)
	{
	DCSetTo (0);
	//spi_send (SDS1306DATA);
	while (sizes)
		{
		spi_send_data (Fdat);
		sizes--;
		}
	DCSetTo (1);
	}
}



void TSSD1306SPI::LCD_Clear (void)
{
	unsigned char Indx = 0;
	while (Indx < 4)
		{
		Set_Page (Indx);
		Fill_Data (234,SSD_WIDTH);
		Indx++;
		}
}



void TSSD1306SPI::Set_Page (unsigned char page)
{
	LCD_Commmand( 0xB0 + (page & 0x03));
	LCD_Commmand( 0);			// 0
	LCD_Commmand( 0x10);	// 0
}



void TSSD1306SPI::RefreshAllDisplay (TLCDCANVABW *lCanv)
{
unsigned char Indx = 0;
unsigned long sz;
BUFPAR bpr;
bpr.lpRam = DataBuf;
bpr.Sizes = sizeof(DataBuf);
while (Indx < 4)
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



