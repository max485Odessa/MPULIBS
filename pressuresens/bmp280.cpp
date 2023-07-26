#include "bmp280.h"
#include "rutine.h"



TBMP280::TBMP280 ()
{
	I2C_obj = 0;
	DevSw = E_B280WORKSW_RESET;			// начало работы со сброса устройства
	BusErrorCount = 0;
	NewDataIsParsed = false;
	f_temperature_valid = false;
	f_pressure_valid = false;
	RDYTimeout = 0;
	SYSBIOS::ADD_TIMER_SYS (&RDYTimeout);
}



void TBMP280::Task ()
{
	if (ReleaseTimeout) return;
	
	switch (DevSw)
		{
		case E_B280WORKSW_RESET:
			{
			if (!ResetCmd ()) 								// команда reset
				{
				BusErrorCount++;
				DevSw = E_B280WORKSW_ERROR;		// ошибка на шине - завершаем работу с линией
				}
			else
				{
				ReleaseTimeout = 600;					// 100 ms
				DevSw = E_B280WORKSW_CBRREAD;
				ReadNormalCounter = C_BM280_READ_AMOUNT;		// лимит разрешенных считываний до переинициализации микросхемы
				}
			break;
			}
		case E_B280WORKSW_CBRREAD:
			{
			if (!ReadCalibrationTable ()) 		// читаем калибровочную таблицу
				{
				BusErrorCount++;
				DevSw = E_B280WORKSW_ERROR;		// ошибка на шине - завершаем работу с линией
				}
			else
				{
				DevSw = E_B280WORKSW_INIT;			// калибровки прочитали - шагаем дальше
				}
			break;
			}
		case E_B280WORKSW_INIT:
			{
			if (!InitRegs ()) 								// инициализируем режим работы
				{
				DevSw = E_B280WORKSW_ERROR;		// ошибка на шине - завершаем работу с линией
				BusErrorCount++;
				}
			else
				{
				DevSw = E_B280WORKSW_WAIT_MEASURE;	// все нормально - шагаем дальше
				RDYTimeout = C_B280_TIMEOUT_COPYMEAS;
				}
			break;
			}
		case E_B280WORKSW_WAIT_MEASURE:
			{
			if (RDYTimeout)
				{
				bool f_state = false;
				if (CheckCopyTableAndMeasure (f_state))
					{
					if (f_state) DevSw = E_B280WORKSW_READDATA;	// преобразование и внутреннее копирование выполнено перейти на следующий шаг
					}
				else
					{
					DevSw = E_B280WORKSW_ERROR;		// ошибка на шине - завершаем работу с линией
					BusErrorCount++;
					}
				}
			else
				{
				DevSw = E_B280WORKSW_ERROR;		// ошибка на шине - таймаут преобразовани€ превышен
				BusErrorCount++;
				}
			break;
			}
		case E_B280WORKSW_READDATA:
			{
			if (!ReadRawDataFrame ()) 
				{
				DevSw = E_B280WORKSW_ERROR;		// ошибка на шине - таймаут преобразовани€ превышен
				BusErrorCount++;
				}
			else
				{
				f_temperature_valid = CalculateTemperature ();		// перед давлением надо произвести расчет температуры
				f_pressure_valid = CalculatePressure ();
				if (f_temperature_valid && f_pressure_valid) NewDataIsParsed = true; 
				if (ReadNormalCounter) ReadNormalCounter--;
				DevSw = E_B280WORKSW_RELEASE;		// считали нормально - завершаем работу с линией
				}
			break;
			}
		case E_B280WORKSW_ERROR:
			{
			if (BusErrorCount) SetBussError ();	// сообщить диспетчеру об ошибке
			DevSw = E_B280WORKSW_RELEASE;
			// тут break не надо
			}
		case E_B280WORKSW_RELEASE:
			{
			F_complete = true;
			break;
			}
		default:
			{
			DevSw = E_B280WORKSW_RESET;
			break;
			}
		}
}



void TBMP280::Start ()
{
	if (!BusErrorCount)
		{
		if (!ReadNormalCounter)
			{
			DevSw = E_B280WORKSW_RESET;		// лимит свободных чтений исчерпан - переинициализаци€
			}
		else
			{
			// лимит еще есть - обычное чтение
			DevSw = E_B280WORKSW_WAIT_MEASURE;
			RDYTimeout = C_B280_TIMEOUT_COPYMEAS;			
			}
		}
	else
		{
		// была ошибка с устройством - переинициализаци€
		DevSw = E_B280WORKSW_RESET;
		BusErrorCount = 0;
		}
	F_complete = false;
}



bool TBMP280::InitRegs ()
{
	bool rv = false;
	do {
			if (!SaveControlMeasur (0xB7)) break;		// mode normal, 16x oversampling
			if (!SaveConfig (0x08)) break;					// filter on
			rv = true;
		} while (false);
	return rv;
}



bool TBMP280::SaveControlMeasur (unsigned char dat)
{
	bool rv = true;
	unsigned char lpdata = dat;
	I2C_obj->WriteFrame (BM280DEVADR, REG_CTRL_MEAS, &lpdata, sizeof(dat));
	return rv;
}



bool TBMP280::SaveConfig (unsigned char dat)
{
	bool rv = true;
	unsigned char lpdata = dat;
	I2C_obj->WriteFrame (BM280DEVADR, REG_CONFIG, &lpdata, sizeof(dat));
	return rv;
}



