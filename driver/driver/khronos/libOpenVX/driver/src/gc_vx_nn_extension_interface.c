/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
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
#include <gc_vx_nn_extension_interface.h>
#include <gc_vx_internal_node_api.h>
#include "gc_hal_types.h"
#include "anchors.h"

#define F16_EXPONENT_BITS 0x1F
#define F16_EXPONENT_SHIFT 10
#define F16_EXPONENT_BIAS 15
#define F16_MANTISSA_BITS 0x3ff
#define F16_MANTISSA_SHIFT (23 - F16_EXPONENT_SHIFT)
#define F16_MAX_EXPONENT (F16_EXPONENT_BITS << F16_EXPONENT_SHIFT)

vx_int16 F32toF16(vx_float32 val)
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

#if NN_LAYER_C
extern vx_status vx_nnk_pool_nn_layer_cpu(vx_uint8_ptr src, vx_int32 type, vx_type_e format, vx_int32 input_width, vx_int32 input_height, vx_int32 depth,
                                vx_int32_ptr output_width, vx_int32_ptr output_height, vx_int32 stride, vx_int32 kernel_size, vx_int32 pad, vx_uint8_ptr dst);
#endif

typedef struct _vx_nn_reshuffleStruct_s
{
    void *data;
    vx_uint32 xSize;
    vx_uint32 ySize;
    vx_uint32 zSize;
    vx_uint32 wSize;
    vx_type_e type;
}
vx_nn_reshuffle_s;

vx_float32 Fp16toFp32(const vx_int16 in)
{
    vx_int32 t1;
    vx_int32 t2;
    vx_int32 t3;
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

#ifdef WIN32
static vx_float32 round(vx_float32 x)
{
#if defined(_M_X64)
    return (vx_float32) _copysignf(floorf(fabsf(x) + 0.5f), x);
#else
    return (vx_float32) _copysign(floorf(fabsf(x) + 0.5f), x);
#endif
}
#endif

vx_int8 Fp32toInt8(vx_float32 val, vx_int8 fixedPointPos)
{
    vx_int8 result = 0;

    if (fixedPointPos > 0)
    {
        vx_int32 data = (vx_int32) round(val * (vx_float32)(1 << fixedPointPos));
        result = (vx_int8)((data > 127) ? 127 : (data < -128) ? -128 : data);

    }
    else
    {
        vx_int32 data = (vx_int32) round(val * (1.0f / (vx_float32)(1 << -fixedPointPos)));
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

VX_INTERNAL_API vx_float32 vxnneGetDate(vx_type_e format, vx_int32 index, vx_uint8_ptr data, vx_uint8 fixPointPos)
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

VX_INTERNAL_API vx_status vxnneSaveDate(vx_type_e format, vx_int32 index, vx_float64 data, vx_ptr dst_data, vx_uint8 fixedPointPos)
{
    vx_int8 fpPos = (vx_int8)fixedPointPos;

    switch(format)
    {
    case VX_TYPE_INT8:
        {
            vx_int8* dst_data_p = (vx_int8*)dst_data;
            dst_data_p[index] = Fp32toInt8((vx_float32)data, fpPos);
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

VX_PRIVATE_API vx_int32 vxoNNExternsionConvlutionRound(vx_float32 in, vx_enum round_type)
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

VX_PRIVATE_API void
vxoNNExternsionInputOutputArguments(
    vx_uint32                    org_input_x,
    vx_uint32                    org_input_y,
    vx_uint32                    stride_x,
    vx_uint32                    stride_y,
    vx_uint32                    pad_x,
    vx_uint32                    pad_y,
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

    inx = (stride_x > 1) ? (gcmALIGN(org_input_x + 2 * pad_x, stride_x) / stride_x) : org_input_x;
    iny = (stride_y > 1) ? (gcmALIGN(org_input_y + 2 * pad_y, stride_y) / stride_y) : org_input_y;

    outx = vxoNNExternsionConvlutionRound((((vx_float32)(org_input_x + 2 * pad_x - weight_x) / (vx_float32)stride_x) + 1), rounding_type);
    outy = vxoNNExternsionConvlutionRound((((vx_float32)(org_input_y + 2 * pad_x - weight_y) / (vx_float32)stride_y) + 1), rounding_type);

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

        if (pool_size_x == 2 && (outx & 0x1))
        {
            *finalOutput_x = outx - 1;
            *finalOutput_y = outy - 1;
        }
        else if (pool_size_x == 3 && !(outx & 0x1))
        {
            *finalOutput_x = outx + 1;
            *finalOutput_y = outy + 1;

            *finalInput_x = inx < (*finalOutput_x) ? inx + 1 : inx;
            *finalInput_y = iny < (*finalOutput_y) ? iny + 1 : iny;
        }
    }
}

#define FC_Z_MAX 16383
#define FC_ACCEL_THRESHOLD 926720
#define FC_SIZE_MAX 134217728

VX_PRIVATE_API vx_bool
vxoNNExternsionAdjustWeightsBiases(
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

    if (wb->weights_sizes[0] == 1 && wb->weights_sizes[1] == 1 && wb->weights_sizes[2] >= FC_Z_MAX)
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
VX_PRIVATE_API void reshuffleData(vx_nn_reshuffle_s *src, vx_uint32 strideStepX, vx_uint32 strideStepY, vx_nn_reshuffle_s *dst)
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

static vx_uint32 _calcInImageInterleaveMode(vx_uint32 x, vx_uint32 mad_per_core, vx_uint32 kernel_xy)
{
    if (x + kernel_xy - 1 > (mad_per_core + 8) / 2)
        return 1;
    else if (x + kernel_xy - 1 > (mad_per_core + 8) / 4)
        return 2;
    else
        return 4;
}

static vx_uint32 _calcOutImageInterleaveMode(vx_uint32 x, vx_uint32 mad_per_core)
{
    return x > mad_per_core / 2 ? 1 : x > mad_per_core / 4 ? 2 : 4;
}

static vx_uint32 _calcImageInterleaveMode(vx_uint32 x, vx_uint32 mad_per_core, vx_uint32 kxy)
{
    return gcmMIN(_calcInImageInterleaveMode(x, mad_per_core, kxy),
                  _calcOutImageInterleaveMode(x, mad_per_core));
}

static vx_uint32 _calcNumOfKernel(vx_uint32 tile_x, vx_uint32 tile_y, vx_uint32 z, vx_uint32 accu_buf_depth, vx_uint32 cores, vx_uint32 interleave_mode)
{
    vx_uint32 numKernel;

    numKernel = accu_buf_depth * interleave_mode / tile_y;

    return gcmMIN(127, (vx_uint32)gcmMIN(numKernel, ceilf((vx_float32)z / cores)));
}

static vx_uint32 _calcComputeCycleCount(vx_uint32 tile_x, vx_uint32 tile_y,
 vx_uint32 kernel_per_core, vx_uint32 x, vx_uint32 y, vx_uint32 z, vx_uint32 kx,
 vx_uint32 ky, vx_uint32 kz, vx_uint32 mad_per_core, vx_uint32 data_size,
 vx_uint32 dp_amount, vx_float32 non_zero_ratio)
{
    vx_uint32 tmp, pipeLatency, interleaveMode, dpKX, dpAmount;
    vx_uint32 accumCycle, tile3DComputeCycle, bottomTile3DComputeCycle, computeCycle;
    vx_float32 dpNonZeroRatio = 1.0;

    pipeLatency = data_size != 8 ? 6 : dp_amount == 1 ? 4 : 1;

    interleaveMode = _calcImageInterleaveMode(tile_x, mad_per_core, kx);
    accumCycle = (vx_uint32)ceilf((vx_float32)tile_y / interleaveMode);

    if (kx * ky == 1)
    {
        tile3DComputeCycle = (vx_uint32)gcmMAX(ceilf((vx_float32)tile_y / interleaveMode) * kernel_per_core, pipeLatency);
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
        accumCycle = (vx_uint32)ceilf((vx_float32)tmp / interleaveMode);

        if (kx * ky == 1)
        {
            bottomTile3DComputeCycle = (vx_uint32)gcmMAX(ceilf((vx_float32)tmp / interleaveMode) * kernel_per_core, pipeLatency);
        }
        else if (accumCycle == 4)
        {
            bottomTile3DComputeCycle = 4 * kernel_per_core;
        }
        else
        {
            bottomTile3DComputeCycle = (vx_uint32)(gcmMAX(ceilf((vx_float32)tmp / interleaveMode), pipeLatency) * kernel_per_core);
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
    if (dpNonZeroRatio * dpKX > non_zero_ratio * kx)
    {
        dpNonZeroRatio = non_zero_ratio;
        dpKX = kx;
    }

    computeCycle = tile3DComputeCycle * (y / tile_y) + bottomTile3DComputeCycle;
    computeCycle = (vx_uint32)((vx_float32)computeCycle * dpKX * ky * kz * z * dpNonZeroRatio * ceilf((vx_float32)x / tile_x) / kernel_per_core);

    return computeCycle;
}

static vx_float32 _calcKernel4DSingleReadRepeated(vx_uint32 tile_x, vx_uint32 tile_y, vx_uint32 x, vx_uint32 y)
{
    return ceilf((vx_float32)x / tile_x) * ceilf((vx_float32)y / tile_y);
}

static vx_float32 _calcKernel4DSingleReadBW(vx_uint32 kx, vx_uint32 ky, vx_uint32 kz, vx_uint32 z, vx_float32 coef_compress_ratio)
{
    return (vx_float32)kx * ky * kz * z * coef_compress_ratio;
}

static vx_float32 _calcTile3DImageSingleReadRepeated(vx_uint32 z, vx_uint32 kernel_per_core, vx_uint32 cores)
{
    return ceilf((vx_float32)z / (kernel_per_core * cores));
}

static vx_float32 _calcTile3DImageSingleReadBW(vx_uint32 tile_x, vx_uint32 tile_y,
 vx_uint32 kx, vx_uint32 ky, vx_uint32 kz, vx_uint32 x, vx_uint32 y, vx_uint32 inx,
 vx_uint32 iny, vx_uint32 brick_mode, vx_uint32 data_size, vx_float32 image_compress_ratio)
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

static vx_uint32 _calcReadBandWidth(vx_uint32 tile_x, vx_uint32 tile_y,
 vx_uint32 kernel_per_core, vx_uint32 x, vx_uint32 y, vx_uint32 z, vx_uint32 kx,
 vx_uint32 ky, vx_uint32 kz, vx_uint32 inx, vx_uint32 iny, vx_uint32 cores,
 vx_uint32 brick_mode, vx_uint32 data_size, vx_float32 coef_compress_ratio,
 vx_float32 image_compress_ratio, vx_uint32 l2cache_size)
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

static vx_uint32 _calcWriteBandWidth(vx_uint32 tile_x, vx_uint32 tile_y, vx_uint32 x, vx_uint32 y, vx_uint32 z, vx_uint32 data_size, vx_float32 image_compress_ratio, vx_uint32 usc_cache_size, vx_uint32 pooling_stride)
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

VX_PRIVATE_API void calculateFilterPerCore(vx_context context, vx_weights_biases_parameter wb, vx_uint32 zindex)
{

    gctSTRING env = gcvNULL;
    gctUINT32 formulaSwitch = 1;

    gcoOS_GetEnv(gcvNULL, "NN_EXT_FORMULA_OPT", &env);
    if (env)
    {
        formulaSwitch = atoi(env) > 1 ? 0 : atoi(env);
    }

    if (!context->nnConfig.isSet)
    {
        gcoVX_GetNNConfig(&context->nnConfig);
    }

    if (!formulaSwitch)
    {
        /* version <= 0.28 */
        vx_uint32 interleaveModeIn, interleaveModeOut, interleaveMode, idealFilterPerCore = 1;
        vx_uint32 tmpOutSize, tmpCovAccuMode, tmpCovMode, tmpInImageTileXSize;
        vx_uint32 madPerCore, halfMadPerCore, quarterMadPerCore;
        vx_uint32 weightWidth, alignedWeightWidth;

        tmpOutSize = wb->output_sizes[0];
        wb->outImageTileXSize[zindex] = gcmMIN(tmpOutSize, context->nnConfig.nnMadPerCoure);

        madPerCore        = context->nnConfig.nnMadPerCoure;
        halfMadPerCore    = madPerCore / 2;
        quarterMadPerCore = madPerCore / 4;

        interleaveModeOut = (wb->outImageTileXSize[zindex] > halfMadPerCore) ? 1 :
                                 (wb->outImageTileXSize[zindex] > quarterMadPerCore) ? 2 : 4;

        madPerCore        = context->nnConfig.nnMadPerCoure + 8;
        halfMadPerCore    = madPerCore / 2;
        quarterMadPerCore = madPerCore / 4;

        tmpInImageTileXSize = gcmMIN(wb->outImageTileXSize[zindex] + wb->org_weights_sizes[0] - 1, context->nnConfig.nnMadPerCoure);

        interleaveModeIn = (tmpInImageTileXSize > halfMadPerCore) ? 1 :
                            (tmpInImageTileXSize > quarterMadPerCore) ? 2 : 4;

        interleaveMode = gcmMIN(interleaveModeIn, interleaveModeOut);
        tmpCovAccuMode = context->nnConfig.nnAccumBufferDepth * interleaveMode;
        tmpCovMode = context->nnConfig.nnInputBufferDepth * interleaveMode;
        tmpOutSize = wb->output_sizes[1];

        alignedWeightWidth = ((wb->org_weights_sizes[0] % wb->stride == 0) ?
                              wb->org_weights_sizes[0] : (wb->org_weights_sizes[0] + (wb->stride - wb->org_weights_sizes[0] % wb->stride)));
        weightWidth = alignedWeightWidth / wb->stride;

        wb->outImageTileYSize[zindex] = gcmMIN(tmpOutSize, gcmMIN(127, gcmMIN(tmpCovMode - weightWidth + 1, tmpCovAccuMode)));

        idealFilterPerCore = tmpCovAccuMode / wb->outImageTileYSize[zindex];
        wb->kernelsPerCore[zindex] = gcmMIN(127, gcmMIN(wb->zgroup_array[zindex], idealFilterPerCore));

#if CONST_FILTER_PERCORE
        {
#define OUT_TILE_X       0
#define OUT_TILE_Y       1
#define KERNEL_PER_CORE  2
            static vx_uint32 argument_Alexnet[4][3] =
            {
                {55, 4, 14},
                {27, 8, 14},
                {13, 13, 17},
                { 1, 1, 56},
            };

            if (tmpOutSize == 55)
            {
                wb->outImageTileXSize[zindex] = argument_Alexnet[0][OUT_TILE_X];
                wb->outImageTileYSize[zindex] = argument_Alexnet[0][OUT_TILE_Y];
                wb->kernelsPerCore[zindex]    = argument_Alexnet[0][KERNEL_PER_CORE];
            }
            else if(tmpOutSize == 27)
            {
                wb->outImageTileXSize[zindex] = argument_Alexnet[1][OUT_TILE_X];
                wb->outImageTileYSize[zindex] = argument_Alexnet[1][OUT_TILE_Y];
                wb->kernelsPerCore[zindex]    = argument_Alexnet[1][KERNEL_PER_CORE];
            }
            else if (tmpOutSize == 13)
            {
                wb->outImageTileXSize[zindex] = argument_Alexnet[2][OUT_TILE_X];
                wb->outImageTileYSize[zindex] = argument_Alexnet[2][OUT_TILE_Y];
                wb->kernelsPerCore[zindex]    = argument_Alexnet[2][KERNEL_PER_CORE];
            }
            else if (tmpOutSize == 1)
            {
                wb->outImageTileXSize[zindex] = argument_Alexnet[3][OUT_TILE_X];
                wb->outImageTileYSize[zindex] = argument_Alexnet[3][OUT_TILE_Y];
                wb->kernelsPerCore[zindex]    = argument_Alexnet[3][KERNEL_PER_CORE];
            }
        }
#endif
    }
    else if (formulaSwitch == 1)
    {
        /* version 0.29 - 0.34 */
        vx_uint32 numCores, madPerCore, accuBuffDepth, inputBuffDepth, dpAmount, l2CacheSize, brickMode, dataSize, uscCacheSize;
        vx_uint32 inXSize, inYSize, outXSize, outYSize, outZSize, kernelXSize, kernelYSize, kernelZSize, poolingSize, poolingStride;
        vx_int32 xOffSet, yOffSet;
        vx_uint32 tmpMaxOutTileYSize, tmpCovAccuMode, tmpCovMode, interleaveMode, cacheLineMode;
        vx_uint32 x, y, k;
        vx_uint32 newCycleCount, newRDBandWidth, newNCRDBandWidth, newWTBandWidth;
        vx_float32 nonZeroRatio, coefCompressRatio, imageCompressRatio, sustainedBandwidth;

        vxoNNExternsionInputOutputArguments(
            wb->layer_type == VX_CONVOLUTIONAL_NETWORK_FULLYCONNECTED_LAYER ? 1 : wb->input_sizes[0],
            wb->layer_type == VX_CONVOLUTIONAL_NETWORK_FULLYCONNECTED_LAYER ? 1 : wb->input_sizes[1],
            wb->stride, wb->stride,
            wb->pad_x, wb->pad_y,
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
        xOffSet = (-1) * (vx_int32)(outXSize + kernelXSize - 1 - inXSize) / 2;
        yOffSet = (-1) * (vx_int32)(outYSize + kernelYSize - 1 - inYSize) / 2;
        dataSize = 8 * (vx_uint32)vxDataType_GetSize((vx_type_e)wb->weights_data_format);
        poolingSize = gcmMAX(1, wb->pooling_size_x);
        poolingStride = wb->pooling_size_x ? 2 : 1;

        numCores = context->nnConfig.nnCoreCount;
        madPerCore = context->nnConfig.nnMadPerCoure;
        inputBuffDepth = context->nnConfig.nnInputBufferDepth;
        accuBuffDepth  = context->nnConfig.nnAccumBufferDepth;

        /* TODO: Add hw config later. */
#define SUSTAINED_BANDWIDTH                    2.5f
#define NN_BRICK_MODE                          0
#define L2_CACHE_SIZE                          0
#define USC_CACHE_SIZE                         8
        if (context->nnConfig.nnDPAmount == 0)
            context->nnConfig.nnDPAmount = dpAmount = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7) ? 3 : 1;
        else
            dpAmount = context->nnConfig.nnDPAmount;
        context->nnConfig.nnL2CacheSize = l2CacheSize = L2_CACHE_SIZE;
        context->nnConfig.nnUSCCacheSize = uscCacheSize = USC_CACHE_SIZE;
        brickMode = NN_BRICK_MODE;

        gcoOS_GetEnv(gcvNULL, "NN_EXT_SUSTAINED_BW", &env);
        if (env)
        {
            wb->sustained_bandwidth = sustainedBandwidth = (vx_float32) atof(env);
        }
        else
        {
            wb->sustained_bandwidth = sustainedBandwidth = SUSTAINED_BANDWIDTH;
        }

        nonZeroRatio = (vx_float32)(wb->all_count[zindex] - wb->zero_count[zindex]) / wb->all_count[zindex];
        coefCompressRatio = gcmMIN(1.0f, (vx_float32)wb->compressed_size[zindex] / wb->orig_size[zindex]);
        imageCompressRatio = 1.0f;

        /* init to default */
        wb->outImageTileXSize[zindex] = outXSize;
        wb->outImageTileYSize[zindex] = outYSize;
        wb->kernelsPerCore[zindex] = outZSize;
        wb->perfCycleCount[zindex]      = ~0UL;
        wb->perfReadBandWidth[zindex]   = ~0UL;
        wb->perfNCReadBandWidth[zindex] = ~0UL;
        wb->perfWriteBandWidth[zindex]  = ~0UL;

        if (dpAmount == 3 && dataSize == 16) madPerCore /= 2;
        wb->current_mad_per_core = madPerCore;

        for (x = 1; x <= gcmMIN(outXSize, madPerCore); x++)
        {
            if ((poolingSize != 2 && poolingSize != 3) ||
                (poolingSize == 2 && x % 2 == 0) ||
                (poolingSize == 3 && x == outXSize))
            {
                interleaveMode = _calcImageInterleaveMode(x, madPerCore, kernelXSize);

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

                        newRDBandWidth = _calcReadBandWidth(x, y, k, outXSize, outYSize, outZSize, kernelXSize, kernelYSize, kernelZSize, outXSize, outYSize, numCores, brickMode, dataSize, coefCompressRatio, imageCompressRatio, l2CacheSize);
                        newNCRDBandWidth = _calcReadBandWidth(x, y, k, outXSize, outYSize, outZSize, kernelXSize, kernelYSize, kernelZSize, outXSize, outYSize, numCores, brickMode, dataSize, coefCompressRatio, imageCompressRatio, 0);

                        newWTBandWidth = _calcWriteBandWidth(x, y, outXSize, outYSize, outZSize, dataSize, imageCompressRatio, uscCacheSize, poolingStride);

                        newCycleCount = _calcComputeCycleCount(x, y, k, outXSize, outYSize, outZSize, kernelXSize, kernelYSize, kernelZSize, madPerCore, dataSize, dpAmount, nonZeroRatio) / numCores;
                        newCycleCount = gcmMAX(newCycleCount, (vx_uint32)((vx_float32)newRDBandWidth / sustainedBandwidth));

                        if ((newCycleCount < wb->perfCycleCount[zindex])
                           || (newCycleCount == wb->perfCycleCount[zindex] && newRDBandWidth  < wb->perfReadBandWidth[zindex])
                           || (newCycleCount == wb->perfCycleCount[zindex] && newRDBandWidth == wb->perfReadBandWidth[zindex] && newWTBandWidth  < wb->perfWriteBandWidth[zindex])
                           || (newCycleCount == wb->perfCycleCount[zindex] && newRDBandWidth == wb->perfReadBandWidth[zindex] && newWTBandWidth == wb->perfWriteBandWidth[zindex] && newNCRDBandWidth < wb->perfNCReadBandWidth[zindex])
                           )
                        {
                            wb->outImageTileXSize[zindex]   = x;
                            wb->outImageTileYSize[zindex]   = y;
                            wb->kernelsPerCore[zindex]      = k;
                            wb->perfCycleCount[zindex]      = newCycleCount;
                            wb->perfReadBandWidth[zindex]   = newRDBandWidth;
                            wb->perfNCReadBandWidth[zindex] = newNCRDBandWidth;
                            wb->perfWriteBandWidth[zindex]  = newWTBandWidth;
                        }
                    }
                }
            }
        }
    }

}

static vx_uint32 gLayerCount = 0;
VX_PRIVATE_API vx_status
vxoWeightsBiasesParameter_ShowPerformance(
    vx_context context,
    vx_weights_biases_parameter wb
    )
{
    vx_uint32 i;
    vx_size cycleCount=0, rdBandWidth=0, ncRDBandWidth=0, wtBandWidth=0;
    vx_uint32 outX = wb->pooling_size_x == 3 && !(wb->output_sizes[0] & 0x1) ? wb->output_sizes[0] + 1 : wb->output_sizes[0];
    vx_uint32 outY = wb->pooling_size_y == 3 && !(wb->output_sizes[1] & 0x1) ? wb->output_sizes[1] + 1 : wb->output_sizes[1];

    printf("%d %s\n\n", gLayerCount++, wb->layer_type==VX_CONVOLUTIONAL_NETWORK_CONVOLUTION_LAYER?"convolution layer":"fc layer");
    if (wb->zgroup_num)
    {
        printf("NumCores: %d\nMadPerCore: %d\nInBuffDepth: %d\nAccumBufferDepth: %d\nDPAmount: %d\nL2CacheSize: %d\nUSCCacheSize: %d\nDataSize: %d\nSustainBandWidth: %.3f\n\n",
               context->nnConfig.nnCoreCount,
               context->nnConfig.nnMadPerCoure,
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
        printf("nonZeroRatio[%d]: %.4f\ncoefCompression[%d]: %.4f\nimageCompression[%d]: %.4f\nOutImageTileXSize[%d]: %d\nOutImageTileYSize[%d]: %d\nKernelsPerCore[%d]: %d\n",
                i, (vx_float32)(wb->all_count[i] - wb->zero_count[i]) / wb->all_count[i],
                i, gcmMIN(1.0f, (vx_float32)wb->compressed_size[i] / wb->orig_size[i]),
                i, 1.0,
                i, wb->outImageTileXSize[i],
                i, wb->outImageTileYSize[i],
                i, wb->kernelsPerCore[i]
                );

        cycleCount += wb->perfCycleCount[i];
        rdBandWidth += wb->perfReadBandWidth[i];
        ncRDBandWidth += wb->perfNCReadBandWidth[i];
        wtBandWidth += wb->perfWriteBandWidth[i];
    }

    printf("\n");
    if (wb->zgroup_num)
    {
        printf("Total ReadBandWidth: %llu (%llu)\nTotal WriteBandWidth: %llu\nTotal CycleCount: %llu\n\n",
                (unsigned long long)rdBandWidth, (unsigned long long)ncRDBandWidth, (unsigned long long)wtBandWidth, (unsigned long long)cycleCount);
        printf("=========================\n");
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_bool WeightBiasBufferAllocate(vx_context context, vx_weights_biases_parameter weight_bias, vx_size size, vx_bool raw_data)
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
    status = gcoVX_AllocateMemory((gctUINT32)size, (gctPOINTER*)&memory->logicals[0], (gctPHYS_ADDR*)&memory->physicals[0], &memory->nodePtrs[0]);
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

VX_PRIVATE_API vx_size calculateWeightBiasBufferSizeForZeroRunLen(vx_weights_biases_parameter wb, vx_tensor weights, uint8_t zeroRunLen, vx_uint32 filtersPerCore, vx_uint32 sliceCount, vx_uint32 z_count, vx_uint32 index, void* weightData)
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
VX_PRIVATE_API void writeBits(uint32_t **buffer, vx_uint32 *bitOffset, vx_uint32 data, vx_uint32 dataBits)
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

VX_PRIVATE_API vx_uint32 readBits(uint32_t **buffer, vx_uint32 *bitOffset, vx_uint32 dataBits)
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
VX_PRIVATE_API void packZeros(uint32_t **buffer, vx_uint32 *bitOffset, vx_uint32 alignedOffset)
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


VX_API_ENTRY vx_status VX_API_CALL vxReleaseWeightsBiasesParameter(vx_weights_biases_parameter *weights_bias)
{
    if ((*weights_bias) == VX_NULL) return VX_ERROR_INVALID_PARAMETERS;

    if ((*weights_bias)->weights_sizes)
    {
        vxFree((*weights_bias)->weights_sizes);
        (*weights_bias)->weights_sizes = VX_NULL;
    }
    if ((*weights_bias)->org_weights_sizes)
    {
        vxFree((*weights_bias)->org_weights_sizes);
        (*weights_bias)->org_weights_sizes = VX_NULL;
    }
    if ((*weights_bias)->biases_sizes)
    {
        vxFree((*weights_bias)->biases_sizes);
        (*weights_bias)->biases_sizes = VX_NULL;
    }
    if ((*weights_bias)->input_sizes)
    {
        vxFree((*weights_bias)->input_sizes);
        (*weights_bias)->input_sizes = VX_NULL;
    }
    if ((*weights_bias)->output_sizes)
    {
        vxFree((*weights_bias)->output_sizes);
        (*weights_bias)->output_sizes = VX_NULL;
    }

    if ((*weights_bias)->tmp_fcaccel_input_ptr)
    {
        vxFree((*weights_bias)->tmp_fcaccel_input_ptr);
        (*weights_bias)->tmp_fcaccel_input_ptr = VX_NULL;
    }
    if ((*weights_bias)->tmp_fcaccel_wb_ptr)
    {
        vxFree((*weights_bias)->tmp_fcaccel_wb_ptr);
        (*weights_bias)->tmp_fcaccel_wb_ptr = VX_NULL;
    }

    if ((*weights_bias)->memory.logicals[0] != VX_NULL)
    {
        gcoVX_FreeMemory((gcsSURF_NODE_PTR)(*weights_bias)->memory.nodePtrs[0]);
        (*weights_bias)->memory.logicals[0]    = VX_NULL;
        (*weights_bias)->memory.nodePtrs[0]    = VX_NULL;
    }
    if ((*weights_bias)->memory.writeLocks[0] != VX_NULL)
    {
        vxDestroyMutex((*weights_bias)->memory.writeLocks[0]);
        (*weights_bias)->memory.writeLocks[0]  = VX_NULL;
    }

    return vxoReference_Release((vx_reference_ptr)weights_bias, VX_TYPE_WEIGHTS_BIASES_PARAMETER, VX_REF_EXTERNAL);
}

VX_INTERNAL_API vx_weights_biases_parameter vxoWeightsBiases_Create(
        vx_context  context,
        vx_enum     layer_type,
        vx_uint32   num_of_dims,
        vx_uint32 * inputs_dims,
        vx_uint32   pad_x,
        vx_uint32   pad_y,
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
    weights_bias->pad_x = pad_x;
    weights_bias->pad_y = pad_y;
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

    vxMemCopy(weights_bias->org_weights_sizes, weights_dims, weights_num_of_dims * sizeof(vx_uint32));
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
        /* Calculate stride = (w + 2*pad - weight)/(output_w - 1) */
        stride = vxoNNExternsionConvlutionRound((vx_float32)(inputs_dims[0] + 2 * pad_x - weights_dims[0]) / (outputs_dims[0] - 1), down_scale_size_rounding);
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
    weights_bias->memroy_head_offset = gcmALIGN((3 + 8 * MAX_WEIGHT_BIAS_GROUPS) * sizeof(vx_uint32), 64);

    return weights_bias;
}

VX_API_ENTRY vx_weights_biases_parameter VX_API_CALL
vxCreateWeightsBiasesParameter(
    vx_context  context,
    vx_enum     layer_type,
    vx_uint32   num_of_dims,
    vx_uint32 * inputs_dims,
    vx_uint32   pad_x,
    vx_uint32   pad_y,
    vx_uint32   pooling_size_x,
    vx_uint32   pooling_size_y,
    vx_enum     down_scale_size_rounding,
    vx_uint32 * convolution_outputs_dims,
    vx_uint32 * pool_outputs_dims,
    vx_uint32   weights_num_of_dims,
    vx_uint32 * weights_dims,
    vx_enum     weights_data_format,
    vx_uint8    weights_fixed_point_pos,
    vx_uint32   biases_num_of_dims,
    vx_uint32 * biases_dims,
    vx_enum     biases_data_format,
    vx_uint8    biases_fixed_point_pos,
    vx_uint32   raw_data_size
    )
{
    vx_weights_biases_parameter weight_bias = VX_NULL;
    vx_uint32 i;

    if (!vxoContext_IsValid(context)) return VX_NULL;

    weight_bias = (vx_weights_biases_parameter)vxoWeightsBiases_Create(context,
                                                                       layer_type,
                                                                       num_of_dims,
                                                                       inputs_dims,
                                                                       pad_x,
                                                                       pad_y,
                                                                       pooling_size_x,
                                                                       pooling_size_y,
                                                                       down_scale_size_rounding,
                                                                       convolution_outputs_dims,
                                                                       weights_num_of_dims,
                                                                       weights_dims,
                                                                       weights_data_format,
                                                                       weights_fixed_point_pos,
                                                                       biases_num_of_dims,biases_dims,
                                                                       biases_data_format,
                                                                       biases_fixed_point_pos);

    if (!WeightBiasBufferAllocate(context, weight_bias, raw_data_size, vx_true_e)) return VX_NULL;

    {
        if (!vxoNNExternsionAdjustWeightsBiases(weight_bias, vx_true_e, vx_true_e, raw_data_size - weight_bias->memroy_head_offset)) return VX_NULL;

        for (i = 0; i < weight_bias->zgroup_num; i++)
        {
            /* Calculate filters per core */
            calculateFilterPerCore(context, weight_bias, i);
        }
    }

    if (vxoReference_GetStatus((vx_reference)weight_bias) != VX_SUCCESS) return VX_NULL;

    return weight_bias;
}

VX_PRIVATE_API void
calculateWeightBiasStreamRelatedSize(
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
    vx_uint8 minZeroRunLen=0, zeroRunLen=6;
    vx_uint32 z_count, index;
    vx_size minKernelBufferSize=~0UL, kernelBufferSize;
    gctSTRING env = gcvNULL;

    index = gcmMAX(0, output_z_index);
    z_count = output_z_index < 0 ?  wb->weights_sizes[3] : wb->zgroup_array[output_z_index];

    {
        gcoOS_GetEnv(gcvNULL, "NN_EXT_ZERO_RUN_LEN", &env);
        if (env)
        {
            zeroRunLen = (vx_uint8)atoi(env);
        }

        if (setZrl >= 0)
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

VX_PRIVATE_API void
fillinKernelBuffer(
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
                        biasData = *(bias_base_ptr + filterIndex);
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
                        vx_uint32 offsetValue = output_final_x * output_final_y * weightSize * filterIndex;
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
    }
}

VX_API_ENTRY vx_weights_biases_parameter VX_API_CALL vxCreateWeightsBiasesParameterFromTensors(
    vx_enum     layer_type,
    vx_uint32   num_of_dims,
    vx_uint32 * inputs_dims,
    vx_uint32   pad_x,
    vx_uint32   pad_y,
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
    vx_context context;
    void* convertedWeightData   = VX_NULL;
    void* weightData            = VX_NULL;

    vx_uint32 weightDimCount    = TENSOR_DIM_NUM(weights);
    vx_uint32 biasDimCount      = TENSOR_DIM_NUM(biases);
    vx_enum weightType          = TENSOR_DATA_TYPE(weights);
    vx_enum biasType            = TENSOR_DATA_TYPE(biases);
    vx_uint8 weightPos          = TENSOR_POS(weights);
    vx_uint8 biasPos            = TENSOR_POS(biases);
    vx_uint32* weightDims       = (vx_uint32 *)vxAllocateAndZeroMemory(weightDimCount * sizeof(vx_uint32));
    vx_uint32* biasDims         = (vx_uint32 *)vxAllocateAndZeroMemory(biasDimCount * sizeof(vx_uint32));
    vx_uint32* weightViewStarts = (vx_uint32 *)vxAllocateAndZeroMemory(weightDimCount * sizeof(vx_uint32));
    vx_uint32* weightViewEnds   = (vx_uint32 *)vxAllocateAndZeroMemory(weightDimCount * sizeof(vx_uint32));
    vx_uint32* biasViewStarts   = (vx_uint32 *)vxAllocateAndZeroMemory(biasDimCount * sizeof(vx_uint32));
    vx_uint32* biasViewEnds     = (vx_uint32 *)vxAllocateAndZeroMemory(biasDimCount * sizeof(vx_uint32));

    vx_uint32 sliceCount;
    vx_uint32 weightCount;
    vx_uint32 filterTotalCount;
    vx_uint32 filterSize;

    vx_size minKernelBufferSize[MAX_WEIGHT_BIAS_GROUPS];
    vx_uint8 minZeroRunLen[MAX_WEIGHT_BIAS_GROUPS];
    vx_uint32 maxZeroRun[MAX_WEIGHT_BIAS_GROUPS];
    vx_size minTotalKernelBufferSize = 0;

    vx_uint32 weightSize         = (vx_uint32)vxDataType_GetSize(weightType);

    vx_uint8_ptr kernelBufferBasePtr = VX_NULL;
    vx_uint8_ptr startWeightDataPtr  = VX_NULL;
    vx_uint32* startBiasDataPtr  = VX_NULL;
    gctPOINTER weightBase        = VX_NULL;
    gctPOINTER biasBase          = VX_NULL;

    vx_uint32 poolingStride      = 0;
    vx_uint32 i                  = 0;
    vx_bool   locked             = vx_false_e;
    vx_status status             = VX_SUCCESS;

    vx_size weightDataBytesOffset = 0, biasDataDWordOffset = 0, compressDataBytesOffset = 0;

    gctSTRING env = gcvNULL;
    vx_uint32 fcThreshold = 0;
    vx_bool useFCAccel = vx_true_e;

    vx_int8 setZeroLength = -1;

    if (optimizations)
        setZeroLength = optimizations->zrl;

    /* Create weight_bias */
    context = vxGetContext((vx_reference)weights);

    if (!context->nnConfig.isSet)
    {
        gcoVX_GetNNConfig(&context->nnConfig);
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

    weights_bias= vxoWeightsBiases_Create(context,
                                          layer_type,
                                          num_of_dims,
                                          inputs_dims,
                                          pad_x,
                                          pad_y,
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
    vxoTensor_GetTensorBaseMemory(biases, &biasBase, VX_NULL);
    if (weights->isViewed)
    {
        startWeightDataPtr = (vx_uint8*)weightBase + filterSize * weightViewStarts[3];
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
        vx_uint32 alignWeightWidth = ((weightDims[0] % weights_bias->stride == 0) ?
                                       weightDims[0] : (weightDims[0] + (weights_bias->stride - weightDims[0] % weights_bias->stride)));
        vx_uint32 alignWeightHeight = ((weightDims[1] % weights_bias->stride == 0) ?
                                       weightDims[1] : (weightDims[1] + (weights_bias->stride - weightDims[1] % weights_bias->stride)));
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
                        if ((x == src.xSize - 1) || (y == src.ySize - 1))
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

    /* Estimate kernel stream size first */
    calculateWeightBiasStreamRelatedSize(
        weights_bias, weights, weightDims[3], sliceCount, -1, weightData, &minKernelBufferSize[0], VX_NULL, VX_NULL, setZeroLength);

    /* Enable or disable FC ACCEL function */
    weights_bias->use_fc_accel = vx_false_e;
    if (layer_type == VX_CONVOLUTIONAL_NETWORK_FULLYCONNECTED_LAYER)
    {
        gcoOS_GetEnv(gcvNULL, "NN_EXT_FC_ACCEL", &env);
        if (env && !atoi(env))
        {
            useFCAccel = vx_false_e;
        }
        else
        {
            vx_float32 nonZeroRatio = (vx_float32)(weights_bias->all_count[0] - weights_bias->zero_count[0]) / weights_bias->all_count[0];
            if (nonZeroRatio > 0.4)
            {
                fcThreshold = FC_ACCEL_THRESHOLD;
                gcoOS_GetEnv(gcvNULL, "NN_EXT_FCACCEL_THRESHOLD", &env);
                if (env)
                {
                    fcThreshold = atoi(env);
                }
            }
            else
            {
                useFCAccel = vx_false_e;
            }
        }
    }

    if (useFCAccel &&
        layer_type == VX_CONVOLUTIONAL_NETWORK_FULLYCONNECTED_LAYER &&
        !(weights_bias->weights_sizes[3] & 0x7) &&
        weights_bias->weights_sizes[2] * weights_bias->weights_sizes[3] > fcThreshold)
    {
        vx_uint32 x, y, newWBSize, weightDepth, biasSize, biasPad;
        vx_uint8 *wbPtr, *wbTmpPtr, *weightPtr, *weightTmpPtr;

        if (!vxoNNExternsionAdjustWeightsBiases(weights_bias, vx_false_e, weights->isViewed, minKernelBufferSize[0]))
        {
            status = VX_FAILURE;
            goto exit;
        }

        biasPad = weights_bias->org_weights_sizes[2] < FC_Z_MAX ? 1 : weights_bias->weights_sizes[1];

        weights_bias->tmp_fcaccel_input_ptr = vxAllocateAndZeroMemory((weights_bias->org_weights_sizes[2] + biasPad) * weightSize);
        if (weights_bias->tmp_fcaccel_input_ptr == VX_NULL)
        {
            status = VX_ERROR_NO_MEMORY;
            goto exit;
        }

        /* Calculate filters per core */
        calculateFilterPerCore(context, weights_bias, 0);

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

        biasSize = sizeof(startBiasDataPtr[0]);
        wbPtr += weightDepth * filterTotalCount * weightSize;
        for (x = 0; x < filterTotalCount; x++, wbPtr+=weightSize)
        {
            _DataGeneralConvert((void*)(startBiasDataPtr+x), (void*)wbPtr, biasSize, weightSize);
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
            calculateFilterPerCore(context, weights_bias, i);

            /* Calculate kernel stream size for best kernesPerCore */
            calculateWeightBiasStreamRelatedSize(
                weights_bias, weights, weights_bias->kernelsPerCore[i], sliceCount, i, weightData, &minKernelBufferSize[i], &minZeroRunLen[i], &maxZeroRun[i], setZeroLength);

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
                weights_bias->kernelsPerCore[i],
                convolution_outputs_dims[0] / poolingStride,
                convolution_outputs_dims[1] / poolingStride,
                kernelBufferBasePtr,
                startWeightDataPtr + weightDataBytesOffset,
                startBiasDataPtr + biasDataDWordOffset
                );

            weightDataBytesOffset += filterSize * filterTotalCount;
            biasDataDWordOffset += filterTotalCount;
            weights_bias->memory_offset_array[i] = compressDataBytesOffset;
            compressDataBytesOffset += minKernelBufferSize[i] + weights_bias->memory_pad;
        }
    }

    gcoOS_GetEnv(gcvNULL, "NN_EXT_SHOW_PERF", &env);
    if (env && atoi(env))
    {
        vxoWeightsBiasesParameter_ShowPerformance(context, weights_bias);
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

VX_PRIVATE_API vx_status
vxoWeightsBiasesParameter_ProcessHead(
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
            if (!weights_bias->outImageTileXSize[i] || !weights_bias->outImageTileYSize[i] || !weights_bias->kernelsPerCore[i])
            {
                if (weights_bias->all_count[i] && weights_bias->orig_size[i] && weights_bias->compressed_size[i])
                {
                    /* Calculate filters per core */
                    calculateFilterPerCore(weights_bias->base.context, weights_bias, i);
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

VX_PRIVATE_API vx_status
vxoWeightsBiasesParameter_Map(
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

VX_PRIVATE_API vx_status
vxoWeightsBiasesParameter_Unmap(
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

VX_INTERNAL_API vx_bool
vxoWeightsBiasesParameter_IsValid(
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

VX_API_ENTRY vx_status VX_API_CALL
vxMapWeightsBiasesParameter(
    vx_weights_biases_parameter     weights_biases,
    vx_map_id *                     map_id,
    vx_uint32 *                     stride,
    void **                         ptr,
    vx_enum                         usage,
    vx_enum                         mem_type,
    vx_uint32                       flags
    )
{
    if (!vxoWeightsBiasesParameter_IsValid(weights_biases)) return VX_ERROR_INVALID_REFERENCE;

    return vxoWeightsBiasesParameter_Map(weights_biases, map_id, stride, ptr, usage, mem_type, flags);
}

VX_API_ENTRY vx_status VX_API_CALL
vxUnmapWeightsBiasesParameter(
    vx_weights_biases_parameter     weights_biases,
    vx_map_id                       map_id
    )
{
    if (!vxoWeightsBiasesParameter_IsValid(weights_biases)) return VX_ERROR_INVALID_REFERENCE;

    return vxoWeightsBiasesParameter_Unmap(weights_biases, map_id);
}

VX_API_ENTRY vx_status VX_API_CALL vxConfigTarget(
    vx_context context,
    vx_int32 dp_amount,
    vx_int32 mac_per_core,
    vx_int32  conv_cores,
    vx_int32 in_buffer_depth,
    vx_int32 accum_buffer_height
)
{
    if (!vxoContext_IsValid(context)) return VX_ERROR_INVALID_REFERENCE;

#define L2_CACHE_SIZE                          0
    context->nnConfig.nnMadPerCoure = mac_per_core;
    context->nnConfig.nnCoreCount   = conv_cores;
    context->nnConfig.nnInputBufferDepth = in_buffer_depth;
    context->nnConfig.nnAccumBufferDepth = accum_buffer_height;
    context->nnConfig.nnDPAmount = dp_amount;
    context->nnConfig.nnL2CacheSize = L2_CACHE_SIZE;
    context->nnConfig.isSet = gcvTRUE;
    return VX_SUCCESS;
}

VX_API_ENTRY vx_uint32 VX_API_CALL vxWeightsBiasesParameterToStream(
    vx_context context,
    vx_weights_biases_parameter weights_biases_parameter,
    vx_uint32 ** weights_biases_stream
)
{
    vx_uint32 bufferSize;
    vx_uint32 bitOffset;
    vx_uint32* kernelBufferPtr = *weights_biases_stream;
    vx_uint8_ptr base = (vx_uint8_ptr)kernelBufferPtr;
    vx_uint32 i;

    gcmASSERT(context);
    /* Write weights biases parameter to the head of data*/
    bitOffset = 0;
    writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->layer_type, 32);
    writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->inout_num_of_dims, 32);
    for (i = 0; i < VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
    {
        if (i < weights_biases_parameter->inout_num_of_dims)
        {
            writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->input_sizes[i], 32);
        }
        else
        {
            writeBits(&kernelBufferPtr, &bitOffset, 0, 32);
        }
    }
    writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->pad_x, 32);
    writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->pad_y, 32);
    writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->pooling_size_x, 32);
    writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->pooling_size_y, 32);
    writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->down_scale_size_rounding, 32);

    for (i = 0; i < VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
    {
        if (i < weights_biases_parameter->inout_num_of_dims)
        {
            writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->output_sizes[i], 32);
        }
        else
        {
            writeBits(&kernelBufferPtr, &bitOffset, 0, 32);
        }
    }

    for (i = 0; i < VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
    {
        if (i < 2)
        {
            writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->output_sizes[i] / weights_biases_parameter->pooling_stride, 32);
        }
        else if (i < weights_biases_parameter->inout_num_of_dims)
        {
            writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->output_sizes[i], 32);
        }
        else
        {
            writeBits(&kernelBufferPtr, &bitOffset, 0, 32);
        }
    }

    writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->weights_num_of_dims, 32);
    for (i = 0; i < VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
    {
        if (i < weights_biases_parameter->weights_num_of_dims)
        {
            writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->weights_sizes[i], 32);
        }
        else
        {
            writeBits(&kernelBufferPtr, &bitOffset, 0, 32);
        }
    }
    writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->weights_data_format, 32);
    writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->weights_fixed_point_pos, 32);

    writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->biases_num_of_dims, 32);
    for (i = 0; i < VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
    {
        if (i < weights_biases_parameter->biases_num_of_dims)
        {
            writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->biases_sizes[i], 32);
        }
        else
        {
            writeBits(&kernelBufferPtr, &bitOffset, 0, 32);
        }
    }
    writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->biases_data_format, 32);
    writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->biases_fixed_point_pos, 32);
    /* Save kernel stream buffer size*/
    writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)weights_biases_parameter->memory_size, 32);

    /* Save kernel per core*/
    for (i = 0; i < MAX_WEIGHT_BIAS_GROUPS; i++)
    {
        writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->zgroup_array[i], 32);
        writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->outImageTileXSize[i], 32);
        writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->outImageTileYSize[i], 32);
        writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->kernelsPerCore[i], 32);
    }
    writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->current_mad_per_core, 32);
    writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)weights_biases_parameter->use_fc_accel, 32);
    writeBits(&kernelBufferPtr, &bitOffset, (vx_uint32)weights_biases_parameter->fc_accel_large_size, 32);
    writeBits(&kernelBufferPtr, &bitOffset, weights_biases_parameter->input_nonzero_count, 32);

    /* Copy kernel stream data*/
    kernelBufferPtr++;
    memcpy(kernelBufferPtr, weights_biases_parameter->memory.logicals[0] - weights_biases_parameter->memroy_head_offset, weights_biases_parameter->memory_size);

    bufferSize = (vx_uint32)((vx_uint8 *)kernelBufferPtr - base) + (vx_uint32)weights_biases_parameter->memory_size;

    return bufferSize;
}

