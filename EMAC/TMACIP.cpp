#include "TMACIP.H"


unsigned char EMACIP::MyMAC[6];
unsigned char EMACIP::MyIP[4];


// определяет начало области памяти после IP заголовка ++
void *EMACIP::get_ip_raw_data (void *maclevel, unsigned short *rawdataipsize)
{
	unsigned char *frm = (unsigned char *)maclevel + sizeof(ETHMAC);
	IPStruct *pf_frm = (IPStruct*)frm;
	unsigned char szip = (pf_frm->IP_VER_IHL_TOS & 0x0F) * 4;
	if (szip >= sizeof(IPStruct))
		{
		frm += szip;
		if (rawdataipsize) 
			{
			unsigned short sz = SWAPB(pf_frm->IP_TOTAL_LENGTH);
			if (sz >= sizeof(IPStruct))
				{
				sz -= szip;
				if (rawdataipsize) *rawdataipsize = sz;
				}
			else
				{
				frm = 0;	// критическая ошибка IP заголовка
				}
			}
		}
	else
		{
		frm = 0;	// критическая ошибка IP заголовка
		}
	return frm;
}


// извлекает из IP заголовка номер протокола ++
unsigned char EMACIP::get_ip_protocol (void *maclevel)
{
	S_MACIP_t *frame = (S_MACIP_t *)maclevel;
	unsigned short prot_ttl = frame->ip.IP_TTL_PROT >> 8;
	return prot_ttl;
}



// проверяет запись на протокол UDP ++
bool EMACIP::is_udp (void *maclevel)
{
	bool rv = false;
	if (get_ip_protocol(maclevel) == PROT_UDP) rv = true;
	return rv;
}



// проверяет запись на протокол TCP ++
bool EMACIP::is_tcp (void *maclevel)
{
	bool rv = false;
	if (get_ip_protocol(maclevel) == PROT_TCP) rv = true;
	return rv;
}



// проверяет запись на протокол ICMP ++
bool EMACIP::is_icmp (void *maclevel)
{
	bool rv = false;
	if (get_ip_protocol(maclevel) == PROT_ICMP) rv = true;
	return rv;
}


// создает mac заголовок
void *EMACIP::create_MAC_header (void *lpFrameRam, unsigned char *lpDestMac, unsigned char *lpSourcMac)
{
void *lrv = (char*)lpFrameRam + sizeof(ETHMAC);
CopyMemory (&((char*)lpFrameRam)[ETH_DA_OFS], (char*)lpDestMac, 6);
CopyMemory (&((char*)lpFrameRam)[ETH_SA_OFS], (char*)lpSourcMac, 6);
((ETHMAC*)lpFrameRam)->EthProtocol = SWAPB(FRAME_IP);
return lrv;
}



// создает ip заголовок ++
void *EMACIP::create_IP_header (void *lpFrameRam, unsigned long DestIP, unsigned long SourcIP, unsigned short sizesUDPData, unsigned char protcl)
{
void *lrv = (char*)lpFrameRam + sizeof(IPStruct);
IPStruct *lpIPss = (IPStruct*) ((char*)lpFrameRam);
unsigned short nDataLengthWords = sizesUDPData;
lpIPss->IP_VER_IHL_TOS = SWAPB (IP_VER_IHL);
lpIPss->IP_TOTAL_LENGTH = SWAPB (IP_HEADER_SIZE + nDataLengthWords);
lpIPss->IP_IDENT = 0;
lpIPss->IP_FLAGS_FRAG = 0;
lpIPss->IP_TTL_PROT = SWAPB (((unsigned short)DEFAULT_TTL << 8) | protcl);
lpIPss->IP_SOURCE = SourcIP;
lpIPss->IP_DESTINATION = DestIP;
lpIPss->IP_HEAD_CHKSUM = 0;
lpIPss->IP_HEAD_CHKSUM = Calc_IP_CRC (lpIPss, IP_HEADER_SIZE);
return lrv;
}


