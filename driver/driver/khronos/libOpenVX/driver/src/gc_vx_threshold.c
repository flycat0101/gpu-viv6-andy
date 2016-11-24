/****************************************************************************
*
*    Copyright (c) 2005 - 2016 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include <gc_vx_common.h>

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
    return (vx_bool)(dataType == VX_TYPE_UINT8);
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseThreshold(vx_threshold *threshold)
{
    return vxoReference_Release((vx_reference_ptr)threshold, VX_TYPE_THRESHOLD, VX_REF_EXTERNAL);
}

VX_API_ENTRY vx_threshold VX_API_CALL vxCreateThreshold(vx_context context, vx_enum thresh_type, vx_enum data_type)
{
    vx_threshold threshold;

    if (!vxoContext_IsValid(context)) return VX_NULL;

    if (!vxoThreshold_IsValidDataType(data_type) || !vxoThreshold_IsValidType(thresh_type))
    {
        return (vx_threshold)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_TYPE);
    }

    threshold = (vx_threshold)vxoReference_Create(context, VX_TYPE_THRESHOLD, VX_REF_EXTERNAL, &context->base);

    if (vxoReference_GetStatus((vx_reference)threshold) != VX_SUCCESS) return threshold;

    threshold->type = thresh_type;

    return threshold;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetThresholdAttribute(vx_threshold threshold, vx_enum attribute, const void *ptr, vx_size size)
{
    vx_enum type;

    if (!vxoReference_IsValidAndSpecific(&threshold->base, VX_TYPE_THRESHOLD)) return VX_ERROR_INVALID_REFERENCE;

    switch (attribute)
    {
        case VX_THRESHOLD_ATTRIBUTE_THRESHOLD_VALUE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_int32, 0x3);

            if (threshold->type != VX_THRESHOLD_TYPE_BINARY) return VX_ERROR_INVALID_PARAMETERS;

            threshold->value = *(vx_int32 *)ptr;

            vxoReference_IncrementWriteCount(&threshold->base);
            break;

        case VX_THRESHOLD_ATTRIBUTE_THRESHOLD_LOWER:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_int32, 0x3);

            if (threshold->type != VX_THRESHOLD_TYPE_RANGE) return VX_ERROR_INVALID_PARAMETERS;

            threshold->lower = *(vx_int32 *)ptr;

            vxoReference_IncrementWriteCount(&threshold->base);
            break;

        case VX_THRESHOLD_ATTRIBUTE_THRESHOLD_UPPER:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_int32, 0x3);

            if (threshold->type != VX_THRESHOLD_TYPE_RANGE) return VX_ERROR_INVALID_PARAMETERS;

            threshold->upper = *(vx_int32 *)ptr;

            vxoReference_IncrementWriteCount(&threshold->base);
            break;

        case VX_THRESHOLD_ATTRIBUTE_TRUE_VALUE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_int32, 0x3);

            threshold->trueValue = *(vx_int32 *)ptr;

            vxoReference_IncrementWriteCount(&threshold->base);
            break;

        case VX_THRESHOLD_ATTRIBUTE_FALSE_VALUE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_int32, 0x3);

            threshold->falseValue = *(vx_int32 *)ptr;

            vxoReference_IncrementWriteCount(&threshold->base);
            break;

        case VX_THRESHOLD_ATTRIBUTE_TYPE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);

            type = *(vx_enum *)ptr;

            if (!vxoThreshold_IsValidType(type)) return VX_ERROR_INVALID_PARAMETERS;

            threshold->type = type;
            break;

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            return VX_ERROR_NOT_SUPPORTED;
    }

    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryThreshold(vx_threshold threshold, vx_enum attribute, void *ptr, vx_size size)
{
    if (!vxoReference_IsValidAndSpecific(&threshold->base, VX_TYPE_THRESHOLD)) return VX_ERROR_INVALID_REFERENCE;

    switch (attribute)
    {
        case VX_THRESHOLD_ATTRIBUTE_THRESHOLD_VALUE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_int32, 0x3);

            if (threshold->type != VX_THRESHOLD_TYPE_BINARY) return VX_ERROR_INVALID_PARAMETERS;

            *(vx_int32 *)ptr = threshold->value;

            vxoReference_IncrementReadCount(&threshold->base);
            break;

        case VX_THRESHOLD_ATTRIBUTE_THRESHOLD_LOWER:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_int32, 0x3);

            if (threshold->type != VX_THRESHOLD_TYPE_RANGE) return VX_ERROR_INVALID_PARAMETERS;

            *(vx_int32 *)ptr = threshold->lower;

            vxoReference_IncrementReadCount(&threshold->base);
            break;

        case VX_THRESHOLD_ATTRIBUTE_THRESHOLD_UPPER:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_int32, 0x3);

            if (threshold->type != VX_THRESHOLD_TYPE_RANGE) return VX_ERROR_INVALID_PARAMETERS;

            *(vx_int32 *)ptr = threshold->upper;

            vxoReference_IncrementReadCount(&threshold->base);
            break;

        case VX_THRESHOLD_ATTRIBUTE_TRUE_VALUE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_int32, 0x3);

            *(vx_int32 *)ptr = threshold->trueValue;

            vxoReference_IncrementReadCount(&threshold->base);
            break;

        case VX_THRESHOLD_ATTRIBUTE_FALSE_VALUE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_int32, 0x3);

            *(vx_int32 *)ptr = threshold->falseValue;

            vxoReference_IncrementReadCount(&threshold->base);
            break;

        case VX_THRESHOLD_ATTRIBUTE_TYPE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);

            *(vx_enum *)ptr = threshold->type;
            break;

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            return VX_ERROR_NOT_SUPPORTED;
    }

    return VX_SUCCESS;
}

