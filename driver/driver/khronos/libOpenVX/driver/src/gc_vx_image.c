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

VX_INTERNAL_API vx_bool vxImageFormat_IsSupported(vx_df_image imageFormat)
{
    switch (imageFormat)
    {
        case VX_DF_IMAGE_RGB:
        case VX_DF_IMAGE_RGBX:
        case VX_DF_IMAGE_NV12:
        case VX_DF_IMAGE_NV21:
        case VX_DF_IMAGE_UYVY:
        case VX_DF_IMAGE_YUYV:
        case VX_DF_IMAGE_IYUV:
        case VX_DF_IMAGE_YUV4:
        case VX_DF_IMAGE_U8:
        case VX_DF_IMAGE_U16:
        case VX_DF_IMAGE_S16:
        case VX_DF_IMAGE_U32:
        case VX_DF_IMAGE_S32:
        case VX_DF_IMAGE_F32:
        case VX_DF_IMAGE_VIRT:
            return vx_true_e;

        default:
            return vx_false_e;
    }
}

VX_INTERNAL_API vx_size vxImageFormat_GetChannelSize(vx_df_image imageFormat)
{
    if (!vxImageFormat_IsSupported(imageFormat)) return 0;

    switch (imageFormat)
    {
        case VX_DF_IMAGE_S16:
        case VX_DF_IMAGE_U16:
            return sizeof(vx_uint16);

        case VX_DF_IMAGE_U32:
        case VX_DF_IMAGE_S32:
        case VX_DF_IMAGE_F32:
            return sizeof(vx_uint32);

        default:
            return 1;
    }
}

VX_PRIVATE_API vx_const_string vxImageFormat_ToString(vx_df_image imageFormat)
{
    static vx_char buffer[5];

    vxStrCopySafe(buffer, 4, (vx_const_string)&imageFormat);

    buffer[4] = '\0';

    return buffer;
}

VX_PRIVATE_API vx_bool vxImageFormat_IsValidWidthAndHeight(vx_uint32 width, vx_uint32 height, vx_df_image imageFormat)
{
    if ((imageFormat == VX_DF_IMAGE_UYVY || imageFormat == VX_DF_IMAGE_YUYV)
        && vxIsOdd(width))
    {
        return vx_false_e;
    }

    if ((imageFormat == VX_DF_IMAGE_IYUV || imageFormat == VX_DF_IMAGE_NV12 || imageFormat == VX_DF_IMAGE_NV21)
        && (vxIsOdd(width) || vxIsOdd(height)))
    {
        return vx_false_e;
    }

    return vx_true_e;
}

VX_PRIVATE_API vx_uint32 vxComputePatchOffset(vx_uint32 x, vx_uint32 y, const vx_imagepatch_addressing_t *patchAddr)
{
    vxmASSERT(patchAddr);

    return patchAddr->stride_x * ((patchAddr->scale_x * x) / VX_SCALE_UNITY)
            + patchAddr->stride_y * ((patchAddr->scale_y * y) / VX_SCALE_UNITY);
}

VX_PRIVATE_API vx_uint32 vxComputePlaneOffset(vx_image image, vx_uint32 x, vx_uint32 y, vx_uint32 planeIndex)
{
    vxmASSERT(image);
    vxmASSERT(planeIndex < image->memory.planeCount);

    return  x * image->memory.strides[planeIndex][VX_DIM_X] / image->scales[planeIndex][VX_DIM_X]
            + y * image->memory.strides[planeIndex][VX_DIM_Y] / image->scales[planeIndex][VX_DIM_Y];
}

VX_PRIVATE_API vx_uint32 vxComputePatchRangeSize(vx_uint32 patchRange, vx_imagepatch_addressing_t *patchAddr)
{
    vxmASSERT(patchAddr);

    return (patchRange * patchAddr->stride_x * patchAddr->scale_x) / VX_SCALE_UNITY;
}

VX_PRIVATE_API vx_uint32 vxComputePlaneRangeSize(vx_image image, vx_uint32 planeRange, vx_uint32 planeIndex)
{
    vxmASSERT(image);
    vxmASSERT(planeIndex < image->memory.planeCount);

    return (planeRange * image->memory.strides[planeIndex][VX_DIM_X]) / image->scales[planeIndex][VX_DIM_X];
}

VX_INTERNAL_API vx_image vxoImage_LocateROI(vx_image image, OUT vx_rectangle_t *rect)
{
    vxmASSERT(image);
    vxmASSERT(rect);

    rect->start_x   = 0;
    rect->start_y   = 0;
    rect->end_x     = image->width;
    rect->end_y     = image->height;

    while (image->parent != VX_NULL && image->parent != image)
    {
        vx_size plane_offset    = image->memory.logicals[0] - image->parent->memory.logicals[0];
        vx_uint32 dy            = (vx_uint32)(plane_offset * image->scales[0][VX_DIM_Y] / image->memory.strides[0][VX_DIM_Y]);
        vx_uint32 dx            = (vx_uint32)((plane_offset - (dy * image->memory.strides[0][VX_DIM_Y] / image->scales[0][VX_DIM_Y]))
                                    * image->scales[0][VX_DIM_X] / image->memory.strides[0][VX_DIM_X]);

        rect->start_x   += dx;
        rect->end_x     += dx;
        rect->start_y   += dy;
        rect->end_y     += dy;

        image = image->parent;
    }

    return image;
}

VX_INTERNAL_API vx_bool vxoImage_IsValid(vx_image image)
{
    if (!vxoReference_IsValidAndSpecific(&image->base, VX_TYPE_IMAGE)) return vx_false_e;

    if (!vxImageFormat_IsSupported(image->format)) return vx_false_e;

    return vx_true_e;
}

