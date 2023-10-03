#include "TMAVWORK.h"
#include "rutine.h"



TMAVSTREAM::TMAVSTREAM (const uint8_t c_mch, const uint32_t afcnt) : c_mavchnl (c_mch), c_alloc_fifo_cnt (afcnt)
{
	//mavfifo = new TTFIFO<S_MAVDATAFRAME_T>(c_alloc_fifo_cnt);
	mavfifolens = new TFIFOLEN (c_alloc_fifo_cnt);
}


// когда сообщение десериализировано оно добавляется в fifo
bool TMAVSTREAM::add_byte (uint8_t data)
{
	bool rv = free_space ();
	if (rv) {
		if (mavlink_parse_char (c_mavchnl, data, &mavdata_tmp.frame, 0)) 
			{
			uint32_t mavmesg_size = mavlink_msg_lenght (const_cast<mavlink_message_t*>(&mavdata_tmp.frame));
			mavfifolens->push (&mavdata_tmp.frame, mavmesg_size);
			}
		}
	return rv;
}



#ifdef SERIALDEBAG
uint32_t TMAVSTREAM::debug_peak ()
{
	return mavfifolens->statistic_bytes_peack ();
}



void TMAVSTREAM::debug_peak_clear ()
{
	mavfifolens->statistic_peack_clear ();
}

#endif


uint32_t TMAVSTREAM::is_data_count ()
{
	return mavfifolens->frame_count ();
}



void TMAVSTREAM::clear ()
{
	mavfifolens->clear ();
}



bool TMAVSTREAM::add_bytes (uint8_t *src, uint8_t sz)
{
	bool rv = true;
	while (sz)
		{
		rv = add_byte (*src++);
		if (rv == false) break;
		sz--;
		}
return rv;
}



uint32_t TMAVSTREAM::free_space ()
{
	return mavfifolens->is_free_space ();
}



bool TMAVSTREAM::push (const S_MAVDATAFRAME_T &msg_src)
{
	bool rv = false;
	uint32_t mavmesg_size = mavlink_msg_lenght (const_cast<mavlink_message_t*>(&msg_src.frame));
	if (mavfifolens->check_push_space (mavmesg_size)) {
		rv = mavfifolens->push ((void*)&msg_src.frame, mavmesg_size);
		}
	return rv;
}



uint16_t TMAVSTREAM::pop (S_MAVDATAFRAME_T &msg_dst)
{
	uint16_t rv = 0;
	uint16_t max_len = sizeof(msg_dst.frame);
	if (mavfifolens->pop (&msg_dst.frame, max_len)) rv = max_len;
	return rv;
}


#ifdef MAVSNIFF
TMAVSNIFF::TMAVSNIFF ()
{
clear ();
}



void TMAVSNIFF::clear ()
{
	snf_add_ix = 0;
	fillmem (snf_arrqg, 0, sizeof(snf_arrqg));
}



long TMAVSNIFF::snf_update (uint32_t mid, uint32_t cmdlng)
{
	long rv = -1, ix = 0;
	bool f_need_add = false;
	while (ix < snf_add_ix)
		{
		if (snf_arrqg[ix].msgid == 76)
			{
			if (snf_arrqg[ix].command_long_id == cmdlng)
				{
				addval_u32 (snf_arrqg[ix].command_cnt, 1);	// counter inc
				rv = ix;
				break;
				}
			}
		else
			{
			if (snf_arrqg[ix].msgid == mid)
				{
				addval_u32 (snf_arrqg[ix].command_cnt, 1);	// counter inc
				rv = ix;
				break;
				}
			}
		ix++;
		}
		
		if (rv == -1)
			{
			if (snf_add_ix < 255)
				{
				snf_arrqg[snf_add_ix].msgid = mid;
				snf_arrqg[snf_add_ix].command_long_id = cmdlng;
				snf_arrqg[snf_add_ix].command_cnt = 0;	// counter clear
				rv = snf_add_ix;
				snf_add_ix++;
				}
			}
	return rv;
}

#endif


