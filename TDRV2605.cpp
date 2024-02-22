#include "TDRV2605.h"

#define C_SHOKINC 70

static const S_VIBRPOINT_T itmlist_prg_1[] = {
  0x40 + C_SHOKINC, 100, 0x40 + C_SHOKINC, 100, 
  0x00, 100,
  0x40 + C_SHOKINC, 50, 0x00, 100, 
  0x40 + C_SHOKINC, 50, 0x00, 100, 
  0x40 + C_SHOKINC, 50, 0x00, 100,
	0,0
};




TDRV2605::TDRV2605 (TI2CIFACE *ifc, S_GPIOPIN *p) : c_chip_adr (DRV2605_ADDR << 1)
{
	en_pin = p;
	_pin_low_init_out_pp ( en_pin, 1);
	_pin_output ( en_pin, false);

	i2c = ifc;
	
	initarr[0] = (TDRV2605::init_tile_funct)&TDRV2605::init_funct_mode;
	initarr[1] = (TDRV2605::init_tile_funct)&TDRV2605::init_funct_rtpin;
	initarr[2] = (TDRV2605::init_tile_funct)&TDRV2605::init_funct_ws_0;
	initarr[3] = (TDRV2605::init_tile_funct)&TDRV2605::init_funct_ws_1;
	initarr[4] = (TDRV2605::init_tile_funct)&TDRV2605::init_funct_overdrv;
	initarr[5] = (TDRV2605::init_tile_funct)&TDRV2605::init_funct_systain_pos;
	initarr[6] = (TDRV2605::init_tile_funct)&TDRV2605::init_funct_systain_neg;
	initarr[7] = (TDRV2605::init_tile_funct)&TDRV2605::init_funct_break;
	initarr[8] = (TDRV2605::init_tile_funct)&TDRV2605::init_funct_audio_max;
	initarr[9] = (TDRV2605::init_tile_funct)&TDRV2605::init_funct_time_pbck;
	initarr[10] = (TDRV2605::init_tile_funct)&TDRV2605::init_funct_control3;
	initarr[11] = (TDRV2605::init_tile_funct)&TDRV2605::init_funct_libdef;
	initarr[12] = (TDRV2605::init_tile_funct)&TDRV2605::init_funct_modedef;

	Effect (EERM_ENDENUM);
}




bool TDRV2605::writeRegister8 (uint8_t reg, uint8_t val)
{
	return i2c->WriteFrame_i2c (c_chip_adr, reg, &val, sizeof(val));
}



bool TDRV2605::readRegister8 (uint8_t reg, uint8_t &dat)
{
	return i2c->ReadFrame_i2c (c_chip_adr, reg, &dat, sizeof(dat));
}


bool TDRV2605::init ()
{
bool rv = false;

if (init_cmd_ix < C_DRVINIT_FUNCT_CNT)
	{
	init_tile_funct fnct = initarr[init_cmd_ix];
	bool rslt = (this->*fnct) ();
	if (rslt) init_ok_cntr++;
	init_cmd_ix++;
	if (init_cmd_ix >= C_DRVINIT_FUNCT_CNT) rv = true;
	}
else
	{
	rv = true;
	}
return rv;
}



bool TDRV2605::init_funct_mode ()
{
	return writeRegister8(DRV2605_REG_MODE, 0x00);
}


bool TDRV2605::init_funct_rtpin ()
{
	return writeRegister8(DRV2605_REG_RTPIN, 0x00);
}


bool TDRV2605::init_funct_ws_0 ()
{
	return writeRegister8(DRV2605_REG_WAVESEQ1, 1);
}


bool TDRV2605::init_funct_ws_1 ()
{
	return writeRegister8(DRV2605_REG_WAVESEQ2, 0);
}


bool TDRV2605::init_funct_overdrv ()
{
 return writeRegister8(DRV2605_REG_OVERDRIVE, 0);
}


bool TDRV2605::init_funct_systain_pos ()
{
	return writeRegister8(DRV2605_REG_SUSTAINPOS, 0);
}


bool TDRV2605::init_funct_systain_neg ()
{
	return writeRegister8(DRV2605_REG_SUSTAINNEG, 0);
}



bool TDRV2605::init_funct_break ()
{
	return writeRegister8(DRV2605_REG_BREAK, 0);
}



bool TDRV2605::init_funct_audio_max ()
{
	return writeRegister8(DRV2605_REG_AUDIOMAX, 0x64);
}