VX_PRIVATE_API void vxoImage_InitializePlane(
        vx_image image, vx_uint32 planeIndex, vx_uint32 channelSize,
        vx_uint32 channels, vx_uint32 width, vx_uint32 height)
{
    vxmASSERT(image);

    image->memory.strides[planeIndex][VX_DIM_CHANNEL]           = channelSize;

    image->memory.dimCount                                      = VX_MAX_DIMS;
    image->memory.dims[planeIndex][VX_DIM_CHANNEL]              = channels;
    image->memory.dims[planeIndex][VX_DIM_X]                    = width;
    image->memory.dims[planeIndex][VX_DIM_Y]                    = height;

    image->scales[planeIndex][VX_DIM_CHANNEL]                   = 1;
    image->scales[planeIndex][VX_DIM_X]                         = 1;
    image->scales[planeIndex][VX_DIM_Y]                         = 1;

    image->bounds[planeIndex][VX_DIM_CHANNEL][VX_BOUND_START]   = 0;
    image->bounds[planeIndex][VX_DIM_CHANNEL][VX_BOUND_END]     = channels;
    image->bounds[planeIndex][VX_DIM_X][VX_BOUND_START]         = 0;
    image->bounds[planeIndex][VX_DIM_X][VX_BOUND_END]           = width;
    image->bounds[planeIndex][VX_DIM_Y][VX_BOUND_START]         = 0;
    image->bounds[planeIndex][VX_DIM_Y][VX_BOUND_END]           = height;
}

VX_INTERNAL_API void vxoImage_Initialize(
        vx_image image, vx_uint32 width, vx_uint32 height, vx_df_image imageFormat)
{
    vx_uint32 channelSize = (vx_uint32)vxImageFormat_GetChannelSize(imageFormat);

    vxmASSERT(image);

    image->width                = width;
    image->height               = height;
    image->format               = imageFormat;

    image->channelRange         = VX_CHANNEL_RANGE_FULL;

    image->region.start_x       = width;
    image->region.start_y       = height;
    image->region.end_x         = 0;
    image->region.end_y         = 0;

    switch (image->format)
    {
        case VX_DF_IMAGE_U8:
        case VX_DF_IMAGE_U16:
        case VX_DF_IMAGE_U32:
        case VX_DF_IMAGE_S16:
        case VX_DF_IMAGE_S32:
        case VX_DF_IMAGE_F32:
            image->colorSpace   = VX_COLOR_SPACE_NONE;
            break;

        default:
            image->colorSpace   = VX_COLOR_SPACE_DEFAULT;
            break;
    }

    switch (image->format)
    {
        case VX_DF_IMAGE_VIRT:
            break;

        case VX_DF_IMAGE_NV12:
        case VX_DF_IMAGE_NV21:
            image->planeCount                           = 2;
            vxoImage_InitializePlane(image, 0, channelSize, 1, image->width, image->height);
            vxoImage_InitializePlane(image, 1, channelSize, 2, image->width / 2, image->height / 2);

            image->scales[1][VX_DIM_X]                  = 2;
            image->scales[1][VX_DIM_Y]                  = 2;
            image->bounds[1][VX_DIM_X][VX_BOUND_END]    *= image->scales[1][VX_DIM_X];
            image->bounds[1][VX_DIM_Y][VX_BOUND_END]    *= image->scales[1][VX_DIM_Y];
            break;

        case VX_DF_IMAGE_RGB:
            image->planeCount                           = 1;
            vxoImage_InitializePlane(image, 0, channelSize, 3, image->width, image->height);
            break;

        case VX_DF_IMAGE_RGBX:
            image->planeCount                           = 1;
            vxoImage_InitializePlane(image, 0, channelSize, 4, image->width, image->height);
            break;

        case VX_DF_IMAGE_UYVY:
        case VX_DF_IMAGE_YUYV:
            image->planeCount                           = 1;
            vxoImage_InitializePlane(image, 0, channelSize, 2, image->width, image->height);
            break;

        case VX_DF_IMAGE_YUV4:
            image->planeCount                           = 3;
            vxoImage_InitializePlane(image, 0, channelSize, 1, image->width, image->height);
            vxoImage_InitializePlane(image, 1, channelSize, 1, image->width, image->height);
            vxoImage_InitializePlane(image, 2, channelSize, 1, image->width, image->height);
            break;

        case VX_DF_IMAGE_IYUV:
            image->planeCount                           = 3;
            vxoImage_InitializePlane(image, 0, channelSize, 1, image->width, image->height);
            vxoImage_InitializePlane(image, 1, channelSize, 1, image->width/2, image->height/2);
            vxoImage_InitializePlane(image, 2, channelSize, 1, image->width/2, image->height/2);
            image->scales[1][VX_DIM_X]                  = 2;
            image->scales[1][VX_DIM_Y]                  = 2;
            image->scales[2][VX_DIM_X]                  = 2;
            image->scales[2][VX_DIM_Y]                  = 2;
            image->bounds[1][VX_DIM_X][VX_BOUND_END]    *= image->scales[1][VX_DIM_X];
            image->bounds[1][VX_DIM_Y][VX_BOUND_END]    *= image->scales[1][VX_DIM_Y];
            image->bounds[2][VX_DIM_X][VX_BOUND_END]    *= image->scales[2][VX_DIM_X];
            image->bounds[2][VX_DIM_Y][VX_BOUND_END]    *= image->scales[2][VX_DIM_Y];
            break;

        case VX_DF_IMAGE_U8:
            image->planeCount                           = 1;
            vxoImage_InitializePlane(image, 0, channelSize, 1, image->width, image->height);
            break;

        case VX_DF_IMAGE_U16:
        case VX_DF_IMAGE_S16:
            image->planeCount                           = 1;
            vxoImage_InitializePlane(image, 0, channelSize, 1, image->width, image->height);
            break;

        case VX_DF_IMAGE_U32:
        case VX_DF_IMAGE_S32:
            image->planeCount                           = 1;
            vxoImage_InitializePlane(image, 0, channelSize, 1, image->width, image->height);
            break;

        case VX_DF_IMAGE_F32:
            image->planeCount                           = 1;
            vxoImage_InitializePlane(image, 0, channelSize, 1, image->width, image->height);
            break;

        default:
            vxError("Unsupported image format: %d", image->format);
            vxmASSERT(0);
            break;
    }

    image->importType           = VX_IMPORT_TYPE_NONE;

    image->memory.planeCount    = image->planeCount;

    vxoImage_Dump(image);
}

