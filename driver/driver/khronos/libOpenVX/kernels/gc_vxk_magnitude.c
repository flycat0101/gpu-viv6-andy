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

vx_status vxMagnitude(vx_node node, vx_image grad_x, vx_image grad_y, vx_image output)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 width = 0, height = 0;
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

    vxQueryImage(grad_x, VX_IMAGE_WIDTH, &width, sizeof(width));
    vxQueryImage(grad_x, VX_IMAGE_HEIGHT, &height, sizeof(height));

    /*index = 0*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, grad_x, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, grad_y, GC_VX_INDEX_AUTO);

    /*index = 2*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);
    if (node->base.context->evisNoInst.isVX2 || node->base.context->evisNoInst.noMagPhase)
    {
        vx_uint8 bin[16] = {0, 32, 64, 96, 0, 0, 0, 0, 16, 16, 16, 16, 0, 0, 0, 0};
        gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, bin, sizeof(bin));
        kernelContext->uniforms[0].index = 3;
        kernelContext->uniforms[0].num = vxmLENGTH_OF(bin);
        kernelContext->uniform_num = 1;
    }

    kernelContext->params.kernel = gcvVX_KERNEL_MAGNITUDE;
    kernelContext->params.xstep = 8;

    kernelContext->params.xmax = width;
    kernelContext->params.ymax = height;

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

