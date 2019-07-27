/****************************************************************************
*
*    Copyright (c) 2005 - 2019 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include <VX/vx.h>
#ifdef WIN32
#include <dirent_win.h>
#else
#include <dirent.h>
#endif

#include <gc_vx_common.h>
#include <gc_vx_nn_util.h>
#include "gc_nn_arch_model.h"
#ifdef USE_LIB_NN_ARCH_PERF
#include "nnArchPerf.h"
#endif

#define AXI_BURST_SIZE 64

#define VX_GET_DATA_FROM_TENSOR64(tensor, index) \
    vxnneGetDataExt64((vx_type_e)TENSOR_DATA_TYPE(tensor), TENSOR_QUANT_TYPE(tensor), index, TENSOR_LOGICAL_ADDR(tensor), TENSOR_POS(tensor), TENSOR_TF_ZEROPOINT(tensor), TENSOR_TF_SCALE(tensor))

#define VX_SAVE_DATA_TO_TENSOR(tensor, data, index) \
    vxnneSaveDataExt((vx_type_e)TENSOR_DATA_TYPE(tensor), TENSOR_QUANT_TYPE(tensor), index, data, TENSOR_LOGICAL_ADDR(tensor), TENSOR_POS(tensor), TENSOR_TF_ZEROPOINT(tensor), TENSOR_TF_SCALE(tensor), TENSOR_ROUNDING_MODE(tensor))


#if NN_LAYER_C
extern vx_status vx_nnk_pool_nn_layer_cpu(vx_uint8_ptr src, vx_int32 type, vx_type_e format, vx_int32 input_width, vx_int32 input_height, vx_int32 depth,
                                vx_int32_ptr output_width, vx_int32_ptr output_height, vx_int32 stride, vx_int32 kernel_size, vx_int32 pad, vx_uint8_ptr dst);
#endif

vx_float32 Fp16toFp32(const vx_uint16 in)
{
    vx_uint32 t1;
    vx_uint32 t2;
    vx_uint32 t3;
    vx_float32 out;

    t1 = in & 0x7fff;                       // Non-sign bits
    t2 = in & 0x8000;                       // Sign bit
    t3 = in & 0x7c00;                       // Exponent

    t1 <<= 13;                              // Align mantissa on MSB
    t2 <<= 16;                              // Shift sign bit into position

    t1 += 0x38000000;                       // Adjust bias

    t1 = (t3 == 0 ? 0 : t1);                // Denormals-as-zero

    t1 |= t2;                               // Re-insert sign bit

    *((uint32_t*)&out) = t1;

    return out;
}

vx_int16 Fp32toFp16(vx_float32 val)
{
    vx_uint32 f32 = (*(vx_uint32 *) &val);
    vx_int16 f16 = 0;
    /* Decode IEEE 754 little-endian 32-bit floating-point value */
    int sign = (f32 >> 16) & 0x8000;
    /* Map exponent to the range [-127,128] */
    int exponent = ((f32 >> 23) & 0xff) - 127;
    int mantissa = f32 & 0x007fffff;
    if (exponent == 128)
    { /* Infinity or NaN */
        if (mantissa)
        {
            /* Flush NaN to 0. */
            f16 = (vx_int16)sign;
        }
        else
        {
            /* Clamp to HALF_MAX/HALF_MIN. */
            f16 = (vx_int16)(sign | ((F16_EXPONENT_BITS - 1) << F16_EXPONENT_SHIFT) | F16_MANTISSA_BITS);
        }
    }
    else if (exponent > 15)
    { /* Overflow - clamp to HALF_MAX/HALF_MIN. */
        f16 = (vx_int16)(sign | ((F16_EXPONENT_BITS - 1) << F16_EXPONENT_SHIFT) | F16_MANTISSA_BITS);
    }
    else if (exponent > -15)
    { /* Representable value */
        /* RTNE */
        int roundingBit = (mantissa >> (F16_MANTISSA_SHIFT - 1)) & 0x1;
        int stickyBits = mantissa & 0xFFF;
        exponent += F16_EXPONENT_BIAS;
        mantissa >>= F16_MANTISSA_SHIFT;
        if (roundingBit)
        {
            if (stickyBits || (mantissa & 0x1))
            {
                mantissa++;
                if (mantissa > F16_MANTISSA_BITS)
                {
                    exponent++;
                    if (exponent > 30)
                    {
                        /* Clamp to HALF_MAX/HALF_MIN. */
                        exponent--;
                        mantissa--;
                    }
                    else
                    {
                        mantissa &= F16_MANTISSA_BITS;
                    }
                }
            }
        }
        f16 = (vx_int16)(sign | exponent << F16_EXPONENT_SHIFT | mantissa);
    }
    else
    {
        f16 = (vx_int16)sign;
    }
    return f16;
}

/* this function is only for fullyconnect acceleration*/
vx_int8 Fp32toInt8_fc(vx_float32 val)
{
    /* Data is already converted to 8bit from IDE by default.
     * Or cast to int8 directly. TODO: correct it later.
     */
    return !(*(vx_int32*)&val >> 8) || (*(vx_int32*)&val >> 8) == ~0 ? *(vx_int8*)&val : (vx_int8)val;
}

#if defined(__linux__)
vx_float64 _copysign(vx_float64 number, vx_float64 sign)
{
    vx_float64 value = gcmABS(number);
    return (sign > 0) ? value : (-value);
}
#endif

VX_INTERNAL_API void calculateSplitSize(
    vx_uint32                    whole_size,
    vx_uint32                    split_num,
    vx_uint32 *                  split_size_array,
    vx_uint32 *                  split_offset_array
    )
{
    split_num = gcmMIN(whole_size, split_num);

    if (split_num <= 1)
    {
        if (split_size_array != VX_NULL)
            split_size_array[0] = whole_size;
        if (split_offset_array != VX_NULL)
            split_offset_array[0] = 0;
    }
    else
    {
        vx_uint32 i;
        vx_uint32 quot = whole_size / split_num;
        vx_uint32 remain = whole_size % split_num;

        for (i = 0; i < split_num; i++)
        {
            if (split_size_array != VX_NULL)
            {
                split_size_array[i] = i < remain ? quot + 1 : quot;
                if (split_offset_array != VX_NULL)
                    split_offset_array[i] = i < remain ? i * split_size_array[i] : remain * (quot + 1) + (i - remain) * split_size_array[i];
            }
        }
    }
}

void vxnneBatch_SetCurrentBatchArray(struct _vxnne_operation_s *operation, vx_uint32 batchIndex)
{
    operation->currBatchIndex = batchIndex;
}

void vxnneMultiChannel_SetCurrentChannel(vxnne_operation_target_e operationTarget)
{
    if ((operationTarget > VXNNE_OPERATION_TARGET_NONE) && (operationTarget < VXNNE_OPERATION_TARGET_SW))
    {
        gcoHAL_SelectChannel(gcvNULL, gcvTRUE, (gctUINT32)operationTarget);
    }
    else
    {
        gcoHAL_SelectChannel(gcvNULL, gcvTRUE, (gctUINT32)VXNNE_OPERATION_TARGET_SH);
    }
}

void vxnneMultiChannel_GetCurrentChannel(vxnne_operation_target_e *operationTarget)
{
    *operationTarget = VXNNE_OPERATION_TARGET_SH;
}


void vxnneMultiChannel_ApplySyncMode(vxnne_sync_mode_e mode, gctUINT32 semaHandle)
{
    switch(mode)
    {
        case VXNNE_SYNC_MODE_HW_WAKE:
            gcoHAL_MCFESemaphore(semaHandle, gcvTRUE);
            break;
        case VXNNE_SYNC_MODE_HW_WAIT:
            gcoHAL_MCFESemaphore(semaHandle, gcvFALSE);
            break;
        case VXNNE_SYNC_MODE_SW_WAIT:
            gcoVX_Flush(gcvTRUE);
            break;
        case VXNNE_SYNC_MODE_SW_WAKE:
            break;
        default: break;
    }
}

vx_float32 roundSimpleRounding(vx_float32 x)
{
#if defined(_M_X64)
    return (vx_float32) _copysignf(floorf(fabsf(x) + 0.5f), x);
#else
    return (vx_float32) _copysign(floorf(fabsf(x) + 0.5f), x);
#endif
}

vx_float64 roundRTNE(vx_float64 x)
{
#define EPSILON 1e-8

    vx_float64 decimal;
    vx_float64 inter;

    decimal = modf((vx_float64)x, &inter);

    if (gcmABS((gcmABS(decimal) - 0.5f)) < EPSILON)
    {
        inter += (vx_int32)(inter) % 2;
    }
    else
    {
        return roundSimpleRounding((vx_float32)x);
    }

    return inter;
}

vx_float32 roundRTZ(vx_float32 x)
{
    return (vx_float32)_copysign(floorf(fabsf(x)), x);
}

vx_float32 roundRTNI(vx_float32 x)
{
    if (x > 0.0f)
    {
        return (vx_float32)_copysign(floorf(fabsf(x)), x);
    }
    else
    {
        return (vx_float32)_copysign(ceilf(fabsf(x)), x);
    }
}

vx_float32 vxnneRound(vx_float32 x, vx_enum roundMode)
{
    switch (roundMode)
    {
    case  VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING:
        return (vx_float32) roundSimpleRounding(x);

    case VX_NN_ROUNDING_MODE_RTNE:
    case VX_ROUND_POLICY_TO_NEAREST_EVEN:
        return (vx_float32) roundRTNE(x);

    case VX_NN_ROUNDING_MODE_RTZ:
    case VX_ROUND_POLICY_TO_ZERO:
        return roundRTZ(x);

    case VX_NN_ROUNDING_MODE_RTNI:
        return roundRTNI(x);
    default:
        return (vx_float32) roundSimpleRounding(x);
    }
}


vx_int8 Fp32toInt8(vx_float32 val, vx_int8 fixedPointPos, vx_int32 roundMode)
{
    vx_int8 result = 0;

    if (fixedPointPos > 0)
    {
        vx_int32 data = (vx_int32) vxnneRound(val * (vx_float32)(1 << fixedPointPos), roundMode);
        result = (vx_int8)((data > 127) ? 127 : (data < -128) ? -128 : data);

    }
    else
    {
        vx_int32 data = (vx_int32) vxnneRound(val * (1.0f / (vx_float32)(1 << -fixedPointPos)), roundMode);
        result = (vx_int8)((data > 127) ? 127 : (data < -128) ? -128 : data);
    }

    return result;
}

vx_float32 Int8toFp32(vx_int8 val, vx_int8 fixedPointPos)
{
    return vxnneConvertDynamicFixPointValueToFloat32((vx_float32)val, fixedPointPos);
}

vx_float32 UchartoFp32(vx_uint8 val, vx_int8 fixedPointPos)
{
    vx_float32 result = 0.0f;

    if (fixedPointPos >= 0)
    {
        result = (vx_float32)val * (1.0f / ((vx_uint32) (1 << fixedPointPos)));
    }
    else
    {
        result = (vx_float32)val * ((vx_float32) (1 << -fixedPointPos));
    }

    return result;
}

vx_float32 Uint8toFp32(vx_uint8 val, vx_int32 zeroPoint, vx_float32 scale)
{
    vx_float32 result = 0.0f;

    result = (val - (vx_uint8)zeroPoint) * scale;

    return result;
}

vx_int8 Fp32toUchar(vx_float32 val, vx_uint8 fixedPointPos, vx_int32 roundMode)
{
    vx_uint8 result = 0;

    if (fixedPointPos >= 0)
    {
        vx_int32 data = (vx_int32) vxnneRound(val * (vx_float32)(1 << fixedPointPos), roundMode);
        result = (vx_uint8)((data > 255) ? 255 : (data < 0) ? 0 : data);
    }
    else
    {
        vx_int32 data = (vx_int32) vxnneRound(val * (1.0f / (vx_float32)(1 << -fixedPointPos)), roundMode);
        result = (vx_uint8)((data > 255) ? 255 : (data < 0) ? 0 : data);
    }

    return result;
}

vx_uint8 Fp32toUint8(vx_float32 val, vx_int32 zeroPoint, vx_float32 scale, vx_int32 roundMode)
{
    vx_uint8 result = 0;
    vx_int32 data;

    data = (vx_int32) vxnneRound((val / scale + (vx_uint8)zeroPoint), roundMode);

    if (data > 255)
        data = 255;

    if (data < 0)
        data = 0;

    result = (vx_uint8)(data);

    return result;
}

vx_int16 Fp32toInt16(vx_float32 val, vx_int8 fixedPointPos, vx_int32 roundMode)
{
    vx_int16 result = 0;

    if (fixedPointPos > 0)
    {
        vx_int32 data = (vx_int32) vxnneRound(val * (vx_float32)(1 << fixedPointPos), roundMode);
        result = (vx_int16)((data > 32767) ? 32767 : (data < -32768) ? -32768 : data);

    }
    else
    {
        vx_int32 data = (vx_int32) vxnneRound(val * (1.0f / (vx_float32)(1 << -fixedPointPos)), roundMode);
        result = (vx_int16)((data > 32767) ? 32767 : (data < -32768) ? -32768 : data);
    }

    return result;
}

vx_float32 vxnneConvertDynamicFixPointValueToFloat32(vx_float32 value, vx_int8 fixedPointPos)
{
    vx_float32 result = 0.0f;

    if (fixedPointPos > 0)
    {
        result = value * (1.0f / ((vx_uint32) (1 << fixedPointPos)));
    }
    else
    {
        result = value * ((vx_float32) (1 << -fixedPointPos));
    }

    return result;
}

vx_float32 Int16toFp32(vx_int16 val, vx_int8 fixedPointPos)
{
    return vxnneConvertDynamicFixPointValueToFloat32((vx_float32)val, fixedPointPos);
}

vx_float32 Int32toFp32(vx_int32 val, vx_int8 fixedPointPos)
{
    return vxnneConvertDynamicFixPointValueToFloat32((vx_float32)val, fixedPointPos);
}

vx_float32 Int64toFp32(vx_int64 val, vx_int8 fixedPointPos)
{
    vx_float32 result = 0.0f;

    if (fixedPointPos > 0)
    {
        result = val * (1.0f / ((vx_uint32) (1 << fixedPointPos)));
    }
    else
    {
        result = val * ((vx_float32) (1 << -fixedPointPos));
    }

    return result;
}


vx_float32 Int32toFp32Quant(vx_int32 val, vx_int32 zeroPoint, vx_float32 scale)
{
    vx_float32 result = 0.0f;

    result = (val - zeroPoint) * scale;

    return result;
}

vx_uint32 vxnneAlignWithStride(vx_uint32 inputSize, vx_uint32 stride)
{
    if (stride == 1)
    {
        return inputSize;
    }
    else
    {
        if ((inputSize % stride) > 0)
        {
            return inputSize + stride - inputSize  % stride;
        }
        else
        {
            return inputSize;
        }
    }
}

void getFP32M0AndN(vx_float32 mult, vx_uint16 *M0, vx_int8 *N)
{
    vx_uint32 uintMult          = *((vx_uint32*)(&mult));
    vx_uint32 tmpMultiply       = 0;
    vx_int32  exp               = 0;
    vx_uint32 postShiftBit6to5  = 0;
    vx_uint32 postShift         = 0;
    vx_int8   tmpPostShift      = 0;

    tmpMultiply         = (uintMult & 0x7FFFFF) >> 8;
    *M0                 = (vx_uint16)((1U << 15) + tmpMultiply);

    exp                 = (uintMult & 0x7F800000) >> 23; /* postShift is Scale's exp*/
    tmpPostShift        = 15 - ((vx_int8)exp - 127);
    postShift           = tmpPostShift & 0x1F;
    tmpPostShift        = tmpPostShift >> 5;
    postShiftBit6to5    = tmpPostShift & 3;

    *N = (vx_int8)(((postShiftBit6to5 << 5) | (postShift & 0x1F)));
    *N = (((vx_int32)*N << 25) >> 25);
}

void calculateActivationRangeFloat16(vx_int32 activation, vx_int16* act_min, vx_int16* act_max)
{
    const vx_int16 qmin = 0xFC00;
    const vx_int16 qmax = 0x7C00;

    if (activation == VX_NN_ACTIVATION_RELU)
    {
        *act_min = Fp32toFp16(0.0);
        *act_max = qmax;
    }
    else if (activation == VX_NN_ACTIVATION_RELU6) {
        *act_min = Fp32toFp16(0.0);
        *act_max = Fp32toFp16(6.0);
    } else if (activation == VX_NN_ACTIVATION_RELU1)
    {
        *act_min = Fp32toFp16(-1.0);
        *act_max = Fp32toFp16(1.0);
    }
    else
    {
        *act_min = qmin;
        *act_max = qmax;
    }
}

void calculateActivationRangeInt16(vx_int32 activation, vx_int8 fixedPointPos, vx_int16* act_min, vx_int16* act_max, vx_int32 roundMode)
{
    const vx_int16 qmin = 0x8000;
    const vx_int16 qmax = 0x7FFF;

    if (activation == VX_NN_ACTIVATION_RELU)
    {
        *act_min = gcmMAX(qmin, Fp32toInt16(0.0, fixedPointPos, roundMode));
        *act_max = qmax;
    }
    else if (activation == VX_NN_ACTIVATION_RELU6) {
        *act_min = gcmMAX(qmin, Fp32toInt16(0.0, fixedPointPos, roundMode));
        *act_max = gcmMIN(qmax, Fp32toInt16(6.0, fixedPointPos, roundMode));
    } else if (activation == VX_NN_ACTIVATION_RELU1)
    {
        *act_min = gcmMAX(qmin, Fp32toInt16(-1.0, fixedPointPos, roundMode));
        *act_max = gcmMIN(qmax, Fp32toInt16(1.0, fixedPointPos, roundMode));
    }
    else
    {
        *act_min = qmin;
        *act_max = qmax;
    }
}

void calculateActivationRangeInt8(vx_int32 activation, vx_int8 fixedPointPos, vx_int8* act_min, vx_int8* act_max, vx_int32 roundMode)
{
    const vx_int8 qmin = 0x80;
    const vx_int8 qmax = 0x7F;

    if (activation == VX_NN_ACTIVATION_RELU)
    {
        *act_min = gcmMAX(qmin, Fp32toInt8(0.0, fixedPointPos, roundMode));
        *act_max = qmax;
    }
    else if (activation == VX_NN_ACTIVATION_RELU6) {
        *act_min = gcmMAX(qmin, Fp32toInt8(0.0, fixedPointPos, roundMode));
        *act_max = gcmMIN(qmax, Fp32toInt8(6.0, fixedPointPos, roundMode));
    } else if (activation == VX_NN_ACTIVATION_RELU1)
    {
        *act_min = gcmMAX(qmin, Fp32toInt8(-1.0, fixedPointPos, roundMode));
        *act_max = gcmMIN(qmax, Fp32toInt8(1.0, fixedPointPos, roundMode));
    }
    else
    {
        *act_min = qmin;
        *act_max = qmax;
    }
}

void calculateActivationRangeUInt8(vx_int32 activation, vx_float32 scale, vx_int32 zero_point, vx_uint8* act_min, vx_uint8* act_max, vx_float32 range_min, vx_float32 range_max)
{
    const vx_uint8 qmin = 0;
    const vx_uint8 qmax = 255;

    if (activation == VX_NN_ACTIVATION_RELU)
    {
        *act_min = gcmMAX(qmin, Fp32toUint8(0.0, zero_point, scale, VX_ROUND_POLICY_TO_NEAREST_EVEN));
        *act_max = qmax;
    }
    else if (activation == VX_NN_ACTIVATION_RELU6)
    {
        *act_min = gcmMAX(qmin, Fp32toUint8(0.0, zero_point, scale, VX_ROUND_POLICY_TO_NEAREST_EVEN));
        *act_max = gcmMIN(qmax, Fp32toUint8(6.0, zero_point, scale, VX_ROUND_POLICY_TO_NEAREST_EVEN));
    }
    else if (activation == VX_NN_ACTIVATION_RELU1)
    {
        *act_min = gcmMAX(qmin, Fp32toUint8(-1.0, zero_point, scale, VX_ROUND_POLICY_TO_NEAREST_EVEN));
        *act_max = gcmMIN(qmax, Fp32toUint8(1.0, zero_point, scale, VX_ROUND_POLICY_TO_NEAREST_EVEN));
    }
    else if (activation == VX_NN_ACTIVATION_BRELU)
    {
        *act_min = gcmMAX(qmin, Fp32toUint8(0, zero_point, scale, VX_ROUND_POLICY_TO_NEAREST_EVEN));
        *act_max = gcmMIN(qmax, Fp32toUint8(range_max, zero_point, scale, VX_ROUND_POLICY_TO_NEAREST_EVEN));
    }
    else
    {
        *act_min = qmin;
        *act_max = qmax;
    }
}

vx_bool getFC_1x1xN_to_NN_kxxkyxkz(vx_uint32 input_size, vx_uint32 *kx, vx_uint32 *ky, vx_uint32 *kz)
{
    vx_uint32 kernel_size[][2] = { {3, 3}, {2, 2}, {4, 4}, {5, 5}, {6, 6}};
    vx_uint32 i = 0;

    for (i = 0; i < sizeof(kernel_size)/sizeof(kernel_size[0]); i ++)
    {
        if (input_size % (kernel_size[i][0] * kernel_size[i][1]) == 0)
        {
            *kx = kernel_size[i][0];
            *ky = kernel_size[i][1];
            *kz = input_size / (kernel_size[i][0] * kernel_size[i][1]);

            return vx_true_e;
        }
    }

    return vx_false_e;
}


vx_status checkGetDataFactor(vx_uint32 data, vx_uint32 *factor, vx_uint32 minLimit, vx_uint32 maxLimit, vx_uint32 alignData)
{
    vx_uint32 i         = 0;
    vx_uint32 maxFactor = alignData - 1;
    vx_status status    = VX_FAILURE;

    for (i = minLimit; i <= maxLimit; i ++)
    {
        if (data % i == 0)
        {
            if (status == VX_FAILURE && data % i == 0)
            {
                *factor      = i;
                maxFactor    = i;
                status       = VX_SUCCESS;
                continue;
            }
            else if ((i % alignData) < (maxFactor % alignData))
            {
               *factor      = i;
               maxFactor    = i;
               status       = VX_SUCCESS;
            }

        }
    }


    return status;
}

vx_bool checkOutputTensorDoAlu(vx_tensor src, vx_tensor dst)
{
    vx_enum    srcFormat             = TENSOR_DATA_TYPE(src);
    vx_enum    srcQFormat            = TENSOR_QUANT_TYPE(src);
    vx_enum    dstFormat             = TENSOR_DATA_TYPE(dst);
    vx_enum    dstQFormat            = TENSOR_QUANT_TYPE(dst);
    vx_context context               = vxoContext_GetFromReference((vx_reference)src);
    vx_bool    formatCheck           = vx_false_e;

    if(context->evisNoInst.supportEVIS)
    {
        formatCheck = (vx_bool)(srcFormat == VX_TYPE_FLOAT16);
    }
    else
    {
        formatCheck = (vx_bool)(srcFormat == VX_TYPE_FLOAT16 || srcFormat == VX_TYPE_FLOAT32);
    }

    if (srcFormat == dstFormat && formatCheck)
    {
        return vx_false_e;
    }
    else if (srcQFormat == dstQFormat && srcQFormat == VX_QUANT_AFFINE_SCALE && srcFormat == dstFormat && srcFormat == VX_TYPE_UINT8)
    {
        vx_float32 srcScale  = TENSOR_TF_SCALE(src);
        vx_float32 dstScale  = TENSOR_TF_SCALE(dst);
        vx_int32   srcZP     = TENSOR_TF_ZEROPOINT(src);
        vx_int32   dstZP     = TENSOR_TF_ZEROPOINT(dst);

        if (srcScale == dstScale && srcZP == dstZP)
        {
            return vx_false_e;
        }

        return vx_true_e;
    }
    else if (srcQFormat == dstQFormat && srcQFormat == VX_QUANT_DYNAMIC_FIXED_POINT && srcFormat == dstFormat)
    {
        vx_int8   srcFixPointPos     = TENSOR_POS(src);
        vx_int8   dstFixPointPos     = TENSOR_POS(dst);

        if (srcFixPointPos == dstFixPointPos)
        {
            return vx_false_e;
        }

        return vx_true_e;
    }

    return vx_true_e;
}

