/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
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
#include <gc_hal_vx.h>
#include "gc_hal_types.h"

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
        f16 = (vx_int16)(sign | F16_MAX_EXPONENT);
        if (mantissa) f16 |= (mantissa & F16_MANTISSA_BITS);

    }
    else if (exponent > 15)
    { /* Overflow - flush to Infinity */
        f16 = (vx_int16)(sign | F16_MAX_EXPONENT);
    }
    else if (exponent > -15)
    { /* Representable value */
        exponent += F16_EXPONENT_BIAS;
        mantissa >>= F16_MANTISSA_SHIFT;
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
    vx_float32 result = 0.0f;

    if (fixedPointPos > 0)
    {
        result = (vx_float32)val * (1.0f / ((vx_float32) (1 << fixedPointPos)));
    }
    else
    {
        result = (vx_float32)val * ((vx_float32) (1 << -fixedPointPos));
    }

    return result;
}

vx_float32 Int32toFp32(vx_int32 val, vx_int8 fixedPointPos)
{
    vx_float32 result = 0.0f;

    if (fixedPointPos > 0)
    {
        result = (vx_float32)val * (1.0f / ((vx_float32) (1 << fixedPointPos)));
    }
    else
    {
        result = (vx_float32)val * ((vx_float32) (1 << -fixedPointPos));
    }

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

void vxnneGetPadValue(
    vx_int32 padX,
    vx_int32 padY,
    vx_uint32 *padXLeft,
    vx_uint32 *padXRight,
    vx_uint32 *padYTop,
    vx_uint32 *padYBottom)
{
    vx_uint32 padXValue = (padX < 0) ? ((-1) * padX) : padX;
    vx_uint32 padYValue = (padY < 0) ? ((-1) * padY) : padY;
    if (padXLeft != VX_NULL)
    {
        *padXLeft = padXValue;
    }

    if (padXRight != VX_NULL)
    {
        *padXRight = (padX < 0) ? (padXValue + 1) : padXValue;
    }

    if (padYTop != VX_NULL)
    {
        *padYTop = padYValue;
    }

    if (padYBottom != VX_NULL)
    {
        *padYBottom = (padY < 0) ? (padYValue + 1) : padYValue;
    }
}

vx_int32 vxnneGetTypeSize(vx_type_e format)
{
    vx_int32 size = 0;
    switch(format)
    {
    case VX_TYPE_INT8:
        size = sizeof(vx_int8);
        break;
    case VX_TYPE_FLOAT16:
        size = sizeof(vx_int16);
        break;
    case VX_TYPE_FLOAT32:
        size = sizeof(vx_float32);
        break;
    case VX_TYPE_INT32:
        size = sizeof(vx_int32);
        break;
    default:
        printf("Not support format :%d\n", format);
        break;
    }

    return size;
}

void vxnneSRAMGetkernelPattenField(
    vx_uint32 vipSRAMSize,
    vx_uint32 kernelStreamSize,
    vx_uint32_ptr kernelCacheStartAddress,
    vx_uint32_ptr kernelCachingMode,
    vx_uint32_ptr kernelPatternLow32Bits,
    vx_uint32_ptr kernelPatternHigh32Bits,
    vx_uint32_ptr kernelPatternMsb
    )
{
    vx_uint32 totalSramSize = vipSRAMSize * 1024;
    vx_uint32 sramkernelCacheSize = 0;
    vx_uint64 kernelPatternBits = 0;
    vx_uint32 cachingMode;
    vx_uint32 ratio = 0;

    sramkernelCacheSize = totalSramSize - *kernelCacheStartAddress;

    if (sramkernelCacheSize <= 0)
    {
        cachingMode = 0;
    }
    else if (kernelStreamSize < sramkernelCacheSize)
    {
        cachingMode = 1;
    }
    else
    {
        cachingMode = 2;
    }

    if (kernelCachingMode != VX_NULL)
    {
        *kernelCachingMode = cachingMode;
    }

    if (cachingMode == 2)
    {
        ratio = (vx_int32)gcoMATH_Ceiling((vx_float32)(kernelStreamSize - sramkernelCacheSize) / sramkernelCacheSize);

        if (ratio >= 63)
        {
            // drv set pattern 0x8000 0000 0000 0000, end addr no more than SRAM size.
            // sram fetch from sram until end of boundary
            ratio = 63;
        }

        if (kernelPatternMsb != VX_NULL)
        {
            *kernelPatternMsb = ratio;
        }

        kernelPatternBits = ((vx_uint64) 1 ) << ratio;

        if (kernelPatternLow32Bits != VX_NULL)
        {
            *kernelPatternLow32Bits = kernelPatternBits & 0xFFFFFFFF;
        }

        if (kernelPatternHigh32Bits != VX_NULL)
        {
            *kernelPatternHigh32Bits = kernelPatternBits >> 32;
        }
    }
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

vx_float32 vxnneGetData(vx_type_e format, vx_int32 index, vx_uint8_ptr data, vx_uint8 fixPointPos)
{
    vx_float32 value = 0;
    vx_int8 fpPos = (vx_int8)fixPointPos;

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
        case VX_TYPE_INT32:
            {
                vx_int32_ptr data_ptr = (vx_int32*)data;
                value = Int32toFp32(data_ptr[index], fpPos);
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
            printf("Not support format :%d\n", format);
            break;
        }
    }
    return value;
}

vx_status vxnneSaveData(vx_type_e format, vx_int32 index, vx_float64 data, vx_ptr dst_data, vx_uint8 fixedPointPos, vx_enum roundMode)
{
    vx_int8 fpPos = (vx_int8)fixedPointPos;

    switch(format)
    {
    case VX_TYPE_INT8:
        {
            vx_int8* dst_data_p = (vx_int8*)dst_data;
            dst_data_p[index] = Fp32toInt8((vx_float32)data, fpPos, roundMode);
        }
        break;
    case VX_TYPE_FLOAT16:
        {
            vx_int16* dst_data_p = (vx_int16*)dst_data;
            dst_data_p[index] = Fp32toFp16((vx_float32)data);
        }
        break;
    case VX_TYPE_FLOAT32:
        {
            vx_float32_ptr dst_data_p = (vx_float32_ptr)dst_data;
            dst_data_p[index] = (vx_float32)(data);
        }
        break;
    default:
        printf("Not support format :%d\n", format);
        break;
    }

    return VX_SUCCESS;
}

vx_int32 vxoNNExternsionConvlutionRound(vx_float32 in, vx_enum round_type)
{
    switch (round_type)
    {
    case VX_CONVOLUTIONAL_NETWORK_DS_SIZE_ROUNDING_FLOOR:
        return (vx_int32)gcoMATH_Floor(in);

    case VX_CONVOLUTIONAL_NETWORK_DS_SIZE_ROUNDING_CEILING:
        return (vx_int32)gcoMATH_Ceiling(in);

    default:
        gcmPRINT("nn extension not support this convolution rounding type %d!", round_type);
    }

    return (vx_int32)in;
}

void vxoNNExternsionInputOutputArguments(
    vx_uint32                    org_input_x,
    vx_uint32                    org_input_y,
    vx_uint32                    stride_x,
    vx_uint32                    stride_y,
    vx_uint32                    pad_x_left,
    vx_uint32                    pad_x_right,
    vx_uint32                    pad_y_top,
    vx_uint32                    pad_y_bottom,
    vx_uint32                    weight_x,
    vx_uint32                    weight_y,
    vx_enum                      rounding_type,
    vx_uint32                    pool_size_x,
    vx_uint32 *                  input_x,
    vx_uint32 *                  input_y,
    vx_uint32 *                  output_x,
    vx_uint32 *                  output_y,
    vx_uint32 *                  finalInput_x,
    vx_uint32 *                  finalInput_y,
    vx_uint32 *                  finalOutput_x,
    vx_uint32 *                  finalOutput_y
    )
{
    vx_uint32 inx, iny, outx, outy;

    inx = stride_x > 1 ? vxnneAlignWithStride(org_input_x + pad_x_left + pad_x_right, stride_x) / stride_x : org_input_x;
    iny = stride_y > 1 ? vxnneAlignWithStride(org_input_y + pad_y_top + pad_y_bottom, stride_y) / stride_y : org_input_y;

    outx = vxoNNExternsionConvlutionRound((((vx_float32)(org_input_x + pad_x_left + pad_x_right - weight_x) / (vx_float32)stride_x) + 1), rounding_type);
    outy = vxoNNExternsionConvlutionRound((((vx_float32)(org_input_y + pad_y_top + pad_y_bottom - weight_y) / (vx_float32)stride_y) + 1), rounding_type);

    if (input_x != VX_NULL) *input_x = inx;
    if (input_y != VX_NULL) *input_y = iny;
    if (output_x != VX_NULL) *output_x = outx;
    if (output_y != VX_NULL) *output_y = outy;

    if ((finalInput_x != VX_NULL) && (finalInput_y != VX_NULL) && (finalOutput_x != VX_NULL) && (finalOutput_y != VX_NULL))
    {
        *finalOutput_x =  outx;
        *finalOutput_y = outy;
        *finalInput_x = inx;
        *finalInput_y = iny;

        if ((pool_size_x == 2 && (outx & 0x1)) || (pool_size_x == 3 && !(outx & 0x1)))
        {
            *finalOutput_x = outx + 1;
            *finalInput_x = inx < (*finalOutput_x) ? inx + 1 : inx;
        }

        if ((pool_size_x == 2 && (outy & 0x1)) || (pool_size_x == 3 && !(outy & 0x1)))
        {
            *finalOutput_y = outy + 1;
            *finalInput_y = iny < (*finalOutput_y) ? iny + 1 : iny;
        }
    }
}

vx_bool vxoNNExternsionAdjustWeightsBiases(
    vx_weights_biases_parameter  wb,
    vx_bool                      process_size,
    vx_bool                      viewed,
    vx_size                      wb_size
    )
{
#if ENABLE_SPLIT_WB
    vx_uint32 i = 0, tmp = wb->weights_sizes[3];
    vx_size max = viewed ? FC_SIZE_MAX*2 : FC_SIZE_MAX;

    if (process_size && wb_size > max)
    {
        /* Weight_bias data cannot large than 128MB in old hw version. Current only for vgg FC layer. */
        do
        {
            if (tmp >= 1024)
            {
                wb->zgroup_array[i++] = 1024;
                tmp -= 1024;
            }
            else
            {
                wb->zgroup_array[i++] = tmp;
                break;
            }
        } while (tmp && i < MAX_WEIGHT_BIAS_GROUPS);

        if (i > MAX_WEIGHT_BIAS_GROUPS) return vx_false_e;
        else wb->zgroup_num = i;
    }
    else
#endif
    {
        wb->zgroup_array[0] = wb->weights_sizes[3];
        wb->zgroup_num = 1;
    }

    if (wb->weights_sizes[0] == 1 && wb->weights_sizes[1] == 1 && wb->weights_sizes[2] >= wb->base.context->options.fcZMax)
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

    return vx_true_e;
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
                    dstZ = x%strideStepX + (y%strideStepY)*strideStepY + (z%src->zSize) * strideStepX * strideStepY;
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

vx_uint32 _calcInImageInterleaveMode(vx_uint32 x, vx_uint32 mad_per_core, vx_uint32 kernel_xy, vx_bool vip7_fp16)
{
    if (vip7_fp16)
    {
        if (x + kernel_xy - 1 > (mad_per_core + 4) / 2)
            return 1;
        else
            return 2;
    }
    else
    {
        if (x + kernel_xy - 1 > (mad_per_core + 8) / 2)
            return 1;
        else if (x + kernel_xy - 1 > (mad_per_core + 8) / 4)
            return 2;
        else
            return 4;
    }
}

vx_uint32 _calcOutImageInterleaveMode(vx_uint32 x, vx_uint32 mad_per_core, vx_bool vip7_fp16)
{
    if (vip7_fp16)
        return x > mad_per_core / 2 ? 1 : 2;
    else
        return x > mad_per_core / 2 ? 1 : x > mad_per_core / 4 ? 2 : 4;
}

vx_uint32 _calcImageInterleaveMode(vx_uint32 x, vx_uint32 mad_per_core, vx_uint32 kxy, vx_bool vip7_fp16)
{
    return gcmMIN(_calcInImageInterleaveMode(x, mad_per_core, kxy, vip7_fp16),
                  _calcOutImageInterleaveMode(x, mad_per_core, vip7_fp16));
}

vx_uint32 _calcNumOfKernel(vx_uint32 tile_x, vx_uint32 tile_y, vx_uint32 z, vx_uint32 accu_buf_depth, vx_uint32 cores, vx_uint32 interleave_mode)
{
    vx_uint32 numKernel;

    numKernel = accu_buf_depth * interleave_mode / tile_y;

    return gcmMIN(127, (vx_uint32)gcmMIN(numKernel, ceilf((vx_float32)z / cores)));
}

vx_uint32 _calcComputeCycleCount(
    vx_uint32 tile_x,
    vx_uint32 tile_y,
    vx_uint32 kernel_per_core,
    vx_uint32 x,
    vx_uint32 y,
    vx_uint32 z,
    vx_uint32 kx,
    vx_uint32 ky,
    vx_uint32 kz,
    vx_uint32 mad_per_core,
    vx_uint32 data_size,
    vx_uint32 dp_amount,
    vx_float32 non_zero_ratio,
    vx_uint32 interleave_mode,
    vx_bool vip7_fp16
    )
{
    vx_uint32 tmp, pipeLatency, dpKX, dpAmount;
    vx_uint32 accumCycle, tile3DComputeCycle, bottomTile3DComputeCycle, computeCycle;
    vx_float32 dpNonZeroRatio = 1.0;

    pipeLatency = data_size != 8 ? 6 : dp_amount == 1 ? 4 : 1;

    accumCycle = (vx_uint32)ceilf((vx_float32)tile_y / interleave_mode);

    if (kx * ky == 1)
    {
        tile3DComputeCycle = (vx_uint32)gcmMAX(ceilf((vx_float32)tile_y / interleave_mode) * kernel_per_core, pipeLatency);
    }
    else if (accumCycle == 4)
    {
        tile3DComputeCycle = 4 * kernel_per_core;
    }
    else
    {
        tile3DComputeCycle = gcmMAX(accumCycle, pipeLatency) * kernel_per_core;
    }

    tmp = y % tile_y;
    if (tmp != 0)
    {
        accumCycle = (vx_uint32)ceilf((vx_float32)tmp / interleave_mode);

        if (kx * ky == 1)
        {
            bottomTile3DComputeCycle = (vx_uint32)gcmMAX(ceilf((vx_float32)tmp / interleave_mode) * kernel_per_core, pipeLatency);
        }
        else if (accumCycle == 4)
        {
            bottomTile3DComputeCycle = 4 * kernel_per_core;
        }
        else
        {
            bottomTile3DComputeCycle = (vx_uint32)(gcmMAX(ceilf((vx_float32)tmp / interleave_mode), pipeLatency) * kernel_per_core);
        }
    }
    else
    {
        bottomTile3DComputeCycle = 0;
    }

    dpAmount = dp_amount;
    while (dpAmount--)
    {
        dpNonZeroRatio *= 1.0f - non_zero_ratio;
    }
    dpNonZeroRatio = 1.0f - dpNonZeroRatio;

    dpKX = (vx_uint32)ceilf((vx_float32)kx / dp_amount);
    if (vip7_fp16 || dpNonZeroRatio * dpKX > non_zero_ratio * kx)
    {
        dpNonZeroRatio = non_zero_ratio;
        dpKX = kx;
    }

    computeCycle = tile3DComputeCycle * (y / tile_y) + bottomTile3DComputeCycle;
    computeCycle = (vx_uint32)((vx_float32)computeCycle * dpKX * ky * kz * z * dpNonZeroRatio * ceilf((vx_float32)x / tile_x) / kernel_per_core);

    return computeCycle;
}

vx_float32 _calcKernel4DSingleReadRepeated(vx_uint32 tile_x, vx_uint32 tile_y, vx_uint32 x, vx_uint32 y)
{
    return ceilf((vx_float32)x / tile_x) * ceilf((vx_float32)y / tile_y);
}

vx_float32 _calcKernel4DSingleReadBW(vx_uint32 kx, vx_uint32 ky, vx_uint32 kz, vx_uint32 z, vx_float32 coef_compress_ratio)
{
    return (vx_float32)kx * ky * kz * z * coef_compress_ratio;
}

vx_float32 _calcTile3DImageSingleReadRepeated(vx_uint32 z, vx_uint32 kernel_per_core, vx_uint32 cores)
{
    return ceilf((vx_float32)z / (kernel_per_core * cores));
}

vx_float32 _calcTile3DImageSingleReadBW(
    vx_uint32 tile_x, vx_uint32 tile_y,
    vx_uint32 kx, vx_uint32 ky, vx_uint32 kz,
    vx_uint32 x, vx_uint32 y,
    vx_uint32 inx, vx_uint32 iny,
    vx_uint32 brick_mode,
    vx_uint32 data_size,
    vx_float32 image_compress_ratio)
{
    vx_uint32 intileX, intileY, ppc;
    vx_float32 tile3DImageSingleReadBW;

    intileX = tile_x + kx - 1;
    intileY = tile_y + ky - 1;

    ppc = 64 / (data_size / 8);

    if ((inx <= intileX && iny <= intileY) || brick_mode == 1)
    {
        tile3DImageSingleReadBW = ceilf((vx_float32)intileX * intileY * kz / ppc) * ppc * image_compress_ratio;
    }
    else if (inx <= intileX)
    {
        tile3DImageSingleReadBW = ceilf((vx_float32)intileX * intileY / ppc) * ppc * kz * image_compress_ratio;
    }
    else
    {
        tile3DImageSingleReadBW = ((vx_uint32)((intileX - 1) / ppc) + 1 + ((vx_float32)((intileX - 1) % ppc) / ppc)) * ppc * intileY * kz * image_compress_ratio;
    }

    return tile3DImageSingleReadBW;
}

vx_uint32 _calcReadBandWidth(
    vx_uint32 tile_x,
    vx_uint32 tile_y,
    vx_uint32 kernel_per_core,
    vx_uint32 x, vx_uint32 y, vx_uint32 z,
    vx_uint32 kx, vx_uint32 ky, vx_uint32 kz,
    vx_uint32 inx, vx_uint32 iny,
    vx_uint32 cores,
    vx_uint32 brick_mode,
    vx_uint32 data_size,
    vx_float32 coef_compress_ratio,
    vx_float32 image_compress_ratio,
    vx_uint32 l2cache_size)
{
    vx_float32 cacheSizeInPixel, kernelIdealCache, imageIdealCache;
    vx_float32 kernelRepeatRead, imageRepeatSingleRead, imageRepeatRead, imageRepeatCacheRead, readBandWidth;

    cacheSizeInPixel = l2cache_size * 1000 / ((vx_float32)data_size / 8);

    kernelRepeatRead = _calcKernel4DSingleReadRepeated(tile_x, tile_y, x, y);
    imageRepeatSingleRead = _calcTile3DImageSingleReadRepeated(z, kernel_per_core, cores);
    imageRepeatRead = imageRepeatSingleRead * ((vx_float32)x / tile_x) * ((vx_float32)y / tile_y);
    imageRepeatCacheRead = (imageRepeatSingleRead - 1) * ((vx_float32)x / tile_x) * ((vx_float32)y / tile_y);
    kernelIdealCache = _calcKernel4DSingleReadBW(kx, ky, kz, z, coef_compress_ratio);
    imageIdealCache = _calcTile3DImageSingleReadBW(tile_x, tile_y, kx, ky, kz, x, y, inx, iny, brick_mode, data_size, image_compress_ratio);

    if (kernelRepeatRead >= imageRepeatRead)
    {
        readBandWidth = kernelIdealCache * kernelRepeatRead - gcmMIN(kernelIdealCache, cacheSizeInPixel) * (kernelRepeatRead - 1);
        cacheSizeInPixel = cacheSizeInPixel - gcmMIN(kernelIdealCache, cacheSizeInPixel);
        readBandWidth = readBandWidth + imageIdealCache * imageRepeatRead - gcmMIN(imageIdealCache, cacheSizeInPixel) * imageRepeatCacheRead;
    }
    else
    {
        readBandWidth = imageIdealCache * imageRepeatRead - gcmMIN(imageIdealCache, cacheSizeInPixel) * imageRepeatCacheRead;
        cacheSizeInPixel = cacheSizeInPixel - gcmMIN(imageIdealCache, cacheSizeInPixel);
        readBandWidth = readBandWidth + kernelIdealCache * kernelRepeatRead - gcmMIN(kernelIdealCache, cacheSizeInPixel) * (kernelRepeatRead - 1);
    }

    readBandWidth = readBandWidth * ((vx_float32)data_size / 8);

    return (vx_uint32)ceilf(readBandWidth);
}

vx_uint32 _calcWriteBandWidth(
    vx_uint32 tile_x,
    vx_uint32 tile_y,
    vx_uint32 x,
    vx_uint32 y,
    vx_uint32 z,
    vx_uint32 data_size,
    vx_float32 image_compress_ratio,
    vx_uint32 usc_cache_size,
    vx_uint32 pooling_stride)
{
    vx_uint32 ppc, poolX, poolY, poolTileX, poolTileY;
    vx_float32 cacheSizeInPixel, allTilesBW, rowTilesBW, tileBW, writeBW;

    cacheSizeInPixel = usc_cache_size * 1000 / ((vx_float32)data_size / 8);
    ppc = 64 / (data_size / 8);

    poolX = x / pooling_stride;
    poolY = y / pooling_stride;
    poolTileX = tile_x / pooling_stride;
    poolTileY = tile_y / pooling_stride;

    allTilesBW = ceilf((vx_float32)poolX * poolY * z / ppc) * ppc * image_compress_ratio;
    rowTilesBW = ceilf((vx_float32)poolX * poolTileY / ppc) * ppc * z * image_compress_ratio;
    tileBW = ceilf((vx_float32)poolTileX / ppc) * ppc * poolTileY * z * image_compress_ratio;

    if (poolTileX == poolX && poolTileY == poolY)
    {
        writeBW = allTilesBW;
    }
    else if (tileBW < (cacheSizeInPixel / 2) || rowTilesBW < (cacheSizeInPixel / 2))
    {
        writeBW = allTilesBW;
    }
    else if (poolTileX == poolX)
    {
        writeBW = rowTilesBW;
    }
    else if (tileBW < cacheSizeInPixel || rowTilesBW < cacheSizeInPixel)
    {
        writeBW = allTilesBW * 1.2f;
    }
    else
    {
        writeBW = tileBW * poolX * poolY / (poolTileX * poolTileY);
    }

    writeBW = writeBW * (data_size / 8);
    return (vx_uint32) writeBW;
}

vx_uint32  _calcTPReadBandWidth(vx_enum type, vx_uint32 x, vx_uint32 y, vx_uint32 z, vx_uint32 kz, vx_float32 coef_compress_ratio, vx_float32 image_nonzero_ratio)
{
    if (type == VX_CONVOLUTIONAL_NETWORK_FULLYCONNECTED_LAYER)
    {
        return (vx_uint32) (x * y * z * kz * coef_compress_ratio * image_nonzero_ratio);
    }
    else /* reshuffle */
    {
        return x * y * z;
    }
}

vx_uint32 _calcTPWriteBandWidth(vx_enum type, vx_uint32 x, vx_uint32 y, vx_uint32 z, vx_uint32 pooling_stride)
{
    if (type == VX_CONVOLUTIONAL_NETWORK_FULLYCONNECTED_LAYER)
    {
        return x * y * z / (pooling_stride * pooling_stride);
    }
    else /* reshuffle */
    {
        return x * y * z / (pooling_stride * pooling_stride);
    }
}

vx_uint32 _calcTPCycleCount(vx_enum type, vx_uint32 x, vx_uint32 y, vx_uint32 z, vx_uint32 kz, vx_uint32 cores, vx_float32 coef_nonzero_ratio, vx_float32 image_nonzero_ratio)
{
    if (type == VX_CONVOLUTIONAL_NETWORK_FULLYCONNECTED_LAYER)
    {
        return (vx_uint32) (x * y * z * kz * coef_nonzero_ratio * image_nonzero_ratio / cores);
    }
    else /* reshuffle */
    {
        return x * y * z / cores;
    }
}

void calculateFilterPerCore(vx_context context, vx_weights_biases_parameter wb, vx_uint32 zindex, vx_bool calctp, vx_enum ltype)
{
    /* version 0.29 - 0.37 */
    vx_uint32 numCores, tpCores, madPerCore, accuBuffDepth, inputBuffDepth, dpAmount, l2CacheSize, brickMode, dataSize, uscCacheSize;
    vx_uint32 inXSize, inYSize, outXSize, outYSize, outZSize, kernelXSize, kernelYSize, kernelZSize, poolingSize, poolingStride;
    vx_int32 xOffSet, yOffSet;
    vx_uint32 tmpMaxOutTileYSize, tmpCovAccuMode, tmpCovMode, interleaveMode, cacheLineMode;
    vx_uint32 x, y, k;
    vx_uint32 newCycleCount[2], newRDBandWidth[2], newNCRDBandWidth[2], newWTBandWidth;
    vx_float32 coefNonZeroRatio, coefCompressRatio, imageCompressRatio, imageNonZeroRatio, sustainedBandwidth;
    vx_bool vip7FP16;

    vxoNNExternsionInputOutputArguments(
        wb->layer_type == VX_CONVOLUTIONAL_NETWORK_FULLYCONNECTED_LAYER ? 1 : wb->input_sizes[0],
        wb->layer_type == VX_CONVOLUTIONAL_NETWORK_FULLYCONNECTED_LAYER ? 1 : wb->input_sizes[1],
        wb->stride, wb->stride,
        wb->pad_x_left, wb->pad_x_right, wb->pad_y_top, wb->pad_y_bottom,
        wb->org_weights_sizes[0], wb->org_weights_sizes[1],
        wb->down_scale_size_rounding,
        wb->pooling_size_x,
        VX_NULL, VX_NULL,
        VX_NULL, VX_NULL,
        &inXSize, &inYSize,
        &outXSize, &outYSize);

    outZSize = wb->zgroup_array[zindex];
    kernelXSize = wb->weights_sizes[0];
    kernelYSize = wb->weights_sizes[1];
    kernelZSize = wb->weights_sizes[2];
    xOffSet = (wb->stride > 1) ? 0 : ((-1) * wb->pad_x_left);
    yOffSet = (wb->stride > 1) ? 0 : ((-1) * wb->pad_y_top);
    dataSize = 8 * (vx_uint32)vxDataType_GetSize((vx_type_e)wb->weights_data_format);
    poolingSize = gcmMAX(1, wb->pooling_size_x);
    poolingStride = wb->pooling_size_x ? 2 : 1;

    numCores = context->nnConfig.nnCoreCount;
    tpCores = context->nnConfig.tpCoreCount;
    madPerCore = context->nnConfig.nnMadPerCore;
    inputBuffDepth = context->nnConfig.nnInputBufferDepth;
    accuBuffDepth  = context->nnConfig.nnAccumBufferDepth;
    l2CacheSize = context->nnConfig.nnL2CacheSize = context->nnConfig.vipSRAMSize;

#define NN_BRICK_MODE                          0
#define USC_CACHE_SIZE                         8
    if (context->nnConfig.nnDPAmount == 0)
        context->nnConfig.nnDPAmount = dpAmount = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7) ? 3 : 1;
    else
        dpAmount = context->nnConfig.nnDPAmount;
    context->nnConfig.nnUSCCacheSize = uscCacheSize = USC_CACHE_SIZE;
    brickMode = NN_BRICK_MODE;
    vip7FP16 = (dpAmount == 3 && dataSize == 16) ? vx_true_e : vx_false_e;

    wb->sustained_bandwidth = sustainedBandwidth = context->options.sustainedBandwidth;

    coefNonZeroRatio = (vx_float32)(wb->all_count[zindex] - wb->zero_count[zindex]) / wb->all_count[zindex];
    coefCompressRatio = gcmMIN(1.0f, (vx_float32)wb->compressed_size[zindex] / wb->orig_size[zindex]);
    imageCompressRatio = 1.0f;
    imageNonZeroRatio = 0.3f;

    if (!calctp || ltype != VX_CONVOLUTIONAL_NETWORK_FULLYCONNECTED_LAYER)
    {
        /* init to default */
        wb->outImageTileXSize[0][zindex] = wb->outImageTileXSize[1][zindex] = outXSize;
        wb->outImageTileYSize[0][zindex] = wb->outImageTileYSize[1][zindex] = outYSize;
        wb->kernelsPerCore[0][zindex]    = wb->kernelsPerCore[1][zindex]    = outZSize;
        wb->perfCycleCount[0][zindex]      = wb->perfCycleCount[1][zindex]      = ~0UL;
        wb->perfReadBandWidth[0][zindex]   = wb->perfReadBandWidth[1][zindex]   = ~0UL;
        wb->perfNCReadBandWidth[0][zindex] = wb->perfNCReadBandWidth[1][zindex] = ~0UL;
        wb->perfWriteBandWidth[0][zindex]  = wb->perfWriteBandWidth[1][zindex]  = ~0UL;

        if (vip7FP16) madPerCore /= 2;
        wb->current_mad_per_core = madPerCore;

        for (x = 1; x <= gcmMIN(outXSize, madPerCore); x++)
        {
            if ((poolingSize != 2 && poolingSize != 3) ||
                (poolingSize == 2 && x % 2 == 0) ||
                (poolingSize == 3 && x == outXSize))
            {
                interleaveMode = _calcImageInterleaveMode(x, madPerCore, kernelXSize, vip7FP16);

                tmpCovMode = inputBuffDepth * interleaveMode;
                tmpCovAccuMode = accuBuffDepth * interleaveMode;

                tmpMaxOutTileYSize = gcmMIN(127, gcmMIN(tmpCovMode-kernelYSize+1, gcmMIN(tmpCovAccuMode, outYSize)));

                for (y = 1; y <= tmpMaxOutTileYSize; y++)
                {
                    if (outXSize - xOffSet <= x + kernelXSize -1 &&
                        outYSize - yOffSet <= y + kernelYSize -1)
                    {
                        cacheLineMode = 1;
                    }
                    else
                    {
                        cacheLineMode = 0;
                    }

                    if ((brickMode || !cacheLineMode || (x == outXSize && y == outYSize)) &&
                        ((poolingSize != 2 && poolingSize != 3) ||
                         (poolingSize == 2 && y % 2 == 0) ||
                         (poolingSize == 3 && (y != 3 || outYSize == 3))))
                    {
                        k = _calcNumOfKernel(x, y, outZSize, accuBuffDepth, numCores, interleaveMode);

                        newRDBandWidth[0] = _calcReadBandWidth(x, y, k, outXSize, outYSize, outZSize, kernelXSize, kernelYSize, kernelZSize, outXSize, outYSize, numCores, brickMode, dataSize, coefCompressRatio, imageCompressRatio, l2CacheSize);
                        newNCRDBandWidth[0] = _calcReadBandWidth(x, y, k, outXSize, outYSize, outZSize, kernelXSize, kernelYSize, kernelZSize, outXSize, outYSize, numCores, brickMode, dataSize, coefCompressRatio, imageCompressRatio, 0);

                        newWTBandWidth = _calcWriteBandWidth(x, y, outXSize, outYSize, outZSize, dataSize, imageCompressRatio, uscCacheSize, poolingStride);

                        newCycleCount[0] = (vx_uint32)((vx_float32)_calcComputeCycleCount(x, y, k, outXSize, outYSize, outZSize, kernelXSize, kernelYSize, kernelZSize, madPerCore, dataSize, dpAmount, coefNonZeroRatio, interleaveMode, vip7FP16) * ceilf((vx_float32)outZSize / numCores) / outZSize);

                        newCycleCount[0] = gcmMAX(newCycleCount[0], (vx_uint32)((vx_float32)newRDBandWidth[0] / sustainedBandwidth));

                        if ((newCycleCount[0] < wb->perfCycleCount[0][zindex])
                           || (newCycleCount[0] == wb->perfCycleCount[0][zindex] && newRDBandWidth[0]  < wb->perfReadBandWidth[0][zindex])
                           || (newCycleCount[0] == wb->perfCycleCount[0][zindex] && newRDBandWidth[0] == wb->perfReadBandWidth[0][zindex] && newWTBandWidth < wb->perfWriteBandWidth[0][zindex])
                           || (newCycleCount[0] == wb->perfCycleCount[0][zindex] && newRDBandWidth[0] == wb->perfReadBandWidth[0][zindex] && newWTBandWidth == wb->perfWriteBandWidth[0][zindex] && newNCRDBandWidth[0] < wb->perfNCReadBandWidth[0][zindex])
                           )
                        {
                            wb->outImageTileXSize[0][zindex]   = x;
                            wb->outImageTileYSize[0][zindex]   = y;
                            wb->kernelsPerCore[0][zindex]      = k;
                            wb->perfCycleCount[0][zindex]      = newCycleCount[0];
                            wb->perfReadBandWidth[0][zindex]   = newRDBandWidth[0];
                            wb->perfNCReadBandWidth[0][zindex] = newNCRDBandWidth[0];
                            wb->perfWriteBandWidth[0][zindex]  = newWTBandWidth;
                        }

                        if (context->options.enableIdealPerf)
                        {
                            newRDBandWidth[1] = _calcReadBandWidth(x, y, k, outXSize, outYSize, outZSize, kernelXSize, kernelYSize, kernelZSize, outXSize, outYSize, numCores, brickMode, dataSize, 1.0f, imageCompressRatio, l2CacheSize);
                            newNCRDBandWidth[1] = _calcReadBandWidth(x, y, k, outXSize, outYSize, outZSize, kernelXSize, kernelYSize, kernelZSize, outXSize, outYSize, numCores, brickMode, dataSize, 1.0f, imageCompressRatio, 0);

                            newCycleCount[1] = (vx_uint32)((vx_float32)_calcComputeCycleCount(x, y, k, outXSize, outYSize, outZSize, kernelXSize, kernelYSize, kernelZSize, madPerCore, dataSize, dpAmount, 1.0f, interleaveMode, vip7FP16) * ceilf((vx_float32)outZSize / numCores) / outZSize);

                            newCycleCount[1] = gcmMAX(newCycleCount[1], (vx_uint32)((vx_float32)newRDBandWidth[1] / sustainedBandwidth));

                            if ((newCycleCount[1] < wb->perfCycleCount[1][zindex])
                               || (newCycleCount[1] == wb->perfCycleCount[1][zindex] && newRDBandWidth[1]  < wb->perfReadBandWidth[1][zindex])
                               || (newCycleCount[1] == wb->perfCycleCount[1][zindex] && newRDBandWidth[1] == wb->perfReadBandWidth[1][zindex] && newWTBandWidth < wb->perfWriteBandWidth[1][zindex])
                               || (newCycleCount[1] == wb->perfCycleCount[1][zindex] && newRDBandWidth[1] == wb->perfReadBandWidth[1][zindex] && newWTBandWidth == wb->perfWriteBandWidth[1][zindex] && newNCRDBandWidth[1] < wb->perfNCReadBandWidth[1][zindex])
                               )
                            {
                                wb->outImageTileXSize[1][zindex]   = x;
                                wb->outImageTileYSize[1][zindex]   = y;
                                wb->kernelsPerCore[1][zindex]      = k;
                                wb->perfCycleCount[1][zindex]      = newCycleCount[1];
                                wb->perfReadBandWidth[1][zindex]   = newRDBandWidth[1];
                                wb->perfNCReadBandWidth[1][zindex] = newNCRDBandWidth[1];
                                wb->perfWriteBandWidth[1][zindex]  = newWTBandWidth;
                            }
                        }
                    }
                }
            }
        }

        vxmASSERT(wb->outImageTileXSize[0][zindex] <= wb->current_mad_per_core);
    }

    if (calctp)
    {
        wb->perfTPReadBandWidth[zindex] = _calcTPReadBandWidth(ltype, outXSize, outYSize, outZSize, kernelZSize, coefCompressRatio, imageNonZeroRatio);
        wb->perfTPWriteBandWidth[zindex] = _calcTPWriteBandWidth(ltype, outXSize, outYSize, outZSize, poolingSize);
        wb->perfTPCycleCount[zindex] = _calcTPCycleCount(ltype, outXSize, outYSize, outZSize, kernelZSize, tpCores, coefNonZeroRatio, imageNonZeroRatio);
    }
}

static vx_uint32 gLayerCount = 0;
vx_status vxoWeightsBiasesParameter_ShowPerformance(
    vx_context context,
    vx_weights_biases_parameter wb
    )
{
    vx_uint32 i;
    vx_size cycleCount[2]={0}, rdBandWidth[2]={0}, ncRDBandWidth[2]={0}, wtBandWidth[2]={0}, cycleCountTP=0, rdBandWidthTP=0, wtBandWidthTP=0;
    vx_uint32 outX = wb->pooling_size_x == 3 && !(wb->output_sizes[0] & 0x1) ? wb->output_sizes[0] + 1 : wb->output_sizes[0];
    vx_uint32 outY = wb->pooling_size_y == 3 && !(wb->output_sizes[1] & 0x1) ? wb->output_sizes[1] + 1 : wb->output_sizes[1];

    printf("%d %s\n\n", gLayerCount++, wb->layer_type==VX_CONVOLUTIONAL_NETWORK_CONVOLUTION_LAYER?"convolution layer":"fc layer");
    if (wb->zgroup_num)
    {
        printf("NumNNCores: %d\nMadPerCore: %d\nInBuffDepth: %d\nAccumBufferDepth: %d\nDPAmount: %d\nL2CacheSize: %d\nUSCCacheSize: %d\nDataSize: %d\nSustainBandWidth: %.3f\n\n",
               context->nnConfig.nnCoreCount,
               context->nnConfig.nnMadPerCore,
               context->nnConfig.nnInputBufferDepth,
               context->nnConfig.nnAccumBufferDepth,
               context->nnConfig.nnDPAmount,
               context->nnConfig.nnL2CacheSize,
               context->nnConfig.nnUSCCacheSize,
               8*(vx_uint32)vxDataType_GetSize((vx_type_e)wb->weights_data_format),
               wb->sustained_bandwidth);

        printf("InImageX: %d\nInImageY: %d\nOutImageX: %d (%d)\nOutImageY: %d (%d)\n",
               wb->input_sizes[0],
               wb->input_sizes[1],
               outX, wb->output_sizes[0],
               outY, wb->output_sizes[1]);

        for(i = 0; i < wb->zgroup_num; i++)
        {
            printf("OutImageZ[%d]: %d (%d)\n", i, wb->zgroup_array[i], wb->org_weights_sizes[3]);
        }

        printf("KernelX: %d (%d)\nKernelY: %d (%d)\nKernelZ: %d (%d)\nPoolingSize: %d\nPoolingStride: %d\n\n",
               wb->weights_sizes[0], wb->org_weights_sizes[0],
               wb->weights_sizes[1], wb->org_weights_sizes[1],
               wb->weights_sizes[2], wb->org_weights_sizes[2],
               wb->pooling_size_x,
               wb->pooling_size_x ? 2 : 1);
    }

    for(i = 0; i < wb->zgroup_num; i++)
    {
        printf("coefNonZeroRatio[%d]: %.4f\ncoefCompression[%d]: %.4f\nimageCompression[%d]: %.4f\nimageNonZeroRatio[%d]: %.4f\n\n",
                i, (vx_float32)(wb->all_count[i] - wb->zero_count[i]) / wb->all_count[i],
                i, gcmMIN(1.0f, (vx_float32)wb->compressed_size[i] / wb->orig_size[i]),
                i, 1.0,
                i, 0.3
                );

        printf("OutImageTileXSize[%d]: %d\nOutImageTileYSize[%d]: %d\nKernelsPerCore[%d]: %d\n",
                i, wb->outImageTileXSize[0][i],
                i, wb->outImageTileYSize[0][i],
                i, wb->kernelsPerCore[0][i]
              );

        cycleCount[0] += wb->perfCycleCount[0][i];
        rdBandWidth[0] += wb->perfReadBandWidth[0][i];
        ncRDBandWidth[0] += wb->perfNCReadBandWidth[0][i];
        wtBandWidth[0] += wb->perfWriteBandWidth[0][i];

        if (context->options.enableIdealPerf)
        {
            printf("\nIdeal Environment:\nOutImageTileXSize[%d]: %d\nOutImageTileYSize[%d]: %d\nKernelsPerCore[%d]: %d\n",
                    i, wb->outImageTileXSize[1][i],
                    i, wb->outImageTileYSize[1][i],
                    i, wb->kernelsPerCore[1][i]
                  );

            cycleCount[1] += wb->perfCycleCount[1][i];
            rdBandWidth[1] += wb->perfReadBandWidth[1][i];
            ncRDBandWidth[1] += wb->perfNCReadBandWidth[1][i];
            wtBandWidth[1] += wb->perfWriteBandWidth[1][i];
        }

        cycleCountTP += wb->perfTPCycleCount[i];
        rdBandWidthTP += wb->perfTPReadBandWidth[i];
        wtBandWidthTP += wb->perfTPWriteBandWidth[i];
    }

    printf("\n");
    if (wb->zgroup_num)
    {
        printf("NN ReadBW: %llu (nocache: %llu)\nNN WriteBW: %llu\nNN CycleCount: %llu\n\n",
                (unsigned long long)rdBandWidth[0], (unsigned long long)ncRDBandWidth[0], (unsigned long long)wtBandWidth[0], (unsigned long long)cycleCount[0]);

        if (context->options.enableIdealPerf)
        {
            printf("Ideal Environment:\n");
            printf("NN ReadBW: %llu (nocache: %llu)\nNN WriteBW: %llu\nNN CycleCount: %llu\n\n",
                    (unsigned long long)rdBandWidth[1], (unsigned long long)ncRDBandWidth[1], (unsigned long long)wtBandWidth[1], (unsigned long long)cycleCount[1]);
        }

        printf("TP ReadBW: %llu\nTP WriteBW: %llu\nTP CycleCount: %llu\n",
                (unsigned long long)rdBandWidthTP, (unsigned long long)wtBandWidthTP, (unsigned long long)cycleCountTP);

        printf("=========================\n\n");
    }

    return VX_SUCCESS;
}

vx_bool WeightBiasBufferAllocate(vx_context context, vx_weights_biases_parameter weight_bias, vx_size size, vx_bool raw_data)
{
    gceSTATUS   status;
    vx_memory memory;

    gcmASSERT(context);
    gcmASSERT(weight_bias);

    memory = &weight_bias->memory;

    if (memory->allocated) return vx_true_e;

    if (!raw_data)
        size += weight_bias->memroy_head_offset;

    memory->allocated = vx_true_e;
    status = gcoVX_AllocateMemory((gctUINT32)size, (gctPOINTER*)&memory->logicals[0], &memory->physicals[0], &memory->nodePtrs[0]);
    context->memoryCount++;
    if (gcmIS_ERROR(status)) goto ErrorExit;

    gcmASSERT(memory->logicals[0]);
    if (!vxCreateMutex(OUT &memory->writeLocks[0]))
    {
        memory->writeLocks[0] = VX_NULL;
        goto ErrorExit;
    }

    if (!raw_data)
    {
        memory->physicals[0] += weight_bias->memroy_head_offset;
        memory->logicals[0] += weight_bias->memroy_head_offset;
    }
    weight_bias->memory_size = size;

    vxoMemory_Dump(memory);
    return vx_true_e;

ErrorExit:
    if (memory->logicals[0] != VX_NULL)
    {
        gcoVX_FreeMemory((gcsSURF_NODE_PTR)memory->nodePtrs[0]);
        memory->logicals[0]    = VX_NULL;
        memory->nodePtrs[0]    = VX_NULL;
    }

    if (memory->writeLocks[0] != VX_NULL)
    {
        vxDestroyMutex(memory->writeLocks[0]);
        memory->writeLocks[0]  = VX_NULL;
    }

    memory->allocated = vx_false_e;

    return vx_false_e;
}

vx_size calculateWeightBiasBufferSizeForZeroRunLen(vx_weights_biases_parameter wb, vx_tensor weights, uint8_t zeroRunLen, vx_uint32 filtersPerCore, vx_uint32 sliceCount, vx_uint32 z_count, vx_uint32 index, void* weightData)
{
    vx_context context           = vxGetContext((vx_reference)wb);
    vx_uint32 nnCoreCount        = context->nnConfig.nnCoreCount;
    vx_size kernelBufferSize     = 0;

    uint32_t maxZeroRun          = (1 << zeroRunLen) - 1;

    vx_uint32 filterTotalCount   = z_count;
    vx_uint32 filterCount        = filtersPerCore; /* filter count every group */
    vx_uint32 batchSize          = nnCoreCount * filterCount;
    vx_uint32 groupCount         = (filterTotalCount + batchSize - 1) / batchSize;
    vx_uint32 totalFilterPerCore = filterTotalCount / nnCoreCount;
    vx_uint32 oddFilterPerCore   = filterTotalCount % nnCoreCount;

    vx_uint32 weightCount        = wb->weights_sizes[0] * wb->weights_sizes[1];
    vx_uint32 weightSize         = (wb->weights_data_format == VX_TYPE_INT8) ? (vx_uint32)vxDataType_GetSize(VX_TYPE_INT8) : (vx_uint32)vxDataType_GetSize(VX_TYPE_INT16);
    vx_uint32 weightBitSize      = weightSize * 8;
    vx_uint32 biasBitSize        = (vx_uint32)vxDataType_GetSize(VX_TYPE_FLOAT32) * 8;
    vx_uint32 filterSliceSize    = weightCount * weightSize;
    vx_uint32 filterSize         = weightCount * weightSize * sliceCount;
    vx_uint8* startDataPtr       = VX_NULL;
    vx_uint8* kernelDataPtr      = VX_NULL;

    vx_uint32 bitOffset          = 0;
    vx_uint32 count              = 0;
    vx_uint32 zeroCount          = 0;

    vx_uint32 coreIndex;
    vx_uint32 i, groupIndex, filterIndex, sliceIndex;
    vx_uint32 weightXIndex, weightYIndex;
    uint32_t zeroRun;
    gctPOINTER weightBase = VX_NULL;

    vx_size weightDataOffset = 0;

    for (i = 0; i < index; i++) weightDataOffset += filterSize * wb->zgroup_array[i];

    vxoTensor_GetTensorBaseMemory(weights, &weightBase, VX_NULL);
    if (weightData != VX_NULL)
        startDataPtr = (vx_uint8*)weightData + weightDataOffset;
    else if (weights->isViewed)
        startDataPtr = (vx_uint8*)weightBase + weightDataOffset +
        weightSize *  sliceCount * weightCount * weights->viewRegion.viewStarts[3];
    else
        startDataPtr = (vx_uint8*)weightBase + weightDataOffset;

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
                        for (weightXIndex = 0; weightXIndex < wb->weights_sizes[0]; weightXIndex++)
                        {
                            vx_uint32 weight;

                            if (wb->weights_data_format == VX_TYPE_INT8)
                                weight = *((vx_int8 *)kernelDataPtr);
                            else
                                weight = *((vx_uint16 *)kernelDataPtr);
                            kernelDataPtr = kernelDataPtr + weightSize;

                            if ((zeroRun == maxZeroRun)
                                || ((sliceIndex == 0)
                                    && (weightXIndex == wb->weights_sizes[0] - 1)
                                    && (weightYIndex == wb->weights_sizes[1] - 1)
                                    && needToWriteBias)
                                || ((weightXIndex == wb->weights_sizes[0] - 1)
                                    && (weightYIndex == wb->weights_sizes[1] - 1)
                                    && (sliceIndex == sliceCount - 1))
                                || (weight != 0)
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
                            }
                            else
                            {
                                zeroRun++;
                            }

                            if (!weight) zeroCount++;
                            count++;
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

        /* pad 0 */
        if (bitOffset)
        {
            kernelBufferSize += 4;
        }
        kernelBufferSize = (kernelBufferSize + 63) & 0xFFFFFFC0;

        wb->all_count[index]  = count;
        wb->zero_count[index] = zeroCount;
    }

    return kernelBufferSize;
}

/* Write dataBits bits from data to buffer starting from bitOffset. */
void writeBits(uint32_t **buffer, vx_uint32 *bitOffset, vx_uint32 data, vx_uint32 dataBits)
{
    vx_uint32 copiedBits;

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
    **buffer = (data >> copiedBits);
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


void calculateWeightBiasStreamRelatedSize(
    vx_weights_biases_parameter wb,
    vx_tensor weights,
    vx_uint32 kernels_per_core,
    vx_uint32 slice_count,
    vx_int32 output_z_index,
    void* weight_data,
    vx_size* min_kernel_buf_size,
    vx_uint8* min_zero_run_len,
    vx_uint32* max_zero_run_len,
    vx_int8 setZrl
    )
 {
    vx_uint8 minZeroRunLen=0, zeroRunLen;
    vx_uint32 z_count, index;
    vx_size minKernelBufferSize=~0UL, kernelBufferSize;

    index = gcmMAX(0, output_z_index);
    z_count = output_z_index < 0 ?  wb->weights_sizes[3] : wb->zgroup_array[output_z_index];

    {
        zeroRunLen = (vx_uint8)wb->base.context->options.nnZeroRunLen;

        /* To save HW logic, when kernel Z size <= 1, minZeroRunLen should be 0 */
        if (weights->dims[2] <= 1)
        {
            minKernelBufferSize = calculateWeightBiasBufferSizeForZeroRunLen(wb, weights, 0, kernels_per_core, slice_count, z_count, index, weight_data);
            minZeroRunLen = 0;
        }
        else if (setZrl >= 0)
        {
            minKernelBufferSize = calculateWeightBiasBufferSizeForZeroRunLen(wb, weights, setZrl, kernels_per_core, slice_count, z_count, index, weight_data);
            minZeroRunLen = setZrl;
        }
        else if (zeroRunLen > 5)
        {
            /* Calulate kernelBufferSize for other zeroRunLen. */
            for (zeroRunLen = 0; zeroRunLen <= 5; zeroRunLen++)
            {
                kernelBufferSize = calculateWeightBiasBufferSizeForZeroRunLen(wb, weights, zeroRunLen, kernels_per_core, slice_count, z_count, index, weight_data);

                if (kernelBufferSize < minKernelBufferSize)
                {
                    minKernelBufferSize = kernelBufferSize;
                    minZeroRunLen = zeroRunLen;
                }
            }
        }
        else
        {
                minKernelBufferSize = calculateWeightBiasBufferSizeForZeroRunLen(wb, weights, zeroRunLen, kernels_per_core, slice_count, z_count, index, weight_data);
                minZeroRunLen = zeroRunLen;
        }
    }
    wb->compressed_size[index] = (vx_uint32)minKernelBufferSize;
    if (wb->weights_data_format == VX_TYPE_INT8)
    {
        wb->orig_size[index] = wb->weights_sizes[0] * wb->weights_sizes[1] * wb->weights_sizes[2] * z_count * sizeof(vx_int8) +
                           z_count * sizeof(vx_float32);
    }
    else
    {
        wb->orig_size[index] = wb->weights_sizes[0] * wb->weights_sizes[1] * wb->weights_sizes[2] * z_count * sizeof(vx_uint16) +
                           z_count * sizeof(vx_float32);
    }

    if (min_kernel_buf_size != VX_NULL)
        *min_kernel_buf_size = minKernelBufferSize;
    if (min_zero_run_len != VX_NULL)
        *min_zero_run_len = minZeroRunLen;
    if (max_zero_run_len != VX_NULL)
        *max_zero_run_len = (1 << minZeroRunLen) - 1;
}

vx_size calculateTPWeightStreamSizeForZeroRunLen(
    vx_weights_biases_parameter wb,
    uint8_t zeroRunLenBitWidth,
    vx_uint32 sliceCount,
    vx_uint32 filter_count,
    vx_uint8_ptr weight_base_ptr
    )
{
    vx_uint32 maxZeroRun        = (1 << zeroRunLenBitWidth) - 1;
    vx_uint32 filterCount       = filter_count;
    vx_uint32 weightSize        = (wb->weights_data_format == VX_TYPE_INT8) ? (vx_uint32)vxDataType_GetSize(VX_TYPE_INT8) : (vx_uint32)vxDataType_GetSize(VX_TYPE_INT16);
    vx_uint32 weightBitSize     = weightSize * 8;
    vx_uint32 filterSize        = weightSize * sliceCount;
    vx_uint8* kernelDataPtr     = weight_base_ptr;
    vx_size kernelBufferSize    = 0;
    vx_uint32 bitOffset         = 0;
    uint32_t zeroRun            = 0;
    vx_uint32 filterIndex;

    /* Fill zeroRunLenBitWidth. */
    bitOffset = 4;
    for (filterIndex = 0; filterIndex < filterCount; filterIndex++)
    {
        vx_uint32 weight = 0;

        if (wb->weights_data_format == VX_TYPE_INT8)
            weight = *((vx_int8 *)kernelDataPtr);
        else
            weight = *((vx_uint16 *)kernelDataPtr);
        kernelDataPtr += filterSize;

        if ((zeroRun == maxZeroRun)
            || (weight != 0)
            || (filterIndex == (filterCount - 1)))
        {
            /* Write zeroRun and weight. */
            bitOffset += zeroRunLenBitWidth + weightBitSize;
            if (bitOffset >= 32)
            {
                bitOffset -= 32;
                kernelBufferSize += 4;
            }
            zeroRun = 0;
        }
        else
        {
            zeroRun++;
        }
    }
    /* pad 0 */
    if (bitOffset)
    {
        kernelBufferSize += 4;
    }
    kernelBufferSize = (kernelBufferSize + 63) & 0xFFFFFFC0;

    return kernelBufferSize;
}

void calculateWeightBiasTPBufferRelatedSize(
    vx_weights_biases_parameter wb,
    vx_int8 setZrl,
    vx_uint32 slice_count,
    vx_uint32 filter_count,
    vx_int32 zgroup_index,
    vx_uint8_ptr weight_base_ptr,
    vx_size* min_kernel_buf_size,
    vx_uint8* min_zero_run_len
    )
 {
    vx_uint32 weightSize            = (wb->weights_data_format == VX_TYPE_INT8) ? (vx_uint32)vxDataType_GetSize(VX_TYPE_INT8) : (vx_uint32)vxDataType_GetSize(VX_TYPE_INT16);
    vx_uint32 biasSize              = (vx_uint32)vxDataType_GetSize(VX_TYPE_FLOAT32);
    vx_uint32 sliceCount            = slice_count;
    vx_uint32 filterCount           = filter_count;
    vx_uint8_ptr kernelDataPtr      = VX_NULL;
    vx_size kernelBufferSize        = 0;
    vx_uint32 bitOffset             = 0;
    vx_int8 zrlBitWidth             = -1;
    vx_uint32 filterIndex, sliceIndex;
    vx_uint8 minZrlBitWidth;
    vx_size minWeightStreamSize;

    if ((setZrl >= 0) && (setZrl <= 9))
    {
        zrlBitWidth = setZrl;
    }
    else
    {
        zrlBitWidth = (vx_int8) wb->base.context->options.tpZeroRunLen;
    }

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
            minWeightStreamSize = calculateTPWeightStreamSizeForZeroRunLen(wb, zrlBitWidth, slice_count, filter_count, kernelDataPtr);
            kernelBufferSize += minWeightStreamSize;
            min_zero_run_len[sliceIndex] = zrlBitWidth;
        }
    }
    else
    {
        for (sliceIndex = 0; sliceIndex < sliceCount; sliceIndex++)
        {
            kernelDataPtr = weight_base_ptr + weightSize * sliceIndex;
            /* Calculate the stream size of 0 zrlBitWidth. */
            minZrlBitWidth = 0;
            minWeightStreamSize = (1 + filterCount * weightSize + 63) & 0xFFFFFFC0;

            /* Calulate weightStreamSize for the rest zrlBitWidth. */
            for (zrlBitWidth = 1; zrlBitWidth <= 9; zrlBitWidth++)
            {
                vx_size weightStreamSize = calculateTPWeightStreamSizeForZeroRunLen(wb, zrlBitWidth, slice_count, filter_count, kernelDataPtr);

                if (weightStreamSize < minWeightStreamSize)
                {
                    minWeightStreamSize = weightStreamSize;
                    minZrlBitWidth = zrlBitWidth;
                }
            }
            kernelBufferSize += minWeightStreamSize;
            min_zero_run_len[sliceIndex] = minZrlBitWidth;
        }
    }

    wb->compressed_size[zgroup_index] = (vx_uint32)kernelBufferSize;
    wb->orig_size[zgroup_index] = slice_count * filter_count * weightSize + filter_count * biasSize;

    *min_kernel_buf_size = kernelBufferSize;
}

