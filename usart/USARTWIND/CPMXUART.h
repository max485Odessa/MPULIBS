#ifndef _H_CAPMUX_UART_H_
#define _H_CAPMUX_UART_H_



#include <vcl.h>
#include <stdint.h>
#include "rutine.h"
#include "TUSARTSTREAMDEF.H"
#include "capsulcore.h"
//#include "TTFIFO.h"



enum EUSRTRCON {EUSRTRCON_NONE = 0, EUSRTRCON_NEED, EUSRTRCON_ENDENUM};


class TCPMXUART: protected TCAPMUX {
    private:
        HANDLE rxMutexHandle;

        HANDLE HandleOpenPort;
        _DCB MyDCB;
        _COMMTIMEOUTS MyTimeout;

        OVERLAPPED writeOL;
        OVERLAPPED readOL;
        COMSTAT comstat;
        COMSTAT comstatrx;

        const uint32_t c_sz_raw_rx_buf;
        const uint32_t c_sz_raw_tx_buf;
        uint8_t *lp_rxtmp_buf;
        uint8_t *lp_txtmp_buf;

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


        EUSRTRCON need_reconnect;
        void reconnect_task ();
        uint32_t lastbaud_rate;
        String last_strportopen;
        bool Reconnect ();


        bool txif_out (void *src, uint32_t sz);


    public:
        TCPMXUART (uint32_t rxsz, uint32_t tx, uint8_t micnt);
        ~TCPMXUART ();


        unsigned char GetListFreePorts (TComboBox *c_box);
        bool is_opened ();
        bool is_recon_status ();

        bool OpenPort (char *lpNames, unsigned long UarrSpeed);
        bool OpenPort (uint16_t n, uint32_t UarrSpeed);
        bool CheckPort (char *lpNames);
        void ClosePort ();

        unsigned long GetRX_BPS ();
        unsigned long GetTX_BPS ();

        bool AddOrReplace_rx_cb (uint32_t mxid, TCPMXRXCB *mcb);
        bool push (uint16_t mux, void *s, uint32_t sz);
        void Task ();

};




#endif
