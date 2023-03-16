#include "ethernet_PCAP.h"
#include "stm32f4xx_hal_eth.h"
#include "tuavparams.h"

ETH_HandleTypeDef TETHCAP::EthHandle;
static TETHCAP *lanobj = 0;

#include "rutine.h"
#include <string.h>

  __align(4) 
   uint8_t Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE]; /* Ethernet Receive Buffer */
  __align(4) 
   uint8_t Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE]; /* Ethernet Transmit Buffer */
  __align(4) 
   ETH_DMADescTypeDef  DMARxDscrTab[ETH_RXBUFNB];/* Ethernet Rx MA Descriptor */
  __align(4) 
   ETH_DMADescTypeDef  DMATxDscrTab[ETH_TXBUFNB];/* Ethernet Tx DMA Descriptor */
	 
extern ETH_DMADescTypeDef  *DMATxDescToSet;
extern ETH_DMADescTypeDef  *DMARxDescToGet;

//__IO ETH_DMARxFrameInfos *DMA_RX_FRAME_infos;


/*
#ifdef __cplusplus
 extern "C" {
#endif 

void ETH_IRQHandler ()
{
	
	if (__HAL_ETH_DMA_GET_FLAG (&TETHCAP::EthHandle, ETH_DMA_FLAG_R))
		{
		if (lanobj) lanobj->Isr_Rx ();
		__HAL_ETH_DMA_CLEAR_IT(&TETHCAP::EthHandle, ETH_DMA_IT_R);
		}
	
	if (__HAL_ETH_DMA_GET_FLAG (&TETHCAP::EthHandle, ETH_DMA_FLAG_T))
		{
		__HAL_ETH_DMA_CLEAR_IT(&TETHCAP::EthHandle, ETH_DMA_FLAG_T);
		}
		
		
  if(__HAL_ETH_DMA_GET_FLAG(&TETHCAP::EthHandle, ETH_DMA_FLAG_AIS))
  {
    __HAL_ETH_DMA_CLEAR_IT(&TETHCAP::EthHandle, ETH_DMA_FLAG_AIS);

  }

	
	__HAL_ETH_DMA_CLEAR_IT (&TETHCAP::EthHandle, ETH_DMA_IT_NIS);
	
	
}

#ifdef __cplusplus
}
#endif
*/

TETHCAP::TETHCAP ()
{
	
rx_tags = new TTFIFO<ETHRECVTAG>(C_ETH_RX_BUFFERS);
cb_recv_data = 0;
cur_rx_size = 0;
	
	DMARxDescToGet = DMARxDscrTab;
	DMATxDescToSet = DMATxDscrTab;
	lanobj = this;
}



/*
void TETHCAP::Isr_Rx ()
{
	HAL_StatusTypeDef rslt = HAL_ETH_GetReceivedFrame_IT(&EthHandle);
	if (rx_tags->is_free_space ())
		{
			if (rslt == HAL_OK)	
				{
				if (EthHandle.RxFrameInfos.length && EthHandle.RxFrameInfos.length <= sizeof(isr_tmp_copy.data))
					{
					CopyMemory (isr_tmp_copy.data, (void*)EthHandle.RxFrameInfos.buffer, EthHandle.RxFrameInfos.length);
					isr_tmp_copy.size = EthHandle.RxFrameInfos.length;
					rx_tags->push (&isr_tmp_copy);
					}
				}
		}
	EthHandle.RxDesc->Status = ETH_DMARXDESC_OWN;
}
*/

bool TETHCAP::is_transmit ()
{
	bool rv = false;
	if (EthHandle.TxDesc->Status & ETH_DMATXDESC_OWN) rv = true;
	return rv;		// false - свободен, true - в процессе передачи
}



/*
long EPCAP::FindMAC_cb (unsigned short protcl)
{
long rv = -1;
long indx = 0;
while (indx < C_CB_MAC_SLOT_AMOUNT)
	{
	if (mac_cb_slots[indx]->GetProtocol() == protcl)
		{
		rv = indx;
		break;
		}
	indx++;
	}
return rv;
}
*/


/*
bool EPCAP::CallMAC_cb (unsigned short protcl, void *lRawMac_in, unsigned short sz)
{
	bool rv = false;
	if (lRawMac_in && sz) {
		long inx = FindMAC_cb (protcl);
		if (inx >= 0) {
			rv = mac_cb_slots[inx]->Rx (lRawMac_in, sz);
			}
		}
	return rv;
}



bool EPCAP::Registered_MAC_cb (unsigned short e_prtcl, EMACIFACE *obj)
{
	bool rv = false;
	long inx_f = FindMAC_cb (e_prtcl);
	if (!inx_f)
		{
		if (mac_slot_cnt < C_CB_MAC_SLOT_AMOUNT)
			{
			inx_f = mac_slot_cnt;
			mac_slot_cnt++;
			}
		}
	if (inx_f >= 0)
		{
		mac_cb_slots[inx_f] = obj;
		rv = true;
		}
	return rv;
}
*/


