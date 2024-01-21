#include "TPARAMSPC.h"
#include "STMSTRING.h"


// table size for EPARAMTYPE
static const uint32_t typesize[EPARAMTYPE_ENDENUM] = {0, sizeof(S_HDRPARAM_T/*bool*/), sizeof(S_CONTROL_INT32_T), sizeof(S_CONTROL_UINT32_T), sizeof(S_CONTROL_INT64_T), sizeof(S_CONTROL_UINT64_T), \
sizeof(S_CONTROL_RAW8_T), sizeof(S_CONTROL_FLOAT_T), sizeof(S_CONTROL_STR32_T)};


IRFPARAMS::IRFPARAMS (IFSTORAGE *m, S_HDRPARAM_T **l, uint16_t pcnt) : list (l), c_list_cnt (pcnt)
{
f_changed = false;
f_load_ok = false;
f_is_data_corrected = false;
mem = m;
uint32_t read_sz = sizeof(S_DATAFLASH_T)*c_list_cnt + sizeof(uint32_t);
//dloc = new S_DATAFLASH_T[c_list_cnt + 1];
dloc = (S_DATAFLASH_T*)new uint8_t[read_sz];
if (mem->file_size() == read_sz)
    {
    if (!mem->Read (0, (uint8_t*)dloc, read_sz)) {
        clear ();
        }
    else
        {
        uint32_t sz_calc = read_sz - sizeof(uint32_t);
        uint32_t crc32 = calculate_crc ((uint8_t*)dloc, sz_calc);
        uint32_t *lcrc32 = (uint32_t*)(((uint8_t*)dloc) + sz_calc);
        if (crc32 != *lcrc32)
            {
            clear ();
            }
        else
            {
            f_load_ok = true;
            }
        }
    }
else
    {
    clear ();
    }
correct_all ();
}



void IRFPARAMS::fillmem (void *d, uint8_t dt, uint32_t sz)
{
uint8_t *dst = (uint8_t*)d;
while (sz)
    {
    *dst++ = dt;
    sz--;
    }
}



IRFPARAMS::~IRFPARAMS ()
{
if (f_changed) save ();
if (dloc) delete []dloc;
}



void IRFPARAMS::clear ()
{
uint32_t sz = sizeof(S_DATAFLASH_T)*c_list_cnt;
char *dst = (char*)dloc;
while (sz)
    {
    *dst++ = 0;
    sz--;
    }
f_changed = true;
}



uint32_t IRFPARAMS::calculate_crc (uint8_t *src, uint32_t sz)
{
uint32_t crc = 0;
if (src && sz)
    {
    uint8_t d8;
    uint32_t d32;
    while (sz)
        {
        d8 = *src++;
        if (!d8)
            {
            d32 = 0xA1B82105;
            }
        else
            {
            if (d8 & 0x24)
                {
                d32 = 0x3128220E;
                }
            else
                {
                d32 = 0xB812E311;
                }
            }
        crc += d32;
        sz--;
        }
    }
return crc;
}



void IRFPARAMS::save ()
{
if (mem)
    {
    uint32_t wr_sz = sizeof(S_DATAFLASH_T)*c_list_cnt;
    uint32_t crc32 = calculate_crc ((uint8_t*)dloc, wr_sz);
    mem->Write (0, (uint8_t*)dloc, wr_sz);
    mem->Write (wr_sz, (uint8_t*)&crc32, sizeof(crc32));
    }
}



EPARAMTYPE IRFPARAMS::gettype (uint32_t ix)
{
	EPARAMTYPE rv = EPARAMTYPE_ENDENUM;
	if (ix < c_list_cnt)
		{
		S_HDRPARAM_T *hdr = const_cast<S_HDRPARAM_T*>(list[ix]);
		rv = hdr->type;
		}
	return rv;
}



uint32_t IRFPARAMS::c_file_size (S_HDRPARAM_T **l)
{
uint32_t sz = 0;
if (l)
    {
    uint32_t cnt = 0;
    while (true)
        {
        if (!l[cnt]) break;
        cnt++;
        }
    if (cnt) sz = sizeof(S_DATAFLASH_T)*cnt + sizeof(uint32_t);
    }
return sz;
}




