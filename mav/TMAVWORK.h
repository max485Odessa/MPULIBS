#ifndef _H_MAVPROCESSOR_H_
#define _H_MAVPROCESSOR_H_


#include "mavlink.h"
#include "TTFIFO.h"
#include "TFIFOLENS.h"
#include "TFTASKIF.h"
#include "tusartdata.h"
#include "SYSBIOS.H"
#include "STMSTRING.h"
//#include "TCDC.h"
#include "mavlink_msg_rc_channels_override.h"
#include "TMAVPARAMS.h"


#define C_MAVLNK_CH_MODEM_TO_QG MAVLINK_COMM_0
#define C_MAVLNK_CH_QG_TO_MODEM MAVLINK_COMM_1


// new id's for command long commands
#define MAV_CMD_SET_BANO 44004
#define MAV_CMD_SET_PARACHUTE 44005
#define MAV_CMD_MCH_SET_MANUAL_CONTROL 44003
//#define MAV_CMD_DTC_STATUS 43200


//enum EPRMINCSW {EPRMINCSW_SOME_PARAM_VALUE = 0, EPRMINCSW_DETECT_LAST_PARAM, EPRMINCSW_SEND_MY_PARAMS, EPRMINCSW_COMPLETE, EPRMINCSW_ENDENUM};

typedef struct {
	mavlink_message_t frame;
	//bool my_frame;
	//mavlink_status_t status;
} S_MAVDATAFRAME_T;


typedef struct {
	uint32_t msgid;
	uint32_t command_long_id;
	uint32_t command_cnt;
} S_SNIFFMAV_T;


class TMAVCHARIN {
	public:
		virtual bool add_byte (uint8_t data) = 0;
		virtual bool add_bytes (uint8_t *src, uint8_t sz) = 0;
};





#ifdef MAVSNIFF
class TMAVSNIFF {
	
		S_SNIFFMAV_T snf_arrqg[255];
		long snf_add_ix;
	
	public:
		TMAVSNIFF ();
		long snf_update (uint32_t mid, uint32_t cmdlng);
		void clear ();
};
#endif



// десериализует байтовый поток, помещает цельный пакет в фифо буфер
class TMAVSTREAM: public TMAVCHARIN {
		const uint8_t c_mavchnl;
		TFIFOLEN *mavfifolens;
	
		const uint32_t c_alloc_fifo_cnt;
		S_MAVDATAFRAME_T mavdata_tmp;
		virtual bool add_byte (uint8_t data) override;
		virtual bool add_bytes (uint8_t *src, uint8_t sz) override;
	
	public:
		TMAVSTREAM (const uint8_t c_mch, const uint32_t afcnt);	// номер mavlink канала, количество сообщений в фифо
		uint32_t free_space ();
		bool push (const S_MAVDATAFRAME_T &msg_src);
		uint16_t pop (S_MAVDATAFRAME_T &msg_dst);
		uint32_t is_data_count ();
		void clear ();
	#ifdef SERIALDEBAG
		uint32_t debug_peak();
		void debug_peak_clear ();
	#endif
};



//enum EFLIGHTMODE {EFLIGHTMODE_AUTO = 0, EFLIGHTMODE_STAB = 1, EFLIGHTMODE_RTL = 2, EFLIGHTMODE_ENDENUM = 3};
/*
enum CUSTOMFLIGHTMODE {
		STABILIZE =     0,  // manual airframe angle with manual throttle
		ACRO =          1,  // manual body-frame angular rate with manual throttle
		ALT_HOLD =      2,  // manual airframe angle with automatic throttle
		AUTO =          3,  // fully automatic waypoint control using mission commands
		GUIDED =        4,  // fully automatic fly to coordinate or fly at velocity/direction using GCS immediate commands
		LOITER =        5,  // automatic horizontal acceleration with automatic throttle
		RTL =           6,  // automatic return to launching point
		CIRCLE =        7,  // automatic circular flight with automatic throttle
		LAND =          9,  // automatic landing with horizontal position control
		DRIFT =        11,  // semi-autonomous position, yaw and throttle control
		SPORT =        13,  // manual earth-frame angular rate control with manual throttle
		FLIP =         14,  // automatically flip the vehicle on the roll axis
		AUTOTUNE =     15,  // automatically tune the vehicle's roll and pitch gains
		POSHOLD =      16,  // automatic position hold with manual override, with automatic throttle
		BRAKE =        17,  // full-brake using inertial/GPS system, no pilot input
		THROW =        18,  // throw to launch mode using inertial/GPS system, no pilot input
		AVOID_ADSB =   19,  // automatic avoidance of obstacles in the macro scale - e.g. full-sized aircraft
		GUIDED_NOGPS = 20,  // guided mode but only accepts attitude and altitude
		SMART_RTL =    21,  // SMART_RTL returns to home by retracing its steps
		FLOWHOLD  =    22,  // FLOWHOLD holds position with optical flow without rangefinder
		FOLLOW    =    23,  // follow attempts to follow another vehicle or ground station
		ZIGZAG    =    24,  // ZIGZAG mode is able to fly in a zigzag manner with predefined point A and point B
		SYSTEMID  =    25,  // System ID mode produces automated system identification signals in the controllers
		AUTOROTATE =   26,  // Autonomous autorotation
		AUTO_RTL =     27,  // Auto RTL, this is not a true mode, AUTO will report as this mode if entered to perform a DO_LAND_START Landing sequence
		TURTLE =       28,  // Flip over after crash
		CUSTOMFLIGHTMODE_ENDENUM = 29,
};
*/


