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

vx_status vxAccumulate(vx_image input, vx_image accum)
{
    gcoVX_Kernel_Context context = {{0}};

    /*index = 0*/
    gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, accum, GC_VX_INDEX_AUTO);

    context.params.kernel = gcvVX_KERNEL_ACCUMULATE;
    context.params.xstep = 8;

    return gcfVX_Kernel(&context);
}

vx_status vxAccumulateWeighted(vx_image input, vx_scalar scalar, vx_image accum)
{
    vx_float32 alpha = 0u;
    gcoVX_Kernel_Context context = {{0}};

    vxAccessScalarValue(scalar, &alpha);

    /*index = 0*/
    gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, accum, GC_VX_INDEX_AUTO);

    context.params.kernel = gcvVX_KERNEL_ACCUMULATE_WEIGHTED;
    context.params.xstep = 16;
    context.params.policy = (gctUINT32)(alpha * 1000);

    vxCommitScalarValue(scalar, &alpha);

    return gcfVX_Kernel(&context);
}

vx_status vxAccumulateSquare(vx_image input, vx_scalar scalar, vx_image accum)
{
    vx_uint32 shift = 0u;
    gcoVX_Kernel_Context context = {{0}};

    vxAccessScalarValue(scalar, &shift);

    /*index = 0*/
    gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, accum, GC_VX_INDEX_AUTO);

    context.params.kernel = gcvVX_KERNEL_ACCUMULATE_SQUARE;
    context.params.xstep = 8;
    context.params.policy = shift;

    vxCommitScalarValue(scalar, &shift);

    return gcfVX_Kernel(&context);
}

