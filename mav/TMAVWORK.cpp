#include "TMAVWORK.h"
#include "rutine.h"
#include "TGlobalISR.h"


static uint16_t last_param_ix = 0;

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
			//TGLOBISR::disable ();
			mavfifolens->push (&mavdata_tmp.frame, mavmesg_size);
			//TGLOBISR::enable ();
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


TMAVCORE::TMAVCORE (TMAVPARAMS *mp, TSERIALUSR *usbser, TSERIALUSR *uartser, uint16_t c_rftoqg_cnt, uint16_t c_qgtorf_cnt)
{
#ifdef SERIALDEBAG
	lost_mavlink_QG_to_RF_cnt = 0;
	lost_mavlink_RF_to_QG_cnt = 0;
	lost_serial_QG_to_RF_cnt = 0;
	lost_serial_RF_to_QG_cnt = 0;
#endif
BoardType = MAV_TYPE_FIXED_WING; 
params = mp;
serial_QG_obj = usbser;
serial_RF_obj = uartser;
mavatom_QG_to_RF = new TMAVSTREAM (C_MAVLNK_CH_QG_TO_MODEM, c_qgtorf_cnt);
mavatom_RF_to_QG = new TMAVSTREAM (C_MAVLNK_CH_MODEM_TO_QG, c_rftoqg_cnt);

restart ();
	
curflight_mode_in = EFLIGHTMODE_NONE;
curflight_mode_out = EFLIGHTMODE_NONE;
cur_arm_status_out = false;
cur_arm_status_in = false;
f_setmode_new_cmd = false;
confdev_param_state = EPRMSTATE_NONE;
f_new_bano_data = false;
bano_data = 0;
//params
//C_MAVID_CONFIGDEV = 
C_MAVID_CONFIGDEV = 2;
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
	f_setmode_new_cmd = false;
	f_enabled = false;
	thrcontrl_sw = ETHRSTATE_NONE;
	//cur_throtle_proc_out = 101;			// 101 $ overflow
}



void TMAVCORE::enabled (bool val)
{
	f_enabled = val;
}







// работа по распределению трафика в обоих направлениях: usb->modem,  modem->usb
void TMAVCORE::Task ()
{
	static S_MAVDATAFRAME_T mavframe_tmp;
	static uint8_t rawbuf[1024];
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
	
	/*
	if (TUSBOBJ::config_detect ())
		{
		// сброс некоторых состояний при открытии порта со стороны QG
		//param_inc_sw = EPRMINCSW_SOME_PARAM_VALUE;
		confdev_param_state = EPRMSTATE_NONE;
		last_param_ix = 0;
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
	*/
	
	// direct: serial QG tx -> (mavfifo)mav_QG_rx -> serial modem rx
	// receive byte stream to atom_mavlink deserialize fifo
	mav_deserializer = mavatom_QG_to_RF;	// мавлинк десериализатор с fifo буфером целых сообщений
		
	
	TGLOBISR::disable ();
	while (true) // извлекаем данные из байтового буфера usb rx, контролируем заполнение mavlink fifo rx
		{
		if (!mavatom_QG_to_RF->free_space ()) break;
		if (!serial_QG_obj->pop (data)) break;
		mav_deserializer->add_byte (data);	// добавляем байтовый поток в mavlink десериализатор
		}
	TGLOBISR::enable ();
	// transmit QG data to RF
		while (mavatom_QG_to_RF->pop (mavframe_tmp))
			{
			if (sniff_filter_changer_QG_to_RF (mavframe_tmp.frame))		// запускаем функционал контроля трафика по направлению USB->MODEM
				{
				free_space = serial_RF_obj->tx_free_space ();
				if (free_space)			// MAVLINK_MAX_PACKET_LEN
					{
					uint16_t len_tx = mavlink_msg_to_send_buffer (rawbuf, &mavframe_tmp.frame);
					if (free_space >= len_tx)
						{
						TGLOBISR::disable ();
						serial_RF_obj->Tx (rawbuf, len_tx);		// переносим цельное сообщение в буфер передачи usb
						TGLOBISR::enable ();
						}
					else
						{
						// игнорируем передачу этого сообщения и обновляем счетчик потери пакета
						#ifdef SERIALDEBAG
							lost_mavlink_QG_to_RF_cnt++;
						#endif
						}
					}
				}
			}		
		
			//SysID_QG_sniff = 0xFF;
			//ComponentID_QG_sniff = 0;
		
	// direct: serial modem rx -> (mavfifo)mav_modem_rx -> serial usb tx
	mav_deserializer = mavatom_RF_to_QG;	// мавлинк десериализатор с fifo буфером целых сообщений
	TGLOBISR::disable ();
	while (true) // извлекаем данные из байтового буфера usb rx, контролируем заполнение mavlink fifo rx
		{
		if (!mavatom_RF_to_QG->free_space ()) break;
		if (!serial_RF_obj->pop (data)) break;
		mav_deserializer->add_byte (data);	// добавляем байтовый поток в mavlink десериализатор
		}
	TGLOBISR::enable ();
	// проверяем обьем байтного fifo буфера, свободный размер должен вмещать максимально возможный мавлинк пакет

		while (mavatom_RF_to_QG->pop (mavframe_tmp))
			{
			if (sniff_filter_changer_RF_to_QG (mavframe_tmp.frame))		// запускаем функционал контроля трафика по направлению MODEM->USB
				{
				// проверяем свободный обьем, если обьема не хватает для принятия полного сообщения - игнорируем передачу этого сообщения и обновляем счетчик потери пакета
				free_space = serial_QG_obj->tx_free_space ();
				if (free_space)		//  >= MAVLINK_MAX_PACKET_LEN
					{
					uint16_t len_tx = mavlink_msg_to_send_buffer (rawbuf, &mavframe_tmp.frame);
					if (len_tx && free_space >= len_tx)
						{
						// переносим цельное сообщение в буфер передачи usb
						TGLOBISR::disable ();
						serial_QG_obj->Tx (rawbuf, len_tx);
						TGLOBISR::enable ();
						}
					else
						{
						#ifdef SERIALDEBAG
							lost_mavlink_RF_to_QG_cnt++;
						#endif
						}
					}
				}	
		// перехват mavlink id борта
		SysID_RF_sniff = mavframe_tmp.frame.sysid;
		ComponentID_RF_sniff = mavframe_tmp.frame.compid;			
		}
		
	heartbeat_to_QG_task ();
	send_system_time_task ();
	send_radio_param_list_to_qg_task ();
	//send_myparams_inc_task ();
	joystickdata_task ();					// отправляет два rc канала (x, y) и ненажатые клавиши, все 16 клавиш в нуле. + throtle value in procents
	setmodechange_task ();
	send_arm_status_task ();
	banomodechange_task ();
	parachute_tx_task ();
	send_throttle_task ();
	//send_mav_throtle_task ();
	
}