void IRFPARAMS::correct_param (uint32_t ix)
{
if (ix < c_list_cnt)
	{
	S_HDRPARAM_T *hdr = (S_HDRPARAM_T*)list[ix];
	switch (hdr->type)
		{
		case EPARAMTYPE_FLOAT:
			{
			S_CONTROL_FLOAT_T *dt = (S_CONTROL_FLOAT_T*)hdr;
			if (dloc[ix].type == EPARAMTYPE_FLOAT)
				{
				float val = dloc[ix].data.u.v_f;

				if (val > dt->max)
                    {
                    val = dt->max;
                    f_is_data_corrected = true;
                    f_changed = true;
                    }
				if (val < dt->min)
                    {
                    val = dt->min;
                    f_is_data_corrected = true;
                    f_changed = true;
                    }
				dloc[ix].data.u.v_f = val;
				}
			else
				{
				dloc[ix].type = EPARAMTYPE_FLOAT;
				dloc[ix].data.u.v_f = dt->def;
                f_is_data_corrected = true;
                f_changed = true;
				}

			break;
			}
		case EPARAMTYPE_U32:
			{
			S_CONTROL_UINT32_T *dt = (S_CONTROL_UINT32_T*)hdr;
			if (dloc[ix].type == EPARAMTYPE_U32)
				{
				uint32_t val = dloc[ix].data.u.v_u32;
				if (val > dt->max)
                    {
                    val = dt->max;
                    f_is_data_corrected = true;
                    f_changed = true;
                    }
				if (val < dt->min)
                    {
                    val = dt->min;
                    f_is_data_corrected = true;
                    f_changed = true;
                    }
				dloc[ix].data.u.v_u32 = val;
				}
			else
				{
				dloc[ix].type = EPARAMTYPE_U32;
				dloc[ix].data.u.v_u32 = dt->def;
                f_is_data_corrected = true;
                f_changed = true;
				}
            f_changed = true;
			break;
			}
		default: break;
		}
	}
}



void IRFPARAMS::correct_all ()
{
	uint32_t ix = 0;
	while (ix < c_list_cnt)
		{
		correct_param (ix);
		ix++;
		}
}



void IRFPARAMS::param_todef (uint32_t ix)
{
if (ix < c_list_cnt)
	{
	S_HDRPARAM_T *hdr = (S_HDRPARAM_T*)list[ix];
	switch (hdr->type)
		{
		case EPARAMTYPE_FLOAT:
			{
			S_CONTROL_FLOAT_T *dt = (S_CONTROL_FLOAT_T*)hdr;
			dloc[ix].type = EPARAMTYPE_FLOAT;
			dloc[ix].data.u.v_f = dt->def;
            f_changed = true;
			break;
			}
		case EPARAMTYPE_U32:
			{
			S_CONTROL_UINT32_T *dt = (S_CONTROL_UINT32_T*)hdr;
			dloc[ix].type = EPARAMTYPE_U32;
			dloc[ix].data.u.v_u32 = dt->def;
            f_changed = true;
			break;
			}
		default: break;
		}
	}
}



void IRFPARAMS::clear_data_todef ()
{
	uint32_t ix = 0;
	while (ix < c_list_cnt)
		{
		param_todef (ix);
		ix++;
		}
}



long IRFPARAMS::get_papam_i32 (uint32_t ix)
{
	long rv = 0;
	get_papam_i32 (ix, rv);
	return rv;
}


bool IRFPARAMS::get_papam_i32 (uint32_t ix, long &dst)
{
	bool rv = false;
	if (ix < c_list_cnt)
		{
		if (dloc[ix].type == EPARAMTYPE_I32)
			{
			dst = dloc[ix].data.u.v_i32;
			rv = true;
			}
		}
	return rv;
}



bool IRFPARAMS::get_papam_u32 (uint32_t ix, uint32_t &dst)
{
	bool rv = false;
	if (ix < c_list_cnt)
		{
		if (dloc[ix].type == EPARAMTYPE_U32)
			{
			dst = dloc[ix].data.u.v_u32;
			rv = true;
			}
		}
	return rv;
}



uint32_t IRFPARAMS::get_papam_u32 (uint32_t ix)
{
	uint32_t rv = 0;
	get_papam_u32 (ix, rv);
	return rv;
}



float IRFPARAMS::get_papam_f (uint32_t ix)
{
	float rv = 0;
	get_papam_f (ix, rv);
	return rv;
}



bool IRFPARAMS::get_papam_f (uint32_t ix, float &dst)
{
	bool rv = false;
	if (ix < c_list_cnt)
		{
		if (dloc[ix].type == EPARAMTYPE_FLOAT)
			{
			dst = dloc[ix].data.u.v_f;
			rv = true;
			}
		}
	return rv;
}



bool IRFPARAMS::get_papam_b (uint32_t ix)
{
	bool rv = false;
	get_papam_b (ix, rv);
	return rv;
}



bool IRFPARAMS::get_papam_b (uint32_t ix, bool &dst)
{
	bool rv = false;
	if (ix < c_list_cnt)
		{
		if (dloc[ix].type == EPARAMTYPE_BOOL)
			{
			dst = dloc[ix].data.u.v_b;
			rv = true;
			}
		}
	return rv;
}



