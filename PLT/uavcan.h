#ifndef __UAVCAN_H__
#define __UAVCAN_H__

#include "rutine.h"

void uavcanInit(void);

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

#include "stm32_def.h"
#include "canard.h"
#include "dwt_stm32_delay.h"
#include "can.h"
#include "LogMessage.h"
#include "TADSBUAV.H"
	
#define APP_NODE_NAME                                               "spaitech.charger.PD"
//#define GIT_HASH                                                    0xBADC0FFE

#define UAVCAN_GET_NODE_INFO_DATA_TYPE_SIGNATURE                    0xee468a8121c46a9e
#define UAVCAN_GET_NODE_INFO_DATA_TYPE_ID                           1
#define UAVCAN_GET_NODE_INFO_RESPONSE_MAX_SIZE                      ((3015 + 7) / 8)

#define UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_ID                          1030
#define UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_SIGNATURE                   0x217f5c87d7ec951d
#define UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_MAX_VALUE                   8192

#define UNIQUE_ID_LENGTH_BYTES                                      16

#define UAVCAN_NODE_STATUS_MESSAGE_SIZE                             7
#define UAVCAN_NODE_STATUS_DATA_TYPE_ID                             341
#define UAVCAN_NODE_STATUS_DATA_TYPE_SIGNATURE                      0x0f0868d0c1a7c6f1

#define UAVCAN_PROTOCOL_PARAM_GETSET_ID                             11
#define UAVCAN_PROTOCOL_PARAM_GETSET_SIGNATURE                      0xa7b622f939d1a4d5    

#define UAVCAN_EQUIPMENT_AIRDATA_TRUEAIRSPEED_ID                    1020
#define UAVCAN_EQUIPMENT_AIRDATA_TRUEAIRSPEED_SIGNATURE             0x306f69e0a591afaa
#define UAVCAN_EQUIPMENT_AIRDATA_TRUEAIRSPEED_SIZE                  32


typedef unsigned long uint;

typedef struct
{
int8_t* name;
int64_t val; 
int64_t min;
int64_t max;
int64_t defval;
} param_t;


typedef struct
{
int8_t* name;
float val; 
float min;
float max;
float defval;
} param_tf32;



/*
typedef struct
{
int8_t const* name;
union {
	int64_t i64;
	float f32;
	bool b;
	} val;
union {
	int64_t i64;
	float f32;
	bool b;
	} min;
union {
	int64_t i64;
	float f32;
	bool b;
	} max;
union {
	int64_t i64;
	float f32;
	bool b;
	} defval;

} param_tu;
*/


void readUniqueID(uint8_t* out_uid);
void showRcpwmonUart(void);
void onTransferReceived(CanardInstance* ins, CanardRxTransfer* transfer);
void SendCanard(void);
void receiveCanard(void);
void showRcpwmonUart(void);
void rawcmdHandleCanard(CanardRxTransfer* transfer);
void getsetHandleCanard(CanardRxTransfer* transfer);
void getNodeInfoHandleCanard(CanardRxTransfer* transfer);
void makeNodeStatusMessage(uint8_t buffer[UAVCAN_NODE_STATUS_MESSAGE_SIZE]);
uint16_t makeNodeInfoMessage(uint8_t buffer[UAVCAN_GET_NODE_INFO_RESPONSE_MAX_SIZE]);
void SendLogMsg(uint8_t* text, const uint size);
void TestAirData();




#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif
