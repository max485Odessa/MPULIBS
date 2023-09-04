#include "RS232_BYTESTAFF_BUFS.h"



TUARTBSTAFBUF::TUARTBSTAFBUF ()
{
HandleOpenPort = INVALID_HANDLE_VALUE;
Inp_Stat = E_SFCMD_SYNC;
f_no_init_ov = true;

recv_push_bufers_indx = 0;
recv_pop_bufers_indx = 0;
f_rx_timeout_on = false;
need_reconnect = EUSRTRCON_NONE;
c_size_tmp_buf = 128000;
lpRxDestRam = new unsigned char [c_size_tmp_buf];

memset (&readOL, 0, sizeof(readOL));
readOL.hEvent = CreateEvent(NULL, TRUE, TRUE, 0);
memset(&writeOL, 0, sizeof(writeOL));
writeOL.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
}



TUARTBSTAFBUF::~TUARTBSTAFBUF ()
{
ClosePort ();
CloseHandle (writeOL.hEvent);
CloseHandle (readOL.hEvent);
}





void TUARTBSTAFBUF::ClosePort ()
{
if (HandleOpenPort != INVALID_HANDLE_VALUE)
    {
    HANDLE lasth =  HandleOpenPort;
    HandleOpenPort = INVALID_HANDLE_VALUE;
    while (!CloseHandle(lasth))
            {
            Sleep (1);
            }
    }
}





bool TUARTBSTAFBUF::UpdateUartParam (HANDLE PortLocal1,DWORD ubrrsets)
{
bool rv=1;

        GetCommState(PortLocal1, &MyDCB);

        MyDCB.DCBlength = sizeof(DCB);
        MyDCB.BaudRate = ubrrsets;

        MyDCB.fBinary = true;//true;
        MyDCB.fParity = false;
        MyDCB.fOutxCtsFlow = false;
        MyDCB.fOutxDsrFlow = false;
        MyDCB.fDtrControl = DTR_CONTROL_DISABLE;
        MyDCB.fDsrSensitivity = false;
        MyDCB.fTXContinueOnXoff = true;//false;//true;
        MyDCB.fOutX = false;
        MyDCB.fInX = false;
        MyDCB.fErrorChar = false;
        MyDCB.fNull = false;
        MyDCB.fRtsControl = RTS_CONTROL_DISABLE;
        MyDCB.fAbortOnError = false;
        MyDCB.fDummy2 = 0;
        MyDCB.XonLim = 10;  // 2048
        MyDCB.XoffLim = 100;  // 2048
        MyDCB.ByteSize = 8;
        MyDCB.Parity = NOPARITY;//PARITY_EVEN;// PARITY_EVEN;//NOPARITY;
        MyDCB.StopBits = ONESTOPBIT;//ONESTOPBIT;//TWOSTOPBITS;//  ONESTOPBIT;

        MyDCB.XonChar = 0x11;
        MyDCB.XoffChar = 0x13;
        MyDCB.ErrorChar = 0;
        MyDCB.EofChar = 0;
        MyDCB.EvtChar = 0;

        MyDCB.wReserved1 = 0;
        if (!SetCommState(PortLocal1,&MyDCB)) rv=0;


        
        MyTimeout.ReadIntervalTimeout=1;
        MyTimeout.ReadTotalTimeoutMultiplier=1;
        MyTimeout.ReadTotalTimeoutConstant=1;
        MyTimeout.WriteTotalTimeoutMultiplier=0;
        MyTimeout.WriteTotalTimeoutConstant=0;     // 1

        /*
        MyTimeout.ReadIntervalTimeout=1000;
        MyTimeout.ReadTotalTimeoutMultiplier=0;
        MyTimeout.ReadTotalTimeoutConstant=0;
        MyTimeout.WriteTotalTimeoutMultiplier=0;
        MyTimeout.WriteTotalTimeoutConstant=0;
        */

        if (!SetCommTimeouts(PortLocal1,&MyTimeout)) rv=0;

return rv;
}