bool TMAVCORE::insert_to_RF ( S_MAVDATAFRAME_T &fr)
{
	bool rv = false;
	static uint8_t rawbuf[512];
	uint32_t free_space = serial_RF_obj->tx_free_space ();
	if (free_space)			// >= MAVLINK_MAX_PACKET_LEN
		{
		uint16_t len_tx = mavlink_msg_to_send_buffer (rawbuf, &fr.frame);
		if (free_space >= len_tx)
			{
			TGLOBISR::disable ();
			serial_RF_obj->Tx (rawbuf, len_tx);		// переносим цельное сообщение в буфер передачи RF модема
			rv = true;
			TGLOBISR::enable ();
			}
		else
			{
			// игнорируем передачу этого сообщения и обновляем счетчик потери пакета
			#ifdef SERIALDEBAG
				lost_mavlink_QG_to_RF_cnt++;
			#endif
			}
		}
	return rv;
}



bool TMAVCORE::insert_to_QG ( S_MAVDATAFRAME_T &fr)
{
	bool rv = false;
	static uint8_t rawbuf[512];
	uint32_t free_space = serial_QG_obj->tx_free_space ();
	
	if (free_space)		//  >= MAVLINK_MAX_PACKET_LEN
		{
		uint16_t len_tx = mavlink_msg_to_send_buffer (rawbuf, &fr.frame);
		if (free_space >= len_tx)
			{
			TGLOBISR::disable ();
			// переносим цельное сообщение в буфер передачи usb
			serial_QG_obj->Tx (rawbuf, len_tx);
			rv = true;
			TGLOBISR::enable ();
			}
		else
			{
			#ifdef SERIALDEBAG
				lost_mavlink_RF_to_QG_cnt++;
			#endif
			}
		}
	return rv;
}



bool TMAVCORE::is_free_space_chan_QG_tx ()
{
	bool rv = false;
	uint32_t free_space = serial_QG_obj->tx_free_space ();
	if (free_space >= MAVLINK_MAX_PACKET_LEN) rv = true;
	return rv;
}



bool TMAVCORE::is_free_space_chan_RF_tx ()
{
	bool rv = false;
	uint32_t free_space = serial_RF_obj->tx_free_space ();
	if (free_space >= MAVLINK_MAX_PACKET_LEN) rv = true;
	return rv;
}



void TMAVCORE::send_radio_param_list_to_qg_task ()
{
	switch (confdev_param_state)
		{
		case EPRMSTATE_LIST_RESP:
			{
			if (confdev_prmix_send < params->param_internal_cnt ())
				{
				if (is_free_space_chan_QG_tx ())
					{
					// передача списка параметров
					S_MVPARAM_HDR_T *prm_tag = params->get_param_tag (confdev_prmix_send);
					if (prm_tag)
						{
						send_mav_frame_myparam_value_to_qg (prm_tag, confdev_prmix_send);		// отправка в направлении RF->QG
						confdev_prmix_send++;
						}
					else
						{
						confdev_param_state = EPRMSTATE_NONE;
						}
					}
				}
			else
				{
				confdev_param_state = EPRMSTATE_NONE;
				}
			break;
			}
		default: break;
		}
}



/*
void TMAVCORE::send_myparams_inc_task ()
{
	if (param_inc_sw == EPRMINCSW_SEND_MY_PARAMS)
		{
		if (is_free_space_chan_RF_tx ())
			{
			// формирование списка параметров
			S_MVPARAM_HDR_T *prm_tag = params->get_param_tag (my_paramix_send);
			if (prm_tag)
				{
				send_mav_frame_myparam_value_to_qg (prm_tag, my_paramix_send);		// отправка в направлении RF->QG
				my_paramix_send++;
				}
			else
				{
				param_inc_sw = EPRMINCSW_COMPLETE;
				}
			}
		}
}
*/



void TMAVCORE::banomodechange_task ()
{
	if (f_new_bano_data)
		{
		if (is_free_space_chan_RF_tx ())
			{
			send_mav_frame_bano (bano_data, EDSTSYS_RF);
			f_new_bano_data = false;
			}
		if (is_free_space_chan_QG_tx ())
			{
			send_mav_frame_bano (bano_data, EDSTSYS_QG);
			f_new_bano_data = false;
			}
		}
}



void TMAVCORE::heartbeat_to_QG_task ()
{
	if (!heartbeat_conf_dev_timeout.get())
		{
		if (is_free_space_chan_QG_tx ())
			{
			send_heartbeat_confdev_to_qg ();
			heartbeat_conf_dev_timeout.set (1000);
			}
		}
}








