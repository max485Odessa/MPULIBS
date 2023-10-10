#ifndef __H_SSD1306_SPI_H__
#define __H_SSD1306_SPI_H__


#include "SSD1306DEF.H"
#include "LCD132x64.h"
#include "rutinel.h"



class TSSD1306SPI {
	private:
		void SpiModeInit ();
		void spi_send (uint8_t bSend);
		void spi_send_cmd (uint8_t bSend);
		void spi_send_data (uint8_t bSend);
		void BitStrobeTx (bool val);
		void DCSetTo (bool val);
		void SetDataTo (bool val);
		void StrobeCLK ();
		
	protected:
		void LCD_Commmand (unsigned char DataByte);
		void LCD_Commmand2 (unsigned char CMDcode, unsigned char DataByte);
		void LCD_Sleep (char set);
		void LCD_Mode (char set_mode);
		char DataBuf[128];
		static const unsigned char SSD_WIDTH = 128;
		static const unsigned char SSD_HEIGHT = 32;
		
	public:
		TSSD1306SPI ();
		void LCD_init (void);
		void LCD_Contrast (unsigned char set_contrast);
		void Set_Page (unsigned char page);
		void Write_Data (char *lSrc, unsigned char sizes);
		void Fill_Data (unsigned char Fdat, unsigned char sizes);
		void LCD_Clear (void);
		void RefreshAllDisplay (TLCDCANVABW *lCanv);
};



#endif


