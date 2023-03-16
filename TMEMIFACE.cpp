#include "TMEMIFACE.h"



T24C02MEM *T24C02MEM::singltn = 0;



T24C02MEM::T24C02MEM ()
{
	timer_autosave = 0;
	SYSBIOS::ADD_TIMER_SYS (&timer_autosave);
	state_sw = E_24C02ST_INIT_LOAD;
	clear_change_flags ();
}


/*
bool TMEMIF::is_ready ()
{
	return f_is_loaded;
}
*/


T24C02MEM *T24C02MEM::obj ()
{
if (!singltn)	
	{
	singltn = new T24C02MEM ();
	}
return singltn;
}



TMEMIF *T24C02MEM::mem_ifc ()
{
return obj ();
}


bool TMEMIF::is_changed ()
{
	bool rv = f_data_is_changed;
	f_data_is_changed = false;
	return rv;
}



bool T24C02MEM::WriteData_l (EGBLPARAM parix, uint32_t data)
{
bool rv = false;
if (parix < EGBLPARAM_ENDENUM)
	{
	if (data != memtags[parix].data_l)
		{
		memchangflags[parix] = true;
		memtags[parix].data_l = data;
		}
	f_data_is_changed = true;
	rv = true;
	}
return rv;	
}







bool T24C02MEM::WriteData_f (EGBLPARAM parix, float data)
{
bool rv = false;
if (parix < EGBLPARAM_ENDENUM)
	{
	if (data != memtags[parix].data_f)
		{
		memchangflags[parix] = true;
		memtags[parix].data_f = data;
		}
	f_data_is_changed = true;
	rv = true;
	}
return rv;	
}



bool T24C02MEM::ReadData_l (EGBLPARAM parix, uint32_t &data)
{
bool rv = false;
if (parix < EGBLPARAM_ENDENUM)
	{
	data = memtags[parix].data_l;
	rv = true;
	}
return rv;	
}



bool T24C02MEM::ReadData_f (EGBLPARAM parix, float &data)
{
bool rv = false;
if (parix < EGBLPARAM_ENDENUM)
	{
	data = memtags[parix].data_f;
	rv = true;
	}
return rv;	
}



uint32_t T24C02MEM::ReadData_l (EGBLPARAM parix)
{
	uint32_t rv = 0;
	ReadData_l (parix, rv);
	return rv;
}



float T24C02MEM::ReadData_f (EGBLPARAM parix)
{
	float rv = 0;
	ReadData_f (parix, rv);
	return rv;
}



void T24C02MEM::clear_change_flags ()
{
	uint32_t ix = 0;
	f_data_is_changed = false;
	while (ix < EGBLPARAM_ENDENUM)
		{
		memchangflags[ix] = false;
		ix++;
		}
}



void TMEMIF::load_all (TI2CIFACE *i2cobj)
{
	uint32_t ix = 0;
	S_MEMPARAMTAG_T tag;
	S_MEMPARAMTAG_T zero_tag;
	zero_tag.data_l = 0;
	i2cobj->Hard_Init ();
	while (ix < EGBLPARAM_ENDENUM)
		{
		if (read_tag (i2cobj, ix, tag))
			{
			memtags[ix] = tag;
			}
		else
			{
			memtags[ix] = zero_tag;
			}
		ix++;
		}
	f_data_is_changed = true;
}



long T24C02MEM::get_needwr_data_ix ()
{
	long rv = -1, ix = 0;
	while (ix < EGBLPARAM_ENDENUM)
		{
		if (memchangflags[ix] == true)
			{
			rv = ix;
			break;
			}
		ix++;
		}
	return rv;
}



bool T24C02MEM::write_tag (TI2CIFACE *i2cobj, uint32_t ix, S_MEMPARAMTAG_T *tag)
{
bool rv = false;
if (i2cobj) {
	if (tag) {
		uint8_t adr_mem = ix * sizeof(S_MEMPARAMTAG_T);
		rv = i2cobj->WriteFrame (C_M24C02I2CADR, adr_mem, (unsigned char *)&tag->data_l, sizeof(tag->data_l));
		memchangflags[rv] = false;
		}
	}
return rv;	
}



bool T24C02MEM::read_tag (TI2CIFACE *i2cobj, uint32_t ix, S_MEMPARAMTAG_T &tag)
{
bool rv = false;
if (i2cobj) {
	uint8_t adr_mem = ix * sizeof(S_MEMPARAMTAG_T);
	rv = i2cobj->ReadFrame (C_M24C02I2CADR, adr_mem, (unsigned char *)&tag.data_l, sizeof(tag.data_l));
	}
return rv;	
}



void T24C02MEM::Start ()
{
	if (BusErrorCount_i2c)
		{
		BusErrorCount_i2c = 0;
		state_sw = E_24C02ST_INIT;
		}
	else
		{
		state_sw = E_24C02ST_CHK_NEED_WR;
		}
	F_complete = false;
}



void T24C02MEM::Task (TI2CIFACE *i2cobj)
{
	switch (state_sw)
		{
		case E_24C02ST_INIT_LOAD:
			{
			i2cobj->Hard_Init ();
			/*
			if (!f_is_loaded)
				{
				load_all (i2cobj);
				f_is_loaded = true;
				}
			*/
			state_sw = E_24C02ST_INIT;
			break;
			}
		case E_24C02ST_INIT:
			{
			i2cobj->Hard_Init ();
			state_sw = E_24C02ST_CHK_NEED_WR;
			break;
			}
		case E_24C02ST_CHK_NEED_WR:
			{
			/* write test
			static bool f_tstwr = true;
			if (f_tstwr)
				{
				WriteData_f (EGBLPARAM_ANGL_BORDER_MIN_X, 12345.4321);
				f_tstwr = false;
				}
			*/
			long ix = get_needwr_data_ix ();
			if (ix >= 0)
				{
				state_sw = E_24C02ST_WR;
				cur_wr_ix = ix;
				memchangflags[ix] = false;
				}
			break;
			}
		case E_24C02ST_WR:
			{
			S_MEMPARAMTAG_T *tag = &memtags[cur_wr_ix];
			if (write_tag (i2cobj, cur_wr_ix, tag))
				{
				state_sw = E_24C02ST_CHK_WR;
				}
			else
				{
				// i2c error
				BusErrorCount_chip++;
				state_sw = E_24C02ST_BUSSERROR; 
				}
			break;
			}
		case E_24C02ST_CHK_WR:
			{
			S_MEMPARAMTAG_T rddat;
			if (read_tag (i2cobj, cur_wr_ix, rddat))
				{
				if (rddat.data_l == memtags[cur_wr_ix].data_l)
					{
					state_sw = E_24C02ST_RELEASE; 
					}
				else
					{
					BusErrorCount_chip++;
					state_sw = E_24C02ST_BUSSERROR; 
					}
				}
			else
				{
				BusErrorCount_i2c++; 	
				state_sw = E_24C02ST_BUSSERROR; 
				}
			break;
			}
		case E_24C02ST_RELEASE:
			{
			F_complete = true;
			break;
			}
		case E_24C02ST_BUSSERROR:
			{
			state_sw = E_24C02ST_RELEASE;
			break;
			}
		default:
			{
			break;
			}
		}
}



