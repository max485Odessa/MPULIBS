#include "TMAVPARAMS.h"



TPARAMCONTRL::TPARAMCONTRL (TEEPROMIF *m, uint32_t start_m, uint32_t size_m) : c_startflash_adr (start_m), c_sizeflash (size_m)
{
	flashmem = m;
}



TMAVPARAMS::TMAVPARAMS (TEEPROMIF *m, uint32_t start_m, uint32_t size_m,  S_MVPARAM_HDR_T **list_int, uint32_t cnt_int, uint32_t cnt_external) : TPARAMCONTRL (m, start_m, size_m)
{
	param_internal = list_int;
	param_int_count = cnt_int;
	
	param_ext_max_alocate = cnt_external;
	param_ext_count = 0;
	param_external = (param_ext_max_alocate)?new S_BASETAGPARAM_T[param_ext_max_alocate]:0;

	if (!load_internal_params ())
		{
		sets_internal_params_to_default ();
		f_need_save = true;
		}
	else
		{
		f_need_save = false;
		}
}



uint32_t TPARAMCONTRL::param_internal_cnt ()
{
	return param_int_count;
}



uint32_t TPARAMCONTRL::param_external_cnt ()
{
	return param_ext_count;
}



uint32_t TPARAMCONTRL::param_full_cnt ()
{
	return param_ext_count + param_int_count;
}



bool TPARAMCONTRL::load_internal_params ()
{
	bool rv = false;
	uint32_t load_cnt;
	uint32_t load_crc32;
	uint32_t read_adr = c_startflash_adr;
	
	if (flashmem->read ( read_adr, (uint8_t*)&load_cnt, sizeof(load_cnt)))
		{
		if (load_cnt == param_int_count)
			{
			read_adr += sizeof(load_cnt);
			S_MDAT_T *savearr = new S_MDAT_T[param_int_count];
			uint32_t loadbytesize =  sizeof(S_MDAT_T) * param_int_count;
			if (flashmem->read (read_adr, (uint8_t*)savearr, loadbytesize))
				{
				read_adr += loadbytesize;
				if (flashmem->read (read_adr, (uint8_t*)&load_crc32, sizeof(load_crc32)))
					{
					if (load_crc32 == GN_CRC32 (savearr, loadbytesize))
						{
						uint32_t ix = 0;
						S_BASETAGPARAM_T *dst;
						while (ix < param_int_count)
							{
							dst = (S_BASETAGPARAM_T*)param_internal[ix];
							//CopyMemorySDC ((char*)&savearr[ix], (char*)&dst->value.u.u32, sizeof(S_MDAT_T));
							dst->value = savearr[ix];
							ix++;
							}
						rv = true;
						}
					}
				}
			delete [] savearr;
			}
		}
	return rv;
}



bool TPARAMCONTRL::save_internal_params ()
{
bool rv = false;
uint32_t write_cnt = param_int_count;
uint32_t write_adr = c_startflash_adr;
flashmem->erase_sector (write_adr);	

if (flashmem->write ( write_adr, (uint8_t*)&write_cnt, sizeof(write_cnt)))
	{
	write_adr += sizeof(write_cnt);
	S_MDAT_T *savearr = new S_MDAT_T[param_int_count];
	uint32_t ix = 0;
	S_BASETAGPARAM_T *dst;	
	while (ix < param_int_count)
		{
		//CopyMemorySDC ((char*)&((S_BASETAGPARAM_T*)param_internal[ix])->value.u.u32, (char*)&savearr[ix], sizeof(S_MDAT_T));
		savearr[ix] = ((S_BASETAGPARAM_T*)param_internal[ix])->value;
		ix++;
		}
	uint32_t writebytesize =  sizeof(S_MDAT_T) * param_int_count;
	if (flashmem->write ( write_adr, (uint8_t*)savearr, writebytesize))
		{
		write_adr += writebytesize;
		uint32_t write_crc32 = GN_CRC32 (savearr, writebytesize);
		if (flashmem->write ( write_adr, (uint8_t*)&write_crc32, sizeof(write_crc32))) 
			{
			f_need_save = false;
			rv = true;
			}
		}
	delete [] savearr;
	}
return rv;
}



void TPARAMCONTRL::sets_internal_params_to_default ()
{
	uint32_t ix = 0;
	while (ix < param_int_count)
		{
		set_param_to_default (ix);
		ix++;
		}
}



