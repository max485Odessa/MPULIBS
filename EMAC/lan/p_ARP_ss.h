#ifndef _H_ARP_PROTOCOL_SUB_SYSTEM_H_
#define _H_ARP_PROTOCOL_SUB_SYSTEM_H_

#include "IPTCPDEFS.H"
//#include "e_protocol_mac_ifce_ss.h"


class TARPSS: public EMACIFACE {
	private:
		void CreateARP_AskFrame (unsigned char *lpDestMAC, unsigned long DestIP);
	
	public:
		TARPSS ();
		virtual void Init (BUFPAR *raw_tx);
		virtual void Task ();
		virtual bool Rx (void *dat_in, unsigned short sizes_in);
	
};


#endif

