#include "TDEVCODE.h"




TDEVCORE::TDEVCORE (TJOYSTIC *j, TAIN *a, TMAVCORE *mav, TM24CIF *mem, TLEDS *l, const uint8_t *srkey, TMAVPARAMS *p, TDRV2605 *sdrv)
{
vibro = sdrv;
panel = new  TLEDPANEL(l, srkey);
chut_sw = ECHUTESW_NONE;
params = p;
f_maxtrotle_key = false;
joystick_obj = j;
ain_obj = a;
mav_obj = mav;
mem_obj = mem;
start_timer.set (1000);		// 1 sek
dev_mode = EDEVMODE_OFF;
restart_system ();
//ledsw = ELEDSW_NONE;
f_cur_bano_tx_state = false;
mav_obj->set_mavsysid_self (params->get_value_d (EINTMVPR_MAVSYSID_SELF).u.u32);
mav_obj->set_mavsysid_dest (params->get_value_d (EINTMVPR_MAVSYSID_DST).u.u32);
//mav_obj->set_pilotboard_type (params->get_value_d (EINTMVPR_PILOT_BOARD_TYPE).u.u32);
}



void TDEVCORE::restart_system ()
{
	mav_obj->restart ();

	procent_led = 0;
}





void TDEVCORE::key_control_task ()
{
	if (start_timer.get ())
		{
		// код по включению дополнительных функций
		// за секунду от старта питания
		
		}
}



void TDEVCORE::calibrate_sub_task ()
{
}



void TDEVCORE::work_sub_task ()
{
	float tmp_x, tmp_y;
	if (joystick_obj->get_axis (tmp_x, tmp_y)) mav_obj->send_joystick_data (tmp_x, tmp_y, 0);
}



bool TDEVCORE::onoff_bano_task (EJSTMSG msg, EJSTCPINS ix)
{
	bool rv = false;
	//static ELEDSW saved_ledsw;
	if (ix == C_ONOFF_BUTTON)
		{
		switch (msg)
			{
			case EJSTMSG_DBLCLICK:
				{
				procent_led = 0;
				panel->mode_onoff_procent (procent_led);
				f_onoff_double_click = true;
				break;
				}
			case EJSTMSG_CLICK:
				{
				if (dev_mode == EDEVMODE_WORK) {
					uint32_t pushtime = joystick_obj->get_pushtime_last (C_ONOFF_BUTTON);
					if (pushtime > 400) {
						f_cur_bano_tx_state = !f_cur_bano_tx_state;
						mav_obj->send_BANO ((f_cur_bano_tx_state)?params->get_value_d (EINTMVPR_BANO_PG).u.u32:0);
						panel->mode_work_bano (f_cur_bano_tx_state);
						
						}
					}
				break;
				}
			default: break;
			}
		}
	if (f_onoff_double_click)
		{
		uint32_t pushtime = joystick_obj->get_pushtime_cur (C_ONOFF_BUTTON);
		joystick_obj->block_next_msg (C_ONOFF_BUTTON);
		if (!pushtime)
			{
			// key pop after double-click 
			//ledsw = saved_ledsw;
			if (dev_mode == EDEVMODE_OFF)
				{
				panel->panel_mode (EPANELMOD_NONE);	
				}
			else
				{
				panel->panel_mode (EPANELMOD_WORK);
				}
			f_onoff_double_click = false;
			}
		else
			{
			// контроль и визуализация вкл/отключения
			uint32_t led_progress;
			if (dev_mode == EDEVMODE_OFF)
				{
				// процесс включения
				led_progress	= pushtime / C_ONOFF_PUSHTIME_SUB;
				led_progress *= 10;
				if (led_progress >= 100)
					{
					f_onoff_double_click = false;
					rv = true;
					}
				else
					{
					procent_led = led_progress;
					panel->panel_mode (EPANELMOD_ON);
					panel->mode_onoff_procent (procent_led);
					vibro_push (); 
					//ledsw = ELEDSW_DEV_START;
					}
				}
			else
				{
				// процесс выключения
				led_progress	= (C_ONOFF_PUSHTIME - pushtime) / C_ONOFF_PUSHTIME_SUB;
				led_progress *= 10;
				if (led_progress <= 0)
					{
					f_onoff_double_click = false;
					rv = true;
					}
				else
					{
					procent_led = led_progress;
					panel->panel_mode (EPANELMOD_OFF);
					panel->mode_onoff_procent (procent_led);
					vibro_push ();
					//ledsw = ELEDSW_DEV_STOP;
					}
				}
			}
		}
	return rv;
}



