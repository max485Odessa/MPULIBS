#ifndef __H_SSD1306_DEF_H__
#define __H_SSD1306_DEF_H__



const unsigned char SSD1306_I2C_ADDRESS	= 0x78;
// size
const unsigned char SSD1306_LCDWIDTH = 128;
const unsigned char SSD1306_LCDHEIGHT = 64;
const unsigned char SSD1306_DEFAULT_SPACE = 5;
// Commands
const unsigned char SSD1306_SETCONTRAST = 0x81;
const unsigned char SSD1306_DISPLAYALLON_RESUME = 0xA4;
const unsigned char SSD1306_DISPLAYALLON = 0xA5;
const unsigned char SSD1306_NORMALDISPLAY = 0xA6;
const unsigned char SSD1306_INVERTDISPLAY = 0xA7;
const unsigned char SSD1306_DISPLAYOFF = 0xAE;
const unsigned char SSD1306_DISPLAYON = 0xAF;
const unsigned char SSD1306_SETDISPLAYOFFSET = 0xD3;
const unsigned char SSD1306_SETCOMPINS = 0xDA;
const unsigned char SSD1306_SETVCOMDETECT = 0xDB;
const unsigned char SSD1306_SETDISPLAYCLOCKDIV = 0xD5;
const unsigned char SSD1306_SETPRECHARGE = 0xD9;
const unsigned char SSD1306_SETMULTIPLEX = 0xA8;
const unsigned char SSD1306_SETLOWCOLUMN = 0x00;
const unsigned char SSD1306_SETHIGHCOLUMN = 0x10;
const unsigned char SSD1306_SETSTARTLINE = 0x40;
const unsigned char SSD1306_MEMORYMODE = 0x20;
const unsigned char SSD1306_COLUMNADDR = 0x21;
const unsigned char SSD1306_PAGEADDR = 0x22;
const unsigned char SSD1306_COMSCANINC = 0xC0;
const unsigned char SSD1306_COMSCANDEC = 0xC8;
const unsigned char SSD1306_SEGREMAP = 0xA0;
const unsigned char SSD1306_CHARGEPUMP = 0x8D;
const unsigned char SSD1306_EXTERNALVCC = 0x1;
const unsigned char SSD1306_SWITCHCAPVCC = 0x2;
// Scrolling const unsigned chars
const unsigned char SSD1306_ACTIVATE_SCROLL = 0x2F;
const unsigned char SSD1306_DEACTIVATE_SCROLL = 0x2E;
const unsigned char SSD1306_SET_VERTICAL_SCROLL_AREA = 0xA3;
const unsigned char SSD1306_RIGHT_HORIZONTAL_SCROLL = 0x26;
const unsigned char SSD1306_LEFT_HORIZONTAL_SCROLL = 0x27;
const unsigned char SSD1306_VERT_AND_RIGHT_HORIZ_SCROLL = 0x29;
const unsigned char SSD1306_VERT_AND_LEFT_HORIZ_SCROLL = 0x2A;

const unsigned char SDS1306COMAND = 0x00;
const unsigned char SDS1306DATA = 0x40;


#endif


