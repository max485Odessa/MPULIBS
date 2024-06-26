#ifndef _H_TUSARTDATA_STM32_H_
#define _H_TUSARTDATA_STM32_H_



#include "hard_rut.h"
#include "rutine.h"
#include "TTFIFO.h"
#include "SYSBIOS.H"
#include "THIZIF.h"



#define C_USART_SPEED_DEF 57600
#if (HRDCPU == 1)
enum ESYSUSART {ESYSUSART_1 = 0, ESYSUSART_2 = 1, ESYSUSART_3 = 2, ESYSUSART_ENDENUM = 3};
#endif
#if (HRDCPU == 4)
enum ESYSUSART {ESYSUSART_1 = 0, ESYSUSART_2 = 1, ESYSUSART_3 = 2, ESYSUSART_4 = 3, ESYSUSART_5 = 4, ESYSUSART_6 = 5, ESYSUSB_1 = 6, ESYSUSART_ENDENUM = 7};
#endif

enum EUSARTPINIX {EUSARTPINIX_TX = 0, EUSARTPINIX_RX = 1, EUSARTPINIX_ENDENUM = 2};



typedef struct {
	S_GPIOPIN pins[EUSARTPINIX_ENDENUM];
	USART_TypeDef *usart;
	uint8_t af_mux_id;
} S_USARTPINS_T;



class TSERIALISR {
	protected:
		ESYSUSART c_port_ix;
	
	public:
		static TSERIALISR *ifc[ESYSUSART_ENDENUM];
		virtual void isr_rx (uint8_t d) = 0;
		virtual bool isr_tx (uint8_t &d) = 0;
		USART_TypeDef *USARTPort;
};



class TSERIALUSR {
	protected:
		TTFIFO<uint8_t> *fifo_tx;
		TTFIFO<uint8_t> *fifo_rx;
		SYSBIOS::Timer timer_link;
		uint32_t c_linkdetect_period;
	
	public:
		virtual bool Tx (void *lTx, uint16_t sz_tx) = 0;
		bool push (uint8_t d);
		virtual bool is_link () = 0;
		uint32_t tx_free_space ();
		virtual bool Tx_status () = 0;
		void clear ();
		bool pop (uint8_t &d);
		virtual uint16_t Rx_check () = 0;
		virtual uint16_t Rx (void *lDst, uint16_t max_size) = 0;
		void SetLinkDetectPeriod (uint32_t t);
	
		#ifdef SERIALDEBAG
			uint32_t debug_peak_tx ();
			uint32_t debug_peak_rx ();
			void debug_peak_clear ();
		#endif
};



class TUSARTOBJ: public TSERIALISR, public TSERIALUSR, public THIZIF {
	protected:
		uint32_t cur_speed;
		virtual void isr_rx (uint8_t d) override;
		virtual bool isr_tx (uint8_t &d) override;
		UART_HandleTypeDef UartHandle;
		void rx_clear ();
		bool f_tx_status;
		
		void Init ();
		virtual void thizif_hiz_outputs (bool f_act_hiz) override;
		
	public:
		TUSARTOBJ (ESYSUSART prt, uint32_t sz_b_tx, uint32_t sz_b_rx);
		void SetSpeed (uint32_t c_speed);
		
		virtual bool is_link () override;
		virtual bool Tx (void *lTx, uint16_t sz_tx) override;
		virtual bool Tx_status () override;
		virtual uint16_t Rx_check () override;
		virtual uint16_t Rx (void *lDst, uint16_t max_size) override;
};






#endif












