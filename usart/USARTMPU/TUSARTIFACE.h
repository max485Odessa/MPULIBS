#ifndef _H_STM32_USART_IFACE_H_
#define _H_STM32_USART_IFACE_H_



//#include "stm32l4xx_ll_usart.h"
#include "stm32l4xx_hal.h"
#include "resources.h"


//enum EUSARTINDX {EUSARTINDX_NONE = 0, EUSARTINDX_GPS = 1, EUSARTINDX_RTCM = 2, EUSARTINDX_MAX = 3};



class TUSART_IFACE
{
	protected:
		unsigned short tx_size;
		unsigned char *lTxAdr;
		bool f_tx_status;
		IRQn_Type nvic_irq;
	
		void USART_ITConfig_TX (bool val);
		void USART_ITConfig_RX (bool val);
		bool ISR_DataTx (unsigned char *lDat);
		void Usart_ISR ();
		virtual void ISR_TxEnd_cb ();
	
	public:
		//USART_TypeDef *USARTPort;
	
		UART_HandleTypeDef huart;
		static TUSART_IFACE *lFS_PUSART[EUSARTIX_ENDENUM];
		static void Usart_ISR_hard (TUSART_IFACE *obj);
		//static const IRQn_Type nvic_irq_tab[EUSARTIX_ENDENUM];
	
		void RawTransmit (unsigned char *lTx, unsigned short sizes);
		bool GetTxStatus ();
		virtual void ISR_DataRx (unsigned char dat) = 0;
	
		void Set_usart_speed (unsigned long c_speed);			// устанавливает параметры скорости (не применяет)
		void Update_usart_sets ();	// применяет текущие настройки порта
		void NVIC_USART_ENABLE (bool stat);
		void USART_Cmd (FunctionalState NewState);

	
};



#endif

