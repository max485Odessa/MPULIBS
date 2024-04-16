#include "TRFMASTER.H"



TRFMASTER::TRFMASTER (ISPI *s, const uint16_t szz, const S_GPIOPIN *p) : TRFM69 (s, szz, p)
{
	acksectortab = new uint8_t[c_datapayload_size];
	rxsector = new uint8_t[c_datapayload_size];
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




void TRFMASTER::send_sector (void *scr, uint8_t sz, ERFFMARK m, ERFTACK ackt, uint8_t sect)
{
}




void TRFMASTER::Task ()
{
	if (is_tx_processed ()) return;
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
				cur_txtimeout.set (100);
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
			if (cur_txtimeout.get ())
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
				cur_txtimeout.set (100);
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
			if (cur_txtimeout.get ())
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



void TRFMASTER::tx_data (void *src, uint32_t sz, IFEVENT *tev)
{
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