enum EFLIGHTMODE {EFLIGHTMODE_NONE = 0, EFLIGHTMODE_AUTO, EFLIGHTMODE_STABILIZE, EFLIGHTMODE_RTL, EFLIGHTMODE_ENDENUM};

// класс контролирующий исходящие состояния: параметры hearbeat, режим полета и т.д. - в зависимости от состония: режим bridge или автономность джойстика
class TMAVOUTSTATES {
		bool f_bridge_mode_active;
	
	public:
		void bridge_mode (bool val);
		bool get_bridgemode ();
	
};

//#define C_MAVID_CONFIGDEV 2
//#define C_PILOT_MAV_ID 1


enum EDSTSYS {EDSTSYS_QG_ROUTE = 0, EDSTSYS_QG_SELF, EDSTSYS_RF, EDSTSYS_ENDENUM};
enum ETHRSTATE {ETHRSTATE_NONE = 0, ETHRSTATE_CRUIS = 1, ETHRSTATE_MAX = 2, ETHRSTATE_MIN = 3, ETHRSTATE_ZERO = 4, ETHRSTATE_ENDENUM};
enum ETHRCHECK {ETHRCHECK_PASS = 0, ETHRCHECK_LOW, ETHRCHECK_HI, ETHRCHECK_ENDENUM};
enum EPARACHUTEACT {EPARACHUTEACT_RELEASE = 2, EPARACHUTEACT_MANUAL_OPEN = 3, PARACHUTE_MANUAL_ENDENUM = 4};
enum EPRMSTATE {EPRMSTATE_NONE = 0, EPRMSTATE_LIST_RESP = 1, EPRMSTATE_ENDENUM};


class TMAVCORE: public TFFC {	
	
		uint8_t C_MAVID_CONFIGDEV;
		uint8_t C_MAVID_PILOT;
		uint8_t C_MAVID_ROUTE;
	
		TMAVSTREAM *mavatom_QG_to_RF;		// fifo mavlink messages for direction USB->MODEM
		TMAVSTREAM *mavatom_RF_to_QG;		// fifo mavlink messages for direction MODEM->USB
		TSERIALUSR *serial_QG_obj;			// serial fifo for usb
		TSERIALUSR *serial_RF_obj;			// serial fifo for modem
	
		TMAVPARAMS *params;
	
		virtual void Task () override;
		SYSBIOS::Timer tim_link_detect;
		SYSBIOS::Timer tim_mav_newracom_link;
		SYSBIOS::Timer tim_mav_dtc_link;
		
	
		#ifdef SERIALDEBAG
			uint32_t lost_mavlink_QG_to_RF_cnt;
			uint32_t lost_mavlink_RF_to_QG_cnt;
			uint32_t lost_serial_QG_to_RF_cnt;
			uint32_t lost_serial_RF_to_QG_cnt;
		#endif
	
		void StatusText ();
		//SYSBIOS::Timer test_txt_period;
		
		uint32_t custom_mode_code (EFLIGHTMODE fm);
		EFLIGHTMODE flight_mode_code_from_custom (uint32_t fm);
	
		uint32_t force_arm_code_act;
		uint8_t BoardType;
		//uint8_t SysID_self;
		//uint8_t SysID_RF_hrtbt;
		//uint8_t ComponentID_QG_hrtbt;
		//uint8_t ComponentID_RF_hrtbt;
		//uint8_t SysID_QG_sniff;
		//uint8_t SysID_RF_sniff;
		//uint8_t ComponentID_QG_sniff;
		//uint8_t ComponentID_RF_sniff;

		uint8_t last_seq_QGtoRF;

		SYSBIOS::Timer airspeed_in_timeout;
		float cur_in_airspeed;
		
		int16_t throttle_gen_value ();
		//SYSBIOS::Timer trottle_timeout;
		SYSBIOS::Timer heartbeat_conf_dev_timeout;
		SYSBIOS::Timer timesync_timeout;
		SYSBIOS::Timer joystick_period;
		SYSBIOS::Timer throttle_period;
		
		//SYSBIOS::Timer parachute_period;

		void qg_to_rf_mavlink_analise_task ();
		void heartbeat_to_QG_task ();
		
