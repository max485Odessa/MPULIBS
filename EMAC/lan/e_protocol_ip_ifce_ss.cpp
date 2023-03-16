#include "e_protocol_ip_ifce_ss.h"



unsigned short EIPIFACE::GetIPProtocol ()
{
	return IPProtocol;
}



void EIPIFACE::SetSelf_MAC (unsigned char *linp)
{
	CopyMemory ((char*)self_mac, (char*)linp, sizeof(self_mac));
}



void EIPIFACE::SetSelf_IP (unsigned long s_ip)
{
	self_ip = s_ip;
}



void EIPIFACE::SetTransmitCall (lpTransmitData cll_tx)
{
	call_tx = cll_tx;
}


