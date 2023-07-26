#include "mag3axis_class.h"



TMAG3AXIS::TMAG3AXIS ()
{
	DevSw = E_QMCWORKSW_INIT;
	f_magvector_valid = false;
	NewDataIsParsed = false;
	RDYTimeout = 0;
	SYSBIOS::ADD_TIMER_SYS (&RDYTimeout);
}


/*
bool TMAG3AXIS::ReadData (char *lDest, unsigned char DevRegAdr, unsigned char size)
{
bool rv = false;
	if (size)
		{
		I2C_obj->Start_I2C ();
		do {
				bool ack_level = false;
				if (I2C_obj->DataOutI2C (C_QMC5833)) break;		// WRITE BIT
				if (I2C_obj->DataOutI2C (DevRegAdr)) break;
				I2C_obj->Start_I2C ();
				if (I2C_obj->DataOutI2C (C_QMC5833 | 1)) break;;		// READ BIT
				while (size)
					{
					if (size == 1) ack_level = true;
					lDest[0] = I2C_obj->DataInI2C (ack_level);
					lDest++;
					size--;
					}
				rv = true;
				} while (false);
		I2C_obj->Stop_I2C ();
		}
	return rv;
}



bool TMAG3AXIS::WriteData (char *lSources, unsigned char DevRegAdr, unsigned char size)
{
	bool rv = false;
	if (size)
		{
		I2C_obj->Start_I2C ();
		do {
				if (I2C_obj->DataOutI2C (C_QMC5833)) break;		// WRITE BIT
				if (I2C_obj->DataOutI2C (DevRegAdr)) break;
				while (size)
					{
					if (I2C_obj->DataOutI2C (lSources[0])) break;
					lSources++;
					size--;
					}
				if (!size) rv = true;
				} while (false);
		I2C_obj->Stop_I2C ();
		}
	return rv;
}
*/


bool TMAG3AXIS::DefaultInit ()
{
	bool rv = false;
	QMC5833_INITFRAME init_f;
	init_f.Control_1 = 0x15;				//                   00010101  mode = 1, odr = 1, rmg = 1, osr = 0(512 filter)
	init_f.Control_2 = 1;			// INT_ENB: 1 - disable, SOFT_RST = 0, ROL_PNT = 0
	init_f.Period = 1;		// рекомендуемые по мануалу
	rv = I2C_obj->WriteFrame (C_QMC5833, C_QMREGADR_CONTROL1, (unsigned char*)&init_f, sizeof(init_f));
	return rv;
}



bool TMAG3AXIS::ReadStatus (unsigned char &dat)
{
	bool rv = false;
	unsigned char recv_d;
	if (I2C_obj->ReadFrame (C_QMC5833, C_QMREGADR_STATUS, &recv_d, sizeof(recv_d)))
		{
		rv = true;
		dat = recv_d;
		}
	return rv;
}



bool TMAG3AXIS::ReadRawDataFrame ()
{
	return I2C_obj->ReadFrame (C_QMC5833, 0, (unsigned char*)&rawdata, sizeof(rawdata));
}



bool TMAG3AXIS::CalculateMagVector ()
{
	bool rv = false;
	gaus.X = rawdata.X;
	gaus.Y = rawdata.Y;
	gaus.Z = rawdata.Z;
	gaus.X *= (float)0.01;
	gaus.Y *= (float)0.01;
	gaus.Z *= (float)0.01;
	rv = true;
	return rv;
}



void TMAG3AXIS::Task ()
{
	if (ReleaseTimeout) return;
	
	switch (DevSw)
		{
		case E_QMCWORKSW_INIT:
			{
			if (!DefaultInit ()) 
				{
				BusErrorCount++;
				DevSw = E_QMCWORKSW_BUSSERROR;		// ошибка на шине - завершаем работу с шиной
				}
			else
				{
				ReleaseTimeout = 100;						// лимит на свободное чтение данных, до принудительной переинициализации
				DevSw = E_QMCWORKSW_STATUS;			// инициализация произведенна - шагаем дальше
				ReadNormalCounter = C_QMC_READ_AMOUNT;
				RDYTimeout = 100;								// 100 ms на ожидание RDY
				}
			break;
			}
		case E_QMCWORKSW_STATUS:
			{
			unsigned char stat_d;
			if (ReadStatus (stat_d))
				{
				if (stat_d & B_RDY) 
					{
					DevSw = E_QMCWORKSW_READDATA;	// RDY прочитан - все ок, шагаем дальше
					}
				else
					{
					if (!RDYTimeout) 
						{
						BusErrorCount++;
						DevSw = E_QMCWORKSW_BUSSERROR;		// если RDY не был выставлен более 100 мс - выйти
						}
					}
				}
			else
				{
				BusErrorCount++;
				DevSw = E_QMCWORKSW_BUSSERROR;		// ошибка на шине - завершаем работу с шиной
				}
			break;
			}
		case E_QMCWORKSW_READDATA:
			{
			if (!ReadRawDataFrame ()) 
				{
				BusErrorCount++;
				DevSw = E_QMCWORKSW_BUSSERROR;	// ошибка на шине - завершаем работу с шиной
				}
			else
				{
				if (ReadNormalCounter) ReadNormalCounter--;
				f_magvector_valid = CalculateMagVector ();
				if (f_magvector_valid)  NewDataIsParsed = true;					// данные готовы
				DevSw = E_QMCWORKSW_RELEASE;	// завершаем работу с шиной после считывания данных
				}
			break;
			}
		case E_QMCWORKSW_BUSSERROR:
			{
			if (BusErrorCount) SetBussError ();	// сообщить диспетчеру об ошибке
			DevSw = E_QMCWORKSW_RELEASE;
			// тут break не нужен
			}
		case E_QMCWORKSW_RELEASE:
			{
			F_complete = true;
			break;
			}
		default:
			{
			DevSw = E_QMCWORKSW_INIT;
			break;
			}
		}
}



void TMAG3AXIS::Start ()
{
	if (!BusErrorCount)
		{
		if (ReadNormalCounter)
			{
			RDYTimeout = 5;		// 5 мс на ожидание статуса
			DevSw = E_QMCWORKSW_STATUS;
			}
		else
			{
			DevSw = E_QMCWORKSW_INIT;	
			}
		}
	else
		{
		DevSw = E_QMCWORKSW_INIT;
		BusErrorCount = 0;
		}
F_complete = false;
}



void TMAG3AXIS::ClearFlagNewData ()
{
	NewDataIsParsed = false;
}



bool TMAG3AXIS::CheckNewData ()
{
	return NewDataIsParsed;
}



GAUSVALUES *TMAG3AXIS::GetGauss ()
{
	
	GAUSVALUES *rv = 0;
	if (f_magvector_valid) rv = &gaus;
	ClearFlagNewData ();
	return rv;
}



