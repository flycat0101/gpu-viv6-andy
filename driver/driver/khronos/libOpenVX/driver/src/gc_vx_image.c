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

#define _GC_OBJ_ZONE            gcdZONE_VX_IMAGE

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

VX_INTERNAL_API vx_uint32 vxComputePlaneOffset(vx_image image, vx_uint32 x, vx_uint32 y, vx_uint32 planeIndex)
{
    vxmASSERT(image);
    vxmASSERT(planeIndex < image->memory.planeCount);

    return  x / image->scales[planeIndex][VX_DIM_X] * image->memory.strides[planeIndex][VX_DIM_X]
            + y / image->scales[planeIndex][VX_DIM_Y] * image->memory.strides[planeIndex][VX_DIM_Y];
}

VX_PRIVATE_API vx_uint32 vxComputePatchRangeSize(vx_uint32 patchRange, const vx_imagepatch_addressing_t *patchAddr)
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
    image->memory.sizes[planeIndex]                             = 0;

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

    gcmHEADER_ARG("image=%p, width=0x%x, height=0x%x, imageFormat=%p", image, width, height, imageFormat);
    vxmASSERT(image);

    image->width                = width;
    image->height               = height;
    image->arraySize            = 0;
    image->sliceSize            = 0;
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

    image->importType           = VX_MEMORY_TYPE_NONE;

    image->memory.planeCount    = image->planeCount;

    image->useInternalMem       = vx_true_e;

    vxoImage_Dump(image);

    gcmFOOTER_NO();
}

VX_INTERNAL_API vx_bool vxoImage_IsVirtualImage(vx_image image)
{
    return image->base.isVirtual;
}

VX_INTERNAL_CALLBACK_API void vxoImage_Destructor(vx_reference ref)
{
    vx_image image = (vx_image)ref;

    gcmHEADER_ARG("ref=%p", ref);
    vxmASSERT(image);

    if (image->importType == VX_MEMORY_TYPE_NONE && image->parent == VX_NULL)
    {
        vxoImage_FreeMemory(image);
    }
    else if (image->parent != VX_NULL)
    {
        vx_uint32 planeIndex;
        vxoReference_Release((vx_reference_ptr)&image->parent, VX_TYPE_IMAGE, VX_REF_INTERNAL);
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
    }
    else if (image->importType != VX_MEMORY_TYPE_NONE)
    {
        vx_uint32 planeIndex;
        vx_context context = vxGetContext((vx_reference)image);

        if(image->useInternalMem == vx_false_e)
        {
            vxmASSERT(image->importType == VX_MEMORY_TYPE_HOST || image->importType == VX_MEMORY_TYPE_HOST_UNCACHED || image->importType == VX_MEMORY_TYPE_INTERNAL);

            if (image->importType != VX_MEMORY_TYPE_INTERNAL)
            {
                vxoImage_FreeWrappedMemory(image);
            }
        }

        for (planeIndex = 0; planeIndex < image->planeCount; planeIndex++)
        {
            if (image->memory.nodePtrs[planeIndex] != VX_NULL && image->memory.logicals[planeIndex] != image->memory.nodePtrs[planeIndex]->logical)
            {
                gcoVX_FreeMemory((gcsSURF_NODE_PTR)image->memory.nodePtrs[planeIndex]);
                image->memory.nodePtrs[planeIndex] = VX_NULL;

                context->memoryCount--;

            }

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
    gcmFOOTER_NO();
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
                "    <address>%p</address>\n"
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

    if (image->importType == VX_MEMORY_TYPE_NONE)
        return vxoMemory_Allocate(image->base.context, &image->memory);
    else
        return vx_false_e;
}


VX_INTERNAL_API void vxoImage_FreeMemory(vx_image image)
{
    vxmASSERT(image);

    vxoMemory_Free(image->base.context, &image->memory);
}


VX_INTERNAL_API vx_bool vxoImage_WrapUserMemory(vx_image image)
{
    vx_uint32 planeIndex;
    vx_bool status = vx_false_e;

    gcmHEADER_ARG("image=%p", image);

    status = vxoMemory_WrapUserMemory(image->base.context, &image->memory);

    if(status == vx_false_e)
    {
        vxmASSERT(image);
        /* For CTS, vxCreateImageFromHandle */
        image->memory.allocated = vx_true_e;

        for (planeIndex = 0; planeIndex < image->planeCount; planeIndex++)
        {
            if (image->memory.nodePtrs[planeIndex] == VX_NULL)
            {
                vx_context context = vxGetContext((vx_reference)image);
                gctUINT32_PTR logical = 0;
                vx_uint32 size = sizeof(vx_uint8);
                if (image->memory.sizes[planeIndex] == 0)
                {
                    vx_uint32 dimIndex;
                    if (image->memory.strides[planeIndex][VX_DIM_CHANNEL] != 0)
                    {
                        size = (gctUINT32)abs(image->memory.strides[planeIndex][VX_DIM_CHANNEL]);
                    }

                    for (dimIndex = 0; (vx_uint32)dimIndex < image->memory.dimCount; dimIndex++)
                    {
                        image->memory.strides[planeIndex][dimIndex] = (gctUINT32)size;
                        size *= (gctUINT32)abs(image->memory.dims[planeIndex][dimIndex]);
                    }

                    image->memory.sizes[planeIndex] = size;
                }
                if (gcvSTATUS_OK != gcoVX_AllocateMemory((gctUINT32)image->memory.sizes[planeIndex], (gctPOINTER*)&logical,
                                    &image->memory.physicals[planeIndex],
                                    &image->memory.nodePtrs[planeIndex]))
                {
                    goto OnError;
                }
                context->memoryCount++;
                if (image->memory.sizes[planeIndex] > 0)
                {
                    gcoOS_MemCopy(logical, image->memory.logicals[planeIndex], image->memory.sizes[planeIndex]);
                }
            }
            if (!vxCreateMutex(OUT &image->memory.writeLocks[planeIndex]))
            {
                goto OnError;
            }
        }
        gcmFOOTER_ARG("%d", vx_true_e);
        return vx_true_e;
    }
    else
    {
        vx_uint32 p = 0;

        image->useInternalMem = vx_false_e;
        for(p = 0; p < image->planeCount; p++)
        {
            gcoOS_CacheFlush(gcvNULL, image->memory.wrappedNode[p], image->memory.logicals[p], image->memory.wrappedSize[p]);
        }
    }
    gcmFOOTER_ARG("%d", vx_true_e);
    return vx_true_e;

OnError:
    for (planeIndex = 0; planeIndex < image->planeCount; planeIndex++)
    {
        if (image->memory.writeLocks[planeIndex] != VX_NULL)
        {
            vxDestroyMutex(image->memory.writeLocks[planeIndex]);
            image->memory.writeLocks[planeIndex]  = VX_NULL;
        }
    }
    gcmFOOTER_ARG("%d", vx_false_e);
    return vx_false_e;
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
    vx_rectangle_t image_rect;

    gcmHEADER_ARG("image=%p, rect=%p", image, rect);
    gcmDUMP_API("$VX vxCreateImageFromROI: image=%p, rect=%p", image, rect);

    if (!vxoImage_IsValid(image))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    if (rect == VX_NULL)
    {
        gcmFOOTER_NO();
        return (vx_image)vxoContext_GetErrorObject(image->base.context, VX_ERROR_INVALID_PARAMETERS);
    }

    if (!vxoMemory_Allocate(image->base.context, &image->memory))
    {
        gcmFOOTER_NO();
        return (vx_image)vxoContext_GetErrorObject(image->base.context, VX_ERROR_NO_RESOURCES);
    }

    subImage = (vx_image)vxoReference_Create(
                            image->base.context, VX_TYPE_IMAGE, VX_REF_EXTERNAL, &image->base.context->base);

    if (vxoReference_GetStatus((vx_reference)subImage) != VX_SUCCESS)
    {
        gcmFOOTER_ARG("subImage=%p", subImage);
        return subImage;
    }
    subImage->parent            = image;

    for(planeIndex = 0; planeIndex < VX_MAX_REF_COUNT; planeIndex++)
    {
        if (image->subimages[planeIndex] == NULL)
        {
            image->subimages[planeIndex] = subImage;
            break;
        }
    }

    /* remember that the scope of the image is the parent image*/
    subImage->base.scope        = (vx_reference)image;

    vxoReference_Increment(&image->base, VX_REF_INTERNAL);

    subImage->format            = image->format;
    subImage->importType        = image->importType;
    subImage->channelRange      = image->channelRange;
    subImage->colorSpace        = image->colorSpace;
    subImage->width             = rect->end_x - rect->start_x;
    subImage->height            = rect->end_y - rect->start_y;
    subImage->planeCount        = image->planeCount;
    subImage->isUniform         = image->isUniform;

    vxGetValidRegionImage(image, &image_rect);

    /* valid rectangle */
    if(rect->start_x > image_rect.end_x ||
       rect->end_x   < image_rect.start_x ||
       rect->start_y > image_rect.end_y ||
       rect->end_y   < image_rect.start_y)
    {
        subImage->region.start_x    =
        subImage->region.start_y    =
        subImage->region.end_x        =
        subImage->region.end_y        = 0;
    }
    else
    {
        subImage->region.start_x    = gcmMAX(image_rect.start_x, rect->start_x) - rect->start_x;
        subImage->region.start_y    = gcmMAX(image_rect.start_y, rect->start_y) - rect->start_y;
        subImage->region.end_x        = gcmMIN(image_rect.end_x, rect->end_x) - rect->start_x;
        subImage->region.end_y        = gcmMIN(image_rect.end_y, rect->end_y) - rect->start_y;
    }

    vxMemCopy(&subImage->scales, &image->scales, sizeof(image->scales));
    vxMemCopy(&subImage->memory, &image->memory, sizeof(image->memory));

    for (planeIndex = 0; planeIndex < subImage->planeCount; planeIndex++)
    {
        vx_uint32 offset = vxComputePlaneOffset(image, rect->start_x, rect->start_y, planeIndex);

        subImage->memory.dims[planeIndex][VX_DIM_X] = subImage->width;
        subImage->memory.dims[planeIndex][VX_DIM_Y] = subImage->height;
        subImage->memory.logicals[planeIndex]       = &image->memory.logicals[planeIndex][offset];

        vxCreateMutex(&subImage->memory.writeLocks[planeIndex]);

        /* keep offset to allow vxSwapImageHandle update ROI pointers*/
        subImage->memory.offset[planeIndex]         = offset;
    }

    subImage->useInternalMem = image->useInternalMem;

    vxoImage_Dump(subImage);

    gcmFOOTER_ARG("subImage=%p", subImage);
    return subImage;
}

VX_PRIVATE_API vx_image vxoImage_Create(
        vx_context context, vx_uint32 width, vx_uint32 height, vx_df_image format, vx_bool isVirtual)
{
    vx_image image;

    gcmHEADER_ARG("context=%p, width=0x%x, height=0x%x, format=%p, isVirtual=0x%x", context, width, height, format, isVirtual);

    if (!vxoContext_IsValid(context))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    if (!vxImageFormat_IsSupported(format))
    {
        gcmFOOTER_NO();
        return (vx_image)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_FORMAT);
    }

    if (!vxImageFormat_IsValidWidthAndHeight(format, width, height))
    {
        gcmFOOTER_NO();
        return (vx_image)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_DIMENSION);
    }

    image = (vx_image)vxoReference_Create(context, VX_TYPE_IMAGE, VX_REF_EXTERNAL, &context->base);

    if (vxoReference_GetStatus((vx_reference)image) != VX_SUCCESS)
    {
        gcmFOOTER_ARG("image=%p", image);
        return image;
    }
    image->base.isVirtual = isVirtual;

    vxoImage_Initialize(image, width, height, format);

    gcmFOOTER_ARG("image=%p", image);
    return image;
}

