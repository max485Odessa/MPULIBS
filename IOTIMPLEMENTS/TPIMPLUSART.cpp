#include "TPIMPLUSART.H"


TUSARTCMDMASTER::TUSARTCMDMASTER (TTXIF *t, local_rf_id_t sa)
{
tx_obj = t;
self_id = sa;
c_txbufer_size = sizeof(S_RFCMDS_SIZES_UNION_T) + sizeof(S_CHANHDR_T) + 32;
txbufer = new uint8_t[c_txbufer_size];
reqslots = new S_ACTIVE_TRANSACTION_SLOT_T[ECMDLAN_ENDENUM];
cur_trid = 0;
clear_actslots ();
}


void TUSARTCMDMASTER::clear_actslots ()
{
uint32_t ix = 0;
while (ix < ECMDLAN_ENDENUM)
    {
    reqslots[ix].f_active = false;
    ix++;
    }
}



TUSARTCMDMASTER::~TUSARTCMDMASTER ()
{
delete [] txbufer;
delete [] reqslots;
}



uint32_t TUSARTCMDMASTER::lenstr (void *s)
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



void TUSARTCMDMASTER::CopyMemorySDC (void *s, void *d, uint32_t sz)
{
char *src = (char*)s;
char *dst = (char*)d;
while (sz)
    {
    *dst++ = *src++;
    sz--;
    }
}



bool TUSARTCMDMASTER::find_protocol (void *s, uint32_t szsrc, uint32_t &find_ix, uint32_t &findsz)
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
                uint16_t clccrc = calculate_crc16cap (src, full_len);
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



uint16_t TUSARTCMDMASTER::calculate_crc16cap (uint8_t *src, uint32_t sz)
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





void TUSARTCMDMASTER::framesend (void *s, uint32_t sz, uint8_t prt, uint32_t tmot)
{
S_CHANHDR_T *capsl = (S_CHANHDR_T*)txbufer;
uint8_t *dst_payload = txbufer + sizeof(S_CHANHDR_T);
uint32_t full_size = sz + sizeof(S_CHANHDR_T);
CopyMemorySDC (s, dst_payload, sz);
capsl->preamble_a = C_PREAMBLE_P1_A;
capsl->preamble_b = C_PREAMBLE_P1_B;
capsl->chan = prt;

capsl->size = sz;
capsl->crc16 = 0;
capsl->crc16 = calculate_crc16cap (txbufer, full_size);
tx_obj->txif_out (txbufer, full_size);
}



void TUSARTCMDMASTER::get_param_req (local_rf_id_t dvid, uint16_t ix, uint32_t tmot)
{
	S_CMD_GET_PARAM_REQ_T reqframe;
	reqframe.hdr.cmd = ECMDLAN_GET_PARAM;
  reqframe.hdr.trid = encode_trid (ERFDTYPE_REQ, ERESPSTATE_OK, cur_trid++);
	reqframe.hdr.cmd_size = sizeof(S_CMD_GET_PARAM_REQ_T);
	reqframe.hdr.dst_id = dvid;
	reqframe.hdr.src_id = self_id;
	reqframe.ix = ix;
	reqframe.name.txt[0] = 0;
	framesend (&reqframe, sizeof(reqframe), C_CHANID_RF_BRIDGE, tmot);
}



void TUSARTCMDMASTER::set_param_req (local_rf_id_t dvid, uint16_t ix, S_RFPARAMVALUE_T prm, uint32_t tmot)
{
	S_CMD_SET_PARAM_REQ_T reqframe;
	reqframe.hdr.cmd = ECMDLAN_SET_PARAM;
    reqframe.hdr.trid = encode_trid (ERFDTYPE_REQ, ERESPSTATE_OK, cur_trid++);
	reqframe.hdr.cmd_size = sizeof(S_CMD_SET_PARAM_REQ_T);
	reqframe.hdr.dst_id = dvid;
	reqframe.hdr.src_id = self_id;
	reqframe.ix = ix;
	reqframe.name.txt[0] = 0;
	reqframe.param = prm;
	framesend (&reqframe, sizeof(reqframe), C_CHANID_RF_BRIDGE, tmot);
}



