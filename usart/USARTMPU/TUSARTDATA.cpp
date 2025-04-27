#include "tusartdata.h"


static const uint32_t parityarr[EUSARTPARITYMODE_ENDENUM] = {UART_PARITY_NONE, UART_PARITY_EVEN, UART_PARITY_ODD};
static const uint32_t stopbityarr[EUSARTSTOPBIT_ENDENUM] = {UART_STOPBITS_1, UART_STOPBITS_2};
TSERIALISR *TSERIALISR::ifc[ESYSUSART_ENDENUM] = {0,0,0,0,0,0};

#include "TUSARTconf.h"



#define _IFCUSART_UART_DISABLE_IT(__HANDLE__, __INTERRUPT__)  ((((__INTERRUPT__) >> 28U) == UART_CR1_REG_INDEX)? ((__HANDLE__)->CR1 &= ~((__INTERRUPT__) & UART_IT_MASK)): \
                                                           (((__INTERRUPT__) >> 28U) == UART_CR2_REG_INDEX)? ((__HANDLE__)->CR2 &= ~((__INTERRUPT__) & UART_IT_MASK)): \
                                                           ((__HANDLE__)->CR3 &= ~ ((__INTERRUPT__) & UART_IT_MASK)))
#define _IFCUSART_UART_ENABLE_IT(__HANDLE__, __INTERRUPT__)   ((((__INTERRUPT__) >> 28U) == UART_CR1_REG_INDEX)? ((__HANDLE__)->CR1 |= ((__INTERRUPT__) & UART_IT_MASK)): \
                                                           (((__INTERRUPT__) >> 28U) == UART_CR2_REG_INDEX)? ((__HANDLE__)->CR2 |= ((__INTERRUPT__) & UART_IT_MASK)): \
                                                           ((__HANDLE__)->CR3 |= ((__INTERRUPT__) & UART_IT_MASK)))
#define _IFCUSART_UART_CLEAR_FLAG(__HANDLE__, __FLAG__) ((__HANDLE__)->SR = ~(__FLAG__))
#define _IFCUSART_UART_ENABLE(__HANDLE__)               ((__HANDLE__)->CR1 |=  USART_CR1_UE)



#ifdef __cplusplus
 extern "C" {
#endif 

void base_txrx_isr (TSERIALISR &obj)
{
	uint16_t flag_cr = obj.USARTPort->CR1;
	uint16_t flag_it = obj.USARTPort->SR;
	uint8_t dat = obj.USARTPort->DR;
  if (flag_it & 32)	obj.isr_rx (dat);

	if (flag_cr & 0x80) {
		if (flag_it & 0x80) {
			uint8_t dat;
			if (obj.isr_tx (dat))
				{
				obj.USARTPort->DR = dat;
				}
			else
				{
				_IFCUSART_UART_DISABLE_IT (obj.USARTPort, UART_IT_TXE);
				_IFCUSART_UART_CLEAR_FLAG (obj.USARTPort, USART_IT_TXE);
				}
			}
		}
}


#ifdef USART1
void USART1_IRQHandler ()
{
	base_txrx_isr (*TSERIALISR::ifc[ESYSUSART_1]);
}
#endif

#ifdef UART1
void UART1_IRQHandler ()
{
	base_txrx_isr (*TSERIALISR::ifc[ESYSUSART_1]);
}
#endif

#ifdef USART2
void USART2_IRQHandler ()
{
	base_txrx_isr (*TSERIALISR::ifc[ESYSUSART_2]);
}
#endif

#ifdef UART2
void UART2_IRQHandler ()
{
	base_txrx_isr (*TSERIALISR::ifc[ESYSUSART_2]);
}
#endif

#ifdef USART3
void USART3_IRQHandler ()
{
	base_txrx_isr (*TSERIALISR::ifc[ESYSUSART_3]);
}
#endif
#ifdef UART3
void UART3_IRQHandler ()
{
	base_txrx_isr (*TSERIALISR::ifc[ESYSUSART_3]);
}
#endif

#ifdef UART4
void UART4_IRQHandler ()
{
	base_txrx_isr (*TSERIALISR::ifc[ESYSUSART_4]);
}
#endif
#ifdef USART4
void USART4_IRQHandler ()
{
	base_txrx_isr (*TSERIALISR::ifc[ESYSUSART_4]);
}
#endif

#ifdef UART5
void UART5_IRQHandler ()
{
	base_txrx_isr (*TSERIALISR::ifc[ESYSUSART_5]);
}
#endif
#ifdef USART5
void USART5_IRQHandler ()
{
	base_txrx_isr (*TSERIALISR::ifc[ESYSUSART_5]);
}
#endif

#ifdef UART6
void UART6_IRQHandler ()
{
	base_txrx_isr (*TSERIALISR::ifc[ESYSUSART_6]);
}
#endif

#ifdef USART6
void USART6_IRQHandler ()
{
	base_txrx_isr (*TSERIALISR::ifc[ESYSUSART_6]);
}
#endif



#ifdef __cplusplus
}
#endif



