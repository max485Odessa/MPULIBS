#ifndef __EthPCAP_H
#define __EthPCAP_H

#include "rutine.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_eth.h"
#include "netconf.h"
#include "SYSBIOS.H"
#include "IPTCPDEFS.H"
#include "TDHCPClient.h"
#include "TFTASKIF.h"
#include "TTCPIP.h"
#include "TTFIFO.h"

#define C_CB_MAC_SLOT_AMOUNT 5
#define C_CB_IP_SLOT_AMOUNT 10
#define C_ETH_LINK_TIMEOUT 5000


#define FixedEthPort ((unsigned short) 14551)
typedef void (*CB_EthernetInputData) (void *Obj, UDPPoint_t *Client_inf, void *lData, unsigned short sizes);

struct ETHRECVTAG {
	unsigned char data[2048];
	unsigned long size;
};

#define C_ETH_RX_BUFFERS 3

class TETHCAP : public EMACIP, public TFFC {
		ETHRECVTAG isr_tmp_copy;
		TTFIFO<ETHRECVTAG> *rx_tags ;
	
		UDPPoint_t BroadcastETHClient;		// заполненная структура адрессов, для broadcast посылки
	
		#ifdef TCP_ENABL
			TTCPIP tcp;
		#endif
	
		TDHCPCLIENT DHCP;
		bool f_dhcp_client_enabled;
		unsigned short cur_rx_size;

		unsigned short MyUDPPort;

		utimer_t tx_eth_timeout;	
		utimer_t Link_timeout;
		utimer_t DHCP_time_req;					// период повторений запроса на получения ip адресса от dhcp сервера
		utimer_t DHCP_timeout;					// таймаут для dhcp операций
	
		unsigned long GetRX_UDP_size (void);
		unsigned char *GetRX_UDP_data (void);
	
		bool F_Eth_Inited;		// флаг
		void WriteWBE (unsigned char *Add, unsigned short Data);
		void CreateARP_AskFrame (unsigned char *lpDestMAC, unsigned long DestIP);

		
		unsigned long RecvEthernetData ();
	
		void *ETH_CreateUDPSave (void *lpFrameRam,unsigned short DestPort, unsigned short SourPort, unsigned short sizesUDPData);
		unsigned short ETH_DataSize_Update (void *lpFrameRam, unsigned short sizesUDPData);
		void *ETH_CreateNULL_length_UDP_Frame (void *lpFrameRam, const UDPPoint_t &D_ip, const UDPPoint_t &S_ip, unsigned short *cur_size);
		UPDFrameStruct *GetRX_FullFrame ();
	
		void Get_src_params (UDPPoint_t *lDst);
		UDPPoint_t Client_inf_last;
		
		bool is_udp ();
		bool is_tcp ();
		bool is_icmp ();

		unsigned long create_icmp_resp (void *mem, const s_macadr_t *lsrc_mac, unsigned long ip, void *payload, unsigned short paysize, unsigned char code, unsigned char types, unsigned short id_s, unsigned short seq_s);
	
		CB_EthernetInputData cb_recv_data;
		void *lp_cb_obj;
	
		void Dhcp_task ();
		void Dhcp_task_recv (UPDFrameStruct *RxFrm);
		
		virtual void raw_send (void *mem, unsigned short sz);
		uint32_t ETH_CheckFrameReceived (void);
	
	public:
		TETHCAP ();
		static ETH_HandleTypeDef EthHandle;
		void Isr_Rx ();
		unsigned char CheckAndClearErr ();
		void Init ();						// установка буферов памяти для приема/передачи и инициализация ethernet периферии
		virtual void Task ();		// реализует DHCP, ARP, буферизирует UDP данные
	
		// если не зарегистрирован CB_EthernetInputData, прием данных осуществляется посредством функций: CheckReceiveData, GetReceiveData, ClearRxBufer.
		unsigned short CheckReceiveData ();
		unsigned short GetUDPData (UDPPoint_t &clnt_inf, void *lDst, unsigned short sz_buf_max);
		unsigned short Get_dst_Port ();
		void ClearRxBufer ();
	
		// передача данных
		unsigned short Transmit_UDP (UDPPoint_t *lDst, void *data, unsigned short sz);
		virtual bool is_transmit ();
	
		void RegisteredReceiveUDP_CB (void *cb_obj, CB_EthernetInputData cb_proc);
		void SetSelf_Port (unsigned short mport);
	
		void DHCP_OnOff (bool val);				// включает отключает DHCP
		bool DHCP_GetState ();
		bool DHCP_GetOfferAdress (unsigned long &ip_offr);
		
		bool Get_Link_Status ();
};


#endif