bool TUARTBSTAFBUF::OpenPort (char *lpNames, unsigned long UarrSpeed)
{
bool rv=0;
if (lpNames)
        {
        String nametmpprt = "\\\\.\\" + (String)lpNames;
        ClosePort ();
        HandleOpenPort = CreateFile (nametmpprt.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED ,0); // FILE_ATTRIBUTE_NORMAL |  FILE_FLAG_OVERLAPPED FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED     // FILE_SHARE_READ | FILE_SHARE_WRITE    = GENERIC_READ | GENERIC_WRITE
        if (HandleOpenPort != INVALID_HANDLE_VALUE)
            {
            last_strportopen = nametmpprt;
            UpdateUartParam (HandleOpenPort,UarrSpeed);
            lastbaud_rate = UarrSpeed;
            f_no_init_ov = true;
            rv = 1;
            }
        }
return rv;
}



bool TUARTBSTAFBUF::Reconnect ()
{
bool rv=false;
if (last_strportopen != "" && lastbaud_rate)
        {
        HandleOpenPort = CreateFile (last_strportopen.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED ,0); // FILE_ATTRIBUTE_NORMAL |  FILE_FLAG_OVERLAPPED FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED     // FILE_SHARE_READ | FILE_SHARE_WRITE    = GENERIC_READ | GENERIC_WRITE
        if (HandleOpenPort != INVALID_HANDLE_VALUE)
            {
            UpdateUartParam (HandleOpenPort,lastbaud_rate);
            f_no_init_ov = true;
            rv = true;
            }
        }
return rv;
}



bool TUARTBSTAFBUF::is_opened ()
{
return (HandleOpenPort != INVALID_HANDLE_VALUE)?true:false;
}



bool TUARTBSTAFBUF::CheckPort (char *lpNames)
{
bool rv=0;
if (lpNames)
        {
        String nametmpprt = "\\\\.\\" + (String)lpNames;
        ClosePort ();
        HANDLE lasth  = CreateFile (nametmpprt.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,0,0);
        if (lasth != INVALID_HANDLE_VALUE)
            {
            while (!CloseHandle(lasth))
                {
                Sleep (1);
                }
            rv = 1;
            }
        }
return rv;
}



unsigned char TUARTBSTAFBUF::GetListFreePorts (TComboBox *c_box)
{
unsigned char rv = 0;
if (c_box)
    {
    c_box->Clear();
    unsigned char Indx = 1;
    String str_prt;
    while (Indx < 120)
        {
        str_prt = "COM" + IntToStr(Indx);
        if (CheckPort (str_prt.c_str()))
            {
            rv++;
            c_box->Items->Add(str_prt);
            }
        Indx++;
        }
    }
return rv;
}



bool TUARTBSTAFBUF::MPWriteFile(HANDLE MyPort1,void *TransmitBufer1,DWORD sizes,DWORD *bc)
{
bool rv=0;
if (MyPort1!=INVALID_HANDLE_VALUE)
        {
        WriteFile(MyPort1,TransmitBufer1,sizes,bc,NULL);
        rv=1;
        }
return rv;
}




unsigned char TUARTBSTAFBUF::CalculateCRC8_ByteStaff (unsigned char *lSrc, unsigned short sizes)
{
unsigned char crv_rv = 0x81;
unsigned char tmpl = 1 << ((sizes & 7));
unsigned char indx = 0;
while (sizes)
    {
    if (!tmpl) tmpl = 1;
    crv_rv = crv_rv + (lSrc[0] + (indx | tmpl));
    lSrc++;
    tmpl = tmpl << 1;
    indx++;
    sizes--;
    }
if (!crv_rv) crv_rv = indx | 0x08;
return crv_rv;
}



unsigned short TUARTBSTAFBUF::UartSTAFF_Coding (unsigned char *lSrc, unsigned char *lDst, unsigned short sizes)
{
unsigned short rv = 0;
if (lDst && lSrc)
	{
	unsigned short size_out = 0;
	unsigned char datt;

	lDst[size_out++] = C_BSTAFF;
	lDst[size_out++] = C_BST_START;

	while (sizes)
		{
		datt = lSrc[0];
		if (datt == C_BSTAFF) lDst[size_out++] = C_BSTAFF;
		lDst[size_out++] = datt;
		lSrc++;
		sizes--;
		}
	lDst[size_out++] = C_BSTAFF;				
	lDst[size_out++] = C_BST_STOP;
	rv = size_out;
	}
return rv;
}