VX_INTERNAL_CALLBACK_API void vxoImage_Destructor(vx_reference ref)
{
    vx_image image = (vx_image)ref;

    vxmASSERT(image);

    if (image->importType == VX_IMPORT_TYPE_NONE && image->parent == VX_NULL)
    {
        vxoImage_FreeMemory(image);
    }
    else if (image->parent != VX_NULL)
    {
        vxoReference_Release((vx_reference_ptr)&image->parent, VX_TYPE_IMAGE, VX_REF_INTERNAL);
    }
    else if (image->importType != VX_IMPORT_TYPE_NONE)
    {
        vx_uint32 planeIndex;

        vxmASSERT(image->importType == VX_IMPORT_TYPE_HOST);

        vxoImage_FreeWrappedMemory(image);

        for (planeIndex = 0; planeIndex < image->planeCount; planeIndex++)
        {
            image->memory.logicals[planeIndex]                  = VX_NULL;
            image->memory.strides[planeIndex][VX_DIM_CHANNEL]   = 0;
            image->memory.strides[planeIndex][VX_DIM_X]         = 0;
            image->memory.strides[planeIndex][VX_DIM_Y]         = 0;

            if (image->memory.writeLocks[planeIndex] != VX_NULL)
            {
                vxDestroyMutex(image->memory.writeLocks[planeIndex]);
                image->memory.writeLocks[planeIndex] = VX_NULL;
            }
        }

        image->memory.allocated = vx_false_e;
    }
}

VX_INTERNAL_API void vxoImage_Dump(vx_image image)
{
    if (image == VX_NULL)
    {
        vxTrace(VX_TRACE_MEMORY, "<image>null</image>\n");
    }
    else
    {
        vxoReference_Dump(&image->base);

        vxoMemory_Dump(&image->memory);

        vxTrace(VX_TRACE_IMAGE,
                "<image>\n"
                "    <address>"VX_FORMAT_HEX"</address>\n"
                "    <width>%u</width>\n"
                "    <height>%u</height>\n"
                "    <format>%s</format>\n"
                "</image>",
                image, image->width, image->height, vxImageFormat_ToString(image->format));
    }
}

VX_INTERNAL_API vx_bool vxoImage_AllocateMemory(vx_image image)
{
    vxmASSERT(image);

    return vxoMemory_Allocate(image->base.context, &image->memory);
}


VX_INTERNAL_API void vxoImage_FreeMemory(vx_image image)
{
    vxmASSERT(image);

    vxoMemory_Free(image->base.context, &image->memory);
}


VX_INTERNAL_API vx_bool vxoImage_WrapUserMemory(vx_image image)
{
    vxmASSERT(image);
#ifdef WIN32
    return vx_true_e;
#else
    return vxoMemory_WrapUserMemory(image->base.context, &image->memory);
#endif
}

VX_INTERNAL_API void vxoImage_FreeWrappedMemory(vx_image image)
{
    vxmASSERT(image);

    vxoMemory_FreeWrappedMemory(image->base.context, &image->memory);
}

VX_API_ENTRY vx_image VX_API_CALL vxCreateImageFromROI(vx_image image, const vx_rectangle_t *rect)
{
    vx_image    subImage;
    vx_uint32   planeIndex;

    if (!vxoImage_IsValid(image)) return VX_NULL;

    if (rect == VX_NULL)
    {
        return (vx_image)vxoContext_GetErrorObject(image->base.context, VX_ERROR_INVALID_PARAMETERS);
    }

    if (!vxoMemory_Allocate(image->base.context, &image->memory))
    {
        return (vx_image)vxoContext_GetErrorObject(image->base.context, VX_ERROR_NO_RESOURCES);
    }

    subImage = (vx_image)vxoReference_Create(
                            image->base.context, VX_TYPE_IMAGE, VX_REF_EXTERNAL, &image->base.context->base);

    if (vxoReference_GetStatus((vx_reference)subImage) != VX_SUCCESS) return subImage;

    subImage->parent            = image;
    vxoReference_Increment(&image->base, VX_REF_INTERNAL);

    subImage->format            = image->format;
    subImage->importType        = image->importType;
    subImage->channelRange      = image->channelRange;
    subImage->colorSpace        = image->colorSpace;
    subImage->width             = rect->end_x - rect->start_x;
    subImage->height            = rect->end_y - rect->start_y;
    subImage->planeCount        = image->planeCount;
    vxMemCopy(&subImage->scales, &image->scales, sizeof(image->scales));
    vxMemCopy(&subImage->memory, &image->memory, sizeof(image->memory));

    for (planeIndex = 0; planeIndex < subImage->planeCount; planeIndex++)
    {
        vx_uint32 offset = vxComputePlaneOffset(image, rect->start_x, rect->start_y, planeIndex);

        subImage->memory.dims[planeIndex][VX_DIM_X] = subImage->width;
        subImage->memory.dims[planeIndex][VX_DIM_Y] = subImage->height;
        subImage->memory.logicals[planeIndex]       = &image->memory.logicals[planeIndex][offset];
    }

    vxoImage_Dump(subImage);

    return subImage;
}

