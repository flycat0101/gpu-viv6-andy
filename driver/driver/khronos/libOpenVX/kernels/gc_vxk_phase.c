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

vx_status vxPhase(vx_node node, vx_image grad_x, vx_image grad_y, vx_image output)
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
#if gcdVX_OPTIMIZER > 2
    kernelContext->uniforms[0].index = 3;
#endif

    if (node->base.context->evisNoInst.isVX2 || node->base.context->evisNoInst.noMagPhase)
    {
        vx_uint8 bin[16] = {0, 32, 64, 96, 0, 0, 0, 0, 8, 8, 8, 8, 0, 0, 0, 0};

        gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, bin, sizeof(bin));
        kernelContext->uniforms[0].index = 3;
        kernelContext->uniforms[0].num = vxmLENGTH_OF(bin);
        kernelContext->uniform_num = 1;

        kernelContext->params.evisNoInst = node->base.context->evisNoInst;
    }

    kernelContext->params.kernel = gcvVX_KERNEL_PHASE;
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

vx_status vxPhase_F16(vx_node node, vx_image grad_x, vx_image grad_y, vx_image output)
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
#if gcdVX_OPTIMIZER > 2
    kernelContext->uniforms[0].index = 3;
#endif

    if (node->base.context->evisNoInst.isVX2 || node->base.context->evisNoInst.noMagPhase)
    {
        vx_uint8 bin[16] = {0, 32, 64, 96, 0, 0, 0, 0, 8, 8, 8, 8, 0, 0, 0, 0};

        vx_uint32 dp_fp16tofp32[16] =
        {
            0x01010101, /* TCfg */
            0x00000000, /* ASelt*/
            0x00010000, 0x00030002, /* ABin*/
            0x02020202, /* BSelt*/
            0x00000000, 0x00000000, /* BBin*/
            0x00000100, /* AccumType, ConstantType, and PostShift*/
            0x00003c00, 0x00000000, 0x00003c00, 0x00000000, 0x00003c00, 0x00000000, 0x00003c00, 0x00000000 /* Constant*/
        };

        vx_uint32 dp_fp16tofp32_4_7[16] = {
            0x01010101, /* TCfg*/
            0x00000000, /* ASelt*/
            0x00050004, 0x00070006, /* ABin*/
            0x02020202, /* BSelt*/
            0x00000000, 0x00000000, /* BBin*/
            0x00000100, /* AccumType, ConstantType, and PostShift*/
            0x00003c00, 0x00000000, 0x00003c00, 0x00000000, 0x00003c00, 0x00000000, 0x00003c00, 0x00000000 /* Constant*/
        }; /* 4-7 f16 -> f32*/

        gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, bin, sizeof(bin));
        kernelContext->uniforms[0].index = 3;
        kernelContext->uniforms[0].num = vxmLENGTH_OF(bin);
        kernelContext->uniform_num = 1;

        gcoOS_MemCopy(&kernelContext->uniforms[1].uniform, dp_fp16tofp32, sizeof(dp_fp16tofp32));
        kernelContext->uniforms[1].index       = 4;
        kernelContext->uniforms[1].num         = 16 * 4;

        gcoOS_MemCopy(&kernelContext->uniforms[2].uniform, dp_fp16tofp32_4_7, sizeof(dp_fp16tofp32_4_7));
        kernelContext->uniforms[2].index       = 8;
        kernelContext->uniforms[2].num         = 16 * 4;
        kernelContext->uniform_num = 3;

        kernelContext->params.evisNoInst = node->base.context->evisNoInst;
    }
    else
    {
        vx_uint32 dp2x8_f16tos16[16] = {
            0x11111111, /* TCfg*/
            0x00000000, /* ASelt*/
            0x03020100, 0x07060504, /* ABin*/
            0x22222222, /* BSelt*/
            0x00000000, 0x00000000, /* BBin*/
            0x00000100, /* AccumType, ConstantType, and PostShift*/
            0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00 /* Constant*/
        };/* dp2x8 f16 -> s16 */

        gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, dp2x8_f16tos16, sizeof(dp2x8_f16tos16));
        kernelContext->uniforms[0].index = 3;
        kernelContext->uniforms[0].num = 16 * 4;
        kernelContext->uniform_num = 1;
    }

    kernelContext->params.kernel = gcvVX_KERNEL_PHASE;
    kernelContext->params.xstep = 8;
    kernelContext->params.policy = VX_DF_IMAGE('F','0','1','6');/*temp for type*/
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

