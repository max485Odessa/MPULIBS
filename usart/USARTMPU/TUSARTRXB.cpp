#include "TUSARTRXB.h"



TUSARTIFB* TUSARTIFB::lFS_PUSART[EUSARTIX_ENDENUM];



#ifdef __cplusplus
 extern "C" {
#endif 


void USART1_IRQHandler ()
{
	TUSARTIFB::Usart_ISR_hard (TUSARTIFB::lFS_PUSART[EUSARTIX_SBUS]);
}


void USART2_IRQHandler ()
{
	TUSARTIFB::Usart_ISR_hard (TUSARTIFB::lFS_PUSART[EUSARTIX_ADSB]);
}



void USART3_IRQHandler ()
{
	TUSARTIFB::Usart_ISR_hard (TUSARTIFB::lFS_PUSART[EUSARTIX_GPS]);
}


#ifdef __cplusplus
}
#endif




void TUSARTIFB::Usart_ISR_hard (TUSARTIFB *obj)
{
	obj->Usart_ISR ();
}



void TUSARTIFB::Usart_ISR ()
{
	unsigned short flag_cr;// = USARTPort->CR1;
	unsigned short flag_it;// = USARTPort->SR;
	unsigned char dat;  //=USARTPort->DR;
	
	
  if (flag_it & 32)	
		{
    ISR_DataRx (dat);
		}
  
	if (flag_cr & 0x80)
		{
		if (flag_it & 0x80)
			{
			if (tx_size)
				{
				//USARTPort->DR = *lTxAdr;
				lTxAdr++;
				tx_size--;
				}
			else
				{
				//USART_ITConfig (USARTPort, USART_IT_TXE, DISABLE);
				//USART_ClearITPendingBit (USARTPort, USART_IT_TXE);
				f_tx_status = false;
				ISR_TxEnd_cb ();
				}
			}
		}
}



void TUSARTIFB::ISR_TxEnd_cb ()
{
}




bool TUSARTIFB::GetTtansmitStatus ()
{
	return f_tx_status;
}



void TUSARTIFB::TransmitData (void *lTx, unsigned short sz_tx)
{
	/*
	lTxAdr = (unsigned char *)lTx;
	tx_size = sz_tx;
	if (sz_tx) 
		{
		f_tx_status = true;
		USART_ClearITPendingBit (USARTPort, USART_IT_TXE);
		USART_ITConfig (USARTPort, USART_IT_TXE, ENABLE);
		}
	*/
}




void TUSARTIFB::Set_usart_speed (unsigned long c_speed)
{
	/*
	cur_speed = c_speed;
  USART_InitStructure.USART_BaudRate = cur_speed;
	*/
	/*
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USARTPort, &USART_InitStructure);
	*/
}



void TUSARTIFB::Update_usart_sets ()
{
	//USART_Init(USARTPort, &USART_InitStructure);
}