TMAVCORE::TMAVCORE (TSERIALUSR *usbser, TSERIALUSR *uartser, uint16_t c_rftoqg_cnt, uint16_t c_qgtorf_cnt)
{
#ifdef SERIALDEBAG
	lost_mavlink_QG_to_RF_cnt = 0;
	lost_mavlink_RF_to_QG_cnt = 0;
	lost_serial_QG_to_RF_cnt = 0;
	lost_serial_RF_to_QG_cnt = 0;
#endif
serial_QG_obj = usbser;
serial_RF_obj = uartser;
mavatom_QG_to_RF = new TMAVSTREAM (C_MAVLNK_CH_QG_TO_MODEM, c_qgtorf_cnt);
mavatom_RF_to_QG = new TMAVSTREAM (C_MAVLNK_CH_MODEM_TO_QG, c_rftoqg_cnt);
joystick_last_start.set (20000);
restart ();
}



void TMAVCORE::send_set_mode (MAV_MODE m)
{
	set_mavmode = m;
	f_new_mavmode_data = true;
}



#ifdef SERIALDEBAG
volatile static uint32_t qg_serial_peak_rx = 0;
volatile static uint32_t qg_serial_peak_tx = 0;
volatile static uint32_t rf_serial_peak_rx = 0;
volatile static uint32_t rf_serial_peak_tx = 0;
volatile static uint32_t rf_mav_frame_peak_rx = 0;
volatile static uint32_t rf_mav_frame_peak_tx = 0;
#endif



void TMAVCORE::restart ()
{
	serial_QG_obj->clear ();
	serial_RF_obj->clear ();
	mavatom_QG_to_RF->clear ();
	mavatom_RF_to_QG->clear ();
	f_new_joystick_data = false;
	f_new_mavmode_data = false;
	f_enabled = false;
}



void TMAVCORE::enabled (bool val)
{
	f_enabled = val;
}




