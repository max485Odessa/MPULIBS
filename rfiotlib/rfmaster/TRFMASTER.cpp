#include "TRFMASTER.H"
#include "rfcmddefine.h"


TRFMASTER::TRFMASTER (TRADIOIF *r, local_rf_id_t slf) : c_datapayload_size (r->frame_size() - sizeof(S_RFMARKTAG_T))
{
	self_id = slf;
	radio = r;
	acksectortab = new uint8_t[c_datapayload_size];	// only payload size
	rxsector = new uint8_t[radio->frame_size()];	// full size
}



void TRFMASTER::clear_rx_src ()
{
}



bool TRFMASTER::check_selfid (S_RFMARKTAG_T *f)
{
	bool rv = false;
	return rv;
}




bool TRFMASTER::check_crc (S_RFMARKTAG_T *f)
{
	bool rv = false;
	return rv;
}



bool TRFMASTER::check_rx_rf_sector ()
{
	bool rv = false;
	return rv;
}



bool TRFMASTER::read_rx_rf_sector (void *src)
{
	bool rv = false;
	return rv;
}



bool TRFMASTER::is_tx_processed ()
{
	bool rv = false;
	return rv;
}





void TRFMASTER::set_harderror_bit (bool v)
{
	f_tx_hard_error = v;
}





void TRFMASTER::set_device_stat_bit (ERFSDEVICE dv)
{
tx_device_status = dv;
}



uint8_t TRFMASTER::tagbytegen (bool f_hrder, bool f_req, ERFFMARK m, ERFTACK ak, ERFSDEVICE dvs)
{
uint8_t rv = m;
rv <<= 5; rv &= 0x60;	// m
rv |= (((uint8_t)ak) << 3) & 0x18;	// ak
rv |= ((uint8_t)dvs) & 0x03;	// dvs
if (f_hrder) rv |= 2;
if (f_req) rv |= 128;
return rv;
}





void TRFMASTER::rx_task ()
{
	if (radio->is_rx ())
		{
		if (radio->rx ((S_RFMARKTAG_T*)rxsector))
			{
			
			}
		}
	
}




// S_RFMARKTAG_T *src, uint16_t sz, ERFMODE endsw_to

void TRFMASTER::send_sector (void *scr, uint8_t sz, ERFFMARK m, ERFTACK ackt, uint8_t sect)
{
	if (scr && sz && sz <= (radio->frame_size() - sizeof(S_RFMARKTAG_T)))
		{
		S_RFMARKTAG_T *hdr = (S_RFMARKTAG_T*)scr;
		hdr->crc = 0;
		hdr->cur_sector = sect;
		hdr->dst_id = dest_id;
		hdr->local_size = sz;
		hdr->maxsectors = cur_maxtxsectors;
		hdr->src_id = self_id;
		hdr->tag = tagbytegen (f_tx_hard_error, f_tx_req_type, m, ackt, tx_device_status);
		hdr->crc = calculate_crc8rf (hdr, sz + sizeof(S_RFMARKTAG_T));
		ERFMODE rfswtomode = ERFMODE_RX;
		if (m == ERFFMARK_MIDLE) rfswtomode = ERFMODE_TX;
		radio->tx (hdr, rfswtomode);
		need_ack = ackt;		
		}

}





