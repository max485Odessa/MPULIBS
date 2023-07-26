/*
 * UAVCAN data structure definition for libcanard.
 *
 * Autogenerated, do not edit.
 *
 * Source file: E:\DSDL\libcanard\dsdl_compiler\pyuavcan\uavcan\dsdl_files\uavcan\protocol\param\10.ExecuteOpcode.uavcan
 */

#ifndef __UAVCAN_PROTOCOL_PARAM_EXECUTEOPCODE
#define __UAVCAN_PROTOCOL_PARAM_EXECUTEOPCODE

#include <stdint.h>
#include "../../canard.h"

#ifdef __cplusplus
extern "C"
{
#endif

/******************************* Source text **********************************
#
# Service to control the node configuration.
#

#
# SAVE operation instructs the remote node to save the current configuration parameters into a non-volatile
# storage. The node may require a restart in order for some changes to take effect.
#
# ERASE operation instructs the remote node to clear its configuration storage and reinitialize the parameters
# with their default values. The node may require a restart in order for some changes to take effect.
#
# Other opcodes may be added in the future (for example, an opcode for switching between multiple configurations).
#
uint8 OPCODE_SAVE  = 0  # Save all parameters to non-volatile storage.
uint8 OPCODE_ERASE = 1  # Clear the non-volatile storage; some changes may take effect only after reboot.
uint8 opcode

#
# Reserved, keep zero.
#
int48 argument

---

#
# If 'ok' (the field below) is true, this value is not used and must be kept zero.
# If 'ok' is false, this value may contain error code. Error code constants may be defined in the future.
#
int48 argument

#
# True if the operation has been performed successfully, false otherwise.
#
bool ok
******************************************************************************/

/********************* DSDL signature source definition ***********************
uavcan.protocol.param.ExecuteOpcode
saturated uint8 opcode
saturated int48 argument
---
saturated int48 argument
saturated bool ok
******************************************************************************/

#define UAVCAN_PROTOCOL_PARAM_EXECUTEOPCODE_ID             10
#define UAVCAN_PROTOCOL_PARAM_EXECUTEOPCODE_NAME           "uavcan.protocol.param.ExecuteOpcode"
#define UAVCAN_PROTOCOL_PARAM_EXECUTEOPCODE_SIGNATURE      (0x3B131AC5EB69D2CDULL)

#define UAVCAN_PROTOCOL_PARAM_EXECUTEOPCODE_REQUEST_MAX_SIZE ((56 + 7)/8)

// Constants
#define UAVCAN_PROTOCOL_PARAM_EXECUTEOPCODE_REQUEST_OPCODE_SAVE               0 // 0
#define UAVCAN_PROTOCOL_PARAM_EXECUTEOPCODE_REQUEST_OPCODE_ERASE              1 // 1

typedef struct
{
    // FieldTypes
    uint8_t    opcode;                        // bit len 8
    int64_t    argument;                      // bit len 48

} uavcan_protocol_param_ExecuteOpcodeRequest;

extern
uint32_t uavcan_protocol_param_ExecuteOpcodeRequest_encode(uavcan_protocol_param_ExecuteOpcodeRequest* source, void* msg_buf);

extern
int32_t uavcan_protocol_param_ExecuteOpcodeRequest_decode(const CanardRxTransfer* transfer, uint16_t payload_len, uavcan_protocol_param_ExecuteOpcodeRequest* dest, uint8_t** dyn_arr_buf);

extern
uint32_t uavcan_protocol_param_ExecuteOpcodeRequest_encode_internal(uavcan_protocol_param_ExecuteOpcodeRequest* source, void* msg_buf, uint32_t offset, uint8_t root_item);

extern
int32_t uavcan_protocol_param_ExecuteOpcodeRequest_decode_internal(const CanardRxTransfer* transfer, uint16_t payload_len, uavcan_protocol_param_ExecuteOpcodeRequest* dest, uint8_t** dyn_arr_buf, int32_t offset);

#define UAVCAN_PROTOCOL_PARAM_EXECUTEOPCODE_RESPONSE_MAX_SIZE ((49 + 7)/8)

// Constants

typedef struct
{
    // FieldTypes
    int64_t    argument;                      // bit len 48
    bool       ok;                            // bit len 1

} uavcan_protocol_param_ExecuteOpcodeResponse;

extern
uint32_t uavcan_protocol_param_ExecuteOpcodeResponse_encode(uavcan_protocol_param_ExecuteOpcodeResponse* source, void* msg_buf);

extern
int32_t uavcan_protocol_param_ExecuteOpcodeResponse_decode(const CanardRxTransfer* transfer, uint16_t payload_len, uavcan_protocol_param_ExecuteOpcodeResponse* dest, uint8_t** dyn_arr_buf);

extern
uint32_t uavcan_protocol_param_ExecuteOpcodeResponse_encode_internal(uavcan_protocol_param_ExecuteOpcodeResponse* source, void* msg_buf, uint32_t offset, uint8_t root_item);

extern
int32_t uavcan_protocol_param_ExecuteOpcodeResponse_decode_internal(const CanardRxTransfer* transfer, uint16_t payload_len, uavcan_protocol_param_ExecuteOpcodeResponse* dest, uint8_t** dyn_arr_buf, int32_t offset);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // __UAVCAN_PROTOCOL_PARAM_EXECUTEOPCODE
