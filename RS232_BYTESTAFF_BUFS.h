#ifndef _H_UART232_BSTAFF_BUFS_H_
#define _H_UART232_BSTAFF_BUFS_H_

/*
Класс канального уровня
*/

#include <vcl.h>
#include <stdint.h>
#include "rutine.h"
enum E_STAFFCMDS {E_SFCMD_SYNC = 1, E_SFCMD_SYNC_STAFF = 2, E_SFCMD_DATA = 3, E_SFCMD_DATA_STAFF = 4};

const unsigned long C_USART_BUFSIZE = 1024;
const unsigned char C_USART_MAXBUF_AMOUNT = 5;

typedef struct {
    unsigned char data[C_USART_BUFSIZE];
    unsigned short indx_recv;      // индекс заполнения буфера, он же и размер
    bool f_full;                        // флаг что буфер заполнен
} SUSARTBUFER;



enum EUSRTRCON {EUSRTRCON_NONE = 0, EUSRTRCON_NEED, EUSRTRCON_ENDENUM};


class TUARTBSTAFBUF {
    private:
        E_STAFFCMDS Inp_Stat;							// текущее состояние приема данных данных
        HANDLE HandleOpenPort;
        _DCB MyDCB;
        _COMMTIMEOUTS MyTimeout;

        OVERLAPPED writeOL;
        OVERLAPPED readOL;
        COMSTAT comstat;
        COMSTAT comstatrx;
        unsigned long c_size_tmp_buf;
        unsigned char *lpRxDestRam;

        SUSARTBUFER RecvBufer[C_USART_MAXBUF_AMOUNT];
        unsigned char recv_bufers_count;        // указывает сколько буферов звполнено
        unsigned char recv_push_bufers_indx;    // указатель текущего заполняемого буфера
        unsigned char recv_pop_bufers_indx;     // указатель буфера чтения
        unsigned long rcvsizee_rxi, dlt_t_rxi;
        DWORD btr_rxi, temp_rxi, mask_rxi, signal_rxi;

        unsigned char TransmitBufer[C_USART_BUFSIZE * 2];
        bool f_no_init_ov;



        bool UpdateUartParam (HANDLE PortLocal1,DWORD ubrrsets);


        bool MPWriteFile(HANDLE MyPort1,void *TransmitBufer1,DWORD sizes,DWORD *bc);
        DWORD bctrns;


        void ISR_Add_Rx_data (unsigned char dat);
        void TmpAddReadData (char dat);

        unsigned long bps_last_tick_rx;
        unsigned long bps_last_tick_tx;

        unsigned long bps_counter_rx_local;
        unsigned long bps_counter_tx_local;
        unsigned long bps_counter_rx;
        unsigned long bps_counter_tx;

        unsigned char CalculateCRC8_ByteStaff (unsigned char *lSrc, unsigned short sizes);
        unsigned short UartSTAFF_Coding (unsigned char *lSrc, unsigned char *lDst, unsigned short sizes);
        void Task_Rx ();

        //virtual void receive_complete (bool f_timeout) = 0;

        bool f_rx_timeout_on;
        unsigned long timeout_tick_value;

        EUSRTRCON need_reconnect;
        void reconnect_task ();
        uint32_t lastbaud_rate;
        String last_strportopen;
        bool Reconnect ();

    public:
        TUARTBSTAFBUF ();
        ~TUARTBSTAFBUF ();
        void Task ();


        unsigned char GetListFreePorts (TComboBox *c_box);
        bool is_opened ();
        bool OpenPort (char *lpNames, unsigned long UarrSpeed);
        bool CheckPort (char *lpNames);
        void ClosePort ();

        bool SendData (char *lSrc, unsigned short sizes);
        bool SendDataRXTime (char *lSrc, unsigned short sizes, unsigned long tcks);

        unsigned short CheckReadData ();                                    // узнать сколько принятых данных
        unsigned short ReadData (char *lDst, unsigned short sizes);         // прочитать данные

        //unsigned long GetOrCheck (char *lDst, unsigned long maxsz);
        unsigned long GetRX_BPS ();
        unsigned long GetTX_BPS ();
};


#endif


 