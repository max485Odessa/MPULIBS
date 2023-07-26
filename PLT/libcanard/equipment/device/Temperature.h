/*
 * UAVCAN data structure definition for libcanard.
 *
 * Autogenerated, do not edit.
 *
 * Source file: E:\DSDL\libcanard\dsdl_compiler\pyuavcan\uavcan\dsdl_files\uavcan\equipment\device\1110.Temperature.uavcan
 */

#ifndef __UAVCAN_EQUIPMENT_DEVICE_TEMPERATURE
#define __UAVCAN_EQUIPMENT_DEVICE_TEMPERATURE

#include <stdint.h>
#include "../../canard.h"

#ifdef __cplusplus
extern "C"
{
#endif

/******************************* Source text **********************************
#
# Generic device temperature
#

uint16 device_id

float16 temperature                  # in kelvin

uint8 ERROR_FLAG_OVERHEATING = 1
uint8 ERROR_FLAG_OVERCOOLING = 2
uint8 error_flags
******************************************************************************/

/********************* DSDL signature source definition ***********************
uavcan.equipment.device.Temperature
saturated uint16 device_id
saturated float16 temperature
saturated uint8 error_flags
******************************************************************************/

#define UAVCAN_EQUIPMENT_DEVICE_TEMPERATURE_ID             1110
#define UAVCAN_EQUIPMENT_DEVICE_TEMPERATURE_NAME           "uavcan.equipment.device.Temperature"
#define UAVCAN_EQUIPMENT_DEVICE_TEMPERATURE_SIGNATURE      (0x70261C28A94144C6ULL)

#define UAVCAN_EQUIPMENT_DEVICE_TEMPERATURE_MAX_SIZE       ((40 + 7)/8)

// Constants
#define UAVCAN_EQUIPMENT_DEVICE_TEMPERATURE_ERROR_FLAG_OVERHEATING            1 // 1
#define UAVCAN_EQUIPMENT_DEVICE_TEMPERATURE_ERROR_FLAG_OVERCOOLING            2 // 2

typedef struct
{
    // FieldTypes
    uint16_t   device_id;                     // bit len 16
    float      temperature;                   // float16 Saturate
    uint8_t    error_flags;                   // bit len 8

} uavcan_equipment_device_Temperature;

extern
uint32_t uavcan_equipment_device_Temperature_encode(uavcan_equipment_device_Temperature* source, void* msg_buf);

extern
int32_t uavcan_equipment_device_Temperature_decode(const CanardRxTransfer* transfer, uint16_t payload_len, uavcan_equipment_device_Temperature* dest, uint8_t** dyn_arr_buf);

extern
uint32_t uavcan_equipment_device_Temperature_encode_internal(uavcan_equipment_device_Temperature* source, void* msg_buf, uint32_t offset, uint8_t root_item);

extern
int32_t uavcan_equipment_device_Temperature_decode_internal(const CanardRxTransfer* transfer, uint16_t payload_len, uavcan_equipment_device_Temperature* dest, uint8_t** dyn_arr_buf, int32_t offset);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // __UAVCAN_EQUIPMENT_DEVICE_TEMPERATURE
