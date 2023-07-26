#include "TMAVLINK.h"



utimer_t TMAVLINKPRS::GlobPosTimeout_period;
float TMAVLINKPRS::last_altitude_ground_level = 0;


TMAVLINKPRS::TMAVLINKPRS ()
{
	LinkTimeout_period_usart = 0;
	SYSBIOS::ADD_TIMER_SYS (&LinkTimeout_period_usart);
	f_enable = true;
	GlobPosTimeout_period = 0;
	SYSBIOS::ADD_TIMER_SYS (&GlobPosTimeout_period);
}



void TMAVLINKPRS::Enabled (bool val)
{
	
}



void TMAVLINKPRS::ISR_DataRx (unsigned char dat)
{
if (f_enable)
	{
	if (cur_isr_bufer_ix < C_MAVRXBUF_AMOUNT && cur_isr_rx_ix < C_MAVRXBUF_SIZE)
		{
		mav_rx_buf[cur_isr_bufer_ix][cur_isr_rx_ix++] = dat;
		}
	}
}



unsigned short TMAVLINKPRS::GetRxData (uint8_t *lDst, unsigned short size_max)
//bool TMAVLINKPRS::GetRxData (uint8_t **memdata, unsigned short &size_out)
{
	unsigned short rv = 0;
	if (cur_isr_rx_ix)
		{
		__disable_irq ();		// критическая секция
		unsigned short copysz = cur_isr_rx_ix;
		if (copysz > size_max) copysz = size_max;
		CopyMemorySDC ((char*)&mav_rx_buf[cur_isr_bufer_ix][0], (char*)lDst, copysz);
		rv = copysz;
		
		cur_isr_rx_ix = 0;		// обнуляем указатель/размер
		cur_isr_bufer_ix++;		// прерывания работают на следующий буфер
		if (cur_isr_bufer_ix >= C_MAVRXBUF_AMOUNT) cur_isr_bufer_ix = 0;
		
		__enable_irq ();
		}
	
	return rv;
}




void TMAVLINKPRS::Init (USART_TypeDef *p)
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

	SetSpeed (115200);
	
  USART_ITConfig (USARTPort, USART_IT_RXNE, ENABLE);
  USART_Cmd (USARTPort, ENABLE);
	
	lFS_PUSART[1] = (TUSARTIFB*)this;
}



bool TMAVLINKPRS::GetLinkStatus ()
{
	bool rv = false;

	return rv;
}




bool TMAVLINKPRS::MavlinkFrame_Rx (mavlink_message_t &msg, mavlink_status_t &s)
{
	bool rv = false;
			
			//AddAndCountMavGuid (msg.msgid);
	
      switch(msg.msgid) 
				{
        case MAVLINK_MSG_ID_HEARTBEAT:  // #0: Heartbeat
          {
					// E.g. read GCS heartbeat and go into
					// comm lost mode if timer times out
					mavlink_heartbeat_t heartbeat;
					mavlink_msg_heartbeat_decode (&msg, &heartbeat);
					
					msg.msgid += 0;
					break;
          }
        case MAVLINK_MSG_ID_SYSTEM_TIME:
					{
					mavlink_system_time_t msag;
					mavlink_msg_system_time_decode (&msg, &msag);
					
					break;
					}
				case MAVLINK_MSG_ID_ADSB_VEHICLE:
					{
					mavlink_adsb_vehicle_t pckt;
					mavlink_msg_adsb_vehicle_decode (&msg, &pckt);
					
					ConvADSB_Frame (&pckt, &adsb_uav);
					f_new_adsb_frame = true;
					break;
					}

       default:
          break;
      }
	return rv;
}


// degrees -> radians
static inline  float radians(float deg)
{
    return deg * DEG_TO_RAD;
}

// radians -> degrees
static inline  float degrees(float rad)
{
    return rad * RAD_TO_DEG;
} 




