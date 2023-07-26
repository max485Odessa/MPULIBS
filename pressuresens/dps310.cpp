#include "dps310.h"
#include "rutine.h"

const long TDPS310SENS::scaling_facts[] = {524288, 1572864, 3670016, 7864320, 253952, 516096, 1040384, 2088960};


TDPS310SENS::TDPS310SENS ()
{
	I2C_obj = 0;
	DevSw = E_DPS310WORKSW_WAITREADY;			// начало работы со сброса устройства
	BusErrorCount = 0;
	NewDataIsParsed = false;
	f_temperature_valid = false;
	f_pressure_valid = false;
	RDYTimeout = 0;
	SYSBIOS::ADD_TIMER_SYS (&RDYTimeout);
	curset_tempSrc = 0;
	curset_tempRate = 0;
	curset_tempPRC = 0;
	curset_prsRate = 0;
	curset_prsPRC = 0;
	curset_cfg = 0;
}




void TDPS310SENS::getTwosComplement (long *raw, unsigned char length)
{
	if (*raw & ((uint32_t)1 << (length - 1)))
	{
		*raw -= (uint32_t)1 << length;
	}
}



void TDPS310SENS::getRawResult(long *dst_raw, void *mem)
{
	unsigned char *buf = (unsigned char*)mem;
	*dst_raw = (uint32_t)buf[0] << 16 | (uint32_t)buf[1] << 8 | (uint32_t)buf[2];
	getTwosComplement (dst_raw, 24);
}



float TDPS310SENS::calcTemp (long raw)
{
	float temp = raw;

	//scale temperature according to scaling table and oversampling
	temp /= scaling_facts[m_tempOsr];

	//update last measured temperature
	//it will be used for pressure compensation
	m_lastTempScal = temp;

	//Calculate compensated temperature
	temp = m_c0Half + m_c1 * temp;

	return temp;
}



float TDPS310SENS::calcPressure (long raw)
{
	float prs = raw;

	//scale pressure according to scaling table and oversampling
	prs /= scaling_facts[m_prsOsr];

	//Calculate compensated pressure
	prs = m_c00 + prs * (m_c10 + prs * (m_c20 + prs * m_c30)) + m_lastTempScal * (m_c01 + prs * (m_c11 + prs * m_c21));


	return prs;
}



unsigned char TDPS310SENS::GetReg_TMP_CFG ()
{
	unsigned char rv = curset_tempSrc;	// бит выбора температурного датчика
	rv = rv | (curset_tempRate << 4) | curset_tempPRC;
	return rv;
}



unsigned char TDPS310SENS::GetReg_PRS_CFG ()
{
	unsigned char rv = curset_prsRate << 4;
	rv |= curset_prsPRC;
	return rv;
}



unsigned char TDPS310SENS::GetReg_GFG_REG ()
{
	return curset_cfg;
}



void TDPS310SENS::configTemp (unsigned char tempMr, unsigned char tempOsr)
{
	curset_tempRate = tempMr & 7;
	curset_tempPRC = tempOsr & 7;
	if (tempOsr > DPS310__OSR_SE)
		{
		curset_cfg = curset_cfg | 8;
		}
	else
		{
		curset_cfg = curset_cfg & (0xFF - 8);
		}
}



void TDPS310SENS::configPressure (unsigned char prsMr, unsigned char prsOsr)
{
	curset_prsRate = prsMr & 7;
	curset_prsPRC = prsOsr & 7;
	if (prsOsr > DPS310__OSR_SE)
		{
		curset_cfg = curset_cfg | 4;
		}
	else
		{
		curset_cfg = curset_cfg & (0xFF - 4);
		}
}



