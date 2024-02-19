#ifndef _H_UART232_STREAM_IMPL_H_
#define _H_UART232_STREAM_IMPL_H_



#include <vcl.h>
#include <stdint.h>
#include "rutine.h"
#include "TUSARTSTREAMDEF.H"
#include "TTFIFO.h"



enum EUSRTRCON {EUSRTRCON_NONE = 0, EUSRTRCON_NEED, EUSRTRCON_ENDENUM};


class TUARTSTREAM: public TRXIF, public TTXIF {
    private:
        HANDLE HandleOpenPort;
        _DCB MyDCB;
        _COMMTIMEOUTS MyTimeout;

        OVERLAPPED writeOL;
        OVERLAPPED readOL;
        COMSTAT comstat;
        COMSTAT comstatrx;
        unsigned long c_size_tmp_buf;
        unsigned char *lpRxDestRam;

        TTFIFO<uint8_t> *fifo_rx;

        unsigned long rcvsizee_rxi, dlt_t_rxi;
        DWORD btr_rxi, temp_rxi, mask_rxi, signal_rxi;

        bool f_no_init_ov;



        bool UpdateUartParam (HANDLE PortLocal1,DWORD ubrrsets);


        bool MPWriteFile(HANDLE MyPort1,void *TransmitBufer1,DWORD sizes,DWORD *bc);
        DWORD bctrns;

        unsigned long bps_last_tick_rx;
        unsigned long bps_last_tick_tx;
        unsigned long bps_counter_rx_local;
        unsigned long bps_counter_tx_local;
        unsigned long bps_counter_rx;
        unsigned long bps_counter_tx;


        void Task_Rx ();
        //void Task_Tx ();



        EUSRTRCON need_reconnect;
        void reconnect_task ();
        uint32_t lastbaud_rate;
        String last_strportopen;
        bool Reconnect ();


        virtual uint32_t rxif_in (void *dst, uint32_t max_sz);
        virtual bool txif_out (void *src, uint32_t sz);
        virtual uint32_t txif_free_space ();
        virtual uint32_t rxif_is_rx ();

        void rxrata_to_fifo (void *src, uint32_t sz);

    public:
        TUARTSTREAM (uint32_t szrx);
        ~TUARTSTREAM ();
        void Task ();


        unsigned char GetListFreePorts (TComboBox *c_box);
        bool is_opened ();
        bool OpenPort (char *lpNames, unsigned long UarrSpeed);
        bool OpenPort (uint16_t n, uint32_t UarrSpeed);
        bool CheckPort (char *lpNames);
        void ClosePort ();

        unsigned long GetRX_BPS ();
        unsigned long GetTX_BPS ();
};


#endif


 