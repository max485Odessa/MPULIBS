#include "capsulcore.h"



TCAPMUX::TCAPMUX (uint32_t rxsz, uint32_t txsz, uint8_t micnt) : c_raw_rx_alloc_size (rxsz), c_raw_tx_alloc_size (txsz), c_muxifcrx_cnt (micnt)
{
	raw_rx = new uint8_t[c_raw_rx_alloc_size];
	raw_tx = new uint8_t[c_raw_tx_alloc_size];
	listifc = new S_CPMXRXDATA_T[c_muxifcrx_cnt];
	ifcadd_ix = 0;
	push_rx_ix = 0;
	push_tx_ix = 0;
	pop_tx_ix = 0;
	size_tx_ix = 0;
}



TCAPMUX::TCAPMUX (void *s_rx, uint32_t rxsz, void *s_tx, uint32_t txsz, uint8_t micnt): c_raw_rx_alloc_size (rxsz), c_raw_tx_alloc_size (txsz), c_muxifcrx_cnt (micnt)
{
	raw_rx = (uint8_t*)s_rx;
	raw_tx = (uint8_t*)s_tx;
	listifc = new S_CPMXRXDATA_T[c_muxifcrx_cnt];
	ifcadd_ix = 0;
	push_rx_ix = 0;
	push_tx_ix = 0;
	pop_tx_ix = 0;
	size_tx_ix = 0;
}



long TCAPMUX::findaifc (uint32_t mxid)
{
	long rv = -1, ix = 0;
	while (ix < ifcadd_ix)
		{
		if (listifc[ix].muxid == mxid)
			{
			rv = ix;
			break;
			}
		ix++;
		}
	return rv;
}



bool TCAPMUX::AddOrReplace_rx_ifc (uint32_t mxid, TCPMXRXCB *mcb)
{
	bool rv = false;
	long fnix = findaifc (mxid);
	if (fnix >= 0)
		{
		listifc[fnix].cb = mcb;
		rv = true;
		}
	else
		{
		if (ifcadd_ix < c_muxifcrx_cnt)
			{
			listifc[ifcadd_ix].cb = mcb;
			listifc[ifcadd_ix].muxid = mxid;	
			ifcadd_ix++;
			rv = true;
			}
		}
	return rv;
}



uint16_t TCAPMUX::muxprotocol_crc (void *data, uint16_t ctrl_size)
{
uint16_t crc = 0;
uint8_t *s = (uint8_t*)data;
while (ctrl_size)
    {
    crc = (*s) + ctrl_size;
    s++;
    ctrl_size--;
    }
return crc;
}



bool TCAPMUX::check_muxcapsule_linedata (void *basebuf, uint32_t *ix_start, uint32_t *mess_size, uint32_t start_ix, uint32_t last_psh_ix)
{
bool rv = false;
uint32_t ix = start_ix, datasize;
S_MUXCAPSULE_HDR_T *lMxCap;
uint8_t *rxdatabuf = (uint8_t*)basebuf;
while (ix < last_psh_ix)
    {
    datasize = last_psh_ix - ix;
    lMxCap = (S_MUXCAPSULE_HDR_T*)&rxdatabuf[ix];
    if (datasize > (sizeof(S_MUXCAPSULE_HDR_T)))
        {
        if (lMxCap->Preamble_A == C_MUXPREAMBLE_A && lMxCap->Preamble_B == C_MUXPREAMBLE_B)
            {
            if (lMxCap->size && lMxCap->size <= C_MUXDATASIZE_MAX)
                {
                unsigned short full_len = sizeof(S_MUXCAPSULE_HDR_T) + lMxCap->size;
                if (datasize >= full_len)
                    {
                    // проверка контрольной суммы
                    uint8_t *s = (uint8_t*)lMxCap;
                    s += sizeof(S_MUXCAPSULE_HDR_T);
                    uint16_t crc = muxprotocol_crc (s, lMxCap->size);
                    if (lMxCap->crc == crc)
                        {
                        if (ix_start)*ix_start = ix;
                        if (mess_size)*mess_size = full_len;
                        rv = true;
                        break;
                        }
                    }
                }
            }
        }
    else
        {
        break;
        }
    ix++;
    }
return rv;
}



char *TCAPMUX::copysdc (char *s, char *d, uint32_t sz)
{
while (sz)
    {
    *d++ = *s++;
    sz--;
    }
return d;
}




