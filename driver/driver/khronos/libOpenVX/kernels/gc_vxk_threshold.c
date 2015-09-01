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

vx_status vxThreshold(vx_image src_image, vx_threshold threshold, vx_image dst_image)
{
    vx_enum type = 0;
    vx_int32 value = 0, lower = 0, upper = 0;
    gcoVX_Kernel_Context context = {{0}};
    vx_uint32 bin[2];

    vxQueryThreshold(threshold, VX_THRESHOLD_ATTRIBUTE_TYPE, &type, sizeof(type));

    if (type == VX_THRESHOLD_TYPE_BINARY)
    {
        vxQueryThreshold(threshold, VX_THRESHOLD_ATTRIBUTE_THRESHOLD_VALUE, &value, sizeof(value));
        bin[0] = FORMAT_VALUE(value + 1);
        bin[1] = FORMAT_VALUE(0xffU);

        gcoOS_MemCopy(&context.uniforms[0].uniform, bin, sizeof(bin));
        context.uniforms[0].num = 2 * 4;
        /* policy == 1, the type is binary, otherwise range */
        context.params.policy = 1;
    }
    else if (type == VX_THRESHOLD_TYPE_RANGE)
    {
        vxQueryThreshold(threshold, VX_THRESHOLD_ATTRIBUTE_THRESHOLD_LOWER, &lower, sizeof(lower));
        vxQueryThreshold(threshold, VX_THRESHOLD_ATTRIBUTE_THRESHOLD_UPPER, &upper, sizeof(upper));
        bin[0] = FORMAT_VALUE(lower);
        bin[1] = FORMAT_VALUE(upper);

        gcoOS_MemCopy(&context.uniforms[0].uniform, bin, sizeof(bin));
        context.uniforms[0].num = 2 * 4;
    }

    context.uniforms[0].index = 2;
    context.uniform_num = 1;

	/*index = 0*/
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, src_image, GC_VX_INDEX_AUTO);

	/*index = 1*/
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, dst_image, GC_VX_INDEX_AUTO);

    context.params.kernel = gcvVX_KERNEL_THRESHOLD;
    context.params.xstep = 16;

    return gcfVX_Kernel(&context);
}

