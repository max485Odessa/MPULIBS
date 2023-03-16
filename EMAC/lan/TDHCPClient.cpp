#include "TDHCPClient.h"

static const unsigned char opt_cookie[4] = {99,130,83,99};		// метка наличия dhcp опций в сообщении


TDHCPCLIENT::TDHCPCLIENT ()
{
	Xid = 0;
	f_is_mac = false;
	f_dhcp_complete = false;
	sw = EDHCPSW_NONE;
}



void TDHCPCLIENT::SetMAC (void *lSrc)
{
if (lSrc)
	{
	f_is_mac = true;
	CopyMemory ((char*)SELFMAC, (char*)lSrc, sizeof(SELFMAC));
	}
}



void *TDHCPCLIENT::AddMagicCookie (void *ldst)
{
	void *lDst = CopyMemory (ldst, (void*)opt_cookie, sizeof(opt_cookie));
	return lDst;
}
	


void *TDHCPCLIENT::AddOption_53 (void *ldst, unsigned char code)
{
	void *lRv = ldst;
	DHCPOPT53 *lOpt = (DHCPOPT53*)ldst;
	lOpt->Code = 53;
	lOpt->Len = 1;
	lOpt->Dat = code;// EDHCPOPCODE_DISCOVER;
	lRv = ((char*)lRv) + sizeof(DHCPOPT53);
	return lRv;
}


void *TDHCPCLIENT::AddOption_50 (void *ldst, unsigned long ip_adr)
{
	void *lRv = ldst;
	DHCPOPT54 *lOpt = (DHCPOPT54*)ldst;
	lOpt->Code = 50;
	lOpt->Len = sizeof(ip_adr);
	lOpt->IP = ip_adr;
	lRv = ((char*)lRv) + sizeof(DHCPOPT54);
	return lRv;
}


void *TDHCPCLIENT::AddOption_54 (void *ldst, unsigned long ip_adr)
{
	void *lRv = ldst;
	DHCPOPT54 *lOpt = (DHCPOPT54*)ldst;
	lOpt->Code = 54;
	lOpt->Len = sizeof(ip_adr);
	lOpt->IP = ip_adr;
	lRv = ((char*)lRv) + sizeof(DHCPOPT54);
	return lRv;
}



void *TDHCPCLIENT::AddOption_end (void *ldst)
{
char *lDst = (char*)ldst;
*lDst = 0xFF;
lDst++;
return lDst;
}


unsigned short TDHCPCLIENT::DHCPDISCOVER_gen (void *lDst, unsigned short max_size)
{
	unsigned short rv = 0;
	f_dhcp_complete = false;
	if (lDst && max_size >= (sizeof(DHCPFORM) + sizeof(DHCPOPT53)))
		{
		Xid++;
		DHCPFORM *Frm = (DHCPFORM*)lDst;
		fillmem (lDst, 0, max_size);
		Frm->Op = 1;
		Frm->HType = 1;
		Frm->HLen = 6;
		Frm->XID = SwapLong(Xid); // 0x3903F326;
			CopyMemory ((char*)(Frm->CHAddr ), (char*)SELFMAC, sizeof(SELFMAC));		// CHAddr
		//CopyMemory ((char*)(Frm->CHAddr + sizeof(Frm->CHAddr) - 6), (char*)SELFMAC, sizeof(SELFMAC));		// CHAddr
		void *lOptAdr = (char*)lDst + sizeof(DHCPFORM);
		// есть опции добавляем magic cookie
		lOptAdr = AddMagicCookie (lOptAdr);
		lOptAdr = AddOption_53 (lOptAdr, EDHCPOPCODE_DISCOVER);
		lOptAdr = AddOption_end (lOptAdr);
		rv = (char*)lOptAdr - (char*)lDst;
		}
	return rv;
}



unsigned short TDHCPCLIENT::DHCPREQUEST_gen (void *lDst, unsigned short max_size, unsigned long dhcp_serv_ip, unsigned long ip_adr)
{
	unsigned short rv = 0;
	if (lDst && max_size >= (sizeof(DHCPFORM) + sizeof(DHCPOPT53) + sizeof(DHCPOPT54)))
		{
		DHCPFORM *Frm = (DHCPFORM*)lDst;
		fillmem (lDst, 0, max_size);
		Frm->Op = 1;
		Frm->HType = 1;
		Frm->HLen = 6;
		Frm->XID = SwapLong(Xid);//0x3903F326;
		CopyMemory ((char*)(Frm->CHAddr ), (char*)SELFMAC, sizeof(SELFMAC));	
		//CopyMemory ((char*)(Frm->CHAddr + sizeof(Frm->CHAddr) - 6), (char*)SELFMAC, sizeof(SELFMAC));
		void *lOptAdr = (char*)lDst + sizeof(DHCPFORM);
		// есть опции добавляем magic cookie
		lOptAdr = AddMagicCookie (lOptAdr);
		lOptAdr = AddOption_53 (lOptAdr, EDHCPOPCODE_REQUEST);
		lOptAdr = AddOption_50 (lOptAdr, ip_adr);
		lOptAdr = AddOption_54 (lOptAdr, dhcp_serv_ip);
		lOptAdr = AddOption_end (lOptAdr);
		rv = (char*)lOptAdr - (char*)lDst;
		}
	return rv;
}



unsigned long TDHCPCLIENT::GetCurentDCHPServer_ip ()
{
	return dhcp_ip;
}



unsigned long TDHCPCLIENT::GetOffers_IP ()
{
	return offer_ip;
}



