#ifndef __NRF2401__H__
#define __NRF2401__H__

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"

namespace N5110 {

void LCDWrite(unsigned char data_or_command, unsigned char data) ;
void LightSetTo (BitAction BitVal);

void Nokia5110_Init (void);
void updateDisplay (void);

#define LOW 0
#define HIGH 1

/* PCD8544-specific defines: */
#define LCD_COMMAND  0 
#define LCD_DATA     1

/* 84x48 LCD Defines: */
#define LCD_WIDTH   84 // Note: x-coordinates go wide
#define LCD_HEIGHT  48 // Note: y-coordinates go high
#define WHITE       0  // For drawing pixels. A 0 draws white.
#define BLACK       1  // A 1 draws black.

enum ENOKIA {scePin = 1, rstPin = 2, dcPin = 3, blPin = 4};

const int sdinPin = 11;  // DN(MOSI) - Serial data, pin 6 on LCD.
const int sclkPin = 13;  // SCLK - Serial clock, pin 7 on LCD.





};

#endif