VX_API_ENTRY vx_image VX_API_CALL vxCreateImage(vx_context context, vx_uint32 width, vx_uint32 height, vx_df_image format)
{
    gcmHEADER_ARG("context=%p, width=0x%x, height=0x%x, format=%p", context, width, height, format);
    gcmDUMP_API("$VX vxCreateImage: context=%p, width=0x%x, height=0x%x, format=%p", context, width, height, format);

    if (!vxoContext_IsValid(context))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    if (format == VX_DF_IMAGE_VIRT)
    {
        gcmFOOTER_NO();
        return (vx_image)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
    }

    if (width == 0 || height == 0)
    {
        gcmFOOTER_NO();
        return (vx_image)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_DIMENSION);
    }

    if (width > OVX_IMAGE_HW_MAX_WIDTH || height > OVX_IMAGE_HW_MAX_HEIGHT)
    {
        vxError("Due to the HW limitation, the width or height of image can not exceed 65535.\n");
        return (vx_image)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_DIMENSION);
    }

    gcmFOOTER_NO();
    return vxoImage_Create(context, width, height, format, vx_false_e);
}

VX_API_ENTRY vx_image VX_API_CALL vxCreateVirtualImage(vx_graph graph, vx_uint32 width, vx_uint32 height, vx_df_image format)
{
    vx_image image;

    gcmHEADER_ARG("graph=%p, width=0x%x, height=0x%x, format=%p", graph, width, height, format);
    gcmDUMP_API("$VX vxCreateVirtualImage: graph=%p, width=0x%x, height=0x%x, format=%p", graph, width, height, format);

    if (!vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }

    if (width > OVX_IMAGE_HW_MAX_WIDTH || height > OVX_IMAGE_HW_MAX_HEIGHT)
    {
        vxError("Due to the HW limitation, the width or height of image can not exceed 65535.\n");
        return VX_NULL;
    }

    image = vxoImage_Create(graph->base.context, width, height, format, vx_true_e);

    if (vxoReference_GetStatus((vx_reference)image) != VX_SUCCESS)
    {
        gcmFOOTER_ARG("image=%p", image);
        return image;
    }
    image->base.scope = (vx_reference)graph;

    gcmFOOTER_ARG("image=%p", image);
    return image;
}

VX_API_ENTRY vx_image VX_API_CALL vxCreateUniformImage(
        vx_context context, vx_uint32 width, vx_uint32 height, vx_df_image format, const vx_pixel_value_t *value)
{
    vx_image image;
    vx_uint32 x, y, planeIndex;
    vx_rectangle_t rect = {0, 0, width, height};

    gcmHEADER_ARG("context=%p, width=0x%x, height=0x%x, format=%p, value=%p", context, width, height, format, value);
    gcmDUMP_API("$VX vxCreateUniformImage: context=%p, width=0x%x, height=0x%x, format=%p, value=%p", context, width, height, format, value);

    if (!vxoContext_IsValid(context))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    if (format == VX_DF_IMAGE_VIRT)
    {
        gcmFOOTER_NO();
        return (vx_image)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_FORMAT);
    }

    if (width == 0 || height == 0)
    {
        gcmFOOTER_NO();
        return (vx_image)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_DIMENSION);
    }

    if (width > OVX_IMAGE_HW_MAX_WIDTH || height > OVX_IMAGE_HW_MAX_HEIGHT)
    {
        vxError("Due to the HW limitation, the width or height of image can not exceed 65535.\n");
        return (vx_image)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_DIMENSION);
    }

    if (value == VX_NULL)
    {
        gcmFOOTER_NO();
        return (vx_image)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
    }

    image = vxoImage_Create(context, width, height, format, vx_false_e);

    if (vxoReference_GetStatus((vx_reference)image) != VX_SUCCESS)
    {
        gcmFOOTER_ARG("image=%p", image);
        return image;
    }
    for (planeIndex = 0; planeIndex < image->planeCount; planeIndex++)
    {
        vx_imagepatch_addressing_t  patchAddr;
        vx_ptr                      basePtr = VX_NULL;
        vx_status                   status;

        status = vxAccessImagePatch(image, &rect, planeIndex, &patchAddr, &basePtr, VX_WRITE_ONLY);

        if (status != VX_SUCCESS)
        {
            vxReleaseImage(&image);
            gcmFOOTER_NO();
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
                        gcmFOOTER_NO();
                        return (vx_image)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_FORMAT);
                } /* switch */
            }
        }

        status = vxCommitImagePatch(image, &rect, planeIndex, &patchAddr, basePtr);

        if (status != VX_SUCCESS)
        {
            vxReleaseImage(&image);
            gcmFOOTER_NO();
            return (vx_image)vxoContext_GetErrorObject(context, status);
        }
    }

    if (vxGetStatus((vx_reference)image) == VX_SUCCESS)
    {
        /* lock the image from being modified again! */
        ((vx_image_s *)image)->isUniform = vx_true_e;
    }

    gcmFOOTER_ARG("image=%p", image);
    return image;
}

VX_API_ENTRY vx_image VX_API_CALL vxCreateImageFromChannel(vx_image image, vx_enum channel)
{
    vx_image_s* subimage = NULL;

    gcmHEADER_ARG("image=%p, channel=0x%x", image, channel);
    gcmDUMP_API("$VX vxCreateImageFromChannel: image=%p, channel=0x%x", image, channel);

    if (vxoImage_IsValid(image) == vx_true_e)
    {
        /* perhaps the parent hasn't been allocated yet? */
        if (vxoMemory_Allocate(image->base.context, &image->memory) == vx_true_e)
        {
            /* check for valid parameters */
            switch (channel)
            {
                case VX_CHANNEL_Y:
                {
                    if (VX_DF_IMAGE_YUV4 != image->format &&
                        VX_DF_IMAGE_IYUV != image->format &&
                        VX_DF_IMAGE_NV12 != image->format &&
                        VX_DF_IMAGE_NV21 != image->format)
                    {
                        vx_context context = vxGetContext((vx_reference)subimage);
                        subimage = (vx_image)vxoError_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
                        gcmFOOTER_ARG("subimage=%p", subimage);
                        return subimage;
                    }
                    break;
                }

                case VX_CHANNEL_U:
                case VX_CHANNEL_V:
                {
                    if (VX_DF_IMAGE_YUV4 != image->format &&
                        VX_DF_IMAGE_IYUV != image->format)
                    {
                        vx_context context = vxGetContext((vx_reference)subimage);
                        subimage = (vx_image)vxoError_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
                        gcmFOOTER_ARG("subimage=%p", subimage);
                        return subimage;
                    }
                    break;
                }

                default:
                {
                    vx_context context = vxGetContext((vx_reference)subimage);
                    subimage = (vx_image)vxoError_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
                    gcmFOOTER_ARG("subimage=%p", subimage);
                    return subimage;
                }
            }

            subimage = (vx_image)vxoReference_Create(image->base.context, VX_TYPE_IMAGE, VX_REF_EXTERNAL, &image->base.context->base);
            if (vxGetStatus((vx_reference)subimage) == VX_SUCCESS)
            {
                vx_uint32 p = 0;

                /* remember that the scope of the subimage is the parent image */
                subimage->base.scope = (vx_reference_s*)image;

                /* refer to our parent image and internally refcount it */
                subimage->parent = image;

                for (p = 0; p < VX_MAX_REF_COUNT; p++)
                {
                    if (image->subimages[p] == NULL)
                    {
                        image->subimages[p] = subimage;
                        break;
                    }
                }

                vxoReference_Increment(&image->base, VX_REF_INTERNAL);

                vxInfo("Creating SubImage from channel {%u}\n", channel);

                /* plane index */
                p = (VX_CHANNEL_Y == channel) ? 0 : ((VX_CHANNEL_U == channel) ? 1 : 2);

                switch (image->format)
                {
                    case VX_DF_IMAGE_YUV4:
                    {
                        /* setup the metadata */
                        subimage->format      = VX_DF_IMAGE_U8;
                        subimage->importType  = image->importType;
                        subimage->channelRange= image->channelRange;
                        subimage->colorSpace  = image->colorSpace;
                        subimage->width       = image->memory.dims[p][VX_DIM_X];
                        subimage->height      = image->memory.dims[p][VX_DIM_Y];
                        subimage->planeCount  = 1;
                        subimage->isUniform   = image->isUniform;

                        memset(&subimage->scales, 0, sizeof(image->scales));
                        memset(&subimage->memory, 0, sizeof(image->memory));

                        subimage->scales[0][VX_DIM_CHANNEL] = 1;
                        subimage->scales[0][VX_DIM_X] = 1;
                        subimage->scales[0][VX_DIM_Y] = 1;

                        subimage->bounds[0][VX_DIM_CHANNEL][VX_BOUND_START] = 0;
                        subimage->bounds[0][VX_DIM_CHANNEL][VX_BOUND_END]   = 1;
                        subimage->bounds[0][VX_DIM_X][VX_BOUND_START] = image->bounds[p][VX_DIM_X][VX_BOUND_START];
                        subimage->bounds[0][VX_DIM_X][VX_BOUND_END]   = image->bounds[p][VX_DIM_X][VX_BOUND_END];
                        subimage->bounds[0][VX_DIM_Y][VX_BOUND_START] = image->bounds[p][VX_DIM_Y][VX_BOUND_START];
                        subimage->bounds[0][VX_DIM_Y][VX_BOUND_END]   = image->bounds[p][VX_DIM_Y][VX_BOUND_END];

                        subimage->memory.dims[0][VX_DIM_CHANNEL] = image->memory.dims[p][VX_DIM_CHANNEL];
                        subimage->memory.dims[0][VX_DIM_X] = image->memory.dims[p][VX_DIM_X];
                        subimage->memory.dims[0][VX_DIM_Y] = image->memory.dims[p][VX_DIM_Y];

                        subimage->memory.strides[0][VX_DIM_CHANNEL] = image->memory.strides[p][VX_DIM_CHANNEL];
                        subimage->memory.strides[0][VX_DIM_X] = image->memory.strides[p][VX_DIM_X];
                        subimage->memory.strides[0][VX_DIM_Y] = image->memory.strides[p][VX_DIM_Y];

                        subimage->memory.logicals[0] = image->memory.logicals[p];
                        subimage->memory.planeCount  = 1;

                        vxCreateMutex(&subimage->memory.writeLocks[0]);

                        break;
                    }

                    case VX_DF_IMAGE_IYUV:
                    case VX_DF_IMAGE_NV12:
                    case VX_DF_IMAGE_NV21:
                    {
                        /* setup the metadata */
                        subimage->format      = VX_DF_IMAGE_U8;
                        subimage->importType  = image->importType;
                        subimage->channelRange= image->channelRange;
                        subimage->colorSpace  = image->colorSpace;
                        subimage->width       = image->memory.dims[p][VX_DIM_X];
                        subimage->height      = image->memory.dims[p][VX_DIM_Y];
                        subimage->planeCount  = 1;
                        subimage->isUniform   = image->isUniform;

                        memset(&subimage->scales, 0, sizeof(image->scales));
                        memset(&subimage->memory, 0, sizeof(image->memory));

                        subimage->scales[0][VX_DIM_CHANNEL] = 1;
                        subimage->scales[0][VX_DIM_X] = 1;
                        subimage->scales[0][VX_DIM_Y] = 1;

                        subimage->bounds[0][VX_DIM_CHANNEL][VX_BOUND_START] = 0;
                        subimage->bounds[0][VX_DIM_CHANNEL][VX_BOUND_END]   = 1;
                        subimage->bounds[0][VX_DIM_X][VX_BOUND_START] = image->bounds[p][VX_DIM_X][VX_BOUND_START];
                        subimage->bounds[0][VX_DIM_X][VX_BOUND_END]   = image->bounds[p][VX_DIM_X][VX_BOUND_END];
                        subimage->bounds[0][VX_DIM_Y][VX_BOUND_START] = image->bounds[p][VX_DIM_Y][VX_BOUND_START];
                        subimage->bounds[0][VX_DIM_Y][VX_BOUND_END]   = image->bounds[p][VX_DIM_Y][VX_BOUND_END];

                        subimage->memory.dims[0][VX_DIM_CHANNEL] = image->memory.dims[p][VX_DIM_CHANNEL];
                        subimage->memory.dims[0][VX_DIM_X] = image->memory.dims[p][VX_DIM_X];
                        subimage->memory.dims[0][VX_DIM_Y] = image->memory.dims[p][VX_DIM_Y];

                        subimage->memory.strides[0][VX_DIM_CHANNEL] = image->memory.strides[p][VX_DIM_CHANNEL];
                        subimage->memory.strides[0][VX_DIM_X] = image->memory.strides[p][VX_DIM_X];
                        subimage->memory.strides[0][VX_DIM_Y] = image->memory.strides[p][VX_DIM_Y];

                        subimage->memory.logicals[0] = image->memory.logicals[p];
                        subimage->memory.planeCount  = 1;

                        vxCreateMutex(&subimage->memory.writeLocks[0]);

                        break;
                    }
                }

                /* set inverted region untill the first write to image */
                subimage->region.start_x = subimage->width;
                subimage->region.start_y = subimage->height;
                subimage->region.end_x   = 0;
                subimage->region.end_y   = 0;

                vxoImage_Dump(subimage);
            }
            else
            {
                vxError("Child image failed to allocate!\n");
            }
        }
        else
        {
            vx_context context;
            vxError("Parent image failed to allocate!\n");
            context = vxGetContext((vx_reference)image);
            subimage = (vx_image)vxoError_GetErrorObject(context, VX_ERROR_NO_MEMORY);
        }

        subimage->useInternalMem = image->useInternalMem;
    }
    else
    {
        vx_context context = vxGetContext((vx_reference)subimage);
        subimage = (vx_image)vxoError_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
    }

    gcmFOOTER_ARG("subimage=%p", subimage);
    return (vx_image)subimage;
}

