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
#include <gc_vx_nn_encoder.h>
#include <gc_vx_nn_util.h>

extern vx_status vxnneAdapter_SWCWHN2WHCN(
    vx_uint8_ptr input_ptr, vx_type_e input_format, vx_enum input_quant_type, vx_uint32 input_depth, vx_uint32 input_width, vx_uint32 input_height, vx_uint32 input_batch, vx_int8 in_fixpoint, vx_int32 in_tf_zp, vx_float32 in_tf_scale, vx_uint8_ptr output_ptr, vx_type_e output_format, vx_enum output_quant_type, vx_uint32 output_depth, vx_uint32 output_width, vx_uint32 output_height, vx_int8 out_fixpoint, vx_int32 out_tf_zp, vx_float32 out_tf_scale, vx_enum out_rounding_mode);



VX_PRIVATE_API vx_bool _adjustNNWeightBias(
    vx_context                   context,
    vx_size                      wb_size,
    vx_uint32_ptr                kx_ptr,
    vx_uint32_ptr                ky_ptr,
    vx_uint32_ptr                kz_ptr,
    vx_uint32_ptr                outz_ptr,
    vx_uint32_ptr                kz_num_ptr,
    vx_uint32_ptr                z_num_ptr,
    vx_uint32_ptr                kz_array,
    vx_uint32_ptr                z_array
    )
{
#define FC_SIZE_MAX 134217728
    vx_uint32 i = 0, tmp = *outz_ptr;
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
        z_array[0] = *outz_ptr;
        *z_num_ptr = 1;
    }

    if (*kx_ptr == 1 && *ky_ptr == 1 && *kz_ptr >= context->options.fcZMax)
    {
        vx_uint32 y = 4;
        vx_uint32 z = *kz_ptr / y;

        while (z >= 16384)
        {
            y <<= 1;
            z = *kz_ptr / y;
        }

        *ky_ptr = y;
        *kz_ptr = z;
    }

    kz_array[0] = *kz_ptr;
    *kz_num_ptr = 1;

    return vx_true_e;
}

VX_PRIVATE_API void _getNNStride(
    vx_enum    layer_type,
    vx_uint32* inputs_dims,
    vx_uint32* outputs_dims,
    vx_uint32* weights_dims,
    vx_uint32  pad_x_left,
    vx_uint32  pad_x_right,
    vx_uint32  pad_y_top,
    vx_uint32  pad_y_bottom,
    vx_enum    down_scale_size_rounding,
    vx_uint32* stride_x,
    vx_uint32* stride_y
    )
{
    vx_uint32 strideX = 1, strideY = 1;

    if (outputs_dims != VX_NULL && inputs_dims != NULL)
    {
        if (layer_type == VX_NN_FULLYCONNECTED_LAYER)
        {
            /* it is fully connected layer */
            strideX = strideY = 1;
        }
        else
        {
            /* Calculate stride = (w + pad_x_left + pad_x_right - weight)/(output_w - 1) */
            strideX = (outputs_dims[0] == 1) ? 1 : vxoNNExternsionConvlutionRound((vx_float32)(inputs_dims[0] + pad_x_left + pad_x_right - weights_dims[0]) / (outputs_dims[0] - 1), down_scale_size_rounding);
            strideY = (outputs_dims[1] == 1) ? 1 : vxoNNExternsionConvlutionRound((vx_float32)(inputs_dims[1] + pad_y_top + pad_y_bottom - weights_dims[1]) / (outputs_dims[1] - 1), down_scale_size_rounding);
        }
    }

    if (stride_x != VX_NULL) *stride_x = *stride_x == 0 ? strideX : *stride_x;
    if (stride_y != VX_NULL) *stride_y = *stride_y == 0 ? strideY : *stride_y;
}

/******************************************************************/

VX_PRIVATE_API vx_status _vxoWeightBias_AllocateMemory(
    vx_context   context,
    vx_memory    memory,
    vx_size*     size_ptr
    )
{
    vx_size size = *size_ptr;

    if (memory->allocated) return VX_SUCCESS;

    if (context->options.enableAllocateContigousMemForKernel)
    {
        if (context->CurrentContigousSize >= size)
        {
            memory->physicals[0] = *context->Physical;
            *context->Physical += (vx_uint32)size;
            memory->logicals[0] = (vx_uint8_ptr)(*context->Logical);
            (*context->Logical) = (*context->Logical) + size;
            memory->allocType = VXNNE_MEM_POOL_TYPE_ORIG_DDR;
            context->CurrentContigousSize -= (vx_uint32)size;
            if (!vxCreateMutex(OUT &memory->writeLocks[0]))
            {
                memory->writeLocks[0] = VX_NULL;
            }
        }
        else
        {
            if (!vxoMemory_AllocateSize(context, memory, size))
                return VX_ERROR_NO_MEMORY;
        }
    }
    else if (!vxoMemory_AllocateSize(context, memory, size))
    {
        return VX_ERROR_NO_MEMORY;
    }

    memory->allocated = vx_true_e;

    vxoMemory_Dump(memory);

    *size_ptr = size;

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status _vxoWeightBias_CalculateSize(
    vx_context                   context,
    vx_weights_biases_parameter  wb,
    vx_enum                      target,
    vx_uint32*                   kernel_per_core,
    vx_uint32                    z_offset,
    vx_bool                      to_allocate,
    vx_size*                     min_kernel_total_buffer_size,
    vx_size*                     min_kernel_buffer_sizes,
    vx_uint8_ptr*                min_zero_run_lens_ptr,
    vx_uint32_ptr                max_zero_run_lens
    )
{
    vx_status status;
    vx_uint32 i = 0, j = 0, index = 0, kzOffset = 0, oneFilterSize, weightSize, kzNum = 1, zNum = 1;
    vx_uint32 nonZeroCount = 0, nonZeroTotalCount = 0, totalCount = 0, allTotalCount = 0, sliceCount = 0, filterCount, kernelPerCore = 0;
    vx_uint32 zArray[MAX_ZGROUP_COUNT], kzArray[MAX_KZGROUP_COUNT];
    vx_size minKernelBufferSizes[MAX_ZGROUP_COUNT*MAX_KZGROUP_COUNT];
    vx_size minTotalKernelBufferSize = 0, origKernelBufferSize = 0, origTotalKernelBufferSize = 0, weightDataBytesOffset = 0, biasDataDWordOffset = 0;
    vx_uint8_ptr weightPtr = VX_NULL, biasPtr = VX_NULL;
    vx_uint8_ptr minZeroRunLensBase = VX_NULL, minZeroRunLens;
    vx_bool hasNNPerFilterPostMultiply = vx_false_e;

    weightSize = (vx_uint32)vxDataType_GetSize((vx_type_e)WB_WEIGHT_DATA_FORMAT(wb));
    oneFilterSize = WB_KERNEL_X(wb) * WB_KERNEL_Y(wb) * WB_KERNEL_Z(wb) * weightSize;

    vxoTensor_GetTensorViewMemory(WB_WEIGHT_TENSOR(wb), (gctPOINTER*)&weightPtr, VX_NULL);

    if (WB_BIAS_TENSOR(wb) != VX_NULL)
        vxoTensor_GetTensorViewMemory(WB_BIAS_TENSOR(wb), (gctPOINTER*)&biasPtr, VX_NULL);

    hasNNPerFilterPostMultiply = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PER_CHANNEL_QUANT) &&
                       ((gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PRELU) && wb->alpha_ref!= VX_NULL && WB_OPERATION_TYPE(wb) == VX_NN_CONV_PRELU) ||
                       ((WB_WEIGHT_QUANT_FORMAT(wb) == VX_QUANT_AFFINE_SCALE_PER_CHANNEL) && (WB_WEIGHT_DATA_FORMAT(wb) == VX_TYPE_UINT8 || WB_WEIGHT_DATA_FORMAT(wb) == VX_TYPE_INT8 )));

    if (target == VXNNE_OPERATION_TARGET_TP)
    {
        vx_uint32 coreCount = context->nnConfig.fixedFeature.tpCoreCount + context->nnConfig.fixedFeature.tpliteCoreCount;

        sliceCount = WB_KERNEL_Z(wb);
        filterCount = WB_OUTPUT_Z(wb);

        /* TP FC can handle up to 512 filters. */
        if (context->options.enableMultiTP && filterCount >= 2 * coreCount)
        {
            /* multi TP path */
            vx_uint32 snum = (filterCount + TP_FC_Z_MAX - 1) / TP_FC_Z_MAX;
            i = snum % coreCount ? gcmALIGN_NP2(snum, coreCount) : snum;
            calculateSplitSize(filterCount, i, zArray, VX_NULL);
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
    }
    if (target == VXNNE_OPERATION_TARGET_TP && i <= MAX_ZGROUP_COUNT)
    {
        vx_uint32 splitIndex = 0;
        zNum = i;
        kzNum = sliceCount % MAX_TP_FC_KZ_SIZE == 0 ? sliceCount / MAX_TP_FC_KZ_SIZE : sliceCount / MAX_TP_FC_KZ_SIZE + 1;
        for(splitIndex = 0; splitIndex < kzNum; splitIndex ++)
        {
            if(splitIndex == (kzNum - 1))
                kzArray[splitIndex] = (sliceCount - MAX_TP_FC_KZ_SIZE * splitIndex);
            else
                kzArray[splitIndex] = MAX_TP_FC_KZ_SIZE;
        }

        WB_COMPRESS_TARGET(wb) = VXNNE_OPERATION_TARGET_TP;

        if (to_allocate)
            minZeroRunLensBase = (vx_uint8 *)vxAllocate(WB_KERNEL_Z(wb) * zNum);
    }
    else
    {
        vx_uint32 kx = WB_KERNEL_X(wb), ky = WB_KERNEL_Y(wb), kz = WB_KERNEL_Z(wb), z = WB_OUTPUT_Z(wb);
        vx_uint32 asize = oneFilterSize * WB_OUTPUT_Z(wb);
        if (!_adjustNNWeightBias(context, asize, &kx, &ky, &kz, &z, &kzNum, &zNum, kzArray, zArray))
        {
            status = VX_ERROR_NO_RESOURCES;
            goto error;
        }

        if (kernel_per_core == VX_NULL || *kernel_per_core == 0)
        {
            status = VX_ERROR_INVALID_VALUE;
            goto error;
        }
        else
        {
            kernelPerCore = *kernel_per_core;
            for (i = 0; i < zNum; i++)
            {
                if (kernelPerCore > zArray[i])
                {
                    kernelPerCore = zArray[i];
                }
            }
            if (*kernel_per_core != kernelPerCore)
            {
                vxInfo("kernel_per_core is out of max size. Change from %d to %d\n", *kernel_per_core, kernelPerCore);
                *kernel_per_core = kernelPerCore;
            }
        }

        WB_KERNEL_X(wb) = kx;
        WB_KERNEL_Y(wb) = ky;
        WB_KERNEL_Z(wb) = kz;
        WB_OUTPUT_Z(wb) = z;

        WB_COMPRESS_TARGET(wb) = VXNNE_OPERATION_TARGET_NN;

        if (to_allocate)
            minZeroRunLensBase = (vx_uint8 *)vxAllocate(kzNum * zNum);
    }

    if (to_allocate && minZeroRunLensBase == VX_NULL)
    {
        status = VX_ERROR_NO_MEMORY;
        goto error;
    }
    else
    {
        minZeroRunLens = minZeroRunLensBase;
    }

    if (WB_SLICE_ARRAY(wb) != VX_NULL && WB_TOTAL_SLICE_NUM(wb) != kzNum * zNum)
    {
        for (i = 0; i < WB_TOTAL_SLICE_NUM(wb); i++)
        {
            if (WB_HUFFMAN_CONFIG_INDEX(wb, i) != VX_NULL)
            {
                vxFree(WB_HUFFMAN_CONFIG_INDEX(wb, i));
                WB_HUFFMAN_CONFIG_INDEX(wb, i) = VX_NULL;
            }
        }

        vxFree(WB_SLICE_ARRAY(wb));
        WB_SLICE_ARRAY(wb) = VX_NULL;
    }

    /* Create slice for each split wb */
    WB_KERNEL_Z_SLICE_NUM(wb) = kzNum;
    WB_OUTPUT_Z_SLICE_NUM(wb) = zNum;
    WB_TOTAL_SLICE_NUM(wb) = kzNum * zNum;

    if (WB_SLICE_ARRAY(wb) == VX_NULL)
    {
        WB_SLICE_ARRAY(wb) = (vx_weight_bias_slice_item)vxAllocateAndZeroMemory(sizeof(vx_weight_bias_slice_item_s) * WB_TOTAL_SLICE_NUM(wb));
        if (WB_SLICE_ARRAY(wb) == VX_NULL)
        {
            status = VX_ERROR_NO_MEMORY;
            goto error;
        }
    }

    for (i = 0; i < WB_OUTPUT_Z_SLICE_NUM(wb); i++)
    {
        kzOffset = 0;
        filterCount = 0;

        for (j = 0; j < WB_KERNEL_Z_SLICE_NUM(wb); j++)
        {
            filterCount = WB_OUTPUT_Z_INDEX(wb, index) = zArray[i];
            sliceCount = WB_KERNEL_Z_INDEX(wb, index) = kzArray[j];

            if ((vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP_COMPRESSION_ENHANCEMENT) && WB_IS_TP_COMPRESS(wb)) ||
                (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_COEF_COMPRESSION_ENHANCEMENT) && !WB_IS_TP_COMPRESS(wb)))
            {
                if (WB_HUFFMAN_CONFIG_INDEX(wb, index) == VX_NULL)
                {
                    WB_HUFFMAN_CONFIG_INDEX(wb, index) = (vx_weight_bias_huffman_cfg)vxAllocateAndZeroMemory(sizeof(vx_weight_bias_huffman_cfg_s));
                    if (WB_HUFFMAN_CONFIG_INDEX(wb, index) == VX_NULL)
                    {
                        status = VX_ERROR_NO_MEMORY;
                        goto error;
                    }
                }
            }

            if (WB_IS_TP_COMPRESS(wb))
            {
                calculateWeightBiasTPBufferRelatedSize(
                    context,
                    WB_HUFFMAN_CONFIG_INDEX(wb, index),
                    WB_KERNEL_X(wb),
                    WB_KERNEL_Y(wb),
                    WB_KERNEL_Z(wb),
                    sliceCount,
                    filterCount,
                    WB_KERNEL_Z(wb),
                    WB_WEIGHT_DATA_FORMAT(wb),
                    WB_BIAS_DATA_FORMAT(wb),
                    WB_SKIP_VALUE(wb),
                    WB_SET_ZERO_LENGTH(wb) >= 0 && WB_SET_ZERO_LENGTH(wb) <= 9 ? WB_SET_ZERO_LENGTH(wb) : (vx_int8)context->options.tpZeroRunLen,
                    weightPtr + kzOffset + weightDataBytesOffset,
                    &totalCount,
                    &nonZeroCount,
                    &origKernelBufferSize,
                    &minKernelBufferSizes[index],
                    minZeroRunLens);

                if (minZeroRunLens != VX_NULL)
                    minZeroRunLens += sliceCount;
            }
            else
            {
                calculateWeightBiasStreamRelatedSize(
                    context,
                    WB_HUFFMAN_CONFIG_INDEX(wb, index),
                    WB_KERNEL_X(wb),
                    WB_KERNEL_Y(wb),
                    sliceCount, /* slice */
                    filterCount, /* z count */
                    kernelPerCore, /* kernel per core */
                    WB_WEIGHT_DATA_FORMAT(wb),
                    (vx_uint32*)(WB_WEIGHT_ZP_POINTER(wb)),
                    WB_BIAS_DATA_FORMAT(wb),
                    WB_INPUT_ZP(wb),
                    WB_SKIP_VALUE(wb),
                    z_offset,
                    WB_KERNEL_Z(wb) <= 1 ? 0 : WB_SET_ZERO_LENGTH(wb),
                    (vx_uint8)context->options.nnZeroRunLen,
                    WB_IS_DEPTH_WISE(wb),
                    weightPtr + kzOffset + weightDataBytesOffset,
                    (biasPtr != VX_NULL) ? biasPtr + biasDataDWordOffset : VX_NULL,
                    &totalCount,
                    &nonZeroCount,
                    &origKernelBufferSize,
                    &minKernelBufferSizes[index],
                    minZeroRunLens != VX_NULL ? &minZeroRunLens[index] : VX_NULL,
                    max_zero_run_lens != VX_NULL ? &max_zero_run_lens[index] : VX_NULL,
                    wb);
            }

            allTotalCount += totalCount;
            nonZeroTotalCount += nonZeroCount;
            origTotalKernelBufferSize += origKernelBufferSize;
            minTotalKernelBufferSize += minKernelBufferSizes[index];

            if (min_kernel_buffer_sizes != VX_NULL)
                min_kernel_buffer_sizes[index] = minKernelBufferSizes[index];

            kzOffset += sliceCount * weightSize;
            index++;
        }

        weightDataBytesOffset += oneFilterSize * filterCount;
        biasDataDWordOffset += filterCount;
    }

    if ((WB_KERNEL_X(wb) == 1 && WB_KERNEL_Y(wb) == 1 &&
         (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3) || vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6)) &&
         (WB_WEIGHT_DATA_FORMAT(wb) == VX_TYPE_INT8 || WB_WEIGHT_DATA_FORMAT(wb) == VX_TYPE_UINT8)) &&
        !gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_ZDP3_NO_COMPRESS_FIX) &&
        !WB_IS_TP_COMPRESS(wb))
    {
        /*force non_zero_ratio to 1 as HW will not be skipping any zero without ZDP3_NO_COMPRESS_FIX*/
        WB_NON_ZERO_RATIO(wb) = 1.0f;
    }
    else
    {
        WB_NON_ZERO_RATIO(wb) = gcmMIN(1.0f, (vx_float64)nonZeroCount / allTotalCount);
    }

    WB_COMPRESS_RATIO(wb) = (vx_float64)minTotalKernelBufferSize / origTotalKernelBufferSize;

    if (min_kernel_total_buffer_size != VX_NULL)
        *min_kernel_total_buffer_size = minTotalKernelBufferSize;
    if (min_zero_run_lens_ptr != VX_NULL)
        *min_zero_run_lens_ptr = minZeroRunLensBase;

    return VX_SUCCESS;

