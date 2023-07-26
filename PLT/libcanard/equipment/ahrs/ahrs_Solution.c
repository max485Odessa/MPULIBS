/*
 * UAVCAN data structure definition for libcanard.
 *
 * Autogenerated, do not edit.
 *
 * Source file: E:\DSDL\libcanard\dsdl_compiler\pyuavcan\uavcan\dsdl_files\uavcan\equipment\ahrs\1000.Solution.uavcan
 */
#include "Solution.h"
#include "../../canard.h"

#ifndef CANARD_INTERNAL_SATURATE
#define CANARD_INTERNAL_SATURATE(x, max) ( ((x) > max) ? max : ( (-(x) > max) ? (-max) : (x) ) );
#endif

#ifndef CANARD_INTERNAL_SATURATE_UNSIGNED
#define CANARD_INTERNAL_SATURATE_UNSIGNED(x, max) ( ((x) >= max) ? max : (x) );
#endif

#if defined(__GNUC__)
# define CANARD_MAYBE_UNUSED(x) x __attribute__((unused))
#else
# define CANARD_MAYBE_UNUSED(x) x
#endif

/**
  * @brief uavcan_equipment_ahrs_Solution_encode_internal
  * @param source : pointer to source data struct
  * @param msg_buf: pointer to msg storage
  * @param offset: bit offset to msg storage
  * @param root_item: for detecting if TAO should be used
  * @retval returns new offset
  */
uint32_t uavcan_equipment_ahrs_Solution_encode_internal(uavcan_equipment_ahrs_Solution* source,
  void* msg_buf,
  uint32_t offset,
  uint8_t CANARD_MAYBE_UNUSED(root_item))
{
    uint32_t c = 0;

    // Compound
    offset = uavcan_Timestamp_encode_internal(&source->timestamp, msg_buf, offset, 0);
    // Static array (orientation_xyzw)
    for (c = 0; c < 4; c++)
    {
        canardEncodeScalar(msg_buf, offset, 16, (void*)(source->orientation_xyzw + c)); // 32767
        offset += 16;
    }

    // Void4
    offset += 4;

    // Dynamic Array (orientation_covariance)
    // - Add array length
    canardEncodeScalar(msg_buf, offset, 4, (void*)&source->orientation_covariance.len);
    offset += 4;

    // - Add array items
    for (c = 0; c < source->orientation_covariance.len; c++)
    {
        canardEncodeScalar(msg_buf,
                           offset,
                           16,
                           (void*)(source->orientation_covariance.data + c));// 32767
        offset += 16;
    }

    // Static array (angular_velocity)
    for (c = 0; c < 3; c++)
    {
        canardEncodeScalar(msg_buf, offset, 16, (void*)(source->angular_velocity + c)); // 32767
        offset += 16;
    }

    // Void4
    offset += 4;

    // Dynamic Array (angular_velocity_covariance)
    // - Add array length
    canardEncodeScalar(msg_buf, offset, 4, (void*)&source->angular_velocity_covariance.len);
    offset += 4;

    // - Add array items
    for (c = 0; c < source->angular_velocity_covariance.len; c++)
    {
        canardEncodeScalar(msg_buf,
                           offset,
                           16,
                           (void*)(source->angular_velocity_covariance.data + c));// 32767
        offset += 16;
    }

    // Static array (linear_acceleration)
    for (c = 0; c < 3; c++)
    {
        canardEncodeScalar(msg_buf, offset, 16, (void*)(source->linear_acceleration + c)); // 32767
        offset += 16;
    }

    // Dynamic Array (linear_acceleration_covariance)
    if (! root_item)
    {
        // - Add array length
        canardEncodeScalar(msg_buf, offset, 4, (void*)&source->linear_acceleration_covariance.len);
        offset += 4;
    }

    // - Add array items
    for (c = 0; c < source->linear_acceleration_covariance.len; c++)
    {
        canardEncodeScalar(msg_buf,
                           offset,
                           16,
                           (void*)(source->linear_acceleration_covariance.data + c));// 32767
        offset += 16;
    }

    return offset;
}

