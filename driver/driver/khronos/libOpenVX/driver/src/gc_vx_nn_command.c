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

#define SHOW_TP_SPLITING_INFO 1

VX_PRIVATE_API vx_float32 Fp21toFp32(const vx_uint32 in)
{
    vx_uint32 t1;
    vx_uint32 t2;
    vx_uint32 t3;
    vx_float32 out;

    t1 = in & 0xfffff;                      // Non-sign bits
    t2 = in & 0x100000;                     // Sign bit
    t3 = in & 0xf8000;                      // Exponent

    t1 <<= 8;                               // Align mantissa on MSB
    t2 <<= 11;                              // Shift sign bit into position

    t1 += 0x38000000;                       // Adjust bias

    t1 = (t3 == 0 ? 0 : t1);                // Denormals-as-zero

    t1 |= t2;                               // Re-insert sign bit

    *((uint32_t*)&out) = t1;

    return out;
}

VX_PRIVATE_API vx_int32 Fp32toFp21(vx_float32 val)
{
    vx_uint32 f32 = (*(vx_uint32 *) &val);
    vx_int32 f21 = 0;
    /* Decode IEEE 754 little-endian 32-bit floating-point value */
    int sign = (f32 >> 11) & 0x100000;
    /* Map exponent to the range [-127,128] */
    int exponent = ((f32 >> 23) & 0xff) - 127;
    int mantissa = f32 & 0x007fffff;
    if (exponent == 128)
    { /* Infinity or NaN */
        if (mantissa)
        {
            /* Flush NaN to 0. */
            f21 = sign;
        }
        else
        {
            /* Clamp to HALF_MAX/HALF_MIN. */
            f21 = sign | ((F16_EXPONENT_BITS - 1) << F21_EXPONENT_SHIFT) | F21_MANTISSA_BITS;
        }
    }
    else if (exponent > 15)
    { /* Overflow - clamp to HALF_MAX/HALF_MIN. */
        f21 = sign | ((F16_EXPONENT_BITS - 1) << F21_EXPONENT_SHIFT) | F21_MANTISSA_BITS;
    }
    else if (exponent > -15)
    { /* Representable value */
        /* RTNE */
        int roundingBit = (mantissa >> (F21_MANTISSA_SHIFT - 1)) & 0x1;
        int stickyBits = mantissa & 0xFFF;
        exponent += F16_EXPONENT_BIAS;
        mantissa >>= F21_MANTISSA_SHIFT;
        if (roundingBit && stickyBits)
        {
            mantissa++;
            if (mantissa > F21_MANTISSA_BITS)
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
                    mantissa &= F21_MANTISSA_BITS;
                }
            }
        }
        f21 = sign | (exponent << F21_EXPONENT_SHIFT) | mantissa;
    }
    else
    {
        f21 = sign;
    }
    return f21;
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

VX_PRIVATE_API vx_int8 getHWDataFormat(vx_enum dataFormat)
{
    switch (dataFormat)
    {
    case VX_TYPE_UINT8:
        return 0x0;
    case VX_TYPE_INT8:
        return 0x2;
    case VX_TYPE_FLOAT16:
        return 0x1;
    case VX_TYPE_INT16:
        return 0x4;
    case VX_TYPE_BFLOAT16:
        return 0x7;
    default:
        break;
    }

    vxError("hw not support this data format. function %s line %d", __FUNCTION__, __LINE__);
    return -1;
}

VX_PRIVATE_API void _checkSramOverflow(
    vx_context context,
    vx_weights_biases_parameter wb,
    vx_nn_cmd_info_u * info,
    vx_uint32 dataUnitByte
    )
{

    vx_uint32 oneNumOfPattern = 0;
    vx_uint32 cacheSizeNeeded;
    vx_uint32 cacheSizeAllocated;
    vx_uint32 decoderWorkNum;
    vx_uint32 dataUnitNum;
    vx_uint32 patternLoopCount;
    vx_uint32 sramDataUnitNum1Core;
    vx_uint32 dataUnitLastLoop;
    vx_uint32 sramDataUnitNumLastLoop = 0;
    vx_uint32 i;
    vx_uint32 coreCountUsed = context->nnConfig.fixedFeature.nnCoreCount;
    vx_uint32 maxSizeOfCore;
    vx_uint64 kernelFullPattern = ((vx_uint64)info->vx_nn_general_cmd_info.kernelPatternHigh32Bits << 32) | info->vx_nn_general_cmd_info.kernelPatternLow32Bits;

    switch (WB_WEIGHT_DATA_FORMAT(wb))
    {
    case VX_TYPE_INT16:
        coreCountUsed = context->nnConfig.fixedFeature.nnCoreCountInt16;
        break;
    case VX_TYPE_UINT8:
    case VX_TYPE_INT8:
        coreCountUsed = context->nnConfig.fixedFeature.nnCoreCountInt8;
        break;
    case VX_TYPE_FLOAT16:
        coreCountUsed = context->nnConfig.fixedFeature.nnCoreCountFloat16;
        break;
    default:
        break;
    }

    maxSizeOfCore = (vx_uint32)wb->slice_array[0].kernel_max_stream_size_percore;

    for (i = 0; i != (info->vx_nn_general_cmd_info.kernelPatternMsb+1); i++)
    {
        oneNumOfPattern += (kernelFullPattern >> i) & 1;
    }

    cacheSizeAllocated  = (info->vx_nn_general_cmd_info.kernelCacheEndAddress - info->vx_nn_general_cmd_info.kernelCacheStartAddress);
    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP0))
    {
        /* v8 hw has post process, treate as one more core. */
        decoderWorkNum      = gcmMIN(info->vx_nn_general_cmd_info.outImageZSize, coreCountUsed) + 1;
    }
    else
    {
        decoderWorkNum      = gcmMIN(info->vx_nn_general_cmd_info.outImageZSize, coreCountUsed);
    }
    dataUnitNum         = maxSizeOfCore / dataUnitByte;

    patternLoopCount    = dataUnitNum / (info->vx_nn_general_cmd_info.kernelPatternMsb + 1); /* full loop count */
    sramDataUnitNum1Core = patternLoopCount * oneNumOfPattern;
    dataUnitLastLoop    = dataUnitNum - patternLoopCount * (info->vx_nn_general_cmd_info.kernelPatternMsb + 1);

    for (i = 0; (i != (info->vx_nn_general_cmd_info.kernelPatternMsb+1)) && (dataUnitLastLoop != 0); i++)
    {
        dataUnitLastLoop--;
        if ((kernelFullPattern >> i) & 1)
        {
            sramDataUnitNumLastLoop += 1;
        }
    }

    sramDataUnitNum1Core += sramDataUnitNumLastLoop;

    cacheSizeNeeded = sramDataUnitNum1Core * dataUnitByte * decoderWorkNum;

    if (cacheSizeAllocated < cacheSizeNeeded)
    {
        vxmASSERT(0);
    }
}

VX_PRIVATE_API vx_status vxnneCommandBuffer_GetNNSplitCommandInfo(
    vx_context                   context,
    vxnne_tensor_info            input,
    vxnne_tensor_info            output,
    vx_op_param                  conv_cmd_ptr,
    vx_nn_cmd_split_info_u *     *sinfo_array_ptr,
    vx_uint32 *                  sinfo_num_ptr
    )
{
#define MAX_NN_SPLIT_XY_NUM  8

    vx_uint32 i = 0, j = 0, k = 0, index = 0;
    vx_weights_biases_parameter wb;
    vx_uint32 inImageXSizeOrig = input->width, inImageYSizeOrig = input->height, outImageXSizeOrig = output->width, outImageYSizeOrig = output->height;
    vx_int32  inImageXOffsetOrig = 0, inImageYOffsetOrig = 0;
    vx_uint32 xcount = gcmALIGN_NP2(input->width, NN_IMAGE_XSIZE_MAX) / NN_IMAGE_XSIZE_MAX;
    vx_uint32 ycount = gcmALIGN_NP2(input->height, NN_IMAGE_YSIZE_MAX) / NN_IMAGE_YSIZE_MAX;
    vx_uint32 zcount = 1;
    vx_uint32 count = 0;
    vx_nn_cmd_split_info_u * sinfoArray;
    vx_uint32 x_sizes[MAX_NN_SPLIT_XY_NUM], x_offsets[MAX_NN_SPLIT_XY_NUM], y_sizes[MAX_NN_SPLIT_XY_NUM], y_offsets[MAX_NN_SPLIT_XY_NUM], z_offsets[MAX_NN_SPLIT_XY_NUM], z_size[MAX_NN_SPLIT_XY_NUM];
    vx_enum inDataFormat = input->dataFormat;
    vx_enum outDataFormat = output->dataFormat;

    wb = (vx_weights_biases_parameter)(conv_cmd_ptr->other_ref);
    vxmASSERT(sinfo_num_ptr != VX_NULL && sinfo_array_ptr != VX_NULL);

    if(wb)
    {

        zcount = wb->slice_z_num != 0 ? wb->slice_z_num : 1;
        z_offsets[0]=0;
        for(k = 0; k < wb->slice_z_num; ++k)
        {
            z_size[k] = wb->slice_array[k].z_count;
            z_offsets[k+1] = z_offsets[k] + wb->slice_array[k].z_count;
        }
    }
    else
        z_size[0] = output->depth;


    vxmASSERT(z_offsets[wb->slice_z_num] == output->depth);

    count = xcount * ycount * zcount;
    sinfoArray = vxAllocateAndZeroMemory(sizeof(vx_nn_cmd_split_info_u) * count);
    if (sinfoArray == VX_NULL) return VX_ERROR_NO_MEMORY;


    if (input->brickMode)
    {
        inImageXOffsetOrig  = 0;
        inImageYOffsetOrig  = 0;
    }
    else
    {
        inImageXOffsetOrig  = (-1) * conv_cmd_ptr->pad_x_left;
        inImageYOffsetOrig  = (-1) * conv_cmd_ptr->pad_y_top;

        switch (context->nnConfig.fixedFeature.nnInImageOffsetBits)
        {
        case 5:
            /* 5-bit XYOffset: [-16, 15]. */
            gcmASSERT((inImageXOffsetOrig >= -16 && inImageXOffsetOrig <= 15) &&
                      (inImageYOffsetOrig >= -16 && inImageYOffsetOrig <= 15));
            break;
        case 4:
            /* 4-bit XYOffset: [-8, 7]. */
            vxmASSERT((inImageXOffsetOrig >= -8 && inImageXOffsetOrig <= 7) &&
                      (inImageYOffsetOrig >= -8 && inImageYOffsetOrig <= 7));
            break;
        case 3:
            /* 3-bit XYOffset: [-4, 3]. */
            vxmASSERT((inImageXOffsetOrig >= -4 && inImageXOffsetOrig <= 3) &&
                      (inImageYOffsetOrig >= -4 && inImageYOffsetOrig <= 3));
            break;
        default:
            vxError("Invalid nnInImageOffsetBits: %u.\n", context->nnConfig.fixedFeature.nnInImageOffsetBits);
            vxmASSERT(0);
            break;
        }
    }

    calculateSplitSize(output->width, xcount, x_sizes, x_offsets);
    calculateSplitSize(output->height, ycount, y_sizes, y_offsets);

    for(k = 0; k < zcount; k++)
    {
        for(j = 0; j < ycount; j++)
        {
            vx_uint32 i_y_size = 0, y_size = 0, i_y_offset = 0, o_y_offset = 0;

            y_size = y_sizes[j];
            o_y_offset = y_offsets[j];

            if (getHwPoolingType(conv_cmd_ptr->pool_type) == VIV_NN_POOLING_NON)
            {
                i_y_size = y_size - 1 + conv_cmd_ptr->kernel_y;
            }
            else
            {
                i_y_size = ((y_size - 1) * conv_cmd_ptr->pool_stride + getHwPoolingSze(conv_cmd_ptr->pool_size_y)) - 1 + conv_cmd_ptr->kernel_y;
            }

            if (j > 0)
            {
                i_y_offset = inImageYOffsetOrig ? gcmMAX(0, (vx_int32)(o_y_offset + inImageYOffsetOrig)) : o_y_offset;
            }

            if (j == ycount - 1)
            {
                i_y_size = inImageYSizeOrig - i_y_offset;
                y_size = outImageYSizeOrig - o_y_offset;
            }

            for(i = 0; i < xcount; i++)
            {
                vx_uint32 i_x_size = 0, x_size = 0, i_x_offset = 0, o_x_offset = 0;
                x_size = x_sizes[i];
                o_x_offset = x_offsets[i];

                if (getHwPoolingType(conv_cmd_ptr->pool_type) == VIV_NN_POOLING_NON)
                {
                    i_x_size = x_size - 1 + conv_cmd_ptr->kernel_x;
                }
                else
                {
                    i_x_size = ((x_size - 1) * conv_cmd_ptr->pool_stride + getHwPoolingSze(conv_cmd_ptr->pool_size_x)) - 1 + conv_cmd_ptr->kernel_x;
                }

                 if (i > 0)
                 {
                    i_x_offset = inImageXOffsetOrig ? gcmMAX(0, (vx_int32)(o_x_offset + inImageXOffsetOrig)) : o_x_offset;
                 }

                 if (i == xcount - 1)
                 {
                        i_x_size = inImageXSizeOrig - i_x_offset;
                        x_size = outImageXSizeOrig - o_x_offset;
                 }

                index = i + j * xcount + k * xcount * ycount;
                sinfoArray[index].vx_nn_general_cmd_split_info.inImageXSize    = i == xcount - 1 ? i_x_size : i_x_size + inImageXOffsetOrig;
                sinfoArray[index].vx_nn_general_cmd_split_info.inImageYSize    = j == ycount - 1 ? i_y_size : i_y_size + inImageYOffsetOrig;
                sinfoArray[index].vx_nn_general_cmd_split_info.inImageXOffset  = gcmMIN(0, (vx_int32)(o_x_offset + inImageXOffsetOrig));
                sinfoArray[index].vx_nn_general_cmd_split_info.inImageYOffset  = gcmMIN(0, (vx_int32)(o_y_offset + inImageYOffsetOrig));
                sinfoArray[index].vx_nn_general_cmd_split_info.outImageXSize   = x_size;
                sinfoArray[index].vx_nn_general_cmd_split_info.outImageYSize   = y_size;
                sinfoArray[index].vx_nn_general_cmd_split_info.outImageZSize   = z_size[k];

                if(k == 0 && j == 0 && i == 0)
                {
                    sinfoArray[0].vx_nn_general_cmd_split_info.inImageAddress  = input->physical.start;
                    sinfoArray[0].vx_nn_general_cmd_split_info.outImageAddress = output->physical.start;
                    sinfoArray[0].vx_nn_general_cmd_split_info.kernelAddress   = (vx_uint32)WB_MEM_PHYSICAL_ADDR_INDEX(wb, 0);
                }
                else if(k > 0 && j ==0 && i == 0)
                {
                    sinfoArray[index].vx_nn_general_cmd_split_info.inImageAddress  = sinfoArray[index - ycount * xcount].vx_nn_general_cmd_split_info.inImageAddress;
                    sinfoArray[index].vx_nn_general_cmd_split_info.outImageAddress = sinfoArray[index - ycount * xcount].vx_nn_general_cmd_split_info.outImageAddress + z_size[k] * vxnneGetTypeSize(outDataFormat);
                    sinfoArray[index].vx_nn_general_cmd_split_info.kernelAddress   = (vx_uint32)WB_MEM_PHYSICAL_ADDR_INDEX(wb, index);
                }
                else if(j > 0 && i == 0)
                {
                    sinfoArray[index].vx_nn_general_cmd_split_info.inImageAddress  = sinfoArray[index - xcount].vx_nn_general_cmd_split_info.inImageAddress + i_y_offset * input->yStride;
                    sinfoArray[index].vx_nn_general_cmd_split_info.outImageAddress = sinfoArray[index - xcount].vx_nn_general_cmd_split_info.outImageAddress + o_y_offset * output->yStride;
                    sinfoArray[index].vx_nn_general_cmd_split_info.kernelAddress   = sinfoArray[index - 1].vx_nn_general_cmd_split_info.kernelAddress;
                }
                else
                {
                    sinfoArray[index].vx_nn_general_cmd_split_info.inImageAddress  = sinfoArray[index - 1].vx_nn_general_cmd_split_info.inImageAddress + i_x_offset * vxnneGetTypeSize(inDataFormat);
                    sinfoArray[index].vx_nn_general_cmd_split_info.outImageAddress = sinfoArray[index - 1].vx_nn_general_cmd_split_info.outImageAddress + o_x_offset * vxnneGetTypeSize(outDataFormat);
                    sinfoArray[index].vx_nn_general_cmd_split_info.kernelAddress   = sinfoArray[index - 1].vx_nn_general_cmd_split_info.kernelAddress;
                }

                vxmASSERT(sinfoArray[index].vx_nn_general_cmd_split_info.outImageXSize <= NN_IMAGE_XSIZE_MAX);
            }
            vxmASSERT(sinfoArray[index].vx_nn_general_cmd_split_info.outImageYSize <= NN_IMAGE_YSIZE_MAX);
        }
    }

    *sinfo_array_ptr = sinfoArray;
    *sinfo_num_ptr = count;

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxnneCommandBuffer_GetNNGeneralCommandInfo(
    vx_context                   context,
    vxnne_tensor_info            input,
    vxnne_tensor_info            output,
    vx_op_param                  conv_cmd_ptr,
    vx_nn_cmd_info_u *           info
    )
{
    vx_uint32 inImageTileSizeX, inImageTileSizeY, outImageTileSizeX, outImageTileSizeY;
    vx_uint32 kernelX, kernelY, kernelZ, kernelsPerCore, nnCoreCount;
    vx_uint32 outZSize;
    vx_enum inDataFormat, inQuantFormat, outDataFormat, outQuantFormat, outRMode;
    vx_int8 inFPP, outFPP;
    vx_int32 inZP, outZP;
    vx_float32 inScale, outScale;
    vx_weights_biases_parameter weights_biases = (vx_weights_biases_parameter) conv_cmd_ptr->other_ref;
    vx_bool isV8 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP0);

    outZSize = output->depth;
    inDataFormat   = input->dataFormat;
    inQuantFormat  = input->quantFormat;
    outDataFormat  = output->dataFormat;
    outQuantFormat = output->quantFormat;
    inFPP  = input->fixedPointPos;
    outFPP = output->fixedPointPos;
    outRMode = output->roundingMode;
    inScale  = input->scale;
    outScale = output->scale;
    inZP  = input->zeroPoint;
    outZP = output->zeroPoint;

    kernelX = conv_cmd_ptr->kernel_x;
    kernelY = conv_cmd_ptr->kernel_y;
    kernelZ = conv_cmd_ptr->kernel_z;
    outImageTileSizeX  = conv_cmd_ptr->out_image_tile_x;
    outImageTileSizeY  = conv_cmd_ptr->out_image_tile_y;
    kernelsPerCore     = conv_cmd_ptr->kernels_per_core;
    nnCoreCount        = conv_cmd_ptr->nnCoreCount;

    info->vx_nn_general_cmd_info.kernelXSize       = kernelX;
    info->vx_nn_general_cmd_info.kernelYSize       = kernelY;
    info->vx_nn_general_cmd_info.kernelZSize       = kernelZ;
    info->vx_nn_general_cmd_info.kernelsPerCore    = kernelsPerCore;
    info->vx_nn_general_cmd_info.kernelAddress     = (vx_uint32)WB_MEM_PHYSICAL_ADDR_INDEX(weights_biases, 0);

    info->vx_nn_general_cmd_info.pooling           = getHwPoolingType(conv_cmd_ptr->pool_type);
    info->vx_nn_general_cmd_info.poolingXYSize     = getHwPoolingSze(conv_cmd_ptr->pool_size_x);
    info->vx_nn_general_cmd_info.relu              = conv_cmd_ptr->enable_relu;
    info->vx_nn_general_cmd_info.outImageZSize     = outZSize;

    info->vx_nn_general_cmd_info.hwDepthWise       = weights_biases->wb_base->hw_depth_wise ? 1 : 0;
    info->vx_nn_general_cmd_info.noZOffset         = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_NO_Z_LOCATION_OFFSET);
    info->vx_nn_general_cmd_info.pRelu             = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PRELU);
    info->vx_nn_general_cmd_info.perChannelPostMul = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PER_CHANNEL_POST_MULTIPLY);

    /* add assert for hw limitation */
    vxmASSERT(info->vx_nn_general_cmd_info.outImageZSize <= NN_IMAGE_ZSIZE_MAX);

    info->vx_nn_general_cmd_info.inImageXstride = input->yStride;
    info->vx_nn_general_cmd_info.inImageYstride = input->zStride / input->yStride;

    info->vx_nn_general_cmd_info.outImageXstride = output->yStride;
    info->vx_nn_general_cmd_info.outImageYstride = output->zStride / output->yStride;

    if (output->sRAM)
    {
        info->vx_nn_general_cmd_info.outImageCircularBufSize = output->circleBufferSize;
        info->vx_nn_general_cmd_info.outImageCircularBufEndAddrPlus1 = output->physical.circularBufEndAddrPlus1;
    }
    else
    {
        info->vx_nn_general_cmd_info.outImageCircularBufSize         = 0;
        info->vx_nn_general_cmd_info.outImageCircularBufEndAddrPlus1 = 0xFFFFFFFF;
    }

    if (input->sRAM)
    {
        /*inImageCircularBufSize*/
        info->vx_nn_general_cmd_info.inImageCircularBufSize = input->circleBufferSize;
        /*inImageCircularBufEndAddrPlus1*/
        info->vx_nn_general_cmd_info.inImageCircularBufEndAddrPlus1  = input->physical.circularBufEndAddrPlus1;
    }
    else
    {
        /*inImageCircularBufSize*/
        info->vx_nn_general_cmd_info.inImageCircularBufSize   = 0;
        /*inImageCircularBufEndAddrPlus1*/
        info->vx_nn_general_cmd_info.inImageCircularBufEndAddrPlus1     = 0xFFFFFFFF;
    }

    info->vx_nn_general_cmd_info.noFlush           = output->flush ? 0 : 1;

    info->vx_nn_general_cmd_info.outImageTileXSize = outImageTileSizeX;
    info->vx_nn_general_cmd_info.outImageTileYSize = outImageTileSizeY;

    /* Fixed same type in this stage. */
    info->vx_nn_general_cmd_info.kernelDataType    = getHWDataFormat(WB_WEIGHT_DATA_FORMAT(weights_biases));
    info->vx_nn_general_cmd_info.inImageDataType   = getHWDataFormat(inDataFormat);
    info->vx_nn_general_cmd_info.outImageDataType  = getHWDataFormat(outDataFormat);

    /* Since V7 HW WORD1 only has 2 bits for kerenl data type, input data type & output data type,
     * int16 value 0x4 will overflow, need add MSB value to WORD15 to cover
     */
    if (WB_WEIGHT_DATA_FORMAT(weights_biases) == VX_TYPE_INT16 || WB_WEIGHT_DATA_FORMAT(weights_biases) == VX_TYPE_BFLOAT16)
        info->vx_nn_general_cmd_info.kernelDataTypeMsb = 1;

    if (inDataFormat == VX_TYPE_INT16 || inDataFormat == VX_TYPE_BFLOAT16)
        info->vx_nn_general_cmd_info.inImageDataTypeMsb = 1;

    if (outDataFormat == VX_TYPE_INT16 || outDataFormat == VX_TYPE_BFLOAT16)
        info->vx_nn_general_cmd_info.outImageDataTypeMsb = 1;

    vxmASSERT(info->vx_nn_general_cmd_info.inImageDataType == info->vx_nn_general_cmd_info.kernelDataType);

    info->vx_nn_general_cmd_info.postMultiplier = 0;
    info->vx_nn_general_cmd_info.roundingMode   = getHWRoundingMode((vx_nn_round_mode_e)outRMode, outDataFormat, vx_false_e);


    info->vx_nn_general_cmd_info.bFloat16Mode = (WB_WEIGHT_DATA_FORMAT(weights_biases) == VX_TYPE_BFLOAT16) ? 1 : 0 ;

    if(info->vx_nn_general_cmd_info.bFloat16Mode)
    {
        vxmASSERT(info->vx_nn_general_cmd_info.kernelDataType == 0x7 &&
                  info->vx_nn_general_cmd_info.inImageDataType == 0x7 &&
                  info->vx_nn_general_cmd_info.outImageDataType == 0x7 &&
                  WB_BIAS_DATA_FORMAT(weights_biases) == VX_TYPE_FLOAT32);
    }

    if (((inDataFormat == VX_TYPE_UINT8) && (inQuantFormat == VX_QUANT_AFFINE_SCALE)) ||
        ((WB_WEIGHT_DATA_FORMAT(weights_biases) == VX_TYPE_UINT8) && (WB_WEIGHT_QUANT_FORMAT(weights_biases) == VX_QUANT_AFFINE_SCALE)) ||
        ((outDataFormat == VX_TYPE_UINT8) && (outQuantFormat == VX_QUANT_AFFINE_SCALE)))
    {
        vx_float32 scale;
        vx_uint32 uintScale;
        vx_uint32 tmpMultiply;
        vx_int32 exp;
        vx_int8 tmpPostShift;
        vx_float32 wScale = WB_WEIGHT_SCALE(weights_biases);
        vx_float32 bScale = WB_BIAS_SCALE(weights_biases);

        if (inQuantFormat  == VX_QUANT_DYNAMIC_FIXED_POINT)
        {
            inScale = vxnneConvertDynamicFixPointValueToFloat32(1.0, inFPP);
        }

        if (WB_WEIGHT_QUANT_FORMAT(weights_biases) == VX_QUANT_DYNAMIC_FIXED_POINT)
        {
            wScale = vxnneConvertDynamicFixPointValueToFloat32(1.0, WB_WEIGHT_FPP(weights_biases));
            bScale = vxnneConvertDynamicFixPointValueToFloat32(1.0, WB_BIAS_FPP(weights_biases));
        }

        if (outQuantFormat == VX_QUANT_DYNAMIC_FIXED_POINT)
        {
            outScale = vxnneConvertDynamicFixPointValueToFloat32(1.0, outFPP);
        }

        scale = inScale * wScale / outScale;
        uintScale = *((vx_uint32*)(&scale));
        exp = (uintScale & 0x7F800000) >> 23; /* postShift is Scale's exp*/

        /* HW design follow the paper, biasScale = inputScale * coefScale*/
        if (!weights_biases->wb_base->no_bias)
        {
            vxmASSERT(gcmABS(bScale - inScale * wScale) < 0.000001);
        }

        if (!isV8)
        {
            /*V7 use 15bit to save post-multiply*/
            tmpMultiply = (uintScale & 0x7FFFFF) >> 8; /* postMultiply is high 15-bit of Scale's mantissa*/

            info->vx_nn_general_cmd_info.postMultiplier = tmpMultiply & 1;
            tmpMultiply = tmpMultiply >> 1;
            info->vx_nn_general_cmd_info.postMultiplierBit6to1 = tmpMultiply & 0x3F;
            tmpMultiply = tmpMultiply >> 6;
            info->vx_nn_general_cmd_info.postMultiplierBit14to7 = tmpMultiply;

            tmpPostShift = 15 - ((vx_int8)exp - 127);
            info->vx_nn_general_cmd_info.postShift = tmpPostShift & 0x1F;
            tmpPostShift = tmpPostShift >> 5;
            info->vx_nn_general_cmd_info.postShiftBit6to5 = tmpPostShift & 3;
        }
        else
        {
            /*V8 use 23bit to save post-multiply*/
            tmpMultiply = uintScale & 0x7FFFFF; /* postMultiply is 23-bit of Scale's mantissa*/

            info->vx_nn_general_cmd_info.postMultiplier = tmpMultiply & 1;
            tmpMultiply = tmpMultiply >> 1;
            info->vx_nn_general_cmd_info.postMultiplierBit6to1 = tmpMultiply & 0x3F;
            tmpMultiply = tmpMultiply >> 6;
            info->vx_nn_general_cmd_info.postMultiplierBit14to7 = tmpMultiply & 0xFF;
            tmpMultiply = tmpMultiply >> 8;
            info->vx_nn_general_cmd_info.postMultiplierBit22to15 = tmpMultiply;

            /*V8 post shift no need add 15*/
            tmpPostShift = 127 - (vx_int8)exp;
            info->vx_nn_general_cmd_info.postShift = tmpPostShift & 0x1F;
            tmpPostShift = tmpPostShift >> 5;
            info->vx_nn_general_cmd_info.postShiftBit6to5 = tmpPostShift & 3;
        }
        info->vx_nn_general_cmd_info.coefZP = WB_WEIGHT_ZP(weights_biases);
        info->vx_nn_general_cmd_info.outputZP = outZP;
    }
    else if ((inQuantFormat == VX_QUANT_DYNAMIC_FIXED_POINT) ||
             (WB_WEIGHT_QUANT_FORMAT(weights_biases) == VX_QUANT_DYNAMIC_FIXED_POINT) ||
             (outQuantFormat == VX_QUANT_DYNAMIC_FIXED_POINT))
    {
        vx_int8 tmpPostShift = 0;
        if (WB_BIAS_TENSOR(weights_biases) != VX_NULL)
        {
            /* fl(in) + fl(weights) == fl(bias) */
           vxmASSERT(inFPP + WB_WEIGHT_FPP(weights_biases) == WB_BIAS_FPP(weights_biases));
        }

        if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT) &&
           (WB_WEIGHT_DATA_FORMAT(weights_biases) == VX_TYPE_INT8 && outDataFormat != VX_TYPE_FLOAT16) &&
           !isV8)
        {
            tmpPostShift += 15;
        }

        if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT) || (context->nnConfig.fixedFeature.nnCoreCountInt16 > 0) || isV8)
        {
            tmpPostShift = tmpPostShift + inFPP - outFPP + WB_WEIGHT_FPP(weights_biases);
            info->vx_nn_general_cmd_info.postShift = tmpPostShift & 0x1F;
            tmpPostShift = tmpPostShift >> 5;
            info->vx_nn_general_cmd_info.postShiftBit6to5 = tmpPostShift & 3;
        } else
        {
            info->vx_nn_general_cmd_info.postShift = tmpPostShift + inFPP - outFPP + WB_WEIGHT_FPP(weights_biases);
        }
    }
    else
    {
        info->vx_nn_general_cmd_info.postShift = 0;
    }

    if (isV8)
    {
        /*This bit to select if the output sequence should run at full speed or if it should be limited so that it does one accum buffer read every 3 cycles
        For V8, default enable it unless write to DDR is the bottleneck*/
        info->vx_nn_general_cmd_info.slowOutput = 1;
    }
    else
        info->vx_nn_general_cmd_info.slowOutput = 0;

#if NN_WSZIE_REG
    info->vx_nn_general_cmd_info.wSize = 1;
#endif

#if NN_LAYER_FLUSH
    info->vx_nn_general_cmd_info.nn_layer_flush = 1;
#else
    info->vx_nn_general_cmd_info.nn_layer_flush = 0;
