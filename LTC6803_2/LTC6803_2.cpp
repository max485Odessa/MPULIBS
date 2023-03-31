#include "ltc6803_2.h"



TBMSMEASURE::TBMSMEASURE () : C_MAXPERMITREAD(100), C_WAITCONV_TIME(135), PEC_POLY (7)
{
	Errors = 0;
	Timer_release = 0;
	SYSBIOS::ADD_TIMER_SYS (&Timer_release);
	WriteConfig ();
}



void TBMSMEASURE::Init (TSPIOBJ *bus_obj)
{
	Spi_obj = bus_obj;
}



void TBMSMEASURE::Start ()
{
	if (!PermitCount)									// проверка лимита свободного чтения
		{	
		// лимит исчерпан - переинициализация
		Timer_release = 0;		// сброс таймера ожидания
		Sw = ESWLTC_INIT;
		}
	else
		{
		// лимит еще есть - запуск преобразования
		Sw = ESWLTC_START;
		}

	f_parse_new_complete = false;
}



void TBMSMEASURE::Task ()
{
	if (Timer_release) return;
		
	switch (Sw)
		{
		case ESWLTC_INIT:
			{
			Sw = ESWLTC_READ;										// сразу на ESWLTC_READ
			PermitCount = C_MAXPERMITREAD;			// лимит свободного чтнения
			ConfigUpdate ();
			Timer_release = 200;								// 130 ms meassure mode - 200 ms release ( 200 - 130 = 70 ms)
			break;
			}
		case ESWLTC_START:
			{
			ltc6803_stcvad();										// старт преобразования
			Sw = ESWLTC_READ;
			Timer_release = C_WAITCONV_TIME;
			break;
			}
		case ESWLTC_READ:
			{
			if (ltc6803_rdcv()) f_parse_new_complete = true;
			if (PermitCount) PermitCount--;
			Sw = ESWLTC_COMPLETE;
			break;
			}
		case ESWLTC_COMPLETE:
			{
			Start ();
			break;
			}
		default:
			{
			Sw = ESWLTC_INIT;
			break;
			}
		}
}



void TBMSMEASURE::WriteConfig ()
{
    configReg[0] = 101;
    configReg[1] = 0;
    configReg[2] = 0;
    configReg[3] = 0;
    configReg[4] = 0;
    configReg[5] = 0;
		conversionStart =  SYSBIOS::GetTickCountLong();
}
		


void TBMSMEASURE::ConfigUpdate ()
{
    const uint8_t BYTES_IN_REG = 6;
    const uint8_t CMD_LEN = 9;
    uint8_t cmd[9];
    uint16_t cfg_pec;
    uint8_t cmd_index;
    cmd[0] = WRCFG;
    cmd[1] = WRCFG_PEC;
    cmd_index = 2;
    for (uint8_t current_byte = 0; current_byte < BYTES_IN_REG; current_byte++)
			{
			cmd[cmd_index++] = configReg[current_byte];
			}
    cfg_pec = pec8_calc(BYTES_IN_REG, configReg);
    cmd[cmd_index] = (uint8_t)cfg_pec;
		Spi_obj->SS_to (false);
		Spi_obj->Trx_sync ((char*)cmd, 0, CMD_LEN);
		Spi_obj->SS_to (true);
}



uint8_t TBMSMEASURE::pec8_calc(uint8_t len, uint8_t *data)
{
	uint8_t  remainder = 0x41;
	int byte;
	uint8_t bit;
	for (byte = 0; byte < len; ++byte)
		{
		 remainder ^= data[byte];
		 for (bit = 8; bit > 0; --bit)
			{
			if (remainder & 128)
				{
				remainder = (remainder << 1) ^ PEC_POLY;
				}
			else
				{
				remainder = (remainder << 1);
				}
			}
		}
	return (remainder);
}


// запуск преобразования
void TBMSMEASURE::ltc6803_stcvad(void)
{
    uint8_t txbuf[2];
    txbuf[0] = C_STCVAD;
    txbuf[1] = C_STCVAD_PEC;
		Spi_obj->SS_to (false);
		Spi_obj->Trx_sync ((char*)txbuf, 0, sizeof(txbuf));
		Spi_obj->SS_to (true);
}



bool TBMSMEASURE::ltc6803_rdcv()
{
	bool rv = false;
	uint8_t txbuf[2];
	uint8_t rx_data[19];
	// команда чтения всех напряжений
	txbuf[0] = 0x04;
	txbuf[1] = 0xDC;
	
	Spi_obj->SS_to (false);
	Spi_obj->Trx_sync ((char*)txbuf, 0, sizeof(txbuf));
	Spi_obj->Trx_sync (0, (char*)rx_data, sizeof(rx_data));
	Spi_obj->SS_to (true);

	// данные считаны - нужно пребразовать
	rv = ParseVoltData ((char*)&rx_data);

	return rv;
}



bool TBMSMEASURE::ParseVoltData (char *lram)
{
	bool rv = false;
	uint16_t temp, temp2;
	char data_counter = 0;
		// данные считаны - нужно пребразовать
	uint8_t received_pec = lram[18];
	uint8_t data_pec = pec8_calc(18, (uint8_t*)&lram[0]);
	if (received_pec == data_pec)
		{
		for (int k = 0; k < 12; k = k + 2)
			{
			temp = lram[data_counter++];
			temp2 = (uint16_t)(lram[data_counter] & 0x0F) << 8;
			bmsdata.volt_cells[k] = (float)(temp + temp2 - 512) * 1.5 / 1000.0;
			temp2 = (lram[data_counter++]) >> 4;
			temp =  (lram[data_counter++]) << 4;
			bmsdata.volt_cells[k + 1] = (float)(temp + temp2 - 512) * 1.5 / 1000.0;
			}
		rv = true;
		}
	return rv;
}



bool TBMSMEASURE::CheckNewData ()
{
	return f_parse_new_complete;
}



SBMSVOLTAGE *TBMSMEASURE::GetNewData ()
{
	SBMSVOLTAGE *rv = 0;
	if (f_parse_new_complete) 
		{
		rv = &bmsdata;
		
		}
	return rv;
}



void TBMSMEASURE::ClearNewData ()
{
	f_parse_new_complete = false;
}