/*
long EPCAP::FindIP_cb (unsigned short protcl)
{
long rv = -1;
long indx = 0;
while (indx < C_CB_IP_SLOT_AMOUNT)
	{
	if (ip_cb_slots[indx]->GetIPProtocol() == protcl)
		{
		rv = indx;
		break;
		}
	indx++;
	}
return rv;
}



bool EPCAP::CallIP_cb (unsigned short protcl, void *lRawIP_in, unsigned short sz)
{
	bool rv = false;
	if (lRawIP_in && sz) {
		long inx = FindIP_cb (protcl);
		if (inx >= 0) {
			rv = ip_cb_slots[inx]->Rx (lRawIP_in, sz);
			}
		}
	return rv;
}



bool EPCAP::Registered_IP_cb (unsigned short e_prtcl, EIPIFACE *obj)
{
	bool rv = false;
	long inx_f = FindIP_cb (e_prtcl);
	if (!inx_f)
		{
		if (ip_slot_cnt < C_CB_IP_SLOT_AMOUNT)
			{
			inx_f = ip_slot_cnt;
			ip_slot_cnt++;
			}
		}
	if (inx_f >= 0)
		{
		ip_cb_slots[inx_f] = obj;
		rv = true;
		}
	return rv;
}
*/


void TETHCAP::RegisteredReceiveUDP_CB (void *cb_obj, CB_EthernetInputData cb_proc)
{
lp_cb_obj = cb_obj;
cb_recv_data = cb_proc;	
}



void TETHCAP::Init ()			// BUFPAR *m_rx, BUFPAR *m_tx
{

	//raw_rx = *m_rx;
	//raw_tx = *m_tx;

  GPIO_InitTypeDef GPIO_InitStructure;

	
	__HAL_RCC_GPIOA_CLK_ENABLE ();
	__HAL_RCC_GPIOB_CLK_ENABLE ();
	__HAL_RCC_GPIOC_CLK_ENABLE ();
	__HAL_RCC_SYSCFG_CLK_ENABLE ();
  __HAL_RCC_ETH_CLK_ENABLE ();

	
	
	
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull = GPIO_NOPULL; 
  GPIO_InitStructure.Alternate = GPIO_AF11_ETH;

#define RMII_MODE 

  //SYSCFG_ETH_MediaInterfaceConfig(SYSCFG_ETH_MediaInterface_RMII);  2.11.21

  GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_7;		// ETH_REF_CLK, ETH_MDIO, ETH_CRS
  HAL_GPIO_Init (GPIOA, &GPIO_InitStructure);

  GPIO_InitStructure.Pin = GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13; // ETH_RX_ER, ETH_TX_EN, ETH_TXD0, ETH_TXD1
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);


  GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5;		// ETH_MDC, ETH_RXD0, ETH_RXD1
  HAL_GPIO_Init (GPIOC, &GPIO_InitStructure);

	
	MyMAC[0]= TUAVPARAMS::GetParam_L (ESAVEPARIX_SELF_MAC_0);
	MyMAC[1]= TUAVPARAMS::GetParam_L (ESAVEPARIX_SELF_MAC_1);
	MyMAC[2]= TUAVPARAMS::GetParam_L (ESAVEPARIX_SELF_MAC_2);
	MyMAC[3]= TUAVPARAMS::GetParam_L (ESAVEPARIX_SELF_MAC_3);  // MAC_ADDR3;
	MyMAC[4]= TUAVPARAMS::GetParam_L (ESAVEPARIX_SELF_MAC_4);  // MAC_ADDR4;
	MyMAC[5]= TUAVPARAMS::GetParam_L (ESAVEPARIX_SELF_MAC_5);  // MAC_ADDR5;
	
	
  EthHandle.Instance = ETH;  
  EthHandle.Init.MACAddr = MyMAC;
  EthHandle.Init.AutoNegotiation = ETH_AUTONEGOTIATION_DISABLE;//ETH_AUTONEGOTIATION_ENABLE;//ETH_AUTONEGOTIATION_DISABLE;
  EthHandle.Init.Speed = ETH_SPEED_100M;
  EthHandle.Init.DuplexMode = ETH_MODE_FULLDUPLEX; //ETH_MODE_HALFDUPLEX;//ETH_MODE_FULLDUPLEX;
  EthHandle.Init.MediaInterface = ETH_MEDIA_INTERFACE_RMII;//ETH_MEDIA_INTERFACE_MII;
  EthHandle.Init.RxMode = ETH_RXPOLLING_MODE;// ETH_RXPOLLING_MODE;// ETH_RXINTERRUPT_MODE;
  EthHandle.Init.ChecksumMode = ETH_CHECKSUM_BY_SOFTWARE;//ETH_CHECKSUM_BY_SOFTWARE;//ETH_CHECKSUM_BY_HARDWARE;
  EthHandle.Init.PhyAddress = KSZ8041NL_PHY_ADR;

  HAL_ETH_Init(&EthHandle);


  HAL_ETH_DMATxDescListInit (&EthHandle, DMATxDescToSet , (uint8_t*)&Tx_Buff, ETH_TXBUFNB); 
  HAL_ETH_DMARxDescListInit (&EthHandle, DMARxDescToGet , (uint8_t*)&Rx_Buff, ETH_RXBUFNB);	
	
  
