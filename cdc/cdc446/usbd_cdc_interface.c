/**
  ******************************************************************************
  * @file    USB_Device/CDC_Standalone/Src/usbd_cdc_interface.c
  * @author  MCD Application Team
  * @brief   Source file for USBD CDC interface
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
//#include "main.h"
#include "stm32f4xx_hal.h"
#include "usbd_desc.h"
#include "usbd_cdc.h" 
#include "usbd_cdc_interface.h"
//#include "stm32446e_eval.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define APP_RX_DATA_SIZE  2048
#define APP_TX_DATA_SIZE  2048

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
USBD_CDC_LineCodingTypeDef LineCoding =
  {
    115200, /* baud rate*/
    0x00,   /* stop bits-1*/
    0x00,   /* parity - none*/
    0x08    /* nb. of bits 8*/
  };

uint8_t UserRxBuffer[APP_RX_DATA_SIZE];/* Received Data over USB are stored in this buffer */
uint8_t UserTxBuffer[APP_TX_DATA_SIZE];/* Received Data over UART (CDC interface) are stored in this buffer */
uint32_t BuffLength;
volatile uint32_t UserTxBufPtrIn = 0;/* Increment this pointer or roll it back to
                               start address when data are received over USART */
volatile uint32_t UserTxBufPtrOut = 0; /* Increment this pointer or roll it back to
                                 start address when data are sent over USB */
	
volatile static uint32_t UserTxBufData_cnt = 0;

/* UART handler declaration */
UART_HandleTypeDef UartHandle;
/* TIM handler declaration */
TIM_HandleTypeDef  TimHandle;
/* USB handler declaration */
extern USBD_HandleTypeDef  USBD_Device;

/* Private function prototypes -----------------------------------------------*/
static int8_t CDC_Itf_Init(void);
static int8_t CDC_Itf_DeInit(void);
static int8_t CDC_Itf_Control(uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Itf_Receive(uint8_t* pbuf, uint32_t *Len);
static int8_t CDC_Itf_TransmitCplt(uint8_t *pbuf, uint32_t *Len, uint8_t epnum);

static void Error_Handler(void);
static void ComPort_Config(void);
//static void TIM_Config(void);
extern void tusbobj_reconnect ();
extern void tusbobj_baudrate (uint32_t br);

USBD_CDC_ItfTypeDef USBD_CDC_fops = 
{
  CDC_Itf_Init,
  CDC_Itf_DeInit,
  CDC_Itf_Control,
  CDC_Itf_Receive,
  CDC_Itf_TransmitCplt
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  CDC_Itf_Init
  *         Initializes the CDC media low layer
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Itf_Init(void)
{
	/*
  UartHandle.Instance          = USARTx;
  UartHandle.Init.BaudRate     = 115200;
  UartHandle.Init.WordLength   = UART_WORDLENGTH_8B;
  UartHandle.Init.StopBits     = UART_STOPBITS_1;
  UartHandle.Init.Parity       = UART_PARITY_NONE;
  UartHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  UartHandle.Init.Mode         = UART_MODE_TX_RX;
  UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;
    
  if(HAL_UART_Init(&UartHandle) != HAL_OK)
  {
    Error_Handler();
  }
  

  if(HAL_UART_Receive_IT(&UartHandle, (uint8_t *)UserTxBuffer, 1) != HAL_OK)
  {

    Error_Handler();
  }

  if(HAL_TIM_Base_Start_IT(&TimHandle) != HAL_OK)
  {
    Error_Handler();
  }
	*/

  USBD_CDC_SetTxBuffer(&USBD_Device, UserTxBuffer, 0);
  USBD_CDC_SetRxBuffer(&USBD_Device, UserRxBuffer);
	
	tusbobj_reconnect ();
  
  return (USBD_OK);
}

/**
  * @brief  CDC_Itf_DeInit
  *         DeInitializes the CDC media low layer
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Itf_DeInit(void)
{
  /* DeInitialize the UART peripheral */
  if(HAL_UART_DeInit(&UartHandle) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }
  return (USBD_OK);
}

/**
  * @brief  CDC_Itf_Control
  *         Manage the CDC class requests
  * @param  Cmd: Command code            
  * @param  Buf: Buffer containing command data (request parameters)
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Itf_Control (uint8_t cmd, uint8_t* pbuf, uint16_t length)
{ 
  switch (cmd)
  {
  case CDC_SEND_ENCAPSULATED_COMMAND:
    /* Add your code here */
    break;

  case CDC_GET_ENCAPSULATED_RESPONSE:
    /* Add your code here */
    break;

  case CDC_SET_COMM_FEATURE:
    /* Add your code here */
    break;

  case CDC_GET_COMM_FEATURE:
    /* Add your code here */
    break;

  case CDC_CLEAR_COMM_FEATURE:
    /* Add your code here */
    break;

  case CDC_SET_LINE_CODING:
    LineCoding.bitrate    = (uint32_t)(pbuf[0] | (pbuf[1] << 8) |\
                            (pbuf[2] << 16) | (pbuf[3] << 24));
    LineCoding.format     = pbuf[4];
    LineCoding.paritytype = pbuf[5];
    LineCoding.datatype   = pbuf[6];
    
    /* Set the new configuration */
    ComPort_Config();
    break;

  case CDC_GET_LINE_CODING:
    pbuf[0] = (uint8_t)(LineCoding.bitrate);
    pbuf[1] = (uint8_t)(LineCoding.bitrate >> 8);
    pbuf[2] = (uint8_t)(LineCoding.bitrate >> 16);
    pbuf[3] = (uint8_t)(LineCoding.bitrate >> 24);
    pbuf[4] = LineCoding.format;
    pbuf[5] = LineCoding.paritytype;
    pbuf[6] = LineCoding.datatype;     
    break;

  case CDC_SET_CONTROL_LINE_STATE:
    /* Add your code here */
    break;

  case CDC_SEND_BREAK:
     /* Add your code here */
    break;    
    
  default:
    break;
  }
  
  return (USBD_OK);
}




