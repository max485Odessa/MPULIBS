#include "SSD1306.H"
#include "LCD132x64.h"




extern void Delay_Ms(unsigned short timss);




void TSSD1306::LCD_init(void)
{
  LCD_Sleep(0);
  Delay_Ms(10);
  LCD_Commmand(SDS1306COMAND, SSD1306_SETDISPLAYCLOCKDIV);
  LCD_Commmand(SDS1306COMAND, 0x80);
  LCD_Commmand(SDS1306COMAND, SSD1306_SETMULTIPLEX);
  LCD_Commmand(SDS1306COMAND, 0x3F);
  LCD_Commmand(SDS1306COMAND, SSD1306_SETDISPLAYOFFSET);
  LCD_Commmand(SDS1306COMAND, 0x00);
  LCD_Commmand(SDS1306COMAND, SSD1306_SETSTARTLINE | 0x00);
    // We use internal charge pump
  LCD_Commmand(SDS1306COMAND, SSD1306_CHARGEPUMP);
  LCD_Commmand(SDS1306COMAND, 0x14);
    // Horizontal memory mode
  LCD_Commmand(SDS1306COMAND, SSD1306_MEMORYMODE);
  LCD_Commmand(SDS1306COMAND, 0x00);
  LCD_Commmand(SDS1306COMAND, SSD1306_SEGREMAP | 0x1);
  LCD_Commmand(SDS1306COMAND, SSD1306_COMSCANDEC);
  LCD_Commmand(SDS1306COMAND, SSD1306_SETCOMPINS);
  LCD_Commmand(SDS1306COMAND, 0x12);
    // Max contrast
  LCD_Commmand(SDS1306COMAND, SSD1306_SETCONTRAST);
  LCD_Commmand(SDS1306COMAND, 0xCF);
  LCD_Commmand(SDS1306COMAND, SSD1306_SETPRECHARGE);
  LCD_Commmand(SDS1306COMAND, 0xF1);
  LCD_Commmand(SDS1306COMAND, SSD1306_SETVCOMDETECT);
  LCD_Commmand(SDS1306COMAND, 0x40);
  LCD_Commmand(SDS1306COMAND, SSD1306_DISPLAYALLON_RESUME);
    // Non-inverted display
  LCD_Mode(0);
    // Turn display back on
  LCD_Sleep(1);

  LCD_Clear();
  Set_Page (0);
}



void TSSD1306::LCD_Contrast(char set_contrast)
{
  LCD_Commmand(SDS1306COMAND, SSD1306_DISPLAYOFF);
  Delay_Ms(10);
  LCD_Commmand(SDS1306COMAND, SSD1306_SETCONTRAST);
  LCD_Commmand(SDS1306COMAND, set_contrast);
  LCD_Commmand(SDS1306COMAND, SSD1306_DISPLAYON);
}



void TSSD1306::LCD_Mode (char mode)
{
if (mode > 1) mode = 1;
LCD_Commmand(SDS1306COMAND, SSD1306_NORMALDISPLAY | mode);
}



void TSSD1306::LCD_Sleep(char set)
{
if (set > 1) set = 1;
LCD_Commmand (SDS1306COMAND, SSD1306_DISPLAYOFF | set);
}



void TSSD1306::LCD_Commmand(unsigned char ControByte, unsigned char DataByte)
{
  Start_I2C();
  DataOutI2C(SSD1306_I2C_ADDRESS);
  DataOutI2C(ControByte);
  DataOutI2C(DataByte);
  Stop_I2C();
}



void TSSD1306::Write_Data (char *lSrc, unsigned char sizes)
{
if (sizes && lSrc)
	{
  Start_I2C ();
  DataOutI2C (SSD1306_I2C_ADDRESS);
  DataOutI2C (SDS1306DATA);
	while (sizes)
		{
		DataOutI2C (lSrc[0]);
		lSrc++;
		sizes--;
		}
	Stop_I2C(); 
	}
}



void TSSD1306::Fill_Data (unsigned char Fdat, unsigned char sizes)
{
if (sizes)
	{
	Start_I2C ();
  DataOutI2C (SSD1306_I2C_ADDRESS);
  DataOutI2C (SDS1306DATA);
	while (sizes)
		{
		DataOutI2C (Fdat);
		sizes--;
		}
	Stop_I2C(); 
	}
}



void TSSD1306::LCD_Clear (void)
{
	unsigned char Indx = 0;
	while (Indx < 8)
		{
		Set_Page(Indx);
		Fill_Data (0,SSD_WIDTH);
		Indx++;
		}
}



void TSSD1306::Set_Page (unsigned char page)
{
	LCD_Commmand(SDS1306COMAND, 0xB0 + (page & 0x0F));
	LCD_Commmand(SDS1306COMAND, 0);			// 0
	LCD_Commmand(SDS1306COMAND, 0x10);	// 0
}



void TSSD1306::RefreshAllDisplay (TLCDCANVABW *lCanv)
{
unsigned char Indx = 0;
unsigned long sz;
BUFPAR bpr;
bpr.lpRam = DataBuf;
bpr.Sizes = sizeof(DataBuf);
while (Indx < 8)
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