// работа по распределению трафика в обоих направлениях: usb->modem,  modem->usb
void TMAVCORE::Task ()
{
	static S_MAVDATAFRAME_T mavframe_tmp;
	static uint8_t rawbuf[512];
	uint32_t free_space;
	TMAVCHARIN *mav_deserializer;
	uint8_t data;
	
	if (!f_enabled) return;
	
	#ifdef SERIALDEBAG
		qg_serial_peak_rx = serial_QG_obj->debug_peak_rx ();
		qg_serial_peak_tx = serial_QG_obj->debug_peak_tx ();
		rf_serial_peak_rx = serial_RF_obj->debug_peak_rx ();
		rf_serial_peak_tx = serial_RF_obj->debug_peak_tx ();
		rf_mav_frame_peak_tx = mavatom_QG_to_RF->debug_peak ();
		rf_mav_frame_peak_rx = mavatom_RF_to_QG->debug_peak ();
	#endif
	
	if (TUSBOBJ::config_detect ())
		{
		// зафиксированно usb переподключение
		#ifdef SERIALDEBAG
			serial_QG_obj->debug_peak_clear ();
			serial_RF_obj->debug_peak_clear ();
			mavatom_QG_to_RF->debug_peak_clear ();
			mavatom_RF_to_QG->debug_peak_clear ();
		#endif
		#ifdef MAVSNIFF
			sniff_qg_modem.clear ();
			sniff_modem_qg.clear ();
		#endif
		// обнулить все фифо буфера
		// ... тут нужен код обнуления
		}
	
	// direct: serial QG tx -> (mavfifo)mav_QG_rx -> serial modem rx
	// receive byte stream to atom_mavlink deserialize fifo
	mav_deserializer = mavatom_QG_to_RF;	// мавлинк десериализатор с fifo буфером целых сообщений
	while (true) // извлекаем данные из байтового буфера usb rx, контролируем заполнение mavlink fifo rx
		{
		if (!mavatom_QG_to_RF->free_space ()) break;
		if (!serial_QG_obj->pop (data)) break;
		mav_deserializer->add_byte (data);	// добавляем байтовый поток в mavlink десериализатор
		}

	// transmit QG data to RF
	free_space = serial_RF_obj->tx_free_space ();
	if (free_space >= MAVLINK_MAX_PACKET_LEN)	
		{
		if (mavatom_QG_to_RF->pop (mavframe_tmp))
			{
			uint16_t len_tx = mavlink_msg_to_send_buffer (rawbuf, &mavframe_tmp.frame);
			if (free_space >= len_tx)
				{
				serial_RF_obj->Tx (rawbuf, len_tx);		// переносим цельное сообщение в буфер передачи usb
				}
			else
				{
				// игнорируем передачу этого сообщения и обновляем счетчик потери пакета
				#ifdef SERIALDEBAG
					lost_mavlink_QG_to_RF_cnt++;
				#endif
				}
			// запускаем функционал контроля трафика по направлению USB->MODEM
			sniff_mavlink_QG_to_RF (mavframe_tmp.frame, 0);
			}		
		}
		
	// direct: serial modem rx -> (mavfifo)mav_modem_rx -> serial usb tx
	mav_deserializer = mavatom_RF_to_QG;	// мавлинк десериализатор с fifo буфером целых сообщений
	while (true) // извлекаем данные из байтового буфера usb rx, контролируем заполнение mavlink fifo rx
		{
		if (!mavatom_RF_to_QG->free_space ()) break;
		if (!serial_RF_obj->pop (data)) break;
		mav_deserializer->add_byte (data);	// добавляем байтовый поток в mavlink десериализатор
		}
	// проверяем обьем байтного fifo буфера, свободный размер должен вмещать максимально возможный мавлинк пакет
	free_space = serial_QG_obj->tx_free_space ();
	if (free_space >= MAVLINK_MAX_PACKET_LEN)
		{
		if (mavatom_RF_to_QG->pop (mavframe_tmp))
			{
			// проверяем свободный обьем, если обьема не хватает для принятия полного сообщения - игнорируем передачу этого сообщения и обновляем счетчик потери пакета
			//uint32_t mavmesg_size = mavlink_msg_lenght (const_cast<mavlink_message_t*>(&mavframe_tmp.frame));
			uint16_t len_tx = mavlink_msg_to_send_buffer (rawbuf, &mavframe_tmp.frame);
			//mavmesg_size |= 2;
			if (free_space >= len_tx)
				{
				// переносим цельное сообщение в буфер передачи usb
				serial_QG_obj->Tx (rawbuf, len_tx);
				}
			else
				{
				#ifdef SERIALDEBAG
					lost_mavlink_RF_to_QG_cnt++;
				#endif
				}
			// запускаем функционал контроля трафика по направлению MODEM->USB
			last_system_id = mavframe_tmp.frame.sysid;
			last_component_id = mavframe_tmp.frame.compid;
			sniff_mavlink_RF_to_QG (mavframe_tmp.frame, 0);
			}		
		}
		
	joystickdata_task ();		// отправляет два rc канала (x, y) и ненажатые клавиши, все 16 в нуле.
}



bool TMAVCORE::insert_to_RF (const S_MAVDATAFRAME_T &fr)
{
	bool rv = mavatom_QG_to_RF->free_space ();
	if (rv) rv = mavatom_QG_to_RF->push (fr);
	return rv;
}



bool TMAVCORE::insert_to_QG (const S_MAVDATAFRAME_T &fr)
{
	bool rv = mavatom_RF_to_QG->free_space ();
	if (rv) rv = mavatom_RF_to_QG->push (fr);
	return rv;
}



bool TMAVCORE::is_modem_link ()
{
	return serial_RF_obj->is_link ();
}



bool TMAVCORE::is_usb_link ()
{
	return serial_QG_obj->is_link ();
}


/*
    case MAVLINK_MSG_ID_MANUAL_CONTROL:
    {
        if (msg.sysid != copter.g.sysid_my_gcs) {
            break; // only accept control from our gcs
        }

        mavlink_manual_control_t packet;
        mavlink_msg_manual_control_decode(&msg, &packet);

        if (packet.target != copter.g.sysid_this_mav) {
            break; // only accept control aimed at us
        }

        if (packet.z < 0) { // Copter doesn't do negative thrust
            break;
        }

        uint32_t tnow = AP_HAL::millis();

        manual_override(copter.channel_roll, packet.y, 1000, 2000, tnow);
        manual_override(copter.channel_pitch, packet.x, 1000, 2000, tnow, true);
        manual_override(copter.channel_throttle, packet.z, 0, 1000, tnow);
        manual_override(copter.channel_yaw, packet.r, 1000, 2000, tnow);

        // a manual control message is considered to be a 'heartbeat'
        // from the ground station for failsafe purposes
        gcs().sysid_myggcs_seen(tnow);
        break;
    }

*/