vx_weights_biases_parameter _createWeightsBiasesParameterFromTensors(
    vx_context  context,
    vx_enum     layer_type,
    vx_uint32   num_of_dims,
    vx_uint32 * inputs_dims,
    vx_uint32   pad_x_left,
    vx_uint32   pad_x_right,
    vx_uint32   pad_y_top,
    vx_uint32   pad_y_bottom,
    vx_uint32   pooling_size_x,
    vx_uint32   pooling_size_y,
    vx_enum     down_scale_size_rounding,
    vx_uint32 * convolution_outputs_dims,
    vx_uint32 * pool_outputs_dims,
    vx_weights_biases_parameter_optimizations_t *optimizations,
    vx_tensor   weights,
    vx_tensor   biases
    )
{
    vx_weights_biases_parameter weights_bias;
    void* convertedWeightData   = VX_NULL;
    void* weightData            = VX_NULL;

    vx_uint32 weightDimCount    = TENSOR_DIM_NUM(weights);
    vx_uint32 biasDimCount      = 0;
    vx_enum weightType          = TENSOR_DATA_TYPE(weights);
    vx_enum biasType            = VX_TYPE_FLOAT32;
    vx_uint8 weightPos          = TENSOR_POS(weights);
    vx_uint8 biasPos            = 0;
    vx_uint32* weightDims       = (vx_uint32 *)vxAllocateAndZeroMemory(weightDimCount * sizeof(vx_uint32));
    vx_uint32* biasDims         = NULL;
    vx_uint32* weightViewStarts = (vx_uint32 *)vxAllocateAndZeroMemory(weightDimCount * sizeof(vx_uint32));
    vx_uint32* weightViewEnds   = (vx_uint32 *)vxAllocateAndZeroMemory(weightDimCount * sizeof(vx_uint32));
    vx_uint32* biasViewStarts   = NULL;
    vx_uint32* biasViewEnds     = NULL;

    vx_uint32 sliceCount;
    vx_uint32 weightCount;
    vx_uint32 filterTotalCount;
    vx_uint32 filterSize;

    vx_size minKernelBufferSize[MAX_ZGROUP_COUNT];
    vx_uint8 minZeroRunLen[MAX_WEIGHT_BIAS_GROUPS];
    vx_uint32 maxZeroRun[MAX_WEIGHT_BIAS_GROUPS];
    vx_size minTotalKernelBufferSize = 0;

    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize((vx_type_e)weightType);
    vx_enum outType;
    vx_uint32 outputSize = 0;

    vx_uint8_ptr kernelBufferBasePtr = VX_NULL;
    vx_uint8_ptr startWeightDataPtr  = VX_NULL;
    vx_uint32* startBiasDataPtr  = VX_NULL;
    gctPOINTER weightBase        = VX_NULL;
    gctPOINTER biasBase          = VX_NULL;

    vx_uint32 poolingStride      = 0;
    vx_uint32 i                  = 0;
    vx_bool   locked             = vx_false_e;
    vx_status status             = VX_SUCCESS;

    vx_size weightDataBytesOffset = 0;
    vx_size biasDataDWordOffset = 0;
    vx_size compressDataBytesOffset = 0;

    vx_uint32 fcThreshold = 0;
    vx_bool useFCAccel = vx_false_e;

    vx_int8 setZeroLength = -1;

    if (weightDims == VX_NULL || weightViewStarts == VX_NULL || weightViewEnds == VX_NULL)
    {
        if (weightViewStarts != VX_NULL)
            vxFree(weightViewStarts);
        if (weightViewEnds != VX_NULL)
            vxFree(weightViewEnds);
        if (weightDims != VX_NULL)
            vxFree(weightDims);

        vxError("vxCreateWeightsBiasesParameterFromTensors: OUT OF MEMORY");
        return VX_NULL;
    }

    context->options.fcZMax = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7) ? (1<<20) - 1 : (1<<14) - 1;

    if (optimizations)
    {
        setZeroLength = optimizations->zrl;
        outType = optimizations->outputFormat;
        outputSize = (vx_uint32)vxDataType_GetSize((vx_type_e)outType);
    }

    /* Get tensor view region */
    vxoTensor_GetTensorViewRegion(weights, weightDimCount, weightViewStarts, weightViewEnds);
    if (weights->isViewed)
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

    if ((layer_type == VX_CONVOLUTIONAL_NETWORK_FULLYCONNECTED_LAYER) && ((weightDims[0] != 1) && (weightDims[1] != 1)))
    {
        vx_int32 index;
        for (index = weightDimCount - 1; index >= 0; index--)
        {
            weightDims[index] = (index == 0 || index == 1) ? 1 : (index == 2) ? (weightDims[index] * weightDims[index-1] * weightDims[index-2]) : weightDims[index];
        }
    }

    if (biases)
    {
        biasDimCount     = TENSOR_DIM_NUM(biases);
        biasType         = TENSOR_DATA_TYPE(biases);
        biasPos          = TENSOR_POS(biases);
        biasDims         = (vx_uint32 *)vxAllocateAndZeroMemory(biasDimCount * sizeof(vx_uint32));
        biasViewStarts   = (vx_uint32 *)vxAllocateAndZeroMemory(biasDimCount * sizeof(vx_uint32));
        biasViewEnds     = (vx_uint32 *)vxAllocateAndZeroMemory(biasDimCount * sizeof(vx_uint32));

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

    weights_bias= vxoWeightsBiases_Create(context,
                                          layer_type,
                                          num_of_dims,
                                          inputs_dims,
                                          pad_x_left,
                                          pad_x_right,
                                          pad_y_top,
                                          pad_y_bottom,
                                          pooling_size_x,
                                          pooling_size_y,
                                          down_scale_size_rounding,
                                          convolution_outputs_dims,
                                          weightDimCount,
                                          weightDims,
                                          weightType,
                                          weightPos,
                                          biasDimCount,
                                          biasDims,
                                          biasType,
                                          biasPos);

    if (weights_bias == VX_NULL)
    {
        status = VX_FAILURE;
        goto exit;
    }

    weights_bias->weightFixedPointPos = weights->tensorBuffer->fixedPointPos;

    sliceCount = weightDims[2] * weights_bias->stride * weights_bias->stride;
    weightCount = weights_bias->weights_sizes[0] * weights_bias->weights_sizes[1];
    filterTotalCount = weights_bias->weights_sizes[3];
    filterSize = weightCount * weightSize * sliceCount;

    if (pool_outputs_dims != VX_NULL)
        poolingStride = convolution_outputs_dims[0] / pool_outputs_dims[0];
    else
        poolingStride = 1;

    weights_bias->pooling_stride = poolingStride;

    /* Get weights & bias base memory */
    vxoTensor_GetTensorBaseMemory(weights, &weightBase, VX_NULL);
    if (biases)
        vxoTensor_GetTensorBaseMemory(biases, &biasBase, VX_NULL);
    if (weights->isViewed)
    {
        startWeightDataPtr = (vx_uint8*)weightBase + filterSize * weightViewStarts[3];
        if (biases)
            startBiasDataPtr = (vx_uint32*)biasBase + weightViewStarts[3];
    }
    else
    {
        startWeightDataPtr = (vx_uint8*)weightBase;
        startBiasDataPtr = (vx_uint32*)biasBase;
    }

    /* If need reshuffle? */
    if (weights_bias->stride > 1)
    {
        /* do reshuffle*/
        vx_uint32 alignWeightWidth = vxnneAlignWithStride(weightDims[0], weights_bias->stride);
        vx_uint32 alignWeightHeight = vxnneAlignWithStride(weightDims[1], weights_bias->stride);
        vx_uint32 depth = weightDims[2];
        vx_nn_reshuffle_s src = {NULL, alignWeightWidth, alignWeightHeight, depth, weightDims[3] * 1, (vx_type_e)TENSOR_DATA_TYPE(weights)};
        vx_nn_reshuffle_s dst = {NULL, weights_bias->weights_sizes[0], weights_bias->weights_sizes[1], weights_bias->weights_sizes[2], weights_bias->weights_sizes[3]*1, (vx_type_e)weights_bias->weights_data_format};
        vx_uint32 x, y, z, w;
        vx_uint32 orgXSize = weightDims[0], orgYSize = weightDims[1], orgZSize = depth;
        vx_uint32 weightItemCount = weights_bias->weights_sizes[0] * weights_bias->weights_sizes[1] * sliceCount * filterTotalCount;

#if VX_C_MEMORY_MANAGE
        vxoMemory_CAllocate(context, &convertedWeightData, weightItemCount * weightSize);
#else
        convertedWeightData = (void*)malloc(weightItemCount * weightSize);
#endif
        /* Allocate temp buffer for weight data */
        vxoMemory_CAllocate(context, &weightData, weightItemCount * weightSize);
        gcmASSERT(weightData);
        gcmASSERT(convertedWeightData);

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
                        vx_uint32 zero = 0;
                        vx_uint8 *converted, *orig;

                        converted = (vx_uint8*)convertedWeightData + fpOffsetSize;

                        if ((x > orgXSize - 1) || (y > orgYSize - 1))
                        {
                            _DataGeneralConvert((void*)&zero, (void*)converted, weightSize, weightSize);
                        }
                        else
                        {
                            orig = (vx_uint8*)startWeightDataPtr + orgOffsetSize;
                            _DataGeneralConvert((void*)orig, (void*)converted, weightSize, weightSize);
                        }
                    }
                }
            }
        }

        /*reshuffle kernel data*/
        src.data   = convertedWeightData;
        dst.data   = weightData;

        reshuffleData(&src, weights_bias->stride, weights_bias->stride, &dst);
        /* re-wrap weight buffer */
        startWeightDataPtr = (vx_uint8*)weightData;

