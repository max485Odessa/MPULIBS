#include "e_protocol_mac_ifce_ss.h"


//unsigned char EMACIFACE::self_mac[6];
//unsigned long EMACIFACE::self_ip;

unsigned short EMACIFACE::GetProtocol ()
{
	return Protocol;
}



void EMACIFACE::SetSelf_MAC (unsigned char *linp)
{
	CopyMemory ((char*)self_mac, (char*)linp, sizeof(self_mac));
}



void EMACIFACE::SetSelf_IP (unsigned long s_ip)
{
	self_ip = s_ip;
}



void EMACIFACE::SetTransmitCall (RawEthernetTransmit cll_tx)
{
	call_tx = cll_tx;
}