error:
    if (WB_SLICE_ARRAY(wb) != VX_NULL)
    {
        for (i = 0; i < WB_TOTAL_SLICE_NUM(wb); i++)
        {
            if (WB_HUFFMAN_CONFIG_INDEX(wb, i) != VX_NULL)
            {
                vxFree(WB_HUFFMAN_CONFIG_INDEX(wb, i));
                WB_HUFFMAN_CONFIG_INDEX(wb, i) = VX_NULL;
            }
        }

        vxFree(WB_SLICE_ARRAY(wb));
        WB_SLICE_ARRAY(wb) = VX_NULL;
    }

    return status;
}

vx_status calculatePostMultiAndPostShift(vx_context context, vx_weights_biases_parameter wb, vx_uint32 *post_mul_shift, vx_uint32 *neg_post_mul_shift)
{
    vx_uint32   filterCount = WB_OUTPUT_Z_INDEX(wb, 0);
    vx_uint32   filterIndex = 0;
    vx_float32  inputScale = WB_INPUT_SCALE(wb);
    vx_float32  outputScale = WB_OUTPUT_SCALE(wb);
    vx_float32  weightScale = 1.f;

    gctPOINTER  alphaPtr = VX_NULL;
    vx_int32    alphaZP = 0;
    vx_int8     alphaFP = 0;
    vx_float32  alphaScale = 0;
    vx_type_e   alphaFormat = VX_TYPE_FLOAT16;
    vx_enum     alphaQuantFormat = 0;

    if (wb->alpha_ref != VX_NULL && WB_OPERATION_TYPE(wb) == VX_NN_CONV_PRELU)
    {
        vx_tensor alpha_tensor;
        vxmASSERT(vxoReference_GetType(wb->alpha_ref) == VX_TYPE_TENSOR);

        alpha_tensor = (vx_tensor)wb->alpha_ref;

        vxoTensor_GetTensorViewMemory(alpha_tensor, (gctPOINTER*)&alphaPtr, VX_NULL);

        alphaZP           = TENSOR_TF_ZEROPOINT(alpha_tensor);
        alphaFP          = TENSOR_POS(alpha_tensor);
        alphaScale        = TENSOR_TF_SCALE(alpha_tensor);
        alphaFormat       = (vx_type_e)TENSOR_DATA_TYPE(alpha_tensor);
        alphaQuantFormat  = TENSOR_QUANT_TYPE(alpha_tensor);
    }

    for(filterIndex = 0; filterIndex < filterCount; ++filterIndex)
    {
        vx_float32 scale = 0;
        vx_uint32 uintScale = 0;
        vx_uint32 exp = 0;

        if (WB_WEIGHT_QUANT_FORMAT(wb) == VX_QUANT_AFFINE_SCALE_PER_CHANNEL)
            weightScale = WB_WEIGHT_SCALE(wb, filterIndex);
        else
            weightScale = WB_WEIGHT_SCALE(wb, 0);
        scale = inputScale * weightScale/outputScale;
        uintScale = *((vx_uint32*)(&scale));
        exp = (uintScale & 0x7F800000) >> 23; /* postShift is Scale's exp*/

        if(gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_FLOAT_POST_MULT))
        {
            post_mul_shift[filterIndex] = uintScale;
        }
        else
        {
            post_mul_shift[filterIndex] = (((127 - (vx_int8)exp) & 0x7F) << 23) | (uintScale & 0x7FFFFF);
        }

        if (wb->alpha_ref != VX_NULL &&
            neg_post_mul_shift != VX_NULL &&
            WB_OPERATION_TYPE(wb) == VX_NN_CONV_PRELU)
        {
            vx_float32 aV = vxnneGetDataExt(alphaFormat, alphaQuantFormat, filterIndex, (vx_uint8_ptr)alphaPtr, alphaFP, alphaZP, alphaScale);
            scale *= aV;

            if(gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_FLOAT_POST_MULT))
            {
                uintScale = *((vx_uint32*)(&scale));
                neg_post_mul_shift[filterIndex] = uintScale;
            }
            else
            {
                if (aV == 0.f)
                    uintScale = 0x20000000;
                else
                    uintScale = *((vx_uint32*)(&scale));

                exp = (uintScale & 0x7F800000) >> 23; /* negPostShift is Scale's exp*/
                neg_post_mul_shift[filterIndex] = (uintScale & 0x80000000) |(((127 - (vx_int8)exp) & 0x7F) << 23) | (uintScale & 0x7FFFFF); /* sign+exp(-bias) +mantissa*/

            }
        }
    }
    return VX_SUCCESS;
}

