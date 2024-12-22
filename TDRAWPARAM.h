#ifndef _H_STM32HALL_GRAPH_PARAM_OBJECT_H_
#define _H_STM32HALL_GRAPH_PARAM_OBJECT_H_



#include <stdint.h>
#include "TPARAMCONTROL.h"
#include "STMSTRING.h"



class TPARAMCB {
	public:
		virtual float get_param_f (uint32_t ix) = 0;
		virtual uint32_t get_param_u32 (uint32_t ix) = 0;
		virtual int32_t get_param_i32 (uint32_t ix) = 0;
};


typedef struct {
	MAV_PARAM_TYPE type;
	TPARAMCB *cb;
	uint32_t cb_p_ix;
	uint8_t cnt;
	uint8_t *txt;
	uint8_t dign_f;
	TSTMSTRING *str;
	uint8_t *txt_pre;
	//uint8_t len_pre;
	uint8_t *txt_post;
	char *txt_bool_on;
	char *txt_bool_off;
	//uint8_t len_post;
	union {
		uint32_t *u32;
		float *fl;
		int32_t *i32;
		uint16_t *u16;
		int16_t *i16;
		uint8_t *u8;
		int8_t *i8;
		bool *bl;
		} u;
} S_TGPH_PARAML_LP_UN_T;



class TGRAPHPARAM {
		S_TGPH_PARAML_LP_UN_T data;
		void update ();
	
	public:
		TGRAPHPARAM ();
	
		explicit TGRAPHPARAM (TPARAMCB *cb, MAV_PARAM_TYPE pt, uint32_t pix, uint8_t c);
	
		explicit TGRAPHPARAM (float *v, uint8_t c, uint8_t dfn);
		explicit TGRAPHPARAM (uint32_t *v, uint8_t c);
		explicit TGRAPHPARAM (int32_t *v, uint8_t c);
		explicit TGRAPHPARAM (uint16_t *v, uint8_t c);
		explicit TGRAPHPARAM (int16_t *v, uint8_t c);
		explicit TGRAPHPARAM (uint8_t *v, uint8_t c);
		explicit TGRAPHPARAM (int8_t *v, uint8_t c);
		explicit TGRAPHPARAM (bool *v, uint8_t c, const char *t_on, const char *t_off);
		void set_prephix (const char *txt);
		void set_postfix (const char *txt);
		const TSTMSTRING &get_txt ();
		TSTMSTRING *get_lp_txt ();

};



#endif
