/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include <gc_vxk_common.h>

#if VIV_NONMAX_SUPPRESSION
vx_status vxNonMaxSuppressionCanny(vx_node node, vx_image i_mag, vx_image i_ang, vx_image i_edge, vx_border_t *borders)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 constantData[2] = {0, 16};
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

    vxQueryImage(i_edge, VX_IMAGE_WIDTH, &width, sizeof(width));
    vxQueryImage(i_edge, VX_IMAGE_HEIGHT, &height, sizeof(height));

    /*index = 0*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, i_mag, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, i_ang, GC_VX_INDEX_AUTO);

    /*index = 2*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, i_edge, GC_VX_INDEX_AUTO);

    gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, constantData, sizeof(constantData));
    kernelContext->uniforms[0].index = 3;
    kernelContext->uniforms[0].num = sizeof(constantData) / sizeof(vx_uint32);
    kernelContext->uniform_num = 1;

    kernelContext->params.kernel   = gcvVX_KERNEL_NONMAXSUPPRESSION_CANNY;

#if gcdVX_OPTIMIZER
    kernelContext->borders         = borders->mode;
#else
    kernelContext->params.borders  = borders->mode;
#endif
    kernelContext->params.row      = width;
    kernelContext->params.col      = height;
    kernelContext->params.xstep    = 1;
    kernelContext->params.ystep    = height;

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
#endif

#if VIV_EUCLIDEAN_NONMAX

vx_status vxEuclideanNonMax_Max(vx_node node, vx_image src, vx_scalar thr, vx_scalar rad, vx_scalar point_count, vx_image point_array)
{
    vx_status status = VX_SUCCESS;

    vx_int32 radius = 0;
    vx_float32 r = 0.0f, thresh = 0.0f;
    vx_df_image format = VX_DF_IMAGE_VIRT;

    status |= vxQueryImage(src, VX_IMAGE_FORMAT, &format, sizeof(format));

    status |= vxReadScalarValue(rad, &r);
    status |= vxReadScalarValue(thr, &thresh);

    radius = (vx_int32)r;
    radius = (radius <= 0 ? 1 : radius);

    if (format == VX_DF_IMAGE_F32)
    {
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

        /*index = 0*/
        gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, src, GC_VX_INDEX_AUTO);

        /*index = 1*/
        gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, point_array, GC_VX_INDEX_AUTO);

        /*index = 2*/
        gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_SCALAR, point_count, GC_VX_INDEX_AUTO);

        kernelContext->params.scale      = thresh;
        kernelContext->params.volume     = (gctUINT32)radius;

        kernelContext->params.kernel     = gcvVX_KERNEL_EUCLIDEAN_NONMAXSUPPRESSION_MAX;

        kernelContext->params.xstep      = src->width;
        kernelContext->params.ystep      = src->height;

        {
            vx_int32 bin[4] = {1, src->width, src->height};
            gcoOS_MemCopy(&kernelContext->uniforms[kernelContext->uniform_num].uniform, bin, sizeof(bin));
            kernelContext->uniforms[kernelContext->uniform_num].index = 3;
            kernelContext->uniforms[kernelContext->uniform_num].num = sizeof(bin) / sizeof(vx_int32);
            kernelContext->uniform_num ++;
        }

        kernelContext->node = node;

        kernelContext->params.groupSizeX =

        kernelContext->params.groupSizeY = 1;

        status |= gcfVX_Kernel(kernelContext);

#if gcdVX_OPTIMIZER
        if (!node || !node->kernelContext)
        {
            vxFree(kernelContext);
        }
#endif
    }

    return status;
}

