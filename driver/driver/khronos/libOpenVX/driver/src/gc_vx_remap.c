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


#include <gc_vx_common.h>

VX_INTERNAL_CALLBACK_API void vxoRemap_Destructor(vx_reference ref)
{
    vx_remap remap = (vx_remap)ref;

    vxoMemory_Free(remap->base.context, &remap->memory);
}

VX_API_ENTRY vx_remap VX_API_CALL vxCreateRemap(
        vx_context context, vx_uint32 src_width, vx_uint32 src_height, vx_uint32 dst_width, vx_uint32 dst_height)
{
    vx_remap remap;

    if (!vxoContext_IsValid(context)) return VX_NULL;

    if (src_width == 0 || src_height == 0 || dst_width == 0 || dst_height == 0)
    {
        return (vx_remap)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
    }

    remap = (vx_remap)vxoReference_Create(context, VX_TYPE_REMAP, VX_REF_EXTERNAL, &context->base);

    if (vxoReference_GetStatus((vx_reference)remap) != VX_SUCCESS) return remap;

    remap->srcWidth     = src_width;
    remap->srcHeight    = src_height;
    remap->destWidth    = dst_width;
    remap->destHeight   = dst_height;

    remap->memory.planeCount                    = 1;
    remap->memory.dimCount                      = 3;
    remap->memory.dims[0][VX_DIM_CHANNEL]       = 2;
    remap->memory.dims[0][VX_DIM_X]             = dst_width;
    remap->memory.dims[0][VX_DIM_Y]             = dst_height;
    remap->memory.strides[0][VX_DIM_CHANNEL]    = sizeof(vx_float32);

    return remap;
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseRemap(vx_remap *remap)
{
    return vxoReference_Release((vx_reference_ptr)remap, VX_TYPE_REMAP, VX_REF_EXTERNAL);
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryRemap(vx_remap remap, vx_enum attribute, void *ptr, vx_size size)
{
    if (!vxoReference_IsValidAndSpecific(&remap->base, VX_TYPE_REMAP)) return VX_ERROR_INVALID_REFERENCE;

    switch (attribute)
    {
        case VX_REMAP_SOURCE_WIDTH:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            *(vx_uint32 *)ptr = remap->srcWidth;
            break;

        case VX_REMAP_SOURCE_HEIGHT:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            *(vx_uint32 *)ptr = remap->srcHeight;
            break;

        case VX_REMAP_DESTINATION_WIDTH:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            *(vx_uint32 *)ptr = remap->destWidth;
            break;

        case VX_REMAP_DESTINATION_HEIGHT:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            *(vx_uint32 *)ptr = remap->destHeight;
            break;

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            return VX_ERROR_NOT_SUPPORTED;
    }

    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetRemapPoint(
        vx_remap remap, vx_uint32 dst_x, vx_uint32 dst_y, vx_float32 src_x, vx_float32 src_y)
{
    vx_float32_ptr ptr;

    if (!vxoReference_IsValidAndSpecific(&remap->base, VX_TYPE_REMAP)) return VX_ERROR_INVALID_REFERENCE;

    if (dst_x >= remap->destWidth || dst_y >= remap->destHeight) return VX_ERROR_INVALID_VALUE;

    if (!vxoMemory_Allocate(remap->base.context, &remap->memory)) return VX_ERROR_NO_MEMORY;

    ptr = (vx_float32_ptr)vxFormatMemoryPtr(&remap->memory, 0, dst_x, dst_y, 0);
    *ptr = src_x;

    ptr = (vx_float32_ptr)vxFormatMemoryPtr(&remap->memory, 1, dst_x, dst_y, 0);
    *ptr = src_y;

    vxoReference_IncrementWriteCount(&remap->base);

    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxGetRemapPoint(
        vx_remap remap, vx_uint32 dst_x, vx_uint32 dst_y, vx_float32 *src_x, vx_float32 *src_y)
{
    vx_float32_ptr ptr;

    if (!vxoReference_IsValidAndSpecific(&remap->base, VX_TYPE_REMAP)) return VX_ERROR_INVALID_REFERENCE;

    if (dst_x >= remap->destWidth || dst_y >= remap->destHeight) return VX_ERROR_INVALID_VALUE;

    ptr = (vx_float32_ptr)vxFormatMemoryPtr(&remap->memory, 0, dst_x, dst_y, 0);
    *src_x = *ptr;

    ptr = (vx_float32_ptr)vxFormatMemoryPtr(&remap->memory, 1, dst_x, dst_y, 0);
    *src_y = *ptr;

    vxoReference_IncrementReadCount(&remap->base);

    return VX_SUCCESS;
}

