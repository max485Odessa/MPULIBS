#include "I2CSOFTWARE.hpp"


TI2CBUS::TI2CBUS (S_I2CSOFTPIN_T *p, uint8_t w) : c_i2c_wait (w)
{
	pnbus = p;
	_pin_low_init_out_od (p->u.arr, EARRI2CPIN_ENDENUM);
}



void TI2CBUS::I2C_INIT()
{
	pnbus->u.name_pins.sck.port->BSRR = pnbus->u.name_pins.sck.pin;		// sck = 1
	pnbus->u.name_pins.sda.port->BSRR = pnbus->u.name_pins.sda.pin;		// sda = 1
	//pins[EARRI2CPIN_SCK].port->BSRR = pins[EARRI2CPIN_SCK].pin;
	//pins[EARRI2CPIN_SDA].port->BSRR = pins[EARRI2CPIN_SDA].pin;
}



void TI2CBUS::I2C_SDA_1 ()
{
	pnbus->u.name_pins.sda.port->BSRR = pnbus->u.name_pins.sda.pin;		// sda = 1
	//pins[EARRI2CPIN_SDA].port->BSRR = pins[EARRI2CPIN_SDA].pin;
}



void TI2CBUS::I2C_SDA_0 ()
{
	pnbus->u.name_pins.sda.port->BSRR = pnbus->u.name_pins.sda.pin << 16;		// sda = 0
	//pins[EARRI2CPIN_SDA].port->BSRR = pins[EARRI2CPIN_SDA].pin << 16;
}



void TI2CBUS::I2C_SCK_1 ()
{
	pnbus->u.name_pins.sck.port->BSRR = pnbus->u.name_pins.sck.pin;		// sck = 1
	//pins[EARRI2CPIN_SCK].port->BSRR = pins[EARRI2CPIN_SCK].pin;
}



void TI2CBUS::I2C_SCK_0 ()
{
	pnbus->u.name_pins.sck.port->BSRR = pnbus->u.name_pins.sck.pin << 16;		// sck = 0
	//pins[EARRI2CPIN_SCK].port->BSRR = pins[EARRI2CPIN_SCK].pin << 16;
}



void TI2CBUS::I2C_SDA_OUT ()
{
	I2C_SDA_1 ();
}



void TI2CBUS::I2C_SDA_IN ()
{
	I2C_SDA_1 ();
}


	
bool TI2CBUS::I2C_SDA_READ ()
{
	bool rv = false;
	if ((pnbus->u.name_pins.sda.port->IDR & pnbus->u.name_pins.sda.pin)) rv = true;
	//if ((pins[EARRI2CPIN_SDA].port->IDR & pins[EARRI2CPIN_SDA].pin)) rv = true;
	return rv;
}



bool TI2CBUS::I2C_SCL_READ ()
{
	bool rv = false;
	if ((pnbus->u.name_pins.sck.port->IDR & pnbus->u.name_pins.sck.pin)) rv = true;
	//if ((pins[EARRI2CPIN_SCK].port->IDR & pins[EARRI2CPIN_SCK].pin)) rv = true;
	return rv;
}


void TI2CBUS::Pause_I2C ()
{
volatile uint8_t cnt = c_i2c_wait;	// 50 3 ������� ����� �� 330 ��� ���������  ���� 10
while (cnt)
	{
	cnt--;
	}
}



void TI2CBUS::Start_I2C()
{
Pause_I2C ();
I2C_SDA_1 ();
Pause_I2C ();
I2C_SCK_1 ();
Pause_I2C ();
I2C_SDA_0 ();
Pause_I2C ();
I2C_SCK_0 ();
Pause_I2C ();
}



void TI2CBUS::Stop_I2C ()
{
I2C_SCK_0 ();
I2C_SDA_0 ();
Pause_I2C ();
I2C_SCK_1 ();
Pause_I2C ();
I2C_SDA_1 ();
Pause_I2C ();
}



void TI2CBUS::I2CPulse ()
{
Pause_I2C ();
I2C_SCK_0 ();	
Pause_I2C ();
I2C_SCK_1 ();
Pause_I2C ();
I2C_SCK_0 ();
Pause_I2C ();
}