#ifdef CHECKSUM_BY_HARDWARE
	long i; 
  /* Enable the TCP, UDP and ICMP checksum insertion for the Tx frames */
  for(i=0; i<ETH_TXBUFNB; i++)
    {
      ETH_DMATxDescChecksumInsertionConfig(&DMATxDscrTab[i], ETH_DMATxDesc_ChecksumTCPUDPICMPFull);
    }
#endif

	HAL_ETH_ConfigMAC (&EthHandle, (ETH_MACInitTypeDef *)NULL);
  HAL_ETH_Start (&EthHandle);
		
	MyIP[0] = 172;
	MyIP[1] = 16;
	MyIP[2] = 10;
	MyIP[3] = 235;

	MyUDPPort = FixedEthPort;

	BroadcastETHClient.MACAdress[0] = 0xFF;
	BroadcastETHClient.MACAdress[1] = 0xFF;
	BroadcastETHClient.MACAdress[2] = 0xFF;
	BroadcastETHClient.MACAdress[3] = 0xFF;
	BroadcastETHClient.MACAdress[4] = 0xFF;
	BroadcastETHClient.MACAdress[5] = 0xFF;
	BroadcastETHClient.IP.l = 0xFFFFFFFF;
	BroadcastETHClient.UDPPort =  FixedEthPort;

	tx_eth_timeout = 0;
	SYSBIOS::ADD_TIMER_SYS (&tx_eth_timeout);
	DHCP_time_req = 0;
	SYSBIOS::ADD_TIMER_SYS (&DHCP_time_req);
	DHCP_timeout = 0;
	SYSBIOS::ADD_TIMER_SYS (&DHCP_timeout);
	Link_timeout = 0;
	SYSBIOS::ADD_TIMER_SYS (&Link_timeout);
	DHCP.SetMAC (MyMAC);	
	#ifdef TCP_ENABL
		tcp.Init (this);
	#endif
	F_Eth_Inited = 1;
	
	//HAL_NVIC_EnableIRQ (ETH_IRQn);
}






unsigned long EthCalculCRC32sm (unsigned char *lpAdr,unsigned long sizess)
{
unsigned long CRC32Sum = 0;
unsigned char tmpb;
while (sizess)
	{
	tmpb = lpAdr[0];
	CRC32Sum = CRC32Sum * (tmpb & 7);
	CRC32Sum = CRC32Sum + tmpb;
	lpAdr++;
	sizess--;
	}
CRC32Sum = CRC32Sum ^ 0xA1F00FFF;
return CRC32Sum;
}


unsigned short checksum (void *b, int len)
{   
		unsigned short *buf = (unsigned short *)b;
    unsigned int sum=0;
    unsigned short result;

    for ( sum = 0; len > 1; len -= 2 )
			{
        sum += (*buf);		//SWAPB
				buf++;
			}
				
    if ( len == 1 ) sum += (*(unsigned char*)buf);		// SWAPB
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}


unsigned short CheckSum(unsigned short *addr, int len)
{
    register int nleft = len;
    unsigned short answer;
    register int sum = 0;
 
    while( nleft > 1 )  {
        sum += (*addr); addr++;		// SWAPB
        nleft -= 1;
    }
 
    sum += (sum >> 16);         
    answer = ~sum;              
    return (answer);
}



unsigned short crc2 (unsigned short* addr, int count)
{
 
    register long sum = 0;
 
    while( count > 1 )  {
        /*  This is the inner loop */
        sum += ((* (unsigned short*) addr++)); // 
        count -= 2;
    }
 
    /*  Add left-over byte, if any */
    if( count > 0 )
        sum += SWAPB(* (unsigned char *) addr);
 
    /*  Fold 32-bit sum to 16 bits */
    while (sum>>16)
        sum = (sum & 0xffff) + (sum >> 16);
 
    return (unsigned short)(~sum);
    
}





void TETHCAP::raw_send (void *lpRamData, unsigned short sizesdata)		// ETH_TransmitFrame
{
unsigned char *buffer =  (unsigned char *)(DMATxDescToSet->Buffer1Addr);
if (buffer)
	{
	CopyMemory((char*)buffer, (char*)lpRamData, sizesdata);
	//HAL_ETH_Start (&EthHandle);				// 21.12.21
	HAL_ETH_TransmitFrame (&EthHandle, sizesdata);
	//tx_eth_timeout = 10;
	//ETH_FlushTransmitFIFO (&EthHandle);
  //ETH_Prepare_Transmit_Descriptors(sizesdata);
	}
}