VX_PRIVATE_API vx_image vxoImage_Create(
        vx_context context, vx_uint32 width, vx_uint32 height, vx_df_image format, vx_bool isVirtual)
{
    vx_image image;

    if (!vxoContext_IsValid(context)) return VX_NULL;

    if (!vxImageFormat_IsSupported(format))
    {
        return (vx_image)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_FORMAT);
    }

    if (!vxImageFormat_IsValidWidthAndHeight(format, width, height))
    {
        return (vx_image)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_DIMENSION);
    }

    image = (vx_image)vxoReference_Create(context, VX_TYPE_IMAGE, VX_REF_EXTERNAL, &context->base);

    if (vxoReference_GetStatus((vx_reference)image) != VX_SUCCESS) return image;

    image->base.isVirtual = isVirtual;

    vxoImage_Initialize(image, width, height, format);

    return image;
}

VX_API_ENTRY vx_image VX_API_CALL vxCreateImage(vx_context context, vx_uint32 width, vx_uint32 height, vx_df_image format)
{
    if (!vxoContext_IsValid(context)) return VX_NULL;

    if (format == VX_DF_IMAGE_VIRT)
    {
        return (vx_image)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
    }

    if (width == 0 || height == 0)
    {
        return (vx_image)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_DIMENSION);
    }

    return vxoImage_Create(context, width, height, format, vx_false_e);
}

VX_API_ENTRY vx_image VX_API_CALL vxCreateVirtualImage(vx_graph graph, vx_uint32 width, vx_uint32 height, vx_df_image format)
{
    vx_image image;

    if (!vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH)) return VX_NULL;

    image = vxoImage_Create(graph->base.context, width, height, format, vx_true_e);

    if (vxoReference_GetStatus((vx_reference)image) != VX_SUCCESS) return image;

    image->base.scope = (vx_reference)graph;

    return image;
}

VX_API_ENTRY vx_image VX_API_CALL vxCreateUniformImage(
        vx_context context, vx_uint32 width, vx_uint32 height, vx_df_image format, const void *value)
{
    vx_image image;
    vx_uint32 x, y, planeIndex;
    vx_rectangle_t rect = {0, 0, width, height};

    if (!vxoContext_IsValid(context)) return VX_NULL;

    if (format == VX_DF_IMAGE_VIRT)
    {
        return (vx_image)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_FORMAT);
    }

    if (width == 0 || height == 0)
    {
        return (vx_image)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_DIMENSION);
    }

    if (value == VX_NULL)
    {
        return (vx_image)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
    }

    image = vxoImage_Create(context, width, height, format, vx_false_e);

    if (vxoReference_GetStatus((vx_reference)image) != VX_SUCCESS) return image;

    for (planeIndex = 0; planeIndex < image->planeCount; planeIndex++)
    {
        vx_imagepatch_addressing_t  patchAddr;
        vx_ptr                      basePtr = VX_NULL;
        vx_status                   status;

        status = vxAccessImagePatch(image, &rect, planeIndex, &patchAddr, &basePtr, VX_WRITE_ONLY);

        if (status != VX_SUCCESS)
        {
            vxReleaseImage(&image);
            return (vx_image)vxoContext_GetErrorObject(context, status);
        }

        for (y = 0; y < patchAddr.dim_y; y += patchAddr.step_y)
        {
            for (x = 0; x < patchAddr.dim_x; x += patchAddr.step_x)
            {
                switch (format)
                {
                    case VX_DF_IMAGE_U8:
                        {
                            vx_uint8_ptr ptr = (vx_uint8_ptr)vxFormatImagePatchAddress2d(basePtr, x, y, &patchAddr);
                            *ptr = *(vx_uint8_ptr)value;
                        }
                        break;

                    case VX_DF_IMAGE_U16:
                        {
                            vx_uint16_ptr ptr = (vx_uint16_ptr)vxFormatImagePatchAddress2d(basePtr, x, y, &patchAddr);
                            *ptr = *(vx_uint16_ptr)value;
                        }
                        break;

                    case VX_DF_IMAGE_U32:
                        {
                            vx_uint32_ptr ptr = (vx_uint32_ptr)vxFormatImagePatchAddress2d(basePtr, x, y, &patchAddr);
                            *ptr = *(vx_uint32_ptr)value;
                        }
                        break;

                    case VX_DF_IMAGE_S16:
                        {
                            vx_int16_ptr ptr = (vx_int16_ptr)vxFormatImagePatchAddress2d(basePtr, x, y, &patchAddr);
                            *ptr = *(vx_int16_ptr)value;
                        }
                        break;

                    case VX_DF_IMAGE_S32:
                        {
                            vx_int32_ptr ptr = (vx_int32_ptr)vxFormatImagePatchAddress2d(basePtr, x, y, &patchAddr);
                            *ptr = *(vx_int32_ptr)value;
                        }
                        break;

                    case VX_DF_IMAGE_RGB:
                    case VX_DF_IMAGE_RGBX:
                        {
                            vx_uint8_ptr pixelPtr = (vx_uint8_ptr)value;
                            vx_uint8_ptr ptr = (vx_uint8_ptr)vxFormatImagePatchAddress2d(basePtr, x, y, &patchAddr);

                            ptr[0] = pixelPtr[0];
                            ptr[1] = pixelPtr[1];
                            ptr[2] = pixelPtr[2];

                            if (format == VX_DF_IMAGE_RGBX) ptr[3] = pixelPtr[3];
                        }
                        break;

                    case VX_DF_IMAGE_YUV4:
                    case VX_DF_IMAGE_IYUV:
                        {
                            vx_uint8_ptr pixelPtr = (vx_uint8_ptr)value;
                            vx_uint8_ptr ptr = (vx_uint8_ptr)vxFormatImagePatchAddress2d(basePtr, x, y, &patchAddr);
                            *ptr = pixelPtr[planeIndex];
                        }
                        break;

                    case VX_DF_IMAGE_NV12:
                        {
                            vx_uint8_ptr pixelPtr = (vx_uint8_ptr)value;
                            vx_uint8_ptr ptr = (vx_uint8_ptr)vxFormatImagePatchAddress2d(basePtr, x, y, &patchAddr);

                            if (planeIndex == 0)
                            {
                                ptr[0] = pixelPtr[0];
                            }
                            else if (planeIndex == 1)
                            {
                                ptr[0] = pixelPtr[1];
                                ptr[1] = pixelPtr[2];
                            }
                        }
                        break;

                    case VX_DF_IMAGE_NV21:
                        {
                            vx_uint8_ptr pixelPtr = (vx_uint8_ptr)value;
                            vx_uint8_ptr ptr = (vx_uint8_ptr)vxFormatImagePatchAddress2d(basePtr, x, y, &patchAddr);

                            if (planeIndex == 0)
                            {
                                ptr[0] = pixelPtr[0];
                            }
                            else if (planeIndex == 1)
                            {
                                ptr[0] = pixelPtr[2];
                                ptr[1] = pixelPtr[1];
                            }
                        }
                        break;

                    case VX_DF_IMAGE_UYVY:
                        {
                            vx_uint8_ptr pixelPtr = (vx_uint8_ptr)value;
                            vx_uint8_ptr ptr = (vx_uint8_ptr)vxFormatImagePatchAddress2d(basePtr, x, y, &patchAddr);

                            if ((x % 2) == 0)
                            {
                                ptr[0] = pixelPtr[1];
                                ptr[1] = pixelPtr[0];
                            }
                            else
                            {
                                ptr[0] = pixelPtr[2];
                                ptr[1] = pixelPtr[0];
                            }
                        }
                        break;

                    case VX_DF_IMAGE_YUYV:
                        {
                            vx_uint8_ptr pixelPtr = (vx_uint8_ptr)value;
                            vx_uint8_ptr ptr = (vx_uint8_ptr)vxFormatImagePatchAddress2d(basePtr, x, y, &patchAddr);

                            if ((x % 2) == 0)
                            {
                                ptr[0] = pixelPtr[0];
                                ptr[1] = pixelPtr[1];
                            }
                            else
                            {
                                ptr[0] = pixelPtr[0];
                                ptr[1] = pixelPtr[2];
                            }
                        }
                        break;

                    default:
                        vxmASSERT(0);
                        return (vx_image)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_FORMAT);
                } /* switch */
            }
        }

        status = vxCommitImagePatch(image, &rect, planeIndex, &patchAddr, basePtr);

        if (status != VX_SUCCESS)
        {
            vxReleaseImage(&image);
            return (vx_image)vxoContext_GetErrorObject(context, status);
        }
    }

    image->isUniform = vx_true_e;

    return image;
}