vx_int32 vxnneGetTypeSize(vx_type_e format)
{
    vx_int32 size = 0;
    switch(format)
    {
    case VX_TYPE_INT8:
    case VX_TYPE_UINT8:
        size = sizeof(vx_int8);
        break;
    case VX_TYPE_INT16:
    case VX_TYPE_FLOAT16:
        size = sizeof(vx_int16);
        break;
    case VX_TYPE_FLOAT32:
        size = sizeof(vx_float32);
        break;
    case VX_TYPE_INT32:
        size = sizeof(vx_int32);
        break;
    case VX_TYPE_INT64:
        size = sizeof(vx_int64);
        break;
    default:
        vxError("Not support format :%d\n", format);
        break;
    }

    return size;
}

vx_uint32 vxnneGetOneNumber(vx_uint32 value)
{
    vx_uint32 count = 0;
    vx_uint32 tempValue = value;

    while (tempValue)
    {
        count++;
        tempValue = tempValue & (tempValue - 1);
    }

    return count;
}

vx_float32 vxnneGetDataQuant(vx_type_e format, vx_int32 index, vx_uint8_ptr data, vx_int32 zeroPoint, vx_float32 scale)
{
    vx_float32 value = 0;

    if (index >= 0)
    {
        switch (format)
        {
        case VX_TYPE_UINT8:
            {
                vx_uint8_ptr data_ptr = (vx_uint8*)data;
                value = Uint8toFp32(data_ptr[index], zeroPoint, scale);
            }
            break;
        case VX_TYPE_INT32:
            {
                vx_int32_ptr data_ptr = (vx_int32*)data;
                value = Int32toFp32Quant(data_ptr[index], zeroPoint, scale);
            }
            break;
        default:
            vxError("Not support format :%d\n", format);
            break;
        }
    }

    return value;
}

vx_status vxnneSaveDataQuant(vx_type_e format, vx_int32 index, vx_float64 data, vx_ptr dst_data, vx_int32 zeroPoint, vx_float32 scale, vx_enum roundMode)
{

    switch(format)
    {
    case VX_TYPE_UINT8:
        {
            vx_uint8* dst_data_p = (vx_uint8*)dst_data;
            dst_data_p[index] = Fp32toUint8((vx_float32)data, zeroPoint, scale, roundMode);
        }
        break;
    default:
        vxError("Not support format :%d\n", format);
        return VX_ERROR_INVALID_FORMAT;
    }

    return VX_SUCCESS;
}

vx_float32 vxnneGetData(vx_type_e format, vx_int32 index, vx_uint8_ptr data, vx_int8 fixPointPos)
{
    vx_float32 value = 0;
    vx_int8 fpPos = fixPointPos;

    if (index >= 0)
    {
        switch(format)
        {
        case VX_TYPE_INT8:
            {
                vx_int8_ptr data_ptr = (vx_int8*)data;
                value = Int8toFp32(data_ptr[index], fpPos);
            }
            break;
        case VX_TYPE_UINT8:
            {
                vx_uint8_ptr data_ptr = (vx_uint8*)data;
                value = UchartoFp32(data_ptr[index], fpPos);
            }
            break;
        case VX_TYPE_INT16:
            {
                vx_int16_ptr data_ptr = (vx_int16*)data;
                value = Int16toFp32(data_ptr[index], fpPos);
            }
            break;
        case VX_TYPE_INT32:
            {
                vx_int32_ptr data_ptr = (vx_int32*)data;
                value = Int32toFp32(data_ptr[index], fpPos);
            }
            break;
        case VX_TYPE_UINT32:
            {
                vx_uint32_ptr data_ptr = (vx_uint32*)data;
                value = (vx_float32)data_ptr[index];
            }
        break;
        case VX_TYPE_INT64:
            {
                vx_int64_ptr data_ptr = (vx_int64*)data;
                value = Int64toFp32(data_ptr[index], fpPos);
            }
            break;
        case VX_TYPE_FLOAT16:
            {
                vx_int16_ptr data_ptr = (vx_int16_ptr)data;
                value = Fp16toFp32(data_ptr[index]);
            }
            break;
        case VX_TYPE_FLOAT32:
            {
                vx_float32_ptr data_ptr = (vx_float32_ptr)data;
                value = data_ptr[index];
            }
            break;
        default:
            vxError("Not support format :%d\n", format);
            break;
        }
    }
    return value;
}

vx_int32 vxnneGetDataInt(vx_type_e format, vx_int32 index, vx_uint8_ptr data, vx_int8 fixPointPos)
{
    vx_int32 value = 0;

    if (index >= 0)
    {
        switch(format)
        {
        case VX_TYPE_INT8:
            {
                vx_int8_ptr data_ptr = (vx_int8*)data;
                value = (vx_int32)data_ptr[index];
            }
            break;
        case VX_TYPE_UINT8:
            {
                vx_uint8_ptr data_ptr = (vx_uint8*)data;
                vx_uint8 data_u8 = data_ptr[index];
                value = 0;
                value |= data_u8;
            }
            break;
        case VX_TYPE_INT16:
            {
                vx_int16_ptr data_ptr = (vx_int16*)data;
                value = (vx_int32)data_ptr[index];
            }
            break;
        case VX_TYPE_INT32:
            {
                vx_int32_ptr data_ptr = (vx_int32*)data;
                value = data_ptr[index];
            }
            break;
        case VX_TYPE_UINT32:
            {
                vx_uint32_ptr data_ptr = (vx_uint32*)data;
                value = 0;
                value |= data_ptr[index];
            }
        break;
        default:
            vxError("Not support format :%d\n", format);
            break;
        }
    }
    return value;
}

vx_status vxnneSaveData(vx_type_e format, vx_int32 index, vx_float64 data, vx_ptr dst_data, vx_int8 fixedPointPos, vx_enum roundMode)
{
    vx_int8 fpPos = fixedPointPos;

    switch(format)
    {
    case VX_TYPE_INT8:
        {
            vx_int8* dst_data_p = (vx_int8*)dst_data;
            dst_data_p[index] = Fp32toInt8((vx_float32)data, fpPos, roundMode);
        }
        break;
    case VX_TYPE_UINT8:
        {
            vx_uint8* dst_data_p = (vx_uint8*)dst_data;
            dst_data_p[index] = Fp32toUchar((vx_float32)data, fpPos, roundMode);
        }
        break;
    case VX_TYPE_INT16:
        {
            vx_int16* dst_data_p = (vx_int16*)dst_data;
            dst_data_p[index] = Fp32toInt16((vx_float32)data, fpPos, roundMode);
        }
        break;
    case VX_TYPE_FLOAT16:
        {
            vx_int16* dst_data_p = (vx_int16*)dst_data;
            dst_data_p[index] = Fp32toFp16((vx_float32)data);
        }
        break;
    case VX_TYPE_INT32:
        {
            vx_int32_ptr dst_data_p = (vx_int32_ptr)dst_data;
            dst_data_p[index] = (vx_int32)data;
        }
        break;
    case VX_TYPE_FLOAT32:
        {
            vx_float32_ptr dst_data_p = (vx_float32_ptr)dst_data;
            dst_data_p[index] = (vx_float32)(data);
        }
        break;
    default:
        vxError("Not support format :%d\n", format);
        break;
    }

    return VX_SUCCESS;
}

vx_float32 vxnneGetDataExt(vx_type_e format, vx_enum quant_format, vx_int32 index, vx_uint8_ptr data, vx_int8 fixPointPos, vx_int32 zeroPoint, vx_float32 scale)
{
    if (quant_format == VX_QUANT_AFFINE_SCALE)
        return vxnneGetDataQuant(format, index, data, zeroPoint, scale);
    else
        return vxnneGetData(format, index, data, fixPointPos);
}

vx_status vxnneSaveDataExt(vx_type_e format, vx_enum quant_format, vx_int32 index, vx_float64 data, vx_ptr dst_data, vx_int8 fixedPointPos, vx_int32 zeroPoint, vx_float32 scale, vx_enum roundMode)
{
    if (format == VX_TYPE_UINT8 && quant_format == VX_QUANT_AFFINE_SCALE)
        return vxnneSaveDataQuant(format, index, data, dst_data, zeroPoint, scale, roundMode);
    else
        return vxnneSaveData(format, index, data, dst_data, fixedPointPos, roundMode);
}

vx_float64 vxnneGetDataExt64(vx_type_e format, vx_enum quant_format, vx_int32 index, vx_uint8_ptr data, vx_int8 fixPointPos, vx_int32 zeroPoint, vx_float32 scale)
{
    if (quant_format == VX_QUANT_AFFINE_SCALE)
        return vxnneGetDataQuant(format, index, data, zeroPoint, scale);
    else
        return vxnneGetData64(format, index, data, fixPointPos);
}

vx_int32 vxoNNExternsionConvlutionRound(vx_float32 in, vx_enum round_type)
{
    switch (round_type)
    {
    case VX_NN_DS_SIZE_ROUNDING_FLOOR:
        return (vx_int32)gcoMATH_Floor(in);

    case VX_NN_DS_SIZE_ROUNDING_CEILING:
        return (vx_int32)gcoMATH_Ceiling(in);

    default:
        vxError("nn extension not support this convolution rounding type %d!", round_type);
    }

    return (vx_int32)in;
}

/* reshuffle data with 4 stride */
void reshuffleData(vx_nn_reshuffle_s *src, vx_uint32 strideStepX, vx_uint32 strideStepY, vx_nn_reshuffle_s *dst)
{
    vx_uint32 x, y, z, w, i;
    vx_uint32 srcXStride, srcYStride, srcZStride;
    vx_uint32 dstXStride, dstYStride, dstZStride, dstWStride;

    srcXStride = (vx_uint32)vxDataType_GetSize(src->type);
    srcYStride = srcXStride * src->xSize;
    srcZStride = srcYStride * src->ySize;

    dstXStride = (vx_uint32)vxDataType_GetSize(src->type);
    dstYStride = dstXStride * dst->xSize;
    dstZStride = dstYStride * dst->ySize;
    dstWStride = dstZStride * dst->zSize;

    for (w = 0; w < src->wSize; w++)
    {
        for (z = 0; z < src->zSize; z++)
        {
            for (y = 0; y < src->ySize; y++)
            {
                for (x = 0; x < src->xSize; x++)
                {
                    uint8_t* dst_data;
                    uint8_t* src_data;
                    vx_uint32 dstX, dstY, dstZ, dstW;
                    src_data = (uint8_t*)src->data + w * dstWStride + z * srcZStride + y * srcYStride + x * srcXStride;
                    dstX = x/strideStepX;
                    dstY = y/strideStepY;
                    dstZ = x%strideStepX + (y%strideStepY)*strideStepX + (z%src->zSize) * strideStepX * strideStepY;
                    dstW = w;
                    dst_data = (uint8_t*)dst->data + dstX * dstXStride + dstY * dstYStride + dstZ * dstZStride + dstW * dstWStride;
                    *dst_data = *src_data;
                    for (i = 0; i < dstXStride; i++)
                    {
                        dst_data[i] = src_data[i];
                    }
                }
            }
         }
    }
}

void initUndefinedHardwareConfig(vx_context context)
{
#ifdef USE_LIB_NN_ARCH_PERF
    char *useLibNNArchPerf = VX_NULL;
#endif
#define USC_CACHE_SIZE                         8
#define CACHED_DATA_READ_FROM_SRAM             1
#define DDR_READ_BANDWIDTH_LIMIT               3.8f
#define DDR_WRITE_BANDWIDTH_LIMIT              3.8f
#define DDR_TOTAL_BANDWIDTH_LIMIT              3.8f
#define AXI_SRAM_READ_BANDWIDTH_LIMIT          16.0f
#define AXI_SRAM_WRITE_BANDWIDTH_LIMIT         16.0f
#define AXI_SRAM_TOTAL_BANDWIDTH_LIMIT         16.0f
#define AXI_BUS_READ_BANDWIDTH_LIMIT           16.0f
#define AXI_BUS_WRITE_BANDWIDTH_LIMIT          16.0f
#define AXI_BUS_TOTAL_BANDWIDTH_LIMIT          32.0f
#define DDR_LATENCY                            0
#define ACCURATE_TILE_BW                       1
#define AXI_SRAM_SLOWED_DOWN_BY_DDR            1
#define FREQ_IN_MHZ                            1000
#define AXI_CLK_FREQ_IN_MHZ                    1000
#define LANES_PER_CORE                         64
#define MAX_TILE_XSIZE                         64
#define MAX_SOC_OUT_STANDING_NUMBER            32
#define NN_WRITE_WITHOUT_USC                   0

    context->nnConfig.unifiedFeature.coefDeltaCordOverFlowZRL8BitFix = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_COEF_DELTA_CORD_OVERFLOW_ZRL_8BIT_FIX) ? 1 : 0;
    context->nnConfig.unifiedFeature.imageNotPackedInSram = !gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_IMAGE_NOT_PACKED_IN_SRAM_FIX) ? 1 : 0;
    context->nnConfig.unifiedFeature.nnUSCCacheSize = USC_CACHE_SIZE;
    context->nnConfig.unifiedFeature.nnCmdSizeInBytes = NNE_COMMAND_SIZE;
    context->nnConfig.unifiedFeature.tpCmdSizeInBytes = TP_COMMAND_SIZE;
    context->nnConfig.unifiedFeature.singlePortAccBuffer = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_SINGLEPORT_ACCUMBUFFER) ? 1 : 0;
    if (context->nnConfig.derivedFeature.nnDPAmount == 0)
    {
        /*for V8 HW: FEATURE_XYDP0 = 1, set XYDPX=XYDPY=0*/
        if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP0))
        {
            context->nnConfig.derivedFeature.nnXYDPX =  0;
            context->nnConfig.derivedFeature.nnXYDPY =  0;
            context->nnConfig.derivedFeature.nnDPAmount = 3;
        }
        else if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP9))
        {
            context->nnConfig.derivedFeature.nnXYDPX =  3;
            context->nnConfig.derivedFeature.nnXYDPY =  3;
            context->nnConfig.derivedFeature.nnDPAmount = 9;
        }
        else if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP6))
        {
            context->nnConfig.derivedFeature.nnXYDPX =  3;
            context->nnConfig.derivedFeature.nnXYDPY =  2;
            context->nnConfig.derivedFeature.nnDPAmount = 6;
        }
        else
        {
            if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
            {
                context->nnConfig.derivedFeature.nnDPAmount = 3;
                context->nnConfig.derivedFeature.nnXYDPX =  3;
                context->nnConfig.derivedFeature.nnXYDPY =  1;
            }
            else {
                context->nnConfig.derivedFeature.nnDPAmount = 1;
                context->nnConfig.derivedFeature.nnXYDPX =  1;
                context->nnConfig.derivedFeature.nnXYDPY =  1;
            }
        }

        context->nnConfig.derivedFeature.nnZDP =
            vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3) ? 3
            : vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6) ? 6
            : 1;
    }

    context->nnConfig.unifiedFeature.smallBatchEnable = (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_SMALLBATCH) && gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_TP_SMALLBATCH)) ? 1 : 0;
    context->nnConfig.unifiedFeature.convOutFifoDepthFix = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_CONVOUT_FIFO_DEPTH_FIX) ? 1 : 0;
    context->nnConfig.unifiedFeature.vipCoefDecodePerf = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_COEF_DECOMPRESS_PERF2X) ? 2 : 1;
    context->nnConfig.unifiedFeature.vipCachedReadFromSram = CACHED_DATA_READ_FROM_SRAM;
    context->nnConfig.unifiedFeature.vipImagePartialCache = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATUER_IMAGE_PARTIAL_CACHE) ? 1 : 0;
    context->nnConfig.unifiedFeature.fullCacheKernelHeadFix = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_FULLCACHE_KERNELHEAD_FIX) ? 1 : 0;
    context->nnConfig.unifiedFeature.conv1x1HalfPerformance = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_CONV1x1_PERF_FIX) ? 0 : 1;
    context->nnConfig.unifiedFeature.cacheLineModeDisabled = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_CACHELINE_MODE_PERF_FIX) ? 0 : 1;
    context->nnConfig.unifiedFeature.per3DTileBubbleFix = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PER3DTILE_BUBBLE_FIX) ? 1 : 0;
    context->nnConfig.unifiedFeature.kernelPerCoreLTOneThirdCoefFix = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_KERNEL_PER_CORE_LESS_THAN_THIRD_COEF_BUFF_DEPTH_FIX) ? 1 : 0;
    context->nnConfig.unifiedFeature.lowEfficiencyOfIDWriteImgBufFix = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_LOW_EFFICIENCY_OF_ID_WRITE_IMGBUF_FIX) ? 1 : 0;
    context->nnConfig.unifiedFeature.tpReOrderFix = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_TP_REORDER_FIX) ? 1 : 0;
    context->nnConfig.unifiedFeature.zdp3NoCompressFix = ((vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3) || vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6))
                                                          && gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_ZDP3_NO_COMPRESS_FIX)) ? 1 : 0;
    context->nnConfig.unifiedFeature.asyncCopyPerfFix = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_ASYNC_COPY_PERF_FIX) ? 1 : 0;
    context->nnConfig.unifiedFeature.zxdp3KernelReadConflictFix = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_ZXDP3_KERNEL_READ_CONFLICT_FIX) ? 1 : 0;
    context->nnConfig.unifiedFeature.xyOffsetLimitationFix = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_XY_OFFSET_LIMITATION_FIX) ? 1 : 0;
    context->nnConfig.unifiedFeature.accurateTileBW = ACCURATE_TILE_BW;
    context->nnConfig.unifiedFeature.lanesPerConv = LANES_PER_CORE;
    context->nnConfig.unifiedFeature.maxTileSize = MAX_TILE_XSIZE;
    context->nnConfig.unifiedFeature.axiSramSlowedDownByAddr = AXI_SRAM_SLOWED_DOWN_BY_DDR;
    context->nnConfig.unifiedFeature.slowNNReqArbitrationFix = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_REQ_SLOWARBITRATION_FIX) ? 1 : 0;

    context->nnConfig.customizedFeature.vipSWTiling = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_SWTILING_PHASE1) ? 1 : 0;
    context->nnConfig.unifiedFeature.axiSramOnlySWTiling = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_SWTILING_PHASE3) ? 0 :
                                                           vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_SWTILING_PHASE1) ? 1 : 0;

    if (context->nnConfig.customizedFeature.ddrReadBWLimit == 0)
    {
        if (context->options.ddrReadBWLimit != 0)
            context->nnConfig.customizedFeature.ddrReadBWLimit = context->options.ddrReadBWLimit;
        else
            context->nnConfig.customizedFeature.ddrReadBWLimit = DDR_READ_BANDWIDTH_LIMIT;
    }
    if (context->nnConfig.customizedFeature.ddrWriteBWLimit == 0)
    {
        if (context->options.ddrWriteBWLimit != 0)
            context->nnConfig.customizedFeature.ddrWriteBWLimit = context->options.ddrWriteBWLimit;
        else
            context->nnConfig.customizedFeature.ddrWriteBWLimit = DDR_WRITE_BANDWIDTH_LIMIT;
    }
    if (context->nnConfig.customizedFeature.ddrTotalBWLimit == 0)
    {
        if (context->options.ddrTotalBWLimit != 0)
            context->nnConfig.customizedFeature.ddrTotalBWLimit = context->options.ddrTotalBWLimit;
        else
            context->nnConfig.customizedFeature.ddrTotalBWLimit = DDR_TOTAL_BANDWIDTH_LIMIT;
    }
    if (context->nnConfig.customizedFeature.axiSramReadBWLimit == 0)
    {
        if (context->options.axiSramReadBWLimit != 0)
            context->nnConfig.customizedFeature.axiSramReadBWLimit = context->options.axiSramReadBWLimit;
        else
            context->nnConfig.customizedFeature.axiSramReadBWLimit = AXI_SRAM_READ_BANDWIDTH_LIMIT;
    }
    if (context->nnConfig.customizedFeature.axiSramWriteBWLimit == 0)
    {
        if (context->options.axiSramWriteBWLimit != 0)
            context->nnConfig.customizedFeature.axiSramWriteBWLimit = context->options.axiSramWriteBWLimit;
        else
            context->nnConfig.customizedFeature.axiSramWriteBWLimit = AXI_SRAM_WRITE_BANDWIDTH_LIMIT;
    }
    if (context->nnConfig.customizedFeature.axiSramTotalBWLimit == 0)
    {
        if (context->options.axiSramTotalBWLimit != 0)
            context->nnConfig.customizedFeature.axiSramTotalBWLimit = context->options.axiSramTotalBWLimit;
        else
            context->nnConfig.customizedFeature.axiSramTotalBWLimit = AXI_SRAM_TOTAL_BANDWIDTH_LIMIT;
    }
    if (context->nnConfig.customizedFeature.axiBusReadBWLimit == 0)
    {
        if (context->options.axiBusReadBWLimit != 0)
            context->nnConfig.customizedFeature.axiBusReadBWLimit = context->options.axiBusReadBWLimit;
        else
            context->nnConfig.customizedFeature.axiBusReadBWLimit = AXI_BUS_READ_BANDWIDTH_LIMIT;
    }
    if (context->nnConfig.customizedFeature.axiBusWriteBWLimit == 0)
    {
        if (context->options.axiBusWriteBWLimit != 0)
            context->nnConfig.customizedFeature.axiBusWriteBWLimit = context->options.axiBusWriteBWLimit;
        else
            context->nnConfig.customizedFeature.axiBusWriteBWLimit = AXI_BUS_WRITE_BANDWIDTH_LIMIT;
    }
    if (context->nnConfig.customizedFeature.axiBusTotalBWLimit == 0)
    {
        if (context->options.axiBusTotalBWLimit != 0)
            context->nnConfig.customizedFeature.axiBusTotalBWLimit = context->options.axiBusTotalBWLimit;
        else
            context->nnConfig.customizedFeature.axiBusTotalBWLimit = AXI_BUS_TOTAL_BANDWIDTH_LIMIT;
    }

    if (context->options.vipSRAMSize != VX_INVALID_VALUE)
        context->nnConfig.customizedFeature.vipSRAMSize = context->options.vipSRAMSize;
    if (context->options.axiSRAMSize != VX_INVALID_VALUE)
        context->nnConfig.customizedFeature.axiSRAMSize = context->options.axiSRAMSize;

    if (context->nnConfig.customizedFeature.ddrLatency == 0)
    {
        if (context->options.ddrLatency != 0)
            context->nnConfig.customizedFeature.ddrLatency = context->options.ddrLatency;
        else
            context->nnConfig.customizedFeature.ddrLatency = DDR_LATENCY;
    }

    if (context->nnConfig.customizedFeature.freqInMHZ == 0)
    {
        if (context->options.freqInMHZ != 0)
            context->nnConfig.customizedFeature.freqInMHZ = context->options.freqInMHZ;
        else
            context->nnConfig.customizedFeature.freqInMHZ = FREQ_IN_MHZ;
    }

    if (context->nnConfig.customizedFeature.maxSocOTNumber == 0)
    {
        if (context->options.maxSocOTNumber != 0)
            context->nnConfig.customizedFeature.maxSocOTNumber = context->options.maxSocOTNumber;
        else
            context->nnConfig.customizedFeature.maxSocOTNumber = MAX_SOC_OUT_STANDING_NUMBER;
    }


    if (context->nnConfig.customizedFeature.axiClockFreqInMHZ == 0)
    {
        if (context->options.axiClockFreqInMHZ != 0)
            context->nnConfig.customizedFeature.axiClockFreqInMHZ = context->options.axiClockFreqInMHZ;
        else
            context->nnConfig.customizedFeature.axiClockFreqInMHZ = AXI_CLK_FREQ_IN_MHZ;
    }

    context->nnConfig.customizedFeature.nnWriteWithoutUSC = NN_WRITE_WITHOUT_USC;
    context->nnConfig.customizedFeature.depthWiseSupport = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_NN_DEPTHWISE_SUPPORT) ? 1 : 0;;
    context->nnConfig.customizedFeature.vipVectorPrune = context->options.enableVectorPrune;


    {
        vx_float32 maxOutstandingCycle = (vx_float32)context->nnConfig.fixedFeature.maxOTNumber * 4;
        context->nnConfig.derivedFeature.internalLatency = (vx_float32)(20.0 + (11.0 + 6.0) * context->nnConfig.customizedFeature.freqInMHZ / context->nnConfig.customizedFeature.axiClockFreqInMHZ);
        context->nnConfig.derivedFeature.totalLatency = 1.0f * (vx_uint32)(context->nnConfig.customizedFeature.ddrLatency + context->nnConfig.derivedFeature.internalLatency + 0.5f);
        context->nnConfig.derivedFeature.ddrReadBWInBytePerCycle = context->nnConfig.customizedFeature.ddrReadBWLimit;
        context->nnConfig.derivedFeature.ddrWriteBWInBytePerCycle = context->nnConfig.customizedFeature.ddrWriteBWLimit;
        if (context->nnConfig.derivedFeature.totalLatency > maxOutstandingCycle)
        {
            vx_float32 ddrBWLimitedByLatency = (16 * maxOutstandingCycle) / context->nnConfig.derivedFeature.totalLatency;
            context->nnConfig.derivedFeature.ddrReadBWInBytePerCycle = gcmMIN(context->nnConfig.derivedFeature.ddrReadBWInBytePerCycle, ddrBWLimitedByLatency);
            context->nnConfig.derivedFeature.ddrWriteBWInBytePerCycle = gcmMIN(context->nnConfig.derivedFeature.ddrWriteBWInBytePerCycle, ddrBWLimitedByLatency);
        }
    }

