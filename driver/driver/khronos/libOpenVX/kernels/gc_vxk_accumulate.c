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


#include <gc_vxk_common.h>

vx_status vxAccumulate(vx_node node, vx_image input, vx_image accum)
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
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, accum, GC_VX_INDEX_AUTO);

    kernelContext->params.kernel = gcvVX_KERNEL_ACCUMULATE;
    kernelContext->params.xstep = 8;

    kernelContext->params.evisNoInst = node->base.context->evisNoInst;

#if GC_VX_ASM
    {
        char source[1024] = {
            "img_load.u8.evis.{0,15} r1 c0 r0.xy\n"
            "img_load.s16.evis.{0,7} r2 c1 r0.xy\n"
            "iadd.s16.evis.clamp.{0,7} r2 r2.s16 r1.u8\n"
            "img_store.s16.evis.{0,7} r2 c1 r0.xy r2\n"
        };
        kernelContext->params.instructions.source = source;
        kernelContext->params.instructions.regs_count = 3;
    }
#endif

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

vx_status vxAccumulateWeighted(vx_node node, vx_image input, vx_scalar scalar, vx_image accum)
{
    vx_status status = VX_SUCCESS;
    vx_float32 alpha = 0u;
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

    vxReadScalarValue(scalar, &alpha);

    /*index = 0*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, accum, GC_VX_INDEX_AUTO);

    kernelContext->params.kernel = gcvVX_KERNEL_ACCUMULATE_WEIGHTED;

    if (node->base.context->evisNoInst.lerp7Output)
    {
        kernelContext->params.xstep = 7;
    }
    else
    {
        kernelContext->params.xstep = 16;
    }

    kernelContext->params.policy = (vx_uint32)(alpha * 1000);

    vxWriteScalarValue(scalar, &alpha);

    kernelContext->params.evisNoInst = node->base.context->evisNoInst;

#if GC_VX_ASM
    {
        char source[1024] = {0}, s[1024] = {
            "img_load.u8.evis.{0,15} r1 c0 r0.xy\n"
            "img_load.s8.evis.{0,15} r2 c1 r0.xy\n"
            "lerp.u8.evis.rtn.{0,15} r2 r2.u8 r1.u8 %ff\n"
            "img_store.s8.evis.{0,15} r2 c1 r0.xy r2\n"
        };
        sprintf(source, s, alpha);
        kernelContext->params.instructions.source = source;
        kernelContext->params.instructions.regs_count = 3;
    }
#endif

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

vx_status vxAccumulateSquare(vx_node node, vx_image input, vx_scalar scalar, vx_image accum)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 shift = 0u;
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

    vxReadScalarValue(scalar, &shift);

    /*index = 0*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, accum, GC_VX_INDEX_AUTO);

    kernelContext->params.kernel = gcvVX_KERNEL_ACCUMULATE_SQUARE;
    kernelContext->params.xstep = 8;
    kernelContext->params.policy = shift;

    vxWriteScalarValue(scalar, &shift);

#if GC_VX_ASM
    {
        char source[1024] = {
            "img_load.evis.u8.{0,15} r1 c0 r0.xy\n"
            "img_load.evis.s16.{0,7} r2 c1 r0.xy\n"
            "iacc_sq.evis.s16.clamp.{0,7} r2 r2.u8 r1.u8 %d\n"
            "img_store.evis.s16.{0,7} r2 c1 r0.xy r2\n"
        };
        sprintf(source, source, shift);
        kernelContext->params.instructions.source = source;
        kernelContext->params.instructions.regs_count = 3;
    }
#endif

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

