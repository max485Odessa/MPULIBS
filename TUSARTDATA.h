#ifndef _H_TUSARTDATA_STM32_H_
#define _H_TUSARTDATA_STM32_H_



#include "hard_rut.h"
#include "rutine.h"
#include "TTFIFO.h"


#define C_USART_SPEED_DEF 57600
enum ESYSUSART {ESYSUSART_1 = 0, ESYSUSART_2 = 1, ESYSUSART_3 = 2, ESYSUSART_4 = 3, ESYSUSART_5 = 4, ESYSUSART_6 = 5, ESYSUSART_ENDENUM};
enum EUSARTPINIX {EUSARTPINIX_TX = 0, EUSARTPINIX_RX = 1, EUSARTPINIX_ENDENUM = 2};

typedef struct {
	S_GPIOPIN pins[EUSARTPINIX_ENDENUM];
	USART_TypeDef *usart;
	uint8_t af_mux_id;
} S_USARTPINS_T;

class TUSARTISR {
	protected:
		ESYSUSART c_port_ix;
	
	public:
		virtual void isr_rx (uint8_t d) = 0;
		virtual bool isr_tx (uint8_t &d) = 0;
		static TUSARTISR *ifc[ESYSUSART_ENDENUM];
		USART_TypeDef *USARTPort;
};



class TUSARTOBJ: public TUSARTISR {
	protected:
		uint32_t cur_speed;
		//USART_InitTypeDef USART_InitStructure;
		UART_HandleTypeDef UartHandle;
		
		TTFIFO<uint8_t> *fifo_tx;
		TTFIFO<uint8_t> *fifo_rx;
	
		virtual void isr_rx (uint8_t d) override;
		virtual bool isr_tx (uint8_t &d) override;
	
		void rx_clear ();
		bool f_tx_status;
	
		void Init ();

		
		
	public:
		TUSARTOBJ (ESYSUSART prt, uint32_t sz_b_tx, uint32_t sz_b_rx);
		void SetSpeed (uint32_t c_speed);
		
		bool Tx (void *lTx, uint16_t sz_tx);
		bool Tx_status ();
	
		uint16_t Rx_check ();
		uint16_t Rx (void *lDst, uint16_t max_size);

};





#endif