#ifdef USE_LIB_NN_ARCH_PERF
    if ((gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "USE_LIB_NN_ARCH_PERF", &useLibNNArchPerf)) && useLibNNArchPerf && atoi(useLibNNArchPerf)))
    {
        BWL_T bwl = {
            /*ddr bw limit*/context->nnConfig.customizedFeature.ddrReadBWLimit, context->nnConfig.customizedFeature.ddrWriteBWLimit, context->nnConfig.customizedFeature.ddrTotalBWLimit,
            /*axi bw limit*/context->nnConfig.customizedFeature.axiSramReadBWLimit,context->nnConfig.customizedFeature.axiSramWriteBWLimit,context->nnConfig.customizedFeature.axiSramTotalBWLimit,
            /*axi-bus bw limit*/context->nnConfig.customizedFeature.axiBusReadBWLimit,context->nnConfig.customizedFeature.axiBusWriteBWLimit,context->nnConfig.customizedFeature.axiBusTotalBWLimit,
            /*internal write bw limite*/ (vx_float32)context->nnConfig.fixedFeature.nnLanesPerOutCycle,
            /*ddr latency*/context->nnConfig.customizedFeature.ddrLatency,
            /*total latency*/context->nnConfig.derivedFeature.totalLatency
        };
        APM_IN_PARAM_T inParam;
        gcsHAL_CHIPIDENTITY chipIdentity;
        gcoHAL_QueryChipIdentityEx(VX_NULL, sizeof(gcsHAL_CHIPIDENTITY), &chipIdentity);
        inParam.chipDef.ChipID = (vx_uint32)chipIdentity.chipModel;
        inParam.chipDef.ChipVersion = chipIdentity.chipRevision;
        inParam.chipDef.ProductID = chipIdentity.productID;
        inParam.chipDef.EcoID = chipIdentity.ecoID;
        inParam.chipDef.CustomerID = chipIdentity.customerID;

        memcpy(&inParam.bwl, &bwl, sizeof(bwl));
        context->apm = CreateAPModel(inParam);
    }
#endif
}

/* Write dataBits bits from data to buffer starting from bitOffset. */
void writeBits(uint32_t **buffer, vx_uint32 *bitOffset, vx_uint32 data, vx_uint32 dataBits)
{
    vx_uint32 copiedBits;
    vx_bool neg = (dataBits > 32 && (vx_int32)data < 0) ? vx_true_e : vx_false_e;
    vx_int32 tmp0, tmp1;

    /*check the overflow for hw limitation, v6 support 24, v7 support 32*/
    if ((vx_int32)data >= (1LL << dataBits))
    {
        vxError("ERROR: data overflow: dataBits = 0x%08x, data = 0x%08x\n", dataBits, data);
    }
    gcmASSERT((vx_int32)data < (1LL << dataBits));

    // Clear upper bits.
    if (dataBits < 32)
    {
        data &= ((1LL << dataBits) - 1);
    }

    // Write the bits in current word.
    if (32 - *bitOffset >= dataBits)
    {
        // Write data to current word.
        data <<= *bitOffset;
        if (*bitOffset)
        {
            **buffer |= data;
        }
        else
        {
            **buffer = data;
        }
        *bitOffset += dataBits;
        return;
    }

    // Write the remaining bits to current word.
    copiedBits = 32 - *bitOffset;
    if (copiedBits)
    {
        **buffer |= (data << *bitOffset);
        dataBits -= copiedBits;
    }

    // Write the rest bits to next word.
    (*buffer)++;
    if (neg)
    {
        tmp0 = ~0;
        tmp1 = (vx_uint32)tmp0 >> (32 - copiedBits);
        tmp0 = (vx_uint32)tmp0 >> copiedBits;
        tmp0 = ~tmp0;
        tmp0 |= (data >> copiedBits);
        tmp0 &= tmp1;
        **buffer = tmp0;
    }
    else
        **buffer = (data >> copiedBits);

    if (dataBits > 32)
    {
        dataBits -= 32;
        (*buffer)++;
        if (neg)
        {
            tmp0 = ~0;
            tmp0 = (vx_uint32)tmp0 >> (32 - dataBits);
            **buffer = tmp0;
        }
        else
            **buffer = 0;
    }
    *bitOffset = dataBits;
}

/* Replace dataBits bits from data to buffer starting from bitOffset. */
void replaceBits(uint32_t **buffer, vx_uint32 *bitOffset, vx_uint32 data, vx_uint32 dataBits)
{
    vx_uint32 copiedBits;
    vx_bool neg = (dataBits > 32 && (vx_int32)data < 0) ? vx_true_e : vx_false_e;
    vx_int32 tmp0, tmp1;

    /*check the overflow for hw limitation, v6 support 24, v7 support 32*/
    if ((vx_int32)data >= (1LL << dataBits))
    {
        vxError("ERROR: data overflow: dataBits = 0x%08x, data = 0x%08x\n", dataBits, data);
    }
    gcmASSERT((vx_int32)data < (1LL << dataBits));

    // Clear upper bits.
    if (dataBits < 32)
    {
        data &= ((1LL << dataBits) - 1);
    }

    // Write the bits in current word.
    if (32 - *bitOffset >= dataBits)
    {
        // Write data to current word.
        tmp0 = ~0;
        tmp1 = ~0;
        tmp0 = (vx_uint32)tmp0 >> (32 - *bitOffset);
        tmp1 = (vx_uint32)tmp1 << (dataBits + *bitOffset);
        tmp0 |= tmp1;
        data <<= *bitOffset;
        if (*bitOffset)
        {
            **buffer &= tmp0;
            **buffer |= data;
        }
        else
        {
            **buffer = data;
        }
        *bitOffset += dataBits;
        return;
    }

    // Write the remaining bits to current word.
    copiedBits = 32 - *bitOffset;
    if (copiedBits)
    {
        tmp0 = ~0;
        tmp0 = (vx_uint32)tmp0 >> copiedBits;
        **buffer &= tmp0;
        **buffer |= (data << *bitOffset);
        dataBits -= copiedBits;
    }

    // Write the rest bits to next word.
    (*buffer)++;
    if (neg)
    {
        tmp0 = ~0;
        tmp1 = (vx_uint32)tmp0 >> (32 - copiedBits);
        tmp0 = (vx_uint32)tmp0 >> copiedBits;
        tmp0 = ~tmp0;
        tmp0 |= (data >> copiedBits);
        tmp0 &= tmp1;
        **buffer |= tmp0;
    }
    else
    {
        tmp0 = ~0;
        tmp0 = (vx_uint32)tmp0 << dataBits;
        **buffer &= tmp0;
        **buffer |= (data >> copiedBits);
    }

    *bitOffset = dataBits;
}


vx_uint32 readBits(uint32_t **buffer, vx_uint32 *bitOffset, vx_uint32 dataBits)
{
    vx_uint32 data;
    vx_uint32 copiedBits;

    if(32 - *bitOffset >= dataBits)
    {
        // Get data from current word.
        data = **buffer >> *bitOffset;
        data &= ((1LL << dataBits) - 1);
        *bitOffset += dataBits;
        return data;
    }

    // Get remaining bits from current word.
    copiedBits = 32 - *bitOffset;
    if(copiedBits)
    {
        data = **buffer >> *bitOffset;
        dataBits -= copiedBits;
    }
    else
    {
        data = 0;
    }

    // Get next word.
    (*buffer)++;

    // Get rest bits from next word.
    data |= (**buffer & ((1LL << dataBits) - 1)) << copiedBits;
    *bitOffset = dataBits;

    return data;
}

/* Pack zeros to next 64-byte alignment. */
void packZeros(uint32_t **buffer, vx_uint32 *bitOffset, vx_uint32 alignedOffset)
{
    // The higher bits of the current word should already be zeros.
    if (*bitOffset)
    {
        (*buffer)++;
        *bitOffset = 0;
    }

    // Check if already aligned.
    if ((((uint32_t)(gctUINTPTR_T)*buffer) & 0x3F) == alignedOffset)
    {
        return;
    }

    // Pack zeros to following words until 64-byte aligned.
    while ((((uint32_t)(gctUINTPTR_T)*buffer) & 0x3F) != alignedOffset)
    {
        **buffer = 0;
        (*buffer)++;
    }
}

void _DataGeneralConvert(void* input_ptr, void* output_ptr, vx_uint32 input_size, vx_uint32 output_size)
{
    if (input_size == output_size)
    {
        if (input_size == 4)
        {
            *(vx_uint32*)output_ptr = *(vx_uint32*)input_ptr;
        }
        else if (input_size == 2)
        {
            *(vx_uint16*)output_ptr = *(vx_uint16*)input_ptr;
        }
        else if (input_size == 1)
        {
            *(vx_int8*)output_ptr = *(vx_int8*)input_ptr;
        }
    }
    else if (input_size == 4)
    {
        if (output_size == 2)
        {
            *(vx_uint16*)output_ptr = Fp32toFp16(*(vx_float32*)input_ptr);
        }
        else if (output_size == 1)
        {
            *(vx_int8*)output_ptr = Fp32toInt8_fc(*(vx_float32*)input_ptr);
        }
    }
}



vx_int32 getHwPoolingType(vx_enum poolingType)
{
    switch (poolingType)
    {
    case VX_NN_POOLING_MAX:
        return VIV_NN_POOLING_MAX;
    case VX_NN_POOLING_AVG:
        return VIV_NN_POOLING_AVG;
    case VX_NN_POOLING_FFP:
        return VIV_NN_POOLING_FIRST_PIXEL;
    default:
        break;
    }

    return VIV_NN_POOLING_NON;
}

vx_int32 getHWRoundingMode(vx_nn_round_mode_e roundingMode, vx_enum dataFormat, vx_bool isTP)
{
    /* Based on VIP_V7_PRD 0.49. */

    if (!isTP)
    {
        switch (dataFormat)
        {
        case VX_TYPE_INT8:
        case VX_TYPE_UINT8:
        case VX_TYPE_INT16:
        case VX_TYPE_UINT16:
            switch (roundingMode)
            {
            case VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING:
                return 0x0;
            case VX_NN_ROUNDING_MODE_RTNE:
                return 0x1;
            case VX_NN_ROUNDING_MODE_RTZ:
                return 0x2;
            case VX_NN_ROUNDING_MODE_RTNI:
                return 0x3;
            default:
                return 0x1;
            }
            break;
        case VX_TYPE_FLOAT16:
            switch (roundingMode)
            {
            case VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING:
            case VX_NN_ROUNDING_MODE_RTNI:
                /* Not supported. Convert it to RTNE. */
            case VX_NN_ROUNDING_MODE_RTNE:
                return 0x1;
            case VX_NN_ROUNDING_MODE_RTZ:
                return 0x2;
            default:
                /* Set default to RTNE. */
                return 0x1;
            }
            break;
        default:
            vxError("Invalid color format: %u.", dataFormat);
            gcmASSERT(0);
            return 0;
        }
    }
    else
    {
        switch (dataFormat)
        {
        case VX_TYPE_INT8:
        case VX_TYPE_UINT8:
        case VX_TYPE_INT16:
        case VX_TYPE_UINT16:
            switch (roundingMode)
            {
            case VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING:
                return 0x0;
            case VX_NN_ROUNDING_MODE_RTNE:
                return 0x1;
            case VX_NN_ROUNDING_MODE_RTZ:
                return 0x2;
            case VX_NN_ROUNDING_MODE_RTNI:
                return 0x3;
            default:
                /* Set default to RTNE. */
                return 0x1;
            }
            break;
        case VX_TYPE_FLOAT16:
            switch (roundingMode)
            {
            case VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING:
            case VX_NN_ROUNDING_MODE_RTNI:
                /* Not supported. Convert it to RTNE. */
            case VX_NN_ROUNDING_MODE_RTNE:
                return 0x1;
            case VX_NN_ROUNDING_MODE_RTZ:
                return 0x2;
            default:
                return 0x0;
            }
            break;
        default:
            vxError("Invalid color format: %u.", dataFormat);
            gcmASSERT(0);
            return 0;
        }
    }
}

vx_int32 getHWBorderMode(vx_enum padMode, gceVX_ACCELERATOR_TYPE accelerator)
{
    switch (accelerator)
    {
    case gcvVX_ACCELERATOR_NN:
        switch (padMode)
        {
        case VX_PAD_CONSTANT:
            return 0x0;
        case VX_PAD_REPLICATE:
            return 0x1;
        case VX_PAD_MIRROR_SYMMETRIC:
            return 0x2;
        case VX_PAD_MIRROR_REFLECT:
            return 0x3;
        default:
            return 0x0;
        }
        break;

    case gcvVX_ACCELERATOR_TP:
        switch (padMode)
        {
        case VX_PAD_CONSTANT:
            return 0x0;
        case VX_PAD_REPLICATE:
            return 0x1;
        case VX_PAD_MIRROR_SYMMETRIC:
            return 0x2;
        case VX_PAD_MIRROR_REFLECT:
            return 0x3;
        default:
            return 0x0;
        }
        break;

    default:
        return 0x0;
    }
}

vx_status vxnneCommandBuffer_ExecuteCommands(
    vx_node                     node,
    vxnne_command_buffer        commandBuffer,
    gceVX_ACCELERATOR_TYPE      type,
    vx_uint32                   gpuId,
    vx_bool                     sync,
    vx_uint32                   syncEventID[]
    )
{
    vx_uint32 i = 0;
    vx_status status = VX_SUCCESS;
    vx_uint32 commandSize = (type == gcvVX_ACCELERATOR_TP) ? TP_COMMAND_SIZE : NNE_COMMAND_SIZE;

    for (i = 0; i < commandBuffer->commandCount; i++)
    {
        gctUINT8  captureBuffer[VX_MAX_NNTP_OPERATION_STATE_SIZE] = {0};
        gctUINT32 actualSize = 0;
        vx_bool needMultiGpuSync = vx_false_e;

        if ((vx_true_e == sync) && (i == (commandBuffer->commandCount - 1)))
        {
            needMultiGpuSync = vx_true_e;
        }

        if (node->graph->binarySave)
        {
            vx_binary_save binarySave = node->graph->binarySave;
            if ((0 == i) && (binarySave->waitCommandsSize > 0))
            {
                /* append wait commands */
                vxMemCopy(captureBuffer, binarySave->waitCommands, binarySave->waitCommandsSize);
            }
            status = gcfVX_CaptureState(captureBuffer + binarySave->waitCommandsSize,
                                        VX_MAX_NNTP_OPERATION_STATE_SIZE,
                                        &actualSize,
                                        gcvTRUE,
                                        gcvFALSE);
            if (status != VX_SUCCESS)
            {
                vxError("failed to capture nn/tp commands\n");
                vxmASSERT(0);
            }
        }

        status = gcfVX_Accel(commandBuffer->physical + i * commandSize, type,
                             commandBuffer->eventID[i], 0, (gctUINT32)gpuId, (gctBOOL)needMultiGpuSync, syncEventID[i]);
        if (status != VX_SUCCESS)
        {
            break;
        }

        if (node->graph->binarySave)
        {
            vx_uint32 cmdPhysical = (vx_uint32)(commandBuffer->physical + i * commandSize);
            vx_binary_save binarySave = node->graph->binarySave;

            status = gcfVX_CaptureState(gcvNULL, 0, &actualSize, gcvFALSE, gcvFALSE);
            if (status != VX_SUCCESS)
            {
                vxError("failed to capture nn/tp commands end\n");
                vxmASSERT(0);
            }

            vxoBinaryGraph_SaveNNTPStates(node,
                                          cmdPhysical,
                                          captureBuffer,
                                          (vx_uint32)actualSize + binarySave->waitCommandsSize);
            binarySave->waitCommandsSize = 0;
        }
    }

    return status;
}

vx_status vxnneOperation_ExecuteCommands(vxnne_operation operation, vxnne_command_buffer commandBuffer)
{
    vx_node   node = operation->layer->node;

    if (operation->target == VXNNE_OPERATION_TARGET_TP)
    {
        return vxnneCommandBuffer_ExecuteCommands(node, commandBuffer,
                                                  gcvVX_ACCELERATOR_TP,
                                                  operation->gpuId,
                                                  operation->mGpuSync,
                                                  operation->engineSync.eventId);
    }
    else
    {
        return vxnneCommandBuffer_ExecuteCommands(node, commandBuffer,
                                                  gcvVX_ACCELERATOR_NN,
                                                  operation->gpuId,
                                                  operation->mGpuSync,
                                                  operation->engineSync.eventId);
    }
}

void ReplaceOperationCmmdZdpOpt(
    vxnne_tensor_info inputInfo,
    vxnne_tensor_info outputInfo,
    vx_weights_biases_parameter wb)
{
    vx_uint32 fitN = 1;
    vx_context context = vxGetContext((vx_reference)wb);
    vx_uint32 inputSize = vxDataType_GetSize((vx_type_e)inputInfo->dataFormat);
    vx_uint32 outputSize = vxDataType_GetSize((vx_type_e)outputInfo->dataFormat);

    calcFitZdp3N(context, inputInfo->width, inputInfo->height, &fitN, 1, wb->wb_base->pooling_size_x);

    /* Need reshape input[x, y, kz] --> [x*y/fitN, fitN, kz] */
    /* Need reshape output[x, y, vz] --> [x*y/fitN, fitN, vz] */
    inputInfo->width = inputInfo->width * inputInfo->height / fitN;
    inputInfo->height = fitN;

    outputInfo->width = outputInfo->width * outputInfo->height / fitN;
    outputInfo->height = fitN;

    inputInfo->yStride        = inputSize * inputInfo->width;
    outputInfo->yStride       = outputSize * outputInfo->width;
}

void ReplaceOperationCmmd1xN(
    vxnne_tensor_info inputInfo,
    vxnne_tensor_info outputInfo,
    vx_weights_biases_parameter wb)
{
    vx_uint32 fitN = 1;
    vx_context context = vxGetContext((vx_reference)wb);
    vx_uint32 inputSize = vxDataType_GetSize((vx_type_e)inputInfo->dataFormat);
    vx_uint32 outputSize = vxDataType_GetSize((vx_type_e)outputInfo->dataFormat);

    fitN = calcFit1xN(context, inputInfo->depth, inputInfo->width, inputInfo->height);

    /* Need reshape input[x, y, kz] --> [x*y, fitN, kz/fitN] */
    /* Need reshape output[x, y, vz] --> [x*y, 1, vz] */
    inputInfo->width = inputInfo->width * inputInfo->height;
    inputInfo->height = fitN;
    inputInfo->depth = inputInfo->depth / fitN;

    outputInfo->width = outputInfo->width * outputInfo->height;
    outputInfo->height = 1;

    inputInfo->yStride        = inputSize * inputInfo->width;
    inputInfo->zStride        = inputInfo->yStride * inputInfo->height;
    outputInfo->yStride       = outputSize * outputInfo->width;
}

vx_uint8 MemPoolTypeToPerfType(
    vx_enum memPoolType)
{
    return memPoolType == VXNNE_MEM_POOL_TYPE_AXI_SRAM ?
                          SW_TILING_FROM_AXI_SRAM :
                          (memPoolType == VXNNE_MEM_POOL_TYPE_VIP_SRAM ? SW_TILING_FROM_VIP_SRAM : SW_TILING_FROM_DDR);
}

