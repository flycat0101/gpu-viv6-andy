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

vx_status vxAbsDiff(vx_node node, vx_image in1, vx_image in2, vx_image output)
{
    vx_status status = VX_SUCCESS;
    gcoVX_Kernel_Context * kernelContext = gcvNULL;
    vx_df_image inputFormat = 0;

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

    vxQueryImage(in1, VX_IMAGE_ATTRIBUTE_FORMAT, &inputFormat, sizeof(vx_df_image));

    /*index = 0*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, in1, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, in2, GC_VX_INDEX_AUTO);

    /*index = 2*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

    kernelContext->params.kernel = gcvVX_KERNEL_ABSDIFF;
    if (inputFormat == VX_DF_IMAGE_U8)
    {
        kernelContext->params.xstep = 16;
    }
    else
    {
        kernelContext->params.xstep = 8;
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