bool TDRV2605::init_funct_time_pbck ()
{
	return writeRegister8 (DRV2605_REG_FEEDBACK, 0x36);
}


bool TDRV2605::init_funct_control3 ()
{
	return writeRegister8(DRV2605_REG_CONTROL3, 0xA0);
}



bool TDRV2605::init_funct_libdef ()
{
	selectLibrary (1);
	return true;
}



bool TDRV2605::init_funct_modedef ()
{
	setMode (DRV2605_MODE_REALTIME);
	return true;
}


bool TDRV2605::setWaveform(uint8_t slot, uint8_t w)
{
	return writeRegister8 (DRV2605_REG_WAVESEQ1 + slot, w);
}



bool TDRV2605::selectLibrary(uint8_t lib)
{
	return writeRegister8(DRV2605_REG_LIBRARY, lib);
}



bool TDRV2605::go(void)
{
	return writeRegister8(DRV2605_REG_GO, 1);
}



bool TDRV2605::stop(void)
{
	return writeRegister8(DRV2605_REG_GO, 0); 
}



bool TDRV2605::setMode (uint8_t mode)
{
	return writeRegister8(DRV2605_REG_MODE, mode);
}



bool TDRV2605::setRealtimeValue(uint8_t rtp)
{
	return writeRegister8(DRV2605_REG_RTPIN, rtp);
}



bool TDRV2605::useERM()
{
	bool rv = false;
	uint8_t rd;
	do	
		{
		if (!readRegister8(DRV2605_REG_FEEDBACK, rd)) break;
		if (!writeRegister8(DRV2605_REG_FEEDBACK, rd & 0x7F)) break;
		rv = true;
		} while (false);
	return rv;
}



bool TDRV2605::useLRA()
{
	bool rv = false;
	uint8_t rd;
	do	
		{
		if (!readRegister8 (DRV2605_REG_FEEDBACK, rd)) break;
		if (!writeRegister8 (DRV2605_REG_FEEDBACK, rd | 0x80)) break;
		rv = true;
		} while (false);
	return rv;
}



void TDRV2605::Task ()
{
	if (relax_timer.get ()) return;

	switch (sw)
		{
		case ESHOCKSW_PIN_EN:
			{
			if (curprog_list) {
				relax_timer.set (DRV2605_POWER_INIT_TIME);		// t(start) Start-up time (0.7-1.5 ms). set ~ 4-5 ms 
				_pin_output (en_pin, true);			// drv en - on
				sw = ESHOCKSW_INIT;
				init_cmd_ix = 0;
				init_ok_cntr = 0;
				}
			else
				{
				sw = ESHOCKSW_STOP;
				}
			break;
			}
		case ESHOCKSW_INIT:
			{
			if (init ()) {
				sw = ESHOCKSW_GO;
				go ();
				}
			break;
			}
		case ESHOCKSW_GO:
			{
			if (curprog_list) {
				uint8_t val_eff = curprog_list->val;
				if (!curprog_list->time && !val_eff) {
					sw = ESHOCKSW_STOP;
					}
				else
					{
					setRealtimeValue (val_eff/4); 
					relax_timer.set (curprog_list->time);
					curprog_list++;
					}
				}
			else
				{
				sw = ESHOCKSW_STOP;
				}
			break;
			}
		case ESHOCKSW_STOP:
			{
			curprog_list = 0;
			stop ();
			_pin_output ( en_pin, false);		// drv en - off
			relax_timer.set (DRV2605_POWER_INIT_TIME);
			sw = ESHOCKSW_NONE;
			break;
			}
		default: break;
		}
}



void TDRV2605::Effect (EERM ef)
{
	curprog_list = 0;
	sw = ESHOCKSW_PIN_EN;
	switch (ef)
		{
		case EERM_EFF_1:
			{
			curprog_list = const_cast<S_VIBRPOINT_T*>(itmlist_prg_1);
			break;
			}
		case EERM_EFF_2:
			{
			curprog_list = const_cast<S_VIBRPOINT_T*>(itmlist_prg_1);
			break;
			}
		case EERM_EFF_3:
			{
			curprog_list = const_cast<S_VIBRPOINT_T*>(itmlist_prg_1);
			break;
			}
		default:
			{
			sw = ESHOCKSW_STOP;
			break;
			}
		}
}