void TMAVCORE::send_BANO (uint8_t prog)
{
	bano_data = prog;
	f_new_bano_data = true;
}



// QG <-SRC-> RF
void TMAVCORE::setmodechange_task ()
{
	if (f_setmode_new_cmd)
		{
		if (is_free_space_chan_RF_tx ())
			{
			sendcmd_set_mode (curflight_mode_out, cur_arm_status_out, EDSTSYS_RF);
			f_setmode_new_cmd = false;
			}
		if (is_free_space_chan_QG_tx ())
			{
			sendcmd_set_mode (curflight_mode_out, cur_arm_status_out, EDSTSYS_QG);
			f_setmode_new_cmd = false;
			}
		}
}



/*
bool TMAVCORE::is_modem_link ()
{
	return serial_RF_obj->is_link ();
}
*/



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



bool TMAVCORE::filter_command_long_QG_to_RF (mavlink_command_long_t &msg)
{
	//static uint32_t cntcmdlng = 0;
	bool rv = false;
	static uint32_t value_s = 0;

	switch (msg.command)
		{
		case MAV_CMD_PREFLIGHT_STORAGE:
			{
			if (msg.param1 == 1)	// PARAM_WRITE_PERSISTENT	Write all parameter values to persistent storage (flash/EEPROM)
				{
				// наземкой она вообще практически никогда не посылается, посылается только со значением 2 для стирания всей памяти
				// write EEPROM function
				params->save_to_flash ();
				}
			break;
			}
		case 0x200:
			{
			value_s = msg.param1;
			switch (value_s)
				{
				case MAVLINK_MSG_ID_AUTOPILOT_VERSION :
					{
					send_mav_autopilot_version ();
					break;
					}
				case MAVLINK_MSG_ID_COMPONENT_INFORMATION:
					{
					send_mav_component_information ();
					break;
					}
				default: break;
				}
			break;
			}
		}
	
	return rv;
}



void TMAVCORE::send_mav_component_information ()
{
	S_MAVDATAFRAME_T mav_tmp;
	mavlink_component_information_t frm;
	fillmem (&frm, 0, sizeof(frm));
	
	mavlink_msg_component_information_encode (C_MAVID_CONFIGDEV, 1, &mav_tmp.frame, &frm);
	insert_to_QG (mav_tmp);
}



void TMAVCORE::send_mav_autopilot_version ()
{
	S_MAVDATAFRAME_T mav_tmp;
	mavlink_autopilot_version_t frm;
	fillmem (&frm, 0, sizeof(frm));
	
	mavlink_msg_autopilot_version_encode (C_MAVID_CONFIGDEV, 1, &mav_tmp.frame, &frm);
	insert_to_QG (mav_tmp);
}



bool TMAVCORE::sniff_filter_changer_QG_to_RF (mavlink_message_t &msg)
{
	bool rv = true;
	last_seq_QGtoRF = msg.seq;
	uint32_t cmdlong = 0;
	//static uint32_t test_data;
	static S_MAVDATAFRAME_T mav_tmp;

      switch(msg.msgid) 
				{
				case MAVLINK_MSG_ID_COMMAND_LONG:		// 76
					{
					mavlink_command_long_t cmdlongframe;
					mavlink_msg_command_long_decode (&msg, &cmdlongframe);
					if (cmdlongframe.target_system == C_MAVID_CONFIGDEV)
						{
						filter_command_long_QG_to_RF (cmdlongframe);
						rv = false;
						}
					cmdlong = cmdlongframe.command;
					break;
					}
        case MAVLINK_MSG_ID_HEARTBEAT:  // #0: Heartbeat
          {
					// E.g. read GCS heartbeat and go into
					// comm lost mode if timer times out
					mavlink_heartbeat_t heartbeat;
					mavlink_msg_heartbeat_decode (&msg, &heartbeat); 
					//test_data = heartbeat.autopilot;
					//rv = false;
					break;
          }
				case MAVLINK_MSG_ID_REQUEST_DATA_STREAM:
					{
					mavlink_request_data_stream_t req_ds;
					mavlink_msg_request_data_stream_decode (&msg, &req_ds);
					//rv = false;
					break;
					}
				case MAVLINK_MSG_ID_MANUAL_CONTROL:
					{
					//mavlink_manual_control_t frame;
					//mavlink_msg_manual_control_decode (&msg, &frame);
					rv = false;			// не транслировать комманду джойстика, так как она формируется платой
					break;
					}
				case MAVLINK_MSG_ID_STATUSTEXT:			// 253
				case MAVLINK_MSG_ID_TIMESYNC:		// 111
					{
					break;
					}
				case MAVLINK_MSG_ID_PARAM_REQUEST_LIST:	// 21
					{
						{
						mavlink_param_request_list_t req_list;
						mavlink_msg_param_request_list_decode (&msg, &req_list);
						if (req_list.target_system == C_MAVID_CONFIGDEV)
							{
							// qg отправил запрос на извлечения списка параметров
							confdev_param_state = EPRMSTATE_LIST_RESP;		// инициализируем выдачу списка
							confdev_prmix_send = 0;	
							rv = false;		// блокировка отправки этой команды 
							}
						}
					break;
					}
				case MAVLINK_MSG_ID_PARAM_REQUEST_READ:
					{
					//if (true)		// msg.sysid == 0xFF
						{
						mavlink_param_request_read_t req_oneparam;
						mavlink_msg_param_request_read_decode (&msg, &req_oneparam);
						if (req_oneparam.target_system == C_MAVID_CONFIGDEV)
							{
							int16_t myreal_ix = -1;
							if (req_oneparam.param_index != -1)
								{
								// запрос по индексу параметра
								myreal_ix = req_oneparam.param_index;
								}
							else
								{
								// запроса по имени параметра
								long dst_ix = -1;
								if (params->get_mavlink_param (req_oneparam.param_id, 0, &dst_ix)) myreal_ix = dst_ix;
								}	
							if (myreal_ix != -1) 
								{
								S_MVPARAM_HDR_T *prm_tag = params->get_param_tag (myreal_ix);
								if (prm_tag) send_mav_frame_myparam_value_to_qg (prm_tag, myreal_ix);
								}
							rv = false;
							}
						}					
					break;
					}
				case MAVLINK_MSG_ID_PARAM_SET:
					{
					//if (true)		// msg.sysid == 0xFF
						{
						mavlink_param_set_t tag;
						mavlink_msg_param_set_decode (&msg, &tag);
						if (tag.target_system == C_MAVID_CONFIGDEV)
							{
							long myreal_ix;
							if (params->update_mavlink_param (tag, &myreal_ix))
								{
								// save to flash
								params->save_to_flash ();
								mavlink_param_value_t param;
								// responce
								param.param_type = tag.param_type;
								param.param_count = params->param_full_cnt ();
								param.param_index = myreal_ix;
								CopyMemorySDC ((char*)&tag.param_value , (char*)&param.param_value, sizeof(param.param_value));		// value tag
								CopyMemorySDC (tag.param_id, param.param_id, sizeof(param.param_id));		// name tag
								
								// отправляем подтверждение записи параметра
								uint16_t lensz = mavlink_msg_param_value_encode_chan (C_MAVID_CONFIGDEV, 1, C_MAVLNK_CH_MODEM_TO_QG, &mav_tmp.frame, &param);
								insert_to_QG (mav_tmp);
								}							
							rv = false;
							}
						
						}

					break;
					}
				default:
					{
					rv = false;
					break;
					}
				}
				
		#ifdef MAVSNIFF	
		sniff_qg_modem.snf_update (msg.msgid, cmdlong);
		#endif
				
	return rv;
}