void IRFPARAMS::set_papam_b (uint32_t ix, bool prm)
{
	if (ix < c_list_cnt)
		{
		if (dloc[ix].type == EPARAMTYPE_BOOL)
			{
			dloc[ix].data.u.v_b = prm;
            f_changed = true;
			}
		}
}



void IRFPARAMS::set_papam_u32 (uint32_t ix, uint32_t prm)
{
	if (ix < c_list_cnt)
		{
		if (dloc[ix].type == EPARAMTYPE_U32)
			{
			dloc[ix].data.u.v_u32 = prm;
            f_changed = true;
			}
		}
}



void IRFPARAMS::set_papam_i32 (uint32_t ix, long prm)
{
	if (ix < c_list_cnt)
		{
		if (dloc[ix].type == EPARAMTYPE_I32)
			{
			dloc[ix].data.u.v_i32 = prm;
            f_changed = true;
			}
		}
}



void IRFPARAMS::set_papam_f (uint32_t ix, float prm)
{
	if (ix < c_list_cnt)
		{
		if (dloc[ix].type == EPARAMTYPE_FLOAT)
			{
			dloc[ix].data.u.v_f = prm;
            f_changed = true;
			}
		}
}



long IRFPARAMS::find_param_to_name (const char *name)
{
long rv = -1, ix = 0;	
if (name)
	{
	uint32_t len_name = TSTMSTRING::lenstr_max (name, sizeof(S_PARAM_CAPTION_T));
	while (ix < c_list_cnt) 
		{
			if (TSTMSTRING::str_compare (const_cast<char*>(list[ix]->name),const_cast<char*>(name), len_name))
			{
			rv = ix;
			break;
			}
		ix++;
		}
	}
return rv;
}


#ifdef RFPARAM_SUPPORT
bool IRFPARAMS::get_param (EPRMIX ix, const S_PARAM_CAPTION_T *name, S_RFPARAMVALUE_T &dst)
{
	bool rv = false;
	long ixfnd = -1;
	
	if (ix < EPRMIX_ENDENUM) 
		{
		ixfnd = ix;
		}
	else
		{
		ixfnd = find_param_to_name (name->txt);
		}

		
	if (ixfnd >= 0)
		{
		S_HDRPARAM_T *hdr = (S_HDRPARAM_T*)list[ixfnd];
		dst.type = hdr->type;
		switch (hdr->type)
			{
			case EPARAMTYPE_U32:
				{
				S_CONTROL_UINT32_T *indata = (S_CONTROL_UINT32_T*)list[ixfnd];
				dst.param.u.dig.def.u.v_u32 = indata->def;
				dst.param.u.dig.max.u.v_u32 = indata->max;
				dst.param.u.dig.min.u.v_u32 = indata->min;
				dst.param.u.dig.val.u.v_u32 = dloc[ixfnd].data.u.v_u32;
				rv = true;
				break;
				}
			case EPARAMTYPE_FLOAT:
				{
				S_CONTROL_FLOAT_T *indata = (S_CONTROL_FLOAT_T*)list[ixfnd];
				dst.param.u.dig.def.u.v_f = indata->def;
				dst.param.u.dig.max.u.v_f = indata->max;
				dst.param.u.dig.min.u.v_f = indata->min;
				dst.param.u.dig.val.u.v_f = dloc[ixfnd].data.u.v_f;
				rv = true;
				break;
				}
			default: break;
			}
		}
	return rv;
}



bool IRFPARAMS::set_param (EPRMIX ix, const S_PARAM_CAPTION_T *name, S_RFPARAMVALUE_T &src)
{
	bool rv = false;
	long ixfnd = -1;
	
	if (ix < EPRMIX_ENDENUM) 
		{
		ixfnd = ix;
		}
	else
		{
		ixfnd = find_param_to_name (name->txt);
		}

		
	if (ixfnd >= 0)
		{
		S_HDRPARAM_T *hdr = (S_HDRPARAM_T*)list[ixfnd];
		if (src.type == hdr->type)
			{
			switch (hdr->type)
				{
				case EPARAMTYPE_U32:
					{
					//S_CONTROL_UINT32_T *outdata = (S_CONTROL_UINT32_T*)list[ix];
					dloc[ix].data.u.v_u32 = src.param.u.dig.val.u.v_u32;
                    f_changed = true;
					rv = true;
					break;
					}
				case EPARAMTYPE_FLOAT:
					{
					//S_CONTROL_FLOAT_T *outdata = (S_CONTROL_FLOAT_T*)list[ix];
					dloc[ix].data.u.v_f = src.param.u.dig.val.u.v_f;
                    f_changed = true;
					rv = true;
					break;
					}
				default: break;
				}
			}
		}
	return rv;
}
#endif
