#ifndef _H_DHCP_CLIENT_STM32_H_
#define _H_DHCP_CLIENT_STM32_H_



#include "rutine.h"
#include "IPTCPDEFS.H"



typedef struct {
	char dat[4];
} view_t;


enum EDHCPOPCODE {EDHCPOPCODE_DISCOVER = 1, EDHCPOPCODE_OFFER = 2, EDHCPOPCODE_REQUEST = 3, EDHCPOPCODE_DECLINE = 4, EDHCPOPCODE_ACK = 5, EDHCPOPCODE_NAK = 6, EDHCPOPCODE_RELEASE = 7, EDHCPOPCODE_INFORM = 8};
enum EDHCPSW {EDHCPSW_NONE = 0, EDHCPSW_DISCOVER = 1, EDHCPSW_OFFER = 2, EDHCPSW_REQUEST = 3, EDHCPSW_ACK = 4, EDHCPSW_COMPLETE = 5, EDHCPSW_ENDENUM = 6};



class TDHCPCLIENT {
		bool f_is_mac;
	
		unsigned char SELFMAC[6];
	
		void *AddMagicCookie (void *ldst);
		void *AddOption_53 (void *ldst, unsigned char code);			// командная опция DHCP
		void *AddOption_54 (void *ldst, unsigned long ip_adr);		// dhcp server
		void *AddOption_50 (void *ldst, unsigned long ip_adr);		// need this ip
		void *AddOption_end (void *ldst);
	
		unsigned long Xid;
					
		unsigned long net_ip_mask;		// маска подсети
		unsigned long router_ip;			// ip роутера
		unsigned long offer_ip;					// предложенный IP адресс
		unsigned long dhcp_ip;				// адрес DHCP сервера
		unsigned long time_ip;				// время аренды
		bool f_dhcp_complete;					// получение ip адреса успешно завершено
	
	public:
		TDHCPCLIENT ();
		EDHCPSW sw;
		unsigned short DHCPDISCOVER_gen (void *lDst, unsigned short max_size);
		unsigned short DHCPREQUEST_gen (void *lDst, unsigned short max_size, unsigned long dhcp_serv_ip, unsigned long req_ip);
		bool CheckOfferFrame (void *lDst);
		bool CheckAck (void *lDst);
		unsigned long GetCurentDCHPServer_ip ();
		unsigned long GetOffers_IP ();
		void SetMAC (void *lSrc);
	
		bool CheckDHCP_Complete ();
		void ClearDHCP_CompleteStatus ();
		
	
};


#endif