void TDEVCORE::throtle_cruise_keycontrol_task ()
{
	uint32_t push_time = joystick_obj->get_pushtime_cur (EJSTCPINS_B1);
	if (last_flight_mode == EFLIGHTMODE_STABILIZE)
		{
		if (!thrmaxmin_timer.get())
			{
			if (push_time)
				{
				if (push_time > 100)
					{
					mav_obj->send_throtle_max_proc ();			
					f_maxtrotle_key = true;		// флаг что нажимали 
					f_mintrotle_key = false;
					thrmaxmin_timer.set (100);
					vibro_push ();
					}
				}
			else
				{
				// отпустили клавишу throtle/cruise
				if (f_maxtrotle_key)
					{
					mav_obj->send_throtle_cruise_proc ();			// тут прописать скорость cruise
					f_maxtrotle_key = false;
					thrmaxmin_timer.set (100);
					}
				}
			}
		}
	else
		{
		f_maxtrotle_key = false;
		f_mintrotle_key = false;
		}
}



void TDEVCORE::stop_keycontrol_task ()
{
	uint32_t push_time = joystick_obj->get_pushtime_cur (EJSTCPINS_B2);
	if (last_flight_mode == EFLIGHTMODE_STABILIZE) 
		{
			if (!thrmaxmin_timer.get())
				{
				if (push_time)
					{
					if (push_time > 100)
						{
						mav_obj->send_throtle_min_proc ();	
						f_maxtrotle_key = false;
						f_mintrotle_key = true;
						thrmaxmin_timer.set(100);
						vibro_push ();
						}
					}
				else
					{
					if (f_mintrotle_key)
						{
						mav_obj->send_throtle_cruise_proc ();	
						f_mintrotle_key = false;
						thrmaxmin_timer.set (100);
						}
					}
				}
		}
	else
		{
		f_maxtrotle_key = false;
		f_mintrotle_key = false;
		}
}




void TDEVCORE::parachute_task ()
{
	if (last_flight_mode == EFLIGHTMODE_STABILIZE || last_flight_mode == EFLIGHTMODE_RTL)
		{
		uint32_t push_time_min = joystick_obj->get_pushtime_cur (EJSTCPINS_B2);		// min
		uint32_t push_time_chute = joystick_obj->get_pushtime_cur (EJSTCPINS_B7);		// parachute
		if (push_time_chute > 1000 && push_time_min > 100)
			{
			if (chut_sw != ECHUTESW_ACTIVATE_ON)
				{
				chut_sw = ECHUTESW_ACTIVATE_ON;
				chute_timer.set (0);	
				vibro_chute ();
				}
			}
		else
			{
			if (chut_sw == ECHUTESW_ACTIVATE_ON)
				{
				//chute_timer.set (params->get_value_d (EINTMVPR_CHUTE_TIME_ON).u.u32);
				chut_sw = ECHUTESW_TIME_OFF;
				}
			}
		}
	else
		{
		chut_sw = ECHUTESW_NONE;
		}
		
		
	switch (chut_sw)
		{
		case ECHUTESW_ACTIVATE_ON:		// открываем люк
			{
			if (!chute_timer.get()) {
				// передача с повторами положения открытия люка
				mav_obj->send_parachute_action (EPARACHUTEACT_MANUAL_OPEN);
				chute_timer.set (100);
				}
			break;
			}
		case ECHUTESW_TIME_OFF:			// ждем некоторое время с открытым люком
			{
			if (!chute_timer.get()) {
				chut_sw = ECHUTESW_ACTIVATE_OFF;
				}
			break;
			}
		case ECHUTESW_ACTIVATE_OFF:		// закрываем люк
			{
			if (!chute_timer.get()) {
						// передача с повторами положения закрытия люка
						mav_obj->send_parachute_action (EPARACHUTEACT_MANUAL_CLOSE);
						chute_timer.set (100);
						chut_sw = ECHUTESW_NONE;
					}
			break;
			}
		default: break;		// ECHUTESW_NONE и д.р.
		}
		
}



