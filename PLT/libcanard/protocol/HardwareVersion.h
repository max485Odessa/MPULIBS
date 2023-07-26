/*
 * UAVCAN data structure definition for libcanard.
 *
 * Autogenerated, do not edit.
 *
 * Source file: E:\DSDL\libcanard\dsdl_compiler\pyuavcan\uavcan\dsdl_files\uavcan\protocol\HardwareVersion.uavcan
 */

#ifndef __UAVCAN_PROTOCOL_HARDWAREVERSION
#define __UAVCAN_PROTOCOL_HARDWAREVERSION

#include <stdint.h>
#include "../canard.h"

#ifdef __cplusplus
extern "C"
{
#endif

/******************************* Source text **********************************
#
# Nested type.
# Generic hardware version information.
# These values should remain unchanged for the device's lifetime.
#

#
# Hardware version code.
#
uint8 major
uint8 minor

#
# Unique ID is a 128 bit long sequence that is globally unique for each node.
# All zeros is not a valid UID.
# If filled with zeros, assume that the value is undefined.
#
uint8[16] unique_id

#
# Certificate of authenticity (COA) of the hardware, 255 bytes max.
#
uint8[<=255] certificate_of_authenticity
******************************************************************************/

/********************* DSDL signature source definition ***********************
uavcan.protocol.HardwareVersion
saturated uint8 major
saturated uint8 minor
saturated uint8[16] unique_id
saturated uint8[<=255] certificate_of_authenticity
******************************************************************************/

#define UAVCAN_PROTOCOL_HARDWAREVERSION_NAME               "uavcan.protocol.HardwareVersion"
#define UAVCAN_PROTOCOL_HARDWAREVERSION_SIGNATURE          (0xAD5C4C933F4A0C4ULL)

#define UAVCAN_PROTOCOL_HARDWAREVERSION_MAX_SIZE           ((2192 + 7)/8)

// Constants

#define UAVCAN_PROTOCOL_HARDWAREVERSION_UNIQUE_ID_LENGTH                                 16
#define UAVCAN_PROTOCOL_HARDWAREVERSION_CERTIFICATE_OF_AUTHENTICITY_MAX_LENGTH           255

typedef struct
{
    // FieldTypes
    uint8_t    major;                         // bit len 8
    uint8_t    minor;                         // bit len 8
    uint8_t    unique_id[16];                 // Static Array 8bit[16] max items
    struct
    {
        uint8_t    len;                       // Dynamic array length
        uint8_t*   data;                      // Dynamic Array 8bit[255] max items
    } certificate_of_authenticity;

} uavcan_protocol_HardwareVersion;

extern
uint32_t uavcan_protocol_HardwareVersion_encode(uavcan_protocol_HardwareVersion* source, void* msg_buf);

extern
int32_t uavcan_protocol_HardwareVersion_decode(const CanardRxTransfer* transfer, uint16_t payload_len, uavcan_protocol_HardwareVersion* dest, uint8_t** dyn_arr_buf);

extern
uint32_t uavcan_protocol_HardwareVersion_encode_internal(uavcan_protocol_HardwareVersion* source, void* msg_buf, uint32_t offset, uint8_t root_item);

extern
int32_t uavcan_protocol_HardwareVersion_decode_internal(const CanardRxTransfer* transfer, uint16_t payload_len, uavcan_protocol_HardwareVersion* dest, uint8_t** dyn_arr_buf, int32_t offset);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // __UAVCAN_PROTOCOL_HARDWAREVERSION
