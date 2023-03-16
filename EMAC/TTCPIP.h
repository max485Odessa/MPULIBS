#ifndef _H_TCP_IP_STM32_H_
#define _H_TCP_IP_STM32_H_



#include "rutine.h"
#include "SYSBIOS.H"
#include "IPTCPDEFS.H"
#include "TMACIP.H"





enum ETCPSW {\
ETCPSW_SYNRECEIVED = 4,\
ETCPSW_ESTABLISHED = 5,\
ETCPSW_FINWAIT1 = 6,\
ETCPSW_CLOSEWAIT = 7,\
ETCPSW_FINWAIT2 = 8,\
ETCPSW_LASTACK = 9,\
ETCPSW_TIMEWAIT = 10,\
ETCPSW_CLOSING = 11,\
ETCPSW_ENDENUM = 12\
};



#ifdef  NOBORLAND
typedef __packed struct _TCP_SOCKET_T{
#else
typedef struct _TCP_SOCKET_T {
#endif 
	ETCPSW sw;
	S_MACIP_t macip_src;
	unsigned long seq_tx;
	unsigned long seq_rx;
	//unsigned long ip_src;			// входящий ip
	unsigned short wind_size;
	unsigned short src_port;	// входящий порт
	unsigned long sn;					// входящий sn
	unsigned long sn_ack;			// подтвержденный sn
	utimer_t TimeOut;
	unsigned short dst_port;
	//unsigned char mac_src[6];		// входящий mac
} TCP_SOCKET_T;



// интерфейсный обьект-сокет (сокет функтор), который передается протоколу верхнего уровня в момент приема данных на сокет
class TSOCKOBJ {
	protected:
		TCP_SOCKET_T *low_sock;
		void *recv_data;
		unsigned long sz_data;
		ETCPSTATE cur_statte;
		EMACIP *macip;
		SLPB tx_buf;
	
	public:
		virtual ETCPSTATE state () = 0;
		virtual void send (void *lsrc, unsigned long sz) = 0;
		virtual void close () = 0;
		virtual unsigned short size () = 0;
		virtual void *data () = 0;
};



class TSOCKCL: public TSOCKOBJ {
	public:
		TSOCKCL ();
		void Init (ETCPSTATE st, void *adr_rx, unsigned long sz_rx, TCP_SOCKET_T *lwsck, EMACIP *mcip, SLPB *tx_b);
		virtual ETCPSTATE state ();
		virtual void send (void *lsrc, unsigned long sz);
		virtual void close ();
		virtual unsigned short size ();
		virtual void *data ();
};



#define C_MAXTCP_AMOUNT 5


typedef void (*CB_TCPPORT_RX) (void *Obj, TSOCKOBJ *sock_n);

#define C_MAXTCPPORT_SLOT_AMOUNT 5

#ifdef  NOBORLAND
typedef __packed struct _CBPORTSLOT{
#else
typedef struct _CBPORTSLOT {
#endif 
	bool f_active;
	void *cb_obj;
	CB_TCPPORT_RX cb_rx;
	unsigned short port;
} CBPORTSLOT;






class TTCPIP {
	
		utimer_t timeout_timer;
		void init_timers ();
		
		CBPORTSLOT port_slots_rutines[C_MAXTCPPORT_SLOT_AMOUNT];
		void ClearRegSlots ();
		CBPORTSLOT *FindActiveRegSlot (unsigned short port_a);
		CBPORTSLOT *FindFreeRegSlot ();
	
		long FindSFree_indx ();
		long FindSToIp_indx (unsigned long ip_adr, unsigned short port);
		TCP_SOCKET_T *GetSoketFromIndx (long inx);
		void ClearS_indx (long indx);
		void ClearS ();
		void ClearSToIp (unsigned long ip_adr, unsigned short port);
		TCP_SOCKET_T *CreateSocket (S_MACIP_t *mac_in, S_TCP_T *tcp_in);		// Создает новый сокет на базе информации входящего TCP сегмента

	
		static void *create_tcp_header (void *ldst, TCP_SOCKET_T *cursock, bool f_opt);
		static unsigned short update_tcp_crc (void *maclevel);
		
		TCP_SOCKET_T cur_sock[C_MAXTCP_AMOUNT];
		//static S_TCP_MIT_T *CreateOutTCPFrame (S_MACIP_t *linp, TCP_SOCKET_T *cursock, bool f_opt);	
		
		static S_TCP_MIT_T *create_half_tcp (void *l_dst, TCP_SOCKET_T *cursock, bool f_opt);	// создает полузаполненный "mac-ip-tcp фрейм" без полей размера и crc
		unsigned short create_full_tcp (TCP_SOCKET_T *sck, void *data, unsigned short sz_tcp_data, unsigned char tcp_flags, bool f_opt_add);	// создает выходной фрейм используя параметры сокета
		
		unsigned char GetTCPFlags (S_TCP_T *flg);
		bool CheckTCPFlags (S_TCP_T *flg, unsigned char mask);
	
		//bool Transmit_TCP (void *lmac, unsigned short sz);		// обновляет контрольные суммы ip заголовков и отправляет пакет
	
		static unsigned char tx_buf[1024];
		bool f_is_tx;
		char *lpTx;
		unsigned short tx_size;
		void send_raw (unsigned short sz);		// передает указанный размер в сеть без анализа содержимого
		void send_tcp (unsigned short sz);		// обновляет поля хранящие размер данных в структурах macip и tcp, с учетом общего размера данных tcp сегмента 
	
		void Transmit ();
		
		static TTCPIP *SinglObj;
		void sock_timeout_task ();
		EMACIP *tx_ifc;
		TSOCKCL sock_singl;
		static unsigned short create_tcp_data_flags_frame (SLPB *l_dst, TCP_SOCKET_T *sck, void *data, unsigned short sz_tcp_data, unsigned char tcp_flags, bool f_opt_add);		// создает запись mac+ip+tcp+data
	
	public:
		TTCPIP ();
		void Init (EMACIP *txfc);
		void Task ();					// в основном контролируются таймауты открытых сокетов
	
		bool RegisteredPort (void *Obj, CB_TCPPORT_RX cb_data, unsigned short port_n);		// регистрирует протокол верхнего уровня на указанный порт
	
		bool in (void *lInp, unsigned short sz);		// входящий TCP трафик с вызовом call_back фукнкций по зарегистрированным номерам портов

		static S_TCP_T *get_tcp_field (void *lmac);
		static void *get_tcp_data_adr (void *lmac);
		static unsigned short get_tcp_data_size (void *lmac);
		static unsigned long get_tcp_header_size (void *maclevel);
		
		static unsigned short create_tcp_data_frame (SLPB *l_dst, TCP_SOCKET_T *sck, void *data, unsigned short sz);	// создает запись mac+ip+tcp+data
		static unsigned short create_tcp_close_frame (SLPB *l_dst, TCP_SOCKET_T *sck);	// создает запись mac+ip+tcp+data с флагом FIN
		static void set_tcp_flag (S_TCP_T *flg, unsigned char flag);
};


#endif