static uint16_t cdc_tx_buffer_free_space ()
{
	return APP_TX_DATA_SIZE - UserTxBufData_cnt;
}


static uint8_t cdc_add_byte_to_usb (uint8_t d)
{
	uint8_t rv = 0;
	if (UserTxBufData_cnt < APP_TX_DATA_SIZE)
		{
		UserTxBuffer[UserTxBufPtrIn] = d;
		UserTxBufPtrIn++;
		if (UserTxBufPtrIn >= APP_TX_DATA_SIZE) UserTxBufPtrIn = 0;
		UserTxBufData_cnt++;
		rv = 1;
		}
	return rv;
}



extern uint8_t tusbobj_tx_data (uint8_t *d);
static void cdc_update_tx_bufer ()
{
		uint8_t data_tx;
		while (cdc_tx_buffer_free_space ()) {
			if (!tusbobj_tx_data (&data_tx)) break;
			cdc_add_byte_to_usb (data_tx);
			}
}

volatile static uint32_t cnt_bytrs_err = 0;

void cdc_usb_tx_task ()
{
  uint32_t buffptr;
  uint32_t buffsize;
  
	if (!UserTxBufData_cnt) cdc_update_tx_bufer ();
	
  if (UserTxBufData_cnt)
  {
    if(UserTxBufPtrOut > UserTxBufPtrIn) 
    {
      buffsize = APP_TX_DATA_SIZE - UserTxBufPtrOut;
    }
    else 
    {
      buffsize = UserTxBufPtrIn - UserTxBufPtrOut;
    }
    
    buffptr = UserTxBufPtrOut;
    
    USBD_CDC_SetTxBuffer(&USBD_Device, (uint8_t*)&UserTxBuffer[buffptr], buffsize);
    
    if(USBD_CDC_TransmitPacket(&USBD_Device) == USBD_OK)
    {
      UserTxBufPtrOut += buffsize;
			UserTxBufData_cnt -= buffsize;
      if (UserTxBufPtrOut == APP_TX_DATA_SIZE)
      {
        UserTxBufPtrOut = 0;
      }
    }
	else
		{
		cnt_bytrs_err++;
		}
  }

}



/*
// void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
void cdc_usb_tx_task ()
{
  uint32_t buffptr;
  uint32_t buffsize;
  
  if (UserTxBufData_cnt)
  {
    if(UserTxBufPtrOut > UserTxBufPtrIn) 
    {
      buffsize = APP_TX_DATA_SIZE - UserTxBufPtrOut;
    }
    else 
    {
      buffsize = UserTxBufPtrIn - UserTxBufPtrOut;
    }
    
    buffptr = UserTxBufPtrOut;
    
    USBD_CDC_SetTxBuffer(&USBD_Device, (uint8_t*)&UserTxBuffer[buffptr], buffsize);
    
    if(USBD_CDC_TransmitPacket(&USBD_Device) == USBD_OK)
    {
      UserTxBufPtrOut += buffsize;
			UserTxBufData_cnt -= buffsize;
      if (UserTxBufPtrOut == APP_RX_DATA_SIZE)
      {
        UserTxBufPtrOut = 0;
      }
    }
  }
}
*/











/*
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

  UserTxBufPtrIn++;
  

  if(UserTxBufPtrIn == APP_RX_DATA_SIZE)
  {
    UserTxBufPtrIn = 0;
  }
  

  HAL_UART_Receive_IT(huart, (uint8_t *)(UserTxBuffer + UserTxBufPtrIn), 1);
}
*/