VX_API_ENTRY vx_weights_biases_parameter VX_API_CALL vxCreateWeightsBiasesParameterFromStream(
    vx_context context,
    vx_uint32 * weights_biases_stream
)
{
    vx_weights_biases_parameter weights_bias;
    vx_uint32 bitOffset = 0;
    vx_uint32* streamBufferPtr = weights_biases_stream;
    vx_uint32 i;

    vx_enum     layerType;
    vx_uint32   numOfDims;
    vx_uint32   inputsDims[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32   padX;
    vx_uint32   padY;
    vx_uint32   poolingSizeX;
    vx_uint32   poolingSizeY;
    vx_enum     downScaleSizeRounding;
    vx_uint32   convolutionOutputsDims[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32   poolingDims[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32   weightsNumOfDims;
    vx_uint32   weightsDims[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_enum     weightsFormat;
    vx_uint8    weightsFixedPointPos;
    vx_uint32   biasesNumOfDims;
    vx_uint32   biasesDims[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_enum     biasesFormat;
    vx_uint8    biasesFixedPointPos;
    vx_uint32   rawDataSize;

    if (!vxoContext_IsValid(context)) return VX_NULL;

    layerType = readBits(&streamBufferPtr, &bitOffset, 32);
    numOfDims = readBits(&streamBufferPtr, &bitOffset, 32);

    for (i = 0; i < VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
    {
        inputsDims[i] = readBits(&streamBufferPtr, &bitOffset, 32);
    }

    padX = readBits(&streamBufferPtr, &bitOffset, 32);
    padY = readBits(&streamBufferPtr, &bitOffset, 32);
    poolingSizeX = readBits(&streamBufferPtr, &bitOffset, 32);
    poolingSizeY = readBits(&streamBufferPtr, &bitOffset, 32);
    downScaleSizeRounding = readBits(&streamBufferPtr, &bitOffset, 32);

    for (i = 0; i < VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
    {
        convolutionOutputsDims[i] = readBits(&streamBufferPtr, &bitOffset, 32);
    }
    for (i = 0; i < VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
    {
        poolingDims[i] = readBits(&streamBufferPtr, &bitOffset, 32);
    }

    weightsNumOfDims = readBits(&streamBufferPtr, &bitOffset, 32);
    for (i = 0; i < VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
    {
        weightsDims[i] = readBits(&streamBufferPtr, &bitOffset, 32);
    }
    weightsFormat = readBits(&streamBufferPtr, &bitOffset, 32);
    weightsFixedPointPos = (vx_uint8)readBits(&streamBufferPtr, &bitOffset, 32);
    biasesNumOfDims = readBits(&streamBufferPtr, &bitOffset, 32);
    for (i = 0; i < VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
    {
        biasesDims[i] = readBits(&streamBufferPtr, &bitOffset, 32);
    }
    biasesFormat = readBits(&streamBufferPtr, &bitOffset, 32);
    biasesFixedPointPos = (vx_uint8)readBits(&streamBufferPtr, &bitOffset, 32);
    rawDataSize = readBits(&streamBufferPtr, &bitOffset, 32);

    weights_bias = (vx_weights_biases_parameter)vxoWeightsBiases_Create(context,
                                                                   layerType,
                                                                   numOfDims,
                                                                   inputsDims,
                                                                   padX,
                                                                   padY,
                                                                   poolingSizeX,
                                                                   poolingSizeY,
                                                                   downScaleSizeRounding,
                                                                   convolutionOutputsDims,
                                                                   weightsNumOfDims,
                                                                   weightsDims,
                                                                   weightsFormat,
                                                                   weightsFixedPointPos,
                                                                   biasesNumOfDims,biasesDims,
                                                                   biasesFormat,
                                                                   biasesFixedPointPos);

    if (vxoReference_GetStatus((vx_reference)weights_bias) != VX_SUCCESS) return VX_NULL;

    if (!WeightBiasBufferAllocate(context, weights_bias, rawDataSize - weights_bias->memroy_head_offset, vx_false_e)) return VX_NULL;
    weights_bias->memory_size = rawDataSize;

    if (!vxoNNExternsionAdjustWeightsBiases(weights_bias, vx_true_e, vx_true_e, weights_bias->memory_size)) return VX_NULL;

    for (i = 0; i < MAX_WEIGHT_BIAS_GROUPS; i++)
    {
        weights_bias->zgroup_array[i] = readBits(&streamBufferPtr, &bitOffset, 32);
        weights_bias->outImageTileXSize[i] = readBits(&streamBufferPtr, &bitOffset, 32);
        weights_bias->outImageTileYSize[i] = readBits(&streamBufferPtr, &bitOffset, 32);
        weights_bias->kernelsPerCore[i] = readBits(&streamBufferPtr, &bitOffset, 32);
    }
    weights_bias->current_mad_per_core = readBits(&streamBufferPtr, &bitOffset, 32);
    weights_bias->use_fc_accel = (vx_bool)readBits(&streamBufferPtr, &bitOffset, 32);
    weights_bias->fc_accel_large_size = (vx_bool)readBits(&streamBufferPtr, &bitOffset, 32);
    weights_bias->input_nonzero_count = readBits(&streamBufferPtr, &bitOffset, 32);

    /* Copy kernel stream data*/
    streamBufferPtr++;
    memcpy(weights_bias->memory.logicals[0] - weights_bias->memroy_head_offset, streamBufferPtr, weights_bias->memory_size);

    return weights_bias;
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

VX_PRIVATE_API vx_int32 getHwPoolingType(vx_enum poolingType)
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

#define NNE_COMMAND_SIZE 64

VX_PRIVATE_API void
fillinCmmdBuff(
    vx_tensor                    inputs,
    vx_weights_biases_parameter  weights_biases,
    vx_uint32                    pad_x,
    vx_uint32                    pad_y,
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

    memset(&info, 0, sizeof(vx_nn_cmd_info_u));

    input_width   = inputs->tensorBuffer->memory.dims[0][0];
    input_height  = inputs->tensorBuffer->memory.dims[0][1];
    org_kernel_x  = weights_biases->org_weights_sizes[0];
    org_kernel_y  = weights_biases->org_weights_sizes[1];
    kernel_x      = weights_biases->weights_sizes[0];
    kernel_y      = weights_biases->weights_sizes[1];
    kernel_z      = weights_biases->weights_sizes[2];
    conv_stride_x = conv_stride_y = weights_biases->stride;
    out_image_z   = weights_biases->zgroup_array[index];
    out_image_tilex = weights_biases->outImageTileXSize[index];
    out_image_tiley = weights_biases->outImageTileYSize[index];
    kernels_per_core = weights_biases->kernelsPerCore[index];
    mad_per_core = weights_biases->current_mad_per_core;

    if (isFullyConnectedLayer)
    {
        input_width  = 1;
        input_height = 1;
    }

    if ((inputs->finalDims[0] == 0) || (outputs->finalDims[0] == 0))
    {
        vxoNNExternsionInputOutputArguments(
            input_width, input_height,
            conv_stride_x, conv_stride_y,
            pad_x, pad_y,
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

    if (outputs->insideDims[0] != outputs->finalDims[0] ||
        outputs->insideDims[1] != outputs->finalDims[1] ||
        out_image_tilex > outputs->finalDims[0])
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
    info.vx_nn_general_cmd_info.inImageXOffset    = (-1) * (vx_int32)(outputs->finalDims[0] + kernel_x - 1 - inputs->finalDims[0]) / 2;
    info.vx_nn_general_cmd_info.inImageYOffset    = (-1) * (vx_int32)(outputs->finalDims[1] + kernel_y - 1 - input_height_ex) / 2;
    info.vx_nn_general_cmd_info.outImageTileXSize = out_image_tilex;
    info.vx_nn_general_cmd_info.outImageTileYSize = out_image_tiley;

    /* Fixed same type in this stage. */
    info.vx_nn_general_cmd_info.kernelDataType    = weights_biases->weights_data_format == VX_TYPE_INT8 ? 0x2 : 0x1;
    info.vx_nn_general_cmd_info.inImageDataType   = info.vx_nn_general_cmd_info.kernelDataType;
    info.vx_nn_general_cmd_info.outImageDataType  = info.vx_nn_general_cmd_info.kernelDataType;

    info.vx_nn_general_cmd_info.postMultiplier    = 0;
    info.vx_nn_general_cmd_info.roundingMode      = weights_biases->weights_data_format == VX_TYPE_INT8 ? 3 : 0;

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

    cmd_buf_ptr = (vx_uint32*)(cmd_buff->memory.logicals[0] + NNE_COMMAND_SIZE * index);

    gcoVX_ProgrammCrossEngine((void*)&info, gcvVX_ACCELERATOR_NN, &cmd_buf_ptr);
}

#define TP_COMMAND_SIZE 128

enum _vx_tp_cmd_type_e
{
    TP_RESHUFFLE,
    TP_SINGLE_FC,
    TP_MAX_POOLING,
    TP_LEAKY_RELU,
    TP_LEAKY_RELU_MAX_POOLING,
};

VX_PRIVATE_API void
fillInCmmdTPBuffer(
    vx_tensor                    inputs,
    vx_weights_biases_parameter  weights_biases,
    vx_tensor                    outputs,
    vx_array                     cmd_buff,
    vx_uint32                    pad_x,
    vx_uint32                    pad_y,
    vx_uint32                    pool_size_x,
    vx_uint32                    pool_size_y,
    vx_bool                      enable_relu,
    vx_enum                      tp_type
    )
{
    vx_nn_cmd_info_u info;
    vx_uint32 * cmd_buf_ptr;
    vx_uint32 inXSize, inYSize, inZSize, outXSize, outYSize, outZSize, outTileXSize, outTileYSize;
    vx_uint32 strideXSize, strideYSize, outSliceSize, poolingStride;

    memset(&info, 0, sizeof(vx_nn_cmd_info_u));

    inXSize = inputs->tensorBuffer->memory.dims[0][0];
    inYSize = inputs->tensorBuffer->memory.dims[0][1];
    inZSize = inputs->tensorBuffer->memory.dims[0][2];

    switch (tp_type)
    {
        case TP_RESHUFFLE:
            strideXSize = weights_biases->stride;
            strideYSize = weights_biases->stride;
            outXSize = (inXSize % strideXSize ? inXSize + (strideXSize - inXSize % strideXSize) : inXSize) / strideXSize;
            outYSize = (inYSize % strideYSize ? inYSize + (strideYSize - inYSize % strideYSize) : inYSize) / strideYSize;
            outZSize = weights_biases->org_weights_sizes[2] * strideXSize * strideYSize;
            outSliceSize = outXSize * outYSize;

            info.vx_nn_tp_cmd_info.inImageXSize = inXSize;
            info.vx_nn_tp_cmd_info.inImageYSize = inYSize;
            info.vx_nn_tp_cmd_info.inImageZSize = inZSize;
            info.vx_nn_tp_cmd_info.inImageStride = inXSize;
            info.vx_nn_tp_cmd_info.inImageSlice = inXSize * inYSize;
            info.vx_nn_tp_cmd_info.inWindowXStart = (-1) * (vx_int32)pad_x;
            info.vx_nn_tp_cmd_info.inWindowYStart = (-1) * (vx_int32)pad_y;
            info.vx_nn_tp_cmd_info.inWindowXEnd = inXSize + pad_x;
            info.vx_nn_tp_cmd_info.inWindowYEnd = inYSize + pad_y;
            info.vx_nn_tp_cmd_info.inTileSequence = 0x0;
            info.vx_nn_tp_cmd_info.inImageGlobalMem = 1;
            info.vx_nn_tp_cmd_info.inImageBaseAddress = inputs->tensorBuffer->memory.physicals[0];
            info.vx_nn_tp_cmd_info.outTileSkipAtborder = 0;
            info.vx_nn_tp_cmd_info.inTileXSize = (outXSize - 1) * strideXSize + strideXSize;
            info.vx_nn_tp_cmd_info.inTileYSize = (outYSize - 1) * strideYSize + strideYSize;
            info.vx_nn_tp_cmd_info.inTileXInc = outXSize * strideXSize;
            info.vx_nn_tp_cmd_info.inTileYInc = outYSize * strideYSize;
            info.vx_nn_tp_cmd_info.outBaseAddress = outputs->tensorBuffer->memory.physicals[0];
            info.vx_nn_tp_cmd_info.outGlobalMem  = 1;
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
            info.vx_nn_tp_cmd_info.outLoop4Inc   = outXSize * outYSize * outZSize; /* brick distance */
            info.vx_nn_tp_cmd_info.outLoop4Count = 1;
            info.vx_nn_tp_cmd_info.outLoop5Inc   = outSliceSize | (outSliceSize << 16);
            info.vx_nn_tp_cmd_info.outLoop5Count = 1;
            info.vx_nn_tp_cmd_info.outLoop6Inc   = 0;
            info.vx_nn_tp_cmd_info.aluFilterPwlSwap = 0;
            info.vx_nn_tp_cmd_info.aluPwlSignSupport = 0;
            info.vx_nn_tp_cmd_info.aluReluEnable = 0;
            break;

        case TP_SINGLE_FC:
            /* TODO */
            break;

        case TP_MAX_POOLING:
            outTileXSize = 32;
            outTileYSize = 16;
            poolingStride = 2;

            info.vx_nn_tp_cmd_info.inImageXSize = inXSize;
            info.vx_nn_tp_cmd_info.inImageYSize = inYSize;
            info.vx_nn_tp_cmd_info.inImageZSize = inZSize;
            info.vx_nn_tp_cmd_info.inImageStride = inXSize;
            info.vx_nn_tp_cmd_info.inImageSlice = inXSize * inYSize;
            info.vx_nn_tp_cmd_info.inWindowXStart = (-1) * (vx_int32)pad_x;
            info.vx_nn_tp_cmd_info.inWindowYStart = (-1) * (vx_int32)pad_y;
            info.vx_nn_tp_cmd_info.inWindowXEnd = inXSize + pad_x;
            info.vx_nn_tp_cmd_info.inWindowYEnd = inYSize + pad_y;
            info.vx_nn_tp_cmd_info.inTileSequence = 0x0;
            info.vx_nn_tp_cmd_info.inImageGlobalMem = 1;
            info.vx_nn_tp_cmd_info.inImageBaseAddress = inputs->tensorBuffer->memory.physicals[0];
            info.vx_nn_tp_cmd_info.inTileXSize = outTileXSize * poolingStride + pool_size_x - poolingStride;
            info.vx_nn_tp_cmd_info.inTileYSize = outTileYSize * poolingStride + pool_size_y - poolingStride;
            info.vx_nn_tp_cmd_info.inTileXInc = outTileXSize * poolingStride;
            info.vx_nn_tp_cmd_info.inTileYInc = outTileYSize * poolingStride;
            info.vx_nn_tp_cmd_info.aluSquarePreshift = 0;
            info.vx_nn_tp_cmd_info.aluSquareEnable = 0;
            info.vx_nn_tp_cmd_info.aluHorzProcessing = 0x3;
            info.vx_nn_tp_cmd_info.aluHorzProcCount = 2;
            info.vx_nn_tp_cmd_info.aluHorzProcStride = 0x1;
            info.vx_nn_tp_cmd_info.aluVertProcessing = 0x3;
            info.vx_nn_tp_cmd_info.aluVertProcCount = pool_size_y - 1;
            info.vx_nn_tp_cmd_info.aluVertProcStride = pool_size_y == 2 ?
                0x0 : 0x1;
            info.vx_nn_tp_cmd_info.aluPwlEnable = 0;
            info.vx_nn_tp_cmd_info.aluMultEnable = 0;
            info.vx_nn_tp_cmd_info.aluLoadPwlLUT = 0;
            info.vx_nn_tp_cmd_info.aluLoadPwlLUTAddress = 0;
            info.vx_nn_tp_cmd_info.aluLoadPwlLUTGlobalMem = 1;
            info.vx_nn_tp_cmd_info.outBaseAddress = outputs->tensorBuffer->memory.physicals[0];
            info.vx_nn_tp_cmd_info.outGlobalMem  = 1;
            info.vx_nn_tp_cmd_info.outTileSkipAtborder = 0;
            info.vx_nn_tp_cmd_info.outBrickMode  = 0;
            info.vx_nn_tp_cmd_info.outLoop0Inc   = 0;
            info.vx_nn_tp_cmd_info.outLoop0Count = 1;
            info.vx_nn_tp_cmd_info.outLoop1Inc   = 1;
            info.vx_nn_tp_cmd_info.outLoop1Count = 0;
            info.vx_nn_tp_cmd_info.outLoop1Reset = 0x1;
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
            info.vx_nn_tp_cmd_info.outLoop6Inc   = inXSize * inYSize;
            info.vx_nn_tp_cmd_info.aluFilterPwlSwap = 0;
            info.vx_nn_tp_cmd_info.aluPwlSignSupport = 0;
            info.vx_nn_tp_cmd_info.aluReluEnable = 0;
            break;

        case TP_LEAKY_RELU:
        case TP_LEAKY_RELU_MAX_POOLING:
        {
            vx_status   status = VX_SUCCESS;
            vx_context  context = vxGetContext((vx_reference)inputs);
            vx_float32  scale = 0.1f;           /* TODO - Should be an input. */
            vx_array    pwlLUTBuffer = VX_NULL; /* TODO - Should be an input. */
            vx_uint16 * pwlLUTBase;
            vx_uint16   base;
            vx_uint16   baseF16;
            vx_float32  baseF32;
            vx_float32  pwlValue;

            /* Prepare PWL LUT. */
            pwlLUTBuffer = vxCreateArray(context, VX_TYPE_UINT16, 1024);
            if (!vxoArray_AllocateMemory(pwlLUTBuffer))
            {
                status |= VX_ERROR_NO_MEMORY;
            }
            pwlLUTBuffer->itemCount = 1024;
            pwlLUTBuffer->base.isStage = vx_true_e;
            pwlLUTBase = (vx_uint16 *) pwlLUTBuffer->memory.logicals[0];

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
                baseF16 = base << 5;
                pwlLUTBase[base] = baseF16;
            }
            for (base = 0x210; base < 0x3F0; base++)
            {
                baseF16 = base << 5;
                baseF32 = Fp16toFp32(baseF16);
                pwlValue = baseF32 * scale;
                pwlLUTBase[base] = F32toF16(pwlValue);
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
                poolingStride = 2;

                info.vx_nn_tp_cmd_info.inWindowXStart = (-1) * (vx_int32)pad_x;
                info.vx_nn_tp_cmd_info.inWindowYStart = (-1) * (vx_int32)pad_y;
                info.vx_nn_tp_cmd_info.inWindowXEnd = inXSize + pad_x;
                info.vx_nn_tp_cmd_info.inWindowYEnd = inYSize + pad_y;
                info.vx_nn_tp_cmd_info.inTileXSize = outTileXSize * poolingStride + pool_size_x - poolingStride;
                info.vx_nn_tp_cmd_info.inTileYSize = outTileYSize * poolingStride + pool_size_y - poolingStride;
                info.vx_nn_tp_cmd_info.inTileXInc = outTileXSize * poolingStride;
                info.vx_nn_tp_cmd_info.inTileYInc = outTileYSize * poolingStride;
                info.vx_nn_tp_cmd_info.aluHorzProcessing = 0x3;
                info.vx_nn_tp_cmd_info.aluHorzProcCount = pool_size_x - 1;
                info.vx_nn_tp_cmd_info.aluHorzProcStride = pool_size_x == 2 ? 0x0
                                                                            : 0x1;
                info.vx_nn_tp_cmd_info.aluVertProcessing = 0x3;
                info.vx_nn_tp_cmd_info.aluVertProcCount = pool_size_y - 1;
                info.vx_nn_tp_cmd_info.aluVertProcStride = pool_size_y == 2 ? 0x0
                                                                            : 0x1;
                info.vx_nn_tp_cmd_info.aluFilterPwlSwap = 1;
            }
            else
            {
                outTileXSize = 64;
                outTileYSize = 16;

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
            info.vx_nn_tp_cmd_info.inImageZSize = inZSize;
            info.vx_nn_tp_cmd_info.inImageStride = inXSize;
            info.vx_nn_tp_cmd_info.inImageSlice = inXSize * inYSize;
            info.vx_nn_tp_cmd_info.inTileSequence = 0x0;
            info.vx_nn_tp_cmd_info.inImageGlobalMem = 1;
            info.vx_nn_tp_cmd_info.inImageBaseAddress = inputs->tensorBuffer->memory.physicals[0];
            info.vx_nn_tp_cmd_info.aluSquarePreshift = 0;
            info.vx_nn_tp_cmd_info.aluSquareEnable = 0;
            info.vx_nn_tp_cmd_info.aluPwlEnable = 1;
            info.vx_nn_tp_cmd_info.aluMultEnable = 0;
            info.vx_nn_tp_cmd_info.aluLoadPwlLUT = 1;
            info.vx_nn_tp_cmd_info.aluLoadPwlLUTAddress = pwlLUTBuffer->memory.physicals[0];
            info.vx_nn_tp_cmd_info.aluLoadPwlLUTGlobalMem = 1;
            info.vx_nn_tp_cmd_info.outBaseAddress = outputs->tensorBuffer->memory.physicals[0];
            info.vx_nn_tp_cmd_info.outGlobalMem  = 1;
            info.vx_nn_tp_cmd_info.outTileSkipAtborder = 0;
            info.vx_nn_tp_cmd_info.outBrickMode  = 0;
            info.vx_nn_tp_cmd_info.outLoop0Inc   = 0;
            info.vx_nn_tp_cmd_info.outLoop0Count = 1;
            info.vx_nn_tp_cmd_info.outLoop1Inc   = 1;
            info.vx_nn_tp_cmd_info.outLoop1Count = 0;
            info.vx_nn_tp_cmd_info.outLoop1Reset = 0x1;
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
            info.vx_nn_tp_cmd_info.outLoop6Inc   = inXSize * inYSize;
            info.vx_nn_tp_cmd_info.aluPwlSignSupport = 1;
            info.vx_nn_tp_cmd_info.aluReluEnable = 0;

            /* TODO - Need to release pwlLUTBuffer after TP Command. */
            /*        It is better to pass in pwlLUTBuffer so that caller can manage it. */
            /*vxReleaseArray(&pwlLUTBuffer);*/

            break;
        }

        default:
            break;
    }

    info.vx_nn_tp_cmd_info.inImageDataType = inputs->tensorBuffer->dataFormat == VX_TYPE_INT8 ? 0x2 : 0x1;
    info.vx_nn_tp_cmd_info.outImageDataType = outputs->tensorBuffer->dataFormat == VX_TYPE_INT8 ? 0x2 : 0x1;
    info.vx_nn_tp_cmd_info.kernelDataType = weights_biases->weights_data_format == VX_TYPE_INT8 ? 0x2 : 0x1;
    info.vx_nn_tp_cmd_info.floatRoundingMode = 1;
    info.vx_nn_tp_cmd_info.integeroundingMode = 0;
    info.vx_nn_tp_cmd_info.last = 1;

    cmd_buf_ptr = (vx_uint32*)(cmd_buff->memory.logicals[0]);

    gcoVX_ProgrammCrossEngine((void*)&info, gcvVX_ACCELERATOR_TP, &cmd_buf_ptr);
}

void vxoNNExternsionDoReshuffle(vx_tensor inputs, vx_tensor outputs, vx_uint32 pad_x, vx_uint32 pad_y, vx_uint32 stride_x, vx_uint32 stride_y)
{
    vx_uint32 x, y, z, w;
    vx_uint32 srcWidth, srcHeight;
    vx_uint32 dstWidth, dstHeight, dstDepth;
    vx_uint32 srcXSize, srcYSize;
    vx_uint32 dstXSize, dstYSize, dstZSize, dstWSize;
    vx_uint32 padWidth, padHeight;
    vx_uint32 elementSize;
    void *srcDataAddr;
    void *dstDataAddr;


    /* do reshuffle*/
    srcWidth  = inputs->tensorBuffer->memory.dims[0][0];
    srcHeight = inputs->tensorBuffer->memory.dims[0][1];
    padWidth  = pad_x + pad_x;
    padHeight = pad_y + pad_y;

    dstWidth  = outputs->tensorBuffer->memory.dims[0][0] / stride_x;
    dstHeight = outputs->tensorBuffer->memory.dims[0][1] / stride_y;
    dstDepth  = outputs->tensorBuffer->memory.dims[0][2] * stride_x * stride_y;

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

                    srcPosX = (z % (stride_x * stride_y)) % stride_x + x * stride_x;
                    srcPosY = (z % (stride_x * stride_y)) / stride_x + y * stride_y;
                    srcPosZ = z / (stride_x * stride_y);
                    if (((srcPosX >= padWidth/2) && (srcPosX <= padWidth/2 + srcWidth - 1)) && ((srcPosY >= padHeight/2) && (srcPosY <= padHeight/2 + srcHeight - 1)))
                    {
                        srcData = (vx_uint8*)srcDataAddr + (srcPosZ * srcYSize * srcXSize + (srcPosY-padHeight/2) * srcXSize + (srcPosX-padWidth/2)) * elementSize;
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

vx_status vxnneLayer_Deinitialize(struct _vxnne_layer_s* layer)
{
    vx_uint32 i;

    for (i = 0; i < VX_MAX_TEMP_TENSORS; i++)
    {
        if (layer->temp_tensors[i] != VX_NULL)
        {
            vxoTensor_ReleaseMemory(layer->temp_tensors[i]);
        }
    }

    if (layer->cmdNNBuff != VX_NULL)
    {
        vxReleaseArray(&layer->cmdNNBuff);
    }

    if (layer->cmdTPBuff != VX_NULL)
    {
        vxReleaseArray(&layer->cmdTPBuff);
    }

    for (i = 0; i < layer->num_operations; i++)
    {
        if (layer->operations[i]->deinitialize != VX_NULL)
        {
            layer->operations[i]->deinitialize(layer->operations[i]);
        }
    }

    return VX_SUCCESS;
}

vx_status vxnneLayer_Dump(
    struct _vxnne_layer_s* layer,
    int verbose
    )
{
    vx_uint32        paramIndex = 0;
    vx_uint32        width;
    vx_uint32        height;
    vx_uint32        depth;
    vx_uint32        batch;
    vx_tensor        outputs;
    vx_char          fileName[256] = {'\0'};
    FILE             *fp;
    static vx_uint32 layerNum = 0;
    vx_uint32        index;
    vx_uint32        elementCount;
    void             *outputsBase = VX_NULL;

    for (paramIndex = 0; paramIndex < layer->node->kernel->signature.paramCount; paramIndex++)
    {
        vx_reference paramRef = layer->node->paramTable[paramIndex];

        if (paramRef == VX_NULL) continue;

        if (!vxmIS_OUTPUT_OR_BIDIRECTION(layer->node->kernel->signature.directionTable[paramIndex])) continue;

        outputs     = (vx_tensor)paramRef;
        width       = outputs->tensorBuffer->memory.dims[0][0];
        height      = outputs->tensorBuffer->memory.dims[0][1];
        depth       = outputs->tensorBuffer->memory.dims[0][2];
        batch       = outputs->tensorBuffer->memory.dims[0][3];
        vxoTensor_GetTensorViewMemory(outputs, &outputsBase, VX_NULL);

        sprintf(fileName,"%s_%02d.txt", layer->name, layerNum);

        fp = fopen(fileName,"w");
        if(!fp)
        {
            printf("can't open the file %s\n",fileName);
            fclose(fp);
            continue;
        }

        if(batch == 0)
        {
            batch = 1;
        }

        printf("***********dump layer :%2d***************\n",layerNum);

        elementCount = width * height * depth * batch;
        for(index = 0; index < elementCount; index++)
        {
            fprintf(fp, "%f\n", vxnneGetDate((vx_type_e)outputs->tensorBuffer->dataFormat, index, (vx_uint8_ptr)outputsBase, outputs->tensorBuffer->fixedPointPos));
        }
        fclose(fp);
    }

    layerNum++;

    return VX_SUCCESS;
}

vx_status vxnneLayer_Free(struct _vxnne_layer_s* layer)
{
    layer->deinitialize(layer);
    gcoOS_Free(gcvNULL, layer);

    return VX_SUCCESS;
}


vx_status vxnneLayer_Execute(vxnne_layer layer)
{
    vx_uint32 i;
    gctSTRING env = gcvNULL;
#if defined(__linux__)
    struct timeval start = gcfVX_PerfStart((vx_reference)layer->node);
#endif

    for (i = 0; i < layer->num_operations; i++)
    {
        layer->operations[i]->execute(layer->operations[i]);
    }

#if defined(__linux__)
    if (layer->node->base.context->perfEnable)
        printf("%s execution time:%10d us\n", layer->name, gcfVX_PerfEnd((vx_reference)layer->node, start));
#endif

    gcoOS_GetEnv(gcvNULL, "NN_LAYER_DUMP", &env);
    if (env && atoi(env))
    {
        layer->dump = vxnneLayer_Dump;
        layer->dump(layer, 0);
    }

    return VX_SUCCESS;
}

vx_status vxnneLayer_Initialize(
    vxnne_layer                 layer,
    vx_char                     *name,
    vx_node                     node,
    vxnne_operation             *operation,
    vxnne_layer_deinitialize_f  deinitialize
    )
{
    layer->name         = name;
    layer->node         = node;
    layer->operations   = operation;

    layer->num_temp_tensors      = 0;
    layer->dump                  = VX_NULL;
    layer->deinitialize          = (deinitialize ? deinitialize :  vxnneLayer_Deinitialize);

    return VX_SUCCESS;
}

vx_status vxnneOperation_Deinitialize(vxnne_operation_s *operation)
{
    return VX_SUCCESS;
}

vx_status vxnneOperation_Initialize(
                vxnne_operation_s               *operation,
                vxnne_layer                     layer,
                vxnne_operation_target_e        target,
                vxnne_operator_e                operatorType,
                vxnne_operation_execute_f       execute,
                vxnne_operation_deinitialize_f  deinitialize
                )
{
    operation->layer         = layer;
    operation->target        = target;
    operation->operatorType = operatorType;
    operation->execute       = execute;
    operation->initialize    = VX_NULL;
    operation->deinitialize  = (deinitialize ? deinitialize :  vxnneOperation_Deinitialize);
    operation->dump          = VX_NULL;
    return VX_SUCCESS;
}

vx_status vxnneShaderOperation_Execute(vxnne_operation_s *operation)
{
    vx_status status;
    vx_uint32 i;
    vxnne_shader_operation shaderOperation  = (vxnne_shader_operation)operation;
    vx_shader    kernelShader;

    kernelShader = shaderOperation->shaderExecutable->kernelShader;

    status = vxoShader_SetParameters(kernelShader, shaderOperation->shaderExecutable->param, shaderOperation->shaderExecutable->paramNum, VX_NULL);
    if (status != VX_SUCCESS) goto error;

    for(i = 0; i < shaderOperation->shaderExecutable->uniformCount; i++)
    {
        status = vxoShader_SetUniform(
                        kernelShader,
                        shaderOperation->shaderExecutable->uniforms[i].name,
                        shaderOperation->shaderExecutable->uniforms[i].count,
                        shaderOperation->shaderExecutable->uniforms[i].data);
        if (status != VX_SUCCESS) goto error;
    }

    status = vxoShader_Execute(kernelShader,
                                    &shaderOperation->shaderExecutable->borderMode,
                                    &shaderOperation->shaderExecutable->shaderParam,
                                    shaderOperation->base.layer->node->base.context->devices,
                                    shaderOperation->base.layer->node->base.context->deviceCount);

    if (operation->layer->node->base.context->perfEnable)
    {
        gcoVX_Flush(gcvTRUE);
    }

error:
    return status;
}

vx_status vxnneShaderExecutable_Destroy(vxnne_shader_executable shaderExecutable)
{
    vx_uint32 i;

    for (i = 0; i < shaderExecutable->paramNum; i++)
    {
       vxoReference_Release(&shaderExecutable->param[i], shaderExecutable->param[i]->type, VX_REF_INTERNAL);
    }

    if (shaderExecutable->uniforms)
    {
        for(i = 0 ; i < shaderExecutable->uniformCount; i++)
        {
            gcoOS_Free(gcvNULL, shaderExecutable->uniforms[i].data);
        }

        gcoOS_Free(gcvNULL, shaderExecutable->uniforms);
    }

    gcoOS_Free(gcvNULL, shaderExecutable);

    return VX_SUCCESS;
}

vx_status vxnneShaderOperation_Deinitialize(vxnne_operation_s *operation)
{
    vxnne_shader_operation shader_operation = (vxnne_shader_operation)operation;
    if (shader_operation->shaderExecutable)
    {
        vxnneShaderExecutable_Destroy(shader_operation->shaderExecutable);

        shader_operation->shaderExecutable = VX_NULL;
    }

    return VX_SUCCESS;
}

vx_status vxnneShaderExecutable_SetParameters(vxnne_shader_executable shaderExecutable, vx_reference parameters[], vx_uint32 paramNum)
{
    vx_uint32 i;

    if (paramNum > VX_MAX_SHADER_PARAMETERS) goto error;

    for (i = 0; i < paramNum; i++)
    {
        shaderExecutable->param[i] = parameters[i];
        vxoReference_Increment(shaderExecutable->param[i], VX_REF_INTERNAL);
    }

    shaderExecutable->paramNum = paramNum;

    return VX_SUCCESS;
error:
    return VX_FAILURE;
}

vx_status vxnneShaderExecutable_SetExecutionParameters(vxnne_shader_executable shaderExecutable, vx_kernel_execution_parameters_t *shaderParam)
{
    shaderExecutable->shaderParam = *shaderParam;

    return VX_SUCCESS;
}

vx_status vxnneShaderExecutable_SetUniform(vxnne_shader_executable shaderExecutable, vx_char *name, vx_uint32 count, void * value)
{
    vx_uint32 size;
    vx_status vStatus = VX_FAILURE;
    gceSTATUS status;

    if (shaderExecutable->uniformCount >= shaderExecutable->kernelShader->numArgs) goto error;

    if (!shaderExecutable->uniforms)
    {
        /*allocat the maximum number uniforms */
        status = gcoOS_Allocate(gcvNULL, shaderExecutable->kernelShader->numArgs * gcmSIZEOF(vx_node_s), (gctPOINTER*)&shaderExecutable->uniforms);
        if (gcmIS_ERROR(status))
        {
            vStatus = VX_FAILURE;
            goto error;
        }
    }

    vStatus = vxoShader_GetUniformSize(shaderExecutable->kernelShader, name, &size);
    if (vStatus != VX_SUCCESS) goto error;

    status = gcoOS_Allocate(gcvNULL, size, (gctPOINTER*)&shaderExecutable->uniforms[shaderExecutable->uniformCount].data);
    if (gcmIS_ERROR(status))
    {
        vStatus = VX_FAILURE;
        goto error;
    }

    gcoOS_MemCopy(shaderExecutable->uniforms[shaderExecutable->uniformCount].data, value, size);

    shaderExecutable->uniforms[shaderExecutable->uniformCount].count = count;
    shaderExecutable->uniforms[shaderExecutable->uniformCount].name  = name;
    shaderExecutable->uniforms[shaderExecutable->uniformCount].size  = size;

    shaderExecutable->uniformCount++;

error:

    return vStatus;
}

vxnne_shader_executable  vxnneKernelShaders_CreateShaderExecutable(vxnne_kernel_shaders kernel, vx_char * subName, vx_border_mode_t *borderMode)
{
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vx_char     kernelName[256]     = {0};
    vx_uint32   i, shaderID;

    gceSTATUS status = gcoOS_Allocate(gcvNULL, gcmSIZEOF(vxnne_shader_executable_s), (gctPOINTER*)&shaderExecutable);
    if (gcmIS_ERROR(status)) goto error;

    gcoOS_ZeroMemory((gctPOINTER)shaderExecutable, gcmSIZEOF(vxnne_shader_executable_s));

    shaderExecutable->borderMode = *borderMode;

    gcoOS_StrCopySafe(kernelName, 256, kernel->kernelName);

    if (subName)
    {
        gcoOS_StrCatSafe(kernelName, 256, subName);
    }

    for(i = 0; i < kernel->kernelShaderCount; i++)
    {
        if (gcoOS_StrCmp(kernel->kernelShader[i*2]->name, kernelName) == 0)
            break;
    }

    if (i == kernel->kernelShaderCount) goto error;

    shaderID = ((shaderExecutable->borderMode.mode == VX_BORDER_MODE_CONSTANT) ? 1 : 0);

    shaderExecutable->kernelShader = kernel->kernelShader[i*2 + shaderID];

    return shaderExecutable;

error:
    if (shaderExecutable) gcoOS_Free(gcvNULL, (gctPOINTER)shaderExecutable);

    return VX_NULL;
}


vx_status vxnneShaderOperation_Initialize(
    vxnne_shader_operation_s            *operation,
    vxnne_layer                         layer,
    vxnne_operator_e                    operatorType,
    vxnne_shader_executable                 shaderExecutable
    )
{
    operation->base.layer           = layer;
    operation->base.dump            = VX_NULL;
    operation->base.execute         = vxnneShaderOperation_Execute;
    operation->base.operatorType   = operatorType;
    operation->base.deinitialize    = vxnneShaderOperation_Deinitialize;
    operation->base.target          = VXNNE_OPERATION_TARGET_SH;

    operation->shaderExecutable        = shaderExecutable;

    return VX_SUCCESS;
}

vxnne_kernel_shaders vxnneGetKernelShadersByEnum(vx_context context, vx_enum kernelEnum)
{
    if (context->kernels[kernelEnum].kernelShader)
    {
        return &context->kernels[kernelEnum];
    }
    else
    {
        return VX_NULL;
    }
}

vxnne_kernel_shaders vxnneAddKernelShadersInProgram(vx_context context, vx_char* kernelName, vx_program program, vx_uint32  paramNum, vx_enum kernelEnum)
{
    vxnne_kernel_shaders kernel = &context->kernels[kernelEnum];

    /* if exists then failed to add */
    if (kernel->kernelShader) return VX_NULL;

    kernel->kernelName  = kernelName;
    kernel->kernelEnum  = kernelEnum;
    kernel->paramNum    = paramNum;

    vxoKernel_CreateShaders(
            program,
            kernelName,
            &kernel->kernelShaderCount,
            &kernel->kernelShader);

    return kernel;
}

vx_status vxnneExecuteSWReshuffle(struct _vxnne_operation_s *operation)
{
    vxnne_reshuffle_operation           reshuffleOperation   = (vxnne_reshuffle_operation)operation;

    vx_tensor inputs = (vx_tensor)reshuffleOperation->inputs;
    vx_weights_biases_parameter weights_biases = (vx_weights_biases_parameter)reshuffleOperation->weights_biases;
    vx_scalar pad_x_s = (vx_scalar)reshuffleOperation->pad_x_s;
    vx_scalar pad_y_s = (vx_scalar)reshuffleOperation->pad_y_s;
    vx_tensor outputs = (vx_tensor)reshuffleOperation->outputs;
    vx_uint32 pad_x, pad_y;
    vx_uint32 stride_x, stride_y;

    vx_status status = VX_SUCCESS;

    pad_x = pad_x_s->value->u32;
    pad_y = pad_y_s->value->u32;

    stride_x = stride_y = weights_biases->stride;

    /* if stride > 1, need do reshuffle with input buffer */
    gcmASSERT (weights_biases->stride > 1);

    {
        vxoNNExternsionDoReshuffle(inputs, outputs, pad_x, pad_y, stride_x, stride_y);
    }


    return status;
}

vx_status vxnneExecuteTPReshuffle(struct _vxnne_operation_s *operation)
{
    vxnne_convolution_relu_pooling_layer layer = (vxnne_convolution_relu_pooling_layer)operation->layer;
    vxnne_reshuffle_operation reshuffleOperation = (vxnne_reshuffle_operation)operation;

    vx_node node = layer->base.node;
    vx_tensor inputs = (vx_tensor)reshuffleOperation->inputs;
    vx_weights_biases_parameter weights_biases = (vx_weights_biases_parameter)reshuffleOperation->weights_biases;
    vx_tensor outputs = (vx_tensor)reshuffleOperation->outputs;

    if (!(weights_biases->stride > 1))
    {
        gcmASSERT(gcvFALSE);
    }

    return vxTPExecute(node, layer->base.cmdTPBuff, VX_NULL, inputs, outputs);
}

vx_status vxnneExecuteSWReSizeCopy(struct _vxnne_operation_s *operation)
{
    vxnne_resize_operation           resizeOperation   = (vxnne_resize_operation)operation;

    vx_tensor srcTensor = (vx_tensor)resizeOperation->inputs;
    vx_tensor dstTensor = (vx_tensor)resizeOperation->outputs;

    gctPOINTER srcLogical = VX_NULL;
    gctPOINTER dstLogical = VX_NULL;

    vxoTensor_GetTensorViewMemory(srcTensor, &srcLogical, VX_NULL);
    vxoTensor_GetTensorViewMemory(dstTensor, &dstLogical, VX_NULL);

    memset(dstLogical,
           0,
           dstTensor->tensorBuffer->memory.strides[0][2] * srcTensor->tensorBuffer->memory.dims[0][2]);

    vxoTensor_CopyTensorPatchEx(
        (vx_uint8_ptr)srcLogical,
        (vx_uint8_ptr)dstLogical,
        2,
        (vx_uint32*)srcTensor->tensorBuffer->memory.dims[0],
        (vx_uint32*)srcTensor->tensorBuffer->memory.strides[0],
        (vx_uint32*)dstTensor->tensorBuffer->memory.strides[0],
        srcTensor->tensorBuffer->dataFormat,
        dstTensor->tensorBuffer->dataFormat
        );

    return VX_SUCCESS;
}

vx_status vxnneExecuteConvolutionReluPooling(struct _vxnne_operation_s *operation)
{
    vxnne_convolution_relu_pooling_layer layer                         = (vxnne_convolution_relu_pooling_layer)operation->layer;
    vxnne_convolution_relu_pooling_operation   convolutionOperation   = (vxnne_convolution_relu_pooling_operation)operation;

    vx_node node = layer->base.node;

    vx_tensor inputs = (vx_tensor)convolutionOperation->inputs;
    vx_weights_biases_parameter weights_biases = (vx_weights_biases_parameter)convolutionOperation->weights_biases;
    vx_tensor outputs = (vx_tensor)convolutionOperation->outputs;

    vx_uint32 input_offset = 0;
    vx_uint32 output_offset = 0;
    vx_uint32 dim_index;
    vx_status status = VX_SUCCESS;

    if (inputs->isViewed)
    {
        for (dim_index = 0; dim_index < inputs->viewRegion.dimCount; dim_index++)
        {
            input_offset += inputs->viewRegion.viewStarts[dim_index] * inputs->tensorBuffer->memory.strides[0][dim_index];
        }
    }

    if (outputs->isViewed)
    {
        for (dim_index = 0; dim_index < outputs->viewRegion.dimCount; dim_index++)
        {
            output_offset += outputs->viewRegion.viewStarts[dim_index] * outputs->tensorBuffer->memory.strides[0][dim_index];
        }
    }


    /* need compute input and output offset */
    /* NN Engine programming. */
    status = vxNNExecute(node, layer->base.cmdNNBuff, 0, weights_biases, 0, inputs, input_offset, outputs, output_offset);

    return status;

}

vx_status vxnneExecuteSWFullyConnected(struct _vxnne_operation_s *operation)
{
    vxnne_fully_connected_sw_operation           fullyConnectedOperation   = (vxnne_fully_connected_sw_operation)operation;

    vx_tensor inputs  = (vx_tensor)fullyConnectedOperation->inputs;
    vx_tensor weights = (vx_tensor)fullyConnectedOperation->weights;
    vx_tensor biases  = (vx_tensor)fullyConnectedOperation->biases;
    vx_tensor outputs = (vx_tensor)fullyConnectedOperation->outputs;
    gctPOINTER inputsBaseLogicalAddr = VX_NULL, outputsBaseLogicalAddr = VX_NULL;
    gctPOINTER weightsBaseLogicalAddr = VX_NULL, biasesBaseLogicalAddr = VX_NULL;
    vx_uint32 i = 0, j = 0;
    vx_uint32 inputCount, outputCount;
    vx_float32 madValue, inputValue, weightValue, biasValue = 0.0f;
    vx_enum srcType, dstType, weightsType, biasesType;
    vx_uint8 inputFpPos = 0, weightFpPos = 0, biasFpPos = 0, outputFpPos = 0;
    vx_float32 result = 0.0f;
    vx_status status = VX_SUCCESS;

    srcType = inputs->tensorBuffer->dataFormat;
    dstType = outputs->tensorBuffer->dataFormat;
    weightsType = weights->tensorBuffer->dataFormat;
    biasesType = biases->tensorBuffer->dataFormat;

    vxoTensor_GetTensorViewMemory(inputs, &inputsBaseLogicalAddr, VX_NULL);
    vxoTensor_GetTensorViewMemory(outputs, &outputsBaseLogicalAddr, VX_NULL);
    vxoTensor_GetTensorViewMemory(weights, &weightsBaseLogicalAddr, VX_NULL);
    vxoTensor_GetTensorViewMemory(biases, &biasesBaseLogicalAddr, VX_NULL);

    inputCount = (vx_uint32)vxoMemory_ComputeElementCount(&inputs->tensorBuffer->memory, 0);
    outputCount = (vx_uint32)vxoMemory_ComputeElementCount(&outputs->tensorBuffer->memory, 0);

    inputFpPos = inputs->tensorBuffer->fixedPointPos;
    weightFpPos = weights->tensorBuffer->fixedPointPos;
    biasFpPos = biases->tensorBuffer->fixedPointPos;
    outputFpPos = outputs->tensorBuffer->fixedPointPos;

    for (i = 0; i < outputCount; i++)
    {
        madValue = 0.0;
        for (j = 0; j < inputCount; j++)
        {
            if (((srcType == VX_TYPE_FLOAT16) && (weightsType == VX_TYPE_FLOAT16) && (biasesType == VX_TYPE_FLOAT32)) ||
                ((srcType == VX_TYPE_FLOAT32) && (weightsType == VX_TYPE_FLOAT32) && (biasesType ==  VX_TYPE_FLOAT32)) ||
                ((srcType == VX_TYPE_INT8) && (weightsType == VX_TYPE_INT8) && (biasesType == VX_TYPE_INT32 || biasesType == VX_TYPE_FLOAT32)))
            {
                inputValue  = vxnneGetDate((vx_type_e)srcType, j, (vx_uint8_ptr)inputsBaseLogicalAddr, inputFpPos);
                weightValue = vxnneGetDate((vx_type_e)weightsType, inputCount * i + j, (vx_uint8_ptr)weightsBaseLogicalAddr, weightFpPos);

                madValue += inputValue * weightValue;
            }
            else
            {
                /* other format not surpport now */
                printf("can't support this input data format\n");
                gcmASSERT(0);
            }
        }

        if (biasesType == VX_TYPE_FLOAT32)
        {
            biasValue = vxnneGetDate((vx_type_e)biasesType, i, (vx_uint8_ptr)biasesBaseLogicalAddr, biasFpPos);
        }
        else if ((weightsType == VX_TYPE_INT8) && (biasesType == VX_TYPE_INT32))
        {
            vx_int8 value = (vx_int8)(*((vx_int32*)biasesBaseLogicalAddr + i));
            biasValue = Int8toFp32(value, biasFpPos);
        }
        else
        {
            printf("can't support this bias data format\n");
            gcmASSERT(0);
        }

        result = madValue + biasValue;

        vxnneSaveDate((vx_type_e)dstType, i, result, outputsBaseLogicalAddr, outputFpPos);
    }

    return status;
}

vx_status vxnneExecuteFullyConnectReluLayer(struct _vxnne_operation_s *operation)
{
    vxnne_fully_connected_relu_layer         layer                         = (vxnne_fully_connected_relu_layer)operation->layer;
    vxnne_fully_connected_relu_nne_operation fullyConnectedReluOperation   = (vxnne_fully_connected_relu_nne_operation)operation;

    vx_tensor inputs  = fullyConnectedReluOperation->inputs;
    vx_weights_biases_parameter weights_biases = fullyConnectedReluOperation->weights_biases;
    vx_tensor outputs = fullyConnectedReluOperation->outputs;

    vx_node node = layer->base.node;

    if (weights_biases->use_fc_accel)
    {
        vx_uint32 tmpPhysical;
        vx_uint8_ptr tmpLogical;

        /* interchange weight_bias and input address */
        tmpPhysical = weights_biases->memory.physicals[0];
        weights_biases->memory.physicals[0] = inputs->tensorBuffer->memory.physicals[0];
        inputs->tensorBuffer->memory.physicals[0] = tmpPhysical;

        tmpLogical = weights_biases->memory.logicals[0];
        weights_biases->memory.logicals[0] = inputs->tensorBuffer->memory.logicals[0];
        inputs->tensorBuffer->memory.logicals[0] = tmpLogical;

        vxNNExecute(node, layer->base.cmdNNBuff, 0, weights_biases, 0, inputs, 0, outputs, 0);

        /* restore weight_bias and input address */
        tmpPhysical = weights_biases->memory.physicals[0];
        weights_biases->memory.physicals[0] = inputs->tensorBuffer->memory.physicals[0];
        inputs->tensorBuffer->memory.physicals[0] = tmpPhysical;

        tmpLogical = weights_biases->memory.logicals[0];
        weights_biases->memory.logicals[0] = inputs->tensorBuffer->memory.logicals[0];
        inputs->tensorBuffer->memory.logicals[0] = tmpLogical;
    }
    else
    {
        vx_uint32 i, wbOffset, outputOffset = 0;

        for (i = 0; i < weights_biases->zgroup_num; i++)
        {
            wbOffset = (vx_uint32)weights_biases->memory_offset_array[i];

            /* NN Engine programming. */
            vxNNExecute(node, layer->base.cmdNNBuff, i*NNE_COMMAND_SIZE, weights_biases, wbOffset, inputs, 0, outputs, outputOffset);

            outputOffset += outputs->finalDims[0] * outputs->finalDims[1] * outputs->tensorBuffer->elementSize * weights_biases->zgroup_array[i];
        }
    }

    return VX_SUCCESS;
}

vx_status vxtpExecuteFullyConnectReluLayer(struct _vxnne_operation_s *operation)
{
    vxnne_fully_connected_relu_layer         layer                         = (vxnne_fully_connected_relu_layer)operation->layer;
    vxnne_fully_connected_relu_nne_operation fullyConnectedReluOperation   = (vxnne_fully_connected_relu_nne_operation)operation;

    vx_tensor inputs  = fullyConnectedReluOperation->inputs;
    vx_weights_biases_parameter weights_biases = fullyConnectedReluOperation->weights_biases;
    vx_tensor outputs = fullyConnectedReluOperation->outputs;

    vx_node node = layer->base.node;

    vxTPExecute(node, layer->base.cmdTPBuff, weights_biases, inputs, outputs);

    return VX_SUCCESS;
}

vx_status vxnneExecuteSWInputConvertWeight(struct _vxnne_operation_s *operation)
{
    vxnne_fully_connected_relu_layer layer = (vxnne_fully_connected_relu_layer)operation->layer;
    vxnne_intput_convert_weight_operation input2weightOperation = (vxnne_intput_convert_weight_operation)operation;

    vx_bool enableRelu = input2weightOperation->enable_relu;
    vx_tensor inputs  = input2weightOperation->inputs;
    vx_weights_biases_parameter weights_biases = input2weightOperation->weights_biases;
    vx_tensor outputs = input2weightOperation->outputs;
    vx_uint8 origFiexedPointPos, newFiexedPointPos = input2weightOperation->output_fp_pos;

    vx_uint32 i, j, k = 0, ySize, sliceSize, elementSize = (vx_uint32)vxDataType_GetSize(TENSOR_DATA_TYPE(inputs)), mask, tmp;
    vx_float32 halfOneF=1.0f, biasBuffer = 0.0f;
    vx_uint8 *inputBufferPtr, *newInputBufferPtr, *newInputBufferOrigPtr;
    vx_uint8 *wbOrigBufferPtr, *wbCurBufferPtr;

    mask = elementSize == 4 ? 0xFFFFFFFF : (1 << (8 * elementSize)) - 1;

    weights_biases->fc_accel_large_size = vx_false_e;
    vxoTensor_GetTensorViewMemory(inputs, (gctPOINTER*)&inputBufferPtr, VX_NULL);
    newInputBufferPtr = newInputBufferOrigPtr = (vx_uint8*) weights_biases->tmp_fcaccel_input_ptr;
    wbOrigBufferPtr =(vx_uint8*) weights_biases->tmp_fcaccel_wb_ptr;
    wbCurBufferPtr = (vx_uint8*) weights_biases->memory.logicals[0];
    sliceSize = weights_biases->org_weights_sizes[3] * elementSize;

    for (i = 0; i < weights_biases->org_weights_sizes[2]; i++, inputBufferPtr+=elementSize)
    {
        _DataGeneralConvert((void*)inputBufferPtr, (void*)newInputBufferPtr, elementSize, elementSize);
        if ((*(vx_uint32*)newInputBufferPtr) & mask)
        {
            newInputBufferPtr += elementSize;
            k++;

            memcpy(wbCurBufferPtr, wbOrigBufferPtr, sliceSize);
            wbCurBufferPtr += sliceSize;
        }
        wbOrigBufferPtr += sliceSize;
    }

    if (k > FC_Z_MAX)
    {
        /* Need reshuffle again. */
        weights_biases->fc_accel_large_size = vx_true_e;
        k = 0;
        vxoTensor_GetTensorViewMemory(inputs, (gctPOINTER*)&inputBufferPtr, VX_NULL);
        newInputBufferPtr = newInputBufferOrigPtr = (vx_uint8*) weights_biases->tmp_fcaccel_input_ptr;
        wbOrigBufferPtr =(vx_uint8*) weights_biases->tmp_fcaccel_wb_ptr;
        wbCurBufferPtr = (vx_uint8*) weights_biases->memory.logicals[0];
        ySize = weights_biases->weights_sizes[1] * elementSize;
        sliceSize = weights_biases->weights_sizes[1] * weights_biases->org_weights_sizes[3] * elementSize;

        for (i = 0; i < weights_biases->weights_sizes[2]; i++, inputBufferPtr+=ySize)
        {
            vx_uint8* inputBufferPtrTmp = inputBufferPtr;
            for (j = 0; j < weights_biases->weights_sizes[1]; j++, inputBufferPtrTmp+=elementSize)
            {
                _DataGeneralConvert((void*)inputBufferPtrTmp, (void*)&tmp, elementSize, elementSize);
                if (tmp & mask) break;
            }

            if (j < weights_biases->weights_sizes[1])
            {
                memcpy(newInputBufferPtr, inputBufferPtr, ySize);
                newInputBufferPtr += ySize;
                k++;

                memcpy(wbCurBufferPtr, wbOrigBufferPtr, sliceSize);
                wbCurBufferPtr += sliceSize;
            }

            wbOrigBufferPtr += sliceSize;
        }
    }

    /* For bias. */
    _DataGeneralConvert((void*)&halfOneF, (void*)newInputBufferPtr, sizeof(halfOneF), elementSize);
    k++;

    memcpy(wbCurBufferPtr, wbOrigBufferPtr, sliceSize);

    weights_biases->input_nonzero_count = k;

    origFiexedPointPos = outputs->tensorBuffer->fixedPointPos;
    outputs->tensorBuffer->fixedPointPos = newFiexedPointPos;

    fillinCmmdBuff(
        inputs,
        weights_biases,
        0, 0,
        VX_CONVOLUTIONAL_NETWORK_DS_SIZE_ROUNDING_FLOOR,
        enableRelu,
        VIV_NN_NONLINEAR_NON,
        0, 0,
        outputs,
        layer->base.cmdNNBuff,
        vx_true_e,
        0);

    outputs->tensorBuffer->fixedPointPos = origFiexedPointPos;

    fillinKernelBuffer(
        weights_biases,
        0, 0, /* zeroRunLen */
        1, /* weightX */
        weights_biases->fc_accel_large_size ? weights_biases->weights_sizes[1] : 1, /* weightY */
        k, /* sliceCount */
        1, /* filterTotalCount */
        1, /* kernelsPerCore */
        1, 1,
        outputs->tensorBuffer->memory.logicals[0],
        newInputBufferOrigPtr,
        (vx_uint32*)&biasBuffer
        );

    return VX_SUCCESS;
}

vx_status vxnneExecuteSWActivation(struct _vxnne_operation_s *operation)
{
    vxnne_activation_sw_operation           activationOperation   = (vxnne_activation_sw_operation)operation;

    vx_tensor inputs  = (vx_tensor)activationOperation->inputs;
    vx_scalar func = (vx_scalar)activationOperation->func;
    vx_scalar a  = (vx_scalar)activationOperation->a;
    vx_scalar b = (vx_scalar)activationOperation->b;
    vx_tensor outputs = (vx_tensor)activationOperation->outputs;

    vx_enum   func_v = func->value->e;
    vx_int32  a_v = a->value->n32;
    vx_int32  b_v = b->value->n32;

    vx_uint32 elementCount = 0;
    vx_uint32 i;
    vx_float32 value = 0.0f, result = 0.0f;
    gctPOINTER inputBase;
    gctPOINTER outputBase;

    vx_type_e inputFormat = (vx_type_e)inputs->tensorBuffer->dataFormat;
    vx_type_e outputFormat = (vx_type_e)outputs->tensorBuffer->dataFormat;
    vx_uint8 inputFpPos = inputs->tensorBuffer->fixedPointPos;
    vx_uint8 outputFpPos = outputs->tensorBuffer->fixedPointPos;

    vx_status status = VX_SUCCESS;

    elementCount = (vx_uint32)vxoMemory_ComputeElementCount(&inputs->tensorBuffer->memory, 0);
    vxoTensor_GetTensorViewMemory(inputs, &inputBase, VX_NULL);
    vxoTensor_GetTensorViewMemory(outputs, &outputBase, VX_NULL);

    if ((inputs->tensorBuffer->dataFormat != VX_TYPE_FLOAT16 && inputs->tensorBuffer->dataFormat != VX_TYPE_FLOAT32 && inputs->tensorBuffer->dataFormat != VX_TYPE_INT8)
        || (outputs->tensorBuffer->dataFormat != VX_TYPE_FLOAT16 && outputs->tensorBuffer->dataFormat != VX_TYPE_FLOAT32 && outputs->tensorBuffer->dataFormat != VX_TYPE_INT8))
    {
        gcmPRINT("input or outputs format is not support");
        status = VX_ERROR_NOT_SUPPORTED;
        return status;
    }

    for (i = 0; i < elementCount; i++)
    {
        value = vxnneGetDate(inputFormat, i, (vx_uint8_ptr)inputBase, inputFpPos);

        switch (func_v)
        {
        case VX_CONVOLUTIONAL_NETWORK_ACTIVATION_LOGISTIC:
            {
                result = 1.0f / (1 + gcoMATH_Exp(value * (-1)));
            }
            break;

        case VX_CONVOLUTIONAL_NETWORK_ACTIVATION_HYPERBOLIC_TAN:
            {
                result = a_v * gcoMATH_Tangent(b_v * value);
            }
            break;

        case VX_CONVOLUTIONAL_NETWORK_ACTIVATION_RELU:
            {
                result = gcoMATH_MAX(0.0f, value);
            }
            break;

        case VX_CONVOLUTIONAL_NETWORK_ACTIVATION_BRELU:
            {
                result = gcoMATH_MIN(a_v, gcoMATH_MAX(0.0f, value));
            }
            break;

        case VX_CONVOLUTIONAL_NETWORK_ACTIVATION_SOFTRELU:
            {
                result = gcoMATH_Log(1 + gcoMATH_Exp(value));
            }
            break;

        case VX_CONVOLUTIONAL_NETWORK_ACTIVATION_ABS:
            {
                result = gcoMATH_Absolute(value);
            }
            break;

        case VX_CONVOLUTIONAL_NETWORK_ACTIVATION_SQUARE:
            {
                result = gcoMATH_Power(value, 2);
            }
            break;

        case VX_CONVOLUTIONAL_NETWORK_ACTIVATION_SQRT:
            {
                result = gcoMATH_SquareRoot(value);
            }
            break;

        case VX_CONVOLUTIONAL_NETWORK_ACTIVATION_LINEAR:
            {
                result = a_v * value + b_v;
            }
            break;

        case VX_CONVOLUTIONAL_NETWORK_ACTIVATION_LEAKYRELU:
            {
                result = (value > 0.0f) ? value : 0.1f * value;
            }
            break;

        default:
            gcmPRINT("this activation func not support");
            status = VX_ERROR_NOT_SUPPORTED;
            return status;
        }

        vxnneSaveDate(outputFormat, i, result, outputBase, outputFpPos);
    }

    return status;

}

vx_status vxnneExecuteSWSoftmax(struct _vxnne_operation_s *operation)
{
    vxnne_softmax_operation           softmaxOperation   = (vxnne_softmax_operation)operation;

    vx_tensor inputTensor  = (vx_tensor)softmaxOperation->inputs;
    vx_tensor probBuffer = (vx_tensor)softmaxOperation->outputs;

    void * pInputBuf;
    void * pfProb;
    vx_float32 * pfProbFP32   = VX_NULL;
    vx_float32 * pfInput = VX_NULL;

    vx_uint32 i;
    vx_float32 fProbSum = 0.0f;
    vx_uint32 singleItemCount;
    vx_float32 fMax = 0.0f;

    vx_type_e inputFormat = (vx_type_e)inputTensor->tensorBuffer->dataFormat;
    vx_type_e outputFormat = (vx_type_e)probBuffer->tensorBuffer->dataFormat;
    vx_uint8 inputFpPos = inputTensor->tensorBuffer->fixedPointPos;
    vx_uint8 outputFpPos = probBuffer->tensorBuffer->fixedPointPos;

    vxoTensor_GetTensorViewMemory(inputTensor, &pInputBuf, VX_NULL);
    vxoTensor_GetTensorViewMemory(probBuffer, &pfProb, VX_NULL);

    singleItemCount = (vx_uint32)vxoMemory_ComputeElementCount(&probBuffer->tensorBuffer->memory, 0);
    pfProbFP32 = (vx_float32*)malloc(singleItemCount * sizeof(vx_float32));
    pfInput = (vx_float32*)malloc(singleItemCount * sizeof(vx_float32));

    /* according to caffe, need to subtract the max to avoid numerical issues, compute the exp, and then normalize. */
    for (i = 0; i < singleItemCount; i++)
    {
        pfInput[i] = vxnneGetDate(inputFormat, i, (vx_uint8_ptr)pInputBuf, inputFpPos);

        fMax = gcmMAX(fMax, pfInput[i]);
    }

    fProbSum = 0.0f;
    for (i = 0; i < singleItemCount; i++)
    {
        pfProbFP32[i] = gcoMATH_Exp(pfInput[i] - fMax);

        fProbSum += pfProbFP32[i];
    }

    for (i=0; i< singleItemCount; i++)
    {
        vxnneSaveDate(outputFormat, i, pfProbFP32[i] / fProbSum, pfProb, outputFpPos);
    }

    if (pfProbFP32 != VX_NULL)
    {
        free(pfProbFP32);
    }

    if (pfInput != VX_NULL)
    {
        free(pfInput);
    }

    return VX_SUCCESS;
}

vx_status vxnneExecuteSWConcat2(struct _vxnne_operation_s *operation)
{
    vxnne_concat2_sw_operation           concatOperation   = (vxnne_concat2_sw_operation)operation;

    vx_tensor input0  = (vx_tensor)concatOperation->inputs0;
    vx_tensor input1  = (vx_tensor)concatOperation->inputs1;
    vx_tensor output = (vx_tensor)concatOperation->outputs;

    vx_uint8_ptr pInput0Buf;
    vx_uint8_ptr pInput1Buf;
    vx_uint8_ptr pOutputBuf;

    vx_uint32 input0Size, input1Size;

    vxoTensor_GetTensorViewMemory(input0, (gctPOINTER *)&pInput0Buf, VX_NULL);
    vxoTensor_GetTensorViewMemory(input1, (gctPOINTER *)&pInput1Buf, VX_NULL);
    vxoTensor_GetTensorViewMemory(output, (gctPOINTER *)&pOutputBuf, VX_NULL);

    input0Size = (vx_uint32)vxoMemory_ComputeSize(&input0->tensorBuffer->memory, 0);
    input1Size = (vx_uint32)vxoMemory_ComputeSize(&input1->tensorBuffer->memory, 0);

    vxMemCopy(pOutputBuf, pInput0Buf, input0Size);
    vxMemCopy(&pOutputBuf[input0Size], pInput1Buf, input1Size);

    return VX_SUCCESS;
}

vx_status vxnneExecuteSWNormalization(struct _vxnne_operation_s *operation)
{
     vxnne_normalization_operation           normalizationOperation   = (vxnne_normalization_operation)operation;

    vx_tensor input  = (vx_tensor)normalizationOperation->inputs;
    vx_tensor output = (vx_tensor)normalizationOperation->outputs;

    void *inputsBase;
    void *outputsBase;

    vx_int32 dimCount = input->tensorBuffer->memory.dimCount;
    vx_int32 width    = input->tensorBuffer->memory.dims[0][0];
    vx_int32 height   = input->tensorBuffer->memory.dims[0][1];
    vx_int32 channel  = input->tensorBuffer->memory.dims[0][2];
    vx_int32 batch    = input->tensorBuffer->memory.dims[0][3];

    vx_int32   norm_size = normalizationOperation->norm_size;
    vx_float32 alpha     = normalizationOperation->alpha;
    vx_float32 beta      = normalizationOperation->beta;
    vx_int32   nsz2      = norm_size/2;
    vx_int32   type      = normalizationOperation->type;
    vx_int32   w=0,h=0,c=0,n=0,b=0,i=0,j=0;
    vx_type_e  inputFormat   = (vx_type_e)input->tensorBuffer->dataFormat;
    vx_type_e  outputFormat  = (vx_type_e)output->tensorBuffer->dataFormat;
    vx_uint8   inputFpPos    = input->tensorBuffer->fixedPointPos;
    vx_uint8   outputFpPos   = output->tensorBuffer->fixedPointPos;
    vx_int32   inputStridec  = input->tensorBuffer->memory.strides[0][2]/(vx_int32)vxDataType_GetSize(inputFormat);
    vx_int32   outputStridec = output->tensorBuffer->memory.strides[0][2]/(vx_int32)vxDataType_GetSize(outputFormat);

    vxoTensor_GetTensorViewMemory(input, (gctPOINTER *)&inputsBase, VX_NULL);
    vxoTensor_GetTensorViewMemory(output, (gctPOINTER *)&outputsBase, VX_NULL);

    if(dimCount==3)
        batch = 1;

    if(type == VX_CONVOLUTIONAL_NETWORK_NORM_SAME_MAP)
    {
        for(b = 0; b < batch; b++)
        {
            for(c = 0; c < channel; c++)
            {
                 for(h=0;h<height;h++)
                 {
                    for(w=0;w<width;w++)
                    {
                        vx_float32 sum=0;
                        vx_float32 val;
                         for(j = h-nsz2;j <= h+nsz2;j++)
                         {
                            for(i = w-nsz2;i <= w+nsz2;i++)
                            {
                               if(j>=0 && i>=0 && j<height && i<width)
                               {
                                   val = vxnneGetDate(inputFormat, (c+b*channel)*inputStridec + width*j + i, (vx_uint8_ptr)inputsBase, inputFpPos);
                                   sum += val*val;
                               }
                            }
                         }

                         val = vxnneGetDate(inputFormat, (c+b*channel)*inputStridec + width*h + w, (vx_uint8_ptr)inputsBase, inputFpPos);
                         val = val/(vx_float32)pow((1+(alpha/(norm_size*norm_size))*sum),beta);
                         vxnneSaveDate(outputFormat, (c+b*channel)*outputStridec + width*h + w, val, (vx_uint8_ptr)outputsBase, outputFpPos);
                    }
                 }
            }
        }
    }
    else
    {
        for(b = 0; b < batch; b++)
        {
            for(c = 0; c < channel; c++)
            {
                 for(h=0;h<height;h++)
                 {
                    for(w=0;w<width;w++)
                    {
                        vx_float32 sum=0;
                        vx_float32 val;
                        for(n = c-nsz2; n <= c+nsz2; n++)
                        {
                            if(n<0 || n >= channel)
                                continue;
                            val = vxnneGetDate(inputFormat, (n+b*channel)*inputStridec + width*h + w, (vx_uint8_ptr)inputsBase, inputFpPos);
                            sum += val*val;
                        }
                        val = vxnneGetDate(inputFormat, (c+b*channel)*inputStridec + width*h + w, (vx_uint8_ptr)inputsBase, inputFpPos);
                        val =val/(vx_float32)pow((1+(alpha/norm_size)*sum),beta);
                        vxnneSaveDate(outputFormat, (c+b*channel)*outputStridec + width*h + w, val, (vx_uint8_ptr)outputsBase, outputFpPos);
                    }
                 }
            }
        }
    }

    return VX_SUCCESS;

}

VX_INTERNAL_API vx_int32 vxnneGetTypeSize(vx_type_e format)
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

#define FP32_MAX 3.402823466e+38F

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
    vx_int32 pad,
    vx_enum rounding,
    vx_uint8_ptr dst,
    vx_uint8 dstFixPointPos,
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
    vx_int32 pad_v = pad;
    vx_int32 width_o = (vx_uint32)(vxoNNExternsionConvlutionRound((vx_float32)((width + 2.0f * pad_v - kernel_v)/stride_v + 1), rounding));
    vx_int32 height_o = (vx_uint32)(vxoNNExternsionConvlutionRound((vx_float32)((height + 2.0f * pad_v - kernel_v)/stride_v + 1), rounding));

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
                vx_int32 pad_h = pad_v, pad_w = pad_v;
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

                        vx_float32 d = vxnneGetDate(srcFormat, index, (vx_uint8_ptr)data, srcFixPointPos);

                        if (d > d_f32)
                            d_f32 = d;

                    }
                }

                vxnneSaveDate(dstFormat, pool_index, d_f32, data_d, dstFixPointPos);
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
    vx_int32 pad,
    vx_enum rounding,
    vx_uint8_ptr dst,
    vx_uint8 dstFixPointPos,
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
    vx_int32 pad_v = pad;
    vx_int32 width_o = (vx_uint32)(vxoNNExternsionConvlutionRound((vx_float32)((width + 2.0f * pad_v - kernel_v)/stride_v), rounding) + 1);
    vx_int32 height_o = (vx_uint32)(vxoNNExternsionConvlutionRound((vx_float32)((height + 2.0f * pad_v - kernel_v)/stride_v), rounding) + 1);

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
                vx_int32 pad_h = pad_v, pad_w = pad_v;
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
                        sum += vxnneGetDate(srcFormat, index, (vx_uint8_ptr)data, srcFixPointPos);
                    }
                }

                vxnneSaveDate(dstFormat, pool_index, sum/((hend - hstart) * (wend - wstart)), data_d, dstFixPointPos);
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
    vx_int32 pad,
    vx_enum rounding,
    vx_uint8_ptr dst,
    vx_uint8 dstFixPointPos,
    vx_type_e dstFormat)
{
    switch (type)
    {
        case 1:
            vxnnePoolingMax(src, srcFixPointPos, srcFormat, input_width, input_height, depth, output_width, output_height, stride, kernel_size, pad, rounding, dst, dstFixPointPos, dstFormat);
        break;
        case 2:
            vxnnePoolingAvg(src, srcFixPointPos, srcFormat, input_width, input_height, depth, output_width, output_height, stride, kernel_size, pad, rounding, dst, dstFixPointPos, dstFormat);
        break;
    }

    return VX_SUCCESS;
}

vx_status vxnneExecuteSWPooling(struct _vxnne_operation_s *operation)
{
    vxnne_pooling_operation           poolingOperation   = (vxnne_pooling_operation)operation;

    vx_tensor inputs  = (vx_tensor)poolingOperation->inputs;
    vx_tensor outputs = (vx_tensor)poolingOperation->outputs;

    vx_enum  poolType_v = poolingOperation->pool_type;
    vx_uint32 poolSizeX_v = poolingOperation->pool_size_x;
    vx_uint32 poolPadX_v = poolingOperation->pool_pad_x;
    vx_enum rounding_v = poolingOperation->rounding;

    vx_status status = VX_SUCCESS;

    gctPOINTER inputsBaseLogicalAddr = VX_NULL;
    gctPOINTER outputsBaseLogicalAddr = VX_NULL;

    vx_int32 inputs_width, inputs_height, depth, outputs_width, outputs_height, out_w, out_h;
    vx_uint32 stride;
    vx_int32 type;

    vxoTensor_GetTensorViewMemory(inputs, &inputsBaseLogicalAddr, VX_NULL);
    vxoTensor_GetTensorViewMemory(outputs, &outputsBaseLogicalAddr, VX_NULL);

    inputs_width   = inputs->tensorBuffer->memory.dims[0][0];
    inputs_height  = inputs->tensorBuffer->memory.dims[0][1];
    depth          = inputs->tensorBuffer->memory.dims[0][2];
    outputs_width  = outputs->tensorBuffer->memory.dims[0][0];
    outputs_height = outputs->tensorBuffer->memory.dims[0][1];

    switch (poolType_v)
    {
    case VX_CONVOLUTIONAL_NETWORK_POOLING_MAX:
        type = 1;
        break;
    case VX_CONVOLUTIONAL_NETWORK_POOLING_AVG:
        type = 2;
        break;
    default:
        gcmPRINT("not support this pool type");
        return VX_ERROR_INVALID_PARAMETERS;
    }

    if ((outputs_width - 1) == 0)
    {
        stride = 1;
    }
    else
    {
        /* Calculate stride = (w + 2*pad - weight)/(output_w - 1) */
        stride = vxoNNExternsionConvlutionRound((vx_float32)(inputs_width + 2 * poolPadX_v - poolSizeX_v) / (outputs_width - 1), rounding_v);
    }

    status = vxnnePoolingCpu((vx_uint8_ptr)inputsBaseLogicalAddr,
                             inputs->tensorBuffer->fixedPointPos,
                             type,
                             (vx_type_e)inputs->tensorBuffer->dataFormat,
                             inputs_width,
                             inputs_height,
                             depth,
                             &out_w,
                             &out_h,
                             stride,
                             poolSizeX_v,
                             poolPadX_v,
                             rounding_v,
                             (vx_uint8_ptr)outputsBaseLogicalAddr,
                             outputs->tensorBuffer->fixedPointPos,
                             (vx_type_e)outputs->tensorBuffer->dataFormat);
    gcmASSERT((out_w == outputs_width) && (out_h == outputs_height));

    return status;

}

vx_status vxnneExecuteTPPooling(struct _vxnne_operation_s *operation)
{
    vxnne_pooling_layer layer = (vxnne_pooling_layer)operation->layer;
    vxnne_pooling_operation poolingOperation   = (vxnne_pooling_operation)operation;

    vx_node node = layer->base.node;
    vx_tensor inputs = (vx_tensor)poolingOperation->inputs;
    vx_tensor outputs = (vx_tensor)poolingOperation->outputs;

    return vxTPExecute(node, layer->base.cmdTPBuff, VX_NULL, inputs, outputs);
}

vx_status vxnneExecutePooling(struct _vxnne_operation_s *operation)
{
    vxnne_pooling_layer layer = (vxnne_pooling_layer)operation->layer;
    vxnne_pooling_operation poolingOperation = (vxnne_pooling_operation)operation;

    vx_tensor inputs  = (vx_tensor)poolingOperation->inputs;
    vx_tensor outputs = (vx_tensor)poolingOperation->outputs;

    vx_node node = layer->base.node;

    /* NN Engine programming. */
    vxNNExecute(node, layer->base.cmdNNBuff, 0, poolingOperation->weights_biases, 0, inputs, 0, outputs, 0);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNConvolutionReluPoolingLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return vxnneLayer_Execute(node->layer);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluPoolingLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluPoolingLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxnneConvolutionReluPoolingerInitializer(
    vx_node node,
    char* name,
    vx_tensor inputs,
    vx_weights_biases_parameter weights_biases,
    vx_scalar pad_x,
    vx_scalar pad_y,
    vx_enum conv_rounding_type,
    vx_bool enable_relu,
    vx_bool enable_pooling,
    vx_enum pool_type,
    vx_uint32 pool_size_x,
    vx_uint32 pool_size_y,
    vx_tensor outputs
    )
{
    vx_status status = VX_SUCCESS;

    vx_context context;
    vx_int32 pad_x_v, pad_y_v;

    vxnne_convolution_relu_pooling_layer  convolutionReluPoolingLayer;

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_convolution_relu_pooling_layer_s), (gctPOINTER*)&convolutionReluPoolingLayer);
    if (!convolutionReluPoolingLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        return status;
    }

    gcoOS_ZeroMemory(convolutionReluPoolingLayer, sizeof(vxnne_convolution_relu_pooling_layer_s));

    vxnneLayer_Initialize(&convolutionReluPoolingLayer->base,
                            name,
                            node,
                            convolutionReluPoolingLayer->operations,
                            VX_NULL);


    vxnneOperation_Initialize(&convolutionReluPoolingLayer->convolution_operation.base,
                              &convolutionReluPoolingLayer->base,
                              VXNNE_OPERATION_TARGET_NN,
                              VXNNE_OPERATOR_CONVOLUTION,
                              vxnneExecuteConvolutionReluPooling,
                              VX_NULL);

    pad_x_v               = pad_x->value->u32;
    pad_y_v               = pad_y->value->u32;

    if (weights_biases->stride > 1)
    {

        vx_uint32 dims = inputs->tensorBuffer->memory.dimCount;
        vx_uint32 sizes[3] = {gcmALIGN(inputs->tensorBuffer->memory.dims[0][0] + 2 * pad_x_v, weights_biases->stride),
                                gcmALIGN(inputs->tensorBuffer->memory.dims[0][1] + 2 * pad_y_v, weights_biases->stride),
                                inputs->tensorBuffer->memory.dims[0][2]};

        vx_tensor reshuffleTensor = vxoTensor_CreateTensor(node->base.context, dims, sizes, TENSOR_DATA_TYPE(inputs), vx_false_e);
        if (vxoTensor_AllocateMemory(reshuffleTensor) != VX_SUCCESS)
        {
            gcmPRINT("vxoTensor_AllocateMemory fail at function %s, line %d", __FUNCTION__, __LINE__);
            status = VX_ERROR_NO_MEMORY;
            goto exit;
        }

        convolutionReluPoolingLayer->base.num_operations    = 2;

#if NN_TP_ENGINE
        if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_TP_ENGINE))
        {
            vxnneOperation_Initialize(&convolutionReluPoolingLayer->reshuffle_operation.base,
                                      &convolutionReluPoolingLayer->base,
                                      VXNNE_OPERATION_TARGET_TP,
                                      VXNNE_OPERATOR_RESHUFFLE,
                                      vxnneExecuteTPReshuffle,
                                      VX_NULL);

            /* create cmd buffer for TP operation */
            context = vxGetContext((vx_reference)node);
            convolutionReluPoolingLayer->base.cmdTPBuff = vxCreateArray(context, VX_TYPE_CHAR, TP_COMMAND_SIZE);
            if (!vxoArray_AllocateMemory(convolutionReluPoolingLayer->base.cmdTPBuff))
            {
                status = VX_ERROR_NO_MEMORY;
                gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
                goto exit;
            }

            fillInCmmdTPBuffer(inputs, weights_biases, reshuffleTensor, convolutionReluPoolingLayer->base.cmdTPBuff, 0, 0, 0, 0, vx_false_e, TP_RESHUFFLE);
        }
        else
#endif
        {
            vxnneOperation_Initialize(&convolutionReluPoolingLayer->reshuffle_operation.base,
                                        &convolutionReluPoolingLayer->base,
                                        VXNNE_OPERATION_TARGET_SW,
                                        VXNNE_OPERATOR_RESHUFFLE,
                                        vxnneExecuteSWReshuffle,
                                        VX_NULL);
        }

        convolutionReluPoolingLayer->operations[0] = (vxnne_operation)&convolutionReluPoolingLayer->reshuffle_operation;

        convolutionReluPoolingLayer->reshuffle_operation.inputs         = inputs;
        convolutionReluPoolingLayer->reshuffle_operation.weights_biases = weights_biases;
        convolutionReluPoolingLayer->reshuffle_operation.pad_x_s        = pad_x;
        convolutionReluPoolingLayer->reshuffle_operation.pad_y_s        = pad_y;
        convolutionReluPoolingLayer->reshuffle_operation.outputs        = reshuffleTensor;

        convolutionReluPoolingLayer->operations[1] = (vxnne_operation)&convolutionReluPoolingLayer->convolution_operation;

        convolutionReluPoolingLayer->convolution_operation.inputs           = reshuffleTensor;
        convolutionReluPoolingLayer->convolution_operation.weights_biases   = weights_biases;
        convolutionReluPoolingLayer->convolution_operation.outputs          = outputs;

        convolutionReluPoolingLayer->base.num_temp_tensors                  = 1;
        convolutionReluPoolingLayer->base.temp_tensors[0] = reshuffleTensor;

    }
    else
    {
        vx_uint32 input_width   = inputs->tensorBuffer->memory.dims[0][0];
        vx_uint32 input_height  = inputs->tensorBuffer->memory.dims[0][1];
        vx_uint32 org_kernel_x  = weights_biases->org_weights_sizes[0];
        vx_uint32 org_kernel_y  = weights_biases->org_weights_sizes[1];
        vx_uint32 conv_stride_x = weights_biases->stride;
        vx_uint32 conv_stride_y = conv_stride_x;

        vxoNNExternsionInputOutputArguments(
            input_width, input_height,
            conv_stride_x, conv_stride_y,
            pad_x_v, pad_y_v,
            org_kernel_x, org_kernel_y,
            conv_rounding_type,
            pool_size_x,
            &inputs->insideDims[0], &inputs->insideDims[1],
            &outputs->insideDims[0], &outputs->insideDims[1],
            &inputs->finalDims[0], &inputs->finalDims[1],
            &outputs->finalDims[0], &outputs->finalDims[1]);

        if (inputs->insideDims[0] < inputs->finalDims[0] || inputs->insideDims[1] < inputs->finalDims[1])
        {
            vx_uint32 sizes[3] =
            {
                inputs->finalDims[0],
                inputs->finalDims[1],
                inputs->tensorBuffer->memory.dims[0][2]
            };

            vx_tensor tmp_tensor;

            tmp_tensor = vxoTensor_CreateTensor(
                            inputs->base.context,
                            inputs->tensorBuffer->memory.dimCount,
                            sizes,
                            inputs->tensorBuffer->dataFormat,
                            vx_false_e);

            if (tmp_tensor == VX_NULL ||
                vxoTensor_AllocateMemory(tmp_tensor) != VX_SUCCESS)
            {
                gcmPRINT("Fail at function %s, line %d", __FUNCTION__, __LINE__);
                status = VX_ERROR_NO_MEMORY;
                goto exit;
            }

            vxnneOperation_Initialize(&convolutionReluPoolingLayer->resize_operation.base,
                                        &convolutionReluPoolingLayer->base,
                                        VXNNE_OPERATION_TARGET_SW,
                                        VXNNE_OPERATOR_RESIZE,
                                        vxnneExecuteSWReSizeCopy,
                                        VX_NULL);

            convolutionReluPoolingLayer->base.num_operations    = 2;
            convolutionReluPoolingLayer->operations[0] = (vxnne_operation)&convolutionReluPoolingLayer->resize_operation;

            convolutionReluPoolingLayer->resize_operation.inputs         = inputs;
            convolutionReluPoolingLayer->resize_operation.outputs        = tmp_tensor;

            convolutionReluPoolingLayer->operations[1] = (vxnne_operation)&convolutionReluPoolingLayer->convolution_operation;

            convolutionReluPoolingLayer->convolution_operation.inputs           = tmp_tensor;
            convolutionReluPoolingLayer->convolution_operation.weights_biases   = weights_biases;
            convolutionReluPoolingLayer->convolution_operation.outputs          = outputs;

            convolutionReluPoolingLayer->base.num_temp_tensors                  = 1;
            convolutionReluPoolingLayer->base.temp_tensors[0] = tmp_tensor;


        }
        else
        {
            convolutionReluPoolingLayer->base.num_operations    = 1;
            convolutionReluPoolingLayer->operations[0] = (vxnne_operation)&convolutionReluPoolingLayer->convolution_operation;

            convolutionReluPoolingLayer->convolution_operation.inputs           = inputs;
            convolutionReluPoolingLayer->convolution_operation.weights_biases   = weights_biases;
            convolutionReluPoolingLayer->convolution_operation.outputs          = outputs;
        }
    }


    /* create cmd buffer for nn layer */
    context = vxGetContext((vx_reference)node);
    convolutionReluPoolingLayer->base.cmdNNBuff  = vxCreateArray(context, VX_TYPE_CHAR, NNE_COMMAND_SIZE);
    if (!vxoArray_AllocateMemory(convolutionReluPoolingLayer->base.cmdNNBuff))
    {
        status = VX_ERROR_NO_MEMORY;
        gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        goto exit;
    }
    if (enable_pooling)
    {
        fillinCmmdBuff(inputs, weights_biases, pad_x_v, pad_y_v, conv_rounding_type, enable_relu, pool_type, pool_size_x, pool_size_y, outputs, convolutionReluPoolingLayer->base.cmdNNBuff, vx_false_e, 0);
    }
    else
    {
        fillinCmmdBuff(inputs, weights_biases, pad_x_v, pad_y_v, conv_rounding_type, enable_relu, pool_type, 0, 0, outputs, convolutionReluPoolingLayer->base.cmdNNBuff, vx_false_e, 0);
    }

    node->layer = &convolutionReluPoolingLayer->base;

    return status;

exit:
    gcoOS_Free(gcvNULL, (gctPOINTER)convolutionReluPoolingLayer);
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluPoolingLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_tensor                   inputs = (vx_tensor)parameters[0];
    vx_weights_biases_parameter weights_biases = (vx_weights_biases_parameter)parameters[1]; // need modify
    vx_scalar                   pad_x_s = (vx_scalar)parameters[2];
    vx_scalar                   pad_y_s = (vx_scalar)parameters[3];
    vx_scalar                   down_scale_size_rounding_s = (vx_scalar)parameters[7];
    vx_scalar                   enable_relu_s = (vx_scalar)parameters[8];
    vx_scalar                   pool_type_s = (vx_scalar)parameters[9];
    vx_scalar                   pool_size_x_s = (vx_scalar)parameters[10];
    vx_scalar                   pool_size_y_s = (vx_scalar)parameters[11];
    vx_tensor                   outputs = (vx_tensor)parameters[12];

    vx_enum                     conv_rounding_type;
    vx_enum                     pool_type;
    vx_uint32                   pool_size_x;
    vx_uint32                   pool_size_y;
    vx_bool                     enable_relu;

    vx_status                   status = VX_SUCCESS;

    conv_rounding_type   = down_scale_size_rounding_s->value->e;
    pool_type            = pool_type_s->value->e;
    pool_size_x          = pool_size_x_s->value->u32;
    pool_size_y          = pool_size_y_s->value->u32;
    enable_relu          = enable_relu_s->value->b;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    status = vxnneConvolutionReluPoolingerInitializer(node,
                                                     "ConvolutionReluPooingLayer",
                                                      inputs,
                                                      weights_biases,
                                                      pad_x_s,
                                                      pad_y_s,
                                                      conv_rounding_type,
                                                      enable_relu,
                                                      vx_true_e,
                                                      pool_type,
                                                      pool_size_x,
                                                      pool_size_y,
                                                      outputs);
    return status;

}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluPoolingLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNConvolutionReluLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return vxnneLayer_Execute(node->layer);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    if (index != vxmLENGTH_OF(nn_ConvolutionReluLayer_params) - 1) return VX_ERROR_INVALID_PARAMETERS;

    ptr->type                 = VX_TYPE_TENSOR;

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{

    vx_tensor                   inputs = (vx_tensor)parameters[0];
    vx_weights_biases_parameter weights_biases = (vx_weights_biases_parameter)parameters[1]; // need modify
    vx_scalar                   pad_x_s = (vx_scalar)parameters[2];
    vx_scalar                   pad_y_s = (vx_scalar)parameters[3];
    vx_scalar                   down_scale_size_rounding_s = (vx_scalar)parameters[7];
    vx_scalar                   enable_relu_s = (vx_scalar)parameters[8];
    vx_tensor                   outputs = (vx_tensor)parameters[9];

    vx_enum                     conv_rounding_type;
    vx_bool                     enable_relu;

    vx_status                   status = VX_SUCCESS;

    conv_rounding_type   = down_scale_size_rounding_s->value->e;
    enable_relu          = enable_relu_s->value->b;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    status = vxnneConvolutionReluPoolingerInitializer(node,
                                                      "ConvolutionReluLayer",
                                                      inputs,
                                                      weights_biases,
                                                      pad_x_s,
                                                      pad_y_s,
                                                      conv_rounding_type,
                                                      enable_relu,
                                                      vx_false_e,
                                                      VIV_NN_POOLING_NON,
                                                      0,
                                                      0,
                                                      outputs);
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionReluLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNFullyConnectedReluLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return vxnneLayer_Execute(node->layer);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNFullyConnectedReluLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNFullyConnectedReluLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    if (index != vxmLENGTH_OF(nn_FullyConnectedReluLayer_params) - 1) return VX_ERROR_INVALID_PARAMETERS;

    ptr->type                 = VX_TYPE_TENSOR;

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNFullyConnectedReluLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_context                  context;
    vx_tensor                   inputs = (vx_tensor)parameters[0];
    vx_weights_biases_parameter weights_biases = (vx_weights_biases_parameter)parameters[1]; // need modify
    vx_scalar                   pad_s = (vx_scalar)parameters[2];
    vx_scalar                   down_scale_size_rounding_s = (vx_scalar)parameters[6];
    vx_scalar                   enable_relu_s = (vx_scalar)parameters[7];
    vx_tensor                   outputs = (vx_tensor)parameters[8];

    vx_uint32                   pad;
    vx_enum                     conv_rounding_type;
    vx_bool                     enable_relu;

    vx_status                   status = VX_SUCCESS;

    pad                  = pad_s->value->u32;
    conv_rounding_type   = down_scale_size_rounding_s->value->e;
    enable_relu          = enable_relu_s->value->b;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    {
        vxnne_fully_connected_relu_layer  fullyConnectReluLayer;

        gcoOS_Allocate(gcvNULL, sizeof(vxnne_fully_connected_relu_layer_s), (gctPOINTER*)&fullyConnectReluLayer);
        if (!fullyConnectReluLayer)
        {
            status = VX_ERROR_NO_MEMORY;
            gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
            goto exit;
        }

        gcoOS_ZeroMemory(fullyConnectReluLayer, sizeof(vxnne_fully_connected_relu_layer_s));

        vxnneLayer_Initialize(&fullyConnectReluLayer->base,
                              "FullyConnectReluLayer",
                              node,
                              fullyConnectReluLayer->operations,
                              VX_NULL);


        {
            vx_uint32 i;

            vxnneOperation_Initialize(&fullyConnectReluLayer->fully_connected_relu_operation.base,
                                      &fullyConnectReluLayer->base,
                                      VXNNE_OPERATION_TARGET_NN,
                                      VXNNE_OPERATOR_FULLYCONNECTED,
                                      vxnneExecuteFullyConnectReluLayer,
                                      VX_NULL);

            if (weights_biases->use_fc_accel)
            {
                vx_uint32 size = (weights_biases->org_weights_sizes[2] + weights_biases->weights_sizes[1]) +
                                  weights_biases->memory_pad / inputs->tensorBuffer->elementSize;
                vx_tensor convertedTensor = vxoTensor_CreateTensor(node->base.context, 1, &size, inputs->tensorBuffer->dataFormat, vx_false_e);

                vxnneOperation_Initialize(&fullyConnectReluLayer->input_convert_weight_operation.base,
                                          &fullyConnectReluLayer->base,
                                          VXNNE_OPERATION_TARGET_SW,
                                          VXNNE_OPERATOR_INPUT2WEIGHT,
                                          vxnneExecuteSWInputConvertWeight,
                                          VX_NULL);

                if (vxoTensor_AllocateMemory(convertedTensor) != VX_SUCCESS)
                {
                    gcmPRINT("vxoTensor_AllocateMemory fail at function %s, line %d", __FUNCTION__, __LINE__);
                    status |= VX_ERROR_NO_MEMORY;
                    return status;
                }

                fullyConnectReluLayer->base.num_operations = 2;

                fullyConnectReluLayer->operations[0] = (vxnne_operation)&fullyConnectReluLayer->input_convert_weight_operation;
                fullyConnectReluLayer->input_convert_weight_operation.inputs                   = inputs;
                fullyConnectReluLayer->input_convert_weight_operation.weights_biases           = weights_biases;
                fullyConnectReluLayer->input_convert_weight_operation.outputs                  = convertedTensor;
                fullyConnectReluLayer->input_convert_weight_operation.enable_relu              = enable_relu;
                fullyConnectReluLayer->input_convert_weight_operation.output_fp_pos            = outputs->tensorBuffer->fixedPointPos;

                fullyConnectReluLayer->operations[1] = (vxnne_operation)&fullyConnectReluLayer->fully_connected_relu_operation;
                fullyConnectReluLayer->fully_connected_relu_operation.inputs                   = convertedTensor;
                fullyConnectReluLayer->fully_connected_relu_operation.weights_biases           = weights_biases;
                fullyConnectReluLayer->fully_connected_relu_operation.outputs                  = outputs;
                convertedTensor->tensorBuffer->fixedPointPos                                   = inputs->tensorBuffer->fixedPointPos;

                fullyConnectReluLayer->base.num_temp_tensors                                   = 1;
                fullyConnectReluLayer->base.temp_tensors[0]                                    = convertedTensor;
            }
            else
            {
                fullyConnectReluLayer->base.num_operations = 1;
                fullyConnectReluLayer->operations[0] = (vxnne_operation)&fullyConnectReluLayer->fully_connected_relu_operation;
                fullyConnectReluLayer->fully_connected_relu_operation.inputs                   = inputs;
                fullyConnectReluLayer->fully_connected_relu_operation.weights_biases           = weights_biases;
                fullyConnectReluLayer->fully_connected_relu_operation.pad                      = pad;
                fullyConnectReluLayer->fully_connected_relu_operation.down_scale_size_rounding = conv_rounding_type;
                fullyConnectReluLayer->fully_connected_relu_operation.outputs                  = outputs;
            }

            node->layer = &fullyConnectReluLayer->base;

            context = vxGetContext((vx_reference)node);
            node->layer->cmdNNBuff = vxCreateArray(context, VX_TYPE_CHAR, NNE_COMMAND_SIZE * weights_biases->zgroup_num);
            if (!vxoArray_AllocateMemory(node->layer->cmdNNBuff))
            {
                status |= VX_ERROR_NO_MEMORY;
                gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
                goto exit;
            }

            for (i = 0; i < weights_biases->zgroup_num; i++)
            {
                /* fill in cmd buffer */
                fillinCmmdBuff(inputs, weights_biases, pad, pad, conv_rounding_type, enable_relu, VIV_NN_NONLINEAR_NON, 0, 0, outputs, node->layer->cmdNNBuff, vx_true_e, i);
            }
        }
    }

exit:
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNFullyConnectedReluLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }
    return VX_SUCCESS;
}

vx_status VX_CALLBACK vxoBaseKernel_NNSoftmaxLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return vxnneLayer_Execute(node->layer);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoSoftmaxLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoSoftmaxLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    if (index != vxmLENGTH_OF(nn_Softmax_params) - 1) return VX_ERROR_INVALID_PARAMETERS;

    ptr->type                 = VX_TYPE_TENSOR;

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoSoftmaxLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status  status                     = VX_SUCCESS;

    vx_tensor  inputs                     = (vx_tensor)parameters[0];
    vx_tensor  outputs                    = (vx_tensor)parameters[1];

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    {
        vxnne_softmax_layer  softmaxLayer;

        gcoOS_Allocate(gcvNULL, sizeof(vxnne_softmax_layer_s), (gctPOINTER*)&softmaxLayer);
        if (!softmaxLayer)
        {
            status = VX_ERROR_NO_MEMORY;
            gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
            goto exit;
        }

        gcoOS_ZeroMemory(softmaxLayer, sizeof(vxnne_softmax_layer_s));

        vxnneLayer_Initialize(&softmaxLayer->base,
                              "SoftmaxLayer",
                              node,
                              softmaxLayer->operations,
                              VX_NULL);

        vxnneOperation_Initialize(&softmaxLayer->softmax_sw_operation.base,
                                  &softmaxLayer->base,
                                  VXNNE_OPERATION_TARGET_SW,
                                  VXNNE_OPERATOR_SOFTMAX,
                                  vxnneExecuteSWSoftmax,
                                  VX_NULL);

        softmaxLayer->base.num_operations    = 1;
        softmaxLayer->operations[0] = (vxnne_operation)&softmaxLayer->softmax_sw_operation;

        softmaxLayer->softmax_sw_operation.inputs           = inputs;
        softmaxLayer->softmax_sw_operation.outputs          = outputs;

        node->layer = &softmaxLayer->base;
    }

exit:
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoSoftmaxLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNConcat2Layer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return vxnneLayer_Execute(node->layer);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConcat2Layer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConcat2Layer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConcat2Layer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status  status                     = VX_SUCCESS;

    vx_tensor  in0                     = (vx_tensor)parameters[0];
    vx_tensor  in1                     = (vx_tensor)parameters[1];
    vx_tensor  out                     = (vx_tensor)parameters[2];

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    {
        vxnne_concat2_layer  concatLayer;

        gcoOS_Allocate(gcvNULL, sizeof(vxnne_concat2_layer_s), (gctPOINTER*)&concatLayer);
        if (!concatLayer)
        {
            status = VX_ERROR_NO_MEMORY;
            gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
            goto exit;
        }

        gcoOS_ZeroMemory(concatLayer, sizeof(vxnne_concat2_layer_s));

        vxnneLayer_Initialize(&concatLayer->base,
                              "ConcatLayer",
                              node,
                              concatLayer->operations,
                              VX_NULL);

        vxnneOperation_Initialize(&concatLayer->concat2_operation.base,
                                  &concatLayer->base,
                                  VXNNE_OPERATION_TARGET_SW,
                                  VXNNE_OPERATOR_CONCAT2,
                                  vxnneExecuteSWConcat2,
                                  VX_NULL);

        concatLayer->base.num_operations    = 1;
        concatLayer->operations[0] = (vxnne_operation)&concatLayer->concat2_operation;

        concatLayer->concat2_operation.inputs0           = in0;
        concatLayer->concat2_operation.inputs1           = in1;
        concatLayer->concat2_operation.outputs           = out;

        node->layer = &concatLayer->base;
    }

exit:
    return status;

}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConcat2Layer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }

    return VX_SUCCESS;
}


VX_PRIVATE_API vx_status VX_CALLBACK vxoNNNormalization(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_SUCCESS;

    status = vxnneLayer_Execute(node->layer);

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNormalization_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}
VX_PRIVATE_API vx_status VX_CALLBACK vxoNormalization_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    if (index != vxmLENGTH_OF(nn_Normalization_params) - 1)
       return VX_ERROR_INVALID_PARAMETERS;

    ptr->type = VX_TYPE_TENSOR;

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNormalization_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;

    vx_tensor  inputs                     = (vx_tensor)parameters[0];
    vx_scalar  type_s                     = (vx_scalar)parameters[1];
    vx_scalar  norm_size_s                = (vx_scalar)parameters[2];
    vx_scalar  alpha_s                    = (vx_scalar)parameters[3];
    vx_scalar  beta_s                     = (vx_scalar)parameters[4];
    vx_tensor  outputs                    = (vx_tensor)parameters[5];

    vx_enum    norm_type                  = type_s->value->e;
    vx_uint32  norm_size                  = norm_size_s->value->u32;
    vx_enum    inputFormat                = inputs->tensorBuffer->dataFormat;
    vx_enum    outputFormat               = outputs->tensorBuffer->dataFormat;
    vx_bool    sammap_flag                = vx_false_e;
    vx_bool    acrossmap_flag             = vx_false_e;
    vx_bool    dataformat_flag            = vx_false_e;
    vxnne_normalization_layer  normalizationLayer;
    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }
    gcoOS_Allocate(gcvNULL, sizeof(vxnne_normalization_layer_s), (gctPOINTER*)&normalizationLayer);
    if (!normalizationLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        goto exit;
    }

    gcoOS_ZeroMemory(normalizationLayer, sizeof(vxnne_normalization_layer_s));

    vxnneLayer_Initialize(&normalizationLayer->base,
                          "NormalizationLayer",
                          node,
                          normalizationLayer->operations,
                          VX_NULL);
    sammap_flag     = (vx_bool)((norm_type == VX_CONVOLUTIONAL_NETWORK_NORM_SAME_MAP) && (norm_size == 3));
    acrossmap_flag  = (vx_bool)((norm_type == VX_CONVOLUTIONAL_NETWORK_NORM_ACROSS_MAPS) && (norm_size == 5));
    dataformat_flag = (vx_bool)((inputFormat == VX_TYPE_FLOAT16 || inputFormat == VX_TYPE_INT8) && (outputFormat == VX_TYPE_FLOAT16 || outputFormat == VX_TYPE_INT8));
    if(dataformat_flag && (sammap_flag || acrossmap_flag)) // shader inplement
    {
        vxnne_shader_executable shaderExecutable;
        shaderExecutable = vxnneGetNormalizationShaderExecutable(node->base.context, VXNNE_KERNEL_NORMALIZATION, &node->kernelAttributes.borderMode, inputs, type_s, norm_size_s, alpha_s, beta_s, outputs);
        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto exit;
        }

        status = vxnneShaderOperation_Initialize(&normalizationLayer->normalization_sh_operation,
                                      &normalizationLayer->base,
                                      VXNNE_OPERATOR_NORMALIZATION,
                                      shaderExecutable);

        if (status != VX_SUCCESS) goto exit;

        normalizationLayer->base.num_operations = 1;
        normalizationLayer->operations[0] = &normalizationLayer->normalization_sh_operation.base;
    }

    else //NNE inplement
    {
        vxnneOperation_Initialize(&normalizationLayer->normalization_sw_operation.base,
                                  &normalizationLayer->base,
                                  VXNNE_OPERATION_TARGET_SW,
                                  VXNNE_OPERATOR_NORMALIZATION,
                                  vxnneExecuteSWNormalization,
                                  vxnneShaderOperation_Deinitialize);

        normalizationLayer->base.num_operations    = 1;
        normalizationLayer->operations[0] = (vxnne_operation)&normalizationLayer->normalization_sw_operation;

        normalizationLayer->normalization_sw_operation.inputs           = inputs;
        normalizationLayer->normalization_sw_operation.type             = type_s->value->e;
        normalizationLayer->normalization_sw_operation.norm_size        = norm_size_s->value->u32;
        normalizationLayer->normalization_sw_operation.alpha            = alpha_s->value->f32;
        normalizationLayer->normalization_sw_operation.beta             = beta_s->value->f32;
        normalizationLayer->normalization_sw_operation.outputs          = outputs;
    }

    node->layer = &normalizationLayer->base;
    return status;

exit:
    if (normalizationLayer != NULL)
        gcoOS_Free(NULL, normalizationLayer);

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNormalization_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNNormalizeImage(vx_node node, const vx_reference *parameters, vx_uint32 num)
{

  /* Need rename this kernel.Uuse this to do softmax */
    vx_tensor inputs_t    = (vx_tensor)parameters[0];        /* fp16*/
    vx_tensor outputs_t   = (vx_tensor)parameters[1];        /* fp16 */

    vx_int16 *inputs          = (vx_int16*)(inputs_t->tensorBuffer->memory.logicals[0]);
    vx_int16 *outputs         = (vx_int16*)(outputs_t->tensorBuffer->memory.logicals[0]);

#if defined(__linux__)
    struct timeval start = gcfVX_PerfStart((vx_reference)node);
#endif
    vxoTensor_GetTensorViewMemory(inputs_t, (gctPOINTER *)&inputs, VX_NULL);
    vxoTensor_GetTensorViewMemory(outputs_t, (gctPOINTER *)&outputs, VX_NULL);
    {
        vx_uint32 width   = inputs_t->tensorBuffer->memory.dims[0][0];
        vx_uint32 height  = inputs_t->tensorBuffer->memory.dims[0][1];
        vx_uint32 channel = inputs_t->tensorBuffer->memory.dims[0][2];
        vx_uint32 batch   = inputs_t->tensorBuffer->memory.dims[0][3];
        vx_uint32 w=0,h=0,c=0,b=0;
        vx_uint32 stridec =  inputs_t->tensorBuffer->memory.strides[0][2]/2;
        if(batch == 0)batch = 1;
        for(b = 0; b < batch; b++)
        {
            vx_float32 sum=0;
            vx_float32 val;
            vx_float32 nmsqr ;
            for(c = 0; c < channel; c++)
            {
                 for(h=0;h<height;h++)
                 {
                    for(w=0;w<width;w++)
                    {
                        val = Fp16toFp32(inputs[(b*channel + c)*stridec + width*h + w]);
                        sum += val*val;
                    }
                 }
            }
            nmsqr = (vx_float32)(1.0/sqrtf(sum));
            for(c = 0; c < channel; c++)
            {
                 for(h=0;h<height;h++)
                 {
                    for(w=0;w<width;w++)
                    {
                        val = Fp16toFp32(inputs[(b*channel + c)*stridec + width*h + w]);
                        val = val * nmsqr;
                        outputs[(b*channel + c)*stridec + width*h + w] = Fp32toFp16(val);
                    }
                 }
            }
        }
    }
#if defined(__linux__)
    if (node->base.context->perfEnable)
        printf("normalization image        CPU  time:%10d us\n", gcfVX_PerfEnd((vx_reference)node, start));
#endif

    return VX_SUCCESS;
}
VX_PRIVATE_API vx_status VX_CALLBACK vxoNormalizeImage_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNormalizeImage_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    if (index != vxmLENGTH_OF(nn_NormalizeImage_params) - 1)
       return VX_ERROR_INVALID_PARAMETERS;

    ptr->type = VX_TYPE_TENSOR;
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNPoolingLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return vxnneLayer_Execute(node->layer);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNPoolingLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNPoolingLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    if (index != vxmLENGTH_OF(nn_PoolingLayer_params) - 1) return VX_ERROR_INVALID_PARAMETERS;

    ptr->type                 = VX_TYPE_TENSOR;

    return VX_SUCCESS;
}