#endif

    inImageTileSizeX = outImageTileSizeX - 1 + kernelX;
    inImageTileSizeY = outImageTileSizeY - 1 + kernelY;

    if (input->brickMode)
    {
        info->vx_nn_general_cmd_info.brickMode = 1;
        info->vx_nn_general_cmd_info.brickDistance = inImageTileSizeX * inImageTileSizeY * kernelZ * vxnneGetTypeSize((vx_type_e)inDataFormat);
    }
    else
    {
        info->vx_nn_general_cmd_info.brickMode = 0;
        info->vx_nn_general_cmd_info.brickDistance = 0;
    }

    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_SRAM))
    {
        if (conv_cmd_ptr->imageCacheMode == VXNNE_SRAM_CACHE_MODE_FULL_CACHE &&
            checkImageCacheMode(info->vx_nn_general_cmd_info.outImageZSize, info->vx_nn_general_cmd_info.kernelsPerCore, nnCoreCount))
        {
            info->vx_nn_general_cmd_info.imageCachingMode    = 1;
            info->vx_nn_general_cmd_info.imageStartAddress   = conv_cmd_ptr->imageCacheStart;
            info->vx_nn_general_cmd_info.imageEndAddress     = conv_cmd_ptr->imageCacheStart + conv_cmd_ptr->imageCacheSize;
            vxmASSERT(conv_cmd_ptr->imageCacheSize != 0);
        }
        else
        {
            info->vx_nn_general_cmd_info.imageCachingMode              = 0;
            info->vx_nn_general_cmd_info.imageStartAddress             = 0;
            info->vx_nn_general_cmd_info.imageEndAddress               = (context->vipSRAM.size <= VX_VIP_SRAM_IMAGE_STREAM_SIZE) ? context->vipSRAM.size : VX_VIP_SRAM_IMAGE_STREAM_SIZE;
            conv_cmd_ptr->imageCacheMode                              = VXNNE_SRAM_CACHE_MODE_NONE;
        }

        if (conv_cmd_ptr->kernelCacheMode == VXNNE_SRAM_CACHE_MODE_STREAM_CACHE)
        {
            info->vx_nn_general_cmd_info.kernelCachingMode = 0;
            info->vx_nn_general_cmd_info.kernelCacheStartAddress = conv_cmd_ptr->kernelCacheStart;
            info->vx_nn_general_cmd_info.kernelCacheEndAddress   = conv_cmd_ptr->kernelCacheStart + conv_cmd_ptr->kernelCacheSize;
            info->vx_nn_general_cmd_info.kernelDirectStreamFromVipSram = 1;
            vxmASSERT(info->vx_nn_general_cmd_info.kernelCacheStartAddress && info->vx_nn_general_cmd_info.kernelCacheEndAddress);
        }
        else if (conv_cmd_ptr->kernelCacheMode == VXNNE_SRAM_CACHE_MODE_FULL_CACHE)
        {
            info->vx_nn_general_cmd_info.kernelCachingMode = 1;
            info->vx_nn_general_cmd_info.kernelCacheStartAddress = conv_cmd_ptr->kernelCacheStart;
            info->vx_nn_general_cmd_info.kernelCacheEndAddress   = conv_cmd_ptr->kernelCacheStart + conv_cmd_ptr->kernelCacheSize;
        }
        else if (conv_cmd_ptr->kernelCacheMode == VXNNE_SRAM_CACHE_MODE_PARTIAL_CACHE)
        {
            vx_uint32 kernelStreamAlignSize = (vx_uint32)gcmALIGN_NP2(WB_CONV_STREAM_ALIGN_SIZE(weights_biases), CACHE_ALIGNMENT_SIZE);

            vx_float32 ratio = (vx_float32)conv_cmd_ptr->kernelCacheSize / (kernelStreamAlignSize - conv_cmd_ptr->kernelCacheSize);

            vx_uint64 kernelPatternBits = 0;
            vx_uint32 dataUnitByte = 64;
            vx_uint32 zeroNum = 0;
            vx_uint32 oneNum = 0;
            vx_uint32 vipSramNeed = 0;
            vx_uint32 partialCacheDataUnit = NN_KS_PARTIAL_CACHE_DATA_UNIT;

            switch (partialCacheDataUnit)
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

            vxmASSERT(kernelStreamAlignSize - conv_cmd_ptr->kernelCacheSize > 0);

            vxnneGetPatternBitAndVipSramSizeNeed(ratio,
                conv_cmd_ptr->kernelCacheSize,
                kernelStreamAlignSize,
                dataUnitByte,
                &oneNum,
                &zeroNum,
                &kernelPatternBits,
                &vipSramNeed
                );

            if (vipSramNeed == 0)
            {
                conv_cmd_ptr->kernelCacheMode = VXNNE_SRAM_CACHE_MODE_NONE;

                info->vx_nn_general_cmd_info.kernelCachingMode = 0;
                info->vx_nn_general_cmd_info.partialCacheDataUnit = 0;
                info->vx_nn_general_cmd_info.kernelPatternMsb = 0;
                info->vx_nn_general_cmd_info.kernelPatternLow32Bits = 0;
                info->vx_nn_general_cmd_info.kernelPatternHigh32Bits = 0;
                info->vx_nn_general_cmd_info.kernelCacheStartAddress = 0;
                info->vx_nn_general_cmd_info.kernelCacheEndAddress = 0;
            }
            else
            {

                vxmASSERT((oneNum + zeroNum) <= VX_KERNEL_PATTERN_BIT_SIZE);
                vxmASSERT((oneNum / zeroNum) <= ratio);

                info->vx_nn_general_cmd_info.kernelPatternMsb        = (oneNum + zeroNum) - 1;
                info->vx_nn_general_cmd_info.kernelPatternLow32Bits  = kernelPatternBits & 0xFFFFFFFF;
                info->vx_nn_general_cmd_info.kernelPatternHigh32Bits = kernelPatternBits >> 32;

                info->vx_nn_general_cmd_info.kernelCachingMode       = 2;
                info->vx_nn_general_cmd_info.partialCacheDataUnit    = partialCacheDataUnit;

                info->vx_nn_general_cmd_info.kernelCacheStartAddress = conv_cmd_ptr->kernelCacheStart;
                info->vx_nn_general_cmd_info.kernelCacheEndAddress = info->vx_nn_general_cmd_info.kernelCacheStartAddress + vipSramNeed;

                vxmASSERT(info->vx_nn_general_cmd_info.kernelCacheEndAddress > info->vx_nn_general_cmd_info.kernelCacheStartAddress);
                vxmASSERT((info->vx_nn_general_cmd_info.kernelCacheEndAddress - info->vx_nn_general_cmd_info.kernelCacheStartAddress) <= conv_cmd_ptr->kernelCacheSize);
            }

            _checkSramOverflow(context, weights_biases, info, dataUnitByte);
        }
        else
        {
            vxmASSERT(conv_cmd_ptr->kernelCacheMode == VXNNE_SRAM_CACHE_MODE_NONE);

            info->vx_nn_general_cmd_info.kernelCachingMode = 0;
            info->vx_nn_general_cmd_info.partialCacheDataUnit = 0;
            info->vx_nn_general_cmd_info.kernelPatternMsb = 0;
            info->vx_nn_general_cmd_info.kernelPatternLow32Bits = 0;
            info->vx_nn_general_cmd_info.kernelPatternHigh32Bits = 0;
            info->vx_nn_general_cmd_info.kernelCacheStartAddress = 0;
            info->vx_nn_general_cmd_info.kernelCacheEndAddress = 0;
        }

        if (context->options.enableSramStreamMode)  /* test for image stream mode and kernel stream mode */
        {
            info->vx_nn_general_cmd_info.imageCachingMode = 0;
            info->vx_nn_general_cmd_info.imageStartAddress = 0;
            info->vx_nn_general_cmd_info.imageEndAddress = 2048;

            info->vx_nn_general_cmd_info.kernelCachingMode = 0;
            info->vx_nn_general_cmd_info.partialCacheDataUnit = 0;
            info->vx_nn_general_cmd_info.kernelPatternMsb = 0;
            info->vx_nn_general_cmd_info.kernelPatternLow32Bits = 0;
            info->vx_nn_general_cmd_info.kernelPatternHigh32Bits = 0;
            info->vx_nn_general_cmd_info.kernelCacheStartAddress = 0;
            info->vx_nn_general_cmd_info.kernelCacheEndAddress = 0;
        }

        vxmASSERT(!(info->vx_nn_general_cmd_info.imageStartAddress & (CACHE_ALIGNMENT_SIZE - 1)));
        vxmASSERT(!(info->vx_nn_general_cmd_info.imageEndAddress & (CACHE_ALIGNMENT_SIZE - 1)));
        vxmASSERT(!(info->vx_nn_general_cmd_info.kernelCacheStartAddress & (CACHE_ALIGNMENT_SIZE - 1)));
        vxmASSERT(conv_cmd_ptr->kernelCacheMode == VXNNE_SRAM_CACHE_MODE_PARTIAL_CACHE || !(info->vx_nn_general_cmd_info.kernelCacheEndAddress & (CACHE_ALIGNMENT_SIZE - 1)));
    }

    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_BORDER_MODE))
    {
        info->vx_nn_general_cmd_info.inImageBorderMode = getHWBorderMode(conv_cmd_ptr->pad_mode, gcvVX_ACCELERATOR_NN);
        info->vx_nn_general_cmd_info.inImageBorderConst = (inDataFormat == VX_TYPE_INT8) ?
                                                            (vx_int8)conv_cmd_ptr->pad_const : (vx_int16)conv_cmd_ptr->pad_const;
    }

    info->vx_nn_general_cmd_info.inImageBorderMode = 0x0;
    if ((inQuantFormat == VX_QUANT_AFFINE_SCALE) && (inDataFormat == VX_TYPE_UINT8))
    {
        info->vx_nn_general_cmd_info.inImageBorderConst = inZP;
    }
    else
    {
        info->vx_nn_general_cmd_info.inImageBorderConst = 0;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API void
_configTPBrickMode(
    vx_weights_biases_parameter weights_biases,
    vxnne_tensor_info input,
    vxnne_tensor_info output,
    vx_arch_perf perf,
    vx_uint32 index,
    vx_bool multi_tp,
    vx_nn_cmd_info_u *info,
    vx_bool last
    )
{
    vx_context context;
    vx_uint32 tpCoreCount;

    vx_uint32 inXSize, inYSize, inZSize;
    vx_uint32 inputBase, outputBase;
    vx_uint32 inWindowYEnd;

    vx_uint32 outTileXSize, outTileYSize;
    vx_uint32 outLeftTileXSize, outRightTileXSize;
    vx_uint32 outUpperTileYSize, outLowerTileYSize;

    vx_uint32 outTilePerSlice, outTileXNum, outTileYNum;
    vx_uint32 outTileSliceUL, outTileSliceUC, outTileSliceUR;
    vx_uint32 outTileSliceCL, outTileSliceCC, outTileSliceCR;
    vx_uint32 outTileSliceLL, outTileSliceLC, outTileSliceLR;
    vx_uint32 sliceNum, outTileYNumInSlice;
    vx_uint32 outImageTileXSize, outImageTileYSize;

    vx_uint32 brickDistSize;

    vx_uint32 strideXSize, strideYSize;

    vx_uint32 kernelXSize = WB_KERNEL_X(weights_biases);
    vx_uint32 kernelYSize = WB_KERNEL_Y(weights_biases);

    gcmASSERT(info != VX_NULL);

    context = vxGetContext((vx_reference)weights_biases);

    tpCoreCount = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_MULTI_TP) ?
                  context->nnConfig.fixedFeature.tpCoreCount : 1;

    inXSize = input->width;
    inYSize = input->height;
    inZSize = input->depth;

    inputBase = outputBase = 0;

    strideXSize = WB_STRIDE_X(weights_biases);
    strideYSize = WB_STRIDE_Y(weights_biases);

    outImageTileXSize = perf->resultInfo.outImageTileXSize;
    outImageTileYSize = perf->resultInfo.outImageTileYSize;

    outTileXSize = outImageTileXSize + kernelXSize - 1;
    outTileYSize = outImageTileYSize + kernelYSize - 1;

    /* The edege regions should be changed according to the in-image size. */
    outTileXNum = (perf->info.outx + outImageTileXSize - 1) / outImageTileXSize;
    outTileYNum = (perf->info.outy + outImageTileYSize - 1) / outImageTileYSize;

    sliceNum = gcmMIN(outTileYNum, tpCoreCount);

    outTileYNumInSlice = outTileYNum / sliceNum;
    if ((outTileYNum % sliceNum) && (index < (outTileYNum % sliceNum)))
    {
        outTileYNumInSlice += 1;
    }

    if ((outTileXNum == 1) && (perf->info.outx % outImageTileXSize))
    {
        outLeftTileXSize = perf->info.outx % outImageTileXSize + kernelXSize - 1;
    }
    else
    {
        outLeftTileXSize = outTileXSize;
    }

    outRightTileXSize = perf->info.outx % outImageTileXSize ?
                        perf->info.outx % outImageTileXSize + kernelXSize - 1 : outTileXSize;

    if ((last && outTileYNumInSlice == 1) &&
        (perf->info.outy % outImageTileYSize))
    {
        outUpperTileYSize = perf->info.outy % outImageTileYSize + kernelYSize - 1;
    }
    else
    {
        outUpperTileYSize = outTileYSize;
    }

    if (last && (perf->info.outy % outImageTileYSize))
    {
        outLowerTileYSize = perf->info.outy % outImageTileYSize + kernelYSize - 1;
    }
    else
    {
        outLowerTileYSize = outTileYSize;
    }

    outTileSliceUL = outUpperTileYSize * outLeftTileXSize;
    outTileSliceUC = outUpperTileYSize * outTileXSize;
    outTileSliceUR = outUpperTileYSize * outRightTileXSize;
    outTileSliceCL = outTileYSize * outLeftTileXSize;
    outTileSliceCC = outTileYSize * outTileXSize;
    outTileSliceCR = outTileYSize * outRightTileXSize;
    outTileSliceLL = outLowerTileYSize * outLeftTileXSize;
    outTileSliceLC = outLowerTileYSize * outTileXSize;
    outTileSliceLR = outLowerTileYSize * outRightTileXSize;

    brickDistSize = outTileXSize * outTileYSize * inZSize * strideXSize * strideYSize;
    outTilePerSlice = outTileXNum * outTileYNumInSlice;

    /* Inputs. */
    info->vx_nn_tp_cmd_info.inImageXSize  = inXSize;
    info->vx_nn_tp_cmd_info.inImageYSize  = inYSize;
    info->vx_nn_tp_cmd_info.inImageZSize  = inZSize;
    info->vx_nn_tp_cmd_info.inImageStride = inXSize;
    info->vx_nn_tp_cmd_info.inImageSlice  = inXSize * inYSize;

    info->vx_nn_tp_cmd_info.inTileSequence      = 0x0;
    info->vx_nn_tp_cmd_info.inImageGlobalMem    = 1;
    info->vx_nn_tp_cmd_info.inImageBaseAddress  = inputBase;

    /* Output paddings in bricks. */
    info->vx_nn_tp_cmd_info.inTileXSize = outTileXSize * strideXSize;
    info->vx_nn_tp_cmd_info.inTileYSize = outTileYSize * strideYSize;
    info->vx_nn_tp_cmd_info.inTileXInc  = (outTileXSize - (kernelXSize - 1)) * strideXSize;
    info->vx_nn_tp_cmd_info.inTileYInc  = (outTileYSize - (kernelYSize - 1)) * strideXSize;

    info->vx_nn_tp_cmd_info.inWindowXStart = (-1) * (vx_int32)WB_PAD_LEFT(weights_biases);
    info->vx_nn_tp_cmd_info.inWindowXEnd   = info->vx_nn_tp_cmd_info.inWindowXStart + WB_PAD_LEFT(weights_biases) + inXSize + WB_PAD_RIGHT(weights_biases) - 1;
    info->vx_nn_tp_cmd_info.inWindowYStart = (-1) * (vx_int32)WB_PAD_TOP(weights_biases) + (index * (outTileYNum / sliceNum) + gcmMIN(index, outTileYNum % sliceNum)) * info->vx_nn_tp_cmd_info.inTileYInc;
    inWindowYEnd = info->vx_nn_tp_cmd_info.inWindowYStart + (outTileYNumInSlice - 1) * info->vx_nn_tp_cmd_info.inTileYInc + info->vx_nn_tp_cmd_info.inTileYSize - 1;
    info->vx_nn_tp_cmd_info.inWindowYEnd = gcmMIN(inWindowYEnd, inYSize + WB_PAD_BOTTOM(weights_biases) - 1);

    /* Outputs. */
    info->vx_nn_tp_cmd_info.outBaseAddress = outputBase + (index * (outTileYNum / sliceNum) + gcmMIN(index, outTileYNum % sliceNum)) * outTileXNum * brickDistSize;
    info->vx_nn_tp_cmd_info.outGlobalMem   = 1;

    info->vx_nn_tp_cmd_info.outLoop0Inc   = (outTileSliceUC << 16) | outTileSliceUL;
    info->vx_nn_tp_cmd_info.outLoop0Count = strideXSize;
    info->vx_nn_tp_cmd_info.outLoop1Inc   = (outTileSliceCL << 16) | outTileSliceUR;
    info->vx_nn_tp_cmd_info.outLoop1Count = 0;
    info->vx_nn_tp_cmd_info.outLoop1Reset = 1;
    info->vx_nn_tp_cmd_info.outLoop2Inc   = (outTileSliceCR << 16) | outTileSliceCC;
    info->vx_nn_tp_cmd_info.outLoop2Count = strideYSize;
    info->vx_nn_tp_cmd_info.outLoop2Reset = 0;
    info->vx_nn_tp_cmd_info.outLoop3Inc   = (outTileSliceLC << 16) | outTileSliceLL;
    info->vx_nn_tp_cmd_info.outLoop3Count = 0;
    info->vx_nn_tp_cmd_info.outLoop3Reset = 1;
    info->vx_nn_tp_cmd_info.outLoop4Inc   = brickDistSize;
    info->vx_nn_tp_cmd_info.outLoop4Count = outTilePerSlice;
    info->vx_nn_tp_cmd_info.outLoop5Inc   = outTileSliceLR;
    info->vx_nn_tp_cmd_info.outLoop5Count = 1;
    info->vx_nn_tp_cmd_info.outLoop6Inc   = 0;

    info->vx_nn_tp_cmd_info.aluFilterPwlSwap = 0;
    info->vx_nn_tp_cmd_info.aluPwlSignSupport = 0;
    info->vx_nn_tp_cmd_info.aluReluEnable = 0;
    info->vx_nn_tp_cmd_info.aluI2FEnable = 0;
    info->vx_nn_tp_cmd_info.aluF2IEnable = 0;

    info->vx_nn_tp_cmd_info.outBrickMode  = 1;
}

#define TP_MAX_XYSIZE (0x1<<16)
#define MULTI_TP_RESHUFFLE_SIZE 100

enum
{
    TP_SPLIT_Z_DIRECTION,
    TP_SPLIT_X_DIRECTION,
    TP_SPLIT_Y_DIRECTION,
};

VX_PRIVATE_API void _calculateTPSplitSizeOffset(
    vx_context                   context,
    vxnne_tensor_info            input,
    vxnne_tensor_info            output,
    vx_op_param                  parameter,
    vx_enum                      split_type,
    vx_uint32                    *split_count,
    vx_uint32                    split_size_array[],
    vx_uint32                    split_offset_array[]
    )
{
    vx_enum tpType = parameter->tpType;
    vx_tensor otherRef = (vx_tensor)parameter->other_ref;
    vx_tp_value_cmd value = parameter->tp_value;

    vx_bool again = vx_false_e;
    vx_enum split = split_type;
    vx_uint32 slice, size;
    vx_uint32 core = tpType != TP_SINGLE_FC ? context->nnConfig.fixedFeature.tpCoreCount :
                                              context->nnConfig.fixedFeature.tpCoreCount + context->nnConfig.fixedFeature.tpliteCoreCount;
    vx_bool mult = context->options.enableMultiTP && core > 1;

    vx_uint32 inputXSize = input->width;
    vx_uint32 inputYSize = input->height;
    vx_uint32 inputZSize = input->depth;
    vx_uint32 outputXSize = output->width;
    vx_uint32 outputYSize = output->height;
    vx_uint32 outputZSize = output->depth;

    if (split_type == TP_SPLIT_X_DIRECTION)
    {
        size = inputXSize;
    }
    else if (split_type == TP_SPLIT_Y_DIRECTION)
    {
        size = inputYSize;
    }
    else /* TP_SPLIT_Z_DIRECTION */
    {
        size = inputZSize;
    }

    switch (tpType)
    {
        case TP_RESHUFFLE:
        {
            /* if inputXSize, inputYSize and inputZSize is too small, not split */
            if (mult && (inputXSize * inputYSize * inputZSize) > MULTI_TP_RESHUFFLE_SIZE)
            {
                slice = gcoMATH_MIN(inputZSize, core);
            }
            else
            {
                slice = 1;
            }
            break;
        }
        case TP_MAX_POOLING:
        case TP_TENSOR_PAD:
        {
            slice = !mult || size < core ? 1 : core;
            break;
        }

        case TP_ADD:
        case TP_ACTIVATION:
        case TP_TENSOR_COPY:
        case TP_TENSOR_COPY4CONCAT:
        {
            if (!mult || size < core || inputXSize != outputXSize || inputYSize != outputYSize || inputZSize != outputZSize)
            {
                slice = 1;
            }
            else
            {
                slice = gcmMIN(size, core);
            }
            break;
        }

        case TP_LRN:
        {
            vxmASSERT(value != VX_NULL);
            if (value->e32[0] == VX_NN_NORMALIZATION_SAME_MAP)
            {
                if (!mult || size < core || inputXSize != outputXSize || inputYSize != outputYSize || inputZSize != outputZSize)
                {
                    slice = 1;
                }
                else
                {
                    slice = gcmMIN(size, core);
                }
            }
            else /* VX_NN_NORMALIZATION_ACROSS_MAPS */
            {
                if (inputXSize * inputYSize < 65536)
                {
                    size = inputXSize * inputYSize;
                    outputYSize = inputZSize;
                }
                else
                {
                    size = inputXSize;
                    outputXSize = outputZSize;
                }
                slice = !mult ? 1 : gcmMIN(size, core);
            }
            break;
        }

        case TP_REORG:
        {
            outputZSize = inputZSize / value->u32[0] / value->u32[0];
            outputXSize *= value->u32[0];
            outputYSize *= value->u32[0];
            if (split_type == TP_SPLIT_Z_DIRECTION)
            {
                size = outputZSize;
            }
            if (split_type == TP_SPLIT_X_DIRECTION)
            {
                size = outputXSize;
            }
            if (split_type == TP_SPLIT_Y_DIRECTION)
            {
                size = outputYSize;
            }
            slice = !mult || size < core ? 1 : core;
            break;
        }

        case TP_REORG_SPACE2BATCH:
        case TP_REORG_BATCH2SPACE:
        {
            vxmASSERT(value != VX_NULL);
            size = inputZSize * value->u32[2];
            slice = 1;
            break;
        }

        case TP_ROI_POOLING:
        case TP_ROI_POOLING_STEP_1:
        case TP_ROI_POOLING_STEP_2:
        {
            vxmASSERT(value != VX_NULL);
            if (!value->e32[0])
            {
                slice = !mult || size < core ? 1 : core;
                outputXSize = inputXSize;
                outputYSize = value->u32[2];
            }
            else
            {
                slice = !mult ? 1 : gcmMIN(value->u32[6], core);
                size = value->u32[6];
            }
            break;
        }

        case TP_UPSAMPLE:
        case TP_UPSAMPLE_CLIP:
        {
            size = outputZSize;
            slice = !mult || size < core ? 1 : core;
            break;
        }

        case TP_TRANSPOSE:
        {
            vx_uint32 i, x, y, dnum = value->u32[0], dsize;
            vx_uint32 dims[VX_CONTEXT_TENSOR_MAX_DIMENSION], strides[VX_CONTEXT_TENSOR_MAX_DIMENSION];
            vxmASSERT(otherRef != VX_NULL);
            vxoTensor_GetTensorDimStride((vx_tensor)otherRef, &dnum, dims, strides);
            vxmASSERT(dims[0] < TP_MAX_XYSIZE && dims[1] < TP_MAX_XYSIZE);
            dsize = TENSOR_DATA_SIZE((vx_tensor)otherRef);
            x = dims[0];
            y = strides[dnum-1] * dims[dnum-1] / dsize / dims[0];
            for (i = 1; i < dnum; i++)
            {
                if (x >= TP_MAX_XYSIZE || y < TP_MAX_XYSIZE)
                {
                    break;
                }
                else
                {
                    x *= dims[i];
                    y /= dims[i];
                }
            }
            if (x < TP_MAX_XYSIZE && y < TP_MAX_XYSIZE)
            {
                slice = 1;
            }
            else
            {
                for (i = dnum - 1; (vx_int32)i >= 0; i--)
                {
                    if (dims[i] > 1) break;
                }
                size = dims[i];
                /* TP X/Y size has max size limitation, must split */
                y = strides[dnum-1] * dims[dnum-1] / dsize / dims[0];
                slice = gcmALIGN_NP2(y, TP_MAX_XYSIZE-1) / (TP_MAX_XYSIZE-1);
            }
            if (slice == 1 && (dnum == 3 || (dnum == 4 && dims[3] == 1)))
            {
                vx_uint32_ptr perm = (vx_uint32*)value->p8[0];
                if (perm[0] == 1 && perm[1] == 0) /* y, x, z */
                {
                    slice = x * y > 128 ? gcmMIN(dims[2], core) : 1;
                    size = dims[2];
                    value->e32[0] = 1;
                }
                else if (perm[0] == 1 && perm[1] == 2) /* y, z, x */
                {
                    slice = x * y > 128 ? gcmMIN(dims[2], core) : 1;
                    size = dims[2];
                    value->e32[0] = 2;
                }
                if (context->hwChipInfo.customerID == 0xAE)
                {
                    if (perm[0] == 2 && perm[1] == 0) /* z, x, y */
                    {
                        slice = gcmMIN(dims[1], core);
                        size = dims[1];
                        value->e32[0] = 3;
                    }
                    else if (perm[0] == 2 && perm[1] == 1) /* z, y, x */
                    {
                        slice = gcmMIN(dims[1], core);
                        size = dims[1];
                        value->e32[0] = 4;
                    }
                    else if (perm[0] == 0 && perm[1] == 2) /* x, z, y */
                    {
                        slice = gcmMIN(dims[2], core);
                        size = dims[2];
                        value->e32[0] = 5;
                    }
                }
            }
            break;
        }

        case TP_SINGLE_FC:
        {
            if (!value->e32[0])
            {
                vxmASSERT(otherRef != VX_NULL);
                slice = ((vx_weights_biases_parameter)otherRef)->slice_num;
            }
            else
            {
                slice = 1;
            }
            break;
        }

        default:
        {
            slice = 1;
            break;
        }
    }

    calculateSplitSize(size, slice, split_size_array, split_offset_array);

    if (slice != 1 && tpType != TP_SINGLE_FC)
    {
        switch (split)
        {
            case TP_SPLIT_Z_DIRECTION:
                if (split_size_array[slice-1] * outputXSize * outputYSize == 1)
                {
                    slice = 1;
                    again = vx_true_e;
                }
                break;

            case TP_SPLIT_X_DIRECTION:
                if (split_size_array[slice-1] * outputYSize * outputZSize == 1)
                {
                    slice = 1;
                    again = vx_true_e;
                }
                break;

            case TP_SPLIT_Y_DIRECTION:
                if (split_size_array[slice-1] * outputXSize * outputZSize == 1)
                {
                    slice = 1;
                    again = vx_true_e;
                }
                break;
        }
    }

    if (again)
    {
        calculateSplitSize(size, slice, split_size_array, split_offset_array);
    }

    *split_count = slice;
}

#define FILL_TP_COMMAND(context, input, output, param, stype, scount, ssizes, soffsets, sinfoarray, info, NAME) \
    _fill_##NAME##_Command(context, input, output, param, info, stype, scount, ssizes, soffsets, sinfoarray)

#define DEFINE_TP_GENERAL_PARAMETER() \
    vx_uint32 inXSize = input->width; \
    vx_uint32 inYSize = input->height; \
    vx_uint32 inZSize = input->depth; \
    vx_uint32 inYStride = input->yStride; \
    vx_uint32 inZStride = input->zStride; \
    vx_uint32 outXSize = output->width; \
    vx_uint32 outYSize = output->height; \
    vx_uint32 outZSize = output->depth; \
    vx_uint32 outYStride = output->yStride; \
    vx_uint32 outZStride = output->zStride; \
    vx_uint32 inputElemSize  = vxnneGetTypeSize(input->dataFormat); \
    vx_uint32 outputElemSize = vxnneGetTypeSize(output->dataFormat); \
    vx_uint32 inputBase = input->physical.start; \
    vx_uint32 outputBase = output->physical.start; \
    vx_tp_value_cmd value_cmd_ptr = parameter->tp_value; \
    vx_tensor other_tensor = (vx_tensor)parameter->other_ref; \
    vx_tensor data_buff = (vx_tensor)parameter->data_buff; \
    inXSize = inXSize; \
    inYSize = inYSize; \
    inZSize = inZSize; \
    inYStride = inYStride; \
    inZStride = inZStride; \
    outXSize = outXSize; \
    outYSize = outYSize; \
    outZSize = outZSize; \
    outYStride = outYStride; \
    outZStride = outZStride; \
    inputElemSize = inputElemSize; \
    outputElemSize = outputElemSize; \
    inputBase = inputBase; \
    outputBase = outputBase; \
    value_cmd_ptr = value_cmd_ptr; \
    other_tensor = other_tensor; \
    data_buff = data_buff

void _fill_TP_RESHUFFLE_Command(
    vx_context                  context,
    vxnne_tensor_info           input,
    vxnne_tensor_info           output,
    vx_op_param                 parameter,
    vx_nn_cmd_info_u*           general_info,
    vx_enum                     split_type,
    vx_uint32                   split_count,
    vx_uint32                   split_sizes[],
    vx_uint32                   split_offsets[],
    vx_nn_cmd_split_info_u*     info_array
    )
{
    vx_uint32 i;
    vx_weights_biases_parameter weights_biases;
    vx_uint32 strideXSize, strideYSize, outStrideSliceSize, outSliceSize;
    vx_uint32 inXSizeTmp, outSize;
    DEFINE_TP_GENERAL_PARAMETER();

    weights_biases = (vx_weights_biases_parameter)other_tensor;
    strideXSize = WB_STRIDE_X(weights_biases);
    strideYSize = WB_STRIDE_Y(weights_biases);
    outStrideSliceSize = strideXSize * strideYSize;
    outSliceSize = outZStride / outputElemSize;

    inXSizeTmp = outXSize * strideXSize;

    for (i = 0; i < split_count; i++)
    {
        info_array[i].vx_tp_general_cmd_split_info.needReorder = vx_false_e;

        if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP_REORDER) &&
            inXSizeTmp <= context->nnConfig.fixedFeature.tpReorderInImageSize)
        {
            vx_bool disableTPReorder = vx_false_e;

            /* Viv: 1948. */
            if (parameter->pad_x_left || parameter->pad_y_top)
            {
                outSize = outXSize * outYSize * strideXSize * strideYSize * split_sizes[i];
                if (!((outSize - 1) % 3))
                {
                    disableTPReorder = vx_true_e;
                }
            }

            if (!disableTPReorder)
            {
                if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_TP_REORDER_FIX))
                {
                    info_array[i].vx_tp_general_cmd_split_info.needReorder = vx_true_e;
                }
                else if (inXSizeTmp <= inXSize + parameter->pad_x_left ||
                         inXSize + parameter->pad_x_left - inXSizeTmp < strideXSize)
                {
                    inXSize = inXSizeTmp;
                    info_array[i].vx_tp_general_cmd_split_info.needReorder = vx_true_e;
                }
            }
        }

        info_array[i].vx_tp_general_cmd_split_info.inImageXSize = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageYSize = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageZSize = split_sizes[i];
        info_array[i].vx_tp_general_cmd_split_info.inImageStride = inYStride / inputElemSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageSlice = inZStride / inputElemSize;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXStart = (-1) * (vx_int32)parameter->pad_x_left;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYStart = (-1) * (vx_int32)parameter->pad_y_top;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXEnd = outXSize * strideXSize + info_array[i].vx_tp_general_cmd_split_info.inWindowXStart - 1;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYEnd = outYSize * strideYSize + info_array[i].vx_tp_general_cmd_split_info.inWindowYStart - 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileSequence = 0x0;
        info_array[i].vx_tp_general_cmd_split_info.inImageBaseAddress = inputBase + inZStride * split_offsets[i];
        info_array[i].vx_tp_general_cmd_split_info.inTileXSize = (outXSize - 1) * strideXSize + strideXSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileYSize = (outYSize - 1) * strideYSize + strideYSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileXInc = outXSize * strideXSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileYInc = outYSize * strideYSize;
        info_array[i].vx_tp_general_cmd_split_info.outBaseAddress = outputBase + outSliceSize * outStrideSliceSize * split_offsets[i] * outputElemSize;

        if (WB_KERNEL_X(weights_biases) == 1 && WB_KERNEL_Y(weights_biases) == 1 &&
            WB_ORG_KERNEL_X(weights_biases) == 1 && WB_ORG_KERNEL_Y(weights_biases) == 1)
        {
            /* optimize reshuffle for FC layer */
            info_array[i].vx_tp_general_cmd_split_info.outBaseAddress = outputBase + outSliceSize * split_offsets[i] * outputElemSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc   = outSliceSize * outZSize / outStrideSliceSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop0Count = strideXSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc   = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Count = outXSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc   = outSliceSize * outZSize / outStrideSliceSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Count = strideYSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc   = outXSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Count = outYSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc   = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop4Count = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc   = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop5Count = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc   = outSliceSize;
        }
        else
        {
            info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc   = outSliceSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop0Count = strideXSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc   = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Count = outXSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc   = outSliceSize * strideXSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Count = strideYSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc   = outXSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Count = outYSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc   = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop4Count = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc   = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop5Count = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc   = outSliceSize * outStrideSliceSize;
        }

        info_array[i].vx_tp_general_cmd_split_info.noFlush = (i == split_count - 1 ? 0 : 1);
        info_array[i].vx_tp_general_cmd_split_info.last = 1;
    }
}

typedef struct _vxnne_tensor_sub_block {
    vx_uint32 offset_x;
    vx_uint32 offset_y;
    vx_uint32 offset_z;

    vx_uint32 size_x;
    vx_uint32 size_y;
    vx_uint32 size_z;

    vx_uint32  pad_left;
    vx_uint32  pad_top;
    vx_uint32  pad_right;
    vx_uint32  pad_bottom;

    vx_uint32 pitch_x;
    vx_uint32 pitch_y;
}
vxnne_tensor_sub_block_s, *vxnne_tensor_sub_block;

