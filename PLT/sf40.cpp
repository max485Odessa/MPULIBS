#include "sf40.h"

//THC595 TSF40::latch_lpn;
//THC595 *TSF40::latch_lpn;

TSF40::TSF40 ()
{
	AddObjectToExecuteManager ();
	f_is_working = false;
	tx_relax_period = 1000;
	SYSBIOS::ADD_TIMER_SYS (&tx_relax_period);
	Angle_clear ();
}



void TSF40::Init (unsigned long speed_set)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	huart.Instance = C_SF40_USART;
	nvic_irq = C_SF40_NVIC_IRQ;
	lFS_PUSART[EUSARTIX_SF40] = (TUSART_IFACE*)this;
	
	//latch_lpn.WriteByte (20);
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_USART1_CLK_ENABLE ();
	
	tx_size = 0;
	f_tx_status = false;
	
	GPIO_InitStructure.Pin = C_SF40_PIN_TX | C_SF40_PIN_RX;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStructure.Alternate = GPIO_AF7_USART1;
	HAL_GPIO_Init (C_SF40_GPIO_PORT, &GPIO_InitStructure);
	
	Set_usart_speed (speed_set);
  huart.Init.WordLength = UART_WORDLENGTH_8B;
  huart.Init.StopBits = UART_STOPBITS_1;
  huart.Init.Parity = UART_PARITY_NONE;
  huart.Init.Mode = UART_MODE_TX_RX;
  huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart.Init.OverSampling = UART_OVERSAMPLING_8;
  huart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_MSBFIRST_INIT;//UART_ADVFEATURE_NO_INIT;
  HAL_UART_Init(&huart);
	
	NVIC_USART_ENABLE (true);		
	USART_ITConfig_RX (true);	
  USART_Cmd (ENABLE);
}



void TSF40::Task ()
{
if (!GetTxStatus ()) return;
	
	if (f_is_receive)
		{
		parse_req();
		f_is_receive = false;
		return;
		}
		
		
	if (f_is_working)
		{
		if (!tx_relax_period)
			{
			send_distance_array ();
			tx_relax_period = 5;
			}
		}
}



void TSF40::ISR_TxEnd_cb ()
{
}



void TSF40::ISR_DataRx (unsigned char dat)
{
	parse_byte (dat);
}



// convert buffer to uint32, uint16
uint32_t TSF40::buff_to_uint32(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3) const
{
    uint32_t leval = (uint32_t)b0 | (uint32_t)b1 << 8 | (uint32_t)b2 << 16 | (uint32_t)b3 << 24;
    return leval;
}



uint16_t TSF40::buff_to_uint16(uint8_t b0, uint8_t b1) const
{
    uint16_t leval = (uint16_t)b0 | (uint16_t)b1 << 8;
    return leval;
}



void TSF40::parse_byte(uint8_t b)
{
    // check that payload buffer is large enough
    static_assert(ARRAY_SIZE(_msg.payload) == PROXIMITY_SF40C_PAYLOAD_LEN_MAX, "AP_Proximity_LightWareSF40C: check _msg.payload array size");
		
    // process byte depending upon current state
    switch (_msg.state) {

    case ParseState::HEADER:
        if (b == PROXIMITY_SF40C_HEADER) {
            _msg.crc_expected = crc_xmodem_update(0, b);
            _msg.state = ParseState::FLAGS_L;
        }
        break;

    case ParseState::FLAGS_L:
        _msg.flags_low = b;
        _msg.crc_expected = crc_xmodem_update(_msg.crc_expected, b);
        _msg.state = ParseState::FLAGS_H;
        break;

    case ParseState::FLAGS_H:
        _msg.flags_high = b;
        _msg.crc_expected = crc_xmodem_update(_msg.crc_expected, b);
        _msg.payload_len = UINT16_VALUE(_msg.flags_high, _msg.flags_low) >> 6;
        if ((_msg.payload_len == 0) || (_msg.payload_len > PROXIMITY_SF40C_PAYLOAD_LEN_MAX)) {
            // invalid payload length, abandon message
            _msg.state = ParseState::HEADER;
        } else {
            _msg.state = ParseState::MSG_ID;
        }
        break;

    case ParseState::MSG_ID:
        _msg.msgid = (MessageID)b;
        _msg.crc_expected = crc_xmodem_update(_msg.crc_expected, b);
        if (_msg.payload_len > 1) {
            _msg.state = ParseState::PAYLOAD;
        } else {
            _msg.state = ParseState::CRC_L;
        }
        _msg.payload_recv = 0;
        break;

    case ParseState::PAYLOAD:
        if (_msg.payload_recv < (_msg.payload_len - 1)) {
            _msg.payload[_msg.payload_recv] = b;
            _msg.payload_recv++;
            _msg.crc_expected = crc_xmodem_update(_msg.crc_expected, b);
        }
        if (_msg.payload_recv >= (_msg.payload_len - 1)) {
            _msg.state = ParseState::CRC_L;
        }
        break;

    case ParseState::CRC_L:
        _msg.crc_low = b;
        _msg.state = ParseState::CRC_H;
        break;

    case ParseState::CRC_H:
        _msg.crc_high = b;
        if (_msg.crc_expected == UINT16_VALUE(_msg.crc_high, _msg.crc_low)) {
						f_is_receive = true;
            //parse_req();
            _last_reply_ms = SYSBIOS::GetTickCount ();
        }
        _msg.state = ParseState::HEADER;
        break;
    }
}



