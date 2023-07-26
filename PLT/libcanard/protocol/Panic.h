/*
 * UAVCAN data structure definition for libcanard.
 *
 * Autogenerated, do not edit.
 *
 * Source file: E:\DSDL\libcanard\dsdl_compiler\pyuavcan\uavcan\dsdl_files\uavcan\protocol\5.Panic.uavcan
 */

#ifndef __UAVCAN_PROTOCOL_PANIC
#define __UAVCAN_PROTOCOL_PANIC

#include <stdint.h>
#include "../canard.h"

#ifdef __cplusplus
extern "C"
{
#endif

/******************************* Source text **********************************
#
# This message may be published periodically to inform network participants that the system has encountered
# an unrecoverable fault and is not capable of further operation.
#
# Nodes that are expected to react to this message should wait for at least MIN_MESSAGES subsequent messages
# with any reason text from any sender published with the interval no higher than MAX_INTERVAL_MS before
# undertaking any emergency actions.
#

uint8 MIN_MESSAGES = 3

uint16 MAX_INTERVAL_MS = 500

#
# Short description that would fit a single CAN frame.
#
uint8[<=7] reason_text
******************************************************************************/

/********************* DSDL signature source definition ***********************
uavcan.protocol.Panic
saturated uint8[<=7] reason_text
******************************************************************************/

#define UAVCAN_PROTOCOL_PANIC_ID                           5
#define UAVCAN_PROTOCOL_PANIC_NAME                         "uavcan.protocol.Panic"
#define UAVCAN_PROTOCOL_PANIC_SIGNATURE                    (0x8B79B4101811C1D7ULL)

#define UAVCAN_PROTOCOL_PANIC_MAX_SIZE                     ((59 + 7)/8)

// Constants
#define UAVCAN_PROTOCOL_PANIC_MIN_MESSAGES                                    3 // 3
#define UAVCAN_PROTOCOL_PANIC_MAX_INTERVAL_MS                               500 // 500

#define UAVCAN_PROTOCOL_PANIC_REASON_TEXT_MAX_LENGTH                                     7

typedef struct
{
    // FieldTypes
    struct
    {
        uint8_t    len;                       // Dynamic array length
        uint8_t*   data;                      // Dynamic Array 8bit[7] max items
    } reason_text;

} uavcan_protocol_Panic;

extern
uint32_t uavcan_protocol_Panic_encode(uavcan_protocol_Panic* source, void* msg_buf);

extern
int32_t uavcan_protocol_Panic_decode(const CanardRxTransfer* transfer, uint16_t payload_len, uavcan_protocol_Panic* dest, uint8_t** dyn_arr_buf);

extern
uint32_t uavcan_protocol_Panic_encode_internal(uavcan_protocol_Panic* source, void* msg_buf, uint32_t offset, uint8_t root_item);

extern
int32_t uavcan_protocol_Panic_decode_internal(const CanardRxTransfer* transfer, uint16_t payload_len, uavcan_protocol_Panic* dest, uint8_t** dyn_arr_buf, int32_t offset);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // __UAVCAN_PROTOCOL_PANIC