void TMAVLINKPRS::ConvADSB_Frame (mavlink_adsb_vehicle_t *lsrc, uavcan_equipment_adsb *ldst)
{
		if (!ldst) return;
		memset (ldst, 0, sizeof(uavcan_equipment_adsb));
	
		ldst->icao_address = lsrc->ICAO_address;
		ldst->tslc = lsrc->tslc;
		ldst->latitude_deg_1e7 = lsrc->lat;
		ldst->longitude_deg_1e7 = lsrc->lon;
		ldst->alt_m = lsrc->altitude / 1000;
    //pkt.altitude = msg.alt_m * 1000;
		ldst->heading = radians((lsrc->heading / 100));
    //pkt.heading = degrees(msg.heading) * 100;
		ldst->velocity[0] = lsrc->hor_velocity / 100;
		ldst->velocity[1] = ldst->velocity[0];
    //pkt.hor_velocity = norm(msg.velocity[0], msg.velocity[1]) * 100;
		ldst->velocity[2] = -(lsrc->ver_velocity / 100);
    //pkt.ver_velocity = -msg.velocity[2] * 100;
		ldst->squawk = lsrc->squawk;

    for (uint8_t i=0; i<9; i++) {
        ldst->callsign[i] = lsrc->callsign[i];
    }
		ldst->traffic_type = lsrc->emitter_type;
		if (lsrc->flags & ADSB_FLAGS_VALID_ALTITUDE) 
			{
			switch (lsrc->altitude_type)
				{
				case ADSB_ALTITUDE_TYPE_PRESSURE_QNH:
					{
					ldst->alt_type = EALT_T_PRESSURE_AMSL;
					break;
					}
				case ADSB_ALTITUDE_TYPE_GEOMETRIC:
					{
					ldst->alt_type = EALT_T_WGS84;
					break;
					}
				}
			}
		/*
    if (msg.alt_type == ardupilot::equipment::trafficmonitor::TrafficReport::ALT_TYPE_PRESSURE_AMSL) {
        pkt.flags |= ADSB_FLAGS_VALID_ALTITUDE;
        pkt.altitude_type = ADSB_ALTITUDE_TYPE_PRESSURE_QNH;
    } else if (msg.alt_type == ardupilot::equipment::trafficmonitor::TrafficReport::ALT_TYPE_WGS84) {
        pkt.flags |= ADSB_FLAGS_VALID_ALTITUDE;
        pkt.altitude_type = ADSB_ALTITUDE_TYPE_GEOMETRIC;
    }
			*/
		if (lsrc->flags & ADSB_FLAGS_VALID_COORDS) ldst->f_lat_lon_valid = true;
		if (lsrc->flags & ADSB_FLAGS_VALID_HEADING) ldst->f_heading_valid = true;
		if (lsrc->flags & ADSB_FLAGS_VALID_VELOCITY) ldst->f_velocity_valid = true;
		if (lsrc->flags & ADSB_FLAGS_VALID_CALLSIGN) ldst->f_callsign_valid = true;
		if (lsrc->flags & ADSB_FLAGS_VALID_SQUAWK) ldst->f_ident_valid = true;
		if (lsrc->flags & ADSB_FLAGS_SIMULATED) ldst->f_simulated_report = true;
		if (lsrc->flags & ADSB_FLAGS_VERTICAL_VELOCITY_VALID) ldst->f_vertical_velocity_valid = true;
		if (lsrc->flags & ADSB_FLAGS_BARO_VALID) ldst->f_baro_valid = true;

}



uavcan_equipment_adsb *TMAVLINKPRS::GetADSB ()
{
uavcan_equipment_adsb *rv = 0;
if (f_new_adsb_frame)
	{
	rv = &adsb_uav;
	f_new_adsb_frame = false;
	}
return rv;
}



void TMAVLINKPRS::Rx_Task ()
{
  static mavlink_message_t msg;
  static mavlink_status_t status;
	static unsigned char bufrx[256];
	
	unsigned short rx_size =  GetRxData (bufrx, sizeof(bufrx));
	uint8_t c;
	uint8_t *lSrc = (uint8_t*)bufrx;
	while (rx_size) 	// формируем мавлинк сообщение
		{	
		c = *lSrc;
    if (mavlink_parse_char (MAVLINK_COMM_0, c, &msg, &status)) 
			{
			MavlinkFrame_Rx (msg, status);
			}
		lSrc++;
		rx_size--;
		}	
}



void TMAVLINKPRS::Task ()
{
Rx_Task ();
}