void TUSARTCMDMASTER::set_param_req (local_rf_id_t dvid, char *name, S_RFPARAMVALUE_T prm, uint32_t tmot)
{
	S_CMD_SET_PARAM_REQ_T reqframe;
	reqframe.hdr.cmd = ECMDLAN_SET_PARAM;
    reqframe.hdr.trid = encode_trid (ERFDTYPE_REQ, ERESPSTATE_OK, cur_trid++);
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
	framesend (&reqframe, sizeof(reqframe), C_CHANID_RF_BRIDGE, tmot);
}



void TUSARTCMDMASTER::get_param_req (local_rf_id_t dvid, char *name, uint32_t tmot)
{
	S_CMD_GET_PARAM_REQ_T reqframe;
	reqframe.hdr.cmd = ECMDLAN_GET_PARAM;
    reqframe.hdr.trid = encode_trid (ERFDTYPE_REQ, ERESPSTATE_OK, cur_trid++);
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
	framesend (&reqframe, sizeof(reqframe), C_CHANID_RF_BRIDGE, tmot);
}



void TUSARTCMDMASTER::get_event_req (local_rf_id_t dvid, uint16_t ix, uint32_t tmot)
{
	S_CMD_GET_EVENT_REQ_T reqframe;
	reqframe.hdr.cmd = ECMDLAN_GET_EVENT;
    reqframe.hdr.trid = encode_trid (ERFDTYPE_REQ, ERESPSTATE_OK, cur_trid++);
	reqframe.hdr.cmd_size = sizeof(S_CMD_GET_EVENT_REQ_T);
	reqframe.hdr.dst_id = dvid;
	reqframe.hdr.src_id = self_id;
	reqframe.ix = ix;
	framesend (&reqframe, sizeof(reqframe), C_CHANID_RF_BRIDGE, tmot);
}



void TUSARTCMDMASTER::call_event_req (local_rf_id_t dvid, uint32_t event_code, uint32_t calltime, uint32_t tmot )
{
	S_CMD_CALL_EVENT_REQ_T reqframe;
	reqframe.hdr.cmd = ECMDLAN_CALL_EVENT;
    reqframe.hdr.trid = encode_trid (ERFDTYPE_REQ, ERESPSTATE_OK, cur_trid++);
	reqframe.hdr.cmd_size = sizeof(S_CMD_CALL_EVENT_REQ_T);
	reqframe.hdr.dst_id = dvid;
	reqframe.hdr.src_id = self_id;
	reqframe.event_code = event_code;
	reqframe.time_event_work = calltime;
	framesend (&reqframe, sizeof(reqframe), C_CHANID_RF_BRIDGE, tmot);
}




void TUSARTCMDMASTER::Task ()
{
}




bool TUSARTCMDMASTER::check_rf_frame (S_RFHEADER_T *rf_frm, uint32_t srcsz)
{
bool rv = false;
if (rf_frm) {
    if (srcsz && rf_frm->cmd_size <= srcsz) rv = calculate_crc8rf ((uint8_t*)rf_frm, rf_frm->cmd_size);
    }
return rv;
}




S_RFHEADER_T *TUSARTCMDMASTER::capsule_open (void *s, uint32_t szsrc, uint32_t &capsz)
{
S_RFHEADER_T *rv = 0;
if (szsrc >= sizeof(S_CHANHDR_T) && szsrc <= (sizeof(S_RFCMDS_SIZES_UNION_T) + sizeof(S_CHANHDR_T)))
    {
    S_CHANHDR_T *caps = (S_CHANHDR_T*)s;
    uint8_t *src = (uint8_t*)s;
    src += sizeof(S_CHANHDR_T);
    S_RFHEADER_T *hdr = (S_RFHEADER_T*)src;
    if (caps->size < szsrc)
        {
        uint32_t incap_size = szsrc - caps->size;
        if (incap_size == hdr->cmd_size)
            {
            capsz = incap_size;
            rv = hdr;
            }
        }
    }
return rv;
}




