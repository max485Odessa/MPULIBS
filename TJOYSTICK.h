#ifndef _H_JOYSTIC_CLASS_H_
#define _H_JOYSTIC_CLASS_H_



#include "hard_rut.h"
#include "SYSBIOS.H"
#include "TFTASKIF.h"



enum EJSTCPINS {EJSTCPINS_B1 = 0, EJSTCPINS_B2, EJSTCPINS_B3, EJSTCPINS_B4, EJSTCPINS_B5, EJSTCPINS_B6, EJSTCPINS_B7, EJSTCPINS_B8, EJSTCPINS_ENDENUM = 8};
enum EJSTCA {EJSTCA_X = 0, EJSTCA_Y = 1, EJSTCA_ENDENUM = 2};
enum EJSTMSG {EJSTMSG_NONE = 0, EJSTMSG_CLICK, EJSTMSG_DBLCLICK, EJSTMSG_ENDENUM = 3};


typedef struct {
	float min_value;
	float max_value;
	float midl_value;
} S_JOYCALIBR_T;


typedef struct {
	S_JOYCALIBR_T calibset;
	// Датчик магнитный не резистивный. Предпологается что в джойстике есть асимметрия диапазона перемещения стиков, поэтому ввел разные значения множителей для каждого направления.
	float mult_band_plus;		// только положительная величина
	float mult_band_minus;	// только положительная величина
	// результаты измерения по диапазону перемещения приводятся к величинам -1.0F,  1.0F
	float cur_axis_value;		// высчитанное текущеее значение джойстика
	bool f_result_ok;
} S_JOYAXISSETS_T;



typedef struct {
	S_GPIOPIN keypin;
	uint32_t push_time;		// pushed time
	uint32_t pop_time;		// time after pop key (for control doubleclick)
	uint32_t last_push_time;	
	uint32_t block_time;		// blocked time for messages
	bool f_block_next_msg;	// blocked next message
	bool pushstate_cur;			// current state
	bool pushstate_prev;		// prev state
	EJSTMSG messg;
} S_KEYSETS_T;



class TJOYSTIC: public TFFC {
		S_KEYSETS_T *pins;
		const uint8_t c_pins_cnt;
		uint8_t gmsg_ix;

		void clear_key (S_KEYSETS_T &kp);
		void update_push_state_all ();
		bool update_push_state (S_KEYSETS_T &ps);
	
		virtual void Task () override;
		uint32_t last_ticks;
	
		SYSBIOS::Timer relax_jstc_tim;
		void joyst_subtask ();
		S_JOYAXISSETS_T axiscalibr[EJSTCA_ENDENUM];	
		float c_volt_move_quant;
		const float *volt_x;
		const float *volt_y;
	
		uint32_t last_pushed_mask;
	
	public:
		TJOYSTIC (S_GPIOPIN *p, const uint8_t pc, const float *x, const float *y);
	
		void block_next_msg (EJSTCPINS p);
		void block_time (EJSTCPINS p, uint32_t tbl);
	
		uint32_t get_pushed_mask ();
		uint32_t get_pushtime_cur (EJSTCPINS p);
		uint32_t get_pushtime_last (EJSTCPINS p);
		EJSTMSG get_message (EJSTCPINS &kn);	
	
		float axis_value (EJSTCA ax);			// -1; 0 ; 1
		void set_calibrate_axis_band (EJSTCA ax, S_JOYCALIBR_T *v);
		bool get_axis (float &val_x, float &val_y);

};



#endif