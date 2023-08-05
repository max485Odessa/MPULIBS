#include "RS232BYTE.H"



TUARTBYTE::TUARTBYTE ()
{
//semafore_sw = ESWRRD_RX;
HandleOpenPort = INVALID_HANDLE_VALUE;
lpTransBufer = 0;
F_BlockUart_cmd = 0;
F_BlockUart_ack = 0;
TransmitOk = false;
ClearRecvStatus ();
F_THREAD_trans_NeedClose = 0;
F_THREAD_recv_NeedClose = 0;
ThrRx_Start = false;
f_no_init_ov = true;
ThreadHandlRecv_RS485 = 0;
ThreadHandlTransm_RS485 = 0;
StrartTHREADRecv_RS485 ();
StrartTHREADTransm_RS485 ();
}


bool TUARTBYTE::is_open ()
{
return (HandleOpenPort != INVALID_HANDLE_VALUE);
}


void TUARTBYTE::CloseExitAll ()
{
while (GetTransmitStatus ()) {Sleep(1);};
CloseTHREADRecv_RS485 ();
CloseTHREADTransm_RS485 ();
ClosePort ();
}


TUARTBYTE::~TUARTBYTE ()
{
CloseExitAll ();
}



void TUARTBYTE::CloseTHREADRecv_RS485 ()
{
if (ThreadHandlRecv_RS485)
    {
    F_THREAD_recv_NeedClose = 1;
    /*
    while (F_THREAD_recv_NeedClose)
        {
        Sleep(1);
        }
    */
    ThreadHandlRecv_RS485=0;
    }
}



void TUARTBYTE::CloseTHREADTransm_RS485 ()
{
if (ThreadHandlTransm_RS485)
    {
    F_THREAD_trans_NeedClose = 1;

    while (F_THREAD_trans_NeedClose)
        {
        Sleep(1);
        }

    ThreadHandlTransm_RS485=0;
    }
}



void TUARTBYTE::ClosePort ()
{
if (HandleOpenPort != INVALID_HANDLE_VALUE)
    {
    HANDLE lasth =  HandleOpenPort;
    HandleOpenPort = INVALID_HANDLE_VALUE;
    while (!CloseHandle(lasth))
            {
            Sleep (1);
            }
    f_no_init_ov = true;
    }

}


HANDLE TUARTBYTE::StrartTHREADRecv_RS485 ()
{
F_THREAD_recv_NeedClose = 0;
ThreadHandlRecv_RS485 = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThreadRecvProcedure,this,THREAD_TERMINATE | THREAD_SUSPEND_RESUME  ,&IdThreadRecv); //  THREAD_TERMINATE | THREAD_SUSPEND_RESUME  THREAD_ALL_ACCESS
SetThreadPriority (ThreadHandlRecv_RS485, THREAD_PRIORITY_NORMAL );     // THREAD_PRIORITY_HIGHEST

Sleep(2);
return ThreadHandlRecv_RS485;
}



HANDLE TUARTBYTE::StrartTHREADTransm_RS485 ()
{
F_THREAD_trans_NeedClose = 0;
ThreadHandlTransm_RS485 = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThreadTransmitProcedure,this,THREAD_TERMINATE | THREAD_SUSPEND_RESUME  ,&IdThreadTransm); // THREAD_TERMINATE | THREAD_SUSPEND_RESUME THREAD_ALL_ACCESS
SetThreadPriority(ThreadHandlTransm_RS485, THREAD_PRIORITY_NORMAL);

Sleep(2);
return ThreadHandlTransm_RS485;
}