bool TUSARTCMDMASTER::parse (void *s, uint32_t szsrc)
{
bool rv = false;
uint32_t frsz = 0;
S_RFHEADER_T *rf_frame = capsule_open (s, szsrc, frsz);
if (rf_frame)
	{
    if (check_rf_frame (rf_frame, frsz))
        {
        if (rf_frame->dst_id == self_id)
            {
            ERFDTYPE tp;
            ERESPSTATE rslt;
            uint8_t tridn;
            decode_trid (rf_frame->trid,  &tp, &rslt, &tridn);

            if (tp == ERFDTYPE_REQ) return false;


            switch (rf_frame->cmd)
                {
                case ECMDLAN_GET_PARAM:
                    {
                    if (rf_frame->cmd_size != sizeof(S_CMD_GET_PARAM_RESP_T)) break;
                    S_CMD_GET_PARAM_RESP_T *in_req = (S_CMD_GET_PARAM_RESP_T*)rf_frame;

                    ERESPSTATE rx_state = (ERESPSTATE)in_req->resp_state;
                    get_param_resp_cb (in_req->hdr.src_id, in_req->ix, const_cast<S_PRMF_CAPTION_T*>(&in_req->name), in_req->param, rx_state);
                    break;
                    }
                case ECMDLAN_SET_PARAM:
                    {
                    if (rf_frame->cmd_size != sizeof(S_CMD_SET_PARAM_RESP_T)) break;
                    S_CMD_SET_PARAM_RESP_T *in_req = (S_CMD_SET_PARAM_RESP_T*)rf_frame;

                    ERESPSTATE rx_state = (ERESPSTATE)in_req->resp_state;
                    set_param_resp_cb (in_req->hdr.src_id, in_req->ix, const_cast<S_PRMF_CAPTION_T*>(&in_req->name), in_req->param, rx_state);

                    break;
                    }
                case ECMDLAN_GET_EVENT:
                    {
                    if (rf_frame->cmd_size != sizeof(S_CMD_GET_EVENT_RESP_T)) break;
                    S_CMD_GET_EVENT_RESP_T *in_req = (S_CMD_GET_EVENT_RESP_T*)rf_frame;

                    ERESPSTATE rx_state = (ERESPSTATE)in_req->resp_state;
                    get_event_resp_cb (in_req->hdr.src_id, in_req->ix, &in_req->event, rx_state);
                    break;
                    }
                case ECMDLAN_CALL_EVENT:
                    {
                    if (rf_frame->cmd_size != sizeof(S_CMD_CALL_EVENT_RESP_T)) break;
                    S_CMD_CALL_EVENT_RESP_T *in_req = (S_CMD_CALL_EVENT_RESP_T*)rf_frame;

                    ERESPSTATE rx_state = (ERESPSTATE)in_req->resp_state;
                    call_event_resp_cb (in_req->hdr.src_id, in_req->event_code, rx_state);
                    break;
                    }
                case ECMDLAN_GET_STATE:
                    {
                    if (rf_frame->cmd_size != sizeof(S_CMD_GET_STATE_RESP_T)) break;
                    S_CMD_GET_STATE_RESP_T *in_req = (S_CMD_GET_STATE_RESP_T*)rf_frame;

                    ERESPSTATE rx_state = (ERESPSTATE)in_req->resp_state;
                    get_state_resp_cb (in_req->hdr.src_id, &in_req->state, rx_state);
                    break;
                    }
                }
            }
        }
	}
return rv;
}



void TUSARTCMDMASTER::get_param_resp_cb (local_rf_id_t svid, uint16_t ix, const S_PRMF_CAPTION_T *name, S_RFPARAMVALUE_T &src, ERESPSTATE rx_state)
{
}



void TUSARTCMDMASTER::set_param_resp_cb (local_rf_id_t svid, uint16_t ix, const S_PRMF_CAPTION_T *name, S_RFPARAMVALUE_T &src, ERESPSTATE rx_state)
{
}



void TUSARTCMDMASTER::get_state_resp_cb (local_rf_id_t svid, S_DEVSTATE_T *src, ERESPSTATE rx_state)
{
}



void TUSARTCMDMASTER::get_event_resp_cb (local_rf_id_t svid, uint16_t ix, S_EVENT_ITEM_T *evnt, ERESPSTATE rx_state)
{
}



void TUSARTCMDMASTER::call_event_resp_cb (local_rf_id_t svid, uint32_t event_code, ERESPSTATE rx_state)
{
}



