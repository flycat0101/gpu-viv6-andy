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

#if VIV_NORM
vx_status vxNorm(vx_node node, vx_image input_x, vx_image input_y, vx_scalar norm_type, vx_image output)
{
    gcoVX_Kernel_Context * kernelContext = gcvNULL;
    vx_status status = VX_SUCCESS;
    vx_enum norm_type_value;
    vx_uint32 width;
    vx_uint32 height;

    vxReadScalarValue(norm_type, &norm_type_value);

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

    vxQueryImage(output, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
    vxQueryImage(output, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));

    /*index = 0*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input_x, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input_y, GC_VX_INDEX_AUTO);

    /*index = 2*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

    kernelContext->params.row       = width;
    kernelContext->params.col       = height;
    kernelContext->params.xmax      = width;
    kernelContext->params.ymax      = height;
    kernelContext->params.xstep     = 8;
    if (norm_type_value == VX_NORM_L1)
    {
        kernelContext->params.kernel    = gcvVX_KERNEL_ELEMENTWISE_NORM;
    }
    else if (norm_type_value == VX_NORM_L2)
    {
        kernelContext->params.kernel    = gcvVX_KERNEL_MAGNITUDE;
        if (node->base.context->evisNoInst.noMagPhase)
        {
            vx_uint8 bin[16] = {0, 32, 64, 96, 0, 0, 0, 0, 16, 16, 16, 16, 0, 0, 0, 0};
            gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, bin, sizeof(bin));
            kernelContext->uniforms[0].index = 3;
            kernelContext->uniforms[0].num = sizeof(bin) / sizeof(vx_uint8);
            kernelContext->uniform_num = 1;
            kernelContext->params.evisNoInst = node->base.context->evisNoInst;
        }
    }

    kernelContext->params.evisNoInst = node->base.context->evisNoInst;

    kernelContext->node = node;

    status = gcfVX_Kernel(kernelContext);

    vxWriteScalarValue(norm_type, &norm_type_value);
#if gcdVX_OPTIMIZER
    if (!node || !node->kernelContext)
    {
        vxFree(kernelContext);
    }
#endif

    return status;
}
#endif

#if VIV_EDGE
#define ET_THRESHOLD 0
#define ET_HYST   1
#define ET_CLAMP   2

vx_status vxEdgeTraceThreshold(vx_node node, vx_image input, vx_threshold threshold, vx_image output)
{
    vx_status status = VX_SUCCESS;
    vx_int32 lower = 0, upper = 0;
    vx_uint32 bin[4];
    vx_uint32 constantData[2];
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

    constantData[0] = FV2(15);
    constantData[1] = FV2(8);

    vxQueryThreshold(threshold, VX_THRESHOLD_ATTRIBUTE_THRESHOLD_LOWER, &lower, sizeof(lower));
    vxQueryThreshold(threshold, VX_THRESHOLD_ATTRIBUTE_THRESHOLD_UPPER, &upper, sizeof(upper));

    bin[0] = FV2(lower+1);
    bin[1] = FV2(upper);
    bin[2] = FV2(upper+1);
    bin[3] = FV2(65535u);

    /*index = 0*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

    gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, bin, sizeof(bin));
    kernelContext->uniforms[0].index           = 2;
    kernelContext->uniforms[0].num             = 16;
    gcoOS_MemCopy(&kernelContext->uniforms[1].uniform, constantData, sizeof(constantData));
    kernelContext->uniforms[1].index           = 3;
    kernelContext->uniforms[1].num             = 8;
    kernelContext->uniform_num                 = 2;

    kernelContext->params.xstep                = 8;
    kernelContext->params.step                 = ET_THRESHOLD;
    kernelContext->params.kernel               = gcvVX_KERNEL_EDGE_TRACE;

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

vx_status vxEdgeTraceHysteresis(vx_node node, vx_image input, vx_scalar flag)
{
    vx_status status = VX_SUCCESS;
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

    vxQueryImage(input, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
    vxQueryImage(input, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));

    /*index = 0*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);
    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_SCALAR, flag, GC_VX_INDEX_AUTO);

    if (node->base.context->evisNoInst.clamp8Output ||
        node->base.context->evisNoInst.noFilter ||
        node->base.context->evisNoInst.noIAdd)
    {
        vx_uint32 bin[4];
        vx_uint8 data[16] = {0, 32, 64, 96, 0, 0, 0, 0, 8, 8, 8, 8, 0, 0, 0, 0};

        bin[0] = width;
        bin[1] = height;
        bin[2] = FV(1);

        gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, bin, sizeof(bin));
        kernelContext->uniforms[0].index           = 2;
        kernelContext->uniforms[0].num             = 4 * 4;
        gcoOS_MemCopy(&kernelContext->uniforms[1].uniform, data, sizeof(data));
        kernelContext->uniforms[1].num             = sizeof(data) / sizeof(vx_uint8);
        kernelContext->uniforms[1].index           = 3;
        kernelContext->uniform_num                 = 2;

        kernelContext->params.xstep                = 2;

        kernelContext->evisNoInst = node->base.context->evisNoInst;
    }
    else
    {
        vx_uint32 bin[2];

        bin[0] = width;
        bin[1] = height;

        gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, bin, sizeof(bin));
        kernelContext->uniforms[0].index           = 2;
        kernelContext->uniforms[0].num             = 8;
        kernelContext->uniform_num                 = 1;

        kernelContext->params.xstep                = 8;
    }

    kernelContext->params.ystep                = height;
#if gcdVX_OPTIMIZER
    kernelContext->borders                     = gcvVX_BORDER_MODE_UNDEFINED;
#else
    kernelContext->params.borders              = gcvVX_BORDER_MODE_UNDEFINED;
#endif
    kernelContext->params.step                 = ET_HYST;
    kernelContext->params.kernel               = gcvVX_KERNEL_EDGE_TRACE;

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

vx_status vxEdgeTraceClamp(vx_node node, vx_image input, vx_image output)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 bin[2];
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

    bin[0] = FORMAT_VALUE(7);
    bin[1] = FORMAT_VALUE(255u);

    /*index = 0*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

    gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, bin, sizeof(bin));
    kernelContext->uniforms[0].index           = 2;
    kernelContext->uniforms[0].num             = 8;
    kernelContext->uniform_num                 = 1;

    kernelContext->params.xstep                = 8;
    kernelContext->params.step                 = ET_CLAMP;
    kernelContext->params.kernel               = gcvVX_KERNEL_EDGE_TRACE;

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

#endif