bool TMAVCORE::is_modem_mav_link ()
{
	return (tim_mav_modem_link_timeout.get())?true:false;
}



bool TMAVCORE::sniff_filter_changer_RF_to_QG (mavlink_message_t &msg)
{
	bool rv = true;
	tim_mav_modem_link_timeout.set (1500);
	
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
					{
					break;
					}
				case MAVLINK_MSG_ID_VFR_HUD:		// 74
					{
					mavlink_vfr_hud_t frame;
					mavlink_msg_vfr_hud_decode (&msg, &frame);
					if (msg.sysid == C_PILOT_MAV_ID)
						{
						cur_throtle_proc_in = frame.throttle;
						cur_in_airspeed = frame.airspeed;
						airspeed_in_timeout.set (1000);
						}
					break;
					}
				case MAVLINK_MSG_ID_PARAM_VALUE:		// 22
					{
					break;
					}
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
					{
					//mavlink_msg_servo_output_raw_decode (&msg, &servo_data_in);
					break;
					}
				case MAVLINK_MSG_ID_MISSION_CURRENT:		// 42
				case MAVLINK_MSG_ID_NAV_CONTROLLER_OUTPUT:		// 62
				case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:		// 3
				
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
					mavlink_heartbeat_t heartbeat;
					mavlink_msg_heartbeat_decode (&msg, &heartbeat);
					if (msg.sysid == C_PILOT_MAV_ID)
						{
						cur_arm_status_in = (heartbeat.base_mode & MAV_MODE_FLAG_SAFETY_ARMED)?true:false;
						if (heartbeat.base_mode & MAV_MODE_FLAG_CUSTOM_MODE_ENABLED)
							{
							curflight_mode_in = flight_mode_code_from_custom (heartbeat.custom_mode);	
							}
						else
							{
							curflight_mode_in = EFLIGHTMODE_NONE;
							}
						}
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
		#ifdef MAVSNIFF	
		sniff_modem_qg.snf_update (msg.msgid, cmdlong);
		#endif
	return rv;
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

	if (!joystick_period.get()) {
		if (f_new_joystick_data) { 
				if (is_free_space_chan_RF_tx ()) 
					{
					send_mav_manual_controll (EDSTSYS_RF);
					f_new_joystick_data = false;
					}
				if (is_free_space_chan_QG_tx ()) 
					{
					send_mav_manual_controll (EDSTSYS_QG);
					f_new_joystick_data = false;
					}
		joystick_period.set (100);
		}
	}

}



/*
void TMAVCORE::send_mav_throtle_task ()
{
	if (curflight_mode_in == EFLIGHTMODE_STABILIZE)		// curflight_mode_in == EFLIGHTMODE_STABILIZE
		{
		if (f_do_change_speed)
			{
			if (is_free_space_chan_RF_tx ())
				{
				send_mav_frame_dochange_speed ();
				f_do_change_speed = false;
				}
			}
		}
}
*/