bool TUARTBYTE::UpdateUartParam (HANDLE PortLocal1,DWORD ubrrsets)
{
bool rv=1;

        //GetCommState(PortLocal1, &MyDCB);

        MyDCB.DCBlength = sizeof(DCB);
        MyDCB.BaudRate = ubrrsets;

        MyDCB.fBinary = true;
        MyDCB.fParity = false;
        MyDCB.fOutxCtsFlow = false;
        MyDCB.fOutxDsrFlow = false;
        MyDCB.fDtrControl = DTR_CONTROL_DISABLE;
        MyDCB.fDsrSensitivity = false;
        MyDCB.fTXContinueOnXoff = false;//true;
        MyDCB.fOutX = false;
        MyDCB.fInX = false;
        MyDCB.fErrorChar = false;
        MyDCB.fNull = false;
        MyDCB.fRtsControl = RTS_CONTROL_DISABLE;
        MyDCB.fAbortOnError = false;
        MyDCB.fDummy2 = 0;
        MyDCB.XonLim = 2048;  // 2048
        MyDCB.XoffLim = 2048;  // 2048
        MyDCB.ByteSize = 8;
        MyDCB.Parity = NOPARITY;//PARITY_EVEN;// PARITY_EVEN;//NOPARITY;
        MyDCB.StopBits = ONESTOPBIT;//TWOSTOPBITS;//  ONESTOPBIT;
        MyDCB.XonChar = 1;
        MyDCB.XoffChar = 2;
        MyDCB.ErrorChar = 3;
        MyDCB.EofChar = 4;
        MyDCB.EvtChar = 5;
        MyDCB.wReserved1 = 0;
        if (!SetCommState(PortLocal1,&MyDCB)) rv=0;


        MyTimeout.ReadIntervalTimeout=1;
        MyTimeout.ReadTotalTimeoutMultiplier=1;
        MyTimeout.ReadTotalTimeoutConstant=1;
        MyTimeout.WriteTotalTimeoutMultiplier=1;
        MyTimeout.WriteTotalTimeoutConstant=1;

        if (!SetCommTimeouts(PortLocal1,&MyTimeout)) rv=0;
        PurgeComm (HandleOpenPort, PURGE_RXCLEAR | PURGE_TXCLEAR);

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
        HandleOpenPort = CreateFile (nametmpprt.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED ,0); // FILE_FLAG_OVERLAPPED FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED     // FILE_SHARE_READ | FILE_SHARE_WRITE    = GENERIC_READ | GENERIC_WRITE
        if (HandleOpenPort != INVALID_HANDLE_VALUE)
            {
            UpdateUartParam (HandleOpenPort,UarrSpeed);
            if (ThreadHandlRecv_RS485 && ThreadHandlTransm_RS485)
                {
                f_no_init_ov = true;
                rv = 1;
                }
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
        HANDLE lasth  = CreateFile (nametmpprt.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING,0,0);
        if (lasth != INVALID_HANDLE_VALUE)
            {
            CloseHandle(lasth);
            Sleep (1);
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
    unsigned char Indx = 1;
    String str_prt;
    UartReadingBlocked (true);
    while (Indx < 32)
        {
        str_prt = "COM" + IntToStr(Indx);
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




void TUARTBYTE::ThreadTrans ()
{
unsigned long dlt_t;
OVERLAPPED writeOL;
  memset(&writeOL, 0, sizeof(writeOL));
  writeOL.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
bps_last_tick_tx = GetTickCount ();
while (!F_THREAD_trans_NeedClose)
    {
    unsigned long c_tick = GetTickCount ();
    dlt_t = c_tick - bps_last_tick_tx;
    if (dlt_t >= 1000)
        {
        // подсчет tx bps
        bps_counter_tx = bps_counter_tx_local;
        bps_counter_tx_local = 0;
        bps_last_tick_tx = c_tick;
        }

    if (HandleOpenPort != INVALID_HANDLE_VALUE && lpTransBufer && SizeTransData)
        {
        if (TransmitOk)
            {

	        WriteFile(HandleOpenPort, lpTransBufer, SizeTransData, &bctrns, &writeOL);
	        DWORD signaled = WaitForSingleObject(writeOL.hEvent, INFINITE);
	        bool ret = signaled == WAIT_OBJECT_0 && GetOverlappedResult(HandleOpenPort, &writeOL, &bctrns, FALSE);
            bps_counter_tx_local += bctrns;
            Sleep (3);
            TransmitOk = false;
            //Sleep (1); 
            }
        else
            {
            //Sleep (1);
            }
        }
    else
        {
        //Sleep (1);
        }
    //Sleep (1);
    }
CloseHandle(writeOL.hEvent);
F_THREAD_trans_NeedClose = false;
}



bool TUARTBYTE::GetTransmitStatus ()
{
bool rv = false;
if (HandleOpenPort != INVALID_HANDLE_VALUE) rv = TransmitOk;
return rv;
}



bool TUARTBYTE::SendData (char *lpRam, unsigned short sizes)
{
bool rv = false;
if (lpRam && sizes)
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



unsigned long WINAPI TUARTBYTE::ThreadTransmitProcedure(unsigned long *lpData)
{
TUARTBYTE *lpObj=(TUARTBYTE *)lpData;
lpObj->ThreadTrans();
return 0;
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



void TUARTBYTE::ThreadRecv ()
{
unsigned char BufDaatTh[C_CANTXBUF_SIZE];                                                             // буфер для хранения одного байта
unsigned char *lpDestRam = BufDaatTh;                                                               // указатель на буфер
unsigned long rcvsizee, dlt_t;
ThrRx_Start = true;
bool f_sync = false;
bps_last_tick_rx = GetTickCount ();

OVERLAPPED readOL;
DWORD btr, temp, mask, signal;
COMSTAT comstat;
memset(&readOL, 0, sizeof(readOL));

  readOL.Internal = readOL.InternalHigh = readOL.Offset = readOL.OffsetHigh = 0;
  readOL.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);

while (!F_THREAD_recv_NeedClose)
    {
    unsigned long c_tick = GetTickCount ();
    dlt_t = c_tick - bps_last_tick_rx;
    if (dlt_t >= 1000)
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
                SetCommMask (HandleOpenPort, EV_RXCHAR);

                f_no_init_ov = false;
                }

            WaitCommEvent (HandleOpenPort, &mask, &readOL);
            signal = WaitForSingleObject (readOL.hEvent, INFINITE);
            if (signal == WAIT_OBJECT_0)
                {
                if(GetOverlappedResult (HandleOpenPort, &readOL, &temp, true))
                    {
                    if((mask & EV_RXCHAR)!=0)
                        {
                        ClearCommError (HandleOpenPort, &temp, &comstat);
                        btr = comstat.cbInQue;
                        if (btr > sizeof(BufDaatTh)) btr = sizeof(BufDaatTh);
                        if (btr)
                          {
                          ReadFile (HandleOpenPort, BufDaatTh, btr, &temp, &readOL);
                          unsigned long ix = 0;
                          bps_counter_rx_local += btr;         // подсчет rx bps
                          while (btr)
                              {
                              ISR_Add_Rx_data (lpDestRam[ix]);
                              btr--;
                              ix++;
                              }
                          //Sleep (1);
                          }
                        }
                    }
                }
            //Sleep (1);
            }
        else
            {
            Sleep (1);
            }
        }
    else
        {
        Sleep (1);
        }




    if (F_BlockUart_ack != F_BlockUart_cmd)
        {
        F_BlockUart_ack = F_BlockUart_cmd;
        }
    }
CloseHandle(readOL.hEvent);
F_THREAD_recv_NeedClose = false;
}



unsigned long WINAPI TUARTBYTE::ThreadRecvProcedure(unsigned long *lpData)
{
TUARTBYTE *lpTMRS485=(TUARTBYTE *)lpData;
lpTMRS485->ThreadRecv ();
return 0;
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
    unsigned long real_copy = maxsz;
    if (rx_size < maxsz) real_copy = rx_size;
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










