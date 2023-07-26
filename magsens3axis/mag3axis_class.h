#ifndef _H_MAGSENSOR_3_AXIS_STM32_H_
#define _H_MAGSENSOR_3_AXIS_STM32_H_



#include "DISPATCHIFACE.H"


//const unsigned char C_QMC5833 = 0x1A;
const unsigned char C_QMC5833 = 0x0D;
const unsigned char C_QMREGADR_STATUS = 0x06;
const unsigned char C_QMREGADR_CONTROL1 = 0x09;
enum E_QMCWORKSW {E_QMCWORKSW_INIT = 0, E_QMCWORKSW_STATUS = 1, E_QMCWORKSW_READDATA = 2, E_QMCWORKSW_BUSSERROR = 3, E_QMCWORKSW_RELEASE = 4};
const unsigned short C_QMC_READ_AMOUNT = 200;
const unsigned char B_RDY = 1;	// ������� ����� RDY (����� ������)


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
		GAUSVALUES gaus;											// ����� ������� ������
		QMC5833_DATA rawdata;									// ����� ����� ������
		utimer_t RDYTimeout;									// ������-������� �������� ��������� ����������� Read Status
	
		//bool ReadData (char *lDest, unsigned char DevRegAdr, unsigned char size);		// +
		//bool WriteData (char *lSources, unsigned char DevRegAdr, unsigned char size);	// +
	
		bool DefaultInit ();									// ������������� ��������� ����������
		bool ReadRawDataFrame ();							// ������ �������� ����� ������
		bool ReadStatus (unsigned char &dat);	// �������� ������� ����������
	
		bool CalculateMagVector ();						// ����������� ����� ������ � ��������
		bool NewDataIsParsed;									// ������ ��������, ������������� � ���������������� ��� �������� �������
	
		E_QMCWORKSW DevSw;										// ������� ������ ���������
		unsigned short ReadNormalCounter;			// ������� ������������ ������ �� ������ ������ �� ����������������� ����������
		unsigned short BusErrorCount;					// ������� ����� ������ �� �����
	
		bool f_magvector_valid;								// ���� ��� ���� �������� ����� ������ � ����������
	
	public:	
		TMAG3AXIS ();
		virtual void Task ();									// ���������� ������ ���������
		virtual void Start ();								// ������ ������ ��������� (������� ����� ��� ��������� � ��������� ������� ����� ��)
	
		// ���������������� �������
		void ClearFlagNewData ();							// �������� ���� - ����� ������
		bool CheckNewData ();									// ��������� ���� - ����� ������ (����� ��������)
		GAUSVALUES *GetGauss ();	// ������� ������������������ ������
		
	
};



#endif

