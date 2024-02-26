#ifndef _h_rfcmd_defines_h_
#define _h_rfcmd_defines_h_

#include <stdint.h>


#define C_SERIALNUMBER_SIZE 5
#define C_PARAMCAPTION_SIZE 32
#define C_MEAS_TXT_VAL_SIZE 8

enum ESENSTYPE {ESENSTYPE_NONE = 0, \
ESENSTYPE_PRESSURE = 1, \
};

enum EPRMFTYPE {EPRMFTYPE_NONE = 0, EPRMFTYPE_BOOL = 1, EPRMFTYPE_I32 = 2, EPRMFTYPE_U32 = 3, EPRMFTYPE_I64 = 4, EPRMFTYPE_U64 = 5, EPRMFTYPE_RAW8 = 6, EPRMFTYPE_FLOAT = 7, EPRMFTYPE_STR32 = 8, EPRMFTYPE_ENDENUM = 9};

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
} S_PRMF_VALUE_T;

typedef struct {
	uint8_t n[C_SERIALNUMBER_SIZE];
} S_RFSERIAL_T;

typedef struct {
	char txt[C_PARAMCAPTION_SIZE];
} S_PRMF_CAPTION_T;

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
			S_PRMF_VALUE_T min;
			S_PRMF_VALUE_T max;
			S_PRMF_VALUE_T def;
			S_PRMF_VALUE_T val;
			} dig;
		char txt[sizeof(S_PRMF_VALUE_T) * 4];	// max size only 32 charsets
		} u;
} S_FULLPARAM_T;


typedef struct {
	uint8_t type;				// EPRMFTYPE
	S_FULLPARAM_T param;
} S_RFPARAMVALUE_T;




typedef struct {
	uint32_t event_code;
	S_PRMF_CAPTION_T caption;
} S_EVENT_ITEM_T;



#pragma pack (pop)


#pragma pack (push,1)

/*
enum ECMDLAN {ECMDLAN_NONE = 0, \
ECMDLAN_GET_PARAM_REQ = 1, ECMDLAN_GET_PARAM_RESP = 2, \
ECMDLAN_SET_PARAM_REQ = 3, ECMDLAN_SET_PARAM_RESP = 4, \
ECMDLAN_GET_EVENT_REQ = 5, ECMDLAN_GET_EVENT_RESP = 6, \
ECMDLAN_CALL_EVENT_REQ = 7, ECMDLAN_CALL_EVENT_RESP = 8, \
ECMDLAN_GET_STATE_REQ = 9, ECMDLAN_GET_STATE_RESP = 10, \
ECMDLAN_ENDENUM = 11};
*/

enum ECMDLAN {ECMDLAN_NONE = 0, \
ECMDLAN_GET_PARAM_REQ = 1,  \
ECMDLAN_SET_PARAM_REQ = 2,  \
ECMDLAN_GET_EVENT_REQ = 3,  \
ECMDLAN_CALL_EVENT_REQ = 4,  \
ECMDLAN_GET_STATE_REQ = 5,  \
ECMDLAN_ENDENUM = 6};



typedef struct {
    uint8_t pack_info;      // 7-6 bits (00 - midle payload, 01 - stop frame, 10 - start frame, 11 - start and stop frame), 5 - bit (1 - master tx, 0 - slave tx), 4-0 (transaction id)
    uint8_t size;           // local size
    uint8_t payload[60];
} S_RFCAPSULA_T;



enum ERFRESPSTAT  {ERFRESPSTAT_OK = 0, ERFRESPSTAT_PROGRESS = 1, ERFRESPSTAT_ERROR = 2};


typedef struct {
	uint8_t cmd;            // cmd
    uint8_t trid;           // (7 - (1 = req, 0 = resp), 6-5 (ERFRESPSTAT), 4-0 (5 bit transaction id))
	uint8_t cmd_size;      // if cmd not support
    uint8_t crc; 
	local_rf_id_t src_id;
	local_rf_id_t dst_id;
} S_RFHEADER_T;


typedef struct {
	uint16_t sensor_type;		// ESENSTYPE
	S_RFPARAMVALUE_T value;
	S_MEAS_TYPE_TXT_T meas_txt;
	S_PRMF_CAPTION_T state_in_txt;
} S_DEVSTATE_T;


// ------------------ GET_PARAM ----------------
// запрашивает параметр по индексу или имени (если индекс -1, то используется имя параметра)
typedef struct {
	S_RFHEADER_T hdr;
	int16_t ix;
	S_PRMF_CAPTION_T name;
} S_CMD_GET_PARAM_REQ_T;



typedef struct {
	S_RFHEADER_T hdr;
	uint8_t resp_state;		// ERESPSTATE
	int16_t ix;
	S_PRMF_CAPTION_T name;
	S_RFPARAMVALUE_T param;
} S_CMD_GET_PARAM_RESP_T;



// ------------------ SET_PARAM ----------------
// устанавливает параметр по индексу или имени (если индекс -1, то используется имя параметра)
typedef struct {
	S_RFHEADER_T hdr;
	int16_t ix;
	S_PRMF_CAPTION_T name;
	S_RFPARAMVALUE_T param;
} S_CMD_SET_PARAM_REQ_T;


typedef struct {
	S_RFHEADER_T hdr;
	uint8_t resp_state;		// ERESPSTATE
	int16_t ix;
	S_PRMF_CAPTION_T name;
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



// sizeof max frame
typedef struct {
    union {
        S_CMD_GET_STATE_REQ_T gs_req;
        S_CMD_GET_STATE_RESP_T gs_resp;

        S_CMD_CALL_EVENT_REQ_T ce_req;
        S_CMD_CALL_EVENT_RESP_T ce_resp;

        S_CMD_GET_EVENT_REQ_T ge_req;
        S_CMD_GET_EVENT_RESP_T ge_resp;

        S_CMD_SET_PARAM_REQ_T sp_req;
        S_CMD_SET_PARAM_RESP_T sp_resp;

        S_CMD_GET_PARAM_RESP_T gp_resp;
        S_CMD_GET_PARAM_REQ_T gp_req;

    } u;
} S_RFCMDS_SIZES_UNION_T;



#pragma pack (pop)


#endif