vx_status vxnnePoolingOperation_Deinitialize(vxnne_operation_s *operation)
{
    vxnne_pooling_operation pooling_operation = (vxnne_pooling_operation)operation;

    if (pooling_operation->weights_biases != VX_NULL)
    {
        vxReleaseWeightsBiasesParameter(&pooling_operation->weights_biases);
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNPoolingLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_tensor  inputs                     = (vx_tensor)parameters[0];
    vx_scalar  poolType                   = (vx_scalar)parameters[1];
    vx_scalar  poolSizeX                  = (vx_scalar)parameters[2];
    vx_scalar  poolSizeY                  = (vx_scalar)parameters[3];
    vx_scalar  poolPadX                   = (vx_scalar)parameters[4];
    vx_scalar  poolPadY                   = (vx_scalar)parameters[5];
    vx_scalar  rounding                   = (vx_scalar)parameters[6];
    vx_tensor  outputs                    = (vx_tensor)parameters[7];

    vx_status  status                     = VX_SUCCESS;

    vx_enum poolTypeValue    = poolType->value->e;
    vx_uint32 poolSizeXValue = poolSizeX->value->u32;
    vx_uint32 poolSizeYValue = poolSizeY->value->u32;
    vx_uint32 poolPadXValue  = poolPadX->value->u32;
    vx_uint32 poolPadYValue  = poolPadY->value->u32;
    vx_enum roundingValue    = rounding->value->e;
    vx_enum inputdata_format  = inputs->tensorBuffer->dataFormat;
    vx_enum outputdata_format = outputs->tensorBuffer->dataFormat;
    vx_bool kernel_AvgPool_flag[3] = {vx_false_e};
    vx_bool dataFormat_AvgPool_flag[3] = {vx_false_e};
    vx_bool avgPool_flag = vx_false_e;

    vx_int32 inputsWidth, inputsDepth, outputsWidth, outputsDepth;

    vxnne_pooling_layer  poolingLayer;
    vx_uint32  stride = 0;
    vx_uint32 totalSize = 0;
    vx_uint32 maxAllocateSize = 256 * 1024 * 1024; /* set max allocate size because fpga out of memory when using nn do avg pooling, max value is 256M */

    inputsWidth   = inputs->tensorBuffer->memory.dims[0][0];
    inputsDepth   = inputs->tensorBuffer->memory.dims[0][2];
    outputsWidth  = outputs->tensorBuffer->memory.dims[0][0];
    outputsDepth  = outputs->tensorBuffer->memory.dims[0][2];

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_pooling_layer_s), (gctPOINTER*)&poolingLayer);
    if (!poolingLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        goto exit;
    }

    gcoOS_ZeroMemory(poolingLayer, sizeof(vxnne_pooling_layer_s));

    poolingLayer->base.name                  = "PoolingLayer";
    poolingLayer->base.node                  = node;
    poolingLayer->base.operations            = poolingLayer->operations;

    poolingLayer->base.num_temp_tensors      = 0;

    poolingLayer->base.dump                  = VX_NULL;
    poolingLayer->base.deinitialize          = vxnneLayer_Deinitialize;

    poolingLayer->base.num_operations    = 1;

    if (outputsWidth == 1)
    {
        stride = 1;
    }
    else
    {
        stride = vxoNNExternsionConvlutionRound((vx_float32)(inputsWidth + 2 * poolPadXValue - poolSizeXValue) / (outputsWidth - 1), roundingValue);
    }

    totalSize = poolSizeXValue * poolSizeYValue * inputsDepth * outputsDepth * (vx_uint32)vxDataType_GetSize((vx_type_e)inputs->tensorBuffer->dataFormat) + outputsDepth * sizeof(vx_float32);

    kernel_AvgPool_flag[0]     = (vx_bool)(stride == 1 && poolSizeXValue == 13 && poolPadXValue == 0);
    kernel_AvgPool_flag[1]     = (vx_bool)(stride == 1 && poolSizeXValue == 7 && poolPadXValue == 0);
    kernel_AvgPool_flag[2]     = (vx_bool)(stride == 1 && poolSizeXValue == 6 && poolPadXValue == 0);
    dataFormat_AvgPool_flag[0] = (vx_bool)(inputdata_format == VX_TYPE_INT8 && outputdata_format == VX_TYPE_FLOAT16);
    dataFormat_AvgPool_flag[1] = (vx_bool)(inputdata_format == VX_TYPE_FLOAT16 && outputdata_format == VX_TYPE_INT8);
    dataFormat_AvgPool_flag[2] = (vx_bool)(inputdata_format == VX_TYPE_INT8 && outputdata_format == VX_TYPE_INT8);
    avgPool_flag   = (vx_bool)(((kernel_AvgPool_flag[0] && dataFormat_AvgPool_flag[0])
        || (kernel_AvgPool_flag[1] && dataFormat_AvgPool_flag[1])
        || (kernel_AvgPool_flag[1] && dataFormat_AvgPool_flag[2])
        || (kernel_AvgPool_flag[2] && dataFormat_AvgPool_flag[0]))
        && (poolTypeValue == VX_CONVOLUTIONAL_NETWORK_POOLING_AVG));

    /* if the needed total size is larger than maxAllocateSize, do pooling with CPU version. maybe need implement avg pooling with shader */
    if ((poolTypeValue == VX_CONVOLUTIONAL_NETWORK_POOLING_AVG) && (stride == 1) && totalSize <= maxAllocateSize && avgPool_flag == vx_false_e)
    {
        /* nne only support NxN average pooling and stride = 1 */
        poolingLayer->pooling_nne_operation.base.layer         = &poolingLayer->base;
        poolingLayer->pooling_nne_operation.base.target        = VXNNE_OPERATION_TARGET_NN;
        poolingLayer->pooling_nne_operation.base.operatorType = VXNNE_OPERATOR_POOLING;
        poolingLayer->pooling_nne_operation.base.execute       = vxnneExecutePooling;
        poolingLayer->pooling_nne_operation.base.deinitialize  = vxnnePoolingOperation_Deinitialize;
        poolingLayer->pooling_nne_operation.base.dump          = VX_NULL;

        poolingLayer->operations[0] = (vxnne_operation)&poolingLayer->pooling_nne_operation;

        poolingLayer->pooling_nne_operation.inputs           = inputs;
        poolingLayer->pooling_nne_operation.pool_type        = poolTypeValue;
        poolingLayer->pooling_nne_operation.pool_size_x      = poolSizeXValue;
        poolingLayer->pooling_nne_operation.pool_size_y      = poolSizeYValue;
        poolingLayer->pooling_nne_operation.pool_pad_x       = poolPadXValue;
        poolingLayer->pooling_nne_operation.pool_pad_y       = poolPadYValue;
        poolingLayer->pooling_nne_operation.rounding         = roundingValue;
        poolingLayer->pooling_nne_operation.outputs          = outputs;

        /* prepare data for nne */
        {
            vx_int8 *weightData = VX_NULL;
            vx_float32 *biasData = VX_NULL;
            vx_uint32 i, j;
            vx_int32 w, z;
            vx_int8 *weightsValuePtr;
            vx_int8 *zerosValuePtr;
            vx_uint32 weightItemCount;
            vx_int8 *pWeightData;
            vx_float32 *pBiasData;
            vx_tensor weights = VX_NULL;
            vx_tensor biases = VX_NULL;
            vx_context context = VX_NULL;
            vx_uint32 numWeightDims = 4, numBiasDims = 1;
            vx_uint32 weightSize[4] = {poolSizeXValue, poolSizeYValue, inputsDepth, outputsDepth};
            vx_uint32 weightStrideSize[4];
            vx_uint32 biasSize[4] = {outputsDepth};
            vx_uint32 biasStrideSize[1];
            vx_tensor_addressing weightUserAddr = NULL;
            vx_tensor_addressing biasUserAddr = NULL;
            vx_weights_biases_parameter weights_biases = VX_NULL;
            vx_type_e inputDataFormat = (vx_type_e)inputs->tensorBuffer->dataFormat;
            vx_type_e weightDataFormat = inputDataFormat;
            vx_type_e biasDataFormat = (weightDataFormat == VX_TYPE_INT8) ? VX_TYPE_INT32 : VX_TYPE_FLOAT32;
            vx_int32 weightItemSize = vxnneGetTypeSize(weightDataFormat);
            vx_int32 biasItemSize = vxnneGetTypeSize(biasDataFormat);
            vx_int8 weightFixPointPos, biasFixPointPos;

            context = vxGetContext((vx_reference)node);
            if (context == VX_NULL)
            {
                gcmPRINT("vxGetContext fail at function %s line %d", __FUNCTION__, __LINE__);
                goto exit;
            }

            /* generate special weight and bias data for average pooling. create weightsBiasesParameters from this specail weight and bias */
#if VX_C_MEMORY_MANAGE

            vxoMemory_CAllocate(node->base.context, (void**)&weightsValuePtr, poolSizeXValue * poolSizeYValue * weightItemSize);
            vxoMemory_CAllocate(node->base.context, (void**)&zerosValuePtr, poolSizeXValue * poolSizeYValue * weightItemSize);

            vxoMemory_CAllocate(node->base.context, (void**)&weightData, poolSizeXValue * poolSizeYValue * inputsDepth * outputsDepth * weightItemSize);
            vxoMemory_CAllocate(node->base.context, (void**)&biasData, outputsDepth * biasItemSize);

#else
            weightsValuePtr = (vx_int8*)malloc(poolSizeXValue * poolSizeYValue * weightItemSize);
            zerosValuePtr = (vx_int8*)malloc(poolSizeXValue * poolSizeYValue * weightItemSize);

            weightData = (vx_float32*)malloc(pool_size_x_v * pool_size_y_v * inputs_depth * outputs_depth * weightItemSize);
            biasData   = (vx_float32*)malloc(outputs_depth * biasItemSize);
#endif

            weightItemCount = poolSizeXValue * poolSizeYValue;

            weightFixPointPos = (vx_int8)(8 - gcoMATH_Ceiling(gcoMATH_Log(1.0f/weightItemCount) + 1));

            biasFixPointPos = weightFixPointPos + (vx_int8)inputs->tensorBuffer->fixedPointPos;

            for (j = 0; j < weightItemCount; j++)
            {
                vxnneSaveDate(weightDataFormat, j, 1.0f/weightItemCount, weightsValuePtr, weightFixPointPos);
                vxnneSaveDate(weightDataFormat, j, 0.0f, zerosValuePtr, weightFixPointPos);
            }

            pWeightData = weightData;
            pBiasData = biasData;

            for (w = 0; w < outputsDepth; w++)
            {
                for (z = 0; z < inputsDepth; z++)
                {
                    if (w == z)
                    {
                        memcpy(pWeightData, weightsValuePtr, weightItemCount * weightItemSize);
                    }
                    else
                    {
                        memcpy(pWeightData, zerosValuePtr, weightItemCount * weightItemSize);
                    }
                    pWeightData += weightItemCount * weightItemSize;
                }
                *pBiasData++ = 0.0f;
            }

            weightStrideSize[0] = weightItemSize;
            for (i = 1; i < numWeightDims; i++)
            {
                weightStrideSize[i] =  weightStrideSize[i-1] * weightSize[i-1];
            }
            weights = vxCreateTensor(context, numWeightDims, weightSize, weightDataFormat, weightFixPointPos);
            weightUserAddr = vxCreateTensorAddressing(context, weightSize, weightStrideSize, (vx_uint8)numWeightDims);

            vxCopyTensorPatch(weights, VX_NULL, weightUserAddr, weightData, VX_WRITE_ONLY,0);

            biasStrideSize[0] = biasItemSize;
            biases = vxCreateTensor(context, numBiasDims, biasSize, biasDataFormat, biasFixPointPos);
            biasUserAddr = vxCreateTensorAddressing(context, biasSize, biasStrideSize, (vx_uint8)numBiasDims);

            vxCopyTensorPatch(biases, VX_NULL, biasUserAddr, biasData, VX_WRITE_ONLY,0);

            weights_biases = vxCreateWeightsBiasesParameterFromTensors(VX_CONVOLUTIONAL_NETWORK_CONVOLUTION_LAYER,
                TENSOR_DIM_NUM(inputs),
                (vx_uint32*)(TENSOR_SIZES(inputs)),
                poolPadXValue,
                poolPadYValue,
                poolSizeXValue,
                poolSizeYValue,
                roundingValue,
                (vx_uint32*)(TENSOR_SIZES(outputs)),
                VX_NULL,
                VX_NULL,
                weights,
                biases);

#if VX_C_MEMORY_MANAGE

            if (weightsValuePtr != VX_NULL)
                vxoMemory_CFree(node->base.context, (void**)&weightsValuePtr);

            if (zerosValuePtr != VX_NULL)
                vxoMemory_CFree(node->base.context, (void**)&zerosValuePtr);

            if (weightData != VX_NULL)
                vxoMemory_CFree(node->base.context, (void**)&weightData);

            if (biasData != VX_NULL)
                vxoMemory_CFree(node->base.context, (void**)&biasData);
#else
            if (weightsValuePtr != VX_NULL)
                free(&weightsValuePtr);
            if (zerosValuePtr != VX_NULL)
                free(zerosValuePtr);
            if (weightData != VX_NULL)
                free(weightData);
            if (biasData != VX_NULL)
                free(biasData);
#endif

            /* create cmd buffer for nn  layer */
            poolingLayer->base.cmdNNBuff = vxCreateArray(context, VX_TYPE_CHAR, NNE_COMMAND_SIZE);
            if (!vxoArray_AllocateMemory(poolingLayer->base.cmdNNBuff))
            {
                status = VX_ERROR_NO_MEMORY;
                gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
                goto exit;
            }

            /* fill in cmd buffer */
            fillinCmmdBuff(inputs, weights_biases, poolPadXValue, poolPadYValue, roundingValue, vx_false_e, VIV_NN_POOLING_NON, 0, 0, outputs, poolingLayer->base.cmdNNBuff, vx_false_e, 0);

            poolingLayer->pooling_nne_operation.weights_biases = weights_biases;

            if (weights != VX_NULL)
            {
                vxReleaseTensor(&weights);
            }

            if (biases != VX_NULL)
            {
                vxReleaseTensor(&biases);
            }

            if (weightUserAddr != VX_NULL)
            {
                vxReleaseTensorAddressing(&weightUserAddr);
            }

            if (biasUserAddr != VX_NULL)
            {
                vxReleaseTensorAddressing(&biasUserAddr);
            }
        }
    }
    else
    {
#if NN_TP_ENGINE
        if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_TP_ENGINE))
        {
            vx_context context;

            vxnneOperation_Initialize(
                &poolingLayer->pooling_tp_operation.base,
                &poolingLayer->base,
                VXNNE_OPERATION_TARGET_TP,
                VXNNE_OPERATOR_POOLING,
                vxnneExecuteTPPooling,
                VX_NULL);

            poolingLayer->base.num_operations = 1;
            poolingLayer->operations[0] = (vxnne_operation)&poolingLayer->pooling_tp_operation;

            poolingLayer->pooling_tp_operation.inputs           = inputs;
            poolingLayer->pooling_tp_operation.outputs          = outputs;

            /* create cmd buffer for TP operation */
            context = vxGetContext((vx_reference)node);
            poolingLayer->base.cmdTPBuff = vxCreateArray(context, VX_TYPE_CHAR, TP_COMMAND_SIZE);
            if (!vxoArray_AllocateMemory(poolingLayer->base.cmdTPBuff))
            {
                status = VX_ERROR_NO_MEMORY;
                gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
                goto exit;
            }

            fillInCmmdTPBuffer(inputs, VX_NULL, outputs, poolingLayer->base.cmdTPBuff, poolPadXValue, poolPadYValue, poolSizeXValue, poolSizeYValue, vx_false_e, TP_MAX_POOLING);
        }
        else
