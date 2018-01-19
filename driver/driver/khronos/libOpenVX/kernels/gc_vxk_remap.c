/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include <gc_vxk_common.h>

vx_status vxRemap(vx_node node, vx_image input, vx_remap remap, vx_enum policy, vx_border_t *borders, vx_image output)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 width = 0, height = 0;
    gcoVX_Kernel_Context * kernelContext = gcvNULL;
    vx_int32 uniformNum = 0;
    vx_int32 index = 0;

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

    vxQueryRemap(remap, VX_REMAP_DESTINATION_WIDTH, &width, sizeof(width));
    vxQueryRemap(remap, VX_REMAP_DESTINATION_HEIGHT, &height, sizeof(height));

    /*index = 0*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_REMAP, remap, GC_VX_INDEX_AUTO);

    /*index = 2*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

    kernelContext->params.kernel = gcvVX_KERNEL_REMAP;
    kernelContext->params.xstep = 16;
    kernelContext->params.policy           = (policy == VX_INTERPOLATION_NEAREST_NEIGHBOR)?gcvVX_INTERPOLATION_TYPE_NEAREST_NEIGHBOR:(policy == VX_INTERPOLATION_BILINEAR?gcvVX_INTERPOLATION_TYPE_BILINEAR:gcvVX_INTERPOLATION_TYPE_NEAREST_NEIGHBOR);
#if gcdVX_OPTIMIZER
    kernelContext->borders                 = borders->mode;
#else
    kernelContext->params.borders          = borders->mode;
#endif
    kernelContext->params.constant_value   = borders->constant_value.U32;

    kernelContext->params.xmax = width;
    kernelContext->params.ymax = height;

    if(borders->mode == VX_BORDER_CONSTANT || borders->mode == VX_BORDER_UNDEFINED)
    {
        vx_uint32 bin[4];
        bin[0] =
        bin[1] =
        bin[2] =
        bin[3] = FORMAT_VALUE(borders->mode == VX_BORDER_CONSTANT?borders->constant_value.U32:0xcd);

        gcoOS_MemCopy(&kernelContext->uniforms[index].uniform, bin, sizeof(bin));
        kernelContext->uniforms[index].num = 4 * 4;
        kernelContext->uniforms[index++].index = 4;
        uniformNum++;
    }

    if (node->base.context->evisNoInst.isVX2 || node->base.context->evisNoInst.noBilinear)
    {
        vx_uint32 constantData[8] = {0, 8, 16, 24, 0, 0, 0, 0};
        gcoOS_MemCopy(&kernelContext->uniforms[1].uniform, constantData, sizeof(constantData));
        kernelContext->uniforms[index].num = vxmLENGTH_OF(constantData);
        kernelContext->uniforms[index++].index = 5;
        uniformNum++;
    }

    kernelContext->uniform_num = uniformNum;

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