#if VX_C_MEMORY_MANAGE
        vxoMemory_CFree(context, &convertedWeightData);
#else
        free(convertedWeightData);
#endif
        convertedWeightData = VX_NULL;
    }

    weights_bias->use_fc_accel = vx_false_e;

    if (layer_type == VX_CONVOLUTIONAL_NETWORK_FULLYCONNECTED_LAYER && weightDims[0] == 1 && weightDims[1] == 1 &&
        context->nnConfig.tpCoreCount && context->options.enableTP && context->options.flagTPFunc[TP_SINGLE_FC])
    {
        vx_uint8 *minFCZeroRunLenBitWidth;

        vx_uint32 i = 0, j, max;
        vx_uint32 filterCount = weights_bias->weights_sizes[3];
        vx_uint32 coreCount = context->nnConfig.tpCoreCount;

        /* TP FC can handle up to 512 filters. */
        if (context->options.enableMultiTP && context->nnConfig.tpCoreCount > 1)
        {
            max = gcmALIGN(filterCount, coreCount) / coreCount;
            for (;;)
            {
                if (max >= TP_FC_Z_MAX && filterCount >= TP_FC_Z_MAX)
                {
                    if (i < MAX_ZGROUP_COUNT)
                        weights_bias->zgroup_array[i] = TP_FC_Z_MAX;
                    filterCount -= TP_FC_Z_MAX;
                    i++;
                }
                else if (filterCount)
                {
                    max = (vx_uint32)((vx_float32)filterCount / coreCount + 0.5f);
                    coreCount = gcmMIN(coreCount, filterCount);
                    for (j = 0; j < coreCount; j++)
                    {
                        if (i < MAX_ZGROUP_COUNT)
                            weights_bias->zgroup_array[i] = j == coreCount-1 ? filterCount : max;
                        filterCount -= max;
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
                        weights_bias->zgroup_array[i] = TP_FC_Z_MAX;
                    filterCount -= TP_FC_Z_MAX;
                    i++;
                }
                else
                {
                    if (i < MAX_ZGROUP_COUNT)
                        weights_bias->zgroup_array[i] = filterCount;
                    i++;
                    break;
                }
            }
        }

        if (i > MAX_ZGROUP_COUNT)
            goto exit;

        weights_bias->zgroup_num = i;
        weights_bias->use_tp_fc = vx_true_e;

        gcmASSERT(weights_bias->weights_sizes[0] == 1);
        gcmASSERT(weights_bias->weights_sizes[1] == 1);
        sliceCount = weights_bias->weights_sizes[2];
        filterSize = sliceCount * weightSize;
        minFCZeroRunLenBitWidth = (vx_uint8 *) vxAllocate(weights_bias->zgroup_num * sliceCount);
        if (minFCZeroRunLenBitWidth == NULL)
        {
            status = VX_ERROR_NO_MEMORY;
            goto exit;
        }

        for (i = 0; i < weights_bias->zgroup_num; i++)
        {
            filterTotalCount = weights_bias->zgroup_array[i];

            /* Calculate kernel stream size for best kernesPerCore */
            calculateWeightBiasTPBufferRelatedSize(
                weights_bias,
                setZeroLength,
                sliceCount,
                filterTotalCount,
                i,
                startWeightDataPtr + weightDataBytesOffset,
                &minKernelBufferSize[i],
                minFCZeroRunLenBitWidth + i * sliceCount);

            minTotalKernelBufferSize += minKernelBufferSize[i];
            weightDataBytesOffset += filterSize * filterTotalCount;
        }

        /* Allocate weight_bias buffer only once */
        if (!WeightBiasBufferAllocate(context, weights_bias, minTotalKernelBufferSize + weights_bias->memory_pad * weights_bias->zgroup_num, vx_false_e))
        {
            status = VX_ERROR_NO_MEMORY;

            if (minFCZeroRunLenBitWidth)
                vxFree(minFCZeroRunLenBitWidth);
            minFCZeroRunLenBitWidth = VX_NULL;

            goto exit;
        }
        vxAcquireMutex(weights_bias->memory.writeLocks[0]);
        locked = vx_true_e;

        weightDataBytesOffset = 0;
        for (i = 0; i < weights_bias->zgroup_num; i++)
        {
            filterTotalCount = weights_bias->zgroup_array[i];
            kernelBufferBasePtr = weights_bias->memory.logicals[0] + compressDataBytesOffset;

            fillinTPKernelBuffer(
                weights_bias,
                minFCZeroRunLenBitWidth + i * sliceCount,
                sliceCount,
                filterTotalCount,
                kernelBufferBasePtr,
                startWeightDataPtr + weightDataBytesOffset,
                startBiasDataPtr + biasDataDWordOffset,
                i);

            calculateFilterPerCore(context, weights_bias, i, vx_true_e, layer_type);

#if TP_FC_DUMP_WEIGHTS_BIASES
            {
                char fileName[32];
                FILE *fp;
                sprintf(fileName, "tp_wb_%d.txt", i);
                fp = fopen(fileName, "w");

                if (weights_bias->weights_data_format == VX_TYPE_INT8)
                {
                    vx_int8 * weightData = (vx_int8 *) (startWeightDataPtr + weightDataBytesOffset);
                    vx_int32 * biasData = (vx_int32 *) (startBiasDataPtr + biasDataDWordOffset);
                    vx_uint32 kz, z;

                    for (z = 0; z < filterTotalCount; z++)
                    {
                        for (kz = 0; kz < sliceCount; kz++)
                        {
                            fprintf(fp, "kz=%4d z=%4d coef=%d\n", kz, z, weightData);
                            weightData++;
                        }
                    }

                    for (z = 0; z < filterTotalCount; z++)
                    {
                        fprintf(fp, "z=%4d bias=%d\n", z, *biasData);
                        biasData++;
                    }
                }
                else
                {
                    vx_uint16 * weightData = (vx_uint16 *) (startWeightDataPtr + weightDataBytesOffset);
                    vx_uint32 * biasData = (vx_uint32 *) (startBiasDataPtr + biasDataDWordOffset);
                    vx_uint32 kz, z;

                    for (z = 0; z < filterTotalCount; z++)
                    {
                        for (kz = 0; kz < sliceCount; kz++)
                        {
                            vx_float32 fcoef = Fp16toFp32(*weightData);

                            fprintf(fp, "kz=%4d z=%4d coef=0x%04x (%f)\n", kz, z, *weightData, fcoef);
                            weightData++;
                        }
                    }

                    for (z = 0; z < filterTotalCount; z++)
                    {
                        fprintf(fp, "z=%4d bias=0x%08x (%f)\n", z, *biasData, *((vx_float32*)biasData));
                        biasData++;
                    }
                }
                fclose(fp);
            }
#endif

            weightDataBytesOffset += filterSize * filterTotalCount;
            biasDataDWordOffset += filterTotalCount;
            weights_bias->memory_offset_array[i] = compressDataBytesOffset;
            weights_bias->memory_sizes_array[i] = minKernelBufferSize[i] + weights_bias->memory_pad;
            compressDataBytesOffset += minKernelBufferSize[i] + weights_bias->memory_pad;
        }
        vxFree(minFCZeroRunLenBitWidth);
    }
    else
    {
        vx_bool calcTPPerf = context->nnConfig.tpCoreCount &&
                             context->options.enableTP &&
                             layer_type != VX_CONVOLUTIONAL_NETWORK_FULLYCONNECTED_LAYER &&
                             weights_bias->stride > 1;

        /* Estimate kernel stream size first */
        calculateWeightBiasStreamRelatedSize(
            weights_bias, weights, weightDims[3], sliceCount, -1, weightData, &minKernelBufferSize[0], VX_NULL, VX_NULL, setZeroLength);

        /* Enable or disable FC ACCEL function */
        if (layer_type == VX_CONVOLUTIONAL_NETWORK_FULLYCONNECTED_LAYER)
        {
            vx_float32 nonZeroRatio = (vx_float32)(weights_bias->all_count[0] - weights_bias->zero_count[0]) / weights_bias->all_count[0];
            useFCAccel = context->options.enableNNFCAccel == 1 ? vx_true_e : vx_false_e;
            if (nonZeroRatio > 0.4)
            {
                fcThreshold = context->options.nnFCAccelThreshold;
            }
            else
            {
                useFCAccel = vx_false_e;
            }
        }

        if (useFCAccel &&
            layer_type == VX_CONVOLUTIONAL_NETWORK_FULLYCONNECTED_LAYER &&
            !(weights_bias->weights_sizes[3] & 0x7) &&
            weights_bias->weights_sizes[2] * weights_bias->weights_sizes[3] > fcThreshold)
        {
            vx_uint32 x, y, newWBSize, weightDepth, biasPad;
            vx_uint8 *wbPtr, *wbTmpPtr, *weightPtr, *weightTmpPtr;

            if (!vxoNNExternsionAdjustWeightsBiases(weights_bias, vx_false_e, weights->isViewed, minKernelBufferSize[0]))
            {

                status = VX_FAILURE;

                goto exit;

            }

            biasPad = weights_bias->org_weights_sizes[2] < context->options.fcZMax ? 1 : weights_bias->weights_sizes[1];

            weights_bias->tmp_fcaccel_input_ptr = vxAllocateAndZeroMemory((weights_bias->org_weights_sizes[2] + biasPad) * weightSize);
            if (weights_bias->tmp_fcaccel_input_ptr == VX_NULL)
            {
                status = VX_ERROR_NO_MEMORY;
                goto exit;
            }

            /* Calculate filters per core */
            calculateFilterPerCore(context, weights_bias, 0, calcTPPerf, layer_type);

            weightDepth = weights_bias->org_weights_sizes[2];
            newWBSize = (weightDepth + biasPad) * filterTotalCount * weightSize;

            /* Allocate weight_bias buffer only once */
            if (!WeightBiasBufferAllocate(context, weights_bias, newWBSize, vx_false_e))
            {
                status = VX_ERROR_NO_MEMORY;
                goto exit;
            }

            weights_bias->tmp_fcaccel_wb_ptr = vxAllocateAndZeroMemory(newWBSize);
            if (weights_bias->tmp_fcaccel_wb_ptr == VX_NULL)
            {
                status = VX_ERROR_NO_MEMORY;
                goto exit;
            }

            wbTmpPtr = wbPtr = (vx_uint8*) weights_bias->tmp_fcaccel_wb_ptr;
            weightTmpPtr = weightPtr = (vx_uint8*) startWeightDataPtr;

            for (y = 0; y < weightDepth; y++)
            {
                for (x = 0; x < filterTotalCount; x++, wbTmpPtr+=weightSize)
                {
                    weightTmpPtr  = weightPtr + (x * weightDepth + y) * weightSize;
                    memcpy(wbTmpPtr, weightTmpPtr, weightSize);
                }
            }

            wbPtr += weightDepth * filterTotalCount * weightSize;
            for (x = 0; x < filterTotalCount; x++, wbPtr+=weightSize)
            {
                vx_float32 f = vxnneGetData((vx_type_e)biasType, x, (vx_uint8_ptr)startBiasDataPtr, biases->tensorBuffer->fixedPointPos);
                vxnneSaveData((vx_type_e)weightType, 0, f, wbPtr, biases->tensorBuffer->fixedPointPos, biases->tensorBuffer->roundingMode);
            }

            weights_bias->use_fc_accel = vx_true_e;
        }
        else
        {
            if (!vxoNNExternsionAdjustWeightsBiases(weights_bias, vx_true_e, weights->isViewed, minKernelBufferSize[0]))
            {

                status = VX_FAILURE;

                goto exit;

            }

            sliceCount = weights_bias->weights_sizes[2];
            weightCount = weights_bias->weights_sizes[0] * weights_bias->weights_sizes[1];
            filterSize = weightCount * sliceCount * weightSize;

            for (i = 0; i < weights_bias->zgroup_num; i++)
            {
                /* Calculate filters per core */
                calculateFilterPerCore(context, weights_bias, i, calcTPPerf, layer_type);

                /* Calculate kernel stream size for best kernesPerCore */
                calculateWeightBiasStreamRelatedSize(
                    weights_bias, weights, weights_bias->kernelsPerCore[0][i], sliceCount, i, weightData, &minKernelBufferSize[i], &minZeroRunLen[i], &maxZeroRun[i], setZeroLength);

                minTotalKernelBufferSize += minKernelBufferSize[i];
            }

            /* Allocate weight_bias buffer only once */
            if (!WeightBiasBufferAllocate(context, weights_bias, minTotalKernelBufferSize + weights_bias->memory_pad * weights_bias->zgroup_num, vx_false_e))
            {
                status = VX_ERROR_NO_MEMORY;
                goto exit;
            }
            vxAcquireMutex(weights_bias->memory.writeLocks[0]);
            locked = vx_true_e;

            for (i = 0; i < weights_bias->zgroup_num; i++)
            {
                filterTotalCount = weights_bias->zgroup_array[i];
                kernelBufferBasePtr = weights_bias->memory.logicals[0] + compressDataBytesOffset;

                fillinKernelBuffer(
                    weights_bias,
                    minZeroRunLen[i],
                    maxZeroRun[i],
                    weights_bias->weights_sizes[0],
                    weights_bias->weights_sizes[1],
                    sliceCount,
                    filterTotalCount,
                    weights_bias->kernelsPerCore[0][i],
                    convolution_outputs_dims[0] / poolingStride,
                    convolution_outputs_dims[1] / poolingStride,
                    kernelBufferBasePtr,
                    startWeightDataPtr + weightDataBytesOffset,
                    outputSize,
                    startBiasDataPtr + biasDataDWordOffset
                    );

                weightDataBytesOffset += filterSize * filterTotalCount;
                if (biases)
                    biasDataDWordOffset += filterTotalCount;
                weights_bias->memory_offset_array[i] = compressDataBytesOffset;
                compressDataBytesOffset += minKernelBufferSize[i] + weights_bias->memory_pad;
                weights_bias->memory_sizes_array[i] = minKernelBufferSize[i] + weights_bias->memory_pad;
            }
        }
    }

exit:
    if (locked) vxReleaseMutex(weights_bias->memory.writeLocks[0]);

    /* Free temp weight data buffer */
    if (convertedWeightData != VX_NULL)
    {
#if VX_C_MEMORY_MANAGE
        vxoMemory_CFree(context, &convertedWeightData);
#else
        free(convertedWeightData);
#endif
        convertedWeightData = VX_NULL;
    }
    if (weightData != VX_NULL)
        vxoMemory_CFree(context, &weightData);
    if (weightDims != VX_NULL)
        vxFree(weightDims);
    if (biasDims != VX_NULL)
        vxFree(biasDims);

    if (weightViewStarts != VX_NULL)
        vxFree(weightViewStarts);
    if (weightViewEnds != VX_NULL)
        vxFree(weightViewEnds);
    if (biasViewStarts != VX_NULL)
        vxFree(biasViewStarts);
    if (biasViewEnds != VX_NULL)
        vxFree(biasViewEnds);

    return status == VX_SUCCESS ? weights_bias : VX_NULL;
}