#endif
        {
            vx_bool kernel_MaxPool_flag     = vx_false_e;
            vx_bool dataformat_MaxPool_flag = vx_false_e;
            vx_bool maxPool_flag            = vx_false_e;
            kernel_MaxPool_flag     = (vx_bool)((stride == 2 && poolSizeXValue == 3 && poolPadXValue == 1) || (stride == 2 && poolSizeXValue == 2 && poolPadXValue == 0)|| (stride == 2 && poolSizeXValue == 3 && poolPadXValue == 0));
            dataformat_MaxPool_flag = (vx_bool)((inputdata_format == VX_TYPE_FLOAT16 || inputdata_format == VX_TYPE_INT8)
                && (outputdata_format == VX_TYPE_FLOAT16 || outputdata_format == VX_TYPE_INT8));
            maxPool_flag            = (vx_bool)(kernel_MaxPool_flag && dataformat_MaxPool_flag && (poolTypeValue == VX_CONVOLUTIONAL_NETWORK_POOLING_MAX));

            if (avgPool_flag || maxPool_flag)
            {
                vxnne_shader_executable shaderExecutable = NULL;
                vx_scalar stride_s = NULL;
                stride_s = vxCreateScalar(node->base.context, VX_TYPE_UINT32, &stride);
                if (!stride_s)
                {
                    status = VX_FAILURE;
                    goto exit;
                }
                if(maxPool_flag)
                    shaderExecutable = vxnneGetMaxPoolingShaderExecutable(node->base.context, VXNNE_KERNEL_MAXPOOLING, &node->kernelAttributes.borderMode,
                    inputs, poolType, stride_s, poolSizeX, poolSizeY, poolPadX, poolPadY, rounding, outputs);
                else if(avgPool_flag)
                    shaderExecutable = vxnneGetAvgPoolingShaderExecutable(node->base.context, VXNNE_KERNEL_AVGPOOLING, &node->kernelAttributes.borderMode,
                    inputs, poolType, stride_s, poolSizeX, poolSizeY, poolPadX, poolPadY, rounding, outputs);

                if (!shaderExecutable)
                {
                    status = VX_FAILURE;
                    vxReleaseScalar(&stride_s);
                    goto exit;
                }

                status = vxnneShaderOperation_Initialize(&poolingLayer->pooling_sh_operation,
                    &poolingLayer->base,
                    VXNNE_OPERATOR_POOLING,
                    shaderExecutable);

                if (status != VX_SUCCESS)
                {
                    vxReleaseScalar(&stride_s);
                    goto exit;
                }

                poolingLayer->base.num_operations = 1;
                poolingLayer->operations[0] = &poolingLayer->pooling_sh_operation.base;
                if (stride_s) (vxReleaseScalar(&stride_s));
            }
            else
            {
                poolingLayer->pooling_sw_operation.base.layer         = &poolingLayer->base;
                poolingLayer->pooling_sw_operation.base.target        = VXNNE_OPERATION_TARGET_SW;
                poolingLayer->pooling_sw_operation.base.operatorType = VXNNE_OPERATOR_POOLING;
                poolingLayer->pooling_sw_operation.base.execute       = vxnneExecuteSWPooling;
                poolingLayer->pooling_sw_operation.base.deinitialize  = vxnneShaderOperation_Deinitialize;
                poolingLayer->pooling_sw_operation.base.dump          = VX_NULL;

                poolingLayer->operations[0] = (vxnne_operation)&poolingLayer->pooling_sw_operation;

                poolingLayer->pooling_sw_operation.inputs           = inputs;
                poolingLayer->pooling_sw_operation.pool_type        = poolType->value->e;
                poolingLayer->pooling_sw_operation.pool_size_x      = poolSizeX->value->u32;
                poolingLayer->pooling_sw_operation.pool_size_y      = poolSizeY->value->u32;
                poolingLayer->pooling_sw_operation.pool_pad_x       = poolPadX->value->u32;
                poolingLayer->pooling_sw_operation.pool_pad_y       = poolPadY->value->u32;
                poolingLayer->pooling_sw_operation.rounding         = rounding->value->e;
                poolingLayer->pooling_sw_operation.outputs          = outputs;
            }
        }
    }

    node->layer = &poolingLayer->base;

    return status;

