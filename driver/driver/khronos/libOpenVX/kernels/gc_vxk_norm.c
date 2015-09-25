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

#if VIV_NORM
vx_status vxNorm(vx_image input_x, vx_image input_y, vx_scalar norm_type, vx_image output)
{
    vx_status status = VX_SUCCESS;
    vx_enum norm_type_value;

    vxAccessScalarValue(norm_type, &norm_type_value);

    if (norm_type_value == VX_NORM_L1)
    {
        gcoVX_Kernel_Context context = {{0}};
        vx_uint32 width;
        vx_uint32 height;

        vxQueryImage(output, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
        vxQueryImage(output, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));

        /*index = 0*/
        gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input_x, GC_VX_INDEX_AUTO);

        /*index = 1*/
        gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input_y, GC_VX_INDEX_AUTO);

        /*index = 2*/
        gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

        context.params.row                  = width;
        context.params.col                  = height;
        context.params.xstep                = 8;

        context.params.kernel = gcvVX_KERNEL_ELEMENTWISE_NORM;

        status = gcfVX_Kernel(&context);
    }
    else if (norm_type_value == VX_NORM_L2)
    {
        status = vxMagnitude(input_x, input_y, output);
    }
    return status;
}
#endif

#if VIV_EDGE
#define ET_THRESHOLD 0
#define ET_HYST   1
#define ET_CLAMP   2

vx_status vivEdgeTraceThreshold(vx_image input, vx_threshold threshold, vx_image output)
{
    vx_status status = VX_SUCCESS;
    gcoVX_Kernel_Context context = {{0}};
    vx_int32 lower = 0, upper = 0;
    vx_uint32 bin[4];
    vx_uint32 constantData[2];

    constantData[0] = FV2(15);
    constantData[1] = FV2(8);

    vxQueryThreshold(threshold, VX_THRESHOLD_ATTRIBUTE_THRESHOLD_LOWER, &lower, sizeof(lower));
    vxQueryThreshold(threshold, VX_THRESHOLD_ATTRIBUTE_THRESHOLD_UPPER, &upper, sizeof(upper));

    bin[0] = FV2(lower+1);
    bin[1] = FV2(upper);
    bin[2] = FV2(upper+1);
    bin[3] = FV2(65535U);

    /*index = 0*/
    gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

    gcoOS_MemCopy(&context.uniforms[0].uniform, bin, sizeof(bin));
    context.uniforms[0].index           = 2;
    context.uniforms[0].num             = 16;
    gcoOS_MemCopy(&context.uniforms[1].uniform, constantData, sizeof(constantData));
    context.uniforms[1].index           = 3;
    context.uniforms[1].num             = 8;
    context.uniform_num                 = 2;

    context.params.xstep                = 8;
    context.params.step                 = ET_THRESHOLD;
    context.params.kernel               = gcvVX_KERNEL_EDGE_TRACE;

    status = gcfVX_Kernel(&context);
    return status;
}

vx_status vivEdgeTraceHysteresis(vx_image input, vx_image output, vx_scalar flag)
{
    vx_status status = VX_SUCCESS;
    gcoVX_Kernel_Context context = {{0}};
    vx_uint32 width;
    vx_uint32 height;
    vx_uint32 bin[2];

    vxQueryImage(input, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
    vxQueryImage(input, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
    bin[0] = width;
    bin[1] = height;

    /*index = 0*/
    gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

    /*index = 2*/
    gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_SCALAR, flag, GC_VX_INDEX_AUTO);

    gcoOS_MemCopy(&context.uniforms[0].uniform, bin, sizeof(bin));
    context.uniforms[0].index           = 3;
    context.uniforms[0].num             = 8;
    context.uniform_num                 = 1;

    context.params.xstep                = 8;
    context.params.ystep                = height;
    context.params.borders              = gcvVX_BORDER_MODE_UNDEFINED;
    context.params.step                 = ET_HYST;
    context.params.kernel               = gcvVX_KERNEL_EDGE_TRACE;

    status = gcfVX_Kernel(&context);

    return status;
}

vx_status vivEdgeTraceClamp(vx_image input, vx_image output)
{
    vx_status status = VX_SUCCESS;
    gcoVX_Kernel_Context context = {{0}};
    vx_uint32 bin[2];

    bin[0] = FORMAT_VALUE(7U);
    bin[1] = FORMAT_VALUE(255U);

    /*index = 0*/
    gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

    gcoOS_MemCopy(&context.uniforms[0].uniform, bin, sizeof(bin));
    context.uniforms[0].index           = 2;
    context.uniforms[0].num             = 8;
    context.uniform_num                 = 1;

    context.params.xstep                = 8;
    context.params.step                 = ET_CLAMP;
    context.params.kernel               = gcvVX_KERNEL_EDGE_TRACE;

    status = gcfVX_Kernel(&context);
    return status;
}

vx_status vxEdgeTrace(vx_image norm, vx_threshold threshold, vx_image output, vx_reference* staging)
{
    vx_status status = VX_SUCCESS;
    vx_image img[2] = {gcvNULL, gcvNULL};
    vx_scalar countScalar;
    vx_uint32 flag = 1, zero = 0;
    vx_uint32 curr, next = 0;
    vx_uint32 j = 0;

    img[0] = (vx_image)staging[0];
    img[1] = (vx_image)staging[1];
    countScalar = (vx_scalar)staging[2];

    status = vivEdgeTraceThreshold(norm,threshold,img[0]);
    /* To Clean up */
    status |= gcfVX_Flush(gcvTRUE);

    while (flag > 0 && j < 11)
    {
        curr = j % 2;
        next = ++j % 2;
        status |= vivEdgeTraceHysteresis(img[curr], img[next], countScalar);
        /* To Clean up */
        status |= gcfVX_Flush(gcvTRUE);
        status |= vxAccessScalarValue(countScalar, &flag);
        status |= vxCommitScalarValue(countScalar, &zero);
    }
    status = vivEdgeTraceClamp(img[next],output);
    /* To Clean up */
    status |= gcfVX_Flush(gcvTRUE);

    return status;
}
#endif
