#include "RS232_STREAM_IMPL.h"



TUARTSTREAM::TUARTSTREAM (uint32_t szrx)
{
fifo_rx = new TTFIFO<uint8_t>(szrx) ;
HandleOpenPort = INVALID_HANDLE_VALUE;
f_no_init_ov = true;


need_reconnect = EUSRTRCON_NONE;
c_size_tmp_buf = 128000;
lpRxDestRam = new unsigned char [c_size_tmp_buf];

memset (&readOL, 0, sizeof(readOL));
readOL.hEvent = CreateEvent(NULL, TRUE, TRUE, 0);
memset(&writeOL, 0, sizeof(writeOL));
writeOL.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
}



TUARTSTREAM::~TUARTSTREAM ()
{
ClosePort ();
CloseHandle (writeOL.hEvent);
CloseHandle (readOL.hEvent);
}



void TUARTSTREAM::ClosePort ()
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



bool TUARTSTREAM::UpdateUartParam (HANDLE PortLocal1,DWORD ubrrsets)
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

        //if (!SetCommTimeouts(PortLocal1,&MyTimeout)) rv=0;

return rv;
}




bool TUARTSTREAM::OpenPort (char *lpNames, unsigned long UarrSpeed)
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



bool TUARTSTREAM::OpenPort (uint16_t n, uint32_t UarrSpeed)
{
String str = "COM" + IntToStr(n);
return OpenPort (str.c_str(), UarrSpeed);
}



bool TUARTSTREAM::Reconnect ()
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



bool TUARTSTREAM::is_opened ()
{
return (HandleOpenPort != INVALID_HANDLE_VALUE)?true:false;
}



bool TUARTSTREAM::CheckPort (char *lpNames)
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



unsigned char TUARTSTREAM::GetListFreePorts (TComboBox *c_box)
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



bool TUARTSTREAM::MPWriteFile(HANDLE MyPort1,void *TransmitBufer1,DWORD sizes,DWORD *bc)
{
bool rv=0;
if (MyPort1!=INVALID_HANDLE_VALUE)
        {
        WriteFile(MyPort1,TransmitBufer1,sizes,bc,NULL);
        rv=1;
        }
return rv;
}



bool TUARTSTREAM::txif_out (void *src, uint32_t sz)
{
bool rv = false;

if (src && sz && HandleOpenPort != INVALID_HANDLE_VALUE)
      {
      //PurgeComm (HandleOpenPort, PURGE_RXCLEAR | PURGE_TXCLEAR);


        unsigned long c_tick = GetTickCount (), dlt_t;
        dlt_t = c_tick - bps_last_tick_tx;
        if (dlt_t >= 1000)
            {
            bps_counter_tx = bps_counter_tx_local;
            bps_counter_tx_local = 0;
            bps_last_tick_tx = c_tick;
            }

        bool f_rslt = WriteFile (HandleOpenPort, src, sz, &bctrns, &writeOL);
        DWORD signaled = WaitForSingleObject(writeOL.hEvent, 10);   // INFINITE   2   1000
        bool ret = signaled == WAIT_OBJECT_0 && GetOverlappedResult (HandleOpenPort, &writeOL, &bctrns, false);      // FALSE
        if (ret)
            {
            bps_counter_tx_local += bctrns;
            rv = true;
            }
        else
            {
            rv = false;
            }
      }
return rv;
}



unsigned long TUARTSTREAM::GetRX_BPS ()
{
  return bps_counter_rx;
}



unsigned long TUARTSTREAM::GetTX_BPS ()
{
  return bps_counter_tx;
}





void TUARTSTREAM::Task ()
{
Task_Rx ();
reconnect_task ();
}



void TUARTSTREAM::reconnect_task ()
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



bool TUARTSTREAM::is_recon_status ()
{
return (need_reconnect == EUSRTRCON_NEED)?true:false;
}



void TUARTSTREAM::Task_Rx ()
{
 if (need_reconnect != EUSRTRCON_NONE) return;
 
    unsigned long c_tick = GetTickCount ();

    // rx bps
    dlt_t_rxi = c_tick - bps_last_tick_rx;
    if (dlt_t_rxi >= 1000)
        {
        bps_counter_rx = bps_counter_rx_local;
        bps_counter_rx_local = 0;
        bps_last_tick_rx = c_tick;
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

        signal_rxi = WaitForSingleObject (readOL.hEvent, 100);   // 40
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
                          rxrata_to_fifo (lpRxDestRam, temp_rxi);
                          bps_counter_rx_local += temp_rxi;
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



void TUARTSTREAM::rxrata_to_fifo (void *src, uint32_t sz)
{
fifo_rx->push ((uint8_t*)src, sz);
}



uint32_t TUARTSTREAM::rxif_in (void *dst, uint32_t max_sz)
{
uint32_t rv = 0;
if (!dst || !max_sz)
    {
    fifo_rx->clear();
    }
else
    {
    rv = fifo_rx->pop ((uint8_t*)dst, max_sz);
    }

return rv;
}



uint32_t TUARTSTREAM::rxif_is_rx ()
{
return fifo_rx->frame_count();
}



uint32_t TUARTSTREAM::txif_free_space ()
{
return 100000;
}













