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

vx_status vxThreshold(vx_node node, vx_image src_image, vx_threshold threshold, vx_image dst_image)
{
    vx_status status = VX_SUCCESS;
    vx_enum type = 0;
    vx_int32 value = 0, lower = 0, upper = 0;
    vx_uint32 bin[2];
    gcoVX_Kernel_Context * kernelContext = gcvNULL;
    vx_uint32 width = 0, height = 0;
    vx_uint32 cnst[4];
    vx_int32 true_value = 0, false_value = 0;
    vx_uint8 trueValue = 0, falseValue = 0;

    status |= vxQueryImage(src_image, VX_IMAGE_WIDTH, &width, sizeof(width));
    status |= vxQueryImage(src_image, VX_IMAGE_HEIGHT, &height, sizeof(height));

    status |= vxQueryThreshold(threshold, VX_THRESHOLD_TRUE_VALUE, &true_value, sizeof(true_value));
    status |= vxQueryThreshold(threshold, VX_THRESHOLD_FALSE_VALUE, &false_value, sizeof(false_value));
    trueValue = (vx_uint8)true_value;
    falseValue = (vx_uint8)false_value;

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
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, src_image, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, dst_image, GC_VX_INDEX_AUTO);

    vxQueryThreshold(threshold, VX_THRESHOLD_TYPE, &type, sizeof(type));

    if (type == VX_THRESHOLD_TYPE_BINARY)
    {
        vxQueryThreshold(threshold, VX_THRESHOLD_THRESHOLD_VALUE, &value, sizeof(value));
        bin[0] = FORMAT_VALUE(value + 1);
        bin[1] = FORMAT_VALUE(0xffu);

        gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, bin, sizeof(bin));
        kernelContext->uniforms[0].num = 2 * 4;
        /* policy == 1, the type is binary, otherwise range */
        kernelContext->params.policy = 1;
    }
    else if (type == VX_THRESHOLD_TYPE_RANGE)
    {
        vxQueryThreshold(threshold, VX_THRESHOLD_THRESHOLD_LOWER, &lower, sizeof(lower));
        vxQueryThreshold(threshold, VX_THRESHOLD_THRESHOLD_UPPER, &upper, sizeof(upper));
        bin[0] = FORMAT_VALUE(lower);
        bin[1] = FORMAT_VALUE(upper);

        gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, bin, sizeof(bin));
        kernelContext->uniforms[0].num = 2 * 4;
    }

    kernelContext->uniforms[0].index = 2;
    kernelContext->uniform_num = 1;

    cnst[0] = 0;
    cnst[1] = 0;
    cnst[2] = 0;
    cnst[3] = 0;

    gcoOS_MemCopy(&kernelContext->uniforms[1].uniform, cnst, sizeof(cnst));
    kernelContext->uniforms[1].index       = 3;
    kernelContext->uniforms[1].num         = 16;
    kernelContext->uniform_num             = 2;

    cnst[0] = 0xffffffff;
    cnst[1] = 0xffffffff;
    cnst[2] = 0xffffffff;
    cnst[3] = 0xffffffff;

    gcoOS_MemCopy(&kernelContext->uniforms[2].uniform, cnst, sizeof(cnst));
    kernelContext->uniforms[2].index       = 4;
    kernelContext->uniforms[2].num         = 16;
    kernelContext->uniform_num             = 3;

    cnst[0] = FORMAT_VALUE(falseValue);
    cnst[1] = FORMAT_VALUE(falseValue);
    cnst[2] = FORMAT_VALUE(falseValue);
    cnst[3] = FORMAT_VALUE(falseValue);

    gcoOS_MemCopy(&kernelContext->uniforms[3].uniform, cnst, sizeof(cnst));
    kernelContext->uniforms[3].index       = 5;
    kernelContext->uniforms[3].num         = 16;
    kernelContext->uniform_num             = 4;

    cnst[0] = FORMAT_VALUE(trueValue);
    cnst[1] = FORMAT_VALUE(trueValue);
    cnst[2] = FORMAT_VALUE(trueValue);
    cnst[3] = FORMAT_VALUE(trueValue);

    gcoOS_MemCopy(&kernelContext->uniforms[4].uniform, cnst, sizeof(cnst));
    kernelContext->uniforms[4].index       = 6;
    kernelContext->uniforms[4].num         = 16;
    kernelContext->uniform_num             = 5;

    kernelContext->params.kernel = gcvVX_KERNEL_THRESHOLD;
    kernelContext->params.xstep = 16;

    kernelContext->params.col        = width;
    kernelContext->params.row        = height;

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