		//bool f_new_joystick_data;
		int16_t joyst_data_pitch;
		int16_t joyst_data_roll;
		uint16_t joyst_buttons;
		void joystickdata_task ();
		void setmodechange_task ();
		void banomodechange_task ();
		void parachute_tx_task ();
		//void send_myparams_inc_task ();
		void send_mav_cmd_mch_manual_controll (EDSTSYS d);
		void send_mav_autopilot_version ();
		void send_mav_component_information ();
		void send_mav_systime ();
		void send_arm_status_task ();
		
		
		uint32_t c_confdev_systime_period;
		SYSBIOS::Timer confdev_systime_period;
		void send_system_time_task ();
		
		EPRMSTATE confdev_param_state;
		void send_radio_param_list_to_qg_task ();
		
		bool f_new_bano_data;
		uint8_t bano_data;
		
		EFLIGHTMODE curflight_mode_in;		
		EFLIGHTMODE curflight_mode_out;	
		bool cur_arm_status_out;
		bool cur_arm_status_in;		// cur_arm_status_newracom_in
		bool f_setmode_new_cmd;
		bool f_arm_new_cmd;
		
		bool is_free_space_chan_RF_tx ();
		bool is_free_space_chan_QG_tx ();
		
		void send_mav_frame_arm_dissarm (EDSTSYS d);		// QG <-SRC-> RF
		void send_heartbeat_confdev_to_qg ();
		void send_mav_frame_bano (uint8_t prog, EDSTSYS d);		// QG <-SRC-> RF
		//void send_mav_frame_do_set_servo (uint8_t an, uint16_t val);
		void send_mav_frame_command_long (uint16_t cmd, float *arr, uint8_t p_cnt, EDSTSYS d);
		void send_commang_long_ack (uint16_t cmd, bool f_ok, EDSTSYS d);
		void send_mav_frame_myparam_value_to_qg (S_MVPARAM_HDR_T *tag, int16_t ix);
		//void send_mav_frame_throttle (ETHRSTATE s, EDSTSYS d);   // QG <-SRC-> RF
		void send_mav_parachute_act (EPARACHUTEACT act_n, EDSTSYS d);
		
		void send_payload_place ();
		
		
		ETHRSTATE thrcontrl_sw;
		uint8_t cur_throtle_proc_in;

		ETHRCHECK check_feedback_throtle_val (uint8_t vprc);
		int16_t procents_to_throtle (uint8_t proc);
		float tmpmavcmdarr[7];
		void clear_tmpmavarr ();
		
		void sendcmd_set_mode (EFLIGHTMODE mmd, bool f_armst,  EDSTSYS d);	// QG <-SRC-> RF
		
		#ifdef MAVSNIFF
			TMAVSNIFF sniff_qg_modem;
			TMAVSNIFF sniff_modem_qg;
		#endif
		
		bool f_enabled;
		
		
		
		//EPRMINCSW param_inc_sw;
		//uint32_t pilot_param_counts;
		int32_t confdev_prmix_send;
		//int32_t find_mparam (const char *nane16assiz, float &dst, uint8_t &typ);
		
		bool f_new_parachute_data;
		EPARACHUTEACT parachute_action;
		//mavlink_servo_output_raw_t servo_data_in;
		
		

	
	protected:
	
		bool sniff_filter_changer_QG_to_RF (mavlink_message_t &msg);
		bool sniff_filter_changer_RF_to_QG (mavlink_message_t &msg);
		bool filter_command_long_QG_to_RF (mavlink_command_long_t &msg, EDSTSYS d);
	
	public:
		TMAVCORE (TMAVPARAMS *mp, TSERIALUSR *usbser, TSERIALUSR *uartser, uint16_t c_rftoqg_cnt, uint16_t c_qgtorf_cnt);		// количество елементов в фифо буфере
		bool insert_to_RF ( S_MAVDATAFRAME_T &fr);		// mavcommand frame
		bool insert_to_QG ( S_MAVDATAFRAME_T &fr);			// mavcommand frame
	

		bool is_newracom_mav_link ();
		bool is_dtc_mav_link ();
		bool is_common_mav_link ();
		bool is_qg_serial_link ();
		void restart ();
		void enabled (bool val);
		//void send_rc_data (uint8_t ix, float v1, float v2);
		void send_joystick_data (float v_pitch, float v_roll);
		void send_set_mode (EFLIGHTMODE m);
		EFLIGHTMODE get_mode ();
		//bool get_arm_status_newracom ();
		//bool get_arm_status_dtc ();
		bool get_arm_status ();
		void send_arm_status (bool v);
		void send_BANO (uint8_t prog);	
		//void send_parachute ();
		void send_parachute_action (EPARACHUTEACT act_n);
		uint8_t get_pilot_throttle ();
	
		//bool get_servo_status (uint8_t ix, uint32_t &srvval);
		void send_throtle_max_proc ();
		void send_throtle_cruise_proc ();
		void send_throtle_min_proc ();
		void send_throtle_zero_proc ();
		
		void set_mavsysid_self (uint32_t d);
		void set_mavsysid_dest (uint32_t d);
		void set_mavsysid_route (uint32_t d);
		//void set_pilotboard_type (uint32_t d);
		void set_forse_arm_mode (bool v);
	
};


#endif