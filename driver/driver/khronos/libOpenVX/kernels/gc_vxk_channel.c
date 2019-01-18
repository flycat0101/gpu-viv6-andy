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


#include <gc_vxk_common.h>

vx_status vxChannelCombine(vx_node node, vx_image inputs[4], vx_image output)
{
    vx_status status = VX_SUCCESS;
    vx_df_image format = 0;
    vx_uint32 constantData[8] = {0, 8, 16, 24, 0, 0, 0, 0};
    vx_uint32 width;
    vx_uint32 height;
    gcoVX_Kernel_Context * kernelContext = gcvNULL;

#if gcdVX_OPTIMIZER
    if (node && node->kernelContext)
    {
        kernelContext = (gcoVX_Kernel_Context *) node->kernelContext;
    }
    else
#endif
    {
        if (node->kernelContext == VX_NULL)
        {
            /* Allocate a local copy for old flow. */
            node->kernelContext = (gcoVX_Kernel_Context *) vxAllocate(sizeof(gcoVX_Kernel_Context));
        }
        kernelContext = (gcoVX_Kernel_Context *)node->kernelContext;
        kernelContext->objects_num = 0;
        kernelContext->uniform_num = 0;
    }

    vxQueryImage(output, VX_IMAGE_WIDTH, &width, sizeof(width));
    vxQueryImage(output, VX_IMAGE_HEIGHT, &height, sizeof(height));
    vxQueryImage(output, VX_IMAGE_FORMAT, &format, sizeof(format));

    switch (format)
    {
        case VX_DF_IMAGE_RGB:
            /*index = 0*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[0], GC_VX_INDEX_AUTO);

            /*index = 1*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[1], GC_VX_INDEX_AUTO);

            /*index = 2*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[2], GC_VX_INDEX_AUTO);

            /*index = 3*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

            gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, constantData, sizeof(constantData));
            kernelContext->uniforms[0].index = 4;
            kernelContext->uniforms[0].num = sizeof(constantData) / sizeof(vx_uint32);
            kernelContext->uniform_num = 1;
            kernelContext->params.outputFormat = gcvSURF_R8G8B8;
            kernelContext->params.xstep = 4;
            break;
        case VX_DF_IMAGE_RGBX:
            /*index = 0*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[0], GC_VX_INDEX_AUTO);

            /*index = 1*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[1], GC_VX_INDEX_AUTO);

            /*index = 2*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[2], GC_VX_INDEX_AUTO);

            /*index = 3*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[3], GC_VX_INDEX_AUTO);

            /*index = 4*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

            gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, constantData, sizeof(constantData));
            kernelContext->uniforms[0].index = 5;
            kernelContext->uniforms[0].num = sizeof(constantData) / sizeof(vx_uint32);
            kernelContext->uniform_num = 1;
            kernelContext->params.outputFormat = gcvSURF_X8R8G8B8;
            kernelContext->params.xstep = 4;
            break;
        case VX_DF_IMAGE_UYVY:
            /*index = 0*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[0], GC_VX_INDEX_AUTO);

            /*index = 1*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[1], GC_VX_INDEX_AUTO);

            /*index = 2*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[2], GC_VX_INDEX_AUTO);

            /*index = 3*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

            gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, constantData, sizeof(constantData));
            kernelContext->uniforms[0].index = 4;
            kernelContext->uniforms[0].num = sizeof(constantData) / sizeof(vx_uint32);
            kernelContext->uniform_num = 1;
            kernelContext->params.outputFormat = gcvSURF_UYVY;
            kernelContext->params.xstep = 8;
            break;
        case VX_DF_IMAGE_YUYV:
            /*index = 0*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[0], GC_VX_INDEX_AUTO);

            /*index = 1*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[1], GC_VX_INDEX_AUTO);

            /*index = 2*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[2], GC_VX_INDEX_AUTO);

            /*index = 3*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

            gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, constantData, sizeof(constantData));
            kernelContext->uniforms[0].index = 4;
            kernelContext->uniforms[0].num = sizeof(constantData) / sizeof(vx_uint32);
            kernelContext->uniform_num = 1;

            kernelContext->params.outputFormat = gcvSURF_YUY2;
            kernelContext->params.xstep = 8;
            break;
        case VX_DF_IMAGE_NV12:
            /*index = 0*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[0], GC_VX_INDEX_AUTO);

            /*index = 1*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[1], GC_VX_INDEX_AUTO);

            /*index = 2*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[2], GC_VX_INDEX_AUTO);

            /*index = 3*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

            gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, constantData, sizeof(constantData));
            kernelContext->uniforms[0].index = 5;
            kernelContext->uniforms[0].num = sizeof(constantData) / sizeof(vx_uint32);
            kernelContext->uniform_num = 1;
            kernelContext->params.outputFormat = gcvSURF_NV12;
            kernelContext->params.xstep = 16;
            break;
        case VX_DF_IMAGE_NV21:
            /*index = 0*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[0], GC_VX_INDEX_AUTO);

            /*index = 1*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[1], GC_VX_INDEX_AUTO);

            /*index = 2*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[2], GC_VX_INDEX_AUTO);

            /*index = 3*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

            gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, constantData, sizeof(constantData));
            kernelContext->uniforms[0].index = 5;
            kernelContext->uniforms[0].num = sizeof(constantData) / sizeof(vx_uint32);
            kernelContext->uniform_num = 1;
            kernelContext->params.outputFormat = gcvSURF_NV21;
            kernelContext->params.xstep = 16;
            break;
        case VX_DF_IMAGE_IYUV:
            /*index = 0*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[0], GC_VX_INDEX_AUTO);

            /*index = 1*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[1], GC_VX_INDEX_AUTO);

            /*index = 2*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[2], GC_VX_INDEX_AUTO);

            /*index = 3*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

            kernelContext->params.outputFormat = gcvSURF_I420;
            kernelContext->params.xstep = 16;
            break;
        case VX_DF_IMAGE_YUV4:
            /*index = 0*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[0], GC_VX_INDEX_AUTO);

            /*index = 1*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[1], GC_VX_INDEX_AUTO);

            /*index = 2*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[2], GC_VX_INDEX_AUTO);

            /*index = 3*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

            kernelContext->params.outputFormat = gcvSURF_AYUV;
            kernelContext->params.xstep = 16;
            break;
        default:
            status = VX_ERROR_INVALID_FORMAT;
            break;
    }
    if (status != VX_SUCCESS)
        return status;

    kernelContext->params.kernel = gcvVX_KERNEL_CHANNEL_COMBINE;
#if gcdVX_OPTIMIZER
    kernelContext->borders = VX_BORDER_CONSTANT;
#else
    kernelContext->params.borders = VX_BORDER_CONSTANT;
#endif
    kernelContext->params.constant_value = 0;
    kernelContext->params.row = width;
    kernelContext->params.col = height;

    kernelContext->node = node;

    status = gcfVX_Kernel(kernelContext);

#if gcdVX_OPTIMIZER
    if (!node || !node->kernelContext)
    {
        vxFree(kernelContext);
    }
#endif
    return status;
}

vx_status vxChannelExtract(vx_node node, vx_image src, vx_scalar channel, vx_image dst)
{
    vx_status status = VX_SUCCESS;
    vx_uint8 constantData[16] = {0, 32, 64, 96, 0, 0, 0, 0, 8, 8, 8, 8, 0, 0, 0, 0};
    vx_df_image format = 0;
    vx_enum chan = -1;
    vx_uint32 width;
    vx_uint32 height;
    gcoVX_Kernel_Context * kernelContext = gcvNULL;

#if gcdVX_OPTIMIZER
    if (node && node->kernelContext)
    {
        kernelContext = (gcoVX_Kernel_Context *) node->kernelContext;
    }
    else
#endif
    {
        if (node->kernelContext == VX_NULL)
        {
            /* Allocate a local copy for old flow. */
            node->kernelContext = (gcoVX_Kernel_Context *) vxAllocate(sizeof(gcoVX_Kernel_Context));
        }
        kernelContext = (gcoVX_Kernel_Context *)node->kernelContext;
        kernelContext->objects_num = 0;
    }

    vxReadScalarValue(channel, &chan);
    vxQueryImage(src, VX_IMAGE_FORMAT, &format, sizeof(format));
    vxQueryImage(src, VX_IMAGE_WIDTH, &width, sizeof(width));
    vxQueryImage(src, VX_IMAGE_HEIGHT, &height, sizeof(height));

    if (format == VX_DF_IMAGE_RGB)
    {
        switch (chan)
        {
             case VX_CHANNEL_0:
             case VX_CHANNEL_R:
                kernelContext->params.volume = 0;
                constantData[0] = 0;
                constantData[1] = 24;
                constantData[2] = 48;
                constantData[3] = 72;
                break;
            case VX_CHANNEL_1:
            case VX_CHANNEL_G:
                kernelContext->params.volume = 1;
                constantData[0] = 8;
                constantData[1] = 32;
                constantData[2] = 56;
                constantData[3] = 80;
                break;
            case VX_CHANNEL_2:
            case VX_CHANNEL_B:
                kernelContext->params.volume = 2;
                constantData[0] = 16;
                constantData[1] = 40;
                constantData[2] = 64;
                constantData[3] = 88;
                break;
            default:
                return VX_ERROR_INVALID_PARAMETERS;
        }
        kernelContext->params.inputFormat = gcvSURF_R8G8B8;
        kernelContext->params.inputMultipleWidth = 3;
        kernelContext->params.xmax = width * 3;
        kernelContext->params.xstep = 12;
    }
    else if (format == VX_DF_IMAGE_RGBX)
    {
        switch (chan)
        {
            case VX_CHANNEL_0:
            case VX_CHANNEL_R:
                kernelContext->params.volume = 0;
                break;
            case VX_CHANNEL_1:
            case VX_CHANNEL_G:
                kernelContext->params.volume = 1;
                break;
            case VX_CHANNEL_2:
            case VX_CHANNEL_B:
                kernelContext->params.volume = 2;
                break;
            case VX_CHANNEL_3:
            case VX_CHANNEL_A:
                kernelContext->params.volume = 3;
                break;
            default:
                return VX_ERROR_INVALID_PARAMETERS;
        }
        constantData[0] += (vx_uint8)(8*kernelContext->params.volume);
        constantData[1] += (vx_uint8)(8*kernelContext->params.volume);
        constantData[2] += (vx_uint8)(8*kernelContext->params.volume);
        constantData[3] += (vx_uint8)(8*kernelContext->params.volume);
        kernelContext->params.inputFormat = gcvSURF_X8R8G8B8;
        kernelContext->params.xstep = 4;
    }
    else if (format == VX_DF_IMAGE_UYVY)
    {
        switch (chan)
        {
            case VX_CHANNEL_Y:
                kernelContext->params.volume = 0;
                constantData[0] = 8;
                constantData[1] = 24;
                constantData[2] = 40;
                constantData[3] = 56;
                constantData[4] = 72;
                constantData[5] = 88;
                constantData[6] = 104;
                constantData[7] = 120;
                constantData[12] += 8;
                constantData[13] += 8;
                constantData[14] += 8;
                constantData[15] += 8;
                kernelContext->params.xstep = 8;
                break;
            case VX_CHANNEL_U:
                kernelContext->params.volume = 1;
                kernelContext->params.xstep = 8;
                break;
            case VX_CHANNEL_V:
                kernelContext->params.volume = 2;
                constantData[0] += 16;
                constantData[1] += 16;
                constantData[2] += 16;
                constantData[3] += 16;
                kernelContext->params.xstep = 8;
                break;
            default:
                return VX_ERROR_INVALID_PARAMETERS;
        }
        kernelContext->params.inputFormat = gcvSURF_UYVY;
    }
     else if (format == VX_DF_IMAGE_YUYV)
    {
        switch (chan)
        {
            case VX_CHANNEL_Y:
                kernelContext->params.volume = 0;
                constantData[1] = 16;
                constantData[2] = 32;
                constantData[3] = 48;
                constantData[4] = 64;
                constantData[5] = 80;
                constantData[6] = 96;
                constantData[7] = 112;
                constantData[12] += 8;
                constantData[13] += 8;
                constantData[14] += 8;
                constantData[15] += 8;
                kernelContext->params.xstep = 8;
                break;
            case VX_CHANNEL_U:
                kernelContext->params.volume = 1;
                constantData[0] += 8;
                constantData[1] += 8;
                constantData[2] += 8;
                constantData[3] += 8;
                kernelContext->params.xstep = 8;
                break;
            case VX_CHANNEL_V:
                kernelContext->params.volume = 2;
                constantData[0] += 24;
                constantData[1] += 24;
                constantData[2] += 24;
                constantData[3] += 24;
                kernelContext->params.xstep = 8;
                break;
            default:
                return VX_ERROR_INVALID_PARAMETERS;
        }
        kernelContext->params.inputFormat = gcvSURF_YUY2;
    }
    else if (format == VX_DF_IMAGE_NV12)
    {
        switch (chan)
        {
        case VX_CHANNEL_Y:
            kernelContext->params.volume = 0;
            break;
        case VX_CHANNEL_U:
            kernelContext->params.volume = 1;
            constantData[1] = 16;
            constantData[2] = 32;
            constantData[3] = 48;
            constantData[4] = 64;
            constantData[5] = 80;
            constantData[6] = 96;
            constantData[7] = 112;
            break;
        case VX_CHANNEL_V:
            kernelContext->params.volume = 2;
            constantData[0] = 8;
            constantData[1] = 24;
            constantData[2] = 40;
            constantData[3] = 56;
            constantData[4] = 72;
            constantData[5] = 88;
            constantData[6] = 104;
            constantData[7] = 120;
            break;
        default:
            return VX_ERROR_INVALID_PARAMETERS;
        }
        constantData[12] = 8;
        constantData[13] = 8;
        constantData[14] = 8;
        constantData[15] = 8;
        kernelContext->params.xstep = 16;
        kernelContext->params.inputFormat = gcvSURF_NV12;
    }
    else if (format == VX_DF_IMAGE_NV21)
    {
        switch (chan)
        {
        case VX_CHANNEL_Y:
            kernelContext->params.volume = 0;
            break;
        case VX_CHANNEL_U:
            kernelContext->params.volume = 1;
            constantData[0] = 8;
            constantData[1] = 24;
            constantData[2] = 40;
            constantData[3] = 56;
            constantData[4] = 72;
            constantData[5] = 88;
            constantData[6] = 104;
            constantData[7] = 120;
            break;
        case VX_CHANNEL_V:
            kernelContext->params.volume = 2;
            constantData[1] = 16;
            constantData[2] = 32;
            constantData[3] = 48;
            constantData[4] = 64;
            constantData[5] = 80;
            constantData[6] = 96;
            constantData[7] = 112;
            break;
        default:
            return VX_ERROR_INVALID_PARAMETERS;
        }
        constantData[12] = 8;
        constantData[13] = 8;
        constantData[14] = 8;
        constantData[15] = 8;
        kernelContext->params.xstep = 16;
        kernelContext->params.inputFormat = gcvSURF_NV21;
    }
    else if (format == VX_DF_IMAGE_YUV4)
    {
        switch (chan)
        {
        case VX_CHANNEL_Y:
            kernelContext->params.volume = 0;
            break;
        case VX_CHANNEL_U:
            kernelContext->params.volume = 1;
            break;
        case VX_CHANNEL_V:
            kernelContext->params.volume = 2;
            break;
        default:
            return VX_ERROR_INVALID_PARAMETERS;
        }
        kernelContext->params.inputFormat = gcvSURF_AYUV;
        kernelContext->params.xstep = 16;
    }
    else if (format == VX_DF_IMAGE_IYUV)
    {
        switch (chan)
        {
        case VX_CHANNEL_Y:
            kernelContext->params.volume = 0;
            kernelContext->params.xstep = 16;
            break;
        case VX_CHANNEL_U:
            kernelContext->params.volume = 1;
            kernelContext->params.xstep = 8;
            break;
        case VX_CHANNEL_V:
            kernelContext->params.volume = 2;
            kernelContext->params.xstep = 8;
            break;
        default:
            return VX_ERROR_INVALID_PARAMETERS;
        }
        kernelContext->params.inputFormat = gcvSURF_I420;
    }
    else
    {
        return VX_ERROR_INVALID_PARAMETERS;
    }


    /*index = 0*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, src, GC_VX_INDEX_AUTO);

    /*index = 3*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, dst, 3);

    kernelContext->params.kernel = gcvVX_KERNEL_CHANNEL_EXTRACT;

    gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, constantData, sizeof(constantData));
    kernelContext->uniforms[0].index = 4;
    kernelContext->uniforms[0].num = sizeof(constantData) / sizeof(vx_uint8);
    kernelContext->uniform_num = 1;

#if gcdVX_OPTIMIZER
    kernelContext->borders = VX_BORDER_CONSTANT;
#else
    kernelContext->params.borders = VX_BORDER_CONSTANT;
#endif
    kernelContext->params.constant_value = 0;
    kernelContext->params.row = width;
    kernelContext->params.col = height;

    kernelContext->node = node;

    status = gcfVX_Kernel(kernelContext);

#if gcdVX_OPTIMIZER
    if (!node || !node->kernelContext)
    {
        vxFree(kernelContext);
    }
#endif

    return status;
}

