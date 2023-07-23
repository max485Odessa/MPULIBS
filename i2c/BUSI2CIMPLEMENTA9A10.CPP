#include "BUSI2CIMPLEMENT.H"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"


static TBUS2DEVIMPLEMA9A10 I2C_BusA9A10;
// для смены линии по другой разводке платы меняются только нижние три параметра (и внимание на блок тактирования RCC_PeriphClockCmd, должен соответствовать порту)
// текущий режим две линии с открытым колектором (хотя возможны вариации по энернгопотреблению, по переводу линии SCL в PUSH-PULL режим)
static const unsigned short SDA_PIN = GPIO_Pin_10;
static const unsigned short SCL_PIN = GPIO_Pin_9;
static const GPIO_TypeDef *PORTPIN = GPIOA;



void TBUS2DEVIMPLEMA9A10::Hard_Init ()
{
	if (!f_hard_init)
		{
		GPIO_InitTypeDef GPIO_InitStruct;
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		// SDA
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;
		GPIO_InitStruct.GPIO_Pin = SDA_PIN;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init((GPIO_TypeDef*)PORTPIN, &GPIO_InitStruct);
		// SCL
		GPIO_InitStruct.GPIO_Pin = SCL_PIN;
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_Init((GPIO_TypeDef*)PORTPIN, &GPIO_InitStruct);

		I2C_INIT ();		// обе линии переводим в 1
		f_hard_init = true;
		}
}



TBUS2DEVIMPLEMA9A10::TBUS2DEVIMPLEMA9A10 ()
{
	f_hard_init = false;
}



void TBUS2DEVIMPLEMA9A10::I2C_INIT()
{
	((GPIO_TypeDef*)PORTPIN)->BSRR = SDA_PIN | SCL_PIN;
}



void TBUS2DEVIMPLEMA9A10::I2C_SDA_1 ()
{
	((GPIO_TypeDef*)PORTPIN)->BSRR = SDA_PIN;
}



void TBUS2DEVIMPLEMA9A10::I2C_SDA_0 ()
{
	((GPIO_TypeDef*)PORTPIN)->BRR = SDA_PIN;
}



void TBUS2DEVIMPLEMA9A10::I2C_SCK_1 ()
{
((GPIO_TypeDef*)PORTPIN)->BSRR = SCL_PIN;
}



void TBUS2DEVIMPLEMA9A10::I2C_SCK_0 ()
{
	((GPIO_TypeDef*)PORTPIN)->BRR = SCL_PIN;
}



void TBUS2DEVIMPLEMA9A10::I2C_SDA_OUT ()
{
	I2C_SDA_1 ();
}



void TBUS2DEVIMPLEMA9A10::I2C_SDA_IN ()
{
	I2C_SDA_1 ();
}


	
bool TBUS2DEVIMPLEMA9A10::I2C_SDA_READ ()
{
	bool rv = false;
	if (GPIO_ReadInputDataBit((GPIO_TypeDef*)PORTPIN, SDA_PIN) == Bit_SET) rv = true;
	return rv;
}



TI2CIFACE *I2C_InitA9A10 ()
{
	I2C_BusA9A10.Hard_Init ();
	return &I2C_BusA9A10;
}


