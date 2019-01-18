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

#include <VX/vx_types.h>
#include <stdio.h>

vx_status vxConvertDepth(vx_node node, vx_image input, vx_image output, vx_scalar spol, vx_scalar sshf)
{
    vx_status status = VX_SUCCESS;
    vx_enum policy = 0;
    vx_int32 shift = 0;
    vx_df_image inputFormat, outputFormat;
    vx_uint32 bin[4];
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

    vxQueryImage(input, VX_IMAGE_FORMAT, &inputFormat, sizeof(inputFormat));
    vxQueryImage(output, VX_IMAGE_FORMAT, &outputFormat, sizeof(outputFormat));

    status = vxReadScalarValue(spol, &policy);
    status = vxReadScalarValue(sshf, &shift);

    /*index = 0*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

    kernelContext->params.kernel        = gcvVX_KERNEL_CONVERTDEPTH;
    kernelContext->params.policy        = (policy == VX_CONVERT_POLICY_SATURATE)?gcvTRUE:gcvFALSE;
    kernelContext->params.volume        = shift;

    if (outputFormat == VX_DF_IMAGE_S32 || outputFormat == VX_DF_IMAGE_U32)
    {
        kernelContext->params.xstep = 4;
        bin[0] = shift;
    }
    else if (outputFormat == VX_DF_IMAGE_S16 || outputFormat == VX_DF_IMAGE_U16)
    {
        kernelContext->params.xstep = 8;
        bin[0] = FV2(shift);
    }
    else
    {
        kernelContext->params.xstep = 8;
        bin[0] = FV(shift);
        bin[1] = FV2(1);
    }
    gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, bin, sizeof(bin));
    kernelContext->uniforms[0].num = 4 * 4;
    kernelContext->uniforms[0].index = 2;
    kernelContext->uniform_num = 1;

    kernelContext->params.evisNoInst = node->base.context->evisNoInst;

    kernelContext->node = node;

    status = gcfVX_Kernel(kernelContext);

#if gcdVX_OPTIMIZER
    if (!node || !node->kernelContext)
    {
        vxFree(kernelContext);
    }
#endif

    status = vxWriteScalarValue(spol, &policy);
    status = vxWriteScalarValue(sshf, &shift);

    return status;
}