static mavlink_message_t test_frame;

// изьять параметры системы наземки из heartbeat
void TMAVCORE::sniff_mavlink_QG_to_RF (mavlink_message_t &msg, mavlink_status_t *s)
{
	
	last_seq_QG = msg.seq;
	uint32_t cmdlong = 0;
      switch(msg.msgid) 
				{
				case MAVLINK_MSG_ID_STATUSTEXT:			// 253
				case MAVLINK_MSG_ID_TIMESYNC:		// 111
				case MAVLINK_MSG_ID_PARAM_REQUEST_LIST:	// 21
					{
					break;
					}
				case MAVLINK_MSG_ID_COMMAND_LONG:		// 76
					{
					mavlink_command_long_t cmdlongframe;
					mavlink_msg_command_long_decode (&msg, &cmdlongframe);
					cmdlong = cmdlongframe.command;
					break;
					}
				case MAVLINK_MSG_ID_MANUAL_CONTROL:
					{
					mavlink_manual_control_t mancntrl;
					mavlink_msg_manual_control_decode (&msg, &mancntrl);
					test_frame = (mavlink_message_t)msg;
					cmdlong += test_frame.compid;
					break;
					}
        case MAVLINK_MSG_ID_HEARTBEAT:  // #0: Heartbeat
          {
					// E.g. read GCS heartbeat and go into
					// comm lost mode if timer times out
					mavlink_heartbeat_t heartbeat;
					mavlink_msg_heartbeat_decode (&msg, &heartbeat);
					
					msg.msgid += 0;
					break;
          }
				}
				

		sniff_qg_modem.snf_update (msg.msgid, cmdlong);
}



void TMAVCORE::sniff_mavlink_RF_to_QG (mavlink_message_t &msg, mavlink_status_t *s)
{
	if (mavatom_RF_to_QG->free_space ())
		{
		if (!test_txt_period.get ())
			{
			//StatusText ();
			test_txt_period.set (2000);
			}
		}
		uint32_t cmdlong = 0;
      switch(msg.msgid) 
				{
				case MAVLINK_MSG_ID_COMMAND_ACK:		// 7
				case MAVLINK_MSG_ID_PARAM_VALUE:		// 22
				case MAVLINK_MSG_ID_SCALED_PRESSURE:		// 29
				case MAVLINK_MSG_ID_SCALED_IMU2:		// 116
				case MAVLINK_MSG_ID_RAW_IMU:		// 27
					{
					break;
					}
				case MAVLINK_MSG_ID_RC_CHANNELS:		// 65
					{
					
					break;
					}
				case MAVLINK_MSG_ID_SERVO_OUTPUT_RAW:		// 36
				case MAVLINK_MSG_ID_MISSION_CURRENT:		// 42
				case MAVLINK_MSG_ID_NAV_CONTROLLER_OUTPUT:		// 62
				case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:		// 3
				case MAVLINK_MSG_ID_VFR_HUD:		// 74
				case MAVLINK_MSG_ID_ALTITUDE:		// 30
				case MAVLINK_MSG_ID_SYSTEM_TIME:	// 2
				case MAVLINK_MSG_ID_GPS_RAW_INT:	// 24
				case MAVLINK_MSG_ID_GPS2_RAW:			// 124
				case MAVLINK_MSG_ID_SCALED_PRESSURE2:		// 137
				case MAVLINK_MSG_ID_TERRAIN_REPORT:		// 136
				case MAVLINK_MSG_ID_VIBRATION:		// 241
				case MAVLINK_MSG_ID_BATTERY_STATUS:	// 147
				case MAVLINK_MSG_ID_POWER_STATUS:		// 125
				case MAVLINK_MSG_ID_STATUSTEXT: // 253
				case MAVLINK_MSG_ID_SCALED_IMU3:		// 129
				case MAVLINK_MSG_ID_AUTOPILOT_VERSION:		// 148
				case MAVLINK_MSG_ID_FILE_TRANSFER_PROTOCOL:		// 110
				case MAVLINK_MSG_ID_TIMESYNC:		// 111
				case MAVLINK_MSG_ID_SYS_STATUS:		// 1
					{
					break;
					}
        case MAVLINK_MSG_ID_HEARTBEAT:  // #0: Heartbeat
          {
					// E.g. read GCS heartbeat and go into
					// comm lost mode if timer times out
					mavlink_heartbeat_t heartbeat;
					mavlink_msg_heartbeat_decode (&msg, &heartbeat);
					
					msg.msgid += 0;
					break;
          }
				case MAVLINK_MSG_ID_COMMAND_LONG:
					{
					mavlink_command_long_t cmdlongframe;
					mavlink_msg_command_long_decode (&msg, &cmdlongframe);
					cmdlong = cmdlongframe.command;
					break;
					}
				}
				
		sniff_modem_qg.snf_update (msg.msgid, cmdlong);
}