bool TCAPMUX::detect_protocols (void *base, uint32_t start_ix, uint32_t stop_ix)
{
	bool rv = false;
	if (stop_ix > start_ix) {
        rv = check_muxcapsule_linedata (base, &detect_find_ix, &detect_find_size, start_ix, stop_ix);
		}
return rv;
}



void TCAPMUX::rx_push_raw (uint8_t *s, uint32_t sz)
{
uint32_t free_size = c_raw_rx_alloc_size - push_rx_ix;
if (sz > free_size) sz = free_size;
copysdc ((char*)s, (char*)&raw_rx[push_rx_ix], sz);
push_rx_ix += sz;
}



void TCAPMUX::cb_execute_mux_frame (uint8_t *s, uint16_t sz)
{
S_MUXCAPSULE_HDR_T *lMxCap = (S_MUXCAPSULE_HDR_T*)s;
long fndix = findaifc (lMxCap->Mux);
if (fndix >= 0) {
	listifc[fndix].cb->capmux_rx_cb (lMxCap->Mux, s + sizeof(S_MUXCAPSULE_HDR_T), lMxCap->size);
	}
}



void TCAPMUX::mux_rx_data ()
{
  uint32_t ix_srch = 0, c_size;
	
  if (push_rx_ix >= c_raw_rx_alloc_size) 
      {
      push_rx_ix  = 0;
      return;
      }
	
  c_size = push_rx_ix;

	if (c_size)
			{
			if (detect_protocols (raw_rx, 0, c_size))
					{
					// запуск callback
					cb_execute_mux_frame (&raw_rx[detect_find_ix], detect_find_size);
					// проверяем нужно ли перемещение оставшихся данных в начало

					uint32_t last_ix = detect_find_ix + detect_find_size;
					uint32_t loc_push_ix = push_rx_ix;		// update new lp
					if (last_ix <= loc_push_ix)
							{
							uint32_t tailsize = loc_push_ix - last_ix;
							copysdc ((char*)&raw_rx[last_ix], (char*)raw_rx, tailsize);
							push_rx_ix = tailsize;
							}
					else
							{
							// непредвиденная ошибка
							push_rx_ix = 0;
							}
					}
			}
}



bool TCAPMUX::tx_free_space_check (uint32_t sz)
{
bool rv = false;
uint32_t freespace = c_raw_tx_alloc_size - size_tx_ix;
if (freespace >= sz) rv = true;
return rv;
}



void TCAPMUX::push_tx_data (char *s, uint32_t sz)
{
uint32_t incsz = sz;
while (sz)
    {
    if (push_tx_ix >= c_raw_tx_alloc_size) push_tx_ix = 0;
    raw_tx[push_tx_ix++] = *s++;
    sz--;
    }
size_tx_ix += incsz;
}



bool TCAPMUX::mux_tx_push (uint16_t mux, void *s, uint32_t sz)
{
bool rv = false;
uint32_t full_tx_sz = sz + sizeof(S_MUXCAPSULE_HDR_T);
rv = tx_free_space_check (full_tx_sz);
if (rv && s && sz)
    {
    S_MUXCAPSULE_HDR_T frame;
    frame.Preamble_A = C_MUXPREAMBLE_A;
    frame.Preamble_B = C_MUXPREAMBLE_B;
    frame.Mux = mux;
    frame.size = sz;
    frame.crc = muxprotocol_crc (s, sz);
    push_tx_data ((char*)&frame, sizeof(S_MUXCAPSULE_HDR_T));
    push_tx_data ((char*)s, sz);
    }
return rv;
}



uint32_t TCAPMUX::tx_pushed_size ()
{
	return size_tx_ix;
}



uint32_t TCAPMUX::tx_pop_raw (void *dst, uint32_t masize)
{
uint32_t real_pop_sz = size_tx_ix;
if (real_pop_sz > masize) real_pop_sz = masize;
uint32_t popsz = real_pop_sz;
uint8_t *d = (uint8_t*)dst;
while (real_pop_sz)
    {
    if (pop_tx_ix >= c_raw_tx_alloc_size) pop_tx_ix = 0;
    *d++ = raw_tx[pop_tx_ix++];
    real_pop_sz--;
    }
size_tx_ix -= popsz;
return popsz;
}