TUSARTCMDSLAVE::TUSARTCMDSLAVE (TTXIF *t, local_rf_id_t sa)
{
tx_obj = t;
self_id = sa;
c_txbufer_size = sizeof(S_RFCMDS_SIZES_UNION_T) + sizeof(S_CHANHDR_T) + 32;
txbufer = new uint8_t[c_txbufer_size];
reqslots = new S_ACTIVE_TRANSACTION_SLOT_T[ECMDLAN_ENDENUM];
cur_trid = 0;
clear_actslots ();
}



uint32_t TUSARTCMDSLAVE::lenstr_sz (void *s, uint32_t sz)
{
uint32_t rv = 0;
char *src = (char*)s;
while (sz)
    {
    if (*src == 0) break;
    src++;
    rv++;
		sz--;
    }
return rv;
}



bool TUSARTCMDSLAVE::check_rf_frame (S_RFHEADER_T *rf_frm, uint32_t srcsz)
{
bool rv = false;
if (rf_frm) {
    if (srcsz && rf_frm->cmd_size <= srcsz) rv = calculate_crc8rf ((uint8_t*)rf_frm, rf_frm->cmd_size);
    }
return rv;
}



S_RFHEADER_T *TUSARTCMDSLAVE::capsule_open (void *s, uint32_t szsrc, uint32_t &capsz)
{
S_RFHEADER_T *rv = 0;
if (szsrc >= sizeof(S_CHANHDR_T) && szsrc <= (sizeof(S_RFCMDS_SIZES_UNION_T) + sizeof(S_CHANHDR_T)))
    {
    S_CHANHDR_T *caps = (S_CHANHDR_T*)s;
    uint8_t *src = (uint8_t*)s;
    src += sizeof(S_CHANHDR_T);
    S_RFHEADER_T *hdr = (S_RFHEADER_T*)src;
    if (caps->size < szsrc)
        {
        uint32_t incap_size = szsrc - caps->size;
        if (incap_size == hdr->cmd_size)
            {
            capsz = incap_size;
            rv = hdr;
            }
        }
    }
return rv;
}



void TUSARTCMDSLAVE::clear_actslots ()
{
uint32_t ix = 0;
while (ix < ECMDLAN_ENDENUM)
    {
    reqslots[ix].f_active = false;
    ix++;
    }
}



uint16_t TUSARTCMDSLAVE::calculate_crc16cap (uint8_t *src, uint32_t sz)
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




bool TUSARTCMDSLAVE::find_protocol (void *s, uint32_t szsrc, uint32_t &find_ix, uint32_t &findsz)
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
                uint16_t clccrc = calculate_crc16cap (src, full_len);
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



