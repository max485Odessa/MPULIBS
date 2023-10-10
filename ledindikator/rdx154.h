#ifndef __FRDX154_H 
#define __FRDX154_H

#include "rutine.h"

#include "I2CSOFTWARE.H"
#include "LCD132x64.h"

#define LCDCMD   112
#define LCDCDATA 114





class TRDX154 : public TI2CIFACE {
	private:
		void Rotate90Data8 (unsigned char *lpInp, unsigned char *lpOut);
		void WriteCommand (unsigned char comands);
		char DataBuf[136];
		static const unsigned char RDX154_WIDTH = 136;
		static const unsigned char RDX154_HEIGHT = 64;

	public:
		TRDX154 ();
		void InitController (unsigned char contrst);
		void UpdateContrast (unsigned char curcntr);
		void Lcd_SetXY (unsigned char Xk,unsigned char Yk);
		void Write_Data (char *lSrc, unsigned char sizes);
		void Set_Page (unsigned char page);
		void RefreshAllDisplay (TLCDCANVABW *lCanv);
};



#endif


