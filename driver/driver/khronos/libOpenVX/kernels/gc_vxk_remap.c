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

vx_status vxRemap(vx_image input, vx_remap remap, vx_enum policy, vx_border_mode_t *borders, vx_image output)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 width = 0, height = 0;
    gcoVX_Kernel_Context context = {{0}};

    vxQueryRemap(remap, VX_REMAP_ATTRIBUTE_DESTINATION_WIDTH, &width, sizeof(width));
    vxQueryRemap(remap, VX_REMAP_ATTRIBUTE_DESTINATION_HEIGHT, &height, sizeof(height));

	/*index = 0*/
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

	/*index = 1*/
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_REMAP, remap, GC_VX_INDEX_AUTO);

	/*index = 2*/
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

    context.params.kernel = gcvVX_KERNEL_REMAP;
    context.params.xstep = 1;
    context.params.policy           = (policy == VX_INTERPOLATION_TYPE_NEAREST_NEIGHBOR)?gcvVX_INTERPOLATION_TYPE_NEAREST_NEIGHBOR:(policy == VX_INTERPOLATION_TYPE_BILINEAR?gcvVX_INTERPOLATION_TYPE_BILINEAR:gcvVX_INTERPOLATION_TYPE_NEAREST_NEIGHBOR);
    context.params.borders          = borders->mode;
    context.params.constant_value   = borders->constant_value;

    context.params.xmax = width;
    context.params.ymax = height;

    if(borders->mode == VX_BORDER_MODE_CONSTANT || borders->mode == VX_BORDER_MODE_UNDEFINED)
    {
        vx_uint32 bin[4];
        bin[0] =
        bin[1] =
        bin[2] =
        bin[3] = FORMAT_VALUE(borders->mode == VX_BORDER_MODE_CONSTANT?borders->constant_value:0xcd);

        gcoOS_MemCopy(&context.uniforms[0].uniform, bin, sizeof(bin));
        context.uniforms[0].num = 4 * 4;
        context.uniforms[0].index = 4;
        context.uniform_num = 1;
    }

    status = gcfVX_Kernel(&context);

    return status;
}
