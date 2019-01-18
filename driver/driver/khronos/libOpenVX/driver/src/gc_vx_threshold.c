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


#include <gc_vx_common.h>

static vx_bool vxoIsValidThresholdFormat(vx_df_image format)
{
    vx_bool ret = vx_false_e;
    if (format == VX_DF_IMAGE_U8  ||
        format == VX_DF_IMAGE_S16 ||
        format == VX_DF_IMAGE_U16 ||
        format == VX_DF_IMAGE_S32 ||
        format == VX_DF_IMAGE_U32 )
    {
        ret = vx_true_e;
    }
    return ret;
}

static vx_bool vxoIsValidThresholdFormatEx(vx_df_image format)
{
    vx_bool ret = vx_false_e;
    if (format == VX_DF_IMAGE_RGB  ||
        format == VX_DF_IMAGE_RGBX ||
        format == VX_DF_IMAGE_NV12 ||
        format == VX_DF_IMAGE_NV21 ||
        format == VX_DF_IMAGE_UYVY ||
        format == VX_DF_IMAGE_YUYV ||
        format == VX_DF_IMAGE_IYUV ||
        format == VX_DF_IMAGE_YUV4 )
    {
        ret = vx_true_e;
    }
    return ret;
}

VX_PRIVATE_API vx_bool vxoThreshold_IsValidType(vx_enum thresholdType)
{
    switch (thresholdType)
    {
        case VX_THRESHOLD_TYPE_BINARY:
        case VX_THRESHOLD_TYPE_RANGE:
            return vx_true_e;

        default:
            return vx_false_e;
    }
}

