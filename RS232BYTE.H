#ifndef _H_UART232_BYTESTAFF_H_
#define _H_UART232_BYTESTAFF_H_

/*
Класс канального уровня
*/

#include <vcl.h>
#include "textrut.hpp"


const unsigned long C_CANRXBUF_SIZE = 32768;
const unsigned long C_CANTXBUF_SIZE = 32768;

class TUARTBYTE {
    private:
        HANDLE HandleOpenPort;
        _DCB MyDCB;
        _COMMTIMEOUTS MyTimeout;

        unsigned long rx_push_ix;
        unsigned long rx_pop_ix;
        unsigned long rx_size;

        unsigned char F_BlockUart_cmd;
        unsigned char F_BlockUart_ack;
        unsigned char *lpTransBufer;
        unsigned int SizeTransData;
        bool TransmitOk;

        char F_THREAD_trans_NeedClose;
        char F_THREAD_recv_NeedClose;
        unsigned long IdThreadRecv;
        unsigned long IdThreadTransm;
        HANDLE ThreadHandlRecv_RS485;
        HANDLE ThreadHandlTransm_RS485;

        void UartReadingBlocked (bool stat);

        static unsigned long WINAPI ThreadRecvProcedure(unsigned long *lpData);
        void ThreadRecv ();

        bool UpdateUartParam (HANDLE PortLocal1,DWORD ubrrsets);
        static unsigned long WINAPI ThreadTransmitProcedure(unsigned long *lpData);
        void ThreadTrans ();

        //bool MPWriteFile(HANDLE MyPort1,void *TransmitBufer1,DWORD sizes,DWORD *bc);
        DWORD bctrns;
        HANDLE StrartTHREADRecv_RS485 ();
        HANDLE StrartTHREADTransm_RS485 ();

        void CloseTHREADRecv_RS485 ();
        void CloseTHREADTransm_RS485 ();
        void ClearRecvStatus ();

        bool f_no_init_ov;
        void ISR_Add_Rx_data (char dat);
        char Pop_rx ();
        void TmpAddReadData (char dat);

        bool ThrRx_Start;

        char txbuf[C_CANTXBUF_SIZE];
        char RecvBufer[C_CANRXBUF_SIZE];

        //ESWRRD semafore_sw;

        unsigned long bps_last_tick_rx;
        unsigned long bps_last_tick_tx;

        unsigned long bps_counter_rx_local;
        unsigned long bps_counter_tx_local;
        unsigned long bps_counter_rx;
        unsigned long bps_counter_tx;


    public:
        TUARTBYTE ();
        ~TUARTBYTE ();

        bool is_open ();
        unsigned char GetListFreePorts (TComboBox *c_box);

        void CloseExitAll ();

        bool OpenPort (char *lpNames, unsigned long UarrSpeed);
        bool CheckPort (char *lpNames);
        void ClosePort ();

        //bool Tx_lock (bool val);
		bool SendData (char *lSrc, unsigned short sizes);
		bool GetTransmitStatus ();

        unsigned long GetOrCheck (char *lDst, unsigned long maxsz);
        unsigned long GetRX_BPS ();
        unsigned long GetTX_BPS ();

};


#endif


 