vx_weights_biases_parameter vxoWeightsBiases_Create(
    vx_context  context,
    vx_enum     layer_type,
    vx_uint32   num_of_dims,
    vx_uint32 * inputs_dims,
    vx_uint32   pad_x_left,
    vx_uint32   pad_x_right,
    vx_uint32   pad_y_top,
    vx_uint32   pad_y_bottom,
    vx_uint32   pooling_size_x,
    vx_uint32   pooling_size_y,
    vx_enum     down_scale_size_rounding,
    vx_uint32 * outputs_dims,
    vx_uint32   weights_num_of_dims,
    vx_uint32 * weights_dims,
    vx_enum     weights_data_format,
    vx_uint8    weights_fixed_point_pos,
    vx_uint32   biases_num_of_dims,
    vx_uint32 * biases_dims,
    vx_enum     biases_data_format,
    vx_uint8    biases_fixed_point_pos
    )
{
    vx_weights_biases_parameter weights_bias;
    vx_uint32 stride;
    vx_uint32 alignedWidth;
    vx_uint32 alignedHeight;
    vx_status status = VX_SUCCESS;

    vxmASSERT(context);

    weights_bias = (vx_weights_biases_parameter)vxoReference_Create(context, VX_TYPE_WEIGHTS_BIASES_PARAMETER, VX_REF_EXTERNAL, &context->base);

    if (vxoReference_GetStatus((vx_reference)weights_bias) != VX_SUCCESS) return VX_NULL;

    weights_bias->inout_num_of_dims = num_of_dims;
    weights_bias->weights_num_of_dims = weights_num_of_dims;
    weights_bias->weights_data_format = weights_data_format;
    weights_bias->weights_fixed_point_pos = weights_fixed_point_pos;
    weights_bias->biases_num_of_dims = biases_num_of_dims;
    weights_bias->biases_data_format = biases_data_format;
    weights_bias->biases_fixed_point_pos = biases_fixed_point_pos;
    weights_bias->pooling_size_x = pooling_size_x;
    weights_bias->pooling_size_y = pooling_size_y;
    weights_bias->pad_x_left = pad_x_left;
    weights_bias->pad_x_right = pad_x_right;
    weights_bias->pad_y_top = pad_y_top;
    weights_bias->pad_y_bottom = pad_y_bottom;
    weights_bias->down_scale_size_rounding = down_scale_size_rounding;
    weights_bias->layer_type = layer_type;

    if (weights_bias->weights_sizes == VX_NULL)
        weights_bias->weights_sizes = (vx_uint32 *)vxAllocateAndZeroMemory(weights_num_of_dims * sizeof(vx_uint32));
    if (weights_bias->org_weights_sizes == VX_NULL)
        weights_bias->org_weights_sizes = (vx_uint32 *)vxAllocateAndZeroMemory(weights_num_of_dims * sizeof(vx_uint32));
    if (weights_bias->biases_sizes == VX_NULL)
    {
        if (biases_num_of_dims != 0)
            weights_bias->biases_sizes = (vx_uint32 *)vxAllocateAndZeroMemory(biases_num_of_dims * sizeof(vx_uint32));
        else
            weights_bias->biases_sizes = (vx_uint32 *)vxAllocateAndZeroMemory(VX_CONTEXT_TENSOR_MAX_DIMENSION * sizeof(vx_uint32));
    }
    if (weights_bias->input_sizes == VX_NULL)
        weights_bias->input_sizes = (vx_uint32 *)vxAllocateAndZeroMemory(num_of_dims * sizeof(vx_uint32));
    if (weights_bias->output_sizes == VX_NULL)
        weights_bias->output_sizes = (vx_uint32 *)vxAllocateAndZeroMemory(num_of_dims * sizeof(vx_uint32));

    if (weights_bias->weights_sizes == VX_NULL || weights_bias->org_weights_sizes == VX_NULL || weights_bias->biases_sizes == VX_NULL ||
        weights_bias->input_sizes == VX_NULL || weights_bias->output_sizes == VX_NULL)
    {
        status = VX_ERROR_NO_MEMORY;
        goto exit;
    }

    vxMemCopy(weights_bias->org_weights_sizes, weights_dims, weights_num_of_dims * sizeof(vx_uint32));
    if (biases_dims)
        vxMemCopy(weights_bias->biases_sizes, biases_dims, biases_num_of_dims * sizeof(vx_uint32));
    vxMemCopy(weights_bias->input_sizes, inputs_dims, num_of_dims * sizeof(vx_uint32));
    vxMemCopy(weights_bias->output_sizes, outputs_dims, num_of_dims * sizeof(vx_uint32));
    weights_bias->weights_sizes[3] = weights_dims[3];

    if ((layer_type == VX_CONVOLUTIONAL_NETWORK_FULLYCONNECTED_LAYER) || ((outputs_dims[0] - 1) == 0))
    {
        /* it is fully connected layer */
        stride = 1;
    }
    else
    {
        /* Calculate stride = (w + pad_x_left + pad_x_right - weight)/(output_w - 1) */
        stride = vxoNNExternsionConvlutionRound((vx_float32)(inputs_dims[0] + pad_x_left + pad_x_right - weights_dims[0]) / (outputs_dims[0] - 1), down_scale_size_rounding);
    }

    weights_bias->stride = stride;

    /*Calculate weight_bias' weight width & height */
    alignedWidth = ((weights_dims[0] % stride == 0) ?
                                       weights_dims[0] : (weights_dims[0] + (stride - weights_dims[0] % stride)));
    alignedHeight = ((weights_dims[1] % stride == 0) ?
                                       weights_dims[1] : (weights_dims[1] + (stride - weights_dims[1] % stride)));
    weights_bias->weights_sizes[0] = alignedWidth / stride;
    weights_bias->weights_sizes[1] = alignedHeight / stride;
    weights_bias->weights_sizes[2] = weights_dims[2] * stride * stride;

    weights_bias->memory_pad = 64;
    /* weight bias file has marks in its head which is aligned to 64 bytes.
     * ---|0x0A0B0C0D|memory_pad|zgroup_num|memory_offset_array[]|zero_count[]|all_cout[]|orig_size[]|compressed_size[]|tile_x[]|tile_y[]|ker_per_core[]|---
     */
    weights_bias->memroy_head_offset = gcmALIGN((3 + 8 * MAX_ZGROUP_COUNT) * sizeof(vx_uint32), 64);

exit:
    if (status == VX_SUCCESS)
        return weights_bias;
    else
    {
        gcmPRINT("vxoWeightsBiases_Create: OUT OF MEMORY\n");
        vxReleaseWeightsBiasesParameter(&weights_bias);
        return VX_NULL;
    }
}

vx_weights_biases_parameter vxoWeightsBiasesFromStream_Create(
    vx_context  context,
    vx_enum     layer_type,
    vx_uint32   num_of_dims,
    vx_uint32 * inputs_dims,
    vx_uint32   pad_x_left,
    vx_uint32   pad_x_right,
    vx_uint32   pad_y_top,
    vx_uint32   pad_y_bottom,
    vx_uint32   pooling_size_x,
    vx_uint32   pooling_size_y,
    vx_enum     down_scale_size_rounding,
    vx_uint32 * outputs_dims,
    vx_uint32   weights_num_of_dims,
    vx_uint32 * weights_dims,
    vx_uint32 * weights_org_dims,
    vx_enum     weights_data_format,
    vx_uint8    weights_fixed_point_pos,
    vx_uint32   biases_num_of_dims,
    vx_uint32 * biases_dims,
    vx_enum     biases_data_format,
    vx_uint8    biases_fixed_point_pos
    )
{
    vx_weights_biases_parameter weights_bias;
    vx_uint32 stride;

    vxmASSERT(context);

    weights_bias = (vx_weights_biases_parameter)vxoReference_Create(context, VX_TYPE_WEIGHTS_BIASES_PARAMETER, VX_REF_EXTERNAL, &context->base);

    if (vxoReference_GetStatus((vx_reference)weights_bias) != VX_SUCCESS) return weights_bias;

    weights_bias->inout_num_of_dims = num_of_dims;
    weights_bias->weights_num_of_dims = weights_num_of_dims;
    weights_bias->weights_data_format = weights_data_format;
    weights_bias->weights_fixed_point_pos = weights_fixed_point_pos;
    weights_bias->biases_num_of_dims = biases_num_of_dims;
    weights_bias->biases_data_format = biases_data_format;
    weights_bias->biases_fixed_point_pos = biases_fixed_point_pos;
    weights_bias->pooling_size_x = pooling_size_x;
    weights_bias->pooling_size_y = pooling_size_y;
    weights_bias->pad_x_left = pad_x_left;
    weights_bias->pad_x_right = pad_x_right;
    weights_bias->pad_y_top = pad_y_top;
    weights_bias->pad_y_bottom = pad_y_bottom;
    weights_bias->down_scale_size_rounding = down_scale_size_rounding;
    weights_bias->layer_type = layer_type;

    if (weights_bias->weights_sizes == VX_NULL)
        weights_bias->weights_sizes = (vx_uint32 *)vxAllocateAndZeroMemory(weights_num_of_dims * sizeof(vx_uint32));
    if (weights_bias->org_weights_sizes == VX_NULL)
        weights_bias->org_weights_sizes = (vx_uint32 *)vxAllocateAndZeroMemory(weights_num_of_dims * sizeof(vx_uint32));
    if (weights_bias->biases_sizes == VX_NULL)
        weights_bias->biases_sizes = (vx_uint32 *)vxAllocateAndZeroMemory(biases_num_of_dims * sizeof(vx_uint32));
    if (weights_bias->input_sizes == VX_NULL)
        weights_bias->input_sizes = (vx_uint32 *)vxAllocateAndZeroMemory(num_of_dims * sizeof(vx_uint32));
    if (weights_bias->output_sizes == VX_NULL)
        weights_bias->output_sizes = (vx_uint32 *)vxAllocateAndZeroMemory(num_of_dims * sizeof(vx_uint32));

    vxMemCopy(weights_bias->org_weights_sizes, weights_org_dims, weights_num_of_dims * sizeof(vx_uint32));
    vxMemCopy(weights_bias->biases_sizes, biases_dims, biases_num_of_dims * sizeof(vx_uint32));
    vxMemCopy(weights_bias->input_sizes, inputs_dims, num_of_dims * sizeof(vx_uint32));
    vxMemCopy(weights_bias->output_sizes, outputs_dims, num_of_dims * sizeof(vx_uint32));
    vxMemCopy(weights_bias->weights_sizes, weights_dims, weights_num_of_dims * sizeof(vx_uint32));

    if ((layer_type == VX_CONVOLUTIONAL_NETWORK_FULLYCONNECTED_LAYER) || ((outputs_dims[0] - 1) == 0))
    {
        /* it is fully connected layer */
        stride = 1;
    }
    else
    {
        /* Calculate stride = (w + 2*pad - weight)/(output_w - 1) */
        stride = vxoNNExternsionConvlutionRound((vx_float32)(inputs_dims[0] + pad_x_left + pad_x_right - weights_dims[0]) / (outputs_dims[0] - 1), down_scale_size_rounding);
    }

    weights_bias->stride = stride;

    weights_bias->memory_pad = 64;
    /* weight bias file has marks in its head which is aligned to 64 bytes.
     * ---|0x0A0B0C0D|memory_pad|zgroup_num|memory_offset_array[]|zero_count[]|all_cout[]|orig_size[]|compressed_size[]|tile_x[]|tile_y[]|ker_per_core[]|---
     */
    weights_bias->memroy_head_offset = gcmALIGN((3 + 8 * MAX_ZGROUP_COUNT) * sizeof(vx_uint32), 64);

    return weights_bias;
}

void fillinKernelBuffer(
    vx_weights_biases_parameter wb,
    vx_uint8 min_zero_run_len,
    vx_uint32 max_zero_run,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 weight_z,
    vx_uint32 filter_count,
    vx_uint32 kernels_per_core,
    vx_uint32 output_final_x,
    vx_uint32 output_final_y,
    vx_uint8_ptr wb_base_ptr,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32 outputSize,
    vx_uint32* bias_base_ptr
    )
{
    vx_context context           = vxGetContext((vx_reference)wb);
    vx_uint32 nnCoreCount        = context->nnConfig.nnCoreCount;

    vx_uint32 coreIndex;
    vx_uint32 groupIndex, filterIndex, sliceIndex;
    vx_uint32 weightXIndex, weightYIndex;
    vx_uint32 bitOffset          = 0;
    vx_uint32 zeroRun            = 0;

    vx_uint32 weightSize         = (wb->weights_data_format == VX_TYPE_INT8) ? (vx_uint32)vxDataType_GetSize(VX_TYPE_INT8) : (vx_uint32)vxDataType_GetSize(VX_TYPE_INT16);
    vx_uint32 weightBitSize      = weightSize * 8;
    vx_uint32 biasSize           = (vx_uint32)vxDataType_GetSize(VX_TYPE_FLOAT32);
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

    if (wb->weights_data_format == VX_TYPE_INT8)
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

    /* kernel Buffer size for each core */
    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        writeBits(&kernelBufferPtr, &bitOffset, kernelStreamSize, 32);
    }
    packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);

    for (coreIndex = 0; coreIndex < nnCoreCount; coreIndex++)
    {
        vx_uint32 coreFilterCount = (coreIndex < oddFilterPerCore) ? totalFilterPerCore + 1 : totalFilterPerCore;

        kernelStreamBasePtr = (vx_uint8_ptr)kernelBufferPtr;

        /* zeroRunLen */
        writeBits(&kernelBufferPtr, &bitOffset, min_zero_run_len, 8);

        writeBits(&kernelBufferPtr, &bitOffset, coreFilterCount, 16);

        /* fill weight value and bias for every group */
        zeroRun = 0;
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
                    }

                    for (weightYIndex = 0; weightYIndex < weight_y; weightYIndex++)
                    {
                        for (weightXIndex = 0; weightXIndex < weight_x; weightXIndex++)
                        {
                            vx_uint32 weight = 0;

                            if (wb->weights_data_format == VX_TYPE_INT8)
                                weight = *((vx_int8 *)kernelDataPtr);
                            else
                                weight = *((vx_uint16 *)kernelDataPtr);
                            kernelDataPtr = kernelDataPtr + weightSize;

                            if ((zeroRun == max_zero_run)
                                || ((sliceIndex == 0)
                                    && (weightXIndex == weight_x - 1)
                                    && (weightYIndex == weight_y - 1)
                                    && needToWriteBias)
                                || ((weightXIndex == weight_x - 1)
                                    && (weightYIndex == weight_y - 1)
                                    && (sliceIndex == sliceCount - 1))
                                || (weight != 0)
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
                        if (outputSize > 0)
                            offsetValue = output_final_x * output_final_y * outputSize * filterIndex;
                        else
                            offsetValue = output_final_x * output_final_y * weightSize * filterIndex;
                        if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
                            writeBits(&kernelBufferPtr, &bitOffset, offsetValue, NN_Z_POSITION_OFFSET_BITS_VIP_V7);
                        else
                            writeBits(&kernelBufferPtr, &bitOffset, offsetValue, NN_Z_POSITION_OFFSET_BITS);

                    }
                }
            }
        }

        /* pad 0 */
        packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);

        /* Go back to update kernelStreamSize. */
        kernelStreamSize = (vx_uint32)((vx_uint8 *)kernelBufferPtr - kernelStreamBasePtr);
        writeBits(&kernelStreamSizePtr, &streamSizeBitOffset, kernelStreamSize, 32);
        wb->kernelStreamSize += kernelStreamSize;
    }
}

void fillinTPKernelBuffer(
    vx_weights_biases_parameter wb,
    vx_uint8* zero_run_len_bit_width,
    vx_uint32 weight_z,
    vx_uint32 filter_count,
    vx_uint8_ptr wb_base_ptr,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr,
    vx_uint32 index
    )
{
    vx_uint32 zeroRun               = 0;
    vx_uint32 weightSize            = (wb->weights_data_format == VX_TYPE_INT8) ? (vx_uint32)vxDataType_GetSize(VX_TYPE_INT8) : (vx_uint32)vxDataType_GetSize(VX_TYPE_INT16);
    vx_uint32 weightBitSize         = weightSize * 8;
    vx_uint32 biasBitSize           = (vx_uint32)vxDataType_GetSize(VX_TYPE_FLOAT32) * 8;
    vx_uint32 biasData              = 0;
    vx_uint32 sliceCount            = weight_z;
    vx_uint32 filterSize            = sliceCount * weightSize;
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
    vx_uint32 zeroCount = 0;

    if (wb->weights_data_format == VX_TYPE_INT8)
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
        biasData = *(bias_base_ptr + filterIndex);
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

            if (wb->weights_data_format == VX_TYPE_INT8)
                weight = *((vx_int8 *)kernelDataPtr);
            else
                weight = *((vx_uint16 *)kernelDataPtr);
            kernelDataPtr += filterSize;

            if ((zeroRun == maxZeroRun)
                || (weight != 0)
                || (filterIndex == (filterCount - 1)))
            {
                if (zeroRunLenBitWidth)
                {
                    writeBits(&kernelBufferPtr, &bitOffset, zeroRun, zeroRunLenBitWidth);
                }
                writeBits(&kernelBufferPtr, &bitOffset, weight, weightBitSize);
                zeroRun = 0;
            }
            else
            {
                zeroRun++;
                if (!weight) zeroCount++;
            }
        }
        packZeros(&kernelBufferPtr, &bitOffset, alignedOffset);

        /* Go back to update kernelStreamSize. */
        kernelStreamSize = (vx_uint32)((vx_uint8 *)kernelBufferPtr - kernelStreamBasePtr);
        gcmASSERT((kernelStreamSize & 0x3F) == 0);
        gcmASSERT(kernelStreamSize < 2048);
        writeBits(&kernelStreamSizePtr, &streamSizeBitOffset, kernelStreamSize >> 6, 5);
    }

    wb->all_count[index]  = sliceCount * filterCount;
    wb->zero_count[index] = zeroCount;
}