bool TI2CBUS::DataOut_I2C (uint8_t datas)
{
uint8_t cnwh = 8;
I2C_SDA_OUT ();
bool I2C_ASK = false;
while (cnwh)
	{
	if (datas & 128)
		{
		I2C_SDA_1 ();
		}
	else
		{
		I2C_SDA_0 ();
		}
	datas=datas<<1;
	I2CPulse ();
	cnwh--;
	}
I2C_SDA_IN ();
//I2C_SDA_0 ();
Pause_I2C ();
I2C_SCK_1 ();
Pause_I2C ();
if (I2C_SDA_READ()) I2C_ASK = true;
I2C_SCK_0 ();
Pause_I2C ();
I2C_SDA_1 ();
I2C_SDA_OUT ();
Pause_I2C ();
return I2C_ASK;
}



bool TI2CBUS::DataOut_SMBUS (uint8_t datas)
{
uint8_t cnwh=8;
I2C_SDA_OUT ();
bool I2C_ASK = false;
while (cnwh)
	{
	if (datas & 128)
		{
		I2C_SDA_1 ();
		}
	else
		{
		I2C_SDA_0 ();
		}
	datas=datas<<1;
		
	Pause_I2C ();
	I2C_SCK_1 ();
	Pause_I2C ();
	
	if (!I2C_SCL_READ())
		{
		// wait
		uint32_t lasttick = SYSBIOS::GetTickCountLong ();
		while (!I2C_SCL_READ())
			{
			if ((SYSBIOS::GetTickCountLong () - lasttick) > C_SMBUS_TIMEOUT) return true; //break;
			}
		Pause_I2C ();
		}
	
	I2C_SCK_0 ();
	Pause_I2C ();
	cnwh--;
	}
I2C_SDA_IN ();
Pause_I2C ();
I2C_SCK_1 ();
Pause_I2C ();
// check sck wait state
if (!I2C_SCL_READ())
	{
	// wait
	uint32_t lasttick = SYSBIOS::GetTickCountLong ();
	while (!I2C_SCL_READ())
		{
		if ((SYSBIOS::GetTickCountLong () - lasttick) > C_SMBUS_TIMEOUT) return true;//break;
		}
	Pause_I2C ();
	}
	
if (I2C_SDA_READ()) I2C_ASK = true;
I2C_SCK_0 ();
Pause_I2C ();
I2C_SDA_1 ();
I2C_SDA_OUT ();
Pause_I2C ();
return I2C_ASK;
}



uint8_t TI2CBUS::DataIn_SMBUS (char RawHardAsk)
{
uint8_t rv = 0;
uint8_t cnwh=8;
I2C_SDA_IN ();
while (cnwh)
	{
	rv = rv << 1;
	I2C_SCK_1 ();
	Pause_I2C ();
	
	if (!I2C_SCL_READ())
		{
		// wait
		uint32_t lasttick = SYSBIOS::GetTickCountLong ();
		while (!I2C_SCL_READ())
			{
			if ((SYSBIOS::GetTickCountLong () - lasttick) > C_SMBUS_TIMEOUT) break;
			}
		Pause_I2C ();
		}
	
	if (I2C_SDA_READ()) rv = rv | 1;
	I2C_SCK_0 ();
	Pause_I2C ();
	cnwh--;
	}
I2C_SDA_OUT ();
if (RawHardAsk)
	{
	I2C_SDA_1 ();
	}
else
	{
	I2C_SDA_0 ();
	}
Pause_I2C ();
I2C_SCK_1 ();
Pause_I2C ();
	if (!I2C_SCL_READ())
		{
		// wait
		uint32_t lasttick = SYSBIOS::GetTickCountLong ();
		while (!I2C_SCL_READ())
			{
			if ((SYSBIOS::GetTickCountLong () - lasttick) > C_SMBUS_TIMEOUT) break;
			}
		Pause_I2C ();
		}
I2C_SCK_0 ();
Pause_I2C ();
I2C_SDA_IN ();
Pause_I2C ();
return rv;
}




uint8_t TI2CBUS::DataIn_I2C (char RawHardAsk)
{
uint8_t rv = 0;
uint8_t cnwh=8;
I2C_SDA_IN ();
while (cnwh)
	{
	rv = rv << 1;
	I2C_SCK_1 ();
	Pause_I2C ();
	if (I2C_SDA_READ()) rv = rv | 1;
	I2C_SCK_0 ();
	Pause_I2C ();
	cnwh--;
	}
I2C_SDA_OUT ();
if (RawHardAsk)
	{
	I2C_SDA_1 ();
	}
else
	{
	I2C_SDA_0 ();
	}
Pause_I2C ();
I2C_SCK_1 ();
Pause_I2C ();
I2C_SCK_0 ();
Pause_I2C ();
I2C_SDA_IN ();
Pause_I2C ();
return rv;
}








