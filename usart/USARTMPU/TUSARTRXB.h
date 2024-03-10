#ifndef _H_TUSART_RXBYTE_STM32_H_
#define _H_TUSART_RXBYTE_STM32_H_




#include "stm32l4xx_hal.h"
#include "rutine.h"
#include "resources.h"




class TUSARTIFB {
	
	protected:
		//BUFPAR tx_buf;
		unsigned long cur_speed;
		//USART_InitTypeDef USART_InitStructure;
		GPIO_InitTypeDef GPIO_Usart_InitStructure;

		void Usart_ISR ();
	
		USART_TypeDef *USARTPort;
		bool f_tx_status;
		
	public:
		unsigned short tx_size;
		unsigned char *lTxAdr;
		static TUSARTIFB *lFS_PUSART[EUSARTIX_ENDENUM];
		static void Usart_ISR_hard (TUSARTIFB *obj);
		virtual void ISR_TxEnd_cb ();
		void TransmitData (void *lTx, unsigned short sz_tx);
		bool GetTtansmitStatus ();
		void Set_usart_speed (unsigned long c_speed);
		void Update_usart_sets ();
	
		virtual void ISR_DataRx (unsigned char dat) = 0;
	
};






#endif

