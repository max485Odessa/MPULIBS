#include "rm3100drotek.hpp"


static const char *chipname = "rm3100";

TRM3100DR::TRM3100DR (uint8_t slv_adr) : TI2CTRDIFACE (slv_adr), C_MEASCYCLE (400), C_PERMIT_AMOUNT (500)		// C_SLAVEADRESS (0x46)
{
	debug_chip_name = chipname;
	RDYTimeout = 0;
	SYSBIOS::ADD_TIMER_SYS (&RDYTimeout);
	f_to_gauss = true;		// � ������ �� ���������
}



const char *TRM3100DR::GetChipName ()
{
	return chipname;
}



bool TRM3100DR::SetPeriodMeas (TI2CBUS *i2cobj, unsigned short cntm)
{
	unsigned char to_reg[6];

	to_reg[0] = cntm >> 8;
	to_reg[1] = cntm;
	to_reg[2] = to_reg[0];
	to_reg[3] = to_reg[1];
	to_reg[4] = to_reg[0];
	to_reg[5] = to_reg[1];	
	return i2cobj->WriteFrame_i2c (C_SLAVEADRESS, C_RM_CCX, to_reg, sizeof(to_reg));
}



bool TRM3100DR::SetContinuosMode (TI2CBUS *i2cobj)
{
	unsigned char TxBuf = 0x79; //CMM_ALL_AXIS_ON | DRDY_WHEN_ALL_AXIS_MEASURED | CM_START;
	return i2cobj->WriteFrame_i2c (C_SLAVEADRESS, C_RM_CCM, &TxBuf, sizeof(TxBuf));
}



bool TRM3100DR::GetDataStatus (TI2CBUS *i2cobj, bool &datstate)
{
	char data;
	bool rv = i2cobj->ReadFrame_i2c (C_SLAVEADRESS, C_RM_STATUS, (unsigned char*)&data, sizeof(data));
	if (rv)
		{
		datstate = false;
		if (data & 128) datstate = true;
		}
	return rv;
}



bool TRM3100DR::InitRM3100 (TI2CBUS *i2cobj)
{
	bool rv = false;
	rv = SetPeriodMeas (i2cobj, C_MEASCYCLE);
	rv = rv & SetContinuosMode (i2cobj);
	rv = rv & SetSampleRate (i2cobj);
	return rv;
}




bool TRM3100DR::SetSampleRate (TI2CBUS *i2cobj)
{
	unsigned char TxBuf = 0x96; // ~37 ��
	return i2cobj->WriteFrame_i2c (C_SLAVEADRESS, C_RM_TMRC, &TxBuf, sizeof(TxBuf));
}



bool TRM3100DR::ReadRawZYZ (TI2CBUS *i2cobj, TRAWRM3100 *lDst)
{
	uint8_t data[9];
	bool rv = i2cobj->ReadFrame_i2c (C_SLAVEADRESS, C_RM_MX, (unsigned char*)data, sizeof(data));
	if (rv)
		{
		uint32_t reading = 0;
		// x
		reading  = data[0] << 24;
		reading  |= (data[1] << 16);
		reading  |= (data[2] << 8);
		lDst->X = reading;
		lDst->X /= 256;
		// y
		reading  = data[3] << 24;
		reading  |= (data[4] << 16);
		reading  |= (data[5] << 8);
		lDst->Y = reading;
		lDst->Y /= 256;
		// z
		reading  = data[6] << 24;
		reading  |= (data[7] << 16);
		reading  |= (data[8] << 8);
		lDst->Z = reading;
		lDst->Z /= 256;
		}
	return rv;
}




bool TRM3100DR::GetRevision (TI2CBUS *i2cobj, unsigned char &revid)
{
	unsigned char buf;
	bool rv = i2cobj->ReadFrame_i2c (C_SLAVEADRESS, C_RM_REVID, &buf, sizeof(buf));
	if (rv) revid = buf;
	return rv;
}




void TRM3100DR::Task (TI2CBUS *i2cobj)
{
	if (!i2cobj) return;
	if (ReleaseTimeout) return;

	switch (Sw)
		{
		case ESWRM31_INIT:
			{
			if (!InitRM3100 (i2cobj))
				{
				// ������ �� ����� I2C - ��������� ��������
				Sw = ESWRM31_ERROR;
				SetBussError_i2c ();
				}
			else
				{
				ReleaseTimeout = 10;			// 50 ms wait
				PermitCount = C_PERMIT_AMOUNT;
				Sw = ESWRM31_GETREVID;		// ��� �������� ����������� ���������� ��������� ��������� RevId
				}
			break;
			}
		case ESWRM31_GETREVID:
			{
			unsigned char revid;
			if (GetRevision (i2cobj, revid))
				{
				if (revid && revid != 0xFF)
					{
					Sw = ESWRM31_READ;
					}
				else
					{
					// ������ - ��������� ��������
					SetBussError_chip ();
					Sw = ESWRM31_ERROR;
					}
				}
			else
				{
				// ������ �� ����� I2C - ��������� ��������
				SetBussError_i2c ();
				Sw = ESWRM31_ERROR;
				}
			break;
			}
		case ESWRM31_READ:
			{
			if (ReadRawZYZ (i2cobj, &raw_xyz))
				{
				f_new_parse_data = ParseData ();
				if (PermitCount) PermitCount--;
				Sw = ESWRM31_COMPLETE;
				}
			else
				{
				// ������ �� ����� I2C - ��������� ��������
				SetBussError_i2c ();
				Sw = ESWRM31_ERROR;
				}
			break;
			}
		case ESWRM31_ERROR:
			{
			Sw = ESWRM31_COMPLETE;
			// ��� break �� �����
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
if (is_soft_reset ())
	{
	Sw = ESWRM31_INIT;
	}
else
	{
	if (!PermitCount)
		{
		// ����� �������� - �����������������
		Sw = ESWRM31_INIT;
		}
	else
		{
		// ����� ���� - ����� �� ������
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
		rv *= tsl_lsb; 				// ��������� � �����
		rv *= 10000;					// ��������� � ������
		return rv;
}



float TRM3100DR::RawToTesla (long rawval)
{
		float rv = rawval;
		const unsigned long gain = 75;
		const float tsl_lsb = 0.000001;	

		rv = rv / gain; 
		rv *= tsl_lsb; 				// ��������� � �����
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
		//meass_xyz.X = RawToGauss (raw_xyz.X);
		//meass_xyz.Y = RawToGauss (raw_xyz.Y);
		meass_xyz.X = RawToGauss (raw_xyz.Y);
		meass_xyz.Y = RawToGauss (raw_xyz.X);
		meass_xyz.Z = RawToGauss (raw_xyz.Z) * (float)0.667;		// ��������� ���������
		}
	else
		{
		//meass_xyz.X = RawToTesla (raw_xyz.X);
		//meass_xyz.Y = RawToTesla (raw_xyz.Y);
		meass_xyz.X = RawToTesla (raw_xyz.Y);
		meass_xyz.Y = RawToTesla (raw_xyz.X);
		meass_xyz.Z = RawToTesla (raw_xyz.Z) * (float)0.667;		// ��������� ���������
		}
	return true;
}