/*
// ��� ������ 8 ���
void TI2CIFACE::SetSlaveAdress (uint8_t dev_adr)
{
	DEVADRESS = dev_adr;
}
*/


bool TI2CBUS::WriteFrame_i2c (uint8_t DEVADRESS, uint8_t DestReg, uint8_t *lTx, unsigned short tx_size)
{
	bool rv = false;

		//DEVADRESS = DEVADRESS << 1;
		Start_I2C ();
		do {
			if (DataOut_I2C (DEVADRESS)) break;		// WRITE BIT = 0
			if (DataOut_I2C (DestReg)) break;
			if (!tx_size || !lTx)
				{
				rv = true;
				break;
				}
			while (tx_size)
				{
				if (DataOut_I2C (lTx[0])) break;
				lTx++;
				tx_size--;
				}
			if (!tx_size) rv = true;
			} while (false);

		Stop_I2C ();

	return rv;
}



bool TI2CBUS::ReadFrame_i2c (uint8_t DEVADRESS, uint8_t SrcReg, uint8_t *lDest, unsigned short rx_size)
{
bool rv = false;
	if (rx_size)
		{
		Start_I2C ();
		do {
				bool ack_level = false;
				if (DataOut_I2C (DEVADRESS)) break;		// WRITE BIT
				if (DataOut_I2C (SrcReg)) break;
				Start_I2C ();
				if (DataOut_I2C (DEVADRESS | 1)) break;;		// READ BIT
				while (rx_size)
					{
					if (rx_size == 1) ack_level = true;
					lDest[0] = DataIn_I2C (ack_level);
					lDest++;
					rx_size--;
					}
				rv = true;
				} while (false);
		Stop_I2C ();
		}
	return rv;
}



bool TI2CBUS::ReadFrame_i2c_no_adress (uint8_t DEVADRESS, uint8_t *lDest, unsigned short rx_size)
{
bool rv = false;
	if (rx_size)
		{
		Start_I2C ();
		do {
			bool ack_level = false;
			if (DataOut_I2C (DEVADRESS | 1)) break;		// READ BIT
			while (rx_size)
				{
				if (rx_size == 1) ack_level = true;
				lDest[0] = DataIn_I2C (ack_level);
				lDest++;
				rx_size--;
				}
			rv = true;
			} while (false);
		Stop_I2C ();
		}
	return rv;
}




bool TI2CBUS::ReadFrame_smbus (uint8_t DEVADRESS, uint8_t SrcReg, uint8_t *lDest, unsigned short rx_size)
{
bool rv = false;
	if (rx_size)
		{
		//DEVADRESS = DEVADRESS << 1;
		Start_I2C ();
		do {
				bool ack_level = false;
				if (DataOut_SMBUS (DEVADRESS)) break;		// WRITE BIT
				if (DataOut_SMBUS (SrcReg)) break;
				Start_I2C ();
				if (DataOut_SMBUS (DEVADRESS | 1)) break;;		// READ BIT
				while (rx_size)
					{
					//if (rx_size == 1) ack_level = true;
					lDest[0] = DataIn_SMBUS (ack_level);
					lDest++;
					rx_size--;
					}
				rv = true;
				} while (false);
		Stop_I2C ();
		}
	return rv;
}



bool TI2CBUS::WriteFrame_smbus (uint8_t DEVADRESS, uint8_t DestReg, uint8_t *lTx, unsigned short tx_size)
{
	bool rv = false;
	if (tx_size)
		{
		//DEVADRESS = DEVADRESS << 1;
		Start_I2C ();
		do {
				if (DataOut_SMBUS (DEVADRESS)) break;		// WRITE BIT
				if (DataOut_SMBUS (DestReg)) break;
				while (tx_size)
					{
					if (DataOut_SMBUS (lTx[0])) break;
					lTx++;
					tx_size--;
					}
				if (!tx_size) rv = true;
				} while (false);
		Stop_I2C ();
		}
	return rv;
}


