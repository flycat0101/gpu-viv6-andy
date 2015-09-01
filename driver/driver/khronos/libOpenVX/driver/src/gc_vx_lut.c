/****************************************************************************
*
*    Copyright (c) 2005 - 2015 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include <gc_vx_common.h>

VX_INTERNAL_CALLBACK_API void vxoLUT_Destructor(vx_reference ref)
{
    vxoArray_Destructor(ref);
}

VX_PUBLIC_API vx_lut vxCreateLUT(vx_context context, vx_enum data_type, vx_size count)
{
    vx_array lut;

    if (!vxoContext_IsValid(context)) return VX_NULL;

    switch (data_type)
    {
        case VX_TYPE_UINT8:
        case VX_TYPE_UINT16:
            break;

        default:
            return (vx_lut)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_TYPE);
    }

    lut = vxoArray_Create(context, data_type, count, vx_false_e, VX_TYPE_LUT);

    if (vxoReference_GetStatus((vx_reference)lut) != VX_SUCCESS) return (vx_lut)lut;

    lut->itemCount = count;

    return (vx_lut)lut;
}

VX_PUBLIC_API vx_status vxReleaseLUT(vx_lut *lut)
{
    return vxoReference_Release((vx_reference_ptr)lut, VX_TYPE_LUT, VX_REF_EXTERNAL);
}

VX_PUBLIC_API vx_status vxQueryLUT(vx_lut lut, vx_enum attribute, void *ptr, vx_size size)
{
    vx_array lutArray = (vx_array)lut;

    if (!vxoReference_IsValidAndSpecific(&lutArray->base, VX_TYPE_LUT)) return VX_ERROR_INVALID_REFERENCE;

    switch (attribute)
    {
        case VX_LUT_ATTRIBUTE_TYPE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);

            *(vx_enum *)ptr = lutArray->itemType;
            break;

        case VX_LUT_ATTRIBUTE_COUNT:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size *)ptr = lutArray->itemCount;
            break;

        case VX_LUT_ATTRIBUTE_SIZE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size *)ptr = lutArray->itemCount * lutArray->itemSize;
            break;

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            return VX_ERROR_NOT_SUPPORTED;
    }

    return VX_SUCCESS;
}

VX_PUBLIC_API vx_status vxAccessLUT(vx_lut lut, void **ptr, vx_enum usage)
{
    vx_array lutArray = (vx_array)lut;

    if (!vxoReference_IsValidAndSpecific(&lutArray->base, VX_TYPE_LUT)) return VX_ERROR_INVALID_REFERENCE;

    return vxoArray_AccessRange(lutArray, 0, lutArray->itemCount, ptr, usage);
}

VX_PUBLIC_API vx_status vxCommitLUT(vx_lut lut, void *ptr)
{
    vx_array lutArray = (vx_array)lut;

    if (!vxoReference_IsValidAndSpecific(&lutArray->base, VX_TYPE_LUT)) return VX_ERROR_INVALID_REFERENCE;

    return vxoArray_CommitRange(lutArray, 0, lutArray->itemCount, ptr);
}