VX_PRIVATE_API vx_status vxnneOperationCommand_GenerateNNCommands(
    vx_context               context,
    vxnne_operation_command  operationCommand,
    vxnne_tiling_rect        input,
    vxnne_tiling_rect        output,
    vxnne_command_buffer     commandBuffer)
{
    vx_status status;
    vxnne_tensor_info_s inputInfo, outputInfo;
    vxnne_convolution_relu_pooling_operation convOperation = (vxnne_convolution_relu_pooling_operation)operationCommand->operation;
    vx_op_param parameter = &operationCommand->parameter;

    vx_uint32 physical  = 0;
    gctPOINTER logical  = gcvNULL;
    vx_node node = convOperation->base.layer->node;

    /* fix the base address here, only for DDR  */
    if (!input->sRAM)
    {
        vxoTensor_GetTensorBatchArrayViewMemory(
                convOperation->inputs,
                operationCommand->batchID,
                &logical,
                &physical);

        gcmASSERT(physical);

        input->physical += physical;
        input->logical   = (vx_uint8*)logical + (vx_uint32)(gctUINTPTR_T)input->logical;
        input->logicalBase = logical;
    }

    inputInfo.width          = input->width;
    inputInfo.height         = input->height;
    inputInfo.depth          = TENSOR_VIEW_SIZE_INDEX(convOperation->inputs, 2);

    vxmASSERT(convOperation->weights_biases->wb_base->hw_depth_wise || (WB_KERNEL_Z(convOperation->weights_biases) == inputInfo.depth));

    inputInfo.dataFormat     = TENSOR_DATA_TYPE(convOperation->inputs);
    inputInfo.roundingMode   = TENSOR_ROUNDING_MODE(convOperation->inputs);
    inputInfo.fixedPointPos  = TENSOR_POS(convOperation->inputs);
    inputInfo.scale          = TENSOR_TF_SCALE(convOperation->inputs);
    inputInfo.quantFormat    = TENSOR_QUANT_TYPE(convOperation->inputs);
    inputInfo.zeroPoint      = TENSOR_TF_ZEROPOINT(convOperation->inputs);
    inputInfo.padZeorValue   = TENSOR_PAD_ZERO_VALUE(convOperation->inputs);
    inputInfo.yStride          = input->yStride;
    inputInfo.zStride          = input->zStride;
    inputInfo.circleBufferSize = input->circleBufferSize;
    inputInfo.physical.start   = input->physical;
    inputInfo.sRAM             = input->sRAM;
    inputInfo.physical.circularBufEndAddrPlus1 = input->circularBufEndAddrPlus1;
    inputInfo.brickMode        = vx_false_e;
    inputInfo.memoryPhysicalBase = input->memoryPhysicalBase;
    inputInfo.memoryLogicalBase  = input->memoryLogicalBase;
    inputInfo.memorySize         = input->memorySize;

    /* fix the base address here, only for DDR  */
    if (!output->sRAM)
    {
        vxoTensor_GetTensorBatchArrayViewMemory(
                convOperation->outputs,
                operationCommand->batchID,
                &logical,
                &physical);

        gcmASSERT(physical);

        output->physical += physical;
        output->logical   = (vx_uint8*)logical + (vx_uint32)(gctUINTPTR_T)output->logical;
        output->logicalBase = logical;
    }

    outputInfo.width          = output->width;
    outputInfo.height         = output->height;
    outputInfo.depth          = TENSOR_VIEW_SIZE_INDEX(convOperation->outputs, 2);

    if (convOperation->pool_size_y != 0)
    {
        outputInfo.width      = operationCommand->cmdInfo.convWidth;
        outputInfo.height     = operationCommand->cmdInfo.convHeight;
    }

    outputInfo.dataFormat     = TENSOR_DATA_TYPE(convOperation->outputs);
    outputInfo.roundingMode   = TENSOR_ROUNDING_MODE(convOperation->outputs);
    outputInfo.fixedPointPos  = TENSOR_POS(convOperation->outputs);
    outputInfo.scale          = TENSOR_TF_SCALE(convOperation->outputs);
    outputInfo.quantFormat    = TENSOR_QUANT_TYPE(convOperation->outputs);
    outputInfo.zeroPoint      = TENSOR_TF_ZEROPOINT(convOperation->outputs);
    outputInfo.padZeorValue   = TENSOR_PAD_ZERO_VALUE(convOperation->outputs);
    outputInfo.yStride        = output->yStride;
    outputInfo.zStride        = output->zStride;
    outputInfo.circleBufferSize = output->circleBufferSize;
    outputInfo.sRAM             = output->sRAM;
    outputInfo.physical.start   = output->physical;
    outputInfo.physical.circularBufEndAddrPlus1 = output->circularBufEndAddrPlus1;
    outputInfo.brickMode        = vx_false_e;
    outputInfo.flush            = operationCommand->cmdInfo.flush;
    outputInfo.memoryPhysicalBase = output->memoryPhysicalBase;
    outputInfo.memorySize         = output->memorySize;

    gcmASSERT(inputInfo.physical.start && outputInfo.physical.start);

    parameter->pad_x_left = operationCommand->cmdInfo.padLeft;
    parameter->pad_y_top  = operationCommand->cmdInfo.padTop;
    parameter->pad_x_right = 0;
    parameter->pad_y_bottom = 0;
    parameter->pad_mode = convOperation->padMode;
    parameter->pad_const = convOperation->padConst != VX_NULL ? convOperation->padConst->value->n32 : 0;
    parameter->conv_rounding_type = convOperation->conv_rounding_type;
    parameter->enable_relu = convOperation->enable_relu;
    parameter->pool_type = convOperation->pool_type;
    parameter->pool_size_x = convOperation->pool_size_x;
    parameter->pool_size_y = convOperation->pool_size_y;
    parameter->pool_stride = operationCommand->operation->parameter.pool_size_x ? operationCommand->operation->parameter.pool_stride : 1;


    if (convOperation->weights_biases->wb_base->do_zdp_opt &&
        context->options.do1xnAfterSwtiling)
    {
        ReplaceOperationCmmdZdpOpt(&inputInfo, &outputInfo, convOperation->weights_biases);
    }
    else if (convOperation->weights_biases->wb_base->do_1xN &&
        context->options.do1xnAfterSwtiling)
    {
        ReplaceOperationCmmd1xN(&inputInfo, &outputInfo, convOperation->weights_biases);
    }

    if (operationCommand->cmdInfo.tilingParam.kernelsPerCore == 0)
    {
        /*ab, ddr->ddr path*/
        parameter->interleave_mode  = convOperation->resultInfo.interleaveMode;
        parameter->kernels_per_core = convOperation->resultInfo.kernelsPerCore;
        parameter->out_image_tile_x = convOperation->resultInfo.outImageTileXSize;
        parameter->out_image_tile_y = convOperation->resultInfo.outImageTileYSize;
        parameter->nnCoreCount      = convOperation->resultInfo.nnCoreCount;
    }
    else
    {
        /* swtiling path */
        parameter->interleave_mode  = operationCommand->cmdInfo.tilingParam.interleaveMode;
        parameter->kernels_per_core = operationCommand->cmdInfo.tilingParam.kernelsPerCore;
        parameter->out_image_tile_x = operationCommand->cmdInfo.tilingParam.outImageTileXSize;
        parameter->out_image_tile_y = operationCommand->cmdInfo.tilingParam.outImageTileYSize;
        parameter->nnCoreCount      = operationCommand->cmdInfo.tilingParam.nnCoreCount;
    }

    vxmASSERT(operationCommand->cmdInfo.wb);

    parameter->kernel_x    = WB_KERNEL_X(operationCommand->cmdInfo.wb);
    parameter->kernel_y    = WB_KERNEL_Y(operationCommand->cmdInfo.wb);
    parameter->kernel_z    = WB_KERNEL_Z(operationCommand->cmdInfo.wb);
    parameter->other_ref   =  (vx_reference)operationCommand->cmdInfo.wb;

    parameter->imageCacheSize   = operationCommand->cmdInfo.imageCacheSize;
    parameter->imageCacheStart  = operationCommand->cmdInfo.imageCacheStart;
    parameter->imageCacheMode   = operationCommand->cmdInfo.imageCacheMode;
    parameter->kernelCacheSize  = operationCommand->cmdInfo.kernelCacheSize;
    parameter->kernelCacheStart = operationCommand->cmdInfo.kernelCacheStart;
    parameter->kernelCacheMode  = operationCommand->cmdInfo.kernelCacheMode;

    vxmONERROR(vxnneCommandBuffer_GenerateCommands(context, node, operationCommand, &inputInfo, &outputInfo, VXNNE_OPERATION_TARGET_NN, parameter, commandBuffer));

#if gcdDUMP
    dumpNNCommandInfo(0, convOperation->weights_biases->slice_num, NULL, operationCommand);
#endif

    if (context->options.collectPerfType == COLLECT_PERF_RUN)
    {
        vx_arch_perf_s perf;
        vx_tensor input, output;
        vx_weights_biases_parameter wb;

        INITIALIZE_STRUCT(perf);

        input  = convOperation->inputs;
        output = convOperation->outputs;
        wb     = operationCommand->cmdInfo.wb;

        perf.resultInfo.calculated        = HALF_DONE;
        perf.resultInfo.interleaveMode    = operationCommand->parameter.interleave_mode;
        perf.resultInfo.outImageTileXSize = operationCommand->parameter.out_image_tile_x;
        perf.resultInfo.outImageTileYSize = operationCommand->parameter.out_image_tile_y;
        perf.resultInfo.kernelsPerCore    = operationCommand->parameter.kernels_per_core;

        perf.swTilingInfo.kernelCacheMode = operationCommand->parameter.kernelCacheMode;
        perf.swTilingInfo.imageCacheMode  = operationCommand->parameter.imageCacheMode;

        calculateArchPerfFromTiling(context,
                                    convOperation->base.layer,
                                    &perf,
                                    &inputInfo,
                                    &outputInfo,
                                    input, output, wb,
                                    operationCommand,
                                    convOperation->base.target,
                                    convOperation->base.operatorType);

         if (context->options.enableNNArchPerfPrint)
            showArchPerformance(context, convOperation->base.layer, &convOperation->base, &perf);

    }

    return VX_SUCCESS;

OnError:
    return status;
}

VX_PRIVATE_API vx_status vxnneOperationCommand_GenerateTPCommands(
    vx_context               context,
    vxnne_operation_command  operationCommand,
    vxnne_tiling_rect        input,
    vxnne_tiling_rect        output,
    vx_tp_value_cmd          tp_values,
    vxnne_command_buffer     commandBuffer
    )
{
    vx_status status;
    vxnne_tensor_info_s inputInfo, outputInfo;
    vxnne_tp_operation tpOperation = (vxnne_tp_operation)operationCommand->operation;
    vx_op_param tpParams = &operationCommand->parameter;

    vx_uint32 physical  = 0;
    gctPOINTER logical  = gcvNULL;

    INITIALIZE_STRUCT(inputInfo);
    INITIALIZE_STRUCT(outputInfo);

    /* fix the base address here, only for DDR  */
    if (!input->sRAM)
    {
        vxoTensor_GetTensorBatchArrayViewMemory(
                tpOperation->input,
                operationCommand->batchID,
                &logical,
                &physical);

        gcmASSERT(physical);

        input->physical += physical;
        input->logical   = (vx_uint8*)logical + (vx_uint32)(gctUINTPTR_T)input->logical;
        input->logicalBase = logical;
    }

    inputInfo.width          = input->width;
    inputInfo.height         = input->height;
    inputInfo.depth          = TENSOR_VIEW_SIZE_INDEX(tpOperation->input, 2);
    inputInfo.dataFormat     = TENSOR_DATA_TYPE(tpOperation->input);
    inputInfo.roundingMode   = TENSOR_ROUNDING_MODE(tpOperation->input);
    inputInfo.fixedPointPos  = TENSOR_POS(tpOperation->input);
    inputInfo.scale          = TENSOR_TF_SCALE(tpOperation->input);
    inputInfo.quantFormat    = TENSOR_QUANT_TYPE(tpOperation->input);
    inputInfo.zeroPoint      = TENSOR_TF_ZEROPOINT(tpOperation->input);
    inputInfo.padZeorValue   = TENSOR_PAD_ZERO_VALUE(tpOperation->input);
    inputInfo.yStride            = input->yStride;
    inputInfo.zStride            = input->zStride;
    inputInfo.circleBufferSize   = input->circleBufferSize;
    inputInfo.sRAM               = input->sRAM;
    inputInfo.memoryPhysicalBase = input->memoryPhysicalBase;
    inputInfo.memoryLogicalBase  = input->memoryLogicalBase;
    inputInfo.memorySize         = input->memorySize;
    inputInfo.physical.start     = input->physical;

    inputInfo.physical.circularBufEndAddrPlus1 = input->circularBufEndAddrPlus1;

    /* fix the base address here, only for DDR  */
    if (!output->sRAM)
    {
        vxoTensor_GetTensorBatchArrayViewMemory(
                tpOperation->output,
                operationCommand->batchID,
                &logical,
                &physical);

        gcmASSERT(physical);

        output->physical += physical;
        output->logical   = (vx_uint8*)logical + (vx_uint32)(gctUINTPTR_T)output->logical;
        output->logicalBase = logical;
    }
    outputInfo.width          = output->width;
    outputInfo.height         = output->height;
    outputInfo.depth          = TENSOR_VIEW_SIZE_INDEX(tpOperation->output, 2);
    outputInfo.dataFormat     = TENSOR_DATA_TYPE(tpOperation->output);
    outputInfo.roundingMode   = TENSOR_ROUNDING_MODE(tpOperation->output);
    outputInfo.fixedPointPos  = TENSOR_POS(tpOperation->output);
    outputInfo.scale          = TENSOR_TF_SCALE(tpOperation->output);
    outputInfo.quantFormat    = TENSOR_QUANT_TYPE(tpOperation->output);
    outputInfo.zeroPoint      = TENSOR_TF_ZEROPOINT(tpOperation->output);
    outputInfo.padZeorValue   = TENSOR_PAD_ZERO_VALUE(tpOperation->output);
    outputInfo.yStride        = output->yStride;
    outputInfo.zStride        = output->zStride;
    outputInfo.circleBufferSize   = output->circleBufferSize;
    outputInfo.sRAM               = output->sRAM;
    outputInfo.memoryPhysicalBase = output->memoryPhysicalBase;
    outputInfo.memorySize         = output->memorySize;
    outputInfo.physical.start     = output->physical;
    outputInfo.physical.circularBufEndAddrPlus1 = output->circularBufEndAddrPlus1;
    outputInfo.flush            = operationCommand->cmdInfo.flush;

    gcmASSERT(inputInfo.physical.start && outputInfo.physical.start);

    *tpParams                    = operationCommand->operation->parameter;
    tpParams->orig_no_pad        = !tpParams->pad_x_left && !tpParams->pad_y_top && !tpParams->pad_x_right && !tpParams->pad_y_bottom;
    tpParams->pad_x_left         = operationCommand->cmdInfo.padLeft;
    tpParams->pad_y_top          = operationCommand->cmdInfo.padTop;
    tpParams->pad_x_right        = 0;
    tpParams->pad_y_bottom       = 0;

    if (operationCommand->operation->operatorType == VXNNE_OPERATOR_FULLYCONNECTED &&
        tpOperation->weights_biases != VX_NULL)
    {
        vx_uint32 outputDims[2] = {output->width, output->height};

        /* compress original wb */
        vxmONERROR(vxoWeightsBiases_Compress(
            context,
            tpOperation->weights_biases,
            0,
            outputDims,
            outputInfo.dataFormat,
            -1));
    }

    vxmONERROR(vxnneCommandBuffer_GenerateCommands(context, tpOperation->base.layer->node, operationCommand, &inputInfo, &outputInfo, VXNNE_OPERATION_TARGET_TP, tpParams, commandBuffer));

    tpOperation->slice_num = commandBuffer->commandCount;

    if (context->options.collectPerfType == COLLECT_PERF_RUN)
    {
        vx_arch_perf_s perf;
        vx_tensor input, output;
        vx_weights_biases_parameter wb;

        INITIALIZE_STRUCT(perf);

        input  = tpOperation->input;
        output = tpOperation->output;
        wb     = tpOperation->weights_biases;

        calculateArchPerfFromTiling(context,
                                    tpOperation->base.layer,
                                    &perf,
                                    &inputInfo,
                                    &outputInfo,
                                    input, output, wb,
                                    operationCommand,
                                    tpOperation->base.target,
                                    tpOperation->base.operatorType);

         if (context->options.enableNNArchPerfPrint)
            showArchPerformance(context, tpOperation->base.layer, &tpOperation->base, &perf);

    }

    return VX_SUCCESS;

OnError:
    return status;
}

VX_INTERNAL_API vx_status vxnneOperationCommand_GenerateCommands(
    vx_context               context,
    vxnne_operation_command  operationCommand)
{
    vxnne_operation  operation = operationCommand->operation;
    vx_node   node = operation->layer->node;
    vx_status status = VX_SUCCESS;

    if (operation->target == VXNNE_OPERATION_TARGET_NN || operation->target == VXNNE_OPERATION_TARGET_TP)
    {
        if (operation->target == VXNNE_OPERATION_TARGET_TP)
        {
            status = vxnneOperationCommand_GenerateTPCommands(
                        context, operationCommand,
                        &operationCommand->inputTile,
                        &operationCommand->outputTile,
                        operation->parameter.tp_value,
                        &operationCommand->commandBuffer);
        }
        else
        {
            status = vxnneOperationCommand_GenerateNNCommands(
                        context, operationCommand,
                        &operationCommand->inputTile,
                        &operationCommand->outputTile,
                        &operationCommand->commandBuffer);
        }

        return status;
    }
    else /* VXNNE_OPERATION_TARGET_SW / SH / SC */
    {
        if (node->graph->binarySave)
        {
            vx_binary_save binarySave = node->graph->binarySave;
            binarySave->operationCmdPhysical[binarySave->currOperationIndex] = gcmPTR_TO_UINT64(operation);
            binarySave->operationOffset[binarySave->currOperationIndex] = binarySave->currOperationOffset;
            binarySave->currOperationIndex++;
            binarySave->currOperationOffset += sizeof(vx_binary_operation_info_s);
        }

        if (context->options.collectPerfType == COLLECT_PERF_RUN)
        {
            if (context->options.enableNNArchPerfPrint)
            {
                showArchPerformance(context, operation->layer, operation, VX_NULL);
            }
        }
    }

    return status;
}

vx_status vxnneOperation_GetInfo(vxnne_operation operation, vxnne_operation_info info)
{
    /* initial value to zero */
    gcoOS_ZeroMemory(info, sizeof(vxnne_operation_info_s));

    info->opType = operation->operatorType;
    info->target = operation->target;
    info->tpType = operation->parameter.tpType;

    if (operation->target != VXNNE_OPERATION_TARGET_NN &&
        operation->target != VXNNE_OPERATION_TARGET_TP &&
        operation->target != VXNNE_OPERATION_TARGET_SH)
    {
        return VX_SUCCESS;
    }

    if (operation->target == VXNNE_OPERATION_TARGET_NN)
    {
        vxnne_convolution_relu_pooling_operation convolution = (vxnne_convolution_relu_pooling_operation)operation;

        info->input         = convolution->inputs;
        info->output        = convolution->outputs;
        info->weightsBiases = convolution->weights_biases;
        info->kernelX       = convolution->weights_biases->weights_sizes[0];
        info->kernelY       = convolution->weights_biases->weights_sizes[1];
        info->kernelZ       = convolution->weights_biases->weights_sizes[2];
        info->pad.left      = operation->parameter.pad_x_left;
        info->pad.right     = 0;
        info->pad.top       = operation->parameter.pad_y_top;
        info->pad.bottom    = 0;

        info->poolType      = operation->parameter.pool_type;
        info->poolSizeX     = operation->parameter.pool_size_x;
        info->poolSizeY     = operation->parameter.pool_size_y;
        info->poolStrideX   = operation->parameter.pool_size_x ? operation->parameter.pool_stride : 1;
        info->poolStrideY   = operation->parameter.pool_size_y ? operation->parameter.pool_stride : 1;

        info->enablePooling = (vx_bool)(operation->parameter.pool_size_x != 0 && operation->parameter.pool_size_y != 0);

        info->reshuffStrideX = 1;
        info->reshuffStrideY = 1;

        info->normStrideX = 1;
        info->normStrideY = 1;

        vxmASSERT(!info->enablePooling || (info->poolType == VX_NN_POOLING_MAX || info->poolType == VX_NN_POOLING_FFP));
        vxmASSERT(!info->enablePooling || (info->poolSizeY == 2 || info->poolSizeY == 3));
    }
    else if (operation->target == VXNNE_OPERATION_TARGET_TP)
    {
        vxnne_tp_operation tp = (vxnne_tp_operation)operation;

        info->input         = tp->input;
        info->output        = tp->output;
        info->weightsBiases = VX_NULL;
        info->kernelX       = 0;
        info->kernelY       = 0;
        info->kernelZ       = 0;
        info->pad.left      = operation->parameter.pad_x_left;
        info->pad.right     = 0;
        info->pad.top       = operation->parameter.pad_y_top;
        info->pad.bottom    = 0;
        info->poolType      = operation->parameter.pool_type;
        info->poolSizeX     = operation->parameter.pool_size_x;
        info->poolSizeY     = operation->parameter.pool_size_y;
        info->poolStrideX   = operation->parameter.pool_size_x ? operation->parameter.pool_stride : 1;
        info->poolStrideY   = operation->parameter.pool_size_y ? operation->parameter.pool_stride : 1;
        info->enablePooling = (vx_bool)(operation->parameter.pool_size_x != 0 && operation->parameter.pool_size_y != 0);

        info->reshuffStrideX = operation->operatorType == VXNNE_OPERATOR_RESHUFFLE ? WB_STRIDE_X(tp->weights_biases) : 1;
        info->reshuffStrideY = operation->operatorType == VXNNE_OPERATOR_RESHUFFLE ? WB_STRIDE_Y(tp->weights_biases) : 1;

        info->normStrideX = 1;
        info->normStrideY = 1;
    }
    else if (operation->target == VXNNE_OPERATION_TARGET_SH)
    {
        info->input = operation->inputsNum > 0 ? (vx_tensor)operation->inputs[0] : (vx_tensor)VX_NULL;
        info->output = operation->outputsNum > 0 ? (vx_tensor)operation->outputs[0] : (vx_tensor)VX_NULL;

        info->reshuffStrideX = 1;
        info->reshuffStrideY = 1;

        info->normStrideX = 1;
        info->normStrideY = 1;

        info->poolStrideX = 1;
        info->poolStrideY = 1;
    }
    else
    {
        vxmASSERT(0);
    }

    vxmASSERT(info->poolStrideX == 1 ||  info->reshuffStrideX == 1);
    vxmASSERT(!info->enablePooling  || (info->poolSizeX != 0 && info->poolSizeY != 0));
    vxmASSERT(info->poolSizeX == info->poolSizeY);
    vxmASSERT(info->poolStrideX == info->poolStrideY);
    vxmASSERT(info->reshuffStrideX >= 1 && info->reshuffStrideY >= 1);
    vxmASSERT(info->poolStrideX >= 1&& info->poolStrideY >= 1);

    return VX_SUCCESS;
}

vx_status vxnneOperation_CalculateDimSize(
    vx_int32                inDimSize,
    vxnne_operation         operation,
    vx_int32*               outDimSize,
    vx_int32                calculateType
    )
{
    if (outDimSize == VX_NULL)
        return VX_ERROR_INVALID_PARAMETERS;

    if (calculateType & VX_DIM_IN_TO_OUT)
    {
        gcmASSERT(0);
    }
    else if (calculateType & VX_DIM_OUT_TO_IN)
    {
        if (operation->operatorType == VXNNE_OPERATOR_CONVOLUTION || operation->operatorType == VXNNE_OPERATOR_DEPTH_WISE_CONV)
        {
            vxnne_convolution_relu_pooling_operation convReluPoolingOperation = (vxnne_convolution_relu_pooling_operation)operation;
            vx_int32    dilationX = (vx_int32)convReluPoolingOperation->dilationX;
            vx_int32    dilationY = (vx_int32)convReluPoolingOperation->dilationY;
            vx_uint32  convPadXLeft = convReluPoolingOperation->pad_x_left;
            vx_uint32  convPadXRight = (convReluPoolingOperation->pad_x_right == 0) ? convReluPoolingOperation->pad_x_left : convReluPoolingOperation->pad_x_right;
            vx_uint32  convPadYTop = convReluPoolingOperation->pad_y_top;
            vx_uint32  convPadYBottom = (convReluPoolingOperation->pad_y_bottom == 0) ? convReluPoolingOperation->pad_y_top : convReluPoolingOperation->pad_y_bottom;
            vx_bool    enable_pooling = convReluPoolingOperation->enable_pooling;
            vx_int32   poolType = convReluPoolingOperation->pool_type;
            vx_uint32  poolSizeX = convReluPoolingOperation->pool_size_x;
            vx_uint32  poolSizeY = convReluPoolingOperation->pool_size_y;
            vx_uint32  kernelXSize = convReluPoolingOperation->weights_biases->weights_sizes[0];
            vx_uint32  kernelYSize = convReluPoolingOperation->weights_biases->weights_sizes[1];
            vx_uint32  convDimSize;
            vx_uint32  poolDimSize = 0;
            vx_uint32  inConvDimSize = inDimSize;
            vx_uint32  convStride = 1;    /* hw only support conv stride = 1 */
            vx_uint32  poolingStride = 2; /* hw only support pooling stride = 2 */

            if (calculateType & VX_DIM_WIDTH ||
                calculateType & VX_DIM_HEIGHT)
            {
                if (enable_pooling &&
                    ((poolType == VX_NN_POOLING_MAX) || (poolType == VX_NN_POOLING_AVG)))
                {
                    vx_uint32 poolPadXLeft = 0, poolPadXRight = 0;
                    vx_uint32 poolPadYTop = 0, poolPadYBottom = 0;

                    if (calculateType & VX_DIM_WIDTH)
                    {
                        poolDimSize = (inDimSize - 1) * poolingStride + poolSizeX - poolPadXLeft - poolPadXRight;
                    }
                    else
                    {
                        poolDimSize = (inDimSize - 1) * poolingStride + poolSizeY - poolPadYTop - poolPadYBottom;
                    }

                    inConvDimSize = poolDimSize;
                }

                if (calculateType & VX_DIM_WIDTH)
                {
                    convDimSize = (inConvDimSize - 1) * convStride + kernelXSize + (kernelXSize - 1) * dilationX - convPadXLeft - convPadXRight;
                }
                else
                {
                    convDimSize = (inConvDimSize - 1) * convStride + kernelYSize + (kernelYSize - 1) * dilationY - convPadYTop - convPadYBottom;
                }

                *outDimSize = convDimSize;
            }
        }
    }

    return VX_SUCCESS;
}

