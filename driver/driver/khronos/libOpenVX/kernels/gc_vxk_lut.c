/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include <gc_vxk_common.h>

vx_status vxTableLookup(vx_node node, vx_image src, vx_lut lut, vx_image dst)
{
    vx_status status = VX_SUCCESS;

    vx_uint32 constantData[8] = {0, 8, 16, 24, 0, 0, 0, 0};
    vx_uint8 bin[16] = {0, 32, 64, 96, 0, 0, 0, 0, 8, 8, 8, 8, 0, 0, 0, 0};
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
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_LUT, lut, GC_VX_INDEX_AUTO);

    /*index = 2*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, dst, GC_VX_INDEX_AUTO);

    gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, constantData, sizeof(constantData));
    kernelContext->uniforms[0].index       = 3;
    kernelContext->uniforms[0].num         = sizeof(constantData) / sizeof(vx_uint32);

    gcoOS_MemCopy(&kernelContext->uniforms[1].uniform, bin, sizeof(bin));
    kernelContext->uniforms[1].index = 4;
    kernelContext->uniforms[1].num = sizeof(bin) / sizeof(vx_uint8);
    kernelContext->uniform_num = 2;

    kernelContext->params.kernel = gcvVX_KERNEL_TABLE_LOOKUP;
    kernelContext->params.xstep = 16;

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