bool TSF40::parse_req ()
{
	bool rv = false;
	switch (_msg.msgid) {
		case MessageID::MOTOR_STATE:
			{
			S_SF40_STATE_PAY pay;
			pay.State = 3;
			send_message (MessageID::MOTOR_STATE, false, (const uint8_t *)&pay, sizeof(pay));	
			break;
			}
		case MessageID::STREAM:
			{
			S_SF40_STREAM_PAY *instream = (S_SF40_STREAM_PAY*)&_msg.payload;				
			if (instream->data == 3)
				{
				S_SF40_STREAM_PAY pay;
				pay.data = 3;
				send_message (MessageID::STREAM, false, (const uint8_t *)&pay, sizeof(pay));
				f_is_working = true;
				}
			else
				{
				f_is_working = false;
				}
			break;
			}
		case MessageID::OUTPUT_RATE:
			{
			S_SF40_OUTRATE_PAY *inrate = (S_SF40_OUTRATE_PAY*)&_msg.payload;
			if (inrate->data == PROXIMITY_SF40C_DESIRED_OUTPUT_RATE)
				{
				S_SF40_OUTRATE_PAY pay;
				pay.data = PROXIMITY_SF40C_DESIRED_OUTPUT_RATE;
				send_message (MessageID::OUTPUT_RATE, false, (const uint8_t *)&pay, sizeof(pay));
				}
			break;
			}
		case MessageID::TOKEN:
			{
			S_SF40_TOKKEN_PAY pay;
			pay.data[0] = 10;
			pay.data[1] = 20;
			send_message (MessageID::TOKEN, false, (const uint8_t *)&pay, sizeof(pay));
			break;
			}
		case MessageID::RESET:
			{
			break;
			}
		default:
			{
			break;
			}
		}
	return rv;
}



void TSF40::push_tx (unsigned char b)
{
	if (tx_add_ix < C_SF40TXBUF_SIZE) txbuf[tx_add_ix++] = b;
}




// send message to sensor
void TSF40::send_message(MessageID msgid, bool write, const uint8_t *payload, uint16_t payload_len)
{
    if ((payload_len > PROXIMITY_SF40C_PAYLOAD_LEN_MAX)) {
        return;
    }

    // check for sufficient space in outgoing buffer
    if (C_SF40TXBUF_SIZE < payload_len + 6U) {
        return;
    }

		tx_add_ix = 0;
    // write header
    push_tx((uint8_t)PROXIMITY_SF40C_HEADER);
    uint16_t crc = crc_xmodem_update(0, PROXIMITY_SF40C_HEADER);

    // write flags including payload length
    const uint16_t flags = ((payload_len+1) << 6) | (write ? 0x01 : 0);
    push_tx(LOWBYTE(flags));
    crc = crc_xmodem_update(crc, LOWBYTE(flags));
    push_tx(HIGHBYTE(flags));
    crc = crc_xmodem_update(crc, HIGHBYTE(flags));

    // msgid
    push_tx((uint8_t)msgid);
    crc = crc_xmodem_update(crc, (uint8_t)msgid);

    // payload
    if ((payload_len > 0) && (payload != nullptr)) {
        for (uint16_t i = 0; i < payload_len; i++) {
            push_tx(payload[i]);
            crc = crc_xmodem_update(crc, payload[i]);
        }
    }

    // checksum
    push_tx(LOWBYTE(crc));
    push_tx(HIGHBYTE(crc));
		
		RawTransmit (txbuf, tx_add_ix);
}



// 7.5 degree
void TSF40::Angle_set (unsigned short angl, unsigned short dimns)
{
	angl = angl % 360;
	const float angl_quant = 7.5;
	float val_ix = angl;
	val_ix /= angl_quant;
	lidar_points[(unsigned short)val_ix] = dimns;	
}



void TSF40::Angle_clear ()
{
	memset (lidar_points, 0, sizeof(lidar_points));
}



void TSF40::send_distance_array ()
{
	S_SF40_DISTANCE_PAY pay;
	memset (&pay, 0, sizeof(pay));
	pay.PointTotal = C_LIDAR_POINTS_MAX;
	pay.PointCount = C_LIDAR_POINTS_MAX;
	unsigned long ix = 0;
	while (ix < C_LIDAR_POINTS_MAX)
		{
		pay.array[ix] = lidar_points[ix];
		ix++;
		}
	send_message (MessageID::DISTANCE_OUTPUT, false, (const uint8_t *)&pay, sizeof(pay));
	
}






