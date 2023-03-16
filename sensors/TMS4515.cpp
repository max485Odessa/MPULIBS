#include "TMS4515.h"



static const char *chipname = "MS4525DO";



TMS4515::TMS4515() :C_SLAVEADRESS (0x28), C_MEASCYCLE (400), C_PERMIT_AMOUNT (500)
{
	debug_chip_name = chipname;
	offset_val_pascal = 0;
	Sw = ESW4515_INIT;
	is_enabled = false;
}




void TMS4515::Task (TI2CIFACE *I2C_obj)
{
	if (ReleaseTimeout) return;
	if (!is_enabled) 
		{
		F_complete = true;
		Sw = ESW4515_COMPLETE;
		ReleaseTimeout = 2000;
		}
		
	switch (Sw)
		{
		case ESW4515_INIT:
			{
			Sw = ESW4515_READ_A;
			PermitCount = C_PERMIT_AMOUNT;
			ReleaseTimeout = 5;
			break;
			}
		case ESW4515_READ_A:
			{
			if (I2C_obj->ReadFrame (C_SLAVEADRESS, 0, rawbuf_a, sizeof(rawbuf_a)))
				{
				Sw = ESW4515_READ_B;
				}
			else
				{
				SetBussError_i2c ();
				Sw = ESW4515_ERROR;	
				}
			ReleaseTimeout = 5;
			break;
			}
		case ESW4515_READ_B:
			{
			if (I2C_obj->ReadFrame (C_SLAVEADRESS, 0, rawbuf_b, sizeof(rawbuf_b)))
				{

				uint8_t status = (rawbuf_a[0] & 0xC0) >> 6;
				if (status == 2 || status == 3) 
					{
					SetBussError_i2c ();
					Sw = ESW4515_ERROR;	
					break;
					}
					
				int16_t dp_raw, dT_raw;
				dp_raw = (rawbuf_a[0] << 8) + rawbuf_a[1];
				dp_raw = 0x3FFF & dp_raw;
				dT_raw = (rawbuf_a[2] << 8) + rawbuf_a[3];
				dT_raw = (0xFFE0 & dT_raw) >> 5;

				int16_t dp_raw2, dT_raw2;
				dp_raw2 = (rawbuf_b[0] << 8) + rawbuf_b[1];
				dp_raw2 = 0x3FFF & dp_raw2;
				dT_raw2 = (rawbuf_b[2] << 8) + rawbuf_b[3];
				dT_raw2 = (0xFFE0 & dT_raw2) >> 5;
				
				if (dp_raw  == 0x3FFF || dp_raw  == 0 || dT_raw  == 0x7FF || dT_raw == 0 ||
						dp_raw2 == 0x3FFF || dp_raw2 == 0 || dT_raw2 == 0x7FF || dT_raw2 == 0) 
					{
					SetBussError_meas ();
					Sw = ESW4515_ERROR;	
					break;
					}
				
				if (abs16 (dp_raw - dp_raw2) > 0xFF || abs16 (dT_raw - dT_raw2) > 0xFF) 
					{
					SetBussError_meas ();
					Sw = ESW4515_ERROR;	
					break;
					}
				
				float press  = _get_pressure (dp_raw);
				float press2 = _get_pressure (dp_raw2);
				float temp  = _get_temperature (dT_raw);
				float temp2 = _get_temperature (dT_raw2);
					
				dif_pressure_pascal = ((press + press2)/2) + offset_val_pascal; 
				temperature_kelvin = (temp + temp2)/2;
				temperature_kelvin += 273.15;
				f_new_parse_data_pres_temp = true;
				Sw = ESW4515_COMPLETE;
				F_complete = true;
				}
			else
				{
				SetBussError_i2c ();
				Sw = ESW4515_ERROR;	
				}
			ReleaseTimeout = 5;
			break;
			}
		case ESW4515_ERROR:
			{
			Sw = ESW4515_COMPLETE;
			F_complete = true;
			break;
			}
		case ESW4515_COMPLETE:
			{
			break;
			}
		default:
			{
			Sw = ESW4515_INIT;
			break;
			}	
		}
}



void TMS4515::enabled (bool val)
{
	is_enabled = val;
}



void TMS4515::set_pascal_offset (long vall)
{
	offset_val_pascal = vall;
}



void TMS4515::Start ()
{
if (CheckError ())	
	{
		Sw = ESW4515_INIT;
		ClearError ();
	}
else
	{
	if (PermitCount)
		{
		Sw = ESW4515_READ_A;
		}
	else
		{
		Sw = ESW4515_INIT;	
		}
	}
F_complete = false;
}




float TMS4515::get_psi_range() const
{
	return c_value_psi_range;
}



void TMS4515::set_psi_range (float val)
{
	c_value_psi_range = val;
}



float TMS4515::_get_pressure(int16_t dp_raw) const
{
    const float P_max = get_psi_range();
    const float P_min = - P_max;
    const float PSI_to_Pa = 6894.757f;

    float diff_press_PSI  = -((dp_raw - 0.1f*16383) * (P_max-P_min)/(0.8f*16383) + P_min);
    float press  = diff_press_PSI * PSI_to_Pa;
    return press;
}





float TMS4515::_get_temperature(int16_t dT_raw) const
{
    float temp  = ((200.0f * dT_raw) / 2047) - 50;
    return temp;
}



bool TMS4515::get_pressure_temp (float &pressv, float &tempv)
{
	bool rv = f_new_parse_data_pres_temp;
	if (rv)
		{
		pressv = dif_pressure_pascal;
		tempv =	temperature_kelvin;
		f_new_parse_data_pres_temp = false;
		}
	return rv;
}


