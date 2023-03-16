#ifndef __H_I2C_SOFTWARE_H__
#define __H_I2C_SOFTWARE_H__


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
	
	public:
		virtual void Hard_Init () = 0;
		void Start_I2C();
		void Stop_I2C ();
		bool DataOut_I2C (unsigned char datas);
		unsigned char DataIn_I2C (char RawHardAsk);
		//void SetSlaveAdress (unsigned char dev_adr);
		bool WriteFrame (unsigned char DEVADRESS, unsigned char DestReg, unsigned char *lTx, unsigned short tx_size);
		bool ReadFrame (unsigned char DEVADRESS, unsigned char SrcReg, unsigned char *lRx, unsigned short rx_size);
		
	
};



#endif