vx_status checkWeightZp(vx_context context, vx_weights_biases_parameter wb)
{
    vx_uint32 filterIndex = 0;

    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PER_CHANNEL_QUANT))
    {
        if (WB_WEIGHT_DATA_FORMAT(wb) == VX_TYPE_INT8)
        {
            for(filterIndex = 0 ; filterIndex < WB_WEIGHT_DIMS_SIZES(wb)[3]; ++filterIndex)
            {
                vxmASSERT(WB_WEIGHT_ZP(wb, filterIndex) == 0);
            }
        }
        else if (WB_WEIGHT_DATA_FORMAT(wb) == VX_TYPE_UINT8)
        {
            if(0)/*(! COFE_FEATURE))*/
            {
                for(filterIndex = 0 ; filterIndex < WB_WEIGHT_DIMS_SIZES(wb)[3]; ++filterIndex)
                {
                    vxmASSERT(WB_WEIGHT_ZP(wb, filterIndex) == WB_WEIGHT_ZP(wb, 0));
                }
            }
        }
    }
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status _vxoWeightBias_Compress(
    vx_context                   context,
    vx_weights_biases_parameter  wb,
    vx_uint32                    kernel_per_core,
    vx_uint32                    z_offset,
    vx_size*                     min_kernel_buffer_sizes,
    vx_uint8_ptr                 min_zero_run_lens,
    vx_uint32_ptr                max_zero_run_lens
    )
{
    vx_status status = VX_SUCCESS;
    vx_size weightDataBytesOffset = 0, biasDataDWordOffset = 0, compressDataBytesOffset = 0;
    vx_uint32 i, j, index = 0, kzOffset = 0, oneFilterSize, weightSize;
    vx_uint8_ptr weightPtr = VX_NULL, alphaPtr = VX_NULL;
    vx_uint32_ptr biasPtr = VX_NULL;
    vx_uint8_ptr zrlTmpPtr = min_zero_run_lens;

    weightSize = (vx_uint32)vxDataType_GetSize((vx_type_e)WB_WEIGHT_DATA_FORMAT(wb));
    oneFilterSize = WB_KERNEL_X(wb) * WB_KERNEL_Y(wb) * WB_KERNEL_Z(wb) * weightSize;

    vxoTensor_GetTensorViewMemory(WB_WEIGHT_TENSOR(wb), (gctPOINTER*)&weightPtr, VX_NULL);

    if (WB_BIAS_TENSOR(wb) != VX_NULL)
        vxoTensor_GetTensorViewMemory(WB_BIAS_TENSOR(wb), (gctPOINTER*)&biasPtr, VX_NULL);

    if (WB_ALPHA_REF(wb) != VX_NULL && (vxoReference_GetType(wb->alpha_ref) == VX_TYPE_TENSOR))
        vxoTensor_GetTensorViewMemory((vx_tensor)(WB_ALPHA_REF(wb)), (gctPOINTER*)&alphaPtr, VX_NULL);

    vxAcquireMutex(WB_MEMORY_LOCK(wb));

    for (i = 0; i < WB_OUTPUT_Z_SLICE_NUM(wb); i++)
    {
        vx_uint32 filterCount = WB_OUTPUT_Z_INDEX(wb, index);

        kzOffset = 0;

        for (j = 0; j < WB_KERNEL_Z_SLICE_NUM(wb); j++)
        {
            vx_uint8_ptr kernelBufferPtr = WB_MEM_LOGICAL_BASE_ADDR(wb) + compressDataBytesOffset;

            vx_uint32 sliceCount = WB_KERNEL_Z_INDEX(wb, index);

            vx_size fillSize;

            if (WB_IS_TP_COMPRESS(wb))
            {
                if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP_COMPRESSION_ENHANCEMENT))
                {
                     vx_uint32* biasStartAddr = VX_NULL;
                     if(WB_BIAS_DATA_FORMAT(wb) == VX_TYPE_INT64)
                        biasStartAddr = !j ? ((biasPtr != VX_NULL) ? (vx_uint32*)(((vx_int64*)biasPtr) + biasDataDWordOffset) : VX_NULL) : VX_NULL;
                    else if(WB_BIAS_DATA_FORMAT(wb) == VX_TYPE_INT32 || WB_BIAS_DATA_FORMAT(wb) == VX_TYPE_FLOAT32)
                        biasStartAddr = !j ? ((biasPtr != VX_NULL) ? biasPtr + biasDataDWordOffset : VX_NULL) : VX_NULL;
                    else
                        vxmASSERT(0);

                    fillSize = fillinTPKernelBufferHuffman(
                        context,
                        WB_HUFFMAN_CONFIG_INDEX(wb, index),
                        zrlTmpPtr,
                        sliceCount,
                        filterCount,
                        WB_KERNEL_Z(wb),
                        WB_WEIGHT_DATA_FORMAT(wb),
                        WB_BIAS_DATA_FORMAT(wb),
                        WB_BIAS_FPP(wb),
                        WB_SKIP_VALUE(wb),
                        kernelBufferPtr,
                        weightPtr + kzOffset + weightDataBytesOffset,
                        biasStartAddr);
                }
                else
                {
                    vx_uint32* biasStartAddr = VX_NULL;
                    if(WB_BIAS_DATA_FORMAT(wb) == VX_TYPE_INT64)
                        biasStartAddr = !j ? ((biasPtr != VX_NULL) ? (vx_uint32*)(((vx_int64*)biasPtr) + biasDataDWordOffset) : VX_NULL) : VX_NULL;
                    else if(WB_BIAS_DATA_FORMAT(wb) == VX_TYPE_INT32 || WB_BIAS_DATA_FORMAT(wb) == VX_TYPE_FLOAT32)
                        biasStartAddr = !j ? ((biasPtr != VX_NULL) ? biasPtr + biasDataDWordOffset : VX_NULL) : VX_NULL;
                    else
                        vxmASSERT(0);

                    fillSize = fillinTPKernelBuffer(
                        context,
                        zrlTmpPtr,
                        sliceCount,
                        filterCount,
                        WB_KERNEL_Z(wb),
                        WB_WEIGHT_DATA_FORMAT(wb),
                        WB_BIAS_DATA_FORMAT(wb),
                        WB_BIAS_FPP(wb),
                        WB_SKIP_VALUE(wb),
                        kernelBufferPtr,
                        weightPtr + kzOffset + weightDataBytesOffset,
                        biasStartAddr);
                }

                vxmASSERT(fillSize <= min_kernel_buffer_sizes[index]);

                zrlTmpPtr += sliceCount;

                WB_MEM_OFFSET_INDEX(wb, index) = compressDataBytesOffset;
                WB_MEM_SIZE_INDEX(wb, index) = min_kernel_buffer_sizes[index];
            }
            else /* NN */
            {
                vx_size kernelAlignStreamSize = 0, kernelStreamFullCacheSize = 0, kernelMaxStreamSizePerCore = 0;
                vx_uint32 numOfVz = 0;

                if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP0))
                {
                    vx_uint32* postMulAndShift = VX_NULL;
                    vx_uint32* negPostMulAndShift = VX_NULL;

                    if(gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PER_CHANNEL_QUANT) &&
                       ((gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PRELU) && wb->alpha_ref!= VX_NULL && WB_OPERATION_TYPE(wb) == VX_NN_CONV_PRELU) ||
                       ((WB_WEIGHT_QUANT_FORMAT(wb) == VX_QUANT_AFFINE_SCALE_PER_CHANNEL)&&(WB_WEIGHT_DATA_FORMAT(wb) == VX_TYPE_UINT8 || WB_WEIGHT_DATA_FORMAT(wb) == VX_TYPE_INT8 ))))
                    {
                        checkWeightZp(context, wb);
                        postMulAndShift = (vx_uint32*)vxAllocateAndZeroMemory(WB_WEIGHT_DIMS_SIZES(wb)[3] * sizeof(vx_uint32));

                        if(gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PRELU) && wb->alpha_ref!= VX_NULL && WB_OPERATION_TYPE(wb) == VX_NN_CONV_PRELU)
                        {
                            negPostMulAndShift = (vx_uint32*)vxAllocateAndZeroMemory(WB_WEIGHT_DIMS_SIZES(wb)[3] * sizeof(vx_uint32));
                        }
                        calculatePostMultiAndPostShift(context, wb, postMulAndShift, negPostMulAndShift);
                    }

                    fillSize = fillinKernelBufferV8Huffman(
                        context,
                        WB_HUFFMAN_CONFIG_INDEX(wb, index),
                        WB_INPUT_FORMAT(wb),
                        WB_KERNEL_X(wb),
                        WB_KERNEL_Y(wb),
                        sliceCount,
                        filterCount,
                        kernel_per_core,
                        WB_WEIGHT_DATA_FORMAT(wb),
                        WB_WEIGHT_QUANT_FORMAT(wb),
                        WB_WEIGHT_ZP_POINTER(wb),
                        WB_BIAS_DATA_FORMAT(wb),
                        WB_INPUT_ZP(wb),
                        WB_SKIP_VALUE(wb),
                        z_offset,
                        WB_IS_DEPTH_WISE(wb),
                        kernelBufferPtr,
                        weightPtr + kzOffset + weightDataBytesOffset,
                        (biasPtr != VX_NULL) ? biasPtr + biasDataDWordOffset : VX_NULL,
                        postMulAndShift,
                        negPostMulAndShift,
                        &kernelAlignStreamSize,
                        &kernelStreamFullCacheSize,
                        &kernelMaxStreamSizePerCore);

                    if(postMulAndShift != VX_NULL)
                    {
                        vxFree(postMulAndShift);
                        postMulAndShift = VX_NULL;
                    }
                    if(negPostMulAndShift != VX_NULL)
                    {
                        vxFree(negPostMulAndShift);
                        negPostMulAndShift = VX_NULL;
                    }
                }
                else if (WB_IS_DEPTH_WISE(wb))
                {
                    vx_uint32* postMulAndShift = VX_NULL;
                    vx_uint32* negPostMulAndShift = VX_NULL;

                    if(gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PER_CHANNEL_QUANT) && (WB_WEIGHT_QUANT_FORMAT(wb) == VX_QUANT_AFFINE_SCALE_PER_CHANNEL)&&(WB_WEIGHT_DATA_FORMAT(wb) == VX_TYPE_UINT8 || WB_WEIGHT_DATA_FORMAT(wb) == VX_TYPE_INT8 ))
                    {
                        postMulAndShift = (vx_uint32*)vxAllocateAndZeroMemory(WB_OUTPUT_Z_INDEX(wb, 0) * sizeof(vx_uint32));
                        negPostMulAndShift = (vx_uint32 *)vxAllocateAndZeroMemory(WB_OUTPUT_Z_INDEX(wb, 0) * sizeof(vx_uint8));
                        calculatePostMultiAndPostShift(context, wb, postMulAndShift, negPostMulAndShift);
                    }
                    fillSize = fillinDepthWiseKernelBuffer(
                        context,
                        min_zero_run_lens[index],
                        max_zero_run_lens[index],
                        WB_KERNEL_X(wb),
                        WB_KERNEL_Y(wb),
                        sliceCount,
                        filterCount,
                        kernel_per_core,
                        WB_WEIGHT_DATA_FORMAT(wb),
                        WB_WEIGHT_QUANT_FORMAT(wb),
                        WB_WEIGHT_ZP(wb, 0),
                        WB_BIAS_DATA_FORMAT(wb),
                        WB_INPUT_ZP(wb),
                        WB_SKIP_VALUE(wb),
                        z_offset,
                        kernelBufferPtr,
                        weightPtr + kzOffset + weightDataBytesOffset,
                        (biasPtr != VX_NULL) ? biasPtr + biasDataDWordOffset : VX_NULL,
                        &numOfVz,
                        &kernelAlignStreamSize,
                        &kernelStreamFullCacheSize,
                        &kernelMaxStreamSizePerCore);
                }
                else if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_COEF_COMPRESSION_ENHANCEMENT))
                {
                    fillSize = fillinKernelBufferHuffman(
                        context,
                        WB_HUFFMAN_CONFIG_INDEX(wb, index),
                        WB_KERNEL_X(wb),
                        WB_KERNEL_Y(wb),
                        sliceCount,
                        filterCount,
                        kernel_per_core,
                        WB_WEIGHT_DATA_FORMAT(wb),
                        WB_WEIGHT_ZP(wb, 0),
                        WB_BIAS_DATA_FORMAT(wb),
                        WB_INPUT_ZP(wb),
                        WB_SKIP_VALUE(wb),
                        z_offset,
                        kernelBufferPtr,
                        weightPtr + kzOffset + weightDataBytesOffset,
                        (biasPtr != VX_NULL) ? biasPtr + biasDataDWordOffset : VX_NULL,
                        &kernelAlignStreamSize,
                        &kernelStreamFullCacheSize,
                        &kernelMaxStreamSizePerCore);
                }
                else
                {
                    if (WB_ZOFFSET_HANDLE_INDEX(wb, index) == VX_NULL)
                    {
                        WB_ZOFFSET_HANDLE_INDEX(wb, index) = (vx_weight_bias_z_offset)vxAllocateAndZeroMemory(filterCount * sizeof(vx_weight_bias_z_offset_s));
                        if (WB_ZOFFSET_HANDLE_INDEX(wb, index) == VX_NULL)
                        {
                            status = VX_ERROR_NO_MEMORY;
                            goto exit;
                        }
                    }

                    if (context->options.enableNonZeroBalance)
                    {
                        fillSize = fillinKernelBufferBalance(
                            context,
                            min_zero_run_lens[index],
                            max_zero_run_lens[index],
                            WB_KERNEL_X(wb),
                            WB_KERNEL_Y(wb),
                            sliceCount,
                            filterCount,
                            kernel_per_core,
                            WB_WEIGHT_DATA_FORMAT(wb),
                            WB_WEIGHT_ZP(wb, 0),
                            WB_BIAS_DATA_FORMAT(wb),
                            WB_INPUT_ZP(wb),
                            WB_SKIP_VALUE(wb),
                            z_offset,
                            kernelBufferPtr,
                            weightPtr + kzOffset + weightDataBytesOffset,
                            (biasPtr != VX_NULL) ? biasPtr + biasDataDWordOffset : VX_NULL,
                            WB_ZOFFSET_HANDLE_INDEX(wb, index),
                            &numOfVz,
                            &kernelAlignStreamSize,
                            &kernelStreamFullCacheSize,
                            &kernelMaxStreamSizePerCore);
                    }
                    else
                    {
                        fillSize = fillinKernelBuffer(
                            context,
                            min_zero_run_lens[index],
                            max_zero_run_lens[index],
                            WB_KERNEL_X(wb),
                            WB_KERNEL_Y(wb),
                            sliceCount,
                            filterCount,
                            kernel_per_core,
                            WB_WEIGHT_DATA_FORMAT(wb),
                            WB_WEIGHT_ZP(wb, 0),
                            WB_BIAS_DATA_FORMAT(wb),
                            WB_INPUT_ZP(wb),
                            WB_SKIP_VALUE(wb),
                            z_offset,
                            kernelBufferPtr,
                            weightPtr + kzOffset + weightDataBytesOffset,
                            (biasPtr != VX_NULL) ? biasPtr + biasDataDWordOffset : VX_NULL,
                            WB_ZOFFSET_HANDLE_INDEX(wb, index),
                            &numOfVz,
                            &kernelAlignStreamSize,
                            &kernelStreamFullCacheSize,
                            &kernelMaxStreamSizePerCore);
                    }
                }

                vxmASSERT(fillSize <= min_kernel_buffer_sizes[index]);

                if (kernelAlignStreamSize != 0) WB_STREAM_ALIGN_SIZE_INDEX(wb, index) = kernelAlignStreamSize;
                if (kernelStreamFullCacheSize != 0) WB_STREAM_FULL_CACHE_SIZE_INDEX(wb, index) = kernelStreamFullCacheSize;
                if (kernelMaxStreamSizePerCore != 0) WB_STREAM_MAX_SIZE_PERCORE_INDEX(wb, index) = kernelMaxStreamSizePerCore;
                if (numOfVz != 0) WB_NUM_OF_VZ_INDEX(wb, index) = numOfVz;

                WB_MEM_OFFSET_INDEX(wb, index) = compressDataBytesOffset;
                WB_MEM_SIZE_INDEX(wb, index) = min_kernel_buffer_sizes[index];
            }

            kzOffset += sliceCount * weightSize;
            compressDataBytesOffset += WB_MEM_SIZE_INDEX(wb, index);

            index++;
        }

        weightDataBytesOffset += oneFilterSize * filterCount;
        biasDataDWordOffset += filterCount;
    }

exit:
    vxReleaseMutex(WB_MEMORY_LOCK(wb));

    if (status != VX_SUCCESS)
    {
        if (WB_SLICE_ARRAY(wb) != VX_NULL)
        {
            for (i = 0; i < WB_TOTAL_SLICE_NUM(wb); i++)
            {
                if (WB_ZOFFSET_HANDLE_INDEX(wb, i) != VX_NULL)
                {
                    vxFree(WB_ZOFFSET_HANDLE_INDEX(wb, i));
                    WB_ZOFFSET_HANDLE_INDEX(wb, i) = VX_NULL;
                }
            }
        }
    }

    return status;
}


VX_PRIVATE_API vx_status _vxoWeightBias_AddToPool(
    vx_context                    context,
    vx_weights_biases_parameter   wb
    )
{
    vx_wb_list wbl = (vx_wb_list)vxAllocateAndZeroMemory(sizeof(vx_wb_list_s));
    if (wbl == VX_NULL) return VX_ERROR_NO_MEMORY;

    wbl->wb = wb;
    wbl->next = VX_NULL;

    if (context->wbList == VX_NULL)
    {
        context->wbList = wbl;
    }
    else
    {
        wbl->next = context->wbList;
        context->wbList = wbl;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API void _vxoWeightBias_RemoveFromPool(
    vx_context                    context,
    vx_weights_biases_parameter   wb
    )
{
    vx_wb_list prev = VX_NULL, current = context->wbList;

    while (current != VX_NULL)
    {
        if (current->wb == wb)
        {
            if (prev != VX_NULL)
            {
                prev->next = current->next;
            }
            else
            {
                context->wbList = current->next;
            }

            vxFree(current);
            break;
        }

        prev = current;
        current = current->next;
    }
}

VX_PRIVATE_API vx_weights_biases_parameter _vxoWeightBias_GetFromPool(
    vx_context                       context,
    vx_weight_bias_general_param     weight_param,
    vx_weight_bias_general_param     bias_param,
    vx_uint32                        orig_stride_x,
    vx_uint32                        orig_stride_y,
    vx_uint32                        stride_x,
    vx_uint32                        stride_y,
    vx_uint32                        skip_value,
    vx_bool                          is_depth_wise,
    vx_bool                          do_1xN_config,
    vx_enum                          layer_type,
    vx_tensor                        weight,
    vx_tensor                        bias,
    vx_reference                     alpha
    )
{
    vx_wb_list current = context->wbList;

    while (current != VX_NULL)
    {
        if (WB_WEIGHT_TENSOR(current->wb)  != weight        ||
            WB_BIAS_TENSOR(current->wb)    != bias          ||
            WB_ALPHA_REF(current->wb)      != alpha         ||
            WB_STRIDE_X(current->wb)       != stride_x      ||
            WB_STRIDE_Y(current->wb)       != stride_y      ||
            WB_ORG_STRIDE_X(current->wb)   != orig_stride_x ||
            WB_ORG_STRIDE_Y(current->wb)   != orig_stride_y ||
            WB_SKIP_VALUE(current->wb)     != skip_value    ||
            WB_IS_DEPTH_WISE(current->wb)  != is_depth_wise ||
            WB_DO_1XN_CONFIG(current->wb)  != do_1xN_config ||
            WB_ORG_LAYER_TYPE(current->wb) != layer_type)
        {
            current = current->next;
        }
        else if (memcmp((void*)&WB_WEIGHT_PARAM(current->wb), (void*)weight_param, sizeof(vx_weight_bias_general_param_s)) ||
                 memcmp((void*)&WB_BIAS_PARAM(current->wb), (void*)bias_param, sizeof(vx_weight_bias_general_param_s)))
        {
            current = current->next;
        }
        else
        {
            return current->wb;
        }
    }

    return VX_NULL;
}


VX_PRIVATE_API vx_status vxoWeightBias_ScaleAndZP(
    vx_weights_biases_parameter         wb,
    vx_weight_bias_general_param        weight_param,
    vx_weight_bias_general_param        bias_param
    )
{
    vx_context context = wb->base.context;

    if(gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PER_CHANNEL_QUANT) && WB_WEIGHT_QUANT_FORMAT(wb) == VX_QUANT_AFFINE_SCALE_PER_CHANNEL &&(WB_WEIGHT_DATA_FORMAT(wb) == VX_TYPE_UINT8 || WB_WEIGHT_DATA_FORMAT(wb) == VX_TYPE_INT8))
    {
         vx_uint32 outChannel = weight_param->scaleCount;

        WB_WEIGHT_SCALE_POINTER(wb) = (vx_float32 *)vxAllocateAndZeroMemory(outChannel * sizeof(vx_float32));
        gcoOS_MemCopy(WB_WEIGHT_SCALE_POINTER(wb), weight_param->quant_scale, outChannel* sizeof(vx_float32));
        WB_BIAS_SCALE_POINTER(wb) = (vx_float32 *)vxAllocateAndZeroMemory(outChannel * sizeof(vx_float32));
        gcoOS_MemCopy(WB_BIAS_SCALE_POINTER(wb), bias_param->quant_scale, outChannel * sizeof(vx_float32));

        WB_WEIGHT_ZP_POINTER(wb) = (vx_int32 *)vxAllocateAndZeroMemory(outChannel * sizeof(vx_int32));
        gcoOS_MemCopy(WB_WEIGHT_ZP_POINTER(wb), weight_param->quant_zero_point, outChannel * sizeof(vx_int32));
        WB_BIAS_ZP_POINTER(wb) = (vx_int32 *)vxAllocateAndZeroMemory(outChannel * sizeof(vx_int32));
        gcoOS_MemCopy(WB_BIAS_ZP_POINTER(wb), bias_param->quant_zero_point, outChannel * sizeof(vx_int32));
    }
    else if(vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT) && (WB_WEIGHT_QUANT_FORMAT(wb)  == VX_QUANT_AFFINE_SCALE) && WB_WEIGHT_DATA_FORMAT(wb) == VX_TYPE_UINT8 )
    {
        vx_uint32 i= 0;
        vx_uint32 outChannel = weight_param->dims_sizes[3];

        WB_WEIGHT_SCALE_POINTER(wb) = (vx_float32 *)vxAllocateAndZeroMemory(outChannel * sizeof(vx_float32));
        WB_BIAS_SCALE_POINTER(wb) = (vx_float32 *)vxAllocateAndZeroMemory(outChannel * sizeof(vx_float32));
        WB_WEIGHT_ZP_POINTER(wb) = (vx_int32 *)vxAllocateAndZeroMemory(outChannel * sizeof(vx_int32));
        WB_BIAS_ZP_POINTER(wb) = (vx_int32 *)vxAllocateAndZeroMemory(outChannel * sizeof(vx_int32));

        for(i = 0; i < outChannel; ++i)
        {
            gcoOS_MemCopy(WB_WEIGHT_SCALE_POINTER(wb) + i, weight_param->quant_scale, sizeof(vx_float32));
            gcoOS_MemCopy(WB_BIAS_SCALE_POINTER(wb) + i, bias_param->quant_scale, sizeof(vx_float32));
            gcoOS_MemCopy(WB_WEIGHT_ZP_POINTER(wb) + i, weight_param->quant_zero_point, sizeof(vx_int32));
            gcoOS_MemCopy(WB_BIAS_ZP_POINTER(wb) + i, bias_param->quant_zero_point, sizeof(vx_int32));
        }

    }
    else
    {
        vx_uint32 outChannel = 1;
        WB_WEIGHT_FPP(wb)= weight_param->fixed_point_pos;
        WB_BIAS_FPP(wb) = bias_param->fixed_point_pos;

        WB_WEIGHT_SCALE_POINTER(wb) = (vx_float32 *)vxAllocateAndZeroMemory(outChannel * sizeof(vx_float32));
        gcoOS_MemCopy(WB_WEIGHT_SCALE_POINTER(wb), weight_param->quant_scale, outChannel* sizeof(vx_float32));
        WB_BIAS_SCALE_POINTER(wb) = (vx_float32 *)vxAllocateAndZeroMemory(outChannel * sizeof(vx_float32));
        gcoOS_MemCopy(WB_BIAS_SCALE_POINTER(wb), bias_param->quant_scale, outChannel * sizeof(vx_float32));

        WB_WEIGHT_ZP_POINTER(wb) = (vx_int32 *)vxAllocateAndZeroMemory(outChannel * sizeof(vx_int32));
        WB_WEIGHT_ZP(wb, 0) = 0;
        WB_BIAS_ZP_POINTER(wb) = (vx_int32 *)vxAllocateAndZeroMemory(outChannel * sizeof(vx_int32));
        WB_BIAS_ZP(wb, 0) = 0;

    }
    return vx_true_e;
}

