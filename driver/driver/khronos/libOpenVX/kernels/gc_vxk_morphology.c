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

vx_status _gcfVX_Morphology(vx_node node, gceVX_KERNEL kernel, vx_image src, vx_image dst, vx_border_mode_t *bordermode)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 height;

    gcoVX_Kernel_Context * kernelContext = gcvNULL;
    vx_df_image inputFormat;

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

    vxQueryImage(src, VX_IMAGE_HEIGHT, &height, sizeof(height));
    vxQueryImage(src, VX_IMAGE_FORMAT, &inputFormat, sizeof(inputFormat));

    /*index = 0*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, src, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, dst, GC_VX_INDEX_AUTO);

    kernelContext->params.kernel       = kernel;

    if (node->base.context->evisNoInst.isVX2)
    {
        if (inputFormat == VX_DF_IMAGE_S16 || inputFormat == VX_DF_IMAGE_U16)
            kernelContext->params.xstep        =  6;
        else
            kernelContext->params.xstep        =  14;
    }
    else if (node->base.context->evisNoInst.noFilter)
    {
        kernelContext->params.xstep        = 6;
    }
    else
    {
        kernelContext->params.xstep        = 8;
    }
    kernelContext->params.ystep        = height;
#if gcdVX_OPTIMIZER
    kernelContext->borders             = bordermode->mode;
#else
    kernelContext->params.borders      = bordermode->mode;
#endif

    if (node->base.context->evisNoInst.noFilter)
    {
        vx_uint32 rect[2];
        rect[0] = height;
        rect[1] = FV(1);
        gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, rect, sizeof(rect));
        kernelContext->uniforms[0].index       = 2;
        kernelContext->uniforms[0].num         = 4 * 2;
    }
    else
    {
        vx_uint32 rect[1];
        rect[0] = height;
        gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, rect, sizeof(rect));
        kernelContext->uniforms[0].index       = 2;
        kernelContext->uniforms[0].num         = 4;
    }

    kernelContext->uniform_num             = 1;

    if(bordermode->mode == VX_BORDER_CONSTANT || bordermode->mode == VX_BORDER_UNDEFINED)
    {
        vx_uint32 bin[4];

        bin[0] =
        bin[1] =
        bin[2] =
        bin[3] = FORMAT_VALUE((bordermode->mode == VX_BORDER_UNDEFINED) ? 0xcd : bordermode->constant_value.U32);

        gcoOS_MemCopy(&kernelContext->uniforms[1].uniform, bin, sizeof(bin));
        kernelContext->uniforms[1].num = 4 * 4;
        kernelContext->uniforms[1].index = 3;
        kernelContext->uniform_num = 2;
    }

    if (node->base.context->evisNoInst.noFilter)
    {
        vx_uint8 bin[16] = {0, 32, 64, 96, 0, 0, 0, 0, 8, 8, 8, 8, 0, 0, 0, 0};
        gcoOS_MemCopy(&kernelContext->uniforms[2].uniform, bin, sizeof(bin));
        kernelContext->uniforms[2].num = vxmLENGTH_OF(bin);
        kernelContext->uniforms[2].index = 4;
        kernelContext->uniform_num = 3;

        kernelContext->evisNoInst = node->base.context->evisNoInst;
    }

    kernelContext->params.evisNoInst = node->base.context->evisNoInst;

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

/* nodeless version of the Erode3x3 kernel*/
vx_status vxErode3x3(vx_node node, vx_image src, vx_image dst, vx_border_t *bordermode)
{
    return _gcfVX_Morphology(node, gcvVX_KERNEL_ERODE_3x3, src, dst, bordermode);
}

/* nodeless version of the Dilate3x3 kernel*/
vx_status vxDilate3x3(vx_node node, vx_image src, vx_image dst, vx_border_t *bordermode)
{
    return _gcfVX_Morphology(node, gcvVX_KERNEL_DILATE_3x3, src, dst, bordermode);
}