VX_PRIVATE_API void
_fill_TP_RESHUFFLE_Command_EX(
    vx_context                  context,
    vxnne_tensor_info           input,
    vxnne_tensor_info           output,
    vx_op_param                 parameter,
    vx_nn_cmd_info_u*           general_info,
    vx_uint32                   split_count,
    vxnne_tensor_sub_block      input_split_blocks,
    vxnne_tensor_sub_block      output_split_blocks,
    vx_nn_cmd_split_info_u*     info_array
    )
{
    vx_uint32 i;
    vx_weights_biases_parameter weights_biases;
    vx_uint32 outSliceSize;
    vx_uint32 inXSizeTmp, outSize;
    vx_uint32 in_offset_x, in_offset_y, in_offset_z;
    vx_uint32 in_size_x, in_size_y, in_size_z;
    vx_uint32 in_pitch_x, in_pitch_y;
    vx_int32 pad_left, pad_top, pad_right, pad_bottom;
    vx_uint32 stride_x = 1, stride_y = 1;
    vx_uint32 out_offset_x, out_offset_y, out_offset_z;
    vx_uint32 out_size_x, out_size_y, out_size_z;
    vx_uint32 out_pitch_x, out_pitch_y;
    vx_bool optimization_for_1x1_conv = vx_false_e;

    DEFINE_TP_GENERAL_PARAMETER();

    weights_biases = (vx_weights_biases_parameter)other_tensor;
    if (weights_biases)
    {
        stride_x = WB_STRIDE_X(weights_biases);
        stride_y = WB_STRIDE_Y(weights_biases);

        optimization_for_1x1_conv = WB_KERNEL_X(weights_biases) == 1 &&
                                    WB_KERNEL_Y(weights_biases) == 1 &&
                                    WB_ORG_KERNEL_X(weights_biases) == 1 &&
                                    WB_ORG_KERNEL_Y(weights_biases) == 1;
    }

    outSliceSize = outZStride / outputElemSize;

    for (i = 0; i < split_count; i++)
    {
        in_offset_x = input_split_blocks[i].offset_x;
        in_offset_y = input_split_blocks[i].offset_y;
        in_offset_z = input_split_blocks[i].offset_z;

        in_size_x = input_split_blocks[i].size_x;
        in_size_y = input_split_blocks[i].size_y;
        in_size_z = input_split_blocks[i].size_z;

        in_pitch_x = input_split_blocks[i].pitch_x;
        in_pitch_y = input_split_blocks[i].pitch_y;

        pad_left = input_split_blocks[i].pad_left;
        pad_top = input_split_blocks[i].pad_top;
        pad_right = input_split_blocks[i].pad_right;
        pad_bottom = input_split_blocks[i].pad_bottom;

        out_offset_x = output_split_blocks[i].offset_x;
        out_offset_y = output_split_blocks[i].offset_y;
        out_offset_z = output_split_blocks[i].offset_z;

        out_size_x = output_split_blocks[i].size_x;
        out_size_y = output_split_blocks[i].size_y;
        out_size_z = output_split_blocks[i].size_z;

        out_pitch_x = output_split_blocks[i].pitch_x;
        out_pitch_y = output_split_blocks[i].pitch_y;

        inXSizeTmp = out_size_x * stride_x;

        info_array[i].vx_tp_general_cmd_split_info.needReorder = vx_false_e;

        if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP_REORDER) &&
            inXSizeTmp <= context->nnConfig.fixedFeature.tpReorderInImageSize)
        {
            vx_bool disableTPReorder = vx_false_e;

            /* Viv: 1948. */
            if (pad_left || pad_top)
            {
                outSize = out_size_x * out_size_y * out_size_z;
                if (!((outSize - 1) % 3))
                {
                    disableTPReorder = vx_true_e;
                }
            }

            if (!disableTPReorder)
            {
                /* Viv: 1919. */
                if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_TP_REORDER_FIX))
                {
                    info_array[i].vx_tp_general_cmd_split_info.needReorder = vx_true_e;
                }
                else if (inXSizeTmp <= in_size_x + pad_left ||
                         in_size_x + pad_left - inXSizeTmp < stride_x)
                {
                    inXSize = inXSizeTmp;
                    info_array[i].vx_tp_general_cmd_split_info.needReorder = vx_true_e;
                }
            }
        }

        info_array[i].vx_tp_general_cmd_split_info.inImageXSize = in_size_x;
        info_array[i].vx_tp_general_cmd_split_info.inImageYSize = in_size_y;
        info_array[i].vx_tp_general_cmd_split_info.inImageZSize = in_size_z;
        info_array[i].vx_tp_general_cmd_split_info.inImageStride = in_pitch_x;
        info_array[i].vx_tp_general_cmd_split_info.inImageSlice = in_pitch_x * in_pitch_y;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXStart = (-1) * pad_left;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYStart = (-1) * pad_top;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXEnd = out_size_x * stride_x - pad_left - 1;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYEnd = out_size_y * stride_y - pad_top - 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileSequence = 0x0;
        info_array[i].vx_tp_general_cmd_split_info.inImageBaseAddress = inputBase + (in_pitch_x * in_pitch_y * in_offset_z + in_pitch_x * in_offset_y + in_offset_x) * inputElemSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileXSize = out_size_x * stride_x;
        info_array[i].vx_tp_general_cmd_split_info.inTileYSize = out_size_y * stride_y;
        info_array[i].vx_tp_general_cmd_split_info.inTileXInc = out_size_x * stride_x;
        info_array[i].vx_tp_general_cmd_split_info.inTileYInc = out_size_y * stride_y;
        info_array[i].vx_tp_general_cmd_split_info.outBaseAddress = outputBase + (out_pitch_x * out_pitch_y * out_offset_z + out_pitch_x * out_offset_y + out_offset_x) * outputElemSize;

        if (optimization_for_1x1_conv)
        {
            /* Optimize reshuffle for 1x1 conv. */
            info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc   = outSliceSize * outZSize / (stride_x * stride_y);
            info_array[i].vx_tp_general_cmd_split_info.outLoop0Count = stride_x;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc   = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Count = out_size_x;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc   = outSliceSize * outZSize / (stride_x * stride_y);
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Count = stride_y;
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc   = out_size_x;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Count = out_size_y;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc   = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop4Count = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc   = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop5Count = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc   = out_pitch_x * out_pitch_y;
        }
        else
        {
            info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc   = out_pitch_x * out_pitch_y;
            info_array[i].vx_tp_general_cmd_split_info.outLoop0Count = stride_x;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc   = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Count = out_size_x;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc   = out_pitch_x * out_pitch_y * stride_x;
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Count = stride_y;
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc   = out_pitch_x;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Count = out_size_y;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc   = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop4Count = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc   = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop5Count = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc   = out_pitch_x * out_pitch_y * stride_x * stride_y;
        }

        info_array[i].vx_tp_general_cmd_split_info.noFlush = (i == split_count - 1 ? 0 : 1);
        info_array[i].vx_tp_general_cmd_split_info.last = 1;
    }
}

void _fill_TP_SINGLE_FC_Command(
    vx_context                  context,
    vxnne_tensor_info           input,
    vxnne_tensor_info           output,
    vx_op_param                 parameter,
    vx_nn_cmd_info_u*           general_info,
    vx_enum                     split_type,
    vx_uint32                   split_count,
    vx_uint32                   split_sizes[],
    vx_uint32                   split_offsets[],
    vx_nn_cmd_split_info_u*     info_array
    )
{
    vx_uint32 i;
    vx_weights_biases_parameter weights_biases;
    vx_uint32 zOffset, kzOffset, kzOffset2, kzGroup;
    DEFINE_TP_GENERAL_PARAMETER();

    vxmASSERT(value_cmd_ptr != VX_NULL);

    weights_biases = (vx_weights_biases_parameter) other_tensor;

    for (i = 0; i < split_count; i++)
    {
        kzGroup = value_cmd_ptr[i].u32[0];
        if (kzGroup == 1)
        {
            vxmASSERT(WB_IS_USE_TP_FC(weights_biases));

            zOffset = value_cmd_ptr[i].u32[1];
            outZSize = WB_OUTPUT_Z_INDEX(weights_biases, i);

            info_array[i].vx_tp_fc_cmd_split_info.inImageXSize = 1;
            info_array[i].vx_tp_fc_cmd_split_info.inImageYSize = 1;
            info_array[i].vx_tp_fc_cmd_split_info.inImageZSize = WB_KERNEL_Z(weights_biases);
            info_array[i].vx_tp_fc_cmd_split_info.inImageStride = 1;
            info_array[i].vx_tp_fc_cmd_split_info.inImageSlice = 1;
            info_array[i].vx_tp_fc_cmd_split_info.inWindowXStart = 0;
            info_array[i].vx_tp_fc_cmd_split_info.inWindowYStart = 0;
            info_array[i].vx_tp_fc_cmd_split_info.inWindowXEnd = 0;
            info_array[i].vx_tp_fc_cmd_split_info.inWindowYEnd = 0;
            info_array[i].vx_tp_fc_cmd_split_info.inTileSequence = 0x3;
            info_array[i].vx_tp_fc_cmd_split_info.inImageBaseAddress = inputBase;
            info_array[i].vx_tp_fc_cmd_split_info.inTileXSize = 1;
            info_array[i].vx_tp_fc_cmd_split_info.inTileYSize = 1;
            info_array[i].vx_tp_fc_cmd_split_info.inTileXInc = 1;
            info_array[i].vx_tp_fc_cmd_split_info.inTileYInc = 1;
            info_array[i].vx_tp_fc_cmd_split_info.aluHorzProcCount = (outZSize - 1) & 0x3F;    /* Lower 6 bits of FcOutZsizeMinusOne. */
            info_array[i].vx_tp_fc_cmd_split_info.aluVertProcCount = (outZSize - 1) >> 6;      /* Higher 3 bits of FcOutZsizeMinusOne. */
            info_array[i].vx_tp_fc_cmd_split_info.aluLoadPwlLUTAddress = (vx_uint32)WB_MEM_PHYSICAL_ADDR_INDEX(weights_biases, i);
            info_array[i].vx_tp_fc_cmd_split_info.outBaseAddress = outputBase + zOffset * outputElemSize;
            info_array[i].vx_tp_fc_cmd_split_info.outLoop0Inc   = 0;
            info_array[i].vx_tp_fc_cmd_split_info.outLoop0Count = 1;
            info_array[i].vx_tp_fc_cmd_split_info.outLoop1Inc   = 0;
            info_array[i].vx_tp_fc_cmd_split_info.outLoop1Count = 1;
            info_array[i].vx_tp_fc_cmd_split_info.outLoop1Reset = 0x0;
            info_array[i].vx_tp_fc_cmd_split_info.outLoop2Inc   = 0;
            info_array[i].vx_tp_fc_cmd_split_info.outLoop2Count = 1;
            info_array[i].vx_tp_fc_cmd_split_info.outLoop2Reset = 0x0;
            info_array[i].vx_tp_fc_cmd_split_info.outLoop3Inc   = 0;
            info_array[i].vx_tp_fc_cmd_split_info.outLoop3Count = 1;
            info_array[i].vx_tp_fc_cmd_split_info.outLoop3Reset = 0x0;
            info_array[i].vx_tp_fc_cmd_split_info.outLoop4Inc   = 1;
            info_array[i].vx_tp_fc_cmd_split_info.outLoop4Count = outZSize;
            info_array[i].vx_tp_fc_cmd_split_info.outLoop5Inc   = 0;
            info_array[i].vx_tp_fc_cmd_split_info.outLoop5Count = 1;
            info_array[i].vx_tp_fc_cmd_split_info.outLoop6Inc   = 0;
        }
        else
        {
            if (value_cmd_ptr[i].e32[0] == 0)
            {
                vxmASSERT(WB_IS_USE_TP_FC(weights_biases));

                zOffset = value_cmd_ptr[i].u32[1];
                kzOffset = value_cmd_ptr[i].u32[2];
                kzOffset2 = value_cmd_ptr[i].u32[3];
                inZSize = WB_KERNEL_Z_INDEX(weights_biases, i);
                outZSize = WB_OUTPUT_Z_INDEX(weights_biases, i);

                info_array[i].vx_tp_fc_cmd_split_info.inImageXSize = 1;
                info_array[i].vx_tp_fc_cmd_split_info.inImageYSize = 1;
                info_array[i].vx_tp_fc_cmd_split_info.inImageZSize = inZSize;
                info_array[i].vx_tp_fc_cmd_split_info.inImageStride = 1;
                info_array[i].vx_tp_fc_cmd_split_info.inImageSlice = 1;
                info_array[i].vx_tp_fc_cmd_split_info.inWindowXStart = 0;
                info_array[i].vx_tp_fc_cmd_split_info.inWindowYStart = 0;
                info_array[i].vx_tp_fc_cmd_split_info.inWindowXEnd = 0;
                info_array[i].vx_tp_fc_cmd_split_info.inWindowYEnd = 0;
                info_array[i].vx_tp_fc_cmd_split_info.inTileSequence = 0x3;
                info_array[i].vx_tp_fc_cmd_split_info.inImageBaseAddress = inputBase + kzOffset * inputElemSize;
                info_array[i].vx_tp_fc_cmd_split_info.inTileXSize = 1;
                info_array[i].vx_tp_fc_cmd_split_info.inTileYSize = 1;
                info_array[i].vx_tp_fc_cmd_split_info.inTileXInc = 1;
                info_array[i].vx_tp_fc_cmd_split_info.inTileYInc = 1;
                info_array[i].vx_tp_fc_cmd_split_info.aluHorzProcCount = (outZSize - 1) & 0x3F;
                info_array[i].vx_tp_fc_cmd_split_info.aluVertProcCount = (outZSize - 1) >> 6;
                info_array[i].vx_tp_fc_cmd_split_info.aluLoadPwlLUTAddress = (vx_uint32)WB_MEM_PHYSICAL_ADDR_INDEX(weights_biases, i);
                info_array[i].vx_tp_fc_cmd_split_info.outBaseAddress = outputBase + (kzOffset2 + zOffset) * outputElemSize;
                info_array[i].vx_tp_fc_cmd_split_info.outLoop0Inc   = 0;
                info_array[i].vx_tp_fc_cmd_split_info.outLoop0Count = 1;
                info_array[i].vx_tp_fc_cmd_split_info.outLoop1Inc   = 0;
                info_array[i].vx_tp_fc_cmd_split_info.outLoop1Count = 1;
                info_array[i].vx_tp_fc_cmd_split_info.outLoop1Reset = 0x0;
                info_array[i].vx_tp_fc_cmd_split_info.outLoop2Inc   = 0;
                info_array[i].vx_tp_fc_cmd_split_info.outLoop2Count = 1;
                info_array[i].vx_tp_fc_cmd_split_info.outLoop2Reset = 0x0;
                info_array[i].vx_tp_fc_cmd_split_info.outLoop3Inc   = 0;
                info_array[i].vx_tp_fc_cmd_split_info.outLoop3Count = 1;
                info_array[i].vx_tp_fc_cmd_split_info.outLoop3Reset = 0x0;
                info_array[i].vx_tp_fc_cmd_split_info.outLoop4Inc   = 1;
                info_array[i].vx_tp_fc_cmd_split_info.outLoop4Count = outZSize;
                info_array[i].vx_tp_fc_cmd_split_info.outLoop5Inc   = 0;
                info_array[i].vx_tp_fc_cmd_split_info.outLoop5Count = 1;
                info_array[i].vx_tp_fc_cmd_split_info.outLoop6Inc   = 0;
            }
            else
            {
                /* sum filter */
                vx_uint32 outTileXSize;
                outZSize = value_cmd_ptr[i].u32[1];
                outTileXSize = gcmMIN(outZSize, 64);

                info_array[i].vx_tp_fc_cmd_split_info.inImageXSize = outZSize;
                info_array[i].vx_tp_fc_cmd_split_info.inImageYSize = kzGroup;
                info_array[i].vx_tp_fc_cmd_split_info.inImageZSize = 1;
                info_array[i].vx_tp_fc_cmd_split_info.inImageStride = outZSize;
                info_array[i].vx_tp_fc_cmd_split_info.inImageSlice = outZSize * kzGroup;
                info_array[i].vx_tp_fc_cmd_split_info.inWindowXStart = 0;
                info_array[i].vx_tp_fc_cmd_split_info.inWindowYStart = 0;
                info_array[i].vx_tp_fc_cmd_split_info.inWindowXEnd = outZSize - 1;
                info_array[i].vx_tp_fc_cmd_split_info.inWindowYEnd = kzGroup - 1;
                info_array[i].vx_tp_fc_cmd_split_info.inTileSequence = 0x0;
                info_array[i].vx_tp_fc_cmd_split_info.inImageBaseAddress = inputBase;
                info_array[i].vx_tp_fc_cmd_split_info.inTileXSize = outTileXSize;
                info_array[i].vx_tp_fc_cmd_split_info.inTileYSize = kzGroup;
                info_array[i].vx_tp_fc_cmd_split_info.inTileXInc = outTileXSize;
                info_array[i].vx_tp_fc_cmd_split_info.inTileYInc = kzGroup;
                info_array[i].vx_tp_fc_cmd_split_info.aluHorzProcCount = 0;
                info_array[i].vx_tp_fc_cmd_split_info.aluVertProcCount = kzGroup - 1;
                info_array[i].vx_tp_fc_cmd_split_info.aluLoadPwlLUTAddress = 0;
                info_array[i].vx_tp_fc_cmd_split_info.outBaseAddress = outputBase;
                info_array[i].vx_tp_fc_cmd_split_info.outLoop0Inc   = 0;
                info_array[i].vx_tp_fc_cmd_split_info.outLoop0Count = 1;
                info_array[i].vx_tp_fc_cmd_split_info.outLoop1Inc   = 1;
                info_array[i].vx_tp_fc_cmd_split_info.outLoop1Count = 0;
                info_array[i].vx_tp_fc_cmd_split_info.outLoop1Reset = 0x1;
                info_array[i].vx_tp_fc_cmd_split_info.outLoop2Inc   = outTileXSize;
                info_array[i].vx_tp_fc_cmd_split_info.outLoop2Count = gcmALIGN(outZSize, outTileXSize) / outTileXSize;
                info_array[i].vx_tp_fc_cmd_split_info.outLoop2Reset = 0;
                info_array[i].vx_tp_fc_cmd_split_info.outLoop3Inc   = 0;
                info_array[i].vx_tp_fc_cmd_split_info.outLoop3Count = 1;
                info_array[i].vx_tp_fc_cmd_split_info.outLoop3Reset = 0;
                info_array[i].vx_tp_fc_cmd_split_info.outLoop4Inc   = 0;
                info_array[i].vx_tp_fc_cmd_split_info.outLoop4Count = 1;
                info_array[i].vx_tp_fc_cmd_split_info.outLoop5Inc   = 0;
                info_array[i].vx_tp_fc_cmd_split_info.outLoop5Count = 1;
                info_array[i].vx_tp_fc_cmd_split_info.outLoop6Inc   = 0;
            }
        }

        info_array[i].vx_tp_fc_cmd_split_info.noFlush = (i == split_count - 1 ? 0 : 1);
        info_array[i].vx_tp_fc_cmd_split_info.last = 1;
    }
}