VX_INTERNAL_API vx_image VX_API_CALL vxoImage_CreateImageFromInternalHandle(
        vx_context context, vx_df_image format, vx_imagepatch_addressing_t *addrs,
        void **ptrs, vx_uint32 *phys)
{
    vx_image    image;
    vx_uint32   planeIndex = 0, dimIndex;

    gcmHEADER_ARG("context=%p, format=%p, addrs=%p, ptrs=%p, phys=%p", context, format, addrs, ptrs, phys);

    if (!vxoContext_IsValid(context))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    if (format == VX_DF_IMAGE_VIRT)
    {
        gcmFOOTER_NO();
        return (vx_image)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_FORMAT);
    }

    if (addrs == VX_NULL || addrs[0].dim_x == 0 || addrs[0].dim_y == 0)
    {
        gcmFOOTER_NO();
        return (vx_image)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_DIMENSION);
    }

    if (addrs[0].dim_x > OVX_IMAGE_HW_MAX_WIDTH || addrs[0].dim_y > OVX_IMAGE_HW_MAX_HEIGHT)
    {
        vxError("Due to the HW limitation, the width or height of image can not exceed 65535.\n");
        return (vx_image)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_DIMENSION);
    }

    image = vxoImage_Create(context, addrs[0].dim_x, addrs[0].dim_y, format, vx_false_e);

    if (vxoReference_GetStatus((vx_reference)image) != VX_SUCCESS)
    {
        gcmFOOTER_ARG("image=%p", image);
        return image;
    }
    image->importType       = VX_MEMORY_TYPE_INTERNAL;

    if (image->planeCount > 1) goto OnError;

    {
        vx_uint32 size = sizeof(vx_uint8);

        image->memory.logicals[planeIndex]                  = (vx_uint8_ptr)ptrs[planeIndex];
        image->memory.physicals[planeIndex]                 = phys[planeIndex];
        image->memory.strides[planeIndex][VX_DIM_CHANNEL]   = (vx_uint32)vxImageFormat_GetChannelSize(format);
        image->memory.strides[planeIndex][VX_DIM_X]         = addrs[planeIndex].stride_x;
        image->memory.strides[planeIndex][VX_DIM_Y]         = addrs[planeIndex].stride_y;
        image->memory.wrappedSize[planeIndex]               = (gctUINT32)size;

        if (image->memory.strides[planeIndex][VX_DIM_CHANNEL] != 0)
        {
            size = (vx_uint32)abs(image->memory.strides[planeIndex][VX_DIM_CHANNEL]);
        }

        for (dimIndex = 0; dimIndex < image->memory.dimCount; dimIndex++)
        {
            image->memory.strides[planeIndex][dimIndex] = size;
            size *= (vx_uint32)abs(image->memory.dims[planeIndex][dimIndex]);
        }

        image->memory.wrappedSize[planeIndex] = (vx_uint32)size;

        if (!vxCreateMutex(OUT &image->memory.writeLocks[planeIndex]))
        {
            goto OnError;
        }
    }

    image->memory.allocated = vx_true_e;
    gcmFOOTER_ARG("image=%p", image);
    return image;

OnError:
    if (image->memory.writeLocks[planeIndex] != VX_NULL)
    {
        vxDestroyMutex(image->memory.writeLocks[planeIndex]);
        image->memory.writeLocks[planeIndex]  = VX_NULL;
    }

    vxReleaseImage(&image);
    gcmFOOTER_NO();
    return (vx_image)vxoContext_GetErrorObject(context, VX_ERROR_NO_RESOURCES);
}

VX_API_ENTRY vx_image VX_API_CALL vxCreateImageFromHandle(
        vx_context context, vx_df_image format, const vx_imagepatch_addressing_t addrs[],
        void * const ptrs[], vx_enum import_type)
{
    vx_image    image;
    vx_uint32   planeIndex;

    gcmHEADER_ARG("context=%p, format=%p, addrs=%p, ptrs=%p, import_type=0x%x", context, format, addrs, ptrs, import_type);
    gcmDUMP_API("$VX vxCreateImageFromHandle: context=%p, format=%p, addrs=%p, ptrs=%p, import_type=0x%x", context, format, addrs, ptrs, import_type);

    if (!vxoContext_IsValid(context))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    if (!vxIsValidImportType(import_type))
    {
        gcmFOOTER_NO();
        return (vx_image)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_FORMAT);
    }

    if (format == VX_DF_IMAGE_VIRT)
    {
        gcmFOOTER_NO();
        return (vx_image)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_FORMAT);
    }

    if (addrs == VX_NULL || addrs[0].dim_x == 0 || addrs[0].dim_y == 0)
    {
        gcmFOOTER_NO();
        return (vx_image)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_DIMENSION);
    }

    if (addrs[0].dim_x > OVX_IMAGE_HW_MAX_WIDTH || addrs[0].dim_y > OVX_IMAGE_HW_MAX_HEIGHT)
    {
        vxError("Due to the HW limitation, the width or height of image can not exceed 65535.\n");
        return (vx_image)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_DIMENSION);
    }

    image = vxoImage_Create(context, addrs[0].dim_x, addrs[0].dim_y, format, vx_false_e);

    if (vxoReference_GetStatus((vx_reference)image) != VX_SUCCESS)
    {
        gcmFOOTER_ARG("image=%p", image);
        return image;
    }
    image->importType       = import_type;

    if (import_type == VX_MEMORY_TYPE_HOST)
    {
        image->memory.wrapFlag  = gcvALLOC_FLAG_USERMEMORY;
        image->memory.isDirty = vx_true_e;
    }
    else if(import_type == VX_MEMORY_TYPE_DMABUF)
    {
        image->memory.wrapFlag  = gcvALLOC_FLAG_DMABUF;
    }

    for (planeIndex = 0; planeIndex < image->planeCount; planeIndex++)
    {
        image->memory.logicals[planeIndex]                  = (vx_uint8_ptr)ptrs[planeIndex];
        image->memory.strides[planeIndex][VX_DIM_CHANNEL]   = (vx_uint32)vxImageFormat_GetChannelSize(format);
        image->memory.strides[planeIndex][VX_DIM_X]         = addrs[planeIndex].stride_x;
        image->memory.strides[planeIndex][VX_DIM_Y]         = addrs[planeIndex].stride_y;
    }

    if (!vxoImage_WrapUserMemory(image)) goto OnError;

    gcmFOOTER_ARG("image=%p", image);
    return image;