void TDPS310SENS::Task ()
{
	if (ReleaseTimeout) return;
	
	switch (DevSw)
		{
		case E_DPS310WORKSW_RESET:
			{
			if (ResetCmd ()) 								// команда reset
				{				
				ReleaseTimeout = 50;					// 50 ms  (готовность микросхемы по даташиту 12 мс - даем больше)
				DevSw = E_DPS310WORKSW_WAITREADY;
				ReadNormalCounter = C_DPS310_READ_AMOUNT;		// лимит разрешенных считываний до переинициализации микросхемы
				}
			else
				{
				BusErrorCount++;
				DevSw = E_DPS310WORKSW_ERROR;		// ошибка на шине - завершаем работу с линией
				}
			break;
			}
		case E_DPS310WORKSW_WAITREADY:
			{
			unsigned char st;
			if (GetReadyStatus (st))
				{
				if ((st & C_DPS_READYMASK) == C_DPS_READYMASK)
					{
					/*
					WriteByte (0x0E, 0xA5);
					WriteByte (0x0F, 0x96);
					WriteByte (0x62, 0x02);
					WriteByte (0x0E, 0x00);
					WriteByte (0x0F, 0x00);	
					*/
						
					DevSw = E_DPS310WORKSW_READCOEF;
					frm_ix = 0;		// стартовый индекс чтения заводских коэфициентов
					}
				else
					{
					BusErrorCount++;
					DevSw = E_DPS310WORKSW_ERROR;		// ошибка на шине - завершаем работу с линией
					}
				}
			else
				{
				BusErrorCount++;
				DevSw = E_DPS310WORKSW_ERROR;		// ошибка на шине - завершаем работу с линией
				}
			break;
			}
		case E_DPS310WORKSW_READCOEF:
			{
			// за каждый заход по байту
			unsigned char dat = 0;
			if (I2C_obj->ReadFrame (DPS310DEVADR, C_DPS310_COEF + frm_ix, &dat, sizeof(dat)))
				{
				coefdata[frm_ix++] = dat;
				if (frm_ix >= C_DPS310_COEF_AMOUNT) 
					{
					CalculateCoeficient ();			// считаем таблицу коэфициентов
					DevSw = E_DPS310WORKSW_RDSRC_TMP; 
					}
				}
			else
				{
				BusErrorCount++;
				DevSw = E_DPS310WORKSW_ERROR;		// ошибка на шине - завершаем работу с линией
				}
			break;
			}
		case E_DPS310WORKSW_RDSRC_TMP:
			{
			// за каждый заход по байту
			unsigned char dat = 0;
			if (I2C_obj->ReadFrame (DPS310DEVADR, C_DPS310_COEF_SRCE, &dat, sizeof(dat)))
				{
				curset_tempSrc = (dat & 0x80);
				frm_ix = 0;	// стартовый индекс конфигурационной последовательности
				DevSw = E_DPS310WORKSW_CONFIG;
				}
			else
				{
				BusErrorCount++;
				DevSw = E_DPS310WORKSW_ERROR;		// ошибка на шине - завершаем работу с линией
				}
			break;
			}
		case E_DPS310WORKSW_CONFIG:					// конфигурация микросхемы
			{
			// за каждый заход по комманде
			switch (frm_ix)
				{
				case 0:
					{
					if (Write_PRSCFG (GetReg_PRS_CFG()))
						{
						frm_ix++;
						}
					else
						{
						BusErrorCount++;
						DevSw = E_DPS310WORKSW_ERROR;		// ошибка на шине - завершаем работу с линией
						}
					break;
					}
				case 1:
					{
					if (Write_TMPCFG (GetReg_TMP_CFG()))
						{
						frm_ix++;
						}
					else
						{
						BusErrorCount++;
						DevSw = E_DPS310WORKSW_ERROR;		// ошибка на шине - завершаем работу с линией
						}
					break;
					}
				case 2:
					{
					if (Write_CFGREG (GetReg_GFG_REG()))
						{
						DevSw = E_DPS310WORKSW_START_TEMP;
						}
					else
						{
						BusErrorCount++;
						DevSw = E_DPS310WORKSW_ERROR;		// ошибка на шине - завершаем работу с линией
						}
					break;
					}
				}
			break;
			}
		case E_DPS310WORKSW_START_TEMP:
			{
			Write_MEASCONTROL (EMDPS310_TEMPERATURE);
			DevSw = E_DPS310WORKSW_WAIT_TEMP;
			RDYTimeout = 500;
			break;
			}
		case E_DPS310WORKSW_WAIT_TEMP:
			{
			if (RDYTimeout)
				{
				unsigned char st;
				if (GetReadyStatus (st))
					{
					if (st & C_STATE_TMP_RDY)
						{
						frm_ix = 0;		// стартовый индекс заполнения температуры
						DevSw = E_DPS310WORKSW_READ_TEMP;
						}
					}
				else
					{
					BusErrorCount++;
					DevSw = E_DPS310WORKSW_ERROR;		// ошибка на шине - завершаем работу с линией
					}
				}
			else
				{
				// время ожидания вышло
				BusErrorCount++;
				DevSw = E_DPS310WORKSW_ERROR;		// ошибка на шине - завершаем работу с линией
				}
			break;
			}
		case E_DPS310WORKSW_READ_TEMP:
			{
			unsigned char dat = 0;
			if (I2C_obj->ReadFrame (DPS310DEVADR, C_DPS310_TMP_B2 + frm_ix, &dat, sizeof(dat)))
				{
				temp_raw[frm_ix++] = dat;
				if (frm_ix >= 3) DevSw = E_DPS310WORKSW_START_PRESS;
				}
			else
				{
				BusErrorCount++;
				DevSw = E_DPS310WORKSW_ERROR;		// ошибка на шине - завершаем работу с линией
				}
			break;
			}
		case E_DPS310WORKSW_START_PRESS:
			{

			Write_MEASCONTROL (EMDPS310_PRESSURE);
			DevSw = E_DPS310WORKSW_WAIT_PRESS;
			RDYTimeout = 500;
			break;
			}
		case E_DPS310WORKSW_WAIT_PRESS:
			{
			if (RDYTimeout)
				{
				unsigned char st;
				if (GetReadyStatus (st))
					{
					if (st & C_STATE_PRS_RDY)
						{
						frm_ix = 0;				// стартовый индекс заполнения давления
						DevSw = E_DPS310WORKSW_READ_PRESS;
						}
					}
				else
					{
					BusErrorCount++;
					DevSw = E_DPS310WORKSW_ERROR;		// ошибка на шине - завершаем работу с линией
					}
				}
			else
				{
				BusErrorCount++;
				DevSw = E_DPS310WORKSW_ERROR;		// ошибка на шине - завершаем работу с линией
				}
			break;
			}
		case E_DPS310WORKSW_READ_PRESS:
			{
			unsigned char dat = 0;
			if (I2C_obj->ReadFrame (DPS310DEVADR, C_DPS310_PSR_B2 + frm_ix, &dat, sizeof(dat)))
				{
				presr_raw[frm_ix++] = dat;
				if (frm_ix >= 3) 
					{
					// тут высчитываем температуру и давление
					CalculateTmpPrs ();
					DevSw = E_DPS310WORKSW_RELEASE;
					if (ReadNormalCounter) ReadNormalCounter--;
					}
				}
			else
				{
				BusErrorCount++;
				DevSw = E_DPS310WORKSW_ERROR;		// ошибка на шине - завершаем работу с линией
				}
			break;
			}
		case E_DPS310WORKSW_ERROR:
			{
			if (BusErrorCount) SetBussError ();	// сообщить диспетчеру об ошибке
			DevSw = E_DPS310WORKSW_RELEASE;
			// тут break не надо
			}
		case E_DPS310WORKSW_RELEASE:
			{
			F_complete = true;
			break;
			}
		default:
			{
			DevSw = E_DPS310WORKSW_RESET;
			break;
			}
		}
}