/*
  axiSRAM return axi address
  vipSRAM return relative sram address (base address is 0)
*/
VX_INTERNAL_API vx_status vxnneSRAM_Allocate(
    vxnne_sram              sRam,
    vx_uint32               size,
    gctPOINTER*             logical,
    vx_uint32*              physical
    )
{
    if (size > sRam->size - sRam->used)
    {
        return VX_ERROR_NO_MEMORY;
    }


    if (logical)
    {
        *logical  = (gctUINT8*)sRam->logical + sRam->used;
    }

    if (physical)
    {
        *physical = sRam->physical + sRam->used;
    }

    sRam->used += size;

    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxnneSRAM_AllocateRest(
    vxnne_sram              sRam,
    vx_uint32*              restSize,
    gctPOINTER*             logical,
    vx_uint32*              physical
    )
{
    if (sRam->size == sRam->used)
    {
        return VX_ERROR_NO_MEMORY;
    }

    *restSize = sRam->size - sRam->used;

    if (logical)
    {
        *logical  = (gctUINT8*)sRam->logical + sRam->used;
    }

    if (physical)
    {
        *physical = sRam->physical + sRam->used;
    }

    sRam->used = sRam->size;

    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxnneSRAM_AllocateEx(
    vxnne_sram              sRam,
    vx_uint32               size,
    gctPOINTER*             logical,
    vx_uint32*              physical,
    vx_enum                 allocateFlag
    )
{
    vxmASSERT(0);
    return VX_SUCCESS;
}


VX_INTERNAL_API vx_status vxnneSRAM_Free(
    vxnne_sram             sRam,
    vx_uint32              size
    )
{
    sRam->used -= size;
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxnneSRAM_Reset(
    vxnne_sram              sRam
    )
{
    sRam->used = 0;
    sRam->tailUsed = 0;
    return VX_SUCCESS;
}

void vxoNNExternsionDoReshuffle(
    vx_uint32 batchIndex,
    vx_tensor inputs,
    vx_tensor outputs,
    vx_uint32 padXLeft,
    vx_uint32 padXRight,
    vx_uint32 padYTop,
    vx_uint32 padYBottom,
    vx_enum   padMode,
    void*     padConst,
    vx_uint32 strideX,
    vx_uint32 strideY,
    vx_uint32 kx,
    vx_uint32 ky)
{
    vx_uint32 x, y, z, w;
    vx_uint32 srcWidth, srcHeight;
    vx_uint32 dstWidth, dstHeight, dstDepth;
    vx_uint32 srcXSize, srcYSize;
    vx_uint32 dstXSize, dstYSize, dstZSize, dstWSize, dstZIndex;
    vx_uint32 elementSize;
    void *srcDataAddr;
    void *dstDataAddr;

    /* do reshuffle*/
    srcWidth  = TENSOR_SIZE_INDEX(inputs, 0);
    srcHeight = TENSOR_SIZE_INDEX(inputs, 1);

    dstWidth  = TENSOR_SIZE_INDEX(outputs, 0);
    dstHeight = TENSOR_SIZE_INDEX(outputs, 1);
    dstDepth  = TENSOR_SIZE_INDEX(outputs, 2);

    srcXSize = srcWidth;
    srcYSize = srcHeight;

    dstXSize = dstWidth;
    dstYSize = dstHeight;
    dstZSize = dstDepth;
    dstWSize = 1;

    elementSize = (vx_uint32)vxDataType_GetSize((vx_type_e)TENSOR_DATA_TYPE(inputs));

    vxoTensor_GetTensorBatchArrayViewMemory(inputs, batchIndex, &srcDataAddr, VX_NULL);
    vxoTensor_GetTensorBatchArrayViewMemory(outputs, batchIndex, &dstDataAddr, VX_NULL);

    for (w = 0;  w < dstWSize; w++)
    {
        for (z = 0; z < dstZSize; z++)
        {
            if (kx == 1 && ky == 1)
            {
                if (z % (strideX * strideY) !=0)
                {
                    continue;
                }
                else
                {
                    dstZIndex = z / (strideX * strideY);
                }
            }
            else
            {
                dstZIndex = z;
            }

            for (y = 0; y < dstYSize; y++)
            {
                for (x = 0; x < dstXSize; x++)
                {
                    vx_uint8 *srcData, *dstData;
                    vx_uint32 srcPosX, srcPosY, srcPosZ;

                    dstData = (vx_uint8*)dstDataAddr + (x + y * dstXSize + dstZIndex * dstXSize * dstYSize) * elementSize;

                    srcPosX = (z % (strideX * strideY)) % strideX + x * strideX;
                    srcPosY = (z % (strideX * strideY)) / strideX + y * strideY;
                    srcPosZ = z / (strideX * strideY);
                    if (((srcPosX >= padXLeft) && (srcPosX <= padXLeft + srcWidth - 1)) && ((srcPosY >= padYTop) && (srcPosY <= padYTop + srcHeight - 1)))
                    {
                        srcData = (vx_uint8*)srcDataAddr + (srcPosZ * srcYSize * srcXSize + (srcPosY-padYTop) * srcXSize + (srcPosX-padXLeft)) * elementSize;
                        memcpy(dstData, srcData, elementSize);
                    }
                    else
                    {
                        if (padMode == VX_PAD_CONSTANT)
                        {
                            memcpy(dstData, padConst, elementSize);
                        }
                        else if ((padMode == VX_PAD_REPLICATE) || (padMode == VX_PAD_MIRROR_SYMMETRIC) || (padMode == VX_PAD_MIRROR_REFLECT))
                        {
                            vx_int32 posX, posY;
                            vx_uint32 srcImagePosX = 0, srcImagePosY = 0;
                            posX = srcPosX - padXLeft;
                            posY = srcPosY - padYTop;
                            if (padMode == VX_PAD_REPLICATE)
                            {
                                srcImagePosX = (posX < 0) ? 0 : ((vx_uint32)posX >= srcWidth) ? (srcWidth - 1) : posX;
                                srcImagePosY = (posY < 0) ? 0 : ((vx_uint32)posY >= srcHeight) ? (srcHeight - 1) : posY;
                            }
                            else if (padMode == VX_PAD_MIRROR_SYMMETRIC)
                            {
                                srcImagePosX = (posX < 0) ? (-posX - 1) : ((vx_uint32)posX >= srcWidth) ? (srcWidth - 1 - (posX - srcWidth)) : posX;
                                srcImagePosY = (posY < 0) ? (-posY - 1) : ((vx_uint32)posY >= srcHeight) ? (srcHeight - 1 - (posY - srcHeight)) : posY;
                            }
                            else if (padMode == VX_PAD_MIRROR_REFLECT)
                            {
                                srcImagePosX = (posX < 0) ? (-posX) : ((vx_uint32)posX >= srcWidth) ? (srcWidth - 2 - (posX - srcWidth)) : posX;
                                srcImagePosY = (posY < 0) ? (-posY) : ((vx_uint32)posY >= srcHeight) ? (srcHeight - 2 - (posY - srcHeight)) : posY;
                            }

                            srcData = (vx_uint8*)srcDataAddr + (srcPosZ * srcYSize * srcXSize + srcImagePosY * srcXSize + srcImagePosX) * elementSize;
                            memcpy(dstData, srcData, elementSize);
                        }
                        else
                        {
                            memcpy(dstData, padConst, elementSize);
                        }
                    }
                }
            }
        }
    }
}

vx_char* vxnneGetOperatorTypeName(vxnne_operator_e operationType)
{
    switch (operationType)
    {
    case VXNNE_OPERATOR_CONVOLUTION:
        return "VXNNE_OPERATOR_CONVOLUTION";

    case VXNNE_OPERATOR_RESHUFFLE:
        return "VXNNE_OPERATOR_RESHUFFLE";

    case VXNNE_OPERATOR_FULLYCONNECTED:
        return "VXNNE_OPERATOR_FULLYCONNECTED";

    case VXNNE_OPERATOR_ACTIVATION:
        return "VXNNE_OPERATOR_ACTIVATION";

    case VXNNE_OPERATOR_POOLING:
        return "VXNNE_OPERATOR_POOLING";

    case VXNNE_OPERATOR_RESIZE:
        return "VXNNE_OPERATOR_RESIZE";

    case VXNNE_OPERATOR_TENSOR_ADD:
        return "VXNNE_OPERATOR_TENSOR_ADD";

    case VXNNE_OPERATOR_TENSOR_TRANS:
        return "VXNNE_OPERATOR_TENSOR_TRANS";

    case VXNNE_OPERATOR_SOFTMAX:
        return "VXNNE_OPERATOR_SOFTMAX";

    case VXNNE_OPERATOR_NORMALIZATION:
        return "VXNNE_OPERATOR_NORMALIZATION";

    case VXNNE_OPERATOR_BATCHNORM:
        return "VXNNE_OPERATOR_BATCHNORM";

    case VXNNE_OPERATOR_INPUT2WEIGHT:
        return "VXNNE_OPERATOR_INPUT2WEIGHT";

    case VXNNE_OPERATOR_RPN:
        return "VXNNE_OPERATOR_RPN";

    case VXNNE_OPERATOR_ROIPOOL:
        return "VXNNE_OPERATOR_ROIPOOL";

    case VXNNE_OPERATOR_CONCAT2:
        return "VXNNE_OPERATOR_CONCAT2";

    case VXNNE_OPERATOR_DILATION_RESHUFFLE:
        return "VXNNE_OPERATOR_DILATION_RESHUFFLE";

    case VXNNE_OPERATOR_DILATION_UPSAMPLE:
        return "VXNNE_OPERATOR_DILATION_UPSAMPLE";

    case VXNNE_OPERATOR_DILATION_UPSAMPLE2:
        return "VXNNE_OPERATOR_DILATION_UPSAMPLE2";

    case VXNNE_OPERATOR_DEPTH_WISE_CONV:
        return "VXNNE_OPERATOR_DEPTH_WISE_CONV";
    default:
        return "unkown operation type";
    }
}

vx_char* vxnneGetOperatorTargetName(vxnne_operation_target_e operationTarget)
{
    switch (operationTarget)
    {
    case VXNNE_OPERATION_TARGET_SW:
        return "VXNNE_OPERATION_TARGET_SW";

    case VXNNE_OPERATION_TARGET_NN:
        return "VXNNE_OPERATION_TARGET_NN";

    case VXNNE_OPERATION_TARGET_SH:
        return "VXNNE_OPERATION_TARGET_SH";

    case VXNNE_OPERATION_TARGET_TP:
        return "VXNNE_OPERATION_TARGET_TP";

    case VXNNE_OPERATION_TARGET_SC:
        return "VXNNE_OPERATION_TARGET_SC";

    default:
        return "unkown operation target";
    }
}

vx_char* vxnneGetCacheModeName(vx_enum cacheMode)
{
    switch (cacheMode)
    {
    case VXNNE_SRAM_CACHE_MODE_NONE:
        return "VXNNE_SRAM_CACHE_MODE_NONE";

    case VXNNE_SRAM_CACHE_MODE_PARTIAL_CACHE:
        return "VXNNE_SRAM_CACHE_MODE_PARTIAL_CACHE";

    case VXNNE_SRAM_CACHE_MODE_FULL_CACHE:
        return "VXNNE_SRAM_CACHE_MODE_FULL_CACHE";

    case VXNNE_SRAM_CACHE_MODE_STREAM_CACHE:
        return "VXNNE_SRAM_CACHE_MODE_STREAM_CACHE";

    default:
        return "unkown cache mode";
    }
}

void _TransposeTensor(vx_uint8_ptr in_addr, vx_uint8_ptr out_addr, vx_uint32 data_size, vx_uint32_ptr dims, vx_uint32_ptr strides, vx_uint32_ptr trans_strides, vx_uint32* perm, vx_uint32 layer)
{
    vx_uint32 dim_stack[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0};
    vx_uint8_ptr in_addr_stack[VX_CONTEXT_TENSOR_MAX_DIMENSION], out_addr_stack[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint8_ptr in_addr_tmp, out_addr_tmp;
    vx_uint32 current = 0;
    vx_bool back = vx_false_e;

    in_addr_tmp = in_addr;
    out_addr_tmp = out_addr;

    for (;;)
    {
        in_addr_stack[current] = in_addr_tmp;
        out_addr_stack[current] = out_addr_tmp;

        if (layer == 1)
        {
            vx_uint32 x, y;
            vx_uint8_ptr new_out_addr = out_addr_tmp;
            for (y = 0; y < dims[perm[1]]; y++)
            {
                for (x = 0; x < dims[perm[0]]; x++)
                {
                    vx_uint8_ptr new_in_addr = in_addr_tmp + (y * strides[perm[1]] + x * strides[perm[0]]);
                    memcpy(new_out_addr, new_in_addr, data_size);
                    new_out_addr += data_size;
                }
            }

            if (!current) break;
            current--;
            layer++;
            back = vx_true_e;
        }
        else if (!back)
        {
            current++;
            layer--;
        }
        else
        {
            dim_stack[current]++;
            if (dim_stack[current] < dims[perm[layer]])
            {
                in_addr_tmp += strides[perm[layer]];
                out_addr_tmp += trans_strides[layer];
                back = vx_false_e;
            }
            else
            {
                dim_stack[current] = 0;
                if (!current) break;
                current--;
                layer++;
                in_addr_tmp = in_addr_stack[current];
                out_addr_tmp = out_addr_stack[current];
            }
        }
    }
}

vx_float64 vxnneGetData64(vx_enum format, vx_int32 index, vx_uint8_ptr src, vx_int8 fixPointPos)
{
    vx_float64 value = 0;
    vx_int8 fpPos = fixPointPos;

    switch(format)
    {
    case VX_TYPE_INT8:
        {
            vx_int8_ptr data_ptr = (vx_int8*)src;
            value = Int8toFp32(data_ptr[index], fpPos);
        }
        break;
    case VX_TYPE_UINT8:
        {
            vx_uint8_ptr data_ptr = (vx_uint8*)src;
            value = UchartoFp32(data_ptr[index], fpPos);
        }
        break;
    case VX_TYPE_FLOAT32:
        {
        vx_float32_ptr data_ptr = (vx_float32_ptr)src;
        value = (vx_float64)data_ptr[index];
        }
        break;
    case VX_TYPE_FLOAT16:
        {
        vx_int16_ptr data_ptr = (vx_int16_ptr)src;
        value = (vx_float64)Fp16toFp32(data_ptr[index]);
        }
        break;
    case VX_TYPE_INT16:
        {
            vx_int16_ptr data_ptr = (vx_int16_ptr)src;
            value = (vx_float64)Int16toFp32(data_ptr[index], fpPos);
        }
        break;
    default:
        vxError("Not support format :%d\n", format);
        break;
    }
    return value;
}

vx_float64 vxnneSaturate(vx_float64 data, vx_enum format)
{
    vx_float64 value = data;

    switch(format)
    {
    case VX_TYPE_FLOAT32:
        SATURATE_SIGN(FP32);
        break;
    case VX_TYPE_FLOAT16:
        SATURATE_SIGN(FP16);
        break;
    case VX_TYPE_INT32:
        SATURATE_SIGN(INT32);
        break;
    case VX_TYPE_INT16:
        SATURATE_SIGN(INT16);
        break;
    case VX_TYPE_INT8:
        SATURATE_SIGN(INT8);
        break;
    case VX_TYPE_UINT32:
        SATURATE_UNSIGN(UINT32);
        break;
    case VX_TYPE_UINT16:
        SATURATE_UNSIGN(UINT16);
        break;
    case VX_TYPE_UINT8:
        SATURATE_UNSIGN(UINT8);
        break;
    default:
        vxError("Not support format :%d\n", format);
        break;
    }

    return value;
}

vx_float64 vxnneWarp(vx_float64 data, vx_enum format)
{
    vx_float64 value = 0;

    switch(format)
    {
    case VX_TYPE_INT64:
        value = (vx_float64)data;
        break;
    case VX_TYPE_FLOAT32:
        value = (vx_float32)data;
        break;
    case VX_TYPE_FLOAT16:
        value = (vx_float32)data;
        break;
    case VX_TYPE_INT32:
        value = (vx_int32)data;
        break;
    case VX_TYPE_INT16:
        value = (vx_int16)data;
        break;
    case VX_TYPE_INT8:
        value = (vx_int8)data;
        break;
    case VX_TYPE_UINT32:
        value = (vx_uint32)data;
        break;
    case VX_TYPE_UINT16:
        value = (vx_uint16)data;
        break;
    case VX_TYPE_UINT8:
        value = (vx_uint8)data;
        break;
    default:
        vxError("Not support format :%d\n", format);
        break;
    }

    return value;
}

VX_PRIVATE_API vx_uint32 _GetTensorOffset(vx_uint32 index, vx_tensor inputTensor, vx_uint32 * out_dims)
{
    vx_uint32 offset = 0;
    vx_uint32 i;

    for(i = 0; i < inputTensor->dimCount; i++)
    {
        offset += inputTensor->strides[i] * (index % out_dims[i]);

        index /= out_dims[i];
    }

    return offset;
}

VX_PRIVATE_API vx_uint32 _GetExpandTensorOffset(vx_uint32 index, vx_tensor inputTensor, vx_uint32 * out_dims)
{
    vx_uint32 offset = 0;
    vx_uint32 i;

    for(i = 0; i < inputTensor->dimCount; i++)
    {
        if(inputTensor->dims[i] == out_dims[i])
            offset += inputTensor->strides[i] * (index % out_dims[i]);

        index /= out_dims[i];
    }

    return offset;
}

vx_status eltwise(
    vx_tensor input1,
    vx_tensor input2,
    vx_float32 scale,
    vx_enum overflow,
    vx_enum scaleRounding,
    vx_enum operation,
    vx_tensor output)
{
    vx_int32 i = 0;


    vx_int32 size = TENSOR_SIZE_INDEX(output, 0) * TENSOR_SIZE_INDEX(output, 1) * TENSOR_SIZE_INDEX(output, 2);

    vx_int8 input1FixPointPos = TENSOR_POS(input1);
    vx_int8 input2FixPointPos = TENSOR_POS(input2);
    vx_int8 outputFixPointPos = TENSOR_POS(output);

    vx_enum input1Format = TENSOR_DATA_TYPE(input1);
    vx_enum input2Format = TENSOR_DATA_TYPE(input2);
    vx_enum outputFormat = TENSOR_DATA_TYPE(output);

    vx_float32 in1_tf_scale = TENSOR_TF_SCALE(input1);
    vx_int32 in1_tf_zeroPoint = TENSOR_TF_ZEROPOINT(input1);
    vx_enum in1_quant_format = TENSOR_QUANT_TYPE(input1);

    vx_float32 in2_tf_scale = TENSOR_TF_SCALE(input2);
    vx_int32 in2_tf_zeroPoint = TENSOR_TF_ZEROPOINT(input2);
    vx_enum in2_quant_format = TENSOR_QUANT_TYPE(input2);

    vx_float32 out_tf_scale = TENSOR_TF_SCALE(output);
    vx_int32 out_tf_zeroPoint = TENSOR_TF_ZEROPOINT(output);
    vx_enum out_quant_format = TENSOR_QUANT_TYPE(output);

    vx_enum outputRoundingMode = TENSOR_ROUNDING_MODE(output);

    for (i = 0; i < size; i ++)
    {
        vx_uint32 in1offset, in2offset, outoffset;
        vx_float64 input1_value;
        vx_float64 input2_value;
        vx_uint8_ptr input1_ptr;
        vx_uint8_ptr input2_ptr;
        vx_uint8_ptr output_ptr;
        vx_float64 value = 0;

        in1offset = _GetExpandTensorOffset(i, input1, output->dims);
        in2offset = _GetExpandTensorOffset(i, input2, output->dims);
        outoffset = _GetTensorOffset(i, output, output->dims);
        input1_ptr = TENSOR_LOGICAL_ADDR(input1) + in1offset;
        input2_ptr = TENSOR_LOGICAL_ADDR(input2) + in2offset;
        output_ptr = TENSOR_LOGICAL_ADDR(output) + outoffset;

        input1_value = vxnneGetDataExt64((vx_type_e)input1Format, in1_quant_format, i, input1_ptr, input1FixPointPos, in1_tf_zeroPoint, in1_tf_scale);
        input2_value = vxnneGetDataExt64((vx_type_e)input2Format, in2_quant_format, i, input2_ptr, input2FixPointPos, in2_tf_zeroPoint, in2_tf_scale);

        switch(operation)
        {
        case VX_TENSOR_OP_ADD:
            value = input1_value + input2_value;
            break;
        case VX_TENSOR_OP_SUB:
            value = input1_value - input2_value;
            break;
        case VX_TENSOR_OP_MUL:
            if (outputFormat == VX_TYPE_INT8 || outputFormat == VX_TYPE_INT32)
            {
                value = vxnneRound((vx_float32)(input1_value * input2_value) * scale, scaleRounding);
            }
            else
            {
            value = (input1_value * input2_value) * scale;
            }
            break;
        }

        switch(overflow)
        {
        case VX_CONVERT_POLICY_SATURATE:
            value = vxnneSaturate(value, outputFormat);
            break;
        case VX_CONVERT_POLICY_WRAP:
            value = vxnneWarp(value, outputFormat);
            break;
        default:
            vxError("Not support format :%d\n", outputFormat);
            break;
        }

        vxnneSaveDataExt((vx_type_e)outputFormat, out_quant_format, i, value, output_ptr, outputFixPointPos, out_tf_zeroPoint, out_tf_scale, outputRoundingMode);
    }

    return VX_SUCCESS;
}

void vx_nn_rpn_qsort_box(vx_float32_ptr box, vx_int32 start, vx_int32 end, vx_int32 num_top)
{
    /*
        box[x] = {x1, y1, x2, y2, score};
    */
    int i;
    vx_float32 pivot_score = box[start * 5 + 4];
    vx_int32 left = start + 1, right = end;
    vx_float32 temp[5];

    while (left <= right)
    {
        while(left <= end && box[left * 5 + 4] >= pivot_score)
            ++left;

        while (right > start && box[right * 5 + 4] <= pivot_score)
            --right;

        if (left <= right)
        {
            /* swap box */
            for(i = 0; i < 5; ++i)
            {
                temp[i] = box[left * 5 + i];
            }
            for(i = 0; i < 5; ++i)
            {
                box[left * 5 + i] = box[right * 5 + i];
            }
            for(i = 0; i < 5; ++i)
            {
                box[right * 5 + i] = temp[i];
            }

            ++left;
            --right;
        }
    }

    if (right > start)
    {
        for(i = 0; i < 5; ++i)
        {
            temp[i] = box[start * 5 + i];
        }

        for(i = 0; i < 5; ++i)
        {
            box[start * 5 + i] = box[right * 5 + i];
        }

        for(i = 0; i < 5; ++i)
        {
            box[right * 5 + i] = temp[i];
        }
    }

    if(start < right - 1)
    {
        vx_nn_rpn_qsort_box(box, start, right - 1, num_top);
    }
    if(right + 1 < num_top && right + 1 < end)
    {
        vx_nn_rpn_qsort_box(box, right + 1, end, num_top);
    }

}
void vx_nn_rpn_qsort_box_fp16(vx_int16_ptr box, vx_int32 start, vx_int32 end, vx_int32 num_top)
{
    /*
        box[x] = {x1, y1, x2, y2, score};
    */
    int i;
    vx_float32 pivot_score = Fp16toFp32(box[start * 5 + 4]);
    vx_int32 left = start + 1, right = end;
    vx_int16 temp[5];

    while (left <= right)
    {
        while(left <= end && Fp16toFp32(box[left * 5 + 4]) >= pivot_score)
            ++left;

        while (right > start && Fp16toFp32(box[right * 5 + 4]) <= pivot_score)
            --right;

        if (left <= right)
        {
            /* swap box */
            for(i = 0; i < 5; ++i)
            {
                temp[i] = box[left * 5 + i];
            }
            for(i = 0; i < 5; ++i)
            {
                box[left * 5 + i] = box[right * 5 + i];
            }
            for(i = 0; i < 5; ++i)
            {
                box[right * 5 + i] = temp[i];
            }

            ++left;
            --right;
        }
    }

    if (right > start)
    {
        for(i = 0; i < 5; ++i)
        {
            temp[i] = box[start * 5 + i];
        }

        for(i = 0; i < 5; ++i)
        {
            box[start * 5 + i] = box[right * 5 + i];
        }

        for(i = 0; i < 5; ++i)
        {
            box[right * 5 + i] = temp[i];
        }
    }

    if(start < right - 1)
    {
        vx_nn_rpn_qsort_box_fp16(box, start, right - 1, num_top);
    }
    if(right + 1 < num_top && right + 1 < end)
    {
        vx_nn_rpn_qsort_box_fp16(box, right + 1, end, num_top);
    }

}
int vx_nn_rpn_transform_box(
    vx_float32_ptr box,
    vx_float32 dx, vx_float32 dy,
    vx_float32 d_log_w, vx_float32 d_log_h,
    vx_float32 img_W, vx_float32 img_H,
    vx_float32 min_box_W, vx_float32 min_box_H
    )
{
    vx_float32 w,h,ctr_x,ctr_y,pred_ctr_x,pred_ctr_y,pred_w,pred_h,box_w,box_h;

    /* width & height of box */
    w = box[2] - box[0] + 1.0f;
    h = box[3] - box[1] + 1.0f;

    /* center location of box */
    ctr_x = box[0] + 0.5f * w;
    ctr_y = box[1] + 0.5f * h;

    /* new center location according to gradient(dx, dy) */
    pred_ctr_x = dx * w + ctr_x;
    pred_ctr_y = dy * h + ctr_y;

    /* new width & height according to gradient d(log w), d(log h) */
    pred_w = expf(d_log_w) * w;
    pred_h = expf(d_log_h) * h;

    /* update upper-left corner location */
    box[0] = pred_ctr_x - 0.5f * pred_w;
    box[1] = pred_ctr_y - 0.5f * pred_h;

    /* update lower-right corner location */
    box[2] = pred_ctr_x + 0.5f * pred_w;
    box[3] = pred_ctr_y + 0.5f * pred_h;

    /* adjust new corner locations to be within the image region */
    box[0] = gcmMAX(0.0f, gcmMIN(box[0], img_W - 1.0f) );
    box[1] = gcmMAX(0.0f, gcmMIN(box[1], img_H - 1.0f) );
    box[2] = gcmMAX(0.0f, gcmMIN(box[2], img_W - 1.0f) );
    box[3] = gcmMAX(0.0f, gcmMIN(box[3], img_H - 1.0f) );

    /* recompute new width & height */
    box_w = box[2] - box[0] + 1.0f;
    box_h = box[3] - box[1] + 1.0f;

    /* check if new box's size >= threshold */
    return (box_w >= min_box_W) * (box_h >= min_box_H);
}
vx_float32 vx_nn_rpn_iou_cpu_f16(vx_int16_ptr a, vx_int16_ptr b)
{
    vx_float32 x1,y1,x2,y2,width,height,area,A_area,B_area;
    vx_float32 A[4], B[4];

    A[0] = Fp16toFp32(a[0]);
    A[1] = Fp16toFp32(a[1]);
    A[2] = Fp16toFp32(a[2]);
    A[3] = Fp16toFp32(a[3]);

    B[0] = Fp16toFp32(b[0]);
    B[1] = Fp16toFp32(b[1]);
    B[2] = Fp16toFp32(b[2]);
    B[3] = Fp16toFp32(b[3]);

    if (A[0] > B[2] || A[1] > B[3] || A[2] < B[0] || A[3] < B[1])
    {
        return 0;
    }

    /* overlapped region (=box) */
    x1 = gcmMAX(A[0], B[0]);
    y1 = gcmMAX(A[1], B[1]);
    x2 = gcmMIN(A[2], B[2]);
    y2 = gcmMIN(A[3], B[3]);

    /* intersection area */
    width    = gcmMAX(0.0f, x2 - x1 + 1.0f);
    height   = gcmMAX(0.0f, y2 - y1 + 1.0f);
    area     = width * height;

    /* area of A, B */
    A_area   = (A[2] - A[0] + 1.0f) * (A[3] - A[1] + 1.0f);
    B_area   = (B[2] - B[0] + 1.0f) * (B[3] - B[1] + 1.0f);

    /* IOU */
    return area / (A_area + B_area - area);
}
vx_float32 vx_nn_rpn_iou_cpu(vx_float32_ptr A, vx_float32_ptr B)
{
    vx_float32 x1,y1,x2,y2,width,height,area,A_area,B_area;

    if (A[0] > B[2] || A[1] > B[3] || A[2] < B[0] || A[3] < B[1])
    {
      return 0;
    }

    /* overlapped region (=box) */
    x1 = gcmMAX(A[0], B[0]);
    y1 = gcmMAX(A[1], B[1]);
    x2 = gcmMIN(A[2], B[2]);
    y2 = gcmMIN(A[3], B[3]);

    /* intersection area */
    width    = gcmMAX(0.0f, x2 - x1 + 1.0f);
    height   = gcmMAX(0.0f, y2 - y1 + 1.0f);
    area     = width * height;

    /* area of A, B */
    A_area   = (A[2] - A[0] + 1.0f) * (A[3] - A[1] + 1.0f);
    B_area   = (B[2] - B[0] + 1.0f) * (B[3] - B[1] + 1.0f);

    /* IOU */
    return area / (A_area + B_area - area);
}
vx_bool vx_nn_rpn_iou_cpu1(vx_float32_ptr A, vx_float32_ptr B,vx_float32 nms_thresh)
{
    vx_float32 x1,y1,x2,y2,width,height,area,A_area,B_area;

    if (A[0] > B[2] || A[1] > B[3] || A[2] < B[0] || A[3] < B[1])
    {
      return 0;
    }

    /* overlapped region (=box) */
    x1 = gcmMAX(A[0], B[0]);
    y1 = gcmMAX(A[1], B[1]);
    x2 = gcmMIN(A[2], B[2]);
    y2 = gcmMIN(A[3], B[3]);

    /* intersection area */
    width    = gcmMAX(0.0f, x2 - x1 + 1.0f);
    height   = gcmMAX(0.0f, y2 - y1 + 1.0f);
    area     = width * height;

    /* area of A, B */
    A_area   = (A[2] - A[0] + 1.0f) * (A[3] - A[1] + 1.0f);
    B_area   = (B[2] - B[0] + 1.0f) * (B[3] - B[1] + 1.0f);

    /* IOU */
    return (vx_bool)(area > (A_area + B_area - area)*nms_thresh);
}
void vx_nn_rpn_nms_cpu_f16(vx_uint32 num_boxes,
                              vx_int16_ptr boxes,
                              vx_uint32_ptr index_out,
                              vx_uint32_ptr num_out,
                              vx_int32 base_index,
                              vx_float32 nms_thresh,
                              vx_uint32 max_num_out)
{
    vx_uint32 i,j,count = 0;
    vx_char_ptr is_dead = (vx_char_ptr)vxAllocateAndZeroMemory(num_boxes * sizeof(vx_char));
    gcoOS_MemFill(is_dead, 0, (num_boxes * sizeof(vx_char)));

    for(i = 0; i < num_boxes; ++i)
    {
        if(is_dead[i])
        {
            continue;
        }

        index_out[count++] = base_index + i;
        if(count == max_num_out)
        {
            break;
        }

        for(j = i + 1; j < num_boxes; ++j)
        {
            if(!is_dead[j] && vx_nn_rpn_iou_cpu_f16(&boxes[i * 5], &boxes[j * 5]) > nms_thresh)
            {
                is_dead[j] = 1;
            }
        }
    }

    *num_out = count;
    vxFree(is_dead);
}

void vx_nn_rpn_nms_cpu(
    vx_uint32 num_boxes,
    vx_float32_ptr boxes,
    vx_uint32_ptr index_out,
    vx_uint32_ptr num_out,
    vx_int32 base_index,
    vx_float32 nms_thresh,
    vx_uint32 max_num_out
    )
{
    if(index_out!=NULL)
    {
        vx_uint32 i,j,count = 0;
        vx_char_ptr is_dead = (vx_char_ptr)vxAllocateAndZeroMemory(num_boxes * sizeof(vx_char));
        memset(is_dead, 0, (num_boxes * sizeof(vx_char)));

        for(i = 0; i < num_boxes; ++i)
        {
            if(is_dead[i])
            {
                continue;
            }

            index_out[count++] = base_index + i;
            if(count == max_num_out)
            {
                break;
            }

            for(j = i + 1; j < num_boxes; ++j)
            {
                if(!is_dead[j] && vx_nn_rpn_iou_cpu(&boxes[i * 5], &boxes[j * 5]) > nms_thresh)
                {
                    is_dead[j] = 1;
                }
            }
        }

        *num_out = count;
        vxFree(is_dead);
    }
    else
    {
        vx_uint32 i,j,count = 0;
        vx_bool is_dead ;
        int call_cnt = 0;
        count = 1;

        for(i = 1; i < num_boxes; i++)
        {
            is_dead = vx_false_e;
            for(j=0;j<count;j++)
            {
                call_cnt ++;
                if(vx_nn_rpn_iou_cpu1(&boxes[j * 5], &boxes[i * 5], nms_thresh))
                {
                    is_dead = vx_true_e;
                    break; //boxes[i * 5] pk dead
                }

            }
            if(is_dead)
                continue;
            //update boxes
            boxes[count*5+0] = boxes[i * 5+0];
            boxes[count*5+1] = boxes[i * 5+1];
            boxes[count*5+2] = boxes[i * 5+2];
            boxes[count*5+3] = boxes[i * 5+3];
            boxes[count*5+4] = boxes[i * 5+4];
            count++;
            if(count == max_num_out)
            {
                break;
            }
        }

        *num_out = count;
   }
}

vx_status vxnnePoolingMax(
    vx_uint8_ptr src,
    vx_int8 srcFixPointPos,
    vx_type_e srcFormat,
    vx_int32 input_width,
    vx_int32 input_height,
    vx_int32 batch,
    vx_int32 depth,
    vx_int32_ptr output_width,
    vx_int32_ptr output_height,
    vx_int32 stride_x,
    vx_int32 stride_y,
    vx_int32 kernel_size_x,
    vx_int32 kernel_size_y,
    vx_uint32 padXLeft,
    vx_uint32 padXRight,
    vx_uint32 padYTop,
    vx_uint32 padYBottom,
    vx_enum rounding,
    vx_uint8_ptr dst,
    vx_int8 dstFixPointPos,
    vx_enum dstRoundingMode,
    vx_type_e dstFormat,
    vx_type_e srcQuantFormat,
    vx_type_e dstQuantFormat,
    vx_int32 inputZP,
    vx_float32 inputScale,
    vx_int32 outputZP,
    vx_float32 outputScale)
{
    vx_uint8_ptr data = 0;
    vx_uint8_ptr data_d = 0;
    vx_int32 i = 0, j = 0, p = 0, b = 0;

    vx_int32 width = input_width;
    vx_int32 height = input_height;
    vx_int32 depth_v = depth;
    vx_int32 stride_x_v = stride_x;
    vx_int32 stride_y_v = stride_y;
    vx_int32 kernel_x_v = kernel_size_x;
    vx_int32 kernel_y_v = kernel_size_y;
    vx_int32 width_o = (vx_uint32)(vxoNNExternsionConvlutionRound(((vx_float32)(width + padXLeft + padXRight - kernel_x_v)/stride_x_v + 1), rounding));
    vx_int32 height_o = (vx_uint32)(vxoNNExternsionConvlutionRound(((vx_float32)(height + padYTop + padYBottom - kernel_y_v)/stride_y_v + 1), rounding));

    *output_width = width_o;
    *output_height = height_o;

    data = src;
    data_d = dst;

    for(b = 0; b < batch; b++)
    {

        for (p = 0; p < depth_v; p ++)
        {
            for (j = 0; j < height_o; j ++)
            {
                for (i = 0; i < width_o; i ++)
                {
                    vx_int32 pad_h = padYTop, pad_w = padXLeft;
                    vx_int32 hstart = j * stride_y_v - pad_h;
                    vx_int32 wstart = i * stride_x_v - pad_w;
                    vx_int32 hend = gcmMIN(hstart + kernel_y_v, height);
                    vx_int32 wend = gcmMIN(wstart + kernel_x_v, width);
                    vx_int32 pool_index = 0;
                    vx_int32 h, w = 0;
                    vx_float32 d_f32 = -FP32_MAX;

                    hstart = gcmMAX(hstart, 0);
                    wstart = gcmMAX(wstart, 0);

                    pool_index = j * (width_o) + i;

                    for (h = hstart; h < hend; ++ h)
                    {
                        for (w = wstart; w < wend; ++ w)
                        {
                            const vx_int32 index = h * (width) + w;
                            vx_float32 d;

                            d = vxnneGetDataExt(srcFormat, srcQuantFormat, index, (vx_uint8_ptr)data, srcFixPointPos, inputZP, inputScale);
                            if (d > d_f32)
                                d_f32 = d;
                        }
                    }

                    vxnneSaveDataExt(dstFormat, dstQuantFormat, pool_index, d_f32, data_d, dstFixPointPos, outputZP, outputScale, dstRoundingMode);
                }
            }

            data += width * height * vxnneGetTypeSize(srcFormat);
            data_d += width_o * height_o * vxnneGetTypeSize(dstFormat);
        }
    }

    return VX_SUCCESS;
}

vx_status vxnnePoolingAvg(
    vx_uint8_ptr src,
    vx_int8 srcFixPointPos,
    vx_type_e srcFormat,
    vx_int32 input_width,
    vx_int32 input_height,
    vx_int32 batch,
    vx_int32 depth,
    vx_int32_ptr output_width,
    vx_int32_ptr output_height,
    vx_int32 stride_x,
    vx_int32 stride_y,
    vx_int32 kernel_size_x,
    vx_int32 kernel_size_y,
    vx_uint32 padXLeft,
    vx_uint32 padXRight,
    vx_uint32 padYTop,
    vx_uint32 padYBottom,
    vx_enum rounding,
    vx_uint8_ptr dst,
    vx_int8 dstFixPointPos,
    vx_enum dstRoundingMode,
    vx_type_e dstFormat,
    vx_type_e srcQuantFormat,
    vx_type_e dstQuantFormat,
    vx_int32 inputZP,
    vx_float32 inputScale,
    vx_int32 outputZP,
    vx_float32 outputScale)
{
    vx_uint8_ptr data = 0;
    vx_uint8_ptr data_d = 0;
    vx_int32 i = 0, j = 0, p = 0, b = 0;

    vx_int32 width = input_width;
    vx_int32 height = input_height;
    vx_int32 depth_v = depth;
    vx_int32 stride_x_v = stride_x;
    vx_int32 stride_y_v = stride_y;
    vx_int32 kernel_x_v = kernel_size_x;
    vx_int32 kernel_y_v = kernel_size_y;
    vx_int32 width_o = (vx_uint32)(vxoNNExternsionConvlutionRound(((vx_float32)(width + padXLeft + padXRight - kernel_x_v)/stride_x_v), rounding) + 1);
    vx_int32 height_o = (vx_uint32)(vxoNNExternsionConvlutionRound(((vx_float32)(height + padYTop + padYBottom - kernel_y_v)/stride_y_v), rounding) + 1);

    *output_width = width_o;
    *output_height = height_o;

    data = src;
    data_d = dst;

    for (b = 0; b < batch; b++)
    {

        for (p = 0; p < depth_v; p ++)
        {
            for (j = 0; j < height_o; j ++)
            {
                for (i = 0; i < width_o; i ++)
                {
                    vx_int32 pad_h = padYTop, pad_w = padXLeft;
                    vx_int32 hstart = j * stride_y_v - pad_h;
                    vx_int32 wstart = i * stride_x_v - pad_w;
                    vx_int32 hend = gcmMIN(hstart + kernel_y_v, height);
                    vx_int32 wend = gcmMIN(wstart + kernel_x_v, width);
                    vx_int32 pool_index = 0;
                    vx_int32 h, w = 0, count = 0;
                    vx_float32 sum = 0;

                    hstart = gcmMAX(hstart, 0);
                    wstart = gcmMAX(wstart, 0);

                    pool_index = j * (width_o) + i;

                    for (h = hstart; h < hend; ++ h)
                    {
                        for (w = wstart; w < wend; ++ w)
                        {
                            const vx_int32 index = h * (width) + w;

                            sum += vxnneGetDataExt(srcFormat, srcQuantFormat, index, (vx_uint8_ptr)data, srcFixPointPos, inputZP, inputScale);
                            count ++;
                        }
                    }

                    vxnneSaveDataExt(dstFormat, dstQuantFormat, pool_index, sum/count, data_d, dstFixPointPos, outputZP, outputScale, dstRoundingMode);
                }

            }

            data += width * height * vxnneGetTypeSize(srcFormat);
            data_d += width_o * height_o * vxnneGetTypeSize(dstFormat);
        }
    }

    return VX_SUCCESS;
}

vx_status vxnnePoolingCpu(
    vx_uint8_ptr src,
    vx_int8 srcFixPointPos,
    vx_int32 type,
    vx_type_e srcFormat,
    vx_int32 input_width,
    vx_int32 input_height,
    vx_int32 batch,
    vx_int32 depth,
    vx_int32_ptr output_width,
    vx_int32_ptr output_height,
    vx_int32 stride_x,
    vx_int32 stride_y,
    vx_int32 kernel_size_x,
    vx_int32 kernel_size_y,
    vx_uint32 padXLeft,
    vx_uint32 padXRight,
    vx_uint32 padYTop,
    vx_uint32 padYBottom,
    vx_enum rounding,
    vx_uint8_ptr dst,
    vx_int8 dstFixPointPos,
    vx_enum dstRoundingMode,
    vx_type_e dstFormat,
    vx_type_e srcQuantFormat,
    vx_type_e dstQuantFormat,
    vx_int32 inputZP,
    vx_float32 inputScale,
    vx_int32 outputZP,
    vx_float32 outputScale)
{
    switch (type)
    {
        case 1:
            vxnnePoolingMax(src,
                            srcFixPointPos,
                            srcFormat,
                            input_width,
                            input_height,
                            batch,
                            depth,
                            output_width,
                            output_height,
                            stride_x,
                            stride_y,
                            kernel_size_x,
                            kernel_size_y,
                            padXLeft,
                            padXRight,
                            padYTop,
                            padYBottom,
                            rounding,
                            dst,
                            dstFixPointPos,
                            dstRoundingMode,
                            dstFormat,
                            srcQuantFormat,
                            dstQuantFormat,
                            inputZP,
                            inputScale,
                            outputZP,
                            outputScale);
        break;
        case 2:
            vxnnePoolingAvg(src,
                            srcFixPointPos,
                            srcFormat,
                            input_width,
                            input_height,
                            batch,
                            depth,
                            output_width,
                            output_height,
                            stride_x,
                            stride_y,
                            kernel_size_x,
                            kernel_size_y,
                            padXLeft,
                            padXRight,
                            padYTop,
                            padYBottom,
                            rounding,
                            dst,
                            dstFixPointPos,
                            dstRoundingMode,
                            dstFormat,
                            srcQuantFormat,
                            dstQuantFormat,
                            inputZP,
                            inputScale,
                            outputZP,
                            outputScale);
        break;
    }

    return VX_SUCCESS;
}

vx_status vxnneCalculateConvTilingParam(
    vx_context                                context,
    vxnne_convolution_relu_pooling_operation  conv_op,
    vxnne_tiling_info                         info,
    vx_uint8                                  inputSRAM,
    vx_uint8                                  outputSRAM,
    vx_uint32                                 count,
    vx_uint32                                 vipSize
    )
{
    vx_uint32 i, minKPK=0xFFFFFFFF, imode = 0;
    vx_arch_perf_s perf;

    for (i = 0; i < count; i++)
    {
        vx_uint32 inputDims[3] = {info[i].input.width, info[i].input.height, TENSOR_SIZE_INDEX(conv_op->inputs, 2)};
        vx_uint32 outputDims[3] = {info[i].output.width, info[i].output.height, TENSOR_SIZE_INDEX(conv_op->outputs, 2)};
        vx_int32 offsets[2] = {(-1)*info[i].padLeft, (-1)*info[i].padTop};

        if (info[i].output.height == 0 ) continue;

        if (conv_op->weights_biases->wb_base->do_zdp_opt &&
            context->options.do1xnAfterSwtiling)
        {
            vx_uint32 fitN = 1;
            vx_bool doZdpOpt = vx_false_e;

            if (inputDims[1] == TENSOR_SIZE_INDEX(conv_op->orig_inputs, 1))
            {
                doZdpOpt = calcFitZdp3N(context,
                                        inputDims[0],
                                        inputDims[1],
                                        &fitN,
                                        1,
                                        conv_op->weights_biases->wb_base->pooling_size_x);

                if (doZdpOpt)
                {
                    /* Need reshape input[x, y, kz] --> [x*y/fitN, fitN, kz] */
                    /* Need reshape output[x, y, vz] --> [x*y/fitN, fitN, vz] */
                    inputDims[0] = inputDims[0] * inputDims[1] / fitN;
                    inputDims[1] = fitN;

                    outputDims[0]= outputDims[0] * outputDims[1] / fitN;
                    outputDims[1] = fitN;
                }
            }

            conv_op->weights_biases->wb_base->do_zdp_opt = doZdpOpt;

        }

        memset(&perf, 0, sizeof(vx_arch_perf_s));

        perf.calculated = vx_false_e;
        perf.swTilingInfo.origInX = TENSOR_SIZE_INDEX(conv_op->inputs, 0);
        perf.swTilingInfo.origInY = TENSOR_SIZE_INDEX(conv_op->inputs, 1);
        perf.swTilingInfo.origOutX = TENSOR_SIZE_INDEX(conv_op->outputs, 0);
        perf.swTilingInfo.origOutY = TENSOR_SIZE_INDEX(conv_op->outputs, 1);
        perf.swTilingInfo.srcBuf = inputSRAM ;
        perf.swTilingInfo.dstBuf = outputSRAM;
        perf.swTilingInfo.kernelBuf = SW_TILING_FROM_VIP_SRAM;
        perf.swTilingInfo.outImageStride = conv_op->outputs->strides[1];
        perf.swTilingInfo.outImageSlice = conv_op->outputs->strides[1] * info[i].output.height;
        calculateArchPerfFromWB(context,
                                &perf,
                                conv_op->weights_biases,
                                inputDims,
                                outputDims,
                                offsets,
                                1,
                                perf.swTilingInfo.srcBuf, perf.swTilingInfo.dstBuf, perf.swTilingInfo.kernelBuf,
                                vipSize,
                                VXNNE_OPERATION_TARGET_NN,
                                VXNNE_OPERATOR_CONVOLUTION);

        info[i].tilingParam.outImageTileXSize = perf.resultInfo.outImageTileXSize;
        info[i].tilingParam.outImageTileYSize = perf.resultInfo.outImageTileYSize;
        info[i].tilingParam.interleaveMode    = perf.resultInfo.interleaveMode;
        info[i].tilingParam.nnCoreCount       = perf.resultInfo.nnCoreCount;
        if (perf.resultInfo.kernelsPerCore < minKPK)
        {
            minKPK = perf.resultInfo.kernelsPerCore;
            imode = perf.resultInfo.interleaveMode;
        }
    }

    for (i = 0; i < count; i++)
    {
        info[i].tilingParam.kernelsPerCore = minKPK;
        info[i].tilingParam.interleaveMode = imode;
    }

    return VX_SUCCESS;
}

vx_status vxnneOperation_InitializeCommand(
    vx_context context,
    vx_graph graph,
    vxnne_operation operation,
    vxnne_operation_command_s * command
    )
{
    vx_status status = VX_SUCCESS;

    vxnne_tiling_rect input  = &command->inputTile;
    vxnne_tiling_rect output = &command->outputTile;

    if (operation->target != VXNNE_OPERATION_TARGET_NN &&
        operation->target != VXNNE_OPERATION_TARGET_TP)
    {
        return VX_SUCCESS;
    }

    input->sRAM = vx_false_e;
    input->circleBufferSize = 0;
    input->circularBufEndAddrPlus1 = 0xFFFFFFFF;

    output->sRAM = vx_false_e;
    output->circleBufferSize = 0;
    output->circularBufEndAddrPlus1 = 0xFFFFFFFF;

    if (operation->target == VXNNE_OPERATION_TARGET_TP)
    {
        vxnne_tp_operation op = (vxnne_tp_operation)operation;

        input->width   = TENSOR_VIEW_SIZE_INDEX(op->input, 0);
        input->height  = TENSOR_VIEW_SIZE_INDEX(op->input, 1);
        input->yStride = TENSOR_STRIDE_INDEX(op->input, 1);
        input->zStride = TENSOR_STRIDE_INDEX(op->input, 2);

        output->width   = TENSOR_VIEW_SIZE_INDEX(op->output, 0);
        output->height  = TENSOR_VIEW_SIZE_INDEX(op->output, 1);
        output->yStride = TENSOR_STRIDE_INDEX(op->output, 1);
        output->zStride = TENSOR_STRIDE_INDEX(op->output, 2);
    }
    else if (operation->target == VXNNE_OPERATION_TARGET_NN)
    {
        vxnne_convolution_relu_pooling_operation op = (vxnne_convolution_relu_pooling_operation)operation;

        input->width   = TENSOR_VIEW_SIZE_INDEX(op->inputs, 0);
        input->height  = TENSOR_VIEW_SIZE_INDEX(op->inputs, 1);
        input->yStride = TENSOR_STRIDE_INDEX(op->inputs, 1);
        input->zStride = TENSOR_STRIDE_INDEX(op->inputs, 2);

        output->width   = TENSOR_VIEW_SIZE_INDEX(op->outputs, 0);
        output->height  = TENSOR_VIEW_SIZE_INDEX(op->outputs, 1);
        output->yStride = TENSOR_STRIDE_INDEX(op->outputs, 1);
        output->zStride = TENSOR_STRIDE_INDEX(op->outputs, 2);

        if (operation->parameter.pool_size_y != 0)
        {
            ComputeInputSize(
                output->width,
                WB_KERNEL_X(op->weights_biases),
                operation->parameter.pad_x_left,
                operation->parameter.pad_x_right,
                operation->parameter.pool_size_x,
                operation->parameter.pool_stride,
                &command->cmdInfo.convWidth,
                1);

            ComputeInputSize(
                output->height,
                WB_KERNEL_Y(op->weights_biases),
                operation->parameter.pad_y_top,
                operation->parameter.pad_y_bottom,
                operation->parameter.pool_size_y,
                operation->parameter.pool_stride,
                &command->cmdInfo.convHeight,
                1);
        }
        else
        {
            command->cmdInfo.convWidth = output->width;
            command->cmdInfo.convHeight = output->height;
        }

        {
            vxnne_convolution_relu_pooling_operation convOperation = (vxnne_convolution_relu_pooling_operation)operation;
            vxnne_mem_request requestList;
            vx_uint32 outImageTileX, outImageTileY, interleaveMode, kernelX, kernelY, inImageZ, inputDataFormat, imageTileSize, kernelbufferSize;
            vxnne_operation_info_s opInfo;
            vxnneOperation_GetInfo(operation, &opInfo);

            outImageTileX  = convOperation->resultInfo.outImageTileXSize;
            outImageTileY  = convOperation->resultInfo.outImageTileYSize;
            interleaveMode = convOperation->resultInfo.interleaveMode;
            kernelX = opInfo.weightsBiases->weights_sizes[0];
            kernelY = opInfo.weightsBiases->weights_sizes[1];
            inImageZ = TENSOR_SIZE_INDEX(opInfo.input, 2);
            inputDataFormat = TENSOR_DATA_TYPE(opInfo.input);

            imageTileSize = caculate3DTileSize(context, outImageTileX, outImageTileY, kernelX, kernelY, inImageZ, inputDataFormat, interleaveMode);

            kernelbufferSize = (vx_uint32)gcmALIGN_NP2(opInfo.weightsBiases->slice_array[0].kernel_align_stream_size, CACHE_ALIGNMENT_SIZE);

            requestList = graph->layer->memRequestList + command->operationID;

            if (context->vipSRAM.size > VX_VIP_SRAM_IMAGE_STREAM_SIZE)
            {
                gcoOS_ZeroMemory(&requestList->kernelCache, sizeof(vx_memory_s));
                requestList->kernelCache.lastUseId = requestList->kernelCache.firstUseId = VXNNE_MEM_ID_INIT_VALUE;
                requestList->kernelCache.sizes[0] = kernelbufferSize;
                requestList->kernelCache.allocType = VXNNE_MEM_POOL_TYPE_SET_CACHE(VXNNE_MEM_POOL_TYPE_VIP_SRAM);
                requestList->kernelCache.allocPriority = VXNNE_MEM_ALLOC_OPTIONAL_PRIORITY_2;
                requestList->kernelCache.allocPartial = vx_true_e;
                requestList->inputMemory[requestList->inputCount] = &requestList->kernelCache;
                requestList->inputCount++;

                gcoOS_ZeroMemory(&requestList->imageCache, sizeof(vx_memory_s));
                requestList->imageCache.lastUseId = requestList->imageCache.firstUseId = VXNNE_MEM_ID_INIT_VALUE;
                requestList->imageCache.sizes[0] = imageTileSize;
                requestList->imageCache.allocType = VXNNE_MEM_POOL_TYPE_SET_CACHE(VXNNE_MEM_POOL_TYPE_VIP_SRAM);
                requestList->imageCache.allocPartial = vx_false_e;
                requestList->imageCache.allocPriority = VXNNE_MEM_ALLOC_OPTIONAL_PRIORITY_1;
                requestList->inputMemory[requestList->inputCount] = &requestList->imageCache;
                requestList->inputCount++;
            }

            status = vxoMemoryPool_RequestList(graph, graph->layer->memRequestList, graph->layer->base.num_operations, command->operationID, 1, VX_NULL);
            if (status != VX_SUCCESS)
            {
                vxmASSERT(0);
                goto OnError;
            }

            if (requestList->imageCache.physicals[0] != 0)
            {
                command->cmdInfo.imageCacheSize  = (vx_uint32)requestList->imageCache.sizes[0];
                command->cmdInfo.imageCacheStart = requestList->imageCache.physicals[0];
                command->cmdInfo.imageCacheMode  = VXNNE_SRAM_CACHE_MODE_FULL_CACHE;
                vxmASSERT(requestList->imageCache.allocPartial == vx_false_e);
            }
            else
            {
                command->cmdInfo.imageCacheMode = VXNNE_SRAM_CACHE_MODE_NONE;
                command->cmdInfo.imageCacheSize = 0;
            }
            if (requestList->kernelCache.physicals[0] != 0)
            {
                command->cmdInfo.kernelCacheSize  = (vx_uint32)requestList->kernelCache.sizes[0];
                command->cmdInfo.kernelCacheStart = requestList->kernelCache.physicals[0];
                command->cmdInfo.kernelCacheMode = requestList->kernelCache.allocPartial ? VXNNE_SRAM_CACHE_MODE_PARTIAL_CACHE : VXNNE_SRAM_CACHE_MODE_FULL_CACHE;
            }
            else
            {
                command->cmdInfo.kernelCacheMode = VXNNE_SRAM_CACHE_MODE_NONE;
                command->cmdInfo.kernelCacheSize = 0;
            }

            if (context->vipSRAM.size > VX_VIP_SRAM_IMAGE_STREAM_SIZE)
            {
                requestList->inputCount = requestList->inputCount - 2;
            }

            command->cmdInfo.wb = opInfo.weightsBiases;
        }
    }
    else
    {
        gcmASSERT(0);
    }

    command->cmdInfo.padLeft = operation->parameter.pad_x_left;
    command->cmdInfo.padTop = operation->parameter.pad_y_top;
    command->cmdInfo.flush = vx_true_e;


    memcpy(&command->parameter, &operation->parameter, sizeof(vx_op_param_s));

OnError:
    return status;
}

vx_uint32 caculate3DTileSize(
    vx_context context,
    vx_uint32 outputTileXSize,
    vx_uint32 outputTileYSize,
    vx_uint32 kernelX,
    vx_uint32 kernelY,
    vx_uint32 inImageZ,
    vx_uint32 dataFormat,
    vx_uint32 interleaveMode
    )
{
    vx_uint32 inImageTileSizeX = outputTileXSize + kernelX - 1;
    vx_uint32 inImageTileSizeY = outputTileYSize + kernelY - 1;
    vx_uint32 imageTileSize  = 0;

    if ((kernelX == 1 && kernelY == 1
        && (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3) || vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6))
        && (dataFormat == VX_TYPE_INT8 || dataFormat == VX_TYPE_UINT8)))
    {
        vx_uint32 zdpNum = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6) ? 6 : 3;
        imageTileSize = gcmALIGN_NP2_SAFE(inImageTileSizeX * interleaveMode * (zdpNum * 2), 16) * gcmCEIL((vx_float32)inImageTileSizeY/interleaveMode) * gcmCEIL((vx_float32)inImageZ/(zdpNum * 2));
    }
    else
    {
        imageTileSize = gcmALIGN_NP2_SAFE(inImageTileSizeX  * inImageTileSizeY, 16) * inImageZ * vxnneGetTypeSize((vx_type_e)dataFormat);
    }

    return gcmALIGN_NP2(imageTileSize, CACHE_ALIGNMENT_SIZE);
}

vx_bool checkImageCacheMode(
    vx_uint32 outImageZSize,
    vx_uint32 kernelsPerCore,
    vx_uint32 nnCoreCount
    )
{
    if (!gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PER3DTILE_BUBBLE_FIX))
    {
        if (gcmCEIL(((vx_float32)outImageZSize / (kernelsPerCore * nnCoreCount))) == 1)
        {
            return vx_false_e;
        }
    }

    return vx_true_e;
}

