#ifndef _H_PROTOCOL_SUB_SYSTEM_MAC_INTERFACE_H_
#define _H_PROTOCOL_SUB_SYSTEM_MAC_INTERFACE_H_


#include "rutine.h"


/*  ---------   Интерфейс обротчика соответствующего протокола MAC уровня.  -----------

Хотя self_MAC и self_IP адресс для всех протоколов единый(то может быть обьявлен статическим для всех обьектов), 
для гибкости они индивидуально дублируются в каждый обьект обработчика.

*/


typedef bool (*RawEthernetTransmit) (void *lData, unsigned short sizes);



class EMACIFACE {
	protected:
		unsigned char self_mac[6];
		unsigned long self_ip;
		unsigned short Protocol;
		BUFPAR raw_tx;
		RawEthernetTransmit call_tx;
	
	public:
		void SetSelf_MAC (unsigned char *linp);
		void SetSelf_IP (unsigned long s_ip);
		void SetTransmitCall (RawEthernetTransmit cll_tx);
		unsigned short GetProtocol ();
		virtual void Init (BUFPAR *raw_tx) = 0;
		virtual void Task () = 0;
		virtual bool Rx (void *dat_in, unsigned short sizes_in) = 0;
};



#endif

