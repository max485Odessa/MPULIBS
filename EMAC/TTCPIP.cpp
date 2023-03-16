#include "TTCPIP.h"

TTCPIP *TTCPIP::SinglObj = 0;
unsigned char TTCPIP::tx_buf[1024];

TSOCKCL::TSOCKCL ()
{
	low_sock = 0;
	recv_data = 0;
	macip = 0;
	tx_buf.lpRam = 0;
}



void TSOCKCL::Init (ETCPSTATE st, void *adr, unsigned long sz, TCP_SOCKET_T *lwsck, EMACIP *mcip, SLPB *tx_b)
{
cur_statte = st;
recv_data = adr;
sz_data = sz;
low_sock = lwsck;
macip = mcip;
if (tx_b) tx_buf = *tx_b;
}



ETCPSTATE TSOCKCL::state ()
{
	return cur_statte;
}



void TSOCKCL::send (void *lsrc, unsigned long sz)
{
	if (tx_buf.lpRam)
		{
		unsigned short sz = TTCPIP::create_tcp_data_frame (&tx_buf, low_sock, lsrc, sz);
		if (sz) macip->raw_send (tx_buf.lpRam, sz);
		}
}



void TSOCKCL::close ()
{
		unsigned short sz = TTCPIP::create_tcp_close_frame (&tx_buf, low_sock);
		if (sz) macip->raw_send (tx_buf.lpRam, sz);
}



unsigned short TSOCKCL::size ()
{
	return sz_data;
}



void *TSOCKCL::data ()
{
	return recv_data;
}



TTCPIP::TTCPIP ()
{
	SinglObj = this;
	ClearS ();
	timeout_timer = 0;
	SYSBIOS::ADD_TIMER_SYS (&timeout_timer);
	init_timers ();
	ClearRegSlots ();
	tx_ifc = 0;	
}



void TTCPIP::ClearS ()
{
	fillmem ((void*)&cur_sock[0], 0, sizeof(TCP_SOCKET_T)*C_MAXTCP_AMOUNT);		// очищаем все слоты
}



void TTCPIP::init_timers ()
{
	unsigned char inx = 0;
	while (inx < C_MAXTCP_AMOUNT)
		{
		SYSBIOS::ADD_TIMER_SYS ((utimer_t*)&cur_sock[inx].TimeOut);
		cur_sock[inx].sw = ETCPSW_CLOSING;
		inx++;
		}
}


void TTCPIP::Init (EMACIP *txfc)
{
	ClearS ();
	tx_ifc = txfc;
}



void TTCPIP::sock_timeout_task ()
{
// отслеживание рабочих сокетов и закрытия соединений по таймауту
unsigned char indx = 0;
while (indx < C_MAXTCP_AMOUNT)	
	{
	if (cur_sock[indx].sw != ETCPSW_CLOSING) {	
		if (!cur_sock[indx].TimeOut)
			{
			// закрываем сокет по таймауту
			cur_sock[indx].sw = ETCPSW_CLOSING;
			//cur_sock[indx].is_opened = false;
			}
		}
	indx++;
	}		
}
	


void TTCPIP::Task ()
{
sock_timeout_task ();
}



// находит свободный слот для сокета
long TTCPIP::FindSFree_indx ()
{
	long rv_indx = -1;
	long indx = 0;
	TCP_SOCKET_T *lsck;
	while (indx < C_MAXTCP_AMOUNT)
		{
		lsck = &cur_sock[indx];
		if (lsck->sw == ETCPSW_CLOSING)
			{
			rv_indx = indx;
			break;
			}
		indx++;
		}
	return rv_indx;
}



long TTCPIP::FindSToIp_indx (unsigned long ip_adr, unsigned short port)
{
	long rv_indx = -1;
	long indx = 0;
	TCP_SOCKET_T *lsck;
	while (indx < C_MAXTCP_AMOUNT)
		{
		lsck = &cur_sock[indx];
		if (lsck->sw != ETCPSW_CLOSING && lsck->src_port == port && lsck->macip_src.ip.IP_SOURCE == ip_adr)
			{
			rv_indx = indx;
			break;
			}
		indx++;
		}
	return rv_indx;
}