unsigned char TETHCAP::CheckAndClearErr ()
{
unsigned char curer = 0;
unsigned long sr = ETH->DMASR;
unsigned long clr_err = 0;
if (sr & ETH_DMA_FLAG_DATATRANSFERERROR)		// ETH_DMA_FLAG_DataTransferError
	{
	curer++;
	}
if (sr & ETH_DMA_FLAG_READWRITEERROR)
	{
	curer++;
	}
	
if (sr & ETH_DMA_FLAG_ACCESSERROR)
	{
	curer++;
	}
	
if (sr & ETH_DMA_FLAG_FBE)
	{
	clr_err |= ETH_DMA_FLAG_FBE;
	curer++;
	}
	
if (sr & ETH_DMA_FLAG_RBU)
	{
	clr_err |= ETH_DMA_FLAG_RBU;
	curer++;
	}
	
if (sr & ETH_DMA_FLAG_RO)
	{
	clr_err |= ETH_DMA_FLAG_RO;
	curer++;
	}
	
if (sr & ETH_DMA_FLAG_AIS)
	{
	clr_err |= ETH_DMA_FLAG_AIS;
	curer++;
	}
	
if (sr & ETH_DMA_FLAG_RWT)
	{
	clr_err |= ETH_DMA_FLAG_RWT;
	curer++;
	}

if (clr_err) ETH->DMASR = clr_err;
	
	/*
		if ((ETH->DMASR & ETH_DMASR_RBUS) != (u32)RESET)  
			{
			ETH->DMASR = ETH_DMASR_RBUS;
			ETH->DMARPDR = 0;
			}
	*/
			
return curer;
}



/*
unsigned long TETHCAP::RecvEthernetData ()
{
	unsigned long rv = 0;
	if (rx_tags)
		{
		ETHRECVTAG dat;
		__disable_irq ();
		if (rx_tags->pop (dat))
			{
			rv = dat.size; 
			CopyMemory ((char*)Rx_Buff, (char*)dat.data, rv);
			}
		__enable_irq();
		}
  return rv;
}
*/


unsigned long TETHCAP::RecvEthernetData ()
{
	unsigned long rv = 0;
	HAL_StatusTypeDef state = HAL_ETH_GetReceivedFrame (&EthHandle);
	if (state == HAL_OK)
		{
		if (EthHandle.RxFrameInfos.SegCount)
			{
			EthHandle.RxFrameInfos.SegCount--;
			rv = EthHandle.RxFrameInfos.length;
			CopyMemory ((char*)Rx_Buff, (char*)EthHandle.RxFrameInfos.buffer, rv);
			EthHandle.RxDesc->Status = ETH_DMARXDESC_OWN;
			}
		}
	else
		{
		EthHandle.RxDesc->Status = ETH_DMARXDESC_OWN;
		}
	return rv;
}



void TETHCAP::WriteWBE(unsigned char *Add, unsigned short Data)
{
  *Add++ = Data >> 8;
  *Add = (char)Data;
}



void TETHCAP::SetSelf_Port (unsigned short mport)
{
	MyUDPPort = mport;
}



unsigned short TETHCAP::Transmit_UDP (UDPPoint_t *lDst, void *pData, unsigned short nDataLength)
{
	unsigned short nDataLengthWords = (nDataLength + 1) & (~1);
	unsigned short rv;
	char *TransmBufers = (char*)Tx_Buff;
	// Ethernet
	CopyMemory((char*)&TransmBufers[ETH_DA_OFS], (char*)lDst->MACAdress, sizeof(MyMAC)); // sizeof(lDst->MACAdress)
	CopyMemory((char*)&TransmBufers[ETH_SA_OFS], (char*)&MyMAC[0], sizeof(MyMAC));
	*(unsigned short *)&TransmBufers[ETH_TYPE_OFS] = SWAPB(FRAME_IP);

	// IP   
	*(unsigned short *)&TransmBufers[IP_VER_IHL_TOS_OFS] = SWAPB(IP_VER_IHL);
	WriteWBE((unsigned char*)&TransmBufers[IP_TOTAL_LENGTH_OFS], IP_HEADER_SIZE + UDP_HEADER_SIZE + nDataLengthWords);
	*(unsigned short *)&TransmBufers[IP_IDENT_OFS] = 0;
	*(unsigned short *)&TransmBufers[IP_FLAGS_FRAG_OFS] = 0;
	*(unsigned short *)&TransmBufers[IP_TTL_PROT_OFS] = SWAPB((DEFAULT_TTL << 8) | PROT_UDP);
	*(unsigned short *)&TransmBufers[IP_HEAD_CHKSUM_OFS] = 0;
	CopyMemory((char*)&TransmBufers[IP_SOURCE_OFS],(char*) &MyIP[0], 4);

	*(unsigned char *)&TransmBufers[IP_DESTINATION_OFS + 0] = lDst->IP.c[0];
	*(unsigned char *)&TransmBufers[IP_DESTINATION_OFS + 1] = lDst->IP.c[1];
	*(unsigned char *)&TransmBufers[IP_DESTINATION_OFS + 2] = lDst->IP.c[2];
	*(unsigned char *)&TransmBufers[IP_DESTINATION_OFS + 3] = lDst->IP.c[3];
	//set_ip_glags (lDst, IP_FLAG_DONTFRAG);
	*(unsigned short *)&TransmBufers[IP_HEAD_CHKSUM_OFS] = Calc_IP_CRC(&TransmBufers[IP_VER_IHL_TOS_OFS], IP_HEADER_SIZE);

	//UDP
	*(unsigned short *)&TransmBufers[UDP_SRCPORT_OFS] = SWAPB(MyUDPPort);
	*(unsigned short *)&TransmBufers[UDP_DSTPORT_OFS] = SWAPB(lDst->UDPPort);
	*(unsigned short *)&TransmBufers[UDP_DATALEN_OFS] = SWAPB(nDataLengthWords + UDP_HEADER_SIZE);
	*(unsigned short *)&TransmBufers[UDP_CRC_OFS] = 0;
	CopyMemory((char*)&TransmBufers[UDP_DATA_OFS], (char*)pData, nDataLength);
	if(nDataLength & 1) TransmBufers[UDP_DATA_OFS + nDataLength] = 0;

	rv = ETH_HEADER_SIZE + IP_HEADER_SIZE + UDP_HEADER_SIZE + nDataLengthWords;
	raw_send ((unsigned char*)TransmBufers, rv);
	return rv;
}





