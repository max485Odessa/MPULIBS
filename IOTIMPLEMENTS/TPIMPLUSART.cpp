#include "TPIMPLUSART.H"


TUSARTMASTER::TUSARTMASTER (TTXIF *t, local_rf_id_t sa)
{
tx_obj = t;
self_id = sa;
c_txbufer_size = sizeof(S_RFCMDS_SIZES_UNION_T) + sizeof(S_CHANHDR_T) + 32;
txbufer = new uint8_t[c_txbufer_size];
}



TUSARTMASTER::~TUSARTMASTER ()
{
delete [] txbufer;
}



uint32_t TUSARTMASTER::lenstr (void *s)
{
uint32_t rv = 0;
char *src = (char*)s;
while (true)
    {
    if (*src == 0) break;
    src++;
    rv++;
    }
return rv;
}



void TUSARTMASTER::CopyMemorySDC (void *s, void *d, uint32_t sz)
{
char *src = (char*)s;
char *dst = (char*)d;
while (sz)
    {
    *dst++ = *src++;
    sz--;
    }

}



bool TUSARTMASTER::find_protocol (void *s, uint32_t szsrc, uint32_t &find_ix, uint32_t &findsz)
{
bool rv = false;
uint32_t ix = 0, datasize;
uint8_t *src = (uint8_t*)s;
S_CHANHDR_T *lhdr;
while (ix < szsrc)
    {
    datasize = szsrc - ix;
    lhdr = (S_CHANHDR_T*)&src[ix];
    if (datasize > (sizeof(S_CHANHDR_T)))
        {
        if (lhdr->preamble_a == C_PREAMBLE_P1_A && lhdr->preamble_b == C_PREAMBLE_P1_B)
            {
            unsigned short full_len = sizeof(S_CHANHDR_T) + lhdr->size;
            if (datasize >= full_len)
                {
                uint16_t svcrc = lhdr->crc16;
                lhdr->crc16 = 0;
                uint16_t clccrc = calculate_crc (src, full_len);
                lhdr->crc16 = svcrc;
                if (clccrc == svcrc)
                    {
                    find_ix = ix;
                    findsz = full_len;
                    rv = true;
                    break;
                    }
                }
            }
        }
    ix++;
    }
return rv;
}



uint16_t TUSARTMASTER::calculate_crc (uint8_t *src, uint32_t sz)
{
uint16_t crc = 0;
uint8_t dat;
while (sz)
    {
    dat = *src++;
    if (!dat)
        {
        crc += 0x0201;
        }
    else
        {
        if (dat == 0xFF)
            {
            crc += 0x1014;
            }
        else
            {
            if (dat & 0x41)
                {
                crc += 0x25;
                }
            else
                {
                crc += dat;
                }
            }
        }
    sz--;
    }
return crc;
}



bool TUSARTMASTER::parse (void *src, uint32_t szsrc)
{
}



void TUSARTMASTER::rawsend (void *s, uint32_t sz, uint32_t tmot)
{
uint8_t *src = (uint8_t*)s;

}



void TUSARTMASTER::get_param_req (local_rf_id_t dvid, uint16_t ix, uint32_t tmot)
{
	S_CMD_GET_PARAM_REQ_T reqframe;
	reqframe.hdr.cmd = ECMDLAN_GET_PARAM_REQ;
	reqframe.hdr.cmd_size = sizeof(S_CMD_GET_PARAM_REQ_T);
	reqframe.hdr.dst_id = dvid;
	reqframe.hdr.src_id = self_id;
	reqframe.ix = ix;
	reqframe.name.txt[0] = 0;
	rawsend (&reqframe, sizeof(reqframe), tmot);
}



void TUSARTMASTER::set_param_req (local_rf_id_t dvid, uint16_t ix, S_RFPARAMVALUE_T prm, uint32_t tmot)
{
	S_CMD_SET_PARAM_REQ_T reqframe;
	reqframe.hdr.cmd = ECMDLAN_SET_PARAM_REQ;
	reqframe.hdr.cmd_size = sizeof(S_CMD_SET_PARAM_REQ_T);
	reqframe.hdr.dst_id = dvid;
	reqframe.hdr.src_id = self_id;
	reqframe.ix = ix;
	reqframe.name.txt[0] = 0;
	reqframe.param = prm;
	rawsend (&reqframe, sizeof(reqframe), tmot);
}



void TUSARTMASTER::set_param_req (local_rf_id_t dvid, char *name, S_RFPARAMVALUE_T prm, uint32_t tmot)
{
	S_CMD_SET_PARAM_REQ_T reqframe;
	reqframe.hdr.cmd = ECMDLAN_SET_PARAM_REQ;
	reqframe.hdr.cmd_size = sizeof(S_CMD_SET_PARAM_REQ_T);
	reqframe.hdr.dst_id = dvid;
	reqframe.hdr.src_id = self_id;
	reqframe.ix = -1;
	reqframe.param = prm;
	if (name)
		{
		uint32_t lnstr = lenstr (name);
		if (lnstr > sizeof(reqframe.name.txt)) lnstr = sizeof(reqframe.name.txt);
		CopyMemorySDC (name, reqframe.name.txt, lnstr);
		}
	else
		{
		reqframe.name.txt[0] = 0;
		}
	rawsend (&reqframe, sizeof(reqframe), tmot);
}