void TTCPIP::ClearS_indx (long indx)
{
	if (indx < C_MAXTCP_AMOUNT) {
		fillmem ((void*)&cur_sock[indx], 0, sizeof(TCP_SOCKET_T));
		}
}



void TTCPIP::ClearSToIp (unsigned long ip_adr, unsigned short port)
{
	long s_slot = FindSToIp_indx (ip_adr, port);
	if (s_slot >= 0) ClearS_indx (s_slot);
}



TCP_SOCKET_T *TTCPIP::GetSoketFromIndx (long inx)
{
	TCP_SOCKET_T *rv = 0;
	if (inx >=0 && inx < C_MAXTCP_AMOUNT) {
		rv = &cur_sock[inx];
		}
	return rv;
}



unsigned short TTCPIP::get_tcp_data_size (void *lmac)
{
unsigned short rv = 0;
if (lmac) {
	rv = EMACIP::get_ip_size (lmac) - get_tcp_header_size (lmac);
	}
return rv;	
}



// высчитывает расположения данных tcp сегмента ++
void *TTCPIP::get_tcp_data_adr (void *lmac)
{
void *lrv = 0;
if (lmac)
	{
	uint8_t *lpa = (uint8_t*)EMACIP::get_ip_raw_data (lmac, 0);
	if (lpa)
		{
		unsigned long tcthdr_size = get_tcp_header_size (lmac);
		if (tcthdr_size >= sizeof(S_TCP_T))
			{
			lpa += tcthdr_size;
			lrv = lpa;
			}
		}
	}
return lrv;	
}



unsigned short TTCPIP::update_tcp_crc (void *maclevel)
{
	unsigned short rv = 0;
	if (maclevel)
		{
		unsigned short szipdata = EMACIP::get_ip_size (maclevel);	
		S_TCP_T *tcpseg = get_tcp_field (maclevel);
		TCP_PSEUDO_HEADER_T pshdr;
		S_MACIP_t *lmacip = (S_MACIP_t*)maclevel;
		pshdr.ip_destination = lmacip->ip.IP_DESTINATION;		
		pshdr.ip_source = lmacip->ip.IP_SOURCE;
		pshdr.protocol = SWAPB(((unsigned short)6));
		pshdr.size = SWAPB(EMACIP::get_ip_size (maclevel));
		tcpseg->checksum = 0;	
		tcpseg->checksum = EMACIP::Calc_TCP_CRC (tcpseg, szipdata, &pshdr);
		rv = szipdata;
		}
	return rv;
}



static const unsigned char c_opt_tct[12] = {0x02, 0x04, 0x05, 0xb4, 0x01, 0x03, 0x03, 0x08, 0x01, 0x01, 0x04, 0x02};

void *TTCPIP::create_tcp_header (void *ldst, TCP_SOCKET_T *cursock, bool f_opt)
{
void *lrv = 0;
if (ldst && cursock)
	{
	S_TCP_T *tccp_frm = (S_TCP_T*)ldst;
	fillmem (ldst, 0, sizeof(S_TCP_T));
	tccp_frm->dst_port = cursock->src_port;
	tccp_frm->src_port = cursock->dst_port;
	tccp_frm->len_flags = (tccp_frm->len_flags & 0xFF0F) + (((sizeof(c_opt_tct) + sizeof(S_TCP_T)) / 4) << 4);
	tccp_frm->w_size = SWAPB(1024); //cursock->wind_size;
	lrv = (tccp_frm + 1);
	if (f_opt) lrv = CopyMemorySDC ((char*)c_opt_tct, (char*)lrv, sizeof(c_opt_tct));
	}
return lrv;	
}



// возвращает размер tcp заголовка (который может быть увеличен из-за присутствующих опций) ++
unsigned long TTCPIP::get_tcp_header_size (void *maclevel)
{
unsigned long rv = 0;
	if (maclevel)
		{
		S_TCP_T *tcp_fr = get_tcp_field (maclevel);
		if (tcp_fr)
			{
			rv = ((tcp_fr->len_flags >> 4) & 0x0F) * 4;
			}
		}
	return rv;
}


