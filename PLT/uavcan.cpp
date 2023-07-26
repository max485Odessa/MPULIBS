#include "uavcan.h"
#include "canard_stm32.h"
#include "SYSBIOS.H"
#include "RestartNode.h"
#include "BeginFirmwareUpdate.h"
#include "Read.h"
#include "GetInfo.h"
#include "ExecuteOpcode.h"
#include "main.h"
#include "canard.h"
#include "cancore.h"
#include "arrayCommand.h"
#include "uav_charger_cmd.h"


#define CANARD_SPIN_PERIOD    500
#define PUBLISHER_PERIOD_mS   1000
#define RC_NUM_CHANNELS       6

CanardInstance g_canard;                //The library instance		23.11.19	убрал static для того чтобы обьявить на него extern (обьект был нужен в моем классе в который хочу перенести весть код с CANARD.C)

//static uint32_t  g_uptime = 0;
//int16_t rc_pwm[RC_NUM_CHANNELS] = {0};


//////////////////////////////////////////////////////////////////////////////////////

static uint16_t IdDD = 0;
static uint16_t IdDD_Broad = 0;
static uint16_t IdDD_resp = 0;


bool shouldAcceptTransfer(const CanardInstance* ins,
                          uint64_t* out_data_type_signature,
                          uint16_t data_type_id,
                          CanardTransferType transfer_type,
                          uint8_t source_node_id)
{
	// индивидуальная адресация
	if (transfer_type == CanardTransferTypeRequest)
		{
		switch (data_type_id)
			{
			case UAVCAN_EQUIPMENT_ACTUATOR_ARRAYCOMMAND_ID:
				{
				*out_data_type_signature = UAVCAN_EQUIPMENT_ACTUATOR_ARRAYCOMMAND_SIGNATURE;
				break;
				}
			case UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_ID:
				{
				*out_data_type_signature = UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_SIGNATURE;
				return true;
				}
			case UAVCAN_PROTOCOL_PARAM_GETSET_ID:
				{
				*out_data_type_signature = UAVCAN_PROTOCOL_PARAM_GETSET_SIGNATURE;
				return true;
				}
			case UAVCAN_PROTOCOL_PARAM_EXECUTEOPCODE_ID:
				{
				*out_data_type_signature = UAVCAN_PROTOCOL_PARAM_EXECUTEOPCODE_SIGNATURE;
				return true;
				}
			case UAVCAN_PROTOCOL_RESTARTNODE_ID:
				{
				*out_data_type_signature = UAVCAN_PROTOCOL_RESTARTNODE_SIGNATURE;
				return true;
				}
			case UAVCAN_PROTOCOL_FILE_READ_ID:
				{
				*out_data_type_signature = UAVCAN_PROTOCOL_FILE_READ_SIGNATURE;
				return true;
				}
			case UAVCAN_PROTOCOL_FILE_GETINFO_ID:
				{
				*out_data_type_signature = UAVCAN_PROTOCOL_FILE_GETINFO_SIGNATURE;
				return true;
				}
			case UAVCAN_GET_NODE_INFO_DATA_TYPE_ID:
				{
				*out_data_type_signature = UAVCAN_GET_NODE_INFO_DATA_TYPE_SIGNATURE;
				return true;
				}
			case UAVCAN_PROTOCOL_FILE_BEGINFIRMWAREUPDATE_ID:
				{
				*out_data_type_signature = UAVCAN_PROTOCOL_FILE_BEGINFIRMWAREUPDATE_SIGNATURE;
				return true;
				}
			default:
				{
				if (!IdDD) IdDD = data_type_id;
				break;
				}
			}
		}
		
	if (transfer_type == CanardTransferTypeResponse)
		{
		switch (data_type_id)
			{
			case UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_ID:
				{
				*out_data_type_signature = UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_SIGNATURE;
				return true;
				}
			case UAVCAN_PROTOCOL_PARAM_GETSET_ID:
				{
				*out_data_type_signature = UAVCAN_PROTOCOL_PARAM_GETSET_SIGNATURE;
				return true;
				}
			case UAVCAN_PROTOCOL_FILE_READ_ID:
				{
				*out_data_type_signature = UAVCAN_PROTOCOL_FILE_READ_SIGNATURE;
				return true;
				}
			case UAVCAN_PROTOCOL_FILE_GETINFO_ID:
				{
				*out_data_type_signature = UAVCAN_PROTOCOL_FILE_GETINFO_SIGNATURE;
				return true;
				}
			default:
				{
				if (!IdDD_resp) IdDD_resp = data_type_id;
				break;
				}
					
			}
		}
		
	

	// broadcast адресация
	if (transfer_type == CanardTransferTypeBroadcast)
		{
		switch (data_type_id)
			{
			case UAVCAN_NODE_STATUS_DATA_TYPE_ID:
				{
				*out_data_type_signature = UAVCAN_NODE_STATUS_DATA_TYPE_SIGNATURE;
				return true;
				}
			case UAVCAN_SPAIBASE_CMD_ID:
				{
				*out_data_type_signature = UAVCAN_SPAIBASE_CMD_SIGNATURE;
				return true;
				}
			case UAVCAN_SCHARGERBAT_DATA_ID:
				{
				*out_data_type_signature = UAVCAN_SCHARGERBAT_DATA_SIGNATURE;
				return true;
				}
			/*
			case UAVCAN_EQUIPMENT_SAFETY_ARMINGSTATUS_ID:
				{
				*out_data_type_signature = UAVCAN_EQUIPMENT_SAFETY_ARMINGSTATUS_SIGNATURE;
				return true;
				}
			*/
			default:
				{
				if (!IdDD_Broad) IdDD_Broad = data_type_id;
				break;
				}
			}
		}

  if (data_type_id == UAVCAN_EQUIPMENT_ACTUATOR_ARRAYCOMMAND_ID) {
    *out_data_type_signature = UAVCAN_EQUIPMENT_ACTUATOR_ARRAYCOMMAND_SIGNATURE;
    return true;
  }

  if (data_type_id == UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_ID) {
    *out_data_type_signature = UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_SIGNATURE;
    return true;
  }
  if (data_type_id == UAVCAN_PROTOCOL_PARAM_GETSET_ID) {
    *out_data_type_signature = UAVCAN_PROTOCOL_PARAM_GETSET_SIGNATURE;
    return true;
  }
  return false;

}

