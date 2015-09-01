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

const vx_float32 vxOrbScales[4] = {
    0.5f,
    VX_SCALE_PYRAMID_ORB,
    VX_SCALE_PYRAMID_ORB * VX_SCALE_PYRAMID_ORB,
    VX_SCALE_PYRAMID_ORB * VX_SCALE_PYRAMID_ORB * VX_SCALE_PYRAMID_ORB
};

VX_INTERNAL_API vx_status vxoPyramid_Initialize(
        vx_pyramid pyramid, vx_size levelCount, vx_float32 scale,
        vx_uint32 width, vx_uint32 height, vx_df_image format)
{
    vxmASSERT(pyramid);

    if (pyramid->levels == VX_NULL)
    {
        pyramid->levelCount = (vx_uint32)levelCount;
        pyramid->scale      = scale;
        pyramid->levels     = (vx_image_ptr)vxAllocateAndZeroMemory(levelCount * sizeof(vx_image));

        if (pyramid->levels == VX_NULL) return VX_ERROR_NO_MEMORY;
    }

    pyramid->width  = width;
    pyramid->height = height;
    pyramid->format = format;

    if (pyramid->width > 0 && pyramid->height > 0 && format != VX_DF_IMAGE_VIRT)
    {
        vx_uint32 levelIndex;
        vx_uint32 width     = pyramid->width;
        vx_uint32 height    = pyramid->height;
        vx_uint32 refWidth  = pyramid->width;
        vx_uint32 refHeight = pyramid->height;

        for (levelIndex = 0; levelIndex < pyramid->levelCount; levelIndex++)
        {
            if (pyramid->levels[levelIndex] == VX_NULL)
            {
                vx_status   status;
                vx_image    image = vxCreateImage(pyramid->base.context, width, height, format);

                status = vxoReference_GetStatus((vx_reference)image);

                if (status != VX_SUCCESS) return status;

                pyramid->levels[levelIndex] = image;

                vxoReference_Increment((vx_reference)pyramid->levels[levelIndex], VX_REF_INTERNAL);

                vxoReference_Decrement((vx_reference)pyramid->levels[levelIndex], VX_REF_EXTERNAL);

                pyramid->levels[levelIndex]->base.scope = (vx_reference)pyramid;

                if (scale == VX_SCALE_PYRAMID_ORB)
                {
                    vx_float32 orbScale = vxOrbScales[(levelIndex + 1) % 4];

                    width   = (vx_uint32)ceilf((vx_float32)refWidth * orbScale);
                    height  = (vx_uint32)ceilf((vx_float32)refHeight * orbScale);

                    if (((levelIndex + 1) % 4) == 0)
                    {
                        refWidth = width;
                        refHeight = height;
                    }
                }
                else
                {
                    width   = (vx_uint32)ceilf((vx_float32)width * scale);
                    height  = (vx_uint32)ceilf((vx_float32)height * scale);
                }
            }
        }
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_pyramid vxoPyramid_Create(
        vx_context context, vx_size levels, vx_float32 scale,
        vx_uint32 width, vx_uint32 height, vx_df_image format, vx_bool isVirtual)
{
    vx_pyramid  pyramid;
    vx_status   status;

    if (!vxoContext_IsValid(context)) return VX_NULL;

    if (scale != VX_SCALE_PYRAMID_HALF && scale != VX_SCALE_PYRAMID_ORB)
    {
        return (vx_pyramid)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
    }

    if (levels == 0 || levels > 8)
    {
        return (vx_pyramid)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
    }

    pyramid = (vx_pyramid)vxoReference_Create(context, VX_TYPE_PYRAMID, VX_REF_EXTERNAL, &context->base);

    if (vxoReference_GetStatus((vx_reference)pyramid) != VX_SUCCESS) return pyramid;

    pyramid->base.isVirtual = isVirtual;

    status = vxoPyramid_Initialize(pyramid, levels, scale, width, height, format);

    if (status != VX_SUCCESS)
    {
        vxReleasePyramid(&pyramid);

        return (vx_pyramid)vxoContext_GetErrorObject(context, status);
    }

    return pyramid;
}

VX_INTERNAL_CALLBACK_API void vxoPyramid_Destructor(vx_reference ref)
{
    vx_pyramid  pyramid = (vx_pyramid)ref;
    vx_uint32   levelIndex;

    for (levelIndex = 0; levelIndex < pyramid->levelCount; levelIndex++)
    {
        vxoReference_Release((vx_reference_ptr)&pyramid->levels[levelIndex], VX_TYPE_IMAGE, VX_REF_INTERNAL);
    }

    vxFree(pyramid->levels);
}

VX_PUBLIC_API vx_status vxReleasePyramid(vx_pyramid *pyramid)
{
    return vxoReference_Release((vx_reference_ptr)pyramid, VX_TYPE_PYRAMID, VX_REF_EXTERNAL);
}

VX_PUBLIC_API vx_pyramid vxCreateVirtualPyramid(
        vx_graph graph, vx_size levels, vx_float32 scale, vx_uint32 width, vx_uint32 height, vx_df_image format)
{
    vx_pyramid pyramid;

    if (!vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH)) return VX_NULL;

    pyramid = vxoPyramid_Create(graph->base.context, levels, scale, width, height, format, vx_true_e);

    if (vxoReference_GetStatus((vx_reference)pyramid) != VX_SUCCESS) return pyramid;

    pyramid->base.scope = (vx_reference)graph;

    return pyramid;
}

VX_PUBLIC_API vx_pyramid vxCreatePyramid(
        vx_context context, vx_size levels, vx_float32 scale, vx_uint32 width, vx_uint32 height, vx_df_image format)
{
    if (!vxoContext_IsValid(context)) return VX_NULL;

    if (width == 0 || height == 0 || format == VX_DF_IMAGE_VIRT)
    {
        return (vx_pyramid)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
    }

    return vxoPyramid_Create(context, levels, scale, width, height, format, vx_false_e);
}

VX_PUBLIC_API vx_status vxQueryPyramid(vx_pyramid pyramid, vx_enum attribute, void *ptr, vx_size size)
{
    if (!vxoReference_IsValidAndSpecific(&pyramid->base, VX_TYPE_PYRAMID)) return VX_ERROR_INVALID_REFERENCE;

    switch (attribute)
    {
        case VX_PYRAMID_ATTRIBUTE_LEVELS:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size *)ptr = pyramid->levelCount;
            break;

        case VX_PYRAMID_ATTRIBUTE_SCALE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_float32, 0x3);

            *(vx_float32 *)ptr = pyramid->scale;
            break;

        case VX_PYRAMID_ATTRIBUTE_WIDTH:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            *(vx_uint32 *)ptr = pyramid->width;
            break;

        case VX_PYRAMID_ATTRIBUTE_HEIGHT:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            *(vx_uint32 *)ptr = pyramid->height;
            break;

        case VX_PYRAMID_ATTRIBUTE_FORMAT:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_df_image, 0x3);

            *(vx_df_image *)ptr = pyramid->format;
            break;

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            return VX_ERROR_NOT_SUPPORTED;
    }

    return VX_SUCCESS;
}

VX_PUBLIC_API vx_image vxGetPyramidLevel(vx_pyramid pyramid, vx_uint32 index)
{
    vx_image image;

    if (!vxoReference_IsValidAndSpecific(&pyramid->base, VX_TYPE_PYRAMID)) return VX_NULL;

    if (index >= pyramid->levelCount)
    {
        return (vx_image)vxoContext_GetErrorObject(pyramid->base.context, VX_ERROR_INVALID_PARAMETERS);
    }

    image = pyramid->levels[index];

    vxoReference_Increment(&image->base, VX_REF_EXTERNAL);

    return image;
}