/******************************************************************/

VX_PRIVATE_API vx_status vxoWeightBias_Initializer(
    vx_weights_biases_parameter                 wb,
    vx_weight_bias_general_param                weight_param,
    vx_weight_bias_general_param                bias_param,
    vx_uint32                                   orig_stride_x,
    vx_uint32                                   orig_stride_y,
    vx_uint32                                   stride_x,
    vx_uint32                                   stride_y,
    vx_int8                                     zero_len,
    vx_uint32                                   *skip_value,
    vx_bool                                     is_depth_wise,
    vx_bool                                     do_1xN_config,
    vx_enum                                     layer_type
    )
{
    vx_uint32 strideX, strideY;
    if (wb == VX_NULL) return VX_ERROR_INVALID_PARAMETERS;
    if (weight_param == VX_NULL || bias_param == VX_NULL) return VX_ERROR_INVALID_PARAMETERS;
    if (!vxoReference_IsValidAndSpecific(&wb->base, VX_TYPE_WEIGHTS_BIASES_PARAMETER)) return VX_ERROR_INVALID_TYPE;

    vxMemCopy(&WB_WEIGHT_PARAM(wb), weight_param, sizeof(vx_weight_bias_general_param_s));

    strideX = stride_x;
    strideY = stride_y;

    vxMemCopy(&WB_BIAS_PARAM(wb), bias_param, sizeof(vx_weight_bias_general_param_s));

    WB_ORG_STRIDE_X(wb)     = orig_stride_x != 0 ? orig_stride_x : strideX;
    WB_ORG_STRIDE_Y(wb)     = orig_stride_y != 0 ? orig_stride_y : strideY;
    WB_STRIDE_X(wb)         = strideX;
    WB_STRIDE_Y(wb)         = strideY;
    WB_SET_ZERO_LENGTH(wb)  = zero_len;
    WB_SKIP_VALUE(wb)       = skip_value[0];
    WB_IS_DEPTH_WISE(wb)    = is_depth_wise;
    WB_DO_1XN_CONFIG(wb)    = do_1xN_config;
    WB_ORG_LAYER_TYPE(wb)   = layer_type;

    if(WB_INPUT_FORMAT(wb) == VX_TYPE_INVALID)
        WB_INPUT_FORMAT(wb) = WB_WEIGHT_DATA_FORMAT(wb);

    vxoWeightBias_ScaleAndZP(wb, weight_param, bias_param);


    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoWeightBias_Deinitializer(
    vx_weights_biases_parameter wb
    )
{
    vx_uint32 i;

    if (wb == VX_NULL) return VX_ERROR_INVALID_PARAMETERS;
    if (!vxoReference_IsValidAndSpecific(&wb->base, VX_TYPE_WEIGHTS_BIASES_PARAMETER)) return VX_ERROR_INVALID_TYPE;

    RESET_WB_COMPRESS_FLAG(wb);

    if (WB_MEMORY_NODE(wb) != VX_NULL)
    {
        vxoMemory_Free(wb->base.context, &WB_MEMORY(wb));
        WB_MEMORY_NODE(wb) = VX_NULL;
    }

    if (WB_SLICE_ARRAY(wb) != VX_NULL)
    {
        for (i = 0; i < WB_TOTAL_SLICE_NUM(wb); i++)
        {
            if (WB_ZOFFSET_HANDLE_INDEX(wb, i) != VX_NULL)
            {
                vxFree(WB_ZOFFSET_HANDLE_INDEX(wb, i));
                WB_ZOFFSET_HANDLE_INDEX(wb, i) = VX_NULL;
            }

            if (WB_HUFFMAN_CONFIG_INDEX(wb, i) != VX_NULL)
            {
                vxFree(WB_HUFFMAN_CONFIG_INDEX(wb, i));
                WB_HUFFMAN_CONFIG_INDEX(wb, i) = VX_NULL;
            }
        }

        vxFree(WB_SLICE_ARRAY(wb));
        WB_SLICE_ARRAY(wb) = VX_NULL;
    }

    if(WB_WEIGHT_SCALE_POINTER(wb) != VX_NULL)
    {
        vxFree(WB_WEIGHT_SCALE_POINTER(wb));
        WB_WEIGHT_SCALE_POINTER(wb) = VX_NULL;
    }

    if(WB_BIAS_SCALE_POINTER(wb) != VX_NULL)
    {
        vxFree(WB_BIAS_SCALE_POINTER(wb));
        WB_BIAS_SCALE_POINTER(wb) = VX_NULL;
    }

    if(WB_WEIGHT_ZP_POINTER(wb) != VX_NULL)
    {
        vxFree(WB_WEIGHT_ZP_POINTER(wb));
        WB_WEIGHT_ZP_POINTER(wb) = VX_NULL;
    }

    if(WB_BIAS_ZP_POINTER(wb) != VX_NULL)
    {
        vxFree(WB_BIAS_ZP_POINTER(wb));
        WB_BIAS_ZP_POINTER(wb) = VX_NULL;
    }

    if (WB_WEIGHT_TENSOR(wb) != VX_NULL)
    {
        vxoReference_Release((vx_reference_ptr)&WB_WEIGHT_TENSOR(wb), VX_TYPE_TENSOR, VX_REF_INTERNAL);
    }

    if (WB_BIAS_TENSOR(wb) != VX_NULL)
    {
        vxoReference_Release((vx_reference_ptr)&WB_BIAS_TENSOR(wb), VX_TYPE_TENSOR, VX_REF_INTERNAL);
    }

    if (WB_ALPHA_REF(wb) != VX_NULL)
    {
        vxoReference_Release((vx_reference_ptr)&WB_ALPHA_REF(wb), VX_TYPE_TENSOR, VX_REF_INTERNAL);
    }

    return VX_SUCCESS;
}


VX_PRIVATE_API vx_status vxoWeightBias_CalculateCompressRatio(
    vx_weights_biases_parameter    wb,
    vx_enum                        target,
    vx_uint32                      kernel_per_core,
    vx_uint32                      z_offset
    )
{
    vx_status status;
    vx_context context;
    vx_uint32 kernelPerCore = kernel_per_core;
    vx_size minTotalKernelBufferSize = 0;

    if (wb == VX_NULL || z_offset == 0) return VX_ERROR_INVALID_PARAMETERS;
    if (kernel_per_core == 0 && target != VXNNE_OPERATION_TARGET_TP) return VX_ERROR_INVALID_PARAMETERS;
    if (!vxoReference_IsValidAndSpecific(&wb->base, VX_TYPE_WEIGHTS_BIASES_PARAMETER)) return VX_ERROR_INVALID_TYPE;
    if (WB_WEIGHT_TENSOR(wb) == VX_NULL) return VX_ERROR_INVALID_VALUE;

    if (WB_IS_CALCULATED(wb) &&
        WB_COMPRESS_TARGET(wb) == target &&
        WB_KERNEL_PER_CORE(wb) == kernel_per_core &&
        WB_Z_OFFSET(wb) == z_offset)
    {
        return VX_SUCCESS;
    }

    context = wb->base.context;

    /* copy original weight data from non-cache video memory to CPU memory to speed up */
    {
        vx_uint8_ptr weightOrigPtr = VX_NULL;
        vx_uint32_ptr biasOrigPtr = VX_NULL;
        vx_uint32 size = 0;

        vxoTensor_GetTensorViewMemory(WB_WEIGHT_TENSOR(wb), (gctPOINTER*)&weightOrigPtr, VX_NULL);
        vxoTensor_GetTensorSize(WB_WEIGHT_TENSOR(wb), &size);
        if (weightOrigPtr != VX_NULL && size > 0)
        {
            WB_WEIGHT_DATA(wb) = (vx_uint8_ptr)vxAllocate(size);
            if (WB_WEIGHT_DATA(wb) == VX_NULL)
            {
                status = VX_ERROR_NO_MEMORY;
                goto exit;
            }
            vxMemCopy(WB_WEIGHT_DATA(wb), weightOrigPtr, size);
        }
        else
        {
            status = VX_ERROR_NO_RESOURCES;
            goto exit;
        }

        if (WB_BIAS_TENSOR(wb) != VX_NULL)
        {
            vxoTensor_GetTensorViewMemory(WB_BIAS_TENSOR(wb), (gctPOINTER*)&biasOrigPtr, VX_NULL);
            if (biasOrigPtr != VX_NULL)
            {
                WB_BIAS_DATA(wb) = biasOrigPtr;
            }
            else
            {
                status = VX_ERROR_NO_RESOURCES;
                goto exit;
            }
        }
    }

    status = _vxoWeightBias_CalculateSize(context, wb, target, &kernelPerCore, z_offset, vx_false_e, &minTotalKernelBufferSize, VX_NULL, VX_NULL, VX_NULL);
    if (status != VX_SUCCESS)
    {
        goto exit;
    }

    WB_MEMORY_SIZE(wb) = minTotalKernelBufferSize;

    WB_KERNEL_PER_CORE(wb) = kernel_per_core;
    WB_Z_OFFSET(wb) = z_offset;
    SET_WB_CALCULATED_FLAG(wb);

exit:
    if (WB_WEIGHT_DATA(wb) != VX_NULL)
    {
        vxFree(WB_WEIGHT_DATA(wb));
        WB_WEIGHT_DATA(wb) = VX_NULL;
    }
    if (WB_BIAS_DATA(wb) != VX_NULL)
    {
        WB_BIAS_DATA(wb) = VX_NULL;
    }

    if (status != VX_SUCCESS)
    {
        RESET_WB_COMPRESS_FLAG(wb);
    }

    return status;
}


VX_PRIVATE_API vx_status vxoWeightBias_Compress(
    vx_weights_biases_parameter    wb,
    vx_enum                        target,
    vx_uint32                      kernel_per_core,
    vx_uint32                      z_offset
    )
{
    vx_status status = VX_SUCCESS;
    vx_context context;
    vx_size minTotalKernelBufferSize = 0;
    vx_size minKernelBufferSizes[MAX_ZGROUP_COUNT*MAX_KZGROUP_COUNT];
    vx_uint8* minZeroRunLens = VX_NULL;
    vx_uint32 maxZeroRunLens[MAX_ZGROUP_COUNT] = {0}, kernelPerCore;
    vx_uint32 nnCoreCount = 0, tpCoreCount = 0;

    if (wb == VX_NULL || z_offset == 0) return VX_ERROR_INVALID_PARAMETERS;
    if (kernel_per_core == 0 && target != VXNNE_OPERATION_TARGET_TP) return VX_ERROR_INVALID_PARAMETERS;
    if (!vxoReference_IsValidAndSpecific(&wb->base, VX_TYPE_WEIGHTS_BIASES_PARAMETER)) return VX_ERROR_INVALID_TYPE;
    if (WB_WEIGHT_TENSOR(wb) == VX_NULL) return VX_ERROR_INVALID_VALUE;

    if (WB_IS_COMPRESSED(wb) &&
        WB_COMPRESS_TARGET(wb) == target &&
        WB_KERNEL_PER_CORE(wb) == kernel_per_core &&
        WB_Z_OFFSET(wb) == z_offset)
    {
        return VX_SUCCESS;
    }

    context = wb->base.context;

    switch (target)
    {
    case VXNNE_OPERATION_TARGET_NN:
        nnCoreCount = (WB_WEIGHT_DATA_FORMAT(wb) == VX_TYPE_INT16) ?
                      context->nnConfig.fixedFeature.nnCoreCountInt16 : (WB_WEIGHT_DATA_FORMAT(wb) == VX_TYPE_FLOAT16) ?
                      context->nnConfig.fixedFeature.nnCoreCountFloat16 : (WB_WEIGHT_DATA_FORMAT(wb) == VX_TYPE_BFLOAT16) ?
                      context->nnConfig.fixedFeature.nnCoreCountBFloat16 :context->nnConfig.fixedFeature.nnCoreCount;

        if (!nnCoreCount)
        {
            vxError("\n  No NN Core Count \n");
            goto exit;
        }

        break;

    case VXNNE_OPERATION_TARGET_TP:
        tpCoreCount = context->nnConfig.fixedFeature.tpCoreCount +
                      context->nnConfig.fixedFeature.tpliteCoreCount;

        if (!tpCoreCount)
        {
            vxError("\n  No TP Core Count \n");
            goto exit;
        }

        break;

    default:
        vxError("\n  Unsupported Target \n");
        goto exit;

        break;
    }

    /* copy original weight and bias data from non-cache video memory to CPU memory to speed up */
    {
        vx_uint8_ptr weightOrigPtr = VX_NULL/*, alphaOrigPtr = VX_NULL*/;
        vx_uint32_ptr biasOrigPtr = VX_NULL;
        vx_uint32 size = 0;

        vxoTensor_GetTensorViewMemory(WB_WEIGHT_TENSOR(wb), (gctPOINTER*)&weightOrigPtr, VX_NULL);
        vxoTensor_GetTensorSize(WB_WEIGHT_TENSOR(wb), &size);
        if (weightOrigPtr != VX_NULL && size > 0)
        {
            WB_WEIGHT_DATA(wb) = (vx_uint8_ptr)vxAllocate(size);
            if (WB_WEIGHT_DATA(wb) == VX_NULL)
            {
                status = VX_ERROR_NO_MEMORY;
                goto exit;
            }
            vxMemCopy(WB_WEIGHT_DATA(wb), weightOrigPtr, size);
        }
        else
        {
            status = VX_ERROR_NO_RESOURCES;
            goto exit;
        }

        if (WB_BIAS_TENSOR(wb) != VX_NULL)
        {
            vxoTensor_GetTensorViewMemory(WB_BIAS_TENSOR(wb), (gctPOINTER*)&biasOrigPtr, VX_NULL);
            vxoTensor_GetTensorSize(WB_BIAS_TENSOR(wb), &size);
            if (biasOrigPtr != VX_NULL && size > 0)
            {
                WB_BIAS_DATA(wb) = (vx_uint32_ptr)vxAllocate(size);
                if (WB_BIAS_DATA(wb) == VX_NULL)
                {
                    status = VX_ERROR_NO_MEMORY;
                    goto exit;
                }
                vxMemCopy(WB_BIAS_DATA(wb), biasOrigPtr, size);
            }
            else
            {
                status = VX_ERROR_NO_RESOURCES;
                goto exit;
            }
        }

        /*if (WB_ALPHA_REF(wb) != VX_NULL)
        {
            vxoTensor_GetTensorViewMemory(WB_ALPHA_TENSOR(wb), (gctPOINTER*)&alphaOrigPtr, VX_NULL);
            if (alphaOrigPtr != VX_NULL)
            {
                WB_ALPHA_DATA(wb) = alphaOrigPtr;
            }
            else
            {
                status = VX_ERROR_NO_RESOURCES;
                goto exit;
            }
        }*/
    }

    kernelPerCore = kernel_per_core;

    status = _vxoWeightBias_CalculateSize(context, wb, target, &kernelPerCore, z_offset, vx_true_e, &minTotalKernelBufferSize, minKernelBufferSizes, &minZeroRunLens, maxZeroRunLens);
    if (status != VX_SUCCESS)
    {
        goto exit;
    }

    if (WB_MEMORY_NODE(wb) != VX_NULL)
    {
        vxoMemory_Free(context, &WB_MEMORY(wb));
        WB_MEMORY_NODE(wb) = VX_NULL;
    }

    status = _vxoWeightBias_AllocateMemory(context, &WB_MEMORY(wb), &minTotalKernelBufferSize);
    if (status != VX_SUCCESS)
    {
        goto exit;
    }
    else
    {
        WB_MEMORY_SIZE(wb) = minTotalKernelBufferSize;
    }

    status = _vxoWeightBias_Compress(context, wb, kernelPerCore, z_offset, minKernelBufferSizes, minZeroRunLens, maxZeroRunLens);
    if (status != VX_SUCCESS)
    {
        goto exit;
    }

    WB_KERNEL_PER_CORE(wb) = kernel_per_core;
    WB_Z_OFFSET(wb) = z_offset;
    SET_WB_COMPRESS_FLAG(wb);

#if gcdDUMP
    gcmDUMP(gcvNULL, "#[weights and biases]\n");
    gcmDUMP_BUFFER(gcvNULL,
                   gcvDUMP_BUFFER_MEMORY,
                   WB_MEM_PHYSICAL_BASE_ADDR(wb),
                   (gctPOINTER)WB_MEM_LOGICAL_BASE_ADDR(wb),
                   0,
                   WB_MEMORY_SIZE(wb));
#endif

exit:
    if (WB_WEIGHT_DATA(wb) != VX_NULL)
    {
        vxFree(WB_WEIGHT_DATA(wb));
        WB_WEIGHT_DATA(wb) = VX_NULL;
    }
    if (WB_BIAS_DATA(wb) != VX_NULL)
    {
        vxFree(WB_BIAS_DATA(wb));
        WB_BIAS_DATA(wb) = VX_NULL;
    }
    if (WB_ALPHA_DATA(wb) != VX_NULL)
    {
        WB_ALPHA_DATA(wb) = VX_NULL;
    }

    if (minZeroRunLens != VX_NULL)
    {
        vxFree(minZeroRunLens);
        minZeroRunLens = VX_NULL;
    }

    if (status != VX_SUCCESS)
    {
        if (WB_MEMORY_NODE(wb) != VX_NULL)
        {
            vxoMemory_Free(context, &WB_MEMORY(wb));
            WB_MEMORY_NODE(wb) = VX_NULL;
        }

        RESET_WB_COMPRESS_FLAG(wb);
    }

    return status;
}

VX_PRIVATE_API vx_status vxoWeightBias_Set_Weight_Bias_Tensor(
    vx_weights_biases_parameter   wb,
    vx_tensor                     weight,
    vx_tensor                     bias
    )
{
    if (wb == VX_NULL) return VX_ERROR_INVALID_PARAMETERS;
    if (!vxoReference_IsValidAndSpecific(&wb->base, VX_TYPE_WEIGHTS_BIASES_PARAMETER)) return VX_ERROR_INVALID_TYPE;

    if (weight == VX_NULL && bias == VX_NULL) return VX_ERROR_INVALID_PARAMETERS;

    if (weight != VX_NULL)
    {
        if (!vxoReference_IsValidAndSpecific(&weight->base, VX_TYPE_TENSOR)) return VX_ERROR_INVALID_TYPE;

        if (WB_WEIGHT_TENSOR(wb) != VX_NULL)
        {
            vxoReference_Release((vx_reference_ptr)&WB_WEIGHT_TENSOR(wb), VX_TYPE_TENSOR, VX_REF_INTERNAL);
        }
        else
        {
            WB_WEIGHT_DIMS_NUM(wb)      = TENSOR_DIM_NUM(weight);
            WB_WEIGHT_DATA_FORMAT(wb)   = TENSOR_DATA_TYPE(weight);
            WB_WEIGHT_FPP(wb)           = TENSOR_POS(weight);
            WB_WEIGHT_QUANT_FORMAT(wb)  = TENSOR_QUANT_TYPE(weight);
            WB_WEIGHT_SCALE(wb, 0)         = TENSOR_TF_SCALE(weight);
            WB_WEIGHT_ZP(wb, 0)            = TENSOR_TF_ZEROPOINT(weight);

            vxMemCopy(&WB_WEIGHT_ORG_DIMS_SIZES(wb), TENSOR_SIZES(weight), sizeof(WB_WEIGHT_DIMS_SIZES(wb)));
            vxMemCopy(&WB_WEIGHT_DIMS_SIZES(wb), TENSOR_SIZES(weight), sizeof(WB_WEIGHT_DIMS_SIZES(wb)));
        }

        WB_WEIGHT_TENSOR(wb) = weight;
        vxoReference_Increment((vx_reference)WB_WEIGHT_TENSOR(wb), VX_REF_INTERNAL);
    }

    if (bias != VX_NULL)
    {
        if (!vxoReference_IsValidAndSpecific(&bias->base, VX_TYPE_TENSOR)) return VX_ERROR_INVALID_TYPE;

        if (WB_BIAS_TENSOR(wb) != VX_NULL)
        {
            vxoReference_Release((vx_reference_ptr)&WB_BIAS_TENSOR(wb), VX_TYPE_TENSOR, VX_REF_INTERNAL);
        }
        else
        {
            WB_BIAS_DIMS_NUM(wb)      = TENSOR_DIM_NUM(bias);
            WB_BIAS_DATA_FORMAT(wb)   = TENSOR_DATA_TYPE(bias);
            WB_BIAS_FPP(wb)           = TENSOR_POS(bias);
            WB_BIAS_QUANT_FORMAT(wb)  = TENSOR_QUANT_TYPE(bias);
            WB_BIAS_SCALE(wb, 0)         = TENSOR_TF_SCALE(bias);
            WB_BIAS_ZP(wb, 0)            = TENSOR_TF_ZEROPOINT(bias);

            vxMemCopy(&WB_BIAS_DIMS_SIZES(wb), TENSOR_SIZES(bias), sizeof(WB_BIAS_DIMS_SIZES(wb)));
        }

        WB_BIAS_TENSOR(wb) = bias;
        vxoReference_Increment((vx_reference)WB_BIAS_TENSOR(wb), VX_REF_INTERNAL);
    }


    RESET_WB_COMPRESS_FLAG(wb);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoWeightBias_Set_Alpha_Tensor(
    vx_weights_biases_parameter   wb,
    vx_reference                 alpha_ref
    )
{
    if (wb == VX_NULL) return VX_ERROR_INVALID_PARAMETERS;
    if (!vxoReference_IsValidAndSpecific(&wb->base, VX_TYPE_WEIGHTS_BIASES_PARAMETER)) return VX_ERROR_INVALID_TYPE;

    if (alpha_ref == VX_NULL) return VX_ERROR_INVALID_PARAMETERS;
    /*if (!vxoReference_IsValidAndSpecific(&alpha, VX_TYPE_TENSOR)) return VX_ERROR_INVALID_TYPE;*/

    if (WB_ALPHA_REF(wb) != VX_NULL)
    {
        vxoReference_Release((vx_reference_ptr)&WB_ALPHA_REF(wb), VX_TYPE_TENSOR, VX_REF_INTERNAL);
    }
    else if(WB_OPERATION_TYPE(wb) == VX_NN_CONV_PRELU)
    {
        vx_tensor alpha;
        vxmASSERT(vxoReference_GetType(alpha_ref) == VX_TYPE_TENSOR);

        alpha = (vx_tensor)alpha_ref;

        WB_ALPHA_DIMS_NUM(wb)      = TENSOR_DIM_NUM(alpha);
        WB_ALPHA_DATA_FORMAT(wb)   = TENSOR_DATA_TYPE(alpha);
        WB_ALPHA_FPP(wb)           = TENSOR_POS(alpha);
        WB_ALPHA_QUANT_FORMAT(wb)  = TENSOR_QUANT_TYPE(alpha);
        WB_ALPHA_SCALE(wb, 0)      = TENSOR_TF_SCALE(alpha);
        WB_ALPHA_ZP(wb, 0)         = TENSOR_TF_ZEROPOINT(alpha);

        vxMemCopy(&WB_ALPHA_DIMS_SIZES(wb), TENSOR_SIZES(alpha), sizeof(WB_ALPHA_DIMS_SIZES(wb)));
    }

    WB_ALPHA_REF(wb) = alpha_ref;
    vxoReference_Increment((vx_reference)WB_ALPHA_REF(wb), VX_REF_INTERNAL);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoWeightBias_Set_Optimization(
    vx_weights_biases_parameter   wb,
    vx_ptr                        opt_ptr,
    vx_uint32                     opt_size
    )
{
    vx_weights_biases_parameter_optimizations_ext2_t* opt;

    if (wb == VX_NULL || opt_ptr == VX_NULL) return VX_ERROR_INVALID_PARAMETERS;
    if (!vxoReference_IsValidAndSpecific(&wb->base, VX_TYPE_WEIGHTS_BIASES_PARAMETER)) return VX_ERROR_INVALID_TYPE;
    //if (opt_size != sizeof(vx_weights_biases_parameter_optimizations_t) &&
    //    opt_size != sizeof(vx_weights_biases_parameter_optimizations_ext_t) &&
    //    opt_size != sizeof(vx_weights_biases_parameter_optimizations_ext2_t))
    //    return VX_ERROR_INVALID_PARAMETERS;

    opt = (vx_weights_biases_parameter_optimizations_ext2_t*)opt_ptr;

    if(opt_size == sizeof(vx_weights_biases_parameter_optimizations_ext2_t))
    {
        WB_INPUT_ZP(wb) = opt->ext.inputZeroPoint;
        WB_INPUT_SCALE(wb) = opt->inputScale;
        WB_INPUT_FORMAT(wb) = opt->inputFormat;
        WB_OUTPUT_SCALE(wb) = opt->outputScale;
    }
    else
    {
        WB_INPUT_ZP(wb) = opt->ext.inputZeroPoint;
        WB_INPUT_SCALE(wb) = 0;
        WB_INPUT_FORMAT(wb) = WB_WEIGHT_DATA_FORMAT(wb);/*VX_TYPE_INVALID :some older case never sent inputFormat*/
        WB_OUTPUT_SCALE(wb) = 0;
    }

    WB_SET_ZERO_LENGTH(wb) = opt->ext.zrl;

    return VX_SUCCESS;
}

/******************************************************************/

VX_PRIVATE_API vx_weights_biases_parameter vxoWeightBias_Create(
    vx_context       context
    )
{
    vx_weights_biases_parameter wb;

    wb = (vx_weights_biases_parameter)vxoReference_Create(context, VX_TYPE_WEIGHTS_BIASES_PARAMETER, VX_REF_INTERNAL, &context->base);
    if (vxoReference_GetStatus((vx_reference)wb) != VX_SUCCESS) return VX_NULL;

    wb->initialize             = vxoWeightBias_Initializer;
    wb->deinitialize           = vxoWeightBias_Deinitializer;
    wb->calc_compress_ratio    = vxoWeightBias_CalculateCompressRatio;
    wb->compress               = vxoWeightBias_Compress;
    wb->set_weight_bias_tensor = vxoWeightBias_Set_Weight_Bias_Tensor;
    wb->set_alpha_tensor       = vxoWeightBias_Set_Alpha_Tensor;
    wb->set_optimization       = vxoWeightBias_Set_Optimization;

    return wb;
}

VX_PRIVATE_API vx_status vxoWeightBias_Release(
    vx_weights_biases_parameter*  wb_ptr
    )
{
    if (wb_ptr == VX_NULL || *wb_ptr == VX_NULL) return VX_ERROR_INVALID_PARAMETERS;

    if (!vxoReference_IsValidAndSpecific(&((*wb_ptr)->base), VX_TYPE_WEIGHTS_BIASES_PARAMETER)) return VX_ERROR_INVALID_TYPE;

    vxoReference_Release((vx_reference_ptr)wb_ptr, VX_TYPE_WEIGHTS_BIASES_PARAMETER, VX_REF_INTERNAL);

    return VX_SUCCESS;
}

/******************************************************************/
VX_INTERNAL_CALLBACK_API void vxoWeightBias_Destructor(
    vx_reference ref
    )
{
    vx_weights_biases_parameter wb = (vx_weights_biases_parameter)ref;
    wb->deinitialize(wb);
}

VX_INTERNAL_API vx_weights_biases_parameter vxoCreateWeightsBiasesParameterFromTensors(
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
    vx_uint32 opt_size,
    vx_enum     output_format,
    vx_enum     convert_format,
    vx_enum     rank_mode,
    vx_tensor   weights,
    vx_tensor   biases,
    vx_reference   alpha,
    vx_enum     conv_operation_type,
    vx_bool     do1xN
    )
{
    vx_weights_biases_parameter weight_bias = VX_NULL;

    vx_uint32 weightDims[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32 weightDimsChanged[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32 weightViewStarts[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32 weightViewEnds[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32 biasDims[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32 biasViewStarts[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32 biasViewEnds[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32 inputDims[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32 convOutputDims[VX_CONTEXT_TENSOR_MAX_DIMENSION];

    vx_uint32 weightDimCount    = TENSOR_DIM_NUM(weights);
    vx_uint32 orgWeightDimCount = TENSOR_DIM_NUM(weights);
    vx_enum weightDataType      = TENSOR_DATA_TYPE(weights);
    vx_enum weightQuantType     = TENSOR_QUANT_TYPE(weights);
    vx_uint32 weightSize        = (vx_uint32)vxDataType_GetSize((vx_type_e)weightDataType);
    vx_uint32 orgWeightSize     = weightSize;

    vx_uint32 biasDimCount      = 0;
    vx_int8 biasFp              = 0;
    vx_float32 biasScale        = 0;
    vx_enum biasDataType        = VX_TYPE_FLOAT32;
    vx_enum biasQuantType       = biases ? TENSOR_QUANT_TYPE(biases) : TENSOR_QUANT_TYPE(weights);
    vx_uint32 i, strideX, strideY, strideXChanged, strideYChanged;
    vx_uint32 *skipValue = VX_NULL;

    vx_weight_bias_general_param_s weight_param, bias_param;

    vx_bool doDepthWise         = vx_false_e;
    vx_bool reallyDo1xN         = vx_false_e;
    vx_bool doZdpOpt            = vx_false_e;

    vx_bool nnSupportFormat = vxnneIsNNSupportFormat(context, weights, VX_NULL, VX_NULL);
    vx_bool hasHwDepthWise = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_NN_DEPTHWISE_SUPPORT);
    vx_bool isV8 = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP0);

    vx_status status = VX_SUCCESS;

    weight_param.quant_scale = VX_NULL;
    weight_param.quant_zero_point = VX_NULL;

    bias_param.quant_scale = VX_NULL;
    bias_param.quant_zero_point = VX_NULL;

    if (!nnSupportFormat && layer_type == VX_NN_CONVOLUTION_LAYER)
    {
        status = VX_ERROR_INVALID_TYPE;
        vxError("vxoCreateWeightsBiasesParameterFromTensors: current NN didn't support this format 0x%x", weightDataType);
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
        }
        else if (num_of_output_dims == 2)
        {
            convOutputDims[0] = 1;
            convOutputDims[1] = 1;
            convOutputDims[2] = convolution_outputs_dims[0];
            convOutputDims[3] = convolution_outputs_dims[1];
        }
    }

    vxMemCopy(weightDimsChanged, weightDims, sizeof(weightDimsChanged));

    if ((vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3) ||
         vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6)) &&
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
        }
    }
    else if (!(vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3) ||
             vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6)) &&
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
            weightDimsChanged[1] = fitN;
            weightDimsChanged[2] /= fitN;

            inputDims[0] *= inputDims[1];
            inputDims[1] = fitN;
            inputDims[2] /= fitN;

            convOutputDims[0] *= convOutputDims[1];
            convOutputDims[1] = 1;
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
            vxError("vxoCreateWeightsBiasesParameterFromTensors: current NN didn't support this bias format 0x%x", biasDataType);
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

    if(gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PER_CHANNEL_QUANT) && (weightQuantType == VX_QUANT_AFFINE_SCALE_PER_CHANNEL) && (weightDataType == VX_TYPE_UINT8 || weightDataType == VX_TYPE_INT8))
    {
        skipValue = (vx_uint32 *)vxAllocateAndZeroMemory(weightDims[3] * sizeof(vx_int32));
        gcoOS_MemCopy(skipValue, TENSOR_TF_ZEROPOINT_POINTER(weights), weightDims[3] * sizeof(vx_int32));
    }
    else if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT) && (weightQuantType == VX_QUANT_AFFINE_SCALE) && weightDataType == VX_TYPE_UINT8)
    {
        skipValue = (vx_uint32 *)vxAllocateAndZeroMemory(sizeof(vx_int32));
        skipValue[0] = TENSOR_TF_ZEROPOINT(weights);
    }
    else
    {
        skipValue = (vx_uint32 *)vxAllocateAndZeroMemory(sizeof(vx_int32));
        skipValue[0] = 0;
    }

    strideX = stride_x;
    strideY = stride_y;
    _getNNStride(layer_type,
                 inputDims,
                 convOutputDims,
                 weightDims,
                 pad_x_left,
                 pad_x_right,
                 pad_y_top,
                 pad_y_bottom,
                 down_scale_size_rounding,
                 &strideX, &strideY);

    strideXChanged = strideX;
    strideYChanged = strideY;

    if ((strideX == 1 && strideY == 1) || weightDimsChanged[0] != 1 || weightDimsChanged[1] != 1)
    {
        vx_uint32 alignedWidth = ((weightDimsChanged[0] % strideX == 0) ? weightDimsChanged[0] : (weightDimsChanged[0] + (strideX - weightDimsChanged[0] % strideX)));
        vx_uint32 alignedHeight = ((weightDimsChanged[1] % strideY == 0) ? weightDimsChanged[1] : (weightDimsChanged[1] + (strideY - weightDimsChanged[1] % strideY)));
        weightDimsChanged[0] = alignedWidth / strideX;
        weightDimsChanged[1] = alignedHeight / strideY;
        weightDimsChanged[2] = weightDimsChanged[2] * strideX * strideY;
    }

    if (!isV8 && layer_type == VX_NN_CONVOLUTION_LAYER &&
        weightDimsChanged[0] != weightDimsChanged[1] && weightDimsChanged[0] != 1)
    {
        /*V7 didn't support MxN, only supoort 1xN*/
        status = VX_ERROR_INVALID_VALUE;
        vxError("vxoCreateWeightsBiasesParameterFromTensors: current NN didn't support this kernel size %d x %d", weightDimsChanged[0], weightDimsChanged[1]);
        goto exit;
    }

    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_FAST_FIRST_PIXEL_POOLING) &&
        !doZdpOpt &&
        weights->dims[0] < 16 &&
        weights->dims[1] < 16 &&
        ((strideX == 2 && strideY == 2) ||
        (strideX == 4 && strideY == 4 && pooling_size_x == 0)) &&
        (layer_type == VX_NN_CONVOLUTION_LAYER ||
         (layer_type == VX_NN_DEPTH_WISE_CONVOLUTION_LAYER && hasHwDepthWise)))
    {
        strideXChanged = 1;
        strideYChanged = 1;
        weightDimsChanged[0] = weights->dims[0];
        weightDimsChanged[1] = weights->dims[1];
        weightDimsChanged[2] = weights->dims[2];
    }
    else if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_FIRST_PIXEL_POOLING) &&
        strideX == 2 && strideY == 2 &&
        !doZdpOpt &&
        pooling_size_x == 0 &&
        ((inputDims[0] % 2 == 0 && layer_type == VX_NN_CONVOLUTION_LAYER) ||
         (layer_type == VX_NN_DEPTH_WISE_CONVOLUTION_LAYER && hasHwDepthWise)))
    {
        /* Per Arch, only support INT8 3x3 conv right now*/
        /* First pixel pooling is 2x2 poooling stride is 2, so convolution output should be even*/
        vx_float32 nonZeroRatio = calculateWeightNonZeroRatio(context, skipValue[0], weights);

        /*V8 has limitation for 1x2 & 2x1, those shape with 2x2 stride can't do FFP*/
        if ((nonZeroRatio * weights->dims[0] * weights->dims[1] < 6.3 ||
            (layer_type == VX_NN_DEPTH_WISE_CONVOLUTION_LAYER && hasHwDepthWise)) &&
            !(isV8 && ((weights->dims[0] == 1 && weights->dims[1] == 2) ||
                       (weights->dims[0] == 2 && weights->dims[1] == 1))) &&
            weights->dims[0] <= 15 && weights->dims[1] <= 15)
        {
            /* If no pooling & stride = 2 && non-zero-ratio * kx * ky less than 0.7 * 9,
               do first pixel pooling(2x2, stride = 2), no need reshuffle */
            strideXChanged = 1;
            strideYChanged = 1;

            weightDimsChanged[0] = weights->dims[0];
            weightDimsChanged[1] = weights->dims[1];
            weightDimsChanged[2] = weights->dims[2];

            //wb_base->pooling_size_x = 2;
            //wb_base->pooling_size_y = 2;
            //wb_base->pooling_stride = 2;
            //wb_base->do_fisrt_pixel_pool = vx_true_e;
        }

        if (layer_type == VX_NN_DEPTH_WISE_CONVOLUTION_LAYER && hasHwDepthWise &&
            (isV8 && ((weights->dims[0] == 1 && weights->dims[1] == 2) ||
                      (weights->dims[0] == 2 && weights->dims[1] == 1))))
        {
            /*V8 has limitation for 1x2 && 2x1, so don't support depth wise with this shape & 2x2 stride*/
            status = VX_ERROR_INVALID_VALUE;
            vxError("vxoCreateWeightsBiasesParameterFromTensors: current NN didn't support this kernel size %d x %d", weightDimsChanged[0], weightDimsChanged[1]);
            goto exit;
        }
    }

    if (layer_type == VX_NN_DEPTH_WISE_CONVOLUTION_LAYER && hasHwDepthWise &&
        strideXChanged == 1 && strideYChanged == 1 &&
        (weightDimsChanged[2] == 1 || weightDimsChanged[3] == 1))
    {
        doDepthWise = vx_true_e;

        if (weightDimsChanged[2] != 1 && weightDimsChanged[3] == 1)
        {
            /*default set kz = 1, vz = outZ*/
            weightDimsChanged[3] = weightDimsChanged[2];
            weightDimsChanged[2] = 1;
        }
    }

    //wb_base->do_1xN = reallyDo1xN;
    //wb_base->do_zdp_opt = doZdpOpt;

    weight_param.num_of_dims      = weightDimCount;
    weight_param.data_format      = weightDataType;
    weight_param.fixed_point_pos  = TENSOR_POS(weights);
    weight_param.quant_format     = weightQuantType;
    vxMemCopy(weight_param.org_dims_sizes, weightDims, sizeof(weight_param.org_dims_sizes));
    vxMemCopy(weight_param.dims_sizes, weightDimsChanged, sizeof(weight_param.dims_sizes));

    bias_param.fixed_point_pos  = biasFp;
    bias_param.num_of_dims      = biasDimCount;
    bias_param.data_format      = biasDataType;
    bias_param.quant_format     = biasQuantType;

    vxMemCopy(bias_param.dims_sizes, biasDims, sizeof(bias_param.dims_sizes));

    if (context->options.enableWBShare)
    {
        weight_bias = _vxoWeightBias_GetFromPool(context,
                                                 &weight_param,
                                                 &bias_param,
                                                 strideX, strideY,
                                                 strideXChanged, strideYChanged,
                                                 skipValue[0],
                                                 doDepthWise,
                                                 do1xN,
                                                 layer_type,
                                                 weights,
                                                 biases,
                                                 alpha);
        if (weight_bias != VX_NULL)
        {
            vxoReference_Increment((vx_reference)weight_bias, VX_REF_INTERNAL);
            status = VX_SUCCESS;
            goto exit;
        }
    }

    weight_bias = vxoWeightBias_Create(context);
    if (weight_bias == VX_NULL)
    {
        status = VX_FAILURE;
        goto exit;
    }



    if(gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_PER_CHANNEL_QUANT) && (weightQuantType == VX_QUANT_AFFINE_SCALE_PER_CHANNEL) &&(weightDataType == VX_TYPE_UINT8 || weightDataType == VX_TYPE_INT8))
    {
        weight_param.scaleCount = TENSOR_TF_SCALE_COUNT(weights);

        weight_param.quant_scale = (vx_float32 *)vxAllocateAndZeroMemory(weight_param.scaleCount * sizeof(vx_float32));
        gcoOS_MemCopy(weight_param.quant_scale, TENSOR_TF_SCALE_POINTER(weights), weight_param.scaleCount * sizeof(vx_float32));
        bias_param.quant_scale = (vx_float32 *)vxAllocateAndZeroMemory(weight_param.scaleCount * sizeof(vx_float32));
        gcoOS_MemCopy(bias_param.quant_scale, TENSOR_TF_SCALE_POINTER(biases), weight_param.scaleCount * sizeof(vx_float32));

        weight_param.quant_zero_point = (vx_int32 *)vxAllocateAndZeroMemory(weight_param.scaleCount * sizeof(vx_int32));
        gcoOS_MemCopy(weight_param.quant_zero_point, TENSOR_TF_ZEROPOINT_POINTER(weights), weight_param.scaleCount * sizeof(vx_int32));
        bias_param.quant_zero_point = (vx_int32 *)vxAllocateAndZeroMemory(weight_param.scaleCount * sizeof(vx_int32));
        gcoOS_MemCopy(bias_param.quant_zero_point, TENSOR_TF_ZEROPOINT_POINTER(biases), weight_param.scaleCount * sizeof(vx_int32));
    }
    else
    {
        weight_param.scaleCount = 1;

        weight_param.quant_scale = (vx_float32 *)vxAllocateAndZeroMemory(weight_param.scaleCount * sizeof(vx_float32));
        bias_param.quant_scale = (vx_float32 *)vxAllocateAndZeroMemory(weight_param.scaleCount * sizeof(vx_float32));

        weight_param.quant_zero_point = (vx_int32 *)vxAllocateAndZeroMemory(weight_param.scaleCount * sizeof(vx_int32));
        bias_param.quant_zero_point = (vx_int32 *)vxAllocateAndZeroMemory(weight_param.scaleCount * sizeof(vx_int32));

        bias_param.quant_scale[0] = biasScale;
        bias_param.quant_zero_point[0] = 1;
        weight_param.quant_scale[0]      = TENSOR_TF_SCALE(weights);
        weight_param.quant_zero_point[0] = TENSOR_TF_ZEROPOINT(weights);

    }

    status = weight_bias->initialize(weight_bias,
                                     &weight_param,
                                     &bias_param,
                                     strideX,
                                     strideY,
                                     strideXChanged,
                                     strideYChanged,
                                     -1,
                                     skipValue,
                                     doDepthWise,
                                     do1xN,
                                     layer_type);
    if (status != VX_SUCCESS) goto exit;

    WB_WEIGHT_TENSOR(weight_bias) = weights;
    vxoReference_Increment((vx_reference)WB_WEIGHT_TENSOR(weight_bias), VX_REF_INTERNAL);

    if (biases != VX_NULL)
    {
        WB_BIAS_TENSOR(weight_bias) = biases;
        vxoReference_Increment((vx_reference)WB_BIAS_TENSOR(weight_bias), VX_REF_INTERNAL);
    }

    if (alpha != VX_NULL && (conv_operation_type == VX_NN_CONV_PRELU || conv_operation_type == VX_NN_CONV_LEAKYRELU))
    {
        WB_OPERATION_TYPE(weight_bias) = conv_operation_type;
        WB_ALPHA_REF(weight_bias) = alpha;
        //weight_bias->set_alpha_tensor(weight_bias, alpha);
        vxoReference_Increment((vx_reference)WB_ALPHA_REF(weight_bias), VX_REF_INTERNAL);
    }

    if (optimizations != VX_NULL)
    {
        weight_bias->set_optimization(weight_bias, optimizations, opt_size);
    }
    if (context->options.enableWBShare)
    {
        _vxoWeightBias_AddToPool(context, weight_bias);
    }

    if(skipValue != VX_NULL)
    {
        vxFree(skipValue);
        skipValue = VX_NULL;
    }

    if(weight_param.quant_scale != VX_NULL)
    {
        vxFree(weight_param.quant_scale);
        weight_param.quant_scale = VX_NULL;
    }
    if(bias_param.quant_scale != VX_NULL)
    {
        vxFree(bias_param.quant_scale);
        bias_param.quant_scale = VX_NULL;
    }

    if(weight_param.quant_zero_point != VX_NULL)
    {
        vxFree(weight_param.quant_zero_point);
        weight_param.quant_zero_point = VX_NULL;
    }
    if(bias_param.quant_zero_point != VX_NULL)
    {
        vxFree(bias_param.quant_zero_point);
        bias_param.quant_zero_point = VX_NULL;
    }

