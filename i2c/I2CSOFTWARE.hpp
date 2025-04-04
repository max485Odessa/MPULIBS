#ifndef __H_I2C_SOFTWARE_H__
#define __H_I2C_SOFTWARE_H__

#include <stdint.h>
#include "SYSBIOS.hpp"
#include "stm32f4xx_hal.h"
#include "I2CSOFTWARE.hpp"
#include "hard_rut.hpp"

enum EARRI2CPIN {EARRI2CPIN_SCK = 0, EARRI2CPIN_SDA = 1, EARRI2CPIN_ENDENUM = 2};

typedef struct {
	union	{
		struct {
			S_GPIOPIN sck;
			S_GPIOPIN sda;
		} name_pins;
	S_GPIOPIN arr[EARRI2CPIN_ENDENUM];
	} u;
	
} S_I2CSOFTPIN_T;

#define C_SMBUS_TIMEOUT (25+1)

class TI2CBUS {		// TI2CIFACE
	private:
		const uint8_t c_i2c_wait;
		void Pause_I2C ();
		void I2CPulse ();
		const S_I2CSOFTPIN_T *pnbus;
		//const S_GPIOPIN *pins;
		void Hard_Init ();
		
	protected:
		void I2C_INIT ();
		void I2C_SDA_1 ();
		void I2C_SDA_0 ();
		void I2C_SCK_1 ();
		void I2C_SCK_0 ();
		void I2C_SDA_OUT ();
		void I2C_SDA_IN ();
		bool I2C_SDA_READ ();	
		bool I2C_SCL_READ ();	
	
	public:
		TI2CBUS ( S_I2CSOFTPIN_T *bus, uint8_t w);
		
		void Start_I2C();
		void Stop_I2C ();
		bool DataOut_I2C (uint8_t datas);
		bool DataOut_SMBUS (uint8_t datas);
		uint8_t DataIn_SMBUS (char RawHardAsk);
		uint8_t DataIn_I2C (char RawHardAsk);

		bool WriteFrame_i2c (uint8_t DEVADRESS, uint8_t DestReg, uint8_t *lTx, unsigned short tx_size);
		bool ReadFrame_i2c (uint8_t DEVADRESS, uint8_t SrcReg, uint8_t *lRx, unsigned short rx_size);
		bool ReadFrame_i2c_no_adress (uint8_t DEVADRESS, uint8_t *lDest, unsigned short rx_size);
		bool ReadFrame_smbus (uint8_t DEVADRESS, uint8_t SrcReg, uint8_t *lDest, unsigned short rx_size);
		bool WriteFrame_smbus (uint8_t DEVADRESS, uint8_t DstReg, uint8_t *l_src, unsigned short tx_size);
		
};




#endif


