#include "p_ARP_ss.h"


TARPSS::TARPSS ()
{
	Protocol = 0x0806;		// ARP
	call_tx = 0;
}



void TARPSS::Init (BUFPAR *r_tx)
{
	raw_tx = *r_tx;
}



void TARPSS::Task ()
{
}



void TARPSS::CreateARP_AskFrame (unsigned char *lpDestMAC, unsigned long DestIP)
{
	unsigned long DstIpCopy = DestIP;
	unsigned char *lpDIP = (unsigned char*)&DstIpCopy;
	EthARPStruct *lpARP = (EthARPStruct *) (raw_tx.lpRam + sizeof(ETHMAC));
	// Ethernet
	char *TransmBufers = (char*)raw_tx.lpRam;
	CopyMemory(&TransmBufers[ETH_DA_OFS],(char*) &lpDestMAC[0], 6);
	CopyMemory(&TransmBufers[ETH_SA_OFS], (char*)&self_mac[0], sizeof(self_mac));
	*(unsigned short *)&TransmBufers[ETH_TYPE_OFS] = SWAPB(FRAME_ARP);
	// ARP ask
	lpARP->ETH_PROTOCOL = SWAPB (0x0001);
	lpARP->IP_PROTOCOL = SWAPB (FRAME_IP);
	lpARP->HardwareSize = 6;
	lpARP->ProtocolSize = 4;
	lpARP->OPCodeRequest = SWAPB(2);
	CopyMemory((char*)&lpARP->SourceMAC[0],(char*) &self_mac[0],sizeof(self_mac));
	CopyMemory((char*)&lpARP->SourceIP,(char*) &self_ip,4);
	CopyMemory((char*)&lpARP->DestinationMAC[0],(char*) &lpDestMAC[0],6);
	CopyMemory((char*)&lpARP->DestinationIP,(char*) &lpDIP[0],4);
}




bool TARPSS::Rx (void *dat_in, unsigned short sizes_in)
{
	bool rv = false;
	// Ethetnet: ARP (обработка принятого пакета и отправка ответа)
	EthARPStruct *lpARP1 = (EthARPStruct*)(((char*)dat_in) + sizeof(ETHMAC));
	if (SWAPB(lpARP1->ETH_PROTOCOL) == 0x0001) {
		if (SWAPB(lpARP1->IP_PROTOCOL) == 0x0800) {
			if (lpARP1->HardwareSize == 6) {
				if (lpARP1->ProtocolSize == 4) {
					if (SWAPB(lpARP1->OPCodeRequest) == 0x0001) {
						if (CompareData ((char*)&lpARP1->DestinationIP,(char*)&self_ip,4)) {
							// IP адрес совпал, подготовить ARP ответ
							CreateARP_AskFrame (&lpARP1->SourceMAC[0],lpARP1->SourceIP);
							if (call_tx) call_tx (raw_tx.lpRam, sizeof(EthARPFull));
							rv = true;
							}
						}
					}
				}
			}
		}
	return rv;
}