exit:
    if (status != VX_SUCCESS)
    {
        if (weight_bias != VX_NULL)
        {
            vxoWeightBias_Release(&weight_bias);
        }

        if(weight_param.quant_scale != VX_NULL)
        {
            vxFree(weight_param.quant_scale);
            weight_param.quant_scale = VX_NULL;
        }
        if(bias_param.quant_scale != VX_NULL)
        {
            vxFree(bias_param.quant_scale);
            bias_param.quant_scale = VX_NULL;
        }

        if(weight_param.quant_zero_point != VX_NULL)
        {
            vxFree(weight_param.quant_zero_point);
            weight_param.quant_zero_point = VX_NULL;
        }
        if(bias_param.quant_zero_point != VX_NULL)
        {
            vxFree(bias_param.quant_zero_point);
            bias_param.quant_zero_point = VX_NULL;
        }

        return VX_NULL;
    }
    else
    {
        return weight_bias;
    }
}

VX_INTERNAL_API vx_weights_biases_parameter vxoCreateWeightsBiasesParameterFromParams(
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
    vx_status status = VX_SUCCESS;
    vx_uint32 *skipValue = VX_NULL;
    vx_uint32 strideX = 0, strideY = 0;
    vx_weights_biases_parameter wb;
    vx_weight_bias_general_param_s weight_param, bias_param;

    wb = vxoWeightBias_Create(context);
    if (wb == VX_NULL) return VX_NULL;

    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TF_QUANT) && weights_data_format == VX_TYPE_UINT8)
    {
        skipValue[0] = 1;
    }

    weight_param.num_of_dims      = weights_num_of_dims;
    weight_param.data_format      = weights_data_format;
    weight_param.fixed_point_pos  = weights_fixed_point_pos;
    weight_param.quant_format     = weights_quant_format;
    //weight_param.quant_scale[0]      = 1.0f;
    //weight_param.quant_zero_point[0] = 1;
    vxMemCopy(weight_param.org_dims_sizes, weights_dims, sizeof(weight_param.org_dims_sizes));
    vxMemCopy(weight_param.dims_sizes, weights_dims, sizeof(weight_param.dims_sizes));

    bias_param.num_of_dims      = biases_num_of_dims;
    bias_param.data_format      = biases_data_format;
    bias_param.fixed_point_pos  = biases_fixed_point_pos;
    bias_param.quant_format     = biases_quant_format;
    //bias_param.quant_scale[0]      = 1.0f;
    //bias_param.quant_zero_point[0] = 1;
    vxMemCopy(bias_param.dims_sizes, biases_dims, sizeof(bias_param.dims_sizes));

    _getNNStride(layer_type,
                 inputs_dims,
                 convolution_outputs_dims,
                 weights_base_dims,
                 pad_x_left,
                 pad_x_right,
                 pad_y_top,
                 pad_y_bottom,
                 down_scale_size_rounding,
                 &strideX, &strideY);

    status = wb->initialize(wb,
                            &weight_param,
                            &bias_param,
                            strideX,
                            strideY,
                            strideX,
                            strideY,
                            -1,
                            skipValue,
                            vx_false_e,
                            vx_false_e,
                            layer_type);

    if (status != VX_SUCCESS)
    {
        vxoWeightBias_Release(&wb);
        return VX_NULL;
    }

    return wb;
}