/*
void TMAVCORE::send_rc_data (uint8_t ix, float v1, float v2)
{
	if (ix && ix < 18)
		{
		if (v1 > 1) v1 = 1;
		if (v1 < -1) v1 = -1;
		if (v2 > 1) v2 = 1;
		if (v2 < -1) v2 = -1;
		uint16_t data = 1500 + v1*500;
		joyst_data_x_us = data;
		data = 1500 + v2*500;
		joyst_data_y_us = data;
		joyst_chan_ix = ix -1;
		f_new_joystick_data = true;
		}
}
*/


void TMAVCORE::send_joystick_data (float v_pitch, float v_roll, uint16_t butns)
{
		if (v_pitch > 1) v_pitch = 1;
		if (v_pitch < -1) v_pitch = -1;
		if (v_roll > 1) v_roll = 1;
		if (v_roll < -1) v_roll = -1;
		joyst_data_pitch = v_pitch*1000;
		joyst_data_roll = v_roll*1000;
		joyst_buttons = butns;
		f_new_joystick_data = true;
}




void TMAVCORE::joystickdata_task ()
{
	if (!joystick_last_start.get())
		{
		if (!joystick_period.get()) {
			if (f_new_joystick_data && mavatom_QG_to_RF->free_space ()) {
					send_mav_manual_controll ();
					//send_rc_override ();
					f_new_joystick_data = false;
				}
			joystick_period.set (200);
			}
		}
}



void TMAVCORE::send_mav_manual_controll ()
{
	S_MAVDATAFRAME_T mav_tmp;
	mavlink_manual_control_t joymanual;
	
	joymanual.target = 1;//last_system_id;
	joymanual.r = INT16_MAX;//INT16_MAX;
	joymanual.z = INT16_MAX;//INT16_MAX;
	joymanual.x = joyst_data_roll;//1000 + joyst_data_roll;
	joymanual.y = joyst_data_pitch;//1000 + joyst_data_pitch;
	joymanual.buttons = joyst_buttons;
	
	//uint16_t lensz = mavlink_msg_manual_control_encode_chan (1, 1, C_MAVLNK_CH_MODEM_TO_QG, &mav_tmp.frame, &joymanual);
	//insert_to_QG (mav_tmp);
	// 0xBE
	uint16_t lensz = mavlink_msg_manual_control_encode_chan (0xFF, 0, C_MAVLNK_CH_QG_TO_MODEM, &mav_tmp.frame, &joymanual);		// 0xFF, 1
	insert_to_RF (mav_tmp);

}




