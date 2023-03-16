#include "TUSARTIFACE.h"
#include "resources.h"



//UART_HandleTypeDef TUSART_IFACE::huarts[EUSARTIX_ENDENUM];
TUSART_IFACE *TUSART_IFACE::lFS_PUSART[EUSARTIX_ENDENUM];



#ifdef __cplusplus
 extern "C" {
#endif 


void USART1_IRQHandler ()
{
	TUSART_IFACE::Usart_ISR_hard (TUSART_IFACE::lFS_PUSART[EUSARTIX_GPS]);
}


void USART2_IRQHandler ()
{
	TUSART_IFACE::Usart_ISR_hard (TUSART_IFACE::lFS_PUSART[EUSARTIX_SBUS]);
}



void LPUART1_IRQHandler ()
{
	TUSART_IFACE::Usart_ISR_hard (TUSART_IFACE::lFS_PUSART[EUSARTIX_ADSB]);
}


#ifdef __cplusplus
}
#endif



void TUSART_IFACE::Usart_ISR_hard (TUSART_IFACE *obj)
{
	obj->Usart_ISR ();
}



void TUSART_IFACE::NVIC_USART_ENABLE (bool stat)
{
	if (stat)
		{
		NVIC_EnableIRQ (nvic_irq);
		}
	else
		{
		NVIC_DisableIRQ (nvic_irq);
		}
}



void TUSART_IFACE::Set_usart_speed (unsigned long c_speed)
{
	huart.Init.BaudRate = c_speed;
}



void TUSART_IFACE::Update_usart_sets ()
{
	UART_SetConfig (&huart);
}



void TUSART_IFACE::ISR_TxEnd_cb ()
{
	
}



void TUSART_IFACE::Usart_ISR ()
{
	unsigned short flag_cr = huart.Instance->CR1;
	unsigned short flag_it = huart.Instance->ISR;
	unsigned char dat = huart.Instance->RDR;
	
  if (flag_it & (1<<5))
		{
    ISR_DataRx (dat);
		}
  
	if (flag_cr & (1<<7))
		{
		if (flag_it & (1<<7))
			{
			unsigned char dat;
			if (ISR_DataTx(&dat))
				{
				huart.Instance->TDR = dat;	
				}
			else
				{
				huart.Instance->CR1 &= ~(((unsigned short)1)<<7);
				huart.Instance->ICR |= 0xDF;
				ISR_TxEnd_cb ();
				}
			}
		}
huart.Instance->ICR |= 0x0F;
}



bool TUSART_IFACE::ISR_DataTx (unsigned char *lDat)
{
bool rv = false;
if (tx_size)
	{
	rv = true;
	lDat[0] = lTxAdr[0];
	lTxAdr++;
	tx_size--;
	}
else
	{
	f_tx_status = false;
	}
return rv;
}



bool TUSART_IFACE::GetTxStatus () 
{
return !f_tx_status;
}



void TUSART_IFACE::RawTransmit (unsigned char *lTx, unsigned short sizes)
{
	lTxAdr = lTx;
	tx_size = sizes;
	if (sizes) 
		{
		f_tx_status = true;
		USART_ITConfig_TX (true);
		}
}



void TUSART_IFACE::USART_ITConfig_TX (bool val)
{
const unsigned short mask = 0x80;
if (val)
	{
	huart.Instance->CR1 |= mask;
	}
else
	{
	huart.Instance->CR1 &= ~mask;
	}
}



void TUSART_IFACE::USART_ITConfig_RX (bool val)
{
const unsigned short mask = 0x20;
if (val)
	{
	huart.Instance->CR1 |= mask;
	}
else
	{
	huart.Instance->CR1 &= ~mask;
	}
}



void TUSART_IFACE::USART_Cmd (FunctionalState NewState)
{
const unsigned short CR1_UE = 1 | 4 | 8;
  
  if (NewState != DISABLE)
  {
    huart.Instance->CR1 |= CR1_UE;
  }
  else
  {
    huart.Instance->CR1 &= ~CR1_UE;
  }
}