unsigned short TETHCAP::ETH_DataSize_Update (void *lpFrameRam, unsigned short sizesUDPData)
{
	unsigned short rv;
	UPDFrameStruct *lFr = (UPDFrameStruct*)lpFrameRam;
	unsigned short nDataLengthWords = (sizesUDPData + 1) & (~1);
	IPStruct *lpIPss = (IPStruct*) ((char*)lpFrameRam + ETH_DATA_OFS);
	// ips size update
	lFr->IP_FRAME.IP_HEAD_CHKSUM = 0;
	lFr->IP_FRAME.IP_TOTAL_LENGTH = SWAPB (IP_HEADER_SIZE + UDP_HEADER_SIZE + nDataLengthWords);
	lFr->IP_FRAME.IP_HEAD_CHKSUM = Calc_IP_CRC (lpIPss, IP_HEADER_SIZE);
	// udp size update
	lFr->UDP_FRAME.DataLen = SWAPB((sizesUDPData + UDP_HEADER_SIZE));
	// update frame size
	rv = ETH_HEADER_SIZE + IP_HEADER_SIZE + UDP_HEADER_SIZE + nDataLengthWords;
	return rv;
}



void *TETHCAP::ETH_CreateUDPSave (void *lpFrameRam,unsigned short DestPort, unsigned short SourPort, unsigned short sizesUDPData)
{
void *lrv = (char*)lpFrameRam + sizeof(UDPStruct);
UDPStruct *lpUDPss = (UDPStruct*) ((char*)lpFrameRam);
lpUDPss->S_Port = SWAPB(SourPort);
lpUDPss->D_Port = SWAPB(DestPort);
lpUDPss->DataLen = SWAPB((sizesUDPData + UDP_HEADER_SIZE));
lpUDPss->CRC16 = 0;
return lrv;
}



unsigned long TETHCAP::create_icmp_resp (void *mem, const s_macadr_t *lsrc_mac, unsigned long DestIP, void *payload, unsigned short paysize, unsigned char code, unsigned char types, unsigned short id_s, unsigned short seq_s)
{
	void *ldst = create_MAC_header (mem, (unsigned char*)lsrc_mac->mac, MyMAC);
	unsigned short rslt_ip_pay = sizeof(S_ICMP_t);
	if (payload && paysize) rslt_ip_pay += paysize;
	ldst = create_IP_header (ldst, DestIP, GetSelf_IP (), rslt_ip_pay, PROT_ICMP);
	S_ICMP_t *icmp_fr = (S_ICMP_t*)ldst;
	icmp_fr->cod = code;
	icmp_fr->id = id_s;
	icmp_fr->seqno = seq_s;
	icmp_fr->type = types;
	icmp_fr->crc = 0;	
	unsigned char *l_src_pay = (unsigned char *)payload;
	unsigned char *l_dst_pay = ((unsigned char *)ldst) + sizeof(S_ICMP_t);
	if (payload && paysize) l_dst_pay = (unsigned char*)CopyMemory (l_dst_pay, l_src_pay, paysize);
	icmp_fr->crc = checksum ((unsigned short* )icmp_fr, rslt_ip_pay);
	
	return l_dst_pay - (unsigned char *)mem;
}



void *TETHCAP::ETH_CreateNULL_length_UDP_Frame (void *lpFrameRam, const UDPPoint_t &D_ip, const UDPPoint_t &S_ip, unsigned short *cur_size)
{
void *lDst = 0;
	do {
			lDst = create_MAC_header (lpFrameRam, (unsigned char*)D_ip.MACAdress, (unsigned char*)S_ip.MACAdress);
			if (!lDst) break;
			lDst = create_IP_header (lDst, D_ip.IP.l, S_ip.IP.l, UDP_HEADER_SIZE, PROT_UDP);
			if (!lDst) break;
			lDst = ETH_CreateUDPSave (lDst, D_ip.UDPPort, S_ip.UDPPort, 0);
		} while (false);
	if (lDst && cur_size) *cur_size = sizeof(UPDFrameStruct);
	return lDst;
}



