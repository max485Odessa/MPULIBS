#ifndef __H_I2C_SOFTWARE_H__
#define __H_I2C_SOFTWARE_H__

#include <stdint.h>
#include "SYSBIOS.H"

//extern unsigned short C_WaitToOneMks;		// 100 для 72 Мгц

/*
typedef void (*PROC_VV)(void);
typedef int (*PROC_CV)(void);

typedef struct {
		PROC_VV INIT;
		PROC_VV SDA_1;
		PROC_VV SDA_0;
		PROC_VV SCK_1;
		PROC_VV SCK_0;
		PROC_VV SDA_OUT;
		PROC_VV SDA_IN;
		PROC_CV SDA_READ;		
} IFACELPCBI2C;
*/


#define C_SMBUS_TIMEOUT (25+1)

class TI2CIFACE {
	private:
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
		virtual bool I2C_SCL_READ () = 0;	
	
		
		//virtual bool SMBUS_SCK_READ () = 0;	
	
	public:
		virtual void Hard_Init () = 0;
		void Start_I2C();
		void Stop_I2C ();
		bool DataOut_I2C (unsigned char datas);
		bool DataOut_SMBUS (unsigned char datas);
		uint8_t DataIn_SMBUS (char RawHardAsk);
		unsigned char DataIn_I2C (char RawHardAsk);
		//void SetSlaveAdress (unsigned char dev_adr);
		bool WriteFrame_i2c (unsigned char DEVADRESS, unsigned char DestReg, unsigned char *lTx, unsigned short tx_size);
		bool ReadFrame_i2c (unsigned char DEVADRESS, unsigned char SrcReg, unsigned char *lRx, unsigned short rx_size);
		bool ReadFrame_smbus (unsigned char DEVADRESS, unsigned char SrcReg, unsigned char *lDest, unsigned short rx_size);
		bool WriteFrame_smbus (unsigned char DEVADRESS, unsigned char DstReg, unsigned char *l_src, unsigned short tx_size);
		
	
};



#endif


