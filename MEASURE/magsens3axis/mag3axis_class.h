#ifndef _H_MAGSENSOR_3_AXIS_STM32_H_
#define _H_MAGSENSOR_3_AXIS_STM32_H_



#include "DISPATCHIFACE.H"


//const unsigned char C_QMC5833 = 0x1A;
const unsigned char C_QMC5833 = 0x0D;
const unsigned char C_QMREGADR_STATUS = 0x06;
const unsigned char C_QMREGADR_CONTROL1 = 0x09;
enum E_QMCWORKSW {E_QMCWORKSW_INIT = 0, E_QMCWORKSW_STATUS = 1, E_QMCWORKSW_READDATA = 2, E_QMCWORKSW_BUSSERROR = 3, E_QMCWORKSW_RELEASE = 4};
const unsigned short C_QMC_READ_AMOUNT = 200;
const unsigned char B_RDY = 1;	// битовая маска RDY (новые данные)


typedef struct {
	short X;
	short Y;
	short Z;
} QMC5833_DATA;



typedef struct {
	unsigned char Control_1;
	unsigned char Control_2;
	unsigned char Period;
} QMC5833_INITFRAME;



typedef struct {
	float X;
	float Y;
	float Z;
} GAUSVALUES;



class TMAG3AXIS : public TI2CTRDIFACE {
	private:
		GAUSVALUES gaus;											// буфер готовых данных
		QMC5833_DATA rawdata;									// буфер сырых данных
		utimer_t RDYTimeout;									// таймер-таймаут ожидания установки нормального Read Status
	
		//bool ReadData (char *lDest, unsigned char DevRegAdr, unsigned char size);		// +
		//bool WriteData (char *lSources, unsigned char DevRegAdr, unsigned char size);	// +
	
		bool DefaultInit ();									// инициализация регистров микросхемы
		bool ReadRawDataFrame ();							// чтение основных сырых данных
		bool ReadStatus (unsigned char &dat);	// проверка статуса готовности
	
		bool CalculateMagVector ();						// преобразует сырые данные в величины
		bool NewDataIsParsed;									// данные получены, преобразованы и забуферизированы для быстрого доступа
	
		E_QMCWORKSW DevSw;										// текущая машина состояния
		unsigned short ReadNormalCounter;			// счетчик разрешенного лимита по чтению данных до переинициализации микросхемы
		unsigned short BusErrorCount;					// счетчик общих ошибок на линии
	
		bool f_magvector_valid;								// флаг что были получены сырые данные с микросхемы
	
	public:	
		TMAG3AXIS ();
		virtual void Task ();									// выполнение машины состояния
		virtual void Start ();								// запуск машины состояния (текущим шагом она управляет и принимает решение какой он)
	
		// пользовательские функции
		void ClearFlagNewData ();							// очистить флаг - Новые Данные
		bool CheckNewData ();									// проверить флаг - Новые Данные (буфер обновлен)
		GAUSVALUES *GetGauss ();	// считать забуферизированные данные
		
	
};



#endif