#if gcdDUMP
void dumpNNCommandInfo(vx_uint32 sliceIndex, vx_uint32 sliceNum, vx_nn_cmd_info_u *info, vxnne_operation_command operationCommand)
{
    if (info != NULL)
    {
        gcmDUMP(gcvNULL,
            "#[info: NN_subimage_idx: %u]\n"
            "#[info: NN_subimage_total_number: %u]\n",
            sliceIndex,
            sliceNum);

        gcmDUMP(gcvNULL,
            "#[info: NN_outimage_base_adddress: 0x%08x]\n"
            "#[info: NN_input_base_adddress: 0x%08x]\n",
            info->vx_nn_general_cmd_info.outImageAddress,
            info->vx_nn_general_cmd_info.inImageAddress);

        gcmDUMP(gcvNULL,
            "#[info: NN_kernel_base_adddress: 0x%08x]\n"
            "#[info: NN_kernels_per_core: %u]\n",
            info->vx_nn_general_cmd_info.kernelAddress,
            info->vx_nn_general_cmd_info.kernelsPerCore);

        gcmDUMP(gcvNULL,
            "#[info: NN_outimage_xsize: %u]\n"
            "#[info: NN_outimage_ysize: %u]\n"
            "#[info: NN_outimage_zsize: %u]\n",
            info->vx_nn_general_cmd_info.outImageXSize,
            info->vx_nn_general_cmd_info.outImageYSize,
            info->vx_nn_general_cmd_info.outImageZSize);

        gcmDUMP(gcvNULL,
            "#[info: NN_outimage_tile_xsize: %u]\n"
            "#[info: NN_outimage_tile_ysize: %u]\n",
            info->vx_nn_general_cmd_info.outImageTileXSize,
            info->vx_nn_general_cmd_info.outImageTileYSize);

        gcmDUMP(gcvNULL,
            "#[info: NN_kernel_xysize: %u]\n"
            "#[info: NN_kernel_zsize: %u]\n"
            "#[info: NN_pooling: %u]\n"
            "#[info: NN_pooling_xysize: %u]\n",
            info->vx_nn_general_cmd_info.kernelXSize,
            info->vx_nn_general_cmd_info.kernelZSize,
            info->vx_nn_general_cmd_info.pooling,
            info->vx_nn_general_cmd_info.poolingXYSize);

        gcmDUMP(gcvNULL,
            "#[info: NN_inimage_xstride: %u]\n"
            "#[info: NN_inimage_ystride: %u]\n"
            "#[info: NN_outimage_xstride: %u]\n",
            info->vx_nn_general_cmd_info.inImageXstride,
            info->vx_nn_general_cmd_info.inImageYstride,
            info->vx_nn_general_cmd_info.outImageXstride);

        gcmDUMP(gcvNULL,
            "#[info: NN_image_caching_mode: %u]\n"
            "#[info: NN_kernel_caching_mode: %u]\n"
            "#[info: NN_partial_cache_data_unit: %u]\n",
            info->vx_nn_general_cmd_info.imageCachingMode,
            info->vx_nn_general_cmd_info.kernelCachingMode,
            info->vx_nn_general_cmd_info.partialCacheDataUnit);

        gcmDUMP(gcvNULL,
            "#[info: NN_inimg_ocbuf_start_address: 0x%08x]\n"
            "#[info: NN_inimg_ocbuf_end_address: 0x%08x]\n",
            info->vx_nn_general_cmd_info.inImageCircularBufEndAddrPlus1 - info->vx_nn_general_cmd_info.inImageCircularBufSize,
            info->vx_nn_general_cmd_info.inImageCircularBufEndAddrPlus1);

        gcmDUMP(gcvNULL,
            "#[info: NN_outimg_ocbuf_size: %u]\n"
            "#[info: NN_outimage_ocbuf_end_address: 0x%08x]\n",
            info->vx_nn_general_cmd_info.outImageCircularBufSize,
            info->vx_nn_general_cmd_info.outImageCircularBufEndAddrPlus1);

        gcmDUMP(gcvNULL,
            "#[info: NN_kernel_cache_start_address: 0x%08x]\n"
            "#[info: NN_kernel_cache_end_address: 0x%08x]\n",
            info->vx_nn_general_cmd_info.kernelCacheStartAddress,
            info->vx_nn_general_cmd_info.kernelCacheEndAddress);

        gcmDUMP(gcvNULL,
            "#[info: NN_image_start_address: 0x%08x]\n"
            "#[info: NN_image_end_address: 0x%08x]\n",
            info->vx_nn_general_cmd_info.imageStartAddress,
            info->vx_nn_general_cmd_info.imageEndAddress);

        gcmDUMP(gcvNULL,
            "#[info: NN_kernel_data_type: %u]\n"
            "#[info: NN_inimage_xoffset: %d]\n"
            "#[info: NN_inimage_yoffset: %d]\n",
            info->vx_nn_general_cmd_info.kernelDataType,
            info->vx_nn_general_cmd_info.inImageXOffset,
            info->vx_nn_general_cmd_info.inImageYOffset);
    }

    if (operationCommand != NULL)
    {
        gcmDUMP(gcvNULL,
            "#[info: NN_operation_command_id: %u]\n"
            "#[info: NN_subimage_start_x: %u]\n"
            "#[info: NN_subimage_start_y: %u]\n",
            operationCommand->operationID,
            operationCommand->outputTile.x,
            operationCommand->outputTile.y
            );
    }
}
#endif

