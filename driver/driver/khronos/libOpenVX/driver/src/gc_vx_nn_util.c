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
    vx_uint32                    split_size_array[],
    vx_uint32                    split_offset_array[]
    )
{
    split_num = gcmMIN(whole_size, split_num);

    if (split_num <= 1)
    {
        split_size_array[0] = whole_size;
        split_offset_array[0] = 0;
    }
    else
    {
        vx_uint32 i;
        vx_uint32 quot = whole_size / split_num;
        vx_uint32 remain = whole_size % split_num;

        for (i = 0; i < split_num; i++)
        {
            split_size_array[i] = i < remain ? quot + 1 : quot;
            split_offset_array[i] = i < remain ? i * split_size_array[i] : remain * (quot + 1) + (i - remain) * split_size_array[i];
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


extern vx_status vxnneAdapter_SWCWHN2WHCN(
            vx_uint8_ptr input_ptr, vx_type_e input_format, vx_enum input_quant_type, vx_uint32 input_depth, vx_uint32 input_width, vx_uint32 input_height,
            vx_uint32 input_batch, vx_int8 in_fixpoint, vx_int32 in_tf_zp, vx_float32 in_tf_scale,
            vx_uint8_ptr output_ptr, vx_type_e output_format, vx_enum output_quant_type, vx_uint32 output_depth, vx_uint32 output_width, vx_uint32 output_height,
            vx_int8 out_fixpoint, vx_int32 out_tf_zp, vx_float32 out_tf_scale, vx_enum out_rounding_mode);

vx_uint32 calcFit1xN(vx_context context, vx_uint32 kernelZ, vx_uint32 inputX, vx_uint32 inputY)
{
    vx_uint32 bitLenthOfKernelSize = 4; /*kernel_x_y_size reg only has 4 bit, the max kernel y value is 15*/
    vx_uint32 bitLenthOfInImageSize = 13; /*inImageXsize reg only has 13 bit, the max in image x value is 8191*/
    vx_uint32 maxKernelSize = (1 << bitLenthOfKernelSize) - 1;
    vx_uint32 maxInImageXSize = (1 << bitLenthOfInImageSize) - 1;
    vx_uint32 maxN = gcmMIN(gcmMIN(context->nnConfig.fixedFeature.nnAccumBufferDepth, maxKernelSize), context->nnConfig.fixedFeature.nnInputBufferDepth);
    vx_uint32 fitN = 1;
    vx_uint32 i;

    if (inputX * inputY > maxInImageXSize)
    {
        return 1;
    }

    for (i = 2; i < maxN; i++)
    {
        if (kernelZ % i == 0)
        {
            fitN = i;
            break;
        }
    }
    return fitN;
}

vx_bool calcFitZdp3N(vx_context context,vx_uint32 inputX, vx_uint32 inputY, vx_uint32* fitN, vx_uint32 stride, vx_uint32 poolingSize)
{
    vx_uint32 bitLenthOfKernelSize = 4; /*kernel_x_y_size reg only has 4 bit, the max kernel y value is 15*/
    vx_uint32 bitLenthOfInImageSize = 13; /*inImageXsize reg only has 13 bit, the max in image x value is 8191*/
    vx_uint32 maxKernelSize = (1 << bitLenthOfKernelSize) - 1;
    vx_uint32 maxInImageXSize = (1 << bitLenthOfInImageSize) - 1;
    vx_uint32 maxN = gcmMIN(gcmMIN(context->nnConfig.fixedFeature.nnAccumBufferDepth, maxKernelSize), context->nnConfig.fixedFeature.nnInputBufferDepth);
    vx_uint32 i;

    /*NX1 not support pooling now*/
    if (poolingSize > 1)
        return vx_false_e;

    if ((inputX * inputY) < maxInImageXSize && stride == 1 && poolingSize <= 1)
    {
        *fitN = 1;
        return vx_true_e;
    }
    else
    {
        for (i = 2; i < maxN; i++)
        {
            if ((inputX * inputY) % i == 0 && (inputX * inputY / i) <= maxInImageXSize && i % stride == 0)
            {
                /* Check pooling size with pooling stride = 2*/
                if (poolingSize <= 1)
                {
                    *fitN = i;
                    return vx_true_e;
                }
                else if (poolingSize == 2 &&
                    ((inputX * inputY) / (i * stride)) % 2 == 0 &&
                    i % 2 == 0 &&
                    ((inputX * inputY) / (i * stride * 2)) % 2 == 0)
                {
                    /* 2x2 pooling out image size should be even */
                    *fitN = i;
                    return vx_true_e;
                }
                else if (poolingSize == 3 &&
                    ((inputX * inputY) / (i * stride)) % 2 == 0 &&
                    i % 2 == 0 &&
                    ((inputX * inputY) / (i * stride * 2)) % 2 == 1)
                {
                    /* 3x3 pooling out image size should be odd */
                    *fitN = i;
                    return vx_true_e;
                }
                else
                    continue;
            }
        }
    }

    return vx_false_e;
}

vx_status replaceKernelBufferZOffset(
    vx_weights_biases_parameter wb,
    vx_uint8_ptr wb_base_ptr,
    vx_int32 z_offset
    )
{
    vx_uint32 filterIndex;
    vx_uint32* kernelBufferPtr = VX_NULL;

    if (wb->zOffsetHandle == VX_NULL)
    {
        vxError("replaceKernelBufferZOffset: No offset");
        return VX_ERROR_INVALID_PARAMETERS;
    }

    vxmASSERT(z_offset > 0);

    for (filterIndex = 0; filterIndex < wb->numOfVz; filterIndex++)
    {
        vx_uint32 offsetValue = z_offset * filterIndex;
        vx_uint32 bitOffset = wb->zOffsetHandle[filterIndex].bitOffset;
        kernelBufferPtr = (vx_uint32*)(wb_base_ptr + wb->zOffsetHandle[filterIndex].ptrOffset);

        if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
            replaceBits(&kernelBufferPtr, &bitOffset, offsetValue, NN_Z_POSITION_OFFSET_BITS_VIP_V7);
        else
            replaceBits(&kernelBufferPtr, &bitOffset, offsetValue, NN_Z_POSITION_OFFSET_BITS);
    }

    wb->orgZOffsetValue = z_offset;

    return VX_SUCCESS;
}

vx_status vxoWeightsBiasesParameter_ProcessHead(
    vx_weights_biases_parameter     weights_bias,
    vx_enum                         usage
    )
{
    vx_uint8_ptr buff = (vx_uint8_ptr)(WB_MEM_LOGICAL_BASE_ADDR(weights_bias) - WB_MEM_HEAD_OFFSET(weights_bias));

    if (VX_READ_ONLY == usage || VX_READ_AND_WRITE == usage)
    {
        *(vx_int32_ptr)buff = 0x0A0B0C0D;
        buff += 4;

        memcpy(buff, weights_bias, sizeof(vx_weights_biases_parameter_s));
        buff += sizeof(vx_weights_biases_parameter_s);

        memcpy(buff, weights_bias->wb_base, sizeof(vx_weights_biases_parameter_base_s));
        buff += sizeof(vx_weights_biases_parameter_base_s);

        memcpy(buff, weights_bias->slice_array, sizeof(vx_weights_biases_slice_s) * weights_bias->slice_num);
        buff += sizeof(vx_weights_biases_slice_s) * weights_bias->slice_num;

        if (weights_bias->zOffsetHandle != VX_NULL)
        {
            *buff++ = 0x1;
            memcpy(buff, weights_bias->zOffsetHandle, sizeof(vx_weights_biases_z_offset_s));
            buff += sizeof(vx_weights_biases_z_offset_s);
        }
        else
        {
            *buff++ = 0x0;
        }

        if (weights_bias->archPerfHandle != VX_NULL)
        {
            *buff++ = 0x1;
            memcpy(buff, weights_bias->archPerfHandle, sizeof(vx_arch_perf_s));
            buff += sizeof(vx_arch_perf_s);
        }
        else
        {
            *buff++ = 0x0;
        }
    }
    else if (VX_WRITE_ONLY == usage || VX_READ_AND_WRITE == usage)
    {
        if (*(vx_int32_ptr)buff != 0x0A0B0C0D)
            return VX_ERROR_INVALID_VALUE;
        buff += 4;

        memcpy(weights_bias, buff, sizeof(vx_weights_biases_parameter_s));
        buff += sizeof(vx_weights_biases_parameter_s);

        weights_bias->wb_base = (vx_weights_biases_parameter_base)vxAllocateAndZeroMemory(sizeof(vx_weights_biases_parameter_base_s));
        if (weights_bias->wb_base == VX_NULL) return VX_ERROR_NO_MEMORY;
        memcpy(weights_bias->wb_base, buff, sizeof(vx_weights_biases_parameter_base_s));
        buff += sizeof(vx_weights_biases_parameter_base_s);

        memcpy(weights_bias->slice_array, buff, sizeof(vx_weights_biases_slice_s) * weights_bias->slice_num);
        buff += sizeof(vx_weights_biases_slice_s) * weights_bias->slice_num;

        if (*buff++ == 1)
        {
            weights_bias->zOffsetHandle = (vx_weights_biases_z_offset)vxAllocateAndZeroMemory(sizeof(vx_weights_biases_z_offset_s));
            if (weights_bias->zOffsetHandle == VX_NULL) return VX_ERROR_NO_MEMORY;
            memcpy(weights_bias->zOffsetHandle, buff, sizeof(vx_weights_biases_z_offset_s));
            buff += sizeof(vx_weights_biases_z_offset_s);
        }

        if (*buff++ == 1)
        {
            weights_bias->archPerfHandle = (vx_arch_perf)vxAllocateAndZeroMemory(sizeof(vx_arch_perf_s));
            if (weights_bias->archPerfHandle == VX_NULL) return VX_ERROR_NO_MEMORY;
            memcpy(weights_bias->archPerfHandle, buff, sizeof(vx_arch_perf_s));
            buff += sizeof(vx_arch_perf_s);
        }
    }
    else
    {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    return VX_SUCCESS;
}

#if MAP_UNMAP_REFERENCE
vx_status vxoWeightsBiasesParameter_Map(
    vx_weights_biases_parameter     weights_biases,
    vx_map_id *                     map_id,
    vx_uint32 *                     stride,
    void **                         ptr,
    vx_enum                         usage,
    vx_enum                         mem_type,
    vx_uint32                       flags
    )
{
    vx_status status = VX_FAILURE;

    /* bad parameters */
    if ((usage < VX_READ_ONLY) || (VX_READ_AND_WRITE < usage) ||
        (ptr == NULL) || (stride == NULL))
    {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    /* if weight_bias is allocated.
     */
    if (weights_biases->memory.allocated == vx_false_e)
    {
        return VX_ERROR_INVALID_REFERENCE;
    }

    vxInfo("vxoWeightsBiasesParameter_Map from "VX_FMT_REF" to ptr %p\n", weights_biases, *ptr);
    {
        vx_uint8 *buf = NULL;
        vx_size size = weights_biases->memory_size;

        if (vxoContext_MemoryMap(
                weights_biases->base.context, (vx_reference)weights_biases, size, usage, mem_type, flags, VX_NULL, (void **)&buf, map_id))
        {
            if (VX_READ_ONLY == usage || VX_READ_AND_WRITE == usage)
            {
                if (vxAcquireMutex(weights_biases->memory.writeLocks[0]) == vx_true_e)
                {
                    vx_uint8 *pSrc = (vx_uint8 *)&weights_biases->memory.logicals[0] - WB_MEM_HEAD_OFFSET(weights_biases);
                    vx_uint8 *pDst = (vx_uint8 *)buf;
                    *stride = (vx_uint32)weights_biases->memory_size;

                    status = vxoWeightsBiasesParameter_ProcessHead(weights_biases, usage);
                    if (status == VX_SUCCESS)
                    {
                        memcpy(pDst, pSrc, size);

                        *ptr = buf;
                        vxoReference_Increment(&weights_biases->base, VX_REF_EXTERNAL);
                    }

                    vxReleaseMutex(weights_biases->memory.writeLocks[0]);
                }
                else
                {
                    status = VX_ERROR_NO_RESOURCES;
                }
            }
            else
            {
                /* write only mode */
                *stride = (vx_uint32)weights_biases->memory_size;
                *ptr = buf;
                vxoReference_Increment(&weights_biases->base, VX_REF_EXTERNAL);
                status = VX_SUCCESS;
            }
        }
        else
        {
            status = VX_FAILURE;
        }
    }

    return status;
}

vx_status vxoWeightsBiasesParameter_Unmap(
    vx_weights_biases_parameter weights_biases,
    vx_map_id                   map_id
    )
{
    vx_status status = VX_FAILURE;

    /* bad parameters */
    if (vxoContext_FindMemoryMap(weights_biases->base.context, (vx_reference)weights_biases, map_id) != vx_true_e)
    {
        vxError("Invalid parameters to unmap weight biases parameter\n");
        return VX_ERROR_INVALID_PARAMETERS;
    }

    vxInfo("vxoWeightsBiasesParameter_Unmap from "VX_FMT_REF"\n", weights_biases);
    {
        vx_context context = weights_biases->base.context;
        vx_memory_map_s* map = &context->memoryMaps[map_id];

        if (map->used && map->ref == (vx_reference)weights_biases)
        {
            if (VX_WRITE_ONLY == map->usage || VX_READ_AND_WRITE == map->usage)
            {
                if (vxAcquireMutex(weights_biases->memory.writeLocks[0]) == vx_true_e)
                {
                    vx_uint8 *pSrc = (vx_uint8 *)map->logical;
                    vx_uint8 *pDst = (vx_uint8 *)&weights_biases->memory.logicals[0] - WB_MEM_HEAD_OFFSET(weights_biases);
                    vx_size size = weights_biases->memory_size;

                    memcpy(pDst, pSrc, size);
                    vxoWeightsBiasesParameter_ProcessHead(weights_biases, map->usage);

                    vxoContext_MemoryUnmap(context, map_id);
                    vxoReference_Decrement(&weights_biases->base, VX_REF_EXTERNAL);
                    vxReleaseMutex(weights_biases->memory.writeLocks[0]);
                    status = VX_SUCCESS;
                }
                else
                {
                    status = VX_ERROR_NO_RESOURCES;
                }
            }
            else
            {
                /* rean only mode */
                vxoContext_MemoryUnmap(weights_biases->base.context, map_id);
                vxoReference_Decrement(&weights_biases->base, VX_REF_EXTERNAL);
                status = VX_SUCCESS;
            }
        }
        else
        {
            status = VX_FAILURE;
        }

        return status;
    }
}
#else
vx_status vxoWeightsBiasesParameter_Map(
    vx_weights_biases_parameter     weights_biases,
    vx_map_id *                     map_id,
    vx_uint32 *                     stride,
    void **                         ptr,
    vx_enum                         usage,
    vx_enum                         mem_type,
    vx_uint32                       flags
    )
{
    vx_status status = VX_FAILURE;

    /* bad parameters */
    if ((usage < VX_READ_ONLY) || (VX_READ_AND_WRITE < usage) ||
        (ptr == NULL) || (stride == NULL))
    {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    /* if weight_bias is allocated.
     */
    if (weights_biases->memory.allocated == vx_false_e)
    {
        return VX_ERROR_INVALID_REFERENCE;
    }

    vxInfo("vxoWeightsBiasesParameter_Map from "VX_FMT_REF" to ptr %p\n", weights_biases, *ptr);
    {
        vx_uint8 *buf = NULL;
        vx_size size = weights_biases->memory_size;

        if (vxoContext_MemoryMap(
                weights_biases->base.context, (vx_reference)weights_biases, size, usage, mem_type, flags, VX_NULL, (void **)&buf, map_id))
        {
            /* write only mode */
            *stride = (vx_uint32)weights_biases->memory_size;
            *ptr = buf;
            vxoReference_Increment(&weights_biases->base, VX_REF_EXTERNAL);
            status = VX_SUCCESS;
        }
        else
        {
            status = VX_FAILURE;
        }
    }

    return status;
}

vx_status vxoWeightsBiasesParameter_Unmap(
    vx_weights_biases_parameter weights_biases,
    vx_map_id                   map_id
    )
{
    vx_status status = VX_FAILURE;

    /* bad parameters */
    if (vxoContext_FindMemoryMap(weights_biases->base.context, (vx_reference)weights_biases, map_id) != vx_true_e)
    {
        vxError("Invalid parameters to unmap weight biases parameter\n");
        return VX_ERROR_INVALID_PARAMETERS;
    }

    vxInfo("vxoWeightsBiasesParameter_Unmap from "VX_FMT_REF"\n", weights_biases);
    {
        vx_context context = weights_biases->base.context;
        vx_memory_map_s* map = &context->memoryMaps[map_id];

        if (map->used && map->ref == (vx_reference)weights_biases)
        {
            /* rean only mode */
            vxoContext_MemoryUnmap(weights_biases->base.context, map_id);
            vxoReference_Decrement(&weights_biases->base, VX_REF_EXTERNAL);
            status = VX_SUCCESS;
        }
        else
        {
            status = VX_FAILURE;
        }

        return status;
    }
}
#endif

vx_bool vxoWeightsBiasesParameter_IsValid(
    vx_weights_biases_parameter     weights_biases
    )
{
    if (weights_biases == VX_NULL)
        return vx_false_e;

    if (!vxoReference_IsValidAndSpecific(&weights_biases->base, VX_TYPE_WEIGHTS_BIASES_PARAMETER))
        return vx_false_e;

    if (weights_biases->weights_sizes[0] == 0)
        return vx_false_e;

    return vx_true_e;
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
    vx_bool                     sync
    )
{
    vx_uint32 i = 0;
    vx_status status = VX_SUCCESS;
    vx_uint32 commandSize = (type == gcvVX_ACCELERATOR_TP) ? TP_COMMAND_SIZE : NNE_COMMAND_SIZE;

    for (i = 0; i < commandBuffer->commandCount; i++)
    {
        gctUINT8  captureBuffer[VX_MAX_NNTP_OPERATION_STATE_SIZE] = {0};
        gctUINT32 actualSize = 0;

        if (node->graph->binarySave)
        {
            status = gcfVX_CaptureState(captureBuffer,
                                        VX_MAX_NNTP_OPERATION_STATE_SIZE,
                                        &actualSize,
                                        gcvTRUE,
                                        gcvFALSE);
        }

        status = gcfVX_Accel(commandBuffer->physical + i * commandSize, type,
                             commandBuffer->eventID[i], 0, (gctUINT32)gpuId, (gctBOOL)sync);
        if (status != VX_SUCCESS)
        {
            break;
        }

        if (node->graph->binarySave)
        {
            vx_uint32 cmdPhysical = (vx_uint32)(commandBuffer->physical + i * commandSize);

            status = gcfVX_CaptureState(gcvNULL, 0, &actualSize, gcvFALSE, gcvFALSE);
            if (status != VX_SUCCESS)
            {
                vxmASSERT(0);
            }
            vxoGraphBinary_ReSaveNNTPInformation(node,
                                                cmdPhysical,
                                                captureBuffer,
                                                (vx_uint32)actualSize);
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
                                                  operation->mGpuSync);
    }
    else
    {
        return vxnneCommandBuffer_ExecuteCommands(node, commandBuffer,
                                                  gcvVX_ACCELERATOR_NN,
                                                  operation->gpuId,
                                                  operation->mGpuSync);
    }
}

void ReplaceOperationCmmdZdpOpt(
    vxnne_tensor_info inputInfo,
    vxnne_tensor_info outputInfo,
    vx_weights_biases_parameter wb)
{
    vx_uint32 fitN = 0;
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
    inputInfo->zStride        = inputInfo->yStride * inputInfo->height;
    outputInfo->yStride       = outputSize * outputInfo->width;
}

void ReplaceOperationCmmd1xN(
    vxnne_tensor_info inputInfo,
    vxnne_tensor_info outputInfo,
    vx_weights_biases_parameter wb)
{
    vx_uint32 fitN = 0;
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
            if ((operation->target == VXNNE_OPERATION_TARGET_SH) || (operation->target == VXNNE_OPERATION_TARGET_SC))
            {
                binarySave->operationCmdPhysical[binarySave->currOperationIndex] = gcmPTR_TO_UINT64(operation);
                binarySave->operationOffset[binarySave->currOperationIndex] = binarySave->currOperationOffset;
                binarySave->currOperationIndex++;
                binarySave->currOperationOffset += sizeof(vx_binary_operation_info_s);
            }
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

vx_size calculateWeightBiasBufferSizeForZeroRunLen(
    vx_context context,
    vx_weights_biases_parameter wb,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_uint32 skip_value,
    vx_uint8 zero_run_len,
    gctPOINTER weight_data,
    vx_uint32 index
    )
{
    vx_enum weightFomat = weight_format;
    vx_uint32 skipValue = skip_value;
    vx_uint8 zeroRunLen = zero_run_len;

    vx_uint32 nnCoreCount = weightFomat == VX_TYPE_INT16 ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : weightFomat == VX_TYPE_FLOAT16 ?
                      context->nnConfig.fixedFeature.nnCoreCountFloat16 : context->nnConfig.fixedFeature.nnCoreCount;

    vx_uint32 maxZeroRun         = (1 << zero_run_len) - 1;

    vx_size kernelBufferSize     = 0;
    vx_uint32 filterTotalCount   = z_count;
    vx_uint32 filterCount        = filters_per_core; /* filter count every group */
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;
    vx_uint32 totalFilterPerCore = filterTotalCount / nnCoreCount;
    vx_uint32 oddFilterPerCore   = filterTotalCount % nnCoreCount;

    vx_uint32 weightCount        = wb->weights_sizes[0] * wb->weights_sizes[1];
    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize(weightFomat);
    vx_uint32 weightBitSize      = weightSize * 8;
    vx_uint32 biasSize           = (vx_uint32)vxDataType_GetSize(bias_format);
    vx_uint32 biasBitSize        = biasSize * 8;

    vx_uint32 sliceCount         = slice_count;
    vx_uint32 filterSliceSize    = weightCount * weightSize;
    vx_uint32 filterSize         = weightCount * weightSize * sliceCount;
    vx_uint8* startDataPtr       = VX_NULL;
    vx_uint8* kernelDataPtr      = VX_NULL;

    vx_uint32 bitOffset          = 0;
    vx_uint32 rsvWeightCount = 0, blockCount = 0, nonZeroCount = 0;

    vx_uint32 coreIndex;
    vx_uint32 groupIndex, filterIndex, sliceIndex;
    vx_uint32 weightXIndex, weightYIndex;
    vx_uint32 zeroRun;

    vx_bool hasVipV7Feature = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7) == gcvSTATUS_TRUE ? vx_true_e : vx_false_e;
    vx_bool hasZDP3 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3);
    vx_bool hasZDP6 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6);
    vx_bool hasXYDP9 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP9);
    vx_bool hasXYDP6 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP6);

    if (weightFomat == VX_TYPE_INT16) biasBitSize = NN_INTEGER_BIAS_BITS_VIP_V7_INT16;

    startDataPtr = (vx_uint8*)weight_data;

    /* Write kernel Buffer size for each core. */
    /* Assume core count is at most 16. */
    kernelBufferSize = 64;

    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        vx_uint32 coreFilterCount = (coreIndex < oddFilterPerCore) ? totalFilterPerCore + 1 : totalFilterPerCore;

        /* Write zeroRunLen and coreFilterCount. */
        bitOffset = 8 + 16;

        /* Write weight value and bias for every group. */
        zeroRun = 0;
        for (groupIndex = 0; groupIndex < groupCount; groupIndex++)
        {
            vx_uint32 actualFilterCount = (groupIndex == groupCount - 1) ?
                                            (coreFilterCount - groupIndex * filterCount) : filterCount;
            vx_uint32 filterStart, filterEnd;

            if ((groupIndex == groupCount - 1)
                && (coreIndex >= (filterTotalCount % nnCoreCount)))
            {
                filterStart = groupIndex * nnCoreCount * filterCount
                            + (filterTotalCount % nnCoreCount) * (actualFilterCount + 1)
                            + (coreIndex - (filterTotalCount % nnCoreCount)) * actualFilterCount;
            }
            else
            {
                filterStart = groupIndex * nnCoreCount * filterCount + coreIndex * actualFilterCount;
            }
            filterEnd = filterStart + actualFilterCount - 1;

            if (wb->weights_sizes[0] == 1 && wb->weights_sizes[1] == 1 &&
                (hasZDP3 || hasZDP6) &&
                (weightFomat == VX_TYPE_INT8 || weightFomat == VX_TYPE_UINT8))
            {
                vx_uint32 zdpNum = hasZDP6 ? 6 : 3;

                /* zdp3 zdp6 can not enable same time */
                vxmASSERT(!(hasZDP3 && hasZDP6));

                for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex += (zdpNum * 2))
                {
                    vx_uint8 group2DCoded = 0;
                    vx_uint32 weight = 0;
                    vx_uint32 realSliceIndex = 0;

                    /* add slices of every filter*/
                    for (filterIndex = filterStart; filterIndex <= filterEnd; filterIndex++)
                    {
                        vx_uint32 nonZeroInBlock1Count = 0; /*Check if there's non-zero point in first zdpNum block*/
                        vx_uint32 nonZeroInBlock2Count = 0; /*Check if there's non-zero point in another zdpNum block*/
                        for (realSliceIndex = sliceIndex; realSliceIndex < sliceIndex + (zdpNum * 2); realSliceIndex++)
                        {
                            vx_uint8 needToWriteBias = (realSliceIndex == 0) ? 1 : 0;

                            if (realSliceIndex >= sliceCount)
                                break;

                            /* add one slice data every filter */
                            kernelDataPtr = startDataPtr + filterIndex * filterSize + filterSliceSize * realSliceIndex;
                            if (weightFomat == VX_TYPE_INT8)
                                weight = *((vx_int8 *)kernelDataPtr);
                            else if (weightFomat == VX_TYPE_UINT8)
                                weight = *((vx_uint8 *)kernelDataPtr);
                            else
                                weight = *((vx_uint16 *)kernelDataPtr);

                            if (realSliceIndex - sliceIndex < zdpNum && weight != skipValue)
                                nonZeroInBlock1Count++;
                            else if (weight != skipValue)
                                nonZeroInBlock2Count++;

                            if ((zeroRun == maxZeroRun)
                                || ((realSliceIndex == 0)
                                    && needToWriteBias)
                                || (realSliceIndex == sliceCount - 1)
                                || (weight != skipValue)
                                || ((filterIndex == filterEnd)
                                    && (group2DCoded == 0)))
                            {
                                /* Write zeroRun and weight. */
                                bitOffset += zeroRunLen + weightBitSize;
                                if (bitOffset >= 32)
                                {
                                    bitOffset -= 32;
                                    kernelBufferSize += 4;
                                }
                                zeroRun = 0;

                                if (needToWriteBias)
                                {
                                    bitOffset += biasBitSize;
                                    /* Write bias. */
                                    if (bitOffset >= 32)
                                    {
                                        bitOffset -= 32;
                                        kernelBufferSize += 4;
                                    }
                                    needToWriteBias = 0;
                                }
                                group2DCoded = 1;
                                rsvWeightCount++;
                            }
                            else
                            {
                                zeroRun++;
                            }

                            /* add offet behind the last point of the last slice of each filter */
                            if (realSliceIndex == sliceCount - 1)
                            {
                                if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                    bitOffset += NN_Z_POSITION_OFFSET_BITS_VIP_V7;
                                else
                                    bitOffset += NN_Z_POSITION_OFFSET_BITS;
                                if (bitOffset >= 32)
                                {
                                    bitOffset -= 32;
                                    kernelBufferSize += 4;
                                }
                            }
                        }
                        if (sliceIndex + zdpNum >= sliceCount)
                            blockCount++;
                        else
                            blockCount += 2;

                        if (nonZeroInBlock1Count)
                            nonZeroCount++;
                        if (nonZeroInBlock2Count)
                            nonZeroCount++;
                    }
                }
            }
            else if ((hasXYDP9 || hasXYDP6)
                     && (weightFomat == VX_TYPE_INT8 || weightFomat == VX_TYPE_UINT8))
            {
                vx_uint32 xStep = 3;
                vx_uint32 yStep = 0;
                vx_uint32 inImageBufferSize = hasXYDP9 ? 2 : 1;

                if (hasXYDP6)
                    yStep = 2;
                else
                    yStep = 3;

                for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex += inImageBufferSize)
                {
                    vx_uint8 group2DCoded = 0;
                    vx_uint32 weight = 0;
                    vx_uint32 realSliceIndex = 0;
                    vx_uint32 realWeightXIndex = 0, realWeightYIndex = 0;
                    vx_uint32 subKxSize = 0, subKySize = 0, subKzSize = 0;

                    subKzSize = gcmMIN((sliceCount - sliceIndex), inImageBufferSize);
                    /* add slices of every filter*/
                    for (filterIndex = filterStart; filterIndex <= filterEnd; filterIndex++)
                    {
                        vx_bool hasWriteBias = vx_false_e;

                        for (weightYIndex = 0; weightYIndex < wb->weights_sizes[1]; weightYIndex += yStep)
                        {
                            subKySize = gcmMIN((wb->weights_sizes[1] - weightYIndex), yStep);
                            for (weightXIndex = 0; weightXIndex < wb->weights_sizes[0]; weightXIndex += xStep)
                            {
                                subKxSize = gcmMIN((wb->weights_sizes[0] - weightXIndex), xStep);
                                for (realSliceIndex = sliceIndex; realSliceIndex < sliceIndex + subKzSize; realSliceIndex++)
                                {
                                    vx_uint8 needToWriteBias = (sliceIndex == 0) ? 1 : 0;
                                    vx_uint32 nonZeroInBlockCount = 0; /*Check if there's non-zero point in one 3x3 block*/

                                    /* Add one slice data every filter. */
                                    kernelDataPtr = startDataPtr + filterIndex * filterSize + filterSliceSize * realSliceIndex;

                                    for (realWeightYIndex = weightYIndex; realWeightYIndex < weightYIndex + subKySize; realWeightYIndex++)
                                    {
                                        for (realWeightXIndex = weightXIndex; realWeightXIndex < weightXIndex + subKxSize; realWeightXIndex++)
                                        {
                                            vx_uint8_ptr realKernelDataPtr = kernelDataPtr + (realWeightYIndex * wb->weights_sizes[0] + realWeightXIndex) * weightSize;

                                            if (weightFomat == VX_TYPE_INT8)
                                                weight = *((vx_int8 *)realKernelDataPtr);
                                            else if (weightFomat == VX_TYPE_UINT8)
                                                weight = *((vx_uint8 *)realKernelDataPtr);
                                            else
                                                weight = *((vx_uint16 *)realKernelDataPtr);

                                            if (weight != skipValue)
                                                nonZeroInBlockCount++;

                                            if ((zeroRun == maxZeroRun)
                                                || ((realSliceIndex == 0)
                                                    && (realWeightXIndex == wb->weights_sizes[0] - 1)
                                                    && (realWeightYIndex == wb->weights_sizes[1] - 1)
                                                    && needToWriteBias)
                                                || ((realSliceIndex == sliceCount - 1)
                                                    && (realWeightXIndex == wb->weights_sizes[0] - 1)
                                                    && (realWeightYIndex == wb->weights_sizes[1] - 1))
                                                || (weight != skipValue)
                                                || ((filterIndex == filterEnd)
                                                    && (realWeightXIndex == wb->weights_sizes[0] - 1)
                                                    && (realWeightYIndex == wb->weights_sizes[1] - 1)
                                                    && (group2DCoded == 0)))
                                            {
                                                /* Write zeroRun and weight. */
                                                bitOffset += zeroRunLen + weightBitSize;
                                                if (bitOffset >= 32)
                                                {
                                                    bitOffset -= 32;
                                                    kernelBufferSize += 4;
                                                }
                                                zeroRun = 0;

                                                if (needToWriteBias && !hasWriteBias)
                                                {
                                                    bitOffset += biasBitSize;
                                                    /* Write bias. */
                                                    if (bitOffset >= 32)
                                                    {
                                                        bitOffset -= 32;
                                                        kernelBufferSize += 4;
                                                    }
                                                    needToWriteBias = 0;
                                                    hasWriteBias = vx_true_e;
                                                }
                                                group2DCoded = 1;
                                                rsvWeightCount++;
                                            }
                                            else
                                            {
                                                zeroRun++;
                                            }

                                            /* add offet behind the last point of the last slice of each filter */
                                            if (realSliceIndex == sliceCount - 1 && realWeightXIndex == wb->weights_sizes[0] - 1 && realWeightYIndex == wb->weights_sizes[1] - 1)
                                            {
                                                 /* Write offsetValue. */
                                                if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                                    bitOffset += NN_Z_POSITION_OFFSET_BITS_VIP_V7;
                                                else
                                                    bitOffset += NN_Z_POSITION_OFFSET_BITS;

                                                if (bitOffset >= 32)
                                                {
                                                    bitOffset -= 32;
                                                    kernelBufferSize += 4;
                                                }
                                            }
                                        }
                                    }
                                    blockCount++;
                                    if (nonZeroInBlockCount)
                                        nonZeroCount++;
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
                {
                    vx_uint8 group2DCoded = 0;

                    /* Add slices of every filter. */
                    for (filterIndex = filterStart; filterIndex <= filterEnd; filterIndex++)
                    {
                        uint8_t needToWriteBias = (sliceIndex == 0) ? 1 : 0;

                        /* Add one slice data every filter. */
                        kernelDataPtr = startDataPtr + filterIndex * filterSize + filterSliceSize * sliceIndex;

                        for (weightYIndex = 0; weightYIndex < wb->weights_sizes[1]; weightYIndex++)
                        {
                            vx_uint32 nonZeroCountInDP3 = 0; /*Check if there's non-zero point in one 3x1 block*/
                            for (weightXIndex = 0; weightXIndex < wb->weights_sizes[0]; weightXIndex++)
                            {
                                vx_uint32 weight;

                                if (weightFomat == VX_TYPE_INT8)
                                    weight = *((vx_int8 *)kernelDataPtr);
                                else if (weightFomat == VX_TYPE_UINT8)
                                    weight = *((vx_uint8 *)kernelDataPtr);
                                else
                                    weight = *((vx_uint16 *)kernelDataPtr);
                                kernelDataPtr = kernelDataPtr + weightSize;

                                if(!hasVipV7Feature || weightFomat == VX_TYPE_FLOAT16)
                                {
                                    /*V6 or FP16, DP1, a block is 1x1*/
                                    blockCount++;
                                    if (weight != skipValue)
                                        nonZeroCount++;
                                }
                                else
                                {
                                    if (weight != skipValue)
                                        nonZeroCountInDP3++;

                                    /*V7, DP3, one block is 3x1*/
                                    if ((weightXIndex + 1) % 3 == 0 || weightXIndex == wb->weights_sizes[0] - 1)
                                    {
                                        blockCount++;
                                        if (nonZeroCountInDP3)
                                            nonZeroCount++;
                                        nonZeroCountInDP3 = 0;
                                    }
                                }

                                if ((zeroRun == maxZeroRun)
                                    || ((sliceIndex == 0)
                                        && (weightXIndex == wb->weights_sizes[0] - 1)
                                        && (weightYIndex == wb->weights_sizes[1] - 1)
                                        && needToWriteBias)
                                    || ((weightXIndex == wb->weights_sizes[0] - 1)
                                        && (weightYIndex == wb->weights_sizes[1] - 1)
                                        && (sliceIndex == sliceCount - 1))
                                    || (weight != skipValue)
                                    || ((filterIndex == filterEnd)
                                        && (weightXIndex == wb->weights_sizes[0] - 1)
                                        && (weightYIndex == wb->weights_sizes[1] - 1)
                                        && (group2DCoded == 0)))
                                {
                                    /* Write zeroRun and weight. */
                                    bitOffset += zeroRunLen + weightBitSize;
                                    if (bitOffset >= 32)
                                    {
                                        bitOffset -= 32;
                                        kernelBufferSize += 4;
                                    }
                                    zeroRun = 0;

                                    if (needToWriteBias)
                                    {
                                        bitOffset += biasBitSize;
                                        /* Write bias. */
                                        if (bitOffset >= 32)
                                        {
                                            bitOffset -= 32;
                                            kernelBufferSize += 4;
                                        }
                                        needToWriteBias = 0;
                                    }
                                    group2DCoded = 1;
                                    rsvWeightCount++;
                                }
                                else
                                {
                                    zeroRun++;
                                }
                            }
                        }

                        /* add offet behind the last point of the last slice of each filter */
                        if (sliceIndex == sliceCount - 1)
                        {
                            /* Write offsetValue. */
                            if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                bitOffset += NN_Z_POSITION_OFFSET_BITS_VIP_V7;
                            else
                                bitOffset += NN_Z_POSITION_OFFSET_BITS;
                            if (bitOffset >= 32)
                            {
                                bitOffset -= 32;
                                kernelBufferSize += 4;
                            }
                        }
                    }
                }
            }
        }

        /* pad 0 */
        if (bitOffset)
        {
            kernelBufferSize += 4;
        }
        kernelBufferSize = (kernelBufferSize + 63) & 0xFFFFFFC0;
    }

    wb->slice_array[index].all_count = blockCount;
    wb->slice_array[index].reserve_weight_count = rsvWeightCount;
    wb->slice_array[index].non_zero_count = nonZeroCount;
    wb->slice_array[index].kernel_orig_size = (filterSize + biasSize) * filterTotalCount;

    return kernelBufferSize;
}

vx_bool calculateWeightBiasBufferSizeForZeroRunLenEx(
    vx_context context,
    vx_weights_biases_parameter wb,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_uint32 skip_value,
    gctPOINTER weight_data,
    vx_uint32 index,
    vx_size* min_size,
    vx_uint8* zero_run_len
    )
{
    vx_enum weightFomat = weight_format;
    vx_uint32 skipValue = skip_value;

    vx_uint32 nnCoreCount = weightFomat == VX_TYPE_INT16 ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : weightFomat == VX_TYPE_FLOAT16 ?
                      context->nnConfig.fixedFeature.nnCoreCountFloat16 : context->nnConfig.fixedFeature.nnCoreCount;

    vx_uint32 filterTotalCount   = z_count;
    vx_uint32 filterCount        = filters_per_core; /* filter count every group */
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;
    vx_uint32 totalFilterPerCore = filterTotalCount / nnCoreCount;
    vx_uint32 oddFilterPerCore   = filterTotalCount % nnCoreCount;

    vx_uint32 weightCount        = wb->weights_sizes[0] * wb->weights_sizes[1];
    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize(weightFomat);
    vx_uint32 weightBitSize      = weightSize * 8;
    vx_uint32 biasSize           = (vx_uint32)vxDataType_GetSize(bias_format);
    vx_uint32 biasBitSize        = biasSize * 8;

    vx_uint32 sliceCount         = slice_count;
    vx_uint32 filterSliceSize    = weightCount * weightSize;
    vx_uint32 filterSize         = weightCount * weightSize * sliceCount;
    vx_uint8* startDataPtr       = VX_NULL;
    vx_uint8* kernelDataPtr      = VX_NULL;

    vx_uint32 bitOffset          = 0;

    vx_uint32 coreIndex;
    vx_uint32 i, j, groupIndex, filterIndex, sliceIndex;
    vx_uint32 weightXIndex, weightYIndex;

    vx_uint32 *origBitsArray = VX_NULL, *maxBasePad = VX_NULL;
    vx_uint32 *zerosPerCoreArray = VX_NULL; /* zerosPerCoreArray[nnCoreCount][MAX_ZRL_LEN + 1 + 2] */
    vx_uint32 minSize = (vx_uint32)~0UL, maxZRL = 0, maxZRLType, bigZRL = 0, blockCount = 0, nonZeroCount = 0;
    vx_uint8  minZrl = 0;
    vx_bool   complete;
    vx_bool hasVipV7Feature = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7) == gcvSTATUS_TRUE ? vx_true_e : vx_false_e;
    vx_bool hasZDP3 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3);
    vx_bool hasZDP6 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6);
    vx_bool hasXYDP9 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP9);
    vx_bool hasXYDP6 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP6);

    vx_uint32 maxZeroRunLen = (1 << context->nnConfig.fixedFeature.zrlBits) - 1;

    if (weightFomat == VX_TYPE_INT16) biasBitSize = NN_INTEGER_BIAS_BITS_VIP_V7_INT16;

    origBitsArray = (vx_uint32 *)vxAllocateAndZeroMemory(nnCoreCount * sizeof(vx_uint32));
    vxmASSERT(origBitsArray != NULL);
    maxBasePad = (vx_uint32 *)vxAllocateAndZeroMemory(nnCoreCount * sizeof(vx_uint32));
    vxmASSERT(maxBasePad != NULL);
    zerosPerCoreArray = (vx_uint32 *)vxAllocateAndZeroMemory(nnCoreCount * (maxZeroRunLen + 1 + 2) * sizeof(vx_uint32));
    vxmASSERT(zerosPerCoreArray != NULL);
    if (origBitsArray == VX_NULL || maxBasePad == NULL || zerosPerCoreArray == NULL)
    {
        if (origBitsArray)
            vxFree(origBitsArray);
        if (maxBasePad)
            vxFree(maxBasePad);
        if (zerosPerCoreArray)
            vxFree(zerosPerCoreArray);
        vxError("calculateWeightBiasBufferSizeForZeroRunLenEx: OUT OF MEMORY");
        return vx_false_e;
    }
    startDataPtr = (vx_uint8*)weight_data;

    /* Write kernel Buffer size for each core. */
    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        vx_uint32 coreFilterCount = (coreIndex < oddFilterPerCore) ? totalFilterPerCore + 1 : totalFilterPerCore;
        vx_uint32 zeroRun = 0;

        /* Write zeroRunLen and coreFilterCount. */
        bitOffset = 8 + 16;

        /* Write weight value and bias for every group. */
        for (groupIndex = 0; groupIndex < groupCount; groupIndex++)
        {
            vx_uint32 actualFilterCount = (groupIndex == groupCount - 1) ? (coreFilterCount - groupIndex * filterCount) : filterCount;
            vx_uint32 filterStart, filterEnd;

            if ((groupIndex == groupCount - 1)
                && (coreIndex >= (filterTotalCount % nnCoreCount)))
            {
                filterStart = groupIndex * nnCoreCount * filterCount
                            + (filterTotalCount % nnCoreCount) * (actualFilterCount + 1)
                            + (coreIndex - (filterTotalCount % nnCoreCount)) * actualFilterCount;
            }
            else
            {
                filterStart = groupIndex * nnCoreCount *filterCount + coreIndex * actualFilterCount;
            }
            filterEnd = filterStart + actualFilterCount - 1;

            if (wb->weights_sizes[0] == 1 && wb->weights_sizes[1] == 1 &&
                (hasZDP3 || hasZDP6) &&
                (weightFomat == VX_TYPE_INT8 || weightFomat == VX_TYPE_UINT8))
            {
                vx_uint32 zdpNum = hasZDP6 ? 6 : 3;

                /* zdp3 zdp6 can not enable same time */
                vxmASSERT(!(hasZDP3 && hasZDP6));

                for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex += (zdpNum * 2))
                {
                    vx_uint8 group2DCoded = 0;
                    vx_uint32 weight = 0;
                    vx_uint32 realSliceIndex = 0;

                    /* add slices of every filter*/
                    for (filterIndex = filterStart; filterIndex <= filterEnd; filterIndex++)
                    {
                        vx_uint32 nonZeroInBlock1Count = 0; /*Check if there's non-zero point in first zdpNum block*/
                        vx_uint32 nonZeroInBlock2Count = 0; /*Check if there's non-zero point in another zdpNum block*/

                        for (realSliceIndex = sliceIndex; realSliceIndex < sliceIndex + (zdpNum * 2); realSliceIndex++)
                        {
                            vx_uint8 needToWriteBias = (realSliceIndex == 0) ? 1 : 0;

                            if (realSliceIndex >= slice_count)
                                break;

                            /* add one slice data every filter */
                            kernelDataPtr = startDataPtr + filterIndex * filterSize + filterSliceSize * realSliceIndex;
                            if (weightFomat == VX_TYPE_INT8)
                                weight = *((vx_int8 *)kernelDataPtr);
                            else if (weightFomat == VX_TYPE_UINT8)
                                weight = *((vx_uint8 *)kernelDataPtr);
                            else
                                weight = *((vx_uint16 *)kernelDataPtr);

                            if (realSliceIndex - sliceIndex < zdpNum && weight != skipValue)
                                nonZeroInBlock1Count++;
                            else if (weight != skipValue)
                                nonZeroInBlock2Count++;

                            if (((realSliceIndex == 0)
                                    && needToWriteBias)
                                || (realSliceIndex == slice_count - 1)
                                || (weight != skipValue)
                                || ((filterIndex == filterEnd)
                                    && (group2DCoded == 0)))
                            {
                                /* Write zeroRun and weight. */
                                if (zeroRun > maxZeroRunLen)
                                {
                                    *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 1) += 1;
                                    *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 2) += zeroRun - maxZeroRunLen;
                                    maxBasePad[coreIndex] += (maxZeroRunLen + 1 + zeroRun) / (maxZeroRunLen + 1);
                                }
                                else
                                {
                                    *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + zeroRun) += 1;
                                }
                                if (zeroRun > maxZRL) maxZRL = zeroRun;
                                zeroRun = 0;

                                if (needToWriteBias)
                                {
                                    bitOffset += biasBitSize;
                                    needToWriteBias = 0;
                                }
                                group2DCoded = 1;
                            }
                            else
                            {
                                zeroRun++;
                            }

                            /* add offet behind the last point of the last slice of each filter */
                            if (realSliceIndex == slice_count - 1)
                            {
                                if (hasVipV7Feature)
                                    bitOffset += NN_Z_POSITION_OFFSET_BITS_VIP_V7;
                                else
                                    bitOffset += NN_Z_POSITION_OFFSET_BITS;
                            }
                        }

                        if (sliceIndex + zdpNum >= slice_count)
                            blockCount++;
                        else
                            blockCount += 2;

                        if (nonZeroInBlock1Count)
                            nonZeroCount++;
                        if (nonZeroInBlock2Count)
                            nonZeroCount++;
                    }
                }
            }
            else if ((hasXYDP9 || hasXYDP6)
                && (weightFomat == VX_TYPE_INT8 || weightFomat == VX_TYPE_UINT8))
            {
                vx_uint32 xStep = 3;
                vx_uint32 yStep = 0;
                vx_uint32 inImageBufferSize = hasXYDP9 ? 2 : 1;

                if (hasXYDP6)
                    yStep = 2;
                else
                    yStep = 3;

                for (sliceIndex = 0; sliceIndex < slice_count; sliceIndex += inImageBufferSize)
                {
                    vx_uint8 group2DCoded = 0;
                    vx_uint32 weight = 0;
                    vx_uint32 realSliceIndex = 0;
                    vx_uint32 realWeightXIndex = 0, realWeightYIndex = 0;
                    vx_uint32 subKxSize = 0, subKySize = 0, subKzSize = 0;

                    subKzSize = gcmMIN((slice_count - sliceIndex), inImageBufferSize);
                    /* add slices of every filter*/
                    for (filterIndex = filterStart; filterIndex <= filterEnd; filterIndex++)
                    {
                        vx_bool hasWriteBias = vx_false_e;

                        for (weightYIndex = 0; weightYIndex < wb->weights_sizes[1]; weightYIndex += yStep)
                        {
                            subKySize = gcmMIN((wb->weights_sizes[1] - weightYIndex), yStep);
                            for (weightXIndex = 0; weightXIndex < wb->weights_sizes[0]; weightXIndex += xStep)
                            {
                                subKxSize = gcmMIN((wb->weights_sizes[0] - weightXIndex), xStep);
                                for (realSliceIndex = sliceIndex; realSliceIndex < sliceIndex + subKzSize; realSliceIndex++)
                                {
                                    vx_uint8 needToWriteBias = (sliceIndex == 0) ? 1 : 0;
                                    vx_uint32 nonZeroInBlockCount = 0; /*Check if there's non-zero point in one 3x3 block*/

                                    /* Add one slice data every filter. */
                                    kernelDataPtr = startDataPtr + filterIndex * filterSize + filterSliceSize * realSliceIndex;

                                    for (realWeightYIndex = weightYIndex; realWeightYIndex < weightYIndex + subKySize; realWeightYIndex++)
                                    {
                                        for (realWeightXIndex = weightXIndex; realWeightXIndex < weightXIndex + subKxSize; realWeightXIndex++)
                                        {
                                            vx_uint8_ptr realKernelDataPtr = kernelDataPtr + (realWeightYIndex * wb->weights_sizes[0] + realWeightXIndex) * weightSize;

                                            if (weightFomat == VX_TYPE_INT8)
                                                weight = *((vx_int8 *)realKernelDataPtr);
                                            else if (weightFomat == VX_TYPE_UINT8)
                                                weight = *((vx_uint8 *)realKernelDataPtr);
                                            else
                                                weight = *((vx_uint16 *)realKernelDataPtr);

                                            if (weight != skipValue)
                                                nonZeroInBlockCount++;

                                            if (((realSliceIndex == 0)
                                                    && (realWeightXIndex == wb->weights_sizes[0] - 1)
                                                    && (realWeightYIndex == wb->weights_sizes[1] - 1)
                                                    && needToWriteBias)
                                                || ((realSliceIndex == slice_count - 1)
                                                    && (realWeightXIndex == wb->weights_sizes[0] - 1)
                                                    && (realWeightYIndex == wb->weights_sizes[1] - 1))
                                                || (weight != skipValue)
                                                || ((filterIndex == filterEnd)
                                                    && (realWeightXIndex == wb->weights_sizes[0] - 1)
                                                    && (realWeightYIndex == wb->weights_sizes[1] - 1)
                                                    && (group2DCoded == 0)))
                                            {
                                                if (zeroRun > maxZeroRunLen)
                                                {
                                                    *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 1) += 1;
                                                    *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 2) += zeroRun - maxZeroRunLen;
                                                    maxBasePad[coreIndex] += (maxZeroRunLen + 1 + zeroRun) / (maxZeroRunLen + 1);
                                                }
                                                else
                                                {
                                                    *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + zeroRun) += 1;
                                                }
                                                if (zeroRun > maxZRL) maxZRL = zeroRun;
                                                zeroRun = 0;

                                                if (needToWriteBias && !hasWriteBias)
                                                {
                                                    bitOffset += biasBitSize;
                                                    needToWriteBias = 0;
                                                    hasWriteBias = vx_true_e;
                                                }
                                                group2DCoded = 1;
                                            }
                                            else
                                            {
                                                zeroRun++;
                                            }

                                            /* add offet behind the last point of the last slice of each filter */
                                            if (realSliceIndex == slice_count - 1 && realWeightXIndex == wb->weights_sizes[0] - 1 && realWeightYIndex == wb->weights_sizes[1] - 1)
                                            {
                                                 /* Write offsetValue. */
                                                if (hasVipV7Feature)
                                                    bitOffset += NN_Z_POSITION_OFFSET_BITS_VIP_V7;
                                                else
                                                    bitOffset += NN_Z_POSITION_OFFSET_BITS;
                                            }
                                        }
                                    }

                                    blockCount++;
                                    if (nonZeroInBlockCount)
                                        nonZeroCount++;
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                for (sliceIndex = 0; sliceIndex < slice_count; sliceIndex++)
                {
                    vx_uint8 group2DCoded = 0;

                    /* Add slices of every filter. */
                    for (filterIndex = filterStart; filterIndex <= filterEnd; filterIndex++)
                    {
                        vx_uint8 needToWriteBias = (sliceIndex == 0) ? 1 : 0;

                        /* Add one slice data every filter. */
                        kernelDataPtr = startDataPtr + filterIndex * filterSize + filterSliceSize * sliceIndex;

                        for (weightYIndex = 0; weightYIndex < wb->weights_sizes[1]; weightYIndex++)
                        {
                            vx_uint32 nonZeroCountInDP3 = 0; /*Check if there's non-zero point in one 3x1 block*/

                            for (weightXIndex = 0; weightXIndex < wb->weights_sizes[0]; weightXIndex++)
                            {
                                vx_uint32 weight;

                                if (weightFomat == VX_TYPE_INT8)
                                    weight = *((vx_int8 *)kernelDataPtr);
                                else if (weightFomat == VX_TYPE_UINT8)
                                    weight = *((vx_uint8 *)kernelDataPtr);
                                else
                                    weight = *((vx_uint16 *)kernelDataPtr);
                                kernelDataPtr = kernelDataPtr + weightSize;

                                if(!hasVipV7Feature || weightFomat == VX_TYPE_FLOAT16)
                                {
                                    /*V6 or FP16, DP1, a block is 1x1*/
                                    blockCount++;
                                    if (weight != skipValue)
                                        nonZeroCount++;
                                }
                                else
                                {
                                    if (weight != skipValue)
                                        nonZeroCountInDP3++;

                                    /*V7, DP3, one block is 3x1*/
                                    if ((weightXIndex + 1) % 3 == 0 || weightXIndex == wb->weights_sizes[0] - 1)
                                    {
                                        blockCount++;
                                        if (nonZeroCountInDP3)
                                            nonZeroCount++;
                                        nonZeroCountInDP3 = 0;
                                    }
                                }

                                if (((sliceIndex == 0)
                                        && (weightXIndex == wb->weights_sizes[0] - 1)
                                        && (weightYIndex == wb->weights_sizes[1] - 1)
                                        && needToWriteBias)
                                    || ((weightXIndex == wb->weights_sizes[0] - 1)
                                        && (weightYIndex == wb->weights_sizes[1] - 1)
                                        && (sliceIndex == slice_count - 1))
                                    || (weight != skipValue)
                                    || ((filterIndex == filterEnd)
                                        && (weightXIndex == wb->weights_sizes[0] - 1)
                                        && (weightYIndex == wb->weights_sizes[1] - 1)
                                        && (group2DCoded == 0))
                                    )
                                {
                                    if (zeroRun > maxZeroRunLen)
                                    {
                                        *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 1) += 1;
                                        *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 2) += zeroRun - maxZeroRunLen;
                                        maxBasePad[coreIndex] += (maxZeroRunLen + 1 + zeroRun) / (maxZeroRunLen + 1);
                                    }
                                    else
                                    {
                                        *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + zeroRun) += 1;
                                    }
                                    if (zeroRun > maxZRL) maxZRL = zeroRun;
                                    zeroRun = 0;

                                    if (needToWriteBias)
                                    {
                                        bitOffset += biasBitSize;
                                        needToWriteBias = 0;
                                    }
                                    group2DCoded = 1;
                                }
                                else
                                {
                                    zeroRun++;
                                }
                            }
                        }

                        /* add offet behind the last point of the last slice of each filter */
                        if (sliceIndex == slice_count - 1)
                        {
                            /* Write offsetValue. */
                            if (hasVipV7Feature)
                                bitOffset += NN_Z_POSITION_OFFSET_BITS_VIP_V7;
                            else
                                bitOffset += NN_Z_POSITION_OFFSET_BITS;
                        }
                    }
                }
            }
        }

        origBitsArray[coreIndex] = bitOffset;
        bigZRL += *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 1);
    }

    /* analyze compressed size with different zrl */
    maxZRL = gcmMIN(maxZRL, maxZeroRunLen);
    maxZRLType = maxZRL ? gcmMIN((vx_uint32)ceilf((vx_float32)(log(maxZRL) / log(2))), context->nnConfig.fixedFeature.zrlBits) : 0;
    complete = (vx_float32)bigZRL / blockCount > 0.1f ? vx_false_e : vx_true_e;
    for (i = 0; i <= maxZRLType; i++)
    {
        vx_uint32 size = 64;
        vx_uint32 base = (1 << i) - 1;
        vx_uint32 rsvWeightCount = 0;

        for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
        {
            vx_uint32 bits = 0, rwc = 0;

            if (base == 0)
            {
                /* take care zrl = 0 */
                for (j = 0; j <= maxZRL; j++)
                {
                    rwc += *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + j) * (j+1);
                }

                if (*(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 1) != 0)
                {
                    vx_uint32 nsum = *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 1);
                    vx_uint32 dsum = *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 2);

                    rwc += maxZeroRunLen * nsum + dsum + nsum;
                }

                bits += rwc * weightBitSize;
            }
            else
            {
                for (j = 0; j <= maxZRL; j++)
                {
                    vx_uint32 zerosPerCore = *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + j);
                    if (!zerosPerCore) continue;
                    if (j <= base) rwc += zerosPerCore;
                    else rwc += ((j + 1 + base) / (base + 1)) * zerosPerCore;
                }

                if (*(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 1) != 0)
                {
                    vx_uint32 nsum = *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 1);
                    vx_uint32 dsum = *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 2);
                    if (base != maxZeroRunLen)
                    {
                        rwc += ((maxZeroRunLen + 1 + base) * nsum + dsum) / (base + 1);
                    }
                    else
                    {
                        rwc += maxBasePad[coreIndex];
                    }
                }

                bits += rwc * (i + weightBitSize);
            }

            /* other bits */
            rsvWeightCount += rwc;
            bits += origBitsArray[coreIndex];
            size += (gcmALIGN(bits, 32) / 32) * 4;
            size  = gcmALIGN(size, 64);
        }

        if (size < minSize)
        {
            minSize = size;
            minZrl = (vx_uint8)i;
            wb->slice_array[index].reserve_weight_count = rsvWeightCount;
        }

        if (!complete) break;
    }

    wb->slice_array[index].all_count = blockCount;
    wb->slice_array[index].non_zero_count = nonZeroCount;
    wb->slice_array[index].kernel_orig_size = (filterSize + biasSize) * filterTotalCount;
    wb->slice_array[index].kernel_stream_size = minSize;

    if (origBitsArray)
        vxFree(origBitsArray);
    if (maxBasePad)
        vxFree(maxBasePad);
    if (zerosPerCoreArray)
        vxFree(zerosPerCoreArray);
    if (min_size != VX_NULL) *min_size = minSize;
    if (zero_run_len != VX_NULL) *zero_run_len = minZrl;
    return complete;
}

vx_size calculateWeightBiasBalanceSizeForZeroRunLen(
    vx_context context,
    vx_weights_biases_parameter wb,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_uint32 skip_value,
    vx_uint8 zero_run_len,
    gctPOINTER weight_data,
    vx_uint32 index
    )
{
    vx_enum weightFomat = weight_format;
    vx_uint32 skipValue = skip_value;
    vx_uint8 zeroRunLen = zero_run_len;

    vx_uint32 nnCoreCount = weightFomat == VX_TYPE_INT16 ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : weightFomat == VX_TYPE_FLOAT16 ?
                      context->nnConfig.fixedFeature.nnCoreCountFloat16 : context->nnConfig.fixedFeature.nnCoreCount;

    vx_uint32 maxZeroRun         = (1 << zero_run_len) - 1;

    vx_size kernelBufferSize     = 0;
    vx_uint32 filterTotalCount   = z_count;
    vx_uint32 filterCount        = filters_per_core; /* filter count every group */
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;
    vx_uint32 totalFilterPerCore = filterTotalCount / nnCoreCount;
    vx_uint32 oddFilterPerCore   = filterTotalCount % nnCoreCount;

    vx_uint32 weightCount        = wb->weights_sizes[0] * wb->weights_sizes[1];
    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize(weightFomat);
    vx_uint32 weightBitSize      = weightSize * 8;
    vx_uint32 biasSize           = (vx_uint32)vxDataType_GetSize(bias_format);
    vx_uint32 biasBitSize        = biasSize * 8;

    vx_uint32 sliceCount         = slice_count;
    vx_uint32 filterSliceSize    = weightCount * weightSize;
    vx_uint32 filterSize         = weightCount * weightSize * sliceCount;
    vx_uint8* startDataPtr       = VX_NULL;
    vx_uint8* kernelDataPtr      = VX_NULL;

    vx_uint32 bitOffset          = 0;
    vx_uint32 rsvWeightCount = 0, blockCount = 0, nonZeroCount = 0;

    vx_uint32 coreIndex;
    vx_uint32 groupIndex, filterIndex, sliceIndex;
    vx_uint32 weightXIndex, weightYIndex;
    vx_uint32 zeroRun;

    vx_bool hasVipV7Feature = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7) == gcvSTATUS_TRUE ? vx_true_e : vx_false_e;
    vx_bool hasZDP3 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3);
    vx_bool hasZDP6 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6);
    vx_bool hasXYDP9 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP9);
    vx_bool hasXYDP6 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP6);

    vx_uint32 totalNonZeroCount = 0;
    vx_float32 averageNZCPerFilter = 0;
    vx_float32 varianceFL32 = 0;
    vx_uint32 variance = 0;
    vx_bool reorder = vx_false_e;

    typedef struct _gcVXNonZeroWeights
    {
        vx_uint32 nonZeroCount;
        vx_uint32 filterIdx;
    }gcVXNonZeroWeights;

    gcVXNonZeroWeights *nonZeroWeights = VX_NULL;
    vx_int32 m = 0;
    vx_uint32 core0FilterCount = 0;
    gcVXNonZeroWeights tmp;

    nonZeroWeights = (gcVXNonZeroWeights*)vxAllocateAndZeroMemory(filterTotalCount * sizeof(gcVXNonZeroWeights));
    if (nonZeroWeights == VX_NULL)
    {
        vxError("fillinKernelBuffer: OUT OF MEMORY");
        return 0;
    }

    /* calc each filter non-zero weights num*/
    for (filterIndex = 0; filterIndex < filterTotalCount; filterIndex++)
    {
        nonZeroWeights[filterIndex].filterIdx = filterIndex;
        for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
        {
            /* add one slice data every filter */
            kernelDataPtr = (vx_uint8*)weight_data + filterIndex * filterSize + filterSliceSize * sliceIndex;

            for (weightYIndex = 0; weightYIndex < wb->weights_sizes[1]; weightYIndex++)
            {
                for (weightXIndex = 0; weightXIndex < wb->weights_sizes[0]; weightXIndex++)
                {
                    vx_uint32 weight = 0;

                    if (weightFomat == VX_TYPE_INT8)
                        weight = *((vx_int8 *)kernelDataPtr);
                    else if (weightFomat == VX_TYPE_UINT8)
                        weight = *((vx_uint8 *)kernelDataPtr);
                    else
                        weight = *((vx_uint16 *)kernelDataPtr);
                    kernelDataPtr = kernelDataPtr + weightSize;

                    if (weight != skipValue)
                        nonZeroWeights[filterIndex].nonZeroCount++;

                }
            }
        }
        totalNonZeroCount += nonZeroWeights[filterIndex].nonZeroCount;
    }

    /*Sort*/
    averageNZCPerFilter = (vx_float32)totalNonZeroCount / filterTotalCount;
    for (filterIndex = 0; filterIndex < filterTotalCount; filterIndex++)
    {
        varianceFL32 += gcoMATH_Power((nonZeroWeights[filterIndex].nonZeroCount - averageNZCPerFilter), 2);
    }
    variance = (vx_uint32)(varianceFL32 / filterTotalCount + 0.5f);

    if (variance > 12)
        reorder = vx_true_e;

    if (reorder)
    {
        /*Sort*/
        for (filterIndex = 0; filterIndex < filterTotalCount; filterIndex++)
        {
            tmp.nonZeroCount = nonZeroWeights[filterIndex].nonZeroCount;
            tmp.filterIdx = filterIndex;

            for (m = filterIndex - 1; m >= 0 && nonZeroWeights[m].nonZeroCount > tmp.nonZeroCount; m--)
            {
                nonZeroWeights[m+1].nonZeroCount = nonZeroWeights[m].nonZeroCount;
                nonZeroWeights[m+1].filterIdx = nonZeroWeights[m].filterIdx;
            }
            nonZeroWeights[m+1].nonZeroCount = tmp.nonZeroCount;
            nonZeroWeights[m+1].filterIdx = tmp.filterIdx;
        }
    }

    if (weightFomat == VX_TYPE_INT16) biasBitSize = NN_INTEGER_BIAS_BITS_VIP_V7_INT16;

    startDataPtr = (vx_uint8*)weight_data;

    /* Write kernel Buffer size for each core. */
    /* Assume core count is at most 16. */
    kernelBufferSize = 64;

    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        vx_uint32 coreFilterCount = (coreIndex < oddFilterPerCore) ? totalFilterPerCore + 1 : totalFilterPerCore;
        vx_uint32 usedKid = 0;
        vx_uint32* filterGroup = VX_NULL;

        /* Write zeroRunLen and coreFilterCount. */
        bitOffset = 8 + 16;

        /* Write weight value and bias for every group. */
        zeroRun = 0;
        for (groupIndex = 0; groupIndex < groupCount; groupIndex++)
        {
            vx_uint32 actualFilterCount = (groupIndex == groupCount - 1) ? (coreFilterCount - groupIndex * filterCount) : filterCount;
            vx_uint32 groupFilterStart = groupIndex * nnCoreCount * filterCount;
            vx_uint32 kid = 0;
            vx_uint32 unusedCoreCount = (filterTotalCount % nnCoreCount == 0) ? 0 : nnCoreCount - (filterTotalCount % nnCoreCount);
            vx_uint32 sortedIndex = 0;
            vx_uint32 filterStart = 0;

            if (coreIndex == 0)
                core0FilterCount = actualFilterCount;

            if (actualFilterCount)
            {
                filterGroup = (vx_uint32*)vxAllocateAndZeroMemory(actualFilterCount * sizeof(vx_uint32));
                if (filterGroup == VX_NULL)
                {
                    vxError("fillinKernelBufferBalance: OUT OF MEMORY");
                    if (nonZeroWeights)
                        vxFree(nonZeroWeights);
                    return vx_false_e;
                }
            }

            if (reorder)
            {
                /*Dispatch filter to each vzGroup*/
                for (kid = 0; kid < actualFilterCount; kid++)
                {
                    if (groupIndex == groupCount - 1 &&
                        kid == core0FilterCount - 1)
                    {
                        if ((kid + usedKid) % 2 == 0)
                            sortedIndex = groupFilterStart + kid * nnCoreCount + coreIndex;
                        else
                            sortedIndex = groupFilterStart + (kid + 1) * nnCoreCount - coreIndex - 1 - unusedCoreCount;
                    }
                    else
                    {
                        if ((kid + usedKid) % 2 == 0)
                            sortedIndex = groupFilterStart + kid * nnCoreCount + coreIndex;
                        else
                            sortedIndex = groupFilterStart + (kid + 1) * nnCoreCount - coreIndex - 1;
                    }

                    filterGroup[kid] = nonZeroWeights[sortedIndex].filterIdx;
                }

                /*sort the vz group real filter to avoid write output cross*/
                for (kid = 0; kid < actualFilterCount; kid++)
                {
                    vx_uint32 tmp = filterGroup[kid];

                    for (m = kid - 1; m >= 0 && filterGroup[m] > tmp; m--)
                    {
                        filterGroup[m+1] = filterGroup[m];
                    }
                    filterGroup[m+1] = tmp;
                }
            }
            else
            {
                /*if variance is small, don't reorder the filters*/
                if ((groupIndex == groupCount - 1)
                && (coreIndex >= (filterTotalCount % nnCoreCount)))
                {
                    filterStart = groupIndex * nnCoreCount * filterCount
                                + (filterTotalCount % nnCoreCount) * (actualFilterCount + 1)
                                + (coreIndex - (filterTotalCount % nnCoreCount)) * actualFilterCount;
                }
                else
                {
                    filterStart = groupIndex * nnCoreCount *filterCount + coreIndex * actualFilterCount;
                }

                for (kid = 0; kid < actualFilterCount; kid++)
                {
                    filterGroup[kid] = filterStart + kid;
                }
            }

            if (wb->weights_sizes[0] == 1 && wb->weights_sizes[1] == 1 &&
                (hasZDP3 || hasZDP6) &&
                (weightFomat == VX_TYPE_INT8 || weightFomat == VX_TYPE_UINT8))
            {
                vx_uint32 zdpNum = hasZDP6 ? 6 : 3;

                /* zdp3 zdp6 can not enable same time */
                vxmASSERT(!(hasZDP3 && hasZDP6));

                for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex += (zdpNum * 2))
                {
                    vx_uint8 group2DCoded = 0;
                    vx_uint32 weight = 0;
                    vx_uint32 realSliceIndex = 0;

                    /* add slices of every filter*/
                    for (kid = 0; kid < actualFilterCount; kid++)
                    {
                        vx_uint32 nonZeroInBlock1Count = 0; /*Check if there's non-zero point in first zdpNum block*/
                        vx_uint32 nonZeroInBlock2Count = 0; /*Check if there's non-zero point in another zdpNum block*/

                        filterIndex = filterGroup[kid];
                        for (realSliceIndex = sliceIndex; realSliceIndex < sliceIndex + (zdpNum * 2); realSliceIndex++)
                        {
                            vx_uint8 needToWriteBias = (realSliceIndex == 0) ? 1 : 0;

                            if (realSliceIndex >= sliceCount)
                                break;

                            /* add one slice data every filter */
                            kernelDataPtr = startDataPtr + filterIndex * filterSize + filterSliceSize * realSliceIndex;
                            if (weightFomat == VX_TYPE_INT8)
                                weight = *((vx_int8 *)kernelDataPtr);
                            else if (weightFomat == VX_TYPE_UINT8)
                                weight = *((vx_uint8 *)kernelDataPtr);
                            else
                                weight = *((vx_uint16 *)kernelDataPtr);

                            if (realSliceIndex - sliceIndex < zdpNum && weight != skipValue)
                                nonZeroInBlock1Count++;
                            else if (weight != skipValue)
                                nonZeroInBlock2Count++;

                            if ((zeroRun == maxZeroRun)
                                || ((realSliceIndex == 0)
                                    && needToWriteBias)
                                || (realSliceIndex == sliceCount - 1)
                                || (weight != skipValue)
                                || ((kid == actualFilterCount - 1)
                                    && (group2DCoded == 0)))
                            {
                                /* Write zeroRun and weight. */
                                bitOffset += zeroRunLen + weightBitSize;
                                if (bitOffset >= 32)
                                {
                                    bitOffset -= 32;
                                    kernelBufferSize += 4;
                                }
                                zeroRun = 0;

                                if (needToWriteBias)
                                {
                                    bitOffset += biasBitSize;
                                    /* Write bias. */
                                    if (bitOffset >= 32)
                                    {
                                        bitOffset -= 32;
                                        kernelBufferSize += 4;
                                    }
                                    needToWriteBias = 0;
                                }
                                group2DCoded = 1;
                                rsvWeightCount++;
                            }
                            else
                            {
                                zeroRun++;
                            }

                            /* add offet behind the last point of the last slice of each filter */
                            if (realSliceIndex == sliceCount - 1)
                            {
                                if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                    bitOffset += NN_Z_POSITION_OFFSET_BITS_VIP_V7;
                                else
                                    bitOffset += NN_Z_POSITION_OFFSET_BITS;
                                if (bitOffset >= 32)
                                {
                                    bitOffset -= 32;
                                    kernelBufferSize += 4;
                                }
                            }
                        }
                        if (sliceIndex + zdpNum >= sliceCount)
                            blockCount++;
                        else
                            blockCount += 2;

                        if (nonZeroInBlock1Count)
                            nonZeroCount++;
                        if (nonZeroInBlock2Count)
                            nonZeroCount++;
                    }
                }
            }
            else if ((hasXYDP9 || hasXYDP6)
                     && (weightFomat == VX_TYPE_INT8 || weightFomat == VX_TYPE_UINT8))
            {
                vx_uint32 xStep = 3;
                vx_uint32 yStep = 0;
                vx_uint32 inImageBufferSize = hasXYDP9 ? 2 : 1;

                if (hasXYDP6)
                    yStep = 2;
                else
                    yStep = 3;

                for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex += inImageBufferSize)
                {
                    vx_uint8 group2DCoded = 0;
                    vx_uint32 weight = 0;
                    vx_uint32 realSliceIndex = 0;
                    vx_uint32 realWeightXIndex = 0, realWeightYIndex = 0;
                    vx_uint32 subKxSize = 0, subKySize = 0, subKzSize = 0;

                    subKzSize = gcmMIN((sliceCount - sliceIndex), inImageBufferSize);
                    /* add slices of every filter*/
                    for (kid = 0; kid < actualFilterCount; kid++)
                    {
                        vx_bool hasWriteBias = vx_false_e;
                        filterIndex = filterGroup[kid];

                        for (weightYIndex = 0; weightYIndex < wb->weights_sizes[1]; weightYIndex += yStep)
                        {
                            subKySize = gcmMIN((wb->weights_sizes[1] - weightYIndex), yStep);
                            for (weightXIndex = 0; weightXIndex < wb->weights_sizes[0]; weightXIndex += xStep)
                            {
                                subKxSize = gcmMIN((wb->weights_sizes[0] - weightXIndex), xStep);
                                for (realSliceIndex = sliceIndex; realSliceIndex < sliceIndex + subKzSize; realSliceIndex++)
                                {
                                    vx_uint8 needToWriteBias = (sliceIndex == 0) ? 1 : 0;
                                    vx_uint32 nonZeroInBlockCount = 0; /*Check if there's non-zero point in one 3x3 block*/

                                    /* Add one slice data every filter. */
                                    kernelDataPtr = startDataPtr + filterIndex * filterSize + filterSliceSize * realSliceIndex;

                                    for (realWeightYIndex = weightYIndex; realWeightYIndex < weightYIndex + subKySize; realWeightYIndex++)
                                    {
                                        for (realWeightXIndex = weightXIndex; realWeightXIndex < weightXIndex + subKxSize; realWeightXIndex++)
                                        {
                                            vx_uint8_ptr realKernelDataPtr = kernelDataPtr + (realWeightYIndex * wb->weights_sizes[0] + realWeightXIndex) * weightSize;

                                            if (weightFomat == VX_TYPE_INT8)
                                                weight = *((vx_int8 *)realKernelDataPtr);
                                            else if (weightFomat == VX_TYPE_UINT8)
                                                weight = *((vx_uint8 *)realKernelDataPtr);
                                            else
                                                weight = *((vx_uint16 *)realKernelDataPtr);

                                            if (weight != skipValue)
                                                nonZeroInBlockCount++;

                                            if ((zeroRun == maxZeroRun)
                                                || ((realSliceIndex == 0)
                                                    && (realWeightXIndex == wb->weights_sizes[0] - 1)
                                                    && (realWeightYIndex == wb->weights_sizes[1] - 1)
                                                    && needToWriteBias)
                                                || ((realSliceIndex == sliceCount - 1)
                                                    && (realWeightXIndex == wb->weights_sizes[0] - 1)
                                                    && (realWeightYIndex == wb->weights_sizes[1] - 1))
                                                || (weight != skipValue)
                                                || ((kid == actualFilterCount - 1)
                                                    && (realWeightXIndex == wb->weights_sizes[0] - 1)
                                                    && (realWeightYIndex == wb->weights_sizes[1] - 1)
                                                    && (group2DCoded == 0)))
                                            {
                                                /* Write zeroRun and weight. */
                                                bitOffset += zeroRunLen + weightBitSize;
                                                if (bitOffset >= 32)
                                                {
                                                    bitOffset -= 32;
                                                    kernelBufferSize += 4;
                                                }
                                                zeroRun = 0;

                                                if (needToWriteBias && !hasWriteBias)
                                                {
                                                    bitOffset += biasBitSize;
                                                    /* Write bias. */
                                                    if (bitOffset >= 32)
                                                    {
                                                        bitOffset -= 32;
                                                        kernelBufferSize += 4;
                                                    }
                                                    needToWriteBias = 0;
                                                    hasWriteBias = vx_true_e;
                                                }
                                                group2DCoded = 1;
                                                rsvWeightCount++;
                                            }
                                            else
                                            {
                                                zeroRun++;
                                            }

                                            /* add offet behind the last point of the last slice of each filter */
                                            if (realSliceIndex == sliceCount - 1 && realWeightXIndex == wb->weights_sizes[0] - 1 && realWeightYIndex == wb->weights_sizes[1] - 1)
                                            {
                                                 /* Write offsetValue. */
                                                if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                                    bitOffset += NN_Z_POSITION_OFFSET_BITS_VIP_V7;
                                                else
                                                    bitOffset += NN_Z_POSITION_OFFSET_BITS;

                                                if (bitOffset >= 32)
                                                {
                                                    bitOffset -= 32;
                                                    kernelBufferSize += 4;
                                                }
                                            }
                                        }
                                    }
                                    blockCount++;
                                    if (nonZeroInBlockCount)
                                        nonZeroCount++;
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
                {
                    vx_uint8 group2DCoded = 0;

                    /* Add slices of every filter. */
                    for (kid = 0; kid < actualFilterCount; kid++)
                    {
                        vx_uint8 needToWriteBias = (sliceIndex == 0) ? 1 : 0;

                        filterIndex = filterGroup[kid];

                        /* Add one slice data every filter. */
                        kernelDataPtr = startDataPtr + filterIndex * filterSize + filterSliceSize * sliceIndex;

                        for (weightYIndex = 0; weightYIndex < wb->weights_sizes[1]; weightYIndex++)
                        {
                            vx_uint32 nonZeroCountInDP3 = 0; /*Check if there's non-zero point in one 3x1 block*/
                            for (weightXIndex = 0; weightXIndex < wb->weights_sizes[0]; weightXIndex++)
                            {
                                vx_uint32 weight;

                                if (weightFomat == VX_TYPE_INT8)
                                    weight = *((vx_int8 *)kernelDataPtr);
                                else if (weightFomat == VX_TYPE_UINT8)
                                    weight = *((vx_uint8 *)kernelDataPtr);
                                else
                                    weight = *((vx_uint16 *)kernelDataPtr);
                                kernelDataPtr = kernelDataPtr + weightSize;

                                if(!hasVipV7Feature || weightFomat == VX_TYPE_FLOAT16)
                                {
                                    /*V6 or FP16, DP1, a block is 1x1*/
                                    blockCount++;
                                    if (weight != skipValue)
                                        nonZeroCount++;
                                }
                                else
                                {
                                    if (weight != skipValue)
                                        nonZeroCountInDP3++;

                                    /*V7, DP3, one block is 3x1*/
                                    if ((weightXIndex + 1) % 3 == 0 || weightXIndex == wb->weights_sizes[0] - 1)
                                    {
                                        blockCount++;
                                        if (nonZeroCountInDP3)
                                            nonZeroCount++;
                                        nonZeroCountInDP3 = 0;
                                    }
                                }

                                if ((zeroRun == maxZeroRun)
                                    || ((sliceIndex == 0)
                                        && (weightXIndex == wb->weights_sizes[0] - 1)
                                        && (weightYIndex == wb->weights_sizes[1] - 1)
                                        && needToWriteBias)
                                    || ((weightXIndex == wb->weights_sizes[0] - 1)
                                        && (weightYIndex == wb->weights_sizes[1] - 1)
                                        && (sliceIndex == sliceCount - 1))
                                    || (weight != skipValue)
                                    || ((kid == actualFilterCount - 1)
                                        && (weightXIndex == wb->weights_sizes[0] - 1)
                                        && (weightYIndex == wb->weights_sizes[1] - 1)
                                        && (group2DCoded == 0)))
                                {
                                    /* Write zeroRun and weight. */
                                    bitOffset += zeroRunLen + weightBitSize;
                                    if (bitOffset >= 32)
                                    {
                                        bitOffset -= 32;
                                        kernelBufferSize += 4;
                                    }
                                    zeroRun = 0;

                                    if (needToWriteBias)
                                    {
                                        bitOffset += biasBitSize;
                                        /* Write bias. */
                                        if (bitOffset >= 32)
                                        {
                                            bitOffset -= 32;
                                            kernelBufferSize += 4;
                                        }
                                        needToWriteBias = 0;
                                    }
                                    group2DCoded = 1;
                                    rsvWeightCount++;
                                }
                                else
                                {
                                    zeroRun++;
                                }
                            }
                        }

                        /* add offet behind the last point of the last slice of each filter */
                        if (sliceIndex == sliceCount - 1)
                        {
                            /* Write offsetValue. */
                            if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                bitOffset += NN_Z_POSITION_OFFSET_BITS_VIP_V7;
                            else
                                bitOffset += NN_Z_POSITION_OFFSET_BITS;
                            if (bitOffset >= 32)
                            {
                                bitOffset -= 32;
                                kernelBufferSize += 4;
                            }
                        }
                    }
                }
            }
            usedKid += actualFilterCount;
            if (filterGroup)
            {
                vxFree(filterGroup);
                filterGroup = VX_NULL;
            }
        }

        /* pad 0 */
        if (bitOffset)
        {
            kernelBufferSize += 4;
        }
        kernelBufferSize = (kernelBufferSize + 63) & 0xFFFFFFC0;
    }

    wb->slice_array[index].all_count = blockCount;
    wb->slice_array[index].reserve_weight_count = rsvWeightCount;
    wb->slice_array[index].non_zero_count = nonZeroCount;
    wb->slice_array[index].kernel_orig_size = (filterSize + biasSize) * filterTotalCount;

    if (nonZeroWeights)
        vxFree(nonZeroWeights);

    return kernelBufferSize;
}

vx_bool calculateWeightBiasBalanceSizeForZeroRunLenEx(
    vx_context context,
    vx_weights_biases_parameter wb,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_uint32 skip_value,
    gctPOINTER weight_data,
    vx_uint32 index,
    vx_size* min_size,
    vx_uint8* zero_run_len
    )
{
    vx_enum weightFomat = weight_format;
    vx_uint32 skipValue = skip_value;

    vx_uint32 nnCoreCount = weightFomat == VX_TYPE_INT16 ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : weightFomat == VX_TYPE_FLOAT16 ?
                      context->nnConfig.fixedFeature.nnCoreCountFloat16 : context->nnConfig.fixedFeature.nnCoreCount;

    vx_uint32 filterTotalCount   = z_count;
    vx_uint32 filterCount        = filters_per_core; /* filter count every group */
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;
    vx_uint32 totalFilterPerCore = filterTotalCount / nnCoreCount;
    vx_uint32 oddFilterPerCore   = filterTotalCount % nnCoreCount;

    vx_uint32 weightCount        = wb->weights_sizes[0] * wb->weights_sizes[1];
    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize(weightFomat);
    vx_uint32 weightBitSize      = weightSize * 8;
    vx_uint32 biasSize           = (vx_uint32)vxDataType_GetSize(bias_format);
    vx_uint32 biasBitSize        = biasSize * 8;

    vx_uint32 sliceCount         = slice_count;
    vx_uint32 filterSliceSize    = weightCount * weightSize;
    vx_uint32 filterSize         = weightCount * weightSize * sliceCount;
    vx_uint8* startDataPtr       = VX_NULL;
    vx_uint8* kernelDataPtr      = VX_NULL;

    vx_uint32 bitOffset          = 0;

    vx_uint32 coreIndex;
    vx_uint32 i, j, groupIndex, filterIndex, sliceIndex;
    vx_uint32 weightXIndex, weightYIndex;

    vx_uint32 *origBitsArray = VX_NULL, *maxBasePad = VX_NULL;
    vx_uint32 *zerosPerCoreArray = VX_NULL; /* zerosPerCoreArray[nnCoreCount][MAX_ZRL_LEN + 1 + 2] */
    vx_uint32 minSize = (vx_uint32)~0UL, maxZRL = 0, maxZRLType, bigZRL = 0, blockCount = 0, nonZeroCount = 0;
    vx_uint8  minZrl = 0;
    vx_bool   complete;
    vx_bool hasVipV7Feature = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7) == gcvSTATUS_TRUE ? vx_true_e : vx_false_e;
    vx_bool hasZDP3 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3);
    vx_bool hasZDP6 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6);
    vx_bool hasXYDP9 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP9);
    vx_bool hasXYDP6 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP6);

    vx_uint32 maxZeroRunLen = (1 << context->nnConfig.fixedFeature.zrlBits) - 1;

    vx_uint32 totalNonZeroCount = 0;
    vx_float32 averageNZCPerFilter = 0;
    vx_float32 varianceFL32 = 0;
    vx_uint32 variance = 0;
    vx_bool reorder = vx_false_e;

    typedef struct _gcVXNonZeroWeights
    {
        vx_uint32 nonZeroCount;
        vx_uint32 filterIdx;
    }gcVXNonZeroWeights;

    gcVXNonZeroWeights *nonZeroWeights = VX_NULL;
    vx_int32 m = 0;
    vx_uint32 core0FilterCount = 0;
    gcVXNonZeroWeights tmp;

    nonZeroWeights = (gcVXNonZeroWeights*)vxAllocateAndZeroMemory(filterTotalCount * sizeof(gcVXNonZeroWeights));
    if (nonZeroWeights == VX_NULL)
    {
        vxError("fillinKernelBuffer: OUT OF MEMORY");
        return vx_false_e;
    }

    /* calc each filter non-zero weights num*/
    for (filterIndex = 0; filterIndex < filterTotalCount; filterIndex++)
    {
        nonZeroWeights[filterIndex].filterIdx = filterIndex;
        for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
        {
            /* add one slice data every filter */
            kernelDataPtr = (vx_uint8*)weight_data + filterIndex * filterSize + filterSliceSize * sliceIndex;

            for (weightYIndex = 0; weightYIndex < wb->weights_sizes[1]; weightYIndex++)
            {
                for (weightXIndex = 0; weightXIndex < wb->weights_sizes[0]; weightXIndex++)
                {
                    vx_uint32 weight = 0;

                    if (weightFomat == VX_TYPE_INT8)
                        weight = *((vx_int8 *)kernelDataPtr);
                    else if (weightFomat == VX_TYPE_UINT8)
                        weight = *((vx_uint8 *)kernelDataPtr);
                    else
                        weight = *((vx_uint16 *)kernelDataPtr);
                    kernelDataPtr = kernelDataPtr + weightSize;

                    if (weight != skipValue)
                        nonZeroWeights[filterIndex].nonZeroCount++;

                }
            }
        }
        totalNonZeroCount += nonZeroWeights[filterIndex].nonZeroCount;
    }

    /*Sort*/
    averageNZCPerFilter = (vx_float32)totalNonZeroCount / filterTotalCount;
    for (filterIndex = 0; filterIndex < filterTotalCount; filterIndex++)
    {
        varianceFL32 += gcoMATH_Power((nonZeroWeights[filterIndex].nonZeroCount - averageNZCPerFilter), 2);
    }
    variance = (vx_uint32)(varianceFL32 / filterTotalCount + 0.5f);

    if (variance > 12)
        reorder = vx_true_e;

    if (reorder)
    {
        /*Sort*/
        for (filterIndex = 0; filterIndex < filterTotalCount; filterIndex++)
        {
            tmp.nonZeroCount = nonZeroWeights[filterIndex].nonZeroCount;
            tmp.filterIdx = filterIndex;

            for (m = filterIndex - 1; m >= 0 && nonZeroWeights[m].nonZeroCount > tmp.nonZeroCount; m--)
            {
                nonZeroWeights[m+1].nonZeroCount = nonZeroWeights[m].nonZeroCount;
                nonZeroWeights[m+1].filterIdx = nonZeroWeights[m].filterIdx;
            }
            nonZeroWeights[m+1].nonZeroCount = tmp.nonZeroCount;
            nonZeroWeights[m+1].filterIdx = tmp.filterIdx;
        }
    }


    if (weightFomat == VX_TYPE_INT16) biasBitSize = NN_INTEGER_BIAS_BITS_VIP_V7_INT16;

    origBitsArray = (vx_uint32 *)vxAllocateAndZeroMemory(nnCoreCount * sizeof(vx_uint32));
    vxmASSERT(origBitsArray != NULL);
    maxBasePad = (vx_uint32 *)vxAllocateAndZeroMemory(nnCoreCount * sizeof(vx_uint32));
    vxmASSERT(maxBasePad != NULL);
    zerosPerCoreArray = (vx_uint32 *)vxAllocateAndZeroMemory(nnCoreCount * (maxZeroRunLen + 1 + 2) * sizeof(vx_uint32));
    vxmASSERT(zerosPerCoreArray != NULL);
    if (origBitsArray == VX_NULL || maxBasePad == NULL || zerosPerCoreArray == NULL)
    {
        if (origBitsArray)
            vxFree(origBitsArray);
        if (maxBasePad)
            vxFree(maxBasePad);
        if (zerosPerCoreArray)
            vxFree(zerosPerCoreArray);
        if (nonZeroWeights)
            vxFree(nonZeroWeights);
        vxError("calculateWeightBiasBufferSizeForZeroRunLenEx: OUT OF MEMORY");
        return vx_false_e;
    }
    startDataPtr = (vx_uint8*)weight_data;

    /* Write kernel Buffer size for each core. */
    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        vx_uint32 coreFilterCount = (coreIndex < oddFilterPerCore) ? totalFilterPerCore + 1 : totalFilterPerCore;
        vx_uint32 zeroRun = 0;
        vx_uint32 usedKid = 0;
        vx_uint32* filterGroup;

        /* Write zeroRunLen and coreFilterCount. */
        bitOffset = 8 + 16;

        /* Write weight value and bias for every group. */
        for (groupIndex = 0; groupIndex < groupCount; groupIndex++)
        {
            vx_uint32 actualFilterCount = (groupIndex == groupCount - 1) ? (coreFilterCount - groupIndex * filterCount) : filterCount;
            vx_uint32 groupFilterStart = groupIndex * nnCoreCount * filterCount;
            vx_uint32 kid = 0;
            vx_uint32 unusedCoreCount = (filterTotalCount % nnCoreCount == 0) ? 0 : nnCoreCount - (filterTotalCount % nnCoreCount);
            vx_uint32 sortedIndex = 0;
            vx_uint32 filterStart = 0;

            if (coreIndex == 0)
                core0FilterCount = actualFilterCount;
            if (actualFilterCount)
            {
                filterGroup = (vx_uint32*)vxAllocateAndZeroMemory(actualFilterCount * sizeof(vx_uint32));
                if (filterGroup == VX_NULL)
                {
                    vxError("fillinKernelBufferBalance: OUT OF MEMORY");
                    if (nonZeroWeights)
                        vxFree(nonZeroWeights);
                    if (origBitsArray)
                        vxFree(origBitsArray);
                    if (maxBasePad)
                        vxFree(maxBasePad);
                    if (zerosPerCoreArray)
                        vxFree(zerosPerCoreArray);
                    return vx_false_e;
                }
            }
            else
                continue;

            if (reorder)
            {
                /*Dispatch filter to each vzGroup*/
                for (kid = 0; kid < actualFilterCount; kid++)
                {
                    if (groupIndex == groupCount - 1 &&
                        kid == core0FilterCount - 1)
                    {
                        if ((kid + usedKid) % 2 == 0)
                            sortedIndex = groupFilterStart + kid * nnCoreCount + coreIndex;
                        else
                            sortedIndex = groupFilterStart + (kid + 1) * nnCoreCount - coreIndex - 1 - unusedCoreCount;
                    }
                    else
                    {
                        if ((kid + usedKid) % 2 == 0)
                            sortedIndex = groupFilterStart + kid * nnCoreCount + coreIndex;
                        else
                            sortedIndex = groupFilterStart + (kid + 1) * nnCoreCount - coreIndex - 1;
                    }

                    filterGroup[kid] = nonZeroWeights[sortedIndex].filterIdx;
                }

                /*sort the vz group real filter to avoid write output cross*/
                for (kid = 0; kid < actualFilterCount; kid++)
                {
                    vx_uint32 tmp = filterGroup[kid];

                    for (m = kid - 1; m >= 0 && filterGroup[m] > tmp; m--)
                    {
                        filterGroup[m+1] = filterGroup[m];
                    }
                    filterGroup[m+1] = tmp;
                }
            }
            else
            {
                /*if variance is small, don't reorder the filters*/
                if ((groupIndex == groupCount - 1)
                && (coreIndex >= (filterTotalCount % nnCoreCount)))
                {
                    filterStart = groupIndex * nnCoreCount * filterCount
                                + (filterTotalCount % nnCoreCount) * (actualFilterCount + 1)
                                + (coreIndex - (filterTotalCount % nnCoreCount)) * actualFilterCount;
                }
                else
                {
                    filterStart = groupIndex * nnCoreCount *filterCount + coreIndex * actualFilterCount;
                }

                for (kid = 0; kid < actualFilterCount; kid++)
                {
                    filterGroup[kid] = filterStart + kid;
                }
            }

            if (wb->weights_sizes[0] == 1 && wb->weights_sizes[1] == 1 &&
                (hasZDP3 || hasZDP6) &&
                (weightFomat == VX_TYPE_INT8 || weightFomat == VX_TYPE_UINT8))
            {
                vx_uint32 zdpNum = hasZDP6 ? 6 : 3;

                /* zdp3 zdp6 can not enable same time */
                vxmASSERT(!(hasZDP3 && hasZDP6));

                for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex += (zdpNum * 2))
                {
                    vx_uint8 group2DCoded = 0;
                    vx_uint32 weight = 0;
                    vx_uint32 realSliceIndex = 0;

                    /* add slices of every filter*/
                    for (kid = 0; kid < actualFilterCount; kid++)
                    {
                        vx_uint32 nonZeroInBlock1Count = 0; /*Check if there's non-zero point in first zdpNum block*/
                        vx_uint32 nonZeroInBlock2Count = 0; /*Check if there's non-zero point in another zdpNum block*/

                        filterIndex = filterGroup[kid];


                        for (realSliceIndex = sliceIndex; realSliceIndex < sliceIndex + (zdpNum * 2); realSliceIndex++)
                        {
                            vx_uint8 needToWriteBias = (realSliceIndex == 0) ? 1 : 0;

                            if (realSliceIndex >= slice_count)
                                break;

                            /* add one slice data every filter */
                            kernelDataPtr = startDataPtr + filterIndex * filterSize + filterSliceSize * realSliceIndex;
                            if (weightFomat == VX_TYPE_INT8)
                                weight = *((vx_int8 *)kernelDataPtr);
                            else if (weightFomat == VX_TYPE_UINT8)
                                weight = *((vx_uint8 *)kernelDataPtr);
                            else
                                weight = *((vx_uint16 *)kernelDataPtr);

                            if (realSliceIndex - sliceIndex < zdpNum && weight != skipValue)
                                nonZeroInBlock1Count++;
                            else if (weight != skipValue)
                                nonZeroInBlock2Count++;

                            if (((realSliceIndex == 0)
                                    && needToWriteBias)
                                || (realSliceIndex == slice_count - 1)
                                || (weight != skipValue)
                                || ((kid == actualFilterCount - 1)
                                    && (group2DCoded == 0)))
                            {
                                /* Write zeroRun and weight. */
                                if (zeroRun > maxZeroRunLen)
                                {
                                    *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 1) += 1;
                                    *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 2) += zeroRun - maxZeroRunLen;
                                    maxBasePad[coreIndex] += (maxZeroRunLen + 1 + zeroRun) / (maxZeroRunLen + 1);
                                }
                                else
                                {
                                    *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + zeroRun) += 1;
                                }
                                if (zeroRun > maxZRL) maxZRL = zeroRun;
                                zeroRun = 0;

                                if (needToWriteBias)
                                {
                                    bitOffset += biasBitSize;
                                    needToWriteBias = 0;
                                }
                                group2DCoded = 1;
                            }
                            else
                            {
                                zeroRun++;
                            }

                            /* add offet behind the last point of the last slice of each filter */
                            if (realSliceIndex == slice_count - 1)
                            {
                                if (hasVipV7Feature)
                                    bitOffset += NN_Z_POSITION_OFFSET_BITS_VIP_V7;
                                else
                                    bitOffset += NN_Z_POSITION_OFFSET_BITS;
                            }
                        }

                        if (sliceIndex + zdpNum >= slice_count)
                            blockCount++;
                        else
                            blockCount += 2;

                        if (nonZeroInBlock1Count)
                            nonZeroCount++;
                        if (nonZeroInBlock2Count)
                            nonZeroCount++;
                    }
                }
            }
            else if ((hasXYDP9 || hasXYDP6)
                && (weightFomat == VX_TYPE_INT8 || weightFomat == VX_TYPE_UINT8))
            {
                vx_uint32 xStep = 3;
                vx_uint32 yStep = 0;
                vx_uint32 inImageBufferSize = hasXYDP9 ? 2 : 1;

                if (hasXYDP6)
                    yStep = 2;
                else
                    yStep = 3;

                for (sliceIndex = 0; sliceIndex < slice_count; sliceIndex += inImageBufferSize)
                {
                    vx_uint8 group2DCoded = 0;
                    vx_uint32 weight = 0;
                    vx_uint32 realSliceIndex = 0;
                    vx_uint32 realWeightXIndex = 0, realWeightYIndex = 0;
                    vx_uint32 subKxSize = 0, subKySize = 0, subKzSize = 0;

                    subKzSize = gcmMIN((slice_count - sliceIndex), inImageBufferSize);
                    /* add slices of every filter*/
                    for (kid = 0; kid < actualFilterCount; kid++)
                    {
                        vx_bool hasWriteBias = vx_false_e;

                        filterIndex = filterGroup[kid];


                        for (weightYIndex = 0; weightYIndex < wb->weights_sizes[1]; weightYIndex += yStep)
                        {
                            subKySize = gcmMIN((wb->weights_sizes[1] - weightYIndex), yStep);
                            for (weightXIndex = 0; weightXIndex < wb->weights_sizes[0]; weightXIndex += xStep)
                            {
                                subKxSize = gcmMIN((wb->weights_sizes[0] - weightXIndex), xStep);
                                for (realSliceIndex = sliceIndex; realSliceIndex < sliceIndex + subKzSize; realSliceIndex++)
                                {
                                    vx_uint8 needToWriteBias = (sliceIndex == 0) ? 1 : 0;
                                    vx_uint32 nonZeroInBlockCount = 0; /*Check if there's non-zero point in one 3x3 block*/

                                    /* Add one slice data every filter. */
                                    kernelDataPtr = startDataPtr + filterIndex * filterSize + filterSliceSize * realSliceIndex;

                                    for (realWeightYIndex = weightYIndex; realWeightYIndex < weightYIndex + subKySize; realWeightYIndex++)
                                    {
                                        for (realWeightXIndex = weightXIndex; realWeightXIndex < weightXIndex + subKxSize; realWeightXIndex++)
                                        {
                                            vx_uint8_ptr realKernelDataPtr = kernelDataPtr + (realWeightYIndex * wb->weights_sizes[0] + realWeightXIndex) * weightSize;

                                            if (weightFomat == VX_TYPE_INT8)
                                                weight = *((vx_int8 *)realKernelDataPtr);
                                            else if (weightFomat == VX_TYPE_UINT8)
                                                weight = *((vx_uint8 *)realKernelDataPtr);
                                            else
                                                weight = *((vx_uint16 *)realKernelDataPtr);

                                            if (weight != skipValue)
                                                nonZeroInBlockCount++;

                                            if (((realSliceIndex == 0)
                                                    && (realWeightXIndex == wb->weights_sizes[0] - 1)
                                                    && (realWeightYIndex == wb->weights_sizes[1] - 1)
                                                    && needToWriteBias)
                                                || ((realSliceIndex == slice_count - 1)
                                                    && (realWeightXIndex == wb->weights_sizes[0] - 1)
                                                    && (realWeightYIndex == wb->weights_sizes[1] - 1))
                                                || (weight != skipValue)
                                                || ((kid == actualFilterCount - 1)
                                                    && (realWeightXIndex == wb->weights_sizes[0] - 1)
                                                    && (realWeightYIndex == wb->weights_sizes[1] - 1)
                                                    && (group2DCoded == 0)))
                                            {
                                                if (zeroRun > maxZeroRunLen)
                                                {
                                                    *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 1) += 1;
                                                    *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 2) += zeroRun - maxZeroRunLen;
                                                    maxBasePad[coreIndex] += (maxZeroRunLen + 1 + zeroRun) / (maxZeroRunLen + 1);
                                                }
                                                else
                                                {
                                                    *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + zeroRun) += 1;
                                                }
                                                if (zeroRun > maxZRL) maxZRL = zeroRun;
                                                zeroRun = 0;

                                                if (needToWriteBias && !hasWriteBias)
                                                {
                                                    bitOffset += biasBitSize;
                                                    needToWriteBias = 0;
                                                    hasWriteBias = vx_true_e;
                                                }
                                                group2DCoded = 1;
                                            }
                                            else
                                            {
                                                zeroRun++;
                                            }

                                            /* add offet behind the last point of the last slice of each filter */
                                            if (realSliceIndex == slice_count - 1 && realWeightXIndex == wb->weights_sizes[0] - 1 && realWeightYIndex == wb->weights_sizes[1] - 1)
                                            {
                                                 /* Write offsetValue. */
                                                if (hasVipV7Feature)
                                                    bitOffset += NN_Z_POSITION_OFFSET_BITS_VIP_V7;
                                                else
                                                    bitOffset += NN_Z_POSITION_OFFSET_BITS;
                                            }
                                        }
                                    }

                                    blockCount++;
                                    if (nonZeroInBlockCount)
                                        nonZeroCount++;
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                for (sliceIndex = 0; sliceIndex < slice_count; sliceIndex++)
                {
                    vx_uint8 group2DCoded = 0;

                    /* Add slices of every filter. */
                    for (kid = 0; kid < actualFilterCount; kid++)
                    {
                        vx_uint8 needToWriteBias = (sliceIndex == 0) ? 1 : 0;

                        filterIndex = filterGroup[kid];


                        /* Add one slice data every filter. */
                        kernelDataPtr = startDataPtr + filterIndex * filterSize + filterSliceSize * sliceIndex;

                        for (weightYIndex = 0; weightYIndex < wb->weights_sizes[1]; weightYIndex++)
                        {
                            vx_uint32 nonZeroCountInDP3 = 0; /*Check if there's non-zero point in one 3x1 block*/

                            for (weightXIndex = 0; weightXIndex < wb->weights_sizes[0]; weightXIndex++)
                            {
                                vx_uint32 weight;

                                if (weightFomat == VX_TYPE_INT8)
                                    weight = *((vx_int8 *)kernelDataPtr);
                                else if (weightFomat == VX_TYPE_UINT8)
                                    weight = *((vx_uint8 *)kernelDataPtr);
                                else
                                    weight = *((vx_uint16 *)kernelDataPtr);
                                kernelDataPtr = kernelDataPtr + weightSize;

                                if(!hasVipV7Feature || weightFomat == VX_TYPE_FLOAT16)
                                {
                                    /*V6 or FP16, DP1, a block is 1x1*/
                                    blockCount++;
                                    if (weight != skipValue)
                                        nonZeroCount++;
                                }
                                else
                                {
                                    if (weight != skipValue)
                                        nonZeroCountInDP3++;

                                    /*V7, DP3, one block is 3x1*/
                                    if ((weightXIndex + 1) % 3 == 0 || weightXIndex == wb->weights_sizes[0] - 1)
                                    {
                                        blockCount++;
                                        if (nonZeroCountInDP3)
                                            nonZeroCount++;
                                        nonZeroCountInDP3 = 0;
                                    }
                                }

                                if (((sliceIndex == 0)
                                        && (weightXIndex == wb->weights_sizes[0] - 1)
                                        && (weightYIndex == wb->weights_sizes[1] - 1)
                                        && needToWriteBias)
                                    || ((weightXIndex == wb->weights_sizes[0] - 1)
                                        && (weightYIndex == wb->weights_sizes[1] - 1)
                                        && (sliceIndex == slice_count - 1))
                                    || (weight != skipValue)
                                    || ((kid == actualFilterCount - 1)
                                        && (weightXIndex == wb->weights_sizes[0] - 1)
                                        && (weightYIndex == wb->weights_sizes[1] - 1)
                                        && (group2DCoded == 0))
                                    )
                                {
                                    if (zeroRun > maxZeroRunLen)
                                    {
                                        *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 1) += 1;
                                        *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 2) += zeroRun - maxZeroRunLen;
                                        maxBasePad[coreIndex] += (maxZeroRunLen + 1 + zeroRun) / (maxZeroRunLen + 1);
                                    }
                                    else
                                    {
                                        *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + zeroRun) += 1;
                                    }
                                    if (zeroRun > maxZRL) maxZRL = zeroRun;
                                    zeroRun = 0;

                                    if (needToWriteBias)
                                    {
                                        bitOffset += biasBitSize;
                                        needToWriteBias = 0;
                                    }
                                    group2DCoded = 1;
                                }
                                else
                                {
                                    zeroRun++;
                                }
                            }
                        }

                        /* add offet behind the last point of the last slice of each filter */
                        if (sliceIndex == slice_count - 1)
                        {
                            /* Write offsetValue. */
                            if (hasVipV7Feature)
                                bitOffset += NN_Z_POSITION_OFFSET_BITS_VIP_V7;
                            else
                                bitOffset += NN_Z_POSITION_OFFSET_BITS;
                        }
                    }
                }
            }
            usedKid += actualFilterCount;
            if (filterGroup)
            {
                vxFree(filterGroup);
                filterGroup = VX_NULL;
            }
        }

        origBitsArray[coreIndex] = bitOffset;
        bigZRL += *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 1);
    }

    /* analyze compressed size with different zrl */
    maxZRL = gcmMIN(maxZRL, maxZeroRunLen);
    maxZRLType = maxZRL ? gcmMIN((vx_uint32)ceilf((vx_float32)(log(maxZRL) / log(2))), context->nnConfig.fixedFeature.zrlBits) : 0;
    complete = (vx_float32)bigZRL / blockCount > 0.1f ? vx_false_e : vx_true_e;
    for (i = 0; i <= maxZRLType; i++)
    {
        vx_uint32 size = 64;
        vx_uint32 base = (1 << i) - 1;
        vx_uint32 rsvWeightCount = 0;

        for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
        {
            vx_uint32 bits = 0, rwc = 0;

            if (base == 0)
            {
                /* take care zrl = 0 */
                for (j = 0; j <= maxZRL; j++)
                {
                    rwc += *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + j) * (j+1);
                }

                if (*(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 1) != 0)
                {
                    vx_uint32 nsum = *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 1);
                    vx_uint32 dsum = *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 2);

                    rwc += maxZeroRunLen * nsum + dsum + nsum;
                }

                bits += rwc * weightBitSize;
            }
            else
            {
                for (j = 0; j <= maxZRL; j++)
                {
                    vx_uint32 zerosPerCore = *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + j);
                    if (!zerosPerCore) continue;
                    if (j <= base) rwc += zerosPerCore;
                    else rwc += ((j + 1 + base) / (base + 1)) * zerosPerCore;
                }

                if (*(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 1) != 0)
                {
                    vx_uint32 nsum = *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 1);
                    vx_uint32 dsum = *(zerosPerCoreArray + coreIndex * (maxZeroRunLen + 1 + 2) + maxZeroRunLen + 2);
                    if (base != maxZeroRunLen)
                    {
                        rwc += ((maxZeroRunLen + 1 + base) * nsum + dsum) / (base + 1);
                    }
                    else
                    {
                        rwc += maxBasePad[coreIndex];
                    }
                }

                bits += rwc * (i + weightBitSize);
            }

            /* other bits */
            rsvWeightCount += rwc;
            bits += origBitsArray[coreIndex];
            size += (gcmALIGN(bits, 32) / 32) * 4;
            size  = gcmALIGN(size, 64);
        }

        if (size < minSize)
        {
            minSize = size;
            minZrl = (vx_uint8)i;
            wb->slice_array[index].reserve_weight_count = rsvWeightCount;
        }

        if (!complete) break;
    }

    wb->slice_array[index].all_count = blockCount;
    wb->slice_array[index].non_zero_count = nonZeroCount;
    wb->slice_array[index].kernel_orig_size = (filterSize + biasSize) * filterTotalCount;
    wb->slice_array[index].kernel_stream_size = minSize;

    if (origBitsArray)
        vxFree(origBitsArray);
    if (maxBasePad)
        vxFree(maxBasePad);
    if (zerosPerCoreArray)
        vxFree(zerosPerCoreArray);
    if (min_size != VX_NULL) *min_size = minSize;
    if (zero_run_len != VX_NULL) *zero_run_len = minZrl;

    if (nonZeroWeights)
        vxFree(nonZeroWeights);

    return complete;
}

#define MAX_HISTO_COUNT 256
#define MAX_SIZE_HISTO_COUNT 9

#define SET_1_LOG (1) /*Default is 1, can be 0, 1, 2, 0 is the best (small win), but can only put in 3 bit Huffman code*/
#define SET_1_SIZE (1<<SET_1_LOG)
#define THROUGHPUT 2       /*The decoder throughput, version 1.0, 2 bits*/

vx_int32 outBest2Run[MAX_RUNLEN_SIZE];
vx_int32 bestRunsSorted[MAX_RUNLEN_SIZE];
vx_int32 best2Runs[SET_1_SIZE], freq[SET_1_SIZE+MAX_RUNLEN_SIZE];
vx_int32 numBestRuns,freqSum;

const vx_int32 sizeCodeLen[] = {2, 2, 3, 3, 3, 4, 5, 5,};
const vx_int32 huffCode[8] = {0, 1, 7, 3, 6, 10, 18, 2};

typedef struct CmpStages2Sym
{
    vx_int32 stageCode[3];
    vx_int32 bitLength[3];
    vx_int32 hideBit;
    vx_int32 size;
}CodeSymbol;

#define DEBUG_COMP_ENHANCE_ENC 0
vx_int32 FindBestSubset(vx_int32 *runZeros, vx_int32 size, vx_int32 subSize, vx_int32 *subSet) /*Return how many bits needs*/
{
    vx_int32 i,j,k = 0, maxFreq;
    vx_int32 runs[256], addBreakFreq[256], minBreak;
    vx_float32 entropy, p;
    for(i = 0; i<256; i++)
        runs[i] = i+1;
    for(i = 0; i<2; i++){
       subSet[i] = i;
        freq[i] = runZeros[i];
        runs[i] = 0;
    }
    for(i = 2; i<subSize ; i++){
        for(j = 0; j<256; j++)
            addBreakFreq[j] = 0x7fffffff; /*big number*/
        for(j = 0; j<size; j++){
            if(runs[j] == 0)
                continue;
            subSet[i] = j;
            for(k = 0; k<i; k++){/*sorting*/
                if(subSet[k] > j){
                    break;
                }
            }
            if(k<i){ /*inset j to subSet[k]*/
                int n;
                for(n = i; n>k; n--)
                    subSet[n] = subSet[n-1];
                subSet[k] = j;
            }
            addBreakFreq[j] = runZeros[j];
            for(k = 0; k<size; k++){
                int n,m;
                if(k == j || runs[k] == 0)
                    continue;
                n = k+ 1;
                while(n){
                   for(m = 0; m<=i; m++){
                        if(subSet[i - m]+1 <= n){
                            addBreakFreq[j] += n/(subSet[i - m]+1) * runZeros[k];
                             n %= subSet[i - m]+1; /*k is separated to (subSet[j]+1)*p + k % (subSet[j]+1), but freq[j] also increase*/
                            break;
                        }
                    }
                }
            }
            for(k = 0; k<i; k++){/*Squeeze out j*/
                if(subSet[k] >= j)
                    subSet[k] = subSet[k+1];
            }
        }
        /*Now we find minimum added in*/
        minBreak = 0x7fffffff;
        for(j = 0; j<size; j++){
            if(addBreakFreq[j] < minBreak){
                k = j;
                minBreak = addBreakFreq[j];
            }
        }
        subSet[i] = k; /*We find the k, with less break;*/
        runs[k] = 0;
        freq[i] = runZeros[k];
        j = k;
        {   /*Sorting in increase order*/
            for(k = 0; k<i; k++){/*sorting*/
                if(subSet[k] > j){
                    break;
                }
            }
            if(k<i){ /*inset j to subSet[k]*/
                int n;
                for(n = i; n>k; n--){
                    subSet[n] = subSet[n-1];
                    freq[n] = freq[n-1];
                }
                subSet[k] = j;
                freq[k] = runZeros[j];
            }
        }

    }
    /*Now we sorted the subset, from largest to smallest*/
    for(i = 0; i<subSize; i++){
        freq[i] = runZeros[subSet[i]];
    }

    /*Since it is a subset, we have to make the other length not in the subset to the sum of subset element*/
    for(i = size - 1; i >= 0; i--){ /*run-length  = i+1*/
        if(runZeros[i]*runs[i]){
            k = i+1; /*runs*/
            while(k){
                for(j = subSize - 1; j >= 0; j--){
                    if(subSet[j]+1 <= k){
                        freq[j] += k/(subSet[j]+1) * runZeros[i];
                         k %= subSet[j]+1; /*k is separated to (subSet[j]+1)*p + k % (subSet[j]+1), but freq[j] also increase*/
                        break;
                    }
                }
                if(k && j < 0){
                    /*printf("Error at %d,%d\n",i,k);*/
                    break;
                }
            }
        }
    }
    /*Now we collect the best 2.*/
    freqSum = 0;
    for(i = 0; i < subSize; i++)
        freqSum += freq[i];

    for(i = 0; i<SET_1_SIZE; i++){
        maxFreq = 0;
        k = i;
        for(j = 0; j<subSize; j++){
            if(maxFreq < freq[j]){
                maxFreq = freq[j];
                k = j;
            }
        }
        if(1){ /*Swap the runlength and frequency*/
            freq[k] *= -1; /*negative it, never be selected again*/
        }
        best2Runs[i] = subSet[k];
    }
    maxFreq = 0;
    for(i = 0; i<subSize; i++){
        if(freq[i]<0){ /*best of 2*/
            maxFreq += -freq[i];
        }
    }
    for(j = 0, i = 0; i < subSize; i++){
        if(freq[i] > 0)
            outBest2Run[j++] = subSet[i];
    }
    /*Finally we calculate the number of bits spending. max2Freq, 1 bit, the rest, Log(subSize-2) bits*/
    p = maxFreq*1.0f/freqSum;
    entropy = (vx_float32)(-(log(p)*p + (1.f-p)*log(1.f-p))/log(2.0f)*freqSum); /*the cost to separate the max2Freq and the others*/
    entropy += maxFreq*SET_1_LOG; /*1-bit for each the max2Freq*/

    k = 0;
    while((1<<k)<(subSize-SET_1_SIZE))k++;
    entropy += k*(freqSum-maxFreq); /*k-bits fro each less frequency*/
    return (vx_int32) entropy;
}

vx_int32 FindBest2PlusRunSets(vx_int32 *runZeros, vx_int32 size, vx_int32 nonZeros)  /*Find the best running set, and set the */
{
    vx_int32  allBits[8];
    vx_int32 i,j, k;
    vx_int32 tempRun[256], keepLast = 0, allSymbols;
    vx_int32 logSize = 0;
    vx_float32 entropy,p;
    while((1<<logSize) < size)
        logSize++;
    if(runZeros[size - 1]*size >= runZeros[0]){
        keepLast = 1;
        nonZeros += runZeros[size - 1];
    }

    for(k = 0; k<logSize*0+1 ; k++){ /*The best entropy, but needs 256 entry huffman table....*/
        for(i = 0; i<size / (1<<k); i++){
            tempRun[i] = runZeros[i];
        }
        for(; i<size-keepLast; i++){
            tempRun[size / (1<<k) - 1] += ((i+1)/ (size / (1<<k)))*runZeros[i];
            j = (i+1)%(size / (1<<k));
            if(j > 0)
                tempRun[j - 1] += runZeros[i];
        }
        allBits[k] = 0;
        for(i = 0; i<size / (1<<k); i++){
            allBits[k] += tempRun[i];
        }
        /*More bits to distinguish non-zeros*/
        allSymbols = nonZeros + allBits[k];
        p = nonZeros/(vx_float32)allSymbols;
        entropy = (vx_float32)(-(log(p)*p + (1.f-p)*log(1.f-p))/log(2.0f)*allSymbols);
        for(i = 0; i<size / (1<<k); i++){
            p = tempRun[i]/(vx_float32)allBits[k];
            if(p > 0.)
            entropy += (vx_float32)(-(log(p)*p + (1.f-p)*log(1.f-p))/log(2.0f)*allBits[k]);
        }
        allBits[k] = (vx_int32)(entropy);
    }
    for(k = LOG_RUN_SIZE1; k >=1 ; k--){
        allBits[k] = FindBestSubset(runZeros, size, SET_1_SIZE+(1<<k), tempRun);
        allSymbols = nonZeros + freqSum;
        p = nonZeros / (vx_float32)allSymbols;
        entropy = (vx_float32)(-(log(p)*p + (1.f-p)*log(1.f-p))/log(2.0f));
        allBits[k] += (vx_int32)(entropy*allSymbols);
    }
    j = 1;
    allBits[0] = allBits[1];
    for (i = 2; i<=LOG_RUN_SIZE1; i++)
    {
        if (allBits[i] < allBits[0])
        {
            j = i;
            allBits[0] = allBits[i];
        }
    }


    FindBestSubset(runZeros, size, SET_1_SIZE+(1<<j), bestRunsSorted);
    return numBestRuns = SET_1_SIZE +(1<<j);
}

void OutputAt(vx_int32 x, vx_uint32** kernelBufferPtr, vx_uint32 *bitOffset, CodeSymbol* codeSymbol)
{
    vx_int32 pos = 0x0;

#if DEBUG_COMP_ENHANCE_ENC
    FILE * pfileSteps = NULL;
    pfileSteps = fopen("stage_steps_enc.txt", "a");

#endif
    if (x % THROUGHPUT == (THROUGHPUT - 1) )
    { /*The whole cycle is down, need to do something*/
#if DEBUG_COMP_ENHANCE_ENC
        if(pfileSteps != NULL)
        {
            fprintf(pfileSteps, "---x:%d\n", x);
        }
#endif
        pos = (x) % (3*THROUGHPUT);

        if (codeSymbol[pos - 1].bitLength[0] < 32)
        {
            codeSymbol[pos - 1].stageCode[0] &= ((1LL << codeSymbol[pos - 1].bitLength[0]) - 1);
        }
        if (codeSymbol[pos - 0].bitLength[0] < 32)
        {
            codeSymbol[pos - 0].stageCode[0] &= ((1LL << codeSymbol[pos - 0].bitLength[0]) - 1);
        }

        writeBits(kernelBufferPtr, bitOffset, codeSymbol[pos - 1].stageCode[0], codeSymbol[pos - 1].bitLength[0]);
        writeBits(kernelBufferPtr, bitOffset, codeSymbol[pos - 0].stageCode[0], codeSymbol[pos - 0].bitLength[0]);

#if DEBUG_COMP_ENHANCE_ENC
        if(pfileSteps != NULL)
        {
            vx_int32 data;
            vx_int32 length;
            data   = codeSymbol[pos - 1].stageCode[0];
            length = codeSymbol[pos - 1].bitLength[0];
            if (length < 32)
            {
                data &= ((1LL << length) - 1);
            }
            fprintf(pfileSteps, "stage 0, code:%d, length:%d\n", data, length);

            data   = codeSymbol[pos - 0].stageCode[0];
            length = codeSymbol[pos - 0].bitLength[0];
            if (length < 32)
            {
                data &= ((1LL << length) - 1);
            }


            fprintf(pfileSteps, "stage 0, code:%d, length:%d\n", data, length);
        }
#endif

        codeSymbol[pos - 1].bitLength[0] = 0;
        codeSymbol[pos - 0].bitLength[0] = 0;

        if (x >= 3)
        {
            if(pos - 2 < 0)
            {
                pos += 6;
            }
            if (codeSymbol[pos - 3].bitLength[1] < 32)
            {
                codeSymbol[pos - 3].stageCode[1] &= ((1LL << codeSymbol[pos - 3].bitLength[1]) - 1);
            }
            if (codeSymbol[pos - 2].bitLength[1] < 32)
            {
                codeSymbol[pos - 2].stageCode[1] &= ((1LL << codeSymbol[pos - 2].bitLength[1]) - 1);
            }
            writeBits(kernelBufferPtr, bitOffset, codeSymbol[pos - 3].stageCode[1], codeSymbol[pos - 3].bitLength[1]);
            writeBits(kernelBufferPtr, bitOffset, codeSymbol[pos - 2].stageCode[1], codeSymbol[pos - 2].bitLength[1]);

#if DEBUG_COMP_ENHANCE_ENC
            if(pfileSteps != NULL)
            {
                vx_int32 data;
                vx_int32 length;

                data   = codeSymbol[pos - 3].stageCode[1];
                length = codeSymbol[pos - 3].bitLength[1];
                if (length < 32)
                {
                    data &= ((1LL << length) - 1);
                }
                fprintf(pfileSteps, "stage 1, code:%d, length:%d\n", data, codeSymbol[pos - 3].bitLength[1]);

                data   = codeSymbol[pos - 2].stageCode[1];
                length = codeSymbol[pos - 2].bitLength[1];
                if (length < 32)
                {
                    data &= ((1LL << length) - 1);
                }
                fprintf(pfileSteps, "stage 1, code:%d, length:%d\n", data, codeSymbol[pos - 2].bitLength[1]);
            }
#endif

            codeSymbol[pos - 3].bitLength[1] = 0;
            codeSymbol[pos - 2].bitLength[1] = 0;
        }

        if (x >= 5)
        {
            if(pos - 4 < 0)
            {
                pos += 6;
            }
            if (codeSymbol[pos - 5].bitLength[2] < 32)
            {
                codeSymbol[pos - 5].stageCode[2] &= ((1LL << codeSymbol[pos - 5].bitLength[2]) - 1);
            }
            if (codeSymbol[pos - 4].bitLength[2] < 32)
            {
                codeSymbol[pos - 4].stageCode[2] &= ((1LL << codeSymbol[pos - 4].bitLength[2]) - 1);
            }
            writeBits(kernelBufferPtr, bitOffset, codeSymbol[pos - 5].stageCode[2], codeSymbol[pos - 5].bitLength[2]);
            writeBits(kernelBufferPtr, bitOffset, codeSymbol[pos - 4].stageCode[2], codeSymbol[pos - 4].bitLength[2]);

#if DEBUG_COMP_ENHANCE_ENC
            if(pfileSteps != NULL)
            {
                vx_int32 data;
                vx_int32 length;

                data   = codeSymbol[pos - 5].stageCode[2];
                length = codeSymbol[pos - 5].bitLength[2];
                if (length < 32)
                {
                    data &= ((1LL << length) - 1);
                }
                fprintf(pfileSteps, "stage 2, code:%d, length:%d\n", data, codeSymbol[pos - 5].bitLength[2]);
                data   = codeSymbol[pos - 4].stageCode[2];
                length = codeSymbol[pos - 4].bitLength[2];
                if (length < 32)
                {
                    data &= ((1LL << length) - 1);
                }
                fprintf(pfileSteps, "stage 2, code:%d, length:%d\n", data, codeSymbol[pos - 4].bitLength[2]);
            }
#endif

            codeSymbol[pos - 5].bitLength[2] = 0;
            codeSymbol[pos - 4].bitLength[2] = 0;
        }
    }
#if DEBUG_COMP_ENHANCE_ENC
    if(pfileSteps != NULL)
    {
        fclose(pfileSteps);
        pfileSteps = NULL;
    }
#endif
}


/*each core end dummy code for stage*/
vx_uint8 dummy0S0 = 0x0; /* stage0 dummy0, 3'b000 for even*/
vx_uint8 dummy1S0 = 0x4; /* stage0 dummy1, 3'b100 for odd*/
vx_uint8 dummyS1  = 0x0; /* stage1 dummy0 and dummy1, zero*/
void addDummy(vx_int32 x, vx_uint32** kernelBufferPtr, vx_uint32* bitOffset, CodeSymbol* codeSymbol, vx_uint32 dummyCount, vx_uint32* dummyStages, vx_uint32* dummyBits)
{
    vx_uint8 dummyS0 = 0;
    vx_int32 j;
    while(x % THROUGHPUT)
    {/*some stage0 didn't output yet*/
        codeSymbol[x%(3*THROUGHPUT)].stageCode[0] = dummyStages[0]; /*huffCode[0];*/
        codeSymbol[x%(3*THROUGHPUT)].bitLength[0] = 3;
        codeSymbol[x%(3*THROUGHPUT)].stageCode[1] = dummyStages[1]; /*huffCode[1];*/
        codeSymbol[x%(3*THROUGHPUT)].bitLength[1] = dummyBits[1]; /*bitLengtg[1];*/
        codeSymbol[x%(3*THROUGHPUT)].stageCode[2] = dummyStages[2]; /*huffCode[2];*/
        codeSymbol[x%(3*THROUGHPUT)].bitLength[2] = dummyBits[2]; /*bitLengtg[2];*/
        OutputAt(x, kernelBufferPtr, bitOffset, codeSymbol);
        x++;
    }

    dummyS0 = (dummyCount & 0x1)? dummy1S0: dummy0S0;
    for (j = 0; j < 2*THROUGHPUT; j++)
    {
        codeSymbol[x%(3*THROUGHPUT)].stageCode[0] = dummyS0; /*huffCode[0];*/
        codeSymbol[x%(3*THROUGHPUT)].bitLength[0] = 3;
        /* StageCode[1] is dummy0S1 and dummy1S2, they both zero*/
        OutputAt(x, kernelBufferPtr, bitOffset, codeSymbol);
        x++;
    }
}

void reorderWeightBiasBufferForHuffman(
    vx_context context,
    vx_weights_biases_parameter wb,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_int32 z_offset,
    vx_int32 output_size,
    vx_uint32 skip_value,
    vx_uint32 output_final_x,
    vx_uint32 output_final_y,
    vx_uint8_ptr reorder_stream,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr,
    vx_uint8* postMul,
    vx_uint8* postShift,
    vx_uint32* reoder_stream_count_per_core,
    vx_uint32* non_coef_index,
    vx_uint32* zrl_limit_index,
    vx_bool calc_perf
    )
{
    vx_uint32 nnCoreCount = weight_format == VX_TYPE_INT16 ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : weight_format == VX_TYPE_FLOAT16 ?
                      context->nnConfig.fixedFeature.nnCoreCountFloat16 : context->nnConfig.fixedFeature.nnCoreCount;

    vx_uint32 filterTotalCount   = z_count;
    vx_uint32 totalFilterPerCore = filterTotalCount / nnCoreCount;
    vx_uint32 oddFilterPerCore   = filterTotalCount % nnCoreCount;
    vx_uint32 filterCount        = filters_per_core; /* filter count every group */
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;

    vx_uint32 weightCount        = wb->weights_sizes[0] * wb->weights_sizes[1];
    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize((vx_type_e)weight_format);
    vx_uint32 weightBitSize      = weightSize * 8;
    vx_uint32 biasSize           = (vx_uint32)vxDataType_GetSize((vx_type_e)bias_format);
    vx_uint32 biasBitSize        = biasSize * 8;
    vx_uint32 biasData           = 0;

    vx_uint32 filterSliceSize    = weightCount * weightSize;
    vx_uint32 filterSize         = weightCount * weightSize * slice_count;
    vx_uint8* kernelDataPtr      = VX_NULL;
    vx_uint8* kernelBufferInt8Ptr    = reorder_stream;
    vx_uint16* kernelBufferInt16Ptr    = (vx_uint16*)reorder_stream;

    vx_uint32 inputZP = wb->wb_base->inputZP;
    vx_uint32 coefZP = wb->wb_base->coefZP;

    vx_uint32 coreIndex;
    vx_uint32 elementIndex = 0;
    vx_uint32 nonCoefIndex = 0;
    vx_uint32 limitZRLIndex = 0;
    vx_uint32 idx = 0;
    vx_uint32 filterIndex, sliceIndex;
    vx_uint32 weightXIndex, weightYIndex;
    vx_uint32 skipValue = skip_value;

    vx_bool hasZDP3 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3);
    vx_bool hasZDP6 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6);
    vx_bool hasXYDP9 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP9);
    vx_bool hasXYDP6 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP6);
    vx_bool hasBiasAtEnd = vx_false_e;
    vx_bool hasNNPerFilterPostMultiply = vx_false_e;
    vx_bool hasNNPerFilterPostShift = vx_false_e;

    vx_float64* nonZeroRatioPerCorePerVZG = VX_NULL;
    vx_uint32 groupIndex;

    typedef struct _gcVXWeightsMinusZP
    {
        vx_int32 sum;
        vx_uint32 filterIdx;
    }gcVXWeightsMinusZP;

    gcVXWeightsMinusZP *weightsMinusZP = VX_NULL;

    if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
    {
        if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
        {
            biasBitSize = NN_INTEGER_BIAS_BITS_VIP_V7;
        }
        else
        {
            biasBitSize = NN_INTEGER_BIAS_BITS;
        }
    }
    else if (weight_format == VX_TYPE_INT16)
    {
        biasBitSize = NN_INTEGER_BIAS_BITS_VIP_V7_INT16;
    }

    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT) && inputZP != 0 && weight_format == VX_TYPE_UINT8)
    {
        weightsMinusZP = (gcVXWeightsMinusZP*)vxAllocateAndZeroMemory(filterTotalCount * sizeof(gcVXWeightsMinusZP));

        if (!weightsMinusZP)
        {
            vxError("fillinKernelBuffer: OUT OF MEMORY");
            goto exit;
        }

        /*Calc sum((coef[i] - coefZP) * InZP) */
        for (filterIndex = 0; filterIndex < filterTotalCount; filterIndex++)
        {
            weightsMinusZP[filterIndex].filterIdx = filterIndex;
            weightsMinusZP[filterIndex].sum = 0;
            for (sliceIndex = 0; sliceIndex < slice_count; sliceIndex++)
            {
                kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * sliceIndex;

                for (weightYIndex = 0; weightYIndex < wb->weights_sizes[1]; weightYIndex++)
                {
                    for (weightXIndex = 0; weightXIndex < wb->weights_sizes[0]; weightXIndex++)
                    {
                        vx_int32 weight = 0;
                        weight = *((vx_uint8 *)kernelDataPtr);
                        kernelDataPtr = kernelDataPtr + weightSize;

                        weightsMinusZP[filterIndex].sum += (weight - coefZP) * inputZP;
                    }
                }
            }
        }
    }

    if (calc_perf)
    {
        if (wb->max_per_core_per_vzgroup_nzr == VX_NULL)
        {
            wb->max_per_core_per_vzgroup_nzr = (vx_float64*)vxAllocateAndZeroMemory(groupCount * sizeof(vx_float64));
            if (wb->max_per_core_per_vzgroup_nzr == VX_NULL)
            {
                vxError("reorderWeightBiasBufferForHuffman: OUT OF MEMORY\n");
                goto exit;
            }
        }

        nonZeroRatioPerCorePerVZG = (vx_float64*)vxAllocateAndZeroMemory(nnCoreCount * groupCount * sizeof(vx_float64));
        if (nonZeroRatioPerCorePerVZG == VX_NULL)
        {
            vxError("reorderWeightBiasBufferForHuffman: OUT OF MEMORY");
            goto exit;
        }
    }
    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        vx_uint32 coreFilterCount = (coreIndex < oddFilterPerCore) ? totalFilterPerCore + 1 : totalFilterPerCore;

        reoder_stream_count_per_core[coreIndex] = 0;


        if (coreFilterCount > 0)
        {
            /* Save kernels per core*/
            if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
            {
                *((vx_uint16 *)kernelBufferInt8Ptr) = (vx_uint16)coreFilterCount;
                kernelBufferInt8Ptr += 2;
                reoder_stream_count_per_core[coreIndex] += (16 / weightBitSize);
            }
            else
            {
                *kernelBufferInt16Ptr = (vx_uint16)coreFilterCount;
                kernelBufferInt16Ptr ++;
                reoder_stream_count_per_core[coreIndex] += (16 / weightBitSize);
            }
            for (idx = 0; idx < (16 / weightBitSize); idx++)
            {
                non_coef_index[nonCoefIndex++] = elementIndex++;
            }
        }

        for (groupIndex = 0; groupIndex < groupCount; groupIndex++)
        {
            vx_uint32 actualFilterCount = (groupIndex == groupCount - 1) ? (coreFilterCount - groupIndex * filterCount) : filterCount;
            vx_uint32 filterStart, filterEnd;
            vx_uint32 blockCount = 0;
            vx_uint32 nonZeroCount = 0;
            vx_uint32 nonZrlIdx = groupIndex + coreIndex * groupCount;

            if ((groupIndex == groupCount - 1)
                && (coreIndex >= (filterTotalCount % nnCoreCount)))
            {
                filterStart = groupIndex * nnCoreCount * filterCount
                            + (filterTotalCount % nnCoreCount) * (actualFilterCount + 1)
                            + (coreIndex - (filterTotalCount % nnCoreCount)) * actualFilterCount;
            }
            else
            {
                filterStart = groupIndex * nnCoreCount *filterCount + coreIndex * actualFilterCount;
            }
            filterEnd = filterStart + actualFilterCount - 1;

            if (wb->weights_sizes[0] == 1 && wb->weights_sizes[1] == 1 &&
                (hasZDP3 || hasZDP6) &&
                (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8))
            {
                vx_uint32 zdpNum = hasZDP6 ? 6 : 3;

                /* zdp3 zdp6 can not enable same time */
                vxmASSERT(!(hasZDP3 && hasZDP6));

                for (sliceIndex = 0; sliceIndex < slice_count; sliceIndex += (zdpNum * 2))
                {
                    vx_uint32 weight = 0;
                    vx_uint32 realSliceIndex = 0;

                    /* add slices of every filter*/
                    for (filterIndex = filterStart; filterIndex <= filterEnd; filterIndex++)
                    {
                        vx_uint32 nonZeroInBlock1Count = 0; /*Check if there's non-zero point in first zdpNum block*/
                        vx_uint32 nonZeroInBlock2Count = 0; /*Check if there's non-zero point in another zdpNum block*/
                        for (realSliceIndex = sliceIndex; realSliceIndex < sliceIndex + (zdpNum * 2); realSliceIndex++)
                        {
                            if (realSliceIndex >= slice_count)
                                break;

                            if (realSliceIndex == 0)
                            {
                                if (bias_base_ptr)
                                    biasData = *(bias_base_ptr + filterIndex);
                                else
                                    biasData = 0;

                                /* NewBias = sum((Coef[i] - CoefZP) * - InZP) + Bias*/
                                if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT)
                                    && inputZP != 0
                                    && weight_format == VX_TYPE_UINT8)
                                {
                                    biasData -= weightsMinusZP[filterIndex].sum;
                                }

                                if (!hasBiasAtEnd)
                                {
                                    *((vx_uint32 *)kernelBufferInt8Ptr) = biasData;
                                    kernelBufferInt8Ptr += 4;
                                    reoder_stream_count_per_core[coreIndex] += (biasBitSize / weightBitSize);
                                    for (idx = 0; idx < (biasBitSize / weightBitSize); idx++)
                                    {
                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                    }
                                }
                            }

                            /* add one slice data every filter */
                            kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * realSliceIndex;
                            if (weight_format == VX_TYPE_INT8)
                                weight = *((vx_int8 *)kernelDataPtr);
                            else if (weight_format == VX_TYPE_UINT8)
                                weight = *((vx_uint8 *)kernelDataPtr);
                            else
                                weight = *((vx_uint16 *)kernelDataPtr);

                            if (realSliceIndex - sliceIndex < zdpNum && weight != skipValue)
                                nonZeroInBlock1Count++;
                            else if (weight != skipValue)
                                nonZeroInBlock2Count++;

                            *kernelBufferInt8Ptr = (vx_uint8)weight;
                            kernelBufferInt8Ptr++;
                            reoder_stream_count_per_core[coreIndex] += 1;

                            if (filterIndex == filterEnd && realSliceIndex == 0)
                                zrl_limit_index[limitZRLIndex++] = elementIndex;

                            elementIndex++;

                            /* add offet behind the last point of the last slice of each filter */
                            if (realSliceIndex == slice_count - 1)
                            {
                                vx_uint32 offsetValue;

                                if (hasNNPerFilterPostMultiply)
                                {
                                    /*add post multiply to each filter*/

                                    vx_uint32 postMulSize = 8;
                                    *kernelBufferInt8Ptr = postMul[filterIndex];
                                    kernelBufferInt8Ptr++;
                                    reoder_stream_count_per_core[coreIndex] += (postMulSize / weightBitSize);
                                    for (idx = 0; idx < (postMulSize / weightBitSize); idx++)
                                    {
                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                    }
                                }
                                if (hasNNPerFilterPostShift)
                                {
                                    /*add post multiply to each filter*/

                                    vx_uint32 postShiftSize = 8;
                                    *kernelBufferInt8Ptr = postShift[filterIndex];
                                    kernelBufferInt8Ptr++;
                                    reoder_stream_count_per_core[coreIndex] += (postShiftSize / weightBitSize);
                                    for (idx = 0; idx < (postShiftSize / weightBitSize); idx++)
                                    {
                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                    }
                                }

                                if (z_offset > 0)
                                    offsetValue = (vx_uint32)z_offset * filterIndex;
                                else if (output_size > 0)
                                    offsetValue = output_final_x * output_final_y * output_size * filterIndex;
                                else
                                    offsetValue = output_final_x * output_final_y * weightSize * filterIndex;

                                if (hasBiasAtEnd)
                                {
                                    *((vx_uint32 *)kernelBufferInt8Ptr) = biasData;
                                    kernelBufferInt8Ptr += 4;
                                    reoder_stream_count_per_core[coreIndex] += (biasBitSize / weightBitSize);

                                    for (idx = 0; idx < (biasBitSize / weightBitSize); idx++)
                                    {
                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                    }
                                }

                                if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                {
                                    *((vx_uint32 *)kernelBufferInt8Ptr) = offsetValue;
                                    kernelBufferInt8Ptr += 4;
                                    reoder_stream_count_per_core[coreIndex] += (NN_Z_POSITION_OFFSET_BITS_VIP_V7 / weightBitSize);
                                    for (idx = 0; idx < (NN_Z_POSITION_OFFSET_BITS_VIP_V7 / weightBitSize); idx++)
                                    {
                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                    }
                                }
                            }
                        }
                        if (sliceIndex + zdpNum >= slice_count)
                            blockCount++;
                        else
                            blockCount += 2;

                        if (nonZeroInBlock1Count)
                            nonZeroCount++;
                        if (nonZeroInBlock2Count)
                            nonZeroCount++;
                    }
                }
            }
            /*Only INT8/UINT8 support XY DP9 now*/
            else if ((hasXYDP9 || hasXYDP6)
                && (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8))
            {
                vx_uint32 xStep = 0, yStep = 0;
                vx_uint32 inImageBufferSize = hasXYDP9 ? 2 : 1;

                /* xydp9 xydp6 can not enable same time */
                vxmASSERT(!(hasXYDP9 && hasXYDP6));

                if (hasXYDP6)
                {
                    xStep = 3;
                    yStep = 2;
                }
                else
                {
                    xStep = 3;
                    yStep = 3;
                }

                for (sliceIndex = 0; sliceIndex < slice_count; sliceIndex += inImageBufferSize)
                {
                    vx_uint32 weight = 0;
                    vx_uint32 realSliceIndex = 0;
                    vx_uint32 realWeightXIndex = 0, realWeightYIndex = 0;
                    vx_uint32 subKxSize = 0, subKySize = 0, subKzSize = 0;

                    subKzSize = gcmMIN((slice_count - sliceIndex), inImageBufferSize);
                    /* add slices of every filter*/
                    for (filterIndex = filterStart; filterIndex <= filterEnd; filterIndex++)
                    {
                        for (weightYIndex = 0; weightYIndex < wb->weights_sizes[1]; weightYIndex += yStep)
                        {
                            subKySize = gcmMIN((wb->weights_sizes[1] - weightYIndex), yStep);
                            for (weightXIndex = 0; weightXIndex < wb->weights_sizes[0]; weightXIndex += xStep)
                            {
                                subKxSize = gcmMIN((wb->weights_sizes[0] - weightXIndex), xStep);
                                for (realSliceIndex = sliceIndex; realSliceIndex < sliceIndex + subKzSize; realSliceIndex++)
                                {
                                    vx_uint32 nonZeroInBlockCount = 0; /*Check if there's non-zero point in one 3x3 block*/
                                    /* add one slice data every filter */
                                    kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * realSliceIndex;
                                    if (realSliceIndex == 0)
                                    {
                                        if (bias_base_ptr)
                                            biasData = *(bias_base_ptr + filterIndex);
                                        else
                                            biasData = 0;

                                        /* NewBias = sum((Coef[i] - CoefZP) * - InZP) + Bias*/
                                        if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT)
                                            && inputZP != 0
                                            && weight_format == VX_TYPE_UINT8)
                                        {
                                            biasData -= weightsMinusZP[filterIndex].sum;
                                        }
                                    }

                                    for (realWeightYIndex = weightYIndex; realWeightYIndex < weightYIndex + subKySize; realWeightYIndex++)
                                    {
                                        for (realWeightXIndex = weightXIndex; realWeightXIndex < weightXIndex + subKxSize; realWeightXIndex++)
                                        {
                                            vx_uint8_ptr realKernelDataPtr = kernelDataPtr + (realWeightYIndex * wb->weights_sizes[0] + realWeightXIndex) * weightSize;

                                            if (weight_format == VX_TYPE_INT8)
                                                weight = *((vx_int8 *)realKernelDataPtr);
                                            else if (weight_format == VX_TYPE_UINT8)
                                                weight = *((vx_uint8 *)realKernelDataPtr);
                                            else
                                                weight = *((vx_uint16 *)realKernelDataPtr);

                                            if (weight != skipValue)
                                                nonZeroInBlockCount++;

                                            if (!hasBiasAtEnd &&
                                                realWeightYIndex == 0 &&
                                                realWeightXIndex == 0 &&
                                                realSliceIndex == 0)
                                            {
                                                *((vx_uint32 *)kernelBufferInt8Ptr) = biasData;
                                                kernelBufferInt8Ptr += 4;
                                                reoder_stream_count_per_core[coreIndex] += (biasBitSize / weightBitSize);
                                                for (idx = 0; idx < (biasBitSize / weightBitSize); idx++)
                                                {
                                                    non_coef_index[nonCoefIndex++] = elementIndex++;
                                                }
                                            }
                                            *kernelBufferInt8Ptr = (vx_uint8)weight;
                                            kernelBufferInt8Ptr++;
                                            reoder_stream_count_per_core[coreIndex] += 1;

                                            if (filterIndex == filterEnd && realSliceIndex == 0 &&
                                                realWeightXIndex == 0 && realWeightYIndex == 0)
                                                zrl_limit_index[limitZRLIndex++] = elementIndex;

                                            elementIndex++;

                                            /* add offet behind the last point of the last slice of each filter */
                                            if (realSliceIndex == slice_count - 1 &&
                                                realWeightXIndex == wb->weights_sizes[0] - 1 &&
                                                realWeightYIndex == wb->weights_sizes[1] - 1)
                                            {
                                                vx_uint32 offsetValue;

                                                if (hasNNPerFilterPostMultiply)
                                                {
                                                    /*add post multiply to each filter*/

                                                    vx_uint32 postMulSize = 8;
                                                    *kernelBufferInt8Ptr = postMul[filterIndex];
                                                    kernelBufferInt8Ptr++;
                                                    reoder_stream_count_per_core[coreIndex] += (postMulSize / weightBitSize);
                                                    for (idx = 0; idx < (postMulSize / weightBitSize); idx++)
                                                    {
                                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                                    }
                                                }
                                                if (hasNNPerFilterPostShift)
                                                {
                                                    /*add post multiply to each filter*/

                                                    vx_uint32 postShiftSize = 8;
                                                    *kernelBufferInt8Ptr = postShift[filterIndex];
                                                    kernelBufferInt8Ptr++;
                                                    reoder_stream_count_per_core[coreIndex] += (postShiftSize / weightBitSize);
                                                    for (idx = 0; idx < (postShiftSize / weightBitSize); idx++)
                                                    {
                                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                                    }
                                                }

                                                if (z_offset > 0)
                                                    offsetValue = (vx_uint32)z_offset * filterIndex;
                                                else if (output_size > 0)
                                                    offsetValue = output_final_x * output_final_y * output_size * filterIndex;
                                                else
                                                    offsetValue = output_final_x * output_final_y * weightSize * filterIndex;

                                                if (hasBiasAtEnd)
                                                {
                                                    *((vx_uint32 *)kernelBufferInt8Ptr) = biasData;
                                                    kernelBufferInt8Ptr += 4;
                                                    reoder_stream_count_per_core[coreIndex] += (biasBitSize / weightBitSize);
                                                    for (idx = 0; idx < (biasBitSize / weightBitSize); idx++)
                                                    {
                                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                                    }
                                                }

                                                if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                                {
                                                    *((vx_uint32 *)kernelBufferInt8Ptr) = offsetValue;
                                                    kernelBufferInt8Ptr += 4;
                                                    reoder_stream_count_per_core[coreIndex] += (NN_Z_POSITION_OFFSET_BITS_VIP_V7 / weightBitSize);
                                                    for (idx = 0; idx < (NN_Z_POSITION_OFFSET_BITS_VIP_V7 / weightBitSize); idx++)
                                                    {
                                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                                    }
                                                }

                                            }
                                        }
                                    }
                                    blockCount++;
                                    if (nonZeroInBlockCount)
                                        nonZeroCount++;
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                for (sliceIndex = 0; sliceIndex < slice_count; sliceIndex++)
                {
                    /* add slices of every filter*/
                    for (filterIndex = filterStart; filterIndex <= filterEnd; filterIndex++)
                    {

                        /* add one slice data every filter */
                        kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * sliceIndex;
                        if (sliceIndex == 0)
                        {
                            if (bias_base_ptr)
                                biasData = *(bias_base_ptr + filterIndex);
                            else
                                biasData = 0;

                            /* NewBias = sum((Coef[i] - CoefZP) * - InZP) + Bias*/
                            if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT)
                                && inputZP != 0
                                && weight_format == VX_TYPE_UINT8)
                            {
                                biasData -= weightsMinusZP[filterIndex].sum;
                            }
                        }

                        for (weightYIndex = 0; weightYIndex < wb->weights_sizes[1]; weightYIndex++)
                        {
                            vx_uint32 nonZeroCountInDP3 = 0; /*Check if there's non-zero point in one 3x1 block*/
                            for (weightXIndex = 0; weightXIndex < wb->weights_sizes[0]; weightXIndex++)
                            {
                                vx_uint32 weight = 0;

                                if (weight_format == VX_TYPE_INT8)
                                    weight = *((vx_int8 *)kernelDataPtr);
                                else if (weight_format == VX_TYPE_UINT8)
                                    weight = *((vx_uint8 *)kernelDataPtr);
                                else
                                    weight = *((vx_uint16 *)kernelDataPtr);
                                kernelDataPtr = kernelDataPtr + weightSize;

                                {
                                    if (weight != skipValue)
                                        nonZeroCountInDP3++;

                                    /*V7, DP3, one block is 3x1*/
                                    if ((weightXIndex + 1) % 3 == 0 || weightXIndex == wb->weights_sizes[0] - 1)
                                    {
                                        blockCount++;
                                        if (nonZeroCountInDP3)
                                            nonZeroCount++;
                                        nonZeroCountInDP3 = 0;
                                    }
                                }

                                if (!hasBiasAtEnd &&
                                    weightXIndex == 0 &&
                                    weightYIndex == 0 &&
                                    sliceIndex == 0)
                                {
                                    if (weight_format == VX_TYPE_INT16)
                                    {
                                        vx_int64 bias64 = 0;
                                        vx_int32 bias32;
                                        vx_int16 bias16;

                                        if ((vx_int32)biasData < 0)
                                        {
                                            bias64 = ~0;
                                            bias64 = (bias64 >> 32) << 32;
                                            bias64 = bias64 | (vx_int64)biasData;
                                        }
                                        else
                                            bias64 = (vx_int64)biasData;

                                        bias32 = (vx_int32)bias64;
                                        *((vx_uint32 *)kernelBufferInt16Ptr) = bias32;
                                        kernelBufferInt16Ptr += 2;
                                        reoder_stream_count_per_core[coreIndex] += (32 / weightBitSize);
                                        bias16 = (vx_int16)(bias64 >> 32);
                                        *kernelBufferInt16Ptr = bias16;
                                        kernelBufferInt16Ptr++;
                                        reoder_stream_count_per_core[coreIndex] += (16 / weightBitSize);
                                    }
                                    else if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
                                    {
                                        *((vx_uint32 *)kernelBufferInt8Ptr) = biasData;
                                        kernelBufferInt8Ptr += 4;
                                        reoder_stream_count_per_core[coreIndex] += (biasBitSize / weightBitSize);
                                    }
                                    else
                                    {
                                        *((vx_uint32 *)kernelBufferInt16Ptr) = biasData;
                                        kernelBufferInt16Ptr += 2;
                                        reoder_stream_count_per_core[coreIndex] += (biasBitSize / weightBitSize);
                                    }

                                    for (idx = 0; idx < (biasBitSize / weightBitSize); idx++)
                                    {
                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                    }
                                }

                                if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
                                {
                                    *kernelBufferInt8Ptr = (vx_uint8)weight;
                                    kernelBufferInt8Ptr++;
                                }
                                else
                                {
                                    if (weight_format == VX_TYPE_FLOAT16)
                                        weight = (weight & 0x7fff) * 2 + weight/(1<<15);

                                    *kernelBufferInt16Ptr = (vx_uint16)weight;
                                    kernelBufferInt16Ptr++;
                                }
                                reoder_stream_count_per_core[coreIndex] += 1;

                                if (filterIndex == filterEnd && sliceIndex == 0 &&
                                    weightXIndex == 0 && weightYIndex == 0)
                                    zrl_limit_index[limitZRLIndex++] = elementIndex;

                                elementIndex++;
                            }
                        }

                        /* add offet behind the last point of the last slice of each filter */
                        if (sliceIndex == slice_count - 1)
                        {
                            vx_uint32 offsetValue;

                            if (hasNNPerFilterPostMultiply && hasNNPerFilterPostShift)
                            {
                                if (weight_format == VX_TYPE_INT16 || weight_format == VX_TYPE_FLOAT16)
                                {
                                    vx_uint32 postMulSize = 16;
                                    vx_uint32 postShiftSize = 16;
                                    /*add post multiply to each filter*/
                                    *kernelBufferInt16Ptr = (vx_uint16)postMul[filterIndex];
                                    kernelBufferInt16Ptr++;
                                    reoder_stream_count_per_core[coreIndex] += (postMulSize / weightBitSize);
                                    /*add post multiply to each filter*/
                                    *kernelBufferInt16Ptr = (vx_uint16)postShift[filterIndex];
                                    kernelBufferInt16Ptr++;
                                    reoder_stream_count_per_core[coreIndex] += (postShiftSize / weightBitSize);
                                }
                                else
                                {
                                    vx_uint32 postMulSize = 8;
                                    vx_uint32 postShiftSize = 8;
                                    /*add post multiply to each filter*/
                                    *kernelBufferInt8Ptr = postMul[filterIndex];
                                    kernelBufferInt8Ptr++;
                                    reoder_stream_count_per_core[coreIndex] += (postMulSize / weightBitSize);
                                    /*add post multiply to each filter*/
                                    *kernelBufferInt8Ptr = postShift[filterIndex];
                                    kernelBufferInt8Ptr++;
                                    reoder_stream_count_per_core[coreIndex] += (postShiftSize / weightBitSize);
                                }

                                for (idx = 0; idx < 2; idx++)
                                {
                                    non_coef_index[nonCoefIndex++] = elementIndex++;
                                }
                            }

                            if (z_offset > 0)
                                offsetValue = (vx_uint32)z_offset * filterIndex;
                            else if (output_size > 0)
                                offsetValue = output_final_x * output_final_y * output_size * filterIndex;
                            else
                                offsetValue = output_final_x * output_final_y * weightSize * filterIndex;

                            if (hasBiasAtEnd)
                            {
                                if (weight_format == VX_TYPE_INT16)
                                {
                                    vx_int64 bias64 = 0;
                                    vx_int32 bias32;
                                    vx_int16 bias16;

                                    if ((vx_int32)biasData < 0)
                                    {
                                        bias64 = ~0;
                                        bias64 = (bias64 >> 32) << 32;
                                        bias64 = bias64 | (vx_int64)biasData;
                                    }
                                    else
                                        bias64 = (vx_int64)biasData;

                                    bias32 = (vx_int32)bias64;
                                    *((vx_uint32 *)kernelBufferInt16Ptr) = bias32;
                                    kernelBufferInt16Ptr += 2;
                                    reoder_stream_count_per_core[coreIndex] += (32 / weightBitSize);
                                    bias16 = (vx_int16)(bias64 >> 32);
                                    *kernelBufferInt16Ptr = bias16;
                                    kernelBufferInt16Ptr++;
                                    reoder_stream_count_per_core[coreIndex] += (16 / weightBitSize);
                                }
                                else if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
                                {
                                    *((vx_uint32 *)kernelBufferInt8Ptr) = biasData;
                                    kernelBufferInt8Ptr += 4;
                                    reoder_stream_count_per_core[coreIndex] += (biasBitSize / weightBitSize);
                                }
                                else
                                {
                                    *((vx_uint32 *)kernelBufferInt16Ptr) = biasData;
                                    kernelBufferInt16Ptr += 2;
                                    reoder_stream_count_per_core[coreIndex] += (biasBitSize / weightBitSize);
                                }

                                for (idx = 0; idx < (biasBitSize / weightBitSize); idx++)
                                {
                                    non_coef_index[nonCoefIndex++] = elementIndex++;
                                }
                            }

                            if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                            {
                                if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
                                {
                                    *((vx_uint32 *)kernelBufferInt8Ptr) = offsetValue;
                                    kernelBufferInt8Ptr += 4;
                                }
                                else
                                {
                                    *((vx_uint32 *)kernelBufferInt16Ptr) = offsetValue;
                                    kernelBufferInt16Ptr += 2;
                                }
                                reoder_stream_count_per_core[coreIndex] += (NN_Z_POSITION_OFFSET_BITS_VIP_V7 / weightBitSize);
                                for (idx = 0; idx < (NN_Z_POSITION_OFFSET_BITS_VIP_V7 / weightBitSize); idx++)
                                {
                                    non_coef_index[nonCoefIndex++] = elementIndex++;
                                }
                            }
                        }
                    }
                }
            }
            if (calc_perf)
                nonZeroRatioPerCorePerVZG[nonZrlIdx] = (vx_float64)nonZeroCount / (vx_float64)blockCount;
        }
    }

    if (calc_perf)
    {
        for (groupIndex = 0; groupIndex < groupCount; groupIndex++)
        {
            for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
            {
                vx_uint32 nonZrlIdx = groupIndex + coreIndex * groupCount;
                if (wb->max_per_core_per_vzgroup_nzr[groupIndex] < nonZeroRatioPerCorePerVZG[nonZrlIdx])
                    wb->max_per_core_per_vzgroup_nzr[groupIndex] = nonZeroRatioPerCorePerVZG[nonZrlIdx];
            }
        }
    }

#if DEBUG_COMP_ENHANCE_ENC
    {
        unsigned char * pChar = (unsigned char*)reorder_stream;
        unsigned char data;
        vx_uint32 i;
        vx_uint32 totalByteCount = weightCount * slice_count * z_count * weightSize + z_count * 4 + z_count * (biasBitSize / 8) + nnCoreCount * 2;
        FILE * pfile1 = NULL;
        pfile1 = fopen("reordered_kernel.txt", "wb");

        if(pfile1 != NULL)
        {
            for(i=0; i!=totalByteCount; i++)
            {
                data = *pChar;
                fprintf(pfile1, "coef: 0x%x\n", data);
                pChar++;
            }

            fclose(pfile1);
        }

    }
#endif

exit:
    if (weightsMinusZP)
        vxFree(weightsMinusZP);
    if (nonZeroRatioPerCorePerVZG)
        vxFree(nonZeroRatioPerCorePerVZG);
    return;
}

#define PRINT_PER_CORE_SIZE 0
void reorderKernelBufferV7HuffmanBalance(
    vx_context context,
    vx_weights_biases_parameter wb,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_int32 z_offset,
    vx_int32 output_size,
    vx_uint32 skip_value,
    vx_uint32 output_final_x,
    vx_uint32 output_final_y,
    vx_uint8_ptr reorder_stream,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr,
    vx_uint8* postMul,
    vx_uint8* postShift,
    vx_uint32* reoder_stream_count_per_core,
    vx_uint32* non_coef_index,
    vx_uint32* zrl_limit_index,
    vx_bool calc_perf
    )
{
    vx_uint32 nnCoreCount = weight_format == VX_TYPE_INT16 ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : weight_format == VX_TYPE_FLOAT16 ?
                      context->nnConfig.fixedFeature.nnCoreCountFloat16 : context->nnConfig.fixedFeature.nnCoreCount;

    vx_uint32 filterTotalCount   = z_count;
    vx_uint32 sliceCount         = slice_count;
    vx_uint32 totalFilterPerCore = filterTotalCount / nnCoreCount;
    vx_uint32 oddFilterPerCore   = filterTotalCount % nnCoreCount;
    vx_uint32 filterCount        = filters_per_core; /* filter count every group */
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;

    vx_uint32 weightCount        = wb->weights_sizes[0] * wb->weights_sizes[1];
    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize((vx_type_e)weight_format);
    vx_uint32 weightBitSize      = weightSize * 8;
    vx_uint32 biasSize           = (vx_uint32)vxDataType_GetSize((vx_type_e)bias_format);
    vx_uint32 biasBitSize        = biasSize * 8;
    vx_uint32 biasData           = 0;

    vx_uint32 filterSliceSize    = weightCount * weightSize;
    vx_uint32 filterSize         = weightCount * weightSize * slice_count;
    vx_uint8* kernelDataPtr      = VX_NULL;
    vx_uint8* kernelBufferInt8Ptr    = reorder_stream;
    vx_uint16* kernelBufferInt16Ptr    = (vx_uint16*)reorder_stream;

    vx_uint32 inputZP = wb->wb_base->inputZP;
    vx_uint32 coefZP = wb->wb_base->coefZP;

    vx_uint32 coreIndex;
    vx_uint32 elementIndex = 0;
    vx_uint32 nonCoefIndex = 0;
    vx_uint32 limitZRLIndex = 0;
    vx_uint32 idx = 0;
    vx_uint32 filterIndex, sliceIndex;
    vx_uint32 weightXIndex, weightYIndex;
    vx_uint32 skipValue = skip_value;

    vx_bool hasZDP3 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3);
    vx_bool hasZDP6 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6);
    vx_bool hasXYDP9 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP9);
    vx_bool hasXYDP6 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP6);
    vx_bool hasBiasAtEnd = vx_false_e;
    vx_bool hasNNPerFilterPostMultiply = vx_false_e;
    vx_bool hasNNPerFilterPostShift = vx_false_e;

    vx_uint32 totalNonZeroCount = 0;
    vx_float32 averageNZCPerFilter = 0;
    vx_float32 varianceFL32 = 0;
    vx_uint32 variance = 0;
    vx_bool reorder = vx_false_e;

    vx_float64* nonZeroRatioPerCorePerVZG = VX_NULL;
    vx_uint32 groupIndex;

    typedef struct _gcVXWeightsMinusZP
    {
        vx_int32 sum;
        vx_uint32 filterIdx;
    }gcVXWeightsMinusZP;

    gcVXWeightsMinusZP *weightsMinusZP = VX_NULL;

    typedef struct _gcVXNonZeroWeights
    {
        vx_uint32 nonZeroCount;
        vx_uint32 filterIdx;
    }gcVXNonZeroWeights;

#if PRINT_PER_CORE_SIZE
    vx_uint32* nonZeroCoefPerCore = VX_NULL;
#endif

    gcVXNonZeroWeights *nonZeroWeights = VX_NULL;
    vx_int32 i = 0;
    vx_uint32 core0FilterCount = 0;
    gcVXNonZeroWeights tmp;

    nonZeroWeights = (gcVXNonZeroWeights*)vxAllocateAndZeroMemory(filterTotalCount * sizeof(gcVXNonZeroWeights));
    if (nonZeroWeights == VX_NULL)
    {
        vxError("fillinKernelBuffer: OUT OF MEMORY");
        goto exit;
    }
#if PRINT_PER_CORE_SIZE
    nonZeroCoefPerCore = (vx_uint32*)vxAllocateAndZeroMemory(nnCoreCount * sizeof(vx_uint32));
    if (nonZeroCoefPerCore == VX_NULL)
    {
        vxError("fillinKernelBuffer: OUT OF MEMORY");
        goto exit;
    }
#endif

    /* calc each filter non-zero weights num*/
    for (filterIndex = 0; filterIndex < filterTotalCount; filterIndex++)
    {
        nonZeroWeights[filterIndex].filterIdx = filterIndex;
        for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
        {
            /* add one slice data every filter */
            kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * sliceIndex;

            for (weightYIndex = 0; weightYIndex < wb->weights_sizes[1]; weightYIndex++)
            {
                for (weightXIndex = 0; weightXIndex < wb->weights_sizes[0]; weightXIndex++)
                {
                    vx_uint32 weight = 0;

                    if (weight_format == VX_TYPE_INT8)
                        weight = *((vx_int8 *)kernelDataPtr);
                    else if (weight_format == VX_TYPE_UINT8)
                        weight = *((vx_uint8 *)kernelDataPtr);
                    else
                        weight = *((vx_uint16 *)kernelDataPtr);
                    kernelDataPtr = kernelDataPtr + weightSize;

                    if (weight != skipValue)
                        nonZeroWeights[filterIndex].nonZeroCount++;

                }
            }
        }
        totalNonZeroCount += nonZeroWeights[filterIndex].nonZeroCount;
    }

    /*Sort*/
    averageNZCPerFilter = (vx_float32)totalNonZeroCount / filterTotalCount;
    for (filterIndex = 0; filterIndex < filterTotalCount; filterIndex++)
    {
        varianceFL32 += gcoMATH_Power((nonZeroWeights[filterIndex].nonZeroCount - averageNZCPerFilter), 2);
    }
    variance = (vx_uint32)(varianceFL32 / filterTotalCount + 0.5f);

    reorder = vx_true_e;

    if (reorder)
    {
        /*Sort*/
        for (filterIndex = 0; filterIndex < filterTotalCount; filterIndex++)
        {
            tmp.nonZeroCount = nonZeroWeights[filterIndex].nonZeroCount;
            tmp.filterIdx = filterIndex;

            for (i = filterIndex - 1; i >= 0 && nonZeroWeights[i].nonZeroCount > tmp.nonZeroCount; i--)
            {
                nonZeroWeights[i+1].nonZeroCount = nonZeroWeights[i].nonZeroCount;
                nonZeroWeights[i+1].filterIdx = nonZeroWeights[i].filterIdx;
            }
            nonZeroWeights[i+1].nonZeroCount = tmp.nonZeroCount;
            nonZeroWeights[i+1].filterIdx = tmp.filterIdx;
        }
    }

    if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
    {
        if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
        {
            biasBitSize = NN_INTEGER_BIAS_BITS_VIP_V7;
        }
        else
        {
            biasBitSize = NN_INTEGER_BIAS_BITS;
        }
    }
    else if (weight_format == VX_TYPE_INT16)
    {
        biasBitSize = NN_INTEGER_BIAS_BITS_VIP_V7_INT16;
    }

    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT) && inputZP != 0 && weight_format == VX_TYPE_UINT8)
    {
        weightsMinusZP = (gcVXWeightsMinusZP*)vxAllocateAndZeroMemory(filterTotalCount * sizeof(gcVXWeightsMinusZP));

        if (!weightsMinusZP)
        {
            vxError("fillinKernelBuffer: OUT OF MEMORY");
            goto exit;
        }

        /*Calc sum((coef[i] - coefZP) * InZP) */
        for (filterIndex = 0; filterIndex < filterTotalCount; filterIndex++)
        {
            weightsMinusZP[filterIndex].filterIdx = filterIndex;
            weightsMinusZP[filterIndex].sum = 0;
            for (sliceIndex = 0; sliceIndex < slice_count; sliceIndex++)
            {
                kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * sliceIndex;

                for (weightYIndex = 0; weightYIndex < wb->weights_sizes[1]; weightYIndex++)
                {
                    for (weightXIndex = 0; weightXIndex < wb->weights_sizes[0]; weightXIndex++)
                    {
                        vx_int32 weight = 0;
                        weight = *((vx_uint8 *)kernelDataPtr);
                        kernelDataPtr = kernelDataPtr + weightSize;

                        weightsMinusZP[filterIndex].sum += (weight - coefZP) * inputZP;
                    }
                }
            }
        }
    }

    if (calc_perf)
    {
        if (wb->max_per_core_per_vzgroup_nzr == VX_NULL)
        {
            wb->max_per_core_per_vzgroup_nzr = (vx_float64*)vxAllocateAndZeroMemory(groupCount * sizeof(vx_float64));
            if (wb->max_per_core_per_vzgroup_nzr == VX_NULL)
            {
                vxError("reorderWeightBiasBufferForHuffman: OUT OF MEMORY\n");
                goto exit;
            }
        }

        nonZeroRatioPerCorePerVZG = (vx_float64*)vxAllocateAndZeroMemory(nnCoreCount * groupCount * sizeof(vx_float64));
        if (nonZeroRatioPerCorePerVZG == VX_NULL)
        {
            vxError("reorderWeightBiasBufferForHuffman: OUT OF MEMORY");
            goto exit;
        }
    }

    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        vx_uint32 coreFilterCount = (coreIndex < oddFilterPerCore) ? totalFilterPerCore + 1 : totalFilterPerCore;

        vx_uint32 usedKid = 0;
        vx_uint32* filterGroup = VX_NULL;

        reoder_stream_count_per_core[coreIndex] = 0;

        if (coreFilterCount > 0)
        {
            /* Save kernels per core*/
            if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
            {
                *((vx_uint16 *)kernelBufferInt8Ptr) = (vx_uint16)coreFilterCount;
                kernelBufferInt8Ptr += 2;
                reoder_stream_count_per_core[coreIndex] += (16 / weightBitSize);
            }
            else
            {
                *kernelBufferInt16Ptr = (vx_uint16)coreFilterCount;
                kernelBufferInt16Ptr ++;
                reoder_stream_count_per_core[coreIndex] += (16 / weightBitSize);
            }
            for (idx = 0; idx < (16 / weightBitSize); idx++)
            {
                non_coef_index[nonCoefIndex++] = elementIndex++;
            }
        }

        for (groupIndex = 0; groupIndex < groupCount; groupIndex++)
        {
            vx_uint32 blockCount = 0;
            vx_uint32 nonZeroCount = 0;
            vx_uint32 nonZrlIdx = groupIndex + coreIndex * groupCount;

            vx_uint32 actualFilterCount = (groupIndex == groupCount - 1) ? (coreFilterCount - groupIndex * filterCount) : filterCount;
            vx_uint32 groupFilterStart = groupIndex * nnCoreCount * filterCount;
            vx_uint32 kid = 0;
            vx_uint32 unusedCoreCount = (filterTotalCount % nnCoreCount == 0) ? 0 : nnCoreCount - (filterTotalCount % nnCoreCount);
            vx_uint32 sortedIndex = 0;
            vx_uint32 filterStart = 0;

            if (coreIndex == 0)
                core0FilterCount = actualFilterCount;

            if (actualFilterCount)
            {
                filterGroup = (vx_uint32*)vxAllocateAndZeroMemory(actualFilterCount * sizeof(vx_uint32));
                if (filterGroup == VX_NULL)
                {
                    vxError("fillinKernelBufferBalance: OUT OF MEMORY");
                    goto exit;
                }
            }

            if (reorder)
            {
                /*Dispatch filter to each vzGroup*/
                for (kid = 0; kid < actualFilterCount; kid++)
                {
                    if (groupIndex == groupCount - 1 &&
                        kid == core0FilterCount - 1)
                    {
                        if ((kid + usedKid) % 2 == 0)
                            sortedIndex = groupFilterStart + kid * nnCoreCount + coreIndex;
                        else
                            sortedIndex = groupFilterStart + (kid + 1) * nnCoreCount - coreIndex - 1 - unusedCoreCount;
                    }
                    else
                    {
                        if ((kid + usedKid) % 2 == 0)
                            sortedIndex = groupFilterStart + kid * nnCoreCount + coreIndex;
                        else
                            sortedIndex = groupFilterStart + (kid + 1) * nnCoreCount - coreIndex - 1;
                    }

                    filterGroup[kid] = nonZeroWeights[sortedIndex].filterIdx;
#if PRINT_PER_CORE_SIZE
                    nonZeroCoefPerCore[coreIndex] += nonZeroWeights[sortedIndex].nonZeroCount;
#endif
                }

                /*sort the vz group real filter to avoid write output cross*/
                for (kid = 0; kid < actualFilterCount; kid++)
                {
                    vx_uint32 tmp = filterGroup[kid];

                    for (i = kid - 1; i >= 0 && filterGroup[i] > tmp; i--)
                    {
                        filterGroup[i+1] = filterGroup[i];
                    }
                    filterGroup[i+1] = tmp;
                }
            }
            else
            {
                /*if variance is small, don't reorder the filters*/
                if ((groupIndex == groupCount - 1)
                && (coreIndex >= (filterTotalCount % nnCoreCount)))
                {
                    filterStart = groupIndex * nnCoreCount * filterCount
                                + (filterTotalCount % nnCoreCount) * (actualFilterCount + 1)
                                + (coreIndex - (filterTotalCount % nnCoreCount)) * actualFilterCount;
                }
                else
                {
                    filterStart = groupIndex * nnCoreCount *filterCount + coreIndex * actualFilterCount;
                }

                for (kid = 0; kid < actualFilterCount; kid++)
                {
                    filterGroup[kid] = filterStart + kid;
                }
            }


            if (wb->weights_sizes[0] == 1 && wb->weights_sizes[1] == 1 &&
                (hasZDP3 || hasZDP6) &&
                (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8))
            {
                vx_uint32 zdpNum = hasZDP6 ? 6 : 3;

                /* zdp3 zdp6 can not enable same time */
                vxmASSERT(!(hasZDP3 && hasZDP6));

                for (sliceIndex = 0; sliceIndex < slice_count; sliceIndex += (zdpNum * 2))
                {
                    vx_uint32 weight = 0;
                    vx_uint32 realSliceIndex = 0;

                    /* add slices of every filter*/
                    for (kid = 0; kid < actualFilterCount; kid++)
                    {
                        vx_uint32 nonZeroInBlock1Count = 0; /*Check if there's non-zero point in first zdpNum block*/
                        vx_uint32 nonZeroInBlock2Count = 0; /*Check if there's non-zero point in another zdpNum block*/

                        filterIndex = filterGroup[kid];

                        for (realSliceIndex = sliceIndex; realSliceIndex < sliceIndex + (zdpNum * 2); realSliceIndex++)
                        {
                            if (realSliceIndex >= slice_count)
                                break;

                            if (realSliceIndex == 0)
                            {
                                if (bias_base_ptr)
                                    biasData = *(bias_base_ptr + filterIndex);
                                else
                                    biasData = 0;

                                /* NewBias = sum((Coef[i] - CoefZP) * - InZP) + Bias*/
                                if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT)
                                    && inputZP != 0
                                    && weight_format == VX_TYPE_UINT8)
                                {
                                    biasData -= weightsMinusZP[filterIndex].sum;
                                }

                                if (!hasBiasAtEnd)
                                {
                                    *((vx_uint32 *)kernelBufferInt8Ptr) = biasData;
                                    kernelBufferInt8Ptr += 4;
                                    reoder_stream_count_per_core[coreIndex] += (biasBitSize / weightBitSize);
                                    for (idx = 0; idx < (biasBitSize / weightBitSize); idx++)
                                    {
                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                    }
                                }
                            }

                            /* add one slice data every filter */
                            kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * realSliceIndex;
                            if (weight_format == VX_TYPE_INT8)
                                weight = *((vx_int8 *)kernelDataPtr);
                            else if (weight_format == VX_TYPE_UINT8)
                                weight = *((vx_uint8 *)kernelDataPtr);
                            else
                                weight = *((vx_uint16 *)kernelDataPtr);

                            if (realSliceIndex - sliceIndex < zdpNum && weight != skipValue)
                                nonZeroInBlock1Count++;
                            else if (weight != skipValue)
                                nonZeroInBlock2Count++;

                            *kernelBufferInt8Ptr = (vx_uint8)weight;
                            kernelBufferInt8Ptr++;
                            reoder_stream_count_per_core[coreIndex] += 1;

                            if ((kid == actualFilterCount - 1) && realSliceIndex == 0)
                                zrl_limit_index[limitZRLIndex++] = elementIndex;

                            elementIndex++;

                            /* add offet behind the last point of the last slice of each filter */
                            if (realSliceIndex == slice_count - 1)
                            {
                                vx_uint32 offsetValue;

                                if (hasNNPerFilterPostMultiply)
                                {
                                    /*add post multiply to each filter*/

                                    vx_uint32 postMulSize = 8;
                                    *kernelBufferInt8Ptr = postMul[filterIndex];
                                    kernelBufferInt8Ptr++;
                                    reoder_stream_count_per_core[coreIndex] += (postMulSize / weightBitSize);
                                    for (idx = 0; idx < (postMulSize / weightBitSize); idx++)
                                    {
                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                    }
                                }
                                if (hasNNPerFilterPostShift)
                                {
                                    /*add post multiply to each filter*/

                                    vx_uint32 postShiftSize = 8;
                                    *kernelBufferInt8Ptr = postShift[filterIndex];
                                    kernelBufferInt8Ptr++;
                                    reoder_stream_count_per_core[coreIndex] += (postShiftSize / weightBitSize);
                                    for (idx = 0; idx < (postShiftSize / weightBitSize); idx++)
                                    {
                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                    }
                                }

                                if (z_offset > 0)
                                    offsetValue = (vx_uint32)z_offset * filterIndex;
                                else if (output_size > 0)
                                    offsetValue = output_final_x * output_final_y * output_size * filterIndex;
                                else
                                    offsetValue = output_final_x * output_final_y * weightSize * filterIndex;

                                if (hasBiasAtEnd)
                                {
                                    *((vx_uint32 *)kernelBufferInt8Ptr) = biasData;
                                    kernelBufferInt8Ptr += 4;
                                    reoder_stream_count_per_core[coreIndex] += (biasBitSize / weightBitSize);

                                    for (idx = 0; idx < (biasBitSize / weightBitSize); idx++)
                                    {
                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                    }
                                }

                                if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                {
                                    *((vx_uint32 *)kernelBufferInt8Ptr) = offsetValue;
                                    kernelBufferInt8Ptr += 4;
                                    reoder_stream_count_per_core[coreIndex] += (NN_Z_POSITION_OFFSET_BITS_VIP_V7 / weightBitSize);
                                    for (idx = 0; idx < (NN_Z_POSITION_OFFSET_BITS_VIP_V7 / weightBitSize); idx++)
                                    {
                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                    }
                                }
                            }
                        }
                        if (sliceIndex + zdpNum >= slice_count)
                            blockCount++;
                        else
                            blockCount += 2;

                        if (nonZeroInBlock1Count)
                            nonZeroCount++;
                        if (nonZeroInBlock2Count)
                            nonZeroCount++;
                    }
                }
            }
            /*Only INT8/UINT8 support XY DP9 now*/
            else if ((hasXYDP9 || hasXYDP6)
                && (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8))
            {
                vx_uint32 xStep = 0, yStep = 0;
                vx_uint32 inImageBufferSize = hasXYDP9 ? 2 : 1;

                /* xydp9 xydp6 can not enable same time */
                vxmASSERT(!(hasXYDP9 && hasXYDP6));

                if (hasXYDP6)
                {
                    xStep = 3;
                    yStep = 2;
                }
                else
                {
                    xStep = 3;
                    yStep = 3;
                }

                for (sliceIndex = 0; sliceIndex < slice_count; sliceIndex += inImageBufferSize)
                {
                    vx_uint32 weight = 0;
                    vx_uint32 realSliceIndex = 0;
                    vx_uint32 realWeightXIndex = 0, realWeightYIndex = 0;
                    vx_uint32 subKxSize = 0, subKySize = 0, subKzSize = 0;

                    subKzSize = gcmMIN((slice_count - sliceIndex), inImageBufferSize);
                    /* add slices of every filter*/
                    for (kid = 0; kid < actualFilterCount; kid++)
                    {
                        filterIndex = filterGroup[kid];
                        for (weightYIndex = 0; weightYIndex < wb->weights_sizes[1]; weightYIndex += yStep)
                        {
                            subKySize = gcmMIN((wb->weights_sizes[1] - weightYIndex), yStep);
                            for (weightXIndex = 0; weightXIndex < wb->weights_sizes[0]; weightXIndex += xStep)
                            {
                                subKxSize = gcmMIN((wb->weights_sizes[0] - weightXIndex), xStep);
                                for (realSliceIndex = sliceIndex; realSliceIndex < sliceIndex + subKzSize; realSliceIndex++)
                                {
                                    vx_uint32 nonZeroInBlockCount = 0; /*Check if there's non-zero point in one 3x3 block*/
                                    /* add one slice data every filter */
                                    kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * realSliceIndex;
                                    if (realSliceIndex == 0)
                                    {
                                        if (bias_base_ptr)
                                            biasData = *(bias_base_ptr + filterIndex);
                                        else
                                            biasData = 0;

                                        /* NewBias = sum((Coef[i] - CoefZP) * - InZP) + Bias*/
                                        if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT)
                                            && inputZP != 0
                                            && weight_format == VX_TYPE_UINT8)
                                        {
                                            biasData -= weightsMinusZP[filterIndex].sum;
                                        }
                                    }

                                    for (realWeightYIndex = weightYIndex; realWeightYIndex < weightYIndex + subKySize; realWeightYIndex++)
                                    {
                                        for (realWeightXIndex = weightXIndex; realWeightXIndex < weightXIndex + subKxSize; realWeightXIndex++)
                                        {
                                            vx_uint8_ptr realKernelDataPtr = kernelDataPtr + (realWeightYIndex * wb->weights_sizes[0] + realWeightXIndex) * weightSize;

                                            if (weight_format == VX_TYPE_INT8)
                                                weight = *((vx_int8 *)realKernelDataPtr);
                                            else if (weight_format == VX_TYPE_UINT8)
                                                weight = *((vx_uint8 *)realKernelDataPtr);
                                            else
                                                weight = *((vx_uint16 *)realKernelDataPtr);

                                            if (weight != skipValue)
                                                nonZeroInBlockCount++;

                                            if (!hasBiasAtEnd &&
                                                realWeightYIndex == 0 &&
                                                realWeightXIndex == 0 &&
                                                realSliceIndex == 0)
                                            {
                                                *((vx_uint32 *)kernelBufferInt8Ptr) = biasData;
                                                kernelBufferInt8Ptr += 4;
                                                reoder_stream_count_per_core[coreIndex] += (biasBitSize / weightBitSize);
                                                for (idx = 0; idx < (biasBitSize / weightBitSize); idx++)
                                                {
                                                    non_coef_index[nonCoefIndex++] = elementIndex++;
                                                }
                                            }
                                            *kernelBufferInt8Ptr = (vx_uint8)weight;
                                            kernelBufferInt8Ptr++;
                                            reoder_stream_count_per_core[coreIndex] += 1;

                                            if ((kid == actualFilterCount - 1) && realSliceIndex == 0 &&
                                                realWeightXIndex == 0 && realWeightYIndex == 0)
                                                zrl_limit_index[limitZRLIndex++] = elementIndex;

                                            elementIndex++;

                                            /* add offet behind the last point of the last slice of each filter */
                                            if (realSliceIndex == slice_count - 1 &&
                                                realWeightXIndex == wb->weights_sizes[0] - 1 &&
                                                realWeightYIndex == wb->weights_sizes[1] - 1)
                                            {
                                                vx_uint32 offsetValue;

                                                if (hasNNPerFilterPostMultiply)
                                                {
                                                    /*add post multiply to each filter*/

                                                    vx_uint32 postMulSize = 8;
                                                    *kernelBufferInt8Ptr = postMul[filterIndex];
                                                    kernelBufferInt8Ptr++;
                                                    reoder_stream_count_per_core[coreIndex] += (postMulSize / weightBitSize);
                                                    for (idx = 0; idx < (postMulSize / weightBitSize); idx++)
                                                    {
                                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                                    }
                                                }
                                                if (hasNNPerFilterPostShift)
                                                {
                                                    /*add post multiply to each filter*/

                                                    vx_uint32 postShiftSize = 8;
                                                    *kernelBufferInt8Ptr = postShift[filterIndex];
                                                    kernelBufferInt8Ptr++;
                                                    reoder_stream_count_per_core[coreIndex] += (postShiftSize / weightBitSize);
                                                    for (idx = 0; idx < (postShiftSize / weightBitSize); idx++)
                                                    {
                                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                                    }
                                                }

                                                if (z_offset > 0)
                                                    offsetValue = (vx_uint32)z_offset * filterIndex;
                                                else if (output_size > 0)
                                                    offsetValue = output_final_x * output_final_y * output_size * filterIndex;
                                                else
                                                    offsetValue = output_final_x * output_final_y * weightSize * filterIndex;

                                                if (hasBiasAtEnd)
                                                {
                                                    *((vx_uint32 *)kernelBufferInt8Ptr) = biasData;
                                                    kernelBufferInt8Ptr += 4;
                                                    reoder_stream_count_per_core[coreIndex] += (biasBitSize / weightBitSize);
                                                    for (idx = 0; idx < (biasBitSize / weightBitSize); idx++)
                                                    {
                                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                                    }
                                                }

                                                if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                                {
                                                    *((vx_uint32 *)kernelBufferInt8Ptr) = offsetValue;
                                                    kernelBufferInt8Ptr += 4;
                                                    reoder_stream_count_per_core[coreIndex] += (NN_Z_POSITION_OFFSET_BITS_VIP_V7 / weightBitSize);
                                                    for (idx = 0; idx < (NN_Z_POSITION_OFFSET_BITS_VIP_V7 / weightBitSize); idx++)
                                                    {
                                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                                    }
                                                }

                                            }
                                        }
                                    }
                                    blockCount++;
                                    if (nonZeroInBlockCount)
                                        nonZeroCount++;
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                for (sliceIndex = 0; sliceIndex < slice_count; sliceIndex++)
                {
                    /* add slices of every filter*/
                    for (kid = 0; kid < actualFilterCount; kid++)
                    {
                        filterIndex = filterGroup[kid];
                        /* add one slice data every filter */
                        kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * sliceIndex;
                        if (sliceIndex == 0)
                        {
                            if (bias_base_ptr)
                                biasData = *(bias_base_ptr + filterIndex);
                            else
                                biasData = 0;

                            /* NewBias = sum((Coef[i] - CoefZP) * - InZP) + Bias*/
                            if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT)
                                && inputZP != 0
                                && weight_format == VX_TYPE_UINT8)
                            {
                                biasData -= weightsMinusZP[filterIndex].sum;
                            }
                        }

                        for (weightYIndex = 0; weightYIndex < wb->weights_sizes[1]; weightYIndex++)
                        {
                            vx_uint32 nonZeroCountInDP3 = 0; /*Check if there's non-zero point in one 3x1 block*/
                            for (weightXIndex = 0; weightXIndex < wb->weights_sizes[0]; weightXIndex++)
                            {
                                vx_uint32 weight = 0;

                                if (weight_format == VX_TYPE_INT8)
                                    weight = *((vx_int8 *)kernelDataPtr);
                                else if (weight_format == VX_TYPE_UINT8)
                                    weight = *((vx_uint8 *)kernelDataPtr);
                                else
                                    weight = *((vx_uint16 *)kernelDataPtr);
                                kernelDataPtr = kernelDataPtr + weightSize;

                                {
                                    if (weight != skipValue)
                                        nonZeroCountInDP3++;

                                    /*V7, DP3, one block is 3x1*/
                                    if ((weightXIndex + 1) % 3 == 0 || weightXIndex == wb->weights_sizes[0] - 1)
                                    {
                                        blockCount++;
                                        if (nonZeroCountInDP3)
                                            nonZeroCount++;
                                        nonZeroCountInDP3 = 0;
                                    }
                                }

                                if (!hasBiasAtEnd &&
                                    weightXIndex == 0 &&
                                    weightYIndex == 0 &&
                                    sliceIndex == 0)
                                {
                                    if (weight_format == VX_TYPE_INT16)
                                    {
                                        vx_int64 bias64 = 0;
                                        vx_int32 bias32;
                                        vx_int16 bias16;

                                        if ((vx_int32)biasData < 0)
                                        {
                                            bias64 = ~0;
                                            bias64 = (bias64 >> 32) << 32;
                                            bias64 = bias64 | (vx_int64)biasData;
                                        }
                                        else
                                            bias64 = (vx_int64)biasData;

                                        bias32 = (vx_int32)bias64;
                                        *((vx_uint32 *)kernelBufferInt16Ptr) = bias32;
                                        kernelBufferInt16Ptr += 2;
                                        reoder_stream_count_per_core[coreIndex] += (32 / weightBitSize);
                                        bias16 = (vx_int16)(bias64 >> 32);
                                        *kernelBufferInt16Ptr = bias16;
                                        kernelBufferInt16Ptr++;
                                        reoder_stream_count_per_core[coreIndex] += (16 / weightBitSize);
                                    }
                                    else if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
                                    {
                                        *((vx_uint32 *)kernelBufferInt8Ptr) = biasData;
                                        kernelBufferInt8Ptr += 4;
                                        reoder_stream_count_per_core[coreIndex] += (biasBitSize / weightBitSize);
                                    }
                                    else
                                    {
                                        *((vx_uint32 *)kernelBufferInt16Ptr) = biasData;
                                        kernelBufferInt16Ptr += 2;
                                        reoder_stream_count_per_core[coreIndex] += (biasBitSize / weightBitSize);
                                    }

                                    for (idx = 0; idx < (biasBitSize / weightBitSize); idx++)
                                    {
                                        non_coef_index[nonCoefIndex++] = elementIndex++;
                                    }
                                }

                                if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
                                {
                                    *kernelBufferInt8Ptr = (vx_uint8)weight;
                                    kernelBufferInt8Ptr++;
                                }
                                else
                                {
                                    if (weight_format == VX_TYPE_FLOAT16)
                                        weight = (weight & 0x7fff) * 2 + weight/(1<<15);

                                    *kernelBufferInt16Ptr = (vx_uint16)weight;
                                    kernelBufferInt16Ptr++;
                                }
                                reoder_stream_count_per_core[coreIndex] += 1;

                                if ((kid == actualFilterCount - 1) && sliceIndex == 0 &&
                                    weightXIndex == 0 && weightYIndex == 0)
                                    zrl_limit_index[limitZRLIndex++] = elementIndex;

                                elementIndex++;
                            }
                        }

                        /* add offet behind the last point of the last slice of each filter */
                        if (sliceIndex == slice_count - 1)
                        {
                            vx_uint32 offsetValue;

                            if (hasNNPerFilterPostMultiply && hasNNPerFilterPostShift)
                            {
                                if (weight_format == VX_TYPE_INT16 || weight_format == VX_TYPE_FLOAT16)
                                {
                                    vx_uint32 postMulSize = 16;
                                    vx_uint32 postShiftSize = 16;
                                    /*add post multiply to each filter*/
                                    *kernelBufferInt16Ptr = (vx_uint16)postMul[filterIndex];
                                    kernelBufferInt16Ptr++;
                                    reoder_stream_count_per_core[coreIndex] += (postMulSize / weightBitSize);
                                    /*add post multiply to each filter*/
                                    *kernelBufferInt16Ptr = (vx_uint16)postShift[filterIndex];
                                    kernelBufferInt16Ptr++;
                                    reoder_stream_count_per_core[coreIndex] += (postShiftSize / weightBitSize);
                                }
                                else
                                {
                                    vx_uint32 postMulSize = 8;
                                    vx_uint32 postShiftSize = 8;
                                    /*add post multiply to each filter*/
                                    *kernelBufferInt8Ptr = postMul[filterIndex];
                                    kernelBufferInt8Ptr++;
                                    reoder_stream_count_per_core[coreIndex] += (postMulSize / weightBitSize);
                                    /*add post multiply to each filter*/
                                    *kernelBufferInt8Ptr = postShift[filterIndex];
                                    kernelBufferInt8Ptr++;
                                    reoder_stream_count_per_core[coreIndex] += (postShiftSize / weightBitSize);
                                }

                                for (idx = 0; idx < 2; idx++)
                                {
                                    non_coef_index[nonCoefIndex++] = elementIndex++;
                                }
                            }

                            if (z_offset > 0)
                                offsetValue = (vx_uint32)z_offset * filterIndex;
                            else if (output_size > 0)
                                offsetValue = output_final_x * output_final_y * output_size * filterIndex;
                            else
                                offsetValue = output_final_x * output_final_y * weightSize * filterIndex;

                            if (hasBiasAtEnd)
                            {
                                if (weight_format == VX_TYPE_INT16)
                                {
                                    vx_int64 bias64 = 0;
                                    vx_int32 bias32;
                                    vx_int16 bias16;

                                    if ((vx_int32)biasData < 0)
                                    {
                                        bias64 = ~0;
                                        bias64 = (bias64 >> 32) << 32;
                                        bias64 = bias64 | (vx_int64)biasData;
                                    }
                                    else
                                        bias64 = (vx_int64)biasData;

                                    bias32 = (vx_int32)bias64;
                                    *((vx_uint32 *)kernelBufferInt16Ptr) = bias32;
                                    kernelBufferInt16Ptr += 2;
                                    reoder_stream_count_per_core[coreIndex] += (32 / weightBitSize);
                                    bias16 = (vx_int16)(bias64 >> 32);
                                    *kernelBufferInt16Ptr = bias16;
                                    kernelBufferInt16Ptr++;
                                    reoder_stream_count_per_core[coreIndex] += (16 / weightBitSize);
                                }
                                else if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
                                {
                                    *((vx_uint32 *)kernelBufferInt8Ptr) = biasData;
                                    kernelBufferInt8Ptr += 4;
                                    reoder_stream_count_per_core[coreIndex] += (biasBitSize / weightBitSize);
                                }
                                else
                                {
                                    *((vx_uint32 *)kernelBufferInt16Ptr) = biasData;
                                    kernelBufferInt16Ptr += 2;
                                    reoder_stream_count_per_core[coreIndex] += (biasBitSize / weightBitSize);
                                }

                                for (idx = 0; idx < (biasBitSize / weightBitSize); idx++)
                                {
                                    non_coef_index[nonCoefIndex++] = elementIndex++;
                                }
                            }

                            if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                            {
                                if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
                                {
                                    *((vx_uint32 *)kernelBufferInt8Ptr) = offsetValue;
                                    kernelBufferInt8Ptr += 4;
                                }
                                else
                                {
                                    *((vx_uint32 *)kernelBufferInt16Ptr) = offsetValue;
                                    kernelBufferInt16Ptr += 2;
                                }
                                reoder_stream_count_per_core[coreIndex] += (NN_Z_POSITION_OFFSET_BITS_VIP_V7 / weightBitSize);
                                for (idx = 0; idx < (NN_Z_POSITION_OFFSET_BITS_VIP_V7 / weightBitSize); idx++)
                                {
                                    non_coef_index[nonCoefIndex++] = elementIndex++;
                                }
                            }
                        }
                    }
                }
            }
            if (calc_perf)
                nonZeroRatioPerCorePerVZG[nonZrlIdx] = (vx_float64)nonZeroCount / (vx_float64)blockCount;

            usedKid += actualFilterCount;
            if (filterGroup)
            {
                vxFree(filterGroup);
                filterGroup = VX_NULL;
            }
        }
    }

    if (calc_perf)
    {
        for (groupIndex = 0; groupIndex < groupCount; groupIndex++)
        {
            for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
            {
                vx_uint32 nonZrlIdx = groupIndex + coreIndex * groupCount;
                if (wb->max_per_core_per_vzgroup_nzr[groupIndex] < nonZeroRatioPerCorePerVZG[nonZrlIdx])
                    wb->max_per_core_per_vzgroup_nzr[groupIndex] = nonZeroRatioPerCorePerVZG[nonZrlIdx];
            }
        }
    }
#if DEBUG_COMP_ENHANCE_ENC
    {
        unsigned char * pChar = (unsigned char*)reorder_stream;
        unsigned char data;
        vx_uint32 i;
        vx_uint32 totalByteCount = weightCount * slice_count * z_count * weightSize + z_count * 4 + z_count * (biasBitSize / 8) + nnCoreCount * 2;
        FILE * pfile1 = NULL;
        pfile1 = fopen("reordered_kernel.txt", "wb");

        if(pfile1 != NULL)
        {
            for(i=0; i!=totalByteCount; i++)
            {
                data = *pChar;
                fprintf(pfile1, "coef: 0x%x\n", data);
                pChar++;
            }

            fclose(pfile1);
        }

    }
#endif

exit:
    if (weightsMinusZP)
        vxFree(weightsMinusZP);
    if (nonZeroRatioPerCorePerVZG)
        vxFree(nonZeroRatioPerCorePerVZG);
    if (nonZeroWeights)
        vxFree(nonZeroWeights);
#if PRINT_PER_CORE_SIZE
    if(nonZeroCoefPerCore)
        vxFree(nonZeroCoefPerCore);
#endif
    return;
}

void analysisKernelStreamForHuffman(
    vx_context context,
    vx_weights_biases_parameter wb,
    vx_uint32 skipValue,
    vx_uint32 nnCoreCount,
    vx_uint8* reorderStream,
    vx_uint32 reorderStreamSize,
    vx_uint32* reorderStreamPerCoreCount,
    vx_uint32* invSizeOrder,
    vx_uint32* nonCoefIndex,
    vx_uint32 nonCoefCount,
    vx_uint32* limitZRLIndex,
    vx_uint32 limitZRLCount,
    vx_uint32 index
    )
{
    vx_uint8  * pBS_U08      = (unsigned char * )reorderStream;
    vx_uint16 * pBS_U16      = (unsigned short *)reorderStream;
    vx_enum weightFormat = wb->wb_base->weights_data_format;
    vx_uint8 bit16Flag   = (weightFormat == VX_TYPE_INT8 || weightFormat == VX_TYPE_UINT8) ? 0 : 1;
    vx_uint32 kernelDataCount  = reorderStreamSize >> bit16Flag;
    vx_uint32 kernelDataBytes  = reorderStreamSize;
    vx_uint32 dataCountToPross = kernelDataCount;

    vx_int32 i, j, k, bias;
    vx_int32 x=0;
    vx_int32 runZeros[256], run; /*The histogram of how many runs*/
    vx_int32 histo[256], sizeHisto[9] = {0,0,0,0,0,0,0,0,0};
    vx_int32 sizeOrder[]  = {0,1,2,3,4,5,6,7,8,};
    vx_int32 codingType   = HUFFMAN_CODING_TYPE_RUN_LEN; /* defalt RZL*/
    vx_int32 prevEncode   = 0, prev = 0, prevHisto[256], prevRunZeros[256], prevRun; /*previous pixel*/
    vx_float32 p, entropy = 0.f, prevEntropy = 0.f;
    vx_int32 coreId = 0, accumSize = 0x0;

    vx_uint32 totalCountIdx = 0;
    vx_uint32 nonCoefCountIdx = 0;
    vx_uint32 limitZRLCountIdx = 0;
    vx_bool isCoef;
    vx_bool isLimitZRL;
    /*per PRD, V8 will disable pre & bias mode*/
    vx_bool isV8 = (vx_bool)(context->nnConfig.derivedFeature.nnXYDPX == 0 || context->nnConfig.derivedFeature.nnXYDPY == 0);

    /* this variable indicate DataAccumCount when Start of core*/
    vx_int32 *accumCountEndOfCore = (vx_int32 *)vxAllocateAndZeroMemory(nnCoreCount * sizeof(vx_int32));

    if (accumCountEndOfCore == VX_NULL)
    {
        vxError("analysisKernelStreamForHuffman: OUT OF MEMORY\n");
        goto exit;
    }

    for (i = 0; i < 256; i++)
    {
        runZeros[i] = histo[i] = 0;
        prevHisto[i] = prevRunZeros[i] = 0;
    }


    for (coreId = 0; coreId != (vx_int32)nnCoreCount; coreId++)
    {
        accumSize += reorderStreamPerCoreCount[coreId];
        accumCountEndOfCore[coreId] = accumSize - 1;
    }

    prevRun = run = 0;
    coreId = 0;
    while(dataCountToPross--)
    {
        vx_uint32 endOfCore = (dataCountToPross + accumCountEndOfCore[coreId] == kernelDataCount - 1) ? 1 : 0;
        if(endOfCore)
        {
            coreId++;
        }

        if (!bit16Flag)
        {
            i = *(pBS_U08++);
        }
        else
        {
            i = *(pBS_U16++);
            if (weightFormat == VX_TYPE_FLOAT16)
            {
                i = (i & 0x7fff) * 2 + i/(1<<15); /*(i&0x8000)/128 + (i&0x7f00)*2 + (i&0xff);*/
            }
        }

        if (nonCoefIndex != VX_NULL &&
            totalCountIdx == nonCoefIndex[nonCoefCountIdx] &&
            nonCoefCountIdx < nonCoefCount)
        {
            isCoef = vx_false_e;
            nonCoefCountIdx++;
        }
        else
            isCoef = vx_true_e;

        if (limitZRLIndex != VX_NULL &&
            totalCountIdx == limitZRLIndex[limitZRLCountIdx] &&
            limitZRLCountIdx < limitZRLCount)
        {
            isLimitZRL = vx_true_e;
            limitZRLCountIdx++;
        }
        else
            isLimitZRL = vx_false_e;

        totalCountIdx++;

        if (i == 0 && isCoef && !isLimitZRL)
            run++;
        if ((run == sizeof(runZeros)/sizeof(int)) || i || endOfCore || !isCoef || isLimitZRL)
        {
            if (run)
            {
                runZeros[run - 1]++;
                run = 0;
            }
        }
        histo[i>>(bit16Flag*8)]++;
        /*Get statistic of prediction from previous pixel*/
        if(bit16Flag == 0 && !isV8)
        {
            j = i;
            i = (i-prev)&0xff;
            prev = j;
            if (i == 0 && isCoef && !isLimitZRL) prevRun++;
            if (prevRun == sizeof(prevRunZeros)/sizeof(int) || i || endOfCore || !isCoef || isLimitZRL)
            {
                if (prevRun){
                    prevRunZeros[prevRun - 1]++;
                    prevRun = 0;
                }
            }
            prevHisto[i]++;
        }

        /* prev encode should be truncated between cores, when analysis, reset prev.*/
        if (endOfCore)
        {
            prev = 0x0;
        }
    }
    if (run)
    { /*last run*/
        runZeros[run - 1]++;
        run = 0;
    }
    if (prevRun)
    { /*last run*/
        prevRunZeros[prevRun - 1]++;
        prevRun = 0;
    }

    j = histo[0];
    bias = 0;

    for (i = 1; i < 256; i++)
    {
        if (histo[i] > j)
        {
            j = histo[i];
            bias = i;
        }
    }
    if (j*3 < histo[0]*4 && bit16Flag == 0)
        bias = 0;

    /*16 bits, we only get the high bit bias, we need to get the low bit bias*/
    if (bit16Flag)
    {
        pBS_U16 = (unsigned short *)reorderStream;
        dataCountToPross = kernelDataCount;
        while(dataCountToPross--)
        {
            i = *(pBS_U16);
            pBS_U16++;
            if (weightFormat == VX_TYPE_FLOAT16)
                i = (i&0x7fff)*2 + i/(1<<15);

            if ((i>>8) == bias)
                prevHisto[i&0xff]++;
        }

        j = prevHisto[0];
        k = 0;

        for (i = 1; i < 256; i++)
        {
            if (prevHisto[i] > j)
            {
                j = prevHisto[i];
                k = i;
            }
        }
        bias = bias*256+k;
    }

    x = 0;
    for (i = 0; i<256; i++)
    {
        p = histo[i] / (vx_float32)(kernelDataCount);
        if (histo[i])
            entropy += -p*(vx_float32)log(p)/(vx_float32)log(2.0f);

        p =  prevHisto[i] / (vx_float32)reorderStreamSize;
        if (prevHisto[i])
            prevEntropy += - p*(vx_float32)log(p)/(vx_float32)log(2.0f);
    }

    if (bit16Flag == 0)
    {
        if ((prevEntropy < entropy || 2*prevHisto[0] + prevHisto[255] + prevHisto[1] > 3*histo[bias] + histo[(bias+1)&0xff] + histo[(bias-1)&0xff]) && !isV8)
        {
            /*per PRD, V8 will disable pre & bias mode*/
            entropy = prevEntropy;
            prevEncode = 1; /*Using prevEncode, prediction from previous pixel*/
            for (i = 0; i < 256; i++)
            {
                histo[i] = prevHisto[i];
                runZeros[i] = prevRunZeros[i];
            }
            bias = 0;
        }
    }
    prevEntropy = entropy;


    if (isV8)
        bias = 0;

    if (bias != 0)
    {
        totalCountIdx = 0;
        nonCoefCountIdx = 0;
        limitZRLCountIdx = 0;
        for (i = 0; i < 256; i++)
        {
            histo[i] = runZeros[i] = 0;
        }
        dataCountToPross = kernelDataCount;
        pBS_U08 = (vx_uint8 * )reorderStream;
        pBS_U16 = (vx_uint16 *)reorderStream;
        while(dataCountToPross--)
        {
            if (bit16Flag == 0)
            {
                i = *(pBS_U08++);
            }
            else
            {
                i = *(pBS_U16++);
                if (weightFormat == VX_TYPE_FLOAT16)
                {
                    i = (i&0x7fff)*2 + i/(1<<15);
                }
            }

            i = (i - bias);

            if (nonCoefIndex != VX_NULL &&
                totalCountIdx == nonCoefIndex[nonCoefCountIdx] &&
                nonCoefCountIdx < nonCoefCount)
            {
                isCoef = vx_false_e;
                nonCoefCountIdx++;
            }
            else
                isCoef = vx_true_e;

            if (limitZRLIndex != VX_NULL &&
                totalCountIdx == limitZRLIndex[limitZRLCountIdx] &&
                limitZRLCountIdx < limitZRLCount)
            {
                isLimitZRL = vx_true_e;
                limitZRLCountIdx++;
            }
            else
                isLimitZRL = vx_false_e;

            totalCountIdx++;

            if (i == 0 && isCoef && !isLimitZRL) run++;
            if (run == sizeof(runZeros) / sizeof(vx_int32) || i || !isCoef || isLimitZRL)
            {
                if(run)
                {
                    runZeros[run - 1]++;
                    run = 0;
                }
            }

            histo[(i>>(bit16Flag*8))&0xff]++;
        }
        if (run)
        { /*last run*/
            runZeros[run - 1]++;
            run = 0;
        }
    }

    /*Get all the runZeros present. supposed to be histo[0], but, for 16-bit, we only count the runs for double zeros.*/
    for (i = 0; i < 256; i++)
    {
        run += runZeros[i] * (i+1);
    }

    if (bit16Flag == 0 && ((unsigned int)histo[0] <= reorderStreamSize / 8 || histo[0] < (histo[1] + histo[255]) * 3/4 || entropy >= 5.25f) && entropy > 3.5f )
    {
         codingType = HUFFMAN_CODING_TYPE_NON_RUN_LEN;
    }
    if (bit16Flag && run < (histo[1] + histo[255]) / 4)
    {
         codingType = HUFFMAN_CODING_TYPE_NON_RUN_LEN;
    }
    if (bit16Flag && entropy > 5.25f)
        codingType = 0; /*Force to test run-length*/


    if (codingType == HUFFMAN_CODING_TYPE_RUN_LEN)
    {
        FindBest2PlusRunSets(runZeros, sizeof(runZeros)/sizeof(int), (reorderStreamSize >> bit16Flag) - run);
    }

    if (codingType == HUFFMAN_CODING_TYPE_NON_RUN_LEN)
    {
        j = 0;
        for (i = 0; i < 8;i++){
            for (; j<(1<<i); j++){
                sizeHisto[i] += histo[j] + histo[j^0xff];
            }
        }

        for (i = 0; i < 7; i++){
            vx_int32 maxFreq = 0, maxPos = i;
            for (j = i; j<8; j++){
                if (maxFreq<sizeHisto[j])
                {
                    maxFreq = sizeHisto[j];
                    maxPos = j;
                }
            }
            if (maxPos > i)
            {
                j = sizeHisto[maxPos];
                sizeHisto[maxPos] = sizeHisto[i];
                sizeHisto[i] = j;

                j = sizeOrder[maxPos];
                sizeOrder[maxPos] = sizeOrder[i];
                sizeOrder[i] = j;
            }
        }
        for (i = 0; i < 8; i++)
        {
            invSizeOrder[sizeOrder[i]] = i;
        }
        entropy = 0.f;
        j = 0;
        for (i = 0; i < 8; i++)
        {
            p = sizeHisto[i]/(vx_float32)kernelDataBytes;
            if(p>0.f)
                entropy += -p*(vx_float32)log(p)/(vx_float32)log(2.0f);
            j += sizeHisto[i]*sizeCodeLen[i];
        }
    }

    if (codingType == HUFFMAN_CODING_TYPE_RUN_LEN)
    {
        j = 1;
        if (bit16Flag)
           histo[0] -= run; /*The zeros not runs*/

        run = 0;
        for (i = 1; i < 8; i++)
        {
            for (; j < (1<<i); j++)
            {
                sizeHisto[i] += histo[j - bit16Flag] + histo[(-j)&0xff];
            }
        }
        sizeHisto[7] += histo[0x80];/*128 haven't put in yet.*/

        /*Get the frequency of run-length.*/
        for (i = 0; i < numBestRuns; i++)
        {
            if (freq[i] < 0) /*The set-1*/
                sizeHisto[0] += abs(freq[i]);
            else /*Set-2*/
                sizeHisto[8] += abs(freq[i]);
        }

        k = sizeHisto[1];
        for (i = 2; i < 7; i++)
        {
            if (sizeHisto[i] < k)
                k = sizeHisto[i];
        }

        /*avoid the run-length be the last*/
        if (sizeHisto[0] <= k)
            sizeHisto[0] = k+2;
        if (sizeHisto[8] <= k)
            sizeHisto[8] = k+2;

        if (sizeHisto[7] <= k) /*The 7 is the escape code, shouldn't be the last (the last will be mergy to 7).*/
            sizeHisto[7] = k+1;


         /*sorting the sizeHisto*/
         for (i = 0; i<8; i++)
         {
            vx_int32 maxFreq = 0, maxPos = i;
            for (j = i; j < 9; j++)
            {
                if(maxFreq < sizeHisto[j])
                {
                    maxFreq = sizeHisto[j];
                    maxPos = j;
                }
            }
            if (maxPos > i)
            {
                j = sizeHisto[maxPos];
                sizeHisto[maxPos] = sizeHisto[i];
                sizeHisto[i] = j;

                j = sizeOrder[maxPos];
                sizeOrder[maxPos] = sizeOrder[i];
                sizeOrder[i] = j;
            }
        }

        if(sizeOrder[7]%8 == 0 && sizeOrder[6]%8 > 0)
        { /*The run-length cannot be the escape code. swap to size 6*/
            j = sizeOrder[6];
            sizeOrder[6] = sizeOrder[7];
            sizeOrder[7] = j;

            j = sizeHisto[6];
            sizeHisto[6] = sizeHisto[7];
            sizeHisto[7] = j;

        }
        else if(sizeOrder[7]%8 == 0 && sizeOrder[6]%8 == 0)
        { /*The run-length cannot be the escape code. swap to size 6*/
            /*5,7,6*/
            j = sizeOrder[6];
            sizeOrder[6] = sizeOrder[7];
            sizeOrder[7] = j;

            j = sizeHisto[6];
            sizeHisto[6] = sizeHisto[7];
            sizeHisto[7] = j;
            /*6,7,5*/
            j = sizeOrder[5];
            sizeOrder[5] = sizeOrder[7];
            sizeOrder[7] = j;

            j = sizeHisto[5];
            sizeHisto[5] = sizeHisto[7];
            sizeHisto[7] = j;
        }

        for(i = 0; i < 9; i++)
            invSizeOrder[sizeOrder[i]] = i;

        entropy = 0.f;
        j = 0;
        x = 0;
        for(i = 0; i < 9; i++)
        {
            x += sizeHisto[i];
        }
        for(i = 0; i < 8; i++)
        {
            p = sizeHisto[i] / (vx_float32)x;
            if (p > 0.f)
                entropy += -p*(vx_float32)log(p)/(vx_float32)log(2.0f);
            j += sizeHisto[i]*sizeCodeLen[i];
        }
    }

    /*per PRD, V8 will disable pre & bias mode*/
    if (isV8)
    {
        gcmASSERT(prevEncode == 0 && bias == 0);
    }
    /* update compression header*/
    wb->huffmanConfig[index].preEncode = (vx_uint8)prevEncode;
    wb->huffmanConfig[index].bit16Flag  = bit16Flag;
    wb->huffmanConfig[index].fp16Flag   = (weightFormat == VX_TYPE_FLOAT16) ? 1 : 0;
    wb->huffmanConfig[index].reserved   = 0x0; /* must be zero*/
    wb->huffmanConfig[index].version    = 0x1; /* 4'b0001 for version 1.0*/
    wb->huffmanConfig[index].runLenTableSize = (codingType == HUFFMAN_CODING_TYPE_NON_RUN_LEN) ? 0: (vx_uint8)numBestRuns;

    /* runlenTable 8 * 18 = 144*/
    for(i = 0; i<SET_1_SIZE; i++)
    {
        wb->huffmanConfig[index].runLenTable[i] = (vx_uint8)best2Runs[i];
    }
    for(; i< numBestRuns; i++)
    {
        wb->huffmanConfig[index].runLenTable[i] = (vx_uint8)outBest2Run[i - SET_1_SIZE];
    }
    for(i = 0; i<8; i+=2)
    {
        wb->huffmanConfig[index].mapToHuffman[i/2] = (vx_uint8)(sizeOrder[i+1]*16 + sizeOrder[i]);
    }
    wb->huffmanConfig[index].avgBias = (vx_uint8)bias;
    wb->huffmanConfig[index].reserved = 0x0; /* reserved 16, must be zero*/

exit:
    if (accumCountEndOfCore != VX_NULL)
        vxFree(accumCountEndOfCore);
}

vx_uint32 calcKernelStreamSizeHuffman(
    vx_context context,
    vx_weights_biases_parameter wb,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_int32 input_zp,
    vx_int32 coef_zp,
    vx_uint32 skip_value,
    vx_uint32 output_final_x,
    vx_uint32 output_final_y,
    vx_int32 z_offset,
    vx_uint32 output_size,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr,
    vx_uint32 index
    )
{
    vx_uint32 kernelSize = 0;
    vx_uint32 kernelBitSize = 0;

    vx_uint32 nnCoreCount = weight_format == VX_TYPE_INT16 ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : weight_format == VX_TYPE_FLOAT16 ?
                      context->nnConfig.fixedFeature.nnCoreCountFloat16 : context->nnConfig.fixedFeature.nnCoreCount;

    vx_uint32 invSizeOrder[MAX_SIZE_HISTO_COUNT] = {0};
    vx_int32 coef = 0, tmp = 0, prev = 0;

    vx_uint32 filterTotalCount   = z_count;
    vx_uint32 usedCoreCount      = (filterTotalCount > nnCoreCount) ? nnCoreCount : filterTotalCount;
    vx_uint32 filterCount        = filters_per_core; /* filter count every group */
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;
    vx_uint32 limitZRLCount      = groupCount * usedCoreCount;
    vx_uint32 *limitZRLIndex     = VX_NULL;

    vx_uint32 weightCount        = weight_x * weight_y;
    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize((vx_type_e)weight_format);
    vx_uint32 weightBitSize = weightSize * 8;
    vx_uint32 biasBitSize      = 0;

    vx_uint32 kernelStreamSizePerCore = 0;
    vx_uint8_ptr kernelDataPtr   = VX_NULL;

    vx_uint32 coreIndex;
    vx_uint32 totalCountIdx = 0;
    vx_uint32 nonCoefCountIdx = 0;

    vx_uint32 bit16Flag = (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8) ? 0 : 1;

    vx_uint32 reorderStreamSize = 0;
    vx_uint32 reorderStreamAllCount = 0;
    vx_uint32 reorderStreamCheckCount = 0;
    vx_uint32* reorderStreamPerCoreCount = VX_NULL;
    vx_uint32 nonCoefCount = 0;
    vx_uint32 limitZRLCountIdx = 0;
    vx_uint32* nonCoefIndex = VX_NULL;

    vx_uint8* reorderStream = VX_NULL;
    vx_uint32 runSet2Bits = 1;
    CodeSymbol codeSymbol[THROUGHPUT * 3];

    if (weight_format == VX_TYPE_INT16)
        biasBitSize = NN_INTEGER_BIAS_BITS_VIP_V7_INT16;
    else
        biasBitSize = NN_INTEGER_BIAS_BITS_VIP_V7;

    reorderStreamAllCount = (filterTotalCount * (biasBitSize / weightBitSize))
        + (weightCount * slice_count * filterTotalCount)
        + (filterTotalCount * (NN_Z_POSITION_OFFSET_BITS_VIP_V7 / weightBitSize))
        + (nnCoreCount * (16 / weightBitSize));/*filterPerCore need 16 bit * nnCoreCount */

    reorderStreamSize = reorderStreamAllCount * weightSize;

    reorderStream = (vx_uint8 *)vxAllocateAndZeroMemory(reorderStreamSize);

    if (!reorderStream)
    {
        vxError("fillinKernelBufferHuffman: OUT OF MEMORY");
        goto exit;
    }

    reorderStreamPerCoreCount = (vx_uint32 *)vxAllocateAndZeroMemory(nnCoreCount * sizeof(vx_uint32));
    if (!reorderStreamPerCoreCount)
    {
        vxError("fillinKernelBufferHuffman: OUT OF MEMORY");
        goto exit;
    }

    nonCoefCount = (filterTotalCount * (biasBitSize / weightBitSize))
        + (filterTotalCount * (NN_Z_POSITION_OFFSET_BITS_VIP_V7 / weightBitSize))
        + (nnCoreCount * (16 / weightBitSize)); /*Those non coef count contain bias, Z_OFF_SET & kernels_per_core*/
    nonCoefIndex = (vx_uint32 *)vxAllocateAndZeroMemory(nonCoefCount * sizeof(vx_uint32));
    if (!nonCoefIndex)
    {
        vxError("fillinKernelBufferHuffman: OUT OF MEMORY");
        goto exit;
    }

    limitZRLIndex = (vx_uint32 *)vxAllocateAndZeroMemory(limitZRLCount * sizeof(vx_uint32));
    if (!limitZRLIndex)
    {
        vxError("fillinKernelBufferHuffman: OUT OF MEMORY");
        goto exit;
    }

    if (context->options.enableNonZeroBalance)
    {
        reorderKernelBufferV7HuffmanBalance(context, wb, slice_count, z_count, filters_per_core, weight_format, bias_format, z_offset,
        output_size, skip_value, output_final_x, output_final_y, reorderStream, weight_base_ptr, bias_base_ptr, VX_NULL, VX_NULL, reorderStreamPerCoreCount, nonCoefIndex, limitZRLIndex, vx_false_e);
    }
    else
    {
        reorderWeightBiasBufferForHuffman(context, wb, slice_count, z_count, filters_per_core, weight_format, bias_format, z_offset,
            output_size, skip_value, output_final_x, output_final_y, reorderStream, weight_base_ptr, bias_base_ptr, VX_NULL, VX_NULL, reorderStreamPerCoreCount, nonCoefIndex, limitZRLIndex, vx_false_e);
    }

    analysisKernelStreamForHuffman(context, wb, 0, nnCoreCount, reorderStream, reorderStreamSize, reorderStreamPerCoreCount, invSizeOrder, nonCoefIndex, nonCoefCount, limitZRLIndex, limitZRLCount, index);

    kernelBitSize += 16;
    kernelBitSize += 8 * MAX_RUNLEN_SIZE;
    kernelBitSize += 8 * LOG_RUN_SIZE1;
    kernelBitSize += 32;
    kernelBitSize += 32 * nnCoreCount;

    kernelSize = (kernelBitSize % 8 == 0) ? kernelBitSize / 8 : kernelBitSize / 8 + 1;
    /*align to 64 byte*/
    kernelSize = (kernelSize + 63) & 0xFFFFFFC0;
    kernelBitSize = 0;

    while(((numBestRuns-SET_1_SIZE)>>runSet2Bits) > 1)
    {
        runSet2Bits++;
    }

    kernelDataPtr = reorderStream;
    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        vx_int32 run = 0x0;
        vx_uint32 dataSizeOfCore = reorderStreamPerCoreCount[coreIndex];
        vx_uint32 dataRemainingOfCore = dataSizeOfCore;
        vx_int32 size = 0;
        vx_int32 code = 0;
        vx_int32 coef16 = 0;
        vx_int32 k = 0, j = 0, m = 0;
        vx_bool isCoef;
        vx_bool isLimitZRL;
        /* update position of each core's beginning of compressed kernel*/
        reorderStreamCheckCount += reorderStreamPerCoreCount[coreIndex];

        memset(codeSymbol, 0, sizeof(CodeSymbol));

        while(dataRemainingOfCore --)
        {
            if (weight_format == VX_TYPE_INT8)
                coef = *((vx_int8 *)kernelDataPtr);
            else if (weight_format == VX_TYPE_UINT8)
                coef = *((vx_uint8 *)kernelDataPtr);
            else if (weight_format == VX_TYPE_INT16 ||
                weight_format == VX_TYPE_FLOAT16)
                coef16 = *((vx_int16 *)kernelDataPtr);
            else
            {
                /*Other format not suppoted now*/
                vxmASSERT(0);
            }
            kernelDataPtr += weightSize;

            if (totalCountIdx == nonCoefIndex[nonCoefCountIdx] &&
                nonCoefCountIdx < nonCoefCount)
            {
                isCoef = vx_false_e;
                nonCoefCountIdx++;
            }
            else
                isCoef = vx_true_e;

            if (limitZRLIndex != VX_NULL &&
                totalCountIdx == limitZRLIndex[limitZRLCountIdx] &&
                limitZRLCountIdx < limitZRLCount)
            {
                isLimitZRL = vx_true_e;
                limitZRLCountIdx++;
            }
            else
                isLimitZRL = vx_false_e;

            totalCountIdx++;

            if (bit16Flag == 0)
            {

                coef = (coef - wb->huffmanConfig[index].avgBias) & 0xFF;
                if (wb->huffmanConfig[index].preEncode)
                {
                    tmp = coef;
                    coef = (coef - prev) & 0xFF;
                    prev = tmp;
                }
            }
            else
            {

                if (wb->huffmanConfig[index].fp16Flag)
                {
                    coef16 = (coef16 & 0x7FFF) * 2 + coef16 / (1<<15);
                }
                coef16 -= wb->huffmanConfig[index].avgBias;
                if (wb->huffmanConfig[index].runLenTableSize == 0x0)
                    coef = (coef16 >> 8) & 0xFF; /*Non RZL*/
                else
                    coef = coef16 & 0xFFFF; /*RZL path*/
            }

            if (wb->huffmanConfig[index].runLenTableSize == 0x0) /* non RZL*/
            {
                tmp = (coef & 0x80)? (coef ^ 0xff): coef;
                for (size = 0; size<7; size++)
                {
                    if (tmp<(1<<size))
                        break;
                }
                k = invSizeOrder[size];
                code = huffCode[k];

                if (sizeCodeLen[k]%2 == 0)/*huffCode length = 2 or 4, pack 1 bit to 3 or 5*/
                    code |= ((coef>>7)<<(sizeCodeLen[k]) );
                kernelBitSize += 3;

                if (sizeCodeLen[k] > 3)
                {
                    kernelBitSize += 2;
                }

                if (size == 0) size = 1; /*Size = 0 mean 0&0xff, needs 1 bit actually*/
                if (sizeCodeLen[k]%2 == 0)
                {
                    kernelBitSize += (size - 1);
                }
                else
                {
                    kernelBitSize += size;
                }

                if (wb->huffmanConfig[index].bit16Flag)
                {/*pack the raw data of the lower 8 bits*/
                    kernelBitSize += 8;
                }
            }
            else /* RZL enable*/
            {
                if (coef == 0 && isCoef && !isLimitZRL)
                {
                    run++;
                    if (dataRemainingOfCore != 0)
                    {
                        continue;
                    }
                }
                if (coef || (dataRemainingOfCore == 0) || !isCoef || isLimitZRL) /* if last pixel, force process run zeros*/
                {
                    /* process run zeros*/
                    while(run)
                    {
                        for (j = numBestRuns - 1; j>=0; j--)
                        {
                            if (run >= bestRunsSorted[j] + 1)
                            {
                                k = run / (bestRunsSorted[j] + 1);
                                run %= (bestRunsSorted[j] + 1);
                                break;
                            }
                        }
                        if (j >= 0)
                        {
                            vx_int32 n = bestRunsSorted[j];
                            size = 8;
                            for (m = 0; m<SET_1_SIZE; m++)
                            {
                                if (best2Runs[m] == n)
                                {
                                    size = 0;
                                    break;
                                }
                            }
                            if (size == 8)
                            {
                                for (m = 0; m<numBestRuns - SET_1_SIZE; m++)
                                    if (outBest2Run[m] == n)
                                        break;

                            }
                            j = invSizeOrder[size];
                            while(k--)
                            {/*k of the runs*/
                                kernelBitSize += 3;

                                if (sizeCodeLen[j] > 3)
                                {
                                    kernelBitSize += 2;
                                }
                                if (size == 0 )
                                {/*The m already code*/
                                    kernelBitSize += ((sizeCodeLen[j]%2 == 0) ? 0:1);
                                }
                                else if (sizeCodeLen[j]%2 == 0)
                                {
                                    kernelBitSize += (runSet2Bits - 1);
                                }
                                else
                                {
                                    kernelBitSize += runSet2Bits;
                                }
                            }
                        }
                    }
                    /*process current pixel*/
                    if (coef)
                    {
                        vx_int8 ch;
                        coef>>=(wb->huffmanConfig[index].bit16Flag * 8);
                        if(coef<0x80)
                            coef+=wb->huffmanConfig[index].bit16Flag; /*Since we have to encode zero (run-length only handle 16-bits zero, 8 bit zero should use Size code*/
                        ch = (vx_int8)coef;
                        coef = abs(ch);
                        for (j = 6; j>=0; j--)
                        {
                            if (coef>=(1<<j))
                            {
                                break;
                            }
                        }
                        size = j+1;
                        if (invSizeOrder[size]==8)
                        { /*The minimum freq, we out the raw data*/
                            size = 7;
                        }

                        j = invSizeOrder[size];
                        kernelBitSize += 3;

                        if (sizeCodeLen[j] > 3)
                        {
                            kernelBitSize += 2;
                        }

                        if (size == 7)
                        {
                            size = 8;
                        }
                        /*Size = 0 mean 0&0xff, needs 1 bit actually*/
                        if (size == 0)
                            size = 1;
                        if (sizeCodeLen[j]%2 == 0)
                        {
                            kernelBitSize += (size - 1);
                        }
                        else
                        {
                            kernelBitSize += size;
                        }
                        if (wb->huffmanConfig[index].bit16Flag)
                        {
                            kernelBitSize += 8;
                        }
                    }
                    else
                    {
                        /*those non-coef count like bias, num_of_vz & z_offset couldn't skip by zero-run as HW requres,
                        need also encode those 0 to huffman code,
                        in ZRL path 8-bit, 0 belong to group7, -128~127, residue bit length is 8, residue code = 0*/
                        size = 7;
                        j = invSizeOrder[size];
                        code = huffCode[j];

                        kernelBitSize += 3;

                        if (sizeCodeLen[j] > 3)
                        {
                            kernelBitSize += 2;
                        }

                        if (size == 7)
                        {
                            size = 8;
                        }

                        if (sizeCodeLen[j] % 2 == 0)
                        {
                            kernelBitSize += (size - 1);
                        }
                        else
                        {
                            kernelBitSize += size;
                        }
                        if (wb->huffmanConfig[index].bit16Flag)
                        {
                            kernelBitSize += 8;
                        }
                    }
                }
            }

            if(dataRemainingOfCore == 0)
            {
                /*Hit the end bitsteam of core, add dummy stage process the next*/
                kernelBitSize += 3;
                kernelBitSize += 2;
                kernelBitSize += 7;

                if (wb->huffmanConfig[index].bit16Flag)
                    kernelBitSize += 8;

                for (j = 0; j < 2*THROUGHPUT; j++)
                {
                    kernelBitSize += 3;
                }
                break;
            }
        }

        /* Go back to update kernelStreamSize. */
        kernelStreamSizePerCore = (kernelBitSize % 8 == 0) ? kernelBitSize / 8 : kernelBitSize / 8 + 1;
        /*align to 64 byte*/
        kernelStreamSizePerCore = (kernelStreamSizePerCore + 63) & 0xFFFFFFC0;
        kernelBitSize = 0;
        kernelSize += kernelStreamSizePerCore;
    }

exit:
    if (reorderStream)
        vxFree(reorderStream);
    if (reorderStreamPerCoreCount)
        vxFree(reorderStreamPerCoreCount);
    if (nonCoefIndex)
        vxFree(nonCoefIndex);
    if (limitZRLIndex)
        vxFree(limitZRLIndex);

    return kernelSize;
}

void fillinKernelBufferHuffman(
    vx_context context,
    vx_weights_biases_parameter wb,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_int32 input_zp,
    vx_int32 coef_zp,
    vx_uint32 skip_value,
    vx_uint32 output_final_x,
    vx_uint32 output_final_y,
    vx_int32 z_offset,
    vx_uint32 output_size,
    vx_uint8_ptr wb_base_ptr,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr,
    vx_uint32 index
    )
{

    vx_uint32 nnCoreCount = weight_format == VX_TYPE_INT16 ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : weight_format == VX_TYPE_FLOAT16 ?
                      context->nnConfig.fixedFeature.nnCoreCountFloat16 : context->nnConfig.fixedFeature.nnCoreCount;

    vx_uint32 invSizeOrder[MAX_SIZE_HISTO_COUNT] = {0};
    vx_int32 coef = 0, tmp = 0, prev = 0;

    vx_uint32 filterTotalCount   = z_count;
    vx_uint32 usedCoreCount      = (filterTotalCount > nnCoreCount) ? nnCoreCount : filterTotalCount;
    vx_uint32 filterCount        = filters_per_core; /* filter count every group */
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;
    vx_uint32 limitZRLCount      = groupCount * usedCoreCount;
    vx_uint32 *limitZRLIndex     = VX_NULL;

    vx_uint32 weightCount        = weight_x * weight_y;
    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize((vx_type_e)weight_format);
    vx_uint32 weightBitSize = weightSize * 8;
    vx_uint32 biasSize         = (vx_uint32)vxDataType_GetSize((vx_type_e)bias_format);
    vx_uint32 biasBitSize      = 0;

    vx_uint32 filterSize         = weightCount * weightSize * slice_count;
    vx_uint32* kernelBufferPtr   = (vx_uint32*)wb_base_ptr;

    vx_uint32* kernelStreamSizePtr = VX_NULL;
    vx_uint32 kernelStreamSize = ((filterSize * filterTotalCount + filterTotalCount * biasSize + filterTotalCount * 3 + 3) + 63) & ~63;
    vx_uint32 alignedOffset      = ((vx_uint32)(gctUINTPTR_T)kernelBufferPtr) & 0x3F;
    vx_uint8_ptr kernelDataPtr   = VX_NULL;
    vx_uint8_ptr kernelStreamBasePtr   = VX_NULL;
    vx_uint32 streamSizeBitOffset = 0;
    vx_uint32 maxKernelStreamSizePerCore = 0;

    vx_uint32 coreIndex;
    vx_uint32 i;
    vx_uint32 totalCountIdx = 0;
    vx_uint32 nonCoefCountIdx = 0;
    vx_uint32 limitZRLCountIdx = 0;

    vx_uint32 bit16Flag = (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8) ? 0 : 1;
    vx_uint32 bitOffset = 0;

    vx_uint32 reorderStreamSize = 0;
    vx_uint32 reorderStreamAllCount = 0;
    vx_uint32 reorderStreamCheckCount = 0;
    vx_uint32* reorderStreamPerCoreCount = VX_NULL;
    vx_uint32 nonCoefCount = 0;
    vx_uint32* nonCoefIndex = VX_NULL;

    vx_uint8* reorderStream = VX_NULL;
    vx_uint32 runSet2Bits = 1;
    CodeSymbol codeSymbol[THROUGHPUT * 3];

    vx_uint32 dummyStage[3] = {0};
    vx_uint32 dummyBitLength[3] = {0};
    vx_bool setDummy = vx_false_e;

    if (weight_format == VX_TYPE_INT16)
        biasBitSize = NN_INTEGER_BIAS_BITS_VIP_V7_INT16;
    else
        biasBitSize = NN_INTEGER_BIAS_BITS_VIP_V7;

    reorderStreamAllCount = (filterTotalCount * (biasBitSize / weightBitSize))
        + (weightCount * slice_count * filterTotalCount)
        + (filterTotalCount * (NN_Z_POSITION_OFFSET_BITS_VIP_V7 / weightBitSize))
        + (usedCoreCount * (16 / weightBitSize));/*filterPerCore need 16 bit * usedCoreCount */

    reorderStreamSize = reorderStreamAllCount * weightSize;

    reorderStream = (vx_uint8 *)vxAllocateAndZeroMemory(reorderStreamSize);

    if (!reorderStream)
    {
        vxError("fillinKernelBufferHuffman: OUT OF MEMORY");
        goto exit;
    }

    reorderStreamPerCoreCount = (vx_uint32 *)vxAllocateAndZeroMemory(nnCoreCount * sizeof(vx_uint32));
    if (!reorderStreamPerCoreCount)
    {
        vxError("fillinKernelBufferHuffman: OUT OF MEMORY");
        goto exit;
    }

    nonCoefCount = (filterTotalCount * (biasBitSize / weightBitSize))
        + (filterTotalCount * (NN_Z_POSITION_OFFSET_BITS_VIP_V7 / weightBitSize))
        + (nnCoreCount * (16 / weightBitSize)); /*Those non coef count contain bias, Z_OFF_SET & kernels_per_core*/
    nonCoefIndex = (vx_uint32 *)vxAllocateAndZeroMemory(nonCoefCount * sizeof(vx_uint32));
    if (!nonCoefIndex)
    {
        vxError("fillinKernelBufferHuffman: OUT OF MEMORY");
        goto exit;
    }

    limitZRLIndex = (vx_uint32 *)vxAllocateAndZeroMemory(limitZRLCount * sizeof(vx_uint32));
    if (!limitZRLIndex)
    {
        vxError("fillinKernelBufferHuffman: OUT OF MEMORY");
        goto exit;
    }

    if (context->options.enableNonZeroBalance)
    {
        reorderKernelBufferV7HuffmanBalance(context, wb, slice_count, z_count, filters_per_core, weight_format, bias_format, z_offset,
            output_size, skip_value, output_final_x, output_final_y, reorderStream, weight_base_ptr, bias_base_ptr, VX_NULL, VX_NULL, reorderStreamPerCoreCount, nonCoefIndex, limitZRLIndex, vx_false_e);
    }
    else
    {
        reorderWeightBiasBufferForHuffman(context, wb, slice_count, z_count, filters_per_core, weight_format, bias_format, z_offset,
            output_size, skip_value, output_final_x, output_final_y, reorderStream, weight_base_ptr, bias_base_ptr, VX_NULL, VX_NULL, reorderStreamPerCoreCount, nonCoefIndex, limitZRLIndex, vx_false_e);
    }

    analysisKernelStreamForHuffman(context, wb, 0, nnCoreCount, reorderStream, reorderStreamSize, reorderStreamPerCoreCount, invSizeOrder, nonCoefIndex, nonCoefCount, limitZRLIndex, limitZRLCount, index);

    /* Write huffman header*/
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].preEncode, 1);
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].bit16Flag, 1);
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].fp16Flag, 1);
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].reserved, 1);
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].version, 4);
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].runLenTableSize, 8);

    for(i = 0; i < MAX_RUNLEN_SIZE; i++)
    {
        writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].runLenTable[i], 8);
    }
    for(i = 0; i < LOG_RUN_SIZE1; i++)
    {
        writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].mapToHuffman[i], 8);
    }
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].avgBias, 16);
    writeBits(&kernelBufferPtr, &bitOffset, 0, 16); /*reserved, must zero*/

    kernelStreamSizePtr = kernelBufferPtr;
    streamSizeBitOffset = bitOffset;
    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        writeBits(&kernelBufferPtr, &bitOffset, kernelStreamSize, 32);
    }
    /*align to 64 byte*/
    packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);
    wb->slice_array[index].kernel_stream_full_cache_size = 0;

    for (i = 0; i < THROUGHPUT * 3; i++)
    {
        codeSymbol[i].stageCode[0] = codeSymbol[i].stageCode[1] = codeSymbol[i].stageCode[2] = 0;
        codeSymbol[i].bitLength[0] = codeSymbol[i].bitLength[1] = codeSymbol[i].bitLength[2] = 0;
    }

    while(((numBestRuns-SET_1_SIZE)>>runSet2Bits) > 1)
    {
        runSet2Bits++;
    }

    kernelDataPtr = reorderStream;
    wb->max_per_core_compression_ratio = 0;

    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {

        vx_uint32 x = 0x0;
        vx_int32 pos = 0x0;
        vx_int32 run = 0x0;
        vx_uint32 dataSizeOfCore = reorderStreamPerCoreCount[coreIndex];
        vx_uint32 dataRemainingOfCore = dataSizeOfCore;
        vx_int32 size = 0;
        vx_int32 code = 0;
        vx_int32 coef16 = 0;
        vx_int32 k = 0, j = 0, m = 0;
        vx_bool isCoef;
        vx_bool isLimitZRL;
        vx_float64 compressionRatio = 0;
        /* update position of each core's beginning of compressed kernel*/
        kernelStreamBasePtr = (vx_uint8*)kernelBufferPtr;
        reorderStreamCheckCount += reorderStreamPerCoreCount[coreIndex];

        memset(codeSymbol, 0, sizeof(CodeSymbol));
        memset(dummyStage, 0, (sizeof(vx_uint32) * 3));
        memset(dummyBitLength, 0, (sizeof(vx_uint32) * 3));
        setDummy = vx_false_e;

        while(dataRemainingOfCore --)
        {

            pos = (x) % (3*THROUGHPUT);

            if (weight_format == VX_TYPE_INT8)
                coef = *((vx_int8 *)kernelDataPtr);
            else if (weight_format == VX_TYPE_UINT8)
                coef = *((vx_uint8 *)kernelDataPtr);
            else if (weight_format == VX_TYPE_INT16 ||
                weight_format == VX_TYPE_FLOAT16)
                coef16 = *((vx_int16 *)kernelDataPtr);
            else
            {
                /*Other format not suppoted now*/
                vxmASSERT(0);
            }
            kernelDataPtr += weightSize;

            if (totalCountIdx == nonCoefIndex[nonCoefCountIdx] &&
                nonCoefCountIdx < nonCoefCount)
            {
                isCoef = vx_false_e;
                nonCoefCountIdx++;
            }
            else
                isCoef = vx_true_e;

            if (limitZRLIndex != VX_NULL &&
                totalCountIdx == limitZRLIndex[limitZRLCountIdx] &&
                limitZRLCountIdx < limitZRLCount)
            {
                isLimitZRL = vx_true_e;
                limitZRLCountIdx++;
            }
            else
                isLimitZRL = vx_false_e;

            totalCountIdx++;

            if (bit16Flag == 0)
            {

                coef = (coef - wb->huffmanConfig[index].avgBias) & 0xFF;
                if (wb->huffmanConfig[index].preEncode)
                {
                    tmp = coef;
                    coef = (coef - prev) & 0xFF;
                    prev = tmp;
                }
            }
            else
            {

                if (wb->huffmanConfig[index].fp16Flag)
                {
                    coef16 = (coef16 & 0x7FFF) * 2 + coef16 / (1<<15);
                }
                coef16 -= wb->huffmanConfig[index].avgBias;
                if (wb->huffmanConfig[index].runLenTableSize == 0x0)
                    coef = (coef16 >> 8) & 0xFF; /*Non RZL*/
                else
                    coef = coef16 & 0xFFFF; /*RZL path*/
            }

            if (wb->huffmanConfig[index].runLenTableSize == 0x0) /* non RZL*/
            {
                if (!setDummy)
                {
                    /*Set dummy all to 0,
                    in nonZRL path 8-bit, 0 belong to group0, -1~0, residue bit length is 1, residue code = 0*/
                    size = 0;
                    k = invSizeOrder[size];
                    code = huffCode[k];

                    dummyStage[0] = code & 0x07;
                    dummyBitLength[0] = 3;

                    if (sizeCodeLen[k] > 3)
                    {
                        dummyStage[1] = code >> 3;
                        dummyBitLength[1] = 2;
                    }

                    dummyStage[2] = 0;
                    if (size == 0) size = 1; /*Size = 0 mean 0&0xff, needs 1 bit actually*/
                    if (sizeCodeLen[k]%2 == 0)
                    {
                        dummyBitLength[2] = size - 1;
                    }
                    else
                    {
                        dummyBitLength[2] = size;
                    }

                    if (wb->huffmanConfig[index].bit16Flag)
                    {/*pack the raw data of the lower 8 bits*/

                        dummyBitLength[2] += 8;
                    }
                    setDummy = vx_true_e;
                }

                tmp = (coef & 0x80)? (coef ^ 0xff): coef;
                for (size = 0; size<7; size++)
                {
                    if (tmp<(1<<size))
                        break;
                }
                k = invSizeOrder[size];
                code = huffCode[k];

                if (sizeCodeLen[k]%2 == 0)/*huffCode length = 2 or 4, pack 1 bit to 3 or 5*/
                    code |= ((coef>>7)<<(sizeCodeLen[k]) );
                codeSymbol[pos].stageCode[0] = code & 0x07;
                codeSymbol[pos].bitLength[0] = 3;

                if (sizeCodeLen[k] > 3)
                {
                    codeSymbol[pos].stageCode[1] = code >> 3;
                    codeSymbol[pos].bitLength[1] = 2;
                }

                codeSymbol[pos].stageCode[2] = tmp &((1<<size) - 1 );
                if (size == 0) size = 1; /*Size = 0 mean 0&0xff, needs 1 bit actually*/
                if (sizeCodeLen[k]%2 == 0)
                {
                    codeSymbol[pos].bitLength[2] = size - 1;
                }
                else
                {
                    codeSymbol[pos].stageCode[2] <<= 1;
                    codeSymbol[pos].stageCode[2] |= (coef>>7); /*The sign bit*/
                    codeSymbol[pos].bitLength[2] = size;
                }

                if (wb->huffmanConfig[index].bit16Flag)
                {/*pack the raw data of the lower 8 bits*/

                    codeSymbol[pos].stageCode[2] = codeSymbol[x%(3*THROUGHPUT)].stageCode[2]* 256 + (coef16 &0xff);
                    codeSymbol[pos].bitLength[2] += 8;
                }
                OutputAt(x, &kernelBufferPtr, &bitOffset, codeSymbol);
                x++; /*counter*/
            }
            else /* RZL enable*/
            {
                if (!setDummy)
                {
                    /*Set dummy all to 0,
                    in ZRL path 8-bit, 0 belong to group7, -128~127, residue bit length is 8, residue code = 0*/
                    size = 7;
                    j = invSizeOrder[size];
                    code = huffCode[j];

                    dummyStage[0] = code & 0x07;
                    dummyBitLength[0] = 3;

                    if (sizeCodeLen[j] > 3)
                    {
                        dummyStage[1] = code >> 3;
                        dummyBitLength[1] = 2;
                    }

                    if (size == 7)
                    {
                        size = 8;
                    }

                    if (sizeCodeLen[j] % 2 == 0)
                    {
                        dummyStage[2] = 0;  /*last bit was coded*/
                        dummyBitLength[2] = size - 1;
                    }
                    else
                    {
                        dummyStage[2] = 0;
                        dummyBitLength[2] = size;
                    }
                    if (wb->huffmanConfig[index].bit16Flag)
                    {
                        dummyStage[2] = 0;
                        dummyBitLength[2] += 8;
                    }

                    setDummy = vx_true_e;
                }

                if (coef == 0 && isCoef && !isLimitZRL)
                {
                    run++;
                    if (dataRemainingOfCore != 0)
                    {
                        continue;
                    }
                }
                if (coef || (dataRemainingOfCore == 0) || !isCoef || isLimitZRL) /* if last pixel, force process run zeros*/
                {
                    /* process run zeros*/
                    while(run)
                    {
                        for (j = numBestRuns - 1; j>=0; j--)
                        {
                            if (run >= bestRunsSorted[j] + 1)
                            {
                                k = run / (bestRunsSorted[j] + 1);
                                run %= (bestRunsSorted[j] + 1);
                                break;
                            }
                        }
                        if (j >= 0)
                        {
                            vx_int32 n = bestRunsSorted[j];
                            size = 8;
                            for (m = 0; m<SET_1_SIZE; m++)
                            {
                                if (best2Runs[m] == n)
                                {
                                    size = 0;
                                    break;
                                }
                            }
                            if (size == 8)
                            {
                                for (m = 0; m<numBestRuns - SET_1_SIZE; m++)
                                    if (outBest2Run[m] == n)
                                        break;

                            }
                            j = invSizeOrder[size];
                            while(k--){/*k of the runs*/
                                code = huffCode[j];
                                if (sizeCodeLen[j] % 2 == 0)/*huffCode length = 2 or 4, pack 1 bit to 3 or 5*/
                                    code |= ((m&1)<<(sizeCodeLen[j]) );
                                codeSymbol[pos].stageCode[0] = code & 0x07;
                                codeSymbol[pos].bitLength[0] = 3;

                                if (sizeCodeLen[j] > 3)
                                {
                                    codeSymbol[pos].stageCode[1] = code >> 3;
                                    codeSymbol[pos].bitLength[1] = 2;
                                }
                                if (size == 0 )
                                {/*The m already code*/
                                    codeSymbol[pos].bitLength[2] = (sizeCodeLen[j]%2 == 0) ? 0:1;
                                    codeSymbol[pos].stageCode[2] = m; /*If (sizeCodeLen[j]%2 == 0), should be m/2, but 0 bit output, doesn't matter*/
                                }
                                else if (sizeCodeLen[j]%2 == 0)
                                {
                                    codeSymbol[pos].stageCode[2] = m/2;
                                    codeSymbol[pos].bitLength[2] = runSet2Bits - 1;
                                }
                                else
                                {
                                    codeSymbol[pos].stageCode[2] = m;
                                    codeSymbol[pos].bitLength[2] = runSet2Bits;
                                }

                                OutputAt(x, &kernelBufferPtr, &bitOffset, codeSymbol);
                                x++;
                                /* update pos*/
                                pos = (x) % (3*THROUGHPUT);
                            }
                        }
                    }
                    /*process current pixel*/
                    if (coef)
                    {
                        vx_int8 ch;
                        coef>>=(wb->huffmanConfig[index].bit16Flag * 8);
                        if(coef<0x80)
                            coef+=wb->huffmanConfig[index].bit16Flag; /*Since we have to encode zero (run-length only handle 16-bits zero, 8 bit zero should use Size code*/
                        ch = (vx_int8)coef;
                        coef = abs(ch);
                        m = coef*2 + (ch<0?1:0);
                        for (j = 6; j>=0; j--)
                        {
                            if (coef>=(1<<j))
                            {
                                break;
                            }
                        }
                        size = j+1;
                        if (invSizeOrder[size]==8)
                        { /*The minimum freq, we out the raw data*/
                            size = 7;
                        }
                        if (size == 7)
                        {
                            m = ch&0xff;
                            if(wb->huffmanConfig[index].bit16Flag)
                                m = (coef16 >> 8) & 0xff;
                        }

                        j = invSizeOrder[size];
                        code = huffCode[j];
                        if (sizeCodeLen[j]%2 == 0)/*huffCode length = 2 or 4, pack 1 bit to 3 or 5*/
                            code |= ((m&1) << (sizeCodeLen[j]));
                        codeSymbol[pos].stageCode[0] = code & 0x07;
                        codeSymbol[pos].bitLength[0] = 3;

                        if (sizeCodeLen[j] > 3)
                        {
                            codeSymbol[pos].stageCode[1] = code >> 3;
                            codeSymbol[pos].bitLength[1] = 2;
                        }

                        if (size == 7)
                        {
                            size = 8;
                        }
                        /*Size = 0 mean 0&0xff, needs 1 bit actually*/
                        if (size == 0)
                            size = 1;
                        if (sizeCodeLen[j]%2 == 0)
                        {
                            codeSymbol[pos].stageCode[2] = m/2;  /*last bit was coded*/
                            codeSymbol[pos].bitLength[2] = size - 1;
                        }
                        else
                        {
                            codeSymbol[pos].stageCode[2] = m;
                            codeSymbol[pos].bitLength[2] = size;
                        }
                        if (wb->huffmanConfig[index].bit16Flag)
                        {
                            codeSymbol[pos].stageCode[2] = codeSymbol[x%(3*THROUGHPUT)].stageCode[2]* 256 + (coef16 & 0xff);
                            codeSymbol[pos].bitLength[2] += 8;
                        }
                        OutputAt(x, &kernelBufferPtr, &bitOffset, codeSymbol);
                        x++; /*counter*/
                    }
                    else
                    {
                        /*those non-coef count like bias, num_of_vz & z_offset couldn't skip by zero-run as HW requres,
                        need also encode those 0 to huffman code,
                        in ZRL path 8-bit, 0 belong to group7, -128~127, residue bit length is 8, residue code = 0*/
                        size = 7;
                        j = invSizeOrder[size];
                        code = huffCode[j];

                        codeSymbol[pos].stageCode[0] = code & 0x07;
                        codeSymbol[pos].bitLength[0] = 3;

                        if (sizeCodeLen[j] > 3)
                        {
                            codeSymbol[pos].stageCode[1] = code >> 3;
                            codeSymbol[pos].bitLength[1] = 2;
                        }

                        if (size == 7)
                        {
                            size = 8;
                        }

                        if (sizeCodeLen[j] % 2 == 0)
                        {
                            codeSymbol[pos].stageCode[2] = 0;  /*last bit was coded*/
                            codeSymbol[pos].bitLength[2] = size - 1;
                        }
                        else
                        {
                            codeSymbol[pos].stageCode[2] = 0;
                            codeSymbol[pos].bitLength[2] = size;
                        }
                        if (wb->huffmanConfig[index].bit16Flag)
                        {
                            codeSymbol[pos].stageCode[2] = codeSymbol[x%(3*THROUGHPUT)].stageCode[2]* 256 + (coef16 & 0xff);
                            codeSymbol[pos].bitLength[2] += 8;
                        }
                        OutputAt(x, &kernelBufferPtr, &bitOffset, codeSymbol);
                        x++; /*counter*/
                    }
                }
            }

            if(dataRemainingOfCore == 0)
            {
                vx_uint32 dummyCount = (x & 0x1)? 5: 4;
                /*Hit the end bitsteam of core, add dummy stage process the next*/
                vxmASSERT(setDummy == vx_true_e);
                addDummy(x, &kernelBufferPtr, &bitOffset, codeSymbol, dummyCount, dummyStage, dummyBitLength);
                break;
            }
        }

        /* Go back to update kernelStreamSize. */
        kernelStreamSize = (vx_uint32)((vx_uint8 *)kernelBufferPtr - kernelStreamBasePtr);
        writeBits(&kernelStreamSizePtr, &streamSizeBitOffset, kernelStreamSize*8 + bitOffset, 32);

        /* pad 0 at the end of core*/
        packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);
        kernelStreamSize = (vx_uint32)((vx_uint8 *)kernelBufferPtr - kernelStreamBasePtr);

        /*With RTL bug fix, full cache mode size needn't using maxSize * coreCount*/
        wb->slice_array[index].kernel_stream_full_cache_size += kernelStreamSize;

        compressionRatio = (vx_float64)kernelStreamSize / (vx_float64)(dataSizeOfCore * weightSize);
        if (wb->max_per_core_compression_ratio < compressionRatio)
            wb->max_per_core_compression_ratio = compressionRatio;

        if (maxKernelStreamSizePerCore < kernelStreamSize)
            maxKernelStreamSizePerCore = kernelStreamSize;

        prev = 0;
    }
    vxmASSERT(reorderStreamAllCount == reorderStreamCheckCount); /*Check if the data count is the same*/
    wb->slice_array[index].kernel_align_stream_size += (vx_size)(maxKernelStreamSizePerCore * usedCoreCount);
exit:
    if (reorderStream)
        vxFree(reorderStream);
    if (reorderStreamPerCoreCount)
        vxFree(reorderStreamPerCoreCount);
    if (nonCoefIndex)
        vxFree(nonCoefIndex);
    if (limitZRLIndex)
        vxFree(limitZRLIndex);

    return;
}

vx_uint32 calcNonZeroCountV8Huffman(
    vx_context context,
    vx_weights_biases_parameter wb,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_enum  weight_format,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32 skip_value
    )
{
    vx_uint32 nnCoreCount = (weight_format == VX_TYPE_INT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : (weight_format == VX_TYPE_FLOAT16) ?
                      context->nnConfig.fixedFeature.nnCoreCountFloat16 : context->nnConfig.fixedFeature.nnCoreCount;
    vx_uint32 filterTotalCount   = z_count;
    vx_uint32 totalFilterPerCore = filterTotalCount / nnCoreCount;
    vx_uint32 oddFilterPerCore   = filterTotalCount % nnCoreCount;
    vx_uint32 filterCount        = filters_per_core; /* filter count every group */
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;

    vx_uint32 weightCount        = wb->weights_sizes[0] * wb->weights_sizes[1];
    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize((vx_type_e)weight_format);

    vx_uint32 filterSliceSize    = weightCount * weightSize;
    vx_uint32 filterSize         = weightCount * weightSize * slice_count;

    vx_uint32 coreIndex;
    vx_uint32 filterIndex;
    vx_uint32 k, sk;
    vx_uint32 kSize;
    vx_uint32 weightXIndex, weightYIndex, weightZIndex;

    vx_bool hasZDP3 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3);
    vx_uint32 dpAmount = hasZDP3 ? 3 : 1;
    vx_uint32 zDPLoopCount = 3;
    vx_uint32 linesInImageBuffer = dpAmount * zDPLoopCount;

    vx_uint32 nonZeroCount = 0;

    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        vx_uint32 coreFilterCount = (coreIndex < oddFilterPerCore) ? totalFilterPerCore + 1 : totalFilterPerCore;
        vx_uint32 groupIndex;
        vx_uint8_ptr realKernelDataPtr = VX_NULL;

        for (groupIndex = 0; groupIndex < groupCount; groupIndex++)
        {
            vx_uint32 actualFilterCount = (groupIndex == groupCount - 1) ? (coreFilterCount - groupIndex * filterCount) : filterCount;
            vx_uint32 filterStart, filterEnd;

            if ((groupIndex == groupCount - 1)
                && (coreIndex >= (filterTotalCount % nnCoreCount)))
            {
                filterStart = groupIndex * nnCoreCount * filterCount
                            + (filterTotalCount % nnCoreCount) * (actualFilterCount + 1)
                            + (coreIndex - (filterTotalCount % nnCoreCount)) * actualFilterCount;
            }
            else
            {
                filterStart = groupIndex * nnCoreCount *filterCount + coreIndex * actualFilterCount;
            }
            filterEnd = filterStart + actualFilterCount - 1;

            for (k = 0; k < weightCount * slice_count; k += linesInImageBuffer)
            {
                kSize = gcmMIN((weightCount * slice_count - k), linesInImageBuffer);

                for (filterIndex = filterStart; filterIndex <= filterEnd; filterIndex++)
                {
                    vx_uint32 coef[9] = {skip_value};
                    vx_bool zeroBlock0 = vx_false_e;
                    vx_bool zeroBlock1 = vx_false_e;
                    vx_bool zeroBlock2 = vx_false_e;

                    for (sk = 0; sk < kSize; sk++)
                    {
                        weightZIndex = (k + sk) / weightCount;
                        weightYIndex = (k + sk) % weightCount;
                        weightYIndex /= wb->weights_sizes[0];
                        weightXIndex = (k + sk) % wb->weights_sizes[0];

                        realKernelDataPtr = weight_base_ptr + filterIndex * filterSize +
                            filterSliceSize * weightZIndex +
                            (weightYIndex * wb->weights_sizes[0] + weightXIndex) * weightSize;

                        if (weight_format == VX_TYPE_INT8)
                            coef[sk] = *((vx_int8 *)realKernelDataPtr);
                        else if (weight_format == VX_TYPE_UINT8)
                            coef[sk] = *((vx_uint8 *)realKernelDataPtr);
                        else
                            coef[sk] = *((vx_uint16 *)realKernelDataPtr);

                    }

                    if (coef[0] == skip_value &&
                        coef[1] == skip_value &&
                        coef[2] == skip_value)
                        zeroBlock0 = vx_true_e;

                    if (coef[3] == skip_value &&
                        coef[4] == skip_value &&
                        coef[5] == skip_value)
                        zeroBlock1 = vx_true_e;

                    if (coef[6] == skip_value &&
                        coef[7] == skip_value &&
                        coef[8] == skip_value)
                        zeroBlock2 = vx_true_e;

                    if (zeroBlock0 && zeroBlock1 && zeroBlock2)
                        nonZeroCount += 0;
                    else if (zeroBlock0 || zeroBlock1 || zeroBlock2)
                        nonZeroCount += 6;
                    else
                        nonZeroCount += 9;
                }
            }
        }
    }
    return nonZeroCount;
}

void reorderDepthWiseKernelBufferV8Huffman(
    vx_context context,
    vx_weights_biases_parameter wb,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_uint32 skip_value,
    vx_enum  weight_format,
    vx_uint8_ptr reorder_stream,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32 nn_core_count,
    vx_uint32* reoder_stream_count_per_core,
    vx_uint32* non_coef_index
    )
{
    vx_uint32 nnCoreCount = nn_core_count;
    vx_uint32 filterTotalCount   = z_count;
    vx_uint32 totalFilterPerCore = filterTotalCount / nnCoreCount;
    vx_uint32 oddFilterPerCore   = filterTotalCount % nnCoreCount;
    vx_uint32 filterCount        = filters_per_core; /* filter count every group */
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;

    vx_uint32 weightCount        = wb->weights_sizes[0] * wb->weights_sizes[1];
    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize((vx_type_e)weight_format);
    vx_uint32 weightBitSize      = weightSize * 8;

    vx_uint32 filterSliceSize    = weightCount * weightSize;
    vx_uint32 filterSize         = weightCount * weightSize * slice_count;
    vx_uint8* kernelBufferInt8Ptr      = reorder_stream;
    vx_uint16* kernelBufferInt16Ptr    = (vx_uint16*)reorder_stream;

    vx_uint32 coreIndex;
    vx_uint32 filterIndex;
    vx_uint32 core0FilterCount = 0;
    vx_uint32 k, sk;
    vx_uint32 kSize;
    vx_uint32 weightXIndex, weightYIndex, weightZIndex;

    vx_bool hasZDP3 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3);
    vx_uint32 dpAmount = hasZDP3 ? 3 : 1;
    vx_uint32 zDPLoopCount = 3;
    vx_uint32 linesInImageBuffer = dpAmount * zDPLoopCount;

    vx_uint32 nonCoefIndex = 0, idx = 0, elementIndex = 0;

    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        vx_uint32 coreFilterCount = (coreIndex < oddFilterPerCore) ? totalFilterPerCore + 1 : totalFilterPerCore;
        vx_uint32 groupIndex;
        vx_uint32 coef = 0;
        vx_uint8_ptr realKernelDataPtr = VX_NULL;
        reoder_stream_count_per_core[coreIndex] = 0;

        if (coreFilterCount > 0)
        {
            /* Save kernels per core*/
            if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
            {
                *((vx_uint16 *)kernelBufferInt8Ptr) = (vx_uint16)coreFilterCount;
                kernelBufferInt8Ptr += 2;
            }
            else
            {
                *kernelBufferInt16Ptr = (vx_uint16)coreFilterCount;
                kernelBufferInt16Ptr ++;
            }
            reoder_stream_count_per_core[coreIndex] += (16 / weightBitSize);

            for (idx = 0; idx < (16 / weightBitSize); idx++)
            {
                non_coef_index[nonCoefIndex++] = elementIndex++;
            }
        }

        for (groupIndex = 0; groupIndex < groupCount; groupIndex++)
        {
            vx_uint32 actualFilterCount = (groupIndex == groupCount - 1) ? (coreFilterCount - groupIndex * filterCount) : filterCount;
            vx_uint32 groupFilterStart = groupIndex * nnCoreCount * filterCount;
            vx_uint32 adjFilterStart = groupFilterStart + nnCoreCount - coreIndex - 1;
            vx_uint32 kid = 0;
            vx_uint32 unusedCoreCount = (filterTotalCount % nnCoreCount == 0) ? 0 : nnCoreCount - (filterTotalCount % nnCoreCount);

            if (coreIndex == 0)
                core0FilterCount = actualFilterCount;

            for (kid = 0; kid < actualFilterCount; kid++)
            {
                if (groupIndex == groupCount - 1 &&
                    kid == core0FilterCount - 1)
                    filterIndex = adjFilterStart + kid * nnCoreCount - unusedCoreCount;
                else
                    filterIndex = adjFilterStart + kid * nnCoreCount;

                for (k = 0; k < weightCount * slice_count; k += linesInImageBuffer)
                {
                    kSize = gcmMIN((weightCount * slice_count - k), linesInImageBuffer);

                    for (sk = 0; sk < linesInImageBuffer; sk++)
                    {
                        weightZIndex = (k + sk) / weightCount;
                        weightYIndex = (k + sk) % weightCount;
                        weightYIndex /= wb->weights_sizes[0];
                        weightXIndex = (k + sk) % wb->weights_sizes[0];
                        /*Packl 0 in the last dp group*/
                        if (sk >= kSize)
                            coef = skip_value;
                        else
                        {
                            realKernelDataPtr = weight_base_ptr + filterIndex * filterSize +
                                filterSliceSize * weightZIndex +
                                (weightYIndex * wb->weights_sizes[0] + weightXIndex) * weightSize;

                            if (weight_format == VX_TYPE_INT8)
                                coef = *((vx_int8 *)realKernelDataPtr);
                            else if (weight_format == VX_TYPE_UINT8)
                                coef = *((vx_uint8 *)realKernelDataPtr);
                            else
                                coef = *((vx_uint16 *)realKernelDataPtr);
                        }
                        if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
                        {
                            vx_int8 newCoef = ((vx_int32)coef - (vx_int32)skip_value) & 0xFF;
                            *kernelBufferInt8Ptr = (vx_uint8)newCoef;
                            kernelBufferInt8Ptr++;
                        }
                        else
                        {
                            if (weight_format == VX_TYPE_FLOAT16)
                                coef = (coef & 0x7fff) * 2 + coef/(1<<15);

                            *kernelBufferInt16Ptr = (vx_uint16)coef;
                            kernelBufferInt16Ptr++;
                        }
                        reoder_stream_count_per_core[coreIndex]++;
                        elementIndex++;
                    }
                }
            }
        }
    }
}

void reorderKernelBufferV8Huffman(
    vx_context context,
    vx_weights_biases_parameter wb,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_uint32 skip_value,
    vx_enum  weight_format,
    vx_uint8_ptr reorder_stream,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32 nn_core_count,
    vx_uint32* reoder_stream_count_per_core,
    vx_uint32* non_coef_index,
    vx_uint32* last_vz_group_index
    )
{
    vx_uint32 nnCoreCount = nn_core_count;
    vx_uint32 filterTotalCount   = z_count;
    vx_uint32 totalFilterPerCore = filterTotalCount / nnCoreCount;
    vx_uint32 oddFilterPerCore   = filterTotalCount % nnCoreCount;
    vx_uint32 filterCount        = filters_per_core; /* filter count every group */
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;

    vx_uint32 weightCount        = wb->weights_sizes[0] * wb->weights_sizes[1];
    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize((vx_type_e)weight_format);
    vx_uint32 weightBitSize      = weightSize * 8;

    vx_uint32 filterSliceSize    = weightCount * weightSize;
    vx_uint32 filterSize         = weightCount * weightSize * slice_count;
    vx_uint8* kernelBufferInt8Ptr    = reorder_stream;
    vx_uint16* kernelBufferInt16Ptr    = (vx_uint16*)reorder_stream;

    vx_uint32 coreIndex;
    vx_uint32 filterIndex;
    vx_uint32 k, sk;
    vx_uint32 kSize;
    vx_uint32 weightXIndex, weightYIndex, weightZIndex;

    vx_bool hasZDP3 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3);
    vx_uint32 dpAmount = hasZDP3 ? 3 : 1;
    vx_uint32 zDPLoopCount = 3;
    vx_uint32 linesInImageBuffer = dpAmount * zDPLoopCount;

    vx_uint32 nonCoefIndex = 0, idx = 0, elementIndex = 0;

    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        vx_uint32 coreFilterCount = (coreIndex < oddFilterPerCore) ? totalFilterPerCore + 1 : totalFilterPerCore;
        vx_uint32 groupIndex;
        vx_uint32 coef = 0;
        vx_uint8_ptr realKernelDataPtr = VX_NULL;
        reoder_stream_count_per_core[coreIndex] = 0;

        if (coreFilterCount > 0)
        {
            /* Save kernels per core*/
            if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
            {
                *((vx_uint16 *)kernelBufferInt8Ptr) = (vx_uint16)coreFilterCount;
                kernelBufferInt8Ptr += 2;
            }
            else
            {
                *kernelBufferInt16Ptr = (vx_uint16)coreFilterCount;
                kernelBufferInt16Ptr ++;
            }
            reoder_stream_count_per_core[coreIndex] += (16 / weightBitSize);

            for (idx = 0; idx < (16 / weightBitSize); idx++)
            {
                non_coef_index[nonCoefIndex++] = elementIndex++;
            }
        }

        for (groupIndex = 0; groupIndex < groupCount; groupIndex++)
        {
            vx_uint32 actualFilterCount = (groupIndex == groupCount - 1) ? (coreFilterCount - groupIndex * filterCount) : filterCount;
            vx_uint32 filterStart, filterEnd;

            if ((groupIndex == groupCount - 1)
                && (coreIndex >= (filterTotalCount % nnCoreCount)))
            {
                filterStart = groupIndex * nnCoreCount * filterCount
                            + (filterTotalCount % nnCoreCount) * (actualFilterCount + 1)
                            + (coreIndex - (filterTotalCount % nnCoreCount)) * actualFilterCount;
            }
            else
            {
                filterStart = groupIndex * nnCoreCount *filterCount + coreIndex * actualFilterCount;
            }
            filterEnd = filterStart + actualFilterCount - 1;

            if (actualFilterCount > 0)
            {
                if (groupIndex == groupCount - 1)
                    last_vz_group_index[coreIndex] = elementIndex;
            }

            for (k = 0; k < weightCount * slice_count; k += linesInImageBuffer)
            {
                kSize = gcmMIN((weightCount * slice_count - k), linesInImageBuffer);

                for (filterIndex = filterStart; filterIndex <= filterEnd; filterIndex++)
                {
                    for (sk = 0; sk < linesInImageBuffer; sk++)
                    {
                        weightZIndex = (k + sk) / weightCount;
                        weightYIndex = (k + sk) % weightCount;
                        weightYIndex /= wb->weights_sizes[0];
                        weightXIndex = (k + sk) % wb->weights_sizes[0];
                        /*Packl 0 in the last dp group*/
                        if (sk >= kSize)
                            coef = skip_value;
                        else
                        {
                            realKernelDataPtr = weight_base_ptr + filterIndex * filterSize +
                                filterSliceSize * weightZIndex +
                                (weightYIndex * wb->weights_sizes[0] + weightXIndex) * weightSize;

                            if (weight_format == VX_TYPE_INT8)
                                coef = *((vx_int8 *)realKernelDataPtr);
                            else if (weight_format == VX_TYPE_UINT8)
                                coef = *((vx_uint8 *)realKernelDataPtr);
                            else
                                coef = *((vx_uint16 *)realKernelDataPtr);
                        }

                        if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
                        {
                            vx_int8 newCoef = ((vx_int32)coef - (vx_int32)skip_value) & 0xFF;
                            *kernelBufferInt8Ptr = (vx_uint8)newCoef;
                            kernelBufferInt8Ptr++;
                        }
                        else
                        {
                            if (weight_format == VX_TYPE_FLOAT16)
                                coef = (coef & 0x7fff) * 2 + coef/(1<<15);

                            *kernelBufferInt16Ptr = (vx_uint16)coef;
                            kernelBufferInt16Ptr++;
                        }
                        reoder_stream_count_per_core[coreIndex]++;
                        elementIndex++;
                    }
                }
            }
        }
    }
}

void reorderKernelBufferV8HuffmanBalance(
    vx_context context,
    vx_weights_biases_parameter wb,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_uint32 skip_value,
    vx_enum  weight_format,
    vx_uint8_ptr reorder_stream,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32 nn_core_count,
    vx_uint32* reoder_stream_count_per_core,
    vx_uint32* non_coef_index,
    vx_uint32* real_vz_index,
    vx_uint32* last_vz_group_index
    )
{
    vx_uint32 nnCoreCount = nn_core_count;
    vx_uint32 filterTotalCount   = z_count;
    vx_uint32 sliceCount         = slice_count;
    vx_uint32 totalFilterPerCore = filterTotalCount / nnCoreCount;
    vx_uint32 oddFilterPerCore   = filterTotalCount % nnCoreCount;
    vx_uint32 filterCount        = filters_per_core; /* filter count every group */
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;

    vx_uint32 weightCount        = wb->weights_sizes[0] * wb->weights_sizes[1];
    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize((vx_type_e)weight_format);
    vx_uint32 weightBitSize      = weightSize * 8;

    vx_uint32 filterSliceSize    = weightCount * weightSize;
    vx_uint32 filterSize         = weightCount * weightSize * slice_count;
    vx_uint8* kernelBufferInt8Ptr    = reorder_stream;
    vx_uint16* kernelBufferInt16Ptr    = (vx_uint16*)reorder_stream;

    vx_uint32 coreIndex;
    vx_uint32 filterIndex;
    vx_uint32 sliceIndex;
    vx_uint32 k, sk;
    vx_uint32 kSize;
    vx_uint32 weightXIndex, weightYIndex, weightZIndex;

    vx_bool hasZDP3 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3);
    vx_uint32 dpAmount = hasZDP3 ? 3 : 1;
    vx_uint32 zDPLoopCount = 3;
    vx_uint32 linesInImageBuffer = dpAmount * zDPLoopCount;

    vx_uint32 nonCoefIndex = 0, idx = 0, elementIndex = 0;
    vx_uint8* kernelDataPtr      = VX_NULL;

    vx_uint32 totalNonZeroCount = 0;
    vx_float32 averageNZCPerFilter = 0;
    vx_float32 varianceFL32 = 0;
    vx_uint32 variance = 0;
    vx_bool reorder = vx_false_e;

    typedef struct _gcVXNonZeroWeights
    {
        vx_uint32 nonZeroCount;
        vx_uint32 filterIdx;
    }gcVXNonZeroWeights;

#if PRINT_PER_CORE_SIZE
    vx_uint32* nonZeroCoefPerCore = VX_NULL;
#endif

    gcVXNonZeroWeights *nonZeroWeights = VX_NULL;
    vx_int32 i = 0;
    vx_uint32 core0FilterCount = 0;
    gcVXNonZeroWeights tmp;

    nonZeroWeights = (gcVXNonZeroWeights*)vxAllocateAndZeroMemory(filterTotalCount * sizeof(gcVXNonZeroWeights));
    if (nonZeroWeights == VX_NULL)
    {
        vxError("fillinKernelBuffer: OUT OF MEMORY");
        goto exit;
    }
#if PRINT_PER_CORE_SIZE
    nonZeroCoefPerCore = (vx_uint32*)vxAllocateAndZeroMemory(nnCoreCount * sizeof(vx_uint32));
    if (nonZeroCoefPerCore == VX_NULL)
    {
        vxError("fillinKernelBuffer: OUT OF MEMORY");
        goto exit;
    }
#endif

    /* calc each filter non-zero weights num*/
    for (filterIndex = 0; filterIndex < filterTotalCount; filterIndex++)
    {
        nonZeroWeights[filterIndex].filterIdx = filterIndex;
        for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
        {
            /* add one slice data every filter */
            kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * sliceIndex;

            for (weightYIndex = 0; weightYIndex < wb->weights_sizes[1]; weightYIndex++)
            {
                for (weightXIndex = 0; weightXIndex < wb->weights_sizes[0]; weightXIndex++)
                {
                    vx_uint32 weight = 0;

                    if (weight_format == VX_TYPE_INT8)
                        weight = *((vx_int8 *)kernelDataPtr);
                    else if (weight_format == VX_TYPE_UINT8)
                        weight = *((vx_uint8 *)kernelDataPtr);
                    else
                        weight = *((vx_uint16 *)kernelDataPtr);
                    kernelDataPtr = kernelDataPtr + weightSize;

                    if (weight != skip_value)
                        nonZeroWeights[filterIndex].nonZeroCount++;

                }
            }
        }
        totalNonZeroCount += nonZeroWeights[filterIndex].nonZeroCount;
    }

    /*Sort*/
    averageNZCPerFilter = (vx_float32)totalNonZeroCount / filterTotalCount;
    for (filterIndex = 0; filterIndex < filterTotalCount; filterIndex++)
    {
        varianceFL32 += gcoMATH_Power((nonZeroWeights[filterIndex].nonZeroCount - averageNZCPerFilter), 2);
    }
    variance = (vx_uint32)(varianceFL32 / filterTotalCount + 0.5f);

    reorder = vx_true_e;

    if (reorder)
    {
        /*Sort*/
        for (filterIndex = 0; filterIndex < filterTotalCount; filterIndex++)
        {
            tmp.nonZeroCount = nonZeroWeights[filterIndex].nonZeroCount;
            tmp.filterIdx = filterIndex;

            for (i = filterIndex - 1; i >= 0 && nonZeroWeights[i].nonZeroCount > tmp.nonZeroCount; i--)
            {
                nonZeroWeights[i+1].nonZeroCount = nonZeroWeights[i].nonZeroCount;
                nonZeroWeights[i+1].filterIdx = nonZeroWeights[i].filterIdx;
            }
            nonZeroWeights[i+1].nonZeroCount = tmp.nonZeroCount;
            nonZeroWeights[i+1].filterIdx = tmp.filterIdx;
        }
    }

    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        vx_uint32 coreFilterCount = (coreIndex < oddFilterPerCore) ? totalFilterPerCore + 1 : totalFilterPerCore;
        vx_uint32 groupIndex;
        vx_uint32 coef = 0;
        vx_uint32 usedKid = 0;
        vx_uint32* filterGroup = VX_NULL;
        vx_uint8_ptr realKernelDataPtr = VX_NULL;
        reoder_stream_count_per_core[coreIndex] = 0;

        if (coreFilterCount > 0)
        {
            /* Save kernels per core*/
            if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
            {
                *((vx_uint16 *)kernelBufferInt8Ptr) = (vx_uint16)coreFilterCount;
                kernelBufferInt8Ptr += 2;
            }
            else
            {
                *kernelBufferInt16Ptr = (vx_uint16)coreFilterCount;
                kernelBufferInt16Ptr ++;
            }
            reoder_stream_count_per_core[coreIndex] += (16 / weightBitSize);

            for (idx = 0; idx < (16 / weightBitSize); idx++)
            {
                non_coef_index[nonCoefIndex++] = elementIndex++;
            }
        }

        for (groupIndex = 0; groupIndex < groupCount; groupIndex++)
        {
            vx_uint32 actualFilterCount = (groupIndex == groupCount - 1) ? (coreFilterCount - groupIndex * filterCount) : filterCount;
            vx_uint32 groupFilterStart = groupIndex * nnCoreCount * filterCount;
            vx_uint32 kid = 0;
            vx_uint32 unusedCoreCount = (filterTotalCount % nnCoreCount == 0) ? 0 : nnCoreCount - (filterTotalCount % nnCoreCount);
            vx_uint32 sortedIndex = 0;
            vx_uint32 filterStart, filterEnd;

            if ((groupIndex == groupCount - 1)
                && (coreIndex >= (filterTotalCount % nnCoreCount)))
            {
                filterStart = groupIndex * nnCoreCount * filterCount
                            + (filterTotalCount % nnCoreCount) * (actualFilterCount + 1)
                            + (coreIndex - (filterTotalCount % nnCoreCount)) * actualFilterCount;
            }
            else
            {
                filterStart = groupIndex * nnCoreCount *filterCount + coreIndex * actualFilterCount;
            }
            filterEnd = filterStart + actualFilterCount - 1;

            if (coreIndex == 0)
                core0FilterCount = actualFilterCount;

            if (actualFilterCount)
            {
                filterGroup = (vx_uint32*)vxAllocateAndZeroMemory(actualFilterCount * sizeof(vx_uint32));
                if (filterGroup == VX_NULL)
                {
                    vxError("fillinKernelBufferBalance: OUT OF MEMORY");
                    goto exit;
                }
                if (groupIndex == groupCount - 1)
                    last_vz_group_index[coreIndex] = elementIndex;
            }

            if (reorder)
            {
                /*Dispatch filter to each vzGroup*/
                for (kid = 0; kid < actualFilterCount; kid++)
                {
                    if (groupIndex == groupCount - 1 &&
                        kid == core0FilterCount - 1)
                    {
                        if ((kid + usedKid) % 2 == 0)
                            sortedIndex = groupFilterStart + kid * nnCoreCount + coreIndex;
                        else
                            sortedIndex = groupFilterStart + (kid + 1) * nnCoreCount - coreIndex - 1 - unusedCoreCount;
                    }
                    else
                    {
                        if ((kid + usedKid) % 2 == 0)
                            sortedIndex = groupFilterStart + kid * nnCoreCount + coreIndex;
                        else
                            sortedIndex = groupFilterStart + (kid + 1) * nnCoreCount - coreIndex - 1;
                    }

                    filterGroup[kid] = nonZeroWeights[sortedIndex].filterIdx;
#if PRINT_PER_CORE_SIZE
                    nonZeroCoefPerCore[coreIndex] += nonZeroWeights[sortedIndex].nonZeroCount;
#endif
                }

                /*sort the vz group real filter to avoid write output cross*/
                for (kid = 0; kid < actualFilterCount; kid++)
                {
                    vx_uint32 tmp = filterGroup[kid];

                    for (i = kid - 1; i >= 0 && filterGroup[i] > tmp; i--)
                    {
                        filterGroup[i+1] = filterGroup[i];
                    }
                    filterGroup[i+1] = tmp;
                }
            }
            else
            {
                /*if variance is small, don't reorder the filters*/
                if ((groupIndex == groupCount - 1)
                && (coreIndex >= (filterTotalCount % nnCoreCount)))
                {
                    filterStart = groupIndex * nnCoreCount * filterCount
                                + (filterTotalCount % nnCoreCount) * (actualFilterCount + 1)
                                + (coreIndex - (filterTotalCount % nnCoreCount)) * actualFilterCount;
                }
                else
                {
                    filterStart = groupIndex * nnCoreCount *filterCount + coreIndex * actualFilterCount;
                }

                for (kid = 0; kid < actualFilterCount; kid++)
                {
                    filterGroup[kid] = filterStart + kid;
                }
            }

            if (real_vz_index)
            {
                vx_uint32 id = 0;
                for (filterIndex = filterStart; filterIndex <= filterEnd; filterIndex++)
                {
                    /*Save real filter index*/
                    real_vz_index[filterIndex] = filterGroup[id++];
                }
            }

            for (k = 0; k < weightCount * slice_count; k += linesInImageBuffer)
            {
                kSize = gcmMIN((weightCount * slice_count - k), linesInImageBuffer);

                for (kid = 0; kid < actualFilterCount; kid++)
                {
                    filterIndex = filterGroup[kid];

                    for (sk = 0; sk < linesInImageBuffer; sk++)
                    {
                        weightZIndex = (k + sk) / weightCount;
                        weightYIndex = (k + sk) % weightCount;
                        weightYIndex /= wb->weights_sizes[0];
                        weightXIndex = (k + sk) % wb->weights_sizes[0];
                        /*Packl 0 in the last dp group*/
                        if (sk >= kSize)
                            coef = skip_value;
                        else
                        {
                            realKernelDataPtr = weight_base_ptr + filterIndex * filterSize +
                                filterSliceSize * weightZIndex +
                                (weightYIndex * wb->weights_sizes[0] + weightXIndex) * weightSize;

                            if (weight_format == VX_TYPE_INT8)
                                coef = *((vx_int8 *)realKernelDataPtr);
                            else if (weight_format == VX_TYPE_UINT8)
                                coef = *((vx_uint8 *)realKernelDataPtr);
                            else
                                coef = *((vx_uint16 *)realKernelDataPtr);
                        }

                        if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
                        {
                            vx_int8 newCoef = ((vx_int32)coef - (vx_int32)skip_value) & 0xFF;
                            *kernelBufferInt8Ptr = (vx_uint8)newCoef;
                            kernelBufferInt8Ptr++;
                        }
                        else
                        {
                            if (weight_format == VX_TYPE_FLOAT16)
                                coef = (coef & 0x7fff) * 2 + coef/(1<<15);

                            *kernelBufferInt16Ptr = (vx_uint16)coef;
                            kernelBufferInt16Ptr++;
                        }
                        reoder_stream_count_per_core[coreIndex]++;
                        elementIndex++;
                    }
                }
            }
            usedKid += actualFilterCount;
            if (filterGroup)
            {
                vxFree(filterGroup);
                filterGroup = VX_NULL;
            }
        }
    }
exit:

    if (nonZeroWeights)
        vxFree(nonZeroWeights);
#if PRINT_PER_CORE_SIZE
    if(nonZeroCoefPerCore)
        vxFree(nonZeroCoefPerCore);
#endif
    return;
}

vx_uint32 calcKernelSizeV8Huffman(
    vx_context context,
    vx_weights_biases_parameter wb,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_enum  weight_format,
    vx_uint32 skip_value,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* post_mul,
    vx_uint32* post_shift,
    vx_uint32* neg_post_mul,
    vx_uint32* neg_post_shift,
    vx_uint32 index
    )
{
    vx_uint32 nnCoreCount = (weight_format == VX_TYPE_INT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : (weight_format == VX_TYPE_FLOAT16) ?
                      context->nnConfig.fixedFeature.nnCoreCountFloat16 : context->nnConfig.fixedFeature.nnCoreCount;

    vx_uint32 kernelSize = 0;
    vx_uint32 kernelBitSize = 0;
    vx_uint32 invSizeOrder[MAX_SIZE_HISTO_COUNT] = {0};
    vx_int32 coef = 0, tmp = 0, prev = 0;

    vx_uint32 filterTotalCount   = z_count;
    vx_uint32 usedCoreCount      = (filterTotalCount > nnCoreCount) ? nnCoreCount : filterTotalCount;

    vx_uint32 weightCount        = weight_x * weight_y;
    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize((vx_type_e)weight_format);
    vx_uint32 weightBitSize      = weightSize * 8;
    vx_uint32 biasBitSize        = (weight_format == VX_TYPE_INT16) ? NN_INTEGER_BIAS_BITS_VIP_V7_INT16 : NN_INTEGER_BIAS_BITS_VIP_V7;
    vx_uint32 biasSize           = biasBitSize / 8;

    vx_uint8_ptr kernelDataPtr   = VX_NULL;

    vx_uint32 coreIndex;
    vx_uint32 i;

    vx_uint32 bit16Flag = (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8) ? 0 : 1;

    vx_uint32 reorderStreamSize = 0;
    vx_uint32 reorderStreamAllCount = 0;
    vx_uint32 reorderStreamCheckCount = 0;
    vx_uint32* reorderStreamPerCoreCount = VX_NULL;
    vx_uint32 filterIndex = 0;

    vx_uint8* reorderStream = VX_NULL;
    vx_uint32 runSet2Bits = 1;
    CodeSymbol codeSymbol[THROUGHPUT * 3];

    vx_bool hasNoBias = vx_false_e;
    vx_bool hasNoZOffset = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_NO_Z_LOCATION_OFFSET);
    vx_bool hasNNPerFilterPostMultiply = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PER_CHANNEL_POST_MULTIPLY);
    vx_bool hasNNPreLU = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PRELU);
    vx_bool hasZDP3 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3);
    vx_uint32 dpAmount = hasZDP3 ? 3 : 1;
    vx_uint32 zDPLoopCount = 3;
    vx_uint32 linesInImageBuffer = dpAmount * zDPLoopCount;
    vx_uint32 numOfInImageBuffer = (vx_uint32)gcoMATH_Ceiling((vx_float32)(weightCount * slice_count) / (vx_float32)linesInImageBuffer);

    vx_uint32 nonCoefCount = 0;
    vx_uint32* nonCoefIndex = VX_NULL;
    vx_uint32* lastVzGroupIndex = VX_NULL;
    vx_uint32 totalCountIdx = 0;
    vx_uint32 nonCoefCountIdx = 0;
    vx_uint32 kernelStreamSizePerCore = 0;
    vx_uint32 limitZRLCountIdx = 0;

    reorderStreamAllCount = (numOfInImageBuffer * linesInImageBuffer * filterTotalCount)
        + (usedCoreCount * (16 / weightBitSize));/*filterPerCore need 16 bit * nnCoreCount */

    reorderStreamSize = reorderStreamAllCount * weightSize;
    if (hasNNPerFilterPostMultiply)
        reorderStreamSize += filterTotalCount; /*Per channel 8 bit zero point*/

    reorderStream = (vx_uint8 *)vxAllocateAndZeroMemory(reorderStreamSize);

    if (!reorderStream)
    {
        vxError("fillinKernelBufferHuffman: OUT OF MEMORY");
        goto exit;
    }

    reorderStreamPerCoreCount = (vx_uint32 *)vxAllocateAndZeroMemory(nnCoreCount * sizeof(vx_uint32));
    if (!reorderStreamPerCoreCount)
    {
        vxError("fillinKernelBufferHuffman: OUT OF MEMORY");
        goto exit;
    }

    nonCoefCount = usedCoreCount * (16 / weightBitSize);
    if (hasNNPerFilterPostMultiply)
        nonCoefCount += filterTotalCount;

    nonCoefIndex = (vx_uint32 *)vxAllocateAndZeroMemory(nonCoefCount * sizeof(vx_uint32));
    if (!nonCoefIndex)
    {
        vxError("fillinKernelBufferHuffman: OUT OF MEMORY");
        goto exit;
    }

    lastVzGroupIndex = (vx_uint32 *)vxAllocateAndZeroMemory(usedCoreCount * sizeof(vx_uint32));
    if (!lastVzGroupIndex)
    {
        vxError("fillinKernelBufferHuffman: OUT OF MEMORY");
        goto exit;
    }

    if (wb->wb_base->hw_depth_wise)
        reorderDepthWiseKernelBufferV8Huffman(context, wb, slice_count, z_count, filters_per_core, skip_value, weight_format, reorderStream, weight_base_ptr, nnCoreCount, reorderStreamPerCoreCount, nonCoefIndex);
    else if (context->options.enableNonZeroBalance && !hasNoZOffset) /*Only those chip has z_offset support zero balance*/
        reorderKernelBufferV8HuffmanBalance(context, wb, slice_count, z_count, filters_per_core, skip_value, weight_format, reorderStream, weight_base_ptr, nnCoreCount, reorderStreamPerCoreCount, nonCoefIndex, VX_NULL, lastVzGroupIndex);
    else
        reorderKernelBufferV8Huffman(context, wb, slice_count, z_count, filters_per_core, skip_value, weight_format, reorderStream, weight_base_ptr, nnCoreCount, reorderStreamPerCoreCount, nonCoefIndex, lastVzGroupIndex);

    analysisKernelStreamForHuffman(context, wb, 0, nnCoreCount, reorderStream, reorderStreamSize, reorderStreamPerCoreCount, invSizeOrder, nonCoefIndex, nonCoefCount, lastVzGroupIndex, usedCoreCount, index);

    kernelBitSize += 16;
    kernelBitSize += 8 * MAX_RUNLEN_SIZE;
    kernelBitSize += 8 * LOG_RUN_SIZE1;
    kernelBitSize += 32;
    kernelBitSize += 32 * nnCoreCount;

    kernelSize = (kernelBitSize % 8 == 0) ? kernelBitSize / 8 : kernelBitSize / 8 + 1;
    /*align to 64 byte*/
    kernelSize = (kernelSize + 63) & 0xFFFFFFC0;
    kernelBitSize = 0;

    for (i = 0; i < THROUGHPUT * 3; i++)
    {
        codeSymbol[i].stageCode[0] = codeSymbol[i].stageCode[1] = codeSymbol[i].stageCode[2] = 0;
        codeSymbol[i].bitLength[0] = codeSymbol[i].bitLength[1] = codeSymbol[i].bitLength[2] = 0;
    }

    while(((numBestRuns-SET_1_SIZE)>>runSet2Bits) > 1)
    {
        runSet2Bits++;
    }

    kernelDataPtr = reorderStream;
    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        vx_int32 run = 0x0;
        vx_uint32 dataSizeOfCore = reorderStreamPerCoreCount[coreIndex];
        vx_uint32 dataRemainingOfCore = dataSizeOfCore;
        vx_int32 size = 0;
        vx_int32 code = 0;
        vx_int32 coef16 = 0;
        vx_int32 k = 0, j = 0, m = 0;
        vx_bool isCoef;
        vx_bool isLimitZRL;
        /* update position of each core's beginning of compressed kernel*/
        reorderStreamCheckCount += reorderStreamPerCoreCount[coreIndex];

        memset(codeSymbol, 0, sizeof(CodeSymbol));

        while(dataRemainingOfCore --)
        {
            if (weight_format == VX_TYPE_INT8)
                coef = *((vx_int8 *)kernelDataPtr);
            else if (weight_format == VX_TYPE_UINT8)
                coef = *((vx_uint8 *)kernelDataPtr);
            else if (weight_format == VX_TYPE_INT16 ||
                weight_format == VX_TYPE_FLOAT16)
                coef16 = *((vx_int16 *)kernelDataPtr);
            else
            {
                /*Other format not suppoted now*/
                vxmASSERT(0);
            }
            kernelDataPtr += weightSize;

            if (totalCountIdx == nonCoefIndex[nonCoefCountIdx] &&
                nonCoefCountIdx < nonCoefCount)
            {
                isCoef = vx_false_e;
                nonCoefCountIdx++;
            }
            else
                isCoef = vx_true_e;

            if (lastVzGroupIndex != VX_NULL &&
                totalCountIdx == lastVzGroupIndex[limitZRLCountIdx] &&
                limitZRLCountIdx < usedCoreCount)
            {
                isLimitZRL = vx_true_e;
                limitZRLCountIdx++;
            }
            else
                isLimitZRL = vx_false_e;

            totalCountIdx++;

            if (bit16Flag == 0)
            {

                coef = (coef - wb->huffmanConfig[index].avgBias) & 0xFF;
                if (wb->huffmanConfig[index].preEncode)
                {
                    tmp = coef;
                    coef = (coef - prev) & 0xFF;
                    prev = tmp;
                }
            }
            else
            {

                if (wb->huffmanConfig[index].fp16Flag)
                {
                    coef16 = (coef16 & 0x7FFF) * 2 + coef16 / (1<<15);
                }
                coef16 -= wb->huffmanConfig[index].avgBias;
                if (wb->huffmanConfig[index].runLenTableSize == 0x0)
                    coef = (coef16 >> 8) & 0xFF; /*Non RZL*/
                else
                    coef = coef16 & 0xFFFF; /*RZL path*/
            }

            if (wb->huffmanConfig[index].runLenTableSize == 0x0) /* non RZL*/
            {
                tmp = (coef & 0x80)? (coef ^ 0xff): coef;
                for (size = 0; size<7; size++)
                {
                    if (tmp<(1<<size))
                        break;
                }
                k = invSizeOrder[size];
                code = huffCode[k];

                if (sizeCodeLen[k]%2 == 0)/*huffCode length = 2 or 4, pack 1 bit to 3 or 5*/
                    code |= ((coef>>7)<<(sizeCodeLen[k]) );
                kernelBitSize += 3;

                if (sizeCodeLen[k] > 3)
                {
                    kernelBitSize += 2;
                }

                if (size == 0) size = 1; /*Size = 0 mean 0&0xff, needs 1 bit actually*/
                if (sizeCodeLen[k]%2 == 0)
                {
                    kernelBitSize += (size - 1);
                }
                else
                {
                    kernelBitSize += size;
                }

                if (wb->huffmanConfig[index].bit16Flag)
                {/*pack the raw data of the lower 8 bits*/
                    kernelBitSize += 8;
                }
            }
            else /* RZL enable*/
            {
                if (coef == 0 && isCoef && !isLimitZRL)
                {
                    run++;
                    if (dataRemainingOfCore != 0)
                    {
                        continue;
                    }
                }
                if (coef || (dataRemainingOfCore == 0) || !isCoef || isLimitZRL) /* if last pixel, force process run zeros*/
                {
                    /* process run zeros*/
                    while(run)
                    {
                        for (j = numBestRuns - 1; j>=0; j--)
                        {
                            if (run >= bestRunsSorted[j] + 1)
                            {
                                k = run / (bestRunsSorted[j] + 1);
                                run %= (bestRunsSorted[j] + 1);
                                break;
                            }
                        }
                        if (j >= 0)
                        {
                            vx_int32 n = bestRunsSorted[j];
                            size = 8;
                            for (m = 0; m<SET_1_SIZE; m++)
                            {
                                if (best2Runs[m] == n)
                                {
                                    size = 0;
                                    break;
                                }
                            }
                            if (size == 8)
                            {
                                for (m = 0; m<numBestRuns - SET_1_SIZE; m++)
                                    if (outBest2Run[m] == n)
                                        break;

                            }
                            j = invSizeOrder[size];
                            while(k--)
                            {/*k of the runs*/
                                kernelBitSize += 3;

                                if (sizeCodeLen[j] > 3)
                                {
                                    kernelBitSize += 2;
                                }
                                if (size == 0 )
                                {/*The m already code*/
                                    kernelBitSize += ((sizeCodeLen[j]%2 == 0) ? 0:1);
                                }
                                else if (sizeCodeLen[j]%2 == 0)
                                {
                                    kernelBitSize += (runSet2Bits - 1);
                                }
                                else
                                {
                                    kernelBitSize += runSet2Bits;
                                }
                            }
                        }
                    }
                    /*process current pixel*/
                    if (coef)
                    {
                        vx_int8 ch;
                        coef>>=(wb->huffmanConfig[index].bit16Flag * 8);
                        if(coef<0x80)
                            coef+=wb->huffmanConfig[index].bit16Flag; /*Since we have to encode zero (run-length only handle 16-bits zero, 8 bit zero should use Size code*/
                        ch = (vx_int8)coef;
                        coef = abs(ch);
                        for (j = 6; j>=0; j--)
                        {
                            if (coef>=(1<<j))
                            {
                                break;
                            }
                        }
                        size = j+1;
                        if (invSizeOrder[size]==8)
                        { /*The minimum freq, we out the raw data*/
                            size = 7;
                        }

                        j = invSizeOrder[size];
                        kernelBitSize += 3;

                        if (sizeCodeLen[j] > 3)
                        {
                            kernelBitSize += 2;
                        }

                        if (size == 7)
                        {
                            size = 8;
                        }
                        /*Size = 0 mean 0&0xff, needs 1 bit actually*/
                        if (size == 0)
                            size = 1;
                        if (sizeCodeLen[j]%2 == 0)
                        {
                            kernelBitSize += (size - 1);
                        }
                        else
                        {
                            kernelBitSize += size;
                        }
                        if (wb->huffmanConfig[index].bit16Flag)
                        {
                            kernelBitSize += 8;
                        }
                    }
                    else if (!isCoef || (dataRemainingOfCore != 0x0) || isLimitZRL)
                    {
                        /*those non-coef count like bias, num_of_vz & z_offset couldn't skip by zero-run as HW requres,
                        need also encode those 0 to huffman code,
                        in ZRL path 8-bit, 0 belong to group7, -128~127, residue bit length is 8, residue code = 0*/
                        size = 7;
                        j = invSizeOrder[size];
                        code = huffCode[j];

                        kernelBitSize += 3;

                        if (sizeCodeLen[j] > 3)
                        {
                            kernelBitSize += 2;
                        }

                        if (size == 7)
                        {
                            size = 8;
                        }

                        if (sizeCodeLen[j] % 2 == 0)
                        {
                            kernelBitSize += (size - 1);
                        }
                        else
                        {
                            kernelBitSize += size;
                        }
                        if (wb->huffmanConfig[index].bit16Flag)
                        {
                            kernelBitSize += 8;
                        }
                    }
                }
            }

            if(dataRemainingOfCore == 0)
            {
                /*Hit the end bitsteam of core, add dummy stage process the next*/
                kernelBitSize += 3;
                kernelBitSize += 2;
                kernelBitSize += 7;

                if (wb->huffmanConfig[index].bit16Flag)
                    kernelBitSize += 8;

                for (j = 0; j < 2*THROUGHPUT; j++)
                {
                    kernelBitSize += 3;
                }
                break;
            }
        }

        /* Go back to update kernelStreamSize. */
        kernelStreamSizePerCore = (kernelBitSize % 8 == 0) ? kernelBitSize / 8 : kernelBitSize / 8 + 1;
        /*align to 64 byte*/
        kernelStreamSizePerCore = (kernelStreamSizePerCore + 63) & 0xFFFFFFC0;
        kernelBitSize = 0;
        kernelSize += kernelStreamSizePerCore;
    }

    /*Add non compressed biases & Z_OFFSET & perFilterPostMul & perFilterPostShift after huffman bit stream*/
    kernelStreamSizePerCore = 0;
    for (filterIndex = 0; filterIndex < filterTotalCount; filterIndex++)
    {
        if (!hasNoBias)
        {
            kernelStreamSizePerCore += biasSize;
        }

        if (hasNNPerFilterPostMultiply)
        {
            kernelStreamSizePerCore += 4;
        }

        if (!hasNoZOffset)
        {
            kernelStreamSizePerCore += NN_Z_POSITION_OFFSET_BITS_VIP_V7 / 8;
        }
        else if (hasNNPreLU)
        {
            kernelStreamSizePerCore += 4;
        }
    }
    /*align to 64 byte*/
    kernelStreamSizePerCore = (kernelStreamSizePerCore + 63) & 0xFFFFFFC0;
    kernelSize += kernelStreamSizePerCore;

exit:
    if (reorderStream)
        vxFree(reorderStream);
    if (reorderStreamPerCoreCount)
        vxFree(reorderStreamPerCoreCount);
    if (nonCoefIndex)
        vxFree(nonCoefIndex);
    if (lastVzGroupIndex)
        vxFree(lastVzGroupIndex);

    return kernelSize;
}

void fillinKernelBufferV8Huffman(
    vx_context context,
    vx_weights_biases_parameter wb,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_int32 input_zp,
    vx_int32 coef_zp,
    vx_uint32 skip_value,
    vx_uint32 output_final_x,
    vx_uint32 output_final_y,
    vx_int32 z_offset,
    vx_uint32 output_size,
    vx_uint8_ptr wb_base_ptr,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr,
    vx_uint32* post_mul,
    vx_uint32* post_shift,
    vx_tensor alpha,
    vx_uint32 index
    )
{

    vx_uint32 nnCoreCount = (weight_format == VX_TYPE_INT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : (weight_format == VX_TYPE_FLOAT16) ?
                      context->nnConfig.fixedFeature.nnCoreCountFloat16 : context->nnConfig.fixedFeature.nnCoreCount;

    vx_uint32 invSizeOrder[MAX_SIZE_HISTO_COUNT] = {0};
    vx_int32 coef = 0, tmp = 0, prev = 0;

    vx_uint32 filterTotalCount   = z_count;
    vx_uint32 usedCoreCount      = (filterTotalCount > nnCoreCount) ? nnCoreCount : filterTotalCount;
    vx_uint32 weightCount        = weight_x * weight_y;
    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize((vx_type_e)weight_format);
    vx_uint32 weightBitSize      = weightSize * 8;
    vx_uint32 biasBitSize        = (weight_format == VX_TYPE_INT16) ? NN_INTEGER_BIAS_BITS_VIP_V7_INT16 : NN_INTEGER_BIAS_BITS_VIP_V7;

    vx_uint32 filterSize         = weightCount * weightSize * slice_count;
    vx_uint32* kernelBufferPtr   = (vx_uint32*)wb_base_ptr;

    vx_uint32* kernelStreamSizePtr = VX_NULL;
    vx_uint32 kernelStreamSize = 0;
    vx_uint32 alignedOffset      = ((vx_uint32)(gctUINTPTR_T)kernelBufferPtr) & 0x3F;
    vx_uint8_ptr kernelDataPtr   = VX_NULL;
    vx_uint8_ptr kernelStreamBasePtr   = VX_NULL;
    vx_uint32 streamSizeBitOffset = 0;
    vx_uint32 maxKernelStreamSizePerCore = 0;

    vx_uint32 coreIndex;
    vx_uint32 i;

    vx_uint32 bit16Flag = (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8) ? 0 : 1;
    vx_uint32 bitOffset = 0;

    vx_uint32 reorderStreamSize = 0;
    vx_uint32 reorderStreamAllCount = 0;
    vx_uint32 reorderStreamCheckCount = 0;
    vx_uint32* reorderStreamPerCoreCount = VX_NULL;
    vx_uint32 filterIndex = 0;

    vx_uint8* reorderStream = VX_NULL;
    vx_uint32 runSet2Bits = 1;
    CodeSymbol codeSymbol[THROUGHPUT * 3];

    vx_bool hasNoBias = vx_false_e;
    vx_bool hasNoZOffset = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_NO_Z_LOCATION_OFFSET);
    vx_bool hasNNPerFilterPostMultiply = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PER_CHANNEL_POST_MULTIPLY);
    vx_bool hasNNPreLU = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PRELU);
    vx_bool hasZDP3 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3);
    vx_uint32 dpAmount = hasZDP3 ? 3 : 1;
    vx_uint32 zDPLoopCount = 3;
    vx_uint32 linesInImageBuffer = dpAmount * zDPLoopCount;
    vx_uint32 numOfInImageBuffer = (vx_uint32)gcoMATH_Ceiling((vx_float32)(weightCount * slice_count) / (vx_float32)linesInImageBuffer);

    vx_uint32 dummyStage[3] = {0};
    vx_uint32 dummyBitLength[3] = {0};
    vx_bool setDummy = vx_false_e;

    vx_uint32 nonCoefCount = 0;
    vx_uint32 totalCountIdx = 0;
    vx_uint32 nonCoefCountIdx = 0;
    vx_uint32* nonCoefIndex = VX_NULL;

    vx_int32* coefSum = VX_NULL;
    vx_uint32* realVzIndex = VX_NULL;
    vx_uint32* lastVzGroupIndex = VX_NULL;
    vx_uint32 limitZRLCountIdx = 0;
    /*prelu parameter alpha*/
    gctPOINTER alphaBase = VX_NULL;
    vx_int32    alphaZP  = 0;
    vx_int8     alphaFP  = 0;
    vx_float32  alphaScale = 0;
    vx_type_e   alphaFormat = 0;
    vx_enum     alphaQuantFormat = 0;

    typedef struct _gcVXWeightsMinusZP
    {
        vx_int32 sum;
        vx_uint32 filterIdx;
    }gcVXWeightsMinusZP;
    gcVXWeightsMinusZP *weightsMinusZP = VX_NULL;

    if (alpha != VX_NULL)
    {
        vxoTensor_GetTensorViewMemory(alpha, &alphaBase, VX_NULL);
        alphaZP = alpha->zeroPoint;
        alphaScale = alpha->scale;
        alphaFP = alpha->fixedPointPos;
        alphaFormat = (vx_type_e)alpha->tensorBuffer->dataFormat;
        alphaQuantFormat = alpha->quantFormat;
    }

    if (weight_format == VX_TYPE_INT16 || weight_format == VX_TYPE_UINT8)
    {
        vx_uint32 sliceIndex, weightXIndex, weightYIndex;
        vx_uint32 filterSliceSize = weight_x * weight_y * weightSize;

        coefSum = (vx_int32*)vxAllocateAndZeroMemory(filterTotalCount * sizeof(vx_int32));
        if (!coefSum)
        {
            vxError("fillinKernelBufferV8Huffman: OUT OF MEMORY");
            goto exit;
        }

        /*Calc sum((coef[i] - coefZP) * InZP) */
        if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT) &&
            input_zp != 0 &&
            wb->wb_base->weights_quant_format == VX_QUANT_AFFINE_SCALE &&
            weight_format == VX_TYPE_UINT8)
        {
            weightsMinusZP = (gcVXWeightsMinusZP*)vxAllocateAndZeroMemory(filterTotalCount * sizeof(gcVXWeightsMinusZP));

            if (!weightsMinusZP)
            {
                vxError("fillinKernelBufferV8Huffman: OUT OF MEMORY");
                goto exit;
            }
        }

        for (filterIndex = 0; filterIndex < filterTotalCount; filterIndex++)
        {
            if (weightsMinusZP != VX_NULL)
            {
                weightsMinusZP[filterIndex].filterIdx = filterIndex;
                weightsMinusZP[filterIndex].sum = 0;
            }
            for (sliceIndex = 0; sliceIndex < slice_count; sliceIndex++)
            {
                /* add one slice data every filter */
                kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * sliceIndex;

                for (weightYIndex = 0; weightYIndex < weight_y; weightYIndex++)
                {
                    for (weightXIndex = 0; weightXIndex < weight_x; weightXIndex++)
                    {
                        vx_int32 weight = 0;
                        if (weight_format == VX_TYPE_UINT8)
                        {
                            weight = *((vx_uint8 *)kernelDataPtr);
                            coefSum[filterIndex] += (weight - skip_value);
                        }
                        else
                        {
                            weight = *((vx_int16 *)kernelDataPtr);
                            coefSum[filterIndex] += weight;
                        }
                        kernelDataPtr = kernelDataPtr + weightSize;

                        if (weightsMinusZP != VX_NULL)
                        {
                            /*Calc sum((coef[i] - coefZP) * InZP) */
                            weightsMinusZP[filterIndex].sum += (weight - coef_zp) * input_zp;
                        }
                    }
                }
            }
        }
    }

    reorderStreamAllCount = (numOfInImageBuffer * linesInImageBuffer * filterTotalCount)
        + (usedCoreCount * (16 / weightBitSize));/*filterPerCore need 16 bit * nnCoreCount */

    reorderStreamSize = reorderStreamAllCount * weightSize;

    if (hasNNPerFilterPostMultiply)
        reorderStreamSize += filterTotalCount; /*Per channel 8 bit zero point*/

    reorderStream = (vx_uint8 *)vxAllocateAndZeroMemory(reorderStreamSize);

    if (!reorderStream)
    {
        vxError("fillinKernelBufferV8Huffman: OUT OF MEMORY");
        goto exit;
    }

    reorderStreamPerCoreCount = (vx_uint32 *)vxAllocateAndZeroMemory(nnCoreCount * sizeof(vx_uint32));
    if (!reorderStreamPerCoreCount)
    {
        vxError("fillinKernelBufferV8Huffman: OUT OF MEMORY");
        goto exit;
    }

    nonCoefCount = usedCoreCount * (16 / weightBitSize);
    if (hasNNPerFilterPostMultiply)
        nonCoefCount += filterTotalCount;

    nonCoefIndex = (vx_uint32 *)vxAllocateAndZeroMemory(nonCoefCount * sizeof(vx_uint32));
    if (!nonCoefIndex)
    {
        vxError("fillinKernelBufferV8Huffman: OUT OF MEMORY");
        goto exit;
    }

    lastVzGroupIndex = (vx_uint32 *)vxAllocateAndZeroMemory(usedCoreCount * sizeof(vx_uint32));
    if (!lastVzGroupIndex)
    {
        vxError("fillinKernelBufferHuffman: OUT OF MEMORY");
        goto exit;
    }

    if (wb->wb_base->hw_depth_wise)
        reorderDepthWiseKernelBufferV8Huffman(context, wb, slice_count, z_count, filters_per_core, skip_value, weight_format, reorderStream, weight_base_ptr, nnCoreCount, reorderStreamPerCoreCount, nonCoefIndex);
    else if (context->options.enableNonZeroBalance && !hasNoZOffset)
    {
        /*Only those chip has z_offset support zero balance*/
        realVzIndex = (vx_uint32 *)vxAllocateAndZeroMemory(z_count * sizeof(vx_uint32));
        if (!realVzIndex)
        {
            vxError("fillinKernelBufferV8Huffman: OUT OF MEMORY");
            goto exit;
        }
        reorderKernelBufferV8HuffmanBalance(context, wb, slice_count, z_count, filters_per_core, skip_value, weight_format, reorderStream, weight_base_ptr, nnCoreCount, reorderStreamPerCoreCount, nonCoefIndex, realVzIndex, lastVzGroupIndex);
    }
    else
        reorderKernelBufferV8Huffman(context, wb, slice_count, z_count, filters_per_core, skip_value, weight_format, reorderStream, weight_base_ptr, nnCoreCount, reorderStreamPerCoreCount, nonCoefIndex, lastVzGroupIndex);

    analysisKernelStreamForHuffman(context, wb, 0, nnCoreCount, reorderStream, reorderStreamSize, reorderStreamPerCoreCount, invSizeOrder, nonCoefIndex, nonCoefCount, lastVzGroupIndex, usedCoreCount, index);

    /* Write huffman header*/
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].preEncode, 1);
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].bit16Flag, 1);
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].fp16Flag, 1);
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].reserved, 1);
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].version, 4);
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].runLenTableSize, 8);

    for(i = 0; i < MAX_RUNLEN_SIZE; i++)
    {
        writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].runLenTable[i], 8);
    }
    for(i = 0; i < LOG_RUN_SIZE1; i++)
    {
        writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].mapToHuffman[i], 8);
    }
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].avgBias, 16);
    writeBits(&kernelBufferPtr, &bitOffset, 0, 16); /*reserved, must zero*/

    kernelStreamSizePtr = kernelBufferPtr;
    streamSizeBitOffset = bitOffset;
    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        writeBits(&kernelBufferPtr, &bitOffset, kernelStreamSize, 32);
    }
    /*align to 64 byte*/
    packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);
    wb->slice_array[index].kernel_stream_full_cache_size = 0;

    for (i = 0; i < THROUGHPUT * 3; i++)
    {
        codeSymbol[i].stageCode[0] = codeSymbol[i].stageCode[1] = codeSymbol[i].stageCode[2] = 0;
        codeSymbol[i].bitLength[0] = codeSymbol[i].bitLength[1] = codeSymbol[i].bitLength[2] = 0;
    }

    while(((numBestRuns-SET_1_SIZE)>>runSet2Bits) > 1)
    {
        runSet2Bits++;
    }

    kernelDataPtr = reorderStream;
    wb->max_per_core_compression_ratio = 0;

    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {

        vx_uint32 x = 0x0;
        vx_int32 pos = 0x0;
        vx_int32 run = 0x0;
        vx_uint32 dataSizeOfCore = reorderStreamPerCoreCount[coreIndex];
        vx_uint32 dataRemainingOfCore = dataSizeOfCore;
        vx_int32 size = 0;
        vx_int32 code = 0;
        vx_int32 coef16 = 0;
        vx_int32 k = 0, j = 0, m = 0;
        vx_bool isCoef;
        vx_bool isLimitZRL;
        vx_float64 compressionRatio = 0;
        /* update position of each core's beginning of compressed kernel*/
        kernelStreamBasePtr = (vx_uint8*)kernelBufferPtr;
        reorderStreamCheckCount += reorderStreamPerCoreCount[coreIndex];

        memset(codeSymbol, 0, sizeof(CodeSymbol));
        memset(dummyStage, 0, (sizeof(vx_uint32) * 3));
        memset(dummyBitLength, 0, (sizeof(vx_uint32) * 3));
        setDummy = vx_false_e;

        while(dataRemainingOfCore --)
        {

            pos = (x) % (3*THROUGHPUT);

            if (weight_format == VX_TYPE_INT8)
                coef = *((vx_int8 *)kernelDataPtr);
            else if (weight_format == VX_TYPE_UINT8)
                coef = *((vx_uint8 *)kernelDataPtr);
            else if (weight_format == VX_TYPE_INT16 ||
                weight_format == VX_TYPE_FLOAT16)
                coef16 = *((vx_int16 *)kernelDataPtr);
            else
            {
                /*Other format not suppoted now*/
                vxmASSERT(0);
            }
            kernelDataPtr += weightSize;

            if (totalCountIdx == nonCoefIndex[nonCoefCountIdx] &&
                nonCoefCountIdx < nonCoefCount)
            {
                isCoef = vx_false_e;
                nonCoefCountIdx++;
            }
            else
                isCoef = vx_true_e;

            if (lastVzGroupIndex != VX_NULL &&
                totalCountIdx == lastVzGroupIndex[limitZRLCountIdx] &&
                limitZRLCountIdx < usedCoreCount)
            {
                isLimitZRL = vx_true_e;
                limitZRLCountIdx++;
            }
            else
                isLimitZRL = vx_false_e;

            totalCountIdx++;

            if (bit16Flag == 0)
            {

                coef = (coef - wb->huffmanConfig[index].avgBias) & 0xFF;
                if (wb->huffmanConfig[index].preEncode)
                {
                    tmp = coef;
                    coef = (coef - prev) & 0xFF;
                    prev = tmp;
                }
            }
            else
            {

                if (wb->huffmanConfig[index].fp16Flag)
                {
                    coef16 = (coef16 & 0x7FFF) * 2 + coef16 / (1<<15);
                }
                coef16 -= wb->huffmanConfig[index].avgBias;
                if (wb->huffmanConfig[index].runLenTableSize == 0x0)
                    coef = (coef16 >> 8) & 0xFF; /*Non RZL*/
                else
                    coef = coef16 & 0xFFFF; /*RZL path*/
            }

            if (wb->huffmanConfig[index].runLenTableSize == 0x0) /* non RZL*/
            {
                if (!setDummy)
                {
                    /*Set dummy all to 0,
                    in nonZRL path 8-bit, 0 belong to group0, -1~0, residue bit length is 1, residue code = 0*/
                    size = 0;
                    k = invSizeOrder[size];
                    code = huffCode[k];

                    dummyStage[0] = code & 0x07;
                    dummyBitLength[0] = 3;

                    if (sizeCodeLen[k] > 3)
                    {
                        dummyStage[1] = code >> 3;
                        dummyBitLength[1] = 2;
                    }

                    dummyStage[2] = 0;
                    if (size == 0) size = 1; /*Size = 0 mean 0&0xff, needs 1 bit actually*/
                    if (sizeCodeLen[k]%2 == 0)
                    {
                        dummyBitLength[2] = size - 1;
                    }
                    else
                    {
                        dummyBitLength[2] = size;
                    }

                    if (wb->huffmanConfig[index].bit16Flag)
                    {/*pack the raw data of the lower 8 bits*/

                        dummyBitLength[2] += 8;
                    }
                    setDummy = vx_true_e;
                }

                tmp = (coef & 0x80)? (coef ^ 0xff): coef;
                for (size = 0; size<7; size++)
                {
                    if (tmp<(1<<size))
                        break;
                }
                k = invSizeOrder[size];
                code = huffCode[k];

                if (sizeCodeLen[k]%2 == 0)/*huffCode length = 2 or 4, pack 1 bit to 3 or 5*/
                    code |= ((coef>>7)<<(sizeCodeLen[k]) );
                codeSymbol[pos].stageCode[0] = code & 0x07;
                codeSymbol[pos].bitLength[0] = 3;

                if (sizeCodeLen[k] > 3)
                {
                    codeSymbol[pos].stageCode[1] = code >> 3;
                    codeSymbol[pos].bitLength[1] = 2;
                }

                codeSymbol[pos].stageCode[2] = tmp &((1<<size) - 1 );
                if (size == 0) size = 1; /*Size = 0 mean 0&0xff, needs 1 bit actually*/
                if (sizeCodeLen[k]%2 == 0)
                {
                    codeSymbol[pos].bitLength[2] = size - 1;
                }
                else
                {
                    codeSymbol[pos].stageCode[2] <<= 1;
                    codeSymbol[pos].stageCode[2] |= (coef>>7); /*The sign bit*/
                    codeSymbol[pos].bitLength[2] = size;
                }

                if (wb->huffmanConfig[index].bit16Flag)
                {/*pack the raw data of the lower 8 bits*/

                    codeSymbol[pos].stageCode[2] = codeSymbol[x%(3*THROUGHPUT)].stageCode[2]* 256 + (coef16 &0xff);
                    codeSymbol[pos].bitLength[2] += 8;
                }
                OutputAt(x, &kernelBufferPtr, &bitOffset, codeSymbol);
                x++; /*counter*/
            }
            else /* RZL enable*/
            {
                if (!setDummy)
                {
                    /*Set dummy all to 0,
                    in ZRL path 8-bit, 0 belong to group7, -128~127, residue bit length is 8, residue code = 0*/
                    size = 7;
                    j = invSizeOrder[size];
                    code = huffCode[j];

                    dummyStage[0] = code & 0x07;
                    dummyBitLength[0] = 3;

                    if (sizeCodeLen[j] > 3)
                    {
                        dummyStage[1] = code >> 3;
                        dummyBitLength[1] = 2;
                    }

                    if (size == 7)
                    {
                        size = 8;
                    }

                    if (sizeCodeLen[j] % 2 == 0)
                    {
                        dummyStage[2] = 0;  /*last bit was coded*/
                        dummyBitLength[2] = size - 1;
                    }
                    else
                    {
                        dummyStage[2] = 0;
                        dummyBitLength[2] = size;
                    }
                    if (wb->huffmanConfig[index].bit16Flag)
                    {
                        dummyStage[2] = 0;
                        dummyBitLength[2] += 8;
                    }

                    setDummy = vx_true_e;
                }

                if (coef == 0 && isCoef && !isLimitZRL)
                {
                    run++;
                    if (dataRemainingOfCore != 0)
                    {
                        continue;
                    }
                }
                if (coef || (dataRemainingOfCore == 0) || !isCoef || isLimitZRL) /* if last pixel, force process run zeros*/
                {
                    /* process run zeros*/
                    while(run)
                    {
                        for (j = numBestRuns - 1; j>=0; j--)
                        {
                            if (run >= bestRunsSorted[j] + 1)
                            {
                                k = run / (bestRunsSorted[j] + 1);
                                run %= (bestRunsSorted[j] + 1);
                                break;
                            }
                        }
                        if (j >= 0)
                        {
                            vx_int32 n = bestRunsSorted[j];
                            size = 8;
                            for (m = 0; m<SET_1_SIZE; m++)
                            {
                                if (best2Runs[m] == n)
                                {
                                    size = 0;
                                    break;
                                }
                            }
                            if (size == 8)
                            {
                                for (m = 0; m<numBestRuns - SET_1_SIZE; m++)
                                    if (outBest2Run[m] == n)
                                        break;

                            }
                            j = invSizeOrder[size];
                            while(k--){/*k of the runs*/
                                code = huffCode[j];
                                if (sizeCodeLen[j] % 2 == 0)/*huffCode length = 2 or 4, pack 1 bit to 3 or 5*/
                                    code |= ((m&1)<<(sizeCodeLen[j]) );
                                codeSymbol[pos].stageCode[0] = code & 0x07;
                                codeSymbol[pos].bitLength[0] = 3;

                                if (sizeCodeLen[j] > 3)
                                {
                                    codeSymbol[pos].stageCode[1] = code >> 3;
                                    codeSymbol[pos].bitLength[1] = 2;
                                }
                                if (size == 0 )
                                {/*The m already code*/
                                    codeSymbol[pos].bitLength[2] = (sizeCodeLen[j]%2 == 0) ? 0:1;
                                    codeSymbol[pos].stageCode[2] = m; /*If (sizeCodeLen[j]%2 == 0), should be m/2, but 0 bit output, doesn't matter*/
                                }
                                else if (sizeCodeLen[j]%2 == 0)
                                {
                                    codeSymbol[pos].stageCode[2] = m/2;
                                    codeSymbol[pos].bitLength[2] = runSet2Bits - 1;
                                }
                                else
                                {
                                    codeSymbol[pos].stageCode[2] = m;
                                    codeSymbol[pos].bitLength[2] = runSet2Bits;
                                }

                                OutputAt(x, &kernelBufferPtr, &bitOffset, codeSymbol);
                                x++;
                                /* update pos*/
                                pos = (x) % (3*THROUGHPUT);
                            }
                        }
                    }
                    /*process current pixel*/
                    if (coef)
                    {
                        vx_int8 ch;
                        coef>>=(wb->huffmanConfig[index].bit16Flag * 8);
                        if(coef<0x80)
                            coef+=wb->huffmanConfig[index].bit16Flag; /*Since we have to encode zero (run-length only handle 16-bits zero, 8 bit zero should use Size code*/
                        ch = (vx_int8)coef;
                        coef = abs(ch);
                        m = coef*2 + (ch<0?1:0);
                        for (j = 6; j>=0; j--)
                        {
                            if (coef>=(1<<j))
                            {
                                break;
                            }
                        }
                        size = j+1;
                        if (invSizeOrder[size]==8)
                        { /*The minimum freq, we out the raw data*/
                            size = 7;
                        }
                        if (size == 7)
                        {
                            m = ch&0xff;
                            if(wb->huffmanConfig[index].bit16Flag)
                                m = (coef16 >> 8) & 0xff;
                        }

                        j = invSizeOrder[size];
                        code = huffCode[j];
                        if (sizeCodeLen[j]%2 == 0)/*huffCode length = 2 or 4, pack 1 bit to 3 or 5*/
                            code |= ((m&1) << (sizeCodeLen[j]));
                        codeSymbol[pos].stageCode[0] = code & 0x07;
                        codeSymbol[pos].bitLength[0] = 3;

                        if (sizeCodeLen[j] > 3)
                        {
                            codeSymbol[pos].stageCode[1] = code >> 3;
                            codeSymbol[pos].bitLength[1] = 2;
                        }

                        if (size == 7)
                        {
                            size = 8;
                        }
                        /*Size = 0 mean 0&0xff, needs 1 bit actually*/
                        if (size == 0)
                            size = 1;
                        if (sizeCodeLen[j]%2 == 0)
                        {
                            codeSymbol[pos].stageCode[2] = m/2;  /*last bit was coded*/
                            codeSymbol[pos].bitLength[2] = size - 1;
                        }
                        else
                        {
                            codeSymbol[pos].stageCode[2] = m;
                            codeSymbol[pos].bitLength[2] = size;
                        }
                        if (wb->huffmanConfig[index].bit16Flag)
                        {
                            codeSymbol[pos].stageCode[2] = codeSymbol[x%(3*THROUGHPUT)].stageCode[2]* 256 + (coef16 & 0xff);
                            codeSymbol[pos].bitLength[2] += 8;
                        }
                        OutputAt(x, &kernelBufferPtr, &bitOffset, codeSymbol);
                        x++; /*counter*/
                    }
                    else if (!isCoef || (dataRemainingOfCore != 0x0) || isLimitZRL)
                    {
                        /*those non-coef count like bias, num_of_vz & z_offset couldn't skip by zero-run as HW requres,
                        need also encode those 0 to huffman code,
                        in ZRL path 8-bit, 0 belong to group7, -128~127, residue bit length is 8, residue code = 0*/
                        size = 7;
                        j = invSizeOrder[size];
                        code = huffCode[j];

                        codeSymbol[pos].stageCode[0] = code & 0x07;
                        codeSymbol[pos].bitLength[0] = 3;

                        if (sizeCodeLen[j] > 3)
                        {
                            codeSymbol[pos].stageCode[1] = code >> 3;
                            codeSymbol[pos].bitLength[1] = 2;
                        }

                        if (size == 7)
                        {
                            size = 8;
                        }

                        if (sizeCodeLen[j] % 2 == 0)
                        {
                            codeSymbol[pos].stageCode[2] = 0;  /*last bit was coded*/
                            codeSymbol[pos].bitLength[2] = size - 1;
                        }
                        else
                        {
                            codeSymbol[pos].stageCode[2] = 0;
                            codeSymbol[pos].bitLength[2] = size;
                        }
                        if (wb->huffmanConfig[index].bit16Flag)
                        {
                            codeSymbol[pos].stageCode[2] = codeSymbol[x%(3*THROUGHPUT)].stageCode[2]* 256 + (coef16 & 0xff);
                            codeSymbol[pos].bitLength[2] += 8;
                        }
                        OutputAt(x, &kernelBufferPtr, &bitOffset, codeSymbol);
                        x++; /*counter*/
                    }
                }
            }

            if(dataRemainingOfCore == 0)
            {
                vx_uint32 dummyCount = (x & 0x1)? 5: 4;
                /*Hit the end bitsteam of core, add dummy stage process the next*/
                vxmASSERT(setDummy == vx_true_e);
                addDummy(x, &kernelBufferPtr, &bitOffset, codeSymbol, dummyCount, dummyStage, dummyBitLength);
                break;
            }
        }

        /* Go back to update kernelStreamSize. */
        kernelStreamSize = (vx_uint32)((vx_uint8 *)kernelBufferPtr - kernelStreamBasePtr);
        writeBits(&kernelStreamSizePtr, &streamSizeBitOffset, kernelStreamSize*8 + bitOffset, 32);

        /* pad 0 at the end of core*/
        packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);
        kernelStreamSize = (vx_uint32)((vx_uint8 *)kernelBufferPtr - kernelStreamBasePtr);
        /*With RTL bug fix, full cache mode size needn't using maxSize * coreCount*/
        wb->slice_array[index].kernel_stream_full_cache_size += kernelStreamSize;

        if (maxKernelStreamSizePerCore < kernelStreamSize)
            maxKernelStreamSizePerCore = kernelStreamSize;

        compressionRatio = (vx_float64)kernelStreamSize / (vx_float64)(dataSizeOfCore * weightSize);
        if (wb->max_per_core_compression_ratio < compressionRatio)
            wb->max_per_core_compression_ratio = compressionRatio;

        prev = 0;
    }
    vxmASSERT(reorderStreamAllCount == reorderStreamCheckCount); /*Check if the data count is the same*/

    /*Add non compressed biases & Z_OFFSET & perFilterPostMul & perFilterPostShift after huffman bit stream*/
    kernelStreamBasePtr = (vx_uint8*)kernelBufferPtr;
    for (filterIndex = 0; filterIndex < filterTotalCount; filterIndex++)
    {

        if (!hasNoBias)
        {
            vx_uint32 biasData = 0;
            vx_uint32 realFilterIndex;

            if (context->options.enableNonZeroBalance &&
                !hasNoZOffset &&
                !wb->wb_base->hw_depth_wise &&
                realVzIndex != VX_NULL)
                realFilterIndex = realVzIndex[filterIndex];
            else
                realFilterIndex = filterIndex;

            if (bias_base_ptr)
                biasData = *(bias_base_ptr + realFilterIndex);
            else
                biasData = 0;

            /* NewBias = sum((Coef[i] - CoefZP) * - InZP) + Bias*/
            if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT)
                && input_zp != 0
                && weight_format == VX_TYPE_UINT8)
            {
                biasData -= weightsMinusZP[realFilterIndex].sum;
            }

            if (weight_format == VX_TYPE_UINT8)
                biasData += coefSum[realFilterIndex] * 128;

            if (weight_format == VX_TYPE_INT16)
            {
                vx_int64 bias64 = 0;
                vx_int64 sum64 = 0;
                vx_int32 bias32;
                vx_int16 bias16;

                if ((vx_int32)biasData < 0)
                {
                    bias64 = ~0;
                    bias64 = (bias64 >> 32) << 32;
                    bias64 = bias64 | (vx_int64)biasData;
                }
                else
                    bias64 = (vx_int64)biasData;

                if (coefSum[realFilterIndex] < 0)
                {
                    sum64 = ~0;
                    sum64 = (sum64 >> 32) << 32;
                    sum64 = sum64 | (vx_int64)coefSum[realFilterIndex];
                }
                else
                    sum64 = (vx_int64)coefSum[realFilterIndex];

                bias64 += sum64 * 128;
                bias32 = (vx_int32)bias64;
                writeBits(&kernelBufferPtr, &bitOffset, bias32, 32);
                bias16 = (vx_int16)(bias64 >> 32);
                writeBits(&kernelBufferPtr, &bitOffset, (vx_int32)bias16, 16);
            }
            else
                writeBits(&kernelBufferPtr, &bitOffset, biasData, biasBitSize);
        }

        if (hasNNPerFilterPostMultiply)
        {
            vx_uint32 postMul = 0;
            vx_uint32 postShift = 0;
            vxmASSERT(post_mul != VX_NULL && post_shift != VX_NULL);

            postMul = *(post_mul + filterIndex);
            postShift = *(post_shift + filterIndex);

            writeBits(&kernelBufferPtr, &bitOffset, postMul, 15);
            writeBits(&kernelBufferPtr, &bitOffset, postShift, 7);
            /*unused zero for 10 bit*/
            writeBits(&kernelBufferPtr, &bitOffset, 0, 10);
        }

        if (!hasNoZOffset)
        {
            vx_uint32 offsetValue;
            vx_uint32 realFilterIndex;

            if (context->options.enableNonZeroBalance &&
                !wb->wb_base->hw_depth_wise &&
                realVzIndex != VX_NULL)
                realFilterIndex = realVzIndex[filterIndex];
            else
                realFilterIndex = filterIndex;

            if (z_offset > 0)
                offsetValue = (vx_uint32)z_offset * realFilterIndex;
            else if (output_size > 0)
                offsetValue = output_final_x * output_final_y * output_size * realFilterIndex;
            else
                offsetValue = output_final_x * output_final_y * weightSize * realFilterIndex;

            writeBits(&kernelBufferPtr, &bitOffset, offsetValue, NN_Z_POSITION_OFFSET_BITS_VIP_V7);
        }
        else if (hasNNPerFilterPostMultiply && hasNNPreLU)
        {
            /*per PRD, prelu only enable when hasNoZOffset == 1 && hasNNPerFilterPostMultiply == 1*/
            vx_int32 negPostMul = 0;
            vx_int32 negPostShift = 0;
            vx_float32 alphaValue = 0;

            if (alphaBase != VX_NULL)
            {
                vx_uint32 uintAlpha;
                vx_int32 exp;

                alphaValue = vxnneGetDataExt(alphaFormat, alphaQuantFormat, filterIndex, (vx_uint8_ptr)alphaBase, alphaFP, alphaZP, alphaScale);

                uintAlpha = *((vx_uint32*)(&alphaValue));
                exp = ((uintAlpha >> 23) & 0xff) - 127;
                /*negPostShift range is [-63, 64]*/
                if (exp > 64)
                    exp = 64;
                else if (exp < -63)
                    exp = -63;

                negPostShift = exp + 63; /*exp = shift - (2^7 - 1) */
                negPostMul = (uintAlpha & 0x7FFFFF) >> 8; /* negMultiply only has 15 bit, using high 15-bit of alpha's mantissa*/
            }

            writeBits(&kernelBufferPtr, &bitOffset, negPostMul, 15);
            writeBits(&kernelBufferPtr, &bitOffset, negPostShift, 7);
            /*unused zero for 10 bit*/
            writeBits(&kernelBufferPtr, &bitOffset, 0, 10);
        }
    }
    /*Also align to 64 byte for post processing stream*/
    packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);
    kernelStreamSize = (vx_uint32)((vx_uint8 *)kernelBufferPtr - kernelStreamBasePtr);
    wb->slice_array[index].kernel_stream_full_cache_size += kernelStreamSize;

    if (maxKernelStreamSizePerCore < kernelStreamSize)
        maxKernelStreamSizePerCore = kernelStreamSize;
    /*Per HW, post processing stream size is needed in SRAM, when partial mode, need be noted as one core*/
    wb->slice_array[index].kernel_align_stream_size = (vx_size)(maxKernelStreamSizePerCore * (usedCoreCount + 1));

exit:
    if (coefSum)
        vxFree(coefSum);
    if (weightsMinusZP)
        vxFree(weightsMinusZP);
    if (reorderStream)
        vxFree(reorderStream);
    if (reorderStreamPerCoreCount)
        vxFree(reorderStreamPerCoreCount);
    if (nonCoefIndex)
        vxFree(nonCoefIndex);
    if (realVzIndex)
        vxFree(realVzIndex);
    if (lastVzGroupIndex)
        vxFree(lastVzGroupIndex);

    return;
}

#define DUMP_TP_ENC 0

void reorderTPKernelBufferHuffman(
    vx_weights_biases_parameter wb,
    vx_uint32 weight_z,
    vx_uint32 filter_count,
    vx_uint32 total_weight_z,
    vx_enum  weight_format,
    vx_uint8_ptr reorder_stream,
    vx_uint8_ptr weight_base_ptr,
    vx_uint8* postMul,
    vx_uint8* postShift
    )
{
    vx_enum weightFomat             = weight_format;
    vx_uint32 weightSize            = (vx_uint32)vxDataType_GetSize((vx_type_e)weightFomat);
    vx_uint32 sliceCount            = weight_z;
    vx_uint32 filterSize            = total_weight_z * weightSize;
    vx_uint32 filterCount           = filter_count;
    vx_uint8* kernelDataPtr         = VX_NULL;
    vx_uint8* kernelBufferInt8Ptr   = reorder_stream;
    vx_uint16* kernelBufferInt16Ptr = (vx_uint16*)reorder_stream;
    vx_uint32 filterIndex, sliceIndex;
#if DUMP_TP_ENC
    static vx_uint32 n = 0;
    vx_char fileName[128];
    FILE * pfile1 = NULL;
    sprintf(fileName, "%s_%d.txt", "reordered_kernel", n++);

    pfile1 = fopen(fileName, "wt");
#endif

    for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
    {
        /* add slices of every filter*/
        kernelDataPtr = weight_base_ptr + weightSize * sliceIndex;
        for (filterIndex = 0; filterIndex < filterCount; filterIndex++)
        {
            vx_uint32 weight = 0;

            if (weightFomat == VX_TYPE_INT8)
                weight = *((vx_int8 *)kernelDataPtr);
            else if (weightFomat == VX_TYPE_UINT8)
                weight = *((vx_uint8 *)kernelDataPtr);
            else
                weight = *((vx_uint16 *)kernelDataPtr);
            kernelDataPtr += filterSize;

            if (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8)
            {
                *kernelBufferInt8Ptr = (vx_uint8)weight;
                kernelBufferInt8Ptr ++;
#if DUMP_TP_ENC
                if(pfile1 != NULL)
                {
                    fprintf(pfile1, "kz:%d, vz:%d, coef:%d\n", sliceIndex, filterIndex, weight);
                }
#endif
            }
            else
            {
                *kernelBufferInt16Ptr = (vx_uint16)weight;
                kernelBufferInt16Ptr ++;
            }
        }
    }

#if DUMP_TP_ENC
    if(pfile1 != NULL)
    {
        fclose(pfile1);
    }
#endif
}

void analysisTPStreamForHuffman(
    vx_weights_biases_parameter wb,
    vx_uint32 sliceCount,
    vx_uint32 filterCount,
    vx_uint8* reorderStream,
    vx_uint32* invSizeOrder,
    vx_uint32 index
    )
{
    vx_uint8  * pBS_U08      = (unsigned char * )reorderStream;
    vx_uint16 * pBS_U16      = (unsigned short *)reorderStream;
    vx_enum weightFormat = wb->wb_base->weights_data_format;
    vx_uint32 weightSize = (vx_uint32)vxDataType_GetSize((vx_type_e)weightFormat);
    vx_uint8 bit16Flag   = (weightFormat == VX_TYPE_INT8 || weightFormat == VX_TYPE_UINT8) ? 0 : 1;
    vx_uint32 kernelDataCount  = sliceCount * filterCount;
    vx_uint32 kernelDataBytes  = kernelDataCount * weightSize;
    vx_uint32 dataCountToPross = kernelDataCount;

    vx_int32 i, j, k, bias;
    vx_int32 x=0;
    vx_int32 runZeros[256], run; /*The histogram of how many runs*/
    vx_int32 histo[256], sizeHisto[9] = {0,0,0,0,0,0,0,0,0};
    vx_int32 sizeOrder[]  = {0,1,2,3,4,5,6,7,8,};
    vx_int32 codingType   = HUFFMAN_CODING_TYPE_RUN_LEN; /* defalt RZL*/
    vx_int32 prevEncode   = 0, prev = 0, prevHisto[256], prevRunZeros[256], prevRun; /*previous pixel*/
    vx_float32 p, entropy = 0.f, prevEntropy = 0.f;
    vx_int32 sliceIndex = 0, accumSize = 0x0;

    /* this variable indicate DataAccumCount when Start of core*/
    vx_int32 *accumCountEndOfCore = (vx_int32 *)vxAllocateAndZeroMemory(sliceCount * sizeof(vx_int32));

    if (accumCountEndOfCore == VX_NULL)
    {
        vxError("analysisKernelStreamForHuffman: OUT OF MEMORY\n");
        goto exit;
    }

    for (i = 0; i < 256; i++)
    {
        runZeros[i] = histo[i] = 0;
        prevHisto[i] = prevRunZeros[i] = 0;
    }


    for (sliceIndex = 0; sliceIndex != (vx_int32)sliceCount; sliceIndex++)
    {
        accumSize += filterCount;
        accumCountEndOfCore[sliceIndex] = accumSize - 1;
    }

    prevRun = run = 0;
    sliceIndex = 0;
    while(dataCountToPross--)
    {
        vx_uint32 endOfCore = (dataCountToPross + accumCountEndOfCore[sliceIndex] == kernelDataCount - 1) ? 1 : 0;
        if(endOfCore)
        {
            sliceIndex++;
        }

        if (!bit16Flag)
        {
            i = *(pBS_U08++);
        }
        else
        {
            i = *(pBS_U16++);
            if (weightFormat == VX_TYPE_FLOAT16)
            {
                i = (i & 0x7fff) * 2 + i/(1<<15); /*(i&0x8000)/128 + (i&0x7f00)*2 + (i&0xff);*/
            }
        }

        if (i == 0) run++;
        if ((run == sizeof(runZeros)/sizeof(int)) || i || endOfCore)
        {
            if (run)
            {
                runZeros[run - 1]++;
                run = 0;
            }
        }
        histo[i>>(bit16Flag*8)]++;
        /*Get statistic of prediction from previous pixel*/
        if(bit16Flag == 0)
        {
            j = i;
            i = (i-prev)&0xff;
            prev = j;
            if (i == 0) prevRun++;
            if (prevRun == sizeof(prevRunZeros)/sizeof(int) || i || endOfCore)
            {
                if (prevRun){
                    prevRunZeros[prevRun - 1]++;
                    prevRun = 0;
                }
            }
            prevHisto[i]++;
        }

        /* prev encode should be truncated between cores, when analysis, reset prev.*/
        if (endOfCore)
        {
            prev = 0x0;
        }
    }
    if (run)
    { /*last run*/
        runZeros[run - 1]++;
        run = 0;
    }
    if (prevRun)
    { /*last run*/
        prevRunZeros[prevRun - 1]++;
        prevRun = 0;
    }

    j = histo[0];
    bias = 0;

    for (i = 1; i < 256; i++)
    {
        if (histo[i] > j)
        {
            j = histo[i];
            bias = i;
        }
    }
    if (j*3 < histo[0]*4 && bit16Flag == 0)
        bias = 0;

    /*16 bits, we only get the high bit bias, we need to get the low bit bias*/
    if (bit16Flag)
    {
        pBS_U16 = (unsigned short *)reorderStream;
        dataCountToPross = kernelDataCount;
        while(dataCountToPross--)
        {
            i = *(pBS_U16);
            pBS_U16++;
            if (weightFormat == VX_TYPE_FLOAT16)
                i = (i&0x7fff)*2 + i/(1<<15);

            if ((i>>8) == bias)
                prevHisto[i&0xff]++;
        }

        j = prevHisto[0];
        k = 0;

        for (i = 1; i < 256; i++)
        {
            if (prevHisto[i] > j)
            {
                j = prevHisto[i];
                k = i;
            }
        }
        bias = bias*256+k;
    }

    x = 0;
    for (i = 0; i<256; i++)
    {
        p = histo[i] / (vx_float32)(kernelDataCount);
        if (histo[i])
            entropy += -p*(vx_float32)log(p)/(vx_float32)log(2.0f);

        p =  prevHisto[i] / (vx_float32)kernelDataBytes;
        if (prevHisto[i])
            prevEntropy += - p*(vx_float32)log(p)/(vx_float32)log(2.0f);
    }

    if (bit16Flag == 0)
    {
        if (prevEntropy < entropy || 2*prevHisto[0] + prevHisto[255] + prevHisto[1] > 3*histo[bias] + histo[(bias+1)&0xff] + histo[(bias-1)&0xff])
        {
            entropy = prevEntropy;
            prevEncode = 1; /*Using prevEncode, prediction from previous pixel*/
            for (i = 0; i < 256; i++)
            {
                histo[i] = prevHisto[i];
                runZeros[i] = prevRunZeros[i];
            }
            bias = 0;
        }
    }
    prevEntropy = entropy;

    if (bias != 0)
    {
        for (i = 0; i < 256; i++)
        {
            histo[i] = runZeros[i] = 0;
        }
        dataCountToPross = kernelDataCount;
        pBS_U08 = (vx_uint8 * )reorderStream;
        pBS_U16 = (vx_uint16 *)reorderStream;
        while(dataCountToPross--)
        {
            if (bit16Flag == 0)
            {
                i = *(pBS_U08++);
            }
            else
            {
                i = *(pBS_U16++);
                if (weightFormat == VX_TYPE_FLOAT16)
                {
                    i = (i&0x7fff)*2 + i/(1<<15);
                }
            }

            i = (i - bias);
            if (i == 0) run++;
            if (run == sizeof(runZeros) / sizeof(vx_int32) || i)
            {
                if(run)
                {
                    runZeros[run - 1]++;
                    run = 0;
                }
            }

            histo[(i>>(bit16Flag*8))&0xff]++;
        }
        if (run)
        { /*last run*/
            runZeros[run - 1]++;
            run = 0;
        }
    }

    /*Get all the runZeros present. supposed to be histo[0], but, for 16-bit, we only count the runs for double zeros.*/
    for (i = 0; i < 256; i++)
    {
        run += runZeros[i] * (i+1);
    }

    if (bit16Flag == 0 && ((unsigned int)histo[0] <= kernelDataBytes / 8 || histo[0] < (histo[1] + histo[255]) * 3/4 || entropy >= 5.25f) && entropy > 3.5f )
    {
         codingType = HUFFMAN_CODING_TYPE_NON_RUN_LEN;
    }
    if (bit16Flag && run < (histo[1] + histo[255]) / 4)
    {
         codingType = HUFFMAN_CODING_TYPE_NON_RUN_LEN;
    }
    if (bit16Flag && entropy > 5.25f)
        codingType = 0; /*Force to test run-length*/


    if (codingType == HUFFMAN_CODING_TYPE_RUN_LEN)
    {
        FindBest2PlusRunSets(runZeros, sizeof(runZeros)/sizeof(int), (kernelDataBytes >> bit16Flag) - run);
    }

    if (codingType == HUFFMAN_CODING_TYPE_NON_RUN_LEN)
    {
        j = 0;
        for (i = 0; i < 8;i++){
            for (; j<(1<<i); j++){
                sizeHisto[i] += histo[j] + histo[j^0xff];
            }
        }

        for (i = 0; i < 7; i++){
            vx_int32 maxFreq = 0, maxPos = i;
            for (j = i; j<8; j++){
                if (maxFreq<sizeHisto[j])
                {
                    maxFreq = sizeHisto[j];
                    maxPos = j;
                }
            }
            if (maxPos > i)
            {
                j = sizeHisto[maxPos];
                sizeHisto[maxPos] = sizeHisto[i];
                sizeHisto[i] = j;

                j = sizeOrder[maxPos];
                sizeOrder[maxPos] = sizeOrder[i];
                sizeOrder[i] = j;
            }
        }
        for (i = 0; i < 8; i++)
        {
            invSizeOrder[sizeOrder[i]] = i;
        }
        entropy = 0.f;
        j = 0;
        for (i = 0; i < 8; i++)
        {
            p = sizeHisto[i]/(vx_float32)kernelDataBytes;
            if(p>0.f)
                entropy += -p*(vx_float32)log(p)/(vx_float32)log(2.0f);
            j += sizeHisto[i]*sizeCodeLen[i];
        }
    }

    if (codingType == HUFFMAN_CODING_TYPE_RUN_LEN)
    {
        j = 1;
        if (bit16Flag)
           histo[0] -= run; /*The zeros not runs*/

        run = 0;
        for (i = 1; i < 8; i++)
        {
            for (; j < (1<<i); j++)
            {
                sizeHisto[i] += histo[j - bit16Flag] + histo[(-j)&0xff];
            }
        }
        sizeHisto[7] += histo[0x80];/*128 haven't put in yet.*/

        /*Get the frequency of run-length.*/
        for (i = 0; i < numBestRuns; i++)
        {
            if (freq[i] < 0) /*The set-1*/
                sizeHisto[0] += abs(freq[i]);
            else /*Set-2*/
                sizeHisto[8] += abs(freq[i]);
        }

        k = sizeHisto[1];
        for (i = 2; i < 7; i++)
        {
            if (sizeHisto[i] < k)
                k = sizeHisto[i];
        }

        /*avoid the run-length be the last*/
        if (sizeHisto[0] <= k)
            sizeHisto[0] = k+2;
        if (sizeHisto[8] <= k)
            sizeHisto[8] = k+2;

        if (sizeHisto[7] <= k) /*The 7 is the escape code, shouldn't be the last (the last will be mergy to 7).*/
            sizeHisto[7] = k+1;


         /*sorting the sizeHisto*/
         for (i = 0; i<8; i++)
         {
            vx_int32 maxFreq = 0, maxPos = i;
            for (j = i; j < 9; j++)
            {
                if(maxFreq < sizeHisto[j])
                {
                    maxFreq = sizeHisto[j];
                    maxPos = j;
                }
            }
            if (maxPos > i)
            {
                j = sizeHisto[maxPos];
                sizeHisto[maxPos] = sizeHisto[i];
                sizeHisto[i] = j;

                j = sizeOrder[maxPos];
                sizeOrder[maxPos] = sizeOrder[i];
                sizeOrder[i] = j;
            }
        }

        if(sizeOrder[7]%8 == 0 && sizeOrder[6]%8 > 0)
        { /*The run-length cannot be the escape code. swap to size 6*/
            j = sizeOrder[6];
            sizeOrder[6] = sizeOrder[7];
            sizeOrder[7] = j;

            j = sizeHisto[6];
            sizeHisto[6] = sizeHisto[7];
            sizeHisto[7] = j;

        }
        else if(sizeOrder[7]%8 == 0 && sizeOrder[6]%8 == 0)
        { /*The run-length cannot be the escape code. swap to size 6*/
            /*5,7,6*/
            j = sizeOrder[6];
            sizeOrder[6] = sizeOrder[7];
            sizeOrder[7] = j;

            j = sizeHisto[6];
            sizeHisto[6] = sizeHisto[7];
            sizeHisto[7] = j;
            /*6,7,5*/
            j = sizeOrder[5];
            sizeOrder[5] = sizeOrder[7];
            sizeOrder[7] = j;

            j = sizeHisto[5];
            sizeHisto[5] = sizeHisto[7];
            sizeHisto[7] = j;
        }

        for(i = 0; i < 9; i++)
            invSizeOrder[sizeOrder[i]] = i;

        entropy = 0.f;
        j = 0;
        x = 0;
        for(i = 0; i < 9; i++)
        {
            x += sizeHisto[i];
        }
        for(i = 0; i < 8; i++)
        {
            p = sizeHisto[i] / (vx_float32)x;
            if (p > 0.f)
                entropy += -p*(vx_float32)log(p)/(vx_float32)log(2.0f);
            j += sizeHisto[i]*sizeCodeLen[i];
        }
    }


    /* update compression header*/
    wb->huffmanConfig[index].preEncode = (vx_uint8)prevEncode;
    wb->huffmanConfig[index].bit16Flag  = bit16Flag;
    wb->huffmanConfig[index].fp16Flag   = (weightFormat == VX_TYPE_FLOAT16) ? 1 : 0;
    wb->huffmanConfig[index].reserved   = 0x0; /* must be zero*/
    wb->huffmanConfig[index].version    = 0x1; /* 4'b0001 for version 1.0*/
    wb->huffmanConfig[index].runLenTableSize = (codingType == HUFFMAN_CODING_TYPE_NON_RUN_LEN) ? 0: (vx_uint8)numBestRuns;

    /* runlenTable 8 * 18 = 144*/
    for(i = 0; i<SET_1_SIZE; i++)
    {
        wb->huffmanConfig[index].runLenTable[i] = (vx_uint8)best2Runs[i];
    }
    for(; i< numBestRuns; i++)
    {
        wb->huffmanConfig[index].runLenTable[i] = (vx_uint8)outBest2Run[i - SET_1_SIZE];
    }
    for(i = 0; i<8; i+=2)
    {
        wb->huffmanConfig[index].mapToHuffman[i/2] = (vx_uint8)(sizeOrder[i+1]*16 + sizeOrder[i]);
    }
    wb->huffmanConfig[index].avgBias = (vx_uint8)bias;
    wb->huffmanConfig[index].reserved = 0x0; /* reserved 16, must be zero*/

exit:
    if (accumCountEndOfCore != VX_NULL)
        vxFree(accumCountEndOfCore);
}

void calcTPKernelBufferSizeHuffman(
    vx_weights_biases_parameter wb,
    vx_uint32 weight_z,
    vx_uint32 filter_count,
    vx_uint32 total_weight_z,
    vx_enum  weight_format,
    vx_uint32 skip_value,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32 index
    )
{
    vx_uint32 invSizeOrder[MAX_SIZE_HISTO_COUNT] = {0};
    vx_int32 coef = 0, tmp = 0, prev = 0;

    vx_uint32 weightSize       = (vx_uint32)vxDataType_GetSize((vx_type_e)weight_format);

    vx_uint32 sliceCount            = weight_z;
    vx_uint32 filterCount           = filter_count;
    vx_uint32 kernelStreamSize      = 0;
    vx_uint32 biasSize              = 0;
    vx_uint32 kernelSize            = 0;
    vx_uint32 kernelBitSize         = 0;

    vx_uint32 nonZeroCoefCount      = 0;
    vx_uint32 rsvCoefCount          = 0;

    vx_uint8_ptr kernelDataPtr      = VX_NULL;

    vx_uint32 sliceIndex = 0;

    vx_uint32 bit16Flag = (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8) ? 0 : 1;

    vx_uint32 reorderStreamAllCount = sliceCount * filterCount;
    vx_uint32 reorderStreamSize = reorderStreamAllCount * weightSize;

    vx_uint8* reorderStream = VX_NULL;
    vx_uint32 runSet2Bits = 1;

    wb->slice_array[index].kernel_stream_size = 0;
    reorderStream = (vx_uint8 *)vxAllocateAndZeroMemory(reorderStreamSize);

    if (!reorderStream)
    {
        vxError("fillinKernelBufferHuffman: OUT OF MEMORY");
        goto exit;
    }

    reorderTPKernelBufferHuffman(wb, weight_z, filter_count, total_weight_z, weight_format,
        reorderStream, weight_base_ptr, VX_NULL, VX_NULL);

    analysisTPStreamForHuffman(wb, sliceCount, filterCount, reorderStream, invSizeOrder, index);

    kernelBitSize += 16;
    kernelBitSize += 8 * MAX_RUNLEN_SIZE;
    kernelBitSize += 8 * LOG_RUN_SIZE1;
    kernelBitSize += 32;
    /*TP huffman update 14 bit to save kernel stream bit size*/
    kernelBitSize += 14 * sliceCount;

    kernelSize = (kernelBitSize % 8 == 0) ? kernelBitSize / 8 : kernelBitSize / 8 + 1;
    /*align to 64 byte*/
    kernelSize = (kernelSize + 63) & 0xFFFFFFC0;
    kernelBitSize = 0;

    biasSize = 4 * filterCount;
    /*align to 64 byte*/
    biasSize = (biasSize + 63) & 0xFFFFFFC0;
    kernelSize += biasSize;

    while(((wb->huffmanConfig[index].runLenTableSize - SET_1_SIZE)>>runSet2Bits) > 1)
    {
        runSet2Bits++;
    }

    kernelDataPtr = reorderStream;
    for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
    {
        vx_int32 run = 0x0;
        vx_uint32 dataSizeOfCore = filterCount;
        vx_uint32 dataRemainingOfCore = dataSizeOfCore;
        vx_int32 size = 0;
        vx_int32 code = 0;
        vx_int32 coef16 = 0;
        vx_int32 k = 0, j = 0, m = 0;

        while(dataRemainingOfCore --)
        {
            if (weight_format == VX_TYPE_INT8)
            {
                coef = *((vx_int8 *)kernelDataPtr);
                if (coef != (vx_int32)skip_value)
                    nonZeroCoefCount++;
            }
            else if (weight_format == VX_TYPE_UINT8)
            {
                coef = *((vx_uint8 *)kernelDataPtr);
                if (coef != (vx_int32)skip_value)
                    nonZeroCoefCount++;
            }
            else if (weight_format == VX_TYPE_INT16 ||
                weight_format == VX_TYPE_FLOAT16)
            {
                coef16 = *((vx_int16 *)kernelDataPtr);
                if (coef16 != (vx_int32)skip_value)
                    nonZeroCoefCount++;
            }
            else
            {
                /*Other format not suppoted now*/
                vxmASSERT(0);
            }
            kernelDataPtr += weightSize;

            if (bit16Flag == 0)
            {

                coef = (coef - wb->huffmanConfig[index].avgBias) & 0xFF;
                if (wb->huffmanConfig[index].preEncode)
                {
                    tmp = coef;
                    coef = (coef - prev) & 0xFF;
                    prev = tmp;
                }
            }
            else
            {

                if (wb->huffmanConfig[index].fp16Flag)
                {
                    coef16 = (coef16 & 0x7FFF) * 2 + coef16 / (1<<15);
                }
                coef16 -= wb->huffmanConfig[index].avgBias;
                if (wb->huffmanConfig[index].runLenTableSize == 0x0)
                    coef = (coef16 >> 8) & 0xFF; /*Non RZL*/
                else
                    coef = coef16 & 0xFFFF; /*RZL path*/
            }

            if (wb->huffmanConfig[index].runLenTableSize == 0x0) /* non RZL*/
            {
                tmp = (coef & 0x80)? (coef ^ 0xff): coef;
                for (size = 0; size<7; size++)
                {
                    if (tmp<(1<<size))
                        break;
                }
                k = invSizeOrder[size];
                code = huffCode[k];

                if (sizeCodeLen[k]%2 == 0)/*huffCode length = 2 or 4, pack 1 bit to 3 or 5*/
                    code |= ((coef>>7)<<(sizeCodeLen[k]) );
                kernelBitSize += 3;

                if (sizeCodeLen[k] > 3)
                {
                    kernelBitSize += 2;
                }

                if (size == 0) size = 1; /*Size = 0 mean 0&0xff, needs 1 bit actually*/
                if (sizeCodeLen[k]%2 == 0)
                {
                    kernelBitSize += (size - 1);
                }
                else
                {
                    kernelBitSize += size;
                }

                if (wb->huffmanConfig[index].bit16Flag)
                {/*pack the raw data of the lower 8 bits*/
                    kernelBitSize += 8;
                }
                rsvCoefCount++;
            }
            else /* RZL enable*/
            {
                if (coef == 0)
                {
                    run++;
                    if (dataRemainingOfCore != 0)
                    {
                        continue;
                    }
                }
                if (coef || (dataRemainingOfCore == 0)) /* if last pixel, force process run zeros*/
                {
                    /* process run zeros*/
                    while(run)
                    {
                        for (j = wb->huffmanConfig[index].runLenTableSize - 1; j>=0; j--)
                        {
                            if (run >= bestRunsSorted[j] + 1)
                            {
                                k = run / (bestRunsSorted[j] + 1);
                                run %= (bestRunsSorted[j] + 1);
                                break;
                            }
                        }
                        if (j >= 0)
                        {
                            vx_int32 n = bestRunsSorted[j];
                            size = 8;
                            for (m = 0; m<SET_1_SIZE; m++)
                            {
                                if (best2Runs[m] == n)
                                {
                                    size = 0;
                                    break;
                                }
                            }
                            if (size == 8)
                            {
                                for (m = 0; m < wb->huffmanConfig[index].runLenTableSize - SET_1_SIZE; m++)
                                    if (outBest2Run[m] == n)
                                        break;

                            }
                            j = invSizeOrder[size];
                            while(k--)
                            {/*k of the runs*/
                                kernelBitSize += 3;

                                if (sizeCodeLen[j] > 3)
                                {
                                    kernelBitSize += 2;
                                }
                                if (size == 0 )
                                {/*The m already code*/
                                    kernelBitSize += ((sizeCodeLen[j]%2 == 0) ? 0:1);
                                }
                                else if (sizeCodeLen[j]%2 == 0)
                                {
                                    kernelBitSize += (runSet2Bits - 1);
                                }
                                else
                                {
                                    kernelBitSize += runSet2Bits;
                                }
                                rsvCoefCount++;
                            }
                        }
                    }
                    /*process current pixel*/
                    if (coef)
                    {
                        vx_int8 ch;
                        coef>>=(wb->huffmanConfig[index].bit16Flag * 8);
                        if(coef<0x80)
                            coef+=wb->huffmanConfig[index].bit16Flag; /*Since we have to encode zero (run-length only handle 16-bits zero, 8 bit zero should use Size code*/
                        ch = (vx_int8)coef;
                        coef = abs(ch);
                        for (j = 6; j>=0; j--)
                        {
                            if (coef>=(1<<j))
                            {
                                break;
                            }
                        }
                        size = j+1;
                        if (invSizeOrder[size]==8)
                        { /*The minimum freq, we out the raw data*/
                            size = 7;
                        }

                        j = invSizeOrder[size];
                        kernelBitSize += 3;

                        if (sizeCodeLen[j] > 3)
                        {
                            kernelBitSize += 2;
                        }

                        if (size == 7)
                        {
                            size = 8;
                        }
                        /*Size = 0 mean 0&0xff, needs 1 bit actually*/
                        if (size == 0)
                            size = 1;
                        if (sizeCodeLen[j]%2 == 0)
                        {
                            kernelBitSize += (size - 1);
                        }
                        else
                        {
                            kernelBitSize += size;
                        }
                        if (wb->huffmanConfig[index].bit16Flag)
                        {
                            kernelBitSize += 8;
                        }
                        rsvCoefCount++;
                    }
                }
            }

            if(dataRemainingOfCore == 0)
            {
                /*Hit the end bitsteam of core, add dummy stage process the next*/
                kernelBitSize += 3;
                kernelBitSize += 2;
                kernelBitSize += 7;

                if (wb->huffmanConfig[index].bit16Flag)
                    kernelBitSize += 8;

                for (j = 0; j < 2*THROUGHPUT; j++)
                {
                    kernelBitSize += 3;
                }
                break;
            }
        }

        prev = 0;
        /* Go back to update kernelStreamSize. */
        kernelStreamSize = (kernelBitSize % 8 == 0) ? kernelBitSize / 8 : kernelBitSize / 8 + 1;
        /*align to 64 byte*/
        kernelStreamSize = (kernelStreamSize + 63) & 0xFFFFFFC0;
        kernelBitSize = 0;
        kernelSize += kernelStreamSize;
    }
    wb->slice_array[index].kernel_stream_size = kernelSize;
    wb->slice_array[index].non_zero_count = nonZeroCoefCount;
    wb->slice_array[index].reserve_weight_count = rsvCoefCount;
exit:
    if (reorderStream)
        vxFree(reorderStream);

    return;
}

void fillinTPKernelBufferHuffman(
    vx_weights_biases_parameter wb,
    vx_uint8* zero_run_len_bit_width,
    vx_uint32 weight_z,
    vx_uint32 filter_count,
    vx_uint32 total_weight_z,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_uint32 skip_value,
    vx_uint8_ptr wb_base_ptr,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr,
    vx_uint32 index
    )
{
    vx_uint32 invSizeOrder[MAX_SIZE_HISTO_COUNT] = {0};
    vx_int32 coef = 0, tmp = 0, prev = 0;

    vx_uint32 weightSize       = (vx_uint32)vxDataType_GetSize((vx_type_e)weight_format);
    vx_uint32 biasBitSize      = NN_INTEGER_BIAS_BITS_VIP_V7;
    vx_uint32 biasData         = 0;

    vx_uint32 sliceCount            = weight_z;
    vx_uint32 filterCount           = filter_count;
    vx_uint32 kernelStreamSize      = 1;
    vx_uint32 kernelSizePerCore     = 0;
    vx_uint32* kernelBufferPtr      = (vx_uint32*) wb_base_ptr;
    vx_uint8* kernelStartPtr        = wb_base_ptr;
    vx_uint32 bitOffset             = 0;
    vx_uint32* kernelStreamSizePtr  = kernelBufferPtr;
    vx_uint32 streamSizeBitOffset   = 0;
    vx_uint32 alignedOffset         = ((vx_uint32)(gctUINTPTR_T)kernelBufferPtr) & 0x3F;
    vx_uint8_ptr kernelDataPtr      = VX_NULL;
    vx_uint8_ptr kernelStreamBasePtr = VX_NULL;

    vx_uint32 i;
    vx_uint32 sliceIndex = 0;
    vx_uint32 filterIndex = 0;

    vx_uint32 bit16Flag = (weight_format == VX_TYPE_INT8 || weight_format == VX_TYPE_UINT8) ? 0 : 1;

    vx_uint32 reorderStreamAllCount = sliceCount * filterCount;
    vx_uint32 reorderStreamSize = reorderStreamAllCount * weightSize;

    vx_uint8* reorderStream = VX_NULL;
    vx_uint32 runSet2Bits = 1;
    CodeSymbol codeSymbol[THROUGHPUT * 3];

    vx_uint32 dummyStage[3] = {0};
    vx_uint32 dummyBitLength[3] = {0};
    vx_bool setDummy = vx_false_e;

    reorderStream = (vx_uint8 *)vxAllocateAndZeroMemory(reorderStreamSize);

    if (!reorderStream)
    {
        vxError("fillinKernelBufferHuffman: OUT OF MEMORY");
        goto exit;
    }

    reorderTPKernelBufferHuffman(wb, weight_z, filter_count, total_weight_z, weight_format,
        reorderStream, weight_base_ptr, VX_NULL, VX_NULL);

    analysisTPStreamForHuffman(wb, sliceCount, filterCount, reorderStream, invSizeOrder, index);

    /* Write huffman header*/
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].preEncode, 1);
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].bit16Flag, 1);
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].fp16Flag, 1);
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].reserved, 1);
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].version, 4);
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].runLenTableSize, 8);

    for(i = 0; i < MAX_RUNLEN_SIZE; i++)
    {
        writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].runLenTable[i], 8);
    }
    for(i = 0; i < LOG_RUN_SIZE1; i++)
    {
        writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].mapToHuffman[i], 8);
    }
    writeBits(&kernelBufferPtr, &bitOffset, wb->huffmanConfig[index].avgBias, 16);
    writeBits(&kernelBufferPtr, &bitOffset, 0, 16); /*reserved, must zero*/

    kernelStreamSizePtr = kernelBufferPtr;
    streamSizeBitOffset = bitOffset;

    /* Fill kernel stream size for each slice. */
    for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
    {
        writeBits(&kernelBufferPtr, &bitOffset, kernelStreamSize, 14);
    }
    packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);

    /* Fill bias for each filter. */
    for (filterIndex = 0; filterIndex < filterCount; filterIndex++)
    {
        biasData = bias_base_ptr == VX_NULL ? 0 : *(bias_base_ptr + filterIndex);
        writeBits(&kernelBufferPtr, &bitOffset, biasData, biasBitSize);
    }
    packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);


    for (i = 0; i < THROUGHPUT * 3; i++)
    {
        codeSymbol[i].stageCode[0] = codeSymbol[i].stageCode[1] = codeSymbol[i].stageCode[2] = 0;
        codeSymbol[i].bitLength[0] = codeSymbol[i].bitLength[1] = codeSymbol[i].bitLength[2] = 0;
    }

    while(((wb->huffmanConfig[index].runLenTableSize - SET_1_SIZE)>>runSet2Bits) > 1)
    {
        runSet2Bits++;
    }

    kernelDataPtr = reorderStream;
    for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
    {

        vx_uint32 x = 0x0;
        vx_int32 pos = 0x0;
        vx_int32 run = 0x0;
        vx_uint32 dataSizeOfCore = filterCount;
        vx_uint32 dataRemainingOfCore = dataSizeOfCore;
        vx_int32 size = 0;
        vx_int32 code = 0;
        vx_int32 coef16 = 0;
        vx_int32 k = 0, j = 0, m = 0;

        /* update position of each core's beginning of compressed kernel*/
        kernelStreamBasePtr = (vx_uint8*)kernelBufferPtr;

        memset(codeSymbol, 0, sizeof(CodeSymbol));
        memset(dummyStage, 0, (sizeof(vx_uint32) * 3));
        memset(dummyBitLength, 0, (sizeof(vx_uint32) * 3));
        setDummy = vx_false_e;

        while(dataRemainingOfCore --)
        {

            pos = (x) % (3*THROUGHPUT);

            if (weight_format == VX_TYPE_INT8)
                coef = *((vx_int8 *)kernelDataPtr);
            else if (weight_format == VX_TYPE_UINT8)
                coef = *((vx_uint8 *)kernelDataPtr);
            else if (weight_format == VX_TYPE_INT16 ||
                weight_format == VX_TYPE_FLOAT16)
                coef16 = *((vx_int16 *)kernelDataPtr);
            else
            {
                /*Other format not suppoted now*/
                vxmASSERT(0);
            }
            kernelDataPtr += weightSize;

            if (bit16Flag == 0)
            {

                coef = (coef - wb->huffmanConfig[index].avgBias) & 0xFF;
                if (wb->huffmanConfig[index].preEncode)
                {
                    tmp = coef;
                    coef = (coef - prev) & 0xFF;
                    prev = tmp;
                }
            }
            else
            {
                if (wb->huffmanConfig[index].fp16Flag)
                {
                    coef16 = (coef16 & 0x7FFF) * 2 + coef16 / (1<<15);
                }
                coef16 -= wb->huffmanConfig[index].avgBias;
                if (wb->huffmanConfig[index].runLenTableSize == 0x0)
                    coef = (coef16 >> 8) & 0xFF; /*Non RZL*/
                else
                    coef = coef16 & 0xFFFF; /*RZL path*/
            }

            if (wb->huffmanConfig[index].runLenTableSize == 0x0) /* non RZL*/
            {
                if (!setDummy)
                {
                    /*Set dummy all to 0,
                    in nonZRL path 8-bit, 0 belong to group0, -1~0, residue bit length is 1, residue code = 0*/
                    size = 0;
                    k = invSizeOrder[size];
                    code = huffCode[k];

                    dummyStage[0] = code & 0x07;
                    dummyBitLength[0] = 3;

                    if (sizeCodeLen[k] > 3)
                    {
                        dummyStage[1] = code >> 3;
                        dummyBitLength[1] = 2;
                    }

                    dummyStage[2] = 0;
                    if (size == 0) size = 1; /*Size = 0 mean 0&0xff, needs 1 bit actually*/
                    if (sizeCodeLen[k]%2 == 0)
                    {
                        dummyBitLength[2] = size - 1;
                    }
                    else
                    {
                        dummyBitLength[2] = size;
                    }

                    if (wb->huffmanConfig[index].bit16Flag)
                    {/*pack the raw data of the lower 8 bits*/

                        dummyBitLength[2] += 8;
                    }
                    setDummy = vx_true_e;
                }

                tmp = (coef & 0x80)? (coef ^ 0xff): coef;
                for (size = 0; size<7; size++)
                {
                    if (tmp<(1<<size))
                        break;
                }
                k = invSizeOrder[size];
                code = huffCode[k];

                if (sizeCodeLen[k]%2 == 0)/*huffCode length = 2 or 4, pack 1 bit to 3 or 5*/
                    code |= ((coef>>7)<<(sizeCodeLen[k]) );
                codeSymbol[pos].stageCode[0] = code & 0x07;
                codeSymbol[pos].bitLength[0] = 3;

                if (sizeCodeLen[k] > 3)
                {
                    codeSymbol[pos].stageCode[1] = code >> 3;
                    codeSymbol[pos].bitLength[1] = 2;
                }

                codeSymbol[pos].stageCode[2] = tmp &((1<<size) - 1 );
                if (size == 0) size = 1; /*Size = 0 mean 0&0xff, needs 1 bit actually*/
                if (sizeCodeLen[k]%2 == 0)
                {
                    codeSymbol[pos].bitLength[2] = size - 1;
                }
                else
                {
                    codeSymbol[pos].stageCode[2] <<= 1;
                    codeSymbol[pos].stageCode[2] |= (coef>>7); /*The sign bit*/
                    codeSymbol[pos].bitLength[2] = size;
                }

                if (wb->huffmanConfig[index].bit16Flag)
                {/*pack the raw data of the lower 8 bits*/

                    codeSymbol[pos].stageCode[2] = codeSymbol[x%(3*THROUGHPUT)].stageCode[2]* 256 + (coef16 &0xff);
                    codeSymbol[pos].bitLength[2] += 8;
                }
                OutputAt(x, &kernelBufferPtr, &bitOffset, codeSymbol);
                x++; /*counter*/
            }
            else /* RZL enable*/
            {
                if (!setDummy)
                {
                    /*Set dummy all to 0,
                    in ZRL path 8-bit, 0 belong to group7, -128~127, residue bit length is 8, residue code = 0*/
                    size = 7;
                    j = invSizeOrder[size];
                    code = huffCode[j];

                    dummyStage[0] = code & 0x07;
                    dummyBitLength[0] = 3;

                    if (sizeCodeLen[j] > 3)
                    {
                        dummyStage[1] = code >> 3;
                        dummyBitLength[1] = 2;
                    }

                    if (size == 7)
                    {
                        size = 8;
                    }

                    if (sizeCodeLen[j] % 2 == 0)
                    {
                        dummyStage[2] = 0;  /*last bit was coded*/
                        dummyBitLength[2] = size - 1;
                    }
                    else
                    {
                        dummyStage[2] = 0;
                        dummyBitLength[2] = size;
                    }
                    if (wb->huffmanConfig[index].bit16Flag)
                    {
                        dummyStage[2] = 0;
                        dummyBitLength[2] += 8;
                    }

                    setDummy = vx_true_e;
                }

                if (coef == 0)
                {
                    run++;
                    if (dataRemainingOfCore != 0)
                    {
                        continue;
                    }
                }
                if (coef || (dataRemainingOfCore == 0)) /* if last pixel, force process run zeros*/
                {
                    /* process run zeros*/
                    while(run)
                    {
                        for (j = wb->huffmanConfig[index].runLenTableSize - 1; j>=0; j--)
                        {
                            if (run >= bestRunsSorted[j] + 1)
                            {
                                k = run / (bestRunsSorted[j] + 1);
                                run %= (bestRunsSorted[j] + 1);
                                break;
                            }
                        }
                        if (j >= 0)
                        {
                            vx_int32 n = bestRunsSorted[j];
                            size = 8;
                            for (m = 0; m<SET_1_SIZE; m++)
                            {
                                if (best2Runs[m] == n)
                                {
                                    size = 0;
                                    break;
                                }
                            }
                            if (size == 8)
                            {
                                for (m = 0; m < wb->huffmanConfig[index].runLenTableSize - SET_1_SIZE; m++)
                                    if (outBest2Run[m] == n)
                                        break;

                            }
                            j = invSizeOrder[size];
                            while(k--){/*k of the runs*/
                                code = huffCode[j];
                                if (sizeCodeLen[j] % 2 == 0)/*huffCode length = 2 or 4, pack 1 bit to 3 or 5*/
                                    code |= ((m&1)<<(sizeCodeLen[j]) );
                                codeSymbol[pos].stageCode[0] = code & 0x07;
                                codeSymbol[pos].bitLength[0] = 3;

                                if (sizeCodeLen[j] > 3)
                                {
                                    codeSymbol[pos].stageCode[1] = code >> 3;
                                    codeSymbol[pos].bitLength[1] = 2;
                                }
                                if (size == 0 )
                                {/*The m already code*/
                                    codeSymbol[pos].bitLength[2] = (sizeCodeLen[j]%2 == 0) ? 0:1;
                                    codeSymbol[pos].stageCode[2] = m; /*If (sizeCodeLen[j]%2 == 0), should be m/2, but 0 bit output, doesn't matter*/
                                }
                                else if (sizeCodeLen[j]%2 == 0)
                                {
                                    codeSymbol[pos].stageCode[2] = m/2;
                                    codeSymbol[pos].bitLength[2] = runSet2Bits - 1;
                                }
                                else
                                {
                                    codeSymbol[pos].stageCode[2] = m;
                                    codeSymbol[pos].bitLength[2] = runSet2Bits;
                                }

                                OutputAt(x, &kernelBufferPtr, &bitOffset, codeSymbol);
                                x++;
                                /* update pos*/
                                pos = (x) % (3*THROUGHPUT);
                            }
                        }
                    }
                    /*process current pixel*/
                    if (coef)
                    {
                        vx_int8 ch;
                        coef>>=(wb->huffmanConfig[index].bit16Flag * 8);
                        if(coef<0x80)
                            coef+=wb->huffmanConfig[index].bit16Flag; /*Since we have to encode zero (run-length only handle 16-bits zero, 8 bit zero should use Size code*/
                        ch = (vx_int8)coef;
                        coef = abs(ch);
                        m = coef*2 + (ch<0?1:0);
                        for (j = 6; j>=0; j--)
                        {
                            if (coef>=(1<<j))
                            {
                                break;
                            }
                        }
                        size = j+1;
                        if (invSizeOrder[size]==8)
                        { /*The minimum freq, we out the raw data*/
                            size = 7;
                        }
                        if (size == 7)
                        {
                            m = ch&0xff;
                            if(wb->huffmanConfig[index].bit16Flag)
                                m = (coef16 >> 8) & 0xff;
                        }

                        j = invSizeOrder[size];
                        code = huffCode[j];
                        if (sizeCodeLen[j]%2 == 0)/*huffCode length = 2 or 4, pack 1 bit to 3 or 5*/
                            code |= ((m&1) << (sizeCodeLen[j]));
                        codeSymbol[pos].stageCode[0] = code & 0x07;
                        codeSymbol[pos].bitLength[0] = 3;

                        if (sizeCodeLen[j] > 3)
                        {
                            codeSymbol[pos].stageCode[1] = code >> 3;
                            codeSymbol[pos].bitLength[1] = 2;
                        }

                        if (size == 7)
                        {
                            size = 8;
                        }
                        /*Size = 0 mean 0&0xff, needs 1 bit actually*/
                        if (size == 0)
                            size = 1;
                        if (sizeCodeLen[j]%2 == 0)
                        {
                            codeSymbol[pos].stageCode[2] = m/2;  /*last bit was coded*/
                            codeSymbol[pos].bitLength[2] = size - 1;
                        }
                        else
                        {
                            codeSymbol[pos].stageCode[2] = m;
                            codeSymbol[pos].bitLength[2] = size;
                        }
                        if (wb->huffmanConfig[index].bit16Flag)
                        {
                            codeSymbol[pos].stageCode[2] = codeSymbol[x%(3*THROUGHPUT)].stageCode[2]* 256 + (coef16 & 0xff);
                            codeSymbol[pos].bitLength[2] += 8;
                        }
                        OutputAt(x, &kernelBufferPtr, &bitOffset, codeSymbol);
                        x++; /*counter*/
                    }

                }
            }

            if(dataRemainingOfCore == 0)
            {
                vx_uint32 dummyCount = (x & 0x1)? 5: 4;
                /*Hit the end bitsteam of core, add dummy stage process the next*/
                vxmASSERT(setDummy == vx_true_e);
                addDummy(x, &kernelBufferPtr, &bitOffset, codeSymbol, dummyCount, dummyStage, dummyBitLength);
                break;
            }
        }

        kernelStreamSize = (vx_uint32)((vx_uint8 *)kernelBufferPtr - kernelStreamBasePtr);
        /*There's 14 bit to save kernel stream bit size*/
        gcmASSERT((kernelStreamSize*8 + bitOffset) < 16384);
        /* Go back to update kernelStreamSize. */
        writeBits(&kernelStreamSizePtr, &streamSizeBitOffset, kernelStreamSize*8 + bitOffset, 14);
        /*align to 64 byte after each kz*/
        packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);
        prev = 0;

    }
    kernelSizePerCore = (vx_uint32)((vx_uint8 *)kernelBufferPtr - kernelStartPtr);
    vxmASSERT(kernelSizePerCore <= wb->slice_array[index].kernel_stream_size);

exit:
    if (reorderStream)
        vxFree(reorderStream);

    return;
}

void calculateWeightBiasStreamRelatedSize(
    vx_context context,
    vx_weights_biases_parameter wb,
    gctPOINTER weight_data,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 kernels_per_core,
    vx_enum weight_format,
    vx_enum bias_format,
    vx_uint32 skip_value,
    vx_int8  set_zrl,
    vx_uint8 option_zero_run_len,
    vx_uint32 index,
    vx_size*   min_kernel_buf_size,
    vx_uint8*  min_zero_run_len,
    vx_uint32* max_zero_run_len
    )
{
    vx_uint8 minZeroRunLen = 0, zeroRunLen = option_zero_run_len;
    vx_size minKernelBufferSize = ~0UL;
    vx_uint32 weightSize = (vx_uint32)vxDataType_GetSize((vx_type_e)weight_format);
    vx_uint32 biasSize = (vx_uint32)vxDataType_GetSize((vx_type_e)bias_format);

    if (weight_format == VX_TYPE_INT16)
        biasSize = 3;

    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP0))
    {
        vx_uint32 nnCoreCount = (weight_format == VX_TYPE_INT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : (weight_format == VX_TYPE_FLOAT16) ?
                      context->nnConfig.fixedFeature.nnCoreCountFloat16 : context->nnConfig.fixedFeature.nnCoreCount;
        vx_uint32 filterPerCore = (z_count % nnCoreCount == 0) ? z_count / nnCoreCount : z_count / nnCoreCount + 1;
        /*VIP V8*/
        minKernelBufferSize = calcKernelSizeV8Huffman(
                                context,
                                wb,
                                wb->weights_sizes[0],
                                wb->weights_sizes[1],
                                wb->weights_sizes[2],
                                wb->weights_sizes[3],
                                filterPerCore,
                                weight_format,
                                skip_value,
                                (vx_uint8_ptr)weight_data,
                                VX_NULL,
                                VX_NULL,
                                VX_NULL,
                                VX_NULL,
                                index
                                );

        wb->slice_array[index].non_zero_count = calcNonZeroCountV8Huffman(context, wb, slice_count, z_count, kernels_per_core, weight_format, (vx_uint8_ptr)weight_data, skip_value);
        wb->slice_array[index].kernel_stream_size = minKernelBufferSize;
        wb->slice_array[index].all_count = wb->weights_sizes[0] * wb->weights_sizes[1] * wb->weights_sizes[2] * wb->weights_sizes[3];
        wb->slice_array[index].kernel_orig_size = wb->slice_array[index].all_count * weightSize + z_count * biasSize;
    }
    else if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_COEF_COMPRESSION_ENHANCEMENT))
    {
        /*V7 Huffman*/
        minKernelBufferSize = calcKernelStreamSizeHuffman(
                                context,
                                wb,
                                wb->weights_sizes[0],
                                wb->weights_sizes[1],
                                wb->weights_sizes[2],
                                wb->weights_sizes[3],
                                kernels_per_core,
                                weight_format,
                                bias_format,
                                wb->wb_base->inputZP,
                                wb->wb_base->coefZP,
                                skip_value,
                                1,
                                1,
                                -1,
                                vxDataType_GetSize((vx_type_e)weight_format),
                                (vx_uint8_ptr)weight_data,
                                VX_NULL,
                                index
                                );
        wb->slice_array[index].kernel_stream_size = minKernelBufferSize;

        /*Only use old path to calculate nonZeroRatio & origKenelSize*/
        calculateWeightBiasBufferSizeForZeroRunLen(context, wb, slice_count, z_count, kernels_per_core, weight_format, bias_format, skip_value, 0, weight_data, index);
    }
    else if (context->options.enableNonZeroBalance)
    {
        if ((wb->weights_sizes[0] == 1 && wb->weights_sizes[1] == 1
            && (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3) || vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6))
            && (wb->wb_base->weights_data_format == VX_TYPE_INT8 || wb->wb_base->weights_data_format == VX_TYPE_UINT8))
            && !gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_ZDP3_NO_COMPRESS_FIX))
        {
            /* Per HW ZDP bug, ZDP3 & ZDP6 zero length should always be 0*/
            minKernelBufferSize = calculateWeightBiasBalanceSizeForZeroRunLen(context, wb, slice_count, z_count, kernels_per_core, weight_format, bias_format, skip_value, 0, weight_data, index);
            minZeroRunLen = 0;
            wb->slice_array[index].kernel_stream_size = minKernelBufferSize;
        }
        else if (set_zrl >= 0)
        {
            minKernelBufferSize = calculateWeightBiasBalanceSizeForZeroRunLen(context, wb, slice_count, z_count, kernels_per_core, weight_format, bias_format, skip_value, set_zrl, weight_data, index);
            minZeroRunLen = set_zrl;
            wb->slice_array[index].kernel_stream_size = minKernelBufferSize;
        }
        else if (zeroRunLen > 5)
        {
            if (!calculateWeightBiasBalanceSizeForZeroRunLenEx(context, wb, slice_count, z_count, kernels_per_core, weight_format, bias_format, skip_value, weight_data, index, &minKernelBufferSize, &minZeroRunLen))
            {
                vx_uint32 minRsvCount = wb->slice_array[index].reserve_weight_count;

                /* Back to calulate kernelBufferSize for other zeroRunLen. */
                for (zeroRunLen = 0; zeroRunLen <= context->nnConfig.fixedFeature.zrlBits; zeroRunLen++)
                {
                    vx_size kernelBufferSize = calculateWeightBiasBalanceSizeForZeroRunLen(context, wb, slice_count, z_count, kernels_per_core, weight_format, bias_format, skip_value, zeroRunLen, weight_data, index);
                    if (kernelBufferSize < minKernelBufferSize)
                    {
                        minKernelBufferSize = kernelBufferSize;
                        minZeroRunLen = zeroRunLen;
                        minRsvCount = wb->slice_array[index].reserve_weight_count;
                    }
                }

                wb->slice_array[index].reserve_weight_count = minRsvCount;
                wb->slice_array[index].kernel_stream_size = minKernelBufferSize;
            }
        }
        else
        {
            minKernelBufferSize = calculateWeightBiasBalanceSizeForZeroRunLen(context, wb, slice_count, z_count, kernels_per_core, weight_format, bias_format, skip_value, zeroRunLen, weight_data, index);
            minZeroRunLen = zeroRunLen;
            wb->slice_array[index].kernel_stream_size = minKernelBufferSize;
        }
    }
    else
    {
        if ((wb->weights_sizes[0] == 1 && wb->weights_sizes[1] == 1
            && (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3) || vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6))
            && (wb->wb_base->weights_data_format == VX_TYPE_INT8 || wb->wb_base->weights_data_format == VX_TYPE_UINT8))
            && !gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_ZDP3_NO_COMPRESS_FIX))
        {
            /* Per HW ZDP bug, ZDP3 & ZDP6 zero length should always be 0*/
            minKernelBufferSize = calculateWeightBiasBufferSizeForZeroRunLen(context, wb, slice_count, z_count, kernels_per_core, weight_format, bias_format, skip_value, 0, weight_data, index);
            minZeroRunLen = 0;
            wb->slice_array[index].kernel_stream_size = minKernelBufferSize;
        }
        else if (set_zrl >= 0)
        {
            minKernelBufferSize = calculateWeightBiasBufferSizeForZeroRunLen(context, wb, slice_count, z_count, kernels_per_core, weight_format, bias_format, skip_value, set_zrl, weight_data, index);
            minZeroRunLen = set_zrl;
            wb->slice_array[index].kernel_stream_size = minKernelBufferSize;
        }
        else if (zeroRunLen > 5)
        {
            if (!calculateWeightBiasBufferSizeForZeroRunLenEx(context, wb, slice_count, z_count, kernels_per_core, weight_format, bias_format, skip_value, weight_data, index, &minKernelBufferSize, &minZeroRunLen))
            {
                vx_uint32 minRsvCount = wb->slice_array[index].reserve_weight_count;

                /* Back to calulate kernelBufferSize for other zeroRunLen. */
                for (zeroRunLen = 0; zeroRunLen <= context->nnConfig.fixedFeature.zrlBits; zeroRunLen++)
                {
                    vx_size kernelBufferSize = calculateWeightBiasBufferSizeForZeroRunLen(context, wb, slice_count, z_count, kernels_per_core, weight_format, bias_format, skip_value, zeroRunLen, weight_data, index);
                    if (kernelBufferSize < minKernelBufferSize)
                    {
                        minKernelBufferSize = kernelBufferSize;
                        minZeroRunLen = zeroRunLen;
                        minRsvCount = wb->slice_array[index].reserve_weight_count;
                    }
                }

                wb->slice_array[index].reserve_weight_count = minRsvCount;
                wb->slice_array[index].kernel_stream_size = minKernelBufferSize;
            }
        }
        else
        {
            minKernelBufferSize = calculateWeightBiasBufferSizeForZeroRunLen(context, wb, slice_count, z_count, kernels_per_core, weight_format, bias_format, skip_value, zeroRunLen, weight_data, index);
            minZeroRunLen = zeroRunLen;
            wb->slice_array[index].kernel_stream_size = minKernelBufferSize;
        }
    }
    if (min_kernel_buf_size != VX_NULL)
        *min_kernel_buf_size = minKernelBufferSize;
    if (min_zero_run_len != VX_NULL)
        *min_zero_run_len = minZeroRunLen;
    if (max_zero_run_len != VX_NULL)
        *max_zero_run_len = (1 << minZeroRunLen) - 1;
}

vx_size estimateWeightBiasStreamSize(
    vx_weights_biases_parameter wb,
    gctPOINTER weight_data
    )
{
    vx_size kernelStreamSize = 0;
    vx_context context = vxGetContext((vx_reference)wb);
    vx_uint32 sliceCount = wb->wb_base->weights_sizes[2];
    vx_uint32 filterCount = wb->wb_base->weights_sizes[3];
    vx_enum weightFormat = wb->wb_base->weights_data_format;
    vx_enum biasFormat = wb->wb_base->biases_data_format;
    vx_uint32 nnCoreCount = (weightFormat == VX_TYPE_INT16) ?
                  context->nnConfig.fixedFeature.nnCoreCountInt16 : (weightFormat == VX_TYPE_FLOAT16) ?
                      context->nnConfig.fixedFeature.nnCoreCountFloat16 : context->nnConfig.fixedFeature.nnCoreCount;
    vx_uint32 filterPerCore = (filterCount % nnCoreCount == 0) ? filterCount / nnCoreCount : filterCount / nnCoreCount + 1;

    calculateWeightBiasStreamRelatedSize(
            context,
            wb,
            weight_data,
            sliceCount, /* slice */
            filterCount, /* z count */
            filterPerCore, /* kernel per core */
            weightFormat,
            biasFormat,
            wb->wb_base->skipValue,
            wb->weights_sizes[2] <= 1 ? 0 : wb->wb_base->setZeroLength,
            (vx_uint8)context->options.nnZeroRunLen,
            0,
            &kernelStreamSize, VX_NULL, VX_NULL);

    return kernelStreamSize;
}

vx_float32 calculateWeightNonZeroRatio(
    vx_context context,
    vx_uint32 skip_value,
    vx_tensor weights
    )
{
    vx_enum weightFomat = TENSOR_DATA_TYPE(weights);
    vx_uint32 skipValue = skip_value;

    gctPOINTER weightData = VX_NULL;

    vx_uint32 weightCount        = weights->dims[0] * weights->dims[1];
    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize((vx_type_e)weightFomat);

    vx_uint32 sliceCount         = weights->dims[2];
    vx_uint32 filterCount        = weights->dims[3];
    vx_uint32 filterSliceSize    = weightCount * weightSize;
    vx_uint32 filterSize         = weightCount * weightSize * sliceCount;
    vx_uint8* startDataPtr       = VX_NULL;
    vx_uint8* kernelDataPtr      = VX_NULL;

    vx_uint32 blockCount = 0, nonZeroCount = 0;
    vx_float32 nonZeroRatio;

    vx_uint32 filterIndex, sliceIndex;
    vx_uint32 weightXIndex, weightYIndex;

    vx_bool hasVipV7Feature = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7) == gcvSTATUS_TRUE ? vx_true_e : vx_false_e;
    vx_bool hasZDP3 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3);
    vx_bool hasZDP6 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6);
    vx_bool hasXYDP9 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP9);
    vx_bool hasXYDP6 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP6);
    vx_uint32 filterStart = 0;
    vx_uint32 filterEnd = filterCount;


    vxoTensor_GetTensorViewMemory(weights, &weightData, VX_NULL);
    startDataPtr = (vx_uint8*)weightData;


    if (weights->dims[0] == 1 && weights->dims[1] == 1 &&
        (hasZDP3 || hasZDP6) &&
        (weightFomat == VX_TYPE_INT8 || weightFomat == VX_TYPE_UINT8))
    {
        vx_uint32 zdpNum = hasZDP6 ? 6 : 3;

        /* zdp3 zdp6 can not enable same time */
        vxmASSERT(!(hasZDP3 && hasZDP6));

        for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex += (zdpNum * 2))
        {
            vx_uint32 weight = 0;
            vx_uint32 realSliceIndex = 0;

            /* add slices of every filter*/
            for (filterIndex = filterStart; filterIndex < filterEnd; filterIndex++)
            {
                vx_uint32 nonZeroInBlock1Count = 0; /*Check if there's non-zero point in first zdpNum block*/
                vx_uint32 nonZeroInBlock2Count = 0; /*Check if there's non-zero point in another zdpNum block*/
                for (realSliceIndex = sliceIndex; realSliceIndex < sliceIndex + (zdpNum * 2); realSliceIndex++)
                {
                    if (realSliceIndex >= sliceCount)
                        break;

                    /* add one slice data every filter */
                    kernelDataPtr = startDataPtr + filterIndex * filterSize + filterSliceSize * realSliceIndex;
                    if (weightFomat == VX_TYPE_INT8)
                        weight = *((vx_int8 *)kernelDataPtr);
                    else if (weightFomat == VX_TYPE_UINT8)
                        weight = *((vx_uint8 *)kernelDataPtr);
                    else
                        weight = *((vx_uint16 *)kernelDataPtr);

                    if (realSliceIndex - sliceIndex < zdpNum && weight != skipValue)
                        nonZeroInBlock1Count++;
                    else if (weight != skipValue)
                        nonZeroInBlock2Count++;
                }
                if (sliceIndex + zdpNum >= sliceCount)
                    blockCount++;
                else
                    blockCount += 2;

                if (nonZeroInBlock1Count)
                    nonZeroCount++;
                if (nonZeroInBlock2Count)
                    nonZeroCount++;
            }
        }
    }
    else if ((hasXYDP9 || hasXYDP6)
                && (weightFomat == VX_TYPE_INT8 || weightFomat == VX_TYPE_UINT8))
    {
        vx_uint32 xStep = 3;
        vx_uint32 yStep = 0;
        vx_uint32 inImageBufferSize = hasXYDP9 ? 2 : 1;

        if (hasXYDP6)
            yStep = 2;
        else
            yStep = 3;

        for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex += inImageBufferSize)
        {
            vx_uint32 weight = 0;
            vx_uint32 realSliceIndex = 0;
            vx_uint32 realWeightXIndex = 0, realWeightYIndex = 0;
            vx_uint32 subKxSize = 0, subKySize = 0, subKzSize = 0;

            subKzSize = gcmMIN((sliceCount - sliceIndex), inImageBufferSize);
            /* add slices of every filter*/
            for (filterIndex = filterStart; filterIndex < filterEnd; filterIndex++)
            {
                for (weightYIndex = 0; weightYIndex < weights->dims[1]; weightYIndex += yStep)
                {
                    subKySize = gcmMIN((weights->dims[1] - weightYIndex), yStep);
                    for (weightXIndex = 0; weightXIndex < weights->dims[0]; weightXIndex += xStep)
                    {
                        subKxSize = gcmMIN((weights->dims[0] - weightXIndex), xStep);
                        for (realSliceIndex = sliceIndex; realSliceIndex < sliceIndex + subKzSize; realSliceIndex++)
                        {
                            vx_uint32 nonZeroInBlockCount = 0; /*Check if there's non-zero point in one 3x3 block*/

                            /* Add one slice data every filter. */
                            kernelDataPtr = startDataPtr + filterIndex * filterSize + filterSliceSize * realSliceIndex;

                            for (realWeightYIndex = weightYIndex; realWeightYIndex < weightYIndex + subKySize; realWeightYIndex++)
                            {
                                for (realWeightXIndex = weightXIndex; realWeightXIndex < weightXIndex + subKxSize; realWeightXIndex++)
                                {
                                    vx_uint8_ptr realKernelDataPtr = kernelDataPtr + (realWeightYIndex * weights->dims[1] + realWeightXIndex) * weightSize;

                                    if (weightFomat == VX_TYPE_INT8)
                                        weight = *((vx_int8 *)realKernelDataPtr);
                                    else if (weightFomat == VX_TYPE_UINT8)
                                        weight = *((vx_uint8 *)realKernelDataPtr);
                                    else
                                        weight = *((vx_uint16 *)realKernelDataPtr);

                                    if (weight != skipValue)
                                        nonZeroInBlockCount++;

                                }
                            }
                            blockCount++;
                            if (nonZeroInBlockCount)
                                nonZeroCount++;
                        }
                    }
                }
            }
        }
    }
    else
    {
        for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
        {
            /* Add slices of every filter. */
            for (filterIndex = filterStart; filterIndex < filterEnd; filterIndex++)
            {
                /* Add one slice data every filter. */
                kernelDataPtr = startDataPtr + filterIndex * filterSize + filterSliceSize * sliceIndex;

                for (weightYIndex = 0; weightYIndex < weights->dims[1]; weightYIndex++)
                {
                    vx_uint32 nonZeroCountInDP3 = 0; /*Check if there's non-zero point in one 3x1 block*/
                    for (weightXIndex = 0; weightXIndex < weights->dims[0]; weightXIndex++)
                    {
                        vx_uint32 weight;

                        if (weightFomat == VX_TYPE_INT8)
                            weight = *((vx_int8 *)kernelDataPtr);
                        else if (weightFomat == VX_TYPE_UINT8)
                            weight = *((vx_uint8 *)kernelDataPtr);
                        else
                            weight = *((vx_uint16 *)kernelDataPtr);
                        kernelDataPtr = kernelDataPtr + weightSize;

                        if(!hasVipV7Feature || weightFomat == VX_TYPE_FLOAT16)
                        {
                            /*V6 or FP16, DP1, a block is 1x1*/
                            blockCount++;
                            if (weight != skipValue)
                                nonZeroCount++;
                        }
                        else
                        {
                            if (weight != skipValue)
                                nonZeroCountInDP3++;

                            /*V7, DP3, one block is 3x1*/
                            if ((weightXIndex + 1) % 3 == 0 || weightXIndex == weights->dims[0] - 1)
                            {
                                blockCount++;
                                if (nonZeroCountInDP3)
                                    nonZeroCount++;
                                nonZeroCountInDP3 = 0;
                            }
                        }
                    }
                }
            }
        }
    }

    nonZeroRatio = (vx_float32)nonZeroCount / (vx_float32)blockCount;

    return nonZeroRatio;
}

vx_size calculateTPWeightStreamSizeForZeroRunLen(
    vx_weights_biases_parameter wb,
    vx_uint32 slice_count,
    vx_uint32 filter_count,
    vx_uint32 total_slice_count,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_uint32 skip_value,
    vx_uint8 zrl,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32_ptr reserve_count,
    vx_uint32_ptr non_zero_count
    )
{
    vx_uint32 maxZeroRun        = (1 << zrl) - 1;
    vx_uint32 filterCount       = filter_count;
    vx_uint32 weightSize        = (vx_uint32)vxDataType_GetSize(weight_format);
    vx_uint32 weightBitSize     = weightSize * 8;
    vx_uint32 filterSize        = weightSize * total_slice_count;
    vx_uint8* kernelDataPtr     = weight_base_ptr;
    vx_uint32 skipValue         = skip_value;
    vx_size kernelBufferSize    = 0;
    vx_uint32 bitOffset         = 0;
    vx_uint32 zeroRun           = 0;
    vx_uint32 filterIndex;
    vx_uint32 rsvWeightCount = 0, nonZeroCount = 0;

    /* Fill zeroRunLenBitWidth. */
    bitOffset = 4;
    for (filterIndex = 0; filterIndex < filterCount; filterIndex++)
    {
        vx_uint32 weight = 0;

        if (WB_WEIGHT_DATA_FORMAT(wb) == VX_TYPE_INT8)
            weight = *((vx_int8 *)kernelDataPtr);
        else if (WB_WEIGHT_DATA_FORMAT(wb) == VX_TYPE_UINT8)
            weight = *((vx_uint8 *)kernelDataPtr);
        else
            weight = *((vx_uint16 *)kernelDataPtr);

        kernelDataPtr += filterSize;

        if ((zeroRun == maxZeroRun)
            || (weight != skipValue)
            || (filterIndex == (filterCount - 1)))
        {
            /* Write zeroRun and weight. */
            bitOffset += zrl + weightBitSize;
            if (bitOffset >= 32)
            {
                bitOffset -= 32;
                kernelBufferSize += 4;
            }
            zeroRun = 0;
            rsvWeightCount++;
        }
        else
        {
            zeroRun++;
        }
        if (weight != skip_value) nonZeroCount++;
    }
    /* pad 0 */
    if (bitOffset)
    {
        kernelBufferSize += 4;
    }
    kernelBufferSize = (kernelBufferSize + 63) & 0xFFFFFFC0;

    if (reserve_count != VX_NULL) *reserve_count = rsvWeightCount;
    if (non_zero_count != VX_NULL) *non_zero_count = nonZeroCount;

    return kernelBufferSize;
}

void calculateWeightBiasTPBufferRelatedSize(
    vx_weights_biases_parameter wb,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32 slice_count,
    vx_uint32 filter_count,
    vx_uint32 total_slice_count,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_uint32 skip_value,
    vx_int8 set_zrl,
    vx_int32 index,
    vx_size* min_kernel_buf_size,
    vx_uint8* min_zero_run_len
    )
{
    vx_uint32 weightSize            = (vx_uint32)vxDataType_GetSize(weight_format);
    vx_uint32 biasSize              = (vx_uint32)vxDataType_GetSize(bias_format);
    vx_uint32 sliceCount            = slice_count;
    vx_uint32 filterCount           = filter_count;
    vx_uint8_ptr kernelDataPtr      = VX_NULL;
    vx_size kernelBufferSize        = 0;
    vx_uint32 bitOffset             = 0;
    vx_int8 zrlBitWidth             = set_zrl;
    vx_uint32 filterIndex, sliceIndex;
    vx_uint8 minZrlBitWidth;
    vx_size minWeightStreamSize;
    vx_uint32 rsvWeightCount = 0, nonZeroCount = 0;
    vx_context context = vxGetContext((vx_reference)wb);

    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP_COMPRESSION_ENHANCEMENT))
    {
        calcTPKernelBufferSizeHuffman(wb, sliceCount, filterCount, wb->wb_base->weights_sizes[2], weight_format, skip_value, weight_base_ptr, index);

        wb->slice_array[index].all_count = wb->weights_sizes[0] * wb->weights_sizes[1] * slice_count * filter_count;
        wb->slice_array[index].kernel_orig_size = slice_count * filter_count * weightSize +
                                                  filter_count * biasSize;

        *min_kernel_buf_size = wb->slice_array[index].kernel_stream_size;
    }
    else
    {
        /* Fill kernel stream size for each slice. */
        for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
        {
            bitOffset += 5;
            if (bitOffset >= 32)
            {
                bitOffset -= 32;
                kernelBufferSize += 4;
            }
        }
        /* pad 0 */
        if (bitOffset)
        {
            bitOffset = 0;
            kernelBufferSize += 4;
        }
        kernelBufferSize = (kernelBufferSize + 63) & 0xFFFFFFC0;

        /* Fill bias for each filter. */
        for (filterIndex = 0; filterIndex < filterCount; filterIndex++)
        {
            kernelBufferSize += biasSize;
        }
        /* pad 0 */
        kernelBufferSize = (kernelBufferSize + 63) & 0xFFFFFFC0;

        /* Fill weight value for every slice */
        if (zrlBitWidth >= 0 && zrlBitWidth <= 9)
        {
            for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
            {
                vx_uint32 rscount, nzcount;
                kernelDataPtr = weight_base_ptr + weightSize * sliceIndex;
                minWeightStreamSize = calculateTPWeightStreamSizeForZeroRunLen(wb, slice_count, filter_count, total_slice_count, weight_format, bias_format, skip_value, zrlBitWidth, kernelDataPtr, &rscount, &nzcount);
                kernelBufferSize += minWeightStreamSize;
                min_zero_run_len[sliceIndex] = zrlBitWidth;
                rsvWeightCount += rscount;
                nonZeroCount += nzcount;
            }
        }
        else
        {
            for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
            {
                vx_uint32 mrscount = 0, mnzcount = 0;

                kernelDataPtr = weight_base_ptr + weightSize * sliceIndex;
                /* Calculate the stream size of 0 zrlBitWidth. */
                minZrlBitWidth = 0;
                minWeightStreamSize = (1 + filterCount * weightSize + 63) & 0xFFFFFFC0;

                /* Calulate weightStreamSize for the rest zrlBitWidth. */
                for (zrlBitWidth = 1; zrlBitWidth <= 9; zrlBitWidth++)
                {
                    vx_uint32 rscount, nzcount;
                    vx_size weightStreamSize = calculateTPWeightStreamSizeForZeroRunLen(wb, slice_count, filter_count, total_slice_count, weight_format, bias_format, skip_value, zrlBitWidth, kernelDataPtr, &rscount, &nzcount);

                    if (!mrscount) mrscount = rscount;
                    if (!mnzcount) mnzcount = nzcount;
                    if (weightStreamSize < minWeightStreamSize)
                    {
                        minWeightStreamSize = weightStreamSize;
                        minZrlBitWidth = zrlBitWidth;
                        mrscount = rscount;
                        mnzcount = nzcount;
                    }
                }
                kernelBufferSize += minWeightStreamSize;
                min_zero_run_len[sliceIndex] = minZrlBitWidth;
                rsvWeightCount += mrscount;
                nonZeroCount += mnzcount;
            }
        }

        wb->slice_array[index].all_count = wb->weights_sizes[0] * wb->weights_sizes[1] * slice_count * filter_count;
        wb->slice_array[index].reserve_weight_count = rsvWeightCount;
        wb->slice_array[index].non_zero_count = nonZeroCount;
        wb->slice_array[index].kernel_stream_size = kernelBufferSize;
        wb->slice_array[index].kernel_orig_size = slice_count * filter_count * (vx_uint32)vxDataType_GetSize(weight_format) +
                                                  filter_count * (vx_uint32)vxDataType_GetSize(bias_format);

        *min_kernel_buf_size = kernelBufferSize;
    }
}

void fillinKernelBuffer(
    vx_context context,
    vx_weights_biases_parameter wb,
    vx_uint8 min_zero_run_len,
    vx_uint32 max_zero_run,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 weight_z,
    vx_uint32 filter_count,
    vx_uint32 kernels_per_core,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_int32 input_zp,
    vx_int32 coef_zp,
    vx_uint32 skip_value,
    vx_uint32 output_final_x,
    vx_uint32 output_final_y,
    vx_int32 z_offset,
    vx_uint32 outputSize,
    vx_uint8_ptr wb_base_ptr,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr,
    vx_uint32 index
    )
{
    vx_enum weightFomat = weight_format;
    vx_int32 inputZP = input_zp;
    vx_int32 coefZP = coef_zp;
    vx_uint32 skipValue = skip_value;

    vx_uint32 nnCoreCount = (weightFomat == VX_TYPE_INT16) ?
        context->nnConfig.fixedFeature.nnCoreCountInt16 : (weightFomat == VX_TYPE_FLOAT16) ?
            context->nnConfig.fixedFeature.nnCoreCountFloat16 : context->nnConfig.fixedFeature.nnCoreCount;

    vx_uint32 coreIndex;
    vx_uint32 groupIndex, filterIndex, sliceIndex;
    vx_uint32 weightXIndex, weightYIndex;
    vx_uint32 bitOffset          = 0;
    vx_uint32 zeroRun            = 0;

    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize(weightFomat);
    vx_uint32 weightBitSize      = weightSize * 8;
    vx_uint32 biasSize           = (vx_uint32)vxDataType_GetSize(bias_format);
    vx_uint32 biasBitSize        = biasSize * 8;
    vx_uint32 biasData           = 0;

    vx_uint32 sliceCount         = weight_z;
    vx_uint32 filterSliceSize    = weight_x * weight_y * weightSize;
    vx_uint32 filterSize         = weight_x * weight_y * sliceCount * weightSize;

    vx_uint32 filterTotalCount   = filter_count;
    vx_uint32 filterCount        = kernels_per_core; /* filter count every group */
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;
    vx_uint32 totalFilterPerCore = filterTotalCount / nnCoreCount;
    vx_uint32 oddFilterPerCore   = filterTotalCount % nnCoreCount;

    vx_uint32 kernelStreamSize   = ((filterSize * filterTotalCount + filterTotalCount * biasSize + filterTotalCount * 3 + 3) + 63) & ~63;
    vx_uint32* kernelBufferPtr   = (vx_uint32*) wb_base_ptr;
    vx_uint32* kernelStreamSizePtr = kernelBufferPtr;
    vx_uint32 streamSizeBitOffset = 0;
    vx_uint32 alignedOffset      = ((vx_uint32)(gctUINTPTR_T)kernelBufferPtr) & 0x3F;
    vx_uint8_ptr kernelDataPtr   = VX_NULL;
    vx_uint8_ptr kernelStreamBasePtr = VX_NULL;
    vx_uint32 usedCoreCount      = (filterTotalCount > nnCoreCount) ? nnCoreCount : filterTotalCount;
    vx_uint32 maxKernelStreamSizePerCore = 0;
    vx_float64* nonZeroRatioPerCorePerVZG = 0;

    typedef struct _gcVXWeightsMinusZP
    {
        vx_int32 sum;
        vx_uint32 filterIdx;
    }gcVXWeightsMinusZP;

    gcVXWeightsMinusZP *weightsMinusZP = VX_NULL;

    if (weightFomat == VX_TYPE_INT8 || weightFomat == VX_TYPE_UINT8)
    {
        if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
        {
            biasBitSize = NN_INTEGER_BIAS_BITS_VIP_V7;
        }
        else
        {
            biasBitSize = NN_INTEGER_BIAS_BITS;
        }
    }
    else if (weightFomat == VX_TYPE_INT16)
    {
        biasBitSize = NN_INTEGER_BIAS_BITS_VIP_V7_INT16;
    }

    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT) && inputZP != 0 && weightFomat == VX_TYPE_UINT8)
    {
        weightsMinusZP = (gcVXWeightsMinusZP*)vxAllocateAndZeroMemory(filter_count * sizeof(gcVXWeightsMinusZP));

        if (!weightsMinusZP)
        {
            vxError("fillinKernelBuffer: OUT OF MEMORY");
            goto exit;
        }

        /*Calc sum((coef[i] - coefZP) * InZP) */
        for (filterIndex = 0; filterIndex < filter_count; filterIndex++)
        {
            weightsMinusZP[filterIndex].filterIdx = filterIndex;
            weightsMinusZP[filterIndex].sum = 0;
            for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
            {
                /* add one slice data every filter */
                kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * sliceIndex;

                for (weightYIndex = 0; weightYIndex < weight_y; weightYIndex++)
                {
                    for (weightXIndex = 0; weightXIndex < weight_x; weightXIndex++)
                    {
                        vx_int32 weight = 0;
                        weight = *((vx_uint8 *)kernelDataPtr);
                        kernelDataPtr = kernelDataPtr + weightSize;

                        weightsMinusZP[filterIndex].sum += (weight - coefZP) * inputZP;
                    }
                }
            }
        }
    }

    if (wb->zOffsetHandle == VX_NULL)
    {
        wb->zOffsetHandle = (vx_weights_biases_z_offset)vxAllocateAndZeroMemory(filter_count * sizeof(vx_weights_biases_z_offset_s));
        if (wb->zOffsetHandle == VX_NULL)
        {
            vxError("fillinKernelBuffer: OUT OF MEMORY");
            goto exit;
        }
    }
    wb->numOfVz = filter_count;

    if (wb->max_per_core_per_vzgroup_nzr == VX_NULL)
    {
        wb->max_per_core_per_vzgroup_nzr = (vx_float64*)vxAllocateAndZeroMemory(groupCount * sizeof(vx_float64));
        if (wb->max_per_core_per_vzgroup_nzr == VX_NULL)
        {
            vxError("fillinKernelBuffer: OUT OF MEMORY\n");
            goto exit;
        }
    }

    nonZeroRatioPerCorePerVZG = (vx_float64*)vxAllocateAndZeroMemory(nnCoreCount * groupCount * sizeof(vx_float64));
    if (nonZeroRatioPerCorePerVZG == VX_NULL)
    {
        vxError("fillinKernelBuffer: OUT OF MEMORY");
        goto exit;
    }

    /* kernel Buffer size for each core */
    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        writeBits(&kernelBufferPtr, &bitOffset, kernelStreamSize, 32);
    }
    packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);
    wb->slice_array[index].kernel_stream_full_cache_size = 0;

    wb->max_per_core_compression_ratio = 0;

    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        vx_uint32 coreFilterCount = (coreIndex < oddFilterPerCore) ? totalFilterPerCore + 1 : totalFilterPerCore;
        vx_uint32 coreOrgKernelSize = coreFilterCount * (filterSize + biasSize);
        vx_float64 compressionRatio = 0;
        kernelStreamBasePtr = (vx_uint8_ptr)kernelBufferPtr;

        /* zeroRunLen */
        if (coreFilterCount > 0)
        {
            /* zeroRunLen */
            writeBits(&kernelBufferPtr, &bitOffset, min_zero_run_len, 8);

            writeBits(&kernelBufferPtr, &bitOffset, coreFilterCount, 16);
        }
        /* fill weight value and bias for every group */
        zeroRun = 0;
        for (groupIndex = 0; groupIndex < groupCount; groupIndex++)
        {
            vx_uint32 actualFilterCount = (groupIndex == groupCount - 1) ? (coreFilterCount - groupIndex * filterCount) : filterCount;
            vx_uint32 filterStart, filterEnd;
            vx_uint32 blockCount = 0;
            vx_uint32 nonZeroCount = 0;
            vx_uint32 nonZrlIdx = groupIndex + groupCount * coreIndex;

            if ((groupIndex == groupCount - 1)
                && (coreIndex >= (filterTotalCount % nnCoreCount)))
            {
                filterStart = groupIndex * nnCoreCount * filterCount
                            + (filterTotalCount % nnCoreCount) * (actualFilterCount + 1)
                            + (coreIndex - (filterTotalCount % nnCoreCount)) * actualFilterCount;
            }
            else
            {
                filterStart = groupIndex * nnCoreCount *filterCount + coreIndex * actualFilterCount;
            }
            filterEnd = filterStart + actualFilterCount - 1;

            /* Only UINT8/INT8 support ZDP3/ZDP6 */
            if (weight_x == 1 && weight_y == 1
                && (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3) || vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6))
                && (weightFomat == VX_TYPE_INT8 || weightFomat == VX_TYPE_UINT8))
            {
                vx_uint32 zdpNum = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6) ? 6 : 3;
                /* zdp3 zdp6 can not enable same time */
                vxmASSERT(!(vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3) && vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6)));

                for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex += (zdpNum * 2))
                {
                    vx_uint8 group2DCoded = 0;
                    vx_uint32 weight = 0;
                    vx_uint32 realSliceIndex = 0;

                    /* add slices of every filter*/
                    for (filterIndex = filterStart; filterIndex <= filterEnd; filterIndex++)
                    {
                        vx_uint32 nonZeroInBlock1Count = 0; /*Check if there's non-zero point in first zdpNum block*/
                        vx_uint32 nonZeroInBlock2Count = 0; /*Check if there's non-zero point in another zdpNum block*/
                        for (realSliceIndex = sliceIndex; realSliceIndex < sliceIndex + (zdpNum * 2); realSliceIndex++)
                        {
                            vx_uint8 needToWriteBias = (realSliceIndex == 0) ? 1 : 0;

                            if (realSliceIndex >= sliceCount)
                                break;

                            /* add one slice data every filter */
                            kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * realSliceIndex;
                            if (realSliceIndex == 0)
                            {
                                if (bias_base_ptr)
                                    biasData = *(bias_base_ptr + filterIndex);
                                else
                                    biasData = 0;

                                /* NewBias = sum((Coef[i] - CoefZP) * - InZP) + Bias*/
                                if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT)
                                    && inputZP != 0
                                    && weightFomat == VX_TYPE_UINT8)
                                {
                                    biasData -= weightsMinusZP[filterIndex].sum;
                                }

                            }

                            if (weightFomat == VX_TYPE_INT8)
                                weight = *((vx_int8 *)kernelDataPtr);
                            else if (weightFomat == VX_TYPE_UINT8)
                                weight = *((vx_uint8 *)kernelDataPtr);
                            else
                                weight = *((vx_uint16 *)kernelDataPtr);

                            if (realSliceIndex - sliceIndex < zdpNum && weight != skipValue)
                                nonZeroInBlock1Count++;
                            else if (weight != skipValue)
                                nonZeroInBlock2Count++;

                            if ((zeroRun == max_zero_run)
                                || ((realSliceIndex == 0)
                                    && needToWriteBias)
                                || (realSliceIndex == sliceCount - 1)
                                || (weight != skipValue)
                                || ((filterIndex == filterEnd)
                                    && (group2DCoded == 0)))
                            {
                                if (min_zero_run_len)
                                {
                                    writeBits(&kernelBufferPtr, &bitOffset, zeroRun, min_zero_run_len);
                                }
                                writeBits(&kernelBufferPtr, &bitOffset, weight, weightBitSize);
                                zeroRun = 0;

                                if (needToWriteBias)
                                {
                                    if (weightFomat == VX_TYPE_INT16)
                                    {
                                        vx_int64 bias64 = 0;
                                        vx_int32 bias32;
                                        vx_int16 bias16;

                                        if ((vx_int32)biasData < 0)
                                        {
                                            bias64 = ~0;
                                            bias64 = (bias64 >> 32) << 32;
                                            bias64 = bias64 | (vx_int64)biasData;
                                        }
                                        else
                                            bias64 = (vx_int64)biasData;

                                        bias32 = (vx_int32)bias64;
                                        writeBits(&kernelBufferPtr, &bitOffset, bias32, 32);
                                        bias16 = (vx_int16)(bias64 >> 32);
                                        writeBits(&kernelBufferPtr, &bitOffset, (vx_int32)bias16, 16);
                                    }
                                    else
                                        writeBits(&kernelBufferPtr, &bitOffset, biasData, biasBitSize);
                                    needToWriteBias = 0;
                                }
                                group2DCoded = 1;
                            }
                            else
                            {
                                zeroRun++;
                            }

                            /* add offet behind the last point of the last slice of each filter */
                            if (realSliceIndex == sliceCount - 1)
                            {
                                vx_uint32 offsetValue;
                                if (z_offset > 0)
                                    offsetValue = (vx_uint32)z_offset * filterIndex;
                                else if (outputSize > 0)
                                    offsetValue = output_final_x * output_final_y * outputSize * filterIndex;
                                else
                                    offsetValue = output_final_x * output_final_y * weightSize * filterIndex;

                                if (wb->zOffsetHandle)
                                {
                                    wb->zOffsetHandle[filterIndex].ptrOffset = (vx_uint32)((vx_uint8 *)kernelBufferPtr - wb_base_ptr);
                                    wb->zOffsetHandle[filterIndex].bitOffset = bitOffset;
                                }

                                if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                    writeBits(&kernelBufferPtr, &bitOffset, offsetValue, NN_Z_POSITION_OFFSET_BITS_VIP_V7);
                                else
                                    writeBits(&kernelBufferPtr, &bitOffset, offsetValue, NN_Z_POSITION_OFFSET_BITS);
                            }
                        }
                        if (sliceIndex + zdpNum >= sliceCount)
                            blockCount++;
                        else
                            blockCount += 2;

                        if (nonZeroInBlock1Count)
                            nonZeroCount++;
                        if (nonZeroInBlock2Count)
                            nonZeroCount++;
                    }
                }
            }
            /*Only INT8/UINT8 support XY DP9 now*/
            else if ((vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP9) || vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP6))
                && (weightFomat == VX_TYPE_INT8 || weightFomat == VX_TYPE_UINT8))
            {
                vx_uint32 xStep = 0, yStep = 0;
                vx_uint32 inImageBufferSize = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP9) ? 2 : 1;

                /* xydp9 xydp6 can not enable same time */
                vxmASSERT(!(vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP9) && vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP6)));

                if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP6))
                {
                    xStep = 3;
                    yStep = 2;
                }
                else
                {
                    xStep = 3;
                    yStep = 3;
                }

                for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex += inImageBufferSize)
                {
                    vx_uint8 group2DCoded = 0;
                    vx_uint32 weight = 0;
                    vx_uint32 realSliceIndex = 0;
                    vx_uint32 realWeightXIndex = 0, realWeightYIndex = 0;
                    vx_uint32 subKxSize = 0, subKySize = 0, subKzSize = 0;

                    subKzSize = gcmMIN((sliceCount - sliceIndex), inImageBufferSize);
                    /* add slices of every filter*/
                    for (filterIndex = filterStart; filterIndex <= filterEnd; filterIndex++)
                    {
                        vx_bool hasWriteBias = vx_false_e;
                        for (weightYIndex = 0; weightYIndex < weight_y; weightYIndex += yStep)
                        {
                            subKySize = gcmMIN((weight_y - weightYIndex), yStep);
                            for (weightXIndex = 0; weightXIndex < weight_x; weightXIndex += xStep)
                            {
                                subKxSize = gcmMIN((weight_x - weightXIndex), xStep);
                                for (realSliceIndex = sliceIndex; realSliceIndex < sliceIndex + subKzSize; realSliceIndex++)
                                {
                                    vx_uint8 needToWriteBias = (sliceIndex == 0) ? 1 : 0;
                                    vx_uint32 nonZeroInBlockCount = 0; /*Check if there's non-zero point in one 3x3 block*/

                                    /* add one slice data every filter */
                                    kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * realSliceIndex;
                                    if (realSliceIndex == 0)
                                    {
                                        if (bias_base_ptr)
                                            biasData = *(bias_base_ptr + filterIndex);
                                        else
                                            biasData = 0;

                                        /* NewBias = sum((Coef[i] - CoefZP) * - InZP) + Bias*/
                                        if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT)
                                            && inputZP != 0
                                            && weightFomat == VX_TYPE_UINT8)
                                        {
                                            biasData -= weightsMinusZP[filterIndex].sum;
                                        }
                                    }

                                    for (realWeightYIndex = weightYIndex; realWeightYIndex < weightYIndex + subKySize; realWeightYIndex++)
                                    {
                                        for (realWeightXIndex = weightXIndex; realWeightXIndex < weightXIndex + subKxSize; realWeightXIndex++)
                                        {
                                            vx_uint8_ptr realKernelDataPtr = kernelDataPtr + (realWeightYIndex * weight_x + realWeightXIndex) * weightSize;

                                            if (weightFomat == VX_TYPE_INT8)
                                                weight = *((vx_int8 *)realKernelDataPtr);
                                            else if (weightFomat == VX_TYPE_UINT8)
                                                weight = *((vx_uint8 *)realKernelDataPtr);
                                            else
                                                weight = *((vx_uint16 *)realKernelDataPtr);

                                            if (weight != skipValue)
                                                nonZeroInBlockCount++;

                                            if ((zeroRun == max_zero_run)
                                                || ((realSliceIndex == 0)
                                                    && (realWeightXIndex == weight_x - 1)
                                                    && (realWeightYIndex == weight_y - 1)
                                                    && needToWriteBias)
                                                || ((realSliceIndex == sliceCount - 1)
                                                    && (realWeightXIndex == weight_x - 1)
                                                    && (realWeightYIndex == weight_y - 1))
                                                || (weight != skipValue)
                                                || ((filterIndex == filterEnd)
                                                    && (realWeightXIndex == weight_x - 1)
                                                    && (realWeightYIndex == weight_y - 1)
                                                    && (group2DCoded == 0)))
                                            {
                                                if (min_zero_run_len)
                                                {
                                                    writeBits(&kernelBufferPtr, &bitOffset, zeroRun, min_zero_run_len);
                                                }
                                                writeBits(&kernelBufferPtr, &bitOffset, weight, weightBitSize);
                                                zeroRun = 0;

                                                if (needToWriteBias && !hasWriteBias)
                                                {
                                                    if (weightFomat == VX_TYPE_INT16)
                                                    {
                                                        vx_int64 bias64 = 0;
                                                        vx_int32 bias32;
                                                        vx_int16 bias16;

                                                        if ((vx_int32)biasData < 0)
                                                        {
                                                            bias64 = ~0;
                                                            bias64 = (bias64 >> 32) << 32;
                                                            bias64 = bias64 | (vx_int64)biasData;
                                                        }
                                                        else
                                                            bias64 = (vx_int64)biasData;

                                                        bias32 = (vx_int32)bias64;
                                                        writeBits(&kernelBufferPtr, &bitOffset, bias32, 32);
                                                        bias16 = (vx_int16)(bias64 >> 32);
                                                        writeBits(&kernelBufferPtr, &bitOffset, (vx_int32)bias16, 16);
                                                    }
                                                    else
                                                        writeBits(&kernelBufferPtr, &bitOffset, biasData, biasBitSize);
                                                    needToWriteBias = 0;
                                                    hasWriteBias = vx_true_e;
                                                }
                                                group2DCoded = 1;
                                            }
                                            else
                                            {
                                                zeroRun++;
                                            }

                                            /* add offet behind the last point of the last slice of each filter */
                                            if (realSliceIndex == sliceCount - 1 && realWeightXIndex == weight_x - 1 && realWeightYIndex == weight_y - 1)
                                            {
                                                vx_uint32 offsetValue;

                                                if (z_offset > 0)
                                                    offsetValue = (vx_uint32)z_offset * filterIndex;
                                                else if (outputSize > 0)
                                                    offsetValue = output_final_x * output_final_y * outputSize * filterIndex;
                                                else
                                                    offsetValue = output_final_x * output_final_y * weightSize * filterIndex;

                                                if (wb->zOffsetHandle)
                                                {
                                                    wb->zOffsetHandle[filterIndex].ptrOffset = (vx_uint32)((vx_uint8 *)kernelBufferPtr - wb_base_ptr);
                                                    wb->zOffsetHandle[filterIndex].bitOffset = bitOffset;
                                                }

                                                if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                                    writeBits(&kernelBufferPtr, &bitOffset, offsetValue, NN_Z_POSITION_OFFSET_BITS_VIP_V7);
                                                else
                                                    writeBits(&kernelBufferPtr, &bitOffset, offsetValue, NN_Z_POSITION_OFFSET_BITS);
                                            }
                                        }
                                    }
                                    blockCount++;
                                    if (nonZeroInBlockCount)
                                        nonZeroCount++;
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
                {
                    vx_uint8 group2DCoded = 0;
                    /* add slices of every filter*/
                    for (filterIndex = filterStart; filterIndex <= filterEnd; filterIndex++)
                    {
                        vx_uint8 needToWriteBias = (sliceIndex == 0) ? 1 : 0;

                        /* add one slice data every filter */
                        kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * sliceIndex;
                        if (sliceIndex == 0)
                        {
                            if (bias_base_ptr)
                                biasData = *(bias_base_ptr + filterIndex);
                            else
                                biasData = 0;

                            /* NewBias = sum((Coef[i] - CoefZP) * - InZP) + Bias*/
                            if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT)
                                && inputZP != 0
                                && weightFomat == VX_TYPE_UINT8)
                            {
                                biasData -= weightsMinusZP[filterIndex].sum;
                            }
                        }

                        for (weightYIndex = 0; weightYIndex < weight_y; weightYIndex++)
                        {
                            vx_uint32 nonZeroCountInDP3 = 0; /*Check if there's non-zero point in one 3x1 block*/
                            for (weightXIndex = 0; weightXIndex < weight_x; weightXIndex++)
                            {
                                vx_uint32 weight = 0;

                                if (weightFomat == VX_TYPE_INT8)
                                    weight = *((vx_int8 *)kernelDataPtr);
                                else if (weightFomat == VX_TYPE_UINT8)
                                    weight = *((vx_uint8 *)kernelDataPtr);
                                else
                                    weight = *((vx_uint16 *)kernelDataPtr);
                                kernelDataPtr = kernelDataPtr + weightSize;

                                {
                                    if (weight != skipValue)
                                        nonZeroCountInDP3++;

                                    /*V7, DP3, one block is 3x1*/
                                    if ((weightXIndex + 1) % 3 == 0 || weightXIndex == wb->weights_sizes[0] - 1)
                                    {
                                        blockCount++;
                                        if (nonZeroCountInDP3)
                                            nonZeroCount++;
                                        nonZeroCountInDP3 = 0;
                                    }
                                }

                                if ((zeroRun == max_zero_run)
                                    || ((sliceIndex == 0)
                                        && (weightXIndex == weight_x - 1)
                                        && (weightYIndex == weight_y - 1)
                                        && needToWriteBias)
                                    || ((weightXIndex == weight_x - 1)
                                        && (weightYIndex == weight_y - 1)
                                        && (sliceIndex == sliceCount - 1))
                                    || (weight != skipValue)
                                    || ((filterIndex == filterEnd)
                                        && (weightXIndex == weight_x - 1)
                                        && (weightYIndex == weight_y - 1)
                                        && (group2DCoded == 0)))
                                {
                                    if (min_zero_run_len)
                                    {
                                        writeBits(&kernelBufferPtr, &bitOffset, zeroRun, min_zero_run_len);
                                    }
                                    writeBits(&kernelBufferPtr, &bitOffset, weight, weightBitSize);
                                    zeroRun = 0;

                                    if (needToWriteBias)
                                    {
                                        if (weightFomat == VX_TYPE_INT16)
                                        {
                                            vx_int64 bias64 = 0;
                                            vx_int32 bias32;
                                            vx_int16 bias16;

                                            if ((vx_int32)biasData < 0)
                                            {
                                                bias64 = ~0;
                                                bias64 = (bias64 >> 32) << 32;
                                                bias64 = bias64 | (vx_int64)biasData;
                                            }
                                            else
                                                bias64 = (vx_int64)biasData;

                                            bias32 = (vx_int32)bias64;
                                            writeBits(&kernelBufferPtr, &bitOffset, bias32, 32);
                                            bias16 = (vx_int16)(bias64 >> 32);
                                            writeBits(&kernelBufferPtr, &bitOffset, (vx_int32)bias16, 16);
                                        }
                                        else
                                            writeBits(&kernelBufferPtr, &bitOffset, biasData, biasBitSize);
                                        needToWriteBias = 0;
                                    }
                                    group2DCoded = 1;
                                }
                                else
                                {
                                    zeroRun++;
                                }
                            }
                        }

                        /* add offet behind the last point of the last slice of each filter */
                        if (sliceIndex == sliceCount - 1)
                        {
                            vx_uint32 offsetValue;
                            if (z_offset > 0)
                                offsetValue = (vx_uint32)z_offset * filterIndex;
                            else if (outputSize > 0)
                                offsetValue = output_final_x * output_final_y * outputSize * filterIndex;
                            else
                                offsetValue = output_final_x * output_final_y * weightSize * filterIndex;

                            if (wb->zOffsetHandle)
                            {
                                wb->zOffsetHandle[filterIndex].ptrOffset = (vx_uint32)((vx_uint8 *)kernelBufferPtr - wb_base_ptr);
                                wb->zOffsetHandle[filterIndex].bitOffset = bitOffset;
                            }

                            if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                writeBits(&kernelBufferPtr, &bitOffset, offsetValue, NN_Z_POSITION_OFFSET_BITS_VIP_V7);
                            else
                                writeBits(&kernelBufferPtr, &bitOffset, offsetValue, NN_Z_POSITION_OFFSET_BITS);

                        }
                    }
                }
            }
            nonZeroRatioPerCorePerVZG[nonZrlIdx] = (vx_float64)nonZeroCount / (vx_float64)blockCount;

        }

        /* pad 0 */
        packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);

        /* Go back to update kernelStreamSize. */
        kernelStreamSize = (vx_uint32)((vx_uint8 *)kernelBufferPtr - kernelStreamBasePtr);
        /*With RTL bug fix, full cache mode size needn't using maxSize * coreCount*/
        wb->slice_array[index].kernel_stream_full_cache_size += kernelStreamSize;

        compressionRatio = (vx_float64)kernelStreamSize / (vx_float64)coreOrgKernelSize;
        if (wb->max_per_core_compression_ratio < compressionRatio)
            wb->max_per_core_compression_ratio = compressionRatio;

        if (kernelStreamSize > maxKernelStreamSizePerCore)
            maxKernelStreamSizePerCore = kernelStreamSize;

        writeBits(&kernelStreamSizePtr, &streamSizeBitOffset, kernelStreamSize, 32);
    }

    wb->slice_array[index].kernel_align_stream_size += (vx_size)(maxKernelStreamSizePerCore * usedCoreCount);

    if (z_offset > 0)
        wb->orgZOffsetValue = z_offset;
    else if (outputSize > 0)
        wb->orgZOffsetValue = output_final_x * output_final_y * outputSize;
    else
        wb->orgZOffsetValue = output_final_x * output_final_y * weightSize;

    for (groupIndex = 0; groupIndex < groupCount; groupIndex++)
    {
        for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
        {
            vx_uint32 nonZrlIdx = groupIndex + coreIndex * groupCount;
            if (wb->max_per_core_per_vzgroup_nzr[groupIndex] < nonZeroRatioPerCorePerVZG[nonZrlIdx])
                wb->max_per_core_per_vzgroup_nzr[groupIndex] = nonZeroRatioPerCorePerVZG[nonZrlIdx];
        }
    }

exit:
    if (weightsMinusZP)
        vxFree(weightsMinusZP);
    if (nonZeroRatioPerCorePerVZG)
        vxFree(nonZeroRatioPerCorePerVZG);
    return;
}

void fillinDepthWiseKernelBuffer(
    vx_context context,
    vx_weights_biases_parameter wb,
    vx_uint8 min_zero_run_len,
    vx_uint32 max_zero_run,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 weight_z,
    vx_uint32 filter_count,
    vx_uint32 kernels_per_core,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_int32 input_zp,
    vx_int32 coef_zp,
    vx_uint32 skip_value,
    vx_uint32 output_final_x,
    vx_uint32 output_final_y,
    vx_int32 z_offset,
    vx_uint32 outputSize,
    vx_uint8_ptr wb_base_ptr,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr,
    vx_uint32 index
    )
{
    vx_enum weightFomat = weight_format;
    vx_int32 inputZP = input_zp;
    vx_int32 coefZP = coef_zp;

    vx_uint32 nnCoreCount = (weightFomat == VX_TYPE_INT16) ?
        context->nnConfig.fixedFeature.nnCoreCountInt16 : (weightFomat == VX_TYPE_FLOAT16) ?
            context->nnConfig.fixedFeature.nnCoreCountFloat16 : context->nnConfig.fixedFeature.nnCoreCount;

    vx_uint32 coreIndex;
    vx_uint32 sliceIndex;
    vx_uint32 weightXIndex, weightYIndex;
    vx_uint32 bitOffset          = 0;

    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize((vx_type_e)weightFomat);
    vx_uint32 weightBitSize      = weightSize * 8;
    vx_uint32 biasSize           = (vx_uint32)vxDataType_GetSize((vx_type_e)bias_format);
    vx_uint32 biasBitSize        = biasSize * 8;
    vx_uint32 biasData           = 0;

    vx_uint32 filterSliceSize    = weight_x * weight_y * weightSize;

    vx_uint32 filterCount        = kernels_per_core; /* filter count every group */
    vx_uint32 filterTotalCount   = filter_count;
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;
    vx_uint32 totalFilterPerCore = filterTotalCount / nnCoreCount;
    vx_uint32 oddFilterPerCore   = filterTotalCount % nnCoreCount;

    vx_uint32 kernelStreamSize   = 0;
    vx_uint32* kernelBufferPtr   = (vx_uint32*) wb_base_ptr;
    vx_uint32* kernelStreamSizePtr = kernelBufferPtr;
    vx_uint32 streamSizeBitOffset = 0;
    vx_uint32 alignedOffset      = ((vx_uint32)(gctUINTPTR_T)kernelBufferPtr) & 0x3F;
    vx_uint8_ptr kernelDataPtr   = VX_NULL;
    vx_uint8_ptr kernelStreamBasePtr = VX_NULL;

    vx_uint32 maxKernelStreamSizePerCore = 0;
    vx_uint32 core0FilterCount = 0;

    vx_bool hasXYDP9 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP9);
    vx_bool hasXYDP6 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP6);
    vx_bool biasAtEnd = vx_false_e;

    typedef struct _gcVXWeightsMinusZP
    {
        vx_int32 sum;
        vx_uint32 filterIdx;
    }gcVXWeightsMinusZP;

    gcVXWeightsMinusZP *weightsMinusZP = VX_NULL;

    if (weightFomat == VX_TYPE_INT8 || weightFomat == VX_TYPE_UINT8)
    {
        if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
        {
            biasBitSize = NN_INTEGER_BIAS_BITS_VIP_V7;
        }
        else
        {
            biasBitSize = NN_INTEGER_BIAS_BITS;
        }
    }
    else if (weightFomat == VX_TYPE_INT16)
    {
        biasBitSize = NN_INTEGER_BIAS_BITS_VIP_V7_INT16;
    }

    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT) && inputZP != 0 && weightFomat == VX_TYPE_UINT8)
    {
        weightsMinusZP = (gcVXWeightsMinusZP*)vxAllocateAndZeroMemory(filterTotalCount * sizeof(gcVXWeightsMinusZP));

        if (!weightsMinusZP)
        {
            vxError("fillinKernelBuffer: OUT OF MEMORY");
            goto exit;
        }

        /*Calc sum((coef[i] - coefZP) * InZP) */
        for (sliceIndex = 0; sliceIndex < filterTotalCount; sliceIndex++)
        {
            weightsMinusZP[sliceIndex].filterIdx = sliceIndex;
            weightsMinusZP[sliceIndex].sum = 0;

            {
                /* add one slice data every filter */
                kernelDataPtr = weight_base_ptr + filterSliceSize * sliceIndex;

                for (weightYIndex = 0; weightYIndex < weight_y; weightYIndex++)
                {
                    for (weightXIndex = 0; weightXIndex < weight_x; weightXIndex++)
                    {
                        vx_int32 weight = 0;
                        weight = *((vx_uint8 *)kernelDataPtr);
                        kernelDataPtr = kernelDataPtr + weightSize;

                        weightsMinusZP[sliceIndex].sum += (weight - coefZP) * inputZP;
                    }
                }
            }
        }
    }
    wb->numOfVz = filterTotalCount;

    /* kernel Buffer size for each core */
    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        writeBits(&kernelBufferPtr, &bitOffset, kernelStreamSize, 32);
    }
    packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);
    wb->slice_array[index].kernel_stream_full_cache_size = 0;

    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        vx_uint32 coreFilterCount = (coreIndex < oddFilterPerCore) ? totalFilterPerCore + 1 : totalFilterPerCore;
        vx_uint32 groupIndex;

        kernelStreamBasePtr = (vx_uint8_ptr)kernelBufferPtr;

        /* zeroRunLen */
        writeBits(&kernelBufferPtr, &bitOffset, min_zero_run_len, 8);
        /*kernels num in each core*/
        writeBits(&kernelBufferPtr, &bitOffset, coreFilterCount, 16);

        for (groupIndex = 0; groupIndex < groupCount; groupIndex++)
        {
            vx_uint32 actualFilterCount = (groupIndex == groupCount - 1) ? (coreFilterCount - groupIndex * filterCount) : filterCount;
            vx_uint32 groupFilterStart = groupIndex * nnCoreCount * filterCount;
            vx_uint32 adjFilterStart = groupFilterStart + nnCoreCount - coreIndex - 1;
            vx_uint32 kid = 0;
            vx_uint32 unusedCoreCount = (filterTotalCount % nnCoreCount == 0) ? 0 : nnCoreCount - (filterTotalCount % nnCoreCount);
            vx_uint32 realSliceIndex = 0;

            if (coreIndex == 0)
                core0FilterCount = actualFilterCount;

            for (kid = 0; kid < actualFilterCount; kid++)
            {
                vx_uint32 xStep = 0, yStep = 0;
                vx_uint32 skxSize = 0, skySize = 0;
                vx_uint32 skx = 0, sky = 0;
                vx_uint32 zeroRun = 0;

                if (groupIndex == groupCount - 1 &&
                    kid == core0FilterCount - 1 &&
                    kid != 0)
                    realSliceIndex = adjFilterStart + kid * nnCoreCount - unusedCoreCount;
                else
                    realSliceIndex = adjFilterStart + kid * nnCoreCount;

                xStep = 3;
                yStep = hasXYDP9 ? 3 : hasXYDP6 ? 2 : 1;
                kernelDataPtr = weight_base_ptr + realSliceIndex * filterSliceSize;

                if (bias_base_ptr)
                    biasData = *(bias_base_ptr + realSliceIndex);
                else
                    biasData = 0;

                /* NewBias = sum((Coef[i] - CoefZP) * - InZP) + Bias*/
                if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT)
                    && inputZP != 0
                    && weightFomat == VX_TYPE_UINT8)
                {
                    biasData -= weightsMinusZP[realSliceIndex].sum;
                }

                for (weightYIndex = 0; weightYIndex < weight_y; weightYIndex += yStep)
                {
                    skySize = gcmMIN(weight_y - weightYIndex, yStep);
                    for (weightXIndex = 0; weightXIndex < weight_x; weightXIndex += xStep)
                    {
                        skxSize = gcmMIN(weight_x - weightXIndex, xStep);
                        for (sky = 0; sky < skySize; sky++)
                        {
                            for (skx = 0; skx < skxSize; skx++)
                            {
                                vx_uint32 realWeightX = weightXIndex + skx;
                                vx_uint32 realWeightY = weightYIndex + sky;
                                vx_uint32 weight;

                                vx_uint8_ptr realKernelDataPtr = kernelDataPtr + (realWeightX * weight_x + realWeightY) * weightSize;

                                if (realWeightX == 0 &&
                                    realWeightY == 0 &&
                                    !biasAtEnd)
                                {
                                    if (weightFomat == VX_TYPE_INT16)
                                    {
                                        vx_int64 bias64 = 0;
                                        vx_int32 bias32;
                                        vx_int16 bias16;

                                        if ((vx_int32)biasData < 0)
                                        {
                                            bias64 = ~0;
                                            bias64 = (bias64 >> 32) << 32;
                                            bias64 = bias64 | (vx_int64)biasData;
                                        }
                                        else
                                            bias64 = (vx_int64)biasData;

                                        bias32 = (vx_int32)bias64;
                                        writeBits(&kernelBufferPtr, &bitOffset, bias32, 32);
                                        bias16 = (vx_int16)(bias64 >> 32);
                                        writeBits(&kernelBufferPtr, &bitOffset, (vx_int32)bias16, 16);
                                    }
                                    else
                                        writeBits(&kernelBufferPtr, &bitOffset, biasData, biasBitSize);
                                }

                                if (weightFomat == VX_TYPE_INT8)
                                    weight = *((vx_int8 *)realKernelDataPtr);
                                else if (weightFomat == VX_TYPE_UINT8)
                                    weight = *((vx_uint8 *)realKernelDataPtr);
                                else
                                    weight = *((vx_uint16 *)realKernelDataPtr);

                                if ((zeroRun == max_zero_run)
                                        || ((realWeightX == weight_x - 1)
                                        && (realWeightY == weight_y - 1))
                                        || (weight != skip_value))
                                {
                                    if (min_zero_run_len)
                                    {
                                        writeBits(&kernelBufferPtr, &bitOffset, zeroRun, min_zero_run_len);
                                    }
                                    writeBits(&kernelBufferPtr, &bitOffset, weight, weightBitSize);
                                    zeroRun = 0;
                                }
                                else
                                {
                                    zeroRun++;
                                }

                                writeBits(&kernelBufferPtr, &bitOffset, weight, weightBitSize);

                                if (realWeightX == weight_x - 1 &&
                                    realWeightY == weight_y - 1)
                                {
                                    /*ZOFFSET*/
                                    vx_uint32 offsetValue;
                                    if (z_offset > 0)
                                        offsetValue = (vx_uint32)z_offset * realSliceIndex;
                                    else if (outputSize > 0)
                                        offsetValue = output_final_x * output_final_y * outputSize * realSliceIndex;
                                    else
                                        offsetValue = output_final_x * output_final_y * weightSize * realSliceIndex;

                                    writeBits(&kernelBufferPtr, &bitOffset, offsetValue, NN_Z_POSITION_OFFSET_BITS_VIP_V7);

                                    /*Bias at the end*/
                                    if (biasAtEnd)
                                    {
                                        if (weightFomat == VX_TYPE_INT16)
                                        {
                                            vx_int64 bias64 = 0;
                                            vx_int32 bias32;
                                            vx_int16 bias16;

                                            if ((vx_int32)biasData < 0)
                                            {
                                                bias64 = ~0;
                                                bias64 = (bias64 >> 32) << 32;
                                                bias64 = bias64 | (vx_int64)biasData;
                                            }
                                            else
                                                bias64 = (vx_int64)biasData;

                                            bias32 = (vx_int32)bias64;
                                            writeBits(&kernelBufferPtr, &bitOffset, bias32, 32);
                                            bias16 = (vx_int16)(bias64 >> 32);
                                            writeBits(&kernelBufferPtr, &bitOffset, (vx_int32)bias16, 16);
                                        }
                                        else
                                            writeBits(&kernelBufferPtr, &bitOffset, biasData, biasBitSize);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        /* pad 0 */
        packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);

        /* Go back to update kernelStreamSize. */
        kernelStreamSize = (vx_uint32)((vx_uint8 *)kernelBufferPtr - kernelStreamBasePtr);
        /*With RTL bug fix, full cache mode size needn't using maxSize * coreCount*/
        wb->slice_array[index].kernel_stream_full_cache_size += kernelStreamSize;

        if (kernelStreamSize > maxKernelStreamSizePerCore)
            maxKernelStreamSizePerCore = kernelStreamSize;

        writeBits(&kernelStreamSizePtr, &streamSizeBitOffset, kernelStreamSize, 32);
    }

    wb->slice_array[index].kernel_align_stream_size += (vx_size)(maxKernelStreamSizePerCore * nnCoreCount);

    if (z_offset > 0)
        wb->orgZOffsetValue = z_offset;
    else if (outputSize > 0)
        wb->orgZOffsetValue = output_final_x * output_final_y * outputSize;
    else
        wb->orgZOffsetValue = output_final_x * output_final_y * weightSize;

exit:
    if (weightsMinusZP)
        vxFree(weightsMinusZP);

    return;
}

void fillinKernelBufferBalance(
    vx_context context,
    vx_weights_biases_parameter wb,
    vx_uint8 min_zero_run_len,
    vx_uint32 max_zero_run,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 weight_z,
    vx_uint32 filter_count,
    vx_uint32 kernels_per_core,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_int32 input_zp,
    vx_int32 coef_zp,
    vx_uint32 skip_value,
    vx_uint32 output_final_x,
    vx_uint32 output_final_y,
    vx_int32 z_offset,
    vx_uint32 outputSize,
    vx_uint8_ptr wb_base_ptr,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr,
    vx_uint32 index
    )
{
    vx_enum weightFomat = weight_format;
    vx_int32 inputZP = input_zp;
    vx_int32 coefZP = coef_zp;
    vx_uint32 skipValue = skip_value;

    vx_uint32 nnCoreCount = (weightFomat == VX_TYPE_INT16) ?
        context->nnConfig.fixedFeature.nnCoreCountInt16 : (weightFomat == VX_TYPE_FLOAT16) ?
            context->nnConfig.fixedFeature.nnCoreCountFloat16 : context->nnConfig.fixedFeature.nnCoreCount;

    vx_uint32 coreIndex;
    vx_uint32 groupIndex, filterIndex, sliceIndex;
    vx_uint32 weightXIndex, weightYIndex;
    vx_uint32 bitOffset          = 0;
    vx_uint32 zeroRun            = 0;

    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize(weightFomat);
    vx_uint32 weightBitSize      = weightSize * 8;
    vx_uint32 biasSize           = (vx_uint32)vxDataType_GetSize(bias_format);
    vx_uint32 biasBitSize        = biasSize * 8;
    vx_uint32 biasData           = 0;
    vx_int64  biasData_64bits    = 0;

    vx_uint32 sliceCount         = weight_z;
    vx_uint32 filterSliceSize    = weight_x * weight_y * weightSize;
    vx_uint32 filterSize         = weight_x * weight_y * sliceCount * weightSize;

    vx_uint32 filterTotalCount   = filter_count;
    vx_uint32 filterCount        = kernels_per_core; /* filter count every group */
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;
    vx_uint32 totalFilterPerCore = filterTotalCount / nnCoreCount;
    vx_uint32 oddFilterPerCore   = filterTotalCount % nnCoreCount;

    vx_uint32 kernelStreamSize   = ((filterSize * filterTotalCount + filterTotalCount * biasSize + filterTotalCount * 3 + 3) + 63) & ~63;
    vx_uint32* kernelBufferPtr   = (vx_uint32*) wb_base_ptr;
    vx_uint32* kernelStreamSizePtr = kernelBufferPtr;
    vx_uint32 streamSizeBitOffset = 0;
    vx_uint32 alignedOffset      = ((vx_uint32)(gctUINTPTR_T)kernelBufferPtr) & 0x3F;
    vx_uint8_ptr kernelDataPtr   = VX_NULL;
    vx_uint8_ptr kernelStreamBasePtr = VX_NULL;
    vx_uint32 usedCoreCount      = (filterTotalCount > nnCoreCount) ? nnCoreCount : filterTotalCount;
    vx_uint32 maxKernelStreamSizePerCore = 0;
    vx_float64* nonZeroRatioPerCorePerVZG = 0;
    vx_uint32 totalNonZeroCount = 0;
    vx_float32 averageNZCPerFilter = 0;
    vx_float32 varianceFL32 = 0;
    vx_uint32 variance = 0;
    vx_bool reorder = vx_false_e;

    typedef struct _gcVXWeightsMinusZP
    {
        vx_int32 sum;
        vx_uint32 filterIdx;
    }gcVXWeightsMinusZP;

    gcVXWeightsMinusZP *weightsMinusZP = VX_NULL;

    typedef struct _gcVXNonZeroWeights
    {
        vx_uint32 nonZeroCount;
        vx_uint32 filterIdx;
    }gcVXNonZeroWeights;

#if PRINT_PER_CORE_SIZE
    vx_uint32* nonZeroCoefPerCore = VX_NULL;
#endif
    gcVXNonZeroWeights *nonZeroWeights = VX_NULL;
    vx_int32 i = 0;
    vx_uint32 core0FilterCount = 0;
    gcVXNonZeroWeights tmp;

    nonZeroWeights = (gcVXNonZeroWeights*)vxAllocateAndZeroMemory(filter_count * sizeof(gcVXNonZeroWeights));
    if (nonZeroWeights == VX_NULL)
    {
        vxError("fillinKernelBuffer: OUT OF MEMORY");
        goto exit;
    }
#if PRINT_PER_CORE_SIZE
    nonZeroCoefPerCore = (vx_uint32*)vxAllocateAndZeroMemory(nnCoreCount * sizeof(vx_uint32));
    if (nonZeroCoefPerCore == VX_NULL)
    {
        vxError("fillinKernelBuffer: OUT OF MEMORY");
        goto exit;
    }
#endif
    /* calc each filter non-zero weights num*/
    for (filterIndex = 0; filterIndex < filter_count; filterIndex++)
    {
        nonZeroWeights[filterIndex].filterIdx = filterIndex;
        for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
        {
            /* add one slice data every filter */
            kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * sliceIndex;

            for (weightYIndex = 0; weightYIndex < weight_y; weightYIndex++)
            {
                for (weightXIndex = 0; weightXIndex < weight_x; weightXIndex++)
                {
                    vx_uint32 weight = 0;

                    if (weightFomat == VX_TYPE_INT8)
                        weight = *((vx_int8 *)kernelDataPtr);
                    else if (weightFomat == VX_TYPE_UINT8)
                        weight = *((vx_uint8 *)kernelDataPtr);
                    else
                        weight = *((vx_uint16 *)kernelDataPtr);
                    kernelDataPtr = kernelDataPtr + weightSize;

                    if (weight != skipValue)
                        nonZeroWeights[filterIndex].nonZeroCount++;

                }
            }
        }
        totalNonZeroCount += nonZeroWeights[filterIndex].nonZeroCount;
    }

    /*Sort*/
    averageNZCPerFilter = (vx_float32)totalNonZeroCount / filter_count;
    for (filterIndex = 0; filterIndex < filter_count; filterIndex++)
    {
        varianceFL32 += gcoMATH_Power((nonZeroWeights[filterIndex].nonZeroCount - averageNZCPerFilter), 2);
    }
    variance = (vx_uint32)(varianceFL32 / filter_count + 0.5f);

    if (variance > 12)
        reorder = vx_true_e;

    if (reorder)
    {
        /*Sort*/
        for (filterIndex = 0; filterIndex < filter_count; filterIndex++)
        {
            tmp.nonZeroCount = nonZeroWeights[filterIndex].nonZeroCount;
            tmp.filterIdx = filterIndex;

            for (i = filterIndex - 1; i >= 0 && nonZeroWeights[i].nonZeroCount > tmp.nonZeroCount; i--)
            {
                nonZeroWeights[i+1].nonZeroCount = nonZeroWeights[i].nonZeroCount;
                nonZeroWeights[i+1].filterIdx = nonZeroWeights[i].filterIdx;
            }
            nonZeroWeights[i+1].nonZeroCount = tmp.nonZeroCount;
            nonZeroWeights[i+1].filterIdx = tmp.filterIdx;
        }
    }

    if (weightFomat == VX_TYPE_INT8 || weightFomat == VX_TYPE_UINT8)
    {
        if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
        {
            biasBitSize = NN_INTEGER_BIAS_BITS_VIP_V7;
        }
        else
        {
            biasBitSize = NN_INTEGER_BIAS_BITS;
        }
    }
    else if (weightFomat == VX_TYPE_INT16)
    {
        biasBitSize = NN_INTEGER_BIAS_BITS_VIP_V7_INT16;
    }

    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT) && inputZP != 0 && weightFomat == VX_TYPE_UINT8)
    {
        weightsMinusZP = (gcVXWeightsMinusZP*)vxAllocateAndZeroMemory(filter_count * sizeof(gcVXWeightsMinusZP));

        if (!weightsMinusZP)
        {
            vxError("fillinKernelBufferBalance: OUT OF MEMORY");
            goto exit;
        }

        /*Calc sum((coef[i] - coefZP) * InZP) */
        for (filterIndex = 0; filterIndex < filter_count; filterIndex++)
        {
            weightsMinusZP[filterIndex].filterIdx = filterIndex;
            weightsMinusZP[filterIndex].sum = 0;
            for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
            {
                /* add one slice data every filter */
                kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * sliceIndex;

                for (weightYIndex = 0; weightYIndex < weight_y; weightYIndex++)
                {
                    for (weightXIndex = 0; weightXIndex < weight_x; weightXIndex++)
                    {
                        vx_int32 weight = 0;
                        weight = *((vx_uint8 *)kernelDataPtr);
                        kernelDataPtr = kernelDataPtr + weightSize;

                        weightsMinusZP[filterIndex].sum += (weight - coefZP) * inputZP;
                    }
                }
            }
        }
    }

    if (wb->zOffsetHandle == VX_NULL)
    {
        wb->zOffsetHandle = (vx_weights_biases_z_offset)vxAllocateAndZeroMemory(filter_count * sizeof(vx_weights_biases_z_offset_s));
        if (wb->zOffsetHandle == VX_NULL)
        {
            vxError("fillinKernelBufferBalance: OUT OF MEMORY");
            goto exit;
        }
    }

    if (wb->max_per_core_per_vzgroup_nzr == VX_NULL)
    {
        wb->max_per_core_per_vzgroup_nzr = (vx_float64*)vxAllocateAndZeroMemory(groupCount * sizeof(vx_float64));
        if (wb->max_per_core_per_vzgroup_nzr == VX_NULL)
        {
            vxError("fillinKernelBuffer: OUT OF MEMORY\n");
            goto exit;
        }
    }

    nonZeroRatioPerCorePerVZG = (vx_float64*)vxAllocateAndZeroMemory(nnCoreCount * groupCount * sizeof(vx_float64));
    if (nonZeroRatioPerCorePerVZG == VX_NULL)
    {
        vxError("fillinKernelBuffer: OUT OF MEMORY");
        goto exit;
    }

    wb->numOfVz = filter_count;

    /* kernel Buffer size for each core */
    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        writeBits(&kernelBufferPtr, &bitOffset, kernelStreamSize, 32);
    }
    packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);
    wb->slice_array[index].kernel_stream_full_cache_size = 0;

    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        vx_uint32 coreFilterCount = (coreIndex < oddFilterPerCore) ? totalFilterPerCore + 1 : totalFilterPerCore;
        vx_uint32 usedKid = 0;
        vx_uint32* filterGroup = VX_NULL;
        vx_uint32 coreOrgKernelSize = coreFilterCount * (filterSize + biasSize);
        vx_float64 compressionRatio = 0;

        kernelStreamBasePtr = (vx_uint8_ptr)kernelBufferPtr;
        if (coreFilterCount > 0)
        {
            /* zeroRunLen */
            writeBits(&kernelBufferPtr, &bitOffset, min_zero_run_len, 8);

            writeBits(&kernelBufferPtr, &bitOffset, coreFilterCount, 16);
        }
        /* fill weight value and bias for every group */
        zeroRun = 0;
        for (groupIndex = 0; groupIndex < groupCount; groupIndex++)
        {
            vx_uint32 actualFilterCount = (groupIndex == groupCount - 1) ? (coreFilterCount - groupIndex * filterCount) : filterCount;
            vx_uint32 groupFilterStart = groupIndex * nnCoreCount * filterCount;
            vx_uint32 kid = 0;
            vx_uint32 unusedCoreCount = (filterTotalCount % nnCoreCount == 0) ? 0 : nnCoreCount - (filterTotalCount % nnCoreCount);
            vx_uint32 sortedIndex = 0;
            vx_uint32 blockCount = 0;
            vx_uint32 nonZeroCount = 0;
            vx_uint32 nonZrlIdx = groupIndex + groupCount * coreIndex;
            vx_uint32 filterStart;

            if (coreIndex == 0)
                core0FilterCount = actualFilterCount;

            if (actualFilterCount)
            {
                filterGroup = (vx_uint32*)vxAllocateAndZeroMemory(actualFilterCount * sizeof(vx_uint32));
                if (filterGroup == VX_NULL)
                {
                    vxError("fillinKernelBufferBalance: OUT OF MEMORY");
                    goto exit;
                }
            }

            if (reorder)
            {
                /*Dispatch filter to each vzGroup*/
                for (kid = 0; kid < actualFilterCount; kid++)
                {
                    if (groupIndex == groupCount - 1 &&
                        kid == core0FilterCount - 1)
                    {
                        if ((kid + usedKid) % 2 == 0)
                            sortedIndex = groupFilterStart + kid * nnCoreCount + coreIndex;
                        else
                            sortedIndex = groupFilterStart + (kid + 1) * nnCoreCount - coreIndex - 1 - unusedCoreCount;
                    }
                    else
                    {
                        if ((kid + usedKid) % 2 == 0)
                            sortedIndex = groupFilterStart + kid * nnCoreCount + coreIndex;
                        else
                            sortedIndex = groupFilterStart + (kid + 1) * nnCoreCount - coreIndex - 1;
                    }

                    filterGroup[kid] = nonZeroWeights[sortedIndex].filterIdx;
#if PRINT_PER_CORE_SIZE
                    nonZeroCoefPerCore[coreIndex] += nonZeroWeights[sortedIndex].nonZeroCount;
#endif
                }

                /*sort the vz group real filter to avoid write output cross*/
                for (kid = 0; kid < actualFilterCount; kid++)
                {
                    vx_uint32 tmp = filterGroup[kid];

                    for (i = kid - 1; i >= 0 && filterGroup[i] > tmp; i--)
                    {
                        filterGroup[i+1] = filterGroup[i];
                    }
                    filterGroup[i+1] = tmp;
                }
            }
            else
            {
                /*if variance is small, don't reorder the filters*/
                if ((groupIndex == groupCount - 1)
                && (coreIndex >= (filterTotalCount % nnCoreCount)))
                {
                    filterStart = groupIndex * nnCoreCount * filterCount
                                + (filterTotalCount % nnCoreCount) * (actualFilterCount + 1)
                                + (coreIndex - (filterTotalCount % nnCoreCount)) * actualFilterCount;
                }
                else
                {
                    filterStart = groupIndex * nnCoreCount *filterCount + coreIndex * actualFilterCount;
                }

                for (kid = 0; kid < actualFilterCount; kid++)
                {
                    filterGroup[kid] = filterStart + kid;
                }
            }

            /* Only UINT8/INT8 support ZDP3/ZDP6 */
            if (weight_x == 1 && weight_y == 1
                && (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3) || vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6))
                && (weightFomat == VX_TYPE_INT8 || weightFomat == VX_TYPE_UINT8))
            {
                vx_uint32 zdpNum = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6) ? 6 : 3;
                /* zdp3 zdp6 can not enable same time */
                vxmASSERT(!(vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3) && vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6)));

                for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex += (zdpNum * 2))
                {
                    vx_uint8 group2DCoded = 0;
                    vx_uint32 weight = 0;
                    vx_uint32 realSliceIndex = 0;

                    /* add slices of every filter*/
                    for (kid = 0; kid < actualFilterCount; kid++)
                    {
                        vx_uint32 nonZeroInBlock1Count = 0; /*Check if there's non-zero point in first zdpNum block*/
                        vx_uint32 nonZeroInBlock2Count = 0; /*Check if there's non-zero point in another zdpNum block*/
                        filterIndex = filterGroup[kid];

                        for (realSliceIndex = sliceIndex; realSliceIndex < sliceIndex + (zdpNum * 2); realSliceIndex++)
                        {
                            vx_uint8 needToWriteBias = (realSliceIndex == 0) ? 1 : 0;

                            if (realSliceIndex >= sliceCount)
                                break;

                            /* add one slice data every filter */
                            kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * realSliceIndex;
                            if (realSliceIndex == 0)
                            {
                                if (bias_base_ptr)
                                    biasData = *(bias_base_ptr + filterIndex);
                                else
                                    biasData = 0;

                                /* NewBias = sum((Coef[i] - CoefZP) * - InZP) + Bias*/
                                if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT)
                                    && inputZP != 0
                                    && weightFomat == VX_TYPE_UINT8)
                                {
                                    biasData -= weightsMinusZP[filterIndex].sum;
                                }

                            }

                            if (weightFomat == VX_TYPE_INT8)
                                weight = *((vx_int8 *)kernelDataPtr);
                            else if (weightFomat == VX_TYPE_UINT8)
                                weight = *((vx_uint8 *)kernelDataPtr);
                            else
                                weight = *((vx_uint16 *)kernelDataPtr);

                            if (realSliceIndex - sliceIndex < zdpNum && weight != skipValue)
                                nonZeroInBlock1Count++;
                            else if (weight != skipValue)
                                nonZeroInBlock2Count++;

                            if ((zeroRun == max_zero_run)
                                || ((realSliceIndex == 0)
                                    && needToWriteBias)
                                || (realSliceIndex == sliceCount - 1)
                                || (weight != skipValue)
                                || ((kid == actualFilterCount - 1)
                                    && (group2DCoded == 0)))
                            {
                                if (min_zero_run_len)
                                {
                                    writeBits(&kernelBufferPtr, &bitOffset, zeroRun, min_zero_run_len);
                                }
                                writeBits(&kernelBufferPtr, &bitOffset, weight, weightBitSize);
                                zeroRun = 0;

                                if (needToWriteBias)
                                {
                                    if (weightFomat == VX_TYPE_INT16)
                                    {
                                        vx_int64 bias64 = 0;
                                        vx_int32 bias32;
                                        vx_int16 bias16;

                                        if ((vx_int32)biasData < 0)
                                        {
                                            bias64 = ~0;
                                            bias64 = (bias64 >> 32) << 32;
                                            bias64 = bias64 | (vx_int64)biasData;
                                        }
                                        else
                                            bias64 = (vx_int64)biasData;

                                        bias32 = (vx_int32)bias64;
                                        writeBits(&kernelBufferPtr, &bitOffset, bias32, 32);
                                        bias16 = (vx_int16)(bias64 >> 32);
                                        writeBits(&kernelBufferPtr, &bitOffset, (vx_int32)bias16, 16);
                                    }
                                    else
                                        writeBits(&kernelBufferPtr, &bitOffset, biasData, biasBitSize);
                                    needToWriteBias = 0;
                                }
                                group2DCoded = 1;
                            }
                            else
                            {
                                zeroRun++;
                            }

                            /* add offet behind the last point of the last slice of each filter */
                            if (realSliceIndex == sliceCount - 1)
                            {
                                vx_uint32 offsetValue;
                                if (z_offset > 0)
                                    offsetValue = (vx_uint32)z_offset * filterIndex;
                                else if (outputSize > 0)
                                    offsetValue = output_final_x * output_final_y * outputSize * filterIndex;
                                else
                                    offsetValue = output_final_x * output_final_y * weightSize * filterIndex;

                                if (wb->zOffsetHandle)
                                {
                                    wb->zOffsetHandle[filterIndex].ptrOffset = (vx_uint32)((vx_uint8 *)kernelBufferPtr - wb_base_ptr);
                                    wb->zOffsetHandle[filterIndex].bitOffset = bitOffset;
                                }

                                if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                    writeBits(&kernelBufferPtr, &bitOffset, offsetValue, NN_Z_POSITION_OFFSET_BITS_VIP_V7);
                                else
                                    writeBits(&kernelBufferPtr, &bitOffset, offsetValue, NN_Z_POSITION_OFFSET_BITS);
                            }
                        }
                        if (sliceIndex + zdpNum >= sliceCount)
                            blockCount++;
                        else
                            blockCount += 2;

                        if (nonZeroInBlock1Count)
                            nonZeroCount++;
                        if (nonZeroInBlock2Count)
                            nonZeroCount++;
                    }
                }
            }
            /*Only INT8/UINT8 support XY DP9 now*/
            else if ((vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP9) || vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP6))
                && (weightFomat == VX_TYPE_INT8 || weightFomat == VX_TYPE_UINT8))
            {
                vx_uint32 xStep = 0, yStep = 0;
                vx_uint32 inImageBufferSize = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP9) ? 2 : 1;

                /* xydp9 xydp6 can not enable same time */
                vxmASSERT(!(vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP9) && vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP6)));

                if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP6))
                {
                    xStep = 3;
                    yStep = 2;
                }
                else
                {
                    xStep = 3;
                    yStep = 3;
                }

                for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex += inImageBufferSize)
                {
                    vx_uint8 group2DCoded = 0;
                    vx_uint32 weight = 0;
                    vx_uint32 realSliceIndex = 0;
                    vx_uint32 realWeightXIndex = 0, realWeightYIndex = 0;
                    vx_uint32 subKxSize = 0, subKySize = 0, subKzSize = 0;

                    subKzSize = gcmMIN((sliceCount - sliceIndex), inImageBufferSize);
                    /* add slices of every filter*/
                    for (kid = 0; kid < actualFilterCount; kid++)
                    {
                        vx_bool hasWriteBias = vx_false_e;

                        filterIndex = filterGroup[kid];

                        for (weightYIndex = 0; weightYIndex < weight_y; weightYIndex += yStep)
                        {
                            subKySize = gcmMIN((weight_y - weightYIndex), yStep);
                            for (weightXIndex = 0; weightXIndex < weight_x; weightXIndex += xStep)
                            {
                                subKxSize = gcmMIN((weight_x - weightXIndex), xStep);
                                for (realSliceIndex = sliceIndex; realSliceIndex < sliceIndex + subKzSize; realSliceIndex++)
                                {
                                    vx_uint8 needToWriteBias = (sliceIndex == 0) ? 1 : 0;
                                    vx_uint32 nonZeroInBlockCount = 0; /*Check if there's non-zero point in one 3x3 block*/

                                    /* add one slice data every filter */
                                    kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * realSliceIndex;
                                    if (realSliceIndex == 0)
                                    {
                                        if (bias_base_ptr)
                                            biasData = *(bias_base_ptr + filterIndex);
                                        else
                                            biasData = 0;

                                        /* NewBias = sum((Coef[i] - CoefZP) * - InZP) + Bias*/
                                        if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT)
                                            && inputZP != 0
                                            && weightFomat == VX_TYPE_UINT8)
                                        {
                                            biasData -= weightsMinusZP[filterIndex].sum;
                                        }
                                    }

                                    for (realWeightYIndex = weightYIndex; realWeightYIndex < weightYIndex + subKySize; realWeightYIndex++)
                                    {
                                        for (realWeightXIndex = weightXIndex; realWeightXIndex < weightXIndex + subKxSize; realWeightXIndex++)
                                        {
                                            vx_uint8_ptr realKernelDataPtr = kernelDataPtr + (realWeightYIndex * weight_x + realWeightXIndex) * weightSize;

                                            if (weightFomat == VX_TYPE_INT8)
                                                weight = *((vx_int8 *)realKernelDataPtr);
                                            else if (weightFomat == VX_TYPE_UINT8)
                                                weight = *((vx_uint8 *)realKernelDataPtr);
                                            else
                                                weight = *((vx_uint16 *)realKernelDataPtr);

                                            if (weight != skipValue)
                                                nonZeroInBlockCount++;

                                            if ((zeroRun == max_zero_run)
                                                || ((realSliceIndex == 0)
                                                    && (realWeightXIndex == weight_x - 1)
                                                    && (realWeightYIndex == weight_y - 1)
                                                    && needToWriteBias)
                                                || ((realSliceIndex == sliceCount - 1)
                                                    && (realWeightXIndex == weight_x - 1)
                                                    && (realWeightYIndex == weight_y - 1))
                                                || (weight != skipValue)
                                                || ((kid == actualFilterCount - 1)
                                                    && (realWeightXIndex == weight_x - 1)
                                                    && (realWeightYIndex == weight_y - 1)
                                                    && (group2DCoded == 0)))
                                            {
                                                if (min_zero_run_len)
                                                {
                                                    writeBits(&kernelBufferPtr, &bitOffset, zeroRun, min_zero_run_len);
                                                }
                                                writeBits(&kernelBufferPtr, &bitOffset, weight, weightBitSize);
                                                zeroRun = 0;

                                                if (needToWriteBias && !hasWriteBias)
                                                {
                                                    if (weightFomat == VX_TYPE_INT16)
                                                    {
                                                        vx_int64 bias64 = 0;
                                                        vx_int32 bias32;
                                                        vx_int16 bias16;

                                                        if ((vx_int32)biasData < 0)
                                                        {
                                                            bias64 = ~0;
                                                            bias64 = (bias64 >> 32) << 32;
                                                            bias64 = bias64 | (vx_int64)biasData;
                                                        }
                                                        else
                                                            bias64 = (vx_int64)biasData;

                                                        bias32 = (vx_int32)bias64;
                                                        writeBits(&kernelBufferPtr, &bitOffset, bias32, 32);
                                                        bias16 = (vx_int16)(bias64 >> 32);
                                                        writeBits(&kernelBufferPtr, &bitOffset, (vx_int32)bias16, 16);
                                                    }
                                                    else
                                                        writeBits(&kernelBufferPtr, &bitOffset, biasData, biasBitSize);
                                                    needToWriteBias = 0;
                                                    hasWriteBias = vx_true_e;
                                                }
                                                group2DCoded = 1;
                                            }
                                            else
                                            {
                                                zeroRun++;
                                            }

                                            /* add offet behind the last point of the last slice of each filter */
                                            if (realSliceIndex == sliceCount - 1 && realWeightXIndex == weight_x - 1 && realWeightYIndex == weight_y - 1)
                                            {
                                                vx_uint32 offsetValue;

                                                if (z_offset > 0)
                                                    offsetValue = (vx_uint32)z_offset * filterIndex;
                                                else if (outputSize > 0)
                                                    offsetValue = output_final_x * output_final_y * outputSize * filterIndex;
                                                else
                                                    offsetValue = output_final_x * output_final_y * weightSize * filterIndex;

                                                if (wb->zOffsetHandle)
                                                {
                                                    wb->zOffsetHandle[filterIndex].ptrOffset = (vx_uint32)((vx_uint8 *)kernelBufferPtr - wb_base_ptr);
                                                    wb->zOffsetHandle[filterIndex].bitOffset = bitOffset;
                                                }

                                                if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                                    writeBits(&kernelBufferPtr, &bitOffset, offsetValue, NN_Z_POSITION_OFFSET_BITS_VIP_V7);
                                                else
                                                    writeBits(&kernelBufferPtr, &bitOffset, offsetValue, NN_Z_POSITION_OFFSET_BITS);
                                            }
                                        }
                                    }
                                    blockCount++;
                                    if (nonZeroInBlockCount)
                                        nonZeroCount++;
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
                {
                    vx_uint8 group2DCoded = 0;
                    /* add slices of every filter*/
                    for (kid = 0; kid < actualFilterCount; kid++)
                    {
                        vx_uint8 needToWriteBias = (sliceIndex == 0) ? 1 : 0;
                        filterIndex = filterGroup[kid];

                        /* add one slice data every filter */
                        kernelDataPtr = weight_base_ptr + filterIndex * filterSize + filterSliceSize * sliceIndex;
                        if (sliceIndex == 0)
                        {
                            if (bias_base_ptr)
                            {
                                if(bias_format == VX_TYPE_INT64)
                                    biasData_64bits = *(((vx_int64 *)bias_base_ptr) + filterIndex);
                                else
                                    biasData = *(bias_base_ptr + filterIndex);
                            }
                            else
                                biasData = 0;

                            /* NewBias = sum((Coef[i] - CoefZP) * - InZP) + Bias*/
                            if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT)
                                && inputZP != 0
                                && weightFomat == VX_TYPE_UINT8)
                            {
                                biasData -= weightsMinusZP[filterIndex].sum;
                            }
                        }

                        for (weightYIndex = 0; weightYIndex < weight_y; weightYIndex++)
                        {
                            vx_uint32 nonZeroCountInDP3 = 0; /*Check if there's non-zero point in one 3x1 block*/
                            for (weightXIndex = 0; weightXIndex < weight_x; weightXIndex++)
                            {
                                vx_uint32 weight = 0;

                                if (weightFomat == VX_TYPE_INT8)
                                    weight = *((vx_int8 *)kernelDataPtr);
                                else if (weightFomat == VX_TYPE_UINT8)
                                    weight = *((vx_uint8 *)kernelDataPtr);
                                else
                                    weight = *((vx_uint16 *)kernelDataPtr);
                                kernelDataPtr = kernelDataPtr + weightSize;

                                {
                                    if (weight != skipValue)
                                        nonZeroCountInDP3++;

                                    /*V7, DP3, one block is 3x1*/
                                    if ((weightXIndex + 1) % 3 == 0 || weightXIndex == wb->weights_sizes[0] - 1)
                                    {
                                        blockCount++;
                                        if (nonZeroCountInDP3)
                                            nonZeroCount++;
                                        nonZeroCountInDP3 = 0;
                                    }
                                }

                                if ((zeroRun == max_zero_run)
                                    || ((sliceIndex == 0)
                                        && (weightXIndex == weight_x - 1)
                                        && (weightYIndex == weight_y - 1)
                                        && needToWriteBias)
                                    || ((weightXIndex == weight_x - 1)
                                        && (weightYIndex == weight_y - 1)
                                        && (sliceIndex == sliceCount - 1))
                                    || (weight != skipValue)
                                    || ((kid == actualFilterCount - 1)
                                        && (weightXIndex == weight_x - 1)
                                        && (weightYIndex == weight_y - 1)
                                        && (group2DCoded == 0)))
                                {
                                    if (min_zero_run_len)
                                    {
                                        writeBits(&kernelBufferPtr, &bitOffset, zeroRun, min_zero_run_len);
                                    }
                                    writeBits(&kernelBufferPtr, &bitOffset, weight, weightBitSize);
                                    zeroRun = 0;

                                    if (needToWriteBias)
                                    {
                                        if (weightFomat == VX_TYPE_INT16)
                                        {
                                            vx_int64 bias64 = 0;
                                            vx_int32 bias32;
                                            vx_int16 bias16;

                                            /*64bits bias*/
                                            if(bias_format == VX_TYPE_INT64)
                                            {
                                                bias32 = (vx_int32)biasData_64bits;
                                                writeBits(&kernelBufferPtr, &bitOffset, bias32, 32);
                                                bias16 = (vx_int16)(biasData_64bits >> 32);
                                                writeBits(&kernelBufferPtr, &bitOffset, (vx_int32)bias16, 16);
                                            }
                                            else    /*32bits bias*/
                                            {
                                                if ((vx_int32)biasData < 0)
                                                {
                                                    bias64 = ~0;
                                                    bias64 = (bias64 >> 32) << 32;
                                                    bias64 = bias64 | (vx_int64)biasData;
                                                }
                                                else
                                                    bias64 = (vx_int64)biasData;

                                                bias32 = (vx_int32)bias64;
                                                writeBits(&kernelBufferPtr, &bitOffset, bias32, 32);
                                                bias16 = (vx_int16)(bias64 >> 32);
                                                writeBits(&kernelBufferPtr, &bitOffset, (vx_int32)bias16, 16);
                                            }
                                        }
                                        else
                                            writeBits(&kernelBufferPtr, &bitOffset, biasData, biasBitSize);
                                        needToWriteBias = 0;
                                    }
                                    group2DCoded = 1;
                                }
                                else
                                {
                                    zeroRun++;
                                }
                            }
                        }

                        /* add offet behind the last point of the last slice of each filter */
                        if (sliceIndex == sliceCount - 1)
                        {
                            vx_uint32 offsetValue;
                            if (z_offset > 0)
                                offsetValue = (vx_uint32)z_offset * filterIndex;
                            else if (outputSize > 0)
                                offsetValue = output_final_x * output_final_y * outputSize * filterIndex;
                            else
                                offsetValue = output_final_x * output_final_y * weightSize * filterIndex;

                            if (wb->zOffsetHandle)
                            {
                                wb->zOffsetHandle[filterIndex].ptrOffset = (vx_uint32)((vx_uint8 *)kernelBufferPtr - wb_base_ptr);
                                wb->zOffsetHandle[filterIndex].bitOffset = bitOffset;
                            }

                            if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                                writeBits(&kernelBufferPtr, &bitOffset, offsetValue, NN_Z_POSITION_OFFSET_BITS_VIP_V7);
                            else
                                writeBits(&kernelBufferPtr, &bitOffset, offsetValue, NN_Z_POSITION_OFFSET_BITS);

                        }
                    }
                }
            }
            nonZeroRatioPerCorePerVZG[nonZrlIdx] = (vx_float64)nonZeroCount / (vx_float64)blockCount;

            usedKid += actualFilterCount;
            if (filterGroup)
            {
                vxFree(filterGroup);
                filterGroup = VX_NULL;
            }
        }

        /* pad 0 */
        packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);

        /* Go back to update kernelStreamSize. */
        kernelStreamSize = (vx_uint32)((vx_uint8 *)kernelBufferPtr - kernelStreamBasePtr);
        /*With RTL bug fix, full cache mode size needn't using maxSize * coreCount*/
        wb->slice_array[index].kernel_stream_full_cache_size += kernelStreamSize;

        compressionRatio = (vx_float64)kernelStreamSize / (vx_float64)coreOrgKernelSize;
        if (wb->max_per_core_compression_ratio < compressionRatio)
            wb->max_per_core_compression_ratio = compressionRatio;

        if (kernelStreamSize > maxKernelStreamSizePerCore)
            maxKernelStreamSizePerCore = kernelStreamSize;

        writeBits(&kernelStreamSizePtr, &streamSizeBitOffset, kernelStreamSize, 32);
#if PRINT_PER_CORE_SIZE
        printf("coreIdx %d, kernel stream size is %d, non-zero coef count is %d\n", coreIndex, kernelStreamSize, nonZeroCoefPerCore[coreIndex]);
#endif
    }

    wb->slice_array[index].kernel_align_stream_size += (vx_size)(maxKernelStreamSizePerCore * usedCoreCount);

    if (z_offset > 0)
        wb->orgZOffsetValue = z_offset;
    else if (outputSize > 0)
        wb->orgZOffsetValue = output_final_x * output_final_y * outputSize;
    else
        wb->orgZOffsetValue = output_final_x * output_final_y * weightSize;

    for (groupIndex = 0; groupIndex < groupCount; groupIndex++)
    {
        for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
        {
            vx_uint32 nonZrlIdx = groupIndex + coreIndex * groupCount;
            if (wb->max_per_core_per_vzgroup_nzr[groupIndex] < nonZeroRatioPerCorePerVZG[nonZrlIdx])
                wb->max_per_core_per_vzgroup_nzr[groupIndex] = nonZeroRatioPerCorePerVZG[nonZrlIdx];
        }
    }

exit:
    if (nonZeroWeights)
        vxFree(nonZeroWeights);
#if PRINT_PER_CORE_SIZE
    if (nonZeroCoefPerCore)
        vxFree(nonZeroCoefPerCore);
#endif
    if (weightsMinusZP)
        vxFree(weightsMinusZP);

    if (nonZeroRatioPerCorePerVZG)
        vxFree(nonZeroRatioPerCorePerVZG);

    return;
}

void fillinTPKernelBuffer(
    vx_weights_biases_parameter wb,
    vx_uint8* zero_run_len_bit_width,
    vx_uint32 weight_z,
    vx_uint32 filter_count,
    vx_uint32 total_weight_z,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_uint32 skip_value,
    vx_uint8_ptr wb_base_ptr,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr,
    vx_uint32 index
    )
{
    vx_uint32 skipValue             = skip_value;
    vx_enum weightFomat             = weight_format;
    vx_uint32 zeroRun               = 0;
    vx_uint32 weightSize            = (vx_uint32)vxDataType_GetSize(weightFomat);
    vx_uint32 weightBitSize         = weightSize * 8;
    vx_uint32 biasSize              = (vx_uint32)vxDataType_GetSize(bias_format);
    vx_uint32 biasBitSize           = biasSize * 8;
    vx_uint32 biasData              = 0;
    vx_uint32 sliceCount            = weight_z;
    vx_uint32 filterSize            = total_weight_z * weightSize;
    vx_uint32 filterCount           = filter_count;
    vx_uint32 kernelStreamSize      = 1;
    vx_uint32* kernelBufferPtr      = (vx_uint32*) wb_base_ptr;
    vx_uint32 bitOffset             = 0;
    vx_uint32* kernelStreamSizePtr  = kernelBufferPtr;
    vx_uint32 streamSizeBitOffset   = 0;
    vx_uint32 alignedOffset         = ((vx_uint32)(gctUINTPTR_T)kernelBufferPtr) & 0x3F;
    vx_uint8_ptr kernelDataPtr      = VX_NULL;
    vx_uint8_ptr kernelStreamBasePtr = VX_NULL;
    vx_uint32 filterIndex, sliceIndex;
    vx_uint32 rsvWeightCount = 0;

    if (weightFomat == VX_TYPE_INT8 || weightFomat == VX_TYPE_UINT8)
    {
        if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
        {
            biasBitSize = NN_INTEGER_BIAS_BITS_VIP_V7;
        }
        else
        {
            biasBitSize = NN_INTEGER_BIAS_BITS;
        }
    }

    /* Fill kernel stream size for each slice. */
    for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
    {
        writeBits(&kernelBufferPtr, &bitOffset, kernelStreamSize, 5);
    }
    packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);

    /* Fill bias for each filter. */
    for (filterIndex = 0; filterIndex < filterCount; filterIndex++)
    {
        biasData = bias_base_ptr == VX_NULL ? 0 : *(bias_base_ptr + filterIndex);
        writeBits(&kernelBufferPtr, &bitOffset, biasData, biasBitSize);
    }
    packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);

    /* Fill weight value for every slice */
    zeroRun = 0;
    for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
    {
        vx_uint8 zeroRunLenBitWidth = zero_run_len_bit_width[sliceIndex];
        vx_uint32 maxZeroRun = (1 << zeroRunLenBitWidth) - 1;

        kernelStreamBasePtr = (vx_uint8_ptr)kernelBufferPtr;
        writeBits(&kernelBufferPtr, &bitOffset, zeroRunLenBitWidth, 4);

        /* add slices of every filter*/
        kernelDataPtr = weight_base_ptr + weightSize * sliceIndex;
        for (filterIndex = 0; filterIndex < filterCount; filterIndex++)
        {
            vx_uint32 weight = 0;

            if (weightFomat == VX_TYPE_INT8)
                weight = *((vx_int8 *)kernelDataPtr);
            else if (weightFomat == VX_TYPE_UINT8)
                weight = *((vx_uint8 *)kernelDataPtr);
            else
                weight = *((vx_uint16 *)kernelDataPtr);
            kernelDataPtr += filterSize;

            if ((zeroRun == maxZeroRun)
                || (weight != skipValue)
                || (filterIndex == (filterCount - 1)))
            {
                if (zeroRunLenBitWidth)
                {
                    writeBits(&kernelBufferPtr, &bitOffset, zeroRun, zeroRunLenBitWidth);
                }
                writeBits(&kernelBufferPtr, &bitOffset, weight, weightBitSize);
                zeroRun = 0;
                rsvWeightCount++;
            }
            else
            {
                zeroRun++;
            }
        }
        packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);

        /* Go back to update kernelStreamSize. */
        kernelStreamSize = (vx_uint32)((vx_uint8 *)kernelBufferPtr - kernelStreamBasePtr);
        gcmASSERT((kernelStreamSize & 0x3F) == 0);
        gcmASSERT(kernelStreamSize < 2048);
        writeBits(&kernelStreamSizePtr, &streamSizeBitOffset, kernelStreamSize >> 6, 5);
    }
}

vx_bool WeightBiasBufferAllocate(
    vx_context context,
    vx_weights_biases_parameter weight_bias,
    vx_size size
    )
{
    vx_memory memory;

    gcmASSERT(context);
    gcmASSERT(weight_bias);

    if (!weight_bias->wb_base->memory_head_offset)
    {
        /* weight bias file has marks in its head which is aligned to 64 bytes.
         * ---|0x0A0B0C0D|wb_base|slice_num|slice_array[]|1+offsethandle|1+perf|---
         */
        WB_MEM_HEAD_OFFSET(weight_bias) = gcmALIGN((6 +
                                                    sizeof(vx_weights_biases_parameter_s) +
                                                    sizeof(vx_weights_biases_parameter_base_s) +
                                                    sizeof(vx_weights_biases_slice_s) * weight_bias->slice_num +
                                                    sizeof(vx_weights_biases_z_offset_s) +
                                                    sizeof(vx_arch_perf_s)), 64);
    }

    memory = &weight_bias->memory;

    if (memory->allocated) return vx_true_e;

    size += WB_MEM_HEAD_OFFSET(weight_bias);

    if (!vxoMemory_AllocateSize(context, memory, size)) return vx_false_e;

    memory->allocated = vx_true_e;

    weight_bias->memory_size = size;

    {
        memory->physicals[0] += WB_MEM_HEAD_OFFSET(weight_bias);
    }
    memory->logicals[0] += WB_MEM_HEAD_OFFSET(weight_bias);

    vxoMemory_Dump(memory);

    return vx_true_e;
}

vx_bool vxoNNExternsionAdjustWeightsBiases(
    vx_context                   context,
    vx_weights_biases_parameter  wb,
    vx_size                      wb_size,
    vx_uint32_ptr                kz_num_ptr,
    vx_uint32_ptr                z_num_ptr,
    vx_uint32_ptr                kz_array,
    vx_uint32_ptr                z_array
    )
{
#define FC_SIZE_MAX 134217728
    vx_uint32 i = 0, tmp = wb->weights_sizes[3];
    vx_size max = FC_SIZE_MAX;

    if (wb_size > max)
    {
        /* Weight_bias data cannot large than 128MB in old hw version. Current only for vgg FC layer. */
        do
        {
            if (tmp >= 1024)
            {
                z_array[i++] = 1024;
                tmp -= 1024;
            }
            else
            {
                z_array[i++] = tmp;
                break;
            }
        } while (tmp && i < MAX_WEIGHT_BIAS_GROUPS);

        if (i > MAX_WEIGHT_BIAS_GROUPS) return vx_false_e;
        else *z_num_ptr = i;
    }
    else
    {
        z_array[0] = wb->weights_sizes[3];
        *z_num_ptr = 1;
    }

    if (wb->weights_sizes[0] == 1 && wb->weights_sizes[1] == 1 && wb->weights_sizes[2] >= context->options.fcZMax)
    {
        vx_uint32 y = 4;
        vx_uint32 z = wb->weights_sizes[2] / y;

        while (z >= 16384)
        {
            y <<= 1;
            z = wb->weights_sizes[2] / y;
        }

        wb->weights_sizes[1] = y;
        wb->weights_sizes[2] = z;
    }

    kz_array[0] = wb->weights_sizes[2];
    *kz_num_ptr = 1;

    return vx_true_e;
}

vx_weights_biases_parameter_base vxoWeightsBiasesBase_Create(
    vx_context  context,
    vx_enum     layer_type,
    vx_uint32 * inputs_dims,
    vx_uint32   pad_x_left,
    vx_uint32   pad_x_right,
    vx_uint32   pad_y_top,
    vx_uint32   pad_y_bottom,
    vx_uint32   pooling_size_x,
    vx_uint32   pooling_size_y,
    vx_uint32   stride_x,
    vx_uint32   stride_y,
    vx_enum     down_scale_size_rounding,
    vx_uint32 * outputs_dims,
    vx_uint32 * pooling_outputs_dims,
    vx_uint32   weights_num_of_dims,
    vx_uint32 * weights_dims,
    vx_enum     weights_data_format,
    vx_enum     weights_quant_format,
    vx_int8     weights_fixed_point_pos,
    vx_int32    weights_zero_point,
    vx_float32  weights_scale,
    vx_uint32   biases_num_of_dims,
    vx_uint32 * biases_dims,
    vx_enum     biases_data_format,
    vx_enum     biases_quant_format,
    vx_int8     biases_fixed_point_pos,
    vx_float32  biases_scale,
    vx_weights_biases_parameter_optimizations_t *optimizations
    )
{
    vx_weights_biases_parameter_base wb_base = VX_NULL;
    vx_uint32 strideX = 1, strideY = 1;
    vx_uint32 alignedWidth;
    vx_uint32 alignedHeight;

    vxmASSERT(context);

    wb_base = (vx_weights_biases_parameter_base)vxoReference_Create(context, VX_TYPE_WEIGHTS_BIASES_PARAMETER_BASE, VX_REF_INTERNAL, &context->base);
    if (vxoReference_GetStatus((vx_reference)wb_base) != VX_SUCCESS)
    {
        vxError("vxoWeightsBiases_CreateBase: FAIL TO CREATE WB BASE\n");
        return VX_NULL;
    }

    wb_base->weights_num_of_dims = weights_num_of_dims;
    wb_base->weights_data_format = weights_data_format;
    wb_base->weights_quant_format = weights_quant_format;
    wb_base->weights_fixed_point_pos = weights_fixed_point_pos;
    wb_base->biases_num_of_dims = biases_num_of_dims;
    wb_base->biases_data_format = biases_data_format;
    wb_base->biases_quant_format = biases_quant_format;
    wb_base->biases_fixed_point_pos = biases_fixed_point_pos;
    wb_base->pooling_size_x = pooling_size_x;
    wb_base->pooling_size_y = pooling_size_y;
    wb_base->pad_x_left = pad_x_left;
    wb_base->pad_x_right = pad_x_right;
    wb_base->pad_y_top = pad_y_top;
    wb_base->pad_y_bottom = pad_y_bottom;
    wb_base->down_scale_size_rounding = down_scale_size_rounding;
    wb_base->setZeroLength = -1;
    if (optimizations) wb_base->setZeroLength = optimizations->zrl;
    wb_base->inputZP = 0;
    if (optimizations) wb_base->inputZP = optimizations->inputZeroPoint;
    wb_base->coefZP    = weights_zero_point;
    wb_base->coefScale = weights_scale;
    wb_base->biasScale = biases_scale;

    if (wb_base->weights_data_format == VX_TYPE_UINT8)
    {
        /* Current ZeroPoint range is 0-255(uint8)*/
        gcmASSERT(wb_base->inputZP <= 255 && wb_base->coefZP <= 255);
    }

    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT) && wb_base->weights_data_format == VX_TYPE_UINT8)
        wb_base->skipValue = wb_base->coefZP;
    else
        wb_base->skipValue = 0;

    vxMemCopy(wb_base->org_weights_sizes, weights_dims, weights_num_of_dims * sizeof(vx_uint32));
    if (biases_dims && biases_num_of_dims)
        vxMemCopy(wb_base->biases_sizes, biases_dims, biases_num_of_dims * sizeof(vx_uint32));

    wb_base->weights_sizes[3] = weights_dims[3];

    if (outputs_dims != VX_NULL && inputs_dims != NULL)
    {
        if (layer_type == VX_NN_FULLYCONNECTED_LAYER && inputs_dims[0] != 1 && inputs_dims[1] == 1)
            wb_base->nn_fc_batch_mode = vx_true_e;

        if ((stride_x > 0) && (stride_y > 0))
        {
            strideX = stride_x;
            strideY = stride_y;
        }
        else if (layer_type == VX_NN_FULLYCONNECTED_LAYER)
        {
            /* it is fully connected layer */
            strideX = strideY = 1;
        }
        else
        {
            /* Calculate stride = (w + pad_x_left + pad_x_right - weight)/(output_w - 1) */
            strideX = (outputs_dims[0] == 1) ? 1 :
                vxoNNExternsionConvlutionRound((vx_float32)(inputs_dims[0] + pad_x_left + pad_x_right - weights_dims[0]) / (outputs_dims[0] - 1), down_scale_size_rounding);
            strideY = (outputs_dims[1] == 1) ? 1 :
                vxoNNExternsionConvlutionRound((vx_float32)(inputs_dims[1] + pad_y_top + pad_y_bottom - weights_dims[1]) / (outputs_dims[1] - 1), down_scale_size_rounding);
        }
    }

    wb_base->strideX = strideX;
    wb_base->strideY = strideY;

    if (((strideX > 1) || (strideY > 1)) && (weights_dims[0] == 1) && (weights_dims[1] == 1))
    {
        wb_base->weights_sizes[0] = weights_dims[0];
        wb_base->weights_sizes[1] = weights_dims[1];
        wb_base->weights_sizes[2] = weights_dims[2];
    }
    else
    {
        /* Calculate weight_bias' weight width & height */
        alignedWidth = ((weights_dims[0] % strideX == 0) ? weights_dims[0] : (weights_dims[0] + (strideX - weights_dims[0] % strideX)));
        alignedHeight = ((weights_dims[1] % strideY == 0) ? weights_dims[1] : (weights_dims[1] + (strideY - weights_dims[1] % strideY)));
        wb_base->weights_sizes[0] = alignedWidth / strideX;
        wb_base->weights_sizes[1] = alignedHeight / strideY;
        wb_base->weights_sizes[2] = weights_dims[2] * strideX * strideY;
    }

    if (outputs_dims != VX_NULL && pooling_outputs_dims != VX_NULL)
    {
        if (pooling_outputs_dims != VX_NULL)
            wb_base->pooling_stride = outputs_dims[0] / pooling_outputs_dims[0];
        else
            wb_base->pooling_stride = 1;
    }

    wb_base->memory_pad = 64;

    return wb_base;
}

VX_INTERNAL_CALLBACK_API void vxoWeightsBiasesBase_Destructor(vx_reference ref)
{
    vx_weights_biases_parameter_base wbBase = (vx_weights_biases_parameter_base)ref;

    if (wbBase->zrlTpFcPtr != VX_NULL)
    {
        vxFree(wbBase->zrlTpFcPtr);
        wbBase->zrlTpFcPtr = VX_NULL;
    }

    if (ref->context->options.enableMemOptimization)
    {
        if (wbBase->reshuffleWeightPtr != VX_NULL)
        {
            vxFree(wbBase->reshuffleWeightPtr);
            wbBase->reshuffleWeightPtr = VX_NULL;
        }
    }
    else
    {
        if (wbBase->weightPtr != VX_NULL)
        {
            vxFree(wbBase->weightPtr);
            wbBase->weightPtr = VX_NULL;
        }
        if (wbBase->biasPtr != VX_NULL)
        {
            vxFree(wbBase->biasPtr);
            wbBase->biasPtr = VX_NULL;
        }
    }

    if (wbBase->origWeight != VX_NULL)
    {
        vxoReference_Release((vx_reference_ptr)&wbBase->origWeight, VX_TYPE_TENSOR, VX_REF_INTERNAL);
        wbBase->origWeight = VX_NULL;
    }

    if (wbBase->origBias != VX_NULL)
    {
        vxoReference_Release((vx_reference_ptr)&wbBase->origBias, VX_TYPE_TENSOR, VX_REF_INTERNAL);
        wbBase->origBias = VX_NULL;
    }

    if (wbBase->origAlpha != VX_NULL)
    {
        vxoReference_Release((vx_reference_ptr)&wbBase->origAlpha, VX_TYPE_TENSOR, VX_REF_INTERNAL);
        wbBase->origBias = VX_NULL;
    }
}

vx_weights_biases_parameter vxoWeightsBiases_Create(
    vx_context                       context,
    vx_weights_biases_parameter_base wb_base,
    vx_uint32 *                      weight_dims,
    vx_enum                          layer_type,
    vx_bool                          first_time
    )
{
    vx_status status = VX_SUCCESS;
    vx_weights_biases_parameter wb = VX_NULL;
    vx_uint32 zArray[MAX_ZGROUP_COUNT], kzArray[MAX_KZGROUP_COUNT];
    vx_uint32 sliceCount = 0, filterCount = 0;
    vx_size minTotalKernelBufferSize = 0;
    vx_size minKernelBufferSize[MAX_ZGROUP_COUNT*MAX_KZGROUP_COUNT];
    vx_uint8_ptr zrlTmpPtr = VX_NULL;
    vx_uint32 kzNum = 0, zNum = 0, oneFilterSize = 0, weightSize = 0, i = 0, j = 0, kzoffset = 0;
    vx_uint8_ptr weight_ptr = VX_NULL;

    vxmASSERT(context);
    vxmASSERT(wb_base);

    if (context->options.enableMemOptimization)
    {
        if (wb_base->reshuffleWeightPtr)
        {
            weight_ptr = wb_base->reshuffleWeightPtr;
        }
        else
        {
            vxoTensor_GetTensorViewMemory(wb_base->origWeight, (gctPOINTER*)(&weight_ptr), VX_NULL);
        }
    }
    else
    {
        weight_ptr = wb_base->weightPtr;
    }

    wb = (vx_weights_biases_parameter)vxoReference_Create(context, VX_TYPE_WEIGHTS_BIASES_PARAMETER, VX_REF_INTERNAL, &context->base);
    if (vxoReference_GetStatus((vx_reference)wb) != VX_SUCCESS)
    {
        vxError("vxoWeightsBiases_Create: FAIL TO CREATE WB\n");
        goto exit;
    }

    wb->wb_base = wb_base;
    wb->weights_sizes[0] = weight_dims[0];
    wb->weights_sizes[1] = weight_dims[1];
    wb->weights_sizes[2] = weight_dims[2];
    wb->weights_sizes[3] = weight_dims[3];

    if (weight_ptr != VX_NULL)
    {
        vx_uint32 index = 0, woffset = 0, acount = 0, nzcount = 0, asize = 0;

        weightSize = (vx_uint32)vxDataType_GetSize(wb_base->weights_data_format);
        oneFilterSize = wb_base->weights_sizes[0] *
                        wb_base->weights_sizes[1] *
                        wb_base->weights_sizes[2] *
                        weightSize;

        /* Estimate sub wb stream size first */
        if (layer_type == VX_NN_FULLYCONNECTED_LAYER &&
            vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP_SINGLE_FC) &&
            weight_dims[3] > 1 && wb_base->nn_fc_batch_mode == vx_false_e && wb_base->biases_data_format != VX_TYPE_INT64)
        {
            vx_uint32 coreCount = context->nnConfig.fixedFeature.tpCoreCount + context->nnConfig.fixedFeature.tpliteCoreCount;
            sliceCount = weight_dims[2];
            filterCount = weight_dims[3];

            gcmASSERT(weight_dims[0] == 1);
            gcmASSERT(weight_dims[1] == 1);

            /* TP FC can handle up to 512 filters. */
            if (context->options.enableMultiTP && filterCount >= 2 * coreCount)
            {
                /* multi TP path */
                vx_uint32 max = gcmALIGN(filterCount, coreCount) / coreCount;
                for (;;)
                {
                    if (max >= TP_FC_Z_MAX && filterCount >= TP_FC_Z_MAX)
                    {
                        if (i < MAX_ZGROUP_COUNT)
                            zArray[i] = TP_FC_Z_MAX;
                        filterCount -= TP_FC_Z_MAX;
                        i++;
                    }
                    else if (filterCount)
                    {
                        vx_uint32 filterPerCoreBase, extraFilters;

                        coreCount = gcmMIN(coreCount, filterCount);
                        filterPerCoreBase  = filterCount / coreCount;
                        extraFilters = filterCount % coreCount;

                        for (j = 0; j < coreCount; j++)
                        {
                            if (i < MAX_ZGROUP_COUNT)
                                zArray[i] = j < extraFilters ? filterPerCoreBase + 1 : filterPerCoreBase;
                            filterCount -= zArray[i];
                            i++;
                        }

                        break;
                    }
                    else break;
                }
            }
            else
            {
                for (;;)
                {
                    if (filterCount > TP_FC_Z_MAX)
                    {
                        if (i < MAX_ZGROUP_COUNT)
                            zArray[i] = TP_FC_Z_MAX;
                        filterCount -= TP_FC_Z_MAX;
                        i++;
                    }
                    else
                    {
                        if (i < MAX_ZGROUP_COUNT)
                            zArray[i] = filterCount;
                        i++;
                        break;
                    }
                }
            }

            if (i > MAX_ZGROUP_COUNT)
                goto exit;
            zNum = i;

            if (sliceCount < (0x1<<16))
            {
                kzNum = 1;
                kzArray[0] = sliceCount;
            }
            else
            {
                /* let's first support 2^16<=kz<2^17 */
                gcmASSERT(sliceCount < (0x1<<17));
                kzNum = 2;
                kzArray[0] = sliceCount / 2;
                kzArray[1] = sliceCount - sliceCount / 2;
            }

            zrlTmpPtr = wb_base->zrlTpFcPtr = (vx_uint8 *)vxAllocate(weight_dims[2] * zNum);
            if (wb_base->zrlTpFcPtr == NULL)
            {
                status = VX_ERROR_NO_MEMORY;
                goto exit;
            }

            wb->use_tp_fc = vx_true_e;
        }
        else if (!vxoNNExternsionAdjustWeightsBiases(context, wb,
                                                     oneFilterSize * wb_base->weights_sizes[3],
                                                     &kzNum, &zNum, kzArray, zArray))
        {
            status = VX_FAILURE;
            goto exit;
        }

        /* Create slice for each split wb */
        wb->slice_num = kzNum * zNum;
        wb->slice_array = (vx_weights_biases_slice)vxAllocateAndZeroMemory(sizeof(vx_weights_biases_slice_s) * wb->slice_num);
        if (wb->slice_array == VX_NULL)
        {
            status = VX_ERROR_NO_MEMORY;
            goto exit;
        }

        if ((vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP_COMPRESSION_ENHANCEMENT) && wb->use_tp_fc) ||
            (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_COEF_COMPRESSION_ENHANCEMENT) && !wb->use_tp_fc))
        {
            wb->huffmanConfig = (vx_weights_biases_huffman_cfg)vxAllocateAndZeroMemory(sizeof(vx_weights_biases_huffman_cfg_s) * wb->slice_num);
            if (wb->huffmanConfig == VX_NULL)
            {
                status = VX_ERROR_NO_MEMORY;
                goto exit;
            }
        }

        for (i = 0; i < zNum; i++)
        {
            kzoffset = 0;
            filterCount = zArray[i];

            for (j = 0; j < kzNum; j++)
            {
                sliceCount = kzArray[j];

                if (wb->use_tp_fc)
                {
                    calculateWeightBiasTPBufferRelatedSize(
                        wb,
                        weight_ptr + kzoffset + woffset,
                        sliceCount,
                        filterCount,
                        wb_base->weights_sizes[2],
                        wb_base->weights_data_format,
                        wb_base->biases_data_format,
                        wb_base->skipValue,
                        wb_base->setZeroLength >= 0 && wb_base->setZeroLength <= 9 ?
                            wb_base->setZeroLength : (vx_int8)context->options.tpZeroRunLen,
                        index,
                        &minKernelBufferSize[index],
                        zrlTmpPtr);

                    zrlTmpPtr += sliceCount;
                    minTotalKernelBufferSize += minKernelBufferSize[index];
                }
                else
                {
                    calculateWeightBiasStreamRelatedSize(
                        context,
                        wb,
                        weight_ptr + kzoffset + woffset,
                        sliceCount, /* slice */
                        filterCount, /* z count */
                        filterCount, /* kernel per core */
                        wb_base->weights_data_format,
                        wb_base->biases_data_format,
                        wb_base->skipValue,
                        weight_dims[2] <= 1 ? 0 : wb_base->setZeroLength,
                        (vx_uint8)context->options.nnZeroRunLen,
                        0,
                        VX_NULL, VX_NULL, VX_NULL);

                    minTotalKernelBufferSize += wb->slice_array[index].kernel_stream_size;
                }

                kzoffset += sliceCount * weightSize;

                acount  += wb->slice_array[index].all_count;
                nzcount += wb->slice_array[index].non_zero_count;
                asize   += (vx_uint32)wb->slice_array[index].kernel_orig_size;

                index++;
            }

            woffset += oneFilterSize * filterCount;
        }

        wb->non_zero_ratio = gcmMIN(1.0f, (vx_float64)nzcount / acount);
        if ((wb->weights_sizes[0] == 1 && wb->weights_sizes[1] == 1
        && (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3) || vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6))
        && (wb->wb_base->weights_data_format == VX_TYPE_INT8 || wb->wb_base->weights_data_format == VX_TYPE_UINT8))
        && !gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_ZDP3_NO_COMPRESS_FIX)
        && !wb->use_tp_fc)
        {
            /*force non_zero_ratio to 1 as HW will not be skipping any zero without ZDP3_NO_COMPRESS_FIX*/
            wb->non_zero_ratio = 1.0f;
        }
        wb->general_compression_ratio = (vx_float64)minTotalKernelBufferSize / asize;

        if (wb->use_tp_fc) wb->wb_base->tpFcStreamTotalSize = minTotalKernelBufferSize;

        for (i = 0; i < zNum; i++)
        {
            for (j = 0; j < kzNum; j++)
            {
                vx_uint32 index = i * kzNum + j;
                wb->slice_array[index].kz_count = kzArray[j];
                wb->slice_array[index].z_count = zArray[i];
            }
        }
        wb->slice_z_num = zNum;
        wb->slice_kz_num = kzNum;

        /* Calculate arch perf in this step and pass it to operation for non-swtiling path. */
        wb->archPerfHandle = (vx_arch_perf)vxAllocateAndZeroMemory(sizeof(vx_arch_perf_s));
        if (wb->archPerfHandle == VX_NULL)
        {
            status = VX_ERROR_NO_MEMORY;
            goto exit;
        }
    }

    if (!first_time) vxoReference_Increment(&wb_base->base, VX_REF_INTERNAL);

exit:

    return status == VX_SUCCESS ? wb : VX_NULL;
}

vx_status vxoWeightsBiases_Compress(
    vx_context                       context,
    vx_weights_biases_parameter      wb,
    vx_uint32                        kernel_per_core,
    vx_uint32 *                      pooling_output_dims,
    vx_enum                          output_format,
    vx_int32                         z_offset
    )
{
    vx_status status = VX_SUCCESS;
    vx_size minKernelBufferSize[MAX_ZGROUP_COUNT*MAX_KZGROUP_COUNT];
    vx_uint8 minZeroRunLen[MAX_ZGROUP_COUNT*MAX_KZGROUP_COUNT] = {0};
    vx_uint32 maxZeroRunLen[MAX_ZGROUP_COUNT] = {0};
    vx_size minTotalKernelBufferSize = 0;
    vx_uint32 oneFilterSize, weightSize, i, j, kzoffset, sliceCount, filterCount;
    vx_weights_biases_parameter_base wb_base = wb->wb_base;
    vx_uint8_ptr zrlTmpPtr = VX_NULL, minZeroRunLenTPFC = wb_base->zrlTpFcPtr;
    vx_uint8_ptr weight_ptr = VX_NULL;
    vx_uint32_ptr bias_ptr = VX_NULL;

    vxmASSERT(context);
    vxmASSERT(wb);
    vxmASSERT(pooling_output_dims != VX_NULL || z_offset > 0);

    if (WB_MEM_SIZE_INDEX(wb, 0) > 0) return status;

    if (context->options.enableMemOptimization)
    {
        if (WB_BASE(wb)->reshuffleWeightPtr != VX_NULL)
        {
            weight_ptr = WB_BASE(wb)->reshuffleWeightPtr;
        }
        else
        {
            vxoTensor_GetTensorViewMemory(WB_WEIGHT_TENSOR(wb), (gctPOINTER*)(&weight_ptr), VX_NULL);
        }

        if (WB_BIAS_TENSOR(wb))
        {
            vxoTensor_GetTensorViewMemory(WB_BIAS_TENSOR(wb), (gctPOINTER*)(&bias_ptr), VX_NULL);
        }
    }
    else
    {
        weight_ptr = wb_base->weightPtr;
        bias_ptr = wb_base->biasPtr;
    }

    vxmASSERT(weight_ptr != VX_NULL);

    weightSize = (vx_uint32)vxDataType_GetSize((vx_type_e)wb_base->weights_data_format);
    oneFilterSize = wb_base->weights_sizes[0] *
                    wb_base->weights_sizes[1] *
                    wb_base->weights_sizes[2] *
                    weightSize;

    if (!wb->use_tp_fc)
    {
        if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP0))
        {
            /* caclulate V8 HUFFMAN stream size need real kernels per core*/
            minTotalKernelBufferSize = calcKernelSizeV8Huffman(
                                        context,
                                        wb,
                                        wb->weights_sizes[0],
                                        wb->weights_sizes[1],
                                        wb->weights_sizes[2],
                                        wb->weights_sizes[3],
                                        kernel_per_core,
                                        wb_base->weights_data_format,
                                        wb_base->skipValue,
                                        weight_ptr,
                                        VX_NULL,
                                        VX_NULL,
                                        VX_NULL,
                                        VX_NULL,
                                        0
                                        );
            minKernelBufferSize[0] = minTotalKernelBufferSize;
        }
        else if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_COEF_COMPRESSION_ENHANCEMENT))
        {
            /* caclulate V7 HUFFMAN stream size need bias real data*/
            minTotalKernelBufferSize = calcKernelStreamSizeHuffman(
                                        context,
                                        wb,
                                        wb->weights_sizes[0],
                                        wb->weights_sizes[1],
                                        wb->weights_sizes[2],
                                        wb->weights_sizes[3],
                                        kernel_per_core,
                                        wb_base->weights_data_format,
                                        wb_base->biases_data_format,
                                        wb_base->inputZP,
                                        wb_base->coefZP,
                                        wb_base->skipValue,
                                        pooling_output_dims == VX_NULL ? 0 : pooling_output_dims[0],
                                        pooling_output_dims == VX_NULL ? 0 : pooling_output_dims[1],
                                        z_offset,
                                        vxDataType_GetSize((vx_type_e)output_format),
                                        weight_ptr,
                                        (bias_ptr != VX_NULL) ? bias_ptr: VX_NULL,
                                        0
                                        );
            minKernelBufferSize[0] = minTotalKernelBufferSize;
        }
        else
        {
            vx_uint32 index = 0;
            vx_size weightDataBytesOffset = 0;

            kzoffset = 0;
            for (i = 0; i < wb->slice_z_num; i++)
            {
                filterCount = wb->slice_array[index].z_count;

                for (j = 0; j < wb->slice_kz_num; j++)
                {
                    sliceCount = wb->slice_array[index].kz_count;

                    calculateWeightBiasStreamRelatedSize(
                        context,
                        wb,
                        weight_ptr + kzoffset + weightDataBytesOffset,
                        sliceCount, /* slice */
                        filterCount, /* z count */
                        kernel_per_core, /* kernel per core */
                        wb_base->weights_data_format,
                        wb_base->biases_data_format,
                        wb_base->skipValue,
                        wb->weights_sizes[2] <= 1 ? 0 : wb_base->setZeroLength,
                        (vx_uint8)context->options.nnZeroRunLen,
                        0,
                        &minKernelBufferSize[index], &minZeroRunLen[index], &maxZeroRunLen[index]);

                    kzoffset += sliceCount * weightSize;
                    minTotalKernelBufferSize += minKernelBufferSize[index];
                    index++;
                }

                weightDataBytesOffset += oneFilterSize * filterCount;
            }
        }
    }
    else
    {
        minTotalKernelBufferSize = wb_base->tpFcStreamTotalSize;
    }

    /* Allocate memory for sub wb */
    if (!WeightBiasBufferAllocate(context,
                                  wb,
                                  minTotalKernelBufferSize + wb_base->memory_pad * wb->slice_num))
    {
        status = VX_ERROR_NO_MEMORY;
        goto exit;
    }

    /* Compress sub wb */
    vxAcquireMutex(wb->memory.writeLocks[0]);
    {
        vx_size weightDataBytesOffset = 0;
        vx_size biasDataDWordOffset = 0;
        vx_size compressDataBytesOffset = 0;
        vx_uint32 index = 0;

        kzoffset = 0;
        zrlTmpPtr = minZeroRunLenTPFC;
        for (i = 0; i < wb->slice_z_num; i++)
        {
            kzoffset = 0;
            filterCount = wb->slice_array[index].z_count;

            for (j = 0; j < wb->slice_kz_num; j++)
            {
                vx_uint8_ptr kernelBufferPtr = wb->memory.logicals[0] + compressDataBytesOffset;

                sliceCount = wb->slice_array[index].kz_count;

                if (wb->use_tp_fc &&
                    vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP_COMPRESSION_ENHANCEMENT))
                {
                    fillinTPKernelBufferHuffman(
                        wb,
                        zrlTmpPtr,
                        sliceCount,
                        filterCount,
                        wb_base->weights_sizes[2],
                        wb_base->weights_data_format,
                        wb_base->biases_data_format,
                        wb_base->skipValue,
                        kernelBufferPtr,
                        weight_ptr + kzoffset + weightDataBytesOffset,
                        !j ? bias_ptr + biasDataDWordOffset : VX_NULL,
                        index);

                    zrlTmpPtr += sliceCount;
                }
                else if (wb->use_tp_fc)
                {
                    fillinTPKernelBuffer(
                        wb,
                        zrlTmpPtr,
                        sliceCount,
                        filterCount,
                        wb_base->weights_sizes[2],
                        wb_base->weights_data_format,
                        wb_base->biases_data_format,
                        wb_base->skipValue,
                        kernelBufferPtr,
                        weight_ptr + kzoffset + weightDataBytesOffset,
                        !j ? bias_ptr + biasDataDWordOffset : VX_NULL,
                        index);

                    zrlTmpPtr += sliceCount;
                }
                else
                {
                    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP0))
                    {
                        fillinKernelBufferV8Huffman(
                            context,
                            wb,
                            wb->weights_sizes[0],
                            wb->weights_sizes[1],
                            sliceCount,
                            filterCount,
                            kernel_per_core,
                            wb_base->weights_data_format,
                            wb_base->biases_data_format,
                            wb_base->inputZP,
                            wb_base->coefZP,
                            wb_base->skipValue,
                            pooling_output_dims == VX_NULL ? 0 : pooling_output_dims[0],
                            pooling_output_dims == VX_NULL ? 0 : pooling_output_dims[1],
                            z_offset,
                            vxDataType_GetSize((vx_type_e)output_format),
                            kernelBufferPtr,
                            weight_ptr + kzoffset + weightDataBytesOffset,
                            (bias_ptr != VX_NULL) ? bias_ptr + biasDataDWordOffset : VX_NULL,
                            VX_NULL,
                            VX_NULL,
                            wb->wb_base->origAlpha,
                            index);
                    }
                    else if (wb->wb_base->hw_depth_wise)
                    {
                        fillinDepthWiseKernelBuffer(
                            context,
                            wb,
                            minZeroRunLen[index],
                            maxZeroRunLen[index],
                            wb->weights_sizes[0],
                            wb->weights_sizes[1],
                            sliceCount,
                            filterCount,
                            kernel_per_core,
                            wb_base->weights_data_format,
                            wb_base->biases_data_format,
                            wb_base->inputZP,
                            wb_base->coefZP,
                            wb_base->skipValue,
                            pooling_output_dims == VX_NULL ? 0 : pooling_output_dims[0],
                            pooling_output_dims == VX_NULL ? 0 : pooling_output_dims[1],
                            z_offset,
                            vxDataType_GetSize((vx_type_e)output_format),
                            kernelBufferPtr,
                            weight_ptr + kzoffset + weightDataBytesOffset,
                            (bias_ptr != VX_NULL) ? bias_ptr + biasDataDWordOffset : VX_NULL,
                            index);
                    }
                    else if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_COEF_COMPRESSION_ENHANCEMENT))
                    {
                        fillinKernelBufferHuffman(
                            context,
                            wb,
                            wb->weights_sizes[0],
                            wb->weights_sizes[1],
                            sliceCount,
                            filterCount,
                            kernel_per_core,
                            wb_base->weights_data_format,
                            wb_base->biases_data_format,
                            wb_base->inputZP,
                            wb_base->coefZP,
                            wb_base->skipValue,
                            pooling_output_dims == VX_NULL ? 0 : pooling_output_dims[0],
                            pooling_output_dims == VX_NULL ? 0 : pooling_output_dims[1],
                            z_offset,
                            vxDataType_GetSize((vx_type_e)output_format),
                            kernelBufferPtr,
                            weight_ptr + kzoffset + weightDataBytesOffset,
                            (bias_ptr != VX_NULL) ? bias_ptr + biasDataDWordOffset : VX_NULL,
                            index
                            );
                    }
                    else if (context->options.enableNonZeroBalance)
                    {
                        fillinKernelBufferBalance(
                            context,
                            wb,
                            minZeroRunLen[index],
                            maxZeroRunLen[index],
                            wb->weights_sizes[0],
                            wb->weights_sizes[1],
                            sliceCount,
                            filterCount,
                            kernel_per_core,
                            wb_base->weights_data_format,
                            wb_base->biases_data_format,
                            wb_base->inputZP,
                            wb_base->coefZP,
                            wb_base->skipValue,
                            pooling_output_dims == VX_NULL ? 0 : pooling_output_dims[0],
                            pooling_output_dims == VX_NULL ? 0 : pooling_output_dims[1],
                            z_offset,
                            vxDataType_GetSize((vx_type_e)output_format),
                            kernelBufferPtr,
                            weight_ptr + kzoffset + weightDataBytesOffset,
                            (bias_ptr != VX_NULL) ? bias_ptr + biasDataDWordOffset : VX_NULL,
                            index);
                    }
                    else
                    {
                        fillinKernelBuffer(
                            context,
                            wb,
                            minZeroRunLen[index],
                            maxZeroRunLen[index],
                            wb->weights_sizes[0],
                            wb->weights_sizes[1],
                            sliceCount,
                            filterCount,
                            kernel_per_core,
                            wb_base->weights_data_format,
                            wb_base->biases_data_format,
                            wb_base->inputZP,
                            wb_base->coefZP,
                            wb_base->skipValue,
                            pooling_output_dims == VX_NULL ? 0 : pooling_output_dims[0],
                            pooling_output_dims == VX_NULL ? 0 : pooling_output_dims[1],
                            z_offset,
                            vxDataType_GetSize((vx_type_e)output_format),
                            kernelBufferPtr,
                            weight_ptr + kzoffset + weightDataBytesOffset,
                            (bias_ptr != VX_NULL) ? bias_ptr + biasDataDWordOffset : VX_NULL,
                            index);
                    }
                }

                kzoffset += sliceCount * weightSize;

                wb->slice_array[index].memory_offset = compressDataBytesOffset;
                wb->slice_array[index].memory_size = wb_base->memory_pad + (wb->use_tp_fc ?
                                                        wb->slice_array[index].kernel_stream_size : minKernelBufferSize[index]);
                compressDataBytesOffset += wb->slice_array[index].memory_size;

                index++;
            }

            weightDataBytesOffset += oneFilterSize * filterCount;
            biasDataDWordOffset += filterCount;
        }
    }
    vxReleaseMutex(wb->memory.writeLocks[0]);

#if gcdDUMP
    gcmDUMP(gcvNULL, "#[weights and biases]\n");
    gcmDUMP_BUFFER(gcvNULL,
                   gcvDUMP_BUFFER_MEMORY,
                   wb->memory.physicals[0],
                   (gctPOINTER)wb->memory.logicals[0],
                   0,
                   wb->memory_size - wb->wb_base->memory_head_offset);
#endif

exit:
    return status;
}

VX_INTERNAL_CALLBACK_API void vxoWeightsBiases_Destructor(vx_reference ref)
{
    vx_weights_biases_parameter wb = (vx_weights_biases_parameter)ref;

    vxoWeightsBiases_Destroy(wb);
}

void vxoWeightsBiases_Destroy(
    vx_weights_biases_parameter wb
    )
{
    if (wb->mGpuWBTable != VX_NULL)
    {
        vxFree(wb->mGpuWBTable);
        wb->mGpuWBTable = VX_NULL;
        wb->mGpuWBCount = 0;
    }

    if (wb->slice_array != VX_NULL)
    {
        vxFree(wb->slice_array);
        wb->slice_array = VX_NULL;
    }

    if (wb->archPerfHandle != VX_NULL)
    {
        vxFree(wb->archPerfHandle);
        wb->archPerfHandle = VX_NULL;
    }

    if (wb->memory.nodePtrs[0] != VX_NULL)
    {
        vxoMemory_Free(wb->base.context, &wb->memory);
    }

    if (wb->sub_wb_vdata)
    {
        if (wb->sub_wb_vdata->slice_array != VX_NULL)
        {
            vxFree(wb->sub_wb_vdata->slice_array);
            wb->sub_wb_vdata->slice_array = VX_NULL;
        }

        if (wb->sub_wb_vdata->wb_memory_ptr != VX_NULL)
        {
            vxFree(wb->sub_wb_vdata->wb_memory_ptr);
            wb->sub_wb_vdata->wb_memory_ptr = VX_NULL;
        }
        vxFree(wb->sub_wb_vdata);
        wb->sub_wb_vdata = VX_NULL;
    }

    if (wb->zOffsetHandle != VX_NULL)
    {
        vxFree(wb->zOffsetHandle);
        wb->zOffsetHandle = VX_NULL;
    }

    if (wb->huffmanConfig != VX_NULL)
    {
        vxFree(wb->huffmanConfig);
        wb->huffmanConfig = VX_NULL;
    }

    if (wb->max_per_core_per_vzgroup_nzr != VX_NULL)
    {
        vxFree(wb->max_per_core_per_vzgroup_nzr);
        wb->max_per_core_per_vzgroup_nzr = VX_NULL;
    }

    vxoReference_Release((vx_reference_ptr)&(wb->wb_base), VX_TYPE_WEIGHTS_BIASES_PARAMETER_BASE, VX_REF_INTERNAL);
}

void vxoWeightsBiases_Clear(vx_weights_biases_parameter wb)
{
    vx_weights_biases_parameter_base wbBase;

    vxmASSERT(wb);

    wbBase = wb->wb_base;

    vxmASSERT(wbBase);

    if (wb->base.context->options.enableMemOptimization)
    {
        if (wbBase->reshuffleWeightPtr != VX_NULL)
        {
            vxFree(wbBase->reshuffleWeightPtr);
            wbBase->reshuffleWeightPtr = VX_NULL;
        }
    }
    else
    {
        if (wbBase->weightPtr != VX_NULL)
        {
            vxFree(wbBase->weightPtr);
            wbBase->weightPtr = VX_NULL;
        }
        if (wbBase->biasPtr != VX_NULL)
        {
            vxFree(wbBase->biasPtr);
            wbBase->biasPtr = VX_NULL;
        }
    }

    if (wbBase->origWeight != VX_NULL)
    {
        vxoReference_Release((vx_reference_ptr)&wbBase->origWeight, VX_TYPE_TENSOR, VX_REF_INTERNAL);
        wbBase->origWeight = VX_NULL;
    }

    if (wbBase->origBias != VX_NULL)
    {
        vxoReference_Release((vx_reference_ptr)&wbBase->origBias, VX_TYPE_TENSOR, VX_REF_INTERNAL);
        wbBase->origBias = VX_NULL;
    }
}

vx_weights_biases_parameter _createWeightsBiasesParameterFromTensors(
    vx_context  context,
    vx_enum     layer_type,
    vx_uint32 * inputs_dims,
    vx_uint32   num_of_input_dims,
    vx_uint32   num_of_output_dims,
    vx_uint32   pad_x_left,
    vx_uint32   pad_x_right,
    vx_uint32   pad_y_top,
    vx_uint32   pad_y_bottom,
    vx_uint32   pooling_size_x,
    vx_uint32   pooling_size_y,
    vx_uint32   stride_x,
    vx_uint32   stride_y,
    vx_enum     down_scale_size_rounding,
    vx_uint32 * convolution_outputs_dims,
    vx_uint32 * pooling_outputs_dims,
    vx_weights_biases_parameter_optimizations_t * optimizations,
    vx_enum     output_format,
    vx_enum     convert_format,
    vx_enum     rank_mode,
    vx_tensor   weights,
    vx_tensor   biases,
    vx_tensor   alpha,
    vx_bool     doPRelu,
    vx_bool     do1xN
    )
{
    vx_weights_biases_parameter_base wb_base;
    vx_weights_biases_parameter weight_bias = VX_NULL;

    vx_uint32 weightDims[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32 weightViewStarts[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32 weightViewEnds[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32 biasDims[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32 biasViewStarts[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32 biasViewEnds[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32 inputDims[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32 convOutputDims[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32 finalOutputDims[VX_CONTEXT_TENSOR_MAX_DIMENSION];

    vx_uint32 weightDimCount    = TENSOR_DIM_NUM(weights);
    vx_uint32 orgWeightDimCount = TENSOR_DIM_NUM(weights);
    vx_enum weightDataType          = TENSOR_DATA_TYPE(weights);
    vx_enum weightQuantType     = TENSOR_QUANT_TYPE(weights);
    vx_uint32 weightSize        = (vx_uint32)vxDataType_GetSize((vx_type_e)weightDataType);
    vx_uint32 orgWeightSize     = weightSize;

    vx_uint32 biasDimCount      = 0;
    vx_int8 biasFp              = 0;
    vx_float32 biasScale        = 0;
    vx_enum biasDataType        = VX_TYPE_FLOAT32;
    vx_enum biasQuantType       = biases ? TENSOR_QUANT_TYPE(biases) : TENSOR_QUANT_TYPE(weights);

    vx_uint32 sliceCount;
    vx_uint32 weightCount;
    vx_uint32 filterTotalCount;
    vx_uint32 i;

    gctPOINTER convertedWeightData  = VX_NULL;
    gctPOINTER weightData           = VX_NULL;
    vx_bool    reallyDo1xN          = vx_false_e;
    vx_bool    doZdpOpt             = vx_false_e;

    vx_bool nnSupportFormat = vx_false_e;

    vx_uint32 strideX, strideY;
    vx_bool hasHwDepthWise = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_NN_DEPTHWISE_SUPPORT);
    vx_bool isV8 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP0);
    vx_status status             = VX_SUCCESS;

    nnSupportFormat = vxnneIsNNSupportFormat(context, weights, VX_NULL, VX_NULL);

    if (!nnSupportFormat && layer_type == VX_NN_CONVOLUTION_LAYER)
    {
        status = VX_ERROR_INVALID_TYPE;
        vxError("_createWeightsBiasesParameterFromTensors: current NN didn't support this format 0x%x", weightDataType);
        goto exit;
    }

    vxoTensor_GetTensorViewRegion(weights, weightDimCount, weightViewStarts, weightViewEnds);

    if (layer_type == VX_NN_FULLYCONNECTED_LAYER && weightDimCount == 2)
        weightDimCount = 4;

    /*Hw didn't support FP32, need convert to supported format*/
    if (weightDataType == VX_TYPE_FLOAT32 && convert_format != 0)
    {
        weightDataType = convert_format;
        weightSize = (vx_uint32)vxDataType_GetSize((vx_type_e)weightDataType);
    }

    if (rank_mode == VX_TENSOR_RANK_CWHN)
    {
        /* If shape is (channel, width, height, batch), need trans to (width, height, channel, batch) */
        vx_uint32 cwhn_dims[4] = {0};
        vx_uint32 input_batch, input_height, input_width, input_depth;
        vx_uint32 output_height, output_width, output_depth;
        vx_uint32 weights_total;
        vx_uint8_ptr buffer = VX_NULL;
        vx_uint8_ptr org_weight_ptr = TENSOR_LOGICAL_ADDR(weights);

        cwhn_dims[0] = TENSOR_VIEW_SIZE_INDEX(weights, 0);
        cwhn_dims[1] = TENSOR_VIEW_SIZE_INDEX(weights, 1);
        cwhn_dims[2] = TENSOR_VIEW_SIZE_INDEX(weights, 2);
        cwhn_dims[3] = TENSOR_VIEW_SIZE_INDEX(weights, 3);

        if (TENSOR_DIM_NUM(weights) == 2)
        {
            weightDims[0] = 1;
            weightDims[1] = 1;
            weightDims[2] = cwhn_dims[1];
            weightDims[3] = cwhn_dims[0];
        }
        else
        {

            weightDims[0] = cwhn_dims[2];
            weightDims[1] = cwhn_dims[1];
            weightDims[2] = cwhn_dims[3];
            weightDims[3] = cwhn_dims[0];
        }

        input_batch = cwhn_dims[0];
        input_height = cwhn_dims[1];
        input_width = cwhn_dims[2];
        input_depth = cwhn_dims[3];

        output_width = weightDims[0];
        output_height = weightDims[1];
        output_depth = weightDims[2];

        weights_total = input_batch * input_height * input_width * input_depth * orgWeightSize;
        buffer = (vx_uint8_ptr)vxAllocateAndZeroMemory(weights_total);

        vxMemCopy(buffer, org_weight_ptr, weights_total);
        gcoOS_MemFill(org_weight_ptr, 0, weights_total);

        vxnneAdapter_SWCWHN2WHCN(buffer, (vx_type_e)TENSOR_DATA_TYPE(weights), TENSOR_QUANT_TYPE(weights), input_depth, input_width, input_height, input_batch, TENSOR_POS(weights), TENSOR_TF_ZEROPOINT(weights), TENSOR_TF_SCALE(weights),
                org_weight_ptr, (vx_type_e)weightDataType, TENSOR_QUANT_TYPE(weights), output_depth, output_width, output_height, TENSOR_POS(weights), TENSOR_TF_ZEROPOINT(weights), TENSOR_TF_SCALE(weights), TENSOR_ROUNDING_MODE(weights));

        vxFree(buffer);
    }
    else if (layer_type == VX_NN_FULLYCONNECTED_LAYER && orgWeightDimCount == 2)
    {
        weightDims[0] = weightDims[1] = 1;
        if (weights->isViewed)
        {
            for (i = 0; i < orgWeightDimCount; i++)
            {
                weightDims[i+2] = weightViewEnds[i] - weightViewStarts[i];
            }
        }
        else
        {
            weightDims[2] = weights->dims[0];
            weightDims[3] = weights->dims[1];
        }
    }
    else if (weights->isViewed)
    {
        for (i = 0; i < weightDimCount; i++)
        {
            weightDims[i] = weightViewEnds[i] - weightViewStarts[i];
        }
    }
    else
    {
        vxMemCopy(weightDims, TENSOR_SIZES(weights), weightDimCount * sizeof(vx_uint32));
    }

    if ((layer_type == VX_NN_FULLYCONNECTED_LAYER) && ((weightDims[0] != 1) && (weightDims[1] != 1)))
    {
        vx_int32 index;
        for (index = weightDimCount - 1; index >= 0; index--)
        {
            weightDims[index] = (index == 0 || index == 1) ? 1 : (index == 2) ?
                (weightDims[index] * weightDims[index-1] * weightDims[index-2]) : weightDims[index];
        }
    }

    for (i = 0; i < VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
    {
        if (inputs_dims[i])
            inputDims[i] = inputs_dims[i];
        if (convolution_outputs_dims[i])
            convOutputDims[i] = convolution_outputs_dims[i];
        if (pooling_outputs_dims[i])
            finalOutputDims[i] = pooling_outputs_dims[i];
    }

    if (layer_type == VX_NN_FULLYCONNECTED_LAYER)
    {
        if (num_of_input_dims == 1)
        {
            inputDims[0] = 1;
            inputDims[1] = 1;
            inputDims[2] = inputs_dims[0];
        }
        else if (num_of_input_dims == 2)
        {
            inputDims[0] = 1;
            inputDims[1] = 1;
            inputDims[2] = inputs_dims[0];
            inputDims[3] = inputs_dims[1];
        }

        if (num_of_output_dims == 1)
        {
            convOutputDims[0] = 1;
            convOutputDims[1] = 1;
            convOutputDims[2] = convolution_outputs_dims[0];

            finalOutputDims[0] = 1;
            finalOutputDims[1] = 1;
            finalOutputDims[2] = pooling_outputs_dims[0];
        }
        else if (num_of_output_dims == 2)
        {
            convOutputDims[0] = 1;
            convOutputDims[1] = 1;
            convOutputDims[2] = convolution_outputs_dims[0];
            convOutputDims[3] = convolution_outputs_dims[1];

            finalOutputDims[0] = 1;
            finalOutputDims[1] = 1;
            finalOutputDims[2] = pooling_outputs_dims[0];
            finalOutputDims[3] = pooling_outputs_dims[1];
        }
    }

    if ((vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3) || vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6)) &&
        !isV8 && /*XYDP0 means V8, need disable this WAR*/
        weightDims[0] == 1 &&
        weightDims[1] == 1 &&
        (pad_x_left == 0 && pad_x_right == 0 && pad_y_top == 0 && pad_y_bottom == 0) &&
        layer_type == VX_NN_CONVOLUTION_LAYER &&
        (TENSOR_DATA_TYPE(weights) == VX_TYPE_UINT8 || TENSOR_DATA_TYPE(weights) == VX_TYPE_INT8) &&
        context->options.enableZdpOpt &&
        do1xN)
    {
        vx_uint32 fitN = 0;
        vx_uint32 fitOutN = 0;

        vx_uint32 index_w = 0, index_h = 1;

        if (rank_mode == VX_TENSOR_RANK_CWHN)
        {
            index_w = 2;
            index_h = 1;
        }

        stride_x = (stride_x > 0) ? stride_x : gcmCEIL((vx_float32)inputDims[index_w] / convOutputDims[index_w]);
        stride_y = (stride_y > 0) ? stride_y : gcmCEIL((vx_float32)inputDims[index_h] / convOutputDims[index_h]);

        doZdpOpt = calcFitZdp3N(context, inputs_dims[index_w], inputs_dims[index_h], &fitN, stride_x, pooling_size_x);
        fitOutN = fitN / stride_x;

        if (doZdpOpt)
        {
            /* Need reshape input[x, y, kz] --> [x*y/fitN, fitN, kz] */
            /* Need reshape output[x, y, vz] --> [x*y/fitN, fitN, vz] */
            inputDims[index_w] = inputDims[index_w] * inputDims[index_h] / fitN;
            inputDims[index_h] = fitN;

            convOutputDims[index_w] = convOutputDims[index_w] * convOutputDims[index_h] / fitOutN;
            convOutputDims[index_h] = fitOutN;

            finalOutputDims[index_w] = convOutputDims[index_w] / (convolution_outputs_dims[index_w] / pooling_outputs_dims[index_w]);
            finalOutputDims[index_h] = convOutputDims[index_h] / (convolution_outputs_dims[index_h] / pooling_outputs_dims[index_h]);
        }
    }
    else if (!(vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3) || vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6)) &&
        !gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_CONV1x1_PERF_FIX) &&
        !isV8 && /*XYDP0 means V8, need disable this WAR*/
        weightDims[0] == 1 &&
        weightDims[1] == 1 &&
        pooling_size_x <= 1 &&
        layer_type == VX_NN_CONVOLUTION_LAYER &&
        (pad_x_left == 0 && pad_x_right == 0 && pad_y_top == 0 && pad_y_bottom == 0) &&
        context->options.nn1x1To1xN &&
        do1xN)
    {
        vx_uint32 fitN = calcFit1xN(context, weightDims[2], inputs_dims[0], inputs_dims[1]);
        stride_x = (stride_x > 0) ? stride_x : gcmCEIL((vx_float32)inputDims[0] / convOutputDims[0]);
        stride_y = (stride_y > 0) ? stride_y : gcmCEIL((vx_float32)inputDims[1] / convOutputDims[1]);
        gcmASSERT(stride_x == stride_y);

        if (fitN > 1 && stride_x == 1)
        {
            reallyDo1xN = vx_true_e;
            /* Need reshape input[x, y, kz] --> [x*y, fitN, kz/fitN] */
            /* Need reshape output[x, y, vz] --> [x*y, 1, vz] */
            /* Need reshape weight[1, 1, kz, vz] --> [1, fitN, kz/fitN, vz] */
            weightDims[1] = fitN;
            weightDims[2] /= fitN;

            inputDims[0] *= inputDims[1];
            inputDims[1] = fitN;
            inputDims[2] /= fitN;

            convOutputDims[0] *= convOutputDims[1];
            convOutputDims[1] = 1;

            finalOutputDims[0] =  convOutputDims[0] / (convolution_outputs_dims[0] / pooling_outputs_dims[0]);
            finalOutputDims[1] =  convOutputDims[1] / (convolution_outputs_dims[1] / pooling_outputs_dims[1]);
        }
    }

    if (biases)
    {
        biasDimCount = TENSOR_DIM_NUM(biases);
        biasDataType = TENSOR_DATA_TYPE(biases);
        biasScale = TENSOR_TF_SCALE(biases);
        biasFp = TENSOR_POS(biases);

        if (biasDataType != VX_TYPE_INT32 &&
            biasDataType != VX_TYPE_FLOAT32 &&
            biasDataType != VX_TYPE_INT64)
        {
            status = VX_ERROR_INVALID_TYPE;
            vxError("_createWeightsBiasesParameterFromTensors: current NN didn't support this bias format 0x%x", biasDataType);
            goto exit;
        }
        vxoTensor_GetTensorViewRegion(biases, biasDimCount, biasViewStarts, biasViewEnds);
        if (biases->isViewed)
        {
            for (i = 0; i < biasDimCount; i++)
            {
                biasDims[i] = biasViewEnds[i] - biasViewStarts[i];
            }
        }
        else
        {
            vxMemCopy(biasDims, TENSOR_SIZES(biases), biasDimCount * sizeof(vx_uint32));
        }
    }

    wb_base = vxoWeightsBiasesBase_Create(context,
                                          layer_type,
                                          inputDims,
                                          pad_x_left,
                                          pad_x_right,
                                          pad_y_top,
                                          pad_y_bottom,
                                          pooling_size_x,
                                          pooling_size_y,
                                          stride_x,
                                          stride_y,
                                          down_scale_size_rounding,
                                          convOutputDims,
                                          finalOutputDims,
                                          weightDimCount,
                                          weightDims,
                                          weightDataType,
                                          weightQuantType,
                                          TENSOR_POS(weights),
                                          TENSOR_TF_ZEROPOINT(weights),
                                          TENSOR_TF_SCALE(weights),
                                          biasDimCount,
                                          biasDims,
                                          biasDataType,
                                          biasQuantType,
                                          biasFp,
                                          biasScale,
                                          optimizations);
    if (wb_base == VX_NULL)
    {
        status = VX_FAILURE;
        goto exit;
    }

    if (!isV8 && layer_type == VX_NN_CONVOLUTION_LAYER &&
        wb_base->weights_sizes[0] != wb_base->weights_sizes[1] &&
        wb_base->weights_sizes[0] != 1)
    {
        /*V7 didn't support MxN, only supoort 1xN*/
        status = VX_ERROR_INVALID_VALUE;
        vxError("_createWeightsBiasesParameterFromTensors: current NN didn't support this kernel size %d x %d", wb_base->weights_sizes[0], wb_base->weights_sizes[1]);
        goto exit;
    }

    wb_base->origWeight = weights;
    vxoReference_Increment((vx_reference)wb_base->origWeight, VX_REF_INTERNAL);
    if (biases != VX_NULL)
    {
        wb_base->origBias = biases;
        wb_base->no_bias = vx_false_e;
        vxoReference_Increment((vx_reference)wb_base->origBias, VX_REF_INTERNAL);
    }
    else
        wb_base->no_bias = vx_true_e;

    if (doPRelu && alpha != VX_NULL)
    {
        wb_base->origAlpha = alpha;
        vxoReference_Increment((vx_reference)wb_base->origAlpha, VX_REF_INTERNAL);
    }

    wb_base->do_1xN = reallyDo1xN;
    wb_base->do_zdp_opt = doZdpOpt;

    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_FIRST_PIXEL_POOLING) &&
        wb_base->strideX == 2 && wb_base->strideY == 2 &&
        !wb_base->do_zdp_opt &&
        (wb_base->weights_data_format == VX_TYPE_INT8 || wb_base->weights_data_format == VX_TYPE_UINT8) &&
        pooling_size_x == 0 &&
        ((inputDims[0] % 2 == 0 && layer_type == VX_NN_CONVOLUTION_LAYER) || (layer_type == VX_NN_DEPTH_WISE_CONVOLUTION_LAYER && hasHwDepthWise)))
    {
        /* Per Arch, only support INT8 3x3 conv right now*/
        /* First pixel pooling is 2x2 poooling stride is 2, so convolution output should be even*/
        vx_float32 nonZeroRatio = calculateWeightNonZeroRatio(context, wb_base->skipValue, weights);

        if (nonZeroRatio * weights->dims[0] * weights->dims[1] < 6.3 || (layer_type == VX_NN_DEPTH_WISE_CONVOLUTION_LAYER && hasHwDepthWise))
        {
            /* If no pooling & stride = 2 && non-zero-ratio * kx * ky less than 0.7 * 9, do first pixel pooling(2x2, stride = 2), no need reshuffle */
            wb_base->strideX = 1;
            wb_base->strideY = 1;
            wb_base->do_fisrt_pixel_pool = vx_true_e;
            wb_base->pooling_size_x = 2;
            wb_base->pooling_size_y = 2;
            wb_base->pooling_stride = 2;

            wb_base->weights_sizes[0] = weights->dims[0];
            wb_base->weights_sizes[1] = weights->dims[1];
            wb_base->weights_sizes[2] = weights->dims[2];
        }
    }

    strideX = wb_base->strideX;
    strideY = wb_base->strideY;

    if (layer_type == VX_NN_DEPTH_WISE_CONVOLUTION_LAYER && hasHwDepthWise &&
       strideX == 1 &&
       strideY == 1 &&
       (wb_base->weights_data_format == VX_TYPE_INT8 || wb_base->weights_data_format == VX_TYPE_UINT8))
    {
        wb_base->hw_depth_wise = vx_true_e;
        if (wb_base->weights_sizes[2] != 1 && wb_base->weights_sizes[3] == 1)
        {
            /*default set kz = 1, vz = outZ*/
            wb_base->weights_sizes[3] = wb_base->weights_sizes[2];
            wb_base->weights_sizes[2] = 1;
        }
    }
    else
        wb_base->hw_depth_wise = vx_false_e;

    if ((weightDims[0] == 1) && (weightDims[1] == 1) && (strideX > 1) && (strideX == strideY))
    {
        sliceCount = weightDims[2];
    }
    else
    {
        sliceCount = weightDims[2] * strideX * strideY;
    }

    weightCount = wb_base->weights_sizes[0] * wb_base->weights_sizes[1];
    filterTotalCount = wb_base->weights_sizes[3];

    if (context->options.enableMemOptimization)
    {
        /* reshuffle weight data and save in wb_base->reshuffleWeightPtr if kernel stride > 1 */
        vxoWeightsBiases_Reshuffle(wb_base);
    }
    else
    {
        gctPOINTER weightBase           = VX_NULL;
        vx_uint8_ptr startWeightDataPtr = VX_NULL;
        gctPOINTER biasBase             = VX_NULL;

        /* Get weights & bias base memory */
        vxoTensor_GetTensorViewMemory(weights, &weightBase, VX_NULL);
        startWeightDataPtr = (vx_uint8*)weightBase;

        /* If need reshuffle? */
        if (((strideX > 1) || (strideY > 1)) && (weightDims[0] != 1 || weightDims[1] != 1))
        {
            /* do reshuffle*/
            vx_uint32 alignWeightWidth = vxnneAlignWithStride(weightDims[0], strideX);
            vx_uint32 alignWeightHeight = vxnneAlignWithStride(weightDims[1], strideY);
            vx_uint32 depth = weightDims[2];
            vx_nn_reshuffle_s src = {NULL, alignWeightWidth, alignWeightHeight, depth, weightDims[3] * 1, (vx_type_e)TENSOR_DATA_TYPE(weights)};
            vx_nn_reshuffle_s dst = {NULL, wb_base->weights_sizes[0], wb_base->weights_sizes[1], wb_base->weights_sizes[2], wb_base->weights_sizes[3]*1, (vx_type_e)wb_base->weights_data_format};
            vx_uint32 x, y, z, w;
            vx_uint32 orgXSize = weightDims[0], orgYSize = weightDims[1], orgZSize = depth;
            vx_uint32 weightItemCount = weightCount * sliceCount * filterTotalCount;

            {
                convertedWeightData = vxAllocateAndZeroMemory(weightItemCount * weightSize);
            }

            /* Allocate temp buffer for weight data */
            {
                weightData = vxAllocateAndZeroMemory(weightItemCount * weightSize);
            }

            if (convertedWeightData == VX_NULL || weightData == VX_NULL)
            {
                status = VX_ERROR_NO_MEMORY;
                goto exit;
            }

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
                            vx_uint32 zero = (TENSOR_DATA_TYPE(weights) == VX_TYPE_UINT8 && TENSOR_QUANT_TYPE(weights) == VX_QUANT_AFFINE_SCALE) ? TENSOR_TF_ZEROPOINT(weights) : 0;
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
            dst.data   = weightData;
            reshuffleData(&src, strideX, strideY, &dst);

            /* re-wrap weight buffer */
            weightBase = weightData;

            {
                vxFree(convertedWeightData);
            }

            convertedWeightData = VX_NULL;
        }

        if (biases) vxoTensor_GetTensorViewMemory(biases, &biasBase, VX_NULL);

        /* Save original weight/bias data for sw-tiling */
        {
            vx_uint32 size;

            size = wb_base->weights_sizes[0] * wb_base->weights_sizes[1] * wb_base->weights_sizes[2] * wb_base->weights_sizes[3] *
                   vxDataType_GetSize((vx_type_e)wb_base->weights_data_format);;
            wb_base->weightPtr = (vx_uint8_ptr)vxAllocate(size);
            if (wb_base->weightPtr == VX_NULL)
            {
                status = VX_ERROR_NO_MEMORY;
                goto exit;
            }
            vxMemCopy(wb_base->weightPtr, weightBase, size);

            size = wb_base->weights_sizes[3] * vxDataType_GetSize((vx_type_e)wb_base->biases_data_format);
            wb_base->biasPtr = (vx_uint32_ptr)vxAllocate(size);
            if (wb_base->biasPtr == VX_NULL)
            {
                status = VX_ERROR_NO_MEMORY;
                goto exit;
            }
            if (biases != VX_NULL)
                vxMemCopy(wb_base->biasPtr, biasBase, size);
        }
    }

    weight_bias = vxoWeightsBiases_Create(context,
                                          wb_base,
                                          wb_base->weights_sizes,
                                          layer_type,
                                          vx_true_e);
    if (weight_bias == VX_NULL)
    {
        status = VX_FAILURE;
        goto exit;
    }

    /* allocate mGpuWBTable memory for multiVIP vData */
    {
        gctUINT32 gpuCount = 1;
        vx_uint32 size = 0;
        gcoVX_GetHWConfigGpuCount(&gpuCount);
        if ((gpuCount > 1) && (context->options.enableMultiVIPCombined))
        {
            size = sizeof(vx_weights_biases_parameter) * gpuCount;
            weight_bias->mGpuWBTable = (vx_weights_biases_parameter*)vxAllocateAndZeroMemory((vx_size)size);
        }
        else
        {
            weight_bias->mGpuWBTable = VX_NULL;
        }
        weight_bias->mGpuWBCount = 0;
    }


exit:
    /* Free temp weight data buffer */
    if (convertedWeightData != VX_NULL)
    {
        {
            vxFree(convertedWeightData);
        }

        convertedWeightData = VX_NULL;
    }

    if (weightData != VX_NULL)
    {
        {
            vxFree(weightData);
        }
    }

    return status == VX_SUCCESS ? weight_bias : VX_NULL;
}

vx_weights_biases_parameter _createWeightsBiasesParameterFromParams(
    vx_context  context,
    vx_enum     layer_type,
    vx_uint32 * inputs_dims,
    vx_uint32   pad_x_left,
    vx_uint32   pad_x_right,
    vx_uint32   pad_y_top,
    vx_uint32   pad_y_bottom,
    vx_uint32   pooling_size_x,
    vx_uint32   pooling_size_y,
    vx_enum     down_scale_size_rounding,
    vx_uint32 * convolution_outputs_dims,
    vx_uint32 * pooling_outputs_dims,
    vx_uint32   weights_num_of_dims,
    vx_uint32 * weights_base_dims,
    vx_uint32 * weights_dims,
    vx_enum     weights_data_format,
    vx_enum     weights_quant_format,
    vx_int8     weights_fixed_point_pos,
    vx_uint32   biases_num_of_dims,
    vx_uint32 * biases_base_dims,
    vx_uint32 * biases_dims,
    vx_enum     biases_data_format,
    vx_enum     biases_quant_format,
    vx_int8     biases_fixed_point_pos
    )
{
    vx_weights_biases_parameter wb;
    vx_weights_biases_parameter_base wb_base;

    wb_base = vxoWeightsBiasesBase_Create(context,
                                          layer_type,
                                          inputs_dims,
                                          pad_x_left,
                                          pad_x_right,
                                          pad_y_top,
                                          pad_y_bottom,
                                          pooling_size_x,
                                          pooling_size_y,
                                          1,
                                          1,
                                          down_scale_size_rounding,
                                          convolution_outputs_dims,
                                          pooling_outputs_dims,
                                          weights_num_of_dims,
                                          weights_base_dims,
                                          weights_data_format,
                                          weights_quant_format,
                                          weights_fixed_point_pos,
                                          1,
                                          1.0f,
                                          biases_num_of_dims,
                                          biases_base_dims,
                                          biases_data_format,
                                          biases_quant_format,
                                          biases_fixed_point_pos,
                                          1.0f,
                                          VX_NULL);
    if (wb_base == VX_NULL) return VX_NULL;

    wb = vxoWeightsBiases_Create(context,
                                 wb_base,
                                 weights_dims,
                                 layer_type,
                                 vx_true_e);
    if (wb == VX_NULL) return VX_NULL;

    return wb;
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