bool TUARTBSTAFBUF::SendData (char *lpRam, unsigned short sizes)
{
bool rv = false;
if (lpRam && sizes && HandleOpenPort != INVALID_HANDLE_VALUE)
      {
      unsigned char *lpTransBufer;
      unsigned int SizeTransData;
      PurgeComm (HandleOpenPort, PURGE_RXCLEAR | PURGE_TXCLEAR);

      SizeTransData = UartSTAFF_Coding (lpRam, TransmitBufer, sizes);
      lpTransBufer = TransmitBufer;


        unsigned long c_tick = GetTickCount (), dlt_t;
        dlt_t = c_tick - bps_last_tick_tx;
        if (dlt_t >= 1000)
            {
            bps_counter_tx = bps_counter_tx_local;
            bps_counter_tx_local = 0;
            bps_last_tick_tx = c_tick;
            }

        bool f_rslt = WriteFile(HandleOpenPort, lpTransBufer, SizeTransData, &bctrns, &writeOL);
        DWORD signaled = WaitForSingleObject(writeOL.hEvent, 1000);   // INFINITE   2
        bool ret = signaled == WAIT_OBJECT_0 && GetOverlappedResult (HandleOpenPort, &writeOL, &bctrns, false);      // FALSE
        if (ret)
            {
            bps_counter_tx_local += bctrns;
            rv = true;
            }
      }
return rv;
}



bool TUARTBSTAFBUF::SendDataRXTime (char *lSrc, unsigned short sizes, unsigned long tcks)
{
 if (tcks)
    {
    f_rx_timeout_on = true;
    timeout_tick_value = GetTickCount () + tcks;
    }
 else
    {
    f_rx_timeout_on = false;
    }
 return SendData (lSrc, sizes);
}





void TUARTBSTAFBUF::TmpAddReadData (char dat)
{

SUSARTBUFER *lBf = &RecvBufer[recv_push_bufers_indx];
lBf->data[lBf->indx_recv] = dat;
lBf->indx_recv++;
if (lBf->indx_recv >= sizeof(lBf->data))
    {
    Inp_Stat = E_SFCMD_SYNC;
    lBf->indx_recv = 0;
    lBf->f_full = false;
    }
}




void TUARTBSTAFBUF::ISR_Add_Rx_data (unsigned char dat)
{
if (recv_bufers_count >= C_USART_MAXBUF_AMOUNT) return;
switch ((char)Inp_Stat)
	{
	case E_SFCMD_SYNC:		// ожидание командного байта
		{
		if (dat == C_BSTAFF) {
            // командный байт принят
			Inp_Stat = E_SFCMD_SYNC_STAFF;
			}
		break;
		}
	case E_SFCMD_SYNC_STAFF:
		{
		if (dat == C_BST_START) {
			// стартовое условие обнаружено
            SUSARTBUFER *lBf = &RecvBufer[recv_push_bufers_indx];
            lBf->indx_recv = 0;
            lBf->f_full = false;
			Inp_Stat = E_SFCMD_DATA;
			}
		else
			{
			Inp_Stat = E_SFCMD_SYNC;		// снова на поиск STAFF
			}
		break;
		}
	case E_SFCMD_DATA:
		{
		if (dat == C_BSTAFF) {
			Inp_Stat = E_SFCMD_DATA_STAFF;	// принята команда в потоке данных
			}
		else
			{
			TmpAddReadData (dat);	// добавить в буфер данные
			}
		break;
		}
	case E_SFCMD_DATA_STAFF:
		{
		switch (dat)
			{
			case C_BSTAFF:
				{
				TmpAddReadData (C_BSTAFF);		// вставка байта
				Inp_Stat = E_SFCMD_DATA;			// продолжить
				break;
				}
			case C_BST_STOP:				// команда стоп
				{
                SUSARTBUFER *lBf = &RecvBufer[recv_push_bufers_indx];
                lBf->f_full = true;
                recv_push_bufers_indx++;
                if (recv_push_bufers_indx >= C_USART_MAXBUF_AMOUNT) recv_push_bufers_indx = 0;  // инкремент кольцевого указателя
                recv_bufers_count++;
                Inp_Stat = E_SFCMD_SYNC;
                //receive_complete (false);
                f_rx_timeout_on = false;
                break;
				}
			default:
				{
				// все остальное это ошибка
				// повторный старт считается ошибкой
                SUSARTBUFER *lBf = &RecvBufer[recv_push_bufers_indx];
                lBf->f_full = false;
                lBf->indx_recv = 0;
				Inp_Stat = E_SFCMD_SYNC;
				break;
				}
			}
		break;
		}
    default:
        {
        Inp_Stat = E_SFCMD_SYNC;
        break;
        }
	}
}



