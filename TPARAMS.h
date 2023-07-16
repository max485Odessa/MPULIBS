#ifndef _h_rf_params_data_h_
#define _h_rf_params_data_h_



#include <stdint.h>



enum EPARAMTYPE {EPARAMTYPE_NONE = 0, EPARAMTYPE_BOOL = 1, EPARAMTYPE_I32 = 2, EPARAMTYPE_U32 = 3, EPARAMTYPE_I64 = 4, EPARAMTYPE_U64 = 5, EPARAMTYPE_RAW8 = 6, EPARAMTYPE_FLOAT = 7, EPARAMTYPE_STR32 = 8, EPARAMTYPE_ENDENUM = 9};
enum EPRMIX {EPRMIX_PRESS_PROFILE = 0, EPRMIX_PRESS_ON_A = 1, EPRMIX_PRESS_OFF_A = 2, EPRMIX_PRESS_ON_B = 3, EPRMIX_PRESS_OFF_B = 4, EPRMIX_TIME_RELAX = 5, EPRMIX_MODE = 6, EPRMIX_CALIBR_PRESS_ZERO = 7, \
EPRMIX_CALIBR_PRESS_MAX = 8, EPRMIX_PRESS_BAR = 9, EPRMIX_ENDENUM = 10};



typedef struct {
	union {
		bool v_b;
		uint8_t raw[8];
		int64_t v_i64;
		uint64_t v_u64;
		int32_t v_i32;
		uint32_t v_u32;
		float v_f;
	} u;
} S_PARAMVALUE_T;



typedef struct {
	const EPARAMTYPE type;
} S_HDRPARAM_T;



typedef struct {
	const EPARAMTYPE type;
	const char *name;
	const float min;
	const float max;
	const float def;
} S_CONTROL_FLOAT_T;


typedef struct {
	const EPARAMTYPE type;
	const char *name;
	const int64_t min;
	const int64_t max;
	const int64_t def;
} S_CONTROL_INT64_T;


typedef struct {
	const EPARAMTYPE type;
	const char *name;
	const uint32_t min;
	const uint32_t max;
	const uint32_t def;
} S_CONTROL_UINT32_T;


typedef struct {
	uint8_t type;			// EPARAMTYPE
	S_PARAMVALUE_T data;
} S_DATAFLASH_T;



class IRFPARAMS {
	protected:
		S_DATAFLASH_T data[EPRMIX_ENDENUM];
		void clear_data_todef ();
		void correct_all ();
		static S_HDRPARAM_T *list[EPRMIX_ENDENUM];
		static IRFPARAMS *singlobj;
	public:
		bool get_papam_i32 (EPRMIX ix, long &dst);
		long get_papam_i32 (EPRMIX ix);
	
		bool get_papam_u32 (EPRMIX ix, uint32_t &dst);
		uint32_t get_papam_u32 (EPRMIX ix);
	
		bool get_papam_f (EPRMIX ix, float &dst);
		float get_papam_f (EPRMIX ix);
	
		bool get_papam_b (EPRMIX ix, bool &dst);
		bool get_papam_b (EPRMIX ix);
		
	
	
		void set_papam_f (EPRMIX ix, float prm);
		void set_papam_b (EPRMIX ix, bool prm);
		void set_papam_u32 (EPRMIX ix, uint32_t prm);
		void set_papam_i32 (EPRMIX ix, long prm);
		 
		 virtual void load ();
		 virtual void save ();
		 void correct_param (EPRMIX ix);
		 void param_todef (EPRMIX ix);
		 EPARAMTYPE gettype (EPRMIX ix);
		 static IRFPARAMS *obj ();
};





#endif
