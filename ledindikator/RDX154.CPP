#include "RDX154.H"
#include "LCD132x64.h"



extern void Delay_us (unsigned short timss);
extern void Delay_Ms(unsigned short timss);


TRDX154::TRDX154 () : TI2CIFACE ()
{

}


void TRDX154::WriteCommand (unsigned char comands)
{
Start_I2C();
DataOutI2C(LCDCMD);
DataOutI2C(comands);
Stop_I2C();
}



void TRDX154::UpdateContrast (unsigned char curcntr)
{
Start_I2C();
DataOutI2C(LCDCMD);
DataOutI2C(129);
DataOutI2C(curcntr);
Stop_I2C();
Delay_Ms (7);
}



void TRDX154::InitController (unsigned char contrst)
{
WriteCommand (0xE2);		// resets
Delay_Ms (30);
WriteCommand (0xEA);	// bias
UpdateContrast (contrst);
WriteCommand (198);
WriteCommand (0xAF);
Delay_Ms (30);
}



void TRDX154::Lcd_SetXY (unsigned char Xk,unsigned char Yk)
{
WriteCommand ((Yk & 0x0F) | 0xB0); // Set page address
WriteCommand (Xk & 0x0F);	// Set column address LSB
WriteCommand (((Xk >> 4) & 15) | 16); // Set column address MSB
}




void TRDX154::Set_Page (unsigned char page)
{
Lcd_SetXY (0, page);
}



void TRDX154::Write_Data (char *lSrc, unsigned char sizes)
{
if (lSrc && sizes)
	{
	Start_I2C();
	DataOutI2C(LCDCDATA);
	while (sizes)
		{
		DataOutI2C(lSrc[0]);
		lSrc++;
		sizes--;
		}
	Stop_I2C ();
	}
}



void TRDX154::RefreshAllDisplay (TLCDCANVABW *lCanv)
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