bool TBMP280::ResetCmd ()
{
	unsigned char lpdata = 0xB6;
	bool rv = I2C_obj->WriteFrame (BM280DEVADR, 0xE0, &lpdata, sizeof(lpdata));
	return rv;
}



bool TBMP280::ReadCalibrationTable ()
{
	bool rv  = I2C_obj->ReadFrame (BM280DEVADR, REG_CALIBRATION, (unsigned char*)&calibrate, sizeof(calibrate));
	return rv;
}



bool TBMP280::CheckCopyTableAndMeasure (bool &f_state)
{
unsigned char stat_raw = 0xFF;
bool rv = ReadRawStatus (stat_raw);
if (rv)
	{
	if (stat_raw & 0x09)		// mask measur & im_upd
		{
		f_state = false;
		}
	else
		{
		f_state = true;				// когда биты сброшены, то преобразование произведенно
		}
	}
return rv;
}



bool TBMP280::ReadRawStatus (unsigned char &stat_raw)
{
	unsigned char dat = 0;
	bool rv = I2C_obj->ReadFrame (BM280DEVADR, REG_STATUS, &dat, sizeof(dat));
	if (rv) stat_raw = dat;
	return rv;
}



bool TBMP280::ReadRawDataFrame ()
{
	bool rv = I2C_obj->ReadFrame (BM280DEVADR, REG_PRESS_MSB, (unsigned char*)&bmraw, sizeof(bmraw));
	return rv;
}



// автоматически сбрасывает флаг что есть новые данные
bool TBMP280::GetPressAndTemper (float *Tempr, float *Press)
{
float rv = NewDataIsParsed;
if (Tempr) *Tempr = Cur_Temp;
if (Press) *Press = Cur_Pressure;
ClearFlagNewData ();
return rv;
}



bool TBMP280::GetPressure (float &DataOut)
{
	if (f_pressure_valid) DataOut = Cur_Pressure + clbr_ofs_prs;
	return f_pressure_valid;
}



bool TBMP280::GetTemperature (float &DataOut)
{
	if (f_temperature_valid) DataOut = Cur_Temp + clbr_ofs_temp;
	return f_temperature_valid;
}



void TBMP280::ClearFlagNewData ()
{
	NewDataIsParsed = false;
}



bool TBMP280::CheckNewData ()
{
	return NewDataIsParsed;
}



unsigned long TBMP280::Get24Bit (char *lram)
{
unsigned long rv = *(unsigned char*)lram; rv <<= 8; lram++;	
rv = rv | *lram; rv <<= 8; lram++;
rv = rv | *lram;
return rv;
}



unsigned long TBMP280::GetRawTemperature ()
{
return Get24Bit (&bmraw.data[3]);
}



unsigned long TBMP280::GetRawPressure ()
{
return Get24Bit (&bmraw.data[0]);
}



bool TBMP280::CalculateTemperature ()
{
	bool rv = false;
	long var1, var2;
	long adc_T = GetRawTemperature ();
	adc_T >>= 4;

  var1 = ((((adc_T >> 3) - ((int32_t)calibrate.dig_T1 << 1))) *
          ((int32_t)calibrate.dig_T2)) >>
         11;

  var2 = (((((adc_T >> 4) - ((int32_t)calibrate.dig_T1)) *
            ((adc_T >> 4) - ((int32_t)calibrate.dig_T1))) >>
           12) *
          ((int32_t)calibrate.dig_T3)) >>
         14;
  t_fine = var1 + var2;

  float T = (t_fine * 5 + 128) >> 8;
	T = T / 100;
	
	if (T <= 100 && T >= -80) 
		{
		Cur_Temp = T;
		rv = true;
		f_temperature_valid = true;
		}
	else
		{
		f_temperature_valid = false;
		}
  return rv;
}



void TBMP280::Config ()
{
}




bool TBMP280::CalculatePressure ()
{
	bool rv = false;
  int64_t var1, var2, p;

  int32_t adc_P = GetRawPressure();
  adc_P >>= 4;

  var1 = ((int64_t)t_fine) - 128000;
  var2 = var1 * var1 * (int64_t)calibrate.dig_P6;
  var2 = var2 + ((var1 * (int64_t)calibrate.dig_P5) << 17);
  var2 = var2 + (((int64_t)calibrate.dig_P4) << 35);
  var1 = ((var1 * var1 * (int64_t)calibrate.dig_P3) >> 8) +
         ((var1 * (int64_t)calibrate.dig_P2) << 12);
  var1 =
      (((((int64_t)1) << 47) + var1)) * ((int64_t)calibrate.dig_P1) >> 33;

  if (var1 == 0) {
		f_pressure_valid = false;
    return false; // avoid exception caused by division by zero
  }
  p = 1048576 - adc_P;
  p = (((p << 31) - var2) * 3125) / var1;
  var1 = (((int64_t)calibrate.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
  var2 = (((int64_t)calibrate.dig_P8) * p) >> 19;

  p = ((p + var1 + var2) >> 8) + (((int64_t)calibrate.dig_P7) << 4);
	float pres_f = p; pres_f = pres_f / 256;
	Cur_Pressure = pres_f;
	rv = true;
	f_pressure_valid = true;
	
	return rv;
}



