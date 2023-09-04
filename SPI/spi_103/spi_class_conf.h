#ifndef _h_spi_class_conf_h_
#define _h_spi_class_conf_h_

#include "hard_rut.h"
enum ESPIPIN {ESPIPIN_CS = 0, ESPIPIN_MOSI = 1, ESPIPIN_SCK = 2, ESPIPIN_MISO = 3, ESPIPIN_ENDENUM = 4};

static const S_GPIOPIN spi_1_initarr[ESPIPIN_ENDENUM] = {{GPIOA, GPIO_Pin_15, true}/*cs*/, {GPIOB, GPIO_Pin_5, true}/*mosi*/, {GPIOB, GPIO_Pin_3, true}/*sck*/, {GPIOB, GPIO_Pin_4, true}/*miso*/};
static const S_GPIOPIN spi_2_initarr[ESPIPIN_ENDENUM] = {{GPIOB, GPIO_Pin_12, true}/*cs*/, {GPIOB, GPIO_Pin_15, true}/*mosi*/, {GPIOB, GPIO_Pin_13, true}/*sck*/, {GPIOB, GPIO_Pin_14, true}/*miso*/};

	

#endif