unsigned short TUARTBSTAFBUF::CheckReadData ()
{
return recv_bufers_count;
}



void TUARTBSTAFBUF::Task ()
{
Task_Rx ();
reconnect_task ();
}



void TUARTBSTAFBUF::reconnect_task ()
{
switch (need_reconnect)
    {
    case EUSRTRCON_NEED:
        {
        if (HandleOpenPort != INVALID_HANDLE_VALUE) ClosePort ();
        if (Reconnect ()) need_reconnect = EUSRTRCON_NONE;
        break;
        }
    }
}




void TUARTBSTAFBUF::Task_Rx ()
{
 if (need_reconnect != EUSRTRCON_NONE) return;
 
    unsigned long c_tick = GetTickCount ();
    dlt_t_rxi = c_tick - bps_last_tick_rx;
    if (dlt_t_rxi >= 1000)
        {
        bps_counter_rx = bps_counter_rx_local;
        bps_counter_rx_local = 0;
        bps_last_tick_rx = c_tick;
        }

    if (f_rx_timeout_on)
        {
        if (c_tick >= timeout_tick_value)
            {
            //receive_complete (true);
            f_rx_timeout_on = false;
            }
        }

    if (HandleOpenPort != INVALID_HANDLE_VALUE)
        {
        if (f_no_init_ov)
            {
            if (!SetCommMask (HandleOpenPort, EV_RXCHAR))
                {
                need_reconnect = EUSRTRCON_NEED;
                return;
                }
            f_no_init_ov = false;
            }

        WaitCommEvent (HandleOpenPort, &mask_rxi, &readOL);
        DWORD modst;
        if (!GetCommModemStatus ( HandleOpenPort, &modst))
            {
            //MessageBox (0, "GetCommModemStatus", "", MB_OK);
            need_reconnect = EUSRTRCON_NEED;
            return;
            }

        signal_rxi = WaitForSingleObject (readOL.hEvent, 40);
        switch (signal_rxi)
            {
            case WAIT_OBJECT_0:
                {
                if (GetOverlappedResult (HandleOpenPort, &readOL, &temp_rxi, false))
                    {
                    if((mask_rxi & EV_RXCHAR)!= 0)
                        {
                        if (!ClearCommError (HandleOpenPort, &temp_rxi, &comstatrx))
                            {
                             //MessageBox (0, "ClearCommError", "", MB_OK);
                             need_reconnect = EUSRTRCON_NEED;
                            return;
                            }
                        btr_rxi = comstatrx.cbInQue;
                        if (btr_rxi > c_size_tmp_buf)
                            {
                            btr_rxi = c_size_tmp_buf;
                            //MessageBox(0, "crit size", "", MB_OK);
                            }

                        if (ReadFile (HandleOpenPort, lpRxDestRam, btr_rxi, &temp_rxi, &readOL))
                          {
                          unsigned long ix = 0;
                          bps_counter_rx_local += temp_rxi;
                          while (temp_rxi)
                            {
                            ISR_Add_Rx_data (lpRxDestRam[ix]);
                            temp_rxi--;
                            ix++;
                            }
                          }

                        }
                    }
                else
                    {
                    //MessageBox (0, "GetOverlappedResult", "", MB_OK);
                    need_reconnect = EUSRTRCON_NEED;
                    return;
                    }
                break;
                }
            case WAIT_FAILED:
                {
                //MessageBox (0, "waitcom failed", "WaitForSingleObject", MB_OK);
                need_reconnect = EUSRTRCON_NEED;
                return;
                break;
                }
            }
        }

}




unsigned short TUARTBSTAFBUF::ReadData (char *lpDest, unsigned short sizes)
{
unsigned short rv = 0;
if (lpDest && sizes && recv_bufers_count)
        {
        SUSARTBUFER *lBf = &RecvBufer[recv_pop_bufers_indx];
        if (sizes > lBf->indx_recv) sizes = lBf->indx_recv;
        rv = sizes;
        CopyMemorySDC (lBf->data, lpDest, rv);
        recv_pop_bufers_indx++;
        if (recv_pop_bufers_indx >= C_USART_MAXBUF_AMOUNT) recv_pop_bufers_indx = 0;   // инкремент кольцевого указателя
        recv_bufers_count--;
        }
return rv;
}












