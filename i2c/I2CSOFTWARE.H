#ifndef __H_I2C_SOFTWARE_H__
#define __H_I2C_SOFTWARE_H__


#include <stdint.h>


class TI2CIFACE {
	private:
		uint8_t c_wait;
		void Pause_I2C ();
		void I2CPulse ();
	
	protected:

		virtual void I2C_INIT() = 0;
		virtual void I2C_SDA_1 () = 0;
		virtual void I2C_SDA_0 () = 0;
		virtual void I2C_SCK_1 () = 0;
		virtual void I2C_SCK_0 () = 0;
		virtual void I2C_SDA_OUT () = 0;
		virtual void I2C_SDA_IN () = 0;
		virtual bool I2C_SDA_READ () = 0;		
	
	public:
		virtual void Hard_Init () = 0;
		void Start_I2C();
		void Stop_I2C ();
		bool DataOut_I2C (unsigned char datas);
		unsigned char DataIn_I2C (char RawHardAsk);

		bool WriteFrame (unsigned char DEVADRESS, unsigned char DestReg, unsigned char *lTx, unsigned short tx_size);
		bool ReadFrame (unsigned char DEVADRESS, unsigned char SrcReg, unsigned char *lRx, unsigned short rx_size);
		
};




#endif