vx_status vxnneOperation_CaculateSRAMCache(
    vx_context      context,
    vxnne_operation operation,
    vx_bool         enableImageCache,
    vx_uint32*      kernelCacheSize,
    vx_uint32*      kernelCacheStart,
    vx_enum*        kernelCacheMode,
    vx_uint32*      imageCacheSize,
    vx_uint32*      imageCacheStart,
    vx_enum*        imageCacheMode
    )
{
    vx_uint32 outImageTileX, outImageTileY, interleaveMode, kernelX, kernelY, inImageZ, inputDataFormat;
    vxnne_operation_info_s opInfo;
    vx_status status = VX_SUCCESS;

    vxmASSERT(kernelCacheSize && kernelCacheStart && kernelCacheMode && imageCacheSize && imageCacheStart && imageCacheMode);

    vxnneOperation_GetInfo(operation, &opInfo);

    if (opInfo.opType != VXNNE_OPERATOR_CONVOLUTION) return VX_SUCCESS;

    outImageTileX  = opInfo.weightsBiases->archPerfHandle->resultInfo.outImageTileXSize;
    outImageTileY  = opInfo.weightsBiases->archPerfHandle->resultInfo.outImageTileYSize;
    interleaveMode = opInfo.weightsBiases->archPerfHandle->resultInfo.interleaveMode;
    kernelX = opInfo.weightsBiases->weights_sizes[0];
    kernelY = opInfo.weightsBiases->weights_sizes[1];
    inImageZ = TENSOR_SIZE_INDEX(opInfo.input, 2);
    inputDataFormat = TENSOR_DATA_TYPE(opInfo.input);

    *imageCacheSize = caculate3DTileSize(context, outImageTileX, outImageTileY, kernelX, kernelY, inImageZ, inputDataFormat, interleaveMode);

    vxmASSERT(*imageCacheSize != 0);

    vxnneSRAM_Reset(&context->vipSRAM);

    if (enableImageCache)
    {
        status = vxnneSRAM_Allocate(
                           &context->vipSRAM,
                           *imageCacheSize,
                           gcvNULL,
                           imageCacheStart);
    }
    else
    {
        status = VX_ERROR_NOT_ALLOCATED;
    }

    if (status == VX_SUCCESS)
    {
        *imageCacheMode = VXNNE_SRAM_CACHE_MODE_FULL_CACHE;
    }
    else
    {
        *imageCacheMode = VXNNE_SRAM_CACHE_MODE_NONE;
        *imageCacheSize = 0;
    }


    *kernelCacheSize = (vx_uint32)gcmALIGN_NP2(WB_STREAM_ALIGN_SIZE_INDEX(opInfo.weightsBiases, 0), CACHE_ALIGNMENT_SIZE);

    status = vxnneSRAM_Allocate(
                       &context->vipSRAM,
                       *kernelCacheSize,
                       gcvNULL,
                       kernelCacheStart);

    if (status == VX_SUCCESS)
    {
        *kernelCacheMode = VXNNE_SRAM_CACHE_MODE_FULL_CACHE;
    }
    else
    {

        status = vxnneSRAM_AllocateRest(
                       &context->vipSRAM,
                       kernelCacheSize,
                       gcvNULL,
                       kernelCacheStart);

        if (status == VX_SUCCESS)
        {
            *kernelCacheMode = VXNNE_SRAM_CACHE_MODE_PARTIAL_CACHE;
        }
        else
        {
            *kernelCacheMode = VXNNE_SRAM_CACHE_MODE_NONE;
            *kernelCacheSize = 0;
        }
    }

    return VX_SUCCESS;
}

vx_status vxnneOperation_NodeDump(
    vxnne_operation_command opCommand
    )
{
    vx_tensor output = VX_NULL;
    vx_uint32 i;
    void *outputsBase = VX_NULL;
    char operFileName[256] = {'\0'};
    char layerFileName[256] = {'\0'};
    FILE *fpLayer = VX_NULL, *fpOperation = VX_NULL;
    vx_uint32 width, height, depth;
    vx_uint32 offset = 0;
    static vx_uint32 layerNum = 0;
    vx_uint32 elementCount = 0;
    vx_uint32 index;
    vx_bool isDumpOperation = vx_false_e;
    size_t len = 0;

    for (i = 0; i < opCommand->operation->outputsNum; i++)
    {
        if (opCommand->operation->outputs[i]->type == VX_TYPE_TENSOR)
        {
            output = (vx_tensor)(opCommand->operation->outputs[i]);

            width       = TENSOR_VIEW_SIZE_INDEX(output, 0);
            height      = TENSOR_VIEW_SIZE_INDEX(output, 1);
            depth       = TENSOR_VIEW_SIZE_INDEX(output, 2);

            vxoTensor_GetTensorBatchArrayViewMemory((vx_tensor)(opCommand->operation->outputs[i]), opCommand->batchID, &outputsBase, VX_NULL);

            if ((opCommand->operation->id == (opCommand->operation->layer->num_operations - 1)) || isDumpOperation)
            {
                if (opCommand->operation->id == (opCommand->operation->layer->num_operations - 1))
                {
                    len = 0;
                    offset = 0;
                    memset(layerFileName, 0, sizeof(layerFileName));
                    len = strlen(((vx_reference)output)->name);
                    if (len > 0)
                    {
                        gcoOS_PrintStrSafe(layerFileName, 256, &offset, "tensorName_%s_NodeID_%d_%s_w_%d_h_%d_d_%d_batchID_%d_out_%d.txt",
                            ((vx_reference)output)->name, opCommand->operation->layer->node->nodeID, opCommand->operation->layer->name, width, height, depth, opCommand->batchID, i);
                    }
                    else
                    {
                        gcoOS_PrintStrSafe(layerFileName, 256, &offset, "NodeID_%d_%s_w_%d_h_%d_d_%d_batchID_%d_out_%d.txt",
                            opCommand->operation->layer->node->nodeID, opCommand->operation->layer->name, width, height, depth, opCommand->batchID, i);
                    }
                    fpLayer = fopen(layerFileName,"w");
                    if(!fpLayer)
                    {
                        vxError("can't open the file %s\n",layerFileName);
                        continue;
                    }
                }

                if (isDumpOperation)
                {
                    offset = 0;
                    memset(operFileName, 0, sizeof(operFileName));
                    gcoOS_PrintStrSafe(operFileName, 256, &offset, "%d_%s_operation_%d.txt", layerNum, opCommand->operation->layer->name, opCommand->operationID);

                    fpOperation = fopen(operFileName, "w");
                    if(!fpOperation)
                    {
                        vxError("can't open the file %s\n",operFileName);
                        continue;
                    }
                }

                vxInfo("***********dump layer :%2d***************\n",layerNum);

                elementCount = width * height * depth;
                for(index = 0; index < elementCount; index++)
                {
                    if (TENSOR_DATA_TYPE(output) == VX_TYPE_UINT8 && TENSOR_QUANT_TYPE(output) == VX_QUANT_AFFINE_SCALE)
                    {
                        if (fpLayer)
                        {
                            fprintf(fpLayer, "%f\n", vxnneGetDataQuant((vx_type_e)TENSOR_DATA_TYPE(output), index, (vx_uint8_ptr)outputsBase, TENSOR_TF_ZEROPOINT(output), TENSOR_TF_SCALE(output)));
                        }

                        if (fpOperation)
                        {
                            fprintf(fpOperation, "%f\n", vxnneGetDataQuant((vx_type_e)TENSOR_DATA_TYPE(output), index, (vx_uint8_ptr)outputsBase, TENSOR_TF_ZEROPOINT(output), TENSOR_TF_SCALE(output)));
                        }
                    }
                    else
                    {
                        if (fpLayer)
                        {
                            fprintf(fpLayer, "%f\n", vxnneGetData((vx_type_e)TENSOR_DATA_TYPE(output), index, (vx_uint8_ptr)outputsBase, TENSOR_POS(output)));
                        }

                        if (fpOperation)
                        {
                            fprintf(fpOperation, "%f\n", vxnneGetData((vx_type_e)TENSOR_DATA_TYPE(output), index, (vx_uint8_ptr)outputsBase, TENSOR_POS(output)));
                        }
                    }
                }

                if (fpLayer)
                {
                    fclose(fpLayer);
                }

                if (fpOperation)
                {
                    fclose(fpOperation);
                }

                layerNum++;
            }
        }
    }

    return VX_SUCCESS;
}

vx_bool vxnneIsNNSupportFormat(
    vx_context context,
    vx_tensor inputTensor,
    vx_weights_biases_parameter wb,
    vx_tensor outputTensor)
{
    vx_bool isNNSupportFormat = vx_false_e;
    vx_enum inputFormat, wbFormat, outputFormat;
    vx_enum inputQuantFormat, wbQuantFormat, outputQuantFormat;

    if (inputTensor == VX_NULL)
    {
        vxError("invalid parameter\n");
        return vx_false_e;
    }

    inputFormat = (inputTensor != NULL) ? TENSOR_DATA_TYPE(inputTensor) : 0;
    wbFormat = (wb != NULL) ? WB_WEIGHT_DATA_FORMAT(wb) : 0;
    outputFormat = (outputTensor != NULL) ? TENSOR_DATA_TYPE(outputTensor) : 0;

    inputQuantFormat = (inputTensor != NULL) ? TENSOR_QUANT_TYPE(inputTensor) : 0;
    wbQuantFormat = (wb != NULL) ? WB_WEIGHT_QUANT_FORMAT(wb) : 0;
    outputQuantFormat = (outputTensor != NULL) ? TENSOR_QUANT_TYPE(outputTensor) : 0;

    switch (inputFormat)
    {
    case VX_TYPE_FLOAT16:
        if ((context->nnConfig.fixedFeature.nnCoreCountFloat16 > 0) &&
            gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_FP16_ALU))
        {
            isNNSupportFormat = vx_true_e;
        }
        break;

    case VX_TYPE_INT16:
        if ((inputQuantFormat == VX_QUANT_DYNAMIC_FIXED_POINT) &&
            gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_INT16_ALU) &&
            (context->nnConfig.fixedFeature.nnCoreCountInt16 > 0))
        {
            isNNSupportFormat = vx_true_e;
        }
        break;

    case VX_TYPE_INT8:
        if ((inputQuantFormat == VX_QUANT_DYNAMIC_FIXED_POINT) &&
            (context->nnConfig.fixedFeature.nnCoreCountInt8 > 0))
        {
            isNNSupportFormat = vx_true_e;
        }
        break;

    case VX_TYPE_UINT8:
        if ((context->nnConfig.fixedFeature.nnCoreCountInt8 > 0) &&
            (((inputQuantFormat == VX_QUANT_AFFINE_SCALE) &&
            vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT)) ||
            (inputQuantFormat == VX_QUANT_DYNAMIC_FIXED_POINT)))
        {
            isNNSupportFormat = vx_true_e;
        }
        break;

    default:
        break;
    }

    if (isNNSupportFormat && (wb != VX_NULL) && (outputTensor != VX_NULL))
    {
        switch (wbFormat)
        {
        case VX_TYPE_UINT8:
        case VX_TYPE_INT8:
            if (((inputFormat == VX_TYPE_UINT8) || (inputFormat == VX_TYPE_INT8)) &&
                ((outputFormat == VX_TYPE_UINT8) || (outputFormat == VX_TYPE_INT8) || (outputFormat == VX_TYPE_UINT16) || (outputFormat == VX_TYPE_INT16) || (outputFormat == VX_TYPE_FLOAT16)))
            {
                return vx_true_e;
            }

        case VX_TYPE_UINT16:
        case VX_TYPE_INT16:
            if (((inputFormat == VX_TYPE_UINT16) || (inputFormat == VX_TYPE_INT16)) &&
                ((outputFormat == VX_TYPE_UINT8) || (outputFormat == VX_TYPE_INT8) || (outputFormat == VX_TYPE_UINT16) || (outputFormat == VX_TYPE_INT16)))
            {
                return vx_true_e;
            }

        case VX_TYPE_FLOAT16:
            if ((inputFormat == VX_TYPE_FLOAT16) &&
                ((outputFormat == VX_TYPE_UINT8) || (outputFormat == VX_TYPE_INT8) || (outputFormat == VX_TYPE_FLOAT16)))
            {
                return vx_true_e;
            }
        default:
            return vx_false_e;
        }
    }
    else
    {
        return isNNSupportFormat;
    }
}

