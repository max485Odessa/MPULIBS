#include "TAK70CAN.hpp"
#include "rutine.hpp"
#include "stm32f4xx_hal.h"
//#include <math.h>

static const unsigned char seq_cmd_unlock[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0XFC};
static const unsigned char seq_cmd_status[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0XFC};
static const unsigned char seq_cmd_exit[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFD};
static const unsigned char seq_cmd_poszero[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0XFE};
//static const float c_diap_w = 3.8197188888;

#define CAN_TXMAILBOX_0   ((uint8_t)0x00)
#define CAN_TXMAILBOX_1   ((uint8_t)0x01)
#define CAN_TXMAILBOX_2   ((uint8_t)0x02)


long TAK70::float_to_uint(float x, float x_min, float x_max, unsigned int bits)
{
/// Converts a float to an unsigned int, given range and number of bits ///
float span = x_max - x_min;
if(x < x_min) x = x_min;
else if(x > x_max) x = x_max;
return (long) ((x- x_min)*((float)((1<<bits)/span)));
}


float TAK70::uint_to_float(int x_int, float x_min, float x_max, int bits)
{
float span = x_max - x_min;
float offset = x_min;
return ((float)x_int)*span/((float)((1<<bits)-1)) + offset;
}




void TAK70::unpack_reply (const CanRxMsg &msg, float &postion, float &speed, float &torque)
{

float P_MIN =-95.5;
float P_MAX =95.5;
float V_MIN =-30;
float V_MAX =30;

/// unpack ints from can buffer ///
int id = msg.Data[0]; //����ID ��
int p_int = (msg.Data[1]<<8)|msg.Data[2]; //Motor position data
int v_int = (msg.Data[3]<<4)|(msg.Data[4]>>4); // Motor speed data
int i_int = ((msg.Data[4]&0xF)<<8)|msg.Data[5]; // Motor torque data
/// convert ints to floats ///
float p = uint_to_float(p_int, P_MIN, P_MAX, 16);
float v = uint_to_float(v_int, V_MIN, V_MAX, 12);
//float i = uint_to_float(i_int, -I_MAX, I_MAX, 12);
float i = uint_to_float(i_int, -50, 50, 12);
if(id == 1)
  {
  postion = p; // Read the corresponding data according to the ID code
  speed = v;
  torque = i;
  }
}




void TAK70::SetCurrent (float val)
{
const float T_MIN =-25;		// nM  -18
const float T_MAX =25;		// nM 18
	unsigned short vl = float_to_uint(val, T_MIN, T_MAX, 12);
	TxMsg.Data[7] = vl & 0xFF;
	TxMsg.Data[6] = (TxMsg.Data[6] & 0xF0) | ((vl >> 8) & 0x0F);
}



void TAK70::SetSpeed (float val)
{
const float V_MIN =-30;	
const float V_MAX =30;	
	unsigned short vl = float_to_uint(val, V_MIN, V_MAX, 12);		
	TxMsg.Data[3] = (TxMsg.Data[3] & 0x0F) | ((vl << 4) & 0xF0);
	TxMsg.Data[2] = (vl >> 4);
}



void TAK70::SetKD (unsigned short val)
{
const float Kd_MIN =0;
const float Kd_MAX =5;
	unsigned short vl = float_to_uint (val, Kd_MIN, Kd_MAX, 12);
	TxMsg.Data[5] = (vl >> 4);
	TxMsg.Data[6] = (TxMsg.Data[6] & 0x0F) | ((vl << 4) & 0xF0);
}




void TAK70::SetKP (unsigned short val)
{
const float Kp_MIN =0;
const float Kp_MAX =500;
	unsigned short vl = float_to_uint(val, Kp_MIN, Kp_MAX, 12);
	TxMsg.Data[3] = (TxMsg.Data[3] & 0xF0) | ((vl >> 8) & 0x0F);
	TxMsg.Data[4] = vl & 0xFF;
}




void TAK70::SetPosition (float val)
{
const float P_MIN =-95.5;	
const float P_MAX =95.5;	
	unsigned short vl = float_to_uint(val, P_MIN, P_MAX, 16);
	TxMsg.Data[0] = vl >> 8;
	TxMsg.Data[1] = vl & 0xFF;
}



void TAK70::Power_enabled (bool val)
{
	is_poweres_state = val;
	_pin_set_to (const_cast<S_GPIOPIN*>(power_pin), is_poweres_state);
}




