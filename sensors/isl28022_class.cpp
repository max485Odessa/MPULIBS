#include "isl28022_class.h"


TCURRENTMEAS::TCURRENTMEAS () : 
	C_PERMIT_READ (200), 			// количество свободных чтений данных до запланированной переинициализации
	C_TIMEOUT_CONV (200),			// таймаут ожидания конвертирования данных в ms 
	C_SLAVEADRESS(0x80)				// I2C код устройства	на шине
{
	SYSBIOS::ADD_TIMER_SYS (&ConvTimeLimit);	// регистрация таймера таймаута конвертирования
	Errors = 0;
}




bool TCURRENTMEAS::CheckConversionComplete ()
{
	bool rv = false;
	if (rawdata.fld.BusVoltage_raw & 2) rv = true;
	return rv;
}


bool TCURRENTMEAS::CheckOverflovValue ()
{
	bool rv = false;
	if (rawdata.fld.BusVoltage_raw & 1) rv = true;
	return rv;
}



void TCURRENTMEAS::SwapRawData ()
{
	unsigned short *lDat = (unsigned short*)&rawdata;
	unsigned char cntr = (sizeof(TISLRAWDATA)/2);
	while (cntr)
		{
		lDat[0] = SwapShort(lDat[0]); 
		lDat++;
		cntr--;
		}
}


bool TCURRENTMEAS::WriteConfig (unsigned short dat)
{
	bool rv = false;
	I2C_obj->SetSlaveAdress (C_SLAVEADRESS);
	unsigned char TxBuf[2]; 
	TxBuf[0] = dat >> 8; TxBuf[1] = dat & 0xFF;
	rv = I2C_obj->WriteFrame (ISLREG_CONFG, TxBuf, sizeof(TxBuf));
	return rv;
}


/*
bool TCURRENTMEAS::ReadRawData (TISLRAWDATA *lDst)
{
	bool rv = false;
	I2C_obj->SetSlaveAdress (C_SLAVEADRESS);
	rv = I2C_obj->ReadFrame (ISLREG_SVOLT, (unsigned char*)&rawdata, sizeof(rawdata));
	SwapRawData ();	// микросхема big endian - у нас little endian
	return rv;
}
*/




bool TCURRENTMEAS::ReadRaw_ShuntVoltage ()
{
	bool rv = false;
	I2C_obj->SetSlaveAdress (C_SLAVEADRESS);
	unsigned short rd_data;
	rv = I2C_obj->ReadFrame (ISLREG_SVOLT, (unsigned char*)&rd_data, sizeof(rd_data));
	if (rv) rawdata.fld.ShuntVoltage_raw = SwapShort (rd_data);
	return rv;
}




bool TCURRENTMEAS::ReadRaw_BusVolatage ()
{
	bool rv = false;
	I2C_obj->SetSlaveAdress (C_SLAVEADRESS);
	unsigned short rd_data;
	rv = I2C_obj->ReadFrame (ISLREG_BVOLT, (unsigned char*)&rd_data, sizeof(rd_data));
	if (rv) rawdata.fld.BusVoltage_raw = SwapShort (rd_data);
	return rv;
}



void TCURRENTMEAS::Task ()
{
	if (ReleaseTimeout) return;
	
	switch (Sw)
		{
		case EISLSW_INIT:
			{
			// RST = 0  BRNG1 = 1  BRNG0 = 1  PG1 = 0  PG0 = 1   BADC3 = 0   BADC2 = 0  BADC1 = 1     = 105 = 0x69
			// BADC0 = 0   SADC3 = 0   SADC2 = 0   SADC1 = 1   SADC0 = 1   MODE2 = 1  MODE1 = 1  MODE0 = 1   = 159   = 0x1F
			// 80 mv - range, 14 bit bus volatage, 15 bit shunt, 508 - us, Mode = (Shunt and bus, continuous)
			const unsigned short Mode = 0x691F;
				
			WriteConfig (Mode);
			PermitCount = C_PERMIT_READ;
			Sw = EISLSW_WAITCONV;
			ReleaseTimeout = 100;		// 100 ms
			ConvTimeLimit = C_TIMEOUT_CONV;
			break;
			}
		case EISLSW_WAITCONV:
			{
			if (ConvTimeLimit)		// проверка лимита ожидания
				{
				// ReadRawData (TISLRAWDATA *lDst);   ReadRaw_ShuntVoltage () && ReadRaw_BusVolatage ()
				if (ReadRaw_ShuntVoltage () && ReadRaw_BusVolatage ())
					{
					if (CheckConversionComplete ())	// проверить бит CNVR 
						{
						Parse_data.f_error_ovf = CheckOverflovValue ();
						f_new_parse_data = ParseData ();
						Sw = EISLSW_COMPLETE;
						if (PermitCount) PermitCount--;
						}
					}
				else
					{
					// ошибка шины, шаг по следующему START - будет INIT
					Errors++;
					Sw = EISLSW_COMPLETE;
					}
				}
			else
				{
				// таймаут ожидания готовности данных превышен, шаг по следующему START - будет INIT
				Errors++;
				Sw = EISLSW_COMPLETE;
				}
			break;
			}
		case EISLSW_COMPLETE:
			{
			F_complete = true;
			break;
			}
		default:
			{
			Sw = EISLSW_INIT;
			break;
			}
		}
}




void TCURRENTMEAS::Start ()
{
	if (Errors || !PermitCount) // если лимит свободного чтения исчерпан или была ошибка - переинициализация
		{
		Sw = EISLSW_INIT;
		Errors = 0;
		}
	else
		{
		Sw = EISLSW_WAITCONV;
		ConvTimeLimit = C_TIMEOUT_CONV;		// время ожидания результата
		}
	F_complete = false;
}




bool TCURRENTMEAS::ParseData ()
{
	bool rv = false;
	// напряжение на шунте
	const float C_SHNTCM_MULT = 2.441481;
	Parse_data.shunt_voltage = (short)rawdata.fld.ShuntVoltage_raw;
	Parse_data.shunt_voltage *= C_SHNTCM_MULT;		// множитель для текущего режим 80 mv, 15 бит
	Parse_data.shunt_voltage /= 1000000;
	// напряжение полное напряжение на батарее
	Parse_data.bus_voltage = (short)(rawdata.fld.BusVoltage_raw >> 2);
	Parse_data.bus_voltage = Parse_data.bus_voltage * 0.004;
	// мощность
	Parse_data.power = (short)rawdata.fld.Power_raw;
	// ток
	Parse_data.Current = (short)rawdata.fld.Current_raw;
	rv = true;
	return rv;
}



void TCURRENTMEAS::ClearFlagNewData ()
{
	f_new_parse_data = false;
}



bool TCURRENTMEAS::CheckNewData ()
{
return f_new_parse_data;
}




TSCURRENT *TCURRENTMEAS::GetCurrent ()
{
	TSCURRENT *rv = 0;
	if (f_new_parse_data) rv = &Parse_data;
	return rv;
}