void TDPS310SENS::CalculateTmpPrs ()
{
		getRawResult (&raw_temperature, temp_raw);		
		getRawResult (&raw_pressure, presr_raw);
		Cur_Temp = calcTemp (raw_temperature);
		Cur_Pressure = calcPressure (raw_pressure);
		f_pressure_valid = true;
		f_temperature_valid = true;
		NewDataIsParsed = true;
}




void TDPS310SENS::Config ()
{
	configTemp(DPS__MEASUREMENT_RATE_4, DPS__OVERSAMPLING_RATE_8);
	configPressure (DPS__MEASUREMENT_RATE_4, DPS__OVERSAMPLING_RATE_8);
}



void TDPS310SENS::CalculateCoeficient ()
{
	//compose coefficients from buffer content
	m_c0Half = ((uint32_t)coefdata[0] << 4) | (((uint32_t)coefdata[1] >> 4) & 0x0F);
	getTwosComplement(&m_c0Half, 12);
	//c0 is only used as c0*0.5, so c0_half is calculated immediately
	m_c0Half = m_c0Half / 2U;

	//now do the same thing for all other coefficients
	m_c1 = (((uint32_t)coefdata[1] & 0x0F) << 8) | (uint32_t)coefdata[2];
	getTwosComplement(&m_c1, 12);
	m_c00 = ((uint32_t)coefdata[3] << 12) | ((uint32_t)coefdata[4] << 4) | (((uint32_t)coefdata[5] >> 4) & 0x0F);
	getTwosComplement(&m_c00, 20);
	m_c10 = (((uint32_t)coefdata[5] & 0x0F) << 16) | ((uint32_t)coefdata[6] << 8) | (uint32_t)coefdata[7];
	getTwosComplement(&m_c10, 20);

	m_c01 = ((uint32_t)coefdata[8] << 8) | (uint32_t)coefdata[9];
	getTwosComplement(&m_c01, 16);

	m_c11 = ((uint32_t)coefdata[10] << 8) | (uint32_t)coefdata[11];
	getTwosComplement(&m_c11, 16);
	m_c20 = ((uint32_t)coefdata[12] << 8) | (uint32_t)coefdata[13];
	getTwosComplement(&m_c20, 16);
	m_c21 = ((uint32_t)coefdata[14] << 8) | (uint32_t)coefdata[15];
	getTwosComplement(&m_c21, 16);
	m_c30 = ((uint32_t)coefdata[16] << 8) | (uint32_t)coefdata[17];
	getTwosComplement(&m_c30, 16);
}