OnError:
    vxReleaseImage(&image);
    gcmFOOTER_NO();
    return (vx_image)vxoContext_GetErrorObject(context, VX_ERROR_NO_RESOURCES);
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseImage(vx_image *image)
{
    gcmHEADER_ARG("image=%p", image);
    gcmDUMP_API("$VX vxReleaseImage: image=%p", image);

    if(image != NULL)
    {
        vx_image self = image[0];
        if(vxoImage_IsValid(self))
        {
            if(vxoImage_IsValid(self->parent))
            {
                vx_uint32 i = 0;
                for (i = 0; i < VX_MAX_REF_COUNT; i++)
                {
                    if (self->parent->subimages[i] == self)
                    {
                        self->parent->subimages[i] = 0;
                        break;
                    }
                }
            }
        }
    }
    gcmFOOTER_NO();
    return vxoReference_Release((vx_reference_ptr)image, VX_TYPE_IMAGE, VX_REF_EXTERNAL);
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryImage(vx_image image, vx_enum attribute, void *ptr, vx_size size)
{
    vx_size     imageSize = 0;
    vx_uint32   planeIndex;

    gcmHEADER_ARG("image=%p, attribute=0x%x, ptr=%p, size=0x%lx", image, attribute, ptr, size);
    gcmDUMP_API("$VX vxQueryImage: image=%p, attribute=0x%x, ptr=%p, size=0x%lx", image, attribute, ptr, size);

    if (!vxoImage_IsValid(image))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }
    switch (attribute)
    {
        case VX_IMAGE_FORMAT:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_df_image, 0x3);

            *(vx_df_image *)ptr = image->format;
            break;

        case VX_IMAGE_WIDTH:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            *(vx_uint32 *)ptr = image->width;
            break;

        case VX_IMAGE_HEIGHT:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            *(vx_uint32 *)ptr = image->height;
            break;

        case VX_IMAGE_PLANES:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size *)ptr = image->planeCount;
            break;

        case VX_IMAGE_SPACE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);

            *(vx_enum *)ptr = image->colorSpace;
            break;

        case VX_IMAGE_RANGE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);

            *(vx_enum *)ptr = image->channelRange;
            break;

        case VX_IMAGE_SIZE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            for (planeIndex = 0; planeIndex < image->planeCount; planeIndex++)
            {
                imageSize += abs(image->memory.strides[planeIndex][VX_DIM_Y]) * image->memory.dims[planeIndex][VX_DIM_Y];
            }

            *(vx_size *)ptr = imageSize;
            break;
        case VX_IMAGE_MEMORY_TYPE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);

            *(vx_enum *)ptr = image->importType;
            break;

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            gcmFOOTER_ARG("%d", VX_ERROR_NOT_SUPPORTED);
            return VX_ERROR_NOT_SUPPORTED;
    }

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetImageAttribute(vx_image image, vx_enum attribute, const void *ptr, vx_size size)
{
    gcmHEADER_ARG("image=%p, attribute=0x%x, ptr=%p, size=0x%lx", image, attribute, ptr, size);
    gcmDUMP_API("$VX vxSetImageAttribute: image=%p, attribute=0x%x, ptr=%p, size=0x%lx", image, attribute, ptr, size);

    if (!vxoImage_IsValid(image))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }
    switch (attribute)
    {
        case VX_IMAGE_SPACE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);

            image->colorSpace = *(vx_enum *)ptr;
            break;

        case VX_IMAGE_RANGE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);

            image->channelRange = *(vx_enum *)ptr;
            break;

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            gcmFOOTER_ARG("%d", VX_ERROR_NOT_SUPPORTED);
            return VX_ERROR_NOT_SUPPORTED;
    }

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_API_ENTRY vx_size VX_API_CALL vxComputeImagePatchSize(
        vx_image image, const vx_rectangle_t *rect, vx_uint32 plane_index)
{
    vx_size pixelCount, pixelSize;

    gcmHEADER_ARG("image=%p, rect=%p, plane_index=0x%x", image, rect, plane_index);
    gcmDUMP_API("$VX vxComputeImagePatchSize: image=%p, rect=%p, plane_index=0x%x", image, rect, plane_index);

    if (!vxoImage_IsValid(image))
    {
        gcmFOOTER_ARG("%d", 0);
        return 0;
    }
    if (rect == VX_NULL)
    {
        gcmFOOTER_ARG("%d", 0);
        return 0;
    }
    if (image->memory.logicals[0] == VX_NULL)
    {
        if (!vxoImage_AllocateMemory(image))
        {
            gcmFOOTER_ARG("%d", 0);
            return 0;
        }
    }

    if (plane_index >= image->planeCount)
    {
        gcmFOOTER_ARG("%d", 0);
        return 0;
    }
    pixelCount  = ((rect->end_x - rect->start_x) / image->scales[plane_index][VX_DIM_X])
                    * ((rect->end_y - rect->start_y) / image->scales[plane_index][VX_DIM_Y]);

    pixelSize   = image->memory.strides[plane_index][VX_DIM_X];

    gcmFOOTER_NO();
    return pixelCount * pixelSize;
}

VX_INTERNAL_ENTRY vx_size VX_API_CALL vxComputeWholeImageSize(
        vx_image image, const vx_rectangle_t *rect, vx_uint32 plane_index)
{
    if (!vxoImage_IsValid(image)) return 0;

    if (rect == VX_NULL) return 0;

    if (image->memory.logicals[0] == VX_NULL)
    {
        if (!vxoImage_AllocateMemory(image)) return 0;
    }

    if (plane_index >= image->planeCount) return 0;

    return image->memory.strides[plane_index][VX_DIM_Y] * image->memory.dims[plane_index][VX_DIM_Y] - rect->start_x * image->memory.strides[plane_index][VX_DIM_X];
}

