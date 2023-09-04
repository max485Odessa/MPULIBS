#include "BUSI2CIMPLEMENT.H"
#include "stm32l4xx_hal.h"
#include "hard_rut.h"



// i2c2 
static const S_GPIOPIN i2cport2[EARRI2CPIN_ENDENUM] = {{GPIOA, GPIO_PIN_9}, {GPIOA, GPIO_PIN_10}};
// i2c3
static const S_GPIOPIN i2cport3[EARRI2CPIN_ENDENUM] = {{GPIOC, GPIO_PIN_10}, {GPIOC, GPIO_PIN_12}};



void TSMBUS::Hard_Init ()
{
		_pin_low_init_out_od ((S_GPIOPIN*)&i2cport2[EARRI2CPIN_SCK], 1);		// _pin_low_init_out_pp
		_pin_low_init_out_od ((S_GPIOPIN*)&i2cport2[EARRI2CPIN_SDA], 1);	
		I2C_INIT ();		// обе линии переводим в 1
}



TSMBUS::TSMBUS ()
{
Hard_Init ();
}



void TSMBUS::I2C_INIT()
{
	i2cport2[EARRI2CPIN_SCK].port->BSRR = i2cport2[EARRI2CPIN_SCK].pin;
	i2cport2[EARRI2CPIN_SDA].port->BSRR = i2cport2[EARRI2CPIN_SDA].pin;
}



void TSMBUS::I2C_SDA_1 ()
{
	i2cport2[EARRI2CPIN_SDA].port->BSRR = i2cport2[EARRI2CPIN_SDA].pin;
	//PORTPIN->BSRR = SDA_PIN;
}



void TSMBUS::I2C_SDA_0 ()
{
	i2cport2[EARRI2CPIN_SDA].port->BRR = i2cport2[EARRI2CPIN_SDA].pin;
	//PORTPIN->BRR = SDA_PIN;
}



void TSMBUS::I2C_SCK_1 ()
{
i2cport2[EARRI2CPIN_SCK].port->BSRR = i2cport2[EARRI2CPIN_SCK].pin;
}



void TSMBUS::I2C_SCK_0 ()
{
	i2cport2[EARRI2CPIN_SCK].port->BRR = i2cport2[EARRI2CPIN_SCK].pin;
	//PORTPIN->BRR = SCL_PIN;
}



void TSMBUS::I2C_SDA_OUT ()
{
	I2C_SDA_1 ();
}



void TSMBUS::I2C_SDA_IN ()
{
	I2C_SDA_1 ();
}



bool TSMBUS::I2C_SDA_READ ()
{
	bool rv = false;
	if (i2cport2[EARRI2CPIN_SDA].port->IDR & i2cport2[EARRI2CPIN_SDA].pin) rv = true;			// GPIO_ReadInputDataBit(PORTPIN, SDA_PIN) == Bit_SET)
	return rv;
}



bool TSMBUS::I2C_SCL_READ ()
{
	bool rv = false;
	if (i2cport2[EARRI2CPIN_SCK].port->IDR & i2cport2[EARRI2CPIN_SCK].pin) rv = true;	
	return rv;
}



// ---------------------------- bus 3



void TBUSI2C_3::Hard_Init ()
{

		_pin_low_init_out_pp ((S_GPIOPIN*)&i2cport3[EARRI2CPIN_SCK], 1);
		_pin_low_init_out_od ((S_GPIOPIN*)&i2cport3[EARRI2CPIN_SDA], 1);

		I2C_INIT ();		// обе линии переводим в 1

}



TBUSI2C_3::TBUSI2C_3 ()
{

}



void TBUSI2C_3::I2C_INIT()
{
	i2cport3[EARRI2CPIN_SCK].port->BSRR = i2cport3[EARRI2CPIN_SCK].pin;
	i2cport3[EARRI2CPIN_SDA].port->BSRR = i2cport3[EARRI2CPIN_SDA].pin;
}



void TBUSI2C_3::I2C_SDA_1 ()
{
	i2cport3[EARRI2CPIN_SDA].port->BSRR = i2cport3[EARRI2CPIN_SDA].pin;
	//PORTPIN->BSRR = SDA_PIN;
}



void TBUSI2C_3::I2C_SDA_0 ()
{
	i2cport3[EARRI2CPIN_SDA].port->BRR = i2cport3[EARRI2CPIN_SDA].pin;
	//PORTPIN->BRR = SDA_PIN;
}



void TBUSI2C_3::I2C_SCK_1 ()
{
i2cport3[EARRI2CPIN_SCK].port->BSRR = i2cport3[EARRI2CPIN_SCK].pin;
}



void TBUSI2C_3::I2C_SCK_0 ()
{
	i2cport3[EARRI2CPIN_SCK].port->BRR = i2cport3[EARRI2CPIN_SCK].pin;
}



void TBUSI2C_3::I2C_SDA_OUT ()
{
	I2C_SDA_1 ();
}



void TBUSI2C_3::I2C_SDA_IN ()
{
	I2C_SDA_1 ();
}



bool TBUSI2C_3::I2C_SDA_READ ()
{
	bool rv = false;
	if (i2cport3[EARRI2CPIN_SDA].port->IDR & i2cport3[EARRI2CPIN_SDA].pin) rv = true;			// GPIO_ReadInputDataBit(PORTPIN, SDA_PIN) == Bit_SET)
	return rv;
}
	

