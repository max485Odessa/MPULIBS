#ifndef _H_TUSART_CLASS_CONFIGURE_H_
#define _H_TUSART_CLASS_CONFIGURE_H_

#ifdef STM32F411xC
	#ifdef STARFLY
static uint8_t usartisr_arr[ESYSUSART_ENDENUM] = {USART1_IRQn, USART2_IRQn, 0, 0, 0, USART6_IRQn};
static S_USARTPINS_T usarthardarr[ESYSUSART_ENDENUM] = {{GPIOA, GPIO_PIN_9/*tx*/, GPIOA, GPIO_PIN_10/*rx*/, USART1, GPIO_AF7_USART1}, {GPIOA, GPIO_PIN_2/*tx*/, GPIOA, GPIO_PIN_3/*rx*/, USART2, GPIO_AF7_USART2}, \
{0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, \
{GPIOC, GPIO_PIN_6/*tx*/, GPIOC, GPIO_PIN_7/*rx*/, USART6, GPIO_AF8_USART6}};
	#endif
#endif


#endif