VX_API_ENTRY vx_image VX_API_CALL vxCreateImageFromHandle(
        vx_context context, vx_df_image format, vx_imagepatch_addressing_t addrs[],
        void *ptrs[], vx_enum import_type)
{
    vx_image    image;
    vx_uint32   planeIndex;

    if (!vxoContext_IsValid(context)) return VX_NULL;

    if (!vxIsValidImportType(import_type))
    {
        return (vx_image)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_FORMAT);
    }

    if (format == VX_DF_IMAGE_VIRT)
    {
        return (vx_image)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_FORMAT);
    }

    if (addrs == VX_NULL || addrs[0].dim_x == 0 || addrs[0].dim_y == 0)
    {
        return (vx_image)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_DIMENSION);
    }

    image = vxoImage_Create(context, addrs[0].dim_x, addrs[0].dim_y, format, vx_false_e);

    if (vxoReference_GetStatus((vx_reference)image) != VX_SUCCESS) return image;

    image->importType       = import_type;

    if (import_type == VX_IMPORT_TYPE_HOST)
    {
        image->memory.wrapFlag  = gcvALLOC_FLAG_USERMEMORY;
    }
    else if(import_type == VX_IMPORT_TYPE_DMABUF)
    {
        image->memory.wrapFlag  = gcvALLOC_FLAG_DMABUF;
    }

    for (planeIndex = 0; planeIndex < image->planeCount; planeIndex++)
    {
        image->memory.logicals[planeIndex]                  = (vx_uint8_ptr)ptrs[planeIndex];
        image->memory.strides[planeIndex][VX_DIM_CHANNEL]   = (vx_uint32)vxImageFormat_GetChannelSize(format);
        image->memory.strides[planeIndex][VX_DIM_X]         = addrs[planeIndex].stride_x;
        image->memory.strides[planeIndex][VX_DIM_Y]         = addrs[planeIndex].stride_y;

        if (!vxCreateMutex(OUT &image->memory.writeLocks[planeIndex]))
        {
            goto OnError;
        }
    }

    if (!vxoImage_WrapUserMemory(image)) goto OnError;

    return image;

