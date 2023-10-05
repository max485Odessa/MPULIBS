#ifndef __H_HX1230_SPI_H__
#define __H_HX1230_SPI_H__


#include "LCD132x64.h"
#include "stm32f10x_gpio.h"

extern void Delay_us (unsigned short timss);


class THX1230SPI {
	private:
		void SpiModeInit ();
		void spi_send (unsigned char bSend);
		void BitStrobeTx (bool val);
		void CSSetTo (bool val);
		void SetDataTo (bool val);
		void StrobeCLK ();
		void ResetTo (bool val);
		
	protected:
		void spi_send_cmd (unsigned char bSend);
		void spi_send_data (unsigned char bSend);
		char DataBuf[96*2];
		static const unsigned char SSD_WIDTH = 96;
		static const unsigned char SSD_HEIGHT = 72;
	
		
	public:
		THX1230SPI ();
		void LCD_init (void);
		void LCD_Contrast (unsigned char set_contrast);
		void Set_Page (unsigned char page);
		void Write_Data (char *lSrc, unsigned char sizes);
		void Fill_Data (unsigned char Fdat, unsigned char sizes);
		void LCD_Clear (void);
		void RefreshAllDisplay (TLCDCANVABW *lCanv);
		void Contrast (unsigned char ucContrast);
		void PowerDown ();
		void PowerUp ();
};



#endif


