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
#define THRESHOLD 0.00001f
#define ROUNDF(x) floor(x + THRESHOLD)

vx_status vxMultiply(vx_node node, vx_image in0, vx_image in1, vx_scalar scale_param, vx_scalar opolicy_param, vx_scalar rpolicy_param, vx_image output)
{
    vx_status status                = VX_SUCCESS;
    vx_float32 scale                = 0.0f;
    vx_float32    logs = 0.0f, logr = 0.0f;
    vx_enum overflow_policy         = -1;
    vx_enum rounding_policy         = -1;
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
    }

    status |= vxAccessScalarValue(scale_param, &scale);
    status |= vxAccessScalarValue(opolicy_param, &overflow_policy);
    status |= vxAccessScalarValue(rpolicy_param, &rounding_policy);

    /*index = 0*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, in0, kernelContext->objects_num);

    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, in1, kernelContext->objects_num);

    /*index = 2*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, kernelContext->objects_num);

    kernelContext->params.policy           = (overflow_policy == VX_CONVERT_POLICY_SATURATE)?gcvTRUE:gcvFALSE;
    kernelContext->params.rounding         = (rounding_policy == VX_ROUND_POLICY_TO_NEAREST_EVEN) ? 1 << 6 : 0;

    kernelContext->params.kernel           = gcvVX_KERNEL_MULTIPLY;

    logs = -gcoMATH_Log2(scale);
    logr = (vx_float32)ROUNDF(logs);

    /* check if the scale is the integer power of 2 */
    if(logs - logr < THRESHOLD)
    {
        kernelContext->params.scale            = logr;
        kernelContext->params.xstep            = 8;
    }
    else
    {
        kernelContext->params.scale            = scale;
        kernelContext->params.xstep            = 4;
    }

    status = gcfVX_Kernel(kernelContext);

#if gcdVX_OPTIMIZER
    if (!node || !node->kernelContext)
    {
        vxFree(kernelContext);
    }
#endif

    return status;
}