void TDEVCORE::arm_keycontrol_task ()
{
	if (last_flight_mode == EFLIGHTMODE_STABILIZE)
		{
		uint32_t push_time = joystick_obj->get_pushtime_cur (EJSTCPINS_B8);
		if (push_time > 1000)		// умови утримання
			{
			if (!mav_obj->get_pilot_throttle ())
				{
				if (!f_arm_key_pushed)
					{
					bool f_cur_pilot_arm_status = mav_obj->get_arm_status ();
					//mav_obj->set_forse_arm_mode (params->get_value_d (EINTMVPR_FORSE_ARM_MODE).u.u32);
					mav_obj->send_arm_status (!f_cur_pilot_arm_status);
					f_arm_key_pushed = true;
					}
				}
			//mav_obj->send_throtle_cruise_proc (params->get_value_d (EINTMVPR_SPD_CRUISE).u.u32);
			}
		else
			{
			// ожидаем отпускание клавиши ARM
			f_arm_key_pushed = false;
			}
		}
	else
		{
		f_arm_key_pushed = false;
		}
}



EFMOD TDEVCORE::translt_mode (EFLIGHTMODE fmd)
{
	EFMOD rv = EFMOD_ENDENUM;
	switch (fmd)
		{
		case EFLIGHTMODE_AUTO:
			{
			rv = EFMOD_AUTO;
			break;
			}
		case EFLIGHTMODE_STABILIZE:
			{
			rv = EFMOD_STAB;
			break;
			}
		case EFLIGHTMODE_RTL:
			{
			rv = EFMOD_RTL;
			break;
			}
		default: break;
		}
	return rv;
}



void TDEVCORE::Task ()
{
	EJSTMSG key_mess;
	EJSTCPINS key_ix;

	key_mess = joystick_obj->get_message (key_ix);

	panel->mode_work_qg_link (mav_obj->is_usb_link ());
	panel->mode_work_rf_link (mav_obj->is_modem_mav_link ());
	
	switch (dev_mode)
		{
		case EDEVMODE_OFF:
			{
			if (onoff_bano_task (key_mess, key_ix))
				{
				//joystick_obj->block_next_msg (C_ONOFF_BUTTON);
				dev_mode = EDEVMODE_START_ON;			
				}
			break;
			}
		case EDEVMODE_START_ON:
			{
			panel->panel_mode (EPANELMOD_WORK);
			dev_mode = EDEVMODE_WORK;
			mav_obj->enabled (true);
			panel->clear ();
			break;
			}
		case EDEVMODE_JOYCALIBR:
			{
			calibrate_sub_task ();
			break;
			}
		case EDEVMODE_WORK:
			{
			if (onoff_bano_task (key_mess, key_ix))
				{
				dev_mode = EDEVMODE_OFF;
				panel->panel_mode (EPANELMOD_NONE);
				mav_obj->enabled (false);
				mav_obj->restart ();
				break;
				}
			if (mav_obj->is_modem_mav_link ())
				{
				last_flight_mode = mav_obj->get_mode ();
				panel->mode_work_responce_mode (translt_mode (last_flight_mode));
				panel->mode_work_arm (mav_obj->get_arm_status ());
				panel->mode_work_bano (f_cur_bano_tx_state);
				
					
				throtle_cruise_keycontrol_task ();		// cruise/throttle key task
				stop_keycontrol_task ();		// stop key task
				arm_keycontrol_task ();			// arm key task
				parachute_task ();					// parachute key task
				
				
				if (key_mess == EJSTMSG_CLICK)
					{
					switch (key_ix)
						{
						case EJSTCPINS_B1:		// throtle/cruise
							{
							break;
							}
						case EJSTCPINS_B2:		// min/stop
							{
							break;
							}
						case EJSTCPINS_B3:	// auto key
							{
							panel->mode_work_request_mode (EFMOD_AUTO);
							mav_obj->send_set_mode (EFLIGHTMODE_AUTO);
							vibro_click ();
							break;
							}
						case EJSTCPINS_B4:	// stabilize key
							{
							panel->mode_work_request_mode (EFMOD_STAB);
							mav_obj->send_set_mode (EFLIGHTMODE_STABILIZE);
							vibro_click ();
							break;
							}
						case EJSTCPINS_B5:	// rtl key
							{
							panel->mode_work_request_mode (EFMOD_RTL);
							mav_obj->send_set_mode (EFLIGHTMODE_RTL);
							vibro_click ();
							break;
							}
						default: break;
						}
					}
				}

			work_sub_task ();
			break;
			}
		case EDEVMODE_STOP_OFF:
			{
			break;
			}
		default: break;
		}
}



void TDEVCORE::vibro_click ()
{
	vibro->Effect (EERM_EFF_1);
}



void TDEVCORE::vibro_push ()
{
	vibro->Effect (EERM_EFF_2);
}



void TDEVCORE::vibro_chute ()
{
vibro->Effect (EERM_EFF_3);
}




