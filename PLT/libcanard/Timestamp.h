/*
 * UAVCAN data structure definition for libcanard.
 *
 * Autogenerated, do not edit.
 *
 * Source file: E:\DSDL\libcanard\dsdl_compiler\pyuavcan\uavcan\dsdl_files\uavcan\Timestamp.uavcan
 */

#ifndef __UAVCAN_TIMESTAMP
#define __UAVCAN_TIMESTAMP

#include <stdint.h>
#include "canard.h"

#ifdef __cplusplus
extern "C"
{
#endif

/******************************* Source text **********************************
#
# Global timestamp in microseconds, 7 bytes.
#
# Use this data type for timestamp fields in messages, like follows:
#   uavcan.Timestamp timestamp
#

uint56 UNKNOWN = 0
truncated uint56 usec     # Microseconds
******************************************************************************/

/********************* DSDL signature source definition ***********************
uavcan.Timestamp
truncated uint56 usec
******************************************************************************/

#define UAVCAN_TIMESTAMP_NAME                              "uavcan.Timestamp"
#define UAVCAN_TIMESTAMP_SIGNATURE                         (0x5BD0B5C81087E0DULL)

#define UAVCAN_TIMESTAMP_MAX_SIZE                          ((56 + 7)/8)

// Constants
#define UAVCAN_TIMESTAMP_UNKNOWN                                              0 // 0

typedef struct
{
    // FieldTypes
    uint64_t   usec;                          // bit len 56				(416999 ���� )

} uavcan_Timestamp;

extern
uint32_t uavcan_Timestamp_encode(uavcan_Timestamp* source, void* msg_buf);

extern
int32_t uavcan_Timestamp_decode(const CanardRxTransfer* transfer, uint16_t payload_len, uavcan_Timestamp* dest, uint8_t** dyn_arr_buf);

extern
uint32_t uavcan_Timestamp_encode_internal(uavcan_Timestamp* source, void* msg_buf, uint32_t offset, uint8_t root_item);

extern
int32_t uavcan_Timestamp_decode_internal(const CanardRxTransfer* transfer, uint16_t payload_len, uavcan_Timestamp* dest, uint8_t** dyn_arr_buf, int32_t offset);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // __UAVCAN_TIMESTAMP
