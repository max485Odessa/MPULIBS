#ifndef _h_spi_class_conf_h_
#define _h_spi_class_conf_h_

#include "hard_rut.h"
enum ESPIPIN {ESPIPIN_CS = 0, ESPIPIN_MOSI = 1, ESPIPIN_SCK = 2, ESPIPIN_MISO = 3, ESPIPIN_ENDENUM = 4};

static const S_GPIOPIN spi_1_initarr[ESPIPIN_ENDENUM] = {{GPIOA, GPIO_PIN_15}/*cs*/, {GPIOB, GPIO_PIN_5}/*mosi*/, {GPIOB, GPIO_PIN_3}/*sck*/, {GPIOB, GPIO_PIN_4}/*miso*/};
static const S_GPIOPIN spi_2_initarr[ESPIPIN_ENDENUM] = {{GPIOB, GPIO_PIN_12}/*cs*/, {GPIOB, GPIO_PIN_15}/*mosi*/, {GPIOB, GPIO_PIN_13}/*sck*/, {GPIOB, GPIO_PIN_14}/*miso*/};

	

#endif


