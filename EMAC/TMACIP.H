#ifndef _H_TMACIP_H_
#define _H_TMACIP_H_

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_eth.h"
#include "IPTCPDEFS.H"
#include "rutine.h"



class EMACIP {
	protected:
		static unsigned char MyMAC[6];
		static unsigned char MyIP[4];

		
	public:
		virtual void raw_send (void *mem, unsigned short sz) = 0;
		virtual bool is_transmit () = 0;
		static void *get_ip_raw_data (void *maclevel, unsigned short *rawdataipsize);
		static unsigned char get_ip_protocol (void *maclevel);
		static bool is_udp (void *maclevel);
		static bool is_tcp (void *maclevel);
		static bool is_icmp (void *maclevel);
		static void *create_MAC_header (void *lpFrameRam, unsigned char *lpDestMac, unsigned char *lpSourcMac);
		static void *create_IP_header (void *lpFrameRam, unsigned long DestIP, unsigned long SourcIP, unsigned short sizesUDPData, unsigned char protcl);
		static void set_ip_glags (void *maclevel, unsigned short fllgs);
		static unsigned short update_ip_size (void *maclevel, unsigned short size_new);		// обновляет размер IP данных
		static unsigned short update_ip_crc (void *maclevel);			// обновляет контрольную сумму всех данных ip сегмента
		static unsigned short get_ip_size (void *maclevel);				// возвращает размер данных ip сегмента
		static unsigned long get_ip_header_size (void *maclevel);	// возвращает размер ip заголовка
		static unsigned short Calc_IP_CRC (void *Start, unsigned short Count);
		static unsigned short Calc_TCP_CRC (void *Start, unsigned short Count, TCP_PSEUDO_HEADER_T *pheader);
	
		void SetSelf_IP (unsigned long ipadr);
		static unsigned long GetSelf_IP ();
		static unsigned char *GetSelfMac ();
};





#endif