// часть механизма подкидывания параметров для возможности редактирования параметров джойстика со стороны наземки
void TMAVCORE::send_mav_frame_myparam_value_to_qg (S_MVPARAM_HDR_T *tag, int16_t ix)
{
	mavlink_param_value_t param;
	S_MAVDATAFRAME_T mav_tmp;

	bool rslt_ok = false;
	switch (tag->type)
		{
		case MAV_PARAM_TYPE_REAL32:
			{
			S_MVPARAM_FLOAT_T *data_f = (S_MVPARAM_FLOAT_T*)tag;
			param.param_value = data_f->value;
			rslt_ok = true;
			break;
			}
		case MAV_PARAM_TYPE_INT32:
		case MAV_PARAM_TYPE_UINT32:
			{
			S_MVPARAM_U32_T *data_u32 = (S_MVPARAM_U32_T*)tag;
			param.param_value = data_u32->value;
			rslt_ok = true;
			break;
			}
		default: break;
		}
	if (rslt_ok)
		{
		param.param_type = tag->type;
		param.param_count = params->param_full_cnt () - 1;
		param.param_index = ix;
		TSTMSTRING str(const_cast<char*>(tag->param_id), param.param_id, sizeof(param.param_id));		// просто копирует строку
		uint16_t lensz = mavlink_msg_param_value_encode_chan (C_MAVID_CONFIGDEV, 1, C_MAVLNK_CH_MODEM_TO_QG, &mav_tmp.frame, &param);
		insert_to_QG (mav_tmp);
		}
}




void TMAVCORE::send_mav_manual_controll (EDSTSYS d)
{
	
	S_MAVDATAFRAME_T mav_tmp;
	mavlink_manual_control_t joymanual;
	
	joymanual.target = 1;
	joymanual.r = 0;//INT16_MAX;
	joymanual.z = throttle_gen_value ();//INT16_MAX;
	joymanual.x = joyst_data_roll;
	joymanual.y = joyst_data_pitch;
	joymanual.buttons = 0;//joyst_buttons;

	//if (bano_data) return;		// fot testing

	switch (d)
		{
		case EDSTSYS_QG:
			{
			mavlink_msg_manual_control_encode_chan (C_MAVID_CONFIGDEV, 1, C_MAVLNK_CH_MODEM_TO_QG, &mav_tmp.frame, &joymanual);  // C_MAVLNK_CH_MODEM_TO_QG
			insert_to_QG (mav_tmp);
			break;
			}
		case EDSTSYS_RF:
			{
			mavlink_msg_manual_control_encode_chan (0xFF, 1, C_MAVLNK_CH_QG_TO_MODEM, &mav_tmp.frame, &joymanual);  // C_MAVLNK_CH_MODEM_TO_QG
			insert_to_RF (mav_tmp);
			break;
			}
		default: break;
		}
}




void TMAVCORE::send_heartbeat_confdev_to_qg ()
{
	S_MAVDATAFRAME_T mav_tmp;
	mavlink_heartbeat_t heartbeat;
		
		heartbeat.type = 1;// MAV_TYPE_ONBOARD_CONTROLLER;// 1;// MAV_TYPE_CHARGING_STATION;//13;// 13;//MAV_TYPE_GCS;//MAV_TYPE_GCS;// MAV_TYPE_CHARGING_STATION;
		heartbeat.autopilot = MAV_AUTOPILOT_ARDUPILOTMEGA;//MAV_AUTOPILOT_INVALID;//MAV_AUTOPILOT_ARDUPILOTMEGA;//3;// MAV_AUTOPILOT_INVALID;//MAV_AUTOPILOT_ARDUPILOTMEGA;// MAV_AUTOPILOT_INVALID;//MAV_AUTOPILOT_ARDUPILOTMEGA;// MAV_AUTOPILOT_INVALID;//MAV_AUTOPILOT_ARDUPILOTMEGA;// MAV_TYPE_CHARGING_STATION;
		heartbeat.base_mode = 17;  // 0x51
		heartbeat.custom_mode = 2;
		heartbeat.system_status = MAV_STATE_ACTIVE;//3;//MAV_STATE_STANDBY;// MAV_STATE_ACTIVE;//MAV_STATE_ACTIVE;//0;//MAV_STATE_ACTIVE;
		heartbeat.mavlink_version = 3;
		
		/*
		heartbeat.type = MAV_TYPE_ANTENNA_TRACKER;// 1  13;// 13;//MAV_TYPE_GCS;//MAV_TYPE_GCS;// MAV_TYPE_CHARGING_STATION;  MAV_TYPE_ANTENNA_TRACKER
		heartbeat.autopilot = 3;//3;// MAV_AUTOPILOT_INVALID;//MAV_AUTOPILOT_ARDUPILOTMEGA;// MAV_AUTOPILOT_INVALID;//MAV_AUTOPILOT_ARDUPILOTMEGA;// MAV_AUTOPILOT_INVALID;//MAV_AUTOPILOT_ARDUPILOTMEGA;// MAV_TYPE_CHARGING_STATION;
		heartbeat.base_mode = 17;  // 0x51
		heartbeat.custom_mode = 2;
		heartbeat.system_status = MAV_STATE_ACTIVE;//3;//MAV_STATE_STANDBY;// MAV_STATE_ACTIVE;//MAV_STATE_ACTIVE;//0;//MAV_STATE_ACTIVE;
		heartbeat.mavlink_version = 3;
		*/
		
	
	mavlink_msg_heartbeat_encode (C_MAVID_CONFIGDEV, 1, &mav_tmp.frame, &heartbeat);
	insert_to_QG (mav_tmp);	
}



void TMAVCORE::set_mavsysid_self (uint32_t d)
{
	
	SysID_QG_hrtbt = d;
	ComponentID_QG_hrtbt = 0;
}



void TMAVCORE::set_mavsysid_dest (uint32_t d)
{
	SysID_RF_hrtbt = d;
	ComponentID_RF_hrtbt = 0;
}


/*
void TMAVCORE::set_pilotboard_type (uint32_t d)
{
	BoardType = d;
}
*/



void TMAVCORE::set_forse_arm_mode (bool v)
{
	if (v)
		{
		force_arm_code_act = 21196;
		}
	else
		{
		force_arm_code_act = 0;
		}
}