unsigned long TETHCAP::GetRX_UDP_size ()
{
unsigned long rv = 0;
UPDFrameStruct *lpRcvFrame = (UPDFrameStruct *)Rx_Buff;
unsigned long fulfrlen = SWAPB(lpRcvFrame->UDP_FRAME.DataLen);
fulfrlen = fulfrlen - UDP_HEADER_SIZE;
if (fulfrlen)
	{
	if (lpRcvFrame->UDP_FRAME.D_Port == SWAPB(MyUDPPort))
		{
		rv = fulfrlen;
		}
	}
return rv;
}



UPDFrameStruct *TETHCAP::GetRX_FullFrame ()
{
UPDFrameStruct *lpRcvFrame = (UPDFrameStruct *)Rx_Buff;
return lpRcvFrame;
}



unsigned char *TETHCAP::GetRX_UDP_data ()
{
unsigned char *lpRv = (unsigned char*)Rx_Buff + sizeof(UPDFrameStruct);
return lpRv;
}




void TETHCAP::Get_src_params (UDPPoint_t *lDst)
{
if (lDst)
	{
	UPDFrameStruct *lpRcvFrame = (UPDFrameStruct *)Rx_Buff;
	lDst->UDPPort = lpRcvFrame->UDP_FRAME.S_Port;
	lDst->IP.l = lpRcvFrame->IP_FRAME.IP_SOURCE;
	CopyMemory ((char*)lDst->MACAdress, (char*)lpRcvFrame->MAC_FRAME.SourceMAC, sizeof(lDst->MACAdress));
	}
}



unsigned short TETHCAP::Get_dst_Port ()
{
	UPDFrameStruct *lpRcvFrame = (UPDFrameStruct *)Rx_Buff;
	return SWAPB(lpRcvFrame->UDP_FRAME.D_Port);
}


/*
unsigned char EPCAP::get_ip_protocol ()
{
	UPDFrameStruct *lpRcvFrame = (UPDFrameStruct *)Rx_Buff;
	unsigned short prot_ttl = lpRcvFrame->IP_FRAME.IP_TTL_PROT >> 8;
	return prot_ttl;
}
*/


bool TETHCAP::is_udp ()
{
	return EMACIP::is_udp (Rx_Buff);
}


bool TETHCAP::is_tcp ()
{
	return EMACIP::is_tcp (Rx_Buff);
}


bool TETHCAP::is_icmp ()
{
	return EMACIP::is_icmp (Rx_Buff);
}






void TETHCAP::CreateARP_AskFrame (unsigned char *lpDestMAC, unsigned long DestIP)
{
	unsigned long DstIpCopy = DestIP;
	unsigned char *lpDIP = (unsigned char*)&DstIpCopy;
	EthARPStruct *lpARP = (EthARPStruct *) ((unsigned char*)Tx_Buff + sizeof(ETHMAC));
	// Ethernet
	char *TransmBufers = (char*)Tx_Buff;
	CopyMemory(&TransmBufers[ETH_DA_OFS],(char*) &lpDestMAC[0], 6);
	CopyMemory(&TransmBufers[ETH_SA_OFS], (char*)&MyMAC[0], 6);
	*(unsigned short *)&TransmBufers[ETH_TYPE_OFS] = SWAPB(FRAME_ARP);
	// ARP ask
	lpARP->ETH_PROTOCOL = SWAPB (0x0001);
	lpARP->IP_PROTOCOL = SWAPB (FRAME_IP);
	lpARP->HardwareSize = 6;
	lpARP->ProtocolSize = 4;
	lpARP->OPCodeRequest = SWAPB(2);
	CopyMemory((char*)&lpARP->SourceMAC[0],(char*) &MyMAC[0],6);
	CopyMemory((char*)&lpARP->SourceIP,(char*) &MyIP[0],4);
	CopyMemory((char*)&lpARP->DestinationMAC[0],(char*) &lpDestMAC[0],6);
	CopyMemory((char*)&lpARP->DestinationIP,(char*) &lpDIP[0],4);
}



void TETHCAP::Dhcp_task_recv (UPDFrameStruct *RxFrm)
{
if (f_dhcp_client_enabled)
	{
	switch (DHCP.sw)
		{
		case EDHCPSW_OFFER:
			{
			char *lDat = (char*)RxFrm + sizeof(UPDFrameStruct);
			if (DHCP.CheckOfferFrame (lDat))
				{
				DHCP.sw = EDHCPSW_REQUEST;			// следующий шаг
				}
			else
				{
				DHCP.sw = EDHCPSW_NONE;					// прерываем выполнение так как произошла ошибка
				}
			break;
			}
		case EDHCPSW_ACK:
			{
			char *lDat = (char*)RxFrm + sizeof(UPDFrameStruct);
			if (DHCP.CheckAck (lDat))
				{
				DHCP.sw = EDHCPSW_COMPLETE;			// следующий шаг
				// обновляем IP
					
				}
			else
				{
				DHCP.sw = EDHCPSW_NONE;					// прерываем выполнение так как произошла ошибка
				}
			break;
			}
		case EDHCPSW_COMPLETE:
			{
			break;
			}
		case EDHCPSW_NONE:
		default:
			{
			break;
			}
		}
	}
}



