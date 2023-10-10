#ifndef __H_SSD1306_H__
#define __H_SSD1306_H__


#include "I2CSOFTWARE.H"
#include "LCD132x64.h"
#include "rutinel.h"
#include "SSD1306DEF.H"


class TSSD1306 : public TI2CIFACE {
	private:
		void LCD_Commmand (unsigned char ControByte, unsigned char DataByte);
		void LCD_Sleep (char set);
		void LCD_Mode (char set_mode);
		char DataBuf[132];
		static const unsigned char SSD_WIDTH = 128;
		static const unsigned char SSD_HEIGHT = 64;
		
	public:
		void LCD_init (void);
		void LCD_Contrast (char set_contrast);
		void Set_Page (unsigned char page);
		void Write_Data (char *lSrc, unsigned char sizes);
		void Fill_Data (unsigned char Fdat, unsigned char sizes);
		void LCD_Clear (void);
		void RefreshAllDisplay (TLCDCANVABW *lCanv);
};

#endif