vx_status vxoWeightsBiasesParameter_ProcessHead(
    vx_weights_biases_parameter     weights_bias,
    vx_enum                         usage
    )
{
    vx_uint32 i;
    vx_uint32* buff = (vx_uint32 *)(weights_bias->memory.logicals[0] - weights_bias->memroy_head_offset);

    if (VX_READ_ONLY == usage || VX_READ_AND_WRITE == usage)
    {
        *buff++ = 0x0A0B0C0D;
        *buff++ = weights_bias->memory_pad;
        *buff++ = weights_bias->zgroup_num;
        memcpy(buff++, weights_bias->memory_offset_array, sizeof(vx_size) * weights_bias->zgroup_num);
        memcpy(buff++, weights_bias->zero_count, sizeof(vx_uint32) * weights_bias->zgroup_num);
        memcpy(buff++, weights_bias->all_count, sizeof(vx_uint32) * weights_bias->zgroup_num);
        memcpy(buff++, weights_bias->orig_size, sizeof(vx_uint32) * weights_bias->zgroup_num);
        memcpy(buff++, weights_bias->compressed_size, sizeof(vx_uint32) * weights_bias->zgroup_num);
        memcpy(buff++, weights_bias->outImageTileXSize, sizeof(vx_uint32) * weights_bias->zgroup_num);
        memcpy(buff++, weights_bias->outImageTileYSize, sizeof(vx_uint32) * weights_bias->zgroup_num);
        memcpy(buff++, weights_bias->kernelsPerCore, sizeof(vx_uint32) * weights_bias->zgroup_num);
    }
    else if (VX_WRITE_ONLY == usage || VX_READ_AND_WRITE == usage)
    {
        if (*buff++ != 0x0A0B0C0D)
            return VX_ERROR_INVALID_VALUE;

        weights_bias->memory_pad = *buff++;
        weights_bias->zgroup_num = *buff++;
        memcpy(weights_bias->memory_offset_array, buff++, sizeof(vx_size) * weights_bias->zgroup_num);
        memcpy(weights_bias->zero_count, buff++, sizeof(vx_uint32) * weights_bias->zgroup_num);
        memcpy(weights_bias->all_count, buff++, sizeof(vx_uint32) * weights_bias->zgroup_num);
        memcpy(weights_bias->orig_size, buff++, sizeof(vx_uint32) * weights_bias->zgroup_num);
        memcpy(weights_bias->compressed_size, buff++, sizeof(vx_uint32) * weights_bias->zgroup_num);
        memcpy(weights_bias->outImageTileXSize, buff++, sizeof(vx_uint32) * weights_bias->zgroup_num);
        memcpy(weights_bias->outImageTileYSize, buff++, sizeof(vx_uint32) * weights_bias->zgroup_num);
        memcpy(weights_bias->kernelsPerCore, buff++, sizeof(vx_uint32) * weights_bias->zgroup_num);

        for (i = 0; i < weights_bias->zgroup_num; i++)
        {
            if (!weights_bias->outImageTileXSize[0][i] || !weights_bias->outImageTileYSize[0][i] || !weights_bias->kernelsPerCore[0][i])
            {
                if (weights_bias->all_count[i] && weights_bias->orig_size[i] && weights_bias->compressed_size[i])
                {
                    /* Calculate filters per core */
                    calculateFilterPerCore(weights_bias->base.context, weights_bias, i, vx_false_e, 0);
                }
                else
                {
                    return VX_ERROR_INVALID_PARAMETERS;
                }
            }
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

    vxError("vxoWeightsBiasesParameter_Map from "VX_FMT_REF" to ptr %p\n", weights_biases, *ptr);
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
                    vx_uint8 *pSrc = (vx_uint8 *)&weights_biases->memory.logicals[0][0] - weights_biases->memroy_head_offset;
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

    vxError("vxoWeightsBiasesParameter_Unmap from "VX_FMT_REF"\n", weights_biases);
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
                    vx_uint8 *pDst = (vx_uint8 *)&weights_biases->memory.logicals[0][0] - weights_biases->memroy_head_offset;
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

    vxError("vxoWeightsBiasesParameter_Map from "VX_FMT_REF" to ptr %p\n", weights_biases, *ptr);
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

    vxError("vxoWeightsBiasesParameter_Unmap from "VX_FMT_REF"\n", weights_biases);
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

    if (weights_biases->weights_sizes == VX_NULL ||
        weights_biases->biases_sizes == VX_NULL)
        return vx_false_e;

    return vx_true_e;
}

VX_PRIVATE_API vx_int32 getHwPoolingSze(vx_uint32 poolingSize)
{
    switch (poolingSize)
    {
    case 2:
        return 0;
    case 3:
        return 1;
    default:
        break;
    }

    return -1;
}

vx_int32 getHwPoolingType(vx_enum poolingType)
{
    switch (poolingType)
    {
    case VX_CONVOLUTIONAL_NETWORK_POOLING_MAX:
        return VIV_NN_POOLING_MAX;
    case VX_CONVOLUTIONAL_NETWORK_POOLING_AVG:
        return VIV_NN_POOLING_AVG;
    default:
        break;
    }

    return VIV_NN_POOLING_NON;
}

vx_int32 getHWRoundingMode(vx_nn_round_mode_e roundingMode)
{
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
        return 0x0;
    }
}

vx_int32 getHWBorderMode(vx_enum padMode)
{
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
}

void fillinCmmdBuff(
    vx_tensor                    inputs,
    vx_weights_biases_parameter  weights_biases,
    vx_uint32                    pad_x_left,
    vx_uint32                    pad_x_right,
    vx_uint32                    pad_y_top,
    vx_uint32                    pad_y_bottom,
    vx_enum                      pad_mode,
    void                        *pad_const,
    vx_enum                      conv_rounding_type,
    vx_bool                      enable_relu,
    vx_enum                      pool_type,
    vx_uint32                    pool_size_x,
    vx_uint32                    pool_size_y,
    vx_tensor                    outputs,
    vx_array                     cmd_buff,
    vx_bool                      isFullyConnectedLayer,
    vx_uint32                    index
    )
{
    vx_nn_cmd_info_u info;
    vx_uint32 input_width, input_height, input_height_ex, out_image_x, out_image_y, out_image_z, out_image_tilex, out_image_tiley;
    vx_uint32 conv_stride_x, conv_stride_y, kernels_per_core, mad_per_core;
    vx_uint32 org_kernel_x, org_kernel_y, kernel_x, kernel_y, kernel_z;
    vx_uint32 * cmd_buf_ptr;
    vx_context context;
    vx_uint32 fullyConnectedBatch = 1;

    vx_uint32 inImageTileSizeX, inImageTileSizeY;
    vx_uint32 distSize;

    /* Create weight_bias */
    context = vxGetContext((vx_reference)inputs);

    memset(&info, 0, sizeof(vx_nn_cmd_info_u));

    input_width   = TENSOR_SIZE_INDEX(inputs, 0);
    input_height  = TENSOR_SIZE_INDEX(inputs, 1);
    org_kernel_x  = weights_biases->org_weights_sizes[0];
    org_kernel_y  = weights_biases->org_weights_sizes[1];
    kernel_x      = weights_biases->weights_sizes[0];
    kernel_y      = weights_biases->weights_sizes[1];
    kernel_z      = weights_biases->weights_sizes[2];
    conv_stride_x = conv_stride_y = weights_biases->stride;
    out_image_z   = weights_biases->zgroup_array[index];
    out_image_tilex = weights_biases->outImageTileXSize[0][index];
    out_image_tiley = weights_biases->outImageTileYSize[0][index];
    kernels_per_core = weights_biases->kernelsPerCore[0][index];
    mad_per_core = weights_biases->current_mad_per_core;

    if (isFullyConnectedLayer)
    {
        if (input_width != 1 && input_height == 1)
        {
            fullyConnectedBatch = input_width;
            /* NNFCAccel is only for 1x1 fullyconnnected, batch fullyConnect disable it */
            context->options.enableNNFCAccel = 0;
        }
        else
        {
            input_width  = 1;
            input_height = 1;
        }
    }

    if ((inputs->finalDims[0] == 0) || (outputs->finalDims[0] == 0))
    {
        vxoNNExternsionInputOutputArguments(
            input_width, input_height,
            conv_stride_x, conv_stride_y,
            pad_x_left, pad_x_right, pad_y_top, pad_y_bottom,
            org_kernel_x, org_kernel_y,
            conv_rounding_type,
            pool_size_x,
            &inputs->insideDims[0], &inputs->insideDims[1],
            &outputs->insideDims[0], &outputs->insideDims[1],
            &inputs->finalDims[0], &inputs->finalDims[1],
            &outputs->finalDims[0], &outputs->finalDims[1]
        );
    }
    input_width  = inputs->finalDims[0];
    input_height = input_height_ex = inputs->finalDims[1];
    out_image_x  = outputs->finalDims[0];
    out_image_y  = outputs->finalDims[1];

    if ((outputs->insideDims[0] != outputs->finalDims[0] ||
        outputs->insideDims[1] != outputs->finalDims[1] ||
        out_image_tilex > outputs->finalDims[0]) && pool_size_x == 3)
    {
        out_image_tilex = outputs->finalDims[0];
    }
    if (isFullyConnectedLayer && kernel_x != kernel_y && kernel_x == 1)
    {
        input_height = input_height_ex = kernel_y;
    }
    if (weights_biases->use_fc_accel && isFullyConnectedLayer)
    {
        if (weights_biases->fc_accel_large_size && kernel_x != kernel_y)
        {
            input_width = out_image_x = out_image_z;
            out_image_y = 1;
            if (!(out_image_z & 0x3F))
            {
                out_image_tilex = 64 > mad_per_core ? mad_per_core : 64;
            }
            else
            {
                out_image_tilex = !(out_image_z & 0xF) ? out_image_z >> 4 : !(out_image_z & 0x3) ? out_image_z >> 2 : out_image_z;
                out_image_tilex = out_image_tilex > mad_per_core ? mad_per_core : out_image_tilex;
            }
            out_image_tiley = 2;
        }
        else
        {
            input_height_ex = inputs->finalDims[1];
            kernel_y = kernel_x = 1;

            switch (out_image_z)
            {
                case 4096:
                case  512:
                    input_width = out_image_x = 64;
                    out_image_tilex = 64 > mad_per_core ? mad_per_core : 64;
                    input_height = out_image_y = out_image_z / input_width;
                    out_image_tiley = 4;
                    break;

                case 1000:
                    input_width = out_image_x = 50;
                    out_image_tilex = 50 > mad_per_core ? 20 : 50;
                    input_height = out_image_y = out_image_z / input_width;
                    out_image_tiley = 4;
                    break;

                default:
                    if (out_image_z & 0x7)
                    {
                        /* should not be here */
                        input_width  = out_image_x = out_image_tilex = out_image_z;
                        input_height = out_image_y = 1;
                        out_image_tiley = 1;
                    }
                    else
                    {
                        input_width = out_image_x = out_image_z / 8;
                        out_image_tilex = out_image_x > mad_per_core ? mad_per_core : out_image_x;
                        input_height = out_image_y = 8;
                        out_image_tiley = 1;
                    }
                    break;
            }
        }

        kernel_z = weights_biases->input_nonzero_count;
        out_image_z  = 1;
        kernels_per_core = 1;
    }

    info.vx_nn_general_cmd_info.kernelXSize       = kernel_x;
    info.vx_nn_general_cmd_info.kernelYSize       = kernel_y;
    info.vx_nn_general_cmd_info.kernelZSize       = kernel_z;
    info.vx_nn_general_cmd_info.kernelsPerCore    = kernels_per_core;
    info.vx_nn_general_cmd_info.pooling           = getHwPoolingType(pool_type);
    info.vx_nn_general_cmd_info.poolingXYSize     = getHwPoolingSze(pool_size_x);
    info.vx_nn_general_cmd_info.relu              = enable_relu;
    info.vx_nn_general_cmd_info.inImageXSize      = input_width;
    info.vx_nn_general_cmd_info.inImageYSize      = input_height;
    info.vx_nn_general_cmd_info.outImageXSize     = out_image_x;
    info.vx_nn_general_cmd_info.outImageYSize     = out_image_y;
    info.vx_nn_general_cmd_info.outImageZSize     = out_image_z;
    info.vx_nn_general_cmd_info.inImageXOffset    = conv_stride_x > 1 ? 0 : (-1) * pad_x_left;
    info.vx_nn_general_cmd_info.inImageYOffset    = conv_stride_y > 1 ? 0 : (-1) * pad_y_top;
    if (fullyConnectedBatch > 1)
    {
        info.vx_nn_general_cmd_info.outImageTileXSize = out_image_tilex * fullyConnectedBatch;
    }
    else
    {
        info.vx_nn_general_cmd_info.outImageTileXSize = out_image_tilex;
    }
    info.vx_nn_general_cmd_info.outImageTileYSize = out_image_tiley;

    /* Fixed same type in this stage. */
    info.vx_nn_general_cmd_info.kernelDataType    = weights_biases->weights_data_format == VX_TYPE_INT8 ? 0x2 : 0x1;
    info.vx_nn_general_cmd_info.inImageDataType   = inputs->tensorBuffer->dataFormat == VX_TYPE_INT8 ? 0x2 : 0x1;
    info.vx_nn_general_cmd_info.outImageDataType  = outputs->tensorBuffer->dataFormat == VX_TYPE_INT8 ? 0x2 : 0x1;

    gcmASSERT(info.vx_nn_general_cmd_info.inImageDataType == info.vx_nn_general_cmd_info.kernelDataType);

    info.vx_nn_general_cmd_info.postMultiplier    = 0;
    info.vx_nn_general_cmd_info.roundingMode      = getHWRoundingMode((vx_nn_round_mode_e)outputs->tensorBuffer->roundingMode);

    if (weights_biases->weights_data_format == VX_TYPE_INT8)
    {
        info.vx_nn_general_cmd_info.postShift     = (vx_int8)inputs->tensorBuffer->fixedPointPos - (vx_int8)outputs->tensorBuffer->fixedPointPos + (vx_int8)weights_biases->weightFixedPointPos;
    }
    else
    {
        info.vx_nn_general_cmd_info.postShift     = 0;
    }

#if NN_WSZIE_REG
    info.vx_nn_general_cmd_info.wSize = 1;
#endif

#if NN_LAYER_FLUSH
    info.vx_nn_general_cmd_info.nn_layer_flush = 1;
#else
    info.vx_nn_general_cmd_info.nn_layer_flush = 0;
#endif


    inImageTileSizeX = out_image_tilex - 1 + kernel_x;
    inImageTileSizeY = out_image_tiley - 1 + kernel_y;

    distSize = inImageTileSizeX * inImageTileSizeY * kernel_z * vxnneGetTypeSize((vx_type_e)inputs->tensorBuffer->dataFormat);

    if (!isFullyConnectedLayer && context->options.enableBrickMode)
    {
        info.vx_nn_general_cmd_info.brickMode = 1;
        info.vx_nn_general_cmd_info.brickDistance = distSize;
    }
    else
    {
        info.vx_nn_general_cmd_info.brickMode = 0;
        info.vx_nn_general_cmd_info.brickDistance = 0;
    }

    if (context->options.enableSRAM && context->nnConfig.vipSRAMSize != 0)
    {
        vx_uint32 imageTileSize = (gcmALIGN((out_image_tilex + (kernel_x - 1) + pad_x_left + pad_x_right) * (out_image_tiley + (kernel_y - 1) + pad_y_top + pad_y_bottom), 16) * kernel_z) * vxnneGetTypeSize(inputs->tensorBuffer->dataFormat);
        imageTileSize = gcmALIGN(imageTileSize, 32);
        if (imageTileSize > context->nnConfig.vipSRAMSize * 1024)
        {
            imageTileSize = 2048;
            info.vx_nn_general_cmd_info.imageCachingMode              = 0;
            info.vx_nn_general_cmd_info.imageStartAddress             = 0;
            info.vx_nn_general_cmd_info.imageEndAddress               = imageTileSize;
        }
        else
        {
            info.vx_nn_general_cmd_info.imageCachingMode              = 1;
            info.vx_nn_general_cmd_info.imageStartAddress             = 0;
            info.vx_nn_general_cmd_info.imageEndAddress               = imageTileSize;
        }


        info.vx_nn_general_cmd_info.kernelCacheStartAddress   = (info.vx_nn_general_cmd_info.imageCachingMode == 0) ? imageTileSize : gcmALIGN((info.vx_nn_general_cmd_info.imageEndAddress), 32);


        vxnneSRAMGetkernelPattenField(context->nnConfig.vipSRAMSize,
                                      weights_biases->kernelStreamSize,
                                      &info.vx_nn_general_cmd_info.kernelCacheStartAddress,
                                      &info.vx_nn_general_cmd_info.kernelCachingMode,
                                      &info.vx_nn_general_cmd_info.kernelPatternLow32Bits,
                                      &info.vx_nn_general_cmd_info.kernelPatternHigh32Bits,
                                      &info.vx_nn_general_cmd_info.kernelPatternMsb
                                      );

        info.vx_nn_general_cmd_info.partialCacheDataUnit          = 0;

        if (info.vx_nn_general_cmd_info.kernelCachingMode == 1)
        {
            info.vx_nn_general_cmd_info.kernelCacheEndAddress     = info.vx_nn_general_cmd_info.kernelCacheStartAddress + weights_biases->kernelStreamSize;
        }
        else if (info.vx_nn_general_cmd_info.kernelCachingMode == 2)
        {
            vx_uint32 dataUnitByte = 64;
            switch (info.vx_nn_general_cmd_info.partialCacheDataUnit)
            {
            case 0:
                dataUnitByte = 64;
                break;
            case 1:
                dataUnitByte = 128;
                break;
            case 2:
                dataUnitByte = 256;
                break;
            case 3:
                dataUnitByte = 512;
                break;
            }

            info.vx_nn_general_cmd_info.kernelCacheEndAddress     = info.vx_nn_general_cmd_info.kernelCacheStartAddress +
                                                                    ((vx_uint32)gcoMATH_Ceiling((vx_float32)weights_biases->kernelStreamSize / (dataUnitByte * (info.vx_nn_general_cmd_info.kernelPatternMsb + 1)))) *
                                                                    (dataUnitByte * (vxnneGetOneNumber(info.vx_nn_general_cmd_info.kernelPatternLow32Bits) + vxnneGetOneNumber(info.vx_nn_general_cmd_info.kernelPatternHigh32Bits)));
            if(info.vx_nn_general_cmd_info.kernelCacheEndAddress > context->nnConfig.vipSRAMSize * 1024)
            {
                vx_uint32 kernelCacheSizeInSram = 0x0;
                info.vx_nn_general_cmd_info.kernelCacheEndAddress = context->nnConfig.vipSRAMSize * 1024;
                kernelCacheSizeInSram = info.vx_nn_general_cmd_info.kernelCacheEndAddress - info.vx_nn_general_cmd_info.kernelCacheStartAddress;
                if(kernelCacheSizeInSram % dataUnitByte != 0 )
                {
                    kernelCacheSizeInSram = (kernelCacheSizeInSram / dataUnitByte) * dataUnitByte;
                    info.vx_nn_general_cmd_info.kernelCacheEndAddress = info.vx_nn_general_cmd_info.kernelCacheStartAddress + kernelCacheSizeInSram;
                }
            }

            gcmASSERT(((info.vx_nn_general_cmd_info.kernelCacheEndAddress - info.vx_nn_general_cmd_info.kernelCacheStartAddress) % dataUnitByte) == 0);
        }

        gcmASSERT(weights_biases->kernelStreamSize > 0);
        if (info.vx_nn_general_cmd_info.imageCachingMode != 0)
        {
            gcmASSERT(info.vx_nn_general_cmd_info.imageEndAddress > info.vx_nn_general_cmd_info.imageStartAddress);
        }

        if (info.vx_nn_general_cmd_info.kernelCachingMode != 0)
        {
            gcmASSERT(info.vx_nn_general_cmd_info.kernelCacheStartAddress >= 0);
            gcmASSERT(info.vx_nn_general_cmd_info.kernelCacheEndAddress > info.vx_nn_general_cmd_info.kernelCacheStartAddress);
            gcmASSERT(info.vx_nn_general_cmd_info.kernelCacheEndAddress <= context->nnConfig.vipSRAMSize * 1024 );
            gcmASSERT((info.vx_nn_general_cmd_info.kernelCacheStartAddress & 0x1f) == 0x0);
        }

        if (context->options.enableSramStreamMode)  /* test for image stream mode and kernel stream mode */
        {
            info.vx_nn_general_cmd_info.imageCachingMode = 0;
            info.vx_nn_general_cmd_info.imageStartAddress = 0;
            info.vx_nn_general_cmd_info.imageEndAddress = 2048;

            info.vx_nn_general_cmd_info.kernelCachingMode = 0;
            info.vx_nn_general_cmd_info.partialCacheDataUnit = 0;
            info.vx_nn_general_cmd_info.kernelPatternMsb = 0;
            info.vx_nn_general_cmd_info.kernelPatternLow32Bits = 0;
            info.vx_nn_general_cmd_info.kernelPatternHigh32Bits = 0;
            info.vx_nn_general_cmd_info.kernelCacheStartAddress = 0;
            info.vx_nn_general_cmd_info.kernelCacheEndAddress = 0;
        }
    }

    if (context->options.enableBorderMode)
    {
        info.vx_nn_general_cmd_info.inImageBorderMode = getHWBorderMode(pad_mode);
        if (pad_const == VX_NULL)
        {
            info.vx_nn_general_cmd_info.imImageBorderConst = 0;
        }
        else
        {
            info.vx_nn_general_cmd_info.imImageBorderConst = (inputs->tensorBuffer->dataFormat == VX_TYPE_INT8) ? (*(vx_int8*)pad_const) : (*(vx_int16*)pad_const);
        }

        if ((info.vx_nn_general_cmd_info.inImageBorderMode == 0x2)
            || (info.vx_nn_general_cmd_info.inImageBorderMode == 0x3))
        {
            gcmASSERT(info.vx_nn_general_cmd_info.inImageXOffset <= 5);
            gcmASSERT(info.vx_nn_general_cmd_info.inImageYOffset <= 5);
        }
    }

    cmd_buf_ptr = (vx_uint32*)(cmd_buff->memory.logicals[0] + NNE_COMMAND_SIZE * index);

    gcoVX_ProgrammCrossEngine((void*)&info, gcvVX_ACCELERATOR_NN, (void*)&context->options, &cmd_buf_ptr);
}

void fillInCmdTPBuffer(
    vx_tensor                    inputs,
    vx_reference                 other_tensor,
    vx_tensor                    outputs,
    vx_array                     cmd_buff,
    vx_array                     data_buff,
    vx_tp_conv_cmd *             conv_cmd_ptr,
    vx_tp_value_cmd *            value_cmd_ptr,
    vx_enum                      tp_type,
    vx_uint32                    index,
    vx_bool                      multi_tp,
    vx_bool                      last
    )
{
    vx_nn_cmd_info_u info;
    vx_uint32 * cmd_buf_ptr;
    vx_uint32 inXSize, inYSize, inZSize, outXSize, outYSize, outZSize, outTileXSize, outTileYSize;
    vx_uint32 strideXSize, strideYSize, outSliceSize, outStrideSliceSize, sliceIndex, strideIndex;
    vx_uint32 poolingStride, inputElemSize, outputElemSize, inputBase, outputBase;
    vx_context context = vxGetContext((vx_reference)cmd_buff);

    memset(&info, 0, sizeof(vx_nn_cmd_info_u));

    inXSize = TENSOR_SIZE_INDEX(inputs, 0);
    inYSize = TENSOR_SIZE_INDEX(inputs, 1);
    inZSize = TENSOR_SIZE_INDEX(inputs, 2);
    inputElemSize = TENSOR_DATA_SIZE(inputs);
    outputElemSize = TENSOR_DATA_SIZE(outputs);

    vxoTensor_GetTensorViewMemory(inputs, VX_NULL, &inputBase);
    vxoTensor_GetTensorViewMemory(outputs, VX_NULL, &outputBase);

    switch (tp_type)
    {
        case TP_RESHUFFLE:
            gcmASSERT(conv_cmd_ptr != VX_NULL);

            strideXSize = ((vx_weights_biases_parameter)other_tensor)->stride;
            strideYSize = ((vx_weights_biases_parameter)other_tensor)->stride;
            outStrideSliceSize = strideXSize * strideYSize;
            outXSize = TENSOR_SIZE_INDEX(outputs, 0) / strideXSize;
            outYSize = TENSOR_SIZE_INDEX(outputs, 1) / strideYSize;
            outZSize = TENSOR_SIZE_INDEX(outputs, 2) * outStrideSliceSize;
            outSliceSize = outXSize * outYSize;

            info.vx_nn_tp_cmd_info.inImageXSize = inXSize;
            info.vx_nn_tp_cmd_info.inImageYSize = inYSize;
            info.vx_nn_tp_cmd_info.inImageZSize = multi_tp ? 1 : inZSize;
            info.vx_nn_tp_cmd_info.inImageStride = inXSize;
            info.vx_nn_tp_cmd_info.inImageSlice = inXSize * inYSize;
            info.vx_nn_tp_cmd_info.inWindowXStart = (-1) * (vx_int32)conv_cmd_ptr->pad_x;
            info.vx_nn_tp_cmd_info.inWindowYStart = (-1) * (vx_int32)conv_cmd_ptr->pad_y;
            info.vx_nn_tp_cmd_info.inWindowXEnd = outXSize * strideXSize + info.vx_nn_tp_cmd_info.inWindowXStart - 1;
            info.vx_nn_tp_cmd_info.inWindowYEnd = outYSize * strideYSize + info.vx_nn_tp_cmd_info.inWindowYStart - 1;
            info.vx_nn_tp_cmd_info.inTileSequence = 0x0;
            info.vx_nn_tp_cmd_info.inImageGlobalMem = 1;
            info.vx_nn_tp_cmd_info.inImageBaseAddress = inputBase + index * inXSize * inYSize * inputElemSize;
            info.vx_nn_tp_cmd_info.outTileSkipAtborder = 0;
            info.vx_nn_tp_cmd_info.inTileXSize = (outXSize - 1) * strideXSize + strideXSize;
            info.vx_nn_tp_cmd_info.inTileYSize = (outYSize - 1) * strideYSize + strideYSize;
            info.vx_nn_tp_cmd_info.inTileXInc = outXSize * strideXSize;
            info.vx_nn_tp_cmd_info.inTileYInc = outYSize * strideYSize;
            info.vx_nn_tp_cmd_info.outBaseAddress = outputBase + index * outSliceSize * outStrideSliceSize * outputElemSize;
            info.vx_nn_tp_cmd_info.outGlobalMem  = 1;

            switch (context->options.typeTPFunc[TP_RESHUFFLE])
            {
                case 0: /* 2D tile walking sequence. */
                default:
                    info.vx_nn_tp_cmd_info.outBrickMode  = 0;
                    info.vx_nn_tp_cmd_info.outLoop0Inc   = outSliceSize;
                    info.vx_nn_tp_cmd_info.outLoop0Count = strideXSize;
                    info.vx_nn_tp_cmd_info.outLoop1Inc   = 1;
                    info.vx_nn_tp_cmd_info.outLoop1Count = outXSize;
                    info.vx_nn_tp_cmd_info.outLoop1Reset = 1;
                    info.vx_nn_tp_cmd_info.outLoop2Inc   = outSliceSize * strideXSize;
                    info.vx_nn_tp_cmd_info.outLoop2Count = strideYSize;
                    info.vx_nn_tp_cmd_info.outLoop2Reset = 0;
                    info.vx_nn_tp_cmd_info.outLoop3Inc   = outXSize;
                    info.vx_nn_tp_cmd_info.outLoop3Count = outYSize;
                    info.vx_nn_tp_cmd_info.outLoop3Reset = 1;
                    info.vx_nn_tp_cmd_info.outLoop4Inc   = 0;
                    info.vx_nn_tp_cmd_info.outLoop4Count = 1;
                    info.vx_nn_tp_cmd_info.outLoop5Inc   = 0;
                    info.vx_nn_tp_cmd_info.outLoop5Count = 1;
                    info.vx_nn_tp_cmd_info.outLoop6Inc   = multi_tp ? 0 : outSliceSize * outStrideSliceSize;
                    break;

                case 1:
                    info.vx_nn_tp_cmd_info.outBrickMode  = 1;
                    info.vx_nn_tp_cmd_info.outLoop0Inc   = outSliceSize | (outSliceSize << 16);
                    info.vx_nn_tp_cmd_info.outLoop0Count = strideXSize;
                    info.vx_nn_tp_cmd_info.outLoop1Inc   = outSliceSize | (outSliceSize << 16);
                    info.vx_nn_tp_cmd_info.outLoop1Count = 0;
                    info.vx_nn_tp_cmd_info.outLoop1Reset = 1;
                    info.vx_nn_tp_cmd_info.outLoop2Inc   = outSliceSize | (outSliceSize << 16);
                    info.vx_nn_tp_cmd_info.outLoop2Count = strideYSize;
                    info.vx_nn_tp_cmd_info.outLoop2Reset = 0;
                    info.vx_nn_tp_cmd_info.outLoop3Inc   = outSliceSize | (outSliceSize << 16);
                    info.vx_nn_tp_cmd_info.outLoop3Count = 0;
                    info.vx_nn_tp_cmd_info.outLoop3Reset = 1;
                    info.vx_nn_tp_cmd_info.outLoop4Inc   = outSliceSize * (multi_tp ? outStrideSliceSize : outZSize); /* brick distance */
                    info.vx_nn_tp_cmd_info.outLoop4Count = 1;
                    info.vx_nn_tp_cmd_info.outLoop5Inc   = outSliceSize | (outSliceSize << 16);
                    info.vx_nn_tp_cmd_info.outLoop5Count = 1;
                    info.vx_nn_tp_cmd_info.outLoop6Inc   = 0;
                    break;

                case 2: /* special 2D tile walking sequence to improve output performance. */
                    sliceIndex = index / strideXSize;
                    strideIndex = index % strideXSize;
                    info.vx_nn_tp_cmd_info.inImageBaseAddress = inputBase + sliceIndex * inXSize * inYSize * inputElemSize + strideIndex;
                    info.vx_nn_tp_cmd_info.outBaseAddress = outputBase + sliceIndex * outSliceSize * outStrideSliceSize * outputElemSize + strideIndex * outSliceSize;
                    info.vx_nn_tp_cmd_info.inImageXSize -= strideIndex;
                    info.vx_nn_tp_cmd_info.inWindowXEnd -= strideXSize - 1;
                    info.vx_nn_tp_cmd_info.inWindowYEnd  = outYSize * strideYSize - 1 + conv_cmd_ptr->pad_y;
                    info.vx_nn_tp_cmd_info.inTileXSize   = 1;
                    info.vx_nn_tp_cmd_info.inTileYSize   = 1;
                    info.vx_nn_tp_cmd_info.inTileXInc    = strideXSize;
                    info.vx_nn_tp_cmd_info.inTileYInc    = 1;
                    info.vx_nn_tp_cmd_info.outBrickMode  = 0;
                    info.vx_nn_tp_cmd_info.outLoop0Inc   = 1;
                    info.vx_nn_tp_cmd_info.outLoop0Count = outXSize;
                    info.vx_nn_tp_cmd_info.outLoop1Inc   = outSliceSize * strideXSize;
                    info.vx_nn_tp_cmd_info.outLoop1Count = strideYSize;
                    info.vx_nn_tp_cmd_info.outLoop1Reset = 0;
                    info.vx_nn_tp_cmd_info.outLoop2Inc   = outXSize;
                    info.vx_nn_tp_cmd_info.outLoop2Count = outYSize;
                    info.vx_nn_tp_cmd_info.outLoop2Reset = 0;
                    info.vx_nn_tp_cmd_info.outLoop3Inc   = 0;
                    info.vx_nn_tp_cmd_info.outLoop3Count = 1;
                    info.vx_nn_tp_cmd_info.outLoop3Reset = 0;
                    info.vx_nn_tp_cmd_info.outLoop4Inc   = 0;
                    info.vx_nn_tp_cmd_info.outLoop4Count = 1;
                    info.vx_nn_tp_cmd_info.outLoop5Inc   = 0;
                    info.vx_nn_tp_cmd_info.outLoop5Count = 1;
                    info.vx_nn_tp_cmd_info.outLoop6Inc   = outSliceSize * outStrideSliceSize;
                    break;
            }

            info.vx_nn_tp_cmd_info.aluFilterPwlSwap = 0;
            info.vx_nn_tp_cmd_info.aluPwlSignSupport = 0;
            info.vx_nn_tp_cmd_info.aluReluEnable = 0;
            info.vx_nn_tp_cmd_info.aluI2FEnable = 0;
            info.vx_nn_tp_cmd_info.aluF2IEnable = 0;
            break;

        case TP_SINGLE_FC:
        {
            vx_weights_biases_parameter weights_biases = (vx_weights_biases_parameter) other_tensor;

            gcmASSERT(conv_cmd_ptr != VX_NULL);
            gcmASSERT(weights_biases->use_tp_fc);
            if (inXSize != 1 || inYSize != 1)
            {
                inZSize *= inXSize * inYSize;
                inXSize = inYSize = 1;
            }
            outZSize = weights_biases->zgroup_array[index];

            info.vx_nn_tp_cmd_info.inImageXSize = inXSize;
            info.vx_nn_tp_cmd_info.inImageYSize = inYSize;
            info.vx_nn_tp_cmd_info.inImageZSize = inZSize;
            info.vx_nn_tp_cmd_info.inImageStride = inXSize;
            info.vx_nn_tp_cmd_info.inImageSlice = inXSize * inYSize;
            info.vx_nn_tp_cmd_info.inWindowXStart = 0;
            info.vx_nn_tp_cmd_info.inWindowYStart = 0;
            info.vx_nn_tp_cmd_info.inWindowXEnd = inXSize - 1;
            info.vx_nn_tp_cmd_info.inWindowYEnd = inYSize - 1;
            info.vx_nn_tp_cmd_info.inTileSequence = 0x3;
            info.vx_nn_tp_cmd_info.inImageGlobalMem = 1;
            info.vx_nn_tp_cmd_info.inImageBaseAddress = inputBase;
            info.vx_nn_tp_cmd_info.inTileXSize = 1;
            info.vx_nn_tp_cmd_info.inTileYSize = 1;
            info.vx_nn_tp_cmd_info.inTileXInc = 1;
            info.vx_nn_tp_cmd_info.inTileYInc = 1;
            info.vx_nn_tp_cmd_info.aluSquarePreshift = 0;
            info.vx_nn_tp_cmd_info.aluSquareEnable = 0;
            info.vx_nn_tp_cmd_info.aluHorzProcessing = weights_biases->weights_data_format == VX_TYPE_INT8 ?
                                                       0x2 :
                                                       0x1;
            info.vx_nn_tp_cmd_info.aluHorzProcCount = (outZSize - 1) & 0x3F;    /* Lower 6 bits of FcOutZsizeMinusOne. */
            info.vx_nn_tp_cmd_info.aluHorzProcStride = 0;
            info.vx_nn_tp_cmd_info.aluVertProcessing = 0;
            info.vx_nn_tp_cmd_info.aluVertProcCount = (outZSize - 1) >> 6;      /* Higher 3 bits of FcOutZsizeMinusOne. */
            info.vx_nn_tp_cmd_info.aluVertProcStride = 0;
            info.vx_nn_tp_cmd_info.aluPwlEnable = 0;
            info.vx_nn_tp_cmd_info.aluMultEnable = 0;
            info.vx_nn_tp_cmd_info.aluLoadPwlLUT = 0;
            info.vx_nn_tp_cmd_info.aluLoadPwlLUTAddress = (gctUINT32)(weights_biases->memory.physicals[0] + weights_biases->memory_offset_array[index]);
            info.vx_nn_tp_cmd_info.aluLoadPwlLUTGlobalMem = 1;
            info.vx_nn_tp_cmd_info.outBaseAddress = outputBase + index * weights_biases->zgroup_array[0] * outputElemSize;
            info.vx_nn_tp_cmd_info.outGlobalMem  = 1;
            info.vx_nn_tp_cmd_info.outTileSkipAtborder = 0;
            info.vx_nn_tp_cmd_info.outBrickMode  = 0;
            info.vx_nn_tp_cmd_info.outLoop0Inc   = 0;
            info.vx_nn_tp_cmd_info.outLoop0Count = 1;
            info.vx_nn_tp_cmd_info.outLoop1Inc   = 0;
            info.vx_nn_tp_cmd_info.outLoop1Count = 1;
            info.vx_nn_tp_cmd_info.outLoop1Reset = 0x0;
            info.vx_nn_tp_cmd_info.outLoop2Inc   = 0;
            info.vx_nn_tp_cmd_info.outLoop2Count = 1;
            info.vx_nn_tp_cmd_info.outLoop2Reset = 0x0;
            info.vx_nn_tp_cmd_info.outLoop3Inc   = 0;
            info.vx_nn_tp_cmd_info.outLoop3Count = 1;
            info.vx_nn_tp_cmd_info.outLoop3Reset = 0x0;
            info.vx_nn_tp_cmd_info.outLoop4Inc   = 1;
            info.vx_nn_tp_cmd_info.outLoop4Count = outZSize;
            info.vx_nn_tp_cmd_info.outLoop5Inc   = 0;
            info.vx_nn_tp_cmd_info.outLoop5Count = 1;
            info.vx_nn_tp_cmd_info.outLoop6Inc   = 0;
            info.vx_nn_tp_cmd_info.aluFilterPwlSwap = 0;
            info.vx_nn_tp_cmd_info.aluPwlSignSupport = 0;
            info.vx_nn_tp_cmd_info.aluReluEnable =  conv_cmd_ptr->enable_relu;
            info.vx_nn_tp_cmd_info.aluInputPreshift   = 0;
            info.vx_nn_tp_cmd_info.aluOutputPostshift =
                 ((TENSOR_DATA_SIZE(inputs) == 1 ? TENSOR_POS(inputs) : 0)
                - (TENSOR_DATA_SIZE(outputs) == 1 ? TENSOR_POS(outputs) : 0)
                + (weights_biases->weights_data_format == VX_TYPE_FLOAT16 ? 0 : weights_biases->weightFixedPointPos));
            info.vx_nn_tp_cmd_info.aluI2FEnable = TENSOR_DATA_SIZE(inputs) == 1 ? 1 : 0;
            info.vx_nn_tp_cmd_info.aluF2IEnable = TENSOR_DATA_SIZE(outputs) == 1 ? 1 : 0;
            break;
        }

        case TP_TRANSPOSE:
            gcmASSERT(!multi_tp);
            gcmASSERT(!index);

            info.vx_nn_tp_cmd_info.inImageGlobalMem = 1;
            info.vx_nn_tp_cmd_info.inImageBaseAddress = inputBase;
            info.vx_nn_tp_cmd_info.outTileSkipAtborder = 0;
            info.vx_nn_tp_cmd_info.outBaseAddress = outputBase;
            info.vx_nn_tp_cmd_info.outGlobalMem  = 1;
            info.vx_nn_tp_cmd_info.aluFilterPwlSwap = 0;
            info.vx_nn_tp_cmd_info.aluPwlSignSupport = 0;
            info.vx_nn_tp_cmd_info.aluReluEnable = 0;
            info.vx_nn_tp_cmd_info.aluI2FEnable = 0;
            info.vx_nn_tp_cmd_info.aluF2IEnable = 0;

            {
                vx_uint32 maxSize = 0x1<<16, total = 1, inXSizeNew, inYSizeNew;
                vx_uint32 i, j, dims[VX_CONTEXT_TENSOR_MAX_DIMENSION], distances[VX_CONTEXT_TENSOR_MAX_DIMENSION];
                vx_uint32_ptr perm = (vx_uint32_ptr) value_cmd_ptr->p8[0];
                vx_uint32 pnum = value_cmd_ptr->u32[0];
                vxoTensor_GetTensorDimStride(inputs, &pnum, dims, VX_NULL);

                for (i = 0; i < pnum; i++)
                {
                    vx_uint32 dim = 1;
                    for (j = 0; j < i; j++)
                        dim *= dims[perm[j]];
                    distances[perm[i]] = dim;
                    total *= dims[i];
                }
                /* Merge input tensor to 2D image. X/YSize < 2^16. */
                inXSizeNew = dims[0];
                inYSizeNew = total / inXSizeNew;
                for (i = 1; i < pnum; i++)
                {
                    if (inYSizeNew < maxSize)
                    {
                        break;
                    }
                    else
                    {
                        inXSizeNew *= dims[i];
                        inYSizeNew /= dims[i];
                    }
                }
                gcmASSERT(inXSizeNew<maxSize && inYSizeNew<maxSize);

                info.vx_nn_tp_cmd_info.inImageXSize   = inXSizeNew;
                info.vx_nn_tp_cmd_info.inImageYSize   = inYSizeNew;
                info.vx_nn_tp_cmd_info.inImageZSize   = 1;
                info.vx_nn_tp_cmd_info.inImageStride  = inXSizeNew;
                info.vx_nn_tp_cmd_info.inImageSlice   = total;
                info.vx_nn_tp_cmd_info.inWindowXStart = 0;
                info.vx_nn_tp_cmd_info.inWindowYStart = 0;
                info.vx_nn_tp_cmd_info.inWindowXEnd   = inXSizeNew - 1;
                info.vx_nn_tp_cmd_info.inWindowYEnd   = inYSizeNew - 1;
                info.vx_nn_tp_cmd_info.inTileXSize    = 1;
                info.vx_nn_tp_cmd_info.inTileYSize    = 1;
                info.vx_nn_tp_cmd_info.inTileXInc     = 1;
                info.vx_nn_tp_cmd_info.inTileYInc     = 1;
                info.vx_nn_tp_cmd_info.outLoop0Inc   = distances[0];
                info.vx_nn_tp_cmd_info.outLoop0Count = dims[0];
                info.vx_nn_tp_cmd_info.outLoop1Inc   = 0;
                info.vx_nn_tp_cmd_info.outLoop1Count = 1;
                info.vx_nn_tp_cmd_info.outLoop1Reset = 0;
                info.vx_nn_tp_cmd_info.outLoop2Inc   = distances[1];
                info.vx_nn_tp_cmd_info.outLoop2Count = dims[1];
                info.vx_nn_tp_cmd_info.outLoop2Reset = 0;
                info.vx_nn_tp_cmd_info.outLoop3Inc   = pnum > 2 ? distances[2] : 0;
                info.vx_nn_tp_cmd_info.outLoop3Count = pnum > 2 ? dims[2] : 1;
                info.vx_nn_tp_cmd_info.outLoop3Reset = 0;
                info.vx_nn_tp_cmd_info.outLoop4Inc   = pnum > 3 ? distances[3] : 0;
                info.vx_nn_tp_cmd_info.outLoop4Count = pnum > 3 ? dims[3] : 1;
                info.vx_nn_tp_cmd_info.outLoop5Inc   = pnum > 4 ? distances[4] : 0;
                info.vx_nn_tp_cmd_info.outLoop5Count = pnum > 4 ? dims[4] : 1;
                info.vx_nn_tp_cmd_info.outLoop6Inc   = pnum > 5 ? distances[5] : 0;
            }
            break;

        case TP_MAX_POOLING:
            gcmASSERT(conv_cmd_ptr != VX_NULL);

            outTileXSize = 32;
            outTileYSize = 16;
            outXSize = TENSOR_SIZE_INDEX(outputs, 0);
            outYSize = TENSOR_SIZE_INDEX(outputs, 1);
            poolingStride = conv_cmd_ptr->pool_stride;

            gcmASSERT(conv_cmd_ptr->pool_size_x != 1);
            gcmASSERT((poolingStride == 1 && conv_cmd_ptr->pool_size_x <= 3) ||
                      ((poolingStride == conv_cmd_ptr->pool_size_x || poolingStride == conv_cmd_ptr->pool_size_x-1) &&
                        conv_cmd_ptr->pool_size_x <= 64));

            info.vx_nn_tp_cmd_info.inImageXSize = inXSize;
            info.vx_nn_tp_cmd_info.inImageYSize = inYSize;
            info.vx_nn_tp_cmd_info.inImageZSize = multi_tp ? 1: inZSize;
            info.vx_nn_tp_cmd_info.inImageStride = inXSize;
            info.vx_nn_tp_cmd_info.inImageSlice = inXSize * inYSize;
            info.vx_nn_tp_cmd_info.inWindowXStart = (-1) * (vx_int32)conv_cmd_ptr->pad_x;
            info.vx_nn_tp_cmd_info.inWindowYStart = (-1) * (vx_int32)conv_cmd_ptr->pad_y;
            info.vx_nn_tp_cmd_info.inWindowXEnd = (outXSize - 1) * poolingStride + conv_cmd_ptr->pool_size_x - 1 - conv_cmd_ptr->pad_x;
            info.vx_nn_tp_cmd_info.inWindowYEnd = (outYSize - 1) * poolingStride + conv_cmd_ptr->pool_size_y - 1 - conv_cmd_ptr->pad_y;
            info.vx_nn_tp_cmd_info.inTileSequence = 0x0;
            info.vx_nn_tp_cmd_info.inImageGlobalMem = 1;
            info.vx_nn_tp_cmd_info.inImageBaseAddress = inputBase + index * inXSize * inYSize * inputElemSize;
            info.vx_nn_tp_cmd_info.inTileXSize = outTileXSize * poolingStride + conv_cmd_ptr->pool_size_x - poolingStride;
            info.vx_nn_tp_cmd_info.inTileYSize = outTileYSize * poolingStride + conv_cmd_ptr->pool_size_y - poolingStride;
            info.vx_nn_tp_cmd_info.inTileXInc = outTileXSize * poolingStride;
            info.vx_nn_tp_cmd_info.inTileYInc = outTileYSize * poolingStride;
            info.vx_nn_tp_cmd_info.aluSquarePreshift = 0;
            info.vx_nn_tp_cmd_info.aluSquareEnable = 0;
            if (poolingStride == 1)
            {
                info.vx_nn_tp_cmd_info.aluHorzProcessing = 0x1;
                info.vx_nn_tp_cmd_info.aluHorzProcCount = conv_cmd_ptr->pool_size_x - 1;
                info.vx_nn_tp_cmd_info.aluHorzProcStride = 0;
                info.vx_nn_tp_cmd_info.aluVertProcessing = 0x1;
                info.vx_nn_tp_cmd_info.aluVertProcCount = conv_cmd_ptr->pool_size_y - 1;
                info.vx_nn_tp_cmd_info.aluVertProcStride = 0;
            }
            else
            {
                info.vx_nn_tp_cmd_info.aluHorzProcessing = 0x3;
                info.vx_nn_tp_cmd_info.aluHorzProcCount = conv_cmd_ptr->pool_size_x - 1;
                info.vx_nn_tp_cmd_info.aluHorzProcStride = conv_cmd_ptr->pool_size_x == poolingStride ?
                    0x0 : 0x1;
                info.vx_nn_tp_cmd_info.aluVertProcessing = 0x3;
                info.vx_nn_tp_cmd_info.aluVertProcCount = conv_cmd_ptr->pool_size_y - 1;
                info.vx_nn_tp_cmd_info.aluVertProcStride = conv_cmd_ptr->pool_size_y == poolingStride ?
                    0x0 : 0x1;
            }
            info.vx_nn_tp_cmd_info.aluPwlEnable = 0;
            info.vx_nn_tp_cmd_info.aluMultEnable = 0;
            info.vx_nn_tp_cmd_info.aluLoadPwlLUT = 0;
            info.vx_nn_tp_cmd_info.aluLoadPwlLUTAddress = 0;
            info.vx_nn_tp_cmd_info.aluLoadPwlLUTGlobalMem = 1;
            info.vx_nn_tp_cmd_info.outBaseAddress = outputBase + index * outXSize * outYSize * outputElemSize;
            info.vx_nn_tp_cmd_info.outGlobalMem  = 1;
            info.vx_nn_tp_cmd_info.outTileSkipAtborder = 0;
            info.vx_nn_tp_cmd_info.outBrickMode  = 0;
            info.vx_nn_tp_cmd_info.outLoop0Inc   = 0;
            info.vx_nn_tp_cmd_info.outLoop0Count = 1;
            info.vx_nn_tp_cmd_info.outLoop1Inc   = 1;
            info.vx_nn_tp_cmd_info.outLoop1Count = 0;
            info.vx_nn_tp_cmd_info.outLoop1Reset = 0x1;
            info.vx_nn_tp_cmd_info.outLoop2Inc   = outXSize;
            info.vx_nn_tp_cmd_info.outLoop2Count = 0;
            info.vx_nn_tp_cmd_info.outLoop2Reset = 0x1;
            info.vx_nn_tp_cmd_info.outLoop3Inc   = outTileXSize;
            info.vx_nn_tp_cmd_info.outLoop3Count = (outXSize + outTileXSize - 1) / outTileXSize;
            info.vx_nn_tp_cmd_info.outLoop3Reset = 0x0;
            info.vx_nn_tp_cmd_info.outLoop4Inc   = outTileYSize * outXSize;
            info.vx_nn_tp_cmd_info.outLoop4Count = (outYSize + outTileYSize - 1) / outTileYSize;
            info.vx_nn_tp_cmd_info.outLoop5Inc   = 0;
            info.vx_nn_tp_cmd_info.outLoop5Count = 1;
            info.vx_nn_tp_cmd_info.outLoop6Inc   = multi_tp ? 0 : outXSize * outYSize;
            info.vx_nn_tp_cmd_info.aluFilterPwlSwap = 0;
            info.vx_nn_tp_cmd_info.aluPwlSignSupport = 0;
            info.vx_nn_tp_cmd_info.aluReluEnable = 0;
            info.vx_nn_tp_cmd_info.aluI2FEnable = TENSOR_DATA_SIZE(inputs) == 1 ? 1 : 0;
            info.vx_nn_tp_cmd_info.aluF2IEnable = TENSOR_DATA_SIZE(outputs) == 1 ? 1 : 0;
            info.vx_nn_tp_cmd_info.aluInputPreshift   = 0;
            info.vx_nn_tp_cmd_info.aluOutputPostshift = (TENSOR_DATA_SIZE(inputs) == 1 ? TENSOR_POS(inputs) : 0) -
                                                        (TENSOR_DATA_SIZE(outputs) == 1 ? TENSOR_POS(outputs) : 0);
            break;

        case TP_LEAKY_RELU:
        case TP_LEAKY_RELU_MAX_POOLING:
        {
            vx_float32  scale = value_cmd_ptr->f32[0];
            vx_uint16 * pwlLUTBase = (vx_uint16 *)data_buff->memory.logicals[0];
            vx_uint16   base, baseF16;
            vx_float32  baseF32;
            vx_float32  pwlValue;

            gcmASSERT(conv_cmd_ptr != VX_NULL);
            gcmASSERT(pwlLUTBase != VX_NULL);

            /* Flush denorms to 0.0f. */
            for (base = 0; base < 0x10; base++)
            {
                pwlLUTBase[base] = 0x0;         /* Half float positive zero. */
            }
            for (base = 0x200; base < 0x210; base++)
            {
                pwlLUTBase[base] = 0x8000;      /* Half float negative zero. */
            }
            for (base = 0x10; base < 0x1F0; base++)
            {
                baseF16 = base << 6;
                pwlLUTBase[base] = baseF16;
            }
            for (base = 0x210; base < 0x3F0; base++)
            {
                baseF16 = base << 6;
                baseF32 = Fp16toFp32(baseF16);
                pwlValue = baseF32 * scale;
                pwlLUTBase[base] = Fp32toFp16(pwlValue);
            }
            pwlLUTBase[0x1F0] = 0x7c00;         /* Half float positive infinity. */
            pwlLUTBase[0x3F0] = 0xfc00;         /* Half float negative infinity. */
            /* NaN in, NaN out. */
            for (base = 0x1F1; base < 0x200; base++)
            {
                pwlLUTBase[base] = 0x7e00;      /* Half float positive NaN. */
            }
            for (base = 0x3F1; base < 0x400; base++)
            {
                pwlLUTBase[base] = 0xfe00;      /* Half float negative NaN. */
            }

            if (tp_type == TP_LEAKY_RELU_MAX_POOLING)
            {
                outTileXSize = 32;
                outTileYSize = 16;
                outXSize = TENSOR_SIZE_INDEX(outputs, 0);
                outYSize = TENSOR_SIZE_INDEX(outputs, 1);
                poolingStride = conv_cmd_ptr->pool_stride;

                gcmASSERT(conv_cmd_ptr->pool_size_x != 1);
                gcmASSERT((poolingStride == 1 && conv_cmd_ptr->pool_size_x <= 5) ||
                          (poolingStride == 2 && conv_cmd_ptr->pool_size_x <= 3));

                info.vx_nn_tp_cmd_info.inWindowXStart = (-1) * (vx_int32)conv_cmd_ptr->pad_x;
                info.vx_nn_tp_cmd_info.inWindowYStart = (-1) * (vx_int32)conv_cmd_ptr->pad_y;
                info.vx_nn_tp_cmd_info.inWindowXEnd = (outXSize - 1) * poolingStride + conv_cmd_ptr->pool_size_x - 1 - conv_cmd_ptr->pad_x;
                info.vx_nn_tp_cmd_info.inWindowYEnd = (outYSize - 1) * poolingStride + conv_cmd_ptr->pool_size_y - 1 - conv_cmd_ptr->pad_y;
                info.vx_nn_tp_cmd_info.inTileXSize = outTileXSize * poolingStride + conv_cmd_ptr->pool_size_x - poolingStride;
                info.vx_nn_tp_cmd_info.inTileYSize = outTileYSize * poolingStride + conv_cmd_ptr->pool_size_y - poolingStride;
                info.vx_nn_tp_cmd_info.inTileXInc = outTileXSize * poolingStride;
                info.vx_nn_tp_cmd_info.inTileYInc = outTileYSize * poolingStride;
                if (poolingStride == 1)
                {
                    info.vx_nn_tp_cmd_info.aluHorzProcessing = 0x1;
                    info.vx_nn_tp_cmd_info.aluHorzProcCount = conv_cmd_ptr->pool_size_x - 1;
                    info.vx_nn_tp_cmd_info.aluHorzProcStride = 0;
                    info.vx_nn_tp_cmd_info.aluVertProcessing = 0x1;
                    info.vx_nn_tp_cmd_info.aluVertProcCount = conv_cmd_ptr->pool_size_y - 1;
                    info.vx_nn_tp_cmd_info.aluVertProcStride = 0;
                }
                else
                {
                    info.vx_nn_tp_cmd_info.aluHorzProcessing = 0x3;
                    info.vx_nn_tp_cmd_info.aluHorzProcCount = conv_cmd_ptr->pool_size_x - 1;
                    info.vx_nn_tp_cmd_info.aluHorzProcStride = conv_cmd_ptr->pool_size_x == 2 ?
                        0x0 : 0x1;
                    info.vx_nn_tp_cmd_info.aluVertProcessing = 0x3;
                    info.vx_nn_tp_cmd_info.aluVertProcCount = conv_cmd_ptr->pool_size_y - 1;
                    info.vx_nn_tp_cmd_info.aluVertProcStride = conv_cmd_ptr->pool_size_y == 2 ?
                        0x0 : 0x1;
                }
                info.vx_nn_tp_cmd_info.aluFilterPwlSwap = 1;
            }
            else
            {
                outTileXSize = 64;
                outTileYSize = 16;
                outXSize = inXSize;
                outYSize = inYSize;

                info.vx_nn_tp_cmd_info.inWindowXStart = 0;
                info.vx_nn_tp_cmd_info.inWindowYStart = 0;
                info.vx_nn_tp_cmd_info.inWindowXEnd = inXSize - 1;
                info.vx_nn_tp_cmd_info.inWindowYEnd = inYSize - 1;
                info.vx_nn_tp_cmd_info.inTileXSize = outTileXSize;
                info.vx_nn_tp_cmd_info.inTileYSize = outTileYSize;
                info.vx_nn_tp_cmd_info.inTileXInc = outTileXSize;
                info.vx_nn_tp_cmd_info.inTileYInc = outTileYSize ;
                info.vx_nn_tp_cmd_info.aluHorzProcCount = 0;
                info.vx_nn_tp_cmd_info.aluVertProcCount = 0;
                info.vx_nn_tp_cmd_info.aluFilterPwlSwap = 0;
            }

            info.vx_nn_tp_cmd_info.inImageXSize = inXSize;
            info.vx_nn_tp_cmd_info.inImageYSize = inYSize;
            info.vx_nn_tp_cmd_info.inImageZSize = multi_tp ? 1 : inZSize;
            info.vx_nn_tp_cmd_info.inImageStride = inXSize;
            info.vx_nn_tp_cmd_info.inImageSlice = inXSize * inYSize;
            info.vx_nn_tp_cmd_info.inTileSequence = 0x0;
            info.vx_nn_tp_cmd_info.inImageGlobalMem = 1;
            info.vx_nn_tp_cmd_info.inImageBaseAddress = inputBase + index * inXSize * inYSize * inputElemSize;
            info.vx_nn_tp_cmd_info.aluSquarePreshift = 0;
            info.vx_nn_tp_cmd_info.aluSquareEnable = 0;
            info.vx_nn_tp_cmd_info.aluPwlEnable = 1;
            info.vx_nn_tp_cmd_info.aluMultEnable = 0;
            info.vx_nn_tp_cmd_info.aluLoadPwlLUT = 1;
            info.vx_nn_tp_cmd_info.aluLoadPwlLUTAddress = data_buff->memory.physicals[0];
            info.vx_nn_tp_cmd_info.aluLoadPwlLUTGlobalMem = 1;
            info.vx_nn_tp_cmd_info.outBaseAddress = outputBase + index * outXSize * outYSize * outputElemSize;
            info.vx_nn_tp_cmd_info.outGlobalMem  = 1;
            info.vx_nn_tp_cmd_info.outTileSkipAtborder = 0;
            info.vx_nn_tp_cmd_info.outBrickMode  = 0;
            info.vx_nn_tp_cmd_info.outLoop0Inc   = 0;
            info.vx_nn_tp_cmd_info.outLoop0Count = 1;
            info.vx_nn_tp_cmd_info.outLoop1Inc   = 1;
            info.vx_nn_tp_cmd_info.outLoop1Count = 0;
            info.vx_nn_tp_cmd_info.outLoop1Reset = 0x1;
            info.vx_nn_tp_cmd_info.outLoop2Inc   = outXSize;
            info.vx_nn_tp_cmd_info.outLoop2Count = 0;
            info.vx_nn_tp_cmd_info.outLoop2Reset = 0x1;
            info.vx_nn_tp_cmd_info.outLoop3Inc   = outTileXSize;
            info.vx_nn_tp_cmd_info.outLoop3Count = (outXSize + outTileXSize - 1) / outTileXSize;
            info.vx_nn_tp_cmd_info.outLoop3Reset = 0x0;
            info.vx_nn_tp_cmd_info.outLoop4Inc   = outTileYSize * outXSize;
            info.vx_nn_tp_cmd_info.outLoop4Count = (outYSize + outTileYSize - 1) / outTileYSize;
            info.vx_nn_tp_cmd_info.outLoop5Inc   = 0;
            info.vx_nn_tp_cmd_info.outLoop5Count = 1;
            info.vx_nn_tp_cmd_info.outLoop6Inc   = multi_tp ? 0 : outXSize * outYSize;
            info.vx_nn_tp_cmd_info.aluPwlSignSupport = 1;
            info.vx_nn_tp_cmd_info.aluReluEnable = 0;
            info.vx_nn_tp_cmd_info.aluI2FEnable = TENSOR_DATA_SIZE(inputs) == 1 ? 1 : 0;
            info.vx_nn_tp_cmd_info.aluF2IEnable = TENSOR_DATA_SIZE(outputs) == 1 ? 1 : 0;
            info.vx_nn_tp_cmd_info.aluInputPreshift = 0;
            info.vx_nn_tp_cmd_info.aluOutputPostshift = (TENSOR_DATA_SIZE(inputs) == 1 ? TENSOR_POS(inputs) : 0) -
                                                        (TENSOR_DATA_SIZE(outputs) == 1 ? TENSOR_POS(outputs) : 0);
            break;
        }

        case TP_LRN:
        {
            vx_float32  alpha = value_cmd_ptr->f32[0];
            vx_float32  beta = value_cmd_ptr->f32[1];
            vx_uint32   kernel = value_cmd_ptr->u32[0];
            vx_float32  shift = 1.0f;
            vx_uint16 * pwlLUTBase = (vx_uint16 *)data_buff->memory.logicals[0];
            vx_uint16   base, fixed16, baseF16;
            vx_float32  baseF32;
            vx_uint32   baseU32;
            vx_float32  pwlValue;
            vx_float32  ks = value_cmd_ptr->e32[0] == VX_CONVOLUTIONAL_NETWORK_NORM_SAME_MAP ? (vx_float32)(kernel * kernel) : (vx_float32)kernel;
            vx_int32    halfK = kernel/2, preShift = 4, preShiftValue = 1 << (preShift*2);
            vx_float32  expValue = inputElemSize == 1 && outputElemSize == 1 ?
                                   (TENSOR_POS(inputs) > 0 ? 1.0f / (1 << TENSOR_POS(inputs)) : 1 << TENSOR_POS(inputs)) : 1.0f;

            gcmASSERT(pwlLUTBase != VX_NULL);
            gcmASSERT(kernel <= 5);
            gcmASSERT(kernel & 0x1);

            expValue *= value_cmd_ptr->e32[0] == VX_CONVOLUTIONAL_NETWORK_NORM_SAME_MAP ? expValue : 1.0f;
            fixed16 = Fp32toFp16(1.0f);
            for (base = 0; base < 0x20; base++)
            {
                pwlLUTBase[base] = fixed16;
            }
            for (base = 0x20; base < 0x3E0; base++)
            {
                baseF16 = base << 5;
                baseF32 = Fp16toFp32(baseF16);
                pwlValue = shift + alpha * (baseF32 * (vx_float32)preShiftValue / ks) * expValue;
                pwlValue = 1.0f / (vx_float32)pow(pwlValue,beta);
                pwlLUTBase[base] = Fp32toFp16(pwlValue);
            }
            baseU32 = (31 - 15 + 127) << 23;
            baseF32 = *((vx_float32*) &baseU32);
            pwlValue = shift + alpha * (baseF32 * (vx_float32)preShiftValue / ks) * expValue;
            pwlValue = 1.0f / (vx_float32)pow(pwlValue,beta);
            fixed16 = Fp32toFp16(pwlValue);
            for (base = 0x3E0; base < 0x400; base++)
            {
                pwlLUTBase[base] = fixed16;
            }

            info.vx_nn_tp_cmd_info.inImageXSize = inXSize;
            info.vx_nn_tp_cmd_info.inImageYSize = inYSize;
            info.vx_nn_tp_cmd_info.inImageZSize = multi_tp ? 1: inZSize;
            info.vx_nn_tp_cmd_info.inImageStride = inXSize;
            info.vx_nn_tp_cmd_info.inImageSlice = inXSize * inYSize;
            info.vx_nn_tp_cmd_info.inImageBaseAddress = inputBase + index * inXSize * inYSize * inputElemSize;
            info.vx_nn_tp_cmd_info.aluLoadPwlLUTAddress = data_buff->memory.physicals[0];
            info.vx_nn_tp_cmd_info.outBaseAddress = outputBase + index * inXSize * inYSize * outputElemSize;
            info.vx_nn_tp_cmd_info.aluPwlEnable = 1;
            info.vx_nn_tp_cmd_info.aluMultEnable = 1;
            info.vx_nn_tp_cmd_info.aluLoadPwlLUT = 1;
            info.vx_nn_tp_cmd_info.aluLoadPwlLUTGlobalMem = 1;
            info.vx_nn_tp_cmd_info.aluInputPreshift = inputElemSize == 1 ? TENSOR_POS(inputs) : 0;
            info.vx_nn_tp_cmd_info.aluOutputPostshift = outputElemSize == 1 ? TENSOR_POS(outputs) : 0;
            info.vx_nn_tp_cmd_info.aluI2FEnable = TENSOR_DATA_SIZE(inputs) == 1 ? 1 : 0;
            info.vx_nn_tp_cmd_info.aluF2IEnable = TENSOR_DATA_SIZE(outputs) == 1 ? 1 : 0;

            if (value_cmd_ptr->e32[0]  == VX_CONVOLUTIONAL_NETWORK_NORM_SAME_MAP)
            {
                outTileXSize = 64;
                outTileYSize = 16;
                info.vx_nn_tp_cmd_info.inWindowXStart = halfK * (-1);
                info.vx_nn_tp_cmd_info.inWindowYStart = halfK * (-1);
                info.vx_nn_tp_cmd_info.inWindowXEnd = inXSize - 1 + halfK;
                info.vx_nn_tp_cmd_info.inWindowYEnd = inYSize - 1 + halfK;
                info.vx_nn_tp_cmd_info.inTileSequence = 0x0;
                info.vx_nn_tp_cmd_info.inImageGlobalMem = 1;
                info.vx_nn_tp_cmd_info.inTileXSize = outTileXSize + kernel - halfK;
                info.vx_nn_tp_cmd_info.inTileYSize = outTileYSize + kernel - halfK;
                info.vx_nn_tp_cmd_info.inTileXInc = outTileXSize;
                info.vx_nn_tp_cmd_info.inTileYInc = outTileYSize;
                info.vx_nn_tp_cmd_info.aluSquarePreshift = preShift;
                info.vx_nn_tp_cmd_info.aluSquareEnable = 1;
                info.vx_nn_tp_cmd_info.aluHorzProcessing = 0x0;
                info.vx_nn_tp_cmd_info.aluHorzProcCount = kernel - 1;
                info.vx_nn_tp_cmd_info.aluHorzProcStride = 0;
                info.vx_nn_tp_cmd_info.aluVertProcessing = 0x0;
                info.vx_nn_tp_cmd_info.aluVertProcCount = kernel - 1;
                info.vx_nn_tp_cmd_info.aluVertProcStride = 0;
                info.vx_nn_tp_cmd_info.outGlobalMem  = 1;
                info.vx_nn_tp_cmd_info.outTileSkipAtborder = 0;
                info.vx_nn_tp_cmd_info.outBrickMode  = 0;
                info.vx_nn_tp_cmd_info.outLoop0Inc   = 0;
                info.vx_nn_tp_cmd_info.outLoop0Count = 1;
                info.vx_nn_tp_cmd_info.outLoop1Inc   = 1;
                info.vx_nn_tp_cmd_info.outLoop1Count = 0;
                info.vx_nn_tp_cmd_info.outLoop1Reset = 0x1;;
                info.vx_nn_tp_cmd_info.outLoop2Inc   = inXSize;
                info.vx_nn_tp_cmd_info.outLoop2Count = 0;
                info.vx_nn_tp_cmd_info.outLoop2Reset = 0x1;
                info.vx_nn_tp_cmd_info.outLoop3Inc   = outTileXSize;
                info.vx_nn_tp_cmd_info.outLoop3Count = (inXSize + outTileXSize - 1) / outTileXSize;
                info.vx_nn_tp_cmd_info.outLoop3Reset = 0x0;
                info.vx_nn_tp_cmd_info.outLoop4Inc   = outTileYSize * inXSize;
                info.vx_nn_tp_cmd_info.outLoop4Count = (inYSize + outTileYSize - 1) / outTileYSize;
                info.vx_nn_tp_cmd_info.outLoop5Inc   = 0;
                info.vx_nn_tp_cmd_info.outLoop5Count = 1;
                info.vx_nn_tp_cmd_info.outLoop6Inc   = multi_tp ? 0 : inXSize * inYSize;
                info.vx_nn_tp_cmd_info.aluFilterPwlSwap = 0;
                info.vx_nn_tp_cmd_info.aluPwlSignSupport = 0;
                info.vx_nn_tp_cmd_info.aluReluEnable = 0;
            }
            else /* VX_CONVOLUTIONAL_NETWORK_NORM_ACROSS_MAPS */
            {
                gcmASSERT(!multi_tp);
                gcmASSERT(!index);

                switch (context->options.typeTPFunc[TP_LRN])
                {
                    case 1: /* Convert to 2D same map LRN. */
                        if (inXSize * inYSize < 65536)
                        {
                            outTileXSize = 32;
                            outTileYSize = inZSize;
                            info.vx_nn_tp_cmd_info.inImageXSize = inXSize * inYSize;
                            info.vx_nn_tp_cmd_info.inImageYSize = inZSize;
                            info.vx_nn_tp_cmd_info.inImageZSize = 1;
                            info.vx_nn_tp_cmd_info.inImageStride = inXSize * inYSize;
                            info.vx_nn_tp_cmd_info.inImageSlice = inXSize * inYSize * inZSize;
                            info.vx_nn_tp_cmd_info.inWindowXStart = 0;
                            info.vx_nn_tp_cmd_info.inWindowYStart = halfK * (-1);
                            info.vx_nn_tp_cmd_info.inWindowXEnd = inXSize * inYSize - 1;
                            info.vx_nn_tp_cmd_info.inWindowYEnd = inZSize - 1 + halfK;
                            info.vx_nn_tp_cmd_info.inTileSequence = 0x0;
                            info.vx_nn_tp_cmd_info.inImageGlobalMem = 1;
                            info.vx_nn_tp_cmd_info.inTileXSize = outTileXSize;
                            info.vx_nn_tp_cmd_info.inTileYSize = outTileYSize + kernel - 1;
                            info.vx_nn_tp_cmd_info.inTileXInc = outTileXSize;
                            info.vx_nn_tp_cmd_info.inTileYInc = outTileYSize;
                            info.vx_nn_tp_cmd_info.aluSquarePreshift = preShift;
                            info.vx_nn_tp_cmd_info.aluSquareEnable = 1;
                            info.vx_nn_tp_cmd_info.aluHorzProcessing = 0x0;
                            info.vx_nn_tp_cmd_info.aluHorzProcCount = 0;
                            info.vx_nn_tp_cmd_info.aluHorzProcStride = 0;
                            info.vx_nn_tp_cmd_info.aluVertProcessing = 0x0;
                            info.vx_nn_tp_cmd_info.aluVertProcCount = kernel - 1;
                            info.vx_nn_tp_cmd_info.aluVertProcStride = 0;
                            info.vx_nn_tp_cmd_info.aluZFilterMode = 0;
                            info.vx_nn_tp_cmd_info.aluZFilterStartOverfetch = 0;
                            info.vx_nn_tp_cmd_info.aluZFilterEndOverfetch = 0;
                            info.vx_nn_tp_cmd_info.outGlobalMem  = 1;
                            info.vx_nn_tp_cmd_info.outTileSkipAtborder = 0;
                            info.vx_nn_tp_cmd_info.outBrickMode  = 0;
                            info.vx_nn_tp_cmd_info.outLoop0Inc   = 0;
                            info.vx_nn_tp_cmd_info.outLoop0Count = 1;
                            info.vx_nn_tp_cmd_info.outLoop1Inc   = 1;
                            info.vx_nn_tp_cmd_info.outLoop1Count = 0;
                            info.vx_nn_tp_cmd_info.outLoop1Reset = 0x1;
                            info.vx_nn_tp_cmd_info.outLoop2Inc   = inXSize * inYSize;
                            info.vx_nn_tp_cmd_info.outLoop2Count = 0;
                            info.vx_nn_tp_cmd_info.outLoop2Reset = 0x1;
                            info.vx_nn_tp_cmd_info.outLoop3Inc   = outTileXSize;
                            info.vx_nn_tp_cmd_info.outLoop3Count = (inXSize * inYSize + outTileXSize - 1) / outTileXSize;
                            info.vx_nn_tp_cmd_info.outLoop3Reset = 0x0;
                            info.vx_nn_tp_cmd_info.outLoop4Inc   = outTileYSize * inXSize;
                            info.vx_nn_tp_cmd_info.outLoop4Count = (inZSize + outTileYSize - 1) / outTileYSize;
                            info.vx_nn_tp_cmd_info.outLoop5Inc   = 0;
                            info.vx_nn_tp_cmd_info.outLoop5Count = 1;
                            info.vx_nn_tp_cmd_info.outLoop6Inc   = 0;
                            info.vx_nn_tp_cmd_info.aluFilterPwlSwap = 0;
                            info.vx_nn_tp_cmd_info.aluPwlSignSupport = 0;
                            info.vx_nn_tp_cmd_info.aluReluEnable = 0;
                            break;
                        }

                    case 0: /* 3D z-filter. */
                    default:
                        outTileXSize = 1;
                        outTileYSize = 1;
                        info.vx_nn_tp_cmd_info.inWindowXStart = 0;
                        info.vx_nn_tp_cmd_info.inWindowYStart = 0;
                        info.vx_nn_tp_cmd_info.inWindowXEnd = inXSize - 1;
                        info.vx_nn_tp_cmd_info.inWindowYEnd = inYSize - 1;
                        info.vx_nn_tp_cmd_info.inTileSequence = 0x1;
                        info.vx_nn_tp_cmd_info.inImageGlobalMem = 1;
                        info.vx_nn_tp_cmd_info.inTileXSize = outTileXSize;
                        info.vx_nn_tp_cmd_info.inTileYSize = outTileYSize;
                        info.vx_nn_tp_cmd_info.inTileXInc = outTileXSize;
                        info.vx_nn_tp_cmd_info.inTileYInc = outTileYSize;
                        info.vx_nn_tp_cmd_info.aluSquarePreshift = preShift;
                        info.vx_nn_tp_cmd_info.aluSquareEnable = 1;
                        info.vx_nn_tp_cmd_info.aluHorzProcessing = 0x0;
                        info.vx_nn_tp_cmd_info.aluHorzProcCount = (kernel < 3) ? (kernel - 1) : 2;
                        info.vx_nn_tp_cmd_info.aluHorzProcStride = 0;
                        info.vx_nn_tp_cmd_info.aluVertProcessing = (kernel < 4) ? 0x0 : 0x2;
                        info.vx_nn_tp_cmd_info.aluVertProcCount = (kernel < 4) ? 0 : kernel - 3;
                        info.vx_nn_tp_cmd_info.aluVertProcStride = 0;
                        info.vx_nn_tp_cmd_info.aluZFilterMode = 1;
                        info.vx_nn_tp_cmd_info.aluZFilterStartOverfetch = halfK;
                        info.vx_nn_tp_cmd_info.aluZFilterEndOverfetch = halfK;
                        info.vx_nn_tp_cmd_info.outGlobalMem  = 1;
                        info.vx_nn_tp_cmd_info.outTileSkipAtborder = 0;
                        info.vx_nn_tp_cmd_info.outBrickMode  = 0;
                        info.vx_nn_tp_cmd_info.outLoop0Inc   = inXSize * inYSize;
                        info.vx_nn_tp_cmd_info.outLoop0Count = inZSize;
                        info.vx_nn_tp_cmd_info.outLoop1Inc   = 1;
                        info.vx_nn_tp_cmd_info.outLoop1Count = inXSize;
                        info.vx_nn_tp_cmd_info.outLoop1Reset = 0;
                        info.vx_nn_tp_cmd_info.outLoop2Inc   = inXSize;
                        info.vx_nn_tp_cmd_info.outLoop2Count = inYSize;
                        info.vx_nn_tp_cmd_info.outLoop2Reset = 0;
                        info.vx_nn_tp_cmd_info.outLoop3Inc   = 0;
                        info.vx_nn_tp_cmd_info.outLoop3Count = 1;
                        info.vx_nn_tp_cmd_info.outLoop3Reset = 0;
                        info.vx_nn_tp_cmd_info.outLoop4Inc   = 0;
                        info.vx_nn_tp_cmd_info.outLoop4Count = 1;
                        info.vx_nn_tp_cmd_info.outLoop5Inc   = 0;
                        info.vx_nn_tp_cmd_info.outLoop5Count = 1;
                        info.vx_nn_tp_cmd_info.outLoop6Inc   = 0;
                        info.vx_nn_tp_cmd_info.aluFilterPwlSwap = 0;
                        info.vx_nn_tp_cmd_info.aluPwlSignSupport = 0;
                        info.vx_nn_tp_cmd_info.aluReluEnable = 0;
                        break;
                }
            }
            break;
        }

        case TP_ROI_POOLING:
        {
            vx_uint32 poolWidth, poolHeight, roiNum, tmpBaseAddr;
            vx_uint32 maxPoolSize, poolXSize, poolYSize, poolZSize;
            vx_float32 scale;

            scale = value_cmd_ptr->f32[0];
            poolWidth = value_cmd_ptr->u32[0];
            poolHeight = value_cmd_ptr->u32[1];
            maxPoolSize = value_cmd_ptr->u32[2];
            poolXSize = value_cmd_ptr->u32[3];
            poolYSize = value_cmd_ptr->u32[4];
            poolZSize = value_cmd_ptr->u32[5];
            tmpBaseAddr = value_cmd_ptr->u32[6];
            roiNum = TENSOR_VIEW_SIZE_INDEX(outputs, 3);
            outTileXSize = inXSize;
            outTileYSize = 16;

            info.vx_nn_tp_cmd_info.aluSquarePreshift = 0;
            info.vx_nn_tp_cmd_info.aluSquareEnable = 0;
            info.vx_nn_tp_cmd_info.aluFilterPwlSwap = 0;
            info.vx_nn_tp_cmd_info.aluPwlSignSupport = 0;
            info.vx_nn_tp_cmd_info.aluPwlEnable = 0;
            info.vx_nn_tp_cmd_info.aluMultEnable = 0;
            info.vx_nn_tp_cmd_info.aluLoadPwlLUT = 0;
            info.vx_nn_tp_cmd_info.aluLoadPwlLUTAddress = 0;
            info.vx_nn_tp_cmd_info.aluLoadPwlLUTGlobalMem = 1;
            info.vx_nn_tp_cmd_info.outGlobalMem  = 1;
            info.vx_nn_tp_cmd_info.outBrickMode  = 0;
            info.vx_nn_tp_cmd_info.aluI2FEnable = TENSOR_DATA_SIZE(inputs) == 1 ? 1 : 0;
            info.vx_nn_tp_cmd_info.aluF2IEnable = TENSOR_DATA_SIZE(outputs) == 1 ? 1 : 0;
            info.vx_nn_tp_cmd_info.aluInputPreshift   = 0;
            info.vx_nn_tp_cmd_info.aluOutputPostshift = (TENSOR_DATA_SIZE(inputs) == 1 ? TENSOR_POS(inputs) : 0) -
                                                        (TENSOR_DATA_SIZE(outputs) == 1 ? TENSOR_POS(outputs) : 0);

            if (value_cmd_ptr->e32[0] == 0)
            {
                info.vx_nn_tp_cmd_info.inImageXSize = inXSize;
                info.vx_nn_tp_cmd_info.inImageYSize = inYSize;
                info.vx_nn_tp_cmd_info.inImageZSize = inZSize;
                info.vx_nn_tp_cmd_info.inImageStride = inXSize;
                info.vx_nn_tp_cmd_info.inImageSlice = inXSize * inYSize;
                info.vx_nn_tp_cmd_info.inWindowXStart = 0;
                info.vx_nn_tp_cmd_info.inWindowYStart = 0;
                info.vx_nn_tp_cmd_info.inWindowXEnd = inXSize - 1 + maxPoolSize - 1;
                info.vx_nn_tp_cmd_info.inWindowYEnd = inYSize - 1;
                info.vx_nn_tp_cmd_info.inTileSequence = 0x0;
                info.vx_nn_tp_cmd_info.inImageGlobalMem = 1;
                info.vx_nn_tp_cmd_info.inImageBaseAddress = inputBase;
                info.vx_nn_tp_cmd_info.inTileListAddress = 0;
                info.vx_nn_tp_cmd_info.inTileListGlobalMem = 1;
                info.vx_nn_tp_cmd_info.outTileSkipAtborder = 1;
                info.vx_nn_tp_cmd_info.inTileXSize = outTileXSize;
                info.vx_nn_tp_cmd_info.inTileYSize = outTileYSize;
                info.vx_nn_tp_cmd_info.inTileXInc = 1;
                info.vx_nn_tp_cmd_info.inTileYInc = outTileYSize;
                info.vx_nn_tp_cmd_info.aluHorzProcessing = 0x2;
                info.vx_nn_tp_cmd_info.aluHorzProcCount = maxPoolSize - 1;
                info.vx_nn_tp_cmd_info.aluHorzProcStride = 0;
                info.vx_nn_tp_cmd_info.aluVertProcessing = 0;
                info.vx_nn_tp_cmd_info.aluVertProcCount = 0;
                info.vx_nn_tp_cmd_info.aluVertProcStride = 0;
                info.vx_nn_tp_cmd_info.outBaseAddress = tmpBaseAddr;
                info.vx_nn_tp_cmd_info.outLoop0Inc   = poolXSize * poolYSize * poolZSize;
                info.vx_nn_tp_cmd_info.outLoop0Count = maxPoolSize;
                info.vx_nn_tp_cmd_info.outLoop1Inc   = poolYSize * maxPoolSize;
                info.vx_nn_tp_cmd_info.outLoop1Count = 0;
                info.vx_nn_tp_cmd_info.outLoop1Reset = 0x1;
                info.vx_nn_tp_cmd_info.outLoop2Inc   = 1;
                info.vx_nn_tp_cmd_info.outLoop2Count = 0;
                info.vx_nn_tp_cmd_info.outLoop2Reset = 0x1;
                info.vx_nn_tp_cmd_info.outLoop3Inc   = poolYSize;
                info.vx_nn_tp_cmd_info.outLoop3Count = maxPoolSize;
                info.vx_nn_tp_cmd_info.outLoop3Reset = 0x0;
                info.vx_nn_tp_cmd_info.outLoop4Inc   = outTileYSize;
                info.vx_nn_tp_cmd_info.outLoop4Count = (inYSize + outTileYSize - 1) / outTileYSize;
                info.vx_nn_tp_cmd_info.outLoop5Inc   = 0;
                info.vx_nn_tp_cmd_info.outLoop5Count = 1;
                info.vx_nn_tp_cmd_info.outLoop6Inc   = poolXSize * poolYSize;
            }
            else if (value_cmd_ptr->e32[0] == 1)
            {
                vx_tp_roi_pool* roiList;
                vx_tensor roiInputTensor = (vx_tensor)other_tensor;
                vx_uint8 *roiInputLgc;
                vx_uint32 i, proposalsInterleaved, zTogether, stride, offset;
                vx_enum roiFormat = TENSOR_DATA_TYPE(roiInputTensor);
                vx_enum roiRounding = TENSOR_ROUNDING_MODE(roiInputTensor);
                vx_uint8 roiFixPos = TENSOR_POS(roiInputTensor);

                gcmASSERT(roiInputTensor != VX_NULL);
                gcmASSERT(data_buff != VX_NULL);

                roiList = (vx_tp_roi_pool*)data_buff->memory.logicals[0];
                vxoTensor_GetTensorViewMemory(roiInputTensor, (gctPOINTER*)&roiInputLgc, VX_NULL);
                stride = TENSOR_VIEW_SIZE_INDEX(roiInputTensor, 2);
                offset = stride == 5 ? 1 : 0;
                proposalsInterleaved = 1;
                zTogether = 1;

                for (i = 0; i < roiNum; i++, roiInputLgc += stride*TENSOR_DATA_SIZE(roiInputTensor), roiList++)
                {
                    vx_uint32 roi_start_w = (vx_int32)vxnneRound((vx_float32)vxnneGetData(roiFormat, offset, roiInputLgc, roiFixPos) * scale, roiRounding);
                    vx_uint32 roi_start_h = (vx_int32)vxnneRound((vx_float32)vxnneGetData(roiFormat, offset + 1, roiInputLgc, roiFixPos) * scale, roiRounding);
                    vx_uint32 roi_end_w   = (vx_int32)vxnneRound((vx_float32)vxnneGetData(roiFormat, offset + 2, roiInputLgc, roiFixPos) * scale, roiRounding);
                    vx_uint32 roi_end_h   = (vx_int32)vxnneRound((vx_float32)vxnneGetData(roiFormat, offset + 3, roiInputLgc, roiFixPos) * scale, roiRounding);

                    vx_uint32 roi_width   = (vx_uint32)gcmMAX(roi_end_w - roi_start_w + 1, 1);
                    vx_uint32 roi_height  = (vx_uint32)gcmMAX(roi_end_h - roi_start_h + 1, 1);

                    roiList->xcoord = roi_start_w;
                    roiList->ycoord = roi_start_h;
                    roiList->poolingHInc = (vx_uint32)vxnneRound((vx_float32)roi_width  * 256.0f / poolWidth, roiRounding);
                    roiList->poolingVInc = (vx_uint32)vxnneRound((vx_float32)roi_height * 256.0f / poolHeight, roiRounding);
                    roiList->last = 0;
                }
                roiList--;
                roiList->last = 1;

                info.vx_nn_tp_cmd_info.inImageXSize = inYSize;
                info.vx_nn_tp_cmd_info.inImageYSize = inXSize;
                info.vx_nn_tp_cmd_info.inImageZSize = inZSize;
                info.vx_nn_tp_cmd_info.inImageStride = poolYSize;
                info.vx_nn_tp_cmd_info.inImageSlice = poolXSize * poolYSize;
                info.vx_nn_tp_cmd_info.inWindowXStart = 0;
                info.vx_nn_tp_cmd_info.inWindowYStart = 0;
                info.vx_nn_tp_cmd_info.inWindowXEnd = inYSize - 1 + maxPoolSize - 1;
                info.vx_nn_tp_cmd_info.inWindowYEnd = inXSize - 1;
                info.vx_nn_tp_cmd_info.inTileSequence = 0x2;
                info.vx_nn_tp_cmd_info.inImageGlobalMem = 1;
                info.vx_nn_tp_cmd_info.inImageBaseAddress = tmpBaseAddr;
                info.vx_nn_tp_cmd_info.inTileListGlobalMem = 1;
                info.vx_nn_tp_cmd_info.outTileSkipAtborder = 0;
                info.vx_nn_tp_cmd_info.inTileListAddress = data_buff->memory.physicals[0];
                info.vx_nn_tp_cmd_info.inTileXSize = poolHeight;
                info.vx_nn_tp_cmd_info.inTileYSize = poolWidth;
                info.vx_nn_tp_cmd_info.inTileXInc = zTogether;
                info.vx_nn_tp_cmd_info.inTileYInc = 0;
                info.vx_nn_tp_cmd_info.aluHorzProcessing = 0x3;
                info.vx_nn_tp_cmd_info.aluHorzProcCount = 1;
                info.vx_nn_tp_cmd_info.aluHorzProcStride = 0;
                info.vx_nn_tp_cmd_info.aluVertProcessing = 0;
                info.vx_nn_tp_cmd_info.aluVertProcCount = 0;
                info.vx_nn_tp_cmd_info.aluVertProcStride = 0;
                info.vx_nn_tp_cmd_info.outBaseAddress = outputBase;
                info.vx_nn_tp_cmd_info.outLoop0Inc   = poolWidth * proposalsInterleaved;
                info.vx_nn_tp_cmd_info.outLoop0Count = poolHeight;
                info.vx_nn_tp_cmd_info.outLoop1Inc   = proposalsInterleaved;
                info.vx_nn_tp_cmd_info.outLoop1Count = poolWidth;
                info.vx_nn_tp_cmd_info.outLoop1Reset = 0x0;
                info.vx_nn_tp_cmd_info.outLoop2Inc   = poolWidth * poolHeight * proposalsInterleaved;
                info.vx_nn_tp_cmd_info.outLoop2Count = zTogether;
                info.vx_nn_tp_cmd_info.outLoop2Reset = 0x0;
                info.vx_nn_tp_cmd_info.outLoop3Inc   = 1;
                info.vx_nn_tp_cmd_info.outLoop3Count = proposalsInterleaved;
                info.vx_nn_tp_cmd_info.outLoop3Reset = 0x0;
                info.vx_nn_tp_cmd_info.outLoop4Inc   = poolWidth * poolHeight * inZSize;
                info.vx_nn_tp_cmd_info.outLoop4Count = roiNum;
                info.vx_nn_tp_cmd_info.outLoop5Inc   = 0;
                info.vx_nn_tp_cmd_info.outLoop5Count = 1;
                info.vx_nn_tp_cmd_info.outLoop6Inc   = poolWidth * poolHeight * proposalsInterleaved * zTogether;
            }
            break;
        }

        default:
            break;
    }

    info.vx_nn_tp_cmd_info.inImageDataType = TENSOR_DATA_TYPE(inputs) == VX_TYPE_INT8 ? 0x2 : 0x1;
    info.vx_nn_tp_cmd_info.outImageDataType = TENSOR_DATA_TYPE(outputs) == VX_TYPE_INT8 ? 0x2 : 0x1;
    info.vx_nn_tp_cmd_info.floatRoundingMode = 0x1;
    info.vx_nn_tp_cmd_info.integeroundingMode = 0x1;
    info.vx_nn_tp_cmd_info.last = (last ? 1 : 0);

    cmd_buf_ptr = (vx_uint32*)(cmd_buff->memory.logicals[0] + TP_COMMAND_SIZE * index);

    gcoVX_ProgrammCrossEngine((void*)&info, gcvVX_ACCELERATOR_TP, VX_NULL, &cmd_buf_ptr);
}

void vxoNNExternsionDoReshuffle(
    vx_tensor inputs,
    vx_tensor outputs,
    vx_uint32 padXLeft,
    vx_uint32 padXRight,
    vx_uint32 padYTop,
    vx_uint32 padYBottom,
    vx_enum   padMode,
    void*     padConst,
    vx_uint32 strideX,
    vx_uint32 strideY)
{
    vx_uint32 x, y, z, w;
    vx_uint32 srcWidth, srcHeight;
    vx_uint32 dstWidth, dstHeight, dstDepth;
    vx_uint32 srcXSize, srcYSize;
    vx_uint32 dstXSize, dstYSize, dstZSize, dstWSize;
    vx_uint32 elementSize;
    void *srcDataAddr;
    void *dstDataAddr;


    /* do reshuffle*/
    srcWidth  = TENSOR_SIZE_INDEX(inputs, 0);
    srcHeight = TENSOR_SIZE_INDEX(inputs, 1);

    dstWidth  = TENSOR_SIZE_INDEX(outputs, 0) / strideX;
    dstHeight = TENSOR_SIZE_INDEX(outputs, 1) / strideY;
    dstDepth  = TENSOR_SIZE_INDEX(outputs, 2) * strideX * strideY;

    srcXSize = srcWidth;
    srcYSize = srcHeight;

    dstXSize = dstWidth;
    dstYSize = dstHeight;
    dstZSize = dstDepth;
    dstWSize = 1;

    elementSize = (vx_uint32)vxDataType_GetSize(TENSOR_DATA_TYPE(inputs));

    vxoTensor_GetTensorViewMemory(inputs, &srcDataAddr, VX_NULL);
    vxoTensor_GetTensorViewMemory(outputs, &dstDataAddr, VX_NULL);

    for (w = 0;  w < dstWSize; w++)
    {
        for (z = 0; z < dstZSize; z++)
        {
            for (y = 0; y < dstYSize; y++)
            {
                for (x = 0; x < dstXSize; x++)
                {
                    vx_uint8 *srcData, *dstData;
                    vx_uint32 srcPosX, srcPosY, srcPosZ;

                    dstData = (vx_uint8*)dstDataAddr + (x + y * dstXSize + z * dstXSize * dstYSize) * elementSize;

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
                            if (padConst == VX_NULL)
                            {
                                memset(dstData, 0, elementSize);
                            }
                            else
                            {
                                memcpy(dstData, padConst, elementSize);
                            }
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
                            memset(dstData, 0, elementSize);
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
    default:
        return "unkown operation target";
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

vx_float64 vxnneGetData64(vx_enum format, vx_int32 index, vx_uint8_ptr src, vx_uint8 fixPointPos)
{
    vx_float64 value = 0;
    vx_int8 fpPos = (vx_int8)fixPointPos;

    switch(format)
    {
    case VX_TYPE_INT8:
        {
            vx_int8_ptr data_ptr = (vx_int8*)src;
            value = Int8toFp32(data_ptr[index], fpPos);
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
    default:
        gcmPRINT("Not support format :%d\n", format);
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
        gcmPRINT("Not support format :%d\n", format);
        break;
    }

    return value;
}

vx_float64 vxnneWarp(vx_float64 data, vx_enum format)
{
    vx_float64 value = 0;

    switch(format)
    {
    case VX_TYPE_FLOAT32:
        value = (vx_float32)data;
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
        gcmPRINT("Not support format :%d\n", format);
        break;
    }

    return value;
}

vx_status eltwise(
    vx_uint8_ptr input1_ptr,
    vx_uint8 input1FixPointPos,
    vx_enum input1Format,
    vx_uint8_ptr input2_ptr,
    vx_uint8 input2FixPointPos,
    vx_enum input2Format,
    vx_int32 size,
    vx_float32 scale,
    vx_enum overflow,
    vx_enum scaleRounding,
    vx_enum operation,
    vx_uint8_ptr output_ptr,
    vx_uint8 outputFixPointPos,
    vx_enum outputRoundingMode,
    vx_enum outputFormat)
{
    vx_int32 i = 0;

    for (i = 0; i < size; i ++)
    {
        vx_float64 input1_value = vxnneGetData64(input1Format, i, input1_ptr, input1FixPointPos);
        vx_float64 input2_value = vxnneGetData64(input2Format, i, input2_ptr, input2FixPointPos);
        vx_float64 value = 0;

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
            gcmPRINT("Not support format :%d\n", outputFormat);
            break;
        }

        vxnneSaveData((vx_type_e)outputFormat, i, value, output_ptr, outputFixPointPos, outputRoundingMode);
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
    vx_uint32 i,j,count = 0;
    vx_char_ptr is_dead = (vx_char_ptr)malloc(num_boxes * sizeof(vx_char));
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
    free(is_dead);
}

vx_status vxnnePoolingMax(
    vx_uint8_ptr src,
    vx_uint8 srcFixPointPos,
    vx_type_e srcFormat,
    vx_int32 input_width,
    vx_int32 input_height,
    vx_int32 depth,
    vx_int32_ptr output_width,
    vx_int32_ptr output_height,
    vx_int32 stride,
    vx_int32 kernel_size,
    vx_uint32 padXLeft,
    vx_uint32 padXRight,
    vx_uint32 padYTop,
    vx_uint32 padYBottom,
    vx_enum rounding,
    vx_uint8_ptr dst,
    vx_uint8 dstFixPointPos,
    vx_enum dstRoundingMode,
    vx_type_e dstFormat
    )
{
    vx_uint8_ptr data = 0;
    vx_uint8_ptr data_d = 0;
    vx_int32 i = 0, j = 0, p = 0;

    vx_int32 width = input_width;
    vx_int32 height = input_height;
    vx_int32 depth_v = depth;
    vx_int32 stride_v = stride;
    vx_int32 kernel_v = kernel_size;
    vx_int32 width_o = (vx_uint32)(vxoNNExternsionConvlutionRound(((vx_float32)(width + padXLeft + padXRight - kernel_v)/stride_v + 1), rounding));
    vx_int32 height_o = (vx_uint32)(vxoNNExternsionConvlutionRound(((vx_float32)(height + padYTop + padYBottom - kernel_v)/stride_v + 1), rounding));

    *output_width = width_o;
    *output_height = height_o;

    data = src;
    data_d = dst;

    for (p = 0; p < depth_v; p ++)
    {
        for (j = 0; j < height_o; j ++)
        {
            for (i = 0; i < width_o; i ++)
            {
                vx_int32 pad_h = padYTop, pad_w = padXLeft;
                vx_int32 hstart = j * stride_v - pad_h;
                vx_int32 wstart = i * stride_v - pad_w;
                vx_int32 hend = gcmMIN(hstart + kernel_v, height);
                vx_int32 wend = gcmMIN(wstart + kernel_v, width);
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

                        vx_float32 d = vxnneGetData(srcFormat, index, (vx_uint8_ptr)data, srcFixPointPos);

                        if (d > d_f32)
                            d_f32 = d;

                    }
                }

                vxnneSaveData(dstFormat, pool_index, d_f32, data_d, dstFixPointPos, dstRoundingMode);
            }
        }

        data += width * height * vxnneGetTypeSize(srcFormat);
        data_d += width_o * height_o * vxnneGetTypeSize(dstFormat);
    }

    return VX_SUCCESS;
}

vx_status vxnnePoolingAvg(
    vx_uint8_ptr src,
    vx_uint8 srcFixPointPos,
    vx_type_e srcFormat,
    vx_int32 input_width,
    vx_int32 input_height,
    vx_int32 depth,
    vx_int32_ptr output_width,
    vx_int32_ptr output_height,
    vx_int32 stride,
    vx_int32 kernel_size,
    vx_uint32 padXLeft,
    vx_uint32 padXRight,
    vx_uint32 padYTop,
    vx_uint32 padYBottom,
    vx_enum rounding,
    vx_uint8_ptr dst,
    vx_uint8 dstFixPointPos,
    vx_enum dstRoundingMode,
    vx_type_e dstFormat
    )
{
    vx_uint8_ptr data = 0;
    vx_uint8_ptr data_d = 0;
    vx_int32 i = 0, j = 0, p = 0;

    vx_int32 width = input_width;
    vx_int32 height = input_height;
    vx_int32 depth_v = depth;
    vx_int32 stride_v = stride;
    vx_int32 kernel_v = kernel_size;
    vx_int32 width_o = (vx_uint32)(vxoNNExternsionConvlutionRound(((vx_float32)(width + padXLeft + padXRight - kernel_v)/stride_v), rounding) + 1);
    vx_int32 height_o = (vx_uint32)(vxoNNExternsionConvlutionRound(((vx_float32)(height + padYTop + padYBottom - kernel_v)/stride_v), rounding) + 1);

    *output_width = width_o;
    *output_height = height_o;

    data = src;
    data_d = dst;

    for (p = 0; p < depth_v; p ++)
    {
        for (j = 0; j < height_o; j ++)
        {
            for (i = 0; i < width_o; i ++)
            {
                vx_int32 pad_h = padYTop, pad_w = padXLeft;
                vx_int32 hstart = j * stride_v - pad_h;
                vx_int32 wstart = i * stride_v - pad_w;
                vx_int32 hend = gcmMIN(hstart + kernel_v, height);
                vx_int32 wend = gcmMIN(wstart + kernel_v, width);
                vx_int32 pool_index = 0;
                vx_int32 h, w = 0;
                vx_float32 sum = 0;

                hstart = gcmMAX(hstart, 0);
                wstart = gcmMAX(wstart, 0);

                pool_index = j * (width_o) + i;

                for (h = hstart; h < hend; ++ h)
                {
                    for (w = wstart; w < wend; ++ w)
                    {
                        const vx_int32 index = h * (width) + w;
                        sum += vxnneGetData(srcFormat, index, (vx_uint8_ptr)data, srcFixPointPos);
                    }
                }

                vxnneSaveData(dstFormat, pool_index, sum/(kernel_v * kernel_v), data_d, dstFixPointPos, dstRoundingMode);
            }

        }

        data += width * height * vxnneGetTypeSize(srcFormat);
        data_d += width_o * height_o * vxnneGetTypeSize(dstFormat);
    }

    return VX_SUCCESS;
}

vx_status vxnnePoolingCpu(
    vx_uint8_ptr src,
    vx_uint8 srcFixPointPos,
    vx_int32 type,
    vx_type_e srcFormat,
    vx_int32 input_width,
    vx_int32 input_height,
    vx_int32 depth,
    vx_int32_ptr output_width,
    vx_int32_ptr output_height,
    vx_int32 stride,
    vx_int32 kernel_size,
    vx_uint32 padXLeft,
    vx_uint32 padXRight,
    vx_uint32 padYTop,
    vx_uint32 padYBottom,
    vx_enum rounding,
    vx_uint8_ptr dst,
    vx_uint8 dstFixPointPos,
    vx_enum dstRoundingMode,
    vx_type_e dstFormat)
{
    switch (type)
    {
        case 1:
            vxnnePoolingMax(src,
                            srcFixPointPos,
                            srcFormat,
                            input_width,
                            input_height,
                            depth,
                            output_width,
                            output_height,
                            stride,
                            kernel_size,
                            padXLeft,
                            padXRight,
                            padYTop,
                            padYBottom,
                            rounding,
                            dst,
                            dstFixPointPos,
                            dstRoundingMode,
                            dstFormat);
        break;
        case 2:
            vxnnePoolingAvg(src,
                            srcFixPointPos,
                            srcFormat,
                            input_width,
                            input_height,
                            depth,
                            output_width,
                            output_height,
                            stride,
                            kernel_size,
                            padXLeft,
                            padXRight,
                            padYTop,
                            padYBottom,
                            rounding,
                            dst,
                            dstFixPointPos,
                            dstRoundingMode,
                            dstFormat);
        break;
    }

    return VX_SUCCESS;
}


