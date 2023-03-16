#ifndef _H_PROTOCOL_SUB_SYSTEM_IP_INTERFACE_H_
#define _H_PROTOCOL_SUB_SYSTEM_IP_INTERFACE_H_


#include "rutine.h"


typedef bool (*lpTransmitData) (void *lData, unsigned short sizes);



class EIPIFACE {
	protected:
		unsigned char self_mac[6];
		unsigned long self_ip;
		unsigned short IPProtocol;
		BUFPAR raw_tx;
		lpTransmitData call_tx;
	
	public:
		void SetSelf_MAC (unsigned char *linp);
		void SetSelf_IP (unsigned long s_ip);
		void SetTransmitCall (lpTransmitData cll_tx);
		unsigned short GetIPProtocol ();
		virtual void Init (BUFPAR *raw_tx) = 0;
		virtual void Task () = 0;
		virtual bool Rx (void *dat_in, unsigned short sizes_in) = 0;
};



#endif