void TDPS310SENS::WriteByte (unsigned char dst_adr, unsigned char dat)
{
	unsigned char lpdata = dat;
	I2C_obj->WriteFrame (DPS310DEVADR, dst_adr, &lpdata, sizeof(lpdata));
}



bool TDPS310SENS::Write_MEASCONTROL (EMDPS310 cmd)
{
	unsigned char lpdata = cmd;
	I2C_obj->WriteFrame (DPS310DEVADR, C_DPS310_MEAS_CFG, &lpdata, sizeof(lpdata));
	return true;
}



bool TDPS310SENS::Write_PRSCFG (unsigned char dat)
{
	unsigned char lpdata = dat;
	m_prsOsr = dat & 7;
	I2C_obj->WriteFrame (DPS310DEVADR, C_DPS310_PRS_CFG, &lpdata, sizeof(lpdata));
	return true;
}



bool TDPS310SENS::Write_TMPCFG (unsigned char dat)
{
	unsigned char lpdata = dat;
	m_tempOsr = dat & 7;
	I2C_obj->WriteFrame (DPS310DEVADR, C_DPS310_TMP_CFG, &lpdata, sizeof(lpdata));
	return true;
}



bool TDPS310SENS::Write_CFGREG (unsigned char dat)
{
	unsigned char lpdata = dat;
	I2C_obj->WriteFrame (DPS310DEVADR, C_DPS310_CFG_REG, &lpdata, sizeof(lpdata));
	return true;	
}



void TDPS310SENS::Start ()
{
	if (!BusErrorCount)
		{
		if (!ReadNormalCounter)
			{
			DevSw = E_DPS310WORKSW_RESET;		// лимит свободных чтений исчерпан - переинициализация
			}
		else
			{
			// лимит еще есть - обычное чтение
			DevSw = E_DPS310WORKSW_START_TEMP;// E_DPS310WORKSW_WAIT_MEASURE;
			RDYTimeout = C_DPS310_TIMEOUT_COPYMEAS;			
			}
		}
	else
		{
		// была ошибка с устройством - переинициализация
		DevSw = E_DPS310WORKSW_RESET;
		BusErrorCount = 0;
		}
	F_complete = false;
}



bool TDPS310SENS::ResetCmd ()
{
	unsigned char lpdata = 9;
	bool rv = I2C_obj->WriteFrame (DPS310DEVADR, C_DPS310_RESET, &lpdata, sizeof(lpdata));
	return true;
}



bool TDPS310SENS::GetReadyStatus (unsigned char &st)
{
	unsigned char dat = 0;
	bool rv = I2C_obj->ReadFrame (DPS310DEVADR, C_DPS310_MEAS_CFG, &dat, sizeof(dat));
	if (rv) st = dat;
	return rv;
}



bool TDPS310SENS::ReadID (unsigned char &id_rd)
{
	unsigned char dat = 0;
	bool rv = I2C_obj->ReadFrame (DPS310DEVADR, C_DPS310_ID, &dat, sizeof(dat));
	if (rv) id_rd = dat;
	return rv;
}




// автоматически сбрасывает флаг что есть новые данные
bool TDPS310SENS::GetPressAndTemper (float *Tempr, float *Press)
{
float rv = NewDataIsParsed;
if (Tempr) *Tempr = Cur_Temp;
if (Press) *Press = Cur_Pressure;
ClearFlagNewData ();
return rv;
}



bool TDPS310SENS::GetPressure (float &DataOut)
{
	if (f_pressure_valid) DataOut = Cur_Pressure + clbr_ofs_prs;
	return f_pressure_valid;
}



bool TDPS310SENS::GetTemperature (float &DataOut)
{
	if (f_temperature_valid) DataOut = Cur_Temp + clbr_ofs_temp;
	return f_temperature_valid;
}



void TDPS310SENS::ClearFlagNewData ()
{
	NewDataIsParsed = false;
}



bool TDPS310SENS::CheckNewData ()
{
	return NewDataIsParsed;
}