////////////////////////////////////////////////////////////////////////////////////


void getNodeInfoHandleCanard(CanardRxTransfer* transfer)
{

  uint8_t buffer[UAVCAN_GET_NODE_INFO_RESPONSE_MAX_SIZE];
  memset (buffer, 0, UAVCAN_GET_NODE_INFO_RESPONSE_MAX_SIZE);
  uint16_t len = makeNodeInfoMessage (buffer);
  int result = canardRequestOrRespond (&g_canard,
                                      transfer->source_node_id,
                                      UAVCAN_GET_NODE_INFO_DATA_TYPE_SIGNATURE,
                                      UAVCAN_GET_NODE_INFO_DATA_TYPE_ID,
                                      &transfer->transfer_id,
                                      transfer->priority,
                                      CanardResponse,
                                      &buffer[0],
                                      (uint16_t)len);

}


extern utimer_t TimerUAV_Trafic;


void SendCanard(void)
{

  const CanardCANFrame* txf = canardPeekTxQueue(&g_canard);
	//TimerCanWait = 5;		
  while (txf) {
    int tx_res = canardSTM32Transmit(txf);		
		if (!tx_res) break;
		//if (TimerCanWait) {};
		//TimerCanWait = 5;
    if (tx_res < 0) {                 // Failure - drop the frame and report
      __ASM volatile("BKPT #01");     // TODO: handle the error properly
    }
    if (tx_res > 0) {
      canardPopTxQueue(&g_canard);
			TimerUAV_Trafic = 1000;
    }
    txf = canardPeekTxQueue(&g_canard);
//		if (!tx_res) break;
  }

}




void makeNodeStatusMessage(uint8_t buffer[UAVCAN_NODE_STATUS_MESSAGE_SIZE])
{
  uint8_t node_health = TUAVCAN::GetHealtStatus();
  uint8_t node_mode   = TUAVCAN::GetNodeMode ();
  memset (buffer, 0, UAVCAN_NODE_STATUS_MESSAGE_SIZE);
  uint32_t uptime_sec = (HAL_GetTick() / 1000);
  canardEncodeScalar(buffer,  0, 32, &uptime_sec);
  canardEncodeScalar(buffer, 32,  2, &node_health);
  canardEncodeScalar(buffer, 34,  3, &node_mode);
}




void readUniqueID(uint8_t* out_uid)
{
// формируем UID мз серийного номера STM32
	unsigned char *lUidSTM = (unsigned char *)0x1FFFF7E8;
  for (uint8_t i = 0; i < UNIQUE_ID_LENGTH_BYTES; i++) {
    out_uid[i] = lUidSTM[i];
  }

}





