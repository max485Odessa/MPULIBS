#include "I2CSOFTWARE.H"






void Lcd_WriteCommand (unsigned char comands)
{
StartI2CLcd();
DataOutI2CLcd(LCDCMD);
DataOutI2CLcd(comands);
StopI2CLcd();
}



void UpdateContrast (unsigned char curcntr)
{
StartI2CLcd();
DataOutI2CLcd(LCDCMD);
DataOutI2CLcd(129);
DataOutI2CLcd(curcntr);
StopI2CLcd();
PausaMs (7);
}



void Lcd_Init (unsigned char contrst)
{
GPIO_InitTypeDef GPIO_InitStructure;
	// GPIO clk - enable
	RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOC, ENABLE);
	// -- SCL
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	// -- SDA
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	

Lcd_WriteCommand (0xE2);		// resets
PausaMs (30);
Lcd_WriteCommand (0xEA);	// bias
UpdateContrast (contrst);
Lcd_WriteCommand (198);
Lcd_WriteCommand (0xAF);
PausaMs (30);
}














// ------------------------------------- HAL абстракция -------------------------------------------






 













void LcdPrintStringRam0(unsigned char *lpRam)
{
unsigned char dt=0;
unsigned char fls=0;
while (1)
	{
	dt=lpRam[0];
	if (!dt)
		{
		if (!fls) CANVAS_DrawCharSlow(48);
		return;
		}
	if (dt!=48) fls=1;
	if (fls) CANVAS_DrawCharSlow(dt);
	lpRam++;
	}
}



unsigned short ABS_m (short datas)
{
unsigned short rv = (unsigned short) datas;
if (rv & 0x8000)
	{
	rv = rv ^ 0xFFFF;
	rv++;
	}
return rv;
}



void LcdPrintNumberShort (short datasvv)
{
unsigned short datas;
unsigned char *lpRams;
unsigned char RNSTR[10];
if (datasvv < 0)
	{
	CANVAS_DrawCharSlow('-');
	datas = ABS_m (datasvv);
	}
else
	{
	datas = (unsigned short) datasvv;
	}
lpRams = ShortToStr (&RNSTR[0],datas);
lpRams[0] = 0;
LcdPrintStringRam0(&RNSTR[0]);
}



void LcdPrintNumberUShort (unsigned short datas)
{
unsigned char RNSTR[10];
unsigned char *lpRams = ShortToStr (&RNSTR[0],datas);
lpRams[0] = 0;
LcdPrintStringRam0(&RNSTR[0]);
}





void PrintProcentsCenter (unsigned char datas,unsigned char xkk,unsigned char ykk,unsigned char sizes)
{
unsigned char RNSTR[10];
unsigned char viswid,fistbl;
unsigned char *lpRams = ShortToStr (&RNSTR[0],datas);
lpRams[0]=0;
viswid = GetDrawStringWidth  ((char*)&RNSTR[0]);	// 4 - ?????? ?????? ????????
fistbl=(sizes-viswid)/2;
lastX = xkk;
lastY = ykk;
DrawVerticalBlankCnt (fistbl);							// ????????? ???????? ? ??????
CANVAS_PrintString ((char*)&RNSTR[0]);						// ????????? ??????
CANVAS_DrawCharSlow ('%');
DrawVerticalBlankCnt ( (xkk+sizes) - lastX);		// ????????? ???????? ? ?????
}



void  PrintStrCentRectRom (unsigned char *lpName,unsigned char xkk,unsigned char ykk,unsigned char sizes)
{
unsigned char viswid = GetDrawStringWidth  ((char*)lpName);
unsigned char fistbl=(sizes-viswid)/2;
lastX = xkk;
lastY = ykk;
DrawVerticalBlankCnt (fistbl);							// ????????? ???????? ? ??????
CANVAS_PrintString ((char*)lpName);						// ????????? ?????
DrawVerticalBlankCnt ( (xkk+sizes) - lastX );		// ????????? ???????? ? ?????
}






/*
void LcdPrintUShortNumberRect (ushort datas,uchar sizes, uchar F_Center)
{
uchar xkk = LCD_LastX;
uchar RNSTR[10];
uchar *lpRams = ShortToStr (&RNSTR[0],datas);
lpRams[0] = 0;
uchar widths = GetVisualWidthStringRam (&RNSTR[0]);
if (F_Center)
	{
	uchar fistbl=(sizes-widths)/2;
	Lcd_DrawBlank (fistbl,0);
	LcdPrintStringRam0(&RNSTR[0]);
	Lcd_DrawBlank ( (xkk+sizes) - LCD_LastX ,0);
	}
else
	{
	LcdPrintStringRam0(&RNSTR[0]);
	if (widths < sizes)
		{
		Lcd_DrawBlank (sizes - widths,0);
		}
	}
}
*/



/*
void DrawPWMOscil (uchar procnt,uchar sizes)
{
sizes --;
ushort pixproc = (sizes * 256) / 100;
uchar pix1 = (pixproc * procnt) / 256;
Lcd_WriteData (255);
uchar pixindx=0,tmpb;
while (pixindx < sizes)
	{
	if (pixindx > pix1)
		{
		tmpb = 128;
		}
	else
		{
		if (pixindx == pix1)
			{
			tmpb = 254;
			}
		else
			{
			tmpb = 1;
			}
		}
	Lcd_WriteData (tmpb);
	pixindx++;
	}
Lcd_WriteData (255);
}
*/











/*
char CheckSerial (unsigned char *lpString)
{
unsigned char rv = 0,cntdt4 = 4,tmpbb;
while (cntdt4)
	{
	tmpbb = PeekROM(&lpString[0]);
	if (tmpbb && tmpbb!=0xFF)
		{
		rv = 1;
		break;
		}
	lpString++;
	cntdt4--;
	}
return rv;
}



void PrintSerialNumberROM (unsigned char *lpString)
{
unsigned char cntdt=4;
lpString=lpString+3;
while (cntdt)
	{
	DrawHexByte (PeekROM(&lpString[0]));
	lpString--;
	cntdt--;
	}
}
*/



void CopyRAMstrToDisplay (unsigned char *lpRam,unsigned char counts)
{
StartI2CLcd();
DataOutI2CLcd(LCDCDATA);
while (counts)
	{
	DataOutI2CLcd(lpRam[0]);
	lpRam++;
	counts--;
	}
StopI2CLcd();
}



void Lcd_SetXY1 (unsigned char Xk,unsigned char Yk)
{
Lcd_WriteCommand ((Yk & 0x0F) | 0xB0); // Set page address
Lcd_WriteCommand (Xk & 0x0F);	// Set column address LSB
Lcd_WriteCommand (((Xk >> 4) & 15) | 16); // Set column address MSB
}



// переносит из буфера картинки на дисплей байт строку по установленным координатам
// atx - координата X пиксельная
// aty - координата Y строки (0-7) табуляционная
void CopyToDispDataStr (unsigned char atx, unsigned char aty, unsigned char cnterb)
{
unsigned char *lpCurRam = Canvas_VideoBufer + (((unsigned short)aty)*Const_PrinterPixelWidth + atx);
Lcd_SetXY1 (atx,aty);
CopyRAMstrToDisplay (lpCurRam,cnterb);
}





void RefreshAllDisplay ()
{
unsigned char cnty = 0;
while (cnty < 8)
	{
	CopyToDispDataStr (0,cnty,Const_PrinterPixelWidth);
	cnty ++;
	}
}



void Rotate90Data8 (unsigned char *lpInp, unsigned char *lpOut)
{
unsigned char cntout = 8, maski = 128,masko,datout;
unsigned short cntallindx;
unsigned char *lpTmpInp;
while (cntout)
	{
	cntallindx = 0;
	datout = 0;
	masko = 1;
	lpTmpInp = lpInp;
	while (cntallindx < 8)
		{
		if (lpTmpInp[0] & maski) datout = datout | masko;
		lpTmpInp = lpTmpInp + CntByteLine;
		masko = masko << 1;
		cntallindx++;
		}
	lpOut[0] = datout;
	lpOut++;
	cntout--;
	maski = maski >> 1;
	}
}



void CanvaLine8ToLCD (unsigned char atx, unsigned char aty, unsigned char cnterb)
{
unsigned char tmpcnt;
unsigned char OutDn8[8];
unsigned char *lpOutDn8;
unsigned char *lpInpDn8 = Canvas_VideoBufer + (((unsigned short)aty)*CntByteLine*8 + atx);
Lcd_SetXY1 (atx,aty);
StartI2CLcd();
DataOutI2CLcd(LCDCDATA);
while (cnterb)
	{
	Rotate90Data8 (lpInpDn8,OutDn8);
	tmpcnt  = 8;
	lpOutDn8 = OutDn8;
	while (tmpcnt && cnterb)
		{
		DataOutI2CLcd(lpOutDn8[0]);
		lpOutDn8++;
		tmpcnt--;
		cnterb--;
		}
	lpInpDn8++;
	}
StopI2CLcd();
}

