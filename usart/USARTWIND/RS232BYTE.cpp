#include "RS232BYTE.H"



TUARTBYTE::TUARTBYTE ()
{
semafore_sw = ECOMSW_RX;
HandleOpenPort = INVALID_HANDLE_VALUE;
lpTransBufer = 0;
F_BlockUart_cmd = 0;
F_BlockUart_ack = 0;
TransmitOk = false;
ClearRecvStatus ();
ThrRx_Start = false;
f_is_connected = false;
f_no_init_ov = true;
c_size_tmp_buf = 128000;
need_reconnect = EUSRTRCON_NONE;
last_strportopen = "";
lpRxDestRam = new unsigned char [c_size_tmp_buf];
f_trafic_enable = true;
    memset (&readOL, 0, sizeof(readOL));
    readOL.hEvent = CreateEvent(NULL, TRUE, TRUE, 0);
  memset(&writeOL, 0, sizeof(writeOL));
  writeOL.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
recon_cb = 0;
discon_cb = 0;
if (f_trafic_enable) wr_trafic_log.CreateStream ("trafic.txt");
}


bool TUARTBYTE::is_open ()
{
return (HandleOpenPort != INVALID_HANDLE_VALUE);
}


void TUARTBYTE::CloseExitAll ()
{
if (HandleOpenPort != INVALID_HANDLE_VALUE)
    {
    while (GetTransmitStatus ()) {::Sleep(1);};
    ClosePort ();
    }
}



bool TUARTBYTE::Reconnect ()
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




void TUARTBYTE::Taks ()
{
    reconnect_task ();
    if (need_reconnect != EUSRTRCON_NONE) return;
    Task_Rx ();
    Task_Tx ();
}




void TUARTBYTE::reconnect_task ()
{
switch (need_reconnect)
    {
    case EUSRTRCON_NEED:
        {
        if (f_is_connected)
            {
            if (HandleOpenPort != INVALID_HANDLE_VALUE)
                {
                ClosePort ();
                f_is_connected = true;
                }
            if (discon_cb) discon_cb ();
            if (Reconnect ())
                {
                f_is_connected = true;
                need_reconnect = EUSRTRCON_NONE;
                if (recon_cb) recon_cb ();
                }
            f_no_init_ov = true;
            }
        else
            {
            /*
            if (Reconnect ())
                {
                f_is_connected = true;
                need_reconnect = EUSRTRCON_NONE;
                if (recon_cb) recon_cb ();
                }
            */
            }
        break;
        }
    }
}


TUARTBYTE::~TUARTBYTE ()
{

CloseExitAll ();
if (f_trafic_enable) wr_trafic_log.CloseStream();
if (lpRxDestRam) delete []lpRxDestRam;
}



void TUARTBYTE::ClosePort ()
{
if (HandleOpenPort != INVALID_HANDLE_VALUE)
    {
    HANDLE lasth =  HandleOpenPort;
    HandleOpenPort = INVALID_HANDLE_VALUE;
    while (!CloseHandle(lasth))
            {
            ::Sleep (1);
            }
    f_no_init_ov = true;
    f_is_connected = false;
    }

}



bool TUARTBYTE::UpdateUartParam (HANDLE PortLocal1,DWORD ubrrsets)
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


        /* 30.1.23
        MyTimeout.ReadIntervalTimeout=1;
        MyTimeout.ReadTotalTimeoutMultiplier=1;
        MyTimeout.ReadTotalTimeoutConstant=5;
        MyTimeout.WriteTotalTimeoutMultiplier=0;
        MyTimeout.WriteTotalTimeoutConstant=0;     // 1
        */

        MyTimeout.ReadIntervalTimeout=10;
        MyTimeout.ReadTotalTimeoutMultiplier=2;
        MyTimeout.ReadTotalTimeoutConstant=2;
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
        //PurgeComm (HandleOpenPort, PURGE_RXCLEAR | PURGE_TXCLEAR);

return rv;
}



