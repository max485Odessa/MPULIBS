#include "SHT40AD1BSensor.hpp"



static const char *chipname = "sht40";



SHT40AD1BSensor::SHT40AD1BSensor(uint8_t slv_adr, uint32_t prs_ms) : TI2CTRDIFACE (slv_adr), c_period_read_ms (prs_ms)
{
  //address = SHT40AD1B_I2C_ADDRESS;
	relax_tim.set (0);
	f_read_is_ok = false;
}



void SHT40AD1BSensor::Start ()
{
	if (is_soft_reset ())
		{
		sw_state = ETEMPSW_INIT;
		}
	else
		{
		if (!PermitCount)
			{
			sw_state = ETEMPSW_INIT;
			}
		else
			{
			sw_state = ETEMPSW_READ;
			}
		}
	F_complete = false;
}



const char *SHT40AD1BSensor::GetChipName ()
{
	return chipname;
}




bool SHT40AD1BSensor::GetHumidity(float &hum_value)
{
  bool rv = f_read_is_ok;
  if (rv) hum_value = data_result.u.s.hum;
  return rv;
}


bool SHT40AD1BSensor::GetTemperature(float &temp_value)
{
	  bool rv = f_read_is_ok;
	  if (rv) temp_value = data_result.u.s.temp;
	  return rv;
}



uint8_t SHT40AD1BSensor::crc_calculate(const uint8_t *data, uint16_t count)
{
  const uint8_t crc8_polynomial = 0x31;
  uint8_t crc = 0xFF;

  /* Calculate 8-bit checksum for given polynomial */
  for (uint16_t index = 0; index < count; index++) {
    crc ^= data[index];
    for (uint8_t crc_bit = 8U; crc_bit > 0U; crc_bit--) {
      crc = ((crc & 0x80U) != 0U) ? ((crc << 1) ^ crc8_polynomial) : (crc << 1);
    }
  }

  return crc;
}


uint8_t SHT40AD1BSensor::crc_check(const uint8_t *data, uint16_t count, uint8_t crc)
{
  return (crc_calculate(data, count) == crc) ? 0U : 1U;
}



bool SHT40AD1BSensor::init ()
{
return true;
}



SHT40AD1BSensor::ERDRESULTTYPE SHT40AD1BSensor::read (S_SHT40RESULT_TAG *dst)
{
	ERDRESULTTYPE rslt = ERDRESULTTYPE_CRC;
	uint8_t data[6] = {0};
	bool rv = i2c_bus->ReadFrame_i2c_no_adress (C_SLAVEADRESS, data, sizeof(data));
	if (rv)
		{
		do	{
			  uint16_t temp_value_raw = (data[0] * 0x100U) + data[1];
			  uint8_t temp_value_crc  = data[2];
			  uint16_t hum_value_raw  = (data[3] * 0x100U) + data[4];
			  uint8_t hum_value_crc   = data[5];

			  /* Check CRC for temperature value */
			  if (crc_check (&data[0], 2, temp_value_crc) != 0U) break;

			  /* Check CRC for humidity value */
			  if (crc_check (&data[3], 2, hum_value_crc) != 0U) break;

			  float temp_value = -45.0f + (175.0f * (float)temp_value_raw / (float)0xFFFF);
			  float hum_value  =  -6.0f + (125.0f * (float)hum_value_raw  / (float)0xFFFF);

			  hum_value = (hum_value > 100.0f) ? 100.0f
						  : (hum_value <   0.0f) ?   0.0f
						  :                        hum_value;

			  if (dst) {
				  dst->u.s.hum = hum_value;
				  dst->u.s.temp = temp_value;
			  	  }

			  rslt = ERDRESULTTYPE_OK;
			} while (false);
		}
	else
		{
		rslt = ERDRESULTTYPE_BUS;
		}
	return rslt;
}



bool SHT40AD1BSensor::req_measure ()
{
	return i2c_bus->WriteFrame_i2c(C_SLAVEADRESS, 0xFD, 0, 0);
}



void SHT40AD1BSensor::Task (TI2CBUS *i2cobj)
{
	i2c_bus = i2cobj;
	if (!timeoutdata_tim.get()) f_read_is_ok = false;
	if (relax_tim.get() || !i2c_bus) return;

	switch (sw_state)
		{
		case ETEMPSW_INIT:
			{
			f_read_is_ok = false;
			if (init ())
				{
				PermitCount = 10000;
				sw_state = ETEMPSW_REQ_MEASURE;
				}
			else
				{
				SetBussError_i2c ();
				sw_state = ETEMPSW_ERROR;
				relax_tim.set (1000);
				}
			break;
			}
		case ETEMPSW_REQ_MEASURE:
			{
			if (req_measure ())
				{
				sw_state = ETEMPSW_READ;
				relax_tim.set (10);	// 10 ms tech wait measure
				}
			else
				{
				SetBussError_i2c ();
				sw_state = ETEMPSW_ERROR;
				relax_tim.set(1000);
				}
			break;
			}
		case ETEMPSW_READ:
			{
			S_SHT40RESULT_TAG data;
			ERDRESULTTYPE rslt = SHT40AD1BSensor::read (&data);

			switch (rslt)
				{
				case  ERDRESULTTYPE_OK:
					{
					data_result = data;
					relax_tim.set(c_period_read_ms);	// температуру читаем 1 раз в сек
					timeoutdata_tim.set (c_period_read_ms * 4);
					sw_state = ETEMPSW_REQ_MEASURE;
					f_read_is_ok = true;
					break;
					}
				case ERDRESULTTYPE_CRC:
					{
					f_read_is_ok = false;
					SetBussError_i2c ();
					sw_state = ETEMPSW_ERROR;
					relax_tim.set(1000);
					break;
					}
				case ERDRESULTTYPE_BUS:
				default:
					{
					f_read_is_ok = false;
					SetBussError_meas ();
					sw_state = ETEMPSW_ERROR;
					relax_tim.set(1000);
					break;
					}
				}
			break;
			}
		case ETEMPSW_ERROR:
			// break not need !
		case ETEMPSW_COMPLETE:
			{
			F_complete = true;
			break;
			}
		default: sw_state = ETEMPSW_INIT; break;
		}
}