void TUSARTMASTER::get_param_req (local_rf_id_t dvid, char *name, uint32_t tmot)
{
	S_CMD_GET_PARAM_REQ_T reqframe;
	reqframe.hdr.cmd = ECMDLAN_GET_PARAM_REQ;
	reqframe.hdr.cmd_size = sizeof(S_CMD_GET_PARAM_REQ_T);
	reqframe.hdr.dst_id = dvid;
	reqframe.hdr.src_id = self_id;
	reqframe.ix = -1;

	if (name)
		{
		uint32_t lnstr = lenstr (name);
		if (lnstr > sizeof(reqframe.name.txt)) lnstr = sizeof(reqframe.name.txt);
		CopyMemorySDC (name, reqframe.name.txt, lnstr);
		}
	else
		{
		reqframe.name.txt[0] = 0;
		}
	rawsend (&reqframe, sizeof(reqframe), tmot);
}



void TUSARTMASTER::get_event_req (local_rf_id_t dvid, uint16_t ix, uint32_t tmot)
{
	S_CMD_GET_EVENT_REQ_T reqframe;
	reqframe.hdr.cmd = ECMDLAN_GET_EVENT_REQ;
	reqframe.hdr.cmd_size = sizeof(S_CMD_GET_EVENT_REQ_T);
	reqframe.hdr.dst_id = dvid;
	reqframe.hdr.src_id = self_id;
	reqframe.ix = ix;
	rawsend (&reqframe, sizeof(reqframe), tmot);
}



void TUSARTMASTER::call_event_req (local_rf_id_t dvid, uint32_t event_code, uint32_t calltime, uint32_t tmot )
{
	S_CMD_CALL_EVENT_REQ_T reqframe;
	reqframe.hdr.cmd = ECMDLAN_CALL_EVENT_REQ;
	reqframe.hdr.cmd_size = sizeof(S_CMD_CALL_EVENT_REQ_T);
	reqframe.hdr.dst_id = dvid;
	reqframe.hdr.src_id = self_id;
	reqframe.event_code = event_code;
	reqframe.time_event_work = calltime;
	rawsend (&reqframe, sizeof(reqframe), tmot);
}




void TUSARTMASTER::Task ()
{
}



void TRFMASTER::RF_recv_cb (uint8_t *data, uint16_t sz, uint16_t rssi)
{
if (sz <= c_rxbuf_size)
	{
	CopyMemorySDC (data, rxbufer, sz);
	S_RFHEADER_T *hdr = (S_RFHEADER_T*)rxbufer;
	if (hdr->dst_id	== self_id)
		{
		switch (hdr->cmd)
			{
			case ECMDLAN_GET_PARAM_RESP:
				{
				if (hdr->cmd_size != sizeof(S_CMD_GET_PARAM_RESP_T)) break;
				S_CMD_GET_PARAM_RESP_T *in_req = (S_CMD_GET_PARAM_RESP_T*)rxbufer;

				ERESPSTATE rx_state = (ERESPSTATE)in_req->resp_state;
				get_param_resp_cb (in_req->hdr.src_id, in_req->ix, const_cast<S_PARAM_CAPTION_T*>(&in_req->name), in_req->param, rx_state);
				break;
				}
			case ECMDLAN_SET_PARAM_RESP:
				{
				if (hdr->cmd_size != sizeof(S_CMD_SET_PARAM_RESP_T)) break;
				S_CMD_SET_PARAM_RESP_T *in_req = (S_CMD_SET_PARAM_RESP_T*)rxbufer;
					
				ERESPSTATE rx_state = (ERESPSTATE)in_req->resp_state;	
				set_param_resp_cb (in_req->hdr.src_id, in_req->ix, const_cast<S_PARAM_CAPTION_T*>(&in_req->name), in_req->param, rx_state);

				break;
				}
			case ECMDLAN_GET_EVENT_RESP:
				{
				if (hdr->cmd_size != sizeof(S_CMD_GET_EVENT_RESP_T)) break;
				S_CMD_GET_EVENT_RESP_T *in_req = (S_CMD_GET_EVENT_RESP_T*)rxbufer;
					
				ERESPSTATE rx_state = (ERESPSTATE)in_req->resp_state;	
				get_event_resp_cb (in_req->hdr.src_id, in_req->ix, &in_req->event, rx_state);
				break;
				}
			case ECMDLAN_CALL_EVENT_RESP:
				{
				if (hdr->cmd_size != sizeof(S_CMD_CALL_EVENT_RESP_T)) break;
				S_CMD_CALL_EVENT_RESP_T *in_req = (S_CMD_CALL_EVENT_RESP_T*)rxbufer;

				ERESPSTATE rx_state = (ERESPSTATE)in_req->resp_state;	
				call_event_resp_cb (in_req->hdr.src_id, in_req->event_code, rx_state);
				break;
				}
			case ECMDLAN_GET_STATE_RESP:
				{
				if (hdr->cmd_size != sizeof(S_CMD_GET_STATE_RESP_T)) break;
				S_CMD_GET_STATE_RESP_T *in_req = (S_CMD_GET_STATE_RESP_T*)rxbufer;

				ERESPSTATE rx_state = (ERESPSTATE)in_req->resp_state;	
				get_state_resp_cb (in_req->hdr.src_id, &in_req->state, rx_state);
				break;
				}
			}
		}
	}
}



void TRFMASTER::RF_txend_cb (bool f_ok)
{
}

 