/**
  * @brief uavcan_equipment_ahrs_Solution_encode
  * @param source : Pointer to source data struct
  * @param msg_buf: Pointer to msg storage
  * @retval returns message length as bytes
  */
uint32_t uavcan_equipment_ahrs_Solution_encode(uavcan_equipment_ahrs_Solution* source, void* msg_buf)
{
    uint32_t offset = 0;

    offset = uavcan_equipment_ahrs_Solution_encode_internal(source, msg_buf, offset, 1);

    return (offset + 7 ) / 8;
}

/**
  * @brief uavcan_equipment_ahrs_Solution_decode_internal
  * @param transfer: Pointer to CanardRxTransfer transfer
  * @param payload_len: Payload message length
  * @param dest: Pointer to destination struct
  * @param dyn_arr_buf: NULL or Pointer to memory storage to be used for dynamic arrays
  *                     uavcan_equipment_ahrs_Solution dyn memory will point to dyn_arr_buf memory.
  *                     NULL will ignore dynamic arrays decoding.
  * @param offset: Call with 0, bit offset to msg storage
  * @retval new offset or ERROR value if < 0
  */
int32_t uavcan_equipment_ahrs_Solution_decode_internal(
  const CanardRxTransfer* transfer,
  uint16_t CANARD_MAYBE_UNUSED(payload_len),
  uavcan_equipment_ahrs_Solution* dest,
  uint8_t** CANARD_MAYBE_UNUSED(dyn_arr_buf),
  int32_t offset)
{
    int32_t ret = 0;
    uint32_t c = 0;

    // Compound
    offset = uavcan_Timestamp_decode_internal(transfer, payload_len, &dest->timestamp, dyn_arr_buf, offset);
    if (offset < 0)
    {
        ret = offset;
        goto uavcan_equipment_ahrs_Solution_error_exit;
    }

    // Static array (orientation_xyzw)
    for (c = 0; c < 4; c++)
    {
        ret = canardDecodeScalar(transfer, (uint32_t)offset, 16, false, (void*)(dest->orientation_xyzw + c));
        if (ret != 16)
        {
            goto uavcan_equipment_ahrs_Solution_error_exit;
        }
        offset += 16;
    }

    // Void4
    offset += 4;

    // Dynamic Array (orientation_covariance)
    //  - Array length, not last item 4 bits
    ret = canardDecodeScalar(transfer,
                             (uint32_t)offset,
                             4,
                             false,
                             (void*)&dest->orientation_covariance.len); // 32767
    if (ret != 4)
    {
        goto uavcan_equipment_ahrs_Solution_error_exit;
    }
    offset += 4;

    //  - Get Array
    if (dyn_arr_buf)
    {
        dest->orientation_covariance.data = (float*)*dyn_arr_buf;
    }

    for (c = 0; c < dest->orientation_covariance.len; c++)
    {
        if (dyn_arr_buf)
        {
            ret = canardDecodeScalar(transfer,
                                     (uint32_t)offset,
                                     16,
                                     false,
                                     (void*)*dyn_arr_buf); // 32767
            if (ret != 16)
            {
                goto uavcan_equipment_ahrs_Solution_error_exit;
            }
            *dyn_arr_buf = (uint8_t*)(((float*)*dyn_arr_buf) + 1);
        }
        offset += 16;
    }

    // Static array (angular_velocity)
    for (c = 0; c < 3; c++)
    {
        ret = canardDecodeScalar(transfer, (uint32_t)offset, 16, false, (void*)(dest->angular_velocity + c));
        if (ret != 16)
        {
            goto uavcan_equipment_ahrs_Solution_error_exit;
        }
        offset += 16;
    }

    // Void4
    offset += 4;

    // Dynamic Array (angular_velocity_covariance)
    //  - Array length, not last item 4 bits
    ret = canardDecodeScalar(transfer,
                             (uint32_t)offset,
                             4,
                             false,
                             (void*)&dest->angular_velocity_covariance.len); // 32767
    if (ret != 4)
    {
        goto uavcan_equipment_ahrs_Solution_error_exit;
    }
    offset += 4;

    //  - Get Array
    if (dyn_arr_buf)
    {
        dest->angular_velocity_covariance.data = (float*)*dyn_arr_buf;
    }

    for (c = 0; c < dest->angular_velocity_covariance.len; c++)
    {
        if (dyn_arr_buf)
        {
            ret = canardDecodeScalar(transfer,
                                     (uint32_t)offset,
                                     16,
                                     false,
                                     (void*)*dyn_arr_buf); // 32767
            if (ret != 16)
            {
                goto uavcan_equipment_ahrs_Solution_error_exit;
            }
            *dyn_arr_buf = (uint8_t*)(((float*)*dyn_arr_buf) + 1);
        }
        offset += 16;
    }

    // Static array (linear_acceleration)
    for (c = 0; c < 3; c++)
    {
        ret = canardDecodeScalar(transfer, (uint32_t)offset, 16, false, (void*)(dest->linear_acceleration + c));
        if (ret != 16)
        {
            goto uavcan_equipment_ahrs_Solution_error_exit;
        }
        offset += 16;
    }

    // Dynamic Array (linear_acceleration_covariance)
    //  - Last item in struct & Root item & (Array Size > 8 bit), tail array optimization
    if (payload_len)
    {
        //  - Calculate Array length from MSG length
        dest->linear_acceleration_covariance.len = ((payload_len * 8) - offset ) / 16; // 16 bit array item size
    }
    else
    {
        // - Array length 4 bits
        ret = canardDecodeScalar(transfer,
                                 (uint32_t)offset,
                                 4,
                                 false,
                                 (void*)&dest->linear_acceleration_covariance.len); // 32767
        if (ret != 4)
        {
            goto uavcan_equipment_ahrs_Solution_error_exit;
        }
        offset += 4;
    }

    //  - Get Array
    if (dyn_arr_buf)
    {
        dest->linear_acceleration_covariance.data = (float*)*dyn_arr_buf;
    }

    for (c = 0; c < dest->linear_acceleration_covariance.len; c++)
    {
        if (dyn_arr_buf)
        {
            ret = canardDecodeScalar(transfer,
                                     (uint32_t)offset,
                                     16,
                                     false,
                                     (void*)*dyn_arr_buf); // 32767
            if (ret != 16)
            {
                goto uavcan_equipment_ahrs_Solution_error_exit;
            }
            *dyn_arr_buf = (uint8_t*)(((float*)*dyn_arr_buf) + 1);
        }
        offset += 16;
    }
    return offset;

uavcan_equipment_ahrs_Solution_error_exit:
    if (ret < 0)
    {
        return ret;
    }
    else
    {
        return -CANARD_ERROR_INTERNAL;
    }
}

/**
  * @brief uavcan_equipment_ahrs_Solution_decode
  * @param transfer: Pointer to CanardRxTransfer transfer
  * @param payload_len: Payload message length
  * @param dest: Pointer to destination struct
  * @param dyn_arr_buf: NULL or Pointer to memory storage to be used for dynamic arrays
  *                     uavcan_equipment_ahrs_Solution dyn memory will point to dyn_arr_buf memory.
  *                     NULL will ignore dynamic arrays decoding.
  * @retval offset or ERROR value if < 0
  */
int32_t uavcan_equipment_ahrs_Solution_decode(const CanardRxTransfer* transfer,
  uint16_t payload_len,
  uavcan_equipment_ahrs_Solution* dest,
  uint8_t** dyn_arr_buf)
{
    const int32_t offset = 0;
    int32_t ret = 0;

    // Clear the destination struct
    for (uint32_t c = 0; c < sizeof(uavcan_equipment_ahrs_Solution); c++)
    {
        ((uint8_t*)dest)[c] = 0x00;
    }

    ret = uavcan_equipment_ahrs_Solution_decode_internal(transfer, payload_len, dest, dyn_arr_buf, offset);

    return ret;
}