void TETHCAP::DHCP_OnOff (bool val)
{
	f_dhcp_client_enabled = val;
	if (!val) {
		DHCP.ClearDHCP_CompleteStatus ();
		DHCP.sw = EDHCPSW_NONE;
		DHCP_time_req = 0;
		}
}



bool TETHCAP::DHCP_GetState ()
{
	return f_dhcp_client_enabled;
}



bool TETHCAP::DHCP_GetOfferAdress (unsigned long &ip_offr)
{
	bool rv = false;
	if (f_dhcp_client_enabled)
		{
		if (DHCP.CheckDHCP_Complete())
			{
			rv = true;
			ip_offr = DHCP.GetOffers_IP ();
			}
		}
	return rv;
}



void TETHCAP::Dhcp_task ()
{
	//f_dhcp_client_enabled = true;
	if (f_dhcp_client_enabled)
		{
		switch (DHCP.sw)
			{
			case EDHCPSW_DISCOVER:
				{
				// формирование DHCPDISCOVER
				unsigned short cur_size = 0;
				UDPPoint_t D_ip;
				D_ip.UDPPort = 67;
				fillmem (D_ip.MACAdress, 0xFF, sizeof(D_ip.MACAdress));
				D_ip.IP.l = 0xFFFFFFFF;
				
				UDPPoint_t S_ip;
				S_ip.UDPPort = 68;
				CopyMemory((char*)S_ip.MACAdress, (char*)&MyMAC[0], sizeof(S_ip.MACAdress));
				S_ip.IP.l = 0;
				void *lFrm = ETH_CreateNULL_length_UDP_Frame (Tx_Buff, D_ip, S_ip, &cur_size);
				if (lFrm && cur_size)
					{
					if (ETH_TX_BUF_SIZE >= cur_size)	
						{
						unsigned short dat_size = DHCP.DHCPDISCOVER_gen (lFrm, ETH_TX_BUF_SIZE - cur_size);
						if (dat_size) {
							unsigned short tx_szz = ETH_DataSize_Update (Tx_Buff, dat_size);
							raw_send ((unsigned char*)Tx_Buff, tx_szz);
							DHCP.sw = EDHCPSW_OFFER;
							DHCP_timeout = 5000;
							}
						}
					}
				break;
				}
			case EDHCPSW_OFFER:
				{
				if (!DHCP_timeout)
					{
					// время ожидания пакета OFFER вышло
					DHCP.sw = EDHCPSW_NONE;
					}
				break;
				}
			case EDHCPSW_REQUEST:
				{
				// формирование DHCPREQUEST
				unsigned short cur_size = 0;
				UDPPoint_t D_ip;
				D_ip.UDPPort = 67;
				fillmem (D_ip.MACAdress, 0xFF, sizeof(D_ip.MACAdress));
				D_ip.IP.l = 0xFFFFFFFF;
					
				UDPPoint_t S_ip;
				S_ip.UDPPort = 68;
				CopyMemory((char*)S_ip.MACAdress, (char*)&MyMAC[0], sizeof(S_ip.MACAdress));
				S_ip.IP.l = 0;
				void *lFrm = ETH_CreateNULL_length_UDP_Frame (Tx_Buff, D_ip, S_ip, &cur_size);
				
				if (lFrm && cur_size)
					{
					if (ETH_TX_BUF_SIZE >= cur_size)	
						{
						unsigned short dat_size = DHCP.DHCPREQUEST_gen (lFrm, ETH_TX_BUF_SIZE - cur_size, DHCP.GetCurentDCHPServer_ip (), DHCP.GetOffers_IP ());
						if (dat_size) {
							unsigned short tx_szz = ETH_DataSize_Update (Tx_Buff, dat_size);
							raw_send ((unsigned char*)Tx_Buff, tx_szz);
							DHCP.sw = EDHCPSW_ACK;
							DHCP_timeout = 5000;
							}
						}
					}	
				break;
				}
			case EDHCPSW_ACK:
				{
				if (!DHCP_timeout)
					{
					// время ожидания пакета ACK вышло
					DHCP.sw = EDHCPSW_NONE;
					}
				}
			case EDHCPSW_COMPLETE:
				{
				break;
				}
			case EDHCPSW_NONE:
			default:
				{
				if (!DHCP_time_req)
					{
					DHCP.sw = EDHCPSW_DISCOVER;
					DHCP_time_req = 15000;
					}	
				break;
				}
			}
		}
}



bool TETHCAP::Get_Link_Status ()
{
bool rv = false;
if (Link_timeout) rv = true;
return rv;
}