exit:
    if (poolingLayer != NULL) gcoOS_Free(NULL, (gctPOINTER)poolingLayer);

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNPoolingLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNFullyConnectedLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return vxnneLayer_Execute(node->layer);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNFullyConnectedLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNFullyConnectedLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNFullyConnectedLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
#define IMG_MAX_WIDTH 65536

    vx_status  status                     = VX_SUCCESS;

    vx_tensor  inputs                     = (vx_tensor)parameters[0];
    vx_tensor  weights                    = (vx_tensor)parameters[1];
    vx_tensor  biases                     = (vx_tensor)parameters[2];
    vx_tensor  outputs                    = (vx_tensor)parameters[8];

    vx_bool   input_flag = vx_false_e;
    vx_bool   weight_flag = vx_false_e;
    vx_bool   bias_flag = vx_false_e;
    vx_bool   output_flag = vx_false_e;
    vx_enum   input_dataformat = inputs->tensorBuffer->dataFormat;
    vx_enum   weight_dataformat = weights->tensorBuffer->dataFormat;
    vx_enum   bias_dataformat = biases->tensorBuffer->dataFormat;
    vx_enum   output_dataformat = outputs->tensorBuffer->dataFormat;
    vx_uint32 num_of_dim = 0;
    vx_uint32 input_size[4] = {0, 0, 0, 0};
    vxnne_fully_connected_relu_layer  fullyConnectedLayer = gcvNULL;
    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    status = vxQueryTensor(inputs, VX_TENSOR_DIMS, input_size, sizeof(input_size));
    if (status != VX_SUCCESS) goto exit;
    input_flag = (vx_bool)((input_dataformat == VX_TYPE_FLOAT16 || (input_dataformat == VX_TYPE_INT8)) && ((input_size[0] * input_size[1] * input_size[2]) < IMG_MAX_WIDTH));

    status = vxQueryTensor(weights, VX_TENSOR_NUM_OF_DIMS, &num_of_dim, sizeof(num_of_dim));
    if (status != VX_SUCCESS) goto exit;
    status = vxQueryTensor(weights, VX_TENSOR_DIMS, input_size, sizeof(input_size));
    if (status != VX_SUCCESS) goto exit;
    weight_flag = (vx_bool)((num_of_dim == 4) && (weight_dataformat == VX_TYPE_FLOAT16 || (weight_dataformat == VX_TYPE_INT8)) && ((input_size[0] * input_size[1] * input_size[2]) < IMG_MAX_WIDTH) && (input_size[3] < IMG_MAX_WIDTH));

    bias_flag = (vx_bool)(bias_dataformat == VX_TYPE_FLOAT32 || (bias_dataformat == VX_TYPE_INT32));

    output_flag = (vx_bool)(output_dataformat == VX_TYPE_FLOAT16 || (output_dataformat == VX_TYPE_INT8));

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_fully_connected_relu_layer_s), (gctPOINTER*)&fullyConnectedLayer);
    if (!fullyConnectedLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        goto exit;
    }

    gcoOS_ZeroMemory(fullyConnectedLayer, sizeof(vxnne_fully_connected_relu_layer_s));

    fullyConnectedLayer->base.name                  = "FullyConnectedLayer";
    fullyConnectedLayer->base.node                  = node;
    fullyConnectedLayer->base.operations            = fullyConnectedLayer->operations;

    fullyConnectedLayer->base.num_temp_tensors      = 0;

    fullyConnectedLayer->base.dump                  = VX_NULL;
    fullyConnectedLayer->base.deinitialize          = vxnneLayer_Deinitialize;

    if(input_flag && weight_flag && bias_flag && output_flag)
    {
        vxnne_shader_executable shaderExecutable;

        shaderExecutable = vxnneGetFullyConnectedShaderExecutable(node->base.context, VXNNE_KERNEL_FULLYCONNECTED, &node->kernelAttributes.borderMode, inputs, weights, biases, outputs);

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto exit;
        }

        status = vxnneShaderOperation_Initialize(&fullyConnectedLayer->fully_connected_SHoperation,
            &fullyConnectedLayer->base,
            VXNNE_OPERATOR_FULLYCONNECTED,
            shaderExecutable);

        if (status != VX_SUCCESS) goto exit;

        fullyConnectedLayer->base.num_operations = 1;
        fullyConnectedLayer->operations[0] = &fullyConnectedLayer->fully_connected_SHoperation.base;
    }
    else
    {
        fullyConnectedLayer->fully_connected_operation.base.layer         = &fullyConnectedLayer->base;
        fullyConnectedLayer->fully_connected_operation.base.target        = VXNNE_OPERATION_TARGET_SW;
        fullyConnectedLayer->fully_connected_operation.base.operatorType = VXNNE_OPERATOR_FULLYCONNECTED;
        fullyConnectedLayer->fully_connected_operation.base.execute       = vxnneExecuteSWFullyConnected;
        fullyConnectedLayer->fully_connected_operation.base.deinitialize  = vxnneShaderOperation_Deinitialize;
        fullyConnectedLayer->fully_connected_operation.base.dump          = VX_NULL;

        fullyConnectedLayer->base.num_operations    = 1;
        fullyConnectedLayer->operations[0] = (vxnne_operation)&fullyConnectedLayer->fully_connected_operation;

        fullyConnectedLayer->fully_connected_operation.inputs           = inputs;
        fullyConnectedLayer->fully_connected_operation.weights          = weights;
        fullyConnectedLayer->fully_connected_operation.biases           = biases;
        fullyConnectedLayer->fully_connected_operation.outputs          = outputs;
    }

    node->layer = &fullyConnectedLayer->base;
    return status;