/**
  * @brief  CDC_Itf_DataRx
  *         Data received over USB OUT endpoint are sent over CDC interface 
  *         through this function.
  * @param  Buf: Buffer of data to be transmitted
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */

extern void tusbobj_rx_data (uint8_t* Buf, uint32_t Len);

static int8_t CDC_Itf_Receive(uint8_t* Buf, uint32_t *Len)
{
	// из usb в serial
	tusbobj_rx_data (Buf, *Len);		// 20.09.23
	/*
	// test echo start
	uint32_t txsz = *Len;
	while (txsz)
		{
		if (!cdc_add_byte_to_usb (*Buf)) break;
		Buf++;
		txsz--;
		}
	// test echo stop
	*/
	USBD_CDC_ReceivePacket (&USBD_Device);	// prepare to rx
  return (USBD_OK);
}

/**
  * @brief  CDC_Itf_TransmitCplt
  *         Data transmitted callback
  *
  * @note
  *         This function is IN transfer complete callback used to inform user that
  *         the submitted Data is successfully sent over USB.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Itf_TransmitCplt(uint8_t *Buf, uint32_t *Len, uint8_t epnum)
{
	//if (!UserTxBufData_cnt) cdc_update_tx_bufer ();
  return (USBD_OK);
}

/**
  * @brief  Tx Transfer completed callback
  * @param  huart: UART handle
  * @retval None
  */

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  //USBD_CDC_ReceivePacket(&USBD_Device);
}


/**
  * @brief  ComPort_Config
  *         Configure the COM Port with the parameters received from host.
  * @param  None.
  * @retval None
  * @note   When a configuration is not supported, a default value is used.
  */
static void ComPort_Config(void)
{
/*
  if(HAL_UART_DeInit(&UartHandle) != HAL_OK)
  {
    Error_Handler();
  }
  
  // set the Stop bit 
  switch (LineCoding.format)
  {
  case 0:
    UartHandle.Init.StopBits = UART_STOPBITS_1;
    break;
  case 2:
    UartHandle.Init.StopBits = UART_STOPBITS_2;
    break;
  default :
    UartHandle.Init.StopBits = UART_STOPBITS_1;
    break;
  }
  
  // set the parity bit
  switch (LineCoding.paritytype)
  {
  case 0:
    UartHandle.Init.Parity = UART_PARITY_NONE;
    break;
  case 1:
    UartHandle.Init.Parity = UART_PARITY_ODD;
    break;
  case 2:
    UartHandle.Init.Parity = UART_PARITY_EVEN;
    break;
  default :
    UartHandle.Init.Parity = UART_PARITY_NONE;
    break;
  }
  
  //set the data type : only 8bits and 9bits is supported 
  switch (LineCoding.datatype)
  {
  case 0x07:
    // With this configuration a parity (Even or Odd) must be set 
    UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
    break;
  case 0x08:
    if(UartHandle.Init.Parity == UART_PARITY_NONE)
    {
      UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
    }
    else 
    {
      UartHandle.Init.WordLength = UART_WORDLENGTH_9B;
    }
    
    break;
  default :
    UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
    break;
  }
	
	
	
*/
	tusbobj_baudrate (LineCoding.bitrate);

/*    
  UartHandle.Init.BaudRate     = LineCoding.bitrate;
  UartHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  UartHandle.Init.Mode         = UART_MODE_TX_RX;
  UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;

  if(HAL_UART_Init(&UartHandle) != HAL_OK)
  {
    // Initialization Error 
    Error_Handler();
  }
*/

  // Start reception: provide the buffer pointer with offset and the buffer size 
 // HAL_UART_Receive_IT(&UartHandle, (uint8_t *)(UserTxBuffer + UserTxBufPtrIn), 1);
}

/**
  * @brief  TIM_Config: Configure TIMx timer
  * @param  None.
  * @retval None
  */
/*
static void TIM_Config(void)
{  

  TimHandle.Instance = TIMx;
  
  // Initialize TIM3 peripheral as follow:
//       + Period = 10000 - 1
       //+ Prescaler = ((SystemCoreClock/2)/10000) - 1
       //+ ClockDivision = 0
       //+ Counter direction = Up
  
  TimHandle.Init.Period = (CDC_POLLING_INTERVAL*1000) - 1;
  TimHandle.Init.Prescaler = 84-1;
  TimHandle.Init.ClockDivision = 0;
  TimHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
  TimHandle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if(HAL_TIM_Base_Init(&TimHandle) != HAL_OK)
  {
    Error_Handler();
  }
}
*/
/**
  * @brief  UART error callbacks
  * @param  UartHandle: UART handle
  * @retval None
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *UartHandle)
{
  /* Transfer error occurred in reception and/or transmission process */
  Error_Handler();
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  /* Add your own code here */
}