bool TDHCPCLIENT::CheckAck (void *lDst)
{
	bool rv = false;
	if (lDst)
		{
		DHCPFORM *Frm = (DHCPFORM*)lDst;
		if (Frm->Op == 2 && Frm->HLen == 6 && Frm->HType == 1 && Frm->Hops == 0)
			{
			void *lOptAdr = (char*)lDst + sizeof(DHCPFORM);
			// проверяем magic cookie
			if (CompareData ((char*)opt_cookie, (char*)lOptAdr, sizeof(opt_cookie)))
				{
				char *lOpt = (char*)lOptAdr + sizeof(opt_cookie);
				DHCPOPT *Copt;
				bool f_error = false;
				bool f_exit = false;
				//bool rslt = false;
				bool f_op53 = false;
				//bool f_op1 = false;
				bool f_op51 = false;
				bool f_op54 = false;
				while (!f_error && !f_exit)
					{
					Copt = (DHCPOPT*)lOpt;
					switch (Copt->Code)
						{
						case 1:
							{
							if (Copt->Len == 4)
								{
								DHCPOPT1 *f_tmp = (DHCPOPT1*)lOpt;
								//f_op1 = true;
								net_ip_mask = f_tmp->Mask;
								}
							else
								{
								f_error = true;
								}
							break;
							}
						case 51:
							{
							if (Copt->Len == 4)
								{
								DHCPOPT51 *f_tmp = (DHCPOPT51*)lOpt;
								f_op51 = true;
								time_ip = f_tmp->IP;
								}
							else
								{
								f_error = true;
								}
							break;
							}
						case 54:
							{
							if (Copt->Len == 4)
								{
								DHCPOPT54 *f_tmp = (DHCPOPT54*)lOpt;
								f_op54 = true;
								dhcp_ip = f_tmp->IP;
								}
							else
								{
								f_error = true;
								}
							break;
							}
						case 53:
							{
							if (Copt->Len == 1)
								{
								DHCPOPT53 *f_tmp = (DHCPOPT53*)lOpt;
									
								//view_t *ldd = (view_t*)lOpt;
								//char *buf = (char*)lOpt;
								if (f_tmp->Dat == EDHCPOPCODE_ACK) 
									{
									f_op53 = true;
									}
								else
									{
									f_error = true;	// выйти если не offer
									}
								}
							else
								{
								f_error = true;
								}
							break;
							}
						case 0xFF:
							{
							f_exit = true;
							break;
							}
						}
					lOpt += Copt->Len + sizeof(DHCPOPT);
					}
					
				if (!f_error && f_op51 && f_op53 && f_op54) rv = true;
				f_dhcp_complete = rv;
				}
			}
		}
	return rv;
}



bool TDHCPCLIENT::CheckDHCP_Complete ()
{
	return f_dhcp_complete;
}



void TDHCPCLIENT::ClearDHCP_CompleteStatus ()
{
	f_dhcp_complete = false;
}



bool TDHCPCLIENT::CheckOfferFrame (void *lDst)
{
	bool rv = false;
	if (lDst)
		{
		DHCPFORM *Frm = (DHCPFORM*)lDst;
		if (Frm->Op == 2 && Frm->HLen == 6 && Frm->HType == 1 && Frm->Hops == 0)
			{
			void *lOptAdr = (char*)lDst + sizeof(DHCPFORM);
			// проверяем magic cookie
			if (CompareData ((char*)opt_cookie, (char*)lOptAdr, sizeof(opt_cookie)))
				{
				char *lOpt = (char*)lOptAdr + sizeof(opt_cookie);
				DHCPOPT *Copt;
				bool f_error = false;
				bool f_exit = false;
				//bool rslt = false;
				bool f_op53 = false;
				//bool f_op1 = false;
				bool f_op51 = false;
				bool f_op54 = false;
				while (!f_error && !f_exit)
					{
					Copt = (DHCPOPT*)lOpt;
					switch (Copt->Code)
						{
						case 1:
							{
							if (Copt->Len == 4)
								{
								DHCPOPT1 *f_tmp = (DHCPOPT1*)lOpt;
								//f_op1 = true;
								net_ip_mask = f_tmp->Mask;
								}
							else
								{
								f_error = true;
								}
							break;
							}
						case 51:
							{
							if (Copt->Len == 4)
								{
								DHCPOPT51 *f_tmp = (DHCPOPT51*)lOpt;
								f_op51 = true;
								time_ip = f_tmp->IP;
								}
							else
								{
								f_error = true;
								}
							break;
							}
						case 54:
							{
							if (Copt->Len == 4)
								{
								DHCPOPT54 *f_tmp = (DHCPOPT54*)lOpt;
								f_op54 = true;
								dhcp_ip = f_tmp->IP;
								}
							else
								{
								f_error = true;
								}
							break;
							}
						case 53:
							{
							if (Copt->Len == 1)
								{
								DHCPOPT53 *f_tmp = (DHCPOPT53*)lOpt;
								if (f_tmp->Dat == EDHCPOPCODE_OFFER) 
									{
									f_op53 = true;
									}
								else
									{
									f_error = true;	// выйти если не offer
									}
								}
							else
								{
								f_error = true;
								}
							break;
							}
						case 0xFF:
							{
							f_exit = true;
							break;
							}
						}
					lOpt += Copt->Len + sizeof(DHCPOPT);
					}
				if (!f_error && f_op51 && f_op53 && f_op54) 
					{
					offer_ip = Frm->YIAddr;
					rv = true;
					}
						
				}
			}
		}
	return rv;
}