exit:
    if (fullyConnectedLayer) gcoOS_Free(gcvNULL, fullyConnectedLayer);
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNFullyConnectedLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNActivationLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return vxnneLayer_Execute(node->layer);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNActivationLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNActivationLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNActivationLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status  status                     = VX_SUCCESS;

    vx_tensor  inputs                     = (vx_tensor)parameters[0];
    vx_scalar  func_s                     = (vx_scalar)parameters[1];
    vx_scalar  a_s                        = (vx_scalar)parameters[2];
    vx_scalar  b_s                        = (vx_scalar)parameters[3];
    vx_tensor  outputs                    = (vx_tensor)parameters[4];

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    {
        vxnne_activation_layer  activationLayer;

        gcoOS_Allocate(gcvNULL, sizeof(vxnne_activation_layer_s), (gctPOINTER*)&activationLayer);
        if (!activationLayer)
        {
            status = VX_ERROR_NO_MEMORY;
            gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
            goto exit;
        }

        gcoOS_ZeroMemory(activationLayer, sizeof(vxnne_activation_layer_s));

        activationLayer->base.name                  = "ActivationLayer";
        activationLayer->base.node                  = node;
        activationLayer->base.operations            = activationLayer->operations;

        activationLayer->base.num_temp_tensors      = 0;

        activationLayer->base.dump                  = VX_NULL;
        activationLayer->base.deinitialize          = vxnneLayer_Deinitialize;


        activationLayer->activation_operation.base.layer         = &activationLayer->base;
        activationLayer->activation_operation.base.target        = VXNNE_OPERATION_TARGET_SW;
        activationLayer->activation_operation.base.operatorType = VXNNE_OPERATOR_ACTIVATION;
        activationLayer->activation_operation.base.execute       = vxnneExecuteSWActivation;
        activationLayer->activation_operation.base.deinitialize  = vxnneShaderOperation_Deinitialize;
        activationLayer->activation_operation.base.dump          = VX_NULL;

        activationLayer->base.num_operations    = 1;
        activationLayer->operations[0] = (vxnne_operation)&activationLayer->activation_operation;

        activationLayer->activation_operation.inputs           = inputs;
        activationLayer->activation_operation.func             = func_s;
        activationLayer->activation_operation.a                = a_s;
        activationLayer->activation_operation.b                = b_s;
        activationLayer->activation_operation.outputs          = outputs;

        node->layer = &activationLayer->base;
    }