uint16_t makeNodeInfoMessage(uint8_t buffer[UAVCAN_GET_NODE_INFO_RESPONSE_MAX_SIZE])
{

  memset(buffer, 0, UAVCAN_GET_NODE_INFO_RESPONSE_MAX_SIZE);
  makeNodeStatusMessage(buffer);

  buffer[7] = TUAVCAN::GetVersion_MAJOR ();//APP_VERSION_MAJOR;
  buffer[8] = TUAVCAN::GetVersion_Minor ();//APP_VERSION_MINOR;
  buffer[9] = 1;                            // Optional field flags, VCS commit is set
  uint32_t u32 = TUAVCAN::GetProgram_CRC(); //GIT_HASH;
  canardEncodeScalar(buffer, 80, 32, &u32);
	buffer[22] = 1;		// hard major
	buffer[23] = 5;		// hard minor
  readUniqueID(&buffer[24]);
  const size_t name_len = lenstr(APP_NODE_NAME);
  memcpy(&buffer[41], APP_NODE_NAME, name_len);
  return 41 + name_len ;

}


/*
void rawcmdHandleCanard(CanardRxTransfer* transfer)
{

  int offset = 0;
  for (int i = 0; i < 6; i++) {
    if (canardDecodeScalar(transfer, offset, 14, true, &rc_pwm[i]) < 14) {
      break;
    }
    offset += 14;
  }
  // rcpwmUpdate(ar);

}
*/


void showRcpwmonUart()
{

  //char str[5];
  //itoa(rc_pwm[0],str,10);
  //HAL_UART_Transmit(&huart1,str,5,0xffff);
  //HAL_UART_Transmit(&huart1,"\n",2,0xffff);
  //printArray(rc_pwm);

}


/*
param_t * getParamByIndex(uint16_t index)
{

  if (index >= ARRAY_SIZE(parameters))  {
    return NULL;
  }
  return &parameters[index];

}
*/


/*
param_t * getParamByName(uint8_t * name)
{

  for (uint16_t i = 0; i < ARRAY_SIZE(parameters); i++)
  {
    if (strncmp((char const*)name, (char const*)parameters[i].name, strlen((char const*)parameters[i].name)) == 0) {
      return &parameters[i];
    }
  }
  return NULL;

}
*/

// 0 - empty, 1 - integer,  2 - REAL + 38, 3 - bool
uint16_t encodeParamCanard(param_t * p, uint8_t * buffer, uint8_t tag)
{

  uint8_t n     = 0, sz_bit = 64;
  uint32_t offset    = 0;
	if (tag == EUAVDIGTYPE_REAL) sz_bit = 32;
  //uint8_t tag   = 1;	// 0 - empty, 1 - integer,  2 - REAL + 38, 3 - bool

  if (p == NULL) {
    tag = 0;
    canardEnSclr(buffer, offset, 5, &n);
    canardEnSclr(buffer, offset, 3, &tag);

    canardEnSclr(buffer, offset, 6, &n);
    canardEnSclr(buffer, offset, 2, &tag);

    canardEnSclr(buffer, offset, 6, &n);
    canardEnSclr(buffer, offset, 2, &tag);
    buffer[offset / 8] = 0;
    return ( offset / 8 + 1 );

  }

  canardEnSclr(buffer, offset, 5, &n);
  canardEnSclr(buffer, offset, 3, &tag);
  canardEnSclr(buffer, offset, sz_bit, &p->val);

  canardEnSclr(buffer, offset, 5, &n);
  canardEnSclr(buffer, offset, 3, &tag);
  canardEnSclr(buffer, offset, sz_bit, &p->defval);

  canardEnSclr(buffer, offset, 5, &n);
  canardEnSclr(buffer, offset, 3, &tag);
  canardEnSclr(buffer, offset, sz_bit, &p->max);

  canardEnSclr(buffer, offset, 5, &n);
  canardEnSclr(buffer, offset, 3, &tag);
  canardEnSclr(buffer, offset, sz_bit, &p->min);

  memcpy(&buffer[offset / 8], p->name, lenstr((char const*)p->name));
  return  (offset / 8 + lenstr((char const*)p->name));
}



