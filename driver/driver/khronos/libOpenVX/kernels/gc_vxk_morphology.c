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

vx_status _gcfVX_Morphology(gceVX_KERNEL kernel, vx_image src, vx_image dst, vx_border_mode_t *bordermode)
{
    gcoVX_Kernel_Context context = {{0}};
    vx_uint32 height;
    vx_uint32 rect[1];

    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
    rect[0] = height;

	/*index = 0*/
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, src, GC_VX_INDEX_AUTO);

	/*index = 1*/
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, dst, GC_VX_INDEX_AUTO);

    context.params.kernel		= kernel;

    context.params.xstep		= 8;
    context.params.ystep        = height;
    context.params.borders	= bordermode->mode;

    gcoOS_MemCopy(&context.uniforms[0].uniform, rect, sizeof(rect));
    context.uniforms[0].index       = 2;
    context.uniforms[0].num         = 4;
    context.uniform_num             = 1;

    if(bordermode->mode == VX_BORDER_MODE_CONSTANT)
    {
        vx_uint32 bin[4];

        bin[0] =
        bin[1] =
        bin[2] =
        bin[3] = FORMAT_VALUE(bordermode->constant_value);

        gcoOS_MemCopy(&context.uniforms[1].uniform, bin, sizeof(bin));
        context.uniforms[1].num = 4 * 4;
        context.uniforms[1].index = 3;
        context.uniform_num = 2;
    }

    return gcfVX_Kernel(&context);
}

/* nodeless version of the Erode3x3 kernel*/
vx_status vxErode3x3(vx_image src, vx_image dst, vx_border_mode_t *bordermode)
{
    return _gcfVX_Morphology(gcvVX_KERNEL_ERODE_3x3, src, dst, bordermode);
}

/* nodeless version of the Dilate3x3 kernel*/
vx_status vxDilate3x3(vx_image src, vx_image dst, vx_border_mode_t *bordermode)
{
    return _gcfVX_Morphology(gcvVX_KERNEL_DILATE_3x3, src, dst, bordermode);
}