void _fill_TP_TRANSPOSE_Command(
    vx_context                  context,
    vxnne_tensor_info           input,
    vxnne_tensor_info           output,
    vx_op_param                 parameter,
    vx_nn_cmd_info_u*           general_info,
    vx_enum                     split_type,
    vx_uint32                   split_count,
    vx_uint32                   split_sizes[],
    vx_uint32                   split_offsets[],
    vx_nn_cmd_split_info_u*     info_array
    )
{
    vx_uint32 totalSize, inXSizeNew, inYSizeNew;
    vx_uint32 i, j, dims[VX_CONTEXT_TENSOR_MAX_DIMENSION], distances[VX_CONTEXT_TENSOR_MAX_DIMENSION], strides[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32_ptr perm;
    vx_uint32 pnum, dsize;
    DEFINE_TP_GENERAL_PARAMETER();

    vxmASSERT(value_cmd_ptr != VX_NULL);

    dsize = TENSOR_DATA_SIZE((vx_tensor)other_tensor);
    perm = (vx_uint32_ptr) value_cmd_ptr->p8[0];
    pnum = value_cmd_ptr->u32[0];

    vxoTensor_GetTensorDimStride((vx_tensor)other_tensor, &pnum, dims, strides);
    for (i = 0; i < pnum; i++)
    {
        vx_uint32 dim = 1;
        for (j = 0; j < i; j++)
            dim *= dims[perm[j]];
        distances[perm[i]] = dim;
    }

    /* Merge input tensor to 2D image. X/YSize < 2^16. */
    totalSize = strides[pnum-1] * dims[pnum-1] / dsize;
    inXSizeNew = dims[0];
    inYSizeNew = totalSize / inXSizeNew;
    if (split_count == 1 || value_cmd_ptr->e32[0] != 0)
    {
        for (i = 1; i < pnum; i++)
        {
            if (inXSizeNew >= TP_MAX_XYSIZE || inYSizeNew < TP_MAX_XYSIZE)
            {
                break;
            }
            else
            {
                inXSizeNew *= dims[i];
                inYSizeNew /= dims[i];
            }
        }
    }
    else
    {
        for (i = pnum - 1; (vx_int32)i >=0; i--)
        {
            if (dims[i] > 1) break;
        }
        pnum = i + 1;
    }

    for (i = 0; i < split_count; i++)
    {
        if (split_count > 1 && value_cmd_ptr->e32[0] == 0)
        {
            inYSizeNew = strides[pnum-1] / dsize / dims[0] * split_sizes[i];
        }
        vxmASSERT(inXSizeNew < TP_MAX_XYSIZE && inYSizeNew < TP_MAX_XYSIZE);

        if (value_cmd_ptr->e32[0] == 1)
        {
            vx_uint32 inTileXSize = gcmMIN(8, inXSize);
            vx_uint32 inTileYSize = gcmMIN(32, inYSize);
            info_array[i].vx_tp_general_cmd_split_info.inTileSequence = 0x0;
            info_array[i].vx_tp_general_cmd_split_info.inImageBaseAddress = inputBase + inZStride * split_offsets[i];
            info_array[i].vx_tp_general_cmd_split_info.outBaseAddress = outputBase + outZStride * split_offsets[i];
            info_array[i].vx_tp_general_cmd_split_info.inImageXSize   = inXSize;
            info_array[i].vx_tp_general_cmd_split_info.inImageYSize   = inYSize;
            info_array[i].vx_tp_general_cmd_split_info.inImageZSize   = split_sizes[i];
            info_array[i].vx_tp_general_cmd_split_info.inImageStride  = inYStride / inputElemSize;
            info_array[i].vx_tp_general_cmd_split_info.inImageSlice   = inZStride / inputElemSize;
            info_array[i].vx_tp_general_cmd_split_info.inWindowXStart = 0;
            info_array[i].vx_tp_general_cmd_split_info.inWindowYStart = 0;
            info_array[i].vx_tp_general_cmd_split_info.inWindowXEnd   = inXSize - 1;
            info_array[i].vx_tp_general_cmd_split_info.inWindowYEnd   = inYSize - 1;
            info_array[i].vx_tp_general_cmd_split_info.inTileXSize    = inTileXSize;
            info_array[i].vx_tp_general_cmd_split_info.inTileYSize    = inTileYSize;
            info_array[i].vx_tp_general_cmd_split_info.inTileXInc     = inTileXSize;
            info_array[i].vx_tp_general_cmd_split_info.inTileYInc     = inTileYSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc    = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop0Count  = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc    = inYSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Count  = inTileXSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset  = 0x1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc    = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Count  = inTileYSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset  = 0x1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc    = inYSize * inTileXSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Count  = (inXSize + inTileXSize - 1) / inTileXSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset  = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc    = inTileYSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop4Count  = (inYSize + inTileYSize - 1) / inTileYSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc    = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop5Count  = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc    = outZStride / outputElemSize;
        }
        else if (value_cmd_ptr->e32[0] == 2)
        {
            info_array[i].vx_tp_general_cmd_split_info.inTileSequence = 0x0;
            info_array[i].vx_tp_general_cmd_split_info.inImageBaseAddress = inputBase + inZStride * split_offsets[i];
            info_array[i].vx_tp_general_cmd_split_info.outBaseAddress = outputBase + inYSize * split_offsets[i];
            info_array[i].vx_tp_general_cmd_split_info.inImageXSize   = inXSize;
            info_array[i].vx_tp_general_cmd_split_info.inImageYSize   = inYSize;
            info_array[i].vx_tp_general_cmd_split_info.inImageZSize   = split_sizes[i];
            info_array[i].vx_tp_general_cmd_split_info.inImageStride  = inYStride / inputElemSize;
            info_array[i].vx_tp_general_cmd_split_info.inImageSlice   = inZStride / inputElemSize;
            info_array[i].vx_tp_general_cmd_split_info.inWindowXStart = 0;
            info_array[i].vx_tp_general_cmd_split_info.inWindowYStart = 0;
            info_array[i].vx_tp_general_cmd_split_info.inWindowXEnd   = inXSize - 1;
            info_array[i].vx_tp_general_cmd_split_info.inWindowYEnd   = inYSize - 1;
            info_array[i].vx_tp_general_cmd_split_info.inTileXSize    = 1;
            info_array[i].vx_tp_general_cmd_split_info.inTileYSize    = inYSize;
            info_array[i].vx_tp_general_cmd_split_info.inTileXInc     = 1;
            info_array[i].vx_tp_general_cmd_split_info.inTileYInc     = inYSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc    = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop0Count  = inYSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc    = inYSize * inZSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Count  = inXSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset  = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc    = inYSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Count  = split_sizes[i];
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset  = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc    = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Count  = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset  = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc    = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop4Count  = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc    = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop5Count  = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc    = 0;
        }
        else if (value_cmd_ptr->e32[0] == 3)
        {
            info_array[i].vx_tp_general_cmd_split_info.inTileSequence = 0x1;
            info_array[i].vx_tp_general_cmd_split_info.inImageBaseAddress = inputBase + inXSize * split_offsets[i];
            info_array[i].vx_tp_general_cmd_split_info.outBaseAddress = outputBase + inZSize * inXSize * split_offsets[i];
            info_array[i].vx_tp_general_cmd_split_info.inImageXSize   = inXSize;
            info_array[i].vx_tp_general_cmd_split_info.inImageYSize   = split_sizes[i];
            info_array[i].vx_tp_general_cmd_split_info.inImageZSize   = inZSize;
            info_array[i].vx_tp_general_cmd_split_info.inImageStride  = inYStride / inputElemSize;
            info_array[i].vx_tp_general_cmd_split_info.inImageSlice   = inZStride / inputElemSize;
            info_array[i].vx_tp_general_cmd_split_info.inWindowXStart = 0;
            info_array[i].vx_tp_general_cmd_split_info.inWindowYStart = 0;
            info_array[i].vx_tp_general_cmd_split_info.inWindowXEnd   = inXSize - 1;
            info_array[i].vx_tp_general_cmd_split_info.inWindowYEnd   = split_sizes[i] - 1;
            info_array[i].vx_tp_general_cmd_split_info.inTileXSize    = 1;
            info_array[i].vx_tp_general_cmd_split_info.inTileYSize    = 1;
            info_array[i].vx_tp_general_cmd_split_info.inTileXInc     = 1;
            info_array[i].vx_tp_general_cmd_split_info.inTileYInc     = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc    = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop0Count  = inZSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc    = inZSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Count  = inXSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset  = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc    = inZSize * inXSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Count  = split_sizes[i];
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset  = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc    = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Count  = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset  = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc    = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop4Count  = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc    = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop5Count  = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc    = 0;
        }
        else if (value_cmd_ptr->e32[0] == 4)
        {
            info_array[i].vx_tp_general_cmd_split_info.inTileSequence = 0x1;
            info_array[i].vx_tp_general_cmd_split_info.inImageBaseAddress = inputBase + inXSize * split_offsets[i];
            info_array[i].vx_tp_general_cmd_split_info.outBaseAddress = outputBase + inZSize * split_offsets[i];
            info_array[i].vx_tp_general_cmd_split_info.inImageXSize   = inXSize;
            info_array[i].vx_tp_general_cmd_split_info.inImageYSize   = split_sizes[i];
            info_array[i].vx_tp_general_cmd_split_info.inImageZSize   = inZSize;
            info_array[i].vx_tp_general_cmd_split_info.inImageStride  = inYStride / inputElemSize;
            info_array[i].vx_tp_general_cmd_split_info.inImageSlice   = inZStride / inputElemSize;
            info_array[i].vx_tp_general_cmd_split_info.inWindowXStart = 0;
            info_array[i].vx_tp_general_cmd_split_info.inWindowYStart = 0;
            info_array[i].vx_tp_general_cmd_split_info.inWindowXEnd   = inXSize - 1;
            info_array[i].vx_tp_general_cmd_split_info.inWindowYEnd   = split_sizes[i] - 1;
            info_array[i].vx_tp_general_cmd_split_info.inTileXSize    = 1;
            info_array[i].vx_tp_general_cmd_split_info.inTileYSize    = 1;
            info_array[i].vx_tp_general_cmd_split_info.inTileXInc     = 1;
            info_array[i].vx_tp_general_cmd_split_info.inTileYInc     = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc    = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop0Count  = inZSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc    = inZSize * inXSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Count  = inXSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset  = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc    = inZSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Count  = split_sizes[i];
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset  = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc    = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Count  = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset  = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc    = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop4Count  = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc    = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop5Count  = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc    = 0;
        }
        else if (value_cmd_ptr->e32[0] == 5)
        {
            info_array[i].vx_tp_general_cmd_split_info.inTileSequence = 0x1;
            info_array[i].vx_tp_general_cmd_split_info.inImageBaseAddress = inputBase + inXSize * split_offsets[i];
            info_array[i].vx_tp_general_cmd_split_info.outBaseAddress = outputBase + inXSize * inZSize * split_offsets[i];
            info_array[i].vx_tp_general_cmd_split_info.inImageXSize   = inXSize;
            info_array[i].vx_tp_general_cmd_split_info.inImageYSize   = split_sizes[i];
            info_array[i].vx_tp_general_cmd_split_info.inImageZSize   = inZSize;
            info_array[i].vx_tp_general_cmd_split_info.inImageStride  = inYStride / inputElemSize;
            info_array[i].vx_tp_general_cmd_split_info.inImageSlice   = inZStride / inputElemSize;
            info_array[i].vx_tp_general_cmd_split_info.inWindowXStart = 0;
            info_array[i].vx_tp_general_cmd_split_info.inWindowYStart = 0;
            info_array[i].vx_tp_general_cmd_split_info.inWindowXEnd   = inXSize - 1;
            info_array[i].vx_tp_general_cmd_split_info.inWindowYEnd   = split_sizes[i] - 1;
            info_array[i].vx_tp_general_cmd_split_info.inTileXSize    = inXSize;
            info_array[i].vx_tp_general_cmd_split_info.inTileYSize    = 1;
            info_array[i].vx_tp_general_cmd_split_info.inTileXInc     = inXSize;
            info_array[i].vx_tp_general_cmd_split_info.inTileYInc     = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc    = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop0Count  = inXSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc    = inXSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Count  = inZSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset  = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc    = inXSize * inZSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Count  = split_sizes[i];
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset  = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc    = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Count  = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset  = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc    = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop4Count  = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc    = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop5Count  = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc    = 0;
        }
        else
        {
            info_array[i].vx_tp_general_cmd_split_info.inImageBaseAddress = inputBase + strides[pnum-1] * split_offsets[i];
            info_array[i].vx_tp_general_cmd_split_info.outBaseAddress = outputBase + distances[pnum-1] * split_offsets[i] * outputElemSize;
            info_array[i].vx_tp_general_cmd_split_info.inImageXSize   = inXSizeNew;
            info_array[i].vx_tp_general_cmd_split_info.inImageYSize   = inYSizeNew;
            info_array[i].vx_tp_general_cmd_split_info.inImageZSize   = 1;
            info_array[i].vx_tp_general_cmd_split_info.inImageStride  = inXSizeNew;
            info_array[i].vx_tp_general_cmd_split_info.inImageSlice   = totalSize;
            info_array[i].vx_tp_general_cmd_split_info.inWindowXStart = 0;
            info_array[i].vx_tp_general_cmd_split_info.inWindowYStart = 0;
            info_array[i].vx_tp_general_cmd_split_info.inWindowXEnd   = inXSizeNew - 1;
            info_array[i].vx_tp_general_cmd_split_info.inWindowYEnd   = inYSizeNew - 1;
            info_array[i].vx_tp_general_cmd_split_info.inTileXSize    = 1;
            info_array[i].vx_tp_general_cmd_split_info.inTileYSize    = 1;
            info_array[i].vx_tp_general_cmd_split_info.inTileXInc     = 1;
            info_array[i].vx_tp_general_cmd_split_info.inTileYInc     = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc   = distances[0];
            info_array[i].vx_tp_general_cmd_split_info.outLoop0Count = dims[0];
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc   = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Count = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc   = distances[1];
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Count = split_count > 1 && pnum == 2 ? split_sizes[i] : dims[1];
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc   = pnum > 2 ? distances[2] : 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Count = split_count > 1 && pnum == 3 ? split_sizes[i] : pnum > 2 ? dims[2] : 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc   = pnum > 3 ? distances[3] : 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop4Count = split_count > 1 && pnum == 4 ? split_sizes[i] : pnum > 3 ? dims[3] : 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc   = pnum > 4 ? distances[4] : 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop5Count = split_count > 1 && pnum == 5 ? split_sizes[i] : pnum > 4 ? dims[4] : 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc   = pnum > 5 ? distances[5] : 0;

            info_array[i].vx_tp_general_cmd_split_info.noFlush = (i == split_count - 1 ? 0 : 1);
            info_array[i].vx_tp_general_cmd_split_info.last = 1;
        }
    }
}

void _fill_TP_MAX_POOLING_Command(
    vx_context                  context,
    vxnne_tensor_info           input,
    vxnne_tensor_info           output,
    vx_op_param                 parameter,
    vx_nn_cmd_info_u*           general_info,
    vx_enum                     split_type,
    vx_uint32                   split_count,
    vx_uint32                   split_sizes[],
    vx_uint32                   split_offsets[],
    vx_nn_cmd_split_info_u*     info_array
    )
{
    vx_uint32 i;
    vx_uint32 poolingStride = parameter->pool_stride;
    vx_uint32 outTileXSize, outTileYSize;
    DEFINE_TP_GENERAL_PARAMETER();

    vxmASSERT((poolingStride == 1 && parameter->pool_size_x <= 3) ||
              ((poolingStride == parameter->pool_size_x || poolingStride == parameter->pool_size_x-1 || parameter->pool_size_x == 1) &&
                parameter->pool_size_x <= 64));

    outTileXSize = (64 + poolingStride - parameter->pool_size_x) / poolingStride;
    outTileYSize = inXSize < 64 ? 16 : 1;

    for (i = 0; i < split_count; i++)
    {
        if (parameter->pool_size_x != 1)
        {
            info_array[i].vx_tp_general_cmd_split_info.inImageXSize = inXSize;
            info_array[i].vx_tp_general_cmd_split_info.inImageYSize = inYSize;
            info_array[i].vx_tp_general_cmd_split_info.inImageZSize = split_sizes[i];
            info_array[i].vx_tp_general_cmd_split_info.inImageStride = inYStride / inputElemSize;
            info_array[i].vx_tp_general_cmd_split_info.inImageSlice = inZStride / inputElemSize;
            info_array[i].vx_tp_general_cmd_split_info.inWindowXStart = (-1) * (vx_int32)parameter->pad_x_left;
            info_array[i].vx_tp_general_cmd_split_info.inWindowYStart = (-1) * (vx_int32)parameter->pad_y_top;
            info_array[i].vx_tp_general_cmd_split_info.inWindowXEnd = (outXSize - 1) * poolingStride + parameter->pool_size_x - 1 - parameter->pad_x_left;
            info_array[i].vx_tp_general_cmd_split_info.inWindowYEnd = (outYSize - 1) * poolingStride + parameter->pool_size_y - 1 - parameter->pad_y_top;

            info_array[i].vx_tp_general_cmd_split_info.inTileSequence = 0x0;
            info_array[i].vx_tp_general_cmd_split_info.inImageBaseAddress = inputBase + inZStride * split_offsets[i];
            info_array[i].vx_tp_general_cmd_split_info.inTileXSize = outTileXSize * poolingStride + parameter->pool_size_x - poolingStride;
            info_array[i].vx_tp_general_cmd_split_info.inTileYSize = outTileYSize * poolingStride + parameter->pool_size_y - poolingStride;
            info_array[i].vx_tp_general_cmd_split_info.inTileXInc = outTileXSize * poolingStride;
            info_array[i].vx_tp_general_cmd_split_info.inTileYInc = outTileYSize * poolingStride;
            info_array[i].vx_tp_general_cmd_split_info.outBaseAddress = outputBase + outZStride * split_offsets[i];
            info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc   = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop0Count = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc   = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Count = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset = 0x1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc   = outXSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Count = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset = 0x1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc   = outTileXSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Count = (outXSize + outTileXSize - 1) / outTileXSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset = 0x0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc   = outTileYSize * outXSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop4Count = (outYSize + outTileYSize - 1) / outTileYSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc   = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop5Count = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc   = outZStride / outputElemSize;
        }
        else
        {
            info_array[i].vx_tp_general_cmd_split_info.inImageXSize = 1;
            info_array[i].vx_tp_general_cmd_split_info.inImageYSize = outXSize;
            info_array[i].vx_tp_general_cmd_split_info.inImageZSize = outYSize * split_sizes[i];
            info_array[i].vx_tp_general_cmd_split_info.inImageStride = poolingStride;
            info_array[i].vx_tp_general_cmd_split_info.inImageSlice = inXSize * poolingStride;
            info_array[i].vx_tp_general_cmd_split_info.inWindowXStart = 0;
            info_array[i].vx_tp_general_cmd_split_info.inWindowYStart = 0;
            info_array[i].vx_tp_general_cmd_split_info.inWindowXEnd = 0;
            info_array[i].vx_tp_general_cmd_split_info.inWindowYEnd = outXSize - 1;

            info_array[i].vx_tp_general_cmd_split_info.inTileSequence = 0x0;
            info_array[i].vx_tp_general_cmd_split_info.inImageBaseAddress = inputBase + inZStride * split_offsets[i];
            info_array[i].vx_tp_general_cmd_split_info.inTileXSize = 1;
            info_array[i].vx_tp_general_cmd_split_info.inTileYSize = 1;
            info_array[i].vx_tp_general_cmd_split_info.inTileXInc = 1;
            info_array[i].vx_tp_general_cmd_split_info.inTileYInc = 1;
            info_array[i].vx_tp_general_cmd_split_info.outBaseAddress = outputBase + outZStride * split_offsets[i];
            info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc   = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop0Count = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc   = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Count = outXSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset = 0x0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc   = outXSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Count = outYSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset = 0x0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc   = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Count = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset = 0x0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc   = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop4Count = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc   = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop5Count = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc   = outZStride / outputElemSize;
        }

        /* Select the correct border mode if padding is not specified. */
        if (parameter->orig_no_pad &&
            ((inXSize - parameter->pool_size_x) % parameter->pool_stride ||
             (inYSize - parameter->pool_size_y) % parameter->pool_stride))
        {
            general_info->vx_nn_tp_cmd_info.inImageBorderMode = 0x1;
        }

        info_array[i].vx_tp_general_cmd_split_info.noFlush = (i == split_count - 1 ? 0 : 1);
        info_array[i].vx_tp_general_cmd_split_info.last = 1;
    }
}

void _fill_TP_ACTIVATION_Command(
    vx_context                  context,
    vxnne_tensor_info           input,
    vxnne_tensor_info           output,
    vx_op_param                 parameter,
    vx_nn_cmd_info_u*           general_info,
    vx_enum                     split_type,
    vx_uint32                   split_count,
    vx_uint32                   split_sizes[],
    vx_uint32                   split_offsets[],
    vx_nn_cmd_split_info_u*     info_array
    )
{
    vx_uint32 i;
    vx_uint32 outTileXSize, outTileYSize;
    vx_bool tileOff = vx_false_e;
    DEFINE_TP_GENERAL_PARAMETER();

    if (inXSize == outXSize && inYSize == outYSize && inZSize == outZSize)
    {
        outTileXSize = 64;
        outTileYSize = 16;
    }
    else
    {
        outTileXSize = outXSize;
        outTileYSize = outYSize;
        tileOff = vx_true_e;
    }

    for (i = 0; i < split_count; i++)
    {
        info_array[i].vx_tp_general_cmd_split_info.inImageXSize = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageYSize = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageZSize = split_sizes[i];
        info_array[i].vx_tp_general_cmd_split_info.inImageBaseAddress = inputBase + inZStride * split_offsets[i];
        info_array[i].vx_tp_general_cmd_split_info.inWindowXStart = 0;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYStart = 0;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXEnd = inXSize - 1;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYEnd = inYSize - 1;
        info_array[i].vx_tp_general_cmd_split_info.inImageStride = inYStride / inputElemSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageSlice = inZStride / inputElemSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileSequence = 0x0;
        info_array[i].vx_tp_general_cmd_split_info.inTileXSize = outTileXSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileYSize = outTileYSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileXInc = outTileXSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileYInc = outTileYSize;
        info_array[i].vx_tp_general_cmd_split_info.outBaseAddress = outputBase + outZStride * split_offsets[i];
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc   = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc   = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Count = tileOff ? outXSize : 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset = tileOff ? 0 : 0x1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc   = outXSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Count = tileOff ? outYSize : 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset = tileOff ? 0 : 0x1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc   = outTileXSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Count = (outXSize + outTileXSize - 1) / outTileXSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset = 0x0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc   = outTileYSize * outXSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Count = (outYSize + outTileYSize - 1) / outTileYSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc   = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc   = outZStride / outputElemSize;

        info_array[i].vx_tp_general_cmd_split_info.noFlush = (i == split_count - 1 ? 0 : 1);
        info_array[i].vx_tp_general_cmd_split_info.last = 1;
    }
}

void _fill_TP_LRN_Command(
    vx_context                  context,
    vxnne_tensor_info           input,
    vxnne_tensor_info           output,
    vx_op_param                 parameter,
    vx_nn_cmd_info_u*           general_info,
    vx_enum                     split_type,
    vx_uint32                   split_count,
    vx_uint32                   split_sizes[],
    vx_uint32                   split_offsets[],
    vx_nn_cmd_split_info_u*     info_array
    )
{
    vx_uint32 i, kernel, halfk;
    vx_uint32 outTileXSize, outTileYSize;
    DEFINE_TP_GENERAL_PARAMETER();

    vxmASSERT(value_cmd_ptr != VX_NULL);

    kernel = value_cmd_ptr->u32[0];
    halfk  = kernel / 2;

    for (i = 0; i < split_count; i++)
    {
        if (value_cmd_ptr->e32[0]  == VX_NN_NORMALIZATION_SAME_MAP)
        {
            outTileXSize = 62;
            outTileYSize = 16;
            info_array[i].vx_tp_general_cmd_split_info.inImageXSize = inXSize;
            info_array[i].vx_tp_general_cmd_split_info.inImageYSize = inYSize;
            info_array[i].vx_tp_general_cmd_split_info.inImageZSize = split_sizes[i];
            info_array[i].vx_tp_general_cmd_split_info.inImageStride = inYStride / inputElemSize;
            info_array[i].vx_tp_general_cmd_split_info.inImageSlice = inZStride / inputElemSize;
            info_array[i].vx_tp_general_cmd_split_info.inImageBaseAddress = inputBase + inZStride * split_offsets[i];
            info_array[i].vx_tp_general_cmd_split_info.outBaseAddress = outputBase + outZStride * split_offsets[i];
            info_array[i].vx_tp_general_cmd_split_info.inWindowXStart = halfk * (-1);
            info_array[i].vx_tp_general_cmd_split_info.inWindowYStart = halfk * (-1);
            info_array[i].vx_tp_general_cmd_split_info.inWindowXEnd = inXSize - 1 + halfk;
            info_array[i].vx_tp_general_cmd_split_info.inWindowYEnd = inYSize - 1 + halfk;
            info_array[i].vx_tp_general_cmd_split_info.inTileSequence = 0x0;
            info_array[i].vx_tp_general_cmd_split_info.inTileXSize = outTileXSize + kernel - halfk;
            info_array[i].vx_tp_general_cmd_split_info.inTileYSize = outTileYSize + kernel - halfk;
            info_array[i].vx_tp_general_cmd_split_info.inTileXInc = outTileXSize;
            info_array[i].vx_tp_general_cmd_split_info.inTileYInc = outTileYSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc   = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop0Count = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc   = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Count = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset = 0x1;;
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc   = inXSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Count = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset = 0x1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc   = outTileXSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Count = (inXSize + outTileXSize - 1) / outTileXSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset = 0x0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc   = outTileYSize * inXSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop4Count = (inYSize + outTileYSize - 1) / outTileYSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc   = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop5Count = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc   = outZStride / outputElemSize;
        }
        else /* VX_NN_NORMALIZATION_ACROSS_MAPS */
        {
            if (inXSize * inYSize < 65536)
            {
                /* Convert to 2D same map LRN. */
                outTileXSize = 32;
                outTileYSize = inZSize;

                info_array[i].vx_tp_general_cmd_split_info.inImageXSize = split_sizes[i];
                info_array[i].vx_tp_general_cmd_split_info.inImageYSize = inZSize;
                info_array[i].vx_tp_general_cmd_split_info.inImageZSize = 1;
                info_array[i].vx_tp_general_cmd_split_info.inImageStride = inZStride / inputElemSize;
                info_array[i].vx_tp_general_cmd_split_info.inImageSlice = inZStride / inputElemSize * inZSize;
                info_array[i].vx_tp_general_cmd_split_info.inImageBaseAddress = inputBase + split_offsets[i] * inputElemSize;
                info_array[i].vx_tp_general_cmd_split_info.outBaseAddress = outputBase + split_offsets[i] * outputElemSize;
                info_array[i].vx_tp_general_cmd_split_info.inWindowXStart = 0;
                info_array[i].vx_tp_general_cmd_split_info.inWindowYStart = halfk * (-1);
                info_array[i].vx_tp_general_cmd_split_info.inWindowXEnd = split_sizes[i] - 1;
                info_array[i].vx_tp_general_cmd_split_info.inWindowYEnd = inZSize - 1 + halfk;
                info_array[i].vx_tp_general_cmd_split_info.inTileSequence = 0x0;
                info_array[i].vx_tp_general_cmd_split_info.inTileXSize = outTileXSize;
                info_array[i].vx_tp_general_cmd_split_info.inTileYSize = outTileYSize + kernel - 1;
                info_array[i].vx_tp_general_cmd_split_info.inTileXInc = outTileXSize;
                info_array[i].vx_tp_general_cmd_split_info.inTileYInc = outTileYSize;
                info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc   = 0;
                info_array[i].vx_tp_general_cmd_split_info.outLoop0Count = 1;
                info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc   = 1;
                info_array[i].vx_tp_general_cmd_split_info.outLoop1Count = 0;
                info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset = 0x1;
                info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc   = outZStride / outputElemSize;
                info_array[i].vx_tp_general_cmd_split_info.outLoop2Count = 0;
                info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset = 0x1;
                info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc   = outTileXSize;
                info_array[i].vx_tp_general_cmd_split_info.outLoop3Count = (split_sizes[i] + outTileXSize - 1) / outTileXSize;
                info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset = 0x0;
                info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc   = outTileYSize * outYStride / outputElemSize;
                info_array[i].vx_tp_general_cmd_split_info.outLoop4Count = (inZSize + outTileYSize - 1) / outTileYSize;
                info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc   = 0;
                info_array[i].vx_tp_general_cmd_split_info.outLoop5Count = 1;
                info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc   = 0;
            }
            else
            {
                /* Turn on z filter. */
                /* Input X-Z plain as X-Y plain. */
                outTileXSize = 32;
                outTileYSize = 1;

                info_array[i].vx_tp_general_cmd_split_info.inImageXSize = split_sizes[i];
                info_array[i].vx_tp_general_cmd_split_info.inImageYSize = inYSize;
                info_array[i].vx_tp_general_cmd_split_info.inImageZSize = inZSize;
                info_array[i].vx_tp_general_cmd_split_info.inImageStride = inYStride / inputElemSize;
                info_array[i].vx_tp_general_cmd_split_info.inImageSlice = inZStride / inputElemSize;
                info_array[i].vx_tp_general_cmd_split_info.inImageBaseAddress = inputBase + split_offsets[i] * inputElemSize;
                info_array[i].vx_tp_general_cmd_split_info.outBaseAddress = outputBase + split_offsets[i] * outputElemSize;
                info_array[i].vx_tp_general_cmd_split_info.inWindowXStart = 0;
                info_array[i].vx_tp_general_cmd_split_info.inWindowYStart = 0;
                info_array[i].vx_tp_general_cmd_split_info.inWindowXEnd = split_sizes[i] - 1;
                info_array[i].vx_tp_general_cmd_split_info.inWindowYEnd = inYSize - 1;
                info_array[i].vx_tp_general_cmd_split_info.inTileSequence = 0x1;
                info_array[i].vx_tp_general_cmd_split_info.inTileXSize = outTileXSize;
                info_array[i].vx_tp_general_cmd_split_info.inTileYSize = outTileYSize;
                info_array[i].vx_tp_general_cmd_split_info.inTileXInc = outTileXSize;
                info_array[i].vx_tp_general_cmd_split_info.inTileYInc = outTileYSize;
                info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc   = 0;
                info_array[i].vx_tp_general_cmd_split_info.outLoop0Count = 1;
                info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc   = 1;
                info_array[i].vx_tp_general_cmd_split_info.outLoop1Count = 0;
                info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset = 0x1;
                info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc   = outZStride / outputElemSize;;
                info_array[i].vx_tp_general_cmd_split_info.outLoop2Count = outZSize;
                info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset = 0x0;
                info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc   = outTileXSize;
                info_array[i].vx_tp_general_cmd_split_info.outLoop3Count = (split_sizes[i] + outTileXSize - 1) / outTileXSize;
                info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset = 0x0;
                info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc   = outYStride / outputElemSize;
                info_array[i].vx_tp_general_cmd_split_info.outLoop4Count = outYSize;
                info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc   = 0;
                info_array[i].vx_tp_general_cmd_split_info.outLoop5Count = 1;
                info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc   = 0;
            }
        }

        info_array[i].vx_tp_general_cmd_split_info.noFlush = (i == split_count - 1 ? 0 : 1);
        info_array[i].vx_tp_general_cmd_split_info.last = 1;
    }
}

void _fill_TP_ROI_POOLING_Command(
    vx_context                  context,
    vxnne_tensor_info           input,
    vxnne_tensor_info           output,
    vx_op_param                 parameter,
    vx_nn_cmd_info_u*           general_info,
    vx_enum                     split_type,
    vx_uint32                   split_count,
    vx_uint32                   split_sizes[],
    vx_uint32                   split_offsets[],
    vx_nn_cmd_split_info_u*     info_array
    )
{
    vx_uint32 i;
    vx_uint32 poolWidth, poolHeight, outTileXSize, outTileYSize;
    vx_uint32 maxPoolSize, poolXSize, poolYSize, poolZSize;
    DEFINE_TP_GENERAL_PARAMETER();

    vxmASSERT(value_cmd_ptr != VX_NULL);

    poolWidth = value_cmd_ptr->u32[0];
    poolHeight = value_cmd_ptr->u32[1];
    maxPoolSize = value_cmd_ptr->u32[2];
    poolXSize = value_cmd_ptr->u32[3];
    poolYSize = value_cmd_ptr->u32[4];
    poolZSize = value_cmd_ptr->u32[5];
    outTileXSize = inXSize;
    outTileYSize = 16;

    for (i = 0; i < split_count; i++)
    {
        if (value_cmd_ptr->e32[0] == 0)
        {
            info_array[i].vx_tp_general_cmd_split_info.inImageXSize = inXSize;
            info_array[i].vx_tp_general_cmd_split_info.inImageYSize = inYSize;
            info_array[i].vx_tp_general_cmd_split_info.inImageZSize = split_sizes[i];
            info_array[i].vx_tp_general_cmd_split_info.inImageStride = inYStride / inputElemSize;
            info_array[i].vx_tp_general_cmd_split_info.inImageSlice = inZStride / inputElemSize;
            info_array[i].vx_tp_general_cmd_split_info.inWindowXStart = 0;
            info_array[i].vx_tp_general_cmd_split_info.inWindowYStart = 0;
            info_array[i].vx_tp_general_cmd_split_info.inWindowXEnd = outTileXSize - 1 + maxPoolSize - 1;
            info_array[i].vx_tp_general_cmd_split_info.inWindowYEnd = inYSize - 1;
            info_array[i].vx_tp_general_cmd_split_info.inTileSequence = 0x0;
            info_array[i].vx_tp_general_cmd_split_info.inImageBaseAddress = inputBase + inZStride * split_offsets[i];
            info_array[i].vx_tp_general_cmd_split_info.inTileListAddress = 0;
            info_array[i].vx_tp_general_cmd_split_info.inTileXSize = outTileXSize;
            info_array[i].vx_tp_general_cmd_split_info.inTileXInc = 1;
            info_array[i].vx_tp_general_cmd_split_info.outBaseAddress = outputBase + poolXSize * poolYSize * split_offsets[i] * outputElemSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc   = poolXSize * poolYSize * poolZSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop0Count = maxPoolSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc   = poolYSize * maxPoolSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Count = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset = 0x1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc   = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Count = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset = 0x1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc   = poolYSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Count = maxPoolSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset = 0x0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc   = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop5Count = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc   = poolXSize * poolYSize;
            info_array[i].vx_tp_general_cmd_split_info.inTileYSize = outTileYSize;
            info_array[i].vx_tp_general_cmd_split_info.inTileYInc = outTileYSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc   = outTileYSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop4Count = (inYSize + outTileYSize - 1) / outTileYSize;
        }
        else if (value_cmd_ptr->e32[0] == 1)
        {
            vx_uint32 proposalsInterleaved, zTogether;
            vx_tensor inputTensor = (vx_tensor)other_tensor;

            vxmASSERT(data_buff != VX_NULL);

            inXSize = TENSOR_VIEW_SIZE_INDEX(inputTensor, 0);
            inYSize = TENSOR_VIEW_SIZE_INDEX(inputTensor, 1);
            inZSize = TENSOR_VIEW_SIZE_INDEX(inputTensor, 2);

            proposalsInterleaved = 1;
            zTogether = 1;

            info_array[i].vx_tp_general_cmd_split_info.inImageXSize = inYSize;
            info_array[i].vx_tp_general_cmd_split_info.inImageYSize = inXSize;
            info_array[i].vx_tp_general_cmd_split_info.inImageZSize = inZSize;
            info_array[i].vx_tp_general_cmd_split_info.inImageStride = poolYSize;
            info_array[i].vx_tp_general_cmd_split_info.inImageSlice = poolXSize * poolYSize;
            info_array[i].vx_tp_general_cmd_split_info.inWindowXStart = 0;
            info_array[i].vx_tp_general_cmd_split_info.inWindowYStart = 0;
            info_array[i].vx_tp_general_cmd_split_info.inWindowXEnd = inYSize - 1 + maxPoolSize - 1;
            info_array[i].vx_tp_general_cmd_split_info.inWindowYEnd = inXSize - 1;
            info_array[i].vx_tp_general_cmd_split_info.inTileSequence = 0x2;
            info_array[i].vx_tp_general_cmd_split_info.inImageBaseAddress = inputBase;
            info_array[i].vx_tp_general_cmd_split_info.inTileListAddress = data_buff->tensorBuffer->memory.physicals[0] + split_offsets[i] * sizeof(vx_tp_roi_pool);
            info_array[i].vx_tp_general_cmd_split_info.inTileXSize = poolHeight;
            info_array[i].vx_tp_general_cmd_split_info.inTileYSize = poolWidth;
            info_array[i].vx_tp_general_cmd_split_info.inTileXInc = zTogether;
            info_array[i].vx_tp_general_cmd_split_info.inTileYInc = 0;
            info_array[i].vx_tp_general_cmd_split_info.outBaseAddress = outputBase + poolWidth * poolHeight * inZSize * split_offsets[i] * outputElemSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc   = poolWidth * proposalsInterleaved;
            info_array[i].vx_tp_general_cmd_split_info.outLoop0Count = poolHeight;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc   = proposalsInterleaved;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Count = poolWidth;
            info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset = 0x0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc   = poolWidth * poolHeight * proposalsInterleaved;
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Count = zTogether;
            info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset = 0x0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc   = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Count = proposalsInterleaved;
            info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset = 0x0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc   = poolWidth * poolHeight * inZSize;
            info_array[i].vx_tp_general_cmd_split_info.outLoop4Count = split_sizes[i];
            info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc   = 0;
            info_array[i].vx_tp_general_cmd_split_info.outLoop5Count = 1;
            info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc   = poolWidth * poolHeight * proposalsInterleaved * zTogether;
        }

        info_array[i].vx_tp_general_cmd_split_info.noFlush = (i == split_count - 1 ? 0 : 1);
        info_array[i].vx_tp_general_cmd_split_info.last = 1;
    }
}

void _fill_TP_REORG_Command(
    vx_context                  context,
    vxnne_tensor_info           input,
    vxnne_tensor_info           output,
    vx_op_param                 parameter,
    vx_nn_cmd_info_u*           general_info,
    vx_enum                     split_type,
    vx_uint32                   split_count,
    vx_uint32                   split_sizes[],
    vx_uint32                   split_offsets[],
    vx_nn_cmd_split_info_u*     info_array
    )
{
    vx_uint32 i, strideXSize, strideYSize;
    DEFINE_TP_GENERAL_PARAMETER();

    vxmASSERT(value_cmd_ptr != VX_NULL);

    strideXSize = strideYSize = value_cmd_ptr->u32[0];
    outXSize = inXSize;
    outYSize = inYSize;
    inXSize = inXSize * strideXSize;
    inYSize = inYSize * strideYSize;
    inZSize = inZSize / strideXSize / strideYSize;

    for (i = 0; i < split_count; i++)
    {
        info_array[i].vx_tp_general_cmd_split_info.inImageXSize = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageYSize = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageZSize = split_sizes[i];
        info_array[i].vx_tp_general_cmd_split_info.inImageStride = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageSlice = inXSize * inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXStart = 0;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYStart = 0;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXEnd = inXSize - 1;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYEnd = inYSize - 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileSequence = 0x0;
        info_array[i].vx_tp_general_cmd_split_info.inImageBaseAddress = inputBase + inXSize * inYSize * split_offsets[i] * inputElemSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileXSize = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileYSize = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileXInc = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileYInc = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.outBaseAddress = outputBase + outXSize * outYSize * split_offsets[i] * outputElemSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc   = outXSize * outYSize * inZSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Count = strideXSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc   = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Count = outXSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc   = outXSize * outYSize * strideXSize * inZSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Count = strideYSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc   = outXSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Count = outYSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc   = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc   = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc   = outXSize * outYSize;

        info_array[i].vx_tp_general_cmd_split_info.noFlush = (i == split_count - 1 ? 0 : 1);
        info_array[i].vx_tp_general_cmd_split_info.last = 1;
    }
}

void _fill_TP_REORG_DEPTH2SPACE_Command(
    vx_context                  context,
    vxnne_tensor_info           input,
    vxnne_tensor_info           output,
    vx_op_param                 parameter,
    vx_nn_cmd_info_u*           general_info,
    vx_enum                     split_type,
    vx_uint32                   split_count,
    vx_uint32                   split_sizes[],
    vx_uint32                   split_offsets[],
    vx_nn_cmd_split_info_u*     info_array
    )
{
    vx_uint32 i, blockSize;
    DEFINE_TP_GENERAL_PARAMETER();

    vxmASSERT(value_cmd_ptr != VX_NULL);

    blockSize = value_cmd_ptr->u32[0];

    vxmASSERT(outXSize == (inXSize * blockSize));
    vxmASSERT(outYSize == (inYSize * blockSize));
    vxmASSERT((outZSize * blockSize * blockSize) == inZSize);

    for (i = 0; i < split_count; i++)
    {
        info_array[i].vx_tp_general_cmd_split_info.inImageXSize = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageYSize = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageZSize = split_sizes[i];
        info_array[i].vx_tp_general_cmd_split_info.inImageStride = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageSlice = inXSize * inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXStart = 0;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYStart = 0;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXEnd = inXSize - 1;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYEnd = inYSize - 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileSequence = 0x0;
        info_array[i].vx_tp_general_cmd_split_info.inImageBaseAddress = inputBase + inXSize * inYSize * split_offsets[i] * inputElemSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileXSize = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileYSize = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileXInc = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileYInc = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.outBaseAddress = outputBase + outXSize * outYSize * split_offsets[i] * outputElemSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc   = blockSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Count = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc   = outXSize * blockSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Count = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc   = outXSize * outYSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Count = outZSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc   = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Count = blockSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc   = outXSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Count = blockSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc   = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc   = 0;

        info_array[i].vx_tp_general_cmd_split_info.noFlush = (i == split_count - 1 ? 0 : 1);
        info_array[i].vx_tp_general_cmd_split_info.last = 1;
    }
}

void _fill_TP_REORG_SPACE2DEPTH_Command(
    vx_context                  context,
    vxnne_tensor_info           input,
    vxnne_tensor_info           output,
    vx_op_param                 parameter,
    vx_nn_cmd_info_u*           general_info,
    vx_enum                     split_type,
    vx_uint32                   split_count,
    vx_uint32                   split_sizes[],
    vx_uint32                   split_offsets[],
    vx_nn_cmd_split_info_u*     info_array
    )
{
    vx_uint32 i, blockSize;
    DEFINE_TP_GENERAL_PARAMETER();

    vxmASSERT(value_cmd_ptr != VX_NULL);

    blockSize = value_cmd_ptr->u32[0];

    vxmASSERT(inXSize == (outXSize * blockSize));
    vxmASSERT(inYSize == (outYSize * blockSize));
    vxmASSERT((inZSize * blockSize * blockSize) == outZSize);

    for (i = 0; i < split_count; i++)
    {
        info_array[i].vx_tp_general_cmd_split_info.inImageXSize = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageYSize = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageZSize = split_sizes[i];
        info_array[i].vx_tp_general_cmd_split_info.inImageStride = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageSlice = inXSize * inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXStart = 0;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYStart = 0;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXEnd = inXSize - 1;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYEnd = inYSize - 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileSequence = 0x0;
        info_array[i].vx_tp_general_cmd_split_info.inImageBaseAddress = inputBase + inXSize * inYSize * split_offsets[i] * inputElemSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileXSize = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileYSize = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileXInc = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileYInc = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.outBaseAddress = outputBase + outXSize * outYSize * split_offsets[i] * outputElemSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc   = outXSize * outYSize * inZSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Count = blockSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc   = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Count = outXSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc   = outXSize * outYSize * inZSize * blockSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Count = blockSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc   = outXSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Count = outYSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc   = outXSize * outYSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Count = inZSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc   = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc   = 0;

        info_array[i].vx_tp_general_cmd_split_info.noFlush = (i == split_count - 1 ? 0 : 1);
        info_array[i].vx_tp_general_cmd_split_info.last = 1;
    }
}

void _fill_TP_REORG_SPACE2BATCH_Command(
    vx_context                  context,
    vxnne_tensor_info           input,
    vxnne_tensor_info           output,
    vx_op_param                 parameter,
    vx_nn_cmd_info_u*           general_info,
    vx_enum                     split_type,
    vx_uint32                   split_count,
    vx_uint32                   split_sizes[],
    vx_uint32                   split_offsets[],
    vx_nn_cmd_split_info_u*     info_array
    )
{
    vx_uint32 i;
    vx_uint32 blockWidth, blockHeight;
    vx_uint32 inNSize;
    vx_uint32 padXLeft, padYTop, padXRight, padYBottom;
    DEFINE_TP_GENERAL_PARAMETER();

    vxmASSERT(value_cmd_ptr != VX_NULL);

    blockWidth  = value_cmd_ptr->u32[0];
    blockHeight = value_cmd_ptr->u32[1];
    inNSize     = value_cmd_ptr->u32[2];
    /* Take input as 3D tensor. */
    padXLeft    = parameter->pad_x_left;
    padYTop     = parameter->pad_y_top;
    padXRight   = outXSize * blockWidth - inXSize - padXLeft;
    padYBottom  = outYSize * blockHeight - inYSize - padYTop;

    vxmASSERT(inZSize == outZSize);

    for (i = 0; i < split_count; i++)
    {
        info_array[i].vx_tp_general_cmd_split_info.inImageXSize = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageYSize = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageZSize = split_sizes[i];
        info_array[i].vx_tp_general_cmd_split_info.inImageStride = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageSlice = inXSize * inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXStart = (-1) * (vx_int32)padXLeft;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYStart = (-1) * (vx_int32)padYTop;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXEnd = inXSize + padXRight - 1;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYEnd = inYSize +padYBottom - 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileSequence = 0x0;
        info_array[i].vx_tp_general_cmd_split_info.inImageBaseAddress = inputBase + inXSize * inYSize * split_offsets[i] * inputElemSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileXSize = inXSize + padXLeft + padXRight;
        info_array[i].vx_tp_general_cmd_split_info.inTileYSize = inYSize + padYTop + padYBottom;
        info_array[i].vx_tp_general_cmd_split_info.inTileXInc = inXSize + padXLeft + padXRight;
        info_array[i].vx_tp_general_cmd_split_info.inTileYInc = inYSize + padYTop + padYBottom;
        info_array[i].vx_tp_general_cmd_split_info.outBaseAddress = outputBase + outXSize * outYSize * split_offsets[i] * outputElemSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc   = outXSize * outYSize * outZSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Count = blockWidth;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc   = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Count = outXSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc   = outXSize * outYSize * outZSize * blockWidth;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Count = blockHeight;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc   = outXSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Count = outYSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc   = outXSize * outYSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Count = outZSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc   = outXSize * outYSize * outZSize * blockWidth * blockHeight;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Count = inNSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc   = 0;

        info_array[i].vx_tp_general_cmd_split_info.noFlush = (i == split_count - 1 ? 0 : 1);
        info_array[i].vx_tp_general_cmd_split_info.last = 1;
    }
}

void _fill_TP_REORG_BATCH2SPACE_Command(
    vx_context                  context,
    vxnne_tensor_info           input,
    vxnne_tensor_info           output,
    vx_op_param                 parameter,
    vx_nn_cmd_info_u*           general_info,
    vx_enum                     split_type,
    vx_uint32                   split_count,
    vx_uint32                   split_sizes[],
    vx_uint32                   split_offsets[],
    vx_nn_cmd_split_info_u*     info_array
    )
{
    vx_uint32 i;
    vx_uint32 blockWidth, blockHeight;
    vx_uint32 inNSize, outNSize;
    DEFINE_TP_GENERAL_PARAMETER();

    vxmASSERT(value_cmd_ptr != VX_NULL);

    blockWidth  = value_cmd_ptr->u32[0];
    blockHeight = value_cmd_ptr->u32[1];
    inNSize     = value_cmd_ptr->u32[2];

    vxmASSERT(inNSize % (blockWidth * blockHeight) == 0);
    outNSize = inNSize / blockWidth / blockHeight;

    vxmASSERT(inZSize == outZSize);

    for (i = 0; i < split_count; i++)
    {
        info_array[i].vx_tp_general_cmd_split_info.inImageXSize = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageYSize = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageZSize = split_sizes[i];
        info_array[i].vx_tp_general_cmd_split_info.inImageStride = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageSlice = inXSize * inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXStart = 0;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYStart = 0;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXEnd = inXSize - 1;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYEnd = inYSize - 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileSequence = 0x0;
        info_array[i].vx_tp_general_cmd_split_info.inImageBaseAddress = inputBase + inXSize * inYSize * split_offsets[i] * inputElemSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileXSize = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileYSize = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileXInc = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileYInc = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.outBaseAddress = outputBase + outXSize * outYSize * split_offsets[i] * outputElemSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc   = blockWidth;
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Count = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc   = outXSize * blockHeight;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Count = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc   = outXSize * outYSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Count = inZSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc   = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Count = blockWidth;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc   = outXSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Count = blockHeight;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc   = outXSize * outYSize * outZSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Count = outNSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc   = 0;

        info_array[i].vx_tp_general_cmd_split_info.noFlush = (i == split_count - 1 ? 0 : 1);
        info_array[i].vx_tp_general_cmd_split_info.last = 1;
    }
}

void _fill_TP_ADD_Command(
    vx_context                  context,
    vxnne_tensor_info           input,
    vxnne_tensor_info           output,
    vx_op_param                 parameter,
    vx_nn_cmd_info_u*           general_info,
    vx_enum                     split_type,
    vx_uint32                   split_count,
    vx_uint32                   split_sizes[],
    vx_uint32                   split_offsets[],
    vx_nn_cmd_split_info_u*     info_array
    )
{
    vx_uint32 i;
    vx_uint32 inputBase2 = 0;
    vx_uint32 outTileXSize = 32;
    vx_uint32 outTileYSize = 1;
    DEFINE_TP_GENERAL_PARAMETER();

    vxmASSERT(other_tensor != VX_NULL);

    outXSize = inXSize;

    if (inputBase && outputBase)
    {
        inputBase2 = ((vxnne_tensor_info)other_tensor)->physical.start;
        if (inputBase2 < inputBase)
        {
            vx_uint32 tmpBase = inputBase;
            inputBase = inputBase2;
            inputBase2 = tmpBase;
        }
    }

    for (i = 0; i < split_count; i++)
    {
        outYSize = inYSize * split_sizes[i];

        info_array[i].vx_tp_general_cmd_split_info.inImageXSize = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageYSize = outYSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageZSize = 2;
        info_array[i].vx_tp_general_cmd_split_info.inImageStride = inYStride / inputElemSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageSlice = inputBase && inputBase2 ? (inputBase2 - inputBase) / inputElemSize : inputElemSize; /* change */
        info_array[i].vx_tp_general_cmd_split_info.inImageBaseAddress = inputBase + inZStride * split_offsets[i];
        info_array[i].vx_tp_general_cmd_split_info.inWindowXStart = 0;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYStart = 0;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXEnd = inXSize - 1;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYEnd = outYSize - 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileSequence = 0x1;
        info_array[i].vx_tp_general_cmd_split_info.inTileXSize = outTileXSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileYSize = outTileYSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileXInc = outTileXSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileYInc = outTileYSize;
        info_array[i].vx_tp_general_cmd_split_info.outBaseAddress = outputBase + outZStride * split_offsets[i];
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc   = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc   = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Count = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset = 0x1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc   = outXSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Count = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset = 0x1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc   = outTileXSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Count = (outXSize + outTileXSize - 1) / outTileXSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset = 0x0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc   = outTileYSize * outXSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Count = (outYSize + outTileYSize - 1) / outTileYSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc   = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc   = outZStride / outputElemSize; /* no use */

        info_array[i].vx_tp_general_cmd_split_info.noFlush = (i == split_count - 1 ? 0 : 1);
        info_array[i].vx_tp_general_cmd_split_info.last = 1;
    }
}

void _fill_TP_REVERSE_Command(
    vx_context                  context,
    vxnne_tensor_info           input,
    vxnne_tensor_info           output,
    vx_op_param                 parameter,
    vx_nn_cmd_info_u*           general_info,
    vx_enum                     split_type,
    vx_uint32                   split_count,
    vx_uint32                   split_sizes[],
    vx_uint32                   split_offsets[],
    vx_nn_cmd_split_info_u*     info_array
    )
{
    vx_uint32 i, dim = 0, outOffset = 0, max = 0x1<<16, total = 1;
    vx_uint32 dims[VX_CONTEXT_TENSOR_MAX_DIMENSION], strides[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_bool reverseArray[VX_CONTEXT_TENSOR_MAX_DIMENSION]= {vx_false_e};
    vx_uint32_ptr axis;
    vx_uint32 axisn;
    DEFINE_TP_GENERAL_PARAMETER();

    vxmASSERT(value_cmd_ptr != VX_NULL);

    axis = (vx_uint32_ptr) value_cmd_ptr->p8[0];
    axisn = value_cmd_ptr->u32[0];

    vxoTensor_GetTensorDimStride((vx_tensor)other_tensor, &dim, dims, strides);

    for (i = 0; i < axisn; i++)
    {
        vxmASSERT(axis[i] < dim);
        reverseArray[axis[i]] = vx_true_e;
    }

    for (i = 0; i < dim; i++)
    {
        if (reverseArray[i])
        {
            outOffset += (dims[i] - 1) * strides[i];
        }
        total *= dims[i];
    }

    inXSize = dims[0];
    inYSize = total / inXSize;
    for (i = 1; i < dim; i++)
    {
        if (inYSize < max)
        {
            break;
        }
        else
        {
            inXSize *= dims[i];
            inYSize /= dims[i];
        }
    }

    for (i = 0; i < split_count; i++)
    {
        info_array[i].vx_tp_general_cmd_split_info.inImageXSize = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageYSize = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageZSize = 1;
        info_array[i].vx_tp_general_cmd_split_info.inImageStride = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageSlice = total;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXStart = 0;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYStart = 0;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXEnd = inXSize - 1;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYEnd = inYSize - 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileSequence = 0x0;
        info_array[i].vx_tp_general_cmd_split_info.inImageBaseAddress = inputBase;
        info_array[i].vx_tp_general_cmd_split_info.inTileXSize = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileYSize = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileXInc = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileYInc = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.outBaseAddress = outputBase + outOffset;
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc   = reverseArray[0] ? -1 : 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Count = dims[0];
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc   = dim < 2 ? 0 : strides[1] / inputElemSize * (reverseArray[1] ? -1 : 1);
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Count = dim < 2 ? 1 : dims[1];
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc   = dim < 3 ? 0 : strides[2] / inputElemSize * (reverseArray[2] ? -1 : 1);
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Count = dim < 3 ? 1 : dims[2];
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc   = dim < 4 ? 0 : strides[3] / inputElemSize * (reverseArray[3] ? -1 : 1);
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Count = dim < 4 ? 1 : dims[3];
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc   = dim < 5 ? 0 : strides[4] / inputElemSize * (reverseArray[4] ? -1 : 1);
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Count = dim < 5 ? 1 : dims[4];
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc   = dim < 6 ? 0 : strides[5] / inputElemSize * (reverseArray[5] ? -1 : 1);
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Count = dim < 6 ? 1 : dims[5];
        info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc   = 0;

        info_array[i].vx_tp_general_cmd_split_info.noFlush = (i == split_count - 1 ? 0 : 1);
        info_array[i].vx_tp_general_cmd_split_info.last = 1;
    }
}

void _fill_TP_UPSAMPLE_Command(
    vx_context                  context,
    vxnne_tensor_info           input,
    vxnne_tensor_info           output,
    vx_op_param                 parameter,
    vx_nn_cmd_info_u*           general_info,
    vx_enum                     split_type,
    vx_uint32                   split_count,
    vx_uint32                   split_sizes[],
    vx_uint32                   split_offsets[],
    vx_nn_cmd_split_info_u*     info_array
    )
{
    vx_uint32 i;
    vx_uint32 strideX, strideY, dilate = 1;
    DEFINE_TP_GENERAL_PARAMETER();

    strideX = gcmALIGN_NP2(outXSize, inXSize)/inXSize;
    strideY = gcmALIGN_NP2(outYSize, inYSize)/inYSize;

    for (i = 0; i < split_count; i++)
    {
        info_array[i].vx_tp_general_cmd_split_info.inImageBaseAddress  = inputBase + inZStride * split_offsets[i] * strideX * strideY;
        info_array[i].vx_tp_general_cmd_split_info.outBaseAddress      = outputBase + outZStride * split_offsets[i];
        info_array[i].vx_tp_general_cmd_split_info.inImageXSize        = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageYSize        = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageZSize        = split_sizes[i] * strideX * strideY;
        info_array[i].vx_tp_general_cmd_split_info.inImageStride       = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageSlice        = inXSize * inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXStart      = 0;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYStart      = 0;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXEnd        = inXSize - 1;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYEnd        = inYSize - 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileXSize         = 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileYSize         = 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileXInc          = 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileYInc          = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc         = strideX;
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Count       = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc         = inXSize * strideX * strideY;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Count       = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset       = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc         = dilate;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Count       = strideX;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset       = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc         = inXSize * strideX;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Count       = strideY;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset       = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc         = outXSize * outYSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Count       = outZSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc         = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Count       = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc         = 0;

        info_array[i].vx_tp_general_cmd_split_info.noFlush = (i == split_count - 1 ? 0 : 1);
        info_array[i].vx_tp_general_cmd_split_info.last = 1;
    }
}

void _fill_TP_UPSAMPLE_CLIP_Command(
    vx_context                  context,
    vxnne_tensor_info           input,
    vxnne_tensor_info           output,
    vx_op_param                 parameter,
    vx_nn_cmd_info_u*           general_info,
    vx_enum                     split_type,
    vx_uint32                   split_count,
    vx_uint32                   split_sizes[],
    vx_uint32                   split_offsets[],
    vx_nn_cmd_split_info_u*     info_array
    )
{
    vx_uint32 i;
    vx_int32 pad_x_left = parameter->pad_x_left, pad_y_top = parameter->pad_y_top;
    DEFINE_TP_GENERAL_PARAMETER();

    for (i = 0; i < split_count; i++)
    {
        info_array[i].vx_tp_general_cmd_split_info.inImageBaseAddress = inputBase + inZStride * split_offsets[i];
        info_array[i].vx_tp_general_cmd_split_info.outBaseAddress = outputBase + outZStride * split_offsets[i];
        info_array[i].vx_tp_general_cmd_split_info.inImageXSize = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageYSize = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageZSize = split_sizes[i];
        info_array[i].vx_tp_general_cmd_split_info.inImageStride = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageSlice = inXSize * inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXStart = pad_x_left;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYStart = pad_y_top;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXEnd = outXSize - 1 + pad_x_left;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYEnd = outYSize - 1 + pad_y_top;
        info_array[i].vx_tp_general_cmd_split_info.inTileXSize = 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileYSize = 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileXInc = 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileYInc = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Count = outXSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc = outXSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Count = outYSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc = outXSize * outYSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Count = outZSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc = 0;

        info_array[i].vx_tp_general_cmd_split_info.noFlush = (i == split_count - 1 ? 0 : 1);
        info_array[i].vx_tp_general_cmd_split_info.last = 1;
    }
}

void _fill_TP_DILATE_UPSAMPLE_Command(
    vx_context                  context,
    vxnne_tensor_info           input,
    vxnne_tensor_info           output,
    vx_op_param                 parameter,
    vx_nn_cmd_info_u*           general_info,
    vx_enum                     split_type,
    vx_uint32                   split_count,
    vx_uint32                   split_sizes[],
    vx_uint32                   split_offsets[],
    vx_nn_cmd_split_info_u*     info_array
    )
{
    vx_uint32 i;
    vx_int32 dilationX, dilationY, stride = 1;
    vx_uint32 batch;
    DEFINE_TP_GENERAL_PARAMETER();

    dilationX = outXSize/inXSize;
    dilationY = outYSize/inYSize;
    batch = dilationX * dilationY;

    for (i = 0; i < split_count; i++)
    {
        info_array[i].vx_tp_general_cmd_split_info.inImageBaseAddress  = inputBase;
        info_array[i].vx_tp_general_cmd_split_info.outBaseAddress      = outputBase;
        info_array[i].vx_tp_general_cmd_split_info.inImageXSize        = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageYSize        = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageZSize        = inZSize * batch;
        info_array[i].vx_tp_general_cmd_split_info.inImageStride       = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageSlice        = inXSize * inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXStart      = 0;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYStart      = 0;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXEnd        = inXSize - 1;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYEnd        = inYSize - 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileXSize         = 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileYSize         = 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileXInc          = 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileYInc          = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc         = dilationX;
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Count       = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc         = inXSize * dilationX * dilationY;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Count       = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset       = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc         = (batch > 1)?(inXSize * dilationX * inYSize * dilationY):stride;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Count       = (batch > 1)?inZSize:dilationX;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset       = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc         = (batch > 1)?1:(inXSize * dilationX);
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Count       = (batch > 1)?dilationX:dilationY;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset       = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc         = (batch > 1)?inXSize * dilationX:(outXSize * outYSize);
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Count       = (batch > 1)? dilationY:outZSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc         = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Count       = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc         = 0;

        info_array[i].vx_tp_general_cmd_split_info.noFlush = (i == split_count - 1 ? 0 : 1);
        info_array[i].vx_tp_general_cmd_split_info.last = 1;
    }
}

void _fill_TP_DILATE_UPSAMPLE2_Command(
    vx_context                  context,
    vxnne_tensor_info           input,
    vxnne_tensor_info           output,
    vx_op_param                 parameter,
    vx_nn_cmd_info_u*           general_info,
    vx_enum                     split_type,
    vx_uint32                   split_count,
    vx_uint32                   split_sizes[],
    vx_uint32                   split_offsets[],
    vx_nn_cmd_split_info_u*     info_array
    )
{
    vx_uint32 i;
    DEFINE_TP_GENERAL_PARAMETER();

    for (i = 0; i < split_count; i++)
    {
        info_array[i].vx_tp_general_cmd_split_info.inImageBaseAddress  = inputBase;
        info_array[i].vx_tp_general_cmd_split_info.outBaseAddress      = outputBase;
        info_array[i].vx_tp_general_cmd_split_info.inImageXSize        = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageYSize        = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageZSize        = inZSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageStride       = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageSlice        = inXSize * inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXStart      = 0;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYStart      = 0;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXEnd        = outXSize - 1;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYEnd        = outYSize - 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileXSize         = 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileYSize         = 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileXInc          = 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileYInc          = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc         = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Count       = outXSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc         = outXSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Count       = outYSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset       = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc         = outXSize * outYSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Count       = outZSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset       = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc         = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Count       = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset       = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc         = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Count       = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc         = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Count       = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc         = 0;

        info_array[i].vx_tp_general_cmd_split_info.noFlush = (i == split_count - 1 ? 0 : 1);
        info_array[i].vx_tp_general_cmd_split_info.last = 1;
    }
}

void _fill_TP_DILATE_RESHUFFLE_Command(
    vx_context                  context,
    vxnne_tensor_info           input,
    vxnne_tensor_info           output,
    vx_op_param                 parameter,
    vx_nn_cmd_info_u*           general_info,
    vx_enum                     split_type,
    vx_uint32                   split_count,
    vx_uint32                   split_sizes[],
    vx_uint32                   split_offsets[],
    vx_nn_cmd_split_info_u*     info_array
    )
{
    vx_uint32 i;
    vx_scalar dilationX;
    vx_int32 dilate;
    DEFINE_TP_GENERAL_PARAMETER();

    dilationX = (vx_scalar)other_tensor;
    dilate = dilationX->value->n32 + 1;

    for (i = 0; i < split_count; i++)
    {
        info_array[i].vx_tp_general_cmd_split_info.inImageBaseAddress  = inputBase;
        info_array[i].vx_tp_general_cmd_split_info.outBaseAddress      = outputBase;
        info_array[i].vx_tp_general_cmd_split_info.inImageXSize        = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageYSize        = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageZSize        = inZSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageStride       = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageSlice        = inXSize * inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXStart      = 0;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYStart      = 0;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXEnd        = outXSize * dilate - 1;//inXSize - 1;/**/
        info_array[i].vx_tp_general_cmd_split_info.inWindowYEnd        = outYSize * dilate - 1;//inYSize - 1;/**/
        info_array[i].vx_tp_general_cmd_split_info.inTileXSize         = 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileYSize         = 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileXInc          = 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileYInc          = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc         = outXSize * outYSize * inZSize;/* 4*4*21=336 */
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Count       = dilate;/* 6 */
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc         = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Count       = outXSize;/* 4 * 4 */
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset       = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc         = outXSize * outYSize * inZSize * dilate;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Count       = dilate;/* 6 */
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset       = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc         = outXSize;/* 4 */
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Count       = outYSize * inZSize;/* 21 */
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset       = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc         = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Count       = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc         = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Count       = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc         = 0;

        info_array[i].vx_tp_general_cmd_split_info.noFlush = (i == split_count - 1 ? 0 : 1);
        info_array[i].vx_tp_general_cmd_split_info.last = 1;
    }
}

void _fill_TP_RNN_INTERLEAVE_Command(
    vx_context                  context,
    vxnne_tensor_info           input,
    vxnne_tensor_info           output,
    vx_op_param                 parameter,
    vx_nn_cmd_info_u*           general_info,
    vx_enum                     split_type,
    vx_uint32                   split_count,
    vx_uint32                   split_sizes[],
    vx_uint32                   split_offsets[],
    vx_nn_cmd_split_info_u*     info_array
    )
{
    vx_uint32 i;
    DEFINE_TP_GENERAL_PARAMETER();

    for (i = 0; i < split_count; i++)
    {
        info_array[i].vx_tp_general_cmd_split_info.inImageBaseAddress  = inputBase;
        info_array[i].vx_tp_general_cmd_split_info.outBaseAddress      = outputBase + parameter->pad_x_left;
        info_array[i].vx_tp_general_cmd_split_info.inImageXSize        = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageYSize        = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageZSize        = 1;
        info_array[i].vx_tp_general_cmd_split_info.inImageStride       = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageSlice        = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXStart      = 0;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYStart      = 0;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXEnd        = inXSize - 1;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYEnd        = inYSize - 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileXSize         = 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileYSize         = 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileXInc          = 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileYInc          = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc         = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Count       = inXSize;/*8*/
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc         = outZSize;/*24*/
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Count       = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset       = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc         = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Count       = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset       = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc         = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Count       = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset       = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc         = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Count       = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc         = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Count       = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc         = 0;

        info_array[i].vx_tp_general_cmd_split_info.noFlush = (i == split_count - 1 ? 0 : 1);
        info_array[i].vx_tp_general_cmd_split_info.last = 1;
    }
}

void _fill_TP_TENSOR_COPY_Command(
    vx_context                  context,
    vxnne_tensor_info           input,
    vxnne_tensor_info           output,
    vx_op_param                 parameter,
    vx_nn_cmd_info_u*           general_info,
    vx_enum                     split_type,
    vx_uint32                   split_count,
    vx_uint32                   split_sizes[],
    vx_uint32                   split_offsets[],
    vx_nn_cmd_split_info_u*     info_array
    )
{
    vx_uint32 i;
    DEFINE_TP_GENERAL_PARAMETER();

    for (i = 0; i < split_count; i++)
    {
        info_array[i].vx_tp_general_cmd_split_info.inImageXSize = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageYSize = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageZSize = split_sizes[i];
        info_array[i].vx_tp_general_cmd_split_info.inImageStride = inYStride / inputElemSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageSlice  = inZStride / inputElemSize;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXStart = 0;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYStart = 0;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXEnd = inXSize - 1;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYEnd = inYSize - 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileSequence = 0x0;
        info_array[i].vx_tp_general_cmd_split_info.inImageBaseAddress = inputBase + inZStride * split_offsets[i];
        info_array[i].vx_tp_general_cmd_split_info.inTileXSize = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileYSize = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileXInc = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileYInc = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.outBaseAddress = outputBase + outZStride * split_offsets[i];
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc   = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc   = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Count = outXSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc   = outYStride / outputElemSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Count = outYSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc   = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc   = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc   = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc   = outZStride / outputElemSize;

        info_array[i].vx_tp_general_cmd_split_info.noFlush = (i == split_count - 1 ? 0 : 1);
        info_array[i].vx_tp_general_cmd_split_info.last = 1;
    }
}

VX_PRIVATE_API void
_fill_TP_TENSOR_COPY_Command_EX(
    vx_context                  context,
    vxnne_tensor_info           input,
    vxnne_tensor_info           output,
    vx_op_param                 parameter,
    vx_nn_cmd_info_u*           general_info,
    vx_uint32                   split_count,
    vxnne_tensor_sub_block      input_split_blocks,
    vxnne_tensor_sub_block      output_split_blocks,
    vx_nn_cmd_split_info_u*     info_array
    )
{
    vx_uint32 i;
    DEFINE_TP_GENERAL_PARAMETER();

    for (i = 0; i < split_count; i++)
    {
        vx_uint32 in_offset_x = input_split_blocks[i].offset_x;
        vx_uint32 in_offset_y = input_split_blocks[i].offset_y;
        vx_uint32 in_offset_z = input_split_blocks[i].offset_z;

        vx_uint32 in_size_x = input_split_blocks[i].size_x;
        vx_uint32 in_size_y = input_split_blocks[i].size_y;
        vx_uint32 in_size_z = input_split_blocks[i].size_z;

        vx_uint32 in_pitch_x = input_split_blocks[i].pitch_x;
        vx_uint32 in_pitch_y = input_split_blocks[i].pitch_y;

        vx_uint32 out_offset_x = output_split_blocks[i].offset_x;
        vx_uint32 out_offset_y = output_split_blocks[i].offset_y;
        vx_uint32 out_offset_z = output_split_blocks[i].offset_z;

        vx_uint32 out_size_x = output_split_blocks[i].size_x;
        vx_uint32 out_size_y = output_split_blocks[i].size_y;

        vx_uint32 out_pitch_x = output_split_blocks[i].pitch_x;
        vx_uint32 out_pitch_y = output_split_blocks[i].pitch_y;

        info_array[i].vx_tp_general_cmd_split_info.inImageXSize = in_size_x;
        info_array[i].vx_tp_general_cmd_split_info.inImageYSize = in_size_y;
        info_array[i].vx_tp_general_cmd_split_info.inImageZSize = in_size_z;
        info_array[i].vx_tp_general_cmd_split_info.inImageStride = in_pitch_x;
        info_array[i].vx_tp_general_cmd_split_info.inImageSlice  = in_pitch_x * in_pitch_y;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXStart = 0;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYStart = 0;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXEnd = in_size_x - 1;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYEnd = in_size_y - 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileSequence = 0x0;
        info_array[i].vx_tp_general_cmd_split_info.inImageBaseAddress = inputBase + (in_pitch_x * in_pitch_y * in_offset_z + in_pitch_x * in_offset_y + in_offset_x) * inputElemSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileXSize = in_size_x;
        info_array[i].vx_tp_general_cmd_split_info.inTileYSize = in_size_y;
        info_array[i].vx_tp_general_cmd_split_info.inTileXInc = in_size_x;
        info_array[i].vx_tp_general_cmd_split_info.inTileYInc = in_size_y;
        info_array[i].vx_tp_general_cmd_split_info.outBaseAddress = outputBase + (out_pitch_x * out_pitch_y * out_offset_z + out_pitch_x * out_offset_y + out_offset_x) * outputElemSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc   = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc   = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Count = out_size_x;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc   = out_pitch_x;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Count = out_size_y;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc   = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc   = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc   = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc   = out_pitch_x * out_pitch_y;

        info_array[i].vx_tp_general_cmd_split_info.noFlush = (i == split_count - 1 ? 0 : 1);
        info_array[i].vx_tp_general_cmd_split_info.last = 1;
    }
}

void _fill_TP_TENSOR_PAD_Command(
    vx_context                  context,
    vxnne_tensor_info           input,
    vxnne_tensor_info           output,
    vx_op_param                 parameter,
    vx_nn_cmd_info_u*           general_info,
    vx_enum                     split_type,
    vx_uint32                   split_count,
    vx_uint32                   split_sizes[],
    vx_uint32                   split_offsets[],
    vx_nn_cmd_split_info_u*     info_array
    )
{
    vx_uint32 i;
    DEFINE_TP_GENERAL_PARAMETER();

    for (i = 0; i < split_count; i++)
    {
        info_array[i].vx_tp_general_cmd_split_info.inImageXSize = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageYSize = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageZSize = split_sizes[i];
        info_array[i].vx_tp_general_cmd_split_info.inImageStride = inYStride / inputElemSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageSlice  = inZStride / inputElemSize;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXStart = (-1) * (vx_int32)parameter->pad_x_left;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYStart = (-1) * (vx_int32)parameter->pad_y_top;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXEnd = info_array[i].vx_tp_general_cmd_split_info.inWindowXStart + outXSize - 1;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYEnd = info_array[i].vx_tp_general_cmd_split_info.inWindowYStart + outYSize - 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileSequence = 0x0;
        info_array[i].vx_tp_general_cmd_split_info.inImageBaseAddress = inputBase + inZStride * split_offsets[i];
        info_array[i].vx_tp_general_cmd_split_info.inTileXSize = outXSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileYSize = outYSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileXInc = outXSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileYInc = outYSize;
        info_array[i].vx_tp_general_cmd_split_info.outBaseAddress = outputBase + outZStride * split_offsets[i];
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc   = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc   = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Count = outXSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc   = outXSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Count = outYSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc   = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc   = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc   = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc   = outZStride / outputElemSize;

        info_array[i].vx_tp_general_cmd_split_info.noFlush = (i == split_count - 1 ? 0 : 1);
        info_array[i].vx_tp_general_cmd_split_info.last = 1;
    }
}

void _fill_TP_TENSOR_SQUEEZE_Command(
    vx_context                  context,
    vxnne_tensor_info           input,
    vxnne_tensor_info           output,
    vx_op_param                 parameter,
    vx_nn_cmd_info_u*           general_info,
    vx_enum                     split_type,
    vx_uint32                   split_count,
    vx_uint32                   split_sizes[],
    vx_uint32                   split_offsets[],
    vx_nn_cmd_split_info_u*     info_array
    )
{
    vx_uint32 i;
    DEFINE_TP_GENERAL_PARAMETER();

    for (i = 0; i < split_count; i++)
    {
        info_array[i].vx_tp_general_cmd_split_info.inImageXSize = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageYSize = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageZSize = split_sizes[i];
        info_array[i].vx_tp_general_cmd_split_info.inImageStride = inYStride / inputElemSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageSlice = inZStride / inputElemSize;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXStart = 0;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYStart = 0;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXEnd = inXSize - 1;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYEnd = inYSize - 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileSequence = 0x0;
        info_array[i].vx_tp_general_cmd_split_info.inImageBaseAddress = inputBase + inZStride * split_offsets[i];
        info_array[i].vx_tp_general_cmd_split_info.inTileXSize = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileYSize = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileXInc = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileYInc = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.outBaseAddress = outputBase + inZStride * split_offsets[i];
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Count = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Count = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc = inZStride / inputElemSize;

        info_array[i].vx_tp_general_cmd_split_info.noFlush = (i == split_count - 1 ? 0 : 1);
        info_array[i].vx_tp_general_cmd_split_info.last = 1;
    }
}

void _fill_TP_TENSOR_STRIDED_SLICE_Command(
    vx_context                  context,
    vxnne_tensor_info           input,
    vxnne_tensor_info           output,
    vx_op_param                 parameter,
    vx_nn_cmd_info_u*           general_info,
    vx_enum                     split_type,
    vx_uint32                   split_count,
    vx_uint32                   split_sizes[],
    vx_uint32                   split_offsets[],
    vx_nn_cmd_split_info_u*     info_array
    )
{
    vx_uint32 i;
    vx_int32 begin_dims[2] = { parameter->tp_value->u32[0], parameter->tp_value->u32[1]};
    vx_int32 end_dims[2] = { parameter->tp_value->u32[2], parameter->tp_value->u32[3] };
    vx_int32 stride_dims[2] = { parameter->tp_value->u32[4], parameter->tp_value->u32[5] };
    DEFINE_TP_GENERAL_PARAMETER();

    for (i = 0; i < split_count; i++)
    {
        info_array[i].vx_tp_general_cmd_split_info.inImageXSize = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageYSize = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageZSize = split_sizes[i];
        info_array[i].vx_tp_general_cmd_split_info.inImageStride = inYStride / inputElemSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageSlice = inZStride / inputElemSize;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXStart = gcmMAX(0, begin_dims[0]);
        info_array[i].vx_tp_general_cmd_split_info.inWindowYStart = gcmMAX(0, begin_dims[1]);
        info_array[i].vx_tp_general_cmd_split_info.inWindowXEnd = gcmMIN((vx_int32)inXSize - 1, end_dims[0] - 1);
        info_array[i].vx_tp_general_cmd_split_info.inWindowYEnd = gcmMIN((vx_int32)inYSize - 1, end_dims[1] - 1);
        info_array[i].vx_tp_general_cmd_split_info.inTileSequence = 0x0;
        info_array[i].vx_tp_general_cmd_split_info.inImageBaseAddress = inputBase + inZStride * split_offsets[i];
        if (stride_dims[0] == 1)
        {
            info_array[i].vx_tp_general_cmd_split_info.inTileXSize = info_array[i].vx_tp_general_cmd_split_info.inWindowXEnd -
                                                                     info_array[i].vx_tp_general_cmd_split_info.inWindowXStart + 1;
        }
        else
        {
            info_array[i].vx_tp_general_cmd_split_info.inTileXSize = 1;
        }
        if (stride_dims[1] == 1)
        {
            info_array[i].vx_tp_general_cmd_split_info.inTileYSize = info_array[i].vx_tp_general_cmd_split_info.inWindowYEnd -
                                                                     info_array[i].vx_tp_general_cmd_split_info.inWindowYStart + 1;
        }
        else
        {
            info_array[i].vx_tp_general_cmd_split_info.inTileYSize = 1;
        }
        info_array[i].vx_tp_general_cmd_split_info.inTileXInc = stride_dims[0];
        info_array[i].vx_tp_general_cmd_split_info.inTileYInc = stride_dims[1];
        info_array[i].vx_tp_general_cmd_split_info.outBaseAddress = outputBase + inZStride * split_offsets[i];
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc   = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc   = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Count = outXSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc   = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc   = outYStride / outputElemSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Count = outYSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc   = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc   = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc   = outZStride / outputElemSize;

        info_array[i].vx_tp_general_cmd_split_info.noFlush = (i == split_count - 1 ? 0 : 1);
        info_array[i].vx_tp_general_cmd_split_info.last = 1;
    }
}

void _fill_TP_TENSOR_SVDF_MAP_Command(
    vx_context                  context,
    vxnne_tensor_info           input,
    vxnne_tensor_info           output,
    vx_op_param                 parameter,
    vx_nn_cmd_info_u*           general_info,
    vx_enum                     split_type,
    vx_uint32                   split_count,
    vx_uint32                   split_sizes[],
    vx_uint32                   split_offsets[],
    vx_nn_cmd_split_info_u*     info_array
    )
{
    vx_uint32 i;
    vx_int32 stride;
    DEFINE_TP_GENERAL_PARAMETER();

    stride = outXSize / inZSize;

    for (i = 0; i < split_count; i++)
    {
        info_array[i].vx_tp_general_cmd_split_info.inImageXSize = inZSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageYSize = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageZSize = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageStride = inZSize / inputElemSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageSlice = inZSize / inputElemSize;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXStart = 0;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYStart = 0;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXEnd = inZSize - 1;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYEnd = inYSize - 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileSequence = 0x0;
        info_array[i].vx_tp_general_cmd_split_info.inImageBaseAddress = inputBase + inZStride * split_offsets[i];
        info_array[i].vx_tp_general_cmd_split_info.inTileXSize = inZSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileYSize = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileXInc = inZSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileYInc = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.outBaseAddress = outputBase + (stride - 1) * inputElemSize + inZStride * split_offsets[i];
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc = stride;
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Count = inZSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc = 1;

        info_array[i].vx_tp_general_cmd_split_info.noFlush = (i == split_count - 1 ? 0 : 1);
        info_array[i].vx_tp_general_cmd_split_info.last = 1;
    }
}

void _fill_TP_LSTM_RESHUFFLE_INPUT_Command(
    vx_context                  context,
    vxnne_tensor_info           input,
    vxnne_tensor_info           output,
    vx_op_param                 parameter,
    vx_nn_cmd_info_u*           general_info,
    vx_enum                     split_type,
    vx_uint32                   split_count,
    vx_uint32                   split_sizes[],
    vx_uint32                   split_offsets[],
    vx_nn_cmd_split_info_u*     info_array
    )
{
    vx_uint32 i;
    vx_int32 inXSize2 = TENSOR_SIZE_INDEX((vx_tensor)parameter->other_ref, 0);
    DEFINE_TP_GENERAL_PARAMETER();

    for (i = 0; i < split_count; i++)
    {
        info_array[i].vx_tp_general_cmd_split_info.inImageBaseAddress  = inputBase + inZStride * split_offsets[i];
        info_array[i].vx_tp_general_cmd_split_info.outBaseAddress      = outputBase + outZStride * split_offsets[i];
        info_array[i].vx_tp_general_cmd_split_info.inImageXSize        = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageYSize        = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageZSize        = split_sizes[i];
        info_array[i].vx_tp_general_cmd_split_info.inImageStride       = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageSlice        = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXStart      = 0;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYStart      = 0;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXEnd        = inXSize - 1;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYEnd        = inYSize - 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileXSize         = 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileYSize         = 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileXInc          = 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileYInc          = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc         = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Count       = inXSize + inXSize2;    /* 2 + 4 */
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc         = outZSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Count       = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset       = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc         = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Count       = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset       = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc         = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Count       = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset       = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc         = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Count       = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc         = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Count       = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc         = 0;

        info_array[i].vx_tp_general_cmd_split_info.noFlush = (i == split_count - 1 ? 0 : 1);
        info_array[i].vx_tp_general_cmd_split_info.last = 1;
    }
}

void _fill_TP_LSTM_STATE_OUT_Command(
    vx_context                  context,
    vxnne_tensor_info           input,
    vxnne_tensor_info           output,
    vx_op_param                 parameter,
    vx_nn_cmd_info_u*           general_info,
    vx_enum                     split_type,
    vx_uint32                   split_count,
    vx_uint32                   split_sizes[],
    vx_uint32                   split_offsets[],
    vx_nn_cmd_split_info_u*     info_array
    )
{
    vx_uint32 i;
    vx_int32 inXSize2 = TENSOR_SIZE_INDEX((vx_tensor)parameter->other_ref, 0);
    DEFINE_TP_GENERAL_PARAMETER();

    for (i = 0; i < split_count; i++)
    {
        info_array[i].vx_tp_general_cmd_split_info.inImageBaseAddress  = inputBase + inZStride * split_offsets[i];
        info_array[i].vx_tp_general_cmd_split_info.outBaseAddress      = outputBase+ inXSize2 * vxnneGetTypeSize(input->dataFormat) + outZStride * split_offsets[i];
        info_array[i].vx_tp_general_cmd_split_info.inImageXSize        = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageYSize        = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageZSize        = split_sizes[i];
        info_array[i].vx_tp_general_cmd_split_info.inImageStride       = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageSlice        = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXStart      = 0;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYStart      = 0;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXEnd        = inXSize - 1;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYEnd        = inYSize - 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileXSize         = 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileYSize         = 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileXInc          = 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileYInc          = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc         = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Count       = inXSize + inXSize2;    /* 2 + 4 */
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc         = outZSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Count       = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset       = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc         = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Count       = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset       = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc         = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Count       = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset       = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc         = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Count       = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc         = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Count       = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc         = 0;

        info_array[i].vx_tp_general_cmd_split_info.noFlush = (i == split_count - 1 ? 0 : 1);
        info_array[i].vx_tp_general_cmd_split_info.last = 1;
    }
}

void _fill_TP_TENSOR_COPY4CONCAT_Command(
    vx_context                  context,
    vxnne_tensor_info           input,
    vxnne_tensor_info           output,
    vx_op_param                 parameter,
    vx_nn_cmd_info_u*           general_info,
    vx_enum                     split_type,
    vx_uint32                   split_count,
    vx_uint32                   split_sizes[],
    vx_uint32                   split_offsets[],
    vx_nn_cmd_split_info_u*     info_array
    )
{
    vx_uint32 i;
    DEFINE_TP_GENERAL_PARAMETER();

    for (i = 0; i < split_count; i++)
    {
        info_array[i].vx_tp_general_cmd_split_info.inImageXSize = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageYSize = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageZSize = split_sizes[i];
        info_array[i].vx_tp_general_cmd_split_info.inImageStride = inYStride / inputElemSize;
        info_array[i].vx_tp_general_cmd_split_info.inImageSlice  = inZStride / inputElemSize;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXStart = 0;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYStart = 0;
        info_array[i].vx_tp_general_cmd_split_info.inWindowXEnd = inXSize - 1;
        info_array[i].vx_tp_general_cmd_split_info.inWindowYEnd = inYSize - 1;
        info_array[i].vx_tp_general_cmd_split_info.inTileSequence = 0x0;
        info_array[i].vx_tp_general_cmd_split_info.inImageBaseAddress = inputBase + inZStride * split_offsets[i];
        info_array[i].vx_tp_general_cmd_split_info.inTileXSize = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileYSize = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileXInc = inXSize;
        info_array[i].vx_tp_general_cmd_split_info.inTileYInc = inYSize;
        info_array[i].vx_tp_general_cmd_split_info.outBaseAddress = outputBase + outZStride * split_offsets[i];
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Inc   = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop0Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Inc   = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Count = outXSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop1Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Inc   = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop2Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Inc   = outYStride / outputElemSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Count = outYSize;
        info_array[i].vx_tp_general_cmd_split_info.outLoop3Reset = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Inc   = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop4Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Inc   = 0;
        info_array[i].vx_tp_general_cmd_split_info.outLoop5Count = 1;
        info_array[i].vx_tp_general_cmd_split_info.outLoop6Inc   = outZStride / outputElemSize;

        info_array[i].vx_tp_general_cmd_split_info.noFlush = (i == split_count - 1 ? 0 : 1);
        info_array[i].vx_tp_general_cmd_split_info.last = 1;
    }
}

/*
 * Split the tensor on X, Y and Z axises.
 */
VX_PRIVATE_API vx_status
_CalculateSplitSizes(vxnne_tensor_info input,
                     vxnne_tensor_info output,
                     vx_op_param parameter,
                     vx_uint32 div_x,
                     vx_uint32 div_y,
                     vx_uint32 div_z,
                     vxnne_tensor_sub_block input_splits,
                     vxnne_tensor_sub_block output_splits
                     )
{
    vx_weights_biases_parameter weights_biases = (vx_weights_biases_parameter)parameter->other_ref;
    vx_uint32 stride_x = 1, stride_y = 1;
    vx_bool optimization_for_1x1_conv = vx_false_e;
    vx_uint32 x, y, z;
    vx_uint32 input_element_size = vxnneGetTypeSize(input->dataFormat);
    vx_uint32 output_element_size = vxnneGetTypeSize(output->dataFormat);
    vx_uint32 input_pitch_x = input->yStride / input_element_size;
    vx_uint32 input_pitch_y = input->zStride / input->yStride;
    vx_uint32 output_center_unit_size_x = 0, output_center_unit_size_y = 0, output_unit_z;
    vx_uint32 output_remainder_size_x = 0, output_remainder_size_y = 0, output_remainder_z;
    vx_uint32 output_pitch_x = output->yStride / output_element_size;
    vx_uint32 output_pitch_y = output->zStride / output->yStride;
    vx_uint32 inferred_input_size_x, inferred_input_size_y;
    vx_uint32 output_left_unit_size_x = 0;
    vx_uint32 output_right_unit_size_x = 0;
    vx_uint32 output_top_unit_size_y = 0;
    vx_uint32 output_bottom_unit_size_y = 0;
    vx_uint32 pad_left, pad_right, pad_top, pad_bottom;
    vx_uint32 index, last_x_index, last_y_index, last_z_index;

    if (weights_biases)
    {
        stride_x = WB_STRIDE_X(weights_biases);
        stride_y = WB_STRIDE_Y(weights_biases);

        optimization_for_1x1_conv = WB_KERNEL_X(weights_biases) == 1 &&
                                    WB_KERNEL_Y(weights_biases) == 1 &&
                                    WB_ORG_KERNEL_X(weights_biases) == 1 &&
                                    WB_ORG_KERNEL_Y(weights_biases) == 1;
    }

    inferred_input_size_x = output->width * stride_x;
    inferred_input_size_y = output->height * stride_y;

    pad_left = parameter->pad_x_left;
    pad_top = parameter->pad_y_top;
    pad_right = (inferred_input_size_x > parameter->pad_x_left + input->width) ?
                inferred_input_size_x - (parameter->pad_x_left + input->width) : 0;
    pad_bottom = (inferred_input_size_y > parameter->pad_y_top + input->height) ?
                 inferred_input_size_y - (parameter->pad_y_top + input->height) : 0;

    output_center_unit_size_x = output->width / div_x;
    output_remainder_size_x = output->width % div_x;
    if ((output_center_unit_size_x +  (output_remainder_size_x ? 1 : 0)) * stride_x <= pad_left)
    {
        output_left_unit_size_x = (pad_left + 1 + stride_x - 1) / stride_x;
    }
    else
    {
        output_left_unit_size_x = output_center_unit_size_x + (output_remainder_size_x ? 1 : 0);
    }

    if (div_x == 2)
    {
        vxmASSERT(pad_right < output->width * stride_x / 2);
        output_right_unit_size_x = output->width - output_left_unit_size_x;

        output_center_unit_size_x = 0;
        output_remainder_size_x = 0;
    }
    else if (div_x > 2)
    {
        if (output_center_unit_size_x * stride_x <= pad_right)
        {
            output_right_unit_size_x = (pad_right + 1 + stride_x - 1) / stride_x;
        }
        else
        {
            output_right_unit_size_x = output_center_unit_size_x;
        }

        output_center_unit_size_x = (output->width - output_left_unit_size_x - output_right_unit_size_x) / (div_x - 2);
        output_remainder_size_x = (output->width - output_left_unit_size_x - output_right_unit_size_x) % (div_x - 2);
    }

    output_center_unit_size_y = output->height / div_y;
    output_remainder_size_y = output->height % div_y;
    if ((output_center_unit_size_y +  (output_remainder_size_y ? 1 : 0)) * stride_y <= pad_top)
    {
        output_top_unit_size_y = (pad_top + 1 + stride_y - 1) / stride_y;
    }
    else
    {
        output_top_unit_size_y = output_center_unit_size_y + (output_remainder_size_y ? 1 : 0);
    }

    if (div_y == 2)
    {
        vxmASSERT(pad_bottom < output->height * stride_y / 2);
        output_bottom_unit_size_y = output->height - output_top_unit_size_y;

        output_center_unit_size_y = 0;
        output_remainder_size_y = 0;
    }
    else if (div_y > 2)
    {
        if (output_center_unit_size_y * stride_y <= pad_bottom)
        {
            output_bottom_unit_size_y = (pad_bottom + 1 + stride_y - 1) / stride_y;
        }
        else
        {
            output_bottom_unit_size_y = output_center_unit_size_y;
        }

        output_center_unit_size_y = (output->height - output_top_unit_size_y - output_bottom_unit_size_y) / (div_y - 2);
        output_remainder_size_y = (output->height - output_top_unit_size_y - output_bottom_unit_size_y) % (div_y - 2);
    }

    output_unit_z = (output->depth / stride_x / stride_y) / div_z;
    output_remainder_z = (output->depth / stride_x / stride_y) % div_z;

    for (z = 0; z < div_z; z++)
    {
        for (y = 0; y < div_y; y++)
        {
            for (x = 0; x < div_x; x++)
            {
                index = div_x * div_y * z + div_x * y + x;
                last_x_index = div_x * div_y * z + div_x * y + x - 1;
                last_y_index = div_x * div_y * z + div_x * (y - 1) + x;
                last_z_index = div_x * div_y * (z - 1) + div_x * y + x;

                /* Split on X axis. */
                if (x == 0)
                {
                    input_splits[index].offset_x = 0;
                    input_splits[index].pad_left = pad_left;

                    output_splits[index].offset_x = 0;
                    output_splits[index].size_x = output_left_unit_size_x;
                }
                else
                {
                    input_splits[index].offset_x = input_splits[last_x_index].offset_x +
                                                   input_splits[last_x_index].size_x;
                    input_splits[index].pad_left = 0;

                    output_splits[index].offset_x = output_splits[last_x_index].offset_x +
                                                    output_splits[last_x_index].size_x;
                    if (x + 1== div_x)
                    {
                        output_splits[index].size_x = output_right_unit_size_x;
                    }
                    else
                    {
                        output_splits[index].size_x = output_center_unit_size_x + ((x - 1 < output_remainder_size_x) ? 1 : 0);
                    }
                }

                if (x + 1 == div_x)
                {
                    /* Last block on X axis. */
                    input_splits[index].pad_right = pad_right;
                    vxmASSERT(input->width > input_splits[index].offset_x);
                    input_splits[index].size_x = input->width - input_splits[index].offset_x;
                }
                else
                {
                    input_splits[index].pad_right = 0;
                    vxmASSERT(output_splits[index].size_x * stride_x > input_splits[index].pad_left);
                    input_splits[index].size_x = output_splits[index].size_x * stride_x -
                                                 input_splits[index].pad_left;
                }

                input_splits[index].pitch_x = input_pitch_x;
                output_splits[index].pitch_x = output_pitch_x;

                /* Split on Y axis. */
                if (y == 0)
                {
                    input_splits[index].offset_y = 0;
                    input_splits[index].pad_top = pad_top;

                    output_splits[index].offset_y = 0;
                    output_splits[index].size_y = output_top_unit_size_y;
                }
                else
                {
                    input_splits[index].offset_y = input_splits[last_y_index].offset_y +
                                                   input_splits[last_y_index].size_y;
                    input_splits[index].pad_top = 0;

                    output_splits[index].offset_y = output_splits[last_y_index].offset_y +
                                                    output_splits[last_y_index].size_y;

                    if (y + 1 == div_y)
                    {
                        output_splits[index].size_y = output_bottom_unit_size_y;
                    }
                    else
                    {
                        output_splits[index].size_y = output_center_unit_size_y + ((y - 1 < output_remainder_size_y) ? 1 : 0);
                    }
                }

                if (y + 1 == div_y)
                {
                    /* Last block on Y axis. */
                    input_splits[index].pad_bottom = pad_bottom;
                    vxmASSERT(input->height > input_splits[index].offset_y);
                    input_splits[index].size_y = input->height - input_splits[index].offset_y;
                }
                else
                {
                    input_splits[index].pad_bottom = 0;
                    vxmASSERT(output_splits[index].size_y * stride_y > input_splits[index].pad_top);
                    input_splits[index].size_y = output_splits[index].size_y * stride_y -
                                                 input_splits[index].pad_top;
                }

                input_splits[index].pitch_y = input_pitch_y;
                output_splits[index].pitch_y = output_pitch_y;

                /* Split on Z axis. */
                if (z == 0)
                {
                    input_splits[index].offset_z = 0;
                    output_splits[index].offset_z = 0;
                }
                else
                {
                    input_splits[index].offset_z = input_splits[last_z_index].offset_z +
                                                   input_splits[last_z_index].size_z;

                    if (optimization_for_1x1_conv)
                    {
                        output_splits[index].offset_z = output_splits[last_z_index].offset_z +
                                                        input_splits[last_z_index].size_z;
                    }
                    else
                    {
                        output_splits[index].offset_z = output_splits[last_z_index].offset_z +
                                                        output_splits[last_z_index].size_z;
                    }
                }

                output_splits[index].size_z = (output_unit_z + ((z < output_remainder_z) ? 1 : 0)) * stride_x * stride_y;

                if (z + 1 == div_z)
                {
                    input_splits[index].size_z = input->depth - input_splits[index].offset_z;
                }
                else
                {
                    input_splits[index].size_z = output_splits[index].size_z / stride_x / stride_y;
                }
            }
        }
    }

#if gcdDEBUG && SHOW_TP_SPLITING_INFO
    gcmPRINT("Splinting info:\n");

    gcmPRINT("div: %u, %u, %u\n", div_x, div_y, div_z);
    gcmPRINT("padding: %u, %u, %u, %u\n",
             parameter->pad_x_left,
             parameter->pad_x_right,
             parameter->pad_y_top,
             parameter->pad_y_bottom);
    gcmPRINT("stride x: %u\n", stride_x);
    gcmPRINT("stride y: %u\n", stride_y);

    gcmPRINT("input size: %u, %u, %u\n",
             input->width,
             input->height,
             input->depth);
    gcmPRINT("output size: %u, %u, %u\n",
             output->width,
             output->height,
             output->depth);

    for (z = 0; z < div_z; z++)
    {
        for (y = 0; y < div_y; y++)
        {
            for (x = 0; x < div_x; x++)
            {
                index = div_x * div_y * z + div_x * y + x;

                gcmPRINT("input[%u][%u][%u]:\n", z, y, x);

                gcmPRINT("    offset: %u, %u, %u\n",
                         input_splits[index].offset_x,
                         input_splits[index].offset_y,
                         input_splits[index].offset_z);
                gcmPRINT("    size: %u, %u, %u\n",
                         input_splits[index].size_x,
                         input_splits[index].size_y,
                         input_splits[index].size_z);
                gcmPRINT("    pitch: %u, %u\n",
                         input_splits[index].pitch_x,
                         input_splits[index].pitch_y);
                gcmPRINT("    padding: l %u, r %u, t %u, b %u\n",
                         input_splits[index].pad_left,
                         input_splits[index].pad_right,
                         input_splits[index].pad_top,
                         input_splits[index].pad_bottom);

                gcmPRINT("output[%u][%u][%u]:\n", z, y, x);

                gcmPRINT("    offset: %u, %u, %u\n",
                         output_splits[index].offset_x,
                         output_splits[index].offset_y,
                         output_splits[index].offset_z);
                gcmPRINT("    size: %u, %u, %u\n",
                         output_splits[index].size_x,
                         output_splits[index].size_y,
                         output_splits[index].size_z);
                gcmPRINT("    pitch: %u, %u\n",
                         output_splits[index].pitch_x,
                         output_splits[index].pitch_y);
            }
        }
    }
#endif

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status
_SplitInputAndOutputForMultiTPCores(vx_context context,
                                    vxnne_tensor_info input,
                                    vxnne_tensor_info output,
                                    vx_op_param parameter,
                                    vx_uint32 *split_count,
                                    vxnne_tensor_sub_block *input_splits,
                                    vxnne_tensor_sub_block *output_splits
                                    )
{
    vx_status status = VX_SUCCESS;

    vx_enum tp_type = parameter->tpType;
    vx_weights_biases_parameter weights_biases = (vx_weights_biases_parameter)parameter->other_ref;
    vx_uint32 stride_x = weights_biases ? WB_STRIDE_X(weights_biases) : 1;
    vx_uint32 stride_y = weights_biases ? WB_STRIDE_Y(weights_biases) : 1;
    vx_uint32 output_size_x = output->width;
    vx_uint32 output_size_y = output->height;
    vx_uint32 output_size_z = output->depth;

    vx_uint32 num_slice;
    vx_uint32 core = tp_type != TP_SINGLE_FC ? context->nnConfig.fixedFeature.tpCoreCount :
                                               context->nnConfig.fixedFeature.tpCoreCount + context->nnConfig.fixedFeature.tpliteCoreCount;
    vx_bool mult = context->options.enableMultiTP && core > 1;
    vx_uint32 pad_left = parameter->pad_x_left;
    vx_uint32 pad_right = output->width * stride_x - pad_left - input->width;
    vx_uint32 pad_top = parameter->pad_y_top;
    vx_uint32 pad_bottom = output->height * stride_y - pad_top - input->height;
    vx_uint32 div_x = 1, div_y = 1, div_z = 1;
    vxnne_tensor_sub_block splits_of_input = VX_NULL;
    vxnne_tensor_sub_block splits_of_output = VX_NULL;

    /* Decide the slice num. */
    switch (tp_type)
    {
        case TP_RESHUFFLE:
        case TP_TENSOR_COPY:
        case TP_TENSOR_COPY4CONCAT:
        {
            /* Don't split if the size is too small. */
            if (mult && (output_size_x * output_size_y * output_size_z) > MULTI_TP_RESHUFFLE_SIZE)
            {
                num_slice = core;
            }
            else
            {
                num_slice = 1;
            }

            if ((output_size_z / stride_x / stride_y) % num_slice == 0)
            {
                div_z = num_slice;
            }
            else if (output_size_y % num_slice == 0 &&
                     output_size_y / num_slice > pad_top &&
                     output_size_y / num_slice > pad_bottom)
            {
                div_y = num_slice;
            }
            else if (output_size_x % num_slice == 0 &&
                     output_size_x / num_slice > pad_left &&
                     output_size_x / num_slice > pad_right)
            {
                div_x = num_slice;
            }
            else
            {
                vx_uint32 max = gcmMAX(gcmMAX(output_size_x * stride_x, output_size_y * stride_y), output_size_z / stride_x / stride_y);

                if (output_size_z / stride_x / stride_y == max)
                {
                    div_z = gcmMIN(core, num_slice);
                }
                else if (output_size_y * stride_y == max)
                {
                    div_y = gcmMIN(core, num_slice);
                }
                else
                {
                    div_x = gcmMIN(core, num_slice);
                }
            }

            break;
        }


        default:
        {
            vxmASSERT("TODO: Shouldn't go here at the moment." && 0);
            num_slice = 1;
            break;
        }
    }

    splits_of_input = (vxnne_tensor_sub_block)vxAllocateAndZeroMemory(gcmSIZEOF(vxnne_tensor_sub_block_s) * (div_x * div_y * div_z));
    if (!splits_of_input)
    {
        vxmONERROR(VX_ERROR_NO_MEMORY);
    }

    splits_of_output = (vxnne_tensor_sub_block)vxAllocateAndZeroMemory(gcmSIZEOF(vxnne_tensor_sub_block_s) * (div_x * div_y * div_z));
    if (!splits_of_output)
    {
        vxmONERROR(VX_ERROR_NO_MEMORY);
    }

    vxmONERROR(_CalculateSplitSizes(input,
                                    output,
                                    parameter,
                                    div_x,
                                    div_y,
                                    div_z,
                                    splits_of_input,
                                    splits_of_output
                                    ));


    *split_count = num_slice;
    *input_splits = splits_of_input;
    *output_splits = splits_of_output;

    return VX_SUCCESS;

OnError:
    if (splits_of_input)
    {
        vxFree(splits_of_input);
    }

    if (splits_of_output)
    {
        vxFree(splits_of_output);
    }

    return status;
}

VX_PRIVATE_API vx_status vxnneCommandBuffer_GetTPSplitCommandInfo(
    vx_context                   context,
    vxnne_tensor_info            input,
    vxnne_tensor_info            output,
    vx_op_param                  parameter,
    vx_nn_cmd_info_u *           info_ptr,
    vx_nn_cmd_split_info_u *     *sinfo_array_ptr,
    vx_uint32 *                  sinfo_num_ptr
    )
{
    vx_nn_cmd_split_info_u * sinfoArray;
    vx_enum tpType, splitTypes[TP_TENSOR_COUNT] = {TP_SPLIT_Z_DIRECTION};

    vx_uint32 i, splitCount = 1;
    vx_uint32 splitSizes[TP_TENSOR_COUNT] = {0};
    vx_uint32 splitOffsets[TP_TENSOR_COUNT] = {0};
    vxnne_tensor_sub_block input_splits = VX_NULL;
    vxnne_tensor_sub_block output_splits = VX_NULL;
    vx_uint32 core;

    vxmASSERT(parameter != VX_NULL);

    tpType = parameter->tpType;

    core = tpType != TP_SINGLE_FC ? context->nnConfig.fixedFeature.tpCoreCount :
                                    context->nnConfig.fixedFeature.tpCoreCount + context->nnConfig.fixedFeature.tpliteCoreCount;

    /* TODO: Change if optimized */
    switch (tpType)
    {
    case TP_RESHUFFLE:
    case TP_TENSOR_COPY:
    case TP_TENSOR_COPY4CONCAT:
        if ((vx_int32)parameter->pad_y_top == -1)
        {
            /*
             * SWTiling subimage uses -1 of pad_y_top to
             * make hardware just output pads if the whole
             * subimage is in the padding area.
             */
            _calculateTPSplitSizeOffset(context,
                                        input,
                                        output,
                                        parameter,
                                        splitTypes[tpType],
                                        &splitCount,
                                        splitSizes,
                                        splitOffsets);
        }
        else
        {
            _SplitInputAndOutputForMultiTPCores(context,
                                                input,
                                                output,
                                                parameter,
                                                &splitCount,
                                                &input_splits,
                                                &output_splits
                                                );
        }
        break;

    default:
        _calculateTPSplitSizeOffset(context, input, output, parameter, splitTypes[tpType], &splitCount, splitSizes, splitOffsets);
        break;
    }

    sinfoArray = vxAllocateAndZeroMemory(sizeof(vx_nn_cmd_split_info_u) * splitCount);
    if (sinfoArray == VX_NULL) return VX_ERROR_NO_MEMORY;

    switch (tpType)
    {
        case TP_RESHUFFLE:
        {
            if ((vx_int32)parameter->pad_y_top == -1)
            {
                /*
                 * SWTiling subimage uses -1 of pad_y_top to
                 * make hardware just output pads if the whole
                 * subimage is in the padding area.
                 */
                FILL_TP_COMMAND(context, input, output, parameter, splitTypes[tpType], splitCount, splitSizes, splitOffsets, sinfoArray, info_ptr, TP_RESHUFFLE);
            }
            else
            {
                _fill_TP_RESHUFFLE_Command_EX(context,
                                              input,
                                              output,
                                              parameter,
                                              info_ptr,
                                              splitCount,
                                              input_splits,
                                              output_splits,
                                              sinfoArray
                                              );
            }
            break;
        }

        case TP_SINGLE_FC:
        {
            FILL_TP_COMMAND(context, input, output, parameter, splitTypes[tpType], splitCount, splitSizes, splitOffsets, sinfoArray, info_ptr, TP_SINGLE_FC);
            break;
        }

        case TP_TRANSPOSE:
        {
            FILL_TP_COMMAND(context, input, output, parameter, splitTypes[tpType], splitCount, splitSizes, splitOffsets, sinfoArray, info_ptr, TP_TRANSPOSE);
            break;
        }

        case TP_MAX_POOLING:
        {
            FILL_TP_COMMAND(context, input, output, parameter, splitTypes[tpType], splitCount, splitSizes, splitOffsets, sinfoArray, info_ptr, TP_MAX_POOLING);
            break;
        }

        case TP_ACTIVATION:
        {
            FILL_TP_COMMAND(context, input, output, parameter, splitTypes[tpType], splitCount, splitSizes, splitOffsets, sinfoArray, info_ptr, TP_ACTIVATION);
            break;
        }

        case TP_LRN:
        {
            FILL_TP_COMMAND(context, input, output, parameter, splitTypes[tpType], splitCount, splitSizes, splitOffsets, sinfoArray, info_ptr, TP_LRN);
            break;
        }

        case TP_ROI_POOLING:
        case TP_ROI_POOLING_STEP_1:
        case TP_ROI_POOLING_STEP_2:
        {
            FILL_TP_COMMAND(context, input, output, parameter, splitTypes[tpType], splitCount, splitSizes, splitOffsets, sinfoArray, info_ptr, TP_ROI_POOLING);
            break;
        }

        case TP_REORG:
        {
            FILL_TP_COMMAND(context, input, output, parameter, splitTypes[tpType], splitCount, splitSizes, splitOffsets, sinfoArray, info_ptr, TP_REORG);
            break;
        }

        case TP_REORG_DEPTH2SPACE:
        {
            FILL_TP_COMMAND(context, input, output, parameter, splitTypes[tpType], splitCount, splitSizes, splitOffsets, sinfoArray, info_ptr, TP_REORG_DEPTH2SPACE);
            break;
        }

        case TP_REORG_SPACE2DEPTH:
        {
            FILL_TP_COMMAND(context, input, output, parameter, splitTypes[tpType], splitCount, splitSizes, splitOffsets, sinfoArray, info_ptr, TP_REORG_SPACE2DEPTH);
            break;
        }

        case TP_REORG_SPACE2BATCH:
        {
            FILL_TP_COMMAND(context, input, output, parameter, splitTypes[tpType], splitCount, splitSizes, splitOffsets, sinfoArray, info_ptr, TP_REORG_SPACE2BATCH);
            break;
        }

        case TP_REORG_BATCH2SPACE:
        {
            FILL_TP_COMMAND(context, input, output, parameter, splitTypes[tpType], splitCount, splitSizes, splitOffsets, sinfoArray, info_ptr, TP_REORG_BATCH2SPACE);
            break;
        }

        case TP_ADD:
        {
            FILL_TP_COMMAND(context, input, output, parameter, splitTypes[tpType], splitCount, splitSizes, splitOffsets, sinfoArray, info_ptr, TP_ADD);
            break;
        }

        case TP_REVERSE:
        {
            FILL_TP_COMMAND(context, input, output, parameter, splitTypes[tpType], splitCount, splitSizes, splitOffsets, sinfoArray, info_ptr, TP_REVERSE);
            break;
        }

        case TP_UPSAMPLE:
        {
            FILL_TP_COMMAND(context, input, output, parameter, splitTypes[tpType], splitCount, splitSizes, splitOffsets, sinfoArray, info_ptr, TP_UPSAMPLE);
            break;
        }

        case TP_UPSAMPLE_CLIP:
        {
            FILL_TP_COMMAND(context, input, output, parameter, splitTypes[tpType], splitCount, splitSizes, splitOffsets, sinfoArray, info_ptr, TP_UPSAMPLE_CLIP);
            break;
        }

        case TP_DILATE_UPSAMPLE:
        {
            FILL_TP_COMMAND(context, input, output, parameter, splitTypes[tpType], splitCount, splitSizes, splitOffsets, sinfoArray, info_ptr, TP_DILATE_UPSAMPLE);
            break;
        }

        case TP_DILATE_UPSAMPLE2:
        {
            FILL_TP_COMMAND(context, input, output, parameter, splitTypes[tpType], splitCount, splitSizes, splitOffsets, sinfoArray, info_ptr, TP_DILATE_UPSAMPLE2);
            break;
        }

        case TP_DILATE_RESHUFFLE:
        {
            FILL_TP_COMMAND(context, input, output, parameter, splitTypes[tpType], splitCount, splitSizes, splitOffsets, sinfoArray, info_ptr, TP_DILATE_RESHUFFLE);
            break;
        }

        case TP_RNN_INTERLEAVE:
        {
            FILL_TP_COMMAND(context, input, output, parameter, splitTypes[tpType], splitCount, splitSizes, splitOffsets, sinfoArray, info_ptr, TP_RNN_INTERLEAVE);
            break;
        }

        case TP_TENSOR_COPY:
        case TP_TENSOR_COPY4CONCAT:
        {
            _fill_TP_TENSOR_COPY_Command_EX(context,
                                            input,
                                            output,
                                            parameter,
                                            info_ptr,
                                            splitCount,
                                            input_splits,
                                            output_splits,
                                            sinfoArray
                                            );
            break;
        }

        case TP_TENSOR_PAD:
        {
            FILL_TP_COMMAND(context, input, output, parameter, splitTypes[tpType], splitCount, splitSizes, splitOffsets, sinfoArray, info_ptr, TP_TENSOR_PAD);
            break;
        }

        case TP_TENSOR_SQUEEZE:
        {
            FILL_TP_COMMAND(context, input, output, parameter, splitTypes[tpType], splitCount, splitSizes, splitOffsets, sinfoArray, info_ptr, TP_TENSOR_SQUEEZE);
            break;
        }

        case TP_TENSOR_STRIDED_SLICE:
        {
            FILL_TP_COMMAND(context, input, output, parameter, splitTypes[tpType], splitCount, splitSizes, splitOffsets, sinfoArray, info_ptr, TP_TENSOR_STRIDED_SLICE);
            break;
        }

        case TP_TENSOR_SVDF_MAP:
        {
            FILL_TP_COMMAND(context, input, output, parameter, splitTypes[tpType], splitCount, splitSizes, splitOffsets, sinfoArray, info_ptr, TP_TENSOR_SVDF_MAP);
            break;
        }

        case TP_LSTM_RESHUFFLE_INPUT:
        {
            FILL_TP_COMMAND(context, input, output, parameter, splitTypes[tpType], splitCount, splitSizes, splitOffsets, sinfoArray, info_ptr, TP_LSTM_RESHUFFLE_INPUT);
            break;
        }

        case TP_LSTM_STATE_OUT:
        {
            FILL_TP_COMMAND(context, input, output, parameter, splitTypes[tpType], splitCount, splitSizes, splitOffsets, sinfoArray, info_ptr, TP_LSTM_STATE_OUT);
            break;
        }

        default:
            break;
    }

    if (input_splits)
    {
        vxFree(input_splits);
        input_splits = VX_NULL;
    }

    if (output_splits)
    {
        vxFree(output_splits);
        output_splits = VX_NULL;
    }

    for (i = 0; i < splitCount; i++)
    {
        vx_uint64 outPixel;
        struct _vx_tp_general_cmd_split_info * tmpPtr;

        if (tpType != TP_SINGLE_FC)
        {
            tmpPtr = &sinfoArray[i].vx_tp_general_cmd_split_info;
        }
        else
        {
            tmpPtr = (struct _vx_tp_general_cmd_split_info*)&sinfoArray[i].vx_tp_fc_cmd_split_info;
        }

        /* TP output pixel must larger than 1 */
        if (tpType == TP_ACTIVATION || tpType == TP_TENSOR_COPY || tpType == TP_ADD ||
            tpType == TP_LRN || tpType == TP_TENSOR_SVDF_MAP || tpType == TP_TENSOR_STRIDED_SLICE)
        {
            outPixel = (vx_uint64)tmpPtr->inImageXSize * tmpPtr->inImageYSize * tmpPtr->inImageZSize;
        }
        else
        {
            outPixel = tmpPtr->outLoop0Count *
                       (!tmpPtr->outLoop1Count ? gcmMIN(tmpPtr->inTileXSize, input->width) : tmpPtr->outLoop1Count) *
                       (!tmpPtr->outLoop2Count ? gcmMIN(tmpPtr->inTileYSize, input->height) : tmpPtr->outLoop2Count) *
                       tmpPtr->outLoop3Count *
                       tmpPtr->outLoop4Count *
                       tmpPtr->outLoop5Count;

            if (tmpPtr->outLoop6Inc != 0)
            {
                outPixel *= tmpPtr->inImageZSize;
            }
        }

        vxmASSERT(outPixel > 1);

        if (tmpPtr->needReorder)
        {
            if (!tmpPtr->aluReorderLoop2Mode)
            {
                tmpPtr->aluReorderBitsUsed = (vx_uint32)gcoMATH_Ceiling(gcoMATH_Log2((vx_float32)tmpPtr->inTileXSize));
            }
            else
            {
                tmpPtr->aluReorderBitsUsed = (vx_uint32)gcoMATH_Ceiling(gcoMATH_Log2((vx_float32)(tmpPtr->inTileXSize * tmpPtr->inTileYSize)));
            }
        }

        if (output->sRAM)
        {
            tmpPtr->outImageCircularBufSize = output->circleBufferSize;
            tmpPtr->outImageCircularBufEndAddrPlus1 = output->physical.circularBufEndAddrPlus1;

            if (tmpPtr->outBaseAddress >= output->physical.circularBufEndAddrPlus1)
            {
                vx_uint32 offset = tmpPtr->outBaseAddress - output->physical.circularBufEndAddrPlus1;
                offset = offset % output->circleBufferSize;
                tmpPtr->outBaseAddress = output->physical.circularBufEndAddrPlus1 - output->circleBufferSize + offset;
            }
            vxmASSERT(tmpPtr->outBaseAddress < output->physical.circularBufEndAddrPlus1);
        }
        else
        {
            tmpPtr->outImageCircularBufSize         = 0;
            tmpPtr->outImageCircularBufEndAddrPlus1 = 0xFFFFFFFF;
        }

        if (input->sRAM)
        {
            tmpPtr->inImageCircularBufSize = input->circleBufferSize;
            tmpPtr->inImageCircularBufEndAddrPlus1  = input->physical.circularBufEndAddrPlus1;

            if (tmpPtr->inImageBaseAddress >= input->physical.circularBufEndAddrPlus1)
            {
                vx_uint32 offset = tmpPtr->inImageBaseAddress - input->physical.circularBufEndAddrPlus1;
                offset = offset % input->circleBufferSize;
                tmpPtr->inImageBaseAddress = input->physical.circularBufEndAddrPlus1 - input->circleBufferSize + offset;
            }
            vxmASSERT(tmpPtr->inImageBaseAddress < input->physical.circularBufEndAddrPlus1);
        }
        else
        {
            tmpPtr->inImageCircularBufSize   = 0;
            tmpPtr->inImageCircularBufEndAddrPlus1     = 0xFFFFFFFF;
        }
    }

    *sinfo_array_ptr = sinfoArray;
    *sinfo_num_ptr = splitCount;

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxnneCommandBuffer_GetTPGeneralCommandInfo(
    vx_context                   context,
    vxnne_tensor_info            input,
    vxnne_tensor_info            output,
    vx_op_param                  conv_cmd_ptr,
    vx_nn_cmd_info_u *           info
    )
{
    vx_uint32 inXSize, inYSize;
    vx_uint32 poolingStride;
    vx_enum inFormat, outFormat, inQFormat, outQFormat, outRMode;
    vx_int8 inFPP, outFPP;
    vx_int32 inZP, outZP, inPadZValue;
    vx_float32 inScale, outScale;
    vx_enum tp_type;
    vx_bool hasInputQuant = vx_false_e;
    vx_bool hasOutputQuant = vx_false_e;
    vx_bool hasWQuant = vx_false_e;
    vx_bool tpRealInt16 = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_TP_REAL_INT16);
    vx_bool tpSimpleInt16 = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_TP_SIMPLE_INT16);
    vx_bool tfQuant = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT);
    vx_tensor other_tensor, data_buff;
    vx_tp_value_cmd value_cmd_ptr;

    inXSize = input->width;
    inYSize = input->height;
    inFormat  = input->dataFormat;
    outFormat = output->dataFormat;
    inFPP  = input->fixedPointPos;
    outFPP = output->fixedPointPos;
    inQFormat = input->quantFormat;
    outQFormat = output->quantFormat;
    outRMode = output->roundingMode;
    inScale  = input->scale;
    outScale = output->scale;
    inZP  = input->zeroPoint;
    outZP = output->zeroPoint;

    vxmASSERT(conv_cmd_ptr != VX_NULL);
    inPadZValue = conv_cmd_ptr->pad_const;
    tp_type = conv_cmd_ptr->tpType;
    other_tensor = (vx_tensor)conv_cmd_ptr->other_ref;
    data_buff = (vx_tensor)conv_cmd_ptr->data_buff;
    value_cmd_ptr = conv_cmd_ptr->tp_value;

    hasInputQuant  = (vx_bool)((inFormat == VX_TYPE_UINT8) && (inQFormat == VX_QUANT_AFFINE_SCALE));
    hasOutputQuant = (vx_bool)((outFormat == VX_TYPE_UINT8) && (outQFormat == VX_QUANT_AFFINE_SCALE));

    if ((tp_type == TP_SINGLE_FC) && other_tensor != VX_NULL &&
        (WB_WEIGHT_DATA_FORMAT((vx_weights_biases_parameter)other_tensor) == VX_TYPE_UINT8) &&
        (WB_WEIGHT_QUANT_FORMAT((vx_weights_biases_parameter)other_tensor) == VX_QUANT_AFFINE_SCALE))
    {
        hasWQuant = vx_true_e;
    }

    /* Common settings. Might be overwritten by specific configuration. */
    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT) && tp_type == TP_RESHUFFLE)
    {
        info->vx_nn_tp_cmd_info.inImageBorderMode = 0x0;
        info->vx_nn_tp_cmd_info.inImageBorderConst = inPadZValue & 0xFF;
    }
    else
    {
        info->vx_nn_tp_cmd_info.inImageBorderMode = getHWBorderMode(conv_cmd_ptr->pad_mode, gcvVX_ACCELERATOR_TP);

        if (info->vx_nn_tp_cmd_info.inImageBorderMode == 0x0)
        {
            info->vx_nn_tp_cmd_info.inImageBorderConst = (inFormat == VX_TYPE_INT8) ? (vx_int8)conv_cmd_ptr->pad_const : (vx_int16)conv_cmd_ptr->pad_const;
        }
    }

    switch (tp_type)
    {
        case TP_SINGLE_FC:
        {
            vx_weights_biases_parameter weights_biases = (vx_weights_biases_parameter) other_tensor;
            vx_uint32 kzGroup;

            vxmASSERT(value_cmd_ptr != VX_NULL);

            kzGroup = value_cmd_ptr->u32[0];
            if (kzGroup == 1)
            {
                info->vx_nn_tp_cmd_info.inImageGlobalMem = 1;
                info->vx_nn_tp_cmd_info.aluSquarePreshift = 0;
                info->vx_nn_tp_cmd_info.aluSquareEnable = 0;
                info->vx_nn_tp_cmd_info.aluHorzProcessing = getHWDataFormat(WB_WEIGHT_DATA_FORMAT(weights_biases));
                info->vx_nn_tp_cmd_info.aluHorzProcStride = 0;
                info->vx_nn_tp_cmd_info.aluVertProcessing = 0;
                info->vx_nn_tp_cmd_info.aluVertProcStride = 0;
                info->vx_nn_tp_cmd_info.aluPwlEnable = 0;
                info->vx_nn_tp_cmd_info.aluMultEnable = 0;
                info->vx_nn_tp_cmd_info.aluLoadPwlLUT = 0;
                info->vx_nn_tp_cmd_info.aluLoadPwlLUTGlobalMem = 1;
                info->vx_nn_tp_cmd_info.outGlobalMem  = 1;
                info->vx_nn_tp_cmd_info.outTileSkipAtborder = 0;
                info->vx_nn_tp_cmd_info.outBrickMode  = 0;
                info->vx_nn_tp_cmd_info.aluFilterPwlSwap = 0;
                info->vx_nn_tp_cmd_info.aluPwlSignSupport = 0;
                info->vx_nn_tp_cmd_info.aluReluEnable =  conv_cmd_ptr->enable_relu;
                info->vx_nn_tp_cmd_info.aluInputPreshift   = 0;
                info->vx_nn_tp_cmd_info.aluOutputPostshift =
                     ((inQFormat == VX_QUANT_DYNAMIC_FIXED_POINT) ? inFPP : 0)
                   - (outQFormat == VX_QUANT_DYNAMIC_FIXED_POINT ? outFPP : 0)
                   + (WB_WEIGHT_QUANT_FORMAT(weights_biases) == VX_QUANT_DYNAMIC_FIXED_POINT ? WB_WEIGHT_FPP(weights_biases) : 0);
                info->vx_nn_tp_cmd_info.aluI2FEnable = (inFormat == VX_TYPE_FLOAT16) ? 0 : 1;
                info->vx_nn_tp_cmd_info.aluF2IEnable = (outFormat == VX_TYPE_FLOAT16) ? 0 : 1;
            }
            else
            {
                if (value_cmd_ptr->e32[0] == 0)
                {
                    info->vx_nn_tp_cmd_info.inTileSequence = 0x3;
                    info->vx_nn_tp_cmd_info.inImageGlobalMem = 1;
                    info->vx_nn_tp_cmd_info.aluHorzProcessing = getHWDataFormat(WB_WEIGHT_DATA_FORMAT(weights_biases));
                    info->vx_nn_tp_cmd_info.aluHorzProcStride = 0;
                    info->vx_nn_tp_cmd_info.aluVertProcessing = 0;
                    info->vx_nn_tp_cmd_info.aluVertProcStride = 0;
                    info->vx_nn_tp_cmd_info.aluLoadPwlLUT = 0;
                    info->vx_nn_tp_cmd_info.aluLoadPwlLUTGlobalMem = 1;
                    info->vx_nn_tp_cmd_info.aluReluEnable = conv_cmd_ptr->enable_relu;
                    info->vx_nn_tp_cmd_info.aluInputPreshift   = 0;
                    info->vx_nn_tp_cmd_info.aluOutputPostshift =
                         ((inQFormat == VX_QUANT_DYNAMIC_FIXED_POINT) ? inFPP : 0)
                       - (outQFormat == VX_QUANT_DYNAMIC_FIXED_POINT ? outFPP : 0)
                       + (WB_WEIGHT_QUANT_FORMAT(weights_biases) == VX_QUANT_DYNAMIC_FIXED_POINT ? WB_WEIGHT_FPP(weights_biases) : 0);
                }
                else
                {
                    /* sum filter */
                    info->vx_nn_tp_cmd_info.inImageGlobalMem = 1;
                    info->vx_nn_tp_cmd_info.aluHorzProcessing = 0;
                    info->vx_nn_tp_cmd_info.aluHorzProcCount = 0;
                    info->vx_nn_tp_cmd_info.aluHorzProcStride = 0;
                    info->vx_nn_tp_cmd_info.aluVertProcessing = 0x0;
                    info->vx_nn_tp_cmd_info.aluVertProcCount = kzGroup - 1;
                    info->vx_nn_tp_cmd_info.aluVertProcStride = 0;
                    info->vx_nn_tp_cmd_info.aluLoadPwlLUT = 0;
                    info->vx_nn_tp_cmd_info.aluLoadPwlLUTGlobalMem = 1;
                    info->vx_nn_tp_cmd_info.aluReluEnable = 0;
                    info->vx_nn_tp_cmd_info.aluInputPreshift = (inQFormat == VX_QUANT_DYNAMIC_FIXED_POINT) ? inFPP : 0;
                    info->vx_nn_tp_cmd_info.aluOutputPostshift = (outQFormat == VX_QUANT_DYNAMIC_FIXED_POINT) ? (0 - outFPP) : 0;
                }
                info->vx_nn_tp_cmd_info.outGlobalMem  = 1;
                info->vx_nn_tp_cmd_info.outTileSkipAtborder = 0;
                info->vx_nn_tp_cmd_info.outBrickMode  = 0;
                info->vx_nn_tp_cmd_info.aluSquarePreshift = 0;
                info->vx_nn_tp_cmd_info.aluSquareEnable = 0;
                info->vx_nn_tp_cmd_info.aluPwlEnable = 0;
                info->vx_nn_tp_cmd_info.aluMultEnable = 0;
                info->vx_nn_tp_cmd_info.aluFilterPwlSwap = 0;
                info->vx_nn_tp_cmd_info.aluPwlSignSupport = 0;
                info->vx_nn_tp_cmd_info.aluI2FEnable = inFormat == VX_TYPE_FLOAT16 ? 0 : 1;
                info->vx_nn_tp_cmd_info.aluF2IEnable = outFormat == VX_TYPE_FLOAT16 ? 0 : 1;

            }
            break;
        }

        case TP_MAX_POOLING:
        {
            poolingStride = conv_cmd_ptr->pool_stride;

            if (poolingStride == 1)
            {
                info->vx_nn_tp_cmd_info.aluHorzProcessing = 0x1;
                info->vx_nn_tp_cmd_info.aluHorzProcCount = conv_cmd_ptr->pool_size_x - 1;
                info->vx_nn_tp_cmd_info.aluHorzProcStride = 0;
                info->vx_nn_tp_cmd_info.aluVertProcessing = 0x1;
                info->vx_nn_tp_cmd_info.aluVertProcCount = conv_cmd_ptr->pool_size_y - 1;
                info->vx_nn_tp_cmd_info.aluVertProcStride = 0;
            }
            else if (conv_cmd_ptr->pool_size_x != 1)
            {
                info->vx_nn_tp_cmd_info.aluHorzProcessing = 0x3;
                info->vx_nn_tp_cmd_info.aluHorzProcCount = conv_cmd_ptr->pool_size_x - 1;
                info->vx_nn_tp_cmd_info.aluHorzProcStride = conv_cmd_ptr->pool_size_x == poolingStride ?
                    0x0 : 0x1;
                info->vx_nn_tp_cmd_info.aluVertProcessing = 0x3;
                info->vx_nn_tp_cmd_info.aluVertProcCount = conv_cmd_ptr->pool_size_y - 1;
                info->vx_nn_tp_cmd_info.aluVertProcStride = conv_cmd_ptr->pool_size_y == poolingStride ?
                    0x0 : 0x1;
            }

            info->vx_nn_tp_cmd_info.inImageGlobalMem = 1;
            info->vx_nn_tp_cmd_info.aluSquarePreshift = 0;
            info->vx_nn_tp_cmd_info.aluSquareEnable = 0;
            info->vx_nn_tp_cmd_info.aluPwlEnable = 0;
            info->vx_nn_tp_cmd_info.aluMultEnable = 0;
            info->vx_nn_tp_cmd_info.aluLoadPwlLUT = 0;
            info->vx_nn_tp_cmd_info.aluLoadPwlLUTAddress = 0;
            info->vx_nn_tp_cmd_info.aluLoadPwlLUTGlobalMem = 1;
            info->vx_nn_tp_cmd_info.outGlobalMem  = 1;
            info->vx_nn_tp_cmd_info.outTileSkipAtborder = 0;
            info->vx_nn_tp_cmd_info.outBrickMode  = 0;
            info->vx_nn_tp_cmd_info.aluFilterPwlSwap = 0;
            info->vx_nn_tp_cmd_info.aluPwlSignSupport = 0;
            info->vx_nn_tp_cmd_info.aluReluEnable = 0;
            info->vx_nn_tp_cmd_info.aluI2FEnable = inFormat == VX_TYPE_FLOAT16 ? 0 : 1;
            info->vx_nn_tp_cmd_info.aluF2IEnable = outFormat == VX_TYPE_FLOAT16 ? 0 : 1;
            info->vx_nn_tp_cmd_info.aluInputPreshift   = 0;
            info->vx_nn_tp_cmd_info.aluOutputPostshift = ((inQFormat == VX_QUANT_DYNAMIC_FIXED_POINT) ? inFPP : 0) - ((outQFormat == VX_QUANT_DYNAMIC_FIXED_POINT) ? outFPP : 0);
            break;
        }

        case TP_ACTIVATION:
        {
            vx_float32  scale;
            vx_uint16 * pwlLUTBase = (vx_uint16 *)data_buff->tensorBuffer->memory.logicals[0];
            vx_uint32 * pwlLUTBaseEx = (vx_uint32 *)data_buff->tensorBuffer->memory.logicals[0];
            vx_uint16   fixed16, base, baseF16;
            vx_uint32   fixed21, baseF21;
            vx_float32  baseF32;
            vx_float32  pwlValue;
            vx_float32  value = inQFormat == VX_QUANT_AFFINE_SCALE ? inScale : 1.0f;

            vxmASSERT(value_cmd_ptr != VX_NULL);
            vxmASSERT(pwlLUTBase != VX_NULL);

            scale = value_cmd_ptr->f32[0];

            switch (value_cmd_ptr->e32[0])
            {
                case VX_NN_ACTIVATION_RELU:
                    if (!gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_TP_REAL_INT16))
                    {
                        for (base = 0; base < 0x10; base++)
                        {
                            pwlLUTBase[base] = 0x0;         /* Half float positive zero. */
                        }
                        for (base = 0x10; base < 0x1F0; base++)
                        {
                            baseF16 = base << 6;
                            pwlLUTBase[base] = baseF16;
                        }
                        for (base = 0x1F0; base < 0x200; base++)
                        {
                            pwlLUTBase[base] = 0x7bff;      /* Half float positive infinity clamp to max. */
                        }
                        for (base = 0x200; base < 0x400; base++)
                        {
                            pwlLUTBase[base] = 0x8000;      /* Half float negative zero. */
                        }
                    }
                    else
                    {
                        for (base = 0; base < 0x10; base++)
                        {
                            pwlLUTBaseEx[base] = 0x0;         /* Half float positive zero. */
                        }
                        for (base = 0x10; base < 0x1F0; base++)
                        {
                            baseF21 = base << 11;
                            pwlLUTBaseEx[base] = baseF21;
                        }
                        for (base = 0x1F0; base < 0x200; base++)
                        {
                            pwlLUTBaseEx[base] = 0xf7fff;      /* Half float positive infinity clamp to max. */
                        }
                        for (base = 0x200; base < 0x400; base++)
                        {
                            pwlLUTBaseEx[base] = 0x100000;      /* Half float negative zero. */
                        }
                    }
                    break;

                case VX_NN_ACTIVATION_LEAKYRELU:
                    if (!gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_TP_REAL_INT16))
                    {
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
                        for (base = 0x1F0; base < 0x200; base++)
                        {
                            pwlLUTBase[base] = 0x7bff;      /* Half float positive infinity clamp to max. */
                        }
                        for (base = 0x3F0; base < 0x400; base++)
                        {
                            pwlLUTBase[base] = 0xfbff;      /* Half float negative infinity clamp to max. */
                        }
                    }
                    else
                    {
                        /* Flush denorms to 0.0f. */
                        for (base = 0; base < 0x10; base++)
                        {
                            pwlLUTBaseEx[base] = 0x0;         /* Half float positive zero. */
                        }
                        for (base = 0x200; base < 0x210; base++)
                        {
                            pwlLUTBaseEx[base] = 0x100000;      /* Half float negative zero. */
                        }
                        for (base = 0x10; base < 0x1F0; base++)
                        {
                            baseF21 = base << 11;
                            pwlLUTBaseEx[base] = baseF21;
                        }
                        for (base = 0x210; base < 0x3F0; base++)
                        {
                            baseF21 = base << 11;
                            baseF32 = Fp21toFp32(baseF21);
                            pwlValue = baseF32 * scale;
                            pwlLUTBaseEx[base] = Fp32toFp21(pwlValue);
                        }
                        for (base = 0x1F0; base < 0x200; base++)
                        {
                            pwlLUTBaseEx[base] = 0xf7fff;      /* Half float positive infinity clamp to max. */
                        }
                        for (base = 0x3F0; base < 0x400; base++)
                        {
                            pwlLUTBaseEx[base] = 0x1f7fff;      /* Half float negative infinity clamp to max. */
                        }
                    }
                    break;

                case VX_NN_ACTIVATION_RELU1:
                    if (!gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_TP_REAL_INT16))
                    {
                        for (base = 0x0; base < 0x10; base++)
                        {
                            pwlLUTBase[base] = 0x0;             /* Flush denorms to 0.0f. */
                        }
                        for (base = 0x10; base < 0x200; base++)
                        {
                            baseF16 = base << 6;
                            baseF32 = Fp16toFp32(baseF16);
                            pwlValue = baseF32 * value;
                            if (pwlValue >= 1.0f) break;
                            pwlLUTBase[base] = Fp32toFp16(pwlValue);
                        }
                        fixed16 = Fp32toFp16(1.0f);
                        for (; base < 0x200; base++)
                        {
                            pwlLUTBase[base] = fixed16;         /* large than one to 1.0f. */
                        }
                        for (base = 0x200; base < 0x210; base++)
                        {
                            pwlLUTBase[base] = 0x8000;          /* Flush negative denorms to -0.0f. */
                        }
                        for (base = 0x210; base < 0x400; base++)
                        {
                            baseF16 = base << 6;
                            baseF32 = Fp16toFp32(baseF16);
                            pwlValue = baseF32 * value;
                            if (pwlValue <= -1.0f) break;
                            pwlLUTBase[base] = Fp32toFp16(pwlValue);
                        }
                        fixed16 = Fp32toFp16(-1.0f);
                        for (; base < 0x400; base++)
                        {
                            pwlLUTBase[base] = fixed16;         /* smaller than negative one to -1.0f. */
                        }
                    }
                    else
                    {
                        for (base = 0x0; base < 0x10; base++)
                        {
                            pwlLUTBaseEx[base] = 0x0;               /* Flush denorms to 0.0f. */
                        }
                        for (base = 0x10; base < 0x200; base++)
                        {
                            baseF21 = base << 11;
                            baseF32 = Fp21toFp32(baseF21);
                            pwlValue = baseF32 * value;
                            if (pwlValue >= 1.0f) break;
                            pwlLUTBaseEx[base] = Fp32toFp21(pwlValue);
                        }
                        fixed21 = Fp32toFp21(1.0f);
                        for (; base < 0x200; base++)
                        {
                            pwlLUTBaseEx[base] = fixed21;           /* large than one to 1.0f. */
                        }
                        for (base = 0x200; base < 0x210; base++)
                        {
                            pwlLUTBaseEx[base] = 0x100000;          /* Flush negative denorms to -0.0f. */
                        }
                        for (base = 0x210; base < 0x400; base++)
                        {
                            baseF21 = base << 11;
                            baseF32 = Fp21toFp32(baseF21);
                            pwlValue = baseF32 * value;
                            if (pwlValue <= -1.0f) break;
                            pwlLUTBaseEx[base] = Fp32toFp21(pwlValue);
                        }
                        fixed21 = Fp32toFp21(-1.0f);
                        for (; base < 0x400; base++)
                        {
                            pwlLUTBaseEx[base] = fixed21;           /* smaller than negative one to -1.0f. */
                        }
                    }
                    break;

                case VX_NN_ACTIVATION_RELU6:
                    if (!gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_TP_REAL_INT16))
                    {
                        for (base = 0; base < 0x10; base++)
                        {
                            pwlLUTBase[base] = 0x0;
                        }
                        for (base = 0x10; base < 0x200; base++)
                        {
                            baseF16 = base << 6;
                            baseF32 = Fp16toFp32(baseF16);
                            pwlValue = baseF32 * value;
                            if (pwlValue >= 6.0f) break;
                            pwlLUTBase[base] = Fp32toFp16(pwlValue);
                        }
                        fixed16 = Fp32toFp16(6.0f);
                        for (; base < 0x200; base++)
                        {
                            pwlLUTBase[base] = fixed16;         /* Clamp large than six to 6.0f. */
                        }
                        for (base = 0x200; base < 0x400; base++)
                        {
                            pwlLUTBase[base] = 0x0;             /* smaller than zero to 0.0f. */
                        }
                    }
                    else
                    {
                        for (base = 0; base < 0x10; base++)
                        {
                            pwlLUTBaseEx[base] = 0x0;
                        }
                        for (base = 0x10; base < 0x200; base++)
                        {
                            baseF21 = base << 11;
                            baseF32 = Fp21toFp32(baseF21);
                            pwlValue = baseF32 * value;
                            if (pwlValue >= 6.0f) break;
                            pwlLUTBaseEx[base] = Fp32toFp21(pwlValue);
                        }
                        fixed21 = Fp32toFp21(6.0f);
                        for (; base < 0x200; base++)
                        {
                            pwlLUTBaseEx[base] = fixed21;         /* Clamp large than six to 6.0f. */
                        }
                        for (base = 0x200; base < 0x400; base++)
                        {
                            pwlLUTBaseEx[base] = 0x0;             /* smaller than zero to 0.0f. */
                        }
                    }
                    break;

                case VX_NN_ACTIVATION_LOGISTIC:
                    if (!gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_TP_REAL_INT16))
                    {
                        fixed16 = Fp32toFp16(0.5f);
                        for (base = 0; base < 0x10; base++)
                        {
                            pwlLUTBase[base] = fixed16;
                        }
                        for (base = 0x10; base < 0x1F0; base++)
                        {
                            baseF16 = base << 6;
                            baseF32 = Fp16toFp32(baseF16);
                            pwlValue = 1.0f / (1.0f + gcoMATH_Exp(0.0f - baseF32 * value));
                            pwlLUTBase[base] = Fp32toFp16(pwlValue);
                        }
                        fixed16 = Fp32toFp16(1.0f);
                        for (base = 0x1F0; base < 0x200; base++)
                        {
                            pwlLUTBase[base] = fixed16;
                        }

                        fixed16 = Fp32toFp16(0.5f);
                        for (base = 0x200; base < 0x210; base++)
                        {
                            pwlLUTBase[base] = fixed16;
                        }
                        for (base = 0x210; base < 0x3F0; base++)
                        {
                            baseF16 = base << 6;
                            baseF32 = Fp16toFp32(baseF16);
                            pwlValue = 1.0f / (1.0f + gcoMATH_Exp(0.0f - baseF32 * value));
                            pwlLUTBase[base] = Fp32toFp16(pwlValue);
                        }
                        fixed16 = Fp32toFp16(0.0f);
                        for (base = 0x3F0; base < 0x400; base++)
                        {
                            pwlLUTBase[base] = fixed16;
                        }
                    }
                    else
                    {
                        fixed21 = Fp32toFp21(0.5f);
                        for (base = 0; base < 0x10; base++)
                        {
                            pwlLUTBaseEx[base] = fixed21;
                        }
                        for (base = 0x10; base < 0x1F0; base++)
                        {
                            baseF21 = base << 11;
                            baseF32 = Fp21toFp32(baseF21);
                            pwlValue = 1.0f / (1.0f + gcoMATH_Exp(0.0f - baseF32 * value));
                            pwlLUTBaseEx[base] = Fp32toFp21(pwlValue);
                        }
                        fixed21 = Fp32toFp21(1.0f);
                        for (base = 0x1F0; base < 0x200; base++)
                        {
                            pwlLUTBaseEx[base] = fixed21;
                        }

                        fixed21 = Fp32toFp21(0.5f);
                        for (base = 0x200; base < 0x210; base++)
                        {
                            pwlLUTBaseEx[base] = fixed21;
                        }
                        for (base = 0x210; base < 0x3F0; base++)
                        {
                            baseF21 = base << 11;
                            baseF32 = Fp21toFp32(baseF21);
                            pwlValue = 1.0f / (1.0f + gcoMATH_Exp(0.0f - baseF32 * value));
                            pwlLUTBaseEx[base] = Fp32toFp21(pwlValue);
                        }
                        fixed21 = Fp32toFp21(0.0f);
                        for (base = 0x3F0; base < 0x400; base++)
                        {
                            pwlLUTBaseEx[base] = fixed21;
                        }
                    }
                    break;

                case VX_NN_ACTIVATION_HYPERBOLIC_TAN:
                    if (!gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_TP_REAL_INT16))
                    {
                        fixed16 = Fp32toFp16(0.0f);
                        for (base = 0; base < 0x20; base++)
                        {
                            pwlLUTBase[base] = fixed16;
                        }
                        for (base = 0x20; base < 0x3E0; base++)
                        {
                            baseF16 = base << 5;
                            baseF32 = Fp16toFp32(baseF16);
                            pwlValue = gcoMATH_TangentH(baseF32 * value);
                            pwlLUTBase[base] = Fp32toFp16(pwlValue);
                        }
                        fixed16 = Fp32toFp16(1.0f);
                        for (base = 0x3E0; base < 0x400; base++)
                        {
                            pwlLUTBase[base] = fixed16;
                        }
                    }
                    else
                    {
                        fixed21 = Fp32toFp21(0.0f);
                        for (base = 0; base < 0x20; base++)
                        {
                            pwlLUTBaseEx[base] = fixed21;
                        }
                        for (base = 0x20; base < 0x3E0; base++)
                        {
                            baseF21 = base << 10;
                            baseF32 = Fp21toFp32(baseF21);
                            pwlValue = gcoMATH_TangentH(baseF32 * value);
                            pwlLUTBaseEx[base] = Fp32toFp21(pwlValue);
                        }
                        fixed21 = Fp32toFp21(1.0f);
                        for (base = 0x3E0; base < 0x400; base++)
                        {
                            pwlLUTBaseEx[base] = fixed21;
                        }
                    }
                    break;

                default:
                    break;
            }

            info->vx_nn_tp_cmd_info.inImageGlobalMem = 1;
            info->vx_nn_tp_cmd_info.aluHorzProcCount = 0;
            info->vx_nn_tp_cmd_info.aluVertProcCount = 0;
            info->vx_nn_tp_cmd_info.aluFilterPwlSwap = 0;
            info->vx_nn_tp_cmd_info.aluSquarePreshift = 0;
            info->vx_nn_tp_cmd_info.aluSquareEnable = 0;
            info->vx_nn_tp_cmd_info.aluPwlEnable = 1;
            info->vx_nn_tp_cmd_info.aluMultEnable = 0;
            info->vx_nn_tp_cmd_info.aluLoadPwlLUT = 1;
            info->vx_nn_tp_cmd_info.aluLoadPwlLUTAddress = data_buff->tensorBuffer->memory.physicals[0];
            info->vx_nn_tp_cmd_info.aluLoadPwlLUTGlobalMem = 1;
            info->vx_nn_tp_cmd_info.outGlobalMem  = 1;
            info->vx_nn_tp_cmd_info.outTileSkipAtborder = 0;
            info->vx_nn_tp_cmd_info.outBrickMode  = 0;
            info->vx_nn_tp_cmd_info.aluPwlSignSupport = (value_cmd_ptr->e32[0] == VX_NN_ACTIVATION_HYPERBOLIC_TAN) ? 0 : 1;
            info->vx_nn_tp_cmd_info.aluReluEnable = 0;
            info->vx_nn_tp_cmd_info.aluI2FEnable = inFormat == VX_TYPE_FLOAT16 ? 0 : 1;
            info->vx_nn_tp_cmd_info.aluF2IEnable = outFormat == VX_TYPE_FLOAT16 ? 0 : 1;
            info->vx_nn_tp_cmd_info.aluInputPreshift = (inQFormat == VX_QUANT_DYNAMIC_FIXED_POINT) ? inFPP : 0;
            info->vx_nn_tp_cmd_info.aluOutputPostshift = (outQFormat == VX_QUANT_DYNAMIC_FIXED_POINT) ? (0 - outFPP) : 0;
            break;
        }

        case TP_LRN:
        {
            vx_float32  alpha, beta, bias, ks;
            vx_uint32   kernel;
            vx_uint16 * pwlLUTBase = (vx_uint16 *)data_buff->tensorBuffer->memory.logicals[0];
            vx_uint32 * pwlLUTBaseEx = (vx_uint32 *)data_buff->tensorBuffer->memory.logicals[0];
            vx_uint16   base, fixed16, baseF16;
            vx_float32  baseF32;
            vx_uint32   baseU32, fixed21, baseF21;
            vx_float32  pwlValue;
            vx_uint32   halfK, preShift = 4, preShiftValue = 1 << (preShift*2);
            vx_float32  value = inQFormat == VX_QUANT_AFFINE_SCALE ? inScale * inScale : 1.0f;

            vxmASSERT(value_cmd_ptr != VX_NULL);
            vxmASSERT(pwlLUTBase != VX_NULL);

            alpha  = value_cmd_ptr->f32[0];
            beta   = value_cmd_ptr->f32[1];
            bias   = value_cmd_ptr->f32[2];
            kernel = value_cmd_ptr->u32[0];
            ks     = (vx_float32)value_cmd_ptr->u32[1];
            halfK  = kernel / 2;

            vxmASSERT(kernel <= 5);
            vxmASSERT(kernel & 0x1);

            /* Setup PWL. */
            if (!gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_TP_REAL_INT16))
            {
                fixed16 = Fp32toFp16(1.0f);
                for (base = 0; base < 0x20; base++)
                {
                    pwlLUTBase[base] = fixed16;
                }
                for (base = 0x20; base < 0x3E0; base++)
                {
                    baseF16 = base << 5;
                    baseF32 = Fp16toFp32(baseF16);
                    pwlValue = bias + alpha * (baseF32 * value * (vx_float32)preShiftValue / ks);
                    pwlValue = 1.0f / (vx_float32)pow(pwlValue, beta);
                    pwlLUTBase[base] = Fp32toFp16(pwlValue);
                }
                baseU32 = (31 - 15 + 127) << 23;
                baseF32 = *((vx_float32*) &baseU32);
                pwlValue = bias + alpha * (baseF32 * (vx_float32)preShiftValue / ks);
                pwlValue = 1.0f / (vx_float32)pow(pwlValue, beta);
                fixed16 = Fp32toFp16(pwlValue);
                for (base = 0x3E0; base < 0x400; base++)
                {
                    pwlLUTBase[base] = fixed16;
                }
            }
            else
            {
                fixed21 = Fp32toFp21(1.0f);
                for (base = 0; base < 0x20; base++)
                {
                    pwlLUTBaseEx[base] = fixed21;
                }
                for (base = 0x20; base < 0x3E0; base++)
                {
                    baseF21 = base << 10;
                    baseF32 = Fp21toFp32(baseF21);
                    pwlValue = bias + alpha * (baseF32 * value * (vx_float32)preShiftValue / ks);
                    pwlValue = 1.0f / (vx_float32)pow(pwlValue, beta);
                    pwlLUTBaseEx[base] = Fp32toFp21(pwlValue);
                }
                baseU32 = (31 - 15 + 127) << 23;
                baseF32 = *((vx_float32*) &baseU32);
                pwlValue = bias + alpha * (baseF32 * (vx_float32)preShiftValue / ks);
                pwlValue = 1.0f / (vx_float32)pow(pwlValue, beta);
                fixed21 = Fp32toFp21(pwlValue);
                for (base = 0x3E0; base < 0x400; base++)
                {
                    pwlLUTBaseEx[base] = fixed21;
                }
            }

            info->vx_nn_tp_cmd_info.inImageGlobalMem = 1;
            info->vx_nn_tp_cmd_info.outGlobalMem  = 1;
            info->vx_nn_tp_cmd_info.outTileSkipAtborder = 0;
            info->vx_nn_tp_cmd_info.outBrickMode  = 0;
            info->vx_nn_tp_cmd_info.aluFilterPwlSwap = 0;
            info->vx_nn_tp_cmd_info.aluPwlSignSupport = 0;
            info->vx_nn_tp_cmd_info.aluReluEnable = 0;
            info->vx_nn_tp_cmd_info.aluLoadPwlLUTAddress = data_buff->tensorBuffer->memory.physicals[0];
            info->vx_nn_tp_cmd_info.aluPwlEnable = 1;
            info->vx_nn_tp_cmd_info.aluMultEnable = 1;
            info->vx_nn_tp_cmd_info.aluLoadPwlLUT = 1;
            info->vx_nn_tp_cmd_info.aluLoadPwlLUTGlobalMem = 1;
            info->vx_nn_tp_cmd_info.aluI2FEnable = inFormat == VX_TYPE_FLOAT16 ? 0 : 1;
            info->vx_nn_tp_cmd_info.aluF2IEnable = outFormat == VX_TYPE_FLOAT16 ? 0 : 1;
            info->vx_nn_tp_cmd_info.aluInputPreshift = (inQFormat == VX_QUANT_DYNAMIC_FIXED_POINT) ? inFPP : 0;
            info->vx_nn_tp_cmd_info.aluOutputPostshift = (outQFormat == VX_QUANT_DYNAMIC_FIXED_POINT) ? (0 - outFPP) : 0;

            if (value_cmd_ptr->e32[0]  == VX_NN_NORMALIZATION_SAME_MAP)
            {
                info->vx_nn_tp_cmd_info.aluSquarePreshift = preShift;
                info->vx_nn_tp_cmd_info.aluSquareEnable = 1;
                info->vx_nn_tp_cmd_info.aluHorzProcessing = 0x0;
                info->vx_nn_tp_cmd_info.aluHorzProcCount = kernel - 1;
                info->vx_nn_tp_cmd_info.aluHorzProcStride = 0;
                info->vx_nn_tp_cmd_info.aluVertProcessing = 0x0;
                info->vx_nn_tp_cmd_info.aluVertProcCount = kernel - 1;
                info->vx_nn_tp_cmd_info.aluVertProcStride = 0;
            }
            else /* VX_NN_NORMALIZATION_ACROSS_MAPS */
            {
                if (inXSize * inYSize < 65536)
                {
                    /* Convert to 2D same map LRN. */
                    info->vx_nn_tp_cmd_info.aluSquarePreshift = preShift;
                    info->vx_nn_tp_cmd_info.aluSquareEnable = 1;
                    info->vx_nn_tp_cmd_info.aluHorzProcessing = 0x0;
                    info->vx_nn_tp_cmd_info.aluHorzProcCount = 0;
                    info->vx_nn_tp_cmd_info.aluHorzProcStride = 0;
                    info->vx_nn_tp_cmd_info.aluVertProcessing = 0x0;
                    info->vx_nn_tp_cmd_info.aluVertProcCount = kernel - 1;
                    info->vx_nn_tp_cmd_info.aluVertProcStride = 0;
                    info->vx_nn_tp_cmd_info.aluZFilterMode = 0;
                    info->vx_nn_tp_cmd_info.inWindowZStartOverfetch = 0;
                    info->vx_nn_tp_cmd_info.inWindowZEndOverfetch = 0;
                }
                else
                {
                    /* Turn on z filter. */
                    /* Input X-Z plain as X-Y plain. */
                    info->vx_nn_tp_cmd_info.aluSquarePreshift = preShift;
                    info->vx_nn_tp_cmd_info.aluSquareEnable = 1;
                    info->vx_nn_tp_cmd_info.aluHorzProcessing = 0x0;
                    info->vx_nn_tp_cmd_info.aluHorzProcCount = 0;
                    info->vx_nn_tp_cmd_info.aluHorzProcStride = 0;
                    info->vx_nn_tp_cmd_info.aluVertProcessing = 0x0;
                    info->vx_nn_tp_cmd_info.aluVertProcCount = kernel - 1;
                    info->vx_nn_tp_cmd_info.aluVertProcStride = 0;
                    info->vx_nn_tp_cmd_info.aluZFilterMode = 1;
                    info->vx_nn_tp_cmd_info.inWindowZStartOverfetch = halfK;
                    info->vx_nn_tp_cmd_info.inWindowZEndOverfetch = halfK;
                }
            }
            break;
        }

        case TP_ROI_POOLING:
        case TP_ROI_POOLING_STEP_1:
        case TP_ROI_POOLING_STEP_2:
        {
            vx_uint32 maxPoolSize;

            vxmASSERT(value_cmd_ptr != VX_NULL);

            maxPoolSize = value_cmd_ptr->u32[2];

            info->vx_nn_tp_cmd_info.aluSquarePreshift = 0;
            info->vx_nn_tp_cmd_info.aluSquareEnable = 0;
            info->vx_nn_tp_cmd_info.aluFilterPwlSwap = 0;
            info->vx_nn_tp_cmd_info.aluPwlSignSupport = 0;
            info->vx_nn_tp_cmd_info.aluPwlEnable = 0;
            info->vx_nn_tp_cmd_info.aluMultEnable = 0;
            info->vx_nn_tp_cmd_info.aluLoadPwlLUT = 0;
            info->vx_nn_tp_cmd_info.aluLoadPwlLUTAddress = 0;
            info->vx_nn_tp_cmd_info.aluLoadPwlLUTGlobalMem = 1;
            info->vx_nn_tp_cmd_info.outGlobalMem  = 1;
            info->vx_nn_tp_cmd_info.outBrickMode  = 0;
            info->vx_nn_tp_cmd_info.aluI2FEnable = inFormat == VX_TYPE_FLOAT16 ? 0 : 1;
            info->vx_nn_tp_cmd_info.aluF2IEnable = outFormat == VX_TYPE_FLOAT16 ? 0 : 1;
            info->vx_nn_tp_cmd_info.aluInputPreshift   = 0;
            info->vx_nn_tp_cmd_info.aluOutputPostshift = ((inQFormat == VX_QUANT_DYNAMIC_FIXED_POINT) ? inFPP : 0) - ((outQFormat == VX_QUANT_DYNAMIC_FIXED_POINT) ? outFPP : 0);

            if (tp_type == TP_ROI_POOLING_STEP_2)
                info->vx_nn_tp_cmd_info.aluReluEnable =  conv_cmd_ptr->enable_relu ? 1 : 0;

            if (value_cmd_ptr->e32[0] == 0)
            {
                info->vx_nn_tp_cmd_info.inImageGlobalMem = 1;
                info->vx_nn_tp_cmd_info.inTileListGlobalMem = 1;
                info->vx_nn_tp_cmd_info.outTileSkipAtborder = 1;
                info->vx_nn_tp_cmd_info.aluHorzProcessing = 0x2;
                info->vx_nn_tp_cmd_info.aluHorzProcCount = maxPoolSize - 1;
                info->vx_nn_tp_cmd_info.aluHorzProcStride = 0;
                info->vx_nn_tp_cmd_info.aluVertProcessing = 0;
                info->vx_nn_tp_cmd_info.aluVertProcCount = 0;
                info->vx_nn_tp_cmd_info.aluVertProcStride = 0;
            }
            else if (value_cmd_ptr->e32[0] == 1)
            {
                info->vx_nn_tp_cmd_info.inImageGlobalMem = 1;
                info->vx_nn_tp_cmd_info.inTileListGlobalMem = 1;
                info->vx_nn_tp_cmd_info.outTileSkipAtborder = 0;
                info->vx_nn_tp_cmd_info.inTileSequence = 0x2;
                info->vx_nn_tp_cmd_info.aluHorzProcessing = 0;
                info->vx_nn_tp_cmd_info.aluHorzProcCount = 0;
                info->vx_nn_tp_cmd_info.aluHorzProcStride = 0;
                info->vx_nn_tp_cmd_info.aluVertProcessing = 0;
                info->vx_nn_tp_cmd_info.aluVertProcCount = 0;
                info->vx_nn_tp_cmd_info.aluVertProcStride = 0;
            }
            break;
        }

        case TP_ADD:
        {
            info->vx_nn_tp_cmd_info.inTileSequence = 0x1;
            info->vx_nn_tp_cmd_info.inImageGlobalMem = 1;
            info->vx_nn_tp_cmd_info.aluHorzProcessing = 0x0;
            info->vx_nn_tp_cmd_info.aluHorzProcCount = 0;
            info->vx_nn_tp_cmd_info.aluHorzProcStride = 0;
            info->vx_nn_tp_cmd_info.aluVertProcessing = 0x0;
            info->vx_nn_tp_cmd_info.aluVertProcCount = 1;
            info->vx_nn_tp_cmd_info.aluVertProcStride = 0;
            info->vx_nn_tp_cmd_info.aluZFilterMode = 1;
            info->vx_nn_tp_cmd_info.inWindowZStartOverfetch = 0;
            info->vx_nn_tp_cmd_info.inWindowZEndOverfetch = 0;
            info->vx_nn_tp_cmd_info.outGlobalMem  = 1;
            info->vx_nn_tp_cmd_info.outTileSkipAtborder = 0;
            info->vx_nn_tp_cmd_info.outBrickMode  = 0;
            info->vx_nn_tp_cmd_info.aluFilterPwlSwap = 0;
            info->vx_nn_tp_cmd_info.aluPwlSignSupport = 0;
            info->vx_nn_tp_cmd_info.aluReluEnable = 0;
            info->vx_nn_tp_cmd_info.aluLoadPwlLUT = 0;
            info->vx_nn_tp_cmd_info.aluLoadPwlLUTAddress = 0;
            info->vx_nn_tp_cmd_info.aluLoadPwlLUTGlobalMem = 1;
            info->vx_nn_tp_cmd_info.aluI2FEnable = inFormat == VX_TYPE_FLOAT16 ? 0 : 1;
            info->vx_nn_tp_cmd_info.aluF2IEnable = outFormat == VX_TYPE_FLOAT16 ? 0 : 1;
            info->vx_nn_tp_cmd_info.aluInputPreshift = (inQFormat == VX_QUANT_DYNAMIC_FIXED_POINT) ? inFPP : 0;
            info->vx_nn_tp_cmd_info.aluOutputPostshift = (outQFormat == VX_QUANT_DYNAMIC_FIXED_POINT) ? (0 - outFPP) : 0;
            break;
        }

        case TP_RESHUFFLE:
        case TP_TRANSPOSE:
        case TP_REORG:
        case TP_REVERSE:
        case TP_RNN_INTERLEAVE:
        case TP_REORG_DEPTH2SPACE:
        case TP_REORG_SPACE2DEPTH:
        case TP_REORG_SPACE2BATCH:
        case TP_REORG_BATCH2SPACE:
        case TP_UPSAMPLE:
        case TP_UPSAMPLE_CLIP:
        case TP_DILATE_UPSAMPLE:
        case TP_DILATE_UPSAMPLE2:
        case TP_DILATE_RESHUFFLE:
        case TP_TENSOR_PAD:
        case TP_TENSOR_SQUEEZE:
        case TP_TENSOR_STRIDED_SLICE:
        case TP_TENSOR_SVDF_MAP:
        case TP_LSTM_RESHUFFLE_INPUT:
        case TP_LSTM_STATE_OUT:
        {
            info->vx_nn_tp_cmd_info.inImageGlobalMem    = 1;
            info->vx_nn_tp_cmd_info.outTileSkipAtborder = 0;
            info->vx_nn_tp_cmd_info.outGlobalMem        = 1;
            info->vx_nn_tp_cmd_info.aluFilterPwlSwap    = 0;
            info->vx_nn_tp_cmd_info.aluPwlSignSupport   = 0;
            info->vx_nn_tp_cmd_info.aluReluEnable       = 0;
            info->vx_nn_tp_cmd_info.aluI2FEnable        = (tfQuant && hasInputQuant) ? 1 : (tpRealInt16 ? (inFormat == VX_TYPE_FLOAT16 ? 0 : 1) : 0);
            info->vx_nn_tp_cmd_info.aluF2IEnable        = (tfQuant && hasOutputQuant) ? 1 : (tpRealInt16 ? (outFormat == VX_TYPE_FLOAT16 ? 0 : 1) : 0);
            break;
        }

        case TP_TENSOR_COPY:
        case TP_TENSOR_COPY4CONCAT:
        {
            info->vx_nn_tp_cmd_info.inImageGlobalMem    = 1;
            info->vx_nn_tp_cmd_info.outTileSkipAtborder = 0;
            info->vx_nn_tp_cmd_info.outGlobalMem        = 1;
            info->vx_nn_tp_cmd_info.aluFilterPwlSwap    = 0;
            info->vx_nn_tp_cmd_info.aluPwlSignSupport   = 0;
            info->vx_nn_tp_cmd_info.aluReluEnable       = 0;
            info->vx_nn_tp_cmd_info.aluI2FEnable        = (tfQuant && hasInputQuant) ? 1 : (inFormat == VX_TYPE_FLOAT16 ? 0 : (tpSimpleInt16 ? 0 : 1));
            info->vx_nn_tp_cmd_info.aluF2IEnable        = (tfQuant && hasOutputQuant) ? 1 : (outFormat == VX_TYPE_FLOAT16 ? 0 : (tpSimpleInt16 ? 0 : 1));
            info->vx_nn_tp_cmd_info.aluInputPreshift    = 0;
            info->vx_nn_tp_cmd_info.aluOutputPostshift  = ((inQFormat == VX_QUANT_DYNAMIC_FIXED_POINT) ? inFPP : 0) - ((outQFormat == VX_QUANT_DYNAMIC_FIXED_POINT) ? outFPP : 0);
            break;
        }

        default:
            break;
    }

    if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_TP_REAL_INT16))
    {
        info->vx_nn_tp_cmd_info.aluI2FEnable = inFormat == VX_TYPE_FLOAT16 ? 0 : 1;
        info->vx_nn_tp_cmd_info.aluF2IEnable = outFormat == VX_TYPE_FLOAT16 ? 0 : 1;
    }

    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT) &&
        (hasInputQuant || hasOutputQuant || hasWQuant) &&
        ((tp_type == TP_SINGLE_FC ||
          tp_type == TP_ACTIVATION ||
          tp_type == TP_ROI_POOLING_STEP_1) ||
         (inZP != outZP || inScale != outScale)))
    {
        vx_float32 scale;
        vx_uint32 uintScale, tmpMultiply;
        vx_int32 exp;
        vx_int8 tmpPostShift;
        vx_weights_biases_parameter weights_biases = VX_NULL;

        /* Regular the input or output quant scale value in case it is not TF quant format. */
        if (!hasOutputQuant)
        {
            outScale = (outQFormat == VX_QUANT_DYNAMIC_FIXED_POINT) ? vxnneConvertDynamicFixPointValueToFloat32(1.0f, outFPP) : 1.0f;
        }

        if (!hasInputQuant)
        {
            inScale = (inQFormat == VX_QUANT_DYNAMIC_FIXED_POINT) ? vxnneConvertDynamicFixPointValueToFloat32(1.0f, inFPP) : 1.0f;
        }

        if (tp_type == TP_SINGLE_FC && other_tensor != VX_NULL)
        {
            vx_float32 wScale = 0.0f;

            weights_biases = (vx_weights_biases_parameter) other_tensor;

            if (hasWQuant)
            {
                wScale = WB_WEIGHT_SCALE(weights_biases);
            }
            else
            {
                wScale = (WB_WEIGHT_DATA_FORMAT(weights_biases) == VX_QUANT_DYNAMIC_FIXED_POINT) ? vxnneConvertDynamicFixPointValueToFloat32(1.0f, WB_WEIGHT_FPP(weights_biases)) : 1.0f;
            }
            scale = inScale * wScale / outScale;
        }
        else if ((tp_type == TP_ACTIVATION) &&
                 (value_cmd_ptr->e32[0] != VX_NN_ACTIVATION_RELU) &&
                 (value_cmd_ptr->e32[0] != VX_NN_ACTIVATION_LEAKYRELU) &&
                 (value_cmd_ptr->e32[0] != VX_NN_ACTIVATION_LEAKYRELU_MAX_POOLING))
        {
            scale = 1.0f / outScale;
        }
        else if (tp_type == TP_ROI_POOLING_STEP_1)
        {
            /* Scaling is handled in VPooling, so no scaling in HPooling. */
            scale = 1.0f;
        }
        else
        {
            scale = inScale / outScale;
        }
        uintScale = *((vx_uint32*)(&scale));
        /* RTNE */
        if (uintScale & 0x80)
        {
            if ((uintScale & 0x7F) || (uintScale & 0x100))
            {
                uintScale += 0x100;
            }
        }
        tmpMultiply = uintScale & 0x7FFFFF;
        exp = (uintScale & 0x7F800000) >> 23; /* postShift is Scale's exp */

        tmpPostShift = (vx_int8)(127 - exp);
        info->vx_nn_tp_cmd_info.aluOutputPostshift = tmpPostShift & 0x1F;
        tmpPostShift = tmpPostShift >> 5;
        info->vx_nn_tp_cmd_info.aluOutputPostshiftBit6to5 = tmpPostShift & 3;

        if (0)
        {
            /* 23-bit post multiplier */
            info->vx_nn_tp_cmd_info.aluOutputPostMultiplier = tmpMultiply & 0x7FFF;
            info->vx_nn_tp_cmd_info.aluOutputPostMultiplierBit22to15 = tmpMultiply >> 15;
        }
        else
        {
            /* postMultiply is high 15-bit of Scale's mantissa */
            info->vx_nn_tp_cmd_info.aluOutputPostMultiplier = tmpMultiply >> 8;
            info->vx_nn_tp_cmd_info.aluOutputPostMultiplierBit22to15 = 0;
        }

        info->vx_nn_tp_cmd_info.coefZP = weights_biases == VX_NULL ? 0 : WB_WEIGHT_ZP(weights_biases);
        if (tp_type == TP_ROI_POOLING_STEP_1)
        {
            /* Zero point is handled in VPooling, so no zero point in HPooling. */
            info->vx_nn_tp_cmd_info.inputZP = 0;
            info->vx_nn_tp_cmd_info.outputZP = 0;
        }
        else
        {
            info->vx_nn_tp_cmd_info.inputZP  = hasInputQuant ? inZP : 0;
            info->vx_nn_tp_cmd_info.outputZP = hasOutputQuant ? outZP : 0;
        }
    }
    else
    {
        vx_int8 tmpPostShift = (vx_int8) info->vx_nn_tp_cmd_info.aluOutputPostshift;
        info->vx_nn_tp_cmd_info.aluOutputPostshift = tmpPostShift & 0x1F;
        tmpPostShift = tmpPostShift >> 5;
        info->vx_nn_tp_cmd_info.aluOutputPostshiftBit6to5 = tmpPostShift & 3;
        info->vx_nn_tp_cmd_info.aluOutputPostMultiplier = 0;
    }

    info->vx_nn_tp_cmd_info.inImageDataType = getHWDataFormat(inFormat);
    info->vx_nn_tp_cmd_info.outImageDataType = getHWDataFormat(outFormat);
    info->vx_nn_tp_cmd_info.floatRoundingMode = getHWRoundingMode((vx_nn_round_mode_e)outRMode, outFormat, vx_true_e);
    info->vx_nn_tp_cmd_info.integeroundingMode = 0x1;

    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxnneCommandBuffer_GenerateCommands(
    vx_context                   context,
    vx_node                      node,
    vxnne_operation_command      operation_command,
    vxnne_tensor_info            input,
    vxnne_tensor_info            output,
    vxnne_operation_target_e     target,
    vx_op_param                  parameter,
    vxnne_command_buffer         command_buffer
    )
{
    vx_uint32 i, count = 0;
    vx_status status = VX_SUCCESS;
    vx_nn_cmd_info_u info;
    vx_nn_cmd_split_info_u *sinfo = VX_NULL;

    memset(&info, 0, sizeof(vx_nn_cmd_info_u));

    if (target == VXNNE_OPERATION_TARGET_NN)
    {
        vxmONERROR(vxnneCommandBuffer_GetNNGeneralCommandInfo(context,
                                                              input,
                                                              output,
                                                              parameter,
                                                              &info));

        vxmONERROR(vxnneCommandBuffer_GetNNSplitCommandInfo(context,
                                                            input,
                                                            output,
                                                            parameter,
                                                            &sinfo,
                                                            &count));
    }
    else if (target == VXNNE_OPERATION_TARGET_TP)
    {
        vxmONERROR(vxnneCommandBuffer_GetTPGeneralCommandInfo(context,
                                                              input,
                                                              output,
                                                              parameter,
                                                              &info));

        vxmONERROR(vxnneCommandBuffer_GetTPSplitCommandInfo(context,
                                                            input,
                                                            output,
                                                            parameter,
                                                            &info,
                                                            &sinfo,
                                                            &count));
    }
    else
    {
        vxmASSERT(0);
    }

    vxmASSERT(count > 0);

    command_buffer->commandCount = count;

    if (target == VXNNE_OPERATION_TARGET_NN)
    {
        vxmONERROR(gcoVX_AllocateMemory(NNE_COMMAND_SIZE * command_buffer->commandCount,
                                        &command_buffer->logical, &command_buffer->physical, &command_buffer->node));
    }
    else if (target == VXNNE_OPERATION_TARGET_TP)
    {
        vxmONERROR(gcoVX_AllocateMemory(TP_COMMAND_SIZE * command_buffer->commandCount,
                                        &command_buffer->logical, &command_buffer->physical, &command_buffer->node));
    }

    command_buffer->eventID = (gctUINT_PTR)vxAllocate(command_buffer->commandCount * sizeof(gctUINT32));
    if (command_buffer->eventID == gcvNULL) vxmONERROR(VX_ERROR_NO_MEMORY);

    for (i = 0; i < command_buffer->commandCount; i++)
    {
        vx_uint8_ptr cmdBufPtr, cmdBufTPtr;
        vx_uint32 cmdBufPhys;

        if (target == VXNNE_OPERATION_TARGET_NN)
        {
            vx_weights_biases_parameter weights_biases = (vx_weights_biases_parameter) parameter->other_ref;

            cmdBufPhys = command_buffer->physical + NNE_COMMAND_SIZE * i;
            cmdBufTPtr = cmdBufPtr = (vx_uint8_ptr)command_buffer->logical + NNE_COMMAND_SIZE * i;

            vxMemCopy(&info.vx_nn_tp_cmd_info, &sinfo[i].vx_nn_general_cmd_split_info, sizeof(struct _vx_nn_general_cmd_split_info));
            memset(cmdBufTPtr, 0, NNE_COMMAND_SIZE);
            gcoVX_ProgrammCrossEngine((void*)&info, gcvVX_ACCELERATOR_NN, (void*)&context->options, (vx_uint32_ptr*)&cmdBufTPtr);

#if gcdDUMP
            gcmDUMP(gcvNULL, "#[nn command]\n");
            gcmDUMP_BUFFER(gcvNULL,
                           gcvDUMP_BUFFER_MEMORY,
                           cmdBufPhys,
                           (gctPOINTER)cmdBufPtr,
                           0,
                           NNE_COMMAND_SIZE);
            dumpNNCommandInfo(i, weights_biases->slice_num, &info, NULL);
#endif

            if (node->graph->binarySave)
            {
                vx_uint8_ptr ksDataLogical  = (vx_uint8_ptr)WB_MEM_LOGICAL_ADDR_INDEX(weights_biases, 0);
                vx_uint32 ksDataPhysical    = (vx_uint32)WB_MEM_PHYSICAL_ADDR_INDEX(weights_biases, 0);
                vx_uint32 ksDataSize        = (vx_uint32)WB_MEM_SIZE_INDEX(weights_biases, 0);

                vxoBinaryGraph_SaveTPNNOperation(node,
                                                 cmdBufPtr,
                                                 cmdBufPhys,
                                                 NNE_COMMAND_SIZE,
                                                 VX_BINARY_OPERATION_TYPE_NN,
                                                 ksDataLogical,
                                                 ksDataPhysical,
                                                 ksDataSize,
                                                 input,
                                                 output,
                                                 info.vx_nn_general_cmd_info.inImageAddress,
                                                 info.vx_nn_general_cmd_info.outImageAddress);
            }

            if (!gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_SMALLBATCH_PHASE1))
            {
                command_buffer->eventID[i] = 0;
            }
            else
            {
                command_buffer->eventID[i] = i != command_buffer->commandCount - 1 ? 1 : 0;
            }
        }
        else if (target == VXNNE_OPERATION_TARGET_TP)
        {
            vx_uint8_ptr lutDataLogical = VX_NULL;
            vx_uint32 lutDataPhysical = 0;
            vx_uint32 lutDataSize = 0;

            cmdBufPhys = command_buffer->physical + TP_COMMAND_SIZE * i;
            cmdBufTPtr = cmdBufPtr = (vx_uint8_ptr)command_buffer->logical + TP_COMMAND_SIZE * i;
            memset(cmdBufPtr, 0, TP_COMMAND_SIZE);
            if (parameter->tpType == TP_SINGLE_FC)
            {
                vxMemCopy(&info.vx_nn_tp_cmd_info, &sinfo[i].vx_tp_fc_cmd_split_info, sizeof(struct _vx_tp_fc_cmd_split_info));
            }
            else
            {
                vxMemCopy(&info.vx_nn_tp_cmd_info, &sinfo[i].vx_tp_general_cmd_split_info, sizeof(struct _vx_tp_general_cmd_split_info));
            }

            gcoVX_ProgrammCrossEngine((void*)&info, gcvVX_ACCELERATOR_TP, VX_NULL, (vx_uint32_ptr*)&cmdBufTPtr);

            if (parameter->data_buff != VX_NULL)
            {
                vx_size size = 0;
                vxoTensor_GetTensorBatchArrayViewMemory(parameter->data_buff, 0, (gctPOINTER)&lutDataLogical, &lutDataPhysical);
                vxoTensor_GetTensorWholeSize(parameter->data_buff, &size);
                lutDataSize = (vx_uint32)size;
            }
            else if (parameter->tpType == TP_SINGLE_FC && parameter->other_ref != VX_NULL)
            {
                vx_weights_biases_parameter weights_biases = (vx_weights_biases_parameter) parameter->other_ref;
                lutDataLogical  = (vx_uint8_ptr)WB_MEM_LOGICAL_ADDR_INDEX(weights_biases, i);
                lutDataPhysical = (vx_uint32)WB_MEM_PHYSICAL_ADDR_INDEX(weights_biases, i);
                lutDataSize     = (vx_uint32)WB_MEM_SIZE_INDEX(weights_biases, i);
            }

#if gcdDUMP
            if (parameter->data_buff != VX_NULL || (parameter->tpType == TP_SINGLE_FC && parameter->other_ref != VX_NULL))
            {
                gcmDUMP(gcvNULL, "#[LUT]\n");
                gcmDUMP_BUFFER(gcvNULL,
                               gcvDUMP_BUFFER_MEMORY,
                               lutDataPhysical,
                               (gctPOINTER)lutDataLogical,
                               0,
                               lutDataSize);
            }

            gcmDUMP(gcvNULL, "#[tp command]\n");
            gcmDUMP_BUFFER(gcvNULL,
                           gcvDUMP_BUFFER_MEMORY,
                           cmdBufPhys,
                           (gctPOINTER)cmdBufPtr,
                           0,
                           TP_COMMAND_SIZE);
#endif

            if (node->graph->binarySave)
            {
                vxoBinaryGraph_SaveTPNNOperation(node,
                                                 cmdBufPtr,
                                                 cmdBufPhys,
                                                 TP_COMMAND_SIZE,
                                                 VX_BINARY_OPERATION_TYPE_TP,
                                                 lutDataLogical,
                                                 lutDataPhysical,
                                                 lutDataSize,
                                                 input,
                                                 output,
                                                 info.vx_nn_tp_cmd_info.inImageBaseAddress,
                                                 info.vx_nn_tp_cmd_info.outBaseAddress);
            }

            command_buffer->eventID[i] = i != command_buffer->commandCount - 1 ? 1 : 0;

            if (operation_command->operation->operatorType == VXNNE_OPERATOR_FULLYCONNECTED)
            {
                command_buffer->eventID[i] |= 0x80000000;
            }
        }
    }