VX_API_ENTRY vx_status VX_API_CALL vxAccessImagePatch(
        vx_image image, const vx_rectangle_t *rect, vx_uint32 plane_index,
        vx_imagepatch_addressing_t *patchAddr, void **ptr, vx_enum usage)
{
    vx_bool         mapped = vx_false_e;
    vx_uint8_ptr    memoryPtr;
    gcmHEADER_ARG("image=%p, rect=%p, plane_index=0x%x, patchAddr=%p, ptr=%p, usage=0x%x", image, rect, plane_index, patchAddr, ptr, usage);
    gcmDUMP_API("$VX vxAccessImagePatch: image=%p, rect=%p, plane_index=0x%x, patchAddr=%p, ptr=%p, usage=0x%x", image, rect, plane_index, patchAddr, ptr, usage);

    if (!vxoImage_IsValid(image))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }
    if (rect == VX_NULL)
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
        return VX_ERROR_INVALID_PARAMETERS;
    }
    if (patchAddr == VX_NULL || ptr == VX_NULL)
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
        return VX_ERROR_INVALID_PARAMETERS;
    }
    if (usage < VX_READ_ONLY || VX_READ_AND_WRITE < usage)
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
        return VX_ERROR_INVALID_PARAMETERS;
    }
    if (image->base.isVirtual && !image->base.accessible)
    {
        gcmFOOTER_ARG("%d", VX_ERROR_OPTIMIZED_AWAY);
        return VX_ERROR_OPTIMIZED_AWAY;
    }
    if (rect->start_x >= rect->end_x || rect->start_y >= rect->end_y)
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
        return VX_ERROR_INVALID_PARAMETERS;
    }
    if (plane_index >= image->planeCount || plane_index >= image->memory.planeCount)
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
        return VX_ERROR_INVALID_PARAMETERS;
    }
    if (image->memory.logicals[0] == VX_NULL)
    {
        if (!vxoImage_AllocateMemory(image))
        {
            gcmFOOTER_ARG("%d", VX_ERROR_NO_MEMORY);
            return VX_ERROR_NO_MEMORY;
        }
    }

    if (image->isUniform)
    {
        if (usage == VX_WRITE_ONLY || usage == VX_READ_AND_WRITE)
        {
            gcmFOOTER_ARG("%d", VX_ERROR_NOT_SUPPORTED);
            return VX_ERROR_NOT_SUPPORTED;
        }
    }

    if (*ptr == VX_NULL)
    {
        mapped = (vx_bool)(usage == VX_READ_ONLY || usage == VX_WRITE_ONLY || usage == VX_READ_AND_WRITE);
    }

    if (mapped)
    {
        if (usage != VX_READ_ONLY)
        {
            if (!vxAcquireMutex(image->memory.writeLocks[plane_index]))
            {
                gcmFOOTER_ARG("%d", VX_ERROR_NO_RESOURCES);
                return VX_ERROR_NO_RESOURCES;
            }
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

        vx_imagepatch_addressing_t *addrSave = (vx_imagepatch_addressing_t *)vxAllocate(sizeof(vx_imagepatch_addressing_t));
        addrSave->stride_x = patchAddr->stride_x;
        addrSave->stride_y = patchAddr->stride_y;

        if (!vxoContext_AddAccessor(image->base.context, vxComputeImagePatchSize(image, rect, plane_index),
                                    usage, *ptr, &image->base, OUT &accessorIndex, addrSave))
        {
            gcmFOOTER_ARG("%d", VX_ERROR_NO_MEMORY);
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
            if (!vxAcquireMutex(image->memory.writeLocks[plane_index]))
            {
                gcmFOOTER_ARG("%d", VX_ERROR_NO_RESOURCES);
                return VX_ERROR_NO_RESOURCES;
            }
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

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxCommitImagePatch(
        vx_image image, const vx_rectangle_t *rect, vx_uint32 plane_index,
        const vx_imagepatch_addressing_t *patchAddr, const void *ptr)
{
    vx_bool     isZeroRect = vx_true_e;
    vx_bool     foundAccessor;
    vx_uint32   accessorIndex;

    gcmHEADER_ARG("image=%p, rect=%p, plane_index=0x%x, patchAddr=%p, ptr=%p", image, rect, plane_index, patchAddr, ptr);
    gcmDUMP_API("$VX vxCommitImagePatch: image=%p, rect=%p, plane_index=0x%x, patchAddr=%p, ptr=%p", image, rect, plane_index, patchAddr, ptr);

    if (!vxoImage_IsValid(image))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }
    if (rect != VX_NULL)
    {
        isZeroRect = (vx_bool)(rect->end_x - rect->start_x == 0 || rect->end_y - rect->start_y == 0);
    }

    if (image->base.isVirtual && !isZeroRect)
    {
        if (!image->base.accessible)
        {
            gcmFOOTER_ARG("%d", VX_ERROR_OPTIMIZED_AWAY);
            return VX_ERROR_OPTIMIZED_AWAY;
        }
    }

    if (!isZeroRect)
    {
        vxmASSERT(rect);

        if (plane_index >= image->planeCount || plane_index >= image->memory.planeCount)
        {
            gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
            return VX_ERROR_INVALID_PARAMETERS;
        }
        if (patchAddr == VX_NULL || ptr == VX_NULL)
        {
            gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
            return VX_ERROR_INVALID_PARAMETERS;
        }
        if (rect->start_x >= rect->end_x || rect->start_y >= rect->end_y)
        {
            gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
            return VX_ERROR_INVALID_PARAMETERS;
        }
        if (rect->end_x - rect->start_x > patchAddr->dim_x)
        {
            gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
            return VX_ERROR_INVALID_PARAMETERS;
        }
        if (rect->end_y - rect->start_y > patchAddr->dim_y)
        {
            gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
            return VX_ERROR_INVALID_PARAMETERS;
        }
        if (rect->end_x > (vx_uint32)image->memory.dims[plane_index][VX_DIM_X]
                            * (vx_uint32)image->scales[plane_index][VX_DIM_X])
        {
            gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
            return VX_ERROR_INVALID_PARAMETERS;
        }

        if (rect->end_y > (vx_uint32)image->memory.dims[plane_index][VX_DIM_Y]
                            * (vx_uint32)image->scales[plane_index][VX_DIM_Y])
        {
            gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
            return VX_ERROR_INVALID_PARAMETERS;
        }
    }

    foundAccessor = vxoContext_SearchAccessor(image->base.context, (vx_ptr)ptr, &accessorIndex);

    if (!isZeroRect)
    {
        vxmASSERT(rect);

        if (image->isUniform)
        {
            gcmFOOTER_ARG("%d", VX_ERROR_NOT_SUPPORTED);
            return VX_ERROR_NOT_SUPPORTED;
        }
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

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxCopyImagePatch(
    vx_image image,
    const vx_rectangle_t* rect,
    vx_uint32 plane_index,
    const vx_imagepatch_addressing_t* addr,
    void* ptr,
    vx_enum usage,
    vx_enum mem_type)
{
    vx_status status = VX_FAILURE;
    vx_uint32 start_x = rect ? rect->start_x : 0u;
    vx_uint32 start_y = rect ? rect->start_y : 0u;
    vx_uint32 end_x = rect ? rect->end_x : 0u;
    vx_uint32 end_y = rect ? rect->end_y : 0u;
    vx_bool zero_area = ((((end_x - start_x) == 0) || ((end_y - start_y) == 0)) ? vx_true_e : vx_false_e);

    gcmHEADER_ARG("image=%p, rect=%p, plane_index=0x%x, addr=%p, ptr=%p, usage=0x%x, mem_type=0x%x", image, rect, plane_index, addr, ptr, usage, mem_type);
    gcmDUMP_API("$VX vxCopyImagePatch: image=%p, rect=%p, plane_index=0x%x, addr=%p, ptr=%p, usage=0x%x, mem_type=0x%x",
        image, rect, plane_index, addr, ptr, usage, mem_type);

    /* bad parameters */
    if (((VX_READ_ONLY != usage) && (VX_WRITE_ONLY != usage)) ||
         (rect == NULL) || (addr == NULL) || (ptr == NULL) )
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        goto exit;
    }

    /* bad references */
    if (!vxoImage_IsValid(image))
    {
        status = VX_ERROR_INVALID_REFERENCE;
        goto exit;
    }

    /* determine if virtual before checking for memory */
    if (image->base.isVirtual == vx_true_e)
    {
        if (image->base.accessible == vx_false_e)
        {
            /* User tried to access a "virtual" image. */
            vxError("Can not access a virtual image\n");
            status = VX_ERROR_OPTIMIZED_AWAY;
            goto exit;
        }
        /* framework trying to access a virtual image, this is ok. */
    }

    /* more bad parameters */
    if (zero_area == vx_false_e &&
        (/*(plane_index >= image->memory.nptrs) ||*/
        (plane_index >= image->planeCount) ||
         (start_x >= end_x) ||
         (start_y >= end_y)))
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        goto exit;
    }

    /* verify has not been call yet or will not be called (immediate mode)
    * this is a trick to "touch" an image so that it can be created
    */
    if ((image->memory.logicals[0] == NULL) && (vxoImage_AllocateMemory(image) == vx_false_e))
    {
        vxError("No memory1!\n");
        status = VX_ERROR_NO_MEMORY;
        goto exit;
    }

    /* can't write to constant */
    if ((image->isUniform == vx_true_e) && (usage == VX_WRITE_ONLY) )
    {
        status = VX_ERROR_NOT_SUPPORTED;
        vxError("Can't write to constant data, only read!\n");
        vxAddLogEntry(&image->base, status, "Can't write to constant data, only read!\n");
        goto exit;
    }

    /*************************************************************************/
    vxInfo("CopyImagePatch from "VX_FMT_REF" to ptr %p from {%u,%u} to {%u,%u} plane %u\n",
        image, ptr, start_x, start_y, end_x, end_y, plane_index);

    if (usage == VX_READ_ONLY)
    {
        /* Copy from image (READ) mode */

        vx_uint32 x;
        vx_uint32 y;
        vx_uint8* pSrc;
        vx_uint8* pDst = (vx_uint8*)ptr;
        vx_imagepatch_addressing_t addr_save = VX_IMAGEPATCH_ADDR_INIT;

        if (image->useInternalMem && image->memory.nodePtrs[plane_index] != VX_NULL && image->memory.logicals[plane_index] != image->memory.nodePtrs[plane_index]->logical)
        {
            pSrc = image->memory.nodePtrs[plane_index]->logical;
        }
        else
        {
            pSrc = image->memory.logicals[plane_index];
        }

        /* Strides given by the application */
        addr_save.dim_x    = addr->dim_x;
        addr_save.dim_y    = addr->dim_y;
        addr_save.stride_x = addr->stride_x;
        addr_save.stride_y = addr->stride_y;

        addr_save.step_x  = image->scales[plane_index][VX_DIM_X];
        addr_save.step_y  = image->scales[plane_index][VX_DIM_Y];
        addr_save.scale_x = VX_SCALE_UNITY / image->scales[plane_index][VX_DIM_X];
        addr_save.scale_y = VX_SCALE_UNITY / image->scales[plane_index][VX_DIM_Y];

        /* copy the patch from the image */
        if (addr_save.stride_x == image->memory.strides[plane_index][VX_DIM_X])
        {
            /* Both have compact lines */
            for (y = start_y; y < end_y; y += addr_save.step_y)
            {
                vx_uint32 srcOffset = vxComputePlaneOffset(image, start_x, y, plane_index);
                vx_uint8* pSrcLine = &pSrc[srcOffset];

                vx_uint32 dstOffset = vxComputePatchOffset(0, (y - start_y), &addr_save);
                vx_uint8* pDstLine = &pDst[dstOffset];

                vx_uint32 len = vxComputePlaneRangeSize(image, end_x - start_x/*image->width*/, plane_index);

                /* vxInfo("%s[%d]: %p[%u] <= %p[%u] for %u\n", __FILE__, __LINE__, pDst, dstOffset, pSrc, srcOffset, len); */

                memcpy(pDstLine, pSrcLine, len);
            }
        }
        else
        {
            /* The destination is not compact, we need to copy per element */
            for (y = start_y; y < end_y; y += addr_save.step_y)
            {
                vx_uint32 srcOffset = vxComputePlaneOffset(image, start_x, y, plane_index);
                vx_uint8* pSrcLine = &pSrc[srcOffset];

                vx_uint8* pDstLine = pDst;

                vx_uint32 len = image->memory.strides[plane_index][VX_DIM_X];

                for (x = start_x; x < end_x; x += addr_save.step_x)
                {
                    /* One element */
                    memcpy(pDstLine, pSrcLine, len);

                    pSrcLine += len;
                    pDstLine += addr_save.stride_x;
                }

                pDst += addr_save.stride_y;
            }
        }

        vxInfo("Copied image into %p\n", ptr);

        vxoReference_IncrementReadCount(&image->base);
    }
    else
    {
        /* Copy to image (WRITE) mode */
        vx_uint32 x;
        vx_uint32 y;
        vx_uint8* pSrc = (vx_uint8*)ptr;
        vx_uint8* pDst;
        vx_imagepatch_addressing_t addr_save = VX_IMAGEPATCH_ADDR_INIT;

        if (image->useInternalMem && image->memory.nodePtrs[plane_index] != VX_NULL && image->memory.logicals[plane_index] != image->memory.nodePtrs[plane_index]->logical)
        {
            pDst = image->memory.nodePtrs[plane_index]->logical;
        }
        else
        {
            pDst = image->memory.logicals[plane_index];
        }

        /* Strides given by the application */
        addr_save.dim_x    = addr->dim_x;
        addr_save.dim_y    = addr->dim_y;
        addr_save.stride_x = addr->stride_x;
        addr_save.stride_y = addr->stride_y;

        addr_save.step_x  = image->scales[plane_index][VX_DIM_X];
        addr_save.step_y  = image->scales[plane_index][VX_DIM_Y];
        addr_save.scale_x = VX_SCALE_UNITY / image->scales[plane_index][VX_DIM_X];
        addr_save.scale_y = VX_SCALE_UNITY / image->scales[plane_index][VX_DIM_Y];

        /* lock image plane from multiple writers */
        if (vxAcquireMutex(image->memory.writeLocks[plane_index]) == vx_false_e)
        {
            status = VX_ERROR_NO_RESOURCES;
            goto exit;
        }

        /* copy the patch to the image */
        if (addr_save.stride_x == image->memory.strides[plane_index][VX_DIM_X])
        {
            /* Both source and destination have compact lines */
            for (y = start_y; y < end_y; y += addr_save.step_y)
            {
                vx_uint32 srcOffset = vxComputePatchOffset(0, (y - start_y), &addr_save);
                vx_uint8* pSrcLine = &pSrc[srcOffset];

                vx_uint32 dstOffset = vxComputePlaneOffset(image, start_x, y, plane_index);
                vx_uint8* pDstLine = &pDst[dstOffset];

                vx_uint32 len = vxComputePatchRangeSize((end_x - start_x), &addr_save);

                /* vxInfo("%s[%d]: %p[%u] <= %p[%u] for %u\n", __FILE__, __LINE__, pDst, srcOffset, pSrc, dstOffset, len); */

                memcpy(pDstLine, pSrcLine, len);
            }
        }
        else
        {
            /* The destination is not compact, we need to copy per element */
            for (y = start_y; y < end_y; y += addr_save.step_y)
            {
                vx_uint8* pSrcLine = pSrc;

                vx_uint32 dstOffset = vxComputePlaneOffset(image, start_x, y, plane_index);
                vx_uint8* pDstLine = &pDst[dstOffset];

                vx_uint32 len = image->memory.strides[plane_index][VX_DIM_X];

                for (x = start_x; x < end_x; x += addr_save.step_x)
                {
                    /* One element */
                    memcpy(pDstLine, pSrcLine, len);

                    pSrcLine += len;
                    pDstLine += addr_save.stride_x;
                }

                pDst += addr_save.stride_y;
            }
        }

        vxInfo("Copied to image from %p\n", ptr);

        vxoReference_IncrementWriteCount(&image->base);

#if gcdDUMP
        {
            vx_uint32 size = image->memory.strides[plane_index][VX_DIM_Y] * image->memory.dims[plane_index][VX_DIM_Y];

            gcmDUMP(gcvNULL, "#[input]\n");
            gcmDUMP_BUFFER(gcvNULL,
                           gcvDUMP_BUFFER_MEMORY,
                           image->memory.physicals[plane_index],
                           image->memory.logicals[plane_index],
                           0,
                           size);
        }
#endif

        /* unlock image plane */
        vxReleaseMutex(image->memory.writeLocks[plane_index]);
    }

    status = VX_SUCCESS;

exit:
    gcmFOOTER_ARG("%d", status);
    return status;
} /* vxCopyImagePatch() */

#if MAP_UNMAP_REFERENCE
VX_API_ENTRY vx_status VX_API_CALL vxMapImagePatch(
    vx_image image,
    const vx_rectangle_t* rect,
    vx_uint32 plane_index,
    vx_map_id* map_id,
    vx_imagepatch_addressing_t* addr,
    void** ptr,
    vx_enum usage,
    vx_enum mem_type,
    vx_uint32 flags)
{
    vx_uint32 start_x = rect ? rect->start_x : 0u;
    vx_uint32 start_y = rect ? rect->start_y : 0u;
    vx_uint32 end_x   = rect ? rect->end_x : 0u;
    vx_uint32 end_y   = rect ? rect->end_y : 0u;
    vx_bool zero_area = ((((end_x - start_x) == 0) || ((end_y - start_y) == 0)) ? vx_true_e : vx_false_e);
    vx_status status = VX_FAILURE;

    gcmDUMP_API("$VX vxMapImagePatch: image=%p, rect=%p, plane_index=0x%x, map_id=%p, addr=%p, ptr=%p, usage=0x%x, mem_type=0x%x, flags=0x%x", image, rect, plane_index, map_id, addr, ptr, usage, mem_type, flags);
    /* bad parameters */
    if ((rect == NULL) || (map_id == NULL) || (addr == NULL) || (ptr == NULL) )
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        goto exit;
    }

    /* bad references */
    if (!vxoImage_IsValid(image))
    {
        status = VX_ERROR_INVALID_REFERENCE;
        goto exit;
    }

    /* determine if virtual before checking for memory */
    if (image->base.isVirtual == vx_true_e)
    {
        if (image->base.accessible == vx_false_e)
        {
            /* User tried to access a "virtual" image. */
            vxError("Can not access a virtual image\n");
            status = VX_ERROR_OPTIMIZED_AWAY;
            goto exit;
        }
        /* framework trying to access a virtual image, this is ok. */
    }

    /* more bad parameters */
    if (zero_area == vx_false_e &&
        (/*(plane_index >= image->memory.nptrs) ||*/
        (plane_index >= image->planeCount) ||
        (start_x >= end_x) ||
        (start_y >= end_y)))
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        goto exit;
    }

    /* verify has not been call yet or will not be called (immediate mode)
    * this is a trick to "touch" an image so that it can be created
    */
    if ((image->memory.logicals[0] == NULL) && (vxoImage_AllocateMemory(image) == vx_false_e))
    {
        vxError("No memory!\n");
        status = VX_ERROR_NO_MEMORY;
        goto exit;
    }

    /* can't write to constant */
    if ((image->isUniform == vx_true_e) &&
        ((usage == VX_WRITE_ONLY) || (usage == VX_READ_AND_WRITE)))
    {
        status = VX_ERROR_NOT_SUPPORTED;
        vxError("Can't write to constant data, only read!\n");
        vxAddLogEntry(&image->base, status, "Can't write to constant data, only read!\n");
        goto exit;
    }

    /*************************************************************************/
    vxInfo("MapImagePatch from "VX_FMT_REF" to ptr %p from {%u,%u} to {%u,%u} plane %u\n",
        image, *ptr, start_x, start_y, end_x, end_y, plane_index);

    /* MAP mode */
    {
        vx_size size;
        vx_memory_map_extra_s extra;
        vx_uint8* buf = 0;

        size = vxComputeImagePatchSize(image, rect, plane_index);

        extra.image_data.plane_index = plane_index;
        extra.image_data.rect        = *rect;

        if (VX_MEMORY_TYPE_HOST == image->importType && vx_true_e == vxoContext_MemoryMap(image->base.context, (vx_reference)image, 0, usage, mem_type, flags, &extra, (void**)&buf, map_id))
        {
            /* use the addressing of the internal format */
            addr->dim_x    = end_x - start_x;
            addr->dim_y    = end_y - start_y;
            addr->stride_x = image->memory.strides[plane_index][VX_DIM_X];
            addr->stride_y = image->memory.strides[plane_index][VX_DIM_Y];
            addr->step_x   = image->scales[plane_index][VX_DIM_X];
            addr->step_y   = image->scales[plane_index][VX_DIM_Y];
            addr->scale_x  = VX_SCALE_UNITY / image->scales[plane_index][VX_DIM_X];
            addr->scale_y  = VX_SCALE_UNITY / image->scales[plane_index][VX_DIM_Y];

            buf = image->memory.logicals[plane_index];

            *ptr = buf;
            vxInfo("Returning mapped pointer %p\n", *ptr);

            vxoReference_Increment(&image->base, VX_REF_EXTERNAL);

            status = VX_SUCCESS;
        }
        else
        /* get mapping buffer of sufficient size and map_id */
        if (vx_true_e == vxoContext_MemoryMap(image->base.context, (vx_reference)image, size, usage, mem_type, flags, &extra, (void**)&buf, map_id))
        {
            /* use the addressing of the internal format */
            addr->dim_x    = end_x - start_x;
            addr->dim_y    = end_y - start_y;
            addr->stride_x = image->memory.strides[plane_index][VX_DIM_X];
            addr->stride_y = addr->stride_x * addr->dim_x / image->scales[plane_index][VX_DIM_Y];/*image->memory.strides[plane_index][VX_DIM_Y];*/
            addr->step_x   = image->scales[plane_index][VX_DIM_X];
            addr->step_y   = image->scales[plane_index][VX_DIM_Y];
            addr->scale_x  = VX_SCALE_UNITY / image->scales[plane_index][VX_DIM_X];
            addr->scale_y  = VX_SCALE_UNITY / image->scales[plane_index][VX_DIM_Y];

            /* initialize mapping buffer with image patch data for read/read-modify-write access */
            if (VX_READ_ONLY == usage || VX_READ_AND_WRITE == usage)
            {
                /* lock image plane even for read access to avoid mix of simultaneous read/write */
                if (vx_true_e == vxAcquireMutex(image->memory.writeLocks[plane_index]))
                {
                    vx_uint32 y;
                    vx_uint8* pSrc = image->memory.logicals[plane_index];
                    vx_uint8* pDst = buf;

                    /* Both have compact lines */
                    for (y = start_y; y < end_y; y += addr->step_y)
                    {
                        vx_uint32 srcOffset = vxComputePlaneOffset(image, start_x, y, plane_index);
                        vx_uint8* pSrcLine = &pSrc[srcOffset];

                        vx_uint32 dstOffset = vxComputePatchOffset(0, (y - start_y), addr);
                        vx_uint8* pDstLine = &pDst[dstOffset];

                        vx_uint32 len = vxComputePlaneRangeSize(image, addr->dim_x, plane_index);

                        /*vxError("%s[%d]: %p[%u] <= %p[%u] for %u\n", __FILE__, __LINE__, pDst, dstOffset, pSrc, srcOffset, len);*/

                        memcpy(pDstLine, pSrcLine, len);
                    }

                    vxoReference_IncrementReadCount(&image->base);

                    status = VX_SUCCESS;

                    /* we're done, unlock the image plane */
                    vxReleaseMutex(image->memory.writeLocks[plane_index]);
                }
                else
                {
                    status = VX_FAILURE;
                    vxError("Can't lock memory plane for mapping\n");
                    goto exit;
                }
            }

            *ptr = buf;
            vxInfo("Returning mapped pointer %p\n", *ptr);

            vxoReference_Increment(&image->base, VX_REF_EXTERNAL);

            status = VX_SUCCESS;
        }
        else
            status = VX_FAILURE;
    }

exit:
    return status;
} /* vxMapImagePatch() */