S_MVPARAM_HDR_T *TPARAMCONTRL::find_param_to_name (char *name, long *dst_ix)
{
	S_MVPARAM_HDR_T *rv = 0;
	long ix = 0;
	TSTMSTRING str;
	uint32_t maxparams_cnt = param_full_cnt ();
	while (ix < maxparams_cnt)
		{
		S_MVPARAM_HDR_T *paramtag = get_param_tag (ix);
		if (!paramtag) break;
		str.set_context (paramtag->param_id, sizeof(paramtag->param_id)-1);
		if (str == name)
			{
			if (dst_ix) *dst_ix = ix;
			rv = paramtag;
			break;
			}
		ix++;
		}
	return rv;
}




S_MVPARAM_HDR_T *TPARAMCONTRL::get_param_tag (long ix)
{
	S_MVPARAM_HDR_T *rv = 0;
	if (ix >=0 && ix < param_full_cnt ()) 
		{
		if (ix < param_int_count)
			{
			// internal params
			rv = const_cast<S_MVPARAM_HDR_T*>(param_internal[ix]);
			}
		else
			{
			ix -= param_int_count;
			// external params
			if (param_external) rv = (S_MVPARAM_HDR_T*)&param_external[ix];
			}
		}
	return rv;
}



S_MVPARAM_HDR_T *TMAVPARAMS::find_params_m (mavlink_param_value_t *p)
{
	return find_param_to_name (p->param_id, 0);
}



long TMAVPARAMS::add_external_params (S_MVPARAM_HDR_T *p)
{
	long rv = -1;
	S_MVPARAM_HDR_T *slot = find_param_to_name (p->param_id, 0);
	if (slot && param_external) {
		if (param_ext_max_alocate && param_ext_count < param_ext_max_alocate) {
			S_BASETAGPARAM_T *ftagsize = (S_BASETAGPARAM_T*)p;
			param_external[param_ext_count] = *ftagsize;
			rv = param_full_cnt ();
			param_ext_count++;
			}
		}
	return rv;
}



S_MDAT_T *TPARAMCONTRL::get_value (char *name)
{
	S_MDAT_T *rv = 0;
	S_MVPARAM_HDR_T *s = find_param_to_name (name, 0);
	if (s)  {
		S_BASETAGPARAM_T *slot = (S_BASETAGPARAM_T*)s;
		rv = &slot->value;
		}
	return rv;
}



S_MDAT_T *TPARAMCONTRL::get_value (long ix)
{
	S_MDAT_T *rv = 0;
	S_MVPARAM_HDR_T *s = get_param_tag (ix);
	if (s) {
		S_BASETAGPARAM_T *slot = (S_BASETAGPARAM_T*)s;
		rv = &slot->value;
		}
	return rv;
}



S_MDAT_T TPARAMCONTRL::get_value_d (long ix)
{
	S_MDAT_T rv = {0};
	S_MVPARAM_HDR_T *s = get_param_tag (ix);
	if (s) {
		S_BASETAGPARAM_T *slot = (S_BASETAGPARAM_T*)s;
		rv = slot->value;
		}
	return rv;
}



bool TPARAMCONTRL::set_value_l (long ix, long d)
{
	bool rv = false;
		S_MVPARAM_HDR_T *paramtag = get_param_tag (ix);
		if (paramtag) {
			if (paramtag->type == MAV_PARAM_TYPE_INT32)
				{
				S_MVPARAM_U32_T *plong = (S_MVPARAM_U32_T*)paramtag;
				if (plong->min > d) d = plong->min;
				if (plong->max < d) d = plong->max;
				plong->value = d;
				rv = true;
				}
			}
	return rv;
}



bool TPARAMCONTRL::set_value (long ix, S_MDAT_T v)
{
	bool rv = false;
		S_BASETAGPARAM_T *slot = (S_BASETAGPARAM_T*)get_param_tag (ix);
		if (slot)
			{
			slot->value = v;
			f_need_save = true;
			}
	return rv;
}




bool TPARAMCONTRL::set_value (char *name, S_MDAT_T v)
{
	bool rv = false;
	S_MVPARAM_HDR_T *s = find_param_to_name (name, 0);
	if (s) {
		S_BASETAGPARAM_T *slot = (S_BASETAGPARAM_T*)s;
		slot->value = v;
		f_need_save = true;
		}
	return rv;
}



