#include "TSBUS_UAV.H"



TSBUSRECEIVER::TSBUSRECEIVER ()
{
	f_frame_new = false;
	clear_rx ();
	Timer_FrameMargin = 0;
	SYSBIOS::ADD_TIMER_ISR (&Timer_FrameMargin);
	AddObjectToExecuteManager ();
}



void TSBUSRECEIVER::clear_rx ()
{
	rx_ix = 0;
	//f_protocol_end_forming = false;
}



void TSBUSRECEIVER::ISR_DataRx (unsigned char dat)
{
	if (rx_ix < sizeof(TSBUSPROTOCOL))
		{
		bufrx[rx_ix++] = SWAPBits (dat);
		//if (rx_ix == sizeof(TSBUSPROTOCOL)) f_protocol_end_forming = true;
		}

	Timer_FrameMargin = C_SBUS_MARGIN_TIME;
		
}



void TSBUSRECEIVER::Init (USART_TypeDef *p)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USARTPort = p;
	
	USART_DeInit(USARTPort);
	
	RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd (RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd (RCC_APB2Periph_USART1, ENABLE);

	f_tx_status = false;

	// RX pin
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
  // TX pin
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

	NVIC_InitTypeDef NVIC_InitStructure;
	
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);


  USART_InitStructure.USART_BaudRate = 100000;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_2;
  USART_InitStructure.USART_Parity = USART_Parity_Even;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx;
	USART_Init(USARTPort, &USART_InitStructure);
	
	
  USART_ITConfig (USARTPort, USART_IT_RXNE, ENABLE);
  USART_Cmd (USARTPort, ENABLE);
	
	lFS_PUSART[1] = (TUSARTIFB*)this;
}




void TSBUSRECEIVER::TranslateArrayTo16Bit (void *lSrc, unsigned short *lDst, unsigned char src_bits, unsigned char amount_arr)
{
	if (amount_arr && lSrc && lDst && src_bits)
		{
		unsigned long cur_src_offset = 0;
		while (amount_arr)
			{
			*lDst++ = SwapBitsLong (CreateValueFromBitMassive ((unsigned char *)lSrc, cur_src_offset, src_bits), 11); 
			cur_src_offset += src_bits;
			amount_arr--;
			} 
		}
}



unsigned short TSBUSRECEIVER::DigChannelValue (bool val)
{
	unsigned short rv = 2047;
	if (!val) rv = 0;
	return rv;
}


void TSBUSRECEIVER::Task ()
{
	if (rx_ix >= C_SBUS_FRAME_SIZE)
		{
		if (!Timer_FrameMargin)
			{
			TSBUSPROTOCOL *p_frame = (TSBUSPROTOCOL*)bufrx; 
			if (!f_frame_new)
				{
				if (p_frame->startbyte == C_SBUS_FIST_BYTE_VALUE && p_frame->endbyte == C_SBUS_LAST_BYTE_VALUE)
					{
					// переформатируем 11 бит в 16 битный массив
					TranslateArrayTo16Bit (&p_frame->array, data_new.channels, C_SBUS_RCBITS_SIZE, C_SBUS_PROTOCOL_ARRAY_SIZE);	// 16
					data_new.channels[16] = DigChannelValue (p_frame->flags & 0x80);
					data_new.channels[17] = DigChannelValue (p_frame->flags & 0x40);
					data_new.flags = p_frame->flags;
					data_new.size = C_SBUS_ARRAY_SIZE;	// 18
					f_frame_new = true;
					}
				}

			clear_rx ();	// разблокировка приема
			}
		}
}



bool TSBUSRECEIVER::GetData (TSBUSDATA *lDst)
{
	bool rv = f_frame_new;
	if (rv && lDst)
		{
		*lDst = data_new;
		f_frame_new = false;
		}
	return rv;
}


/*
static TSBUSDATA sbus_data;


#define C_TESTRC_CHANNEL_AMOUNT 16
#define C_RC_TEST_MAX_VAL 1900
#define C_RC_TEST_MIN_VAL 1100
static unsigned short RC_array[C_TESTRC_CHANNEL_AMOUNT];
static char RC_step_dir[C_TESTRC_CHANNEL_AMOUNT];


static void RC_Generator_Init ()
{
memset ((char*)&RC_array, 0, sizeof(RC_array));
memset ((char*)&RC_step_dir, 0, sizeof(RC_step_dir));
RC_step_dir[0] = -2;
RC_step_dir[1] = 2;
RC_step_dir[2] = 1;
RC_step_dir[3] = 0;
RC_step_dir[4] = 0;
RC_step_dir[5] = 0;
RC_step_dir[6] = 0;
RC_step_dir[7] = -2;
RC_step_dir[8] = -1;
RC_step_dir[9] = -2;
RC_step_dir[10] = -1;
RC_step_dir[11] = 2;
RC_step_dir[12] = 1;
RC_step_dir[13] = -1;
RC_step_dir[14] = 1;
RC_step_dir[15] = 2;
}



static void RC_Test_Genarator ()
{

		unsigned char ix = 0;
		long rc_dat;
		char inc_dec;
		while (ix < C_TESTRC_CHANNEL_AMOUNT)
			{
			rc_dat = RC_array[ix];
			inc_dec = RC_step_dir[ix];
			if (inc_dec)
				{
				if (inc_dec > 0)
					{
					rc_dat += inc_dec;
					if (rc_dat >= C_RC_TEST_MAX_VAL)
						{
						rc_dat = C_RC_TEST_MAX_VAL;
						inc_dec = -inc_dec;
						}
					}
				else
					{
					rc_dat += inc_dec;
					if (rc_dat <= C_RC_TEST_MIN_VAL)
						{
						rc_dat = C_RC_TEST_MIN_VAL;
						inc_dec = -inc_dec;
						}
					}
				RC_array[ix] = (unsigned short)(rc_dat & 0xFFFF);
				RC_step_dir[ix] = inc_dec;
				}
			ix++;
			}
}



static void Testing_SBUS_data ()
{
	static unsigned long tick_last = 0;
	unsigned long cur_ticks = SYSBIOS::GetTickCountLong ();
	if (cur_ticks != tick_last)
		{
		RC_Test_Genarator ();
		CopyMemorySDC ((char*)&RC_array, (char*)&sbus_data.channels, C_SBUS_DATA_ARRAY * 2);
		sbus_data.flags = 0;
		sbus_data.size = C_SBUS_DATA_ARRAY;
		tick_last = cur_ticks;
		}
	
}

*/