VX_API_ENTRY vx_status VX_API_CALL vxUnmapImagePatch(vx_image image, vx_map_id map_id)
{
    vx_status status = VX_FAILURE;

    gcmDUMP_API("$VX vxUnmapImagePatch: image=%p, map_id=%p", image, map_id);
    /* bad references */
    if (!vxoImage_IsValid(image))
    {
        status = VX_ERROR_INVALID_REFERENCE;
        goto exit;
    }

    /* bad parameters */
    if (vxoContext_FindMemoryMap(image->base.context, (vx_reference)image, map_id) != vx_true_e)
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        vxError("Invalid parameters to unmap image patch\n");
        return status;
    }

    {
        vx_context       context = image->base.context;
        vx_memory_map_s* map     = &context->memoryMaps[map_id];

        if (map->used &&
            map->ref == (vx_reference)image)
        {
            vx_uint32      plane_index = map->extra.image_data.plane_index;
            vx_rectangle_t rect = map->extra.image_data.rect;

            /* commit changes for write access */
            if (VX_WRITE_ONLY == map->usage || VX_READ_AND_WRITE == map->usage)
            {
                /* lock image plane for simultaneous write */
                if (vx_true_e == vxAcquireMutex(image->memory.writeLocks[plane_index]))
                {
                    vx_uint32 y;
                    vx_uint8* pSrc = (vx_uint8*)map->logical;
                    vx_uint8* pDst = image->memory.logicals[plane_index];
                    vx_imagepatch_addressing_t addr = VX_IMAGEPATCH_ADDR_INIT;

                    /* use the addressing of the internal format */
                    addr.dim_x    = rect.end_x - rect.start_x;
                    addr.dim_y    = rect.end_y - rect.start_y;
                    addr.stride_x = image->memory.strides[plane_index][VX_DIM_X];
                    addr.stride_y = image->memory.strides[plane_index][VX_DIM_Y];
                    addr.step_x   = image->scales[plane_index][VX_DIM_X];
                    addr.step_y   = image->scales[plane_index][VX_DIM_Y];
                    addr.scale_x  = VX_SCALE_UNITY / image->scales[plane_index][VX_DIM_X];
                    addr.scale_y  = VX_SCALE_UNITY / image->scales[plane_index][VX_DIM_Y];

                    /* Both source and destination have compact lines */
                    for (y = rect.start_y; y < rect.end_y; y += addr.step_y)
                    {
                        vx_uint32 srcOffset = vxComputePatchOffset(0, (y - rect.start_y), &addr);
                        vx_uint8* pSrcLine = &pSrc[srcOffset];

                        vx_uint32 dstOffset = vxComputePlaneOffset(image, rect.start_x, y, plane_index);
                        vx_uint8* pDstLine = &pDst[dstOffset];

                        vx_uint32 len = vxComputePatchRangeSize((rect.end_x - rect.start_x), &addr);

                        vxInfo("%s[%d]: %p[%u] <= %p[%u] for %u\n", __FILE__, __LINE__, pDst, srcOffset, pSrc, dstOffset, len);

                        if (pDstLine && pSrc)
                            memcpy(pDstLine, pSrcLine, len);
                        else
                            vxError("%s[%d]: invalid address: %p[%u] <= %p[%u] for %u\n", __FUNCTION__, __LINE__, pDst, srcOffset, pSrc, dstOffset, len);

                    }

                    /* we're done, unlock the image plane */
                    vxReleaseMutex(image->memory.writeLocks[plane_index]);
                }
                else
                {
                    status = VX_FAILURE;
                    vxError("Can't lock memory plane for unmapping\n");
                    goto exit;
                }
            }

            /* freeing mapping buffer */
            vxoContext_MemoryUnmap(context, map_id);

            vxoReference_Decrement(&image->base, VX_REF_EXTERNAL);

            status = VX_SUCCESS;
        }
        else
            status = VX_FAILURE;
    }