void TMAVCORE::send_mav_frame_command_long (uint16_t cmd, float *arr, uint8_t p_cnt, EDSTSYS d)
{
	S_MAVDATAFRAME_T mav_tmp;
	mavlink_command_long_t framecmdlong;

	framecmdlong.command = cmd;
	framecmdlong.confirmation = 0;
	framecmdlong.target_component = 0;
	framecmdlong.target_system = (d == EDSTSYS_QG)?0xFF:1;
	
	framecmdlong.param1 = *arr++;
	framecmdlong.param2 = *arr++;
	framecmdlong.param3 = *arr++;
	framecmdlong.param4 = *arr++;
	framecmdlong.param5 = *arr++;
	framecmdlong.param6 = *arr++;
	framecmdlong.param7 = *arr;

	switch (d)
		{
		case EDSTSYS_QG:
			{
			mavlink_msg_command_long_encode_chan (C_MAVID_CONFIGDEV, 1, C_MAVLNK_CH_MODEM_TO_QG, &mav_tmp.frame, &framecmdlong);  // C_MAVLNK_CH_MODEM_TO_QG
			insert_to_QG (mav_tmp);
			break;
			}
		case EDSTSYS_RF:
			{
			mavlink_msg_command_long_encode_chan (0xFF, 0, C_MAVLNK_CH_QG_TO_MODEM, &mav_tmp.frame, &framecmdlong);  // C_MAVLNK_CH_MODEM_TO_QG
			insert_to_RF (mav_tmp);
			break;
			}
		default: break;
		}
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
	// MAV_SEVERITY_ERROR, MAV_SEVERITY_CRITICAL, MAV_SEVERITY_ALERT
	txtmav.severity = MAV_SEVERITY_WARNING;//MAV_SEVERITY_DEBUG;//MAV_SEVERITY_INFO;//MAV_SEVERITY_EMERGENCY;

	j_count++;

	uint16_t lensz = mavlink_msg_statustext_encode_chan (C_MAVID_CONFIGDEV, 1, C_MAVLNK_CH_MODEM_TO_QG, &mav_tmp.frame, &txtmav);
	insert_to_QG (mav_tmp);	
}



//float tmpmavcmdarr[7]
void TMAVCORE::clear_tmpmavarr ()
{
	uint8_t ix = 0;
	while (ix < 7)
		{
		tmpmavcmdarr[ix] = 0;
		ix++;
		}
}



void TMAVCORE::sendcmd_set_mode (EFLIGHTMODE mmd, bool f_armst, EDSTSYS d)
{
	if (mmd < EFLIGHTMODE_ENDENUM)
		{
		clear_tmpmavarr ();
			
		tmpmavcmdarr[0] = MAV_MODE_FLAG_CUSTOM_MODE_ENABLED | ((f_armst)?MAV_MODE_FLAG_SAFETY_ARMED:0);
		tmpmavcmdarr[1] = custom_mode_code (mmd); 
		send_mav_frame_command_long (MAV_CMD_DO_SET_MODE, tmpmavcmdarr, 7, d);
		}
}



void TMAVCORE::send_payload_place ()
{
		clear_tmpmavarr ();
		send_mav_frame_command_long (MAV_CMD_NAV_PAYLOAD_PLACE , tmpmavcmdarr, 7, EDSTSYS_QG);
}




void TMAVCORE::send_mav_frame_arm_dissarm (EDSTSYS d)
{
		clear_tmpmavarr ();
		tmpmavcmdarr[0] = cur_arm_status_out;
		tmpmavcmdarr[1] = force_arm_code_act;
		send_mav_frame_command_long (MAV_CMD_COMPONENT_ARM_DISARM, tmpmavcmdarr, 7, d);
}



// QG <-SRC-> RF
void TMAVCORE::send_arm_status_task ()
{
	if (f_arm_new_cmd)
		{
		if (is_free_space_chan_RF_tx ())
			{
			send_mav_frame_arm_dissarm (EDSTSYS_RF);
			f_arm_new_cmd = false;
			}
		if (is_free_space_chan_QG_tx ())
			{
			send_mav_frame_arm_dissarm (EDSTSYS_QG);
			f_arm_new_cmd = false;
			}
		}
}



int16_t TMAVCORE::procents_to_throtle (uint8_t proc)
{
	int16_t rv;// = 0;//INT16_MAX;
	if (proc > 100) proc = 100;
	
		float quant, level = proc;
		if (true)		// тут будет проверка параметра в какой диапазон переводить
			{
			// for result 0 to +1000
			quant = 1000.0F / 100;
			}
		else
			{
			// for result -1000 to +1000
			level -= 50;		// при параметре менее 50%, будет минусовое значение
			quant = 1000.0F / 50;
			}
		rv = quant * level;
		
	return rv;
}



uint8_t TMAVCORE::get_pilot_throttle ()
{
	return cur_throtle_proc_in;
}



ETHRCHECK TMAVCORE::check_feedback_throtle_val (uint8_t vprc)
{
	ETHRCHECK rv = ETHRCHECK_ENDENUM;
	if (cur_throtle_proc_in >= 0 && cur_throtle_proc_in <= 100)
		{
		uint32_t jitval = 2;
		long min_val = vprc;//45;//params->get_value_d (0).u.u32;
		long max_val = min_val;
		min_val -= jitval;
		max_val += jitval;
		if (min_val < 0) min_val = 0;
		if (max_val > 100) max_val = 100;
		

		if (cur_throtle_proc_in <= min_val)
			{
			rv = ETHRCHECK_LOW;
			}
		else
			{
			if (cur_throtle_proc_in > max_val)
				{
				rv = ETHRCHECK_HI;
				}
			else
				{
				rv = ETHRCHECK_PASS;
				}
			}
		}
	return rv;
}