void TPARAMCONTRL::set_param_to_default (long ix)
{
	S_MVPARAM_HDR_T *s = get_param_tag (ix);
	if (s) {
		S_BASETAGPARAM_T *slot = (S_BASETAGPARAM_T*)s;
		switch (slot->hdr.type)
			{
			case MAV_PARAM_TYPE_UINT32:
			case MAV_PARAM_TYPE_INT32:
				{
				slot->value.u.u32 = slot->def_value.u.u32;
				break;
				}
			case MAV_PARAM_TYPE_REAL32:
				{
				slot->value.u.f = slot->def_value.u.f;
				break;
				}
			default: break;
			}
		}
}



// param_count and param_index not copies
bool TMAVPARAMS::get_mavlink_param (char *name, mavlink_param_value_t *dst, long *dst_ix)
{
	bool rv = false;
	S_BASETAGPARAM_T *s = (S_BASETAGPARAM_T*)find_param_to_name (name, dst_ix);
	if (s) {
		if (dst) update_mavparamval (s, *dst);
		rv = true;
		}
	return rv;
}



// param_count and param_index not copies
void TMAVPARAMS::update_mavparamval (S_BASETAGPARAM_T *s, mavlink_param_value_t &dst)
{
		CopyMemorySDC (s->hdr.param_id, dst.param_id, sizeof(dst.param_id));		// name tag copy
		dst.param_type = s->hdr.type;		// copy type
		switch (s->hdr.type)
			{
			case MAV_PARAM_TYPE_REAL32:
				{
				dst.param_value = s->value.u.f;
				break;
				}
			case MAV_PARAM_TYPE_INT32:
			case MAV_PARAM_TYPE_UINT32:
				{
				dst.param_value = s->value.u.u32;
				break;
				}
			default: break;
			}
		//CopyMemorySDC ((char*)&s->value.u.u32, (char*)&dst.param_value, sizeof(dst.param_value));	// copy value
}



// param_count and param_index not copies
bool TMAVPARAMS::get_mavlink_param (long ix, mavlink_param_value_t &dst)
{
	bool rv = false;
	S_BASETAGPARAM_T *s = (S_BASETAGPARAM_T*)get_param_tag (ix);
	if (s) {
		update_mavparamval (s, dst);
		rv = true;
		}
	return rv;
}



bool TMAVPARAMS::update_mavlink_param (const mavlink_param_value_t &src)
{
	bool rv = false;
	S_BASETAGPARAM_T *dst = (S_BASETAGPARAM_T*)find_param_to_name (const_cast<char*>(src.param_id), 0);
	if (dst) {
		if (src.param_type == dst->hdr.type) {		// src.param_type == dst->hdr.type
			switch (src.param_type)
				{
				case MAV_PARAM_TYPE_INT32:
				case MAV_PARAM_TYPE_UINT32:
					{
					dst->value.u.u32 = src.param_value;
					break;
					}
				case MAV_PARAM_TYPE_REAL32:
					{
					dst->value.u.f = src.param_value;
					//CopyMemorySDC ((char*)&src.param_value, (char*)&dst->value, sizeof(dst->value));	// copy value
					break;
					}
				}
			f_need_save = true;
			rv = true;
			}
		}
	return rv;
}



bool TMAVPARAMS::update_mavlink_param (const mavlink_param_set_t &src, long *dst_ix)
{
	bool rv = false;
	S_BASETAGPARAM_T *dst = (S_BASETAGPARAM_T*)find_param_to_name (const_cast<char*>(src.param_id), dst_ix);
	if (dst) {
		if (src.param_type == dst->hdr.type) 
			{
			switch (src.param_type)
				{
				case MAV_PARAM_TYPE_INT32:
				case MAV_PARAM_TYPE_UINT32:
					{
					dst->value.u.u32 = src.param_value;
					break;
					}
				case MAV_PARAM_TYPE_REAL32:
					{
					dst->value.u.f = src.param_value;
					//CopyMemorySDC ((char*)&src.param_value, (char*)&dst->value, sizeof(dst->value));	// copy value
					break;
					}
				}
			
			f_need_save = true;
			rv = true;
			}
		}
	return rv;
}



void TMAVPARAMS::save_to_flash ()
{
	save_internal_params ();
}

