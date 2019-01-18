/****************************************************************************
*
*    Copyright (c) 2005 - 2019 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include <gc_vxk_common.h>

vx_status _gcfVX_AddSubOpration(vx_node node, gceVX_KERNEL kernel, vx_image in0, vx_image in1, vx_scalar policy_param, vx_image output)
{
    vx_status status = VX_SUCCESS;
    vx_enum overflow_policy = -1;
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

        memset(&kernelContext->params, 0, sizeof(gcsVX_KERNEL_PARAMETERS));
    }

    vxReadScalarValue(policy_param, &overflow_policy);

    /*index = 0*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, in0, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, in1, GC_VX_INDEX_AUTO);

    /*index = 2*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

    kernelContext->params.kernel           = kernel;
    kernelContext->params.xstep            = 8;

    kernelContext->params.policy           = (overflow_policy == VX_CONVERT_POLICY_SATURATE)?gcvTRUE:gcvFALSE;

    kernelContext->params.evisNoInst       = node->base.context->evisNoInst;

    vxWriteScalarValue(policy_param, &overflow_policy);

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

vx_status vxAddition(vx_node node, vx_image in0, vx_image in1, vx_scalar policy_param, vx_image output)
{
    return _gcfVX_AddSubOpration(node, gcvVX_KERNEL_ADD, in0, in1, policy_param, output);
}

/* nodeless version of the Subtraction kernel*/
vx_status vxSubtraction(vx_node node, vx_image in0, vx_image in1, vx_scalar policy_param, vx_image output)
{
    return _gcfVX_AddSubOpration(node, gcvVX_KERNEL_SUBTRACT, in0, in1, policy_param, output);
}