exit:
    vxError("return %d\n", status);

    return status;
} /* vxUnmapImagePat*/
#else
VX_API_ENTRY vx_status VX_API_CALL vxMapImagePatch(
    vx_image image,
    const vx_rectangle_t* rect,
    vx_uint32 plane_index,
    vx_map_id* map_id,
    vx_imagepatch_addressing_t* addr,
    void** ptr,
    vx_enum usage,
    vx_enum mem_type,
    vx_uint32 flags)
{
    vx_uint32 start_x = rect ? rect->start_x : 0u;
    vx_uint32 start_y = rect ? rect->start_y : 0u;
    vx_uint32 end_x   = rect ? rect->end_x : 0u;
    vx_uint32 end_y   = rect ? rect->end_y : 0u;
    vx_bool zero_area = ((((end_x - start_x) == 0) || ((end_y - start_y) == 0)) ? vx_true_e : vx_false_e);
    vx_status status = VX_FAILURE;

    gcmHEADER_ARG("image=%p, rect=%p, plane_index=0x%x, map_id=%p, addr=%p, ptr=%p, usage=0x%x, mem_type=0x%x, flags=0x%x",
        image, rect, plane_index, map_id, addr, ptr, usage, mem_type, flags);
    gcmDUMP_API("$VX vxMapImagePatch: image=%p, rect=%p, plane_index=0x%x, map_id=%p, addr=%p, ptr=%p, usage=0x%x, mem_type=0x%x, flags=0x%x",
        image, rect, plane_index, map_id, addr, ptr, usage, mem_type, flags);

    /* bad parameters */
    if ((rect == NULL) || (map_id == NULL) || (addr == NULL) || (ptr == NULL) )
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        goto exit;
    }

    /* bad references */
    if (!vxoImage_IsValid(image))
    {
        status = VX_ERROR_INVALID_REFERENCE;
        goto exit;
    }

    /* determine if virtual before checking for memory */
    if (image->base.isVirtual == vx_true_e)
    {
        if (image->base.accessible == vx_false_e)
        {
            /* User tried to access a "virtual" image. */
            vxError("Can not access a virtual image\n");
            status = VX_ERROR_OPTIMIZED_AWAY;
            goto exit;
        }
        /* framework trying to access a virtual image, this is ok. */
    }

    /* more bad parameters */
    if (zero_area == vx_false_e &&
        (/*(plane_index >= image->memory.nptrs) ||*/
        (plane_index >= image->planeCount) ||
        (start_x >= end_x) ||
        (start_y >= end_y) ||
        (end_x > image->width) ||
        (end_y > image->height)
        ))
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        goto exit;
    }

    /* verify has not been call yet or will not be called (immediate mode)
    * this is a trick to "touch" an image so that it can be created
    */
    if ((image->memory.logicals[0] == NULL) && (vxoImage_AllocateMemory(image) == vx_false_e))
    {
        vxError("No memory2!\n");
        status = VX_ERROR_NO_MEMORY;
        goto exit;
    }

    /* can't write to constant */
    if ((image->isUniform == vx_true_e) &&
        ((usage == VX_WRITE_ONLY) || (usage == VX_READ_AND_WRITE)))
    {
        status = VX_ERROR_NOT_SUPPORTED;
        vxError("Can't write to constant data, only read!\n");
        vxAddLogEntry(&image->base, status, "Can't write to constant data, only read!\n");
        goto exit;
    }

    /*************************************************************************/
    vxInfo("MapImagePatch from "VX_FMT_REF" to ptr %p from {%u,%u} to {%u,%u} plane %u\n",
        image, *ptr, start_x, start_y, end_x, end_y, plane_index);

    /* MAP mode */
    {
        vx_size size;
        vx_memory_map_extra_s extra;
        vx_uint8* buf = 0;

        size = vxComputeImagePatchSize(image, rect, plane_index);

        extra.image_data.plane_index = plane_index;
        extra.image_data.rect        = *rect;

        if (vx_true_e == vxoContext_MemoryMap(image->base.context, (vx_reference)image, size, usage, mem_type, flags, &extra, (void **)&buf, map_id))
        {
            /* use the addressing of the internal format */
            addr->dim_x    = end_x - start_x;
            addr->dim_y    = end_y - start_y;
            addr->stride_x = image->memory.strides[plane_index][VX_DIM_X];
            addr->stride_y = image->memory.strides[plane_index][VX_DIM_Y];
            addr->step_x   = image->scales[plane_index][VX_DIM_X];
            addr->step_y   = image->scales[plane_index][VX_DIM_Y];
            addr->scale_x  = VX_SCALE_UNITY / image->scales[plane_index][VX_DIM_X];
            addr->scale_y  = VX_SCALE_UNITY / image->scales[plane_index][VX_DIM_Y];
            *ptr = buf;
            vxoReference_Increment(&image->base, VX_REF_EXTERNAL);
            if((image->useInternalMem == vx_false_e) && ((usage == VX_READ_ONLY) || (usage == VX_READ_AND_WRITE)))
            {
                {
                    gcoOS_CacheInvalidate(gcvNULL, image->memory.wrappedNode[plane_index], image->memory.logicals[plane_index], image->memory.wrappedSize[plane_index]);
                }
            }
            status = VX_SUCCESS;
        }
        else
        {
            vxError("failed to map image\n");
            status = VX_FAILURE;
        }
    }

exit:
    gcmFOOTER_ARG("%d", status);
    return status;
} /* vxMapImagePatch() */

VX_API_ENTRY vx_status VX_API_CALL vxUnmapImagePatch(vx_image image, vx_map_id map_id)
{
    vx_status status = VX_FAILURE;

    gcmHEADER_ARG("image=%p, map_id=%p", image, map_id);
    gcmDUMP_API("$VX vxUnmapImagePatch: image=%p, map_id=%p", image, map_id);

    /* bad references */
    if (!vxoImage_IsValid(image))
    {
        status = VX_ERROR_INVALID_REFERENCE;
        goto exit;
    }

    /* bad parameters */
    if (vxoContext_FindMemoryMap(image->base.context, (vx_reference)image, map_id) != vx_true_e)
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        vxError("Invalid parameters to unmap image patch\n");
        gcmFOOTER_ARG("%d", status);
        return status;
    }

    {
        vx_context       context = image->base.context;
        vx_memory_map_s* map     = &context->memoryMaps[map_id];
        vx_uint32        p = 0;
        vx_enum usage = map->usage;

        if (map->used &&
            map->ref == (vx_reference)image)
        {
            /* freeing mapping buffer */
            vxoContext_MemoryUnmap(context, map_id);

            vxoReference_Decrement(&image->base, VX_REF_EXTERNAL);

            status = VX_SUCCESS;

            if((image->useInternalMem == vx_false_e) && ((usage == VX_WRITE_ONLY) || (usage == VX_READ_AND_WRITE)))
            {
                p = context->memoryMaps[map_id].extra.image_data.plane_index;
                gcoOS_CacheFlush(gcvNULL, image->memory.wrappedNode[p], image->memory.logicals[p], image->memory.wrappedSize[p]);
            }
            else if((image->useInternalMem == vx_true_e) &&
                    ((usage == VX_WRITE_ONLY) || (usage == VX_READ_AND_WRITE)) &&
                    (image->importType == VX_MEMORY_TYPE_HOST))
            {
                p = context->memoryMaps[map_id].extra.image_data.plane_index;
                {
                    if(image->memory.nodePtrs[p] && (image->memory.nodePtrs[p]->logical != image->memory.logicals[p]))
                    {
                        vx_rectangle_t rect;
                        vx_size size = 0;

                        vxGetValidRegionImage(image, &rect);
                        size = vxComputeImagePatchSize(image, &rect, p);
                        if(size > 0) gcoOS_MemCopy(image->memory.nodePtrs[p]->logical, image->memory.logicals[p], size);
                    }
                }
            }
        }
        else
        {
            vxError("failed to unmap image\n");
            status = VX_FAILURE;
        }
    }

exit:
    gcmFOOTER_ARG("%d", status);
    return status;
} /* vxUnmapImagePat*/
#endif

VX_API_ENTRY vx_status VX_API_CALL vxSetImageValidRectangle(vx_image image, const vx_rectangle_t* rect)
{
    vx_status status = VX_ERROR_INVALID_REFERENCE;

    gcmHEADER_ARG("image=%p, rect=%p", image, rect);
    gcmDUMP_API("$VX vxSetImageValidRectangle: image=%p, rect=%p", image, rect);

    if (vxoImage_IsValid(image) == vx_true_e)
    {
        if (rect)
        {
            if ((rect->start_x <= rect->end_x) && (rect->start_y <= rect->end_y) &&
                (rect->end_x <= image->width) && (rect->end_y <= image->height))
            {
                image->region.start_x = rect->start_x;
                image->region.start_y = rect->start_y;
                image->region.end_x   = rect->end_x;
                image->region.end_y   = rect->end_y;
                status = VX_SUCCESS;
            }
            else
                status = VX_ERROR_INVALID_PARAMETERS;
        }
        else
        {
            image->region.start_x = 0;
            image->region.start_y = 0;
            image->region.end_x   = image->width;
            image->region.end_y   = image->height;

            status = VX_SUCCESS;
        }
    }
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_API_ENTRY void * VX_API_CALL vxFormatImagePatchAddress1d(
        void *ptr, vx_uint32 planeIndex, const vx_imagepatch_addressing_t *patchAddr)
{
    gcmHEADER_ARG("ptr=%p, planeIndex=0x%x, patchAddr=%p", ptr, planeIndex, patchAddr);
    gcmDUMP_API("$VX vxFormatImagePatchAddress1d: ptr=%p, planeIndex=0x%x, patchAddr=%p", ptr, planeIndex, patchAddr);

    if (ptr == VX_NULL)
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    if (planeIndex >= patchAddr->dim_x * patchAddr->dim_y)
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    gcmFOOTER_NO();
    return (vx_uint8_ptr)ptr
            + vxComputePatchOffset(planeIndex % patchAddr->dim_x, planeIndex / patchAddr->dim_x, patchAddr);
}

VX_API_ENTRY void * VX_API_CALL vxFormatImagePatchAddress2d(
        void *ptr, vx_uint32 x, vx_uint32 y, const vx_imagepatch_addressing_t *patchAddr)
{
    gcmHEADER_ARG("ptr=%p, x=0x%x, y=0x%x, patchAddr=%p", ptr, x, y, patchAddr);
    gcmDUMP_API("$VX vxFormatImagePatchAddress2d: ptr=%p, x=0x%x, y=0x%x, patchAddr=%p", ptr, x, y, patchAddr);

    if (ptr == VX_NULL)
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    if (x >= patchAddr->dim_x || y >= patchAddr->dim_y)
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }

    gcmFOOTER_NO();
    return (vx_uint8_ptr)ptr + vxComputePatchOffset(x, y, patchAddr);
}

