#ifndef _H_JOYSTIC_CLASS_H_
#define _H_JOYSTIC_CLASS_H_



#include "hard_rut.h"
#include "SYSBIOS.H"
#include "TFTASKIF.h"



enum EJSTCPINS {EJSTCPINS_B1 = 0, EJSTCPINS_B2, EJSTCPINS_B3, EJSTCPINS_B4, EJSTCPINS_B5, EJSTCPINS_B6, EJSTCPINS_B7, EJSTCPINS_B8, EJSTCPINS_ENDENUM = 8};
enum EJSTCA {EJSTCA_X = 0, EJSTCA_Y, EJSTCA_ENDENUM};
enum EJSTMSG {EJSTMSG_NONE = 0, EJSTMSG_CLICK, EJSTMSG_DBLCLICK, EJSTMSG_ENDENUM = 3};


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
		S_KEYSETS_T *pins;//[EJSTCPINS_ENDENUM];
		const uint8_t c_pins_cnt;
		uint8_t gmsg_ix;

		void clear_key (S_KEYSETS_T &kp);
		void update_push_state_all ();
		void update_push_state ( S_KEYSETS_T &ps);
		const float *volt_x;
		const float *volt_y;
	
		virtual void Task () override;
		uint32_t last_ticks;
		
	
	public:
		TJOYSTIC (S_GPIOPIN *p, const uint8_t pc, const float *x, const float *y);
	
		void block_next_msg (EJSTCPINS p);
		void block_time (EJSTCPINS p, uint32_t tbl);
	
		uint32_t get_pushtime_cur (EJSTCPINS p);
		uint32_t get_pushtime_last (EJSTCPINS p);
		EJSTMSG get_message (EJSTCPINS &kn);	
		float axis_value (EJSTCA ax);			// -1; 0 ; 1
		
};



#endif