bool TUSARTCMDSLAVE::parse (void *s, uint32_t szsrc)
{
bool rv = false;
uint32_t frsz = 0;
S_RFHEADER_T *rf_frame = capsule_open (s, szsrc, frsz);
if (rf_frame)
	{
    if (check_rf_frame (rf_frame, frsz))
        {
        if (rf_frame->dst_id == self_id)
            {
            ERFDTYPE tp;
            decode_trid (rf_frame->trid,  &tp, 0, &last_rx_trid);
            if (tp == ERFDTYPE_RESP) return false;

            switch (rf_frame->cmd)
                {
                case ECMDLAN_GET_PARAM:
                    {
                    if (rf_frame->cmd_size != sizeof(S_CMD_GET_PARAM_RESP_T)) break;
                    S_CMD_GET_PARAM_RESP_T *in_req = (S_CMD_GET_PARAM_RESP_T*)rf_frame;
											
                    S_RFPARAMVALUE_T prm;
                    ERESPSTATE rslt = get_param_req_cb (in_req->hdr.src_id, in_req->ix, const_cast<S_PRMF_CAPTION_T*>(&in_req->name), prm);
										send_get_param_resp (in_req->hdr.dst_id, in_req->ix, const_cast<S_PRMF_CAPTION_T*>(&in_req->name), prm, rslt);
                    break;
                    }
                case ECMDLAN_SET_PARAM:
                    {
                    if (rf_frame->cmd_size != sizeof(S_CMD_SET_PARAM_RESP_T)) break;
                    S_CMD_SET_PARAM_RESP_T *in_req = (S_CMD_SET_PARAM_RESP_T*)rf_frame;
											
										S_RFPARAMVALUE_T prm;
                    ERESPSTATE rslt = set_param_req_cb (in_req->hdr.src_id, in_req->ix, const_cast<S_PRMF_CAPTION_T*>(&in_req->name), in_req->param);
										send_set_param_resp (in_req->hdr.dst_id, in_req->ix, const_cast<S_PRMF_CAPTION_T*>(&in_req->name), prm, rslt);
                    break;
                    }
                case ECMDLAN_GET_EVENT:
                    {
                    if (rf_frame->cmd_size != sizeof(S_CMD_GET_EVENT_RESP_T)) break;
                    S_CMD_GET_EVENT_RESP_T *in_req = (S_CMD_GET_EVENT_RESP_T*)rf_frame;

										S_EVENT_ITEM_T evnt;
                    ERESPSTATE rslt = get_event_req_cb (in_req->hdr.src_id, in_req->ix, evnt);
										send_get_event_resp (in_req->hdr.dst_id, in_req->ix, evnt, rslt);
                    break;
                    }
                case ECMDLAN_CALL_EVENT:
                    {
                    if (rf_frame->cmd_size != sizeof(S_CMD_CALL_EVENT_RESP_T)) break;
                    S_CMD_CALL_EVENT_RESP_T *in_req = (S_CMD_CALL_EVENT_RESP_T*)rf_frame;
											
                    ERESPSTATE rslt = call_event_req_cb (in_req->hdr.src_id, in_req->event_code);
										send_call_event_resp (in_req->hdr.dst_id, in_req->event_code, rslt);
                    break;
                    }
                case ECMDLAN_GET_STATE:
                    {
                    if (rf_frame->cmd_size != sizeof(S_CMD_GET_STATE_RESP_T)) break;
                    S_CMD_GET_STATE_RESP_T *in_req = (S_CMD_GET_STATE_RESP_T*)rf_frame;

										S_DEVSTATE_T stt;
                    ERESPSTATE rslt = get_state_req_cb (in_req->hdr.src_id, stt);
										send_get_state_resp (in_req->hdr.dst_id, stt, rslt);
                    break;
                    }
                }
            }
        }
	}
return rv;
}



void TUSARTCMDSLAVE::framesend (void *s, uint32_t sz, uint8_t prt)
{
S_CHANHDR_T *capsl = (S_CHANHDR_T*)txbufer;
uint8_t *dst_payload = txbufer + sizeof(S_CHANHDR_T);
uint32_t full_size = sz + sizeof(S_CHANHDR_T);
CopyMemorySDC (s, dst_payload, sz);
capsl->preamble_a = C_PREAMBLE_P1_A;
capsl->preamble_b = C_PREAMBLE_P1_B;
capsl->chan = prt;
capsl->size = sz;
capsl->crc16 = 0;
capsl->crc16 = calculate_crc16cap (txbufer, full_size);
tx_obj->txif_out (txbufer, full_size);
}



void TUSARTCMDSLAVE::send_get_param_resp (local_rf_id_t dvid, uint16_t ix, const S_PRMF_CAPTION_T *name, const S_RFPARAMVALUE_T &src, ERESPSTATE rx_state)
{
	S_CMD_GET_PARAM_RESP_T respframe;
	respframe.hdr.cmd = ECMDLAN_GET_PARAM;
  respframe.hdr.trid = encode_trid (ERFDTYPE_RESP, rx_state, last_rx_trid);
	respframe.hdr.cmd_size = sizeof(S_CMD_GET_PARAM_RESP_T);
	respframe.hdr.dst_id = dvid;
	respframe.hdr.src_id = self_id;
	respframe.ix = ix;
	respframe.name.txt[0] = 0;
	framesend (&respframe, sizeof(respframe), C_CHANID_DONGLE);
}



void TUSARTCMDSLAVE::send_set_param_resp (local_rf_id_t dvid, uint16_t ix, const S_PRMF_CAPTION_T *name, const S_RFPARAMVALUE_T &src, ERESPSTATE rx_state)
{
	S_CMD_SET_PARAM_RESP_T respframe;
	respframe.hdr.cmd = ECMDLAN_SET_PARAM;
  respframe.hdr.trid = encode_trid (ERFDTYPE_RESP, rx_state, last_rx_trid);
	respframe.hdr.cmd_size = sizeof(S_CMD_SET_PARAM_RESP_T);
	respframe.hdr.dst_id = dvid;
	respframe.hdr.src_id = self_id;
	respframe.ix = ix;
	if (name)
		{
		uint32_t lnstr = lenstr_sz ((void*)name->txt, sizeof(name->txt));
		CopyMemorySDC ((void*)name->txt, respframe.name.txt, lnstr);
		}
	else
		{
		respframe.name.txt[0] = 0;
		}
	framesend (&respframe, sizeof(respframe), C_CHANID_DONGLE);
}