bool TSERIALISR::is_hiz_mode ()
{
	return f_is_hiz_mode_activate;
}




bool TSERIALUSR::push (uint8_t d)
{
	return fifo_tx->push ((uint8_t*)&d, 1);
}



bool TSERIALUSR::pop (uint8_t &d)
{
	return fifo_rx->pop (d);
}



void TSERIALUSR::SetLinkDetectPeriod (uint32_t t)
{
	c_linkdetect_period = t;
}




#ifdef SERIALDEBAG
uint32_t TSERIALUSR::debug_peak_tx ()
{
return fifo_tx->statistic_peak ();
}



uint32_t TSERIALUSR::debug_peak_rx ()
{
	return fifo_rx->statistic_peak ();
}



void TSERIALUSR::debug_peak_clear ()
{
	fifo_rx->statistic_peak_clear ();
	fifo_tx->statistic_peak_clear ();
}



#endif




void TUSARTOBJ::hard_constr_init (ESYSUSART prt, uint32_t sz_b_tx, uint32_t sz_b_rx, bool f_md, EUSARTPARITYMODE pm, EUSARTSTOPBIT sbm)
{
	f_tx_open_drain = f_md;
	c_linkdetect_period = 2000;
	c_port_ix = prt;
	fifo_tx = new TTFIFO<uint8_t>(sz_b_tx);
	fifo_rx = new TTFIFO<uint8_t>(sz_b_rx);

	ifc[c_port_ix] = this;
	USARTPort = usarthardarr[c_port_ix].usart;
	UartHandle.Instance        = USARTPort;
	hard_usart_clock_enable (USARTPort);
	thizif_hiz_outputs (false);

	default_settings (pm, sbm);
	SetSpeed (C_USART_SPEED_DEF);

	Init ();
}



void TSERIALUSR::SetSpeed (uint32_t c_speed)
{
  UartHandle.Init.BaudRate   = c_speed;
  UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
  UartHandle.Init.StopBits   = UART_STOPBITS_1;
  UartHandle.Init.Parity     = UART_PARITY_NONE;
  UartHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
  UartHandle.Init.Mode       = UART_MODE_TX_RX;
  UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init (&UartHandle);
}



void TUSARTOBJ::default_settings (EUSARTPARITYMODE pm, EUSARTSTOPBIT sbm)
{
	  UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
	  UartHandle.Init.StopBits   = stopbityarr[sbm];
	  UartHandle.Init.Parity     = parityarr[pm];
	  UartHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
	  UartHandle.Init.Mode       = UART_MODE_TX_RX;
	  UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;
}




uint32_t TSERIALUSR::tx_free_space ()
{
	return fifo_tx->is_free_space ();
}



void TSERIALUSR::clear ()
{
	fifo_tx->clear ();
	fifo_rx->clear ();
}



TUSARTOBJ::TUSARTOBJ (ESYSUSART prt, uint32_t sz_b_tx, uint32_t sz_b_rx)
{
	hard_constr_init ( prt,  sz_b_tx,  sz_b_rx, false, EUSARTPARITYMODE_NONE, EUSARTSTOPBIT_1);
}



TUSARTOBJ::TUSARTOBJ (ESYSUSART prt, uint32_t sz_b_tx, uint32_t sz_b_rx, bool f_tx_od)
{
	hard_constr_init ( prt,  sz_b_tx,  sz_b_rx, f_tx_od, EUSARTPARITYMODE_NONE, EUSARTSTOPBIT_1);
}



TUSARTOBJ::TUSARTOBJ (ESYSUSART prt, uint32_t sz_b_tx, uint32_t sz_b_rx, bool f_tx_od, EUSARTPARITYMODE pm, EUSARTSTOPBIT sbm)
{
	hard_constr_init ( prt,  sz_b_tx,  sz_b_rx, f_tx_od, pm, sbm);
}



/*
TUSARTOBJ::TUSARTOBJ (ESYSUSART prt, uint32_t sz_b_tx, uint32_t sz_b_rx)
{
	c_linkdetect_period = 2000;
	c_port_ix = prt;
	fifo_tx = new TTFIFO<uint8_t>(sz_b_tx);
	fifo_rx = new TTFIFO<uint8_t>(sz_b_rx);
	Init ();
}
*/