vx_bool vxnneIsTPSupportFormat(
    vx_context context,
    vx_tensor inputTensor,
    vx_weights_biases_parameter wb,
    vx_tensor outputTensor)
{
    vx_bool isTpSupportFormat[3] = {vx_false_e, vx_false_e, vx_false_e};
    vx_enum dataFormat[3] = {0, 0, 0};
    vx_enum quantFormat[3] = {0, 0, 0};
    vx_uint32 i = 0;

    if ((inputTensor == VX_NULL) && (outputTensor == VX_NULL) && (wb == VX_NULL))
    {
        vxError("invalid parameter\n");
        return vx_false_e;
    }

    dataFormat[0] = (inputTensor != NULL) ? TENSOR_DATA_TYPE(inputTensor) : 0;
    dataFormat[1] = (wb != NULL) ? WB_WEIGHT_DATA_FORMAT(wb) : 0;
    dataFormat[2] = (outputTensor != NULL) ? TENSOR_DATA_TYPE(outputTensor) : 0;

    quantFormat[0] = (inputTensor != NULL) ? TENSOR_QUANT_TYPE(inputTensor) : 0;
    quantFormat[1] = (wb != NULL) ? WB_WEIGHT_QUANT_FORMAT(wb) : 0;
    quantFormat[2] = (outputTensor != NULL) ? TENSOR_QUANT_TYPE(outputTensor) : 0;

    for (i = 0; i < vxmLENGTH_OF(dataFormat); i++)
    {
        switch (dataFormat[i])
        {
            case VX_TYPE_FLOAT16:
                if (context->nnConfig.fixedFeature.tpCoreCount > 0)
                {
                    isTpSupportFormat[i] = vx_true_e;
                }
                break;

            case VX_TYPE_INT16:
                if ((quantFormat[i] == VX_QUANT_DYNAMIC_FIXED_POINT) &&
                    (context->nnConfig.fixedFeature.tpCoreCount > 0) &&
                    (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_TP_REAL_INT16) || gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_TP_SIMPLE_INT16)))
                {
                    isTpSupportFormat[i] = vx_true_e;
                }
                break;

            case VX_TYPE_INT8:
                if ((quantFormat[i] == VX_QUANT_DYNAMIC_FIXED_POINT) &&
                    (context->nnConfig.fixedFeature.tpCoreCount > 0))

                {
                    isTpSupportFormat[i] = vx_true_e;
                }
                break;

            case VX_TYPE_UINT8:
                if ((context->nnConfig.fixedFeature.tpCoreCount > 0) &&
                    (((quantFormat[i] == VX_QUANT_AFFINE_SCALE) &&
                    vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT)) ||
                    (quantFormat[i] == VX_QUANT_DYNAMIC_FIXED_POINT)))

                {
                    isTpSupportFormat[i] = vx_true_e;
                }
                break;

            default:
                break;
        }
    }

    if ((inputTensor == VX_NULL) || (outputTensor == VX_NULL) || (wb == VX_NULL))
    {
        if (((inputTensor != VX_NULL) && (isTpSupportFormat[0] == vx_false_e)) ||
            ((wb != VX_NULL) && (isTpSupportFormat[1] == vx_false_e)) ||
            ((outputTensor != VX_NULL) && (isTpSupportFormat[2] == vx_false_e)))
        {
            return vx_false_e;
        }
        else
        {
            return vx_true_e;
        }
    }
    else
    {
        return (vx_bool)(isTpSupportFormat[0] && isTpSupportFormat[1] && isTpSupportFormat[2]);
    }
}

/* Get irreducible fraction of ratio, return numeration and denominator */
void vxnneGetIrreducibleFraction(vx_float32 ratio, vx_uint32 *numerationPtr, vx_uint32 *denominatorPtr)
{
    vx_uint32 x, y;
    vx_float32 bestRatio = 0.0f;

    if ((numerationPtr == VX_NULL) || (denominatorPtr == VX_NULL))
    {
        return;
    }

    if (ratio >= (VX_KERNEL_PATTERN_BIT_SIZE - 1))
    {
        *numerationPtr = (VX_KERNEL_PATTERN_BIT_SIZE - 1);
        *denominatorPtr = 1;
        return;
    }
    else if ((ratio < 1.0f) &&
             ((1.0f / ratio) >= (VX_KERNEL_PATTERN_BIT_SIZE - 1)))
    {
        *numerationPtr = 1;
        *denominatorPtr = (VX_KERNEL_PATTERN_BIT_SIZE - 1);
        return;
    }

    for (y = 1; y < VX_KERNEL_PATTERN_BIT_SIZE; y++)
    {
        for (x = 1; x < VX_KERNEL_PATTERN_BIT_SIZE; x++)
        {
            vx_float32 temp = ((vx_float32)x / y);

            if (((x + y) > VX_KERNEL_PATTERN_BIT_SIZE) ||
                (temp > ratio))
            {
                break;
            }

            if (temp > bestRatio)
            {
                bestRatio = temp;
                *numerationPtr = x;
                *denominatorPtr = y;
            }
        }
    }

    vxmASSERT(((vx_float32)(*numerationPtr)/(*denominatorPtr)) <= ratio);
}

/* get kernel pattern bits, equidistribute binary bit '1' and '0' for hw requirement */
void vxnneGetKernelPatternBits(vx_uint32 oneNum, vx_uint32 zeroNum, vx_uint64 *kernelPatternBitPtr)
{
    vx_uint32 i = 0;
    vx_uint32 min = gcmMIN(oneNum, zeroNum);
    vx_uint32 delta = gcmABS((gctINT32)(oneNum - zeroNum));
    vx_uint64 result = 0;
    vx_uint64 temp1 = 0;
    vx_uint64 temp2 = 0;

    if (kernelPatternBitPtr == VX_NULL)
    {
        return;
    }

    /* compute repeated part(binary is "10"), if oneNum is 3 and zeroNum is 2, kernelPatternBit may be "10101", there are two repeated part "10" */
    for (i = 0; i < min; i++)
    {
        if (i == 0)
        {
            result = 0x2;
        }
        else
        {
            result = result << 2;
            result = result | 0x2;
        }
    }

    /* compute the rest binary "1" or "0" part and integrate with the repeated part */
    if (oneNum > zeroNum)
    {
        temp1 = ((((vx_uint64)-1) >> (64 - delta)));
        temp2 = temp1 << (min * 2);
        result = result | temp2;
    }
    else
    {
        result = result << delta;
    }

    *kernelPatternBitPtr = result;
}

void vxoWeightsBiases_Reshuffle(
    vx_weights_biases_parameter_base      wb_base
    )
{
    vx_nn_reshuffle_s src = {
        NULL,
        vxnneAlignWithStride(wb_base->org_weights_sizes[0], wb_base->strideX),
        vxnneAlignWithStride(wb_base->org_weights_sizes[1], wb_base->strideY),
        wb_base->org_weights_sizes[2],
        wb_base->org_weights_sizes[3],
        (vx_type_e)wb_base->weights_data_format
    };
    vx_nn_reshuffle_s dst = {
        NULL,
        wb_base->weights_sizes[0],
        wb_base->weights_sizes[1],
        wb_base->weights_sizes[2],
        wb_base->weights_sizes[3],
        (vx_type_e)wb_base->weights_data_format
    };
    vx_uint32 x, y, z, w;
    vx_uint32 orgXSize = wb_base->org_weights_sizes[0];
    vx_uint32 orgYSize = wb_base->org_weights_sizes[1];
    vx_uint32 orgZSize = wb_base->org_weights_sizes[2];
    vx_uint32 weightItemCount = wb_base->weights_sizes[0] * wb_base->weights_sizes[1] * wb_base->weights_sizes[2] *wb_base->weights_sizes[3];
    vx_uint32 weightSize = (vx_uint32)vxDataType_GetSize((vx_type_e)wb_base->weights_data_format);
    vx_uint8_ptr startWeightDataPtr = VX_NULL;
    gctPOINTER convertedWeightData  = VX_NULL;

    if (((wb_base->strideX > 1) || (wb_base->strideY > 1)) &&
        (wb_base->org_weights_sizes[0] != 1 || wb_base->org_weights_sizes[1] != 1) &&
        wb_base->reshuffleWeightPtr == VX_NULL)
    {
        wb_base->reshuffleWeightPtr = (vx_uint8_ptr)vxAllocateAndZeroMemory(weightItemCount * weightSize);
        if (wb_base->reshuffleWeightPtr == VX_NULL)
        {
            goto exit;
        }

        {
            convertedWeightData = vxAllocateAndZeroMemory(weightItemCount * weightSize);
        }

        if (convertedWeightData == VX_NULL)
        {
            goto exit;
        }

        vxoTensor_GetTensorViewMemory(wb_base->origWeight, (gctPOINTER*)(&startWeightDataPtr), VX_NULL);

        for (w = 0; w < src.wSize; w++)
        {
            for (z = 0; z < src.zSize; z++)
            {
                /* convert float32 to float16 and fill edge with 0 */
                for (y = 0; y < src.ySize; y++)
                {
                    for (x = 0; x < src.xSize; x++)
                    {
                        vx_uint32 orgOffsetSize = (w * orgZSize * orgYSize * orgXSize + z * orgYSize * orgXSize + y * orgXSize + x) * weightSize;
                        vx_uint32 fpOffsetSize = (w * src.zSize * src.ySize * src.xSize + z * src.ySize * src.xSize + y * src.xSize + x) * weightSize;
                        vx_uint32 zero = (TENSOR_DATA_TYPE(wb_base->origWeight) == VX_TYPE_UINT8 && TENSOR_QUANT_TYPE(wb_base->origWeight) == VX_QUANT_AFFINE_SCALE) ?
                            TENSOR_TF_ZEROPOINT(wb_base->origWeight) : 0;
                        vx_uint8 *converted, *orig;

                        converted = (vx_uint8*)convertedWeightData + fpOffsetSize;

                        if ((x > orgXSize - 1) || (y > orgYSize - 1))
                        {
                            _DataGeneralConvert((void*)&zero, (void*)converted, weightSize, weightSize);
                        }
                        else
                        {
                            orig = startWeightDataPtr + orgOffsetSize;
                            _DataGeneralConvert((void*)orig, (void*)converted, weightSize, weightSize);
                        }
                    }
                }
            }
        }

        /*reshuffle kernel data*/
        src.data   = convertedWeightData;
        dst.data   = wb_base->reshuffleWeightPtr;
        reshuffleData(&src, wb_base->strideX, wb_base->strideY, &dst);

        if (convertedWeightData)
        {
                vxFree(convertedWeightData);

            convertedWeightData = VX_NULL;
        }

        return;

exit:
        if (wb_base->reshuffleWeightPtr)
        {
            vxFree(wb_base->reshuffleWeightPtr);
            wb_base->reshuffleWeightPtr = VX_NULL;
        }
    }
}

vx_status vxo_insertHandel(vxnne_execution_layer   executionLayer)
{
    vx_uint32               i, j, n= 0;

    if(executionLayer == VX_NULL)
        return vx_true_e;

    for(j = 0; j< MAX_HANDEL; ++j)
    {
        executionLayer->swapHandle[j] = VX_NULL;
    }
    executionLayer->swapcount = 0;

    for (i = 0; i < executionLayer->opIndicesNum; i++)
    {
        vx_uint32 j, k;
        vx_uint32 tileOffset = 0;
        vxnne_operation op = executionLayer->operations[executionLayer->opIndices[i].operationID];

        if(op->target != VXNNE_OPERATION_TARGET_TP && op->target != VXNNE_OPERATION_TARGET_NN)
            continue;

        if (!executionLayer->opIndices[i].inputTile.sRAM)
        {
            for (j = 0, n = executionLayer->swapcount; j < op->inputsNum; j++)
            {
                if ((op->inputs[j] != VX_NULL)  && (op->inputs[j]->type == VX_TYPE_TENSOR || op->inputs[j]->type == VX_TYPE_IMAGE))
                {
                    if(op->inputs[j]->type == VX_TYPE_TENSOR)
                    {
                        if((!vxoTensor_IsVirtualTensor((vx_tensor)op->inputs[j])) && ((vx_tensor)op->inputs[j])->tensorBuffer->memory.wrapFlag == gcvALLOC_FLAG_USERMEMORY)
                        {
                            if(executionLayer->swapHandle[n] == VX_NULL)
                            {
                                executionLayer->swapHandle[n] = (vx_swapHandel *) vxAllocate(sizeof(vx_swapHandel));
                            }
                            executionLayer->swapHandle[n]->ref = (vx_reference)op->inputs[j];
                            tileOffset = executionLayer->opIndices[i].inputTile.physical - ((vx_tensor)op->inputs[j])->tensorBuffer->memory.physicals[0];
                        }
                        else
                            continue;
                    }
                    else if(op->inputs[j]->type == VX_TYPE_IMAGE)
                    {
                        if(!vxoImage_IsVirtualImage((vx_image)op->inputs[j]) && ((vx_image)op->inputs[j])->memory.wrapFlag == gcvALLOC_FLAG_USERMEMORY)
                        {
                            if(executionLayer->swapHandle[n] == VX_NULL)
                            {
                                executionLayer->swapHandle[n] = (vx_swapHandel *) vxAllocate(sizeof(vx_swapHandel));
                            }

                            executionLayer->swapHandle[n]->ref = (vx_reference)op->inputs[j];
                            tileOffset = executionLayer->opIndices[i].inputTile.physical - ((vx_image)op->inputs[j])->memory.physicals[0];
                        }
                        else
                            continue;
                    }

                    if(op->target == VXNNE_OPERATION_TARGET_NN)
                    {
                        vx_uint32 offset[1], count;
                        for(k = 0; k < (((&executionLayer->opIndices[i])->commandBuffer)).commandCount; ++k)
                        {
                            if(k == 0)
                                count = vxoBinaryGraph_SearchPattern((gctUINT32_PTR)((vx_uint32_ptr)(((vx_uint8_ptr)(&executionLayer->opIndices[i])->commandBuffer.logical) + NNE_COMMAND_SIZE * k)), NNE_COMMAND_SIZE, ((executionLayer->opIndices[i]).inputTile).physical, offset, vx_false_e);

                            executionLayer->swapHandle[n]->cmdAddr[k] = ((vx_uint32_ptr)(((vx_uint8_ptr)(&executionLayer->opIndices[i])->commandBuffer.logical) + NNE_COMMAND_SIZE * k)) + offset[0]/gcmSIZEOF(gctUINT32);
                            executionLayer->swapHandle[n]->offset[k] = (*(executionLayer->swapHandle[n]->cmdAddr[k]) - *(executionLayer->swapHandle[n]->cmdAddr[0])) + tileOffset; /*tensor or image input address offset*/
                        }
                        tileOffset = 0;
                    }
                    if(op->target == VXNNE_OPERATION_TARGET_TP)
                    {
                        vx_uint32 offset[1], count;
                        for(k = 0; k < (((&executionLayer->opIndices[i])->commandBuffer)).commandCount; ++k)
                        {
                            if(k == 0)
                                count = vxoBinaryGraph_SearchPattern(((vx_uint32_ptr)(((vx_uint8_ptr)(&executionLayer->opIndices[i])->commandBuffer.logical) + TP_COMMAND_SIZE * k)), TP_COMMAND_SIZE, ((executionLayer->opIndices[i]).inputTile).physical, offset, vx_false_e);

                            executionLayer->swapHandle[n]->cmdAddr[k]= ((vx_uint32_ptr)(((vx_uint8_ptr)(&executionLayer->opIndices[i])->commandBuffer.logical) + TP_COMMAND_SIZE * k)) + offset[0]/gcmSIZEOF(gctUINT32);
                            executionLayer->swapHandle[n]->offset[k] = (*(executionLayer->swapHandle[n]->cmdAddr[k]) - *(executionLayer->swapHandle[n]->cmdAddr[0])) + tileOffset;/*tensor or image input address offset*/
                        }
                        tileOffset = 0;
                    }
                    executionLayer->swapHandle[n]->orgAddress = ((executionLayer->opIndices[i]).inputTile).physical;
                    ++n;
                    executionLayer->swapcount = n;
                }
            }
        }
        if (!executionLayer->opIndices[i].outputTile.sRAM)
        {
            for (j = 0; j < op->outputsNum; j++)
            {
                if ((op->outputs[j] != VX_NULL) && (op->outputs[j]->type == VX_TYPE_TENSOR || op->outputs[j]->type == VX_TYPE_IMAGE))
                {

                    if(op->outputs[j]->type == VX_TYPE_TENSOR)
                    {
                        if((!vxoTensor_IsVirtualTensor((vx_tensor)op->outputs[j])) && ((vx_tensor)op->outputs[j])->tensorBuffer->memory.wrapFlag == gcvALLOC_FLAG_USERMEMORY)
                        {
                            if(executionLayer->swapHandle[n] == VX_NULL)
                            {
                                executionLayer->swapHandle[n] = (vx_swapHandel *) vxAllocate(sizeof(vx_swapHandel));
                            }
                            executionLayer->swapHandle[n]->ref = (vx_reference)op->outputs[j];
                            tileOffset = executionLayer->opIndices[i].outputTile.physical - ((vx_tensor)op->outputs[j])->tensorBuffer->memory.physicals[0];
                        }
                        else
                            continue;
                    }
                    else if(op->outputs[j]->type == VX_TYPE_IMAGE)
                    {
                        if(!vxoImage_IsVirtualImage((vx_image)op->outputs[j]) && ((vx_image)op->outputs[j])->memory.wrapFlag == gcvALLOC_FLAG_USERMEMORY)
                        {
                             if(executionLayer->swapHandle[n] == VX_NULL)
                            {
                                executionLayer->swapHandle[n] = (vx_swapHandel *) vxAllocate(sizeof(vx_swapHandel));
                            }

                            executionLayer->swapHandle[n]->ref = (vx_reference)op->outputs[j];
                            tileOffset = executionLayer->opIndices[i].outputTile.physical - ((vx_image)op->outputs[j])->memory.physicals[0];
                        }
                        else
                            continue;
                    }


                    if(op->target == VXNNE_OPERATION_TARGET_NN)
                    {
                        vx_uint32 offset[1], count;
                        for(k = 0;k < (((&executionLayer->opIndices[i])->commandBuffer)).commandCount; ++k)
                        {
                            if(k == 0)
                                count = vxoBinaryGraph_SearchPattern((gctUINT32_PTR)((vx_uint32_ptr)(((vx_uint8_ptr)(&executionLayer->opIndices[i])->commandBuffer.logical) + NNE_COMMAND_SIZE * k)), NNE_COMMAND_SIZE, ((executionLayer->opIndices[i]).outputTile).physical, offset, vx_false_e);

                            executionLayer->swapHandle[n]->cmdAddr[k] = ((vx_uint32_ptr)(((vx_uint8_ptr)(&executionLayer->opIndices[i])->commandBuffer.logical) + NNE_COMMAND_SIZE * k)) + offset[0]/gcmSIZEOF(gctUINT32);
                            executionLayer->swapHandle[n]->offset[k] = (*(executionLayer->swapHandle[n]->cmdAddr[k]) - *(executionLayer->swapHandle[n]->cmdAddr[0])) + tileOffset;
                        }
                        tileOffset = 0;
                    }
                    if(op->target == VXNNE_OPERATION_TARGET_TP)
                    {
                        vx_uint32 offset[1], count;
                        for(k = 0; k < (((&executionLayer->opIndices[i])->commandBuffer)).commandCount; ++k)
                        {
                            if(k == 0)
                                count = vxoBinaryGraph_SearchPattern((gctUINT32_PTR)((vx_uint32_ptr)(((vx_uint8_ptr)(&executionLayer->opIndices[i])->commandBuffer.logical) + TP_COMMAND_SIZE * k)), TP_COMMAND_SIZE, ((executionLayer->opIndices[i]).outputTile).physical, offset, vx_false_e);

                            executionLayer->swapHandle[n]->cmdAddr[k] = ((vx_uint32_ptr)(((vx_uint8_ptr)(&executionLayer->opIndices[i])->commandBuffer.logical) + TP_COMMAND_SIZE * k)) + offset[0]/gcmSIZEOF(gctUINT32);
                            executionLayer->swapHandle[n]->offset[k] =  (*(executionLayer->swapHandle[n]->cmdAddr[k]) - *(executionLayer->swapHandle[n]->cmdAddr[0])) + tileOffset;
                        }
                        tileOffset = 0;
                    }

                    executionLayer->swapHandle[n]->orgAddress = ((executionLayer->opIndices[i]).outputTile).physical;
                    ++n;
                    executionLayer->swapcount = n;
                }
            }
        }

    }
    return  vx_true_e;
}

vx_bool vxo_updateSwapHandle(vx_graph graph)
{
    vx_uint32 i = 0, j = 0, k;
    vx_bool isSwaped = vx_false_e;

    vxnne_execution_layer   executionLayer = (vxnne_execution_layer)&graph->layer->base;

    if(executionLayer == VX_NULL)
        return vx_false_e;

    for (i = 0; i < executionLayer->opIndicesNum; i++)
    {
        for (; j < executionLayer->swapcount; ++j)
        {
            if (executionLayer->swapHandle[j] != VX_NULL && executionLayer->swapHandle[j]->ref != VX_NULL)
            {
                if(executionLayer->swapHandle[j]->ref->type == VX_TYPE_TENSOR)
                {
                    vx_uint32 offset;
                    vxoTensor_GetTensorViewOffset(((vx_tensor)executionLayer->swapHandle[j]->ref), &offset);

                    for(k = 0; k < (((&executionLayer->opIndices[i])->commandBuffer)).commandCount; ++k)
                    {
                        if(((vx_tensor)executionLayer->swapHandle[j]->ref)->tensorBuffer->memory.physicals[0] + offset + executionLayer->swapHandle[j]->offset[k]!= (executionLayer->swapHandle[j]->orgAddress))
                        {
                            *(executionLayer->swapHandle[j]->cmdAddr[k]) = (vx_uint32)(((vx_tensor)executionLayer->swapHandle[j]->ref)->tensorBuffer->memory.physicals[0] + executionLayer->swapHandle[j]->offset[k]);
                            isSwaped = vx_true_e;
                        }
                    }

                }
                else if(executionLayer->swapHandle[j]->ref->type == VX_TYPE_IMAGE)
                {

                    for(k = 0; k < (((&executionLayer->opIndices[i])->commandBuffer)).commandCount; ++k)
                    {
                        if(((vx_image)(executionLayer->swapHandle[j]->ref))->memory.physicals[0] + executionLayer->swapHandle[j]->offset[k] != executionLayer->swapHandle[j]->orgAddress)
                        {
                            *(executionLayer->swapHandle[j]->cmdAddr[k]) = (vx_uint32)(((vx_image)(executionLayer->swapHandle[j]->ref))->memory.physicals[0] + executionLayer->swapHandle[j]->offset[k]);
                            isSwaped = vx_true_e;
                        }
                    }
                }
                else
                    vxmASSERT(0);
            }
        }
    }
    return  isSwaped;
}

vx_bool _IsSameDataType(
    vx_tensor src,
    vx_tensor dst
    )
{
    return (TENSOR_DATA_TYPE(src) == TENSOR_DATA_TYPE(dst));
}

vx_bool _IsSameQuantType(
    vx_tensor src,
    vx_tensor dst
    )
{
    vx_bool result = vx_false_e;

    if (TENSOR_QUANT_TYPE(src) == TENSOR_QUANT_TYPE(dst))
    {
        switch (TENSOR_QUANT_TYPE(src))
        {
        case VX_QUANT_NONE:
            result = vx_true_e;
            break;

        case VX_QUANT_DYNAMIC_FIXED_POINT:
            if (TENSOR_POS(src) == TENSOR_POS(dst))
            {
                result = vx_true_e;
            }
            break;

        case VX_QUANT_AFFINE_SCALE:
            if (TENSOR_TF_SCALE(src) == TENSOR_TF_SCALE(dst) &&
                TENSOR_TF_ZEROPOINT(src) == TENSOR_TF_ZEROPOINT(dst))
            {
                result = vx_true_e;
            }
            break;

        default:
            break;
        }
    }

    return result;
}

vx_bool _IsSameType(
    vx_tensor src,
    vx_tensor dst
    )
{
    return (_IsSameDataType(src, dst) && _IsSameQuantType(src, dst));
}