// устанавливает ip флаги ++
void EMACIP::set_ip_glags (void *maclevel, unsigned short fllgs)
{
S_MACIP_t *ip_frm = (S_MACIP_t*)maclevel;
ip_frm->ip.IP_FLAGS_FRAG |= (~IP_FRAGOFS_MASK & fllgs);
}


// обновляет размер IP данных ++
unsigned short EMACIP::update_ip_size (void *maclevel, unsigned short size_new)
{
unsigned short rv = 0, iphdr_size;
S_MACIP_t *ip_frm = (S_MACIP_t*)maclevel;
iphdr_size = get_ip_header_size (maclevel);
if (iphdr_size)
	{
	rv = size_new + iphdr_size;
	ip_frm->ip.IP_TOTAL_LENGTH = SWAPB(rv);
	}
return rv;
}


// обновляет контрольную сумму всех данных ip сегмента ++
unsigned short EMACIP::update_ip_crc (void *maclevel)
{
unsigned short rv = 0;
S_MACIP_t *ip_frm = (S_MACIP_t*)maclevel;
unsigned short ipsize = get_ip_header_size (maclevel);
if (ipsize)
	{
	ip_frm->ip.IP_HEAD_CHKSUM = 0;
	ip_frm->ip.IP_HEAD_CHKSUM = Calc_IP_CRC (&ip_frm->ip, ipsize);
	rv = ipsize;
	}
return rv;
}



// возвращает размер ip заголовка ++
unsigned long EMACIP::get_ip_header_size (void *maclevel)
{
S_MACIP_t *ip_frm = (S_MACIP_t*)maclevel;
unsigned char szip = (ip_frm->ip.IP_VER_IHL_TOS & 0x0F) * 4;
return szip;
}



// возвращает размер данных ip сегмента ++
unsigned short EMACIP::get_ip_size (void *maclevel)
{
unsigned short rv = 0, iphdr_size = get_ip_header_size (maclevel), ipfullsize;
if (iphdr_size >= sizeof(IPStruct))
	{
	S_MACIP_t *ip_frm = (S_MACIP_t*)maclevel;
	ipfullsize = SWAPB(ip_frm->ip.IP_TOTAL_LENGTH);
	if (ipfullsize >= sizeof(IPStruct)) rv = ipfullsize - iphdr_size;
	}
return rv;
}



// вычисляет crc ip ++
unsigned short EMACIP::Calc_IP_CRC (void *Start, unsigned short Count)
{
  unsigned long Sum = 0;
  unsigned short * piStart;                       

  piStart = (unsigned short *)Start;             
  while (Count > 1) {                      
    Sum += *piStart++;
    Count -= 2;
  }

  if (Count)                                   
    Sum += *(unsigned char *)piStart;
  
  while (Sum >> 16)                          
    Sum = (Sum & 0xFFFF) + (Sum >> 16);
  
  return ~Sum;
}



// вычисляет crc tcp ++
unsigned short EMACIP::Calc_TCP_CRC (void *Start, unsigned short Count, TCP_PSEUDO_HEADER_T *pheader)
{
  unsigned long Sum = 0;
  unsigned short *piStart;          

	piStart = (unsigned short *)pheader;
	unsigned char cnt = 6;
	while (cnt)
		{
		Sum += *piStart++;
		cnt--;
		}
	

  piStart = (unsigned short *)Start;             
  while (Count > 1) {                      
    Sum += *piStart++;
    Count -= 2;
  }

  if (Count)                                   
    Sum += *(unsigned char *)piStart;
  
  while (Sum >> 16)                          
    Sum = (Sum & 0xFFFF) + (Sum >> 16);
  
  return ~Sum;
}



// ++
void EMACIP::SetSelf_IP (unsigned long ipadr)
{
	CopyMemory((char*)&MyIP,(char*)&ipadr, sizeof(long));
}



// ++
unsigned long EMACIP::GetSelf_IP ()
{
	unsigned long rv = 0;
	CopyMemory((char*)&rv,(char*)&MyIP, sizeof(long));
	return rv;
}



// ++
unsigned char *EMACIP::GetSelfMac ()
{
	return MyMAC;
}



