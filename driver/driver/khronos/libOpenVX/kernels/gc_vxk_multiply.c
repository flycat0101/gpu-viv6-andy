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
#define THRESHOLD 0.00001f
#define ROUNDF(x) floor(x + THRESHOLD)

vx_status vxMultiply(vx_node node, vx_image in0, vx_image in1, vx_scalar scale_param, vx_scalar opolicy_param, vx_scalar rpolicy_param, vx_image output)
{
    vx_status status                = VX_SUCCESS;
    vx_float32 scale                = 0.0f;
    vx_float32    logs = 0.0f, logr = 0.0f;
    vx_enum overflow_policy         = -1;
    vx_enum rounding_policy         = -1;
    vx_float32 frac_part            = 0.0f;
    vx_uint32 int_part              = 0;
    gcoVX_Kernel_Context * kernelContext = gcvNULL;
    vx_df_image format = 0;

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
    vxQueryImage(output, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
    status |= vxReadScalarValue(scale_param, &scale);
    status |= vxReadScalarValue(opolicy_param, &overflow_policy);
    status |= vxReadScalarValue(rpolicy_param, &rounding_policy);

    /*index = 0*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, in0, kernelContext->objects_num);

    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, in1, kernelContext->objects_num);

    /*index = 2*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, kernelContext->objects_num);

    kernelContext->params.policy           = (overflow_policy == VX_CONVERT_POLICY_SATURATE)?gcvTRUE:gcvFALSE;
    kernelContext->params.rounding         = (rounding_policy == VX_ROUND_POLICY_TO_NEAREST_EVEN) ? 1 << 6 : 0;

    kernelContext->params.kernel           = gcvVX_KERNEL_MULTIPLY;

    if (scale == 0)
    {
        kernelContext->params.volume           = 1;
        kernelContext->params.scale            = 0;
        kernelContext->params.xstep            = 8;
    }
    else if (scale <= 1)
    {
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
    }
    else
    {
        int_part = (vx_uint32)floor(scale);
        frac_part = scale - (vx_float32)floor(scale);
        logs = -gcoMATH_Log2(frac_part);
        logr = (vx_float32)ROUNDF(logs);
        kernelContext->params.volume           = 2;
        kernelContext->params.clamp            = int_part;
        kernelContext->params.xstep            = 4;

        /* check if the fractor is the integer power of 2 */
        if (frac_part == 0)
            kernelContext->params.scale            = frac_part;
        else if(logs - logr < THRESHOLD)
            kernelContext->params.scale            = logr;
        else
            kernelContext->params.scale            = frac_part;

        {
            vx_float32 bin[4];

            bin[0] =
            bin[1] =
            bin[2] =
            bin[3] = frac_part;

            gcoOS_MemCopy(&kernelContext->uniforms[kernelContext->uniform_num].uniform, bin, sizeof(bin));
            kernelContext->uniforms[kernelContext->uniform_num].num = 4;
            kernelContext->uniforms[kernelContext->uniform_num++].index = 3;
        }
        if (format == VX_DF_IMAGE_S16 || format == VX_DF_IMAGE_U16)
        {
            vx_uint8 constantData[16] = {0, 32, 64, 96, 0, 0, 0, 0, 16, 16, 16, 16, 0, 0, 0, 0};
            gcoOS_MemCopy(&kernelContext->uniforms[kernelContext->uniform_num].uniform, constantData, sizeof(constantData));
            kernelContext->uniforms[kernelContext->uniform_num].num = sizeof(constantData) / sizeof(vx_uint8);
            kernelContext->uniforms[kernelContext->uniform_num++].index = 4;
        }
        else
        {
            vx_uint8 constantData[16] = {0, 32, 64, 96, 0, 0, 0, 0, 8, 8, 8, 8, 0, 0, 0, 0};
            gcoOS_MemCopy(&kernelContext->uniforms[kernelContext->uniform_num].uniform, constantData, sizeof(constantData));
            kernelContext->uniforms[kernelContext->uniform_num].num = sizeof(constantData) / sizeof(vx_uint8);
            kernelContext->uniforms[kernelContext->uniform_num++].index = 4;
        }

    }

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

