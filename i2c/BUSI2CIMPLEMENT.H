#ifndef _H_IMPLEMENT_I2C_H_
#define _H_IMPLEMENT_I2C_H_



#include "I2CSOFTWARE.H"
enum EARRI2CPIN {EARRI2CPIN_SCK = 0, EARRI2CPIN_SDA = 1, EARRI2CPIN_ENDENUM = 2};

class TBUSI2C_2 : public TI2CIFACE{
	private:
		//bool f_hard_init;
	
	protected:
		virtual void I2C_INIT();
		virtual void I2C_SDA_1 ();
		virtual void I2C_SDA_0 ();
		virtual void I2C_SCK_1 ();
		virtual void I2C_SCK_0 ();
		virtual void I2C_SDA_OUT ();
		virtual void I2C_SDA_IN ();
		virtual bool I2C_SDA_READ ();	
	public:
		TBUSI2C_2 ();
		virtual void Hard_Init ();
		
};



class TBUSI2C_3 : public TI2CIFACE{
	private:
		//bool f_hard_init;
	
	protected:
		virtual void I2C_INIT();
		virtual void I2C_SDA_1 ();
		virtual void I2C_SDA_0 ();
		virtual void I2C_SCK_1 ();
		virtual void I2C_SCK_0 ();
		virtual void I2C_SDA_OUT ();
		virtual void I2C_SDA_IN ();
		virtual bool I2C_SDA_READ ();	

	public:
		TBUSI2C_3 ();
		virtual void Hard_Init ();
		
};


//TI2CIFACE *I2C_Init ();		// выдает под текущюю аппаратную архитектуру общий обьект I2C шины на которой щас два устройства



#endif