OnError:
    if (sinfo != VX_NULL)
    {
        vxFree(sinfo);
        sinfo = VX_NULL;
    }

    if (status != VX_SUCCESS)
    {
        if (command_buffer->node != VX_NULL)
        {
            gcoVX_FreeMemory(command_buffer->node);
            command_buffer->node = VX_NULL;
        }

        if (command_buffer->eventID != VX_NULL)
        {
            vxFree(command_buffer->eventID);
            command_buffer->eventID = VX_NULL;
        }
    }
    return status;
}

VX_INTERNAL_API vx_status vxnneModifyNNLastNoflushBit(
    vx_context                   context,
    vxnne_command_buffer         command_buffer,
    vxnne_operation              operation,
    vx_uint8                     value
    )
{
    vx_uint32 offset = NNE_COMMAND_SIZE * (command_buffer->commandCount - 1);
    vx_uint8_ptr ptr = (vx_uint8_ptr)command_buffer->logical + offset;
    vx_uint32 data = *((vx_uint32_ptr)ptr + 3);

    if (value == 0)
    {
        data &= ~(0x1 << 3);
    }
    else
    {
        data |= 0x1 << 3;
    }

    *((vx_uint32_ptr)ptr + 3) = data;

    vxoBinaryGraph_ReSaveNNTPCommand(operation, command_buffer->physical + offset,
                                     3 * sizeof(vx_uint32), data);

    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_TP_SMALLBATCH_PHASE1))
    {
        command_buffer->eventID[command_buffer->commandCount - 1] = value;
    }

    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxnneModifyTPLastNoflushBit(
    vx_context                   context,
    vxnne_command_buffer         command_buffer,
    vxnne_operation              operation,
    vx_uint8                     value
    )
{
    vx_uint32 offset = TP_COMMAND_SIZE * (command_buffer->commandCount - 1);
    vx_uint8_ptr ptr = (vx_uint8_ptr)command_buffer->logical + offset;
    vx_uint32 data = *((vx_uint32_ptr)ptr + 12);

    if (value == 0)
    {
        data &= ~(0x1 << 30);
    }
    else
    {
        data |= 0x1 << 30;
    }

    *((vx_uint32_ptr)ptr + 12) = data;

    vxoBinaryGraph_ReSaveNNTPCommand(operation, command_buffer->physical + offset,
                                        12 * sizeof(vx_uint32), data);

    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_TP_SMALLBATCH_PHASE1))
    {
        vx_bool flag = command_buffer->eventID[command_buffer->commandCount - 1] & 0x80000000 ? vx_true_e : vx_false_e;
        command_buffer->eventID[command_buffer->commandCount - 1] = flag ? value | 0x80000000 : value;
    }

    return VX_SUCCESS;
}