VX_INTERNAL_API vx_weights_biases_parameter vxoCreateWeightsBiasesParameterFromTensorsPRelu(
    vx_enum     layer_type,
    vx_uint32 * inputs_dims,
    vx_uint32 * convolution_outputs_dims,
    vx_uint32 * pool_outputs_dims,
    const vx_nn_convolution_relu_pooling_params convolution_relu_pooling_params,
    vx_size size_of_convolution_relu_pooling_params,
    vx_weights_biases_parameter_optimizations_t *optimizations,
    vx_size size_of_optimizations,
    vx_tensor   weights,
    vx_tensor   biases,
    vx_tensor   alpha)
{
    vx_weights_biases_parameter wb;
    vx_context context = vxGetContext((vx_reference)weights);
    vx_uint32 stride_x = 0;
    vx_uint32 stride_y = 0;
    vx_uint32 num_of_input_dims = 0;
    vx_uint32 num_of_output_dims = 0;
    vx_enum output_format = TENSOR_DATA_TYPE(weights);
    vx_enum convert_format = 0;
    vx_enum rank_mode = VX_TENSOR_RANK_WHCN;

    vx_enum layer = layer_type;

    gcmDUMP_API("$VX vxoCreateWeightsBiasesParameterFromTensorsPRelu: layer_type=0x%x, inputs_dims=%p, "
        "convolution_outputs_dims=%p, pool_outputs_dims=%p, convolution_relu_pooling_params=%p, "
        "size_of_convolution_relu_pooling_params=0x%lx, optimizations=%p, size_of_optimizations=0x%lx, weights=%p, biases=%p",
        layer_type, inputs_dims, convolution_outputs_dims, pool_outputs_dims, convolution_relu_pooling_params, size_of_convolution_relu_pooling_params, optimizations, size_of_optimizations, weights, biases);

    if (!vxoContext_IsValid(context)) return VX_NULL;

    if (size_of_convolution_relu_pooling_params == sizeof(vx_nn_convolution_relu_pooling_params_ext_t))
    {
        vx_nn_convolution_relu_pooling_params_ext_t conv_ext = *((vx_nn_convolution_relu_pooling_params_ext_t*)(convolution_relu_pooling_params));
        stride_x = conv_ext.stride_x;
        stride_y = conv_ext.stride_y;
    }
    else if (size_of_convolution_relu_pooling_params == sizeof(vx_nn_convolution_relu_pooling_params_ext2_t))
    {
        vx_nn_convolution_relu_pooling_params_ext2_t conv_ext2 = *((vx_nn_convolution_relu_pooling_params_ext2_t*)(convolution_relu_pooling_params));
        stride_x = conv_ext2.ext.stride_x;
        stride_y = conv_ext2.ext.stride_y;
        rank_mode = conv_ext2.src_rank_mode;
        convert_format = conv_ext2.convert_dst_format;

        if (conv_ext2.depth_multiplier == 1 &&
            (TENSOR_DATA_TYPE(weights) == VX_TYPE_INT8 || TENSOR_DATA_TYPE(weights) == VX_TYPE_UINT8) &&
            vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_NN_DEPTHWISE_SUPPORT))
            layer = VX_NN_DEPTH_WISE_CONVOLUTION_LAYER;
    }
    else if (size_of_convolution_relu_pooling_params != sizeof(vx_nn_convolution_relu_pooling_params_t))
    {
        vxError("Invalid parameter convolution_relu_pooling_params");
        return VX_NULL;
    }

    if (optimizations)
    {
        if (size_of_optimizations == sizeof(vx_weights_biases_parameter_optimizations_t))
            output_format = optimizations->outputFormat;
        else if (size_of_optimizations == sizeof(vx_weights_biases_parameter_optimizations_ext_t))
        {
            vx_weights_biases_parameter_optimizations_ext_t opt = *((vx_weights_biases_parameter_optimizations_ext_t *)optimizations);
            output_format = opt.outputFormat;
            num_of_input_dims = opt.num_of_input_dims;
            num_of_output_dims = opt.num_of_output_dims;
        }
        else if (size_of_optimizations == sizeof(vx_weights_biases_parameter_optimizations_ext2_t))
        {
            vx_weights_biases_parameter_optimizations_ext_t opt = *((vx_weights_biases_parameter_optimizations_ext_t *)optimizations);
            output_format = opt.outputFormat;
            num_of_input_dims = opt.num_of_input_dims;
            num_of_output_dims = opt.num_of_output_dims;
        }
        else
        {
            vxError("Invalid parameter convolution_relu_pooling_params");
            return VX_NULL;
        }
    }

    wb = vxoCreateWeightsBiasesParameterFromTensors(
            context,
            layer,
            inputs_dims,
            num_of_input_dims,
            num_of_output_dims,
            convolution_relu_pooling_params->pad_x_left,
            convolution_relu_pooling_params->pad_x_right,
            convolution_relu_pooling_params->pad_y_top,
            convolution_relu_pooling_params->pad_y_bottom,
            convolution_relu_pooling_params->pool_size_x,
            convolution_relu_pooling_params->pool_size_y,
            stride_x,
            stride_y,
            convolution_relu_pooling_params->down_scale_size_rounding,
            convolution_outputs_dims,
            pool_outputs_dims,
            optimizations,
            (vx_uint32)size_of_optimizations,
            output_format,
            convert_format,
            rank_mode,
            weights,
            biases,
            (vx_reference)alpha,
            VX_NN_CONV_PRELU,
            vx_true_e);

    return wb;
}



