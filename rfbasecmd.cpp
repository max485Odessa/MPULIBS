#include "rfbasecmd.h"



TRFBASECMD::TRFBASECMD (IFCRFTX *objc, uint16_t abcnt_max, uint32_t rxalcsz) : c_abon_max (abcnt_max), c_rxbuf_size (rxalcsz)
{
rxbufer = new uint8_t[c_rxbuf_size];
abitm = new S_ABONENT_ITEM_T[c_abon_max];
tx_timeout = 0;
SYSBIOS::ADD_TIMER_SYS (&tx_timeout);
txobj = objc;
abon_list_amount = 0;
f_tx_state = false;
}






/*
void TRFBASECMD::RF_recv_cb (uint8_t *data, uint16_t sz, uint16_t rssi)
{
if (sz <= c_rxbuf_size)
	{
	CopySDC_Data (data, rxbufer, sz);
	S_RFHEADER_T *hdr = (S_RFHEADER_T*)rxbufer;
	if (hdr->dst_id	== self_id)
		{
		switch (hdr->cmd)
			{
			case ECMDLAN_GET_PARAM_REQ:
				{
				if (hdr->cmd_size != sizeof(S_CMD_GET_PARAM_REQ_T)) break;
				S_CMD_GET_PARAM_REQ_T *in_req = (S_CMD_GET_PARAM_REQ_T*)rxbufer;

				break;
				}
			case ECMDLAN_SET_PARAM_REQ:
				{
				break;
				}
			case ECMDLAN_GET_EVENT_REQ:
				{
				break;
				}
			case ECMDLAN_CALL_EVENT_REQ:
				{
				break;
				}
			case ECMDLAN_GET_STATE_REQ:
				{
				break;
				}
			}
		}
	}
}



void TRFBASECMD::RF_txend_cb (bool f_ok)
{
	tx_timeout = 0;
	f_tx_state = false;
}
*/




long TRFBASECMD::abnt_find (const S_RFSERIAL_T *sn)
{
	long rv = -1;

		long ix = 0;
		while (ix < abon_list_amount)
			{
			if (str_compare ((char*)sn, (char*)abitm[ix].serial.n, sizeof(S_RFSERIAL_T)))
				{
				rv = ix;
				break;
				}
			ix++;
			}

	return rv;
}



long TRFBASECMD::abnt_find (local_rf_id_t snl)
{
	long rv = -1;

		long ix = 0;
		while (ix < abon_list_amount)
			{
			if (snl == abitm[ix].id)
				{
				rv = ix;
				break;
				}
			ix++;
			}

	return rv;
}



bool TRFBASECMD::add_abonent (S_ABONENT_ITEM_T *abnt)
{
	bool rv = false;
	if (abnt)
		{
		long fndix = abnt_find (&abnt->serial);
		if (fndix < 0)
			{
			if (abon_list_amount < c_abon_max)
				{
				abitm[abon_list_amount] = *abnt;
				abon_list_amount++;
				rv = true;
				}
			}
		}
	return rv;
}



void TRFBASECMD::set_selfid (local_rf_id_t sid)
{
	self_id = sid;
}




void TRFBASECMD::clear_abonent_list ()
{
	abon_list_amount = 0;
}



void TRFBASECMD::rawsend (void *src, uint16_t sz, uint16_t timout)
{
	txobj->send ((uint8_t*)src, sz);
	f_tx_state = true;
	tx_timeout = timout;
}



bool TRFBASECMD::is_free ()
{
	return !f_tx_state;
}




void TRFMASTER::get_param_req (local_rf_id_t dvid, uint16_t ix, uint32_t tmot)
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



void TRFMASTER::set_param_req (local_rf_id_t dvid, uint16_t ix, S_RFPARAMVALUE_T prm, uint32_t tmot)
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



void TRFMASTER::set_param_req (local_rf_id_t dvid, char *name, S_RFPARAMVALUE_T prm, uint32_t tmot)
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
		CopySDC_Data (name, reqframe.name.txt, lnstr);
		}
	else
		{
		reqframe.name.txt[0] = 0;
		}
	rawsend (&reqframe, sizeof(reqframe), tmot);
}



void TRFMASTER::get_param_req (local_rf_id_t dvid, char *name, uint32_t tmot)
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
		CopySDC_Data (name, reqframe.name.txt, lnstr);
		}
	else
		{
		reqframe.name.txt[0] = 0;
		}
	rawsend (&reqframe, sizeof(reqframe), tmot);
}