VX_PRIVATE_API vx_bool vxoThreshold_IsValidDataType(vx_enum dataType)
{
    return (vx_bool)(dataType == VX_TYPE_INT8 ||
                    dataType == VX_TYPE_UINT8 ||
                    dataType == VX_TYPE_INT16 ||
                    dataType == VX_TYPE_UINT16 ||
                    dataType == VX_TYPE_INT32 ||
                    dataType == VX_TYPE_UINT32);
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseThreshold(vx_threshold *threshold)
{
    gcmDUMP_API("$VX vxReleaseThreshold: threshold=%p", threshold);

    return vxoReference_Release((vx_reference_ptr)threshold, VX_TYPE_THRESHOLD, VX_REF_EXTERNAL);
}

VX_API_ENTRY vx_threshold VX_API_CALL vxCreateThreshold(vx_context context, vx_enum thresh_type, vx_enum data_type)
{
    vx_threshold threshold;

    gcmDUMP_API("$VX vxCreateThreshold: context=%p, thresh_type=0x%x, data_type=0x%x", context, thresh_type, data_type);

    if (!vxoContext_IsValid(context)) return VX_NULL;

    if (!vxoThreshold_IsValidDataType(data_type) || !vxoThreshold_IsValidType(thresh_type))
    {
        return (vx_threshold)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_TYPE);
    }

    threshold = (vx_threshold)vxoReference_Create(context, VX_TYPE_THRESHOLD, VX_REF_EXTERNAL, &context->base);

    if (vxoReference_GetStatus((vx_reference)threshold) != VX_SUCCESS) return threshold;

    threshold->thresholdType = thresh_type;
    threshold->dataType      = data_type;

    switch(data_type)
    {
    case VX_TYPE_INT8:
        threshold->trueValue.U8 = VX_DEFAULT_THRESHOLD_TRUE_VALUE;
        threshold->falseValue.U8 = VX_DEFAULT_THRESHOLD_FALSE_VALUE;
        break;
    case VX_TYPE_UINT8:
        threshold->trueValue.U8 = VX_DEFAULT_THRESHOLD_TRUE_VALUE;
        threshold->falseValue.U8 = VX_DEFAULT_THRESHOLD_FALSE_VALUE;
        break;
    case VX_TYPE_UINT16:
        threshold->trueValue.U16 = VX_DEFAULT_THRESHOLD_TRUE_VALUE;
        threshold->falseValue.U16 = VX_DEFAULT_THRESHOLD_FALSE_VALUE;
        break;
    case VX_TYPE_INT16:
        threshold->trueValue.S16 = VX_DEFAULT_THRESHOLD_TRUE_VALUE;
        threshold->falseValue.S16 = VX_DEFAULT_THRESHOLD_FALSE_VALUE;
        break;
    case VX_TYPE_INT32:
        threshold->trueValue.S32 = VX_DEFAULT_THRESHOLD_TRUE_VALUE;
        threshold->falseValue.S32 = VX_DEFAULT_THRESHOLD_FALSE_VALUE;
        break;
    case VX_TYPE_UINT32:
        threshold->trueValue.U32 = VX_DEFAULT_THRESHOLD_TRUE_VALUE;
        threshold->falseValue.U32 = VX_DEFAULT_THRESHOLD_FALSE_VALUE;
        break;
    default:
        break;
    }
    return threshold;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetThresholdAttribute(vx_threshold threshold, vx_enum attribute, const void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;

    gcmDUMP_API("$VX vxSetThresholdAttribute: threshold=%p, attribute=0x%x, ptr=%p, size=0x%lx", threshold, attribute, ptr, size);

    if (vxoReference_IsValidAndSpecific(&threshold->base, VX_TYPE_THRESHOLD))
    {
        switch (attribute)
        {
            case VX_THRESHOLD_THRESHOLD_VALUE:
                if (vxmIS_VALID_PARAMETERS(ptr, size, vx_int32, 0x3) &&
                    (threshold->thresholdType == VX_THRESHOLD_TYPE_BINARY))
                {
                    threshold->value.S32 = *(vx_int32 *)ptr;
                    vxoReference_IncrementReadCount(&threshold->base);
                }
                else if(vxmIS_VALID_PARAMETERS(ptr, size, vx_pixel_value_t, 0x3) &&
                        (threshold->thresholdType == VX_THRESHOLD_TYPE_BINARY))
                {
                    threshold->value = *(vx_pixel_value_t *)ptr;
                    vxoReference_IncrementReadCount(&threshold->base);
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_THRESHOLD_LOWER:
                if (vxmIS_VALID_PARAMETERS(ptr, size, vx_int32, 0x3) &&
                    (threshold->thresholdType == VX_THRESHOLD_TYPE_RANGE))
                {
                    threshold->lower.S32 = *(vx_int32 *)ptr;
                    vxoReference_IncrementReadCount(&threshold->base);
                }
                else if(vxmIS_VALID_PARAMETERS(ptr, size, vx_pixel_value_t, 0x3) &&
                        (threshold->thresholdType == VX_THRESHOLD_TYPE_RANGE))
                {
                    threshold->lower = *(vx_pixel_value_t *)ptr;
                    vxoReference_IncrementReadCount(&threshold->base);
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_THRESHOLD_UPPER:
                if (vxmIS_VALID_PARAMETERS(ptr, size, vx_int32, 0x3) &&
                    (threshold->thresholdType == VX_THRESHOLD_TYPE_RANGE))
                {
                    threshold->upper.S32 = *(vx_int32 *)ptr;
                    vxoReference_IncrementReadCount(&threshold->base);
                }
                else if(vxmIS_VALID_PARAMETERS(ptr, size, vx_pixel_value_t, 0x3) &&
                        (threshold->thresholdType == VX_THRESHOLD_TYPE_RANGE))
                {
                    threshold->upper = *(vx_pixel_value_t *)ptr;
                    vxoReference_IncrementReadCount(&threshold->base);
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_TRUE_VALUE:
                if (vxmIS_VALID_PARAMETERS(ptr, size, vx_int32, 0x3))
                {
                    threshold->trueValue.S32 = *(vx_int32 *)ptr;
                    vxoReference_IncrementReadCount(&threshold->base);
                }
                else if(vxmIS_VALID_PARAMETERS(ptr, size, vx_pixel_value_t, 0x3))
                {
                    threshold->trueValue = *(vx_pixel_value_t *)ptr;
                    vxoReference_IncrementReadCount(&threshold->base);
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_FALSE_VALUE:
                if (vxmIS_VALID_PARAMETERS(ptr, size, vx_int32, 0x3))
                {
                    threshold->falseValue.S32 = *(vx_int32 *)ptr;
                    vxoReference_IncrementReadCount(&threshold->base);
                }
                else if(vxmIS_VALID_PARAMETERS(ptr, size, vx_pixel_value_t, 0x3))
                {
                    threshold->falseValue = *(vx_pixel_value_t *)ptr;
                    vxoReference_IncrementReadCount(&threshold->base);
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                status = VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }
    else
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryThreshold(vx_threshold threshold, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = VX_SUCCESS;

    gcmDUMP_API("$VX vxQueryThreshold: threshold=%p, attribute=0x%x, ptr=%p, size=0x%lx", threshold, attribute, ptr, size);


    if (vxoReference_IsValidAndSpecific(&threshold->base, VX_TYPE_THRESHOLD))
    {
        switch (attribute)
        {
            case VX_THRESHOLD_THRESHOLD_VALUE:
                if (vxmIS_VALID_PARAMETERS(ptr, size, vx_int32, 0x3) &&
                    (threshold->thresholdType == VX_THRESHOLD_TYPE_BINARY))
                {
                    *(vx_int32 *)ptr = threshold->value.S32;
                    vxoReference_IncrementReadCount(&threshold->base);
                }
                else if(vxmIS_VALID_PARAMETERS(ptr, size, vx_pixel_value_t, 0x3) &&
                        (threshold->thresholdType == VX_THRESHOLD_TYPE_BINARY))
                {
                    *(vx_pixel_value_t *)ptr = threshold->value;
                    vxoReference_IncrementReadCount(&threshold->base);
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_THRESHOLD_LOWER:
                if (vxmIS_VALID_PARAMETERS(ptr, size, vx_int32, 0x3) &&
                    (threshold->thresholdType == VX_THRESHOLD_TYPE_RANGE))
                {
                    *(vx_int32 *)ptr = threshold->lower.S32;
                    vxoReference_IncrementReadCount(&threshold->base);
                }
                else if(vxmIS_VALID_PARAMETERS(ptr, size, vx_pixel_value_t, 0x3) &&
                        (threshold->thresholdType == VX_THRESHOLD_TYPE_RANGE))
                {
                    *(vx_pixel_value_t *)ptr = threshold->lower;
                    vxoReference_IncrementReadCount(&threshold->base);
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_THRESHOLD_UPPER:
                if (vxmIS_VALID_PARAMETERS(ptr, size, vx_int32, 0x3) &&
                    (threshold->thresholdType == VX_THRESHOLD_TYPE_RANGE))
                {
                    *(vx_int32 *)ptr = threshold->upper.S32;
                    vxoReference_IncrementReadCount(&threshold->base);
                }
                else if(vxmIS_VALID_PARAMETERS(ptr, size, vx_pixel_value_t, 0x3) &&
                        (threshold->thresholdType == VX_THRESHOLD_TYPE_RANGE))
                {
                    *(vx_pixel_value_t *)ptr = threshold->upper;
                    vxoReference_IncrementReadCount(&threshold->base);
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_TRUE_VALUE:
                if (vxmIS_VALID_PARAMETERS(ptr, size, vx_int32, 0x3))
                {
                    *(vx_int32 *)ptr = threshold->trueValue.S32;
                    vxoReference_IncrementReadCount(&threshold->base);
                }
                else if(vxmIS_VALID_PARAMETERS(ptr, size, vx_pixel_value_t, 0x3))
                {
                    *(vx_pixel_value_t *)ptr = threshold->trueValue;
                    vxoReference_IncrementReadCount(&threshold->base);
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_FALSE_VALUE:
                if (vxmIS_VALID_PARAMETERS(ptr, size, vx_int32, 0x3))
                {
                    *(vx_int32 *)ptr = threshold->falseValue.S32;
                    vxoReference_IncrementReadCount(&threshold->base);
                }
                else if(vxmIS_VALID_PARAMETERS(ptr, size, vx_pixel_value_t, 0x3))
                {
                    *(vx_pixel_value_t *)ptr = threshold->falseValue;
                    vxoReference_IncrementReadCount(&threshold->base);
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_DATA_TYPE:
                if (vxmIS_VALID_PARAMETERS(ptr, size, vx_enum, 0x3))
                {
                    *(vx_enum *)ptr = threshold->dataType;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_TYPE:
                if (vxmIS_VALID_PARAMETERS(ptr, size, vx_enum, 0x3))
                {
                    *(vx_enum *)ptr = threshold->thresholdType;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_INPUT_FORMAT:
                if (vxmIS_VALID_PARAMETERS(ptr, size, vx_df_image, 0x3))
                {
                    *(vx_df_image *)ptr = threshold->input_format;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case VX_THRESHOLD_OUTPUT_FORMAT:
                if (vxmIS_VALID_PARAMETERS(ptr, size, vx_df_image, 0x3))
                {
                     *(vx_df_image *)ptr = threshold->output_format;
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                status = VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }
    else
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

VX_API_ENTRY vx_threshold VX_API_CALL vxCreateThresholdForImage(
    vx_context context,
    vx_enum thresh_type,
    vx_df_image input_format,
    vx_df_image output_format)
{
//
    vx_threshold threshold;

    gcmDUMP_API("$VX vxCreateThresholdForImage: context=%p, thresh_type=0x%x, input_format=0x%x, output_format=0x%x",
        context, thresh_type, input_format, output_format);


    if (!vxoContext_IsValid(context)) return VX_NULL;

    if (!vxoThreshold_IsValidType(thresh_type))
    {
        return (vx_threshold)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_TYPE);
    }
    if (((vxoIsValidThresholdFormat  (input_format) == vx_false_e) &&
          (vxoIsValidThresholdFormatEx(input_format) == vx_false_e)) ||
         ((vxoIsValidThresholdFormat  (output_format) == vx_false_e) &&
          (vxoIsValidThresholdFormatEx(output_format) == vx_false_e)) )
    {
        vxAddLogEntry(&context->base, VX_ERROR_INVALID_TYPE, "Invalid input or output format\n");
        threshold = (vx_threshold)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_TYPE);
    }
    threshold = (vx_threshold)vxoReference_Create(context, VX_TYPE_THRESHOLD, VX_REF_EXTERNAL, &context->base);

    if (vxoReference_GetStatus((vx_reference)threshold) != VX_SUCCESS) return threshold;

    //threshold->thresholdType = thresh_type;
    //threshold->dataType      = data_type;

    if (vxGetStatus((vx_reference)threshold) == VX_SUCCESS && threshold->base.type == VX_TYPE_THRESHOLD)
    {
        threshold->thresholdType = thresh_type;
        threshold->input_format = input_format;
        threshold->output_format = output_format;
        switch (output_format)
        {
            case VX_DF_IMAGE_RGB:
            {
                threshold->dataType = VX_TYPE_DF_IMAGE;
                threshold->trueValue.RGB[0] = VX_DEFAULT_THRESHOLD_TRUE_VALUE;
                threshold->trueValue.RGB[1] = VX_DEFAULT_THRESHOLD_TRUE_VALUE;
                threshold->trueValue.RGB[2] = VX_DEFAULT_THRESHOLD_TRUE_VALUE;
                threshold->falseValue.RGB[0] = VX_DEFAULT_THRESHOLD_FALSE_VALUE;
                threshold->falseValue.RGB[1] = VX_DEFAULT_THRESHOLD_FALSE_VALUE;
                threshold->falseValue.RGB[2] = VX_DEFAULT_THRESHOLD_FALSE_VALUE;
                break;
            }
            case VX_DF_IMAGE_RGBX:
            {
                threshold->dataType = VX_TYPE_DF_IMAGE;
                threshold->trueValue.RGBX[0] = VX_DEFAULT_THRESHOLD_TRUE_VALUE;
                threshold->trueValue.RGBX[1] = VX_DEFAULT_THRESHOLD_TRUE_VALUE;
                threshold->trueValue.RGBX[2] = VX_DEFAULT_THRESHOLD_TRUE_VALUE;
                threshold->trueValue.RGBX[3] = VX_DEFAULT_THRESHOLD_TRUE_VALUE;
                threshold->falseValue.RGBX[0] = VX_DEFAULT_THRESHOLD_FALSE_VALUE;
                threshold->falseValue.RGBX[1] = VX_DEFAULT_THRESHOLD_FALSE_VALUE;
                threshold->falseValue.RGBX[2] = VX_DEFAULT_THRESHOLD_FALSE_VALUE;
                threshold->falseValue.RGBX[3] = VX_DEFAULT_THRESHOLD_FALSE_VALUE;
                break;
            }
            case VX_DF_IMAGE_NV12:
            case VX_DF_IMAGE_NV21:
            case VX_DF_IMAGE_UYVY:
            case VX_DF_IMAGE_YUYV:
            case VX_DF_IMAGE_IYUV:
            case VX_DF_IMAGE_YUV4:
            {
                threshold->dataType = VX_TYPE_DF_IMAGE;
                threshold->trueValue.YUV[0] = VX_DEFAULT_THRESHOLD_TRUE_VALUE;
                threshold->trueValue.YUV[1] = VX_DEFAULT_THRESHOLD_TRUE_VALUE;
                threshold->trueValue.YUV[2] = VX_DEFAULT_THRESHOLD_TRUE_VALUE;
                threshold->falseValue.YUV[0] = VX_DEFAULT_THRESHOLD_FALSE_VALUE;
                threshold->falseValue.YUV[1] = VX_DEFAULT_THRESHOLD_FALSE_VALUE;
                threshold->falseValue.YUV[2] = VX_DEFAULT_THRESHOLD_FALSE_VALUE;
                break;
            }
            case VX_DF_IMAGE_U8:
            {
                threshold->dataType = VX_TYPE_UINT8;
                threshold->trueValue.U8  = VX_DEFAULT_THRESHOLD_TRUE_VALUE;
                threshold->falseValue.U8 = VX_DEFAULT_THRESHOLD_FALSE_VALUE;
                break;
            }
            case VX_DF_IMAGE_S16:
            {
                threshold->dataType = VX_TYPE_INT16;
                threshold->trueValue.S16  = VX_S16_THRESHOLD_TRUE_VALUE;
                threshold->falseValue.S16 = VX_S16_THRESHOLD_FALSE_VALUE;
                break;
            }
            case VX_DF_IMAGE_U16:
            {
                threshold->dataType = VX_TYPE_UINT16;
                threshold->trueValue.U16  = VX_U16_THRESHOLD_TRUE_VALUE;
                threshold->falseValue.U16 = VX_U16_THRESHOLD_FALSE_VALUE;
                break;
            }
            case VX_DF_IMAGE_S32:
            {
                threshold->dataType = VX_TYPE_INT32;
                threshold->trueValue.S32  = VX_S32_THRESHOLD_TRUE_VALUE;
                threshold->falseValue.S32 = VX_S32_THRESHOLD_FALSE_VALUE;
                break;
            }
            case VX_DF_IMAGE_U32:
            {
                threshold->dataType = VX_TYPE_UINT32;
                threshold->trueValue.U32  = VX_U32_THRESHOLD_TRUE_VALUE;
                threshold->falseValue.U32 = VX_U32_THRESHOLD_FALSE_VALUE;
                break;
            }
            default:
            {
                threshold->dataType = VX_TYPE_INVALID;
                break;
            }
        }
    }
    return threshold;
}

VX_API_ENTRY vx_threshold VX_API_CALL vxCreateVirtualThresholdForImage(vx_graph graph,
                                                                       vx_enum thresh_type,
                                                                       vx_df_image input_format,
                                                                       vx_df_image output_format)
{
    vx_threshold threshold = NULL;
    vx_reference_s *gref = (vx_reference_s *)graph;

    gcmDUMP_API("$VX vxCreateVirtualThresholdForImage: graph=%p, thresh_type=0x%x, input_format=0x%x, output_format=0x%x",
        graph, thresh_type, input_format, output_format);


    if(vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH))
    {
        threshold = vxCreateThresholdForImage(gref->context, thresh_type, input_format, output_format);
        if (vxGetStatus((vx_reference)threshold) == VX_SUCCESS && threshold->base.type == VX_TYPE_THRESHOLD)
        {
            threshold->base.scope = (vx_reference_s *)graph;
            threshold->base.isVirtual = vx_true_e;
        }
        else
        {
            threshold = (vx_threshold)vxoContext_GetErrorObject(graph->base.context, VX_ERROR_INVALID_PARAMETERS);
        }
    }
    return threshold;
}

VX_API_ENTRY vx_status VX_API_CALL vxCopyThresholdOutput(vx_threshold threshold,
                                                         vx_pixel_value_t * true_value_ptr,
                                                         vx_pixel_value_t * false_value_ptr,
                                                         vx_enum usage,
                                                         vx_enum user_mem_type)
{
    vx_status status = VX_ERROR_INVALID_REFERENCE;

    gcmDUMP_API("$VX vxCopyThresholdOutput: threshold=%p, true_value_ptr=%p, false_value_ptr=%p, usage=0x%x"\
        " user_mem_type=0x%x", threshold, true_value_ptr, false_value_ptr, usage, user_mem_type);


    if (vxoReference_IsValidAndSpecific(&threshold->base, VX_TYPE_THRESHOLD) == vx_false_e)
    {
        status = VX_ERROR_INVALID_REFERENCE;
        return status;
    }

    if (threshold->base.isVirtual == vx_true_e)
    {
        if (threshold->base.accessible == vx_false_e)
        {
            status = VX_ERROR_OPTIMIZED_AWAY;
            return status;
        }
    }

    if (VX_MEMORY_TYPE_HOST == user_mem_type)
    {
        vx_size size;
        if (usage == VX_READ_ONLY)
        {
            vxAcquireMutex(threshold->base.lock);
            size = sizeof(vx_pixel_value_t);
            if (true_value_ptr)
            {
                memcpy(true_value_ptr, &threshold->trueValue, size);
            }
            if (false_value_ptr)
            {
                memcpy(false_value_ptr, &threshold->falseValue, size);
            }
            vxReleaseMutex(threshold->base.lock);
            vxoReference_IncrementReadCount(&threshold->base);
            status = VX_SUCCESS;
        }
        else if (usage == VX_WRITE_ONLY)
        {
            vxAcquireMutex(threshold->base.lock);
            size = sizeof(vx_pixel_value_t);
            if (true_value_ptr)
            {
                memcpy(&threshold->trueValue, true_value_ptr, size);
            }
            if (false_value_ptr)
            {
                memcpy(&threshold->falseValue, false_value_ptr, size);
            }
            vxReleaseMutex(threshold->base.lock);
            vxoReference_IncrementWriteCount(&threshold->base);
            status = VX_SUCCESS;
        }
        else
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }
    else
    {
        status = VX_ERROR_NO_MEMORY;
    }
    return status;
}


VX_API_ENTRY vx_status VX_API_CALL vxCopyThresholdRange(vx_threshold threshold,
                                                        vx_pixel_value_t * lower_value_ptr,
                                                        vx_pixel_value_t * upper_value_ptr,
                                                        vx_enum usage,
                                                        vx_enum user_mem_type)
{
    vx_status status = VX_ERROR_INVALID_REFERENCE;

    gcmDUMP_API("$VX vxCopyThresholdRange: threshold=%p, lower_value_ptr=%p, upper_value_ptr=%p, usage=0x%x,"\
        " user_mem_type=0x%x", threshold, lower_value_ptr, upper_value_ptr, usage, user_mem_type);


    if (vxoReference_IsValidAndSpecific(&threshold->base, VX_TYPE_THRESHOLD) == vx_false_e)
    {
        status = VX_ERROR_INVALID_REFERENCE;
        return status;
    }

    if (threshold->base.isVirtual == vx_true_e)
    {
        if (threshold->base.accessible == vx_false_e)
        {
            status = VX_ERROR_OPTIMIZED_AWAY;
            return status;
        }
    }

    if (VX_MEMORY_TYPE_HOST == user_mem_type)
    {
        vx_size size;
        if (usage == VX_READ_ONLY)
        {
            vxAcquireMutex(threshold->base.lock);
            size = sizeof(vx_pixel_value_t);
            if (lower_value_ptr)
            {
                memcpy(lower_value_ptr, &threshold->lower, size);
            }
            if (upper_value_ptr)
            {
                memcpy(upper_value_ptr, &threshold->upper, size);
            }
            vxReleaseMutex(threshold->base.lock);
            vxoReference_IncrementReadCount(&threshold->base);
            status = VX_SUCCESS;
        }
        else if (usage == VX_WRITE_ONLY)
        {
            vxAcquireMutex(threshold->base.lock);
            size = sizeof(vx_pixel_value_t);
            if (lower_value_ptr)
            {
                memcpy(&threshold->lower, lower_value_ptr, size);
            }
            if (upper_value_ptr)
            {
                memcpy(&threshold->upper, upper_value_ptr, size);
            }
            vxReleaseMutex(threshold->base.lock);
            vxoReference_IncrementWriteCount(&threshold->base);
            status = VX_SUCCESS;
        }
        else
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }
    else
    {
        status = VX_ERROR_NO_MEMORY;
    }

    return status;
}


VX_API_ENTRY vx_status VX_API_CALL vxCopyThresholdValue(vx_threshold threshold,
                                                        vx_pixel_value_t * value_ptr,
                                                        vx_enum usage,
                                                        vx_enum user_mem_type
                                                        )
{
    vx_status status = VX_ERROR_INVALID_REFERENCE;

    gcmDUMP_API("$VX vxCopyThresholdValue: threshold=%p, value_ptr=%p, usage=0x%x, user_mem_type=0x%x",
        threshold, value_ptr, usage, user_mem_type);


    if (vxoReference_IsValidAndSpecific(&threshold->base, VX_TYPE_THRESHOLD) == vx_false_e)
    {
        status = VX_ERROR_INVALID_REFERENCE;
        return status;
    }

    if (threshold->base.isVirtual == vx_true_e)
    {
        if (threshold->base.accessible == vx_false_e)
        {
            status = VX_ERROR_OPTIMIZED_AWAY;
            return status;
        }
    }

    if (VX_MEMORY_TYPE_HOST == user_mem_type)
    {
        vx_size size;
        if (usage == VX_READ_ONLY)
        {
            vxAcquireMutex(threshold->base.lock);
            size = sizeof(vx_pixel_value_t);
            if (value_ptr)
            {
                memcpy(value_ptr, &threshold->value, size);
            }
            vxReleaseMutex(threshold->base.lock);
            vxoReference_IncrementReadCount(&threshold->base);
            status = VX_SUCCESS;
        }
        else if (usage == VX_WRITE_ONLY)
        {
            vxAcquireMutex(threshold->base.lock);
            size = sizeof(vx_pixel_value_t);
            if (value_ptr)
            {
                memcpy(&threshold->value, value_ptr, size);
            }
            vxReleaseMutex(threshold->base.lock);
            vxoReference_IncrementWriteCount(&threshold->base);
            status = VX_SUCCESS;
        }
        else
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }
    else
    {
        status = VX_ERROR_NO_MEMORY;
    }

    return status;
}