exit:

    return status;

}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNActivationLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }

    return VX_SUCCESS;
}

/***************************************************************************************************************************
 *                                                       Leaky Relu
 ***************************************************************************************************************************/

vx_status vxnneExecuteSWLeakyRelu(struct _vxnne_operation_s *operation)
{
    vxnne_activation_sw_operation activationOperation   = (vxnne_activation_sw_operation)operation;

    vx_tensor inputs  = (vx_tensor)activationOperation->inputs;
    vx_scalar negative_slopes  = (vx_scalar)activationOperation->a;
    vx_tensor outputs = (vx_tensor)activationOperation->outputs;

    vx_float32  negative_slope_v = negative_slopes->value->f32;

    vx_uint32 elementCount = 0;
    vx_uint32 i;
    vx_float32 result = 0.0f;
    gctPOINTER inputBase;
    gctPOINTER outputBase;
    vx_type_e inputFormat  = (vx_type_e)inputs->tensorBuffer->dataFormat;
    vx_type_e outputFormat = (vx_type_e)outputs->tensorBuffer->dataFormat;

    vx_status status = VX_SUCCESS;

    elementCount = (vx_uint32)vxoMemory_ComputeElementCount(&inputs->tensorBuffer->memory, 0);
    vxoTensor_GetTensorViewMemory(inputs, &inputBase, VX_NULL);
    vxoTensor_GetTensorViewMemory(outputs, &outputBase, VX_NULL);

    if ((inputs->tensorBuffer->dataFormat != VX_TYPE_FLOAT16 && inputs->tensorBuffer->dataFormat != VX_TYPE_FLOAT32 && inputs->tensorBuffer->dataFormat != VX_TYPE_INT8)
        || (outputs->tensorBuffer->dataFormat != VX_TYPE_FLOAT16 && outputs->tensorBuffer->dataFormat != VX_TYPE_FLOAT32 && outputs->tensorBuffer->dataFormat != VX_TYPE_INT8))
    {
        gcmPRINT("input or outputs format is not support");
        status = VX_ERROR_NOT_SUPPORTED;
        return status;
    }

    for (i = 0; i < elementCount; i++)
    {
        vx_float32 data = vxnneGetDate(inputFormat, i, (vx_uint8_ptr)inputBase, inputs->tensorBuffer->fixedPointPos);

        result = (data > 0.0f) ? data : negative_slope_v * data;

        vxnneSaveDate(outputFormat, i, result, (vx_uint8_ptr)outputBase, outputs->tensorBuffer->fixedPointPos);
    }
    return status;

}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNLeakyReluLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return vxnneLayer_Execute(node->layer);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNLeakyReluLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNLeakyReluLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNLeakyReluLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status                     = VX_SUCCESS;

    vx_tensor inputs                     = (vx_tensor)parameters[0];
    vx_scalar negative_slopes            = (vx_scalar)parameters[1];
    vx_tensor outputs                    = (vx_tensor)parameters[2];
    vx_enum   inputFormat                = inputs->tensorBuffer->dataFormat;
    vx_enum   outputFormat               = outputs->tensorBuffer->dataFormat;

    vxnne_activation_layer  activationLayer = VX_NULL;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_activation_layer_s), (gctPOINTER*)&activationLayer);
    if (!activationLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        return status;
    }

    gcoOS_ZeroMemory(activationLayer, sizeof(vxnne_activation_layer_s));

    activationLayer->base.name                  = "LeakyReluLayer";
    activationLayer->base.node                  = node;
    activationLayer->base.operations            = activationLayer->operations;

    activationLayer->base.num_temp_tensors      = 0;

    activationLayer->base.dump                  = VX_NULL;
    activationLayer->base.deinitialize          = vxnneLayer_Deinitialize;

    if((inputFormat == VX_TYPE_FLOAT16 || inputFormat == VX_TYPE_INT8) && (outputFormat == VX_TYPE_FLOAT16 || outputFormat == VX_TYPE_INT8))
    {
        vxnne_shader_executable shaderExecutable;

        shaderExecutable = vxnneGetLeakyReluShaderExecutable(node->base.context, VXNNE_KERNEL_NN_LEAKY, &node->kernelAttributes.borderMode, inputs, negative_slopes, outputs);

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto exit;
        }

        status = vxnneShaderOperation_Initialize(&activationLayer->activation_SHoperation,
                                        &activationLayer->base,
                                        VXNNE_OPERATOR_ACTIVATION,
                                        shaderExecutable);

        if (status != VX_SUCCESS) goto exit;

        activationLayer->base.num_operations = 1;
        activationLayer->operations[0] = &activationLayer->activation_SHoperation.base;

    }
    else
    {
        activationLayer->activation_operation.base.layer         = &activationLayer->base;
        activationLayer->activation_operation.base.target        = VXNNE_OPERATION_TARGET_SW;
        activationLayer->activation_operation.base.operatorType  = VXNNE_OPERATOR_ACTIVATION;
        activationLayer->activation_operation.base.execute       = vxnneExecuteSWLeakyRelu;
        activationLayer->activation_operation.base.deinitialize  = VX_NULL;
        activationLayer->activation_operation.base.dump          = VX_NULL;

        activationLayer->base.num_operations = 1;
        activationLayer->operations[0] = (vxnne_operation)&activationLayer->activation_operation;

        activationLayer->activation_operation.inputs           = inputs;
        activationLayer->activation_operation.func             = VX_NULL;
        activationLayer->activation_operation.a                = negative_slopes;
        activationLayer->activation_operation.b                = VX_NULL;
        activationLayer->activation_operation.outputs          = outputs;
    }

    node->layer = &activationLayer->base;
    return status;

exit:
    if (activationLayer) gcoOS_Free(gcvNULL, (gctPOINTER)activationLayer);
    return status;

}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNLeakyReluLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }

    return VX_SUCCESS;
}

/***************************************************************************************************************************
 *                                                       Batch Normalization
 ***************************************************************************************************************************/

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNBatchNormalizationLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return vxnneLayer_Execute(node->layer);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNBatchNormalizationLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNBatchNormalizationLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

vx_status vxnneExecuteSWBatchNorm(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;

    vxnne_batchnorm_sw_operation activationOperation = (vxnne_batchnorm_sw_operation)operation;

    vx_scalar globals  = (vx_scalar)activationOperation->global;
    vx_scalar epss     = (vx_scalar)activationOperation->eps;
    vx_tensor means    = (vx_tensor)activationOperation->mean;
    vx_tensor variances= (vx_tensor)activationOperation->variance;
    vx_tensor outputs  = (vx_tensor)activationOperation->outputs;
    /*
    vx_tensor gamma    = (vx_tensor)activationOperation->gamma;
    vx_tensor beta     = (vx_tensor)activationOperation->beta;
    */

    vx_bool global = globals->value->b;
    vx_float32 eps = epss->value->f32;

    vx_uint8_ptr output;
    vx_uint8_ptr mean;
    vx_uint8_ptr variance;
    vx_int32 batch = outputs->viewRegion.viewEnds[0] - outputs->viewRegion.viewStarts[0];
    vx_int32 filters = outputs->viewRegion.viewEnds[1] - outputs->viewRegion.viewStarts[1];
    vx_int32 width = outputs->viewRegion.viewEnds[2] - outputs->viewRegion.viewStarts[2];
    vx_int32 height = outputs->viewRegion.viewEnds[3] - outputs->viewRegion.viewStarts[3];
    vx_int32 spatial = width * height;
    vx_type_e format = (vx_type_e)(outputs->tensorBuffer->dataFormat);

    vx_int32 b = 0, f = 0, i = 0;

    vxoTensor_GetTensorViewMemory(outputs, (gctPOINTER*)&output, VX_NULL);
    vxoTensor_GetTensorViewMemory(means, (gctPOINTER*)&mean, VX_NULL);
    vxoTensor_GetTensorViewMemory(variances, (gctPOINTER*)&variance, VX_NULL);

    if (global)
    {
        /****************************************************************************
         *
         *              batch0          batch1              batch n
         *          _____________    _____________       _____________
         * filter0 |*   w*h      |  |*    w*h     |     |*    w*h     |
         * filter1 |             |  |             |     |             |
         *   ...   |             |  |             | ... |             |
         *   ...   |             |  |             |     |             |
         * filterm |             |  |             |     |             |
         *          -------------    -------------       -------------
         *
         ****************************************************************************/

        if((means != VX_NULL) && (variances != VX_NULL))
        {
            for(b = 0; b < batch; ++b){
                for(f = 0; f < filters; ++f){
                    for(i = 0; i < spatial; ++i){
                        vx_int32 index = b * filters * spatial + f * spatial + i;
                        vx_float32 meanf = vxnneGetDate(format, f, mean, means->tensorBuffer->fixedPointPos);
                        vx_float32 variancef = vxnneGetDate(format, f, variance, variances->tensorBuffer->fixedPointPos);
                        vx_float32 outputf = vxnneGetDate(format, index, output, outputs->tensorBuffer->fixedPointPos);
                        outputf = (outputf - meanf)/(sqrtf(variancef) + eps);

                        vxnneSaveDate(format, index, outputf, output, outputs->tensorBuffer->fixedPointPos);
                    }
                }
            }
        }
    }
    else
    {
        /*
        vx_scalar moving_average_fractions = (vx_scalar)activationOperation->moving_average_fraction;
        vx_float32 moving_average_fraction = moving_average_fractions->value->f32;
        */
        printf("Not support\n");
        status = VX_ERROR_NOT_SUPPORTED;
    }

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNBatchNormalizationLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status  status                     = VX_SUCCESS;

    vx_scalar  globals                    = (vx_scalar)parameters[0];
    vx_scalar  mafs                       = (vx_scalar)parameters[1];
    vx_scalar  epss                       = (vx_scalar)parameters[2];
    vx_tensor  means                      = (vx_tensor)parameters[3];
    vx_tensor  variances                  = (vx_tensor)parameters[4];
    vx_tensor  gamma                      = (vx_tensor)parameters[5];
    vx_tensor  beta                       = (vx_tensor)parameters[6];
    vx_tensor  outputs                    = (vx_tensor)parameters[7];

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    {
        vxnne_batchnorm_layer  batchnormLayer;

        gcoOS_Allocate(gcvNULL, sizeof(vxnne_batchnorm_layer_s), (gctPOINTER*)&batchnormLayer);
        if (!batchnormLayer)
        {
            status = VX_ERROR_NO_MEMORY;
            gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
            goto exit;
        }

        gcoOS_ZeroMemory(batchnormLayer, sizeof(vxnne_batchnorm_layer_s));

        batchnormLayer->base.name                  = "BatchNormalizationLayer";
        batchnormLayer->base.node                  = node;
        batchnormLayer->base.operations            = batchnormLayer->operations;

        batchnormLayer->base.num_temp_tensors      = 0;

        batchnormLayer->base.dump                  = VX_NULL;
        batchnormLayer->base.deinitialize          = vxnneLayer_Deinitialize;


        batchnormLayer->batchnorm_operation.base.layer         = &batchnormLayer->base;
        batchnormLayer->batchnorm_operation.base.target        = VXNNE_OPERATION_TARGET_SW;
        batchnormLayer->batchnorm_operation.base.operatorType = VXNNE_OPERATOR_BATCHNORM;
        batchnormLayer->batchnorm_operation.base.execute       = vxnneExecuteSWBatchNorm;
        batchnormLayer->batchnorm_operation.base.deinitialize  = VX_NULL;
        batchnormLayer->batchnorm_operation.base.dump          = VX_NULL;

        batchnormLayer->base.num_operations    = 1;
        batchnormLayer->operations[0] = (vxnne_operation)&batchnormLayer->batchnorm_operation;

        batchnormLayer->batchnorm_operation.global           = globals;
        batchnormLayer->batchnorm_operation.moving_average_fraction = mafs;
        batchnormLayer->batchnorm_operation.eps              = epss;
        batchnormLayer->batchnorm_operation.mean             = means;
        batchnormLayer->batchnorm_operation.variance         = variances;
        batchnormLayer->batchnorm_operation.gamma            = gamma;
        batchnormLayer->batchnorm_operation.beta             = beta;
        batchnormLayer->batchnorm_operation.outputs          = outputs;

        node->layer = &batchnormLayer->base;
    }

exit:

    return status;

}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNBatchNormalizationLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_float64 vxnneGetDate64(vx_enum format, vx_int32 index, vx_uint8_ptr src, vx_uint8 fixPointPos)
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
        value = Fp16toFp32(data_ptr[index]);
        }
        break;
    default:
        gcmPRINT("Not support format :%d\n", format);
        break;
    }
    return value;
}

#define FP32_MIN -3.402823466e+38F

#define FP16_MAX 3.402823466e+38F
#define FP16_MIN -3.402823466e+38F

#define SATURATE_SIGN(type) \
    if (value > type##_MAX) \
            value = type##_MAX; \
        else if (value < type##_MIN) \
            value = type##_MIN;

#define SATURATE_UNSIGN(type) \
    if (value > type##_MAX) \
            value = type##_MAX; \
        else if (value < 0) \
            value = 0;

VX_PRIVATE_API vx_float64 vxnneSaturate(vx_float64 data, vx_enum format)
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

VX_PRIVATE_API vx_float64 vxnneWarp(vx_float64 data, vx_enum format)
{
    vx_float64 value = 0;

    switch(format)
    {
    case VX_TYPE_FLOAT32:
        value = (vx_float32)value;
    case VX_TYPE_FLOAT16:
        value = (vx_float32)value;
        break;
    case VX_TYPE_INT32:
        value = (vx_int32)value;
        break;
    case VX_TYPE_INT16:
        value = (vx_int16)value;
        break;
    case VX_TYPE_INT8:
        value = (vx_int8)value;
        break;
    case VX_TYPE_UINT32:
        value = (vx_uint32)value;
        break;
    case VX_TYPE_UINT16:
        value = (vx_uint16)value;
        break;
    case VX_TYPE_UINT8:
        value = (vx_uint8)value;
        break;
    default:
        gcmPRINT("Not support format :%d\n", format);
        break;
    }

    return value;
}

typedef enum
{
    VX_TENSOR_OP_ADD,
    VX_TENSOR_OP_MUL,
    VX_TENSOR_OP_SUB,
}
vx_op_type;

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
    vx_enum rounding,
    vx_enum operation,
    vx_uint8_ptr output_ptr,
    vx_uint8 outputFixPointPos,
    vx_enum outputFormat)
{
    vx_int32 i = 0;

    for (i = 0; i < size; i ++)
    {
        vx_float64 input1_value = vxnneGetDate64(input1Format, i, input1_ptr, input1FixPointPos);
        vx_float64 input2_value = vxnneGetDate64(input2Format, i, input2_ptr, input2FixPointPos);
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
            value = (input1_value * input2_value) * scale;
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

        vxnneSaveDate((vx_type_e)outputFormat, i, value, output_ptr, outputFixPointPos);
    }

    return VX_SUCCESS;
}

vx_status vxnneExecuteSWEltwise(struct _vxnne_operation_s *operation)
{
    vxnne_eltwise_sw_operation eltwiseOperation   = (vxnne_eltwise_sw_operation)operation;

    vx_tensor input1 = eltwiseOperation->input1;
    vx_tensor input2 = eltwiseOperation->input2;
    vx_tensor output = eltwiseOperation->output;

    vx_enum kernel = eltwiseOperation->kernel;
    vx_int32 dim1 = input1->viewRegion.dimCount;
    vx_int32 dim2 = input2->viewRegion.dimCount;
    vx_enum overflow = eltwiseOperation->overflow->value->e;
    vx_int32 size = 0;
    vx_uint8_ptr input1_ptr = TENSOR_LOGICAL_ADDR(input1);
    vx_uint8_ptr input2_ptr = TENSOR_LOGICAL_ADDR(input2);
    vx_uint8_ptr output_ptr = TENSOR_LOGICAL_ADDR(output);

    vx_uint8 input1FixPointPos = input1->tensorBuffer->fixedPointPos;
    vx_uint8 input2FixPointPos = input2->tensorBuffer->fixedPointPos;
    vx_uint8 outputFixPointPos = output->tensorBuffer->fixedPointPos;

    vx_enum input1Format = input1->tensorBuffer->dataFormat;
    vx_enum input2Format = input2->tensorBuffer->dataFormat;
    vx_enum outputFormat = output->tensorBuffer->dataFormat;

    if (dim1 == dim2)
    {
        switch(kernel)
        {
        case VX_KERNEL_NN_TENSOR_ADD:
            eltwise(input1_ptr, input1FixPointPos, input1Format, input2_ptr, input2FixPointPos, input2Format, size, 1.f, overflow, VX_ROUND_POLICY_TO_ZERO, VX_TENSOR_OP_ADD, output_ptr, outputFixPointPos, outputFormat);
            break;
        case VX_KERNEL_NN_TENSOR_SUB:
            eltwise(input1_ptr, input1FixPointPos, input1Format, input2_ptr, input2FixPointPos, input2Format, size, 1.f, overflow, VX_ROUND_POLICY_TO_ZERO, VX_TENSOR_OP_SUB, output_ptr, outputFixPointPos, outputFormat);
            break;
        case VX_KERNEL_NN_TENSOR_MUL:
            {
                vx_enum rounding = eltwiseOperation->rounding->value->e;
                vx_float32 scale = eltwiseOperation->scale->value->f32;
                eltwise(input1_ptr, input1FixPointPos, input1Format, input2_ptr, input2FixPointPos, input2Format, size, scale, overflow, rounding, VX_TENSOR_OP_MUL, output_ptr, outputFixPointPos, outputFormat);
            }
            break;
        default:
            gcmPRINT("Not support kenrel: %d\n", kernel);
            break;
        }
    }
    else
        gcmPRINT("Difference dim\n");
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNTensorEltwise(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    vx_status  status  = VX_SUCCESS;

    vx_tensor input1   = (vx_tensor)parameters[0];
    vx_tensor input2   = (vx_tensor)parameters[1];
    vx_scalar scale = NULL;
    vx_scalar overflow = (vx_scalar)parameters[2];
    vx_scalar rounding = NULL;
    vx_tensor output   = (vx_tensor)parameters[3];
    vx_enum kernel     = node->kernel->enumeration;
    vxnne_eltwise_layer eltwiseLayer = {0};
    vxnne_eltwise_sw_operation_s * operation = VX_NULL;

    if (kernel == VX_KERNEL_NN_TENSOR_MUL)
    {
        scale = (vx_scalar)parameters[2];
        overflow = (vx_scalar)parameters[3];
        rounding = (vx_scalar)parameters[4];
        output   = (vx_tensor)parameters[5];
    }

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_eltwise_layer_s), (gctPOINTER*)&eltwiseLayer);
    if (!eltwiseLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        goto exit;
    }

    gcoOS_ZeroMemory(eltwiseLayer, sizeof(vxnne_eltwise_layer_s));

    eltwiseLayer->base.name                  = "eltwiseLayer";
    eltwiseLayer->base.node                  = node;
    eltwiseLayer->base.operations            = eltwiseLayer->operations;

    eltwiseLayer->base.num_temp_tensors      = 0;

    eltwiseLayer->base.dump                  = VX_NULL;
    eltwiseLayer->base.deinitialize          = vxnneLayer_Deinitialize;

    operation = &eltwiseLayer->eltwise_operation;

    operation->base.layer         = &eltwiseLayer->base;
    operation->base.target        = VXNNE_OPERATION_TARGET_SW;
    operation->base.operatorType = VXNNE_OPERATOR_ACTIVATION;
    operation->base.execute       = vxnneExecuteSWEltwise;
    operation->base.deinitialize  = VX_NULL;
    operation->base.dump          = VX_NULL;

    eltwiseLayer->base.num_operations    = 1;
    eltwiseLayer->operations[0] = (vxnne_operation)&eltwiseLayer->eltwise_operation;

    operation->kernel           = node->kernel->enumeration;
    operation->input1           = input1;
    operation->input2           = input2;
    operation->scale            = scale;
    operation->overflow         = overflow;
    operation->rounding         = rounding;
    operation->output           = output;

    node->layer = &eltwiseLayer->base;

exit:
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorEltwise_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorEltwise_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorEltwise_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    /* to do */
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorEltwise_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }
    return VX_SUCCESS;
}


vx_status vxnneExecuteSWTensorAdd(vxnne_operation operation)
{
    vxnne_tensor_add_operation eltwiseOperation   = (vxnne_tensor_add_operation)operation;

    vx_tensor input1 = eltwiseOperation->input0;
    vx_tensor input2 = eltwiseOperation->input1;
    vx_tensor output = eltwiseOperation->output;

    vx_int32 dim1 = input1->viewRegion.dimCount;
    vx_int32 dim2 = input2->viewRegion.dimCount;
    vx_enum overflow = eltwiseOperation->policy->value->e;
    vx_int32 size = input1->tensorBuffer->memory.dims[0][0] * input1->tensorBuffer->memory.dims[0][1] * input1->tensorBuffer->memory.dims[0][2];
    vx_uint8_ptr input1_ptr = TENSOR_LOGICAL_ADDR(input1);
    vx_uint8_ptr input2_ptr = TENSOR_LOGICAL_ADDR(input2);
    vx_uint8_ptr output_ptr = TENSOR_LOGICAL_ADDR(output);
    vx_enum input1Format = input1->tensorBuffer->dataFormat;
    vx_enum input2Format = input2->tensorBuffer->dataFormat;
    vx_enum outputFormat = output->tensorBuffer->dataFormat;
    vx_uint8 input1FpPos = input1->tensorBuffer->fixedPointPos;
    vx_uint8 input2FpPos = input2->tensorBuffer->fixedPointPos;
    vx_uint8 outputFpPos = output->tensorBuffer->fixedPointPos;

    if (dim1 == dim2)
    {
        eltwise(input1_ptr, input1FpPos, input1Format, input2_ptr, input2FpPos, input2Format, size, 1.f, overflow, VX_ROUND_POLICY_TO_ZERO, VX_TENSOR_OP_ADD, output_ptr, outputFpPos, outputFormat);
    }
    else
        gcmPRINT("Difference dim\n");

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNTensorAdd(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return vxnneLayer_Execute(node->layer);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorAdd_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorAdd_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorAdd_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;

    vx_tensor input0   = (vx_tensor)parameters[0];
    vx_tensor input1   = (vx_tensor)parameters[1];
    vx_scalar policy   = (vx_scalar)parameters[2];
    vx_tensor output   = (vx_tensor)parameters[3];

    vx_bool format_flag = vx_false_e;

    vxnne_tensor_add_layer tensor_add_layer = VX_NULL;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_tensor_add_layer_s), (gctPOINTER*)&tensor_add_layer);
    if (!tensor_add_layer)
    {
        status = VX_ERROR_NO_MEMORY;
        gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        goto exit;
    }

    gcoOS_ZeroMemory(tensor_add_layer, sizeof(vxnne_tensor_add_layer_s));

    vxnneLayer_Initialize(&tensor_add_layer->base, "TensorAddLayer", node, tensor_add_layer->operations, VX_NULL);

    format_flag = (vx_bool)((input0->tensorBuffer->dataFormat == VX_TYPE_FLOAT16) && (input1->tensorBuffer->dataFormat == VX_TYPE_FLOAT16) && (output->tensorBuffer->dataFormat == VX_TYPE_FLOAT16));
    format_flag |= (vx_bool)((input0->tensorBuffer->dataFormat == VX_TYPE_INT8) && (input1->tensorBuffer->dataFormat == VX_TYPE_INT8) && (output->tensorBuffer->dataFormat == VX_TYPE_FLOAT16));
    if (format_flag)
    {
        vxnne_shader_executable shaderExecutable;

        shaderExecutable = vxnneGetTensorAddShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_ADD, &node->kernelAttributes.borderMode, input0, input1, policy, output);

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto exit;
        }

        status = vxnneShaderOperation_Initialize(&tensor_add_layer->tensorAddSH,
                                        &tensor_add_layer->base,
                                        VXNNE_OPERATOR_TENSOR_ADD,
                                        shaderExecutable);

        if (status != VX_SUCCESS)
            goto exit;

        tensor_add_layer->base.num_operations = 1;
        tensor_add_layer->operations[0] = &tensor_add_layer->tensorAddSH.base;
    }
    else
    {
        vxnneOperation_Initialize(&tensor_add_layer->tensorAddSW.base,
                                &tensor_add_layer->base,
                                VXNNE_OPERATION_TARGET_SW,
                                VXNNE_OPERATOR_TENSOR_ADD,
                                vxnneExecuteSWTensorAdd,
                                vxnneShaderOperation_Deinitialize);

        tensor_add_layer->base.num_operations = 1;
        tensor_add_layer->operations[0] = &tensor_add_layer->tensorAddSW.base;

        tensor_add_layer->tensorAddSW.input0    = input0;
        tensor_add_layer->tensorAddSW.input1    = input1;
        tensor_add_layer->tensorAddSW.policy    = policy;
        tensor_add_layer->tensorAddSW.output    = output;
    }

    node->layer = &tensor_add_layer->base;
    return status;

exit:
    if (tensor_add_layer)
        gcoOS_Free(NULL, (gctPOINTER)tensor_add_layer);

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorAdd_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNRPNLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return vxnneLayer_Execute(node->layer);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNRPNLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNRPNLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

#define SCALE_BACK_RATIO    1.0f /* 500x375 -> 800x600 */
#define CLIP_WIDTH          800
#define CLIP_HEIGHT         600

#define TOP_N               6000
#define OVERLAP_THRES       0.7f

int vx_nnk_rpn_layer_cpu_compare(const void * a, const void * b)
{
    float f0 = *(float*)a;
    float f1 = *(float*)b;
    float df = f1 - f0;
    int result = (df == 0.0f)? 0: 1;

    int s = (((*(int*)&df) >>31) <<1) + 1;

    result *= s;

    return result;
}

static void vx_nnk_rpn_layer_cpu_nms(vx_float32_ptr data, vx_int32_ptr pick_count, vx_float32 overlap)
{
    vx_float32 fProb;
    vx_float32 area[TOP_N], w, h, xx0, yy0, xx1, yy1, inter, iou;


    vx_uint32 i = 0, r = 0, p = 0;
    vx_uint32 to_pick = 0, picked = 0;

    r = *pick_count;

    for (i=0; i<r; i++)
    {
        w = data[i * 5 + 3] - data[i * 5 + 1] + 1.0f;
        h = data[i * 5 + 4] - data[i * 5 + 2] + 1.0f;

        area[i] = w * h;
    }

    picked = 0;
    to_pick = r;
    p = 0;

    while (to_pick > 1 && p < r)
    {
        /* Pick p. And remove overlapped with p*/
        for (i= p+1; i<r; i++)
        {
            fProb = *(data + i*5);
            if (fProb <= 0.0)
                continue;

            xx0 = gcmMAX(data[i * 5 + 1], data[p * 5 + 1]);
            yy0 = gcmMAX(data[i * 5 + 2], data[p * 5 + 2]);
            xx1 = gcmMIN(data[i * 5 + 3], data[p * 5 + 3]);
            yy1 = gcmMIN(data[i * 5 + 4], data[p * 5 + 4]);

            w = gcmMAX(0.0f, xx1-xx0+1.0f);
            h = gcmMAX(0.0f, yy1-yy0+1.0f);

            inter = w*h;
            iou = inter /(area[i]+area[p] - inter);

            if (iou > overlap)
            {
                data[i * 5] = -1.0f; /* remove */
                to_pick--;
            }
        }

        picked++;
        to_pick--;

        do {
            p++;
        }
        while (data[i * 5] <= 0.0 && p < r);
    }

    *pick_count = picked;
}

#define GET_SIZE_FROM_TENSOR(tensor, index) (tensor->viewRegion.viewEnds[index] - tensor->viewRegion.viewStarts[index])