OnError:
    for (planeIndex = 0; planeIndex < image->planeCount; planeIndex++)
    {
        if (image->memory.writeLocks[planeIndex] != VX_NULL)
        {
            vxDestroyMutex(image->memory.writeLocks[planeIndex]);
            image->memory.writeLocks[planeIndex]  = VX_NULL;
        }
    }

    vxReleaseImage(&image);

    return (vx_image)vxoContext_GetErrorObject(context, VX_ERROR_NO_RESOURCES);
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseImage(vx_image *image)
{
    return vxoReference_Release((vx_reference_ptr)image, VX_TYPE_IMAGE, VX_REF_EXTERNAL);
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryImage(vx_image image, vx_enum attribute, void *ptr, vx_size size)
{
    vx_size     imageSize = 0;
    vx_uint32   planeIndex;

    if (!vxoImage_IsValid(image)) return VX_ERROR_INVALID_REFERENCE;

    switch (attribute)
    {
        case VX_IMAGE_ATTRIBUTE_FORMAT:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_df_image, 0x3);

            *(vx_df_image *)ptr = image->format;
            break;

        case VX_IMAGE_ATTRIBUTE_WIDTH:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            *(vx_uint32 *)ptr = image->width;
            break;

        case VX_IMAGE_ATTRIBUTE_HEIGHT:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            *(vx_uint32 *)ptr = image->height;
            break;

        case VX_IMAGE_ATTRIBUTE_PLANES:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size *)ptr = image->planeCount;
            break;

        case VX_IMAGE_ATTRIBUTE_SPACE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);

            *(vx_enum *)ptr = image->colorSpace;
            break;

        case VX_IMAGE_ATTRIBUTE_RANGE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);

            *(vx_enum *)ptr = image->channelRange;
            break;

        case VX_IMAGE_ATTRIBUTE_SIZE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            for (planeIndex = 0; planeIndex < image->planeCount; planeIndex++)
            {
                imageSize += abs(image->memory.strides[planeIndex][VX_DIM_Y]) * image->memory.dims[planeIndex][VX_DIM_Y];
            }

            *(vx_size *)ptr = imageSize;
            break;

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            return VX_ERROR_NOT_SUPPORTED;
    }

    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetImageAttribute(vx_image image, vx_enum attribute, const void *ptr, vx_size size)
{
    if (!vxoImage_IsValid(image)) return VX_ERROR_INVALID_REFERENCE;

    switch (attribute)
    {
        case VX_IMAGE_ATTRIBUTE_SPACE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);

            image->colorSpace = *(vx_enum *)ptr;
            break;

        case VX_IMAGE_ATTRIBUTE_RANGE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);

            image->channelRange = *(vx_enum *)ptr;
            break;

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            return VX_ERROR_NOT_SUPPORTED;
    }

    return VX_SUCCESS;
}

VX_API_ENTRY vx_size VX_API_CALL vxComputeImagePatchSize(
        vx_image image, const vx_rectangle_t *rect, vx_uint32 plane_index)
{
    vx_size pixelCount, pixelSize;

    if (!vxoImage_IsValid(image)) return 0;

    if (rect == VX_NULL) return 0;

    if (image->memory.logicals[0] == VX_NULL)
    {
        if (!vxoImage_AllocateMemory(image)) return 0;
    }

    if (plane_index >= image->planeCount) return 0;

    pixelCount  = ((rect->end_x - rect->start_x) / image->scales[plane_index][VX_DIM_X])
                    * ((rect->end_y - rect->start_y) / image->scales[plane_index][VX_DIM_Y]);

    pixelSize   = image->memory.strides[plane_index][VX_DIM_X];

    return pixelCount * pixelSize;
}

