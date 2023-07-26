/*
 * UAVCAN data structure definition for libcanard.
 *
 * Autogenerated, do not edit.
 *
 * Source file: E:\DSDL\libcanard\dsdl_compiler\pyuavcan\uavcan\dsdl_files\uavcan\equipment\ahrs\1000.Solution.uavcan
 */

#ifndef __UAVCAN_EQUIPMENT_AHRS_SOLUTION
#define __UAVCAN_EQUIPMENT_AHRS_SOLUTION

#include <stdint.h>
#include "../../canard.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../Timestamp.h"

/******************************* Source text **********************************
#
# Inertial data and orientation in body frame.
#

uavcan.Timestamp timestamp

#
# Normalized quaternion
#
float16[4] orientation_xyzw
void4
float16[<=9] orientation_covariance

#
# rad/sec
#
float16[3] angular_velocity
void4
float16[<=9] angular_velocity_covariance

#
# m/s^2
#
float16[3] linear_acceleration
float16[<=9] linear_acceleration_covariance
******************************************************************************/

/********************* DSDL signature source definition ***********************
uavcan.equipment.ahrs.Solution
uavcan.Timestamp timestamp
saturated float16[4] orientation_xyzw
void4
saturated float16[<=9] orientation_covariance
saturated float16[3] angular_velocity
void4
saturated float16[<=9] angular_velocity_covariance
saturated float16[3] linear_acceleration
saturated float16[<=9] linear_acceleration_covariance
******************************************************************************/

#define UAVCAN_EQUIPMENT_AHRS_SOLUTION_ID                  1000
#define UAVCAN_EQUIPMENT_AHRS_SOLUTION_NAME                "uavcan.equipment.ahrs.Solution"
#define UAVCAN_EQUIPMENT_AHRS_SOLUTION_SIGNATURE           (0x72A63A3C6F41FA9BULL)

#define UAVCAN_EQUIPMENT_AHRS_SOLUTION_MAX_SIZE            ((668 + 7)/8)

// Constants

#define UAVCAN_EQUIPMENT_AHRS_SOLUTION_ORIENTATION_XYZW_LENGTH                           4
#define UAVCAN_EQUIPMENT_AHRS_SOLUTION_ORIENTATION_COVARIANCE_MAX_LENGTH                 9
#define UAVCAN_EQUIPMENT_AHRS_SOLUTION_ANGULAR_VELOCITY_LENGTH                           3
#define UAVCAN_EQUIPMENT_AHRS_SOLUTION_ANGULAR_VELOCITY_COVARIANCE_MAX_LENGTH            9
#define UAVCAN_EQUIPMENT_AHRS_SOLUTION_LINEAR_ACCELERATION_LENGTH                        3
#define UAVCAN_EQUIPMENT_AHRS_SOLUTION_LINEAR_ACCELERATION_COVARIANCE_MAX_LENGTH         9

typedef struct
{
    // FieldTypes
    uavcan_Timestamp timestamp;                     //
    float      orientation_xyzw[4];           // Static Array 16bit[4] max items
    // void4
    struct
    {
        uint8_t    len;                       // Dynamic array length
        float*     data;                      // Dynamic Array 16bit[9] max items
    } orientation_covariance;
    float      angular_velocity[3];           // Static Array 16bit[3] max items
    // void4
    struct
    {
        uint8_t    len;                       // Dynamic array length
        float*     data;                      // Dynamic Array 16bit[9] max items
    } angular_velocity_covariance;
    float      linear_acceleration[3];        // Static Array 16bit[3] max items
    struct
    {
        uint8_t    len;                       // Dynamic array length
        float*     data;                      // Dynamic Array 16bit[9] max items
    } linear_acceleration_covariance;

} uavcan_equipment_ahrs_Solution;

extern
uint32_t uavcan_equipment_ahrs_Solution_encode(uavcan_equipment_ahrs_Solution* source, void* msg_buf);

extern
int32_t uavcan_equipment_ahrs_Solution_decode(const CanardRxTransfer* transfer, uint16_t payload_len, uavcan_equipment_ahrs_Solution* dest, uint8_t** dyn_arr_buf);

extern
uint32_t uavcan_equipment_ahrs_Solution_encode_internal(uavcan_equipment_ahrs_Solution* source, void* msg_buf, uint32_t offset, uint8_t root_item);

extern
int32_t uavcan_equipment_ahrs_Solution_decode_internal(const CanardRxTransfer* transfer, uint16_t payload_len, uavcan_equipment_ahrs_Solution* dest, uint8_t** dyn_arr_buf, int32_t offset);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // __UAVCAN_EQUIPMENT_AHRS_SOLUTION
