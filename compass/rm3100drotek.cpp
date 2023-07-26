#include "rm3100drotek.h"



TRM3100DR::TRM3100DR () : C_SLAVEADRESS (0x23), C_MEASCYCLE (400), C_PERMIT_AMOUNT (500)		// C_SLAVEADRESS (0x46)
{
	RDYTimeout = 0;
	SYSBIOS::ADD_TIMER_SYS (&RDYTimeout);
	f_to_gauss = true;		// в гауссы по умолчанию
}



bool TRM3100DR::SetPeriodMeas (unsigned short cntm)
{
	unsigned char to_reg[6];

	to_reg[0] = cntm >> 8;
	to_reg[1] = cntm;
	to_reg[2] = to_reg[0];
	to_reg[3] = to_reg[1];
	to_reg[4] = to_reg[0];
	to_reg[5] = to_reg[1];	
	return I2C_obj->WriteFrame (C_SLAVEADRESS, C_RM_CCX, to_reg, sizeof(to_reg));
}



bool TRM3100DR::SetContinuosMode ()
{
	unsigned char TxBuf = 0x79; //CMM_ALL_AXIS_ON | DRDY_WHEN_ALL_AXIS_MEASURED | CM_START;
	return I2C_obj->WriteFrame (C_SLAVEADRESS, C_RM_CCM, &TxBuf, sizeof(TxBuf));
}



bool TRM3100DR::GetDataStatus (bool &datstate)
{
	char data;
	bool rv = I2C_obj->ReadFrame (C_SLAVEADRESS, C_RM_STATUS, (unsigned char*)&data, sizeof(data));
	if (rv)
		{
		datstate = false;
		if (data & 128) datstate = true;
		}
	return rv;
}



bool TRM3100DR::InitRM3100 ()
{
	bool rv = false;
	//I2C_obj->SetSlaveAdress (C_SLAVEADRESS);
	rv = SetPeriodMeas (C_MEASCYCLE);
	rv = rv & SetContinuosMode ();
	rv = rv & SetSampleRate ();
	return rv;
}




bool TRM3100DR::SetSampleRate ()
{
	unsigned char TxBuf = 0x96; // ~37 гц
	return I2C_obj->WriteFrame (C_SLAVEADRESS, C_RM_TMRC, &TxBuf, sizeof(TxBuf));
}



bool TRM3100DR::ReadRawZYZ (TRAWRM3100 *lDst)
{
	char data[9];
	bool rv = I2C_obj->ReadFrame (C_SLAVEADRESS, C_RM_MX, (unsigned char*)data, sizeof(data));
	if (rv)
		{
		long reading = 0;
    reading  = *data <<8;
    reading |= *(data+1);
    reading  = reading <<8;
    reading |= *(data+2);
    reading  = reading <<8;
    lDst->X    = reading >>8;

    reading  = *(data+3) <<8;
    reading |= *(data+4);
    reading  = reading <<8;
    reading |= *(data+5);
    reading  = reading <<8;
    lDst->Y    = reading >>8;

    reading  = *(data+6) <<8;
    reading |= *(data+7);
    reading  = reading <<8;
    reading |= *(data+8);
    reading  = reading <<8;
    lDst->Z    = reading >>8;
		}
	return rv;
}



bool TRM3100DR::GetRevision (unsigned char &revid)
{
	unsigned char buf;
	bool rv = I2C_obj->ReadFrame (C_SLAVEADRESS, C_RM_REVID, &buf, sizeof(buf));
	if (rv) revid = buf;
	return rv;
}




void TRM3100DR::Task ()
{
	if (ReleaseTimeout) return;
	
	switch (Sw)
		{
		case ESWRM31_INIT:
			{
			if (!InitRM3100 ())
				{
				// ошибка на линии I2C - завершить операцию
				Sw = ESWRM31_ERROR;
				Errors++;
				}
			else
				{
				ReleaseTimeout = 10;			// 50 ms wait
				PermitCount = C_PERMIT_AMOUNT;
				Sw = ESWRM31_GETREVID;		// для проверки доступности микросхемы проверяем сигнатуру RevId
				}
			break;
			}
		case ESWRM31_GETREVID:
			{
			unsigned char revid;
			if (GetRevision (revid))
				{
				if (revid && revid != 0xFF)
					{
					Sw = ESWRM31_READ;
					}
				else
					{
					// ошибка - завершить операцию
					Errors++;
					Sw = ESWRM31_ERROR;
					}
				}
			else
				{
				// ошибка на линии I2C - завершить операцию
				Errors++;
				Sw = ESWRM31_ERROR;
				}
			break;
			}
		case ESWRM31_READ:
			{
			if (ReadRawZYZ (&raw_xyz))
				{
				f_new_parse_data = ParseData ();
				if (PermitCount) PermitCount--;
				Sw = ESWRM31_COMPLETE;
				}
			else
				{
				// ошибка на линии I2C - завершить операцию
				Errors++;
				Sw = ESWRM31_ERROR;
				}
			break;
			}
		case ESWRM31_ERROR:
			{
			if (Errors) SetBussError ();	// сообщить диспетчеру об ошибке
			Sw = ESWRM31_COMPLETE;
			// тут break не нужен
			}
		case ESWRM31_COMPLETE:
			{
			F_complete = true;
			break;
			}
		default:
			{
			Sw = ESWRM31_INIT;
			break;
			}
		}
}



void TRM3100DR::Start ()
{
if (Errors)
	{
	Errors = 0;
	Sw = ESWRM31_INIT;
	}
else
	{
	if (!PermitCount)
		{
		// лимит исчерпан - переинициализация
		Sw = ESWRM31_INIT;
		}
	else
		{
		// лимит есть - выйти на чтение
		Sw = ESWRM31_READ;
		}
	}
F_complete = false;
}


	
void TRM3100DR::ClearFlagNewData ()
{
	f_new_parse_data = false;
}



bool TRM3100DR::CheckNewData ()
{
	return f_new_parse_data;
}



GAUSVALUES *TRM3100DR::GetXYZ ()
{
	GAUSVALUES *rv = 0;
	if (f_new_parse_data) rv = &meass_xyz;
	return rv;
}



float TRM3100DR::RawToGauss (long rawval)
{
		float rv = rawval;
		const unsigned long gain = 75;
		const float tsl_lsb = 0.000001;	
	
		rv = rv / gain; 
		rv *= tsl_lsb; 				// переводим в Теслы
		rv *= 10000;					// переводим в Гауссы
		return rv;
}



float TRM3100DR::RawToTesla (long rawval)
{
		float rv = rawval;
		const unsigned long gain = 75;
		const float tsl_lsb = 0.000001;	

		rv = rv / gain; 
		rv *= tsl_lsb; 				// переводим в Теслы
		return rv;
}



void TRM3100DR::SetMeassTypeGauss (bool c_types)
{
	f_to_gauss = c_types;
}



bool TRM3100DR::ParseData ()
{
	if (f_to_gauss)
		{
		meass_xyz.X = RawToGauss (raw_xyz.X);
		meass_xyz.Y = RawToGauss (raw_xyz.Y);
		meass_xyz.Z = RawToGauss (raw_xyz.Z) * (float)0.667;		// коэфицент материала
		}
	else
		{
		meass_xyz.X = RawToTesla (raw_xyz.X);
		meass_xyz.Y = RawToTesla (raw_xyz.Y);
		meass_xyz.Z = RawToTesla (raw_xyz.Z) * (float)0.667;		// коэфицент материала
		}
	return true;
}