vx_status vxnneExecuteSWRPN(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;
    vxnne_tensor_rpn_operation rpnOperation = (vxnne_tensor_rpn_operation)operation;

    vx_tensor score  = (vx_tensor)rpnOperation->score;
    vx_tensor bbox = (vx_tensor)rpnOperation->bbox;
    vx_tensor outputs  = (vx_tensor)rpnOperation->output;

    vx_int32 score_depth = GET_SIZE_FROM_TENSOR(score, 1);
    vx_int32 score_height = GET_SIZE_FROM_TENSOR(score, 2);
    vx_int32 score_width = GET_SIZE_FROM_TENSOR(score, 3);
    vx_int32 score_count = score_width * score_height * score_depth/2;/*51 x 39 x 18 -> 17901 x 2 */

    vx_int32 bbox_depth = GET_SIZE_FROM_TENSOR(bbox, 1);
    vx_int32 bbox_height = GET_SIZE_FROM_TENSOR(bbox, 2);
    vx_int32 bbox_width = GET_SIZE_FROM_TENSOR(bbox, 3);
    vx_uint32 bbox_count = bbox_width * bbox_height * bbox_depth;

    vx_type_e format = (vx_type_e)outputs->tensorBuffer->dataFormat;

    vx_int32 output_width = GET_SIZE_FROM_TENSOR(outputs, 1);
    vx_int32 output_roi = GET_SIZE_FROM_TENSOR(outputs, 0);

    vx_float32_ptr score_data;
    vx_float32_ptr bbox_data;
    vx_float32_ptr output_data;

    vx_int32 i = 0, w = 0, h = 0, d = 0;
    vx_int32 real_roi = TOP_N, offset = 0;
    vx_float32_ptr score_buffer = (vx_float32_ptr)malloc((score_count * 2) * sizeof(vx_float32));
    vx_float32_ptr buffer = (vx_float32_ptr)malloc((score_count + bbox_count) * sizeof(vx_float32));

    vxoTensor_GetTensorViewMemory(score, (gctPOINTER *)&score_data, VX_NULL);
    vxoTensor_GetTensorViewMemory(bbox, (gctPOINTER *)&bbox_data, VX_NULL);
    vxoTensor_GetTensorViewMemory(outputs, (gctPOINTER *)&output_data, VX_NULL);

    /* (1) FP16-> FP32 */


    /* (2) Reshape score 51 x 39 x 18 -> 17901 x 2 */

    /* (3) Softmax */
    /*                   float16/32
     *           __________________________
     *    score |* _ _ _51 x 39 x 9  _ _ _ |
     *          |*______51 x 39 x 9________|
     *    bbox  |****                      |
     *          |_ _ _ _ _ _ _ _ _ _ _ _ _ |                  __________________________
     *          |                          |   =>  score_buf |* _ _ _51 x 39 x 9  _ _ _ |
     *          |_ _ _ _ _ _ _ _ _ _ _ _ _ |        float32  |*______51 x 39 x 9________|
     *          |                          |
     *          |_ _ _ _ _ _ _ _ _ _ _ _ _ |
     *          |                          |
     *          |_ _ _ _ _ _ _ _ _ _ _ _ _ |
     *
     *
     */
    for (i = 0; i < score_count; i++)
    {
        vx_float32 score0 = (vx_float32)vxnneGetDate(format, i, (vx_uint8_ptr)score_data, score->tensorBuffer->fixedPointPos);
        vx_float32 score1 = (vx_float32)vxnneGetDate(format, i + score_count, (vx_uint8_ptr)score_data, score->tensorBuffer->fixedPointPos);
        vx_float32 sum = 0.f, max = gcmMAX(score0, score1);

        score0 -= max;
        score1 -= max;

        score0 = expf(score0);
        score1 = expf(score1);
        sum = score0 + score1;

        score_buffer[i] = score0 / sum;
        score_buffer[i + score_count] = score1 / sum;
    }

    /* (4) reshape score probability. Background is the first. Object is the second. Use the second pScore + 17901.*/
    /* float32 _____
     *      0 |s0   |
     *      1 |s1   |
     *      2 |s2   |
     *      3 |s3   |
     * =>   4 |s4   |
     *      5 |s5   |
     *      . | .   |
     *      . | .   |
     *      . | .   |
     *  19701 |_____|
     *
     */

    for (w = 0; w < score_width; w ++)
    {
        for (h = 0; h < score_height; h ++)
        {
            for (d = 0; d < score_depth/2; d ++)
            {
                vx_uint32 index = d * score_height * score_width + h * score_width + w;

                buffer[offset] = score_buffer[index + score_count];
                offset += 5;
            }
        }
    }
    offset = 0;
    /* (5) BBox reshape  Reshaped Prob BBox merged in pProbBox */
    /* float32 _______
     *      0 |s0 xxxx|
     *      1 |s1 xxxx|
     *      2 |s2 xxxx|
     *      3 |s3 xxxx|
     * =>   4 |s4 xxxx|
     *      5 |s5 xxxx|
     *      . | .     |
     *      . | .     |
     *      . | .     |
     *  19701 |_______|
     *
     */
    for (w = 0; w < bbox_width; w ++)
    {
        for (h = 0; h < bbox_height; h ++)
        {
            for (d = 0; d < bbox_depth; d ++)
            {
                vx_uint32 index = d * score_height * score_width + h * score_width + w;

                if (d%4 == 0)offset++;

                buffer[offset] = (vx_float32)vxnneGetDate(format, index, (vx_uint8_ptr)bbox_data, bbox->tensorBuffer->fixedPointPos);

                offset ++;
            }
        }
    }

    /* (6)  BBox transform */

    for (i = 0; i < score_count; i++)
    {
        vx_float32 src_w = anchors[i][2] - anchors[i][0] + 1;
        vx_float32 src_h = anchors[i][3] - anchors[i][1] + 1;
        vx_float32 src_ctr_x = anchors[i][0] + 0.5f * (src_w-1);
        vx_float32 src_ctr_y = anchors[i][1] + 0.5f * (src_h-1);

        vx_float32 dst_ctr_x = buffer[5 * i + 1];
        vx_float32 dst_ctr_y = buffer[5 * i + 2];
        vx_float32 dst_scl_x = buffer[5 * i + 3];
        vx_float32 dst_scl_y = buffer[5 * i + 4];

        vx_float32 pred_ctr_x = (dst_ctr_x*src_w) + src_ctr_x;
        vx_float32 pred_ctr_y = (dst_ctr_y*src_h) + src_ctr_y;

        vx_float32 pred_w = expf(dst_scl_x) * src_w;
        vx_float32 pred_h = expf(dst_scl_y) * src_h;

        buffer[5 * i + 1] = gcmMAX(gcmMIN((pred_ctr_x - 0.5f * (pred_w-1)) * SCALE_BACK_RATIO, CLIP_WIDTH), 1);
        buffer[5 * i + 2] = gcmMAX(gcmMIN((pred_ctr_y - 0.5f * (pred_h-1)) * SCALE_BACK_RATIO, CLIP_HEIGHT), 1);
        buffer[5 * i + 3] = gcmMAX(gcmMIN((pred_ctr_x + 0.5f * (pred_w-1)) * SCALE_BACK_RATIO, CLIP_WIDTH), 1);
        buffer[5 * i + 4] = gcmMAX(gcmMIN((pred_ctr_y + 0.5f * (pred_h-1)) * SCALE_BACK_RATIO, CLIP_HEIGHT), 1);
    }

    /* (7) filter out too small boxes */


    /* (8) sort */
    qsort ((void*)buffer, score_count, 5 * sizeof(vx_float32), vx_nnk_rpn_layer_cpu_compare);

    /* (9) NMS */
    vx_nnk_rpn_layer_cpu_nms((vx_float32_ptr)buffer, &real_roi, OVERLAP_THRES);

    real_roi = gcmMIN(gcmMIN(real_roi, NUM_ROI), output_roi);

    /* (10) Copy to dst.  */
    offset = 0;
    memset(output_data, 0, real_roi * output_width);

    for (i = 0; i < TOP_N; i++)
    {
        if (buffer[5 * i] > 0)
        {
            if (output_width == 5)
            {
                vxnneSaveDate(format, offset, buffer[5 * i], (vx_uint8_ptr)output_data, outputs->tensorBuffer->fixedPointPos);
                vxnneSaveDate(format, offset + 1, buffer[5 * i + 1], (vx_uint8_ptr)output_data, outputs->tensorBuffer->fixedPointPos);
                vxnneSaveDate(format, offset + 2, buffer[5 * i + 2], (vx_uint8_ptr)output_data, outputs->tensorBuffer->fixedPointPos);
                vxnneSaveDate(format, offset + 3, buffer[5 * i + 3], (vx_uint8_ptr)output_data, outputs->tensorBuffer->fixedPointPos);
                vxnneSaveDate(format, offset + 4, buffer[5 * i + 4], (vx_uint8_ptr)output_data, outputs->tensorBuffer->fixedPointPos);

                offset += 5;

            }
            else if (output_width == 4)
            {
                /*         ___5___            _4__
                 *      0 |s0 xxxx|        0 |xxxx|
                 *      1 |s1 xxxx|        0 |xxxx|
                 *      2 |s2 xxxx|        0 |xxxx|
                 *      3 |s3 xxxx|        0 |xxxx|
                 *      4 |s4 xxxx|        0 |xxxx|
                 *      5 |s5 xxxx|  =>    0 |xxxx|
                 *      . | .     |        . | .  |
                 *      . | .     |        . | .  |
                 *      . | .     |        . | .  |
                 *  19701 |_______|    19701 |____|
                 *
                 */
                vxnneSaveDate(format, offset, buffer[5 * i + 1], (vx_uint8_ptr)output_data, outputs->tensorBuffer->fixedPointPos);
                vxnneSaveDate(format, offset + 1, buffer[5 * i + 2], (vx_uint8_ptr)output_data, outputs->tensorBuffer->fixedPointPos);
                vxnneSaveDate(format, offset + 2, buffer[5 * i + 3], (vx_uint8_ptr)output_data, outputs->tensorBuffer->fixedPointPos);
                vxnneSaveDate(format, offset + 3, buffer[5 * i + 4], (vx_uint8_ptr)output_data, outputs->tensorBuffer->fixedPointPos);
                offset += 4;
            }
            else
                printf("Not support item size: %d!\n", output_width);

            if (offset > (5 * real_roi))
                break;
        }
    }
    free(score_buffer);
    free(buffer);

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNRPNLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status  status                     = VX_SUCCESS;

    vx_tensor  score                      = (vx_tensor)parameters[0];
    vx_tensor  bbox                       = (vx_tensor)parameters[1];
    vx_tensor  outputs                    = (vx_tensor)parameters[2];

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    {
        vxnne_tensor_rpn_layer rpnLayer;

        gcoOS_Allocate(gcvNULL, sizeof(vxnne_batchnorm_layer_s), (gctPOINTER*)&rpnLayer);
        if (!rpnLayer)
        {
            status = VX_ERROR_NO_MEMORY;
            gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
            goto exit;
        }

        gcoOS_ZeroMemory(rpnLayer, sizeof(vxnne_batchnorm_layer_s));

        rpnLayer->base.name                  = "RpnLayer";
        rpnLayer->base.node                  = node;
        rpnLayer->base.operations            = rpnLayer->operations;

        rpnLayer->base.num_temp_tensors      = 0;

        rpnLayer->base.dump                  = VX_NULL;
        rpnLayer->base.deinitialize          = vxnneLayer_Deinitialize;


        rpnLayer->tensorRpnSW.base.layer         = &rpnLayer->base;
        rpnLayer->tensorRpnSW.base.target        = VXNNE_OPERATION_TARGET_SW;
        rpnLayer->tensorRpnSW.base.operatorType  = VXNNE_OPERATOR_RPN;
        rpnLayer->tensorRpnSW.base.execute       = vxnneExecuteSWRPN;
        rpnLayer->tensorRpnSW.base.deinitialize  = VX_NULL;
        rpnLayer->tensorRpnSW.base.dump          = VX_NULL;

        rpnLayer->base.num_operations    = 1;
        rpnLayer->operations[0] = (vxnne_operation)&rpnLayer->tensorRpnSW;

        rpnLayer->tensorRpnSW.score           = score;
        rpnLayer->tensorRpnSW.bbox            = bbox;
        rpnLayer->tensorRpnSW.output          = outputs;

        node->layer = &rpnLayer->base;
    }

exit:

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNRPNLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }

    return VX_SUCCESS;
}

/**************************************************************************************
 *                     ROI POOL
 *************************************************************************************/

vx_status vxnneExecuteSWROIPool(struct _vxnne_operation_s *operation)
{
    vx_status status = VX_SUCCESS;

    vxnne_tensor_roipool_operation roipoolOperation = (vxnne_tensor_roipool_operation)operation;

    vx_tensor input_data  = roipoolOperation->input_data;
    vx_tensor input_rois  = roipoolOperation->input_rois;
    vx_tensor outputs     = roipoolOperation->output;
    vx_scalar spatial_scales = roipoolOperation->spatial_scales;

    vx_int32 height = GET_SIZE_FROM_TENSOR(input_data, 2);/*39*/
    vx_int32 width = GET_SIZE_FROM_TENSOR(input_data, 3);/*51*/

    vx_int32 stride_w = GET_SIZE_FROM_TENSOR(input_rois, 1);/*5 or 4*/

    vx_int32 num_rois = GET_SIZE_FROM_TENSOR(outputs, 0);/*247*/
    vx_int32 channels = GET_SIZE_FROM_TENSOR(outputs, 1);/*256*/
    vx_int32 pooled_height = GET_SIZE_FROM_TENSOR(outputs, 2);/*6*/
    vx_int32 pooled_width = GET_SIZE_FROM_TENSOR(outputs, 3);/*6*/

    vx_type_e format = (vx_type_e)outputs->tensorBuffer->dataFormat;
    vx_int32 item_size = vxnneGetTypeSize(format);

    vx_uint8_ptr input_data_ptr;
    vx_uint8_ptr rois_data_ptr;
    vx_uint8_ptr output_data_ptr;

    vx_float32 spatial_scale = spatial_scales->value->f32;
    vx_int32 n, c, ph, pw, h, w;

    vxoTensor_GetTensorViewMemory(input_data, (gctPOINTER *)&input_data_ptr, VX_NULL);
    vxoTensor_GetTensorViewMemory(input_rois, (gctPOINTER *)&rois_data_ptr, VX_NULL);
    vxoTensor_GetTensorViewMemory(outputs, (gctPOINTER *)&output_data_ptr, VX_NULL);

    for (n = 0; n < num_rois; ++n)
    {
        vx_int32 offset = 0, roi_batch_ind = 0, roi_start_w = 0, roi_start_h = 0, roi_end_w = 0, roi_end_h = 0;
        vx_int32 roi_height = 0, roi_width = 0;
        vx_float32 roi_size_scale_h = 0, roi_size_scale_w = 0;
        vx_uint8_ptr batch_data = VX_NULL;

        if (stride_w == 5)
        {
            offset = 1;
            roi_batch_ind = (vx_int32)vxnneGetDate(format, 0, rois_data_ptr, input_rois->tensorBuffer->fixedPointPos);
        }

        roi_start_w = (vx_int32)(round((vx_float32)vxnneGetDate(format, offset, rois_data_ptr, input_rois->tensorBuffer->fixedPointPos) * spatial_scale));
        roi_start_h = (vx_int32)round((vx_float32)vxnneGetDate(format, offset + 1, rois_data_ptr, input_rois->tensorBuffer->fixedPointPos) * spatial_scale);
        roi_end_w = (vx_int32)round((vx_float32)vxnneGetDate(format, offset + 2, rois_data_ptr, input_rois->tensorBuffer->fixedPointPos) * spatial_scale);
        roi_end_h = (vx_int32)round((vx_float32)vxnneGetDate(format, offset + 3, rois_data_ptr, input_rois->tensorBuffer->fixedPointPos) * spatial_scale);

        roi_height = (vx_int32)gcmMAX(roi_end_h - roi_start_h + 1, 1);
        roi_width = (vx_int32)gcmMAX(roi_end_w - roi_start_w + 1, 1);
        roi_size_scale_h = (vx_float32)(roi_height) / (vx_float32)(pooled_height);
        roi_size_scale_w = (vx_float32)(roi_width) / (vx_float32)(pooled_width);

        batch_data = input_data_ptr + roi_batch_ind * channels * width * height * item_size;

        for (c = 0; c < channels; ++c)
        {
            for (ph = 0; ph < pooled_height; ++ph)
            {
                for (pw = 0; pw < pooled_width; ++pw)
                {
                    /*
                     *  Compute pooling region for this output unit:
                     *  start (included) = floor(ph * roi_height / pooled_height_)
                     *  end (excluded) = ceil((ph + 1) * roi_height / pooled_height_)
                     */
                    vx_int32 hstart = (vx_int32)(floor((vx_float32)(ph) * roi_size_scale_h));
                    vx_int32 wstart = (vx_int32)(floor((vx_float32)(pw) * roi_size_scale_w));
                    vx_int32 hend = (vx_int32)(ceil((vx_float32)(ph + 1) * roi_size_scale_h));
                    vx_int32 wend = (vx_int32)(ceil((vx_float32)(pw + 1) * roi_size_scale_w));

                    vx_int32 pool_index = 0;
                    vx_bool is_empty = vx_false_e;
                    vx_float32 output_data_v = 0;

                    hstart = gcmMIN(gcmMAX(hstart + roi_start_h, 0), height);
                    hend = gcmMIN(gcmMAX(hend + roi_start_h, 0), height);
                    wstart = gcmMIN(gcmMAX(wstart + roi_start_w, 0), width);
                    wend = gcmMIN(gcmMAX(wend + roi_start_w, 0), width);

                    is_empty = (vx_bool)((hend <= hstart) || (wend <= wstart));

                    pool_index = ph * pooled_width + pw;

                    vxnneSaveDate(format, pool_index, is_empty?0:(-FP32_MAX), output_data_ptr, outputs->tensorBuffer->fixedPointPos);

                    output_data_v = (vx_float32)vxnneGetDate(format, pool_index, output_data_ptr, outputs->tensorBuffer->fixedPointPos);

                    for (h = hstart; h < hend; ++h)
                    {
                        for (w = wstart; w < wend; ++w)
                        {
                            const vx_int32 index = h * width + w;
                            vx_float32 batch_data_v = (vx_float32)vxnneGetDate(format, index, batch_data, input_data->tensorBuffer->fixedPointPos);

                            if (batch_data_v > output_data_v)
                                output_data_v = batch_data_v;
                        }
                    }

                    vxnneSaveDate(format, pool_index, output_data_v, output_data_ptr, outputs->tensorBuffer->fixedPointPos);
                }
            }

            /* Increment all data pointers by one channel*/
            batch_data += width * height * item_size;
            output_data_ptr += pooled_width * pooled_height * item_size;
        }

        /* Increment ROI data pointer */
        if (stride_w == 5)
            rois_data_ptr += 5 * item_size;
        else
            rois_data_ptr += 4 * item_size;
    }

    vxReleaseScalar(&spatial_scales);

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNROIPoolLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return vxnneLayer_Execute(node->layer);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNROIPoolLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNROIPoolLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNROIPoolLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status  status                     = VX_SUCCESS;

    vx_tensor  input_data                 = (vx_tensor)parameters[0];
    vx_tensor  input_rois                 = (vx_tensor)parameters[1];
    vx_scalar  pool_types                 = (vx_scalar)parameters[2];
    vx_tensor  outputs                    = (vx_tensor)parameters[3];

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    {
        vx_float32 spatial_scale = 0.0625f;
        vxnne_tensor_roipool_layer roipoolLayer;
        vx_scalar spatial_scales = vxCreateScalar(vxGetContext((vx_reference)input_data), VX_TYPE_ENUM, &spatial_scale);

        gcoOS_Allocate(gcvNULL, sizeof(vxnne_batchnorm_layer_s), (gctPOINTER*)&roipoolLayer);
        if (!roipoolLayer)
        {
            status = VX_ERROR_NO_MEMORY;
            gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
            goto exit;
        }


        gcoOS_ZeroMemory(roipoolLayer, sizeof(vxnne_batchnorm_layer_s));

        roipoolLayer->base.name                  = "ROIPoolLayer";
        roipoolLayer->base.node                  = node;
        roipoolLayer->base.operations            = roipoolLayer->operations;

        roipoolLayer->base.num_temp_tensors      = 0;

        roipoolLayer->base.dump                  = VX_NULL;
        roipoolLayer->base.deinitialize          = vxnneLayer_Deinitialize;


        roipoolLayer->tensorROIPoolSW.base.layer         = &roipoolLayer->base;
        roipoolLayer->tensorROIPoolSW.base.target        = VXNNE_OPERATION_TARGET_SW;
        roipoolLayer->tensorROIPoolSW.base.operatorType  = VXNNE_OPERATOR_ROIPOOL;
        roipoolLayer->tensorROIPoolSW.base.execute       = vxnneExecuteSWROIPool;
        roipoolLayer->tensorROIPoolSW.base.deinitialize  = VX_NULL;
        roipoolLayer->tensorROIPoolSW.base.dump          = VX_NULL;

        roipoolLayer->base.num_operations    = 1;
        roipoolLayer->operations[0] = (vxnne_operation)&roipoolLayer->tensorROIPoolSW;

        roipoolLayer->tensorROIPoolSW.input_data      = input_data;
        roipoolLayer->tensorROIPoolSW.input_rois      = input_rois;
        roipoolLayer->tensorROIPoolSW.pool_types      = pool_types;
        roipoolLayer->tensorROIPoolSW.spatial_scales  = spatial_scales;
        roipoolLayer->tensorROIPoolSW.output          = outputs;

        node->layer = &roipoolLayer->base;

    }

exit:

    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNROIPoolLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }

    return VX_SUCCESS;
}

vx_status vxnneExecuteSWConvolution(vxnne_operation operation)
{
    vxnne_convolution_operation convolutionOperation   = (vxnne_convolution_operation)operation;

    vx_tensor inputs                = convolutionOperation->inputs;
    vx_tensor weights               = convolutionOperation->weights;
    vx_tensor biases                = convolutionOperation->biases;
    vx_scalar padX                  = convolutionOperation->padX;
    vx_scalar padY                  = convolutionOperation->padY;
    vx_scalar downScaleSizeRounding = convolutionOperation->downScaleSizeRounding;
    vx_tensor outputs               = convolutionOperation->outputs;

    vx_uint32 batch = 1;
    void * inputBaseLogical;
    void * outputBaseLogical;

    void *weightsBaseLogical;
    void *biasesBaseLogical;

    vx_uint32 inputWidth, inputHeight, inputDepth, outputWidth, outputHeight, outputDepth;
    vx_uint32 kernelXYSize, stride;
    vx_uint32 k, p, j, i;
    vx_uint8_ptr dataSrc;
    vx_uint8_ptr dataDst;
    vx_uint8_ptr dataWeight;
    vx_uint8_ptr dataBias;
    vx_type_e inputFormat;
    vx_type_e weightFormat;
    vx_type_e biasFormat;
    vx_type_e outputFormat;

    vx_enum downScaleSizeRoundingValue = downScaleSizeRounding->value->e;
    vx_uint32 padXValue = padX->value->n32;
    vx_uint32 padYValue = padY->value->n32;

    batch = (inputs->tensorBuffer->memory.dims[0][3] == 0) ? 1 : inputs->tensorBuffer->memory.dims[0][3];

    vxoTensor_GetTensorViewMemory(inputs, &inputBaseLogical, VX_NULL);
    vxoTensor_GetTensorViewMemory(weights, &weightsBaseLogical, VX_NULL);
    vxoTensor_GetTensorViewMemory(biases, &biasesBaseLogical, VX_NULL);
    vxoTensor_GetTensorViewMemory(outputs, &outputBaseLogical, VX_NULL);

    dataSrc      = (vx_uint8_ptr)inputBaseLogical;
    dataDst      = (vx_uint8_ptr)outputBaseLogical;
    dataWeight   = (vx_uint8_ptr)weightsBaseLogical;
    dataBias     = (vx_uint8_ptr)biasesBaseLogical;
    inputFormat  = (vx_type_e)(inputs->tensorBuffer->dataFormat);
    weightFormat = (vx_type_e)(weights->tensorBuffer->dataFormat);
    biasFormat   = (vx_type_e)(biases->tensorBuffer->dataFormat);
    outputFormat = (vx_type_e)(inputs->tensorBuffer->dataFormat);

    inputWidth   = inputs->tensorBuffer->memory.dims[0][0];
    inputHeight  = inputs->tensorBuffer->memory.dims[0][1];
    inputDepth   = inputs->tensorBuffer->memory.dims[0][2];
    outputWidth  = outputs->tensorBuffer->memory.dims[0][0];
    outputHeight = outputs->tensorBuffer->memory.dims[0][1];
    outputDepth  = outputs->tensorBuffer->memory.dims[0][2];

    kernelXYSize = weights->tensorBuffer->memory.dims[0][0];

    /* Calculate stride = (w + 2*pad - weight)/(output_w - 1) */
    stride = vxoNNExternsionConvlutionRound((vx_float32)(inputWidth + 2 * padXValue - kernelXYSize) / (outputWidth - 1), downScaleSizeRoundingValue);

    gcoOS_MemFill(outputBaseLogical, 0, outputWidth * outputHeight * outputDepth * vxnneGetTypeSize(outputFormat));

    for (k = 0; k < batch; k++)
    {
        dataSrc    = (vx_uint8_ptr)inputBaseLogical + k * inputWidth * inputHeight * inputDepth * vxnneGetTypeSize(inputFormat);
        dataWeight = (vx_uint8_ptr)weightsBaseLogical;
        dataDst    = (vx_uint8_ptr)outputBaseLogical + k * outputWidth * outputHeight * outputDepth * vxnneGetTypeSize(outputFormat);

        for (p = 0; p < outputDepth; p ++)
        {
            for (j = 0; j < outputHeight; j ++)
            {
                for (i = 0; i < outputWidth; i ++)
                {
                    vx_int32 hStart = j * stride - padYValue;
                    vx_int32 wStart = i * stride - padXValue;
                    vx_int32 hEnd = gcmMIN(hStart + kernelXYSize, inputHeight);
                    vx_int32 wEnd = gcmMIN(wStart + kernelXYSize, inputWidth);
                    vx_int32 indexOut = 0;
                    vx_int32 indexBias = 0;
                    vx_int32 h, w = 0;
                    vx_int32 m, n = 0;
                    vx_uint32 d;
                    vx_float32 sum = 0;
                    vx_uint32 kernelXStart, kernelYStart;

                    kernelYStart = hStart < 0 ? padYValue : 0;
                    kernelXStart = wStart < 0 ? padXValue : 0;
                    hStart = gcmMAX(hStart, 0);
                    wStart = gcmMAX(wStart, 0);

                    indexOut = j * (outputWidth) + i;

                    for (d = 0; d < inputDepth; d++)
                    {
                        for (h = hStart, n = kernelYStart; h < hEnd; ++ h, n++)
                        {
                            for (w = wStart, m = kernelXStart; w < wEnd; ++ w, m++)
                            {
                                const vx_int32 indexSrc = d * inputWidth * inputHeight + h * (inputWidth) + w;
                                const vx_int32 indexWeight = d * kernelXYSize * kernelXYSize + n * kernelXYSize + m;
                                sum += vxnneGetDate(inputFormat, indexSrc, (vx_uint8_ptr)dataSrc, inputs->tensorBuffer->fixedPointPos) *
                                       vxnneGetDate(weightFormat, indexWeight, (vx_uint8_ptr)dataWeight, weights->tensorBuffer->fixedPointPos);
                            }
                        }
                    }

                    indexBias = p;

                    if (biasFormat == VX_TYPE_FLOAT32)
                    {
                        sum += vxnneGetDate(biasFormat, indexBias, (vx_uint8_ptr)dataBias, biases->tensorBuffer->fixedPointPos);
                    }
                    else if (weightFormat == VX_TYPE_FLOAT16 && biasFormat == VX_TYPE_INT32)
                    {
                        vx_int8 value = (vx_int8)(*((vx_int32*)dataBias + i));
                        vx_float32 biasValue = Int8toFp32(value, biases->tensorBuffer->fixedPointPos);
                        sum += biasValue;
                    }
                    else
                    {
                        printf("can't support this bias data format\n");
                        gcmASSERT(0);
                    }

                    vxnneSaveDate(outputFormat, indexOut, sum, dataDst, outputs->tensorBuffer->fixedPointPos);
                }
            }

            dataWeight += kernelXYSize * kernelXYSize * inputDepth * vxnneGetTypeSize(weightFormat);
            dataDst += outputWidth * outputHeight * vxnneGetTypeSize(outputFormat);
        }
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNConvolutionLayer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return vxnneLayer_Execute(node->layer);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionLayer_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;

    vx_tensor inputs                = (vx_tensor)parameters[0];
    vx_tensor weights               = (vx_tensor)parameters[1];
    vx_tensor biases                = (vx_tensor)parameters[2];
    vx_scalar padX                  = (vx_scalar)parameters[3];
    vx_scalar padY                  = (vx_scalar)parameters[4];
    vx_scalar accumulatorBits       = (vx_scalar)parameters[5];
    vx_scalar overflowPolicy        = (vx_scalar)parameters[6];
    vx_scalar roundingPolicy        = (vx_scalar)parameters[7];
    vx_scalar downScaleSizeRounding = (vx_scalar)parameters[8];
    vx_tensor outputs               = (vx_tensor)parameters[9];

    vxnne_convolution_layer convolutionLayer = VX_NULL;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_convolution_layer_s), (gctPOINTER*)&convolutionLayer);
    if (!convolutionLayer)
    {
        status = VX_ERROR_NO_MEMORY;
        gcmPRINT("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        goto exit;
    }

    gcoOS_ZeroMemory(convolutionLayer, sizeof(vxnne_convolution_layer_s));

    vxnneLayer_Initialize(&convolutionLayer->base, "ConvolutionLayer", node, convolutionLayer->operations, VX_NULL);

    vxnneOperation_Initialize(&convolutionLayer->convolutionSW.base,
                            &convolutionLayer->base,
                            VXNNE_OPERATION_TARGET_SW,
                            VXNNE_OPERATOR_CONVOLUTION,
                            vxnneExecuteSWConvolution,
                            VX_NULL);

    convolutionLayer->base.num_operations = 1;
    convolutionLayer->operations[0] = &convolutionLayer->convolutionSW.base;

    convolutionLayer->convolutionSW.inputs                = inputs;
    convolutionLayer->convolutionSW.weights               = weights;
    convolutionLayer->convolutionSW.biases                = biases;
    convolutionLayer->convolutionSW.padX                  = padX;
    convolutionLayer->convolutionSW.padY                  = padY;
    convolutionLayer->convolutionSW.accumulatorBits       = accumulatorBits;
    convolutionLayer->convolutionSW.overflowPolicy        = overflowPolicy;
    convolutionLayer->convolutionSW.roundingPolicy        = roundingPolicy;
    convolutionLayer->convolutionSW.downScaleSizeRounding = downScaleSizeRounding;
    convolutionLayer->convolutionSW.outputs               = outputs;


    node->layer = &convolutionLayer->base;

exit:
    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNConvolutionLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
    }

    return VX_SUCCESS;
}