bool TUARTBYTE::OpenPort (char *lpNames, unsigned long UarrSpeed)
{
bool rv=0;
if (lpNames)
        {
        String nametmpprt = "\\\\.\\" + (String)lpNames;
        UartReadingBlocked (true);
        ClosePort ();
        // FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED
        HandleOpenPort = CreateFile (nametmpprt.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED  ,0); // FILE_ATTRIBUTE_NORMAL FILE_ATTRIBUTE_NORMAL FILE_FLAG_OVERLAPPED FILE_FLAG_OVERLAPPED FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED     // FILE_SHARE_READ | FILE_SHARE_WRITE    = GENERIC_READ | GENERIC_WRITE
        if (HandleOpenPort != INVALID_HANDLE_VALUE)
            {
            last_strportopen = nametmpprt;
            UpdateUartParam (HandleOpenPort, UarrSpeed);
            lastbaud_rate = UarrSpeed;
            f_no_init_ov = true;
            f_is_connected = true;
            rv = 1;
            }
        UartReadingBlocked (false);
        }
return rv;
}



bool TUARTBYTE::CheckPort (char *lpNames)
{
bool rv=0;
if (lpNames)
        {
        String nametmpprt = "\\\\.\\" + (String)lpNames;
        //ClosePort ();
        HANDLE lasth  = CreateFile (nametmpprt.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,0,0);
        if (lasth != INVALID_HANDLE_VALUE)
            {
            CloseHandle(lasth);
            ::Sleep (1);
            rv = 1;
            }
        }
return rv;
}



unsigned char TUARTBYTE::GetListFreePorts (TComboBox *c_box)
{
unsigned char rv = 0;
if (c_box)
    {
    c_box->Clear();
    unsigned short Indx = 1;
    String str_prt;
    UartReadingBlocked (true);
    while (Indx < 256)
        {
        str_prt = "COM";
        str_prt += LongToStr(Indx).c_str();
        if (CheckPort (str_prt.c_str()))
            {
            rv++;
            c_box->Items->Add(str_prt);
            }
        Indx++;
        }
    UartReadingBlocked (false);
    }
return rv;
}






bool TUARTBYTE::GetTransmitStatus ()
{
bool rv = false;
if (HandleOpenPort != INVALID_HANDLE_VALUE)
    {
    rv = false; // TransmitOk;
    }

return rv;
}



bool TUARTBYTE::SendData (char *lpRam, unsigned short sizes)
{
bool rv = false;
if (lpRam && sizes && is_open())
      {
      //PurgeComm (HandleOpenPort, PURGE_RXCLEAR | PURGE_TXCLEAR);
      CopyMemorySDC (lpRam, txbuf, sizes);
      SizeTransData = sizes;
      lpTransBufer = txbuf;
      TransmitOk = true;
      rv = true;
      }
return rv;
}



void TUARTBYTE::ISR_Add_Rx_data (char dat)
{
if (rx_size >= C_CANRXBUF_SIZE) return;
if (rx_push_ix >= C_CANRXBUF_SIZE) rx_push_ix = 0;

RecvBufer[rx_push_ix++] = dat;
rx_size++;
}



void TUARTBYTE::UartReadingBlocked (bool stat)
{

if (ThrRx_Start)
    {
    if (stat)
        {
        F_BlockUart_cmd = 1;
        while (!F_BlockUart_ack) {Sleep (1);};
        }
    else
        {
        F_BlockUart_cmd = 0;
        while (F_BlockUart_ack) {Sleep (1);};
        }
    }
else
    {
    F_BlockUart_cmd = stat;
    }

}