void TETHCAP::Task ()
{
	if (!F_Eth_Inited) return;
	
	Dhcp_task ();						// реализация отправки запросов dhcp
		#ifdef TCP_ENABL
			tcp.Task ();
		#endif	
	if (cur_rx_size) return;
	
	
	//CheckAndClearErr ();
	
	unsigned short SzRcvDtmp = RecvEthernetData ();
	
	if (SzRcvDtmp)
		{
		Link_timeout = C_ETH_LINK_TIMEOUT;
		// анализ протоколов принятых пакетов
		// определение служебных, вспомогательных протоколов: ARP
		ETHMAC *lpMacFr  = (ETHMAC*)Rx_Buff;
		S_MACIP_t *macip = (S_MACIP_t*)Rx_Buff;
		unsigned short CurProt = SWAPB(lpMacFr->EthProtocol);
		switch (CurProt)
			{
			case 0x0806:
				{
				// Ethetnet: ARP (обработка принятого пакета и отправка ответа)
				EthARPStruct *lpARP1 = (EthARPStruct*)(((char*)Rx_Buff) + sizeof(ETHMAC));
				if (SWAPB(lpARP1->ETH_PROTOCOL) == 0x0001)
					{
					if (SWAPB(lpARP1->IP_PROTOCOL) == 0x0800)
						{
						if (lpARP1->HardwareSize == 6)
							{
							if (lpARP1->ProtocolSize == 4)
								{
								if (SWAPB(lpARP1->OPCodeRequest) == 0x0001)
									{
									if (CompareData ((char*)&lpARP1->DestinationIP,(char*)&MyIP[0],4))
										{
										// IP адрес совпал, подготовить ARP ответ
										CreateARP_AskFrame (&lpARP1->SourceMAC[0],lpARP1->SourceIP);
										raw_send ((unsigned char*)Tx_Buff, sizeof(EthARPFull));
										}
									}
								}
							}
						}
					}
				break;
				}
			case 0x0800:			// IP: протоколы
				{
				uint8_t ip_prot = get_ip_protocol (Rx_Buff);
				switch (ip_prot)
					{
					case PROT_ICMP:		// отработка PING
						{
						unsigned short rawdataipsize = 0;
						S_ICMP_t *icmp_frm = (S_ICMP_t*)get_ip_raw_data (Rx_Buff, &rawdataipsize);
						if (icmp_frm)
							{
							// ping -n 5 -l 10 172.16.10.10
							static S_ICMP_t data;
							data = *icmp_frm;
							switch (data.type)
								{
								case 8:
									{
									if (icmp_frm->cod == 0)
										{
										unsigned short last_crc = icmp_frm->crc;
										icmp_frm->crc = 0;
										unsigned short crc = checksum ((unsigned short* )icmp_frm, rawdataipsize);
										if (crc == last_crc)
											{
											unsigned short tx_pay = rawdataipsize - sizeof(S_ICMP_t);
											if (tx_pay <= 950)
												{
												char txbuf[1024];
												char *lpay = (char*)icmp_frm;
												lpay += sizeof(S_ICMP_t);	
												unsigned long txsz = create_icmp_resp (txbuf, (const s_macadr_t *)macip->mac.SourceMAC, macip->ip.IP_SOURCE, lpay, tx_pay, 0, 0, icmp_frm->id, icmp_frm->seqno);
												if (txsz) raw_send ((unsigned char*)txbuf, txsz);
												}
											}
										}
									break;
									}
								default:
									{
									break;
									}
								}
							}
						break;
						}
					case PROT_TCP:
						{
						#ifdef TCP_ENABL
							tcp.in (Rx_Buff, SzRcvDtmp);
						#endif
						break;
						}
					case PROT_UDP:
						{
							UPDFrameStruct *RxFrm = GetRX_FullFrame ();
							unsigned short port = SwapShort(RxFrm->UDP_FRAME.D_Port);
							switch (port)
								{
								case 68:		// dhcp acks
									{
									Dhcp_task_recv (RxFrm);		// реализация приема dhcp
									break;
									}
								default:
									{
									if (port == MyUDPPort || port == 0xFFFF)
										{
										if (cb_recv_data)
											{
											// cb - зарегистрирован, передаем данные туда
											UDPPoint_t Client_inf;
											Get_src_params (&Client_inf);
											(*cb_recv_data) (lp_cb_obj, &Client_inf, GetRX_UDP_data (), GetRX_UDP_size ());
											}
										else
											{
											// регистрации cb небыло, сохраняем параметры и работаем по запросу
											cur_rx_size = GetRX_UDP_size ();
											Get_src_params (&Client_inf_last);
											}
										}
									break;
									}
								}
						break;
						}
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



unsigned short TETHCAP::CheckReceiveData ()
{
	return cur_rx_size;
}



unsigned short TETHCAP::GetUDPData (UDPPoint_t &clnt_inf, void *lDst, unsigned short sz_max)
{
	unsigned short rv = 0;
	if (cur_rx_size)
		{
		Get_src_params (&clnt_inf);
		if (lDst && sz_max)
			{
			unsigned short real_sz = cur_rx_size;// GetRX_UDP_size ();
			if (real_sz > sz_max) real_sz = sz_max;
			CopyMemory ((char*)lDst, (char*)GetRX_UDP_data (), real_sz);
			rv = real_sz;
			}
		ClearRxBufer ();	// разблокируем прием данных
		}
	return rv;
}



void TETHCAP::ClearRxBufer ()
{
	cur_rx_size = 0;
}