void TRFMASTER::tx_task ()
{
	if (is_tx_processed ()) return;
	if (need_ack != ERFTACK_NONE) {};
	switch (sw_tx)
		{
		case ESWTXA_TX_FIST:
			{
			if (repeate_cnt)
				{
				cur_txsize = tx_byte_cnt;
				if (cur_txsize > c_datapayload_size) cur_txsize = c_datapayload_size;
				cur_txmark = ERFFMARK_FIST;
				cur_txsect = 0;
				send_sector (tx_src, cur_txsize, cur_txmark, ERFTACK_ACK_A, cur_txsect);
				clear_rx_src ();
				cur_swtimeout.set (100);
				sw_tx = ESWTXA_RESP_FIST;
				}
			else
				{
				// error
				}
			break;
			}
		case ESWTXA_RESP_FIST:
			{
			if (cur_swtimeout.get ())
				{
				if (read_rx_rf_sector (rxsector))
					{
					S_RFMARKTAG_T *rxhdr = (S_RFMARKTAG_T*)rxsector;
					if (check_crc (rxhdr) && check_selfid (rxhdr))
						{
						if (rxhdr->cur_sector == cur_txsect)
							{
							tx_byte_cnt -= cur_txsize;
							if (tx_byte_cnt)
								{
								tx_src += cur_txsize;
								cur_txsect++;
								if (tx_midl_sector_count)
									{
									sw_tx = ESWTXA_TX_MIDLE;
									}
								else
									{
									sw_tx = ESWTXA_TX_LAST;
									repeate_cnt = c_set_repeate;
									}
								}
							else
								{
								// tx end
								}
							}
						}
					}
				}
			else
				{
				// is rx - timeout
				if (repeate_cnt)
					{
					repeate_cnt--;
					sw_tx = ESWTXA_TX_FIST;
					}
				else
					{
					// end repeate - error
					}
				}
			break;
			}
		case ESWTXA_TX_MIDLE:
			{
			if (tx_midl_sector_count)
				{
				cur_txsize = tx_byte_cnt;
				if (cur_txsize > c_datapayload_size) cur_txsize = c_datapayload_size;
				send_sector (tx_src, cur_txsize, ERFFMARK_MIDLE, ERFTACK_NONE, cur_txsect);
				cur_txsect++;
				tx_src += cur_txsize;
				tx_byte_cnt -= cur_txsize;
				tx_midl_sector_count--;
				}
			else
				{
				sw_tx = ESWTXA_TX_LAST;
				repeate_cnt = c_set_repeate;
				}
			break;
			}
		case ESWTXA_TX_LAST:
			{
			if (repeate_cnt)
				{
				cur_txsize = tx_byte_cnt;
				if (cur_txsize > c_datapayload_size) cur_txsize = c_datapayload_size;
				cur_txmark = ERFFMARK_LAST;
				send_sector (tx_src, cur_txsize, cur_txmark, ERFTACK_ACK_B, cur_txsect);
				clear_rx_src ();
				cur_swtimeout.set (100);
				sw_tx = ESWTXA_RESP_LAST;
				}
			else
				{
				// error
				}
			break;
			}
		case ESWTXA_RESP_LAST:
			{
			if (cur_swtimeout.get ())
				{
				if (read_rx_rf_sector (rxsector))
					{
					S_RFMARKTAG_T *rxhdr = (S_RFMARKTAG_T*)rxsector;
					if (check_crc (rxhdr)  && check_selfid (rxhdr)) sw_tx = ESWTXA_COMPLETE;
					}
				}
			else
				{
				// is rx - timeout
				if (repeate_cnt)
					{
					repeate_cnt--;
					sw_tx = ESWTXA_TX_LAST;
					}
				else
					{
					// end repeate - error
					}
				}
			break;
			}
		case ESWTXA_TX_FIST_LAST:
		case ESWTXA_RESP_FIST_LAST:
			{
			break;
			}
		case ESWTXA_COMPLETE:
			{
			break;
			}
		default: break;
		}
}



void TRFMASTER::Task ()
{
tx_task ();
rx_task ();
}



void TRFMASTER::tx_request_data (local_rf_id_t dstid, void *src, uint32_t sz)
{
	dest_id = dstid;
	f_tx_req_type = true;
	if (sz)
		{
		uint16_t secsz = sz / c_datapayload_size;
		secsz += ((sz % c_datapayload_size)?1:0);
		cur_maxtxsectors = secsz;
		tx_byte_cnt = sz;
		tx_src = (uint8_t*)src;
		repeate_cnt = c_set_repeate;
		switch (secsz)
			{
			case 1:
				{
				sw_tx = ESWTXA_TX_FIST_LAST;
				break;
				}
			default:
				{
				sw_tx = ESWTXA_TX_FIST;
				tx_midl_sector_count = secsz - 2;
				break;
				}
			}
		}
		
}

