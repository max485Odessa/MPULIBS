#ifndef _H_UDP_PROTOCOL_SUB_SYSTEM_H_
#define _H_UDP_PROTOCOL_SUB_SYSTEM_H_



#include "IPTCPDEFS.H"
#include "e_protocol_ip_ifce_ss.h"



class TUDPSS: public EIPIFACE {
	private:
		
	public:
		TUDPSS ();
		virtual void Init (BUFPAR *raw_tx);
		virtual void Task ();
		virtual bool Rx (void *dat_in, unsigned short sizes_in);
};


#endif


