#ifndef _h_rfcmd_defines_h_
#define _h_rfcmd_defines_h_

#include <stdint.h>
//#include "TPARAMS.h"

#define C_SERIALNUMBER_SIZE 5
#define C_PARAMCAPTION_SIZE 32
#define C_MEAS_TXT_VAL_SIZE 8

enum ESENSTYPE {ESENSTYPE_NONE = 0, \
ESENSTYPE_PRESSURE = 1, \
};

enum EPARAMTYPE {EPARAMTYPE_NONE = 0, EPARAMTYPE_BOOL = 1, EPARAMTYPE_I32 = 2, EPARAMTYPE_U32 = 3, EPARAMTYPE_I64 = 4, EPARAMTYPE_U64 = 5, EPARAMTYPE_RAW8 = 6, EPARAMTYPE_FLOAT = 7, EPARAMTYPE_STR32 = 8, EPARAMTYPE_ENDENUM = 9};

typedef uint16_t local_rf_id_t;
enum ERESPSTATE {ERESPSTATE_OK = 0, ERESPSTATE_ERROR = 1, ERESPSTATE_TIMEOUT = 2, ERESPSTATE_ENDENUM = 3};

//enum EEVRESULT {EEVRESULT_OK = 0, EEVRESULT_PROCESS = 1, EEVRESULT_ERROR = 2, EEVRESULT_TIMEOUT = 3, EEVRESULT_ENDENUM = 4};

#pragma pack (push,1)

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
	uint8_t n[C_SERIALNUMBER_SIZE];
} S_RFSERIAL_T;

typedef struct {
	char txt[C_PARAMCAPTION_SIZE];
} S_PARAM_CAPTION_T;

typedef struct {
	char txt[C_MEAS_TXT_VAL_SIZE];
} S_MEAS_TYPE_TXT_T;



typedef struct {
	S_RFSERIAL_T serial;
	uint8_t type;
	uint8_t group;
	uint8_t mask;
	local_rf_id_t id;
} S_ABONENT_ITEM_T;






typedef struct {
	union {
		struct {
			S_PARAMVALUE_T min;
			S_PARAMVALUE_T max;
			S_PARAMVALUE_T def;
			S_PARAMVALUE_T val;
			} dig;
		char txt[sizeof(S_PARAMVALUE_T) * 4];	// max size only 32 charsets
		} u;
} S_FULLPARAM_T;


typedef struct {
	uint8_t type;				// EPARAMTYPE
	S_FULLPARAM_T param;
} S_RFPARAMVALUE_T;




typedef struct {
	uint32_t event_code;
	S_PARAM_CAPTION_T caption;
} S_EVENT_ITEM_T;



#pragma pack (pop)


#pragma pack (push,1)

enum ECMDLAN {ECMDLAN_NONE = 0, \
ECMDLAN_GET_PARAM_REQ = 1, ECMDLAN_GET_PARAM_RESP = 2, \
ECMDLAN_SET_PARAM_REQ = 3, ECMDLAN_SET_PARAM_RESP = 4, \
ECMDLAN_GET_EVENT_REQ = 5, ECMDLAN_GET_EVENT_RESP = 6, \
ECMDLAN_CALL_EVENT_REQ = 7, ECMDLAN_CALL_EVENT_RESP = 8, \
ECMDLAN_GET_STATE_REQ = 9, ECMDLAN_GET_STATE_RESP = 10, \
ECMDLAN_ENDENUM = 11};



typedef struct {
	uint16_t cmd;
	uint16_t cmd_size;
	local_rf_id_t src_id;
	local_rf_id_t dst_id;
} S_RFHEADER_T;


typedef struct {
	uint16_t sensor_type;		// ESENSTYPE
	S_RFPARAMVALUE_T value;
	S_MEAS_TYPE_TXT_T meas_txt;
	S_PARAM_CAPTION_T state_in_txt;
} S_DEVSTATE_T;


// ------------------ GET_PARAM ----------------
// запрашивает параметр по индексу или имени (если индекс -1, то используется имя параметра)
typedef struct {
	S_RFHEADER_T hdr;
	int16_t ix;
	S_PARAM_CAPTION_T name;
} S_CMD_GET_PARAM_REQ_T;



typedef struct {
	S_RFHEADER_T hdr;
	uint8_t resp_state;		// ERESPSTATE
	int16_t ix;
	S_PARAM_CAPTION_T name;
	S_RFPARAMVALUE_T param;
} S_CMD_GET_PARAM_RESP_T;



// ------------------ SET_PARAM ----------------
// устанавливает параметр по индексу или имени (если индекс -1, то используется имя параметра)
typedef struct {
	S_RFHEADER_T hdr;
	int16_t ix;
	S_PARAM_CAPTION_T name;
	S_RFPARAMVALUE_T param;
} S_CMD_SET_PARAM_REQ_T;


typedef struct {
	S_RFHEADER_T hdr;
	uint8_t resp_state;		// ERESPSTATE
	int16_t ix;
	S_PARAM_CAPTION_T name;
	S_RFPARAMVALUE_T param;
} S_CMD_SET_PARAM_RESP_T;



// ------------------ GET_EVENT ----------------
typedef struct {
	S_RFHEADER_T hdr;
	int16_t ix;
} S_CMD_GET_EVENT_REQ_T;


typedef struct {
	S_RFHEADER_T hdr;
	uint8_t resp_state;		// ERESPSTATE
	int16_t ix;
	S_EVENT_ITEM_T event;
} S_CMD_GET_EVENT_RESP_T;



// ------------------ CALL_EVENT ----------------
typedef struct {
	S_RFHEADER_T hdr;
	uint32_t event_code;
	uint32_t time_event_work;		// время работы события lsb = 1 ms
} S_CMD_CALL_EVENT_REQ_T;



typedef struct {
	S_RFHEADER_T hdr;
	uint8_t resp_state;		// ERESPSTATE
	uint32_t event_code;
} S_CMD_CALL_EVENT_RESP_T;


// ------------------ GET_STATE ----------------
typedef struct {
	S_RFHEADER_T hdr;
} S_CMD_GET_STATE_REQ_T;



typedef struct {
	S_RFHEADER_T hdr;
	uint8_t resp_state;		// ERESPSTATE
	S_DEVSTATE_T state;
} S_CMD_GET_STATE_RESP_T;

#pragma pack (pop)


#endif