TAK70::TAK70 (const S_GPIOPIN *pin, bool f_state) : power_pin (pin)
{
	//power_pin = pin;
	_pin_low_init_out_pp (const_cast<S_GPIOPIN*>(power_pin), 1);
	Power_enabled (f_state);

	tx_period_timer = 1000;
	SYSBIOS::ADD_TIMER_SYS (&tx_period_timer);	
	status_period_timer = 1000;
	SYSBIOS::ADD_TIMER_SYS (&status_period_timer);	
	mydata_period_timer = 1000;
	SYSBIOS::ADD_TIMER_SYS (&mydata_period_timer);
	accell_timer = 0;
	SYSBIOS::ADD_TIMER_SYS (&accell_timer);
	f_status_que = false;
	sw = EAK70SW_GETSTATE;
	sw_next = EAK70SW_GETSTATE;
	cur_speed = 0;
	cur_pos = 0;
	prev_quant_pos = 201;
	RotateCounterAK = 0;
	CoilMetter = 0;
	mode = EAKMODE_AUTO;
	key_state = EMTRDIRMOV_NONE;
	c_accel_max = 14;
	f_pos_back_run = false;
	c_accel_incdec_val = 0.2;
}




void TAK70::Init ()
{
	CAN_FilterTypeDef  sFilterConfig;
	GPIO_InitTypeDef   GPIO_InitStruct;
	CAN_HandleTypeDef     CanHandle;


	__HAL_RCC_CAN1_CLK_ENABLE ();
	hard_gpio_clock_enable (GPIOD);

	GPIO_InitStruct.Pin = GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Alternate =  GPIO_AF9_CAN1;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	/* CAN1 RX GPIO pin configuration */
	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Alternate =  GPIO_AF9_CAN1;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);


	/*##-3- Configure the NVIC #################################################*/
	/* NVIC configuration for CAN1 Reception complete interrupt */
	// is pending mode
	//HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 1, 0);
	//HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);




	/*##-1- Configure the CAN peripheral #######################################*/
	CanHandle.Instance = CAN1;

	CanHandle.Init.TimeTriggeredMode = DISABLE;
	CanHandle.Init.AutoBusOff = DISABLE;
	CanHandle.Init.AutoWakeUp = DISABLE;
	CanHandle.Init.AutoRetransmission = DISABLE;//ENABLE;
	CanHandle.Init.ReceiveFifoLocked = DISABLE;
	CanHandle.Init.TransmitFifoPriority = DISABLE;
	CanHandle.Init.Mode = CAN_MODE_NORMAL;
	CanHandle.Init.SyncJumpWidth = CAN_SJW_1TQ;// CAN_SJW_3TQ;//CAN_SJW_1TQ;
	CanHandle.Init.TimeSeg1 = CAN_BS1_3TQ;
	CanHandle.Init.TimeSeg2 = CAN_BS2_3TQ;
	CanHandle.Init.Prescaler = 6;//5;



	  if (HAL_CAN_Init(&CanHandle) != HAL_OK)
	  {
	    /* Initialization Error */
	    //Error_Handler();
	  }

	  /*##-2- Configure the CAN Filter ###########################################*/
	  sFilterConfig.FilterBank = 0;
	  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	  sFilterConfig.FilterIdHigh = 0x0000;
	  sFilterConfig.FilterIdLow = 0x0000;
	  sFilterConfig.FilterMaskIdHigh = 0x0000;
	  sFilterConfig.FilterMaskIdLow = 0x0000;
	  sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
	  sFilterConfig.FilterActivation = ENABLE;
	  sFilterConfig.SlaveStartFilterBank = 14;


	  if (HAL_CAN_ConfigFilter(&CanHandle, &sFilterConfig) != HAL_OK)
	  {
	    /* Filter configuration Error */
	    //Error_Handler();
	  }

	  /*##-3- Start the CAN peripheral ###########################################*/
	  if (HAL_CAN_Start(&CanHandle) != HAL_OK)
	  {
	    /* Start Error */
	    //Error_Handler();
	  }

	  /*##-4- Activate CAN RX notification #######################################*/
	  if (HAL_CAN_ActivateNotification(&CanHandle, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
	  {
	    /* Notification Error */
	    //Error_Handler();
	  }

	  /*##-5- Configure Transmission process #####################################*/
	  /*
	  TxHeader.StdId = 0x321;
	  TxHeader.ExtId = 0x01;
	  TxHeader.RTR = CAN_RTR_DATA;
	  TxHeader.IDE = CAN_ID_STD;
	  TxHeader.DLC = 2;
	  TxHeader.TransmitGlobalTime = DISABLE;
	  */
}

/*
void TAK70::Init ()
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);


  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);


  CAN_DeInit(CAN1);
  CAN_StructInit(&CAN_InitStructure);


  CAN_InitStructure.CAN_TTCM = DISABLE;
  CAN_InitStructure.CAN_ABOM = DISABLE;
  CAN_InitStructure.CAN_AWUM = DISABLE;
  CAN_InitStructure.CAN_NART = DISABLE;
  CAN_InitStructure.CAN_RFLM = DISABLE;
  CAN_InitStructure.CAN_TXFP = DISABLE;
  CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;
  

  CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
  CAN_InitStructure.CAN_BS1 = CAN_BS1_3tq;
  CAN_InitStructure.CAN_BS2 = CAN_BS2_2tq;
  CAN_InitStructure.CAN_Prescaler = 6;
  CAN_Init(CAN1, &CAN_InitStructure);


  CAN_FilterInitStructure.CAN_FilterNumber = 0;

  CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
  CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
  CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
  CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
  CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
  CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
  CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 0;
  CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
  CAN_FilterInit(&CAN_FilterInitStructure);
}
*/



uint8_t CAN_TransmitStatus(CAN_TypeDef* CANx, uint8_t TransmitMailbox)
{
  uint32_t state = 0;

  /* Check the parameters */
  assert_param(IS_CAN_ALL_PERIPH(CANx));
  assert_param(IS_CAN_TRANSMITMAILBOX(TransmitMailbox));

  switch (TransmitMailbox)
  {
    case (CAN_TXMAILBOX_0):
      state =   CANx->TSR &  (CAN_TSR_RQCP0 | CAN_TSR_TXOK0 | CAN_TSR_TME0);
      break;
    case (CAN_TXMAILBOX_1):
      state =   CANx->TSR &  (CAN_TSR_RQCP1 | CAN_TSR_TXOK1 | CAN_TSR_TME1);
      break;
    case (CAN_TXMAILBOX_2):
      state =   CANx->TSR &  (CAN_TSR_RQCP2 | CAN_TSR_TXOK2 | CAN_TSR_TME2);
      break;
    default:
      state = CAN_TxStatus_Failed;
      break;
  }
  switch (state)
  {
      /* transmit pending  */
    case (0x0): state = CAN_TxStatus_Pending;
      break;
      /* transmit failed  */
     case (CAN_TSR_RQCP0 | CAN_TSR_TME0): state = CAN_TxStatus_Failed;
      break;
     case (CAN_TSR_RQCP1 | CAN_TSR_TME1): state = CAN_TxStatus_Failed;
      break;
     case (CAN_TSR_RQCP2 | CAN_TSR_TME2): state = CAN_TxStatus_Failed;
      break;
      /* transmit succeeded  */
    case (CAN_TSR_RQCP0 | CAN_TSR_TXOK0 | CAN_TSR_TME0):state = CAN_TxStatus_Ok;
      break;
    case (CAN_TSR_RQCP1 | CAN_TSR_TXOK1 | CAN_TSR_TME1):state = CAN_TxStatus_Ok;
      break;
    case (CAN_TSR_RQCP2 | CAN_TSR_TXOK2 | CAN_TSR_TME2):state = CAN_TxStatus_Ok;
      break;
    default: state = CAN_TxStatus_Failed;
      break;
  }
  return (uint8_t) state;
}




void TAK70::Task_auto ()
{
if (!tx_period_timer)
			{
			sw = sw_next;
			uint8_t rslt = CAN_TransmitStatus (CAN1, 0);
			if (rslt != CAN_TxStatus_Pending)	
				{
				static unsigned short cntr;
				static unsigned short cntr_down;
				switch (sw)
					{
					case EAK70SW_NONE:
						{
						break;
						}
					case EAK70SW_GETSTATE:
						{						
						send_cmd_zeropos ();
						tx_period_timer = 50;	
						sw_next = EAK70SW_INIT;
						break;
						}
					case EAK70SW_INIT:
						{						
						send_cmd_read_status ();			// 1 step - motor on
						tx_period_timer = 50;	
						sw_next = EAK70SW_INIT_B;
						break;
						}
					case EAK70SW_INIT_B:
						{						
						send_cmd_zeropos ();
						tx_period_timer = 50;	
						cur_speed = 0;
						cur_cur = 0.0;
						sw_next = EAK70SW_PASIVE;
						break;
						}
					case EAK70SW_STOP_0:
						{
						cntr = 70;
						cntr_down = 70;
						cur_speed = 0;
						cur_cur = 0;
						tx_period_timer = 100;
						sw_next = EAK70SW_LEFT;
						break;
						}
					case EAK70SW_LEFT:
						{
						if (cntr)
							{
							cur_speed += -0.2;
							cur_cur = -1;
							tx_period_timer = 100;
							cntr--;
							}
						else
							{
							if (cntr_down)
								{
								cur_speed += 0.2;
								cur_cur = -1;
								tx_period_timer = 100;
								cntr_down--;
								}
							else
								{
								sw_next = EAK70SW_STOP_1;
								}
							}
						break;
						}
					case EAK70SW_STOP_1:
						{
						cntr = 70;
						cntr_down = 70;
						cur_speed = 0;								// speed = 0
						cur_cur = 0;
						cur_speed = 0;
						tx_period_timer = 100;
						sw_next = EAK70SW_RIGHT;
						break;
						}
					case EAK70SW_RIGHT:
						{
						if (cntr)
							{
							cur_speed += 0.2;
							cur_cur = 1;
							tx_period_timer = 100;
							cntr--;
							}
						else
							{
							if (cntr_down)
								{
								cur_speed += -0.2;
								cur_cur = 1;
								tx_period_timer = 100;
								cntr_down--;
								}
							else
								{
								sw_next = EAK70SW_STOP_2;
								}
							}
						break;
						}
					case EAK70SW_STOP_2:
						{
						cur_speed = 0;
						cur_cur = 0;
						tx_period_timer = 100;
						sw_next = EAK70SW_START_RIGHT_SPEED_A;
						cntr = 2;
						break;
						}
					case EAK70SW_START_RIGHT_SPEED_A:
						{
						cur_speed = 0.01;
						cur_cur = 1;
						tx_period_timer = 3000;
						sw_next = EAK70SW_START_RIGHT_SPEED_B;
						break;
						}
					case EAK70SW_START_RIGHT_SPEED_B:
						{
						cur_speed = -0.01;
						cur_cur = -1;
						tx_period_timer = 3000;
						sw_next = EAK70SW_START_RIGHT_SPEED_A;
						if (cntr)
							{
							cntr--;
							if (!cntr)
								{
								sw_next = EAK70SW_PASIVE;
								cur_speed = 0;
								cur_cur = 0;
								send_cmd_read_status ();	
								}
							}
						break;
						}
					case EAK70SW_PASIVE:
						{
						//cur_speed = 0;
						//cur_cur = 0.0;
						tx_period_timer = 1000;
						status_period_timer = 100;
						break;
						}
					case EAK70SW_SERVOTEST:
						{
						cur_pos = 0;
						cur_cur = 1.5;
						tx_period_timer = 1000;
						//send_current_servo_data ();
						pos_add_val = 0.1;
						break;
						}
					case EAK70SW_SERV_RUN:
						{
						cur_cur = 1;
						//pos_add_val=1;
						cur_pos += pos_add_val;
						/*
						if (cur_pos > 95)
							{
							cur_pos = -95;
							pos_add_val = 0.1;
							}
						else
							{
							if (cur_pos < -95) 
								{
								pos_add_val = -0.1;
								cur_pos = 95;
								}
							}
						*/
						
						//send_current_servo_data ();
						//status_period_timer = 50;
						tx_period_timer = 20;
						//status_period_timer = 100;
						//send_current_servo_data ();
						break;
						}
					case EAK70SW_EXIT:
						{
						send_cmd_exit ();
						tx_period_timer = 10;
						sw_next = EAK70SW_NONE;
						break;
						}
					default:
						{
						break;
						}
					}
				}
			else
				{
				tx_period_timer = 20;
				}
			}

}



uint8_t CAN_MessagePending(CAN_TypeDef* CANx, uint8_t FIFONumber)
{
  uint8_t message_pending=0;
  /* Check the parameters */
  if (FIFONumber == CAN_FIFO0)
  {
    message_pending = (uint8_t)(CANx->RF0R&(uint32_t)0x03);
  }
  else if (FIFONumber == CAN_FIFO1)
  {
    message_pending = (uint8_t)(CANx->RF1R&(uint32_t)0x03);
  }
  else
  {
    message_pending = 0;
  }
  return message_pending;
}


void TAK70::Task_Manual ()
{
}



void CAN_Receive(CAN_TypeDef* CANx, uint8_t FIFONumber, CanRxMsg* RxMessage)
{

  /* Get the Id */
  RxMessage->IDE = (uint8_t)0x04 & CANx->sFIFOMailBox[FIFONumber].RIR;
  if (RxMessage->IDE == CAN_Id_Standard)
  {
    RxMessage->StdId = (uint32_t)0x000007FF & (CANx->sFIFOMailBox[FIFONumber].RIR >> 21);
  }
  else
  {
    RxMessage->ExtId = (uint32_t)0x1FFFFFFF & (CANx->sFIFOMailBox[FIFONumber].RIR >> 3);
  }

  RxMessage->RTR = (uint8_t)0x02 & CANx->sFIFOMailBox[FIFONumber].RIR;
  /* Get the DLC */
  RxMessage->DLC = (uint8_t)0x0F & CANx->sFIFOMailBox[FIFONumber].RDTR;
  /* Get the FMI */
  RxMessage->FMI = (uint8_t)0xFF & (CANx->sFIFOMailBox[FIFONumber].RDTR >> 8);
  /* Get the data field */
  RxMessage->Data[0] = (uint8_t)0xFF & CANx->sFIFOMailBox[FIFONumber].RDLR;
  RxMessage->Data[1] = (uint8_t)0xFF & (CANx->sFIFOMailBox[FIFONumber].RDLR >> 8);
  RxMessage->Data[2] = (uint8_t)0xFF & (CANx->sFIFOMailBox[FIFONumber].RDLR >> 16);
  RxMessage->Data[3] = (uint8_t)0xFF & (CANx->sFIFOMailBox[FIFONumber].RDLR >> 24);
  RxMessage->Data[4] = (uint8_t)0xFF & CANx->sFIFOMailBox[FIFONumber].RDHR;
  RxMessage->Data[5] = (uint8_t)0xFF & (CANx->sFIFOMailBox[FIFONumber].RDHR >> 8);
  RxMessage->Data[6] = (uint8_t)0xFF & (CANx->sFIFOMailBox[FIFONumber].RDHR >> 16);
  RxMessage->Data[7] = (uint8_t)0xFF & (CANx->sFIFOMailBox[FIFONumber].RDHR >> 24);
  /* Release the FIFO */
  /* Release FIFO0 */
  if (FIFONumber == CAN_FIFO0)
  {
    CANx->RF0R |= CAN_RF0R_RFOM0;
  }
  /* Release FIFO1 */
  else /* FIFONumber == CAN_FIFO1 */
  {
    CANx->RF1R |= CAN_RF1R_RFOM1;
  }
}



void motor_receive(float* motor_pos, float* motor_spd, float* motor_cur, int8_t* motor_temp, int8_t* motor_error, CanRxMsg* message) {

  int16_t pos_int = message->Data[0] << 8 | message->Data[1];
  int16_t spd_int = message->Data[2] << 8 | message->Data[3];
  int16_t cur_int = message->Data[4] << 8 | message->Data[5];
  if (motor_pos)*motor_pos = (float)(pos_int * 0.1f);
  if (motor_spd)*motor_spd = (float)(spd_int * 10.0f);
  if (motor_cur)*motor_cur = (float)(cur_int * 0.01f);
  if (motor_temp)*motor_temp = message->Data[6];
  if (motor_error)*motor_error = message->Data[7];
}




void TAK70::Task ()
{
unsigned long cur_time_stamp = SYSBIOS::GetTickCountLong ();
f_motor_link_state = ((cur_time_stamp - last_timestamp_rx_state) > 1000)? false:true;

		

		if (!status_period_timer)
			{
			uint8_t rslt = CAN_TransmitStatus (CAN1, 0);
			if (rslt != CAN_TxStatus_Pending)	
				{
				switch (mode)
					{
					case EAKMODE_AUTO:
						{
						//send_current_motor_data ();
						break;
						}
					case EAKMODE_MANUAL:
						{
						//send_current_motor_data ();
						break;
						}
					default:
						{
						break;
						}
					}
				//status_period_timer = 33;
				status_period_timer = 1000;
				}
			}
			
accell_task ();
//pos_back_task ();
	
if (!mydata_period_timer)
	{
	uint8_t rslt = CAN_TransmitStatus (CAN1, 0);
	//if (rslt != CAN_TxStatus_Pending)	send_my_params ();
	mydata_period_timer = 150;
	}
	
				switch (mode)
					{
					case EAKMODE_AUTO:
						{
					//	Task_auto ();
						break;
						}
					case EAKMODE_MANUAL:
						{
					//	Task_Manual ();
						break;
						}
					default:
						{
						break;
						}
					}



		
	
	static uint8_t rcv_frm_cnt;
	rcv_frm_cnt = CAN_MessagePending (CAN1, CAN_FIFO0);
	if (rcv_frm_cnt)
		{
		CanRxMsg msg;
		CAN_Receive(CAN1, CAN_FIFO0, &msg);
		if (true) {		// msg.StdId == 0 && msg.Data[0] == 0
			float motor_pos, motor_spd, motor_cur;//
			int8_t motor_temp, motor_error;
			motor_receive(&motor_pos, &motor_spd, &motor_cur, &motor_temp, &motor_error, &msg);
			//unpack_reply (msg, state_pos, state_speed, state_current);
			last_timestamp_rx_state = cur_time_stamp;
			//movement_task (state_pos);
			f_new_motor_state = true;
			
			if ((cur_time_stamp - last_fps_time) >= 1000)
				{
				cntr_fps = cntr_fps_acc;
				cntr_fps_acc = 0;
				last_fps_time = cur_time_stamp;
				}
			else
				{
				cntr_fps_acc++;
				}
			}
		}
}



uint8_t CAN_Transmit(CAN_TypeDef* CANx, CanTxMsg* TxMessage)
{
  uint8_t transmit_mailbox = 0;


  /* Select one empty transmit mailbox */
  if ((CANx->TSR&CAN_TSR_TME0) == CAN_TSR_TME0)
  {
    transmit_mailbox = 0;
  }
  else if ((CANx->TSR&CAN_TSR_TME1) == CAN_TSR_TME1)
  {
    transmit_mailbox = 1;
  }
  else if ((CANx->TSR&CAN_TSR_TME2) == CAN_TSR_TME2)
  {
    transmit_mailbox = 2;
  }
  else
  {
    transmit_mailbox = CAN_TxStatus_NoMailBox;
  }

  if (transmit_mailbox != CAN_TxStatus_NoMailBox)
  {
    /* Set up the Id */
    CANx->sTxMailBox[transmit_mailbox].TIR &= TMIDxR_TXRQ;
    if (TxMessage->IDE == CAN_Id_Standard)
    {
      assert_param(IS_CAN_STDID(TxMessage->StdId));
      CANx->sTxMailBox[transmit_mailbox].TIR |= ((TxMessage->StdId << 21) | \
                                                  TxMessage->RTR);
    }
    else
    {
      assert_param(IS_CAN_EXTID(TxMessage->ExtId));
      CANx->sTxMailBox[transmit_mailbox].TIR |= ((TxMessage->ExtId << 3) | \
                                                  TxMessage->IDE | \
                                                  TxMessage->RTR);
    }

    /* Set up the DLC */
    TxMessage->DLC &= (uint8_t)0x0000000F;
    CANx->sTxMailBox[transmit_mailbox].TDTR &= (uint32_t)0xFFFFFFF0;
    CANx->sTxMailBox[transmit_mailbox].TDTR |= TxMessage->DLC;

    /* Set up the data field */
    CANx->sTxMailBox[transmit_mailbox].TDLR = (((uint32_t)TxMessage->Data[3] << 24) |
                                             ((uint32_t)TxMessage->Data[2] << 16) |
                                             ((uint32_t)TxMessage->Data[1] << 8) |
                                             ((uint32_t)TxMessage->Data[0]));
    CANx->sTxMailBox[transmit_mailbox].TDHR = (((uint32_t)TxMessage->Data[7] << 24) |
                                             ((uint32_t)TxMessage->Data[6] << 16) |
                                             ((uint32_t)TxMessage->Data[5] << 8) |
                                             ((uint32_t)TxMessage->Data[4]));
    /* Request transmission */
    CANx->sTxMailBox[transmit_mailbox].TIR |= TMIDxR_TXRQ;
  }
  return transmit_mailbox;
}



void TAK70::send_my_params ()
{
	TxMsg.StdId = 0x55;//std_id;
	TxMsg.ExtId = 0;
	TxMsg.RTR = CAN_RTR_DATA;
	TxMsg.IDE = CAN_ID_STD;
	TxMsg.DLC = 8;
	
	S_MYPARAMS_T *frame = (S_MYPARAMS_T*)TxMsg.Data;
	frame->metter = CoilMetter;
	frame->last_dir = last_detect_dir;
	frame->is_move = (detect_dir != EMTRDIRMOV_NONE)?true:false;
	frame->fps = cntr_fps;
	
	CAN_Transmit (CAN1, &TxMsg);
	status_period_timer = 10;	// ���������� �� 100 �� ������� ����������� �������
}



static void *memcpy ( void * destination, const void * source, long num )
{
	char *lDst = (char*)destination, *lSrc = (char*)source;
while (num)
	{
	*lDst++ = lSrc[0];
	lSrc++;
	num--;
	}
return lDst;
}


void TAK70::send_cmd_read_status ()
{
	TxMsg.StdId = 1;//std_id;
	TxMsg.ExtId = 0;
	TxMsg.RTR = CAN_RTR_DATA;
	TxMsg.IDE = CAN_ID_STD;
	TxMsg.DLC = 8;
	
	memcpy (TxMsg.Data, seq_cmd_status, 8 );
	CAN_Transmit (CAN1, &TxMsg);
	status_period_timer = 100;	// ���������� �� 100 �� ������� ����������� �������
}



void TAK70::send_cmd_zeropos ()
{
	TxMsg.StdId = 1;//std_id;
	TxMsg.ExtId = 0;
	TxMsg.RTR = CAN_RTR_DATA;
	TxMsg.IDE = CAN_ID_STD;
	TxMsg.DLC = 8;
	
	memcpy (TxMsg.Data, seq_cmd_poszero, 8 );
	CAN_Transmit (CAN1, &TxMsg);
	status_period_timer = 100;	// ���������� �� 100 �� ������� ����������� �������
}




void TAK70::send_cmd_unlock ()
{
	TxMsg.StdId = 1;//std_id;
	TxMsg.ExtId = 0;
	TxMsg.RTR = CAN_RTR_DATA;
	TxMsg.IDE = CAN_ID_STD;
	TxMsg.DLC = 8;
	
	memcpy (TxMsg.Data, seq_cmd_unlock, 8 );
	CAN_Transmit (CAN1, &TxMsg);
	status_period_timer = 100;	// ���������� �� 100 �� ������� ����������� �������
}



void TAK70::send_cmd_exit ()
{
	TxMsg.StdId = 1;//std_id;
  TxMsg.ExtId = 0;
  TxMsg.RTR = CAN_RTR_DATA;
  TxMsg.IDE = CAN_ID_STD;
  TxMsg.DLC = 8;
	
	memcpy (TxMsg.Data, seq_cmd_exit, 8 );
	CAN_Transmit (CAN1, &TxMsg);
	status_period_timer = 100;	// ���������� �� 100 �� ������� ����������� �������
}



void TAK70::send_current_motor_data ()	// float spd, float cur
{
	TxMsg.StdId = 1;
	TxMsg.ExtId = 1;
	TxMsg.RTR = CAN_RTR_DATA;
	TxMsg.IDE = CAN_ID_STD;
	TxMsg.DLC = 8;

	//float cur = 1;
	//if (cur_speed < 0) cur = cur * -1;
	
	
	SetSpeed (cur_speed);
	SetCurrent (cur_cur);
	if (cur_cur == 0) 
		{
		SetKD (0);
		}
	else
		{
		SetKD (1);
		}
		
	SetKP (0);
	SetPosition (0);	
	
	CAN_Transmit (CAN1, &TxMsg);
}



/*
void TAK70::send_current_servo_data ()
{
  TxMessage.StdId = 1;
  TxMessage.ExtId = 1;
  TxMessage.RTR = CAN_RTR_DATA;
  TxMessage.IDE = CAN_ID_STD;
  TxMessage.DLC = 8;		


	SetSpeed (0);
	SetCurrent (cur_cur);
	if (cur_cur == 0) 
		{
		SetKP (0);
		}
	else
		{
		SetKP (10);
		}
	SetKD (0);	
	SetPosition (cur_pos);	

	
	CAN_Transmit (CAN1, &TxMessage);
}
*/


float TAK70::GetSpeed ()
{
	return state_speed;
}



float TAK70::GetPos ()
{
	return state_pos;
}



float TAK70::GetCurrent ()
{
	return state_current;
}



bool TAK70::GetLink ()
{
	return f_motor_link_state;
}



float TAK70::conv_quant_191 (float val)
{
	return val + C_QUANT_AK70_POLE;
}



void TAK70::movement_task (float curpos_pos)
{
curpos_pos = conv_quant_191 (curpos_pos);
if (prev_quant_pos < 200)
	{
	float delt = curpos_pos - prev_quant_pos;
	float abs_delt = (delt < 0)?delt * -1: delt;
	if (abs_delt >= C_RPQ_AK70)
		{
		// �� ���������� �������� ������ ����������, ��� ������� ����� ��������� ����� �������
		if (curpos_pos < prev_quant_pos)
			{
			detect_dir = EMTRDIRMOV_UP;
			delt = C_QUANT_AK70_DIAP - prev_quant_pos;
			delt += curpos_pos;
			}
		else
			{
			delt = C_QUANT_AK70_DIAP - curpos_pos;
			delt += prev_quant_pos;
			delt *= -1;
			detect_dir = EMTRDIRMOV_DOWN;
			}
		}
	else
		{
		if (delt < 0)
			{
			detect_dir = EMTRDIRMOV_DOWN;
			}
		else
			{
			if (delt > 0)
				{
				detect_dir = EMTRDIRMOV_UP;
				}
			else
				{
				detect_dir = EMTRDIRMOV_NONE;
				}
			}
		}	
	RotateCounterAK += delt;
	}
else
	{
	detect_dir = EMTRDIRMOV_NONE;		// ����������� �� ����������
	}
prev_quant_pos = curpos_pos;

if (detect_dir != EMTRDIRMOV_NONE) last_detect_dir = 	detect_dir;
	
CoilMetter = get_wire_external_lenght () / 100;
}



float TAK70::get_wire_external_lenght ()
{
	float rp_count = RotateCounterAK / C_RPQ_AK70;
	float lenght = C_COIL_DIAMETER;
	lenght *= C_PI;
	return rp_count * lenght;
}



void TAK70::SetMode (EAKMODE  mdd)
{
	if (mdd != mode)
		{
		switch (mode)
			{
			case EAKMODE_AUTO:
				{
				break;
				}
			case EAKMODE_MANUAL:
				{
				manual_point_pos = state_pos;
				manual_point_metter = CoilMetter;
				break;
				}
			default:
				{
				break;
				}
			}
		mode = mdd;
		}
}



EAKMODE TAK70::GetMode ()
{
	return mode;
}



void TAK70::KeyPush (EMTRDIRMOV ckcod)
{
	if (key_state != ckcod)
		{
		switch (ckcod)
			{
			case EMTRDIRMOV_UP:
				{
				break;
				}
			case EMTRDIRMOV_DOWN:
				{

				accel_state = EACCL_INC;
				break;
				}
			default:
				{
				manual_point_pos = state_pos;
				manual_point_metter = CoilMetter;
				accel_state = EACCL_DEC;
				break;
				}
			}
		key_state = ckcod;
		}
}



void TAK70::accell_task ()
{
	if (!accell_timer)
		{
		switch (accel_state)
			{
			case EACCL_DEC:
				{
				float valdec = c_accel_incdec_val * 2;
				if (valdec < cur_speed)
					{
					cur_speed -= valdec;
					cur_cur = 0.5 + (cur_speed/4);
					accell_timer = 40;
					}
				else
					{
					cur_speed = 0;
					cur_cur = 0;
					accel_state = EACCL_NOME;		// ��������� ����������
					accell_timer = 100;
					}
				break;
				}
			case EACCL_INC:
				{
				cur_speed += c_accel_incdec_val;
				cur_cur = 1 + (cur_speed/4);
				if (cur_speed > c_accel_max)
					{
					cur_speed = c_accel_max;
					}
				accell_timer = 80;
				break;
				}
			default:
				{
				accell_timer = 200;
				break;
				}
			}
		}
}



void TAK70::pos_back_task ()
{
	if (CoilMetter > manual_point_metter)
		{
		float delt = CoilMetter - manual_point_metter;
		if (delt > 0.05)
			{
			if (!f_pos_back_run)
				{
				f_pos_back_run = true;
				accel_state = EACCL_INC;
				}
			}
		else
			{
			if (f_pos_back_run)
				{
				accel_state = EACCL_DEC;
				f_pos_back_run = false;
				}
			}
		}
	else
		{
		if (f_pos_back_run)
			{
			accel_state = EACCL_DEC;
			f_pos_back_run = false;
			}
		}
}