VX_INTERNAL_API vx_weights_biases_parameter vxoCreateWeightsBiasesParameterFromTensorsLeakyRelu(
    vx_enum     layer_type,
    vx_uint32 * inputs_dims,
    vx_uint32 * convolution_outputs_dims,
    vx_uint32 * pool_outputs_dims,
    const vx_nn_convolution_relu_pooling_params convolution_relu_pooling_params,
    vx_size size_of_convolution_relu_pooling_params,
    vx_weights_biases_parameter_optimizations_t *optimizations,
    vx_size size_of_optimizations,
    vx_tensor   weights,
    vx_tensor   biases,
    vx_scalar   alpha)
{
    vx_weights_biases_parameter wb;
    vx_context context = vxGetContext((vx_reference)weights);
    vx_uint32 stride_x = 0;
    vx_uint32 stride_y = 0;
    vx_uint32 num_of_input_dims = 0;
    vx_uint32 num_of_output_dims = 0;
    vx_enum output_format = TENSOR_DATA_TYPE(weights);
    vx_enum convert_format = 0;
    vx_enum rank_mode = VX_TENSOR_RANK_WHCN;

    vx_enum layer = layer_type;

    gcmDUMP_API("$VX vxoCreateWeightsBiasesParameterFromTensorsPRelu: layer_type=0x%x, inputs_dims=%p, "
        "convolution_outputs_dims=%p, pool_outputs_dims=%p, convolution_relu_pooling_params=%p, "
        "size_of_convolution_relu_pooling_params=0x%lx, optimizations=%p, size_of_optimizations=0x%lx, weights=%p, biases=%p",
        layer_type, inputs_dims, convolution_outputs_dims, pool_outputs_dims, convolution_relu_pooling_params, size_of_convolution_relu_pooling_params, optimizations, size_of_optimizations, weights, biases);

    if (!vxoContext_IsValid(context)) return VX_NULL;

    if (size_of_convolution_relu_pooling_params == sizeof(vx_nn_convolution_relu_pooling_params_ext_t))
    {
        vx_nn_convolution_relu_pooling_params_ext_t conv_ext = *((vx_nn_convolution_relu_pooling_params_ext_t*)(convolution_relu_pooling_params));
        stride_x = conv_ext.stride_x;
        stride_y = conv_ext.stride_y;
    }
    else if (size_of_convolution_relu_pooling_params == sizeof(vx_nn_convolution_relu_pooling_params_ext2_t))
    {
        vx_nn_convolution_relu_pooling_params_ext2_t conv_ext2 = *((vx_nn_convolution_relu_pooling_params_ext2_t*)(convolution_relu_pooling_params));
        stride_x = conv_ext2.ext.stride_x;
        stride_y = conv_ext2.ext.stride_y;
        rank_mode = conv_ext2.src_rank_mode;
        convert_format = conv_ext2.convert_dst_format;

        if (conv_ext2.depth_multiplier == 1 &&
            (TENSOR_DATA_TYPE(weights) == VX_TYPE_INT8 || TENSOR_DATA_TYPE(weights) == VX_TYPE_UINT8) &&
            vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_NN_DEPTHWISE_SUPPORT))
            layer = VX_NN_DEPTH_WISE_CONVOLUTION_LAYER;
    }
    else if (size_of_convolution_relu_pooling_params != sizeof(vx_nn_convolution_relu_pooling_params_t))
    {
        vxError("Invalid parameter convolution_relu_pooling_params");
        return VX_NULL;
    }

    if (optimizations)
    {
        if (size_of_optimizations == sizeof(vx_weights_biases_parameter_optimizations_t))
            output_format = optimizations->outputFormat;
        else if (size_of_optimizations == sizeof(vx_weights_biases_parameter_optimizations_ext_t))
        {
            vx_weights_biases_parameter_optimizations_ext_t opt = *((vx_weights_biases_parameter_optimizations_ext_t *)optimizations);
            output_format = opt.outputFormat;
            num_of_input_dims = opt.num_of_input_dims;
            num_of_output_dims = opt.num_of_output_dims;
        }
        else if (size_of_optimizations == sizeof(vx_weights_biases_parameter_optimizations_ext2_t))
        {
            vx_weights_biases_parameter_optimizations_ext_t opt = *((vx_weights_biases_parameter_optimizations_ext_t *)optimizations);
            output_format = opt.outputFormat;
            num_of_input_dims = opt.num_of_input_dims;
            num_of_output_dims = opt.num_of_output_dims;
        }
        else
        {
            vxError("Invalid parameter convolution_relu_pooling_params");
            return VX_NULL;
        }
    }

    wb = vxoCreateWeightsBiasesParameterFromTensors(
            context,
            layer,
            inputs_dims,
            num_of_input_dims,
            num_of_output_dims,
            convolution_relu_pooling_params->pad_x_left,
            convolution_relu_pooling_params->pad_x_right,
            convolution_relu_pooling_params->pad_y_top,
            convolution_relu_pooling_params->pad_y_bottom,
            convolution_relu_pooling_params->pool_size_x,
            convolution_relu_pooling_params->pool_size_y,
            stride_x,
            stride_y,
            convolution_relu_pooling_params->down_scale_size_rounding,
            convolution_outputs_dims,
            pool_outputs_dims,
            optimizations,
            size_of_optimizations,
            output_format,
            convert_format,
            rank_mode,
            weights,
            biases,
            (vx_reference)alpha,
            VX_NN_CONV_LEAKYRELU,
            vx_true_e);

    return wb;
}

VX_INTERNAL_API vx_weights_biases_parameter vxoCreateWeightsBiasesFromWeightBias(
    vx_context                  context,
    vx_weights_biases_parameter old_wb,
    vx_uint32*                  weight_dims,
    vx_uint32                   weight_num_of_dims
    )
{
    vx_weights_biases_parameter wb;

    vxmASSERT(old_wb != VX_NULL);

    wb = vxoWeightBias_Create(context);
    if (wb == VX_NULL) return VX_NULL;

    vxMemCopy(&WB_EXTERNAL_PARAM(wb), &WB_EXTERNAL_PARAM(old_wb), sizeof(WB_EXTERNAL_PARAM(wb)));

    vxoWeightBias_ScaleAndZP(wb, &(WB_EXTERNAL_PARAM(old_wb).weight_param), &(WB_EXTERNAL_PARAM(old_wb).bias_param));

    if (WB_WEIGHT_TENSOR(old_wb) != VX_NULL)
    {
        WB_WEIGHT_TENSOR(wb) = WB_WEIGHT_TENSOR(old_wb);
        vxoReference_Increment((vx_reference)WB_WEIGHT_TENSOR(wb), VX_REF_INTERNAL);
    }
    if (WB_BIAS_TENSOR(old_wb) != VX_NULL)
    {
        WB_BIAS_TENSOR(wb) = WB_BIAS_TENSOR(old_wb);
        vxoReference_Increment((vx_reference)WB_BIAS_TENSOR(wb), VX_REF_INTERNAL);
    }
    if (WB_ALPHA_REF(old_wb) != VX_NULL)
    {
        WB_ALPHA_REF(wb) = WB_ALPHA_REF(old_wb);
        vxoReference_Increment((vx_reference)WB_ALPHA_REF(wb), VX_REF_INTERNAL);
    }


    if (weight_dims != VX_NULL &&
        weight_num_of_dims > 0 &&
        weight_num_of_dims < VX_CONTEXT_TENSOR_MAX_DIMENSION)
    {
        vxMemCopy(WB_WEIGHT_DIMS_SIZES(wb), weight_dims, sizeof(vx_uint32) * weight_num_of_dims);
    }

    return wb;
}

