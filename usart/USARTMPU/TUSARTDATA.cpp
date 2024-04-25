#include "tusartdata.h"


#define _IFCUSART_UART_DISABLE_IT(__HANDLE__, __INTERRUPT__)  ((((__INTERRUPT__) >> 28U) == UART_CR1_REG_INDEX)? ((__HANDLE__)->CR1 &= ~((__INTERRUPT__) & UART_IT_MASK)): \
                                                           (((__INTERRUPT__) >> 28U) == UART_CR2_REG_INDEX)? ((__HANDLE__)->CR2 &= ~((__INTERRUPT__) & UART_IT_MASK)): \
                                                           ((__HANDLE__)->CR3 &= ~ ((__INTERRUPT__) & UART_IT_MASK)))
#define _IFCUSART_UART_ENABLE_IT(__HANDLE__, __INTERRUPT__)   ((((__INTERRUPT__) >> 28U) == UART_CR1_REG_INDEX)? ((__HANDLE__)->CR1 |= ((__INTERRUPT__) & UART_IT_MASK)): \
                                                           (((__INTERRUPT__) >> 28U) == UART_CR2_REG_INDEX)? ((__HANDLE__)->CR2 |= ((__INTERRUPT__) & UART_IT_MASK)): \
                                                           ((__HANDLE__)->CR3 |= ((__INTERRUPT__) & UART_IT_MASK)))
#define _IFCUSART_UART_CLEAR_FLAG(__HANDLE__, __FLAG__) ((__HANDLE__)->SR = ~(__FLAG__))
#define _IFCUSART_UART_ENABLE(__HANDLE__)               ((__HANDLE__)->CR1 |=  USART_CR1_UE)


#if (HRDCPU == 1)
TSERIALISR *TSERIALISR::ifc[ESYSUSART_ENDENUM] = {0,0,0};
static uint8_t usartisr_arr[ESYSUSART_ENDENUM] = {USART1_IRQn, USART2_IRQn, USART3_IRQn};
static S_USARTPINS_T usarthardarr[ESYSUSART_ENDENUM] = {{GPIOB, GPIO_PIN_6/*tx*/, GPIOB, GPIO_PIN_7/*rx*/, USART1, 0}, {GPIOA, GPIO_PIN_2/*tx*/, GPIOA, GPIO_PIN_3/*rx*/, USART2, 0}, \
{GPIOC, GPIO_PIN_10/*tx*/, GPIOC, GPIO_PIN_11/*rx*/, USART3, 0}};
#endif

#if (HRDCPU == 4)
TSERIALISR *TSERIALISR::ifc[ESYSUSART_ENDENUM] = {0,0,0,0,0,0};
static uint8_t usartisr_arr[ESYSUSART_ENDENUM] = {USART1_IRQn, USART2_IRQn, USART3_IRQn, UART4_IRQn, UART5_IRQn, USART6_IRQn};
static S_USARTPINS_T usarthardarr[ESYSUSART_ENDENUM] = {{GPIOA, GPIO_PIN_9/*tx*/, GPIOA, GPIO_PIN_10/*rx*/, USART1, GPIO_AF7_USART1}, {GPIOA, GPIO_PIN_2/*tx*/, GPIOA, GPIO_PIN_3/*rx*/, USART2, GPIO_AF7_USART2}, \
{GPIOC, GPIO_PIN_10/*tx*/, GPIOC, GPIO_PIN_11/*rx*/, USART3, GPIO_AF7_USART3}, {GPIOC, GPIO_PIN_10/*tx*/, GPIOC, GPIO_PIN_11/*rx*/, UART4, GPIO_AF8_UART4}, {GPIOC, GPIO_PIN_12/*tx*/, GPIOD, GPIO_PIN_2/*rx*/, UART5, GPIO_AF8_UART5}, \
{GPIOC, GPIO_PIN_6/*tx*/, GPIOC, GPIO_PIN_7/*rx*/, USART6, GPIO_AF8_USART6}};
#endif


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



void USART1_IRQHandler ()
{
	base_txrx_isr (*TSERIALISR::ifc[ESYSUSART_1]);
}

#ifdef USART2
void USART2_IRQHandler ()
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

#ifdef UART4
void UART4_IRQHandler ()
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

#ifdef UART6
void USART6_IRQHandler ()
{
	base_txrx_isr (*TSERIALISR::ifc[ESYSUSART_6]);
}
#endif



#ifdef __cplusplus
}
#endif



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
	c_linkdetect_period = 2000;
	c_port_ix = prt;
	fifo_tx = new TTFIFO<uint8_t>(sz_b_tx);
	fifo_rx = new TTFIFO<uint8_t>(sz_b_rx);
	Init ();
}





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



void TUSARTOBJ::Init ()
{
	ifc[c_port_ix] = this;
	USARTPort = usarthardarr[c_port_ix].usart;
	UartHandle.Instance        = USARTPort;
	hard_usart_clock_enable (USARTPort);
	
	SetSpeed (C_USART_SPEED_DEF);
	
	//_pin_low_init_out_pp_af ( usarthardarr[c_port_ix].af_mux_id, &usarthardarr[c_port_ix].pins[EUSARTPINIX_TX]);
	#if (HRDCPU == 1)
	_pin_low_init_out_pp_af ( &usarthardarr[c_port_ix].pins[EUSARTPINIX_TX], EHRTGPIOSPEED_MID);
	_pin_low_init_out_od_af ( &usarthardarr[c_port_ix].pins[EUSARTPINIX_RX], EHRTGPIOSPEED_MID);
	#endif
	#if (HRDCPU == 4)
	_pin_low_init_out_pp_af ( usarthardarr[c_port_ix].af_mux_id, &usarthardarr[c_port_ix].pins[EUSARTPINIX_TX],EHRTGPIOSPEED_MID);
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



void TUSARTOBJ::SetSpeed (uint32_t c_speed)
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


