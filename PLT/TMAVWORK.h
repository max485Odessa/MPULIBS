#ifndef _H_MAVPROCESSOR_H_
#define _H_MAVPROCESSOR_H_


#include "mavlink.h"
#include "TTFIFO.h"
#include "TFIFOLENS.h"
#include "TFTASKIF.h"
#include "tusartdata.h"
#include "SYSBIOS.H"
#include "STMSTRING.h"
#include "TCDC.h"
#include "mavlink_msg_rc_channels_override.h"

#define C_MAVLNK_CH_MODEM_TO_QG MAVLINK_COMM_0
#define C_MAVLNK_CH_QG_TO_MODEM MAVLINK_COMM_1


typedef struct {
	mavlink_message_t frame;
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
	
		//TTFIFO<S_MAVDATAFRAME_T> *mavfifo;
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



class TMAVCORE: public TFFC {	
	
		TMAVSTREAM *mavatom_QG_to_RF;		// fifo mavlink messages for direction USB->MODEM
		TMAVSTREAM *mavatom_RF_to_QG;		// fifo mavlink messages for direction MODEM->USB
		TSERIALUSR *serial_QG_obj;			// serial fifo for usb
		TSERIALUSR *serial_RF_obj;			// serial fifo for modem
		virtual void Task () override;
		SYSBIOS::Timer tim_link_detect;
	
		#ifdef SERIALDEBAG
			uint32_t lost_mavlink_QG_to_RF_cnt;
			uint32_t lost_mavlink_RF_to_QG_cnt;
			uint32_t lost_serial_QG_to_RF_cnt;
			uint32_t lost_serial_RF_to_QG_cnt;
		#endif
	
		void StatusText ();
		SYSBIOS::Timer test_txt_period;
	
		uint8_t last_system_id, last_component_id;
		uint8_t last_seq_QG;
	
		
		SYSBIOS::Timer joystick_period;

		
		bool f_new_joystick_data;
		int16_t joyst_data_pitch;
		int16_t joyst_data_roll;
		uint16_t joyst_buttons;
		void joystickdata_task ();
		void send_mav_manual_controll ();
		SYSBIOS::Timer joystick_last_start;
		//void send_rc_override ();
		
		bool f_new_mavmode_data;
		MAV_MODE set_mavmode;
		
		bool send_command_long (uint16_t cmd, float *arr, uint8_t p_cnt);
		
		#ifdef MAVSNIFF
			TMAVSNIFF sniff_qg_modem;
			TMAVSNIFF sniff_modem_qg;
		#endif
		
		bool f_enabled;
	
	protected:
		void sniff_mavlink_QG_to_RF (mavlink_message_t &msg, mavlink_status_t *s);
		void sniff_mavlink_RF_to_QG (mavlink_message_t &msg, mavlink_status_t *s);
	
	public:
		TMAVCORE (TSERIALUSR *usbser, TSERIALUSR *uartser, uint16_t c_rftoqg_cnt, uint16_t c_qgtorf_cnt);		// количество елементов в фифо буфере
		bool insert_to_RF (const S_MAVDATAFRAME_T &fr);		// mavcommand frame
		bool insert_to_QG (const S_MAVDATAFRAME_T &fr);			// mavcommand frame
	
		bool is_modem_link ();
		bool is_usb_link ();
		void restart ();
		void enabled (bool val);
		
		//void send_rc_data (uint8_t ix, float v1, float v2);
		void send_joystick_data (float v_pitch, float v_roll, uint16_t butns);
		void send_set_mode (MAV_MODE m);
};


#endif