/*
S_TCP_MIT_T *TTCPIP::CreateOutTCPFrame (S_MACIP_t *lmacip_in, TCP_SOCKET_T *cursock, bool f_opt)
{
S_TCP_MIT_T *lrv = 0;
	if (lmacip_in && cursock)
		{
		lrv = (S_TCP_MIT_T*)tx_buf;
		void *ldst = EMACIP::CreateMACSave (tx_buf, lmacip_in->mac.SourceMAC, EMACIP::GetSelfMac());
		ldst = EMACIP::CreateIPSave (ldst, lmacip_in->ip.IP_SOURCE, EMACIP::GetSelf_IP(), 0, PROT_TCP);
		unsigned short idnt = SWAPB(lmacip_in->ip.IP_IDENT);
		idnt++;
		lrv->IP.IP_IDENT = 	SWAPB (idnt);
		ldst = create_tcp_header (ldst, cursock, f_opt);
		}
	return lrv;
}
*/



S_TCP_MIT_T *TTCPIP::create_half_tcp (void *l_dst, TCP_SOCKET_T *cursock, bool f_opt)
{
S_TCP_MIT_T *lrv = 0;
	if (cursock)
		{
		lrv = (S_TCP_MIT_T*)l_dst;		// tx_buf
		void *ldst = EMACIP::create_MAC_header (tx_buf, cursock->macip_src.mac.SourceMAC, EMACIP::GetSelfMac());
		ldst = EMACIP::create_IP_header (ldst, cursock->macip_src.ip.IP_SOURCE, EMACIP::GetSelf_IP(), 0, PROT_TCP);
		unsigned short idnt = SWAPB(cursock->macip_src.ip.IP_IDENT);
		idnt++;
		lrv->IP.IP_IDENT = 	SWAPB (idnt);
		ldst = create_tcp_header (ldst, cursock, f_opt);
		}
	return lrv;
}



// Создает новый сокет на базе информации входящего TCP сегмента (сохраняет параметры клиента открывшего TCP соединение)
TCP_SOCKET_T *TTCPIP::CreateSocket (S_MACIP_t *mac_in, S_TCP_T *tcp_in)		// TCP_FRAME_T *fr_in
{
	TCP_SOCKET_T *lrv = 0;
	long free_s = FindSFree_indx ();
	if (free_s >= 0 && mac_in && tcp_in)
		{
		TCP_SOCKET_T *s_fr = GetSoketFromIndx (free_s);
		if (s_fr)
			{
			s_fr->macip_src = *mac_in;
			s_fr->sw = ETCPSW_SYNRECEIVED;
			//s_fr->ip_src = mac_in->ip.IP_SOURCE;
			s_fr->dst_port = tcp_in->dst_port;
			s_fr->src_port = tcp_in->src_port;
			//CopyMemorySDC ((char*)mac_in->mac.SourceMAC, (char*)s_fr->mac_src, sizeof(s_fr->mac_src));
			s_fr->sn = SwapLong(tcp_in->sn);
			s_fr->sn_ack = s_fr->sn + 1000000;
			s_fr->seq_rx = 0;
			s_fr->seq_tx = 0;
			s_fr->TimeOut = C_BASE_TCP_TIMEOUT;		// сразу же устанавливаем таймаут простоя
			lrv = s_fr;
			}
		}
	return lrv;
}



unsigned char TTCPIP::GetTCPFlags (S_TCP_T *flg)
{
	unsigned char rv = 0;
	if (flg) {
		rv = ((flg->len_flags >> 8) & 0xFF);
		}
	return rv;
}



void TTCPIP::set_tcp_flag (S_TCP_T *flg, unsigned char flag)
{
	if (flg) {
		flg->len_flags |= (((unsigned short)flag) << 8);
		}
}



bool TTCPIP::CheckTCPFlags (S_TCP_T *flg, unsigned char mask)
{
	bool rv = false;
	unsigned char bitt_f = GetTCPFlags (flg);
	if (bitt_f & mask) rv = true;
	return rv;
}



void TTCPIP::ClearRegSlots ()
{
	fillmem ((void*)port_slots_rutines, 0, sizeof(port_slots_rutines));
}



CBPORTSLOT *TTCPIP::FindActiveRegSlot (unsigned short port_a)
{
CBPORTSLOT *rv = 0;
long inx = 0;
while (inx < C_MAXTCPPORT_SLOT_AMOUNT) {
	if (port_slots_rutines[inx].f_active) {
		if (port_slots_rutines[inx].port == port_a) {
			rv = &port_slots_rutines[inx];
			break;
			}
		}
	inx++;
	}
return rv;
}


	
CBPORTSLOT *TTCPIP::FindFreeRegSlot ()
{
CBPORTSLOT *rv = 0;
long inx = 0;
while (inx < C_MAXTCPPORT_SLOT_AMOUNT) {
	if (!port_slots_rutines[inx].f_active) {
			rv = &port_slots_rutines[inx];
			break;
			}
	inx++;
	}
return rv;
}



bool TTCPIP::RegisteredPort (void *Obj, CB_TCPPORT_RX cb_d_rx, unsigned short port_n)
{
	bool rv = false;
	CBPORTSLOT *slt = 0;
	slt = FindActiveRegSlot (port_n);
	if (!slt) slt = FindFreeRegSlot ();
	if (slt) 
		{
		slt->cb_obj = Obj;
		slt->cb_rx = cb_d_rx;
		slt->f_active = true;
		slt->port = port_n;
		rv = true;
		}
	return rv;
}



/*
CB_TCPPORT_RX TTCPIP::FindRegPort (unsigned short port_a)
{
CB_TCPPORT_RX rv = 0;
CBPORTSLOT *slt = FindActiveRegSlot (port_a);
if (slt) rv = slt->cb_rx;
return rv;
}
*/


/*
bool TTCPIP::GetTCPData (S_TCP_T *lrf, char **ldat, unsigned short &sz)
{
bool rv = false;
if (lrf && ldat) {
		char *lDst = (char*)lrf;
		lDst += sizeof(S_TCP_MIT_T);
		unsigned short ofs_dat = (lrf->len_flags >> 12) & 0x0F;
		ofs_dat *= 4;
		// !!! Сюда приплюсовать размер опций
		*ldat = lDst + ofs_dat;
		sz = sizeof(S_TCP_MIT_T) + ofs_dat;
	}
return rv;
}
*/



S_TCP_T *TTCPIP::get_tcp_field (void *lmac)
{
S_TCP_T *rv = 0;
if (lmac){
		rv = (S_TCP_T*)EMACIP::get_ip_raw_data (lmac, 0);
	}
return rv;	
}