VX_INTERNAL_API vx_status vxoReleaseWeightsBiases(
    vx_weights_biases_parameter*  wb_ptr
    )
{
    return vxoWeightBias_Release(wb_ptr);
}

VX_INTERNAL_API vx_status vxoCalculateNNCompressionFirstTime(
    vx_context                   context,
    vx_weights_biases_parameter  wb,
    vx_tensor                    output
    )
{
    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_XYDP0))
    {
        vx_uint32 nnCoreCount = (WB_WEIGHT_DATA_FORMAT(wb) == VX_TYPE_INT16) ?
        context->nnConfig.fixedFeature.nnCoreCountInt16 : (WB_WEIGHT_DATA_FORMAT(wb) == VX_TYPE_FLOAT16) ?
        context->nnConfig.fixedFeature.nnCoreCountFloat16 : (WB_WEIGHT_DATA_FORMAT(wb) == VX_TYPE_BFLOAT16) ?
        context->nnConfig.fixedFeature.nnCoreCountBFloat16 :context->nnConfig.fixedFeature.nnCoreCount;

        vx_uint32 filterPerCore = (WB_OUTPUT_Z(wb) % nnCoreCount == 0) ?
        WB_OUTPUT_Z(wb) / nnCoreCount : WB_OUTPUT_Z(wb) / nnCoreCount + 1;

        wb->calc_compress_ratio(wb, VXNNE_OPERATION_TARGET_NN, filterPerCore, TENSOR_STRIDE_INDEX(output, 2));
    }
    else
    {
        wb->calc_compress_ratio(wb, VXNNE_OPERATION_TARGET_NN, WB_OUTPUT_Z(wb), TENSOR_STRIDE_INDEX(output, 2));
    }

    return VX_SUCCESS;
}

/**********************************************************************************************/

VX_API_ENTRY vx_weights_biases_parameter VX_API_CALL vxCreateWeightsBiasesParameter(
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
    vx_int8     weights_fixed_point_pos,
    vx_uint32   biases_num_of_dims,
    vx_uint32 * biases_dims,
    vx_enum     biases_data_format,
    vx_int8     biases_fixed_point_pos,
    vx_uint32   raw_data_size
    )
{
    vx_weights_biases_parameter wb;
    vx_uint32 padXLeft, padXRight, padYTop, padYBottom;

    gcmDUMP_API("$VX vxCreateWeightsBiasesParameter: context=%p, layer_type=0x%x, num_of_dims=0x%x, inputs_dims=%p, pad_x=0x%x, pad_y=0x%x, pooling_size_x=0x%x, pooling_size_y=0x%x,"\
        " down_scale_size_rounding=0x%x, convolution_outputs_dims=%p, pool_outputs_dims=%p, weights_num_of_dims=0x%x, weights_dims=%p, weights_data_format=0x%x, weights_fixed_point_pos=0x%x,"\
        " biases_num_of_dims=0x%x, biases_dims=%p, biases_data_format=0x%x, biases_fixed_point_pos=0x%x, raw_data_size=0x%x", context, layer_type, num_of_dims, inputs_dims, pad_x, pad_y, pooling_size_x, pooling_size_y,
        down_scale_size_rounding, convolution_outputs_dims, pool_outputs_dims, weights_num_of_dims, weights_dims, weights_data_format, weights_fixed_point_pos, biases_num_of_dims, biases_dims, biases_data_format, biases_fixed_point_pos, raw_data_size);

    if (!vxoContext_IsValid(context)) return VX_NULL;

    padXLeft = padXRight = pad_x;
    padYTop = padYBottom = pad_y;

    wb = vxoCreateWeightsBiasesParameterFromParams(
            context,
            layer_type,
            inputs_dims,
            padXLeft,
            padXRight,
            padYTop,
            padYBottom,
            pooling_size_x,
            pooling_size_y,
            down_scale_size_rounding,
            convolution_outputs_dims,
            pool_outputs_dims,
            weights_num_of_dims,
            weights_dims,
            weights_dims,
            weights_data_format,
            VX_QUANT_DYNAMIC_FIXED_POINT,
            weights_fixed_point_pos,
            biases_num_of_dims,
            biases_dims,
            biases_dims,
            biases_data_format,
            VX_QUANT_DYNAMIC_FIXED_POINT,
            biases_fixed_point_pos);

    return wb;
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
    vx_weights_biases_parameter wb;
    vx_context context = vxGetContext((vx_reference)weights);
    vx_uint32 padXLeft, padXRight, padYTop, padYBottom;
    vx_enum outType = TENSOR_DATA_TYPE(weights);

    gcmDUMP_API("$VX vxCreateWeightsBiasesParameterFromTensors: layer_type=0x%x, num_of_dims=0x%x, inputs_dims=%p, pad_x=0x%x, pad_y=0x%x, pooling_size_x=0x%x, pooling_size_y=0x%x,"\
        " down_scale_size_rounding=0x%x, convolution_outputs_dims=%p, pool_outputs_dims=%p, optimizations=%p, weights=%p, biases=%p",
        layer_type, num_of_dims, inputs_dims, pad_x, pad_y, pooling_size_x, pooling_size_y, down_scale_size_rounding, convolution_outputs_dims, pool_outputs_dims, optimizations, weights, biases);

    if (!vxoContext_IsValid(context)) return VX_NULL;


    padXLeft = padXRight = pad_x;
    padYTop = padYBottom = pad_y;

    if (optimizations)
        outType = optimizations->outputFormat;

    wb = vxoCreateWeightsBiasesParameterFromTensors(
            context,
            layer_type,
            inputs_dims,
            num_of_dims,
            num_of_dims,
            padXLeft,
            padXRight,
            padYTop,
            padYBottom,
            pooling_size_x,
            pooling_size_y,
            0,
            0,
            down_scale_size_rounding,
            convolution_outputs_dims,
            pool_outputs_dims,
            optimizations,
            0,
            outType,
            0,
            VX_TENSOR_RANK_WHCN,
            weights,
            biases,
            VX_NULL,
            VX_NN_CONV_ONLY,
            vx_true_e);

    return wb;
}

VX_API_ENTRY vx_weights_biases_parameter VX_API_CALL vxCreateWeightsBiasesParameterFromTensors2(
    vx_enum     layer_type,
    vx_uint32   num_of_dims,
    vx_uint32 * inputs_dims,
    vx_uint32 * convolution_outputs_dims,
    vx_uint32 * pool_outputs_dims,
    vx_enum     output_format,
    const vx_nn_convolution_relu_pooling_params convolution_relu_pooling_params,
    vx_size size_of_convlution_relu_pooling_params,
    vx_weights_biases_parameter_optimizations_t *optimizations,
    vx_tensor   weights,
    vx_tensor   biases)
{
    vx_weights_biases_parameter wb;
    vx_context context = vxGetContext((vx_reference)weights);
    vx_uint32 stride_x = 0;
    vx_uint32 stride_y = 0;
    vx_enum convert_format = 0;
    vx_enum rank_mode = VX_TENSOR_RANK_WHCN;
    vx_enum layer = layer_type;

    gcmDUMP_API("$VX vxCreateWeightsBiasesParameterFromTensors2: layer_type=0x%x, num_of_dims=0x%x, inputs_dims=%p, convolution_outputs_dims=%p, pool_outputs_dims=%p, output_format=0x%x,"\
        " convolution_relu_pooling_params=%p, size_of_convlution_relu_pooling_paramss=0x%lx, optimizations=%p, weights=%p, biases=%p",
        layer_type, num_of_dims, inputs_dims, convolution_outputs_dims, pool_outputs_dims, output_format, convolution_relu_pooling_params, size_of_convlution_relu_pooling_params, optimizations, weights, biases);


    if (!vxoContext_IsValid(context)) return VX_NULL;

    if (size_of_convlution_relu_pooling_params == sizeof(vx_nn_convolution_relu_pooling_params_ext_t))
    {
        vx_nn_convolution_relu_pooling_params_ext_t conv_ext = *((vx_nn_convolution_relu_pooling_params_ext_t*)(convolution_relu_pooling_params));
        stride_x = conv_ext.stride_x;
        stride_y = conv_ext.stride_y;
    }
    else if (size_of_convlution_relu_pooling_params == sizeof(vx_nn_convolution_relu_pooling_params_ext2_t))
    {
        vx_nn_convolution_relu_pooling_params_ext2_t conv_ext2 = *((vx_nn_convolution_relu_pooling_params_ext2_t*)(convolution_relu_pooling_params));
        stride_x = conv_ext2.ext.stride_x;
        stride_y = conv_ext2.ext.stride_y;
        rank_mode = conv_ext2.src_rank_mode;
        convert_format = conv_ext2.convert_dst_format;

        if (conv_ext2.depth_multiplier == 1 &&
            vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_NN_DEPTHWISE_SUPPORT))
            layer = VX_NN_DEPTH_WISE_CONVOLUTION_LAYER;
    }
    else if (size_of_convlution_relu_pooling_params != sizeof(vx_nn_convolution_relu_pooling_params_t))
    {
        vxError("Invalid parameter convolution_relu_pooling_params");
        return VX_NULL;
    }

    wb = vxoCreateWeightsBiasesParameterFromTensors(
            context,
            layer,
            inputs_dims,
            num_of_dims,
            num_of_dims,
            convolution_relu_pooling_params->pad_x_left,
            convolution_relu_pooling_params->pad_x_right,
            convolution_relu_pooling_params->pad_y_top,
            convolution_relu_pooling_params->pad_y_bottom,
            convolution_relu_pooling_params->pool_size_x,
            convolution_relu_pooling_params->pool_size_y,
            stride_x,
            stride_y,
            convolution_relu_pooling_params->down_scale_size_rounding,
            convolution_outputs_dims,
            pool_outputs_dims,
            optimizations,
            0,
            output_format,
            convert_format,
            rank_mode,
            weights,
            biases,
            VX_NULL,
            VX_NN_CONV_ONLY,
            vx_true_e);

    return wb;
}


VX_API_ENTRY vx_weights_biases_parameter VX_API_CALL vxCreateWeightsBiasesParameterFromTensors3(
    vx_enum     layer_type,
    vx_uint32 * inputs_dims,
    vx_uint32 * convolution_outputs_dims,
    vx_uint32 * pool_outputs_dims,
    const vx_nn_convolution_relu_pooling_params convolution_relu_pooling_params,
    vx_size size_of_convolution_relu_pooling_params,
    vx_weights_biases_parameter_optimizations_t *optimizations,
    vx_size size_of_optimizations,
    vx_tensor   weights,
    vx_tensor   biases)
{
    vx_weights_biases_parameter wb;
    vx_context context = vxGetContext((vx_reference)weights);
    vx_uint32 stride_x = 0;
    vx_uint32 stride_y = 0;
    vx_uint32 num_of_input_dims = 0;
    vx_uint32 num_of_output_dims = 0;
    vx_enum output_format = TENSOR_DATA_TYPE(weights);
    vx_enum convert_format = 0;
    vx_enum rank_mode = VX_TENSOR_RANK_WHCN;

    vx_enum layer = layer_type;

    gcmDUMP_API("$VX vxCreateWeightsBiasesParameterFromTensors3: layer_type=0x%x, inputs_dims=%p, "
        "convolution_outputs_dims=%p, pool_outputs_dims=%p, convolution_relu_pooling_params=%p, "
        "size_of_convolution_relu_pooling_params=0x%lx, optimizations=%p, size_of_optimizations=0x%lx, weights=%p, biases=%p",
        layer_type, inputs_dims, convolution_outputs_dims, pool_outputs_dims, convolution_relu_pooling_params, size_of_convolution_relu_pooling_params, optimizations, size_of_optimizations, weights, biases);

    if (!vxoContext_IsValid(context)) return VX_NULL;

    if (size_of_convolution_relu_pooling_params == sizeof(vx_nn_convolution_relu_pooling_params_ext_t))
    {
        vx_nn_convolution_relu_pooling_params_ext_t conv_ext = *((vx_nn_convolution_relu_pooling_params_ext_t*)(convolution_relu_pooling_params));
        stride_x = conv_ext.stride_x;
        stride_y = conv_ext.stride_y;
    }
    else if (size_of_convolution_relu_pooling_params == sizeof(vx_nn_convolution_relu_pooling_params_ext2_t))
    {
        vx_nn_convolution_relu_pooling_params_ext2_t conv_ext2 = *((vx_nn_convolution_relu_pooling_params_ext2_t*)(convolution_relu_pooling_params));
        stride_x = conv_ext2.ext.stride_x;
        stride_y = conv_ext2.ext.stride_y;
        rank_mode = conv_ext2.src_rank_mode;
        convert_format = conv_ext2.convert_dst_format;

        if (conv_ext2.depth_multiplier == 1 &&
            vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_NN_DEPTHWISE_SUPPORT))
            layer = VX_NN_DEPTH_WISE_CONVOLUTION_LAYER;
    }
    else if (size_of_convolution_relu_pooling_params != sizeof(vx_nn_convolution_relu_pooling_params_t))
    {
        vxError("Invalid parameter convolution_relu_pooling_params");
        return VX_NULL;
    }

    if (optimizations)
    {
        if (size_of_optimizations == sizeof(vx_weights_biases_parameter_optimizations_t))
            output_format = optimizations->outputFormat;
        else if (size_of_optimizations == sizeof(vx_weights_biases_parameter_optimizations_ext_t))
        {
            vx_weights_biases_parameter_optimizations_ext_t opt = *((vx_weights_biases_parameter_optimizations_ext_t *)optimizations);
            output_format = opt.outputFormat;
            num_of_input_dims = opt.num_of_input_dims;
            num_of_output_dims = opt.num_of_output_dims;
        }
        else if (size_of_optimizations == sizeof(vx_weights_biases_parameter_optimizations_ext2_t))
        {
            vx_weights_biases_parameter_optimizations_ext2_t opt = *((vx_weights_biases_parameter_optimizations_ext2_t *)optimizations);
            output_format = opt.ext.outputFormat;
            num_of_input_dims = opt.ext.num_of_input_dims;
            num_of_output_dims = opt.ext.num_of_output_dims;
        }
        else
        {
            vxError("Invalid parameter convolution_relu_pooling_params");
            return VX_NULL;
        }
    }

    wb = vxoCreateWeightsBiasesParameterFromTensors(
            context,
            layer,
            inputs_dims,
            num_of_input_dims,
            num_of_output_dims,
            convolution_relu_pooling_params->pad_x_left,
            convolution_relu_pooling_params->pad_x_right,
            convolution_relu_pooling_params->pad_y_top,
            convolution_relu_pooling_params->pad_y_bottom,
            convolution_relu_pooling_params->pool_size_x,
            convolution_relu_pooling_params->pool_size_y,
            stride_x,
            stride_y,
            convolution_relu_pooling_params->down_scale_size_rounding,
            convolution_outputs_dims,
            pool_outputs_dims,
            optimizations,
            size_of_optimizations,
            output_format,
            convert_format,
            rank_mode,
            weights,
            biases,
            VX_NULL,
            VX_NN_CONV_ONLY,
            vx_true_e);

    return wb;
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseWeightsBiasesParameter(
    vx_weights_biases_parameter *weights_bias)
{
    gcmDUMP_API("$VX vxReleaseWeightsBiasesParameter: weights_bias=%p", weights_bias);

    if ((*weights_bias) == VX_NULL)
    {
        vxError("%s[%d]: Weights_bias is invalid!\n", __FUNCTION__, __LINE__);
        return VX_ERROR_INVALID_PARAMETERS;
    }

    return vxoWeightBias_Release(weights_bias);
}

