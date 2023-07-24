#include "BUSI2CIMPLEMENT.H"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "hard_rut.h"


// i2c1 
static const S_GPIOPIN i2cport1[EARRI2CPIN_ENDENUM] = {{GPIOB, GPIO_Pin_6}, {GPIOB, GPIO_Pin_7}};
// i2c2 
//static const S_GPIOPIN i2cport2[EARRI2CPIN_ENDENUM] = {{GPIOA, GPIO_Pin_9}, {GPIOA, GPIO_Pin_10}};
// i2c3
//static const S_GPIOPIN i2cport3[EARRI2CPIN_ENDENUM] = {{GPIOC, GPIO_Pin_10}, {GPIOC, GPIO_Pin_12}};



void TBUSI2C_I::Hard_Init ()
{
	_pin_low_init_out_pp ((S_GPIOPIN*)&i2c_pins[EARRI2CPIN_SCK], 1);
	_pin_low_init_out_od ((S_GPIOPIN*)&i2c_pins[EARRI2CPIN_SDA], 1);
	I2C_INIT ();
}



void TBUSI2C_I::I2C_INIT()
{
	// sda and scl to 1
	i2c_pins[EARRI2CPIN_SCK].port->BSRR = i2c_pins[EARRI2CPIN_SCK].pin;
	i2c_pins[EARRI2CPIN_SDA].port->BSRR = i2c_pins[EARRI2CPIN_SDA].pin;
}



TBUSI2C_I::TBUSI2C_I (S_GPIOPIN *pins)
{
i2c_pins = pins;
Hard_Init ();
}



void TBUSI2C_I::I2C_SDA_1 ()
{
	i2c_pins[EARRI2CPIN_SDA].port->BSRR = i2c_pins[EARRI2CPIN_SDA].pin;
	//PORTPIN->BSRR = SDA_PIN;
}



void TBUSI2C_I::I2C_SDA_0 ()
{
	i2c_pins[EARRI2CPIN_SDA].port->BRR = i2c_pins[EARRI2CPIN_SDA].pin;
	//PORTPIN->BRR = SDA_PIN;
}



void TBUSI2C_I::I2C_SCK_1 ()
{
i2c_pins[EARRI2CPIN_SCK].port->BSRR = i2c_pins[EARRI2CPIN_SCK].pin;
}



void TBUSI2C_I::I2C_SCK_0 ()
{
	i2c_pins[EARRI2CPIN_SCK].port->BRR = i2c_pins[EARRI2CPIN_SCK].pin;
	//PORTPIN->BRR = SCL_PIN;
}



void TBUSI2C_I::I2C_SDA_OUT ()
{
	I2C_SDA_1 ();
}



void TBUSI2C_I::I2C_SDA_IN ()
{
	I2C_SDA_1 ();
}



bool TBUSI2C_I::I2C_SDA_READ ()
{
	bool rv = false;
	if (i2c_pins[EARRI2CPIN_SDA].port->IDR & i2c_pins[EARRI2CPIN_SDA].pin) rv = true;			// GPIO_ReadInputDataBit(PORTPIN, SDA_PIN) == Bit_SET)
	return rv;
}
	




