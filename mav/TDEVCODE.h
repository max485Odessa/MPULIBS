#ifndef _H_JOYSTICK_DEV_CODE_CORE_H_
#define _H_JOYSTICK_DEV_CODE_CORE_H_



#include "app.h"
#include "SYSBIOS.H"
#include "TDRV2605.h"
#include "TFTASKIF.h"
#include "TM24Cxxx.h"
#include "TJOYSTICK.h"
#include "TAIN.h"
//#include "TCDC.h"
#include "tusartdata.h"
#include "TMAVWORK.h"
#include "hard_rut.h"
#include "led_W2812.h"
#include "TMAVPARAMS.h"
#include "TPANELLEDS.h"
#include "TDRV2605.h"



enum ECONMOD {ECONMOD_ONLINE = 0, ECONMOD_OFFLINE, ECONMOD_ENDENUM};
enum EDEVMODE {EDEVMODE_OFF = 0, EDEVMODE_START_ON, EDEVMODE_JOYCALIBR, EDEVMODE_WORK, EDEVMODE_STOP_OFF,  EDEVMODE_ENDENUM};

// режимы индикации: включение пульта, отключение, обычный режим, отображение каких либо уровней
//enum ELEDSW {ELEDSW_NONE = 0, ELEDSW_DEV_START, ELEDSW_DEV_STOP, ELEDSW_DEV_WORK, ELEDSW_LEVEL, ELEDSW_ENDENUM};



#define C_ONOFF_BUTTON EJSTCPINS_B6
#define C_ONOFF_PUSHTIME 3000
#define C_ONOFF_PUSHTIME_SUB (C_ONOFF_PUSHTIME/10)



enum ECHUTESW {ECHUTESW_NONE = 0, ECHUTESW_ACTIVATE_ON, ECHUTESW_TIME_OFF, ECHUTESW_ACTIVATE_OFF, ECHUTESW_ENDENUM };





class TDEVCORE: public TFFC {
		TJOYSTIC *joystick_obj;
		TAIN *ain_obj;
		TMAVCORE *mav_obj;
		TM24CIF *mem_obj;
		//TUSBOBJ *usb_obj;
		TDRV2605 *vibro;
		
		TMAVPARAMS *params;
		
		TLEDPANEL *panel;
		
		void update_lightkey_flymode (EFLIGHTMODE md);
		void ledkeystate_clear ();
		void gen_progress (uint8_t proc, float lightmaxlevel, EBASECOLOR colb);
		EFLIGHTMODE last_flight_mode;
	
		virtual void Task () override;
	
		EDEVMODE dev_mode;
		ECONMOD connect_mode;
		bool f_arm_key_pushed;
		SYSBIOS::Timer start_timer;
	
		void key_control_task ();
		void calibrate_sub_task ();
		void work_sub_task ();
		//void led_sub_task ();
	
		bool f_cur_bano_tx_state;
		bool onoff_bano_task (EJSTMSG msg, EJSTCPINS ix);
	
		void restart_system ();
		
		
		void procents (uint8_t proc, EBASECOLOR clrs);
		uint8_t procent_led;
		uint32_t onoffpushtime;
		bool f_onoff_double_click;
		
		
		SYSBIOS::Timer thrmaxmin_timer;
		bool f_maxtrotle_key;
		bool f_mintrotle_key;
		void throtle_cruise_keycontrol_task ();
		void stop_keycontrol_task ();
		void arm_keycontrol_task ();
		void parachute_task ();
		
		ECHUTESW chut_sw;
		SYSBIOS::Timer chute_timer;
		
		static EFMOD translt_mode (EFLIGHTMODE);
		
		
		void vibro_click ();
		void vibro_push ();
		void vibro_chute ();
		
	public:
		
		TDEVCORE (TJOYSTIC *j, TAIN *a, TMAVCORE *mav, TM24CIF *mem, TLEDS *l, const uint8_t *srkey, TMAVPARAMS *p, TDRV2605 *sdrv);
		//static uint8_t *get_reindex_ledmatrix ();
};



#endif