VX_API_ENTRY vx_status VX_API_CALL vxGetValidRegionImage(vx_image image, vx_rectangle_t *rect)
{
    gcmHEADER_ARG("image=%p, rect=%p", image, rect);
    gcmDUMP_API("$VX vxGetValidRegionImage: image=%p, rect=%p", image, rect);

    if (!vxoImage_IsValid(image))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }
    if (rect == VX_NULL)
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
        return VX_ERROR_INVALID_PARAMETERS;
    }
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

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxSwapImageHandle(vx_image image, void* const new_ptrs[], void* prev_ptrs[], vx_size num_planes)
{
    vx_status status = VX_SUCCESS;

    gcmHEADER_ARG("image=%p, new_ptrs=%p, prev_ptrs=%p, num_planes=0x%lx", image, new_ptrs, prev_ptrs, num_planes);
    gcmDUMP_API("$VX vxSwapImageHandle: image=%p, new_ptrs=%p, prev_ptrs=%p, num_planes=0x%lx", image, new_ptrs, prev_ptrs, num_planes);

    if (vxoImage_IsValid(image) == vx_true_e)
    {
        if (image->importType != VX_MEMORY_TYPE_NONE)
        {
            vx_uint32 p, i;

            if (new_ptrs != NULL)
            {
                for (p = 0; p < image->planeCount; p++)
                {
                    if (new_ptrs[p] == NULL)
                    {
                        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
                        return VX_ERROR_INVALID_PARAMETERS;
                    }
                }
            }

            if (prev_ptrs != NULL && image->parent != NULL)
            {
                /* do not return prev pointers for subimages */
                gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
                return VX_ERROR_INVALID_PARAMETERS;
            }

            if (prev_ptrs != NULL && image->parent == NULL)
            {
                /* return previous image handlers */
                for (p = 0; p < image->planeCount; p++)
                {
                    if (image->useInternalMem == vx_false_e && image->memory.logicals[p] && image->memory.wrappedSize[p])
                    {
                        gcoOS_CacheInvalidate(gcvNULL, image->memory.wrappedNode[p], image->memory.logicals[p], image->memory.wrappedSize[p]);
                    }
                    prev_ptrs[p] = image->memory.logicals[p];
                }
            }

            /* visit each subimage of this image and reclaim its pointers */
            for (p = 0; p < VX_MAX_REF_COUNT; p++)
            {
                if (image->subimages[p] != NULL)
                {
                    if (new_ptrs == NULL)
                        status = vxSwapImageHandle(image->subimages[p], NULL, NULL, image->planeCount);
                    else
                    {
                        vx_uint8* ptrs[4];

                        for (i = 0; i < image->subimages[p]->planeCount; i++)
                        {
                            vx_uint32 offset = image->subimages[p]->memory.offset[i];
                            ptrs[i] = (vx_uint8*)new_ptrs[i] + offset;
                        }

                        status = vxSwapImageHandle(image->subimages[p], (void**)ptrs, NULL, image->planeCount);
                    }

                    break;
                }
            }

            /* reclaim previous and set new handlers for this image */
            if (new_ptrs == NULL)
            {
                for (p = 0; p < image->planeCount; p++)
                {
                    image->memory.logicals[p] = 0;
                }
            }
            else
            {
                vx_uint8* ptr = gcvNULL;
                /*for non host ptr image or roi/channel image, should not release and reallocat memory*/
                if ((image->importType == VX_MEMORY_TYPE_HOST) && (image->parent == gcvNULL))
                {
                    vx_context context = vxGetContext((vx_reference)image);

                    if(vx_true_e == image->useInternalMem)
                    {
                        for (p = 0; p < image->planeCount; p++)
                        {
                            /*don't need to release old memory nodeptrs and reallocate memory, just update data*/
                            ptr = (vx_uint8*)new_ptrs[p];
                            image->memory.logicals[p] = ptr;

                            if (image->memory.nodePtrs[p] != VX_NULL && image->memory.logicals[p] != image->memory.nodePtrs[p]->logical)
                            {
                                if (image->memory.sizes[p] > 0)
                                    gcoOS_MemCopy(image->memory.nodePtrs[p]->logical, image->memory.logicals[p], image->memory.sizes[p]);
                            }
                        }
                    }
                    else
                    {
                        vxoMemory_FreeWrappedMemory(context, &image->memory);
                        vxmASSERT(!image->memory.allocated);
                        for (p = 0; p < image->planeCount; p++)
                        {
                            /* offset is non zero if this is a subimage of some image */
                            ptr = (vx_uint8*)new_ptrs[p];
                            image->memory.logicals[p] = ptr;
                        }
                        vxoMemory_WrapUserMemory(context, &image->memory);
                    }
                }
                else
                {
                    for (p = 0; p < image->planeCount; p++)
                    {
                        /* offset is non zero if this is a subimage of some image */
                        ptr = (vx_uint8*)new_ptrs[p];
                        image->memory.logicals[p] = ptr;
                    }
                }
            }

            /* clear flag if pointers were reclaimed */
            image->memory.allocated = (new_ptrs == NULL) ? vx_false_e : vx_true_e;
        }
        else
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }
    }
    else
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxSwapImage(vx_image image0, vx_image image1)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 p;
    vx_uint8_ptr ptr;
    vx_uint32 physical;
    gcmHEADER_ARG("tensor0=%p, tensor1=%p", image0, image1);
    gcmDUMP_API("$VX vxSwapTensor: tensor0=%p, tensor1=%p", image0, image1);

    if(image0->memory.wrapFlag != gcvALLOC_FLAG_USERMEMORY || image1->memory.wrapFlag != gcvALLOC_FLAG_USERMEMORY)
        return vx_false_e;
    if(!(vxoImage_IsValid(image0) && vxoImage_IsValid(image1)))
        return vx_false_e;
    if(image0->planeCount != image1->planeCount)
       return vx_false_e;

    for (p = 0; p < image0->planeCount; p++)
    {
        ptr = image0->memory.logicals[p];
        physical = image0->memory.physicals[p];

        image0->memory.logicals[p] = image0->memory.logicals[p];
        image0->memory.physicals[p] = image0->memory.physicals[p];
        image0->memory.logicals[p] = (vx_uint8_ptr)ptr;
        image0->memory.physicals[p] = physical;
    }
    gcmFOOTER_ARG("%d", status);
    return status;
}
VX_API_ENTRY vx_status VX_API_CALL vxSetImagePixelValues(vx_image image, const vx_pixel_value_t *pixel_value)
{
    vx_status status = VX_SUCCESS;

    gcmHEADER_ARG("image=%p, pixel_value=%p", image, pixel_value);
    gcmDUMP_API("$VX vxSetImagePixelValues: image=%p, pixel_value=%p", image, pixel_value);

    if(pixel_value == VX_NULL)
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
        return VX_ERROR_INVALID_PARAMETERS;
    }

    if (vxoImage_IsValid(image) == vx_true_e)
    {
        if(image->isUniform == vx_true_e || image->base.isVirtual)
        {
            gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
            return VX_ERROR_INVALID_REFERENCE;
        }
        else
        {
            vx_uint32 x, y, p;
            vx_size planes = 0;
            vx_rectangle_t rect = {0,0,0,0};
            vx_map_id map_id;
            vx_df_image format = 0;

            vxQueryImage(image, VX_IMAGE_WIDTH, &rect.end_x, sizeof(rect.end_x));
            vxQueryImage(image, VX_IMAGE_HEIGHT, &rect.end_y, sizeof(rect.end_y));
            vxQueryImage(image, VX_IMAGE_FORMAT, &format, sizeof(format));
            vxQueryImage(image, VX_IMAGE_PLANES, &planes, sizeof(planes));

            for (p = 0; p < planes; p++)
            {
                vx_imagepatch_addressing_t addr;
                void *base = NULL;
                if (vxMapImagePatch(image, &rect, p, &map_id, &addr, &base, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0) == VX_SUCCESS)
                {
                    for (y = 0; y < addr.dim_y; y+=addr.step_y)
                    {
                        for (x = 0; x < addr.dim_x; x+=addr.step_x)
                        {
                            if (format == VX_DF_IMAGE_U8)
                            {
                                vx_uint8 *ptr = (vx_uint8 *)vxFormatImagePatchAddress2d(base, x, y, &addr);
                                *ptr = pixel_value->U8;
                            }
                            else if (format == VX_DF_IMAGE_U16)
                            {
                                vx_uint16 *ptr = (vx_uint16 *)vxFormatImagePatchAddress2d(base, x, y, &addr);
                                *ptr = pixel_value->U16;
                            }
                            else if (format == VX_DF_IMAGE_U32)
                            {
                                vx_uint32 *ptr = (vx_uint32 *)vxFormatImagePatchAddress2d(base, x, y, &addr);
                                *ptr = pixel_value->U32;
                            }
                            else if (format == VX_DF_IMAGE_S16)
                            {
                                vx_int16 *ptr = (vx_int16 *)vxFormatImagePatchAddress2d(base, x, y, &addr);
                                *ptr = pixel_value->S16;
                            }
                            else if (format == VX_DF_IMAGE_S32)
                            {
                                vx_int32 *ptr = (vx_int32 *)vxFormatImagePatchAddress2d(base, x, y, &addr);
                                *ptr = pixel_value->S32;
                            }
                            else if ((format == VX_DF_IMAGE_RGB)  ||
                                     (format == VX_DF_IMAGE_RGBX))
                            {
                                vx_uint8 *ptr = (vx_uint8 *)vxFormatImagePatchAddress2d(base, x, y, &addr);
                                ptr[0] = pixel_value->RGBX[0];
                                ptr[1] = pixel_value->RGBX[1];
                                ptr[2] = pixel_value->RGBX[2];
                                if (format == VX_DF_IMAGE_RGBX) ptr[3] = pixel_value->RGBX[3];
                            }
                            else if ((format == VX_DF_IMAGE_YUV4) ||
                                     (format == VX_DF_IMAGE_IYUV))
                            {
                                vx_uint8 *pixel = (vx_uint8 *)&pixel_value->YUV;
                                vx_uint8 *ptr = (vx_uint8 *)vxFormatImagePatchAddress2d(base, x, y, &addr);
                                *ptr = pixel[p];
                            }
                            else if ((p == 0) &&
                                     ((format == VX_DF_IMAGE_NV12) ||
                                      (format == VX_DF_IMAGE_NV21)))
                            {
                                vx_uint8 *pixel = (vx_uint8 *)&pixel_value->YUV;
                                vx_uint8 *ptr = (vx_uint8 *)vxFormatImagePatchAddress2d(base, x, y, &addr);
                                *ptr = pixel[0];
                            }
                            else if ((p == 1) && (format == VX_DF_IMAGE_NV12))
                            {
                                vx_uint8 *pixel = (vx_uint8 *)&pixel_value->YUV;
                                vx_uint8 *ptr = (vx_uint8 *)vxFormatImagePatchAddress2d(base, x, y, &addr);
                                ptr[0] = pixel[1];
                                ptr[1] = pixel[2];
                            }
                            else if ((p == 1) && (format == VX_DF_IMAGE_NV21))
                            {
                                vx_uint8 *pixel = (vx_uint8 *)&pixel_value->YUV;
                                vx_uint8 *ptr = (vx_uint8 *)vxFormatImagePatchAddress2d(base, x, y, &addr);
                                ptr[0] = pixel[2];
                                ptr[1] = pixel[1];
                            }
                            else if (format == VX_DF_IMAGE_UYVY)
                            {
                                vx_uint8 *pixel = (vx_uint8 *)&pixel_value->YUV;
                                vx_uint8 *ptr = (vx_uint8 *)vxFormatImagePatchAddress2d(base, x, y, &addr);
                                if (x % 2 == 0)
                                {
                                    ptr[0] = pixel[1];
                                    ptr[1] = pixel[0];
                                }
                                else
                                {
                                    ptr[0] = pixel[2];
                                    ptr[1] = pixel[0];
                                }
                            }
                            else if (format == VX_DF_IMAGE_YUYV)
                            {
                                vx_uint8 *pixel = (vx_uint8 *)&pixel_value->YUV;
                                vx_uint8 *ptr = (vx_uint8 *)vxFormatImagePatchAddress2d(base, x, y, &addr);
                                if (x % 2 == 0)
                                {
                                    ptr[0] = pixel[0];
                                    ptr[1] = pixel[1];
                                }
                                else
                                {
                                    ptr[0] = pixel[0];
                                    ptr[1] = pixel[2];
                                }
                            }
                        }
                    }
                    if (vxUnmapImagePatch(image, map_id) != VX_SUCCESS)
                    {
                        vxError("Failed to set initial image patch on plane %u on const image!\n", p);
                        status = VX_FAILURE;
                        break;
                    }
                }
                else
                {
                    vxError("Failed to get image patch on plane %u in const image!\n",p);
                    status = VX_FAILURE;
                    break;
                }
            }
        }
    }
    else
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

