#ifndef _H_MAVPARAMS_H_
#define _H_MAVPARAMS_H_


#include <stdint.h>
#include "TM24Cxxx.h"
#include "mavlink.h"
#include "STMSTRING.h"
#include "TM24Cxxx.h"



// список внутрених параметров

#define C_EXTMVPR EINTMVPR_ENDENUM  // 


#pragma pack (push, 1)

typedef struct {
	char param_id[16];
} S_MAVPARAMNAME_T;

typedef struct {
	char param_id[16];
	MAV_PARAM_TYPE type;
} S_MVPARAM_HDR_T;



typedef struct {
	S_MVPARAM_HDR_T hdr;
	float value;
	float min;
	float max;
	float def_value;
} S_MVPARAM_FLOAT_T;



typedef struct {
	S_MVPARAM_HDR_T hdr;
	uint32_t value;
	uint32_t min;
	uint32_t max;
	uint32_t def_value;
} S_MVPARAM_U32_T;


typedef struct {
	S_MVPARAM_HDR_T hdr;
	int32_t value;
	int32_t min;
	int32_t max;
	int32_t def_value;
} S_MVPARAM_I32_T;



typedef struct {
	union {
			uint32_t u32;
			float f;
	} u;
} S_MDAT_T;



typedef struct {
	S_MVPARAM_HDR_T hdr;
	S_MDAT_T value;
	S_MDAT_T min;
	S_MDAT_T max;
	S_MDAT_T def_value;
} S_BASETAGPARAM_T;



typedef struct {
	uint32_t params_cnt;
	S_MDAT_T data;
	uint32_t crc32;
} S_SAVE_PARAMS_T;



#pragma pack (pop)


class TPARAMCONTRL {
	protected:
		bool f_need_save;
		S_MVPARAM_HDR_T **param_internal;
		uint32_t param_int_count;
		S_BASETAGPARAM_T *param_external;
		uint32_t param_ext_max_alocate;
		uint32_t param_ext_count;
	
		TEEPROMIF *flashmem;
		const uint32_t c_startflash_adr;
		const uint32_t c_sizeflash;	

		bool load_internal_params ();
		bool save_internal_params ();
		
		
		S_MVPARAM_HDR_T *find_param_to_name (char *name, long *dst_ix);
		
		TPARAMCONTRL (TEEPROMIF *m, uint32_t start_m, uint32_t size_m);
		
	
	public:
		void set_param_to_default (long ix);
		bool set_value (char *p, S_MDAT_T v);
		bool set_value (long ix, S_MDAT_T v);
		bool set_value_l (long ix, long d);
		S_MDAT_T *get_value (char *name);
		S_MDAT_T *get_value (long ix);
		S_MDAT_T get_value_d (long ix);
		S_MVPARAM_HDR_T *get_param_tag (long ix);
		void sets_internal_params_to_default ();
		uint32_t param_internal_cnt ();
		uint32_t param_external_cnt ();
		uint32_t param_full_cnt ();
		bool is_changed ();
};



class TMAVPARAMS: public TPARAMCONTRL {
		
		S_MVPARAM_HDR_T *find_params_m (mavlink_param_value_t *p);
		void update_mavparamval (S_BASETAGPARAM_T *s, mavlink_param_value_t &dst);
	
	public:
		TMAVPARAMS (TEEPROMIF *m, uint32_t start_m, uint32_t size_m, S_MVPARAM_HDR_T **list_int, uint32_t cnt_int, uint32_t cnt_external);
		long add_external_params (S_MVPARAM_HDR_T *p);		
	
		bool get_mavlink_param (char *name, mavlink_param_value_t *dst, long *dst_ix);
		bool get_mavlink_param (long ix, mavlink_param_value_t &dst);
		bool update_mavlink_param (const mavlink_param_value_t &src);
		bool update_mavlink_param (const mavlink_param_set_t &src, long *dst_ix);
		void save_to_flash ();

};



#endif