void TMAVCORE::send_mav_frame_throttle (ETHRSTATE s, EDSTSYS d)
{
		clear_tmpmavarr ();
		tmpmavcmdarr[0] = s;
		tmpmavcmdarr[1] = 500;
		send_mav_frame_command_long (42102, tmpmavcmdarr, 7, d);
}



// QG <-SRC-> RF
void TMAVCORE::send_throttle_task ()
{
	if (!throttle_period.get ()) {
		if (thrcontrl_sw >= ETHRSTATE_CRUIS && thrcontrl_sw <= ETHRSTATE_MIN)
			{
			if (is_free_space_chan_RF_tx ())
				{	
				send_mav_frame_throttle (thrcontrl_sw, EDSTSYS_RF);
				thrcontrl_sw = ETHRSTATE_NONE;
				}
			if (is_free_space_chan_QG_tx ())
				{
				send_mav_frame_throttle (thrcontrl_sw, EDSTSYS_QG);
				thrcontrl_sw = ETHRSTATE_NONE;
				}
			}
		throttle_period.set (100);
		}
}



/*
// MAV_CMD_DO_CHANGE_SPEED  опробовать команду
// поздадача формирования throttle значения
// отслеживает состояния отсылки и повтора комманд установки throtle в зависимости от клавишных комманд
int16_t TMAVCORE::mav_throtle_task ()
{
	int16_t rv = INT16_MAX;
	switch (thrcontrl_sw)
		{
		case ETHTCONTRLSW_MAX:
			{
			// параметр ускорения должен применяться сразу и повторяться периодически пока зажата клавиша Throtle
			rv = procents_to_throtle (cur_throtle_proc_out);
			thrcontrl_sw = ETHTCONTRLSW_CRUISE;
			break;
			}
		case ETHTCONTRLSW_CRUISE:
			{
			ETHRCHECK rslt = check_feedback_throtle_val (cur_throtle_proc_out);
			switch (rslt)
				{	
				case ETHRCHECK_PASS:
				case ETHRCHECK_LOW:
					{
					// текущая скорость <= cruise уровня
					thrcontrl_sw = ETHTCONTRLSW_NONE;
					break;
					}
				default:
					{
					// повторяем установку cruise скорости
					// в случае, когда неправильное feedback значение или величина больше отсылаемого уровня
					// rv = procents_to_throtle (cur_throtle_proc_out);
					break;
					}
				}
			rv = procents_to_throtle (cur_throtle_proc_out);
			break;
			}
		case ETHTCONTRLSW_MIN:
			{
			ETHRCHECK rslt = check_feedback_throtle_val (cur_throtle_proc_out);
			switch (rslt)
				{	
				case ETHRCHECK_PASS:
				case ETHRCHECK_LOW:
					{
					// скорость или ниже cruise уровня или равна
					thrcontrl_sw = ETHTCONTRLSW_NONE;
					break;
					}
				default:
					{
					// повторяем установку cruise скорости
					// в случае, когда неправильное feedback значение или величина больше отсылаемого уровня
					//rv = procents_to_throtle (cur_throtle_proc_out);
					break;
					}
				}
			rv = procents_to_throtle (cur_throtle_proc_out);
			break;
			}
		default: break;
		}
	return rv;
}
*/


void TMAVCORE::send_throtle_max_proc ()
{
	thrcontrl_sw = ETHRSTATE_MAX;
	trottle_timeout.set (200);
}



void TMAVCORE::send_throtle_cruise_proc ()
{
	thrcontrl_sw = ETHRSTATE_CRUIS;
	trottle_timeout.set (0);
}



void TMAVCORE::send_throtle_min_proc ()
{
	thrcontrl_sw = ETHRSTATE_MIN;
	trottle_timeout.set (200);
}




int16_t TMAVCORE::throttle_gen_value ()
{
	int16_t rv = INT16_MAX;
	
	switch (thrcontrl_sw)
		{
		case ETHRSTATE_MAX:
			{
			if (trottle_timeout.get ()) rv = procents_to_throtle (100);
			break;
			}
		case ETHRSTATE_MIN:
			{
			if (trottle_timeout.get ()) rv = procents_to_throtle (0);
			break;
			}
		default: 
			break;
		}
	return rv;
}



/*
void TMAVCORE::send_throtle_proc (uint32_t prc)
{
	//f_do_change_speed = true;
	
	//dochangespeed_param[0] = 2;		// throtle in procent
	//dochangespeed_param[1] = -1;
	//dochangespeed_param[2] = prc;
	
	
	//dochangespeed_param[0] = 2;		// throtle in procent
	//dochangespeed_param[1] = -2;
	//dochangespeed_param[2] = -2;
	
}



void TMAVCORE::send_cruise_m_s (float val)
{
	//f_do_change_speed = true;
	
	//dochangespeed_param[0] = 0;			// throtle in m/s
	//dochangespeed_param[1] = val;
	//dochangespeed_param[2] = -1;
	
	
	//dochangespeed_param[0] = 0;			// throtle in m/s
	//dochangespeed_param[1] = -2;
	//dochangespeed_param[2] = -2;
	
}



void TMAVCORE::send_mav_frame_dochange_speed ()
{
		clear_tmpmavarr ();
		tmpmavcmdarr[0] = dochangespeed_param[0]; 
		tmpmavcmdarr[1] = dochangespeed_param[1];
		tmpmavcmdarr[2] = dochangespeed_param[2];
		send_mav_frame_command_long (MAV_CMD_DO_CHANGE_SPEED, tmpmavcmdarr, 7);
}

*/


