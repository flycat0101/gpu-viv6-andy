/****************************************************************************
*
*    Copyright (c) 2005 - 2015 by Vivante Corp.  All rights reserved.
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

vx_status vxMultiply(vx_image in0, vx_image in1, vx_scalar scale_param, vx_scalar opolicy_param, vx_scalar rpolicy_param, vx_image output)
{
    vx_status status                = VX_SUCCESS;
    gcoVX_Kernel_Context context    = {{0}};
    vx_float32 scale                = 0.0f;
    vx_float32    logs = 0.0f, logr = 0.0f;
    vx_enum overflow_policy         = -1;
    vx_enum rounding_policy         = -1;

    status |= vxAccessScalarValue(scale_param, &scale);
    status |= vxAccessScalarValue(opolicy_param, &overflow_policy);
    status |= vxAccessScalarValue(rpolicy_param, &rounding_policy);

    /*index = 0*/
    gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, in0, context.objects_num);

    /*index = 1*/
    gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, in1, context.objects_num);

    /*index = 2*/
    gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, context.objects_num);

    context.params.policy           = (overflow_policy == VX_CONVERT_POLICY_SATURATE)?gcvTRUE:gcvFALSE;
    context.params.rounding         = (rounding_policy == VX_ROUND_POLICY_TO_NEAREST_EVEN) ? 1 << 6 : 0;

    context.params.kernel           = gcvVX_KERNEL_MULTIPLY;

    logs = -gcoMATH_Log2(scale);
    logr = (vx_float32)ROUNDF(logs);

    /* check if the scale is the integer power of 2 */
    if(logs - logr < THRESHOLD)
    {
        context.params.scale            = logr;
        context.params.xstep            = 8;
    }
    else
    {
        context.params.scale            = scale;
        context.params.xstep            = 4;
    }

    return gcfVX_Kernel(&context);
}