void TUSARTCMDSLAVE::send_get_state_resp (local_rf_id_t dvid, const S_DEVSTATE_T &src, ERESPSTATE rx_state)
{
	S_CMD_GET_STATE_RESP_T respframe;
	respframe.hdr.cmd = ECMDLAN_GET_STATE;
  respframe.hdr.trid = encode_trid (ERFDTYPE_RESP, rx_state, last_rx_trid);
	respframe.hdr.cmd_size = sizeof(S_CMD_GET_STATE_RESP_T);
	respframe.hdr.dst_id = dvid;
	respframe.hdr.src_id = self_id;
	respframe.state = src;
	framesend (&respframe, sizeof(respframe), C_CHANID_DONGLE);
}



void TUSARTCMDSLAVE::send_get_event_resp (local_rf_id_t dvid, uint16_t ix, const S_EVENT_ITEM_T &evnt, ERESPSTATE rx_state)
{
	S_CMD_GET_EVENT_RESP_T respframe;
	respframe.hdr.cmd = ECMDLAN_GET_EVENT;
  respframe.hdr.trid = encode_trid (ERFDTYPE_RESP, rx_state, last_rx_trid);
	respframe.hdr.cmd_size = sizeof(S_CMD_GET_EVENT_RESP_T);
	respframe.hdr.dst_id = dvid;
	respframe.hdr.src_id = self_id;
	respframe.event = evnt;
	framesend (&respframe, sizeof(respframe), C_CHANID_DONGLE);
}



void TUSARTCMDSLAVE::send_call_event_resp (local_rf_id_t dvid, uint32_t event_code, ERESPSTATE rx_state)
{
	S_CMD_CALL_EVENT_RESP_T respframe;
	respframe.hdr.cmd = ECMDLAN_CALL_EVENT;
  respframe.hdr.trid = encode_trid (ERFDTYPE_RESP, rx_state, last_rx_trid);
	respframe.hdr.cmd_size = sizeof(S_CMD_CALL_EVENT_RESP_T);
	respframe.hdr.dst_id = dvid;
	respframe.hdr.src_id = self_id;
	respframe.event_code = event_code;
	framesend (&respframe, sizeof(respframe), C_CHANID_DONGLE);
}



void TUSARTCMDSLAVE::CopyMemorySDC (void *s, void *d, uint32_t sz)
{
char *src = (char*)s;
char *dst = (char*)d;
while (sz)
    {
    *dst++ = *src++;
    sz--;
    }
}



/*
ERESPSTATE TUSARTCMDSLAVE::get_param_req_cb (local_rf_id_t svid, uint16_t ix, const S_PRMF_CAPTION_T *name, S_RFPARAMVALUE_T &dst)
{
	ERESPSTATE rv = ERESPSTATE_ERROR;
	return rv;
}



ERESPSTATE TUSARTCMDSLAVE::set_param_req_cb (local_rf_id_t svid, uint16_t ix, const S_PRMF_CAPTION_T *name, S_RFPARAMVALUE_T &prm)
{
	ERESPSTATE rv = ERESPSTATE_ERROR;
	return rv;
}



ERESPSTATE TUSARTCMDSLAVE::get_state_req_cb (local_rf_id_t svid, S_DEVSTATE_T &prm)
{
	ERESPSTATE rv = ERESPSTATE_ERROR;
	return rv;
}



ERESPSTATE TUSARTCMDSLAVE::get_event_req_cb (local_rf_id_t svid, uint16_t ix, S_EVENT_ITEM_T &evnt)
{
	ERESPSTATE rv = ERESPSTATE_ERROR;
	return rv;
}



ERESPSTATE TUSARTCMDSLAVE::call_event_req_cb (local_rf_id_t svid, uint32_t event_code)
{
	ERESPSTATE rv = ERESPSTATE_ERROR;
	return rv;
}
*/



void TUSARTCMDSLAVE::Task ()
{
}



