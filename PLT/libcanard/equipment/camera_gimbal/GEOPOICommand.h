/*
 * UAVCAN data structure definition for libcanard.
 *
 * Autogenerated, do not edit.
 *
 * Source file: E:\DSDL\libcanard\dsdl_compiler\pyuavcan\uavcan\dsdl_files\uavcan\equipment\camera_gimbal\1041.GEOPOICommand.uavcan
 */

#ifndef __UAVCAN_EQUIPMENT_CAMERA_GIMBAL_GEOPOICOMMAND
#define __UAVCAN_EQUIPMENT_CAMERA_GIMBAL_GEOPOICOMMAND

#include <stdint.h>
#include "../../canard.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include "Mode.h"

/******************************* Source text **********************************
#
# Generic camera gimbal control.
#
# This message can only be used in the following modes:
#  - COMMAND_MODE_GEO_POI
#

uint8 gimbal_id

#
# Target operation mode - how to handle this message.
# See the list of acceptable modes above.
#
Mode mode

#
# Coordinates of the POI (point of interest).
#
int32 longitude_deg_1e7    # 1 LSB = 1e-7 deg
int32 latitude_deg_1e7
int22 height_cm            # 1 LSB = 10 mm

uint2 HEIGHT_REFERENCE_ELLIPSOID = 0
uint2 HEIGHT_REFERENCE_MEAN_SEA_LEVEL = 1
uint2 height_reference
******************************************************************************/

/********************* DSDL signature source definition ***********************
uavcan.equipment.camera_gimbal.GEOPOICommand
saturated uint8 gimbal_id
uavcan.equipment.camera_gimbal.Mode mode
saturated int32 longitude_deg_1e7
saturated int32 latitude_deg_1e7
saturated int22 height_cm
saturated uint2 height_reference
******************************************************************************/

#define UAVCAN_EQUIPMENT_CAMERA_GIMBAL_GEOPOICOMMAND_ID    1041
#define UAVCAN_EQUIPMENT_CAMERA_GIMBAL_GEOPOICOMMAND_NAME  "uavcan.equipment.camera_gimbal.GEOPOICommand"
#define UAVCAN_EQUIPMENT_CAMERA_GIMBAL_GEOPOICOMMAND_SIGNATURE (0x9371428A92F01FD6ULL)

#define UAVCAN_EQUIPMENT_CAMERA_GIMBAL_GEOPOICOMMAND_MAX_SIZE ((104 + 7)/8)

// Constants
#define UAVCAN_EQUIPMENT_CAMERA_GIMBAL_GEOPOICOMMAND_HEIGHT_REFERENCE_ELLIPSOID          0 // 0
#define UAVCAN_EQUIPMENT_CAMERA_GIMBAL_GEOPOICOMMAND_HEIGHT_REFERENCE_MEAN_SEA_LEVEL          1 // 1

typedef struct
{
    // FieldTypes
    uint8_t    gimbal_id;                     // bit len 8
    uavcan_equipment_camera_gimbal_Mode mode;                          //
    int32_t    longitude_deg_1e7;             // bit len 32
    int32_t    latitude_deg_1e7;              // bit len 32
    int32_t    height_cm;                     // bit len 22
    uint8_t    height_reference;              // bit len 2

} uavcan_equipment_camera_gimbal_GEOPOICommand;

extern
uint32_t uavcan_equipment_camera_gimbal_GEOPOICommand_encode(uavcan_equipment_camera_gimbal_GEOPOICommand* source, void* msg_buf);

extern
int32_t uavcan_equipment_camera_gimbal_GEOPOICommand_decode(const CanardRxTransfer* transfer, uint16_t payload_len, uavcan_equipment_camera_gimbal_GEOPOICommand* dest, uint8_t** dyn_arr_buf);

extern
uint32_t uavcan_equipment_camera_gimbal_GEOPOICommand_encode_internal(uavcan_equipment_camera_gimbal_GEOPOICommand* source, void* msg_buf, uint32_t offset, uint8_t root_item);

extern
int32_t uavcan_equipment_camera_gimbal_GEOPOICommand_decode_internal(const CanardRxTransfer* transfer, uint16_t payload_len, uavcan_equipment_camera_gimbal_GEOPOICommand* dest, uint8_t** dyn_arr_buf, int32_t offset);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // __UAVCAN_EQUIPMENT_CAMERA_GIMBAL_GEOPOICOMMAND