EFLIGHTMODE TMAVCORE::get_mode ()
{
	return curflight_mode_in;
}



bool TMAVCORE::get_arm_status ()
{
	return cur_arm_status_in;
}



void TMAVCORE::send_arm_status (bool v)
{
	cur_arm_status_out = v;
	f_arm_new_cmd = true;
}







void TMAVCORE::send_set_mode (EFLIGHTMODE m)
{
	curflight_mode_out = m;
	f_setmode_new_cmd = true;
}


/*
void TMAVCORE::send_mav_frame_parachute (PARACHUTE_ACTION act)
{
	clear_tmpmavarr ();
	tmpmavcmdarr[0] = act;
	send_mav_frame_command_long (MAV_CMD_DO_PARACHUTE, tmpmavcmdarr, 7);
}
*/




static const uint32_t convcustommode_plane[EFLIGHTMODE_ENDENUM] = {29/*none*/, 9/*auto*/, 2/*stabilize*/, 11/*rtl*/};
static const uint32_t convcustommode_copter[EFLIGHTMODE_ENDENUM] = {29/*none*/, 3/*auto*/, 0/*stabilize*/, 6/*rtl*/};
uint32_t TMAVCORE::custom_mode_code (EFLIGHTMODE fm)
{
	uint32_t rv = 29;
	uint32_t *arr = 0;
	
	if (fm < EFLIGHTMODE_ENDENUM)
		{
		switch (BoardType)
			{
			case MAV_TYPE_FIXED_WING:		// 1
				{
				arr = const_cast<uint32_t*>(convcustommode_plane);
				break;
				}
			case MAV_TYPE_TRICOPTER:		// 15
				{
				arr = const_cast<uint32_t*>(convcustommode_copter);
				break;
				}
			}
		if (arr) rv = arr[fm];
		}
	return rv;
}



EFLIGHTMODE TMAVCORE::flight_mode_code_from_custom (uint32_t cmod)
{
EFLIGHTMODE rv = EFLIGHTMODE_NONE;
uint32_t *arr = 0;
	switch (BoardType)
		{
		case MAV_TYPE_FIXED_WING:		// 1
			{
			arr = const_cast<uint32_t*>(convcustommode_plane);
			break;
			}
		case MAV_TYPE_TRICOPTER:		// 15
			{
			arr = const_cast<uint32_t*>(convcustommode_copter);
			break;
			}
		}
	if (arr) {
		uint32_t ix = EFLIGHTMODE_NONE;
		while (ix < EFLIGHTMODE_ENDENUM) {
			if (arr[ix] == cmod) {
				rv = (EFLIGHTMODE)ix;
				break;
				}
			ix++;
			}
		}
return rv;
}



/*
void TMAVCORE::send_mav_frame_do_set_servo (uint8_t an, uint16_t val)
{
		clear_tmpmavarr ();
		tmpmavcmdarr[0] = an;
		tmpmavcmdarr[1] = val;
		send_mav_frame_command_long (MAV_CMD_DO_SET_SERVO, tmpmavcmdarr, 7);
}
*/



void TMAVCORE::send_mav_frame_bano (uint8_t prog, EDSTSYS d)
{
		clear_tmpmavarr ();
		tmpmavcmdarr[0] = prog;
		send_mav_frame_command_long (42101, tmpmavcmdarr, 7, d);
}



void TMAVCORE::send_mav_parachute_act (EPARACHUTEACT act_n, EDSTSYS d)
{
		clear_tmpmavarr ();
		tmpmavcmdarr[0] = act_n;
		send_mav_frame_command_long (MAV_CMD_DO_PARACHUTE, tmpmavcmdarr, 7, d);
}



void TMAVCORE::send_mav_systime ()
{
	S_MAVDATAFRAME_T mav_tmp;
	mavlink_system_time_t frm;
	frm.time_boot_ms = SYSBIOS::GetTickCountLong ();
	frm.time_unix_usec = frm.time_boot_ms * 1000;
	
	uint16_t lensz = mavlink_msg_system_time_encode (C_MAVID_CONFIGDEV, 1, &mav_tmp.frame, &frm);
	insert_to_QG (mav_tmp);	
}



void TMAVCORE::send_system_time_task ()
{
	if (!confdev_systime_period.get ())
		{
		if (is_free_space_chan_QG_tx ()) send_mav_systime ();
		confdev_systime_period.set (1000);
		}
}




void TMAVCORE::parachute_tx_task ()
{
// rate передачи контролируется в TDEVCODE
if (f_new_parachute_data)
	{
	if (is_free_space_chan_RF_tx ())
		{
		send_mav_parachute_act (parachute_action, EDSTSYS_RF);
		f_new_parachute_data = false;
		}
	if (is_free_space_chan_QG_tx ())
		{
		send_mav_parachute_act (parachute_action, EDSTSYS_QG);
		f_new_parachute_data = false;
		}
	}
}



/*
// ix start = 1
bool TMAVCORE::get_servo_status (uint8_t ix, uint32_t &srvval)
{
	bool rv = false;
	uint16_t *linadr = 0;
	if (ix) {
		ix--;
		if (ix <= 7)		// servo_data_in
			{
			linadr = &servo_data_in.servo1_raw;
			}
		else
			{
			if (ix <= 15)
				{
				ix -= 8;
				linadr = &servo_data_in.servo9_raw;
				}
			}
		if (linadr)
			{
			uint32_t val = linadr[ix];
			srvval = val;
			rv = true;
			}
		}
	return rv;
}
*/



void TMAVCORE::send_parachute_action (EPARACHUTEACT act_n)
{
	parachute_action = act_n;
	f_new_parachute_data = true;
}

// MAV_CMD_DO_SET_SERVO (187 )