VX_API_ENTRY vx_status VX_API_CALL vxAccessImagePatch(
        vx_image image, const vx_rectangle_t *rect, vx_uint32 plane_index,
        vx_imagepatch_addressing_t *patchAddr, void **ptr, vx_enum usage)
{
    vx_bool         mapped = vx_false_e;
    vx_uint8_ptr    memoryPtr;

    if (!vxoImage_IsValid(image)) return VX_ERROR_INVALID_REFERENCE;

    if (rect == VX_NULL) return VX_ERROR_INVALID_PARAMETERS;

    if (patchAddr == VX_NULL || ptr == VX_NULL) return VX_ERROR_INVALID_PARAMETERS;

    if (usage < VX_READ_ONLY || VX_READ_AND_WRITE < usage) return VX_ERROR_INVALID_PARAMETERS;

    if (image->base.isVirtual && !image->base.accessible) return VX_ERROR_OPTIMIZED_AWAY;

    if (patchAddr == VX_NULL || ptr == VX_NULL) return VX_ERROR_INVALID_PARAMETERS;

    if (rect->start_x >= rect->end_x || rect->start_y >= rect->end_y) return VX_ERROR_INVALID_PARAMETERS;

    if (plane_index >= image->planeCount || plane_index >= image->memory.planeCount) return VX_ERROR_INVALID_PARAMETERS;

    if (image->memory.logicals[0] == VX_NULL)
    {
        if (!vxoImage_AllocateMemory(image)) return VX_ERROR_NO_MEMORY;
    }

    if (image->isUniform)
    {
        if (usage == VX_WRITE_ONLY || usage == VX_READ_AND_WRITE) return VX_ERROR_NOT_SUPPORTED;
    }

    if (*ptr == VX_NULL)
    {
        mapped = (vx_bool)(usage == VX_READ_ONLY || usage == VX_WRITE_ONLY || usage == VX_READ_AND_WRITE);
    }

    if (mapped)
    {
        if (usage != VX_READ_ONLY)
        {
            if (!vxAcquireMutex(image->memory.writeLocks[plane_index])) return VX_ERROR_NO_RESOURCES;
        }

        vxoMemory_Dump(&image->memory);

        memoryPtr = (vx_uint8_ptr)image->memory.logicals[plane_index];

        patchAddr->dim_x    = rect->end_x - rect->start_x;
        patchAddr->dim_y    = rect->end_y - rect->start_y;
        patchAddr->stride_x = image->memory.strides[plane_index][VX_DIM_X];
        patchAddr->stride_y = image->memory.strides[plane_index][VX_DIM_Y];
        patchAddr->step_x   = image->scales[plane_index][VX_DIM_X];
        patchAddr->step_y   = image->scales[plane_index][VX_DIM_Y];
        patchAddr->scale_x  = VX_SCALE_UNITY / image->scales[plane_index][VX_DIM_X];
        patchAddr->scale_y  = VX_SCALE_UNITY / image->scales[plane_index][VX_DIM_Y];

        *ptr = &memoryPtr[vxComputePatchOffset(rect->start_x, rect->start_y, patchAddr)];

        vxoReference_IncrementReadCount(&image->base);

        vxoReference_Increment(&image->base, VX_REF_EXTERNAL);
    }
    else
    {
        vx_uint32 accessorIndex;

        vx_imagepatch_addressing_t *addrSave = vxAllocate(sizeof(vx_imagepatch_addressing_t));
        addrSave->stride_x = patchAddr->stride_x;
        addrSave->stride_y = patchAddr->stride_y;

        if (!vxoContext_AddAccessor(image->base.context, vxComputeImagePatchSize(image, rect, plane_index),
                                    usage, *ptr, &image->base, OUT &accessorIndex, addrSave))
        {
            return VX_ERROR_NO_MEMORY;
        }

        *ptr = image->base.context->accessorTable[accessorIndex].ptr;
    }

    if (*ptr != VX_NULL && !mapped)
    {
        vx_uint32 y;
        vx_imagepatch_addressing_t *addrSave = (vx_imagepatch_addressing_t*)image->base.context->accessorTable->extraDataPtr;

        memoryPtr = (vx_uint8_ptr)(*ptr);

        if (usage != VX_READ_ONLY)
        {
            if (!vxAcquireMutex(image->memory.writeLocks[plane_index])) return VX_ERROR_NO_RESOURCES;
        }

        patchAddr->dim_x    = rect->end_x - rect->start_x;
        patchAddr->dim_y    = rect->end_y - rect->start_y;
        patchAddr->step_x   = image->scales[plane_index][VX_DIM_X];
        patchAddr->step_y   = image->scales[plane_index][VX_DIM_Y];
        patchAddr->scale_x  = VX_SCALE_UNITY / image->scales[plane_index][VX_DIM_X];
        patchAddr->scale_y  = VX_SCALE_UNITY / image->scales[plane_index][VX_DIM_Y];

        if (usage == VX_READ_ONLY || usage == VX_READ_AND_WRITE)
        {
            if (addrSave->stride_x == image->memory.strides[plane_index][VX_DIM_X])
            {
                for (y = rect->start_y; y < rect->end_y; y+=patchAddr->step_y)
                {
                    vx_uint32 planeOffset, patchOffset, rangeSize;

                    planeOffset = vxComputePlaneOffset(image, rect->start_x, y, plane_index);
                    patchOffset = vxComputePatchOffset(0, (y - rect->start_y), patchAddr);
                    rangeSize = vxComputePlaneRangeSize(image, patchAddr->dim_x, plane_index);

                    vxMemCopy(&memoryPtr[patchOffset], &image->memory.logicals[plane_index][planeOffset], rangeSize);
                }
            }
            else
            {
                vx_uint8 *tmp = (vx_uint8*)*ptr;
                vx_uint32 x;

                vx_uint8 *pDestLine = &tmp[0];
                for (y = rect->start_y; y < rect->end_y; y+=patchAddr->step_y)
                {
                    vx_uint8 *pDest = pDestLine;

                    vx_uint32 offset = vxComputePlaneOffset(image, rect->start_x, y, plane_index);
                    vx_uint8 *pSrc = &image->memory.logicals[plane_index][offset];

                    for (x = rect->start_x; x < rect->end_x; x+=patchAddr->step_x)
                    {
                        memcpy(pDest, pSrc, image->memory.strides[plane_index][VX_DIM_X]);

                        pSrc += image->memory.strides[plane_index][VX_DIM_X];
                        pDest += addrSave->stride_x;
                    }

                    pDestLine += addrSave->stride_y;
                }
            }

            vxoReference_IncrementReadCount(&image->base);
        }

        vxoReference_Increment(&image->base, VX_REF_EXTERNAL);
    }

    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxCommitImagePatch(
        vx_image image, vx_rectangle_t *rect, vx_uint32 plane_index,
        vx_imagepatch_addressing_t *patchAddr, const void *ptr)
{
    vx_bool     isZeroRect = vx_true_e;
    vx_bool     foundAccessor;
    vx_uint32   accessorIndex;

    if (!vxoImage_IsValid(image)) return VX_ERROR_INVALID_REFERENCE;

    if (rect != VX_NULL)
    {
        isZeroRect = (vx_bool)(rect->end_x - rect->start_x == 0 || rect->end_y - rect->start_y == 0);
    }

    if (image->base.isVirtual && !isZeroRect)
    {
        if (!image->base.accessible) return VX_ERROR_OPTIMIZED_AWAY;
    }

    if (!isZeroRect)
    {
        vxmASSERT(rect);

        if (plane_index >= image->planeCount || plane_index >= image->memory.planeCount) return VX_ERROR_INVALID_PARAMETERS;

        if (patchAddr == VX_NULL || ptr == VX_NULL) return VX_ERROR_INVALID_PARAMETERS;

        if (rect->start_x >= rect->end_x || rect->start_y >= rect->end_y) return VX_ERROR_INVALID_PARAMETERS;

        if (rect->end_x - rect->start_x > patchAddr->dim_x) return VX_ERROR_INVALID_PARAMETERS;

        if (rect->end_y - rect->start_y > patchAddr->dim_y) return VX_ERROR_INVALID_PARAMETERS;

        if (rect->end_x > (vx_uint32)image->memory.dims[plane_index][VX_DIM_X]
                            * (vx_uint32)image->scales[plane_index][VX_DIM_X])
        {
             return VX_ERROR_INVALID_PARAMETERS;
        }

        if (rect->end_y > (vx_uint32)image->memory.dims[plane_index][VX_DIM_Y]
                            * (vx_uint32)image->scales[plane_index][VX_DIM_Y])
        {
             return VX_ERROR_INVALID_PARAMETERS;
        }
    }

    foundAccessor = vxoContext_SearchAccessor(image->base.context, (vx_ptr)ptr, &accessorIndex);

    if (!isZeroRect)
    {
        vxmASSERT(rect);

        if (image->isUniform) return VX_ERROR_NOT_SUPPORTED;

        if (foundAccessor && image->base.context->accessorTable[accessorIndex].usage == VX_READ_ONLY)
        {
            vxoContext_RemoveAccessor(image->base.context, accessorIndex);
        }
        else
        {
            vx_uint8_ptr    memoryPtr           = (vx_uint8_ptr)ptr;
            vx_int32        end;
            vx_bool         isExternalMemory    = vx_true_e;

            if (image->region.start_x   > rect->start_x)    image->region.start_x   = rect->start_x;
            if (image->region.start_y   > rect->start_y)    image->region.start_y   = rect->start_y;
            if (image->region.end_x     < rect->end_x)      image->region.end_x     = rect->end_x;
            if (image->region.end_y     < rect->end_y)      image->region.end_y     = rect->end_y;

            end = (image->memory.dims[plane_index][VX_DIM_Y] * image->memory.strides[plane_index][VX_DIM_Y]);

            if (image->memory.logicals[plane_index] <= memoryPtr
                && memoryPtr < &image->memory.logicals[plane_index][end])
            {
                if (image->memory.strides[plane_index][VX_DIM_Y]
                    != image->memory.dims[plane_index][VX_DIM_X] * image->memory.strides[plane_index][VX_DIM_X])
                {
                    vx_uint8_ptr    base        = image->memory.logicals[plane_index];
                    vx_size         offset      = (vx_size)(memoryPtr - base)
                                                    % image->memory.strides[plane_index][VX_DIM_Y];

                    if (offset < (vx_size)image->memory.dims[plane_index][VX_DIM_X]
                                    * (vx_size)image->memory.strides[plane_index][VX_DIM_X])
                    {
                        isExternalMemory = vx_false_e;
                    }
                }
                else
                {
                    isExternalMemory = vx_false_e;
                }
            }

            if (isExternalMemory || foundAccessor)
            {
                vx_uint32 x, y;
                vx_imagepatch_addressing_t * addrSave = (vx_imagepatch_addressing_t *)image->base.context->accessorTable->extraDataPtr;

                if (addrSave->stride_x == image->memory.strides[plane_index][VX_DIM_X])
                {
                    for (y = rect->start_y; y < rect->end_y; y += patchAddr->step_y)
                    {
                        vx_uint32 planOffset, patchOffset, rangeSize;

                        planOffset  = vxComputePlaneOffset(image, rect->start_x, y, plane_index);
                        patchOffset = vxComputePatchOffset(0, y - rect->start_y, patchAddr);
                        rangeSize   = vxComputePatchRangeSize(rect->end_x - rect->start_x, patchAddr);

                        vxMemCopy(&image->memory.logicals[plane_index][planOffset], &memoryPtr[patchOffset], rangeSize);
                    }
                }
                else
                {
                    vx_uint8 *tmp = (vx_uint8*)ptr;
                    vx_uint8 *pDestLine = &tmp[0];
                    for (y = rect->start_y; y < rect->end_y; y+=patchAddr->step_y)
                    {
                        vx_uint8 *pSrc = pDestLine;

                        vx_uint32 offset = vxComputePlaneOffset(image, rect->start_x, y, plane_index);
                        vx_uint8 *pDest = &image->memory.logicals[plane_index][offset];

                        for (x = rect->start_x; x < rect->end_x; x+=patchAddr->step_x)
                        {
                            memcpy(pDest, pSrc, image->memory.strides[plane_index][VX_DIM_X]);

                            pDest += image->memory.strides[plane_index][VX_DIM_X];
                            pSrc += addrSave->stride_x;
                        }

                        pDestLine += addrSave->stride_y;
                    }
                }

                if (foundAccessor) vxoContext_RemoveAccessor(image->base.context, accessorIndex);
            }

            vxoReference_IncrementWriteCount(&image->base);
        }

        vxReleaseMutex(image->memory.writeLocks[plane_index]);
    }
    else
    {
        if (foundAccessor) vxoContext_RemoveAccessor(image->base.context, accessorIndex);
    }

    vxoReference_Decrement(&image->base, VX_REF_EXTERNAL);

    return VX_SUCCESS;
}

VX_API_ENTRY void * VX_API_CALL vxFormatImagePatchAddress1d(
        void *ptr, vx_uint32 planeIndex, const vx_imagepatch_addressing_t *patchAddr)
{
    if (ptr == VX_NULL) return VX_NULL;

    if (planeIndex >= patchAddr->dim_x * patchAddr->dim_y) return VX_NULL;

    return (vx_uint8_ptr)ptr
            + vxComputePatchOffset(planeIndex % patchAddr->dim_x, planeIndex / patchAddr->dim_x, patchAddr);
}

VX_API_ENTRY void * VX_API_CALL vxFormatImagePatchAddress2d(
        void *ptr, vx_uint32 x, vx_uint32 y, const vx_imagepatch_addressing_t *patchAddr)
{
    if (ptr == VX_NULL) return VX_NULL;

    if (x >= patchAddr->dim_x || y >= patchAddr->dim_y) return VX_NULL;

    return (vx_uint8_ptr)ptr + vxComputePatchOffset(x, y, patchAddr);
}

VX_API_ENTRY vx_status VX_API_CALL vxGetValidRegionImage(vx_image image, vx_rectangle_t *rect)
{
    if (!vxoImage_IsValid(image)) return VX_ERROR_INVALID_REFERENCE;

    if (rect == VX_NULL) return VX_ERROR_INVALID_PARAMETERS;

    if (image->region.start_x <= image->region.end_x && image->region.start_y <= image->region.end_y)
    {
        rect->start_x   = image->region.start_x;
        rect->start_y   = image->region.start_y;
        rect->end_x     = image->region.end_x;
        rect->end_y     = image->region.end_y;
    }
    else
    {
        rect->start_x   = 0;
        rect->start_y   = 0;
        rect->end_x     = image->width;
        rect->end_y     = image->height;
    }

    return VX_SUCCESS;
}