/*
void TMAVCORE::send_rc_override ()
{
	S_MAVDATAFRAME_T mav_tmp;
	mavlink_rc_channels_override_t rc_ovrrd;
	rc_ovrrd.target_component = 0;//last_component_id;
	rc_ovrrd.target_system = last_system_id;
	uint8_t ix = 0;
	if (f_new_joystick_data)
		{
		rc_ovrrd.chan1_raw = joyst_data_x_us;
		rc_ovrrd.chan2_raw = joyst_data_y_us;
		rc_ovrrd.chan3_raw = joyst_data_x_us;
		rc_ovrrd.chan4_raw = joyst_data_x_us;
		rc_ovrrd.chan10_raw = joyst_data_x_us;
		rc_ovrrd.chan11_raw = joyst_data_x_us;
		}
	else
		{
		rc_ovrrd.chan1_raw = 0;
		rc_ovrrd.chan2_raw = 0;
		rc_ovrrd.chan3_raw = 0;
		rc_ovrrd.chan4_raw = 0;
		rc_ovrrd.chan10_raw = 0;
		rc_ovrrd.chan11_raw = 0;
		}
	//rc_ovrrd.chan3_raw = 0;
	//rc_ovrrd.chan4_raw = 0;
	rc_ovrrd.chan5_raw = 0;
	rc_ovrrd.chan6_raw = 0;
	rc_ovrrd.chan7_raw = 0;
	rc_ovrrd.chan8_raw = 0;
	rc_ovrrd.chan9_raw = 0;
	//rc_ovrrd.chan10_raw = 0;
	//rc_ovrrd.chan11_raw = 0;
	rc_ovrrd.chan12_raw = 0;
	rc_ovrrd.chan13_raw = 0;
	rc_ovrrd.chan14_raw = 0;
	rc_ovrrd.chan15_raw = 0;
	rc_ovrrd.chan16_raw = 0;
	rc_ovrrd.chan17_raw = 0;
	rc_ovrrd.chan18_raw = 0;
	
	
	uint16_t lensz = mavlink_msg_rc_channels_override_encode_chan  (last_system_id, last_component_id, C_MAVLNK_CH_MODEM_TO_QG, &mav_tmp.frame, &rc_ovrrd);
	//lensz = mavlink_msg_to_send_buffer((uint8_t*)rawbuf, &mav_tmp.frame);
	insert_to_QG (mav_tmp);
}
*/



bool TMAVCORE::send_command_long (uint16_t cmd, float *arr, uint8_t p_cnt)
{
	bool rv = false;
	S_MAVDATAFRAME_T mav_tmp;
	mavlink_command_long_t framecmdlong;
	uint8_t ix = 0;
	framecmdlong.command = cmd;
	framecmdlong.confirmation = 0;
	framecmdlong.target_component = 0;
	framecmdlong.target_system = 1;
	float *dst = &framecmdlong.param1;
	while (p_cnt)
		{
		*dst++ = *arr++;
		p_cnt--;
		}
	mavlink_msg_command_long_encode_chan (1, 0, C_MAVLNK_CH_QG_TO_MODEM, &mav_tmp.frame, &framecmdlong);
	insert_to_RF (mav_tmp);
	return rv;
}




void TMAVCORE::StatusText ()
{
	S_MAVDATAFRAME_T mav_tmp;
	//mavlink_message_t tx_message; 
	mavlink_statustext_t txtmav;
	static uint32_t j_count = 0;
	TSTMSTRING str (txtmav.text, sizeof(txtmav.text)-5);
	str = "JOYSTICK TEST: ";
	str.Add_ULong(j_count);
	// MAV_SEVERITY_INFO, MAV_SEVERITY_DEBUG, MAV_SEVERITY_NOTICE, MAV_SEVERITY_WARNING
	// MAV_SEVERITY_ERROR, MAV_SEVERITY_CRITICAL, MAV_SEVERITY_ALERT - ????????? ?????? ?????? ?? ?????? ????
	txtmav.severity = MAV_SEVERITY_WARNING;//MAV_SEVERITY_DEBUG;//MAV_SEVERITY_INFO;//MAV_SEVERITY_EMERGENCY;

	j_count++;

	uint16_t lensz = mavlink_msg_statustext_encode_chan (last_system_id, last_component_id, C_MAVLNK_CH_MODEM_TO_QG, &mav_tmp.frame, &txtmav);
	//uint16_t lnsz = mavlink_msg_to_send_buffer((uint8_t*)rawbuf, &mav_tmp.frame);
	insert_to_QG (mav_tmp);	
}

