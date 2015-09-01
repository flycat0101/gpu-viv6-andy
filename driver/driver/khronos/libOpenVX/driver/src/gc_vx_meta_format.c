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

VX_INTERNAL_API vx_meta_format vxoMetaFormat_Create(vx_context context)
{
    vx_meta_format metaFormat;

    vxmASSERT(context);

    metaFormat = (vx_meta_format)vxoReference_Create(context, VX_TYPE_META_FORMAT, VX_REF_EXTERNAL, &context->base);

    if (vxoReference_GetStatus((vx_reference)metaFormat) != VX_SUCCESS) return metaFormat;

    metaFormat->size = sizeof(vx_meta_format_s);
    metaFormat->type = VX_TYPE_INVALID;

    return metaFormat;
}

VX_INTERNAL_API void vxoMetaFormat_Release(vx_meta_format_ptr metaFormatPtr)
{
    vxoReference_Release((vx_reference_ptr)metaFormatPtr, VX_TYPE_META_FORMAT, VX_REF_EXTERNAL);
}

VX_PUBLIC_API vx_status vxSetMetaFormatAttribute(vx_meta_format meta_format, vx_enum attribute, void *ptr, vx_size size)
{
    if (!vxoReference_IsValidAndSpecific(&meta_format->base, VX_TYPE_META_FORMAT)) return VX_ERROR_INVALID_REFERENCE;

    if (VX_TYPE(attribute) != (vx_uint32)meta_format->type) return VX_ERROR_INVALID_TYPE;

    switch (attribute)
    {
        case VX_META_FORMAT_ATTRIBUTE_DELTA_RECTANGLE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_delta_rectangle_t, 0x3);

            meta_format->u.imageInfo.delta = *(vx_delta_rectangle_t *)ptr;
            break;

        case VX_IMAGE_ATTRIBUTE_FORMAT:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_df_image, 0x3);

            meta_format->u.imageInfo.format = *(vx_df_image *)ptr;
            break;

        case VX_IMAGE_ATTRIBUTE_HEIGHT:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            meta_format->u.imageInfo.height = *(vx_uint32 *)ptr;
            break;

        case VX_IMAGE_ATTRIBUTE_WIDTH:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            meta_format->u.imageInfo.width = *(vx_uint32 *)ptr;
            break;

        case VX_ARRAY_ATTRIBUTE_CAPACITY:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            meta_format->u.arrayInfo.capacity = *(vx_size *)ptr;
            break;

        case VX_ARRAY_ATTRIBUTE_ITEMTYPE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);

            meta_format->u.arrayInfo.itemType = *(vx_enum *)ptr;
            break;

        case VX_PYRAMID_ATTRIBUTE_FORMAT:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_df_image, 0x3);

            meta_format->u.pyramidInfo.format = *(vx_df_image *)ptr;
            break;

        case VX_PYRAMID_ATTRIBUTE_HEIGHT:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            meta_format->u.pyramidInfo.height = *(vx_uint32 *)ptr;
            break;

        case VX_PYRAMID_ATTRIBUTE_WIDTH:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            meta_format->u.pyramidInfo.width = *(vx_uint32 *)ptr;
            break;

        case VX_PYRAMID_ATTRIBUTE_LEVELS:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            meta_format->u.pyramidInfo.levelCount = *(vx_size *)ptr;
            break;

        case VX_PYRAMID_ATTRIBUTE_SCALE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_float32, 0x3);

            meta_format->u.pyramidInfo.scale = *(vx_float32 *)ptr;
            break;

        case VX_SCALAR_ATTRIBUTE_TYPE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);

            meta_format->u.scalarInfo.type = *(vx_enum *)ptr;
            break;

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            return VX_ERROR_NOT_SUPPORTED;
    }

    return VX_SUCCESS;
}
