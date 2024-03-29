/*
 * UAVCAN data structure definition for libcanard.
 *
 * Autogenerated, do not edit.
 *
 * Source file: E:\DSDL\libcanard\dsdl_compiler\pyuavcan\uavcan\dsdl_files\uavcan\equipment\hardpoint\1070.Command.uavcan
 */

#ifndef __UAVCAN_EQUIPMENT_HARDPOINT_COMMAND
#define __UAVCAN_EQUIPMENT_HARDPOINT_COMMAND

#include <stdint.h>
#include "../../canard.h"

#ifdef __cplusplus
extern "C"
{
#endif

/******************************* Source text **********************************
#
# Generic cargo holder/hardpoint command.
#

uint8 hardpoint_id

#
# Either a binary command (0 - release, 1+ - hold) or bitmask
#
uint16 command
******************************************************************************/

/********************* DSDL signature source definition ***********************
uavcan.equipment.hardpoint.Command
saturated uint8 hardpoint_id
saturated uint16 command
******************************************************************************/

#define UAVCAN_EQUIPMENT_HARDPOINT_COMMAND_ID              1070
#define UAVCAN_EQUIPMENT_HARDPOINT_COMMAND_NAME            "uavcan.equipment.hardpoint.Command"
#define UAVCAN_EQUIPMENT_HARDPOINT_COMMAND_SIGNATURE       (0xA1A036268B0C3455ULL)

#define UAVCAN_EQUIPMENT_HARDPOINT_COMMAND_MAX_SIZE        ((24 + 7)/8)

// Constants

typedef struct
{
    // FieldTypes
    uint8_t    hardpoint_id;                  // bit len 8
    uint16_t   command;                       // bit len 16

} uavcan_equipment_hardpoint_Command;

extern
uint32_t uavcan_equipment_hardpoint_Command_encode(uavcan_equipment_hardpoint_Command* source, void* msg_buf);

extern
int32_t uavcan_equipment_hardpoint_Command_decode(const CanardRxTransfer* transfer, uint16_t payload_len, uavcan_equipment_hardpoint_Command* dest, uint8_t** dyn_arr_buf);

extern
uint32_t uavcan_equipment_hardpoint_Command_encode_internal(uavcan_equipment_hardpoint_Command* source, void* msg_buf, uint32_t offset, uint8_t root_item);

extern
int32_t uavcan_equipment_hardpoint_Command_decode_internal(const CanardRxTransfer* transfer, uint16_t payload_len, uavcan_equipment_hardpoint_Command* dest, uint8_t** dyn_arr_buf, int32_t offset);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // __UAVCAN_EQUIPMENT_HARDPOINT_COMMAND