vx_status vxEuclideanNonMax_Sort(vx_node node, vx_scalar point_count, vx_image point_array, vx_image sort_array)
{
    vx_status status = VX_SUCCESS;

    vx_int32 constantData[4] = {0};
    vx_float32_ptr constantData_f = (vx_float32_ptr)&constantData;

    gcoVX_Kernel_Context * kernelContext = gcvNULL;

    *constantData_f = 100.0f;

    kernelContext = (gcoVX_Kernel_Context *) vxAllocate(sizeof(gcoVX_Kernel_Context));

    vxZeroMemory(kernelContext, sizeof(gcoVX_Kernel_Context));

    kernelContext->objects_num = 0;
    kernelContext->uniform_num = 0;

    /*index = 0*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_SCALAR, point_count, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, point_array, GC_VX_INDEX_AUTO);

    /*index = 2*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, sort_array, GC_VX_INDEX_AUTO);

    kernelContext->params.kernel     = gcvVX_KERNEL_EUCLIDEAN_NONMAXSUPPRESSION_SORT;

    kernelContext->params.xstep      = 6;
    kernelContext->params.ystep      = 10 * 1024;

    gcoOS_MemCopy(&kernelContext->uniforms[kernelContext->uniform_num].uniform, constantData, sizeof(constantData));
    kernelContext->uniforms[kernelContext->uniform_num].index = 3;
    kernelContext->uniforms[kernelContext->uniform_num].num = sizeof(constantData) / sizeof(vx_int32);
    kernelContext->uniform_num ++;

    kernelContext->node = node;

    kernelContext->params.groupSizeX =

    kernelContext->params.groupSizeY = 1;

    status |= gcfVX_Kernel(kernelContext);

    vxFree(kernelContext);

    return status;
}

vx_status vxEuclideanNonMaxSuppression_NonMax(vx_node node, vx_image arrays, vx_scalar count, vx_scalar rad, vx_image dst)
{
    vx_status status = VX_SUCCESS;

    gcoVX_Kernel_Context * kernelContext = gcvNULL;

#if gcdVX_OPTIMIZER
    if (node && node->kernelContext)
    {
        kernelContext = (gcoVX_Kernel_Context *) node->kernelContext;
    }
    else
#endif
    kernelContext = (gcoVX_Kernel_Context *) vxAllocate(sizeof(gcoVX_Kernel_Context));

    vxZeroMemory(kernelContext, sizeof(gcoVX_Kernel_Context));

    kernelContext->objects_num = 0;
    kernelContext->uniform_num = 0;

    /*index = 0*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, arrays, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_SCALAR, count, GC_VX_INDEX_AUTO);

    /*index = 2*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, dst, GC_VX_INDEX_AUTO);

    kernelContext->params.kernel     = gcvVX_KERNEL_EUCLIDEAN_NONMAXSUPPRESSION_NONMAX;

    kernelContext->params.xstep      = 6;
    kernelContext->params.ystep      = 10 * 1024;

    kernelContext->params.volume     = rad->value->n32;

    {
        vx_int32 bin[4] = {30, -30, 1, -1};
        gcoOS_MemCopy(&kernelContext->uniforms[kernelContext->uniform_num].uniform, bin, sizeof(bin));
        kernelContext->uniforms[kernelContext->uniform_num].index = 3;
        kernelContext->uniforms[kernelContext->uniform_num].num = sizeof(bin) / sizeof(vx_int32);
        kernelContext->uniform_num ++;
    }

    kernelContext->node = node;
    kernelContext->params.groupSizeX =

    kernelContext->params.groupSizeY = 1;

    status |= gcfVX_Kernel(kernelContext);

    vxFree(kernelContext);

    return status;
}

vx_status vxEuclideanNonMaxSuppression(vx_node node, vx_image src, vx_scalar thr, vx_scalar rad, vx_image dst)
{
    vx_status status = VX_SUCCESS;

    if (rad->value->f32 == 30.0f)
    {
        vx_int32 point_count = 0, size = 1024 * 10;
        vx_context context = vxGetContext((vx_reference)node);
        vx_scalar point_count_s = vxCreateScalar(context, VX_TYPE_INT32, &point_count);
        vx_image point_array_s = vxCreateImage(context, 6, size, VX_DF_IMAGE_S16);
        vx_image sort_array_s = vxCreateImage(context, 6, size, VX_DF_IMAGE_S16);

        vxmONERROR_FALSE(vxoImage_AllocateMemory(point_array_s));
        vxmONERROR_FALSE(vxoImage_AllocateMemory(sort_array_s));

        vxZeroMemory(dst->memory.logicals[0], sizeof(vx_float32) * dst->width * dst->height);

        memset(point_array_s->memory.logicals[0], 0, sizeof(vx_float32) * size * 3);
        memset(sort_array_s->memory.logicals[0], 0, sizeof(vx_float32) * size * 3);

        vxEuclideanNonMax_Max(node, src, thr, rad, point_count_s, point_array_s);
        vxEuclideanNonMax_Sort(node, point_count_s, point_array_s, sort_array_s);
        vxEuclideanNonMaxSuppression_NonMax(node, sort_array_s, point_count_s, rad, dst);

        vxReleaseScalar(&point_count_s);
        vxReleaseImage(&point_array_s);
        vxReleaseImage(&sort_array_s);
    }
    else
    {
        vx_int32 radius = 0;
        vx_float32 r = 0.0f, thresh = 0.0f;
        vx_df_image format = VX_DF_IMAGE_VIRT;
        vx_uint32 width = 0, height = 0;

        status |= vxQueryImage(src, VX_IMAGE_FORMAT, &format, sizeof(format));
        status |= vxQueryImage(src, VX_IMAGE_WIDTH, &width, sizeof(width));
        status |= vxQueryImage(src, VX_IMAGE_HEIGHT, &height, sizeof(height));

        status |= vxReadScalarValue(rad, &r);
        status |= vxReadScalarValue(thr, &thresh);

        radius = (vx_int32)r;
        radius = (radius <= 0 ? 1 : radius);

        if (format == VX_DF_IMAGE_F32)
        {
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

            vxZeroMemory(dst->memory.logicals[0], sizeof(vx_float32) * dst->width * dst->height);

            /*index = 0*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, src, GC_VX_INDEX_AUTO);

            /*index = 1*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, dst, GC_VX_INDEX_AUTO);

            kernelContext->params.scale      = thresh;
            kernelContext->params.volume     = (gctUINT32)radius;

            kernelContext->params.kernel     = gcvVX_KERNEL_EUCLIDEAN_NONMAXSUPPRESSION;

            kernelContext->params.xstep      = width;
            kernelContext->params.ystep      = height;
            kernelContext->params.col        = width;
            kernelContext->params.row        = height;

            kernelContext->node = node;

            status |= gcfVX_Kernel(kernelContext);

#if gcdVX_OPTIMIZER
            if (!node || !node->kernelContext)
            {
                vxFree(kernelContext);
            }
#endif
        }
    }

OnError:
    return status;
}
#endif