void TUARTBYTE::Task_Rx ()
{


    unsigned long c_tick = GetTickCount ();
    dlt_t_rxi = c_tick - bps_last_tick_rx;
    if (dlt_t_rxi >= 1000)
        {
        // подсчет rx bps
        bps_counter_rx = bps_counter_rx_local;
        bps_counter_rx_local = 0;
        bps_last_tick_rx = c_tick;
        }

    if (!F_BlockUart_cmd)
        {
        if (HandleOpenPort != INVALID_HANDLE_VALUE)
            {
            if (f_no_init_ov)
                {
                if (!SetCommMask (HandleOpenPort, EV_RXCHAR))
                    {
                    need_reconnect = EUSRTRCON_NEED;
                    return;
                    }
                //WaitCommEvent (HandleOpenPort, &mask_rxi, &readOL);
                f_no_init_ov = false;
                }


            if (true)       // semafore_sw != ECOMSW_TX
                {
                WaitCommEvent (HandleOpenPort, &mask_rxi, &readOL);

                DWORD modst;
                if (!GetCommModemStatus ( HandleOpenPort, &modst))
                    {
                    //MessageBox (0, "GetCommModemStatus", "", MB_OK);
                    need_reconnect = EUSRTRCON_NEED;
                    return;
                    }

                signal_rxi = WaitForSingleObject (readOL.hEvent, 400);          // 50
                switch (signal_rxi)
                    {
                    case WAIT_OBJECT_0:
                        {
                        if (GetOverlappedResult (HandleOpenPort, &readOL, &temp_rxi, true))        // wait - true
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
                                    MessageBox(0, "crit size", "", MB_OK);
                                    }

                                if (ReadFile (HandleOpenPort, lpRxDestRam, btr_rxi, &temp_rxi, &readOL))      // &readOL
                                  {
                                  if (f_trafic_enable) wr_trafic_log.WriteBlock (lpRxDestRam, temp_rxi);
                                  unsigned long ix = 0;
                                  bps_counter_rx_local += temp_rxi;         // подсчет rx bps
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
                            //need_reconnect = EUSRTRCON_NEED;
                            return;
                            }
                        //WaitCommEvent (HandleOpenPort, &mask_rxi, &readOL);
                        break;
                        }
                    case WAIT_FAILED:
                        {
                        //need_reconnect = EUSRTRCON_NEED;
                        //return;
                        //WaitCommEvent (HandleOpenPort, &mask_rxi, &readOL);
                        break;
                        }

                    default:
                        {
                        //WaitCommEvent (HandleOpenPort, &mask_rxi, &readOL);
                        break;
                        }
                    }
                }



            }
        }
}



void TUARTBYTE::Task_Tx ()
{
    unsigned long c_tick = GetTickCount (), dlt_t;
    dlt_t = c_tick - bps_last_tick_tx;
    if (dlt_t >= 1000)
        {
        // подсчет tx bps
        bps_counter_tx = bps_counter_tx_local;
        bps_counter_tx_local = 0;
        bps_last_tick_tx = c_tick;
        }

    if (HandleOpenPort != INVALID_HANDLE_VALUE)
        {
        if (TransmitOk  && lpTransBufer && SizeTransData)
            {
            bool f_rslt = WriteFile(HandleOpenPort, lpTransBufer, SizeTransData, &bctrns, &writeOL);
            DWORD signaled = WaitForSingleObject(writeOL.hEvent, INFINITE );        // 1000
            bool ret = signaled == WAIT_OBJECT_0 && GetOverlappedResult (HandleOpenPort, &writeOL, &bctrns, false);   
            if (ret)
                {
                TransmitOk = false;
                bps_counter_tx_local += bctrns;
                }
            }
        }
    else
        {
        Sleep (100);
        }
}



void TUARTBYTE::ClearRecvStatus ()
{
rx_push_ix = 0;
rx_pop_ix = 0;
rx_size = 0;
}



char TUARTBYTE::Pop_rx ()
{
char rv = 0;
if (rx_size)
    {
    if (rx_pop_ix >= C_CANRXBUF_SIZE) rx_pop_ix = 0;
    rv = RecvBufer[rx_pop_ix++];
    rx_size--;
    }
return rv;
}



unsigned long TUARTBYTE::GetOrCheck (char *lDst, unsigned long maxsz)
{
unsigned long rv = 0;
if (lDst && maxsz && rx_size)
    {
    unsigned long real_copy = rx_size;
    if (real_copy > maxsz) real_copy = maxsz;
    rv = real_copy;

    while (real_copy)
        {
        *lDst++ = Pop_rx ();
        real_copy--;
        }
    }
return rv;
}



unsigned long TUARTBYTE::GetRX_BPS ()
{
return bps_counter_rx;
}



unsigned long TUARTBYTE::GetTX_BPS ()
{
return bps_counter_tx;
}