void getsetHandleCanard (CanardRxTransfer* transfer)
{

  uint16_t index = 0xFFFF;
  uint8_t tag    = 0;
  int offset     = 0;
  int64_t val    = 0;
	float val_f    = 0;
	param_t param_local;

  canardDecodeScalar(transfer, offset,  13, false, &index);
  offset += 13;
  canardDecodeScalar(transfer, offset, 3, false, &tag);
  offset += 3;

  if (tag == EUAVDIGTYPE_INT) {
    canardDecodeScalar(transfer, offset, 64, false, &val);
    offset += 64;
  }
	
  if (tag == EUAVDIGTYPE_REAL) {
    canardDecodeScalar (transfer, offset, 32, true, &val_f);
    offset += 32;
  }
	
  uint16_t n = transfer->payload_len - offset / 8 ;
  uint8_t name[16]      = "";
  for (int i = 0; i < n; i++) {
    canardDecodeScalar(transfer, offset, 8, false, &name[i]);
    offset += 8;
  }

  param_t * p = NULL;

	S_HDR_TAG *lp_src = 0;
	
  if (lenstr((char const*)name)) {
		lp_src = TUAVPARAMS::GetParamTag ((char*)name, &index); 
  } else {
		lp_src = TUAVPARAMS::GetParamTag (index);
  }
	uint8_t type_dig = 0;
	memset (&param_local, 0, sizeof(param_local));
	
	if (index != 0xFFFF && lp_src) {
		if (TUAVPARAMS::GetTag (&param_local, (ESAVEPARIX)index))
			{
			p = &param_local;
			type_dig = lp_src->type;
			}
		}
	
  if (p) {
		if (index != 0xFFFF)
			{
			switch (tag)
				{
				case EUAVDIGTYPE_REAL:
					{
					TUAVPARAMS::SetTagValue_f32 (lp_src, val_f, (ESAVEPARIX)index);
					CopyMemorySDC ((char*)&val_f, (char*)&p->val, sizeof(val_f));
					break;
					}
				case EUAVDIGTYPE_INT:
					{
					TUAVPARAMS::SetTagValue_i64 (lp_src, val, (ESAVEPARIX)index);
					p->val = val;
					break;
					}
				}
			}
  }
	
  uint8_t  buffer[64] = "";
  uint16_t len = encodeParamCanard(p, buffer, type_dig);
  int result = canardRequestOrRespond(&g_canard,
                                      transfer->source_node_id,
                                      UAVCAN_PROTOCOL_PARAM_GETSET_SIGNATURE,
                                      UAVCAN_PROTOCOL_PARAM_GETSET_ID,
                                      &transfer->transfer_id,
                                      transfer->priority,
                                      CanardResponse,
                                      &buffer[0],
                                      (uint16_t)len);

}




void SendLogMsg(uint8_t* text, const uint size)
{
  static uint32_t publish_time = 0;

  if (HAL_GetTick() < publish_time + PUBLISHER_PERIOD_mS) {
    return; // rate limiting
  }
  publish_time = HAL_GetTick();

  static uint8_t transfer_id = 0;

  uavcan_protocol_debug_LogMessage log;

  uint8_t nID[] = "44";
  log.source.len = 2;
  log.source.data = nID;

  log.text.len = size;
  log.text.data = text;

  uint8_t buffer[UAVCAN_PROTOCOL_DEBUG_LOGMESSAGE_MAX_SIZE];
  memset(buffer, 0x00, UAVCAN_PROTOCOL_DEBUG_LOGMESSAGE_MAX_SIZE);

  uint offset = 0;
  offset = uavcan_protocol_debug_LogMessage_encode_internal(&log, &buffer[0], offset, 1);

  canardBroadcast(&g_canard, UAVCAN_PROTOCOL_DEBUG_LOGMESSAGE_SIGNATURE, UAVCAN_PROTOCOL_DEBUG_LOGMESSAGE_ID, &transfer_id, CANARD_TRANSFER_PRIORITY_LOW, &buffer[0], (offset + 7) / 8);
}



void TestAirData()
{
  static uint32_t publish_time = 0;

  if (HAL_GetTick() < publish_time + PUBLISHER_PERIOD_mS)
  {
    return; // rate limiting
  }
  publish_time = HAL_GetTick();

  static uint8_t transfer_id = 0;

  uint8_t buffer[UAVCAN_EQUIPMENT_AIRDATA_TRUEAIRSPEED_SIZE];
  memset(buffer, 0x00, UAVCAN_EQUIPMENT_AIRDATA_TRUEAIRSPEED_SIZE);

  float valFloat;
  uint32_t offset = 0;

  valFloat = 2.5; // true air speed
  canardEnSclr(buffer, offset, 16, &valFloat);
  //offset += 16;

  valFloat = 6.25; // true air speed variance
  canardEnSclr(buffer, offset, 16, &valFloat);
  //offset += 16;

  canardBroadcast(&g_canard,
                  UAVCAN_EQUIPMENT_AIRDATA_TRUEAIRSPEED_SIGNATURE,
                  UAVCAN_EQUIPMENT_AIRDATA_TRUEAIRSPEED_ID,
                  &transfer_id,
                  CANARD_TRANSFER_PRIORITY_LOW,
                  &buffer[0],
                  UAVCAN_EQUIPMENT_AIRDATA_TRUEAIRSPEED_SIZE);


}