bool TTCPIP::in (void *lInp, unsigned short sz)
{
	bool rv = false;
	if (lInp && sz)
		{
		S_TCP_T *tcpfield_in = get_tcp_field (lInp);	// косвенно проверяем целостность ip структуры
		if (!tcpfield_in) return rv;									// выходим - так как IP данные повреждены
		unsigned long tcphdr_size = get_tcp_header_size (lInp);
		if (tcphdr_size < sizeof(S_TCP_T)) return rv;
		
		unsigned long ipseg_size = EMACIP::get_ip_size (lInp);
		
			
		S_MACIP_t *macip_in = (S_MACIP_t*)lInp;
	
		unsigned long src_ip = macip_in->ip.IP_SOURCE;
		unsigned short src_port = tcpfield_in->src_port;
			
		long s_indx = FindSToIp_indx (src_ip, src_port);
		if (s_indx >= 0)
			{
			// сокет открыт, проверяем состояние соединения
			TCP_SOCKET_T *sock = GetSoketFromIndx (s_indx);
			if (sock)
				{
				if (CheckTCPFlags (tcpfield_in, C_TCP_FIN_BIT))
					{
					// отработка закрытия соединения в любом состоянии
					unsigned short sz = create_full_tcp (sock, 0, 0, C_TCP_FIN_BIT | C_TCP_ACK_BIT, false);
						
					if (sz) // tx_ifc && 
						{
						S_TCP_MIT_T *rf_out = (S_TCP_MIT_T*)tx_buf;
						rf_out->IP.IP_IDENT = macip_in->ip.IP_IDENT;
						rf_out->TCP.sn_ack = tcpfield_in->sn + 1;
						rf_out->TCP.sn = tcpfield_in->sn_ack + 100;
						
						sock->sn = rf_out->TCP.sn;
						sock->sn_ack = rf_out->TCP.sn_ack;
							
						send_tcp (sz);
						//tx_ifc->raw_send (tx_buf, sz);
							
						sock->TimeOut = C_BASE_TCP_TIMEOUT;
						sock->sw = ETCPSW_LASTACK;
						}
					}
				
				switch (sock->sw)
					{
					case ETCPSW_SYNRECEIVED:
						{
						// ждем ack
						if (CheckTCPFlags (tcpfield_in, C_TCP_ACK_BIT)) {
							sock->sw = ETCPSW_ESTABLISHED;
							sock->TimeOut = C_BASE_TCP_TIMEOUT;
							}
						break;
						}
					case ETCPSW_ESTABLISHED:
						{
						// режим передачи данных
						// тут происходит вызов rx call_back's зарегистрированнных портов
						// отправка tcp подтверждения
						unsigned short tcpdatasize = get_tcp_data_size (macip_in);	
							
						unsigned short rawtx_sz = create_full_tcp (sock, 0, 0, C_TCP_ACK_BIT, false);
						S_TCP_MIT_T *rf_out = (S_TCP_MIT_T*)tx_buf;	
						//S_TCP_MIT_T *rf_out = CreateOutTCPFrame (macip_in, sock, false);
						//set_tcp_flag (&rf_out->TCP, (C_TCP_ACK_BIT));
						sock->seq_tx += tcpdatasize;
						rf_out->TCP.sn_ack = SwapLong (sock->sn + sock->seq_tx);
						rf_out->TCP.sn = SwapLong (sock->sn_ack + sock->seq_rx);
						
						send_tcp (rawtx_sz);
						//Transmit_TCP (tx_buf, 0);
						// подготавливаем принятые данные для протокола верхнего уровня
						SLPB b_tx;
						b_tx.lpRam = tx_buf;
						b_tx.Sizes = sizeof(tx_buf);
						sock_singl.Init (ETCPSTATE_RX, TTCPIP::get_tcp_data_adr (lInp), tcpdatasize, sock, tx_ifc, &b_tx);
						
						CBPORTSLOT *slot_port_cb = FindActiveRegSlot (sock->src_port);
						
						if (slot_port_cb) {
							CB_TCPPORT_RX cb_port = slot_port_cb->cb_rx;
							if (cb_port) cb_port (slot_port_cb->cb_obj, &sock_singl);
							}
							
						break;
						}
					case ETCPSW_LASTACK:
						{
						// завершающий ack от клиента
						if (CheckTCPFlags (tcpfield_in, C_TCP_ACK_BIT)) {
							// ack принят завершаем соединение нормально
							sock->sw = ETCPSW_CLOSING;
							sock->TimeOut = 0;
							}
						else
							{
							// в противном случае оно завершится по таймауту
							}
						break;
						}
					default:
						{
						break;
						}
					}
				}
			}
		else
			{
			// сокет отсутствовал, проверяем sync бит
			if (CheckTCPFlags (tcpfield_in, C_TCP_SYN_BIT))
				{
				// есть SYNC - это новое соединение
				// создаем сокет
				TCP_SOCKET_T *sock_op = CreateSocket (macip_in, tcpfield_in);
				if (sock_op)
					{
					unsigned short rawtx_sz = create_full_tcp (sock_op, 0, 0, C_TCP_SYN_BIT | C_TCP_ACK_BIT, true);
					S_TCP_MIT_T *rf_out = (S_TCP_MIT_T*)tx_buf;	
						
					//S_TCP_MIT_T *rf_out = CreateOutTCPFrame (macip_in, sock_op, true);
					//set_tcp_flag (&rf_out->TCP, (C_TCP_SYN_BIT | C_TCP_ACK_BIT));
					//sock_op->sn++;
					sock_op->seq_tx++;
					rf_out->TCP.sn_ack = SwapLong (sock_op->sn + sock_op->seq_tx);
					rf_out->TCP.sn = SwapLong (sock_op->sn_ack + sock_op->seq_rx);
					
					send_tcp (rawtx_sz);
					
					//Transmit_TCP (tx_buf, 0);
					}
				}
			else
				{
				// игнорируем пакет, так как соединения не существовало и не был установлен sync
				}
			}
			
		}
	return rv;
}



// обновляет контрольную сумму ip/tcp и передает данные
// sz - размер полезных данных tcp сегмента
void TTCPIP::send_tcp (unsigned short sz_tcp_data)	// void *lmac, 
{
//bool rv = false;
if (sz_tcp_data)
	{
	//static unsigned short cur_ident = 10;
	//S_MACIP_t *ip_frm = (S_MACIP_t*)lmac;
	void *lmac = tx_buf;
	unsigned short var_ip_size = EMACIP::get_ip_header_size (lmac);
	unsigned short var_tcp_size = get_tcp_header_size (lmac);
	unsigned short sz_tags = sizeof(ETHMAC) + var_ip_size + var_tcp_size;
	unsigned short full_raw_size = sz_tags + sz_tcp_data;
	EMACIP::set_ip_glags (lmac, IP_FLAG_DONTFRAG);
	EMACIP::update_ip_size (lmac, var_tcp_size + sz_tcp_data);
	EMACIP::update_ip_crc (lmac);
	update_tcp_crc (lmac);
	send_raw (full_raw_size);
	}
}



void TTCPIP::send_raw (unsigned short sz)
{
	if (tx_ifc && sz) tx_ifc->raw_send (tx_buf, sz);
}



unsigned short TTCPIP::create_full_tcp (TCP_SOCKET_T *sck, void *l_data, unsigned short sz_tcp_data, unsigned char tcp_flags, bool f_opt_add)
{
SLPB buftx;
buftx.lpRam = tx_buf;
buftx.Sizes = sizeof(buftx);
return create_tcp_data_flags_frame (&buftx, sck, l_data, sz_tcp_data, tcp_flags, f_opt_add);
}



// l_dst - буфер формирования пакета, sck - текущий сокет, l_data - данные tcp сегмента, sz_tcp_data - размер tcp данных, tcp_flags - флаги tcp
unsigned short TTCPIP::create_tcp_data_flags_frame (SLPB *l_dst, TCP_SOCKET_T *sck, void *l_data, unsigned short sz_tcp_data, unsigned char tcp_flags, bool f_opt_add)
{
	unsigned short rv = 0;
	if (sck && l_dst)
		{
		S_TCP_MIT_T *rf_out = create_half_tcp (l_dst, sck, f_opt_add);
		void *lmac = rf_out;
		//unsigned short var_ip_hdr_size = EMACIP::get_ip_header_size (lmac);
		unsigned short var_tcp_hdr_size = get_tcp_header_size (lmac);

		EMACIP::set_ip_glags (lmac, IP_FLAG_DONTFRAG);
		EMACIP::update_ip_size (lmac, var_tcp_hdr_size + sz_tcp_data);
		set_tcp_flag (get_tcp_field (lmac), tcp_flags);
		EMACIP::update_ip_crc (lmac);
		if (l_data) 
			{
			void *ldata_dst = get_tcp_data_adr (lmac);
			ldata_dst = CopyMemory (ldata_dst, l_data, sz_tcp_data);
			rv = ((unsigned char*)ldata_dst) - (unsigned char*)l_dst->lpRam;
			}
		update_tcp_crc (lmac);
		}
	return rv;
}



unsigned short TTCPIP::create_tcp_data_frame (SLPB *l_dst, TCP_SOCKET_T *sck, void *data, unsigned short sz_tcp_data)
{
	return create_tcp_data_flags_frame (l_dst, sck, data, sz_tcp_data, 0, false);
}



unsigned short TTCPIP::create_tcp_close_frame (SLPB *l_dst, TCP_SOCKET_T *sck)
{
	return create_tcp_data_flags_frame (l_dst, sck, 0, 0, C_TCP_FIN_BIT, false);
}