bool TUSARTOBJ::Tx_status ()
{
	return f_tx_status;
}



bool TUSARTOBJ::Tx (void *lTx, uint16_t sz_tx)
{
	bool rv = false;
	if (fifo_tx->is_free_space () >= sz_tx)
		{
		rv = fifo_tx->push ((uint8_t*)lTx, sz_tx);
		f_tx_status = rv;
		if (rv) _IFCUSART_UART_ENABLE_IT (USARTPort, UART_IT_TXE);
		}
return rv;
}



uint16_t TUSARTOBJ::Rx_check ()
{
	return fifo_rx->frame_count ();
}



bool TUSARTOBJ::is_link ()
{
	return (timer_link.get())?true:false;
}




uint16_t TUSARTOBJ::Rx (void *lD, uint16_t max_size)
{
	return fifo_rx->pop ((uint8_t*)lD, max_size);
}



void TUSARTOBJ::isr_rx (uint8_t d)
{
	fifo_rx->push (d);
	timer_link.set (c_linkdetect_period);
}



bool TUSARTOBJ::isr_tx (uint8_t &d)
{
	bool rv = fifo_tx->pop (d);
	if (!rv) f_tx_status = false;
	return rv;
}



void TUSARTOBJ::rx_clear ()
{
fifo_rx->clear ();
}



void TUSARTOBJ::thizif_hiz_outputs (bool f_act_hiz)
{
	if (!f_act_hiz)
		{
		if (f_tx_open_drain)
			{
			_pin_low_init_out_od_af ( usarthardarr[c_port_ix].af_mux_id, &usarthardarr[c_port_ix].pins[EUSARTPINIX_TX], EHRTGPIOSPEED_HI);
			}
		else
			{
			_pin_low_init_out_pp_af ( usarthardarr[c_port_ix].af_mux_id, &usarthardarr[c_port_ix].pins[EUSARTPINIX_TX], EHRTGPIOSPEED_HI);
			}
		}
	else
		{
		_pin_low_init_in (&usarthardarr[c_port_ix].pins[EUSARTPINIX_TX], 1, EHRTGPIOSPEED_HI, EHRTGPIOPULL_NO);
		}
}


void TUSARTOBJ::Init ()
{
	ifc[c_port_ix] = this;
	USARTPort = usarthardarr[c_port_ix].usart;
	UartHandle.Instance        = USARTPort;
	hard_usart_clock_enable (USARTPort);
	
	SetSpeed (C_USART_SPEED_DEF);
	
	//_pin_low_init_out_pp_af ( usarthardarr[c_port_ix].af_mux_id, &usarthardarr[c_port_ix].pins[EUSARTPINIX_TX]);
	#if (HRDCPU == 1)
	thizif_hiz_outputs (false);
	//_pin_low_init_out_pp_af ( &usarthardarr[c_port_ix].pins[EUSARTPINIX_TX], EHRTGPIOSPEED_MID);
	_pin_low_init_out_od_af ( &usarthardarr[c_port_ix].pins[EUSARTPINIX_RX], EHRTGPIOSPEED_MID);
	#endif
	#if (HRDCPU == 4)
	thizif_hiz_outputs (false);
	//_pin_low_init_out_pp_af ( usarthardarr[c_port_ix].af_mux_id, &usarthardarr[c_port_ix].pins[EUSARTPINIX_TX],EHRTGPIOSPEED_MID);
	_pin_low_init_out_od_af ( usarthardarr[c_port_ix].af_mux_id, &usarthardarr[c_port_ix].pins[EUSARTPINIX_RX],EHRTGPIOSPEED_MID);
	#endif
		
	_IFCUSART_UART_DISABLE_IT (USARTPort, USART_IT_RXNE);
	_IFCUSART_UART_CLEAR_FLAG (USARTPort, USART_IT_RXNE);
	_IFCUSART_UART_DISABLE_IT (USARTPort, USART_IT_TXE);
	_IFCUSART_UART_CLEAR_FLAG (USARTPort, USART_IT_TXE);
	
	fifo_tx->clear ();
	fifo_rx->clear ();
	f_tx_status = false;
		
	_IFCUSART_UART_ENABLE_IT (USARTPort, USART_IT_RXNE);
  _IFCUSART_UART_ENABLE (USARTPort);
	HAL_NVIC_EnableIRQ ((IRQn_Type)usartisr_arr[c_port_ix]);
}