void TRFMASTER::get_event_req (local_rf_id_t dvid, uint16_t ix, uint32_t tmot)
{
	S_CMD_GET_EVENT_REQ_T reqframe;
	reqframe.hdr.cmd = ECMDLAN_GET_EVENT_REQ;
	reqframe.hdr.cmd_size = sizeof(S_CMD_GET_EVENT_REQ_T);
	reqframe.hdr.dst_id = dvid;
	reqframe.hdr.src_id = self_id;
	reqframe.ix = ix;
	rawsend (&reqframe, sizeof(reqframe), tmot);
}



void TRFMASTER::call_event_req (local_rf_id_t dvid, uint32_t event_code, uint32_t calltime, uint32_t tmot )
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


void IFCRFTX::setrx_cb (IFCRFRX *obj)
{
	eventsobj_cb = obj;
}



TRFMASTER::TRFMASTER (IFCRFTX *objc, uint16_t abcnt, uint32_t rxalcsz): TRFBASECMD (objc, abcnt, rxalcsz)
{
	txobj->setrx_cb (this);
}



void TRFMASTER::Task ()
{
}



void TRFMASTER::RF_recv_cb (uint8_t *data, uint16_t sz, uint16_t rssi)
{
if (sz <= c_rxbuf_size)
	{
	CopySDC_Data (data, rxbufer, sz);
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



TRFSLAVE::TRFSLAVE (IFCRFTX *objc, uint16_t abcnt, uint32_t rxalcsz): TRFBASECMD (objc, abcnt, rxalcsz)
{
	txobj->setrx_cb (this);
}



void TRFSLAVE::Task ()
{
}



void TRFSLAVE::RF_recv_cb (uint8_t *data, uint16_t sz, uint16_t rssi)
{
if (sz <= c_rxbuf_size)
	{
	CopySDC_Data (data, rxbufer, sz);
	uint32_t tx_size = 0;
	S_RFHEADER_T *hdr = (S_RFHEADER_T*)rxbufer;
	if (hdr->dst_id	== self_id)
		{
		switch (hdr->cmd)
			{
			case ECMDLAN_GET_PARAM_REQ:
				{
				if (hdr->cmd_size != sizeof(S_CMD_GET_PARAM_REQ_T)) break;
				S_CMD_GET_PARAM_REQ_T *in_req = (S_CMD_GET_PARAM_REQ_T*)rxbufer;
				S_RFPARAMVALUE_T userparam;
				S_CMD_GET_PARAM_RESP_T *resp_frame = (S_CMD_GET_PARAM_RESP_T*)txbufer;
				resp_frame->hdr.cmd = ECMDLAN_GET_PARAM_RESP;
				resp_frame->hdr.cmd_size = sizeof(S_CMD_GET_PARAM_RESP_T);
				resp_frame->hdr.src_id = self_id;
				resp_frame->hdr.dst_id = in_req->hdr.src_id;
					
				if (user_get_param_req_cb (in_req->hdr.src_id, in_req->ix, const_cast<S_PARAM_CAPTION_T*>(&in_req->name), userparam))
					{
					resp_frame->resp_state = ERESPSTATE_OK;
					resp_frame->ix = in_req->ix;
					resp_frame->name = in_req->name;
					resp_frame->param = userparam;
					}
				else
					{
					resp_frame->resp_state = ERESPSTATE_ERROR;
					resp_frame->ix = in_req->ix;
					resp_frame->name = in_req->name;		
					}
				tx_size = resp_frame->hdr.cmd_size;
				break;
				}
			case ECMDLAN_SET_PARAM_REQ:
				{
				if (hdr->cmd_size != sizeof(S_CMD_SET_PARAM_REQ_T)) break;
				S_CMD_SET_PARAM_REQ_T *in_req = (S_CMD_SET_PARAM_REQ_T*)rxbufer;
				S_CMD_SET_PARAM_RESP_T *resp_frame = (S_CMD_SET_PARAM_RESP_T*)txbufer;
				resp_frame->hdr.cmd = ECMDLAN_SET_PARAM_RESP;
				resp_frame->hdr.cmd_size = sizeof(S_CMD_SET_PARAM_RESP_T);
				resp_frame->hdr.src_id = self_id;
				resp_frame->hdr.dst_id = in_req->hdr.src_id;

				if (user_set_param_req_cb (in_req->hdr.src_id, in_req->ix, const_cast<S_PARAM_CAPTION_T*>(&in_req->name), &resp_frame->param))
					{
					resp_frame->resp_state = ERESPSTATE_OK;
					resp_frame->ix = in_req->ix;
					resp_frame->name = in_req->name;
					resp_frame->param = resp_frame->param;
					}
				else
					{
					resp_frame->resp_state = ERESPSTATE_ERROR;
					resp_frame->ix = in_req->ix;
					resp_frame->name = in_req->name;		
					}
				tx_size = resp_frame->hdr.cmd_size;
				break;
				}
			case ECMDLAN_GET_EVENT_REQ:
				{
				if (hdr->cmd_size != sizeof(S_CMD_GET_EVENT_REQ_T)) break;
				S_CMD_GET_EVENT_REQ_T *in_req = (S_CMD_GET_EVENT_REQ_T*)rxbufer;
				S_CMD_GET_EVENT_RESP_T *resp_frame = (S_CMD_GET_EVENT_RESP_T*)txbufer;
				resp_frame->hdr.cmd = ECMDLAN_GET_EVENT_RESP;
				resp_frame->hdr.cmd_size = sizeof(S_CMD_GET_EVENT_RESP_T);
				resp_frame->hdr.src_id = self_id;
				resp_frame->hdr.dst_id = in_req->hdr.src_id;
				S_EVENT_ITEM_T event_user;
				resp_frame->ix = in_req->ix;
				if (user_get_event_req_cb (in_req->hdr.src_id, in_req->ix, &event_user))
					{
					resp_frame->resp_state = ERESPSTATE_OK;
					resp_frame->event = event_user;
					}
				else
					{
					resp_frame->resp_state = ERESPSTATE_ERROR;
					}
				tx_size = resp_frame->hdr.cmd_size;
				break;
				}
			case ECMDLAN_CALL_EVENT_REQ:
				{
				if (hdr->cmd_size != sizeof(S_CMD_CALL_EVENT_REQ_T)) break;
				S_CMD_CALL_EVENT_REQ_T *in_req = (S_CMD_CALL_EVENT_REQ_T*)rxbufer;
				S_CMD_CALL_EVENT_RESP_T *resp_frame = (S_CMD_CALL_EVENT_RESP_T*)txbufer;
				resp_frame->hdr.cmd = ECMDLAN_CALL_EVENT_RESP;
				resp_frame->hdr.cmd_size = sizeof(S_CMD_CALL_EVENT_RESP_T);
				resp_frame->hdr.src_id = self_id;
				resp_frame->hdr.dst_id = in_req->hdr.src_id;
				resp_frame->event_code = in_req->event_code;
				if (user_call_event_req_cb (in_req->hdr.src_id, in_req->event_code, in_req->time_event_work))
					{
					resp_frame->resp_state = ERESPSTATE_OK;
					}
				else
					{
					resp_frame->resp_state = ERESPSTATE_ERROR;
					}
				tx_size = resp_frame->hdr.cmd_size;
				break;
				}
			case ECMDLAN_GET_STATE_REQ:
				{
				if (hdr->cmd_size != sizeof(S_CMD_GET_STATE_REQ_T)) break;
				S_CMD_GET_STATE_REQ_T *in_req = (S_CMD_GET_STATE_REQ_T*)rxbufer;
				S_CMD_GET_STATE_RESP_T *resp_frame = (S_CMD_GET_STATE_RESP_T*)txbufer;
				resp_frame->hdr.cmd = ECMDLAN_GET_STATE_RESP;
				resp_frame->hdr.cmd_size = sizeof(S_CMD_GET_STATE_RESP_T);
				resp_frame->hdr.src_id = self_id;
				resp_frame->hdr.dst_id = in_req->hdr.src_id;
				S_DEVSTATE_T devst;
				if (user_get_state_req_cb (in_req->hdr.src_id, &devst))
					{
					resp_frame->resp_state = ERESPSTATE_OK;
					resp_frame->state = devst;
					}
				else
					{
					resp_frame->resp_state = ERESPSTATE_ERROR;
					}
				tx_size = resp_frame->hdr.cmd_size;
				break;
				}
			}
		}
	if (tx_size) txobj->send (txbufer, tx_size); // �������� ������
	}

}



void TRFSLAVE::RF_txend_cb (bool f_ok)
{
}


