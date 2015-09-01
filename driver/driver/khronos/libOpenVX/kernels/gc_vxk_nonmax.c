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

#if VIV_NONMAX_SUPPRESSION
vx_status vxNonMaxSuppression(vx_image i_mag, vx_image i_ang, vx_image i_edge, vx_border_mode_t *borders)
{
    vx_status status = VX_SUCCESS;
    gcoVX_Kernel_Context context = {{0}};
    vx_uint32 constantData[2] = {0, 16};
    vx_uint32 width;
    vx_uint32 height;
    vxQueryImage(i_edge, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
    vxQueryImage(i_edge, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));

	/*index = 0*/
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, i_mag, GC_VX_INDEX_AUTO);

	/*index = 1*/
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, i_ang, GC_VX_INDEX_AUTO);

	/*index = 2*/
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, i_edge, GC_VX_INDEX_AUTO);

    gcoOS_MemCopy(&context.uniforms[0].uniform, constantData, sizeof(constantData));
    context.uniforms[0].index = 3;
    context.uniforms[0].num = sizeof(constantData) / sizeof(vx_uint32);
    context.uniform_num = 1;

    context.params.kernel     = gcvVX_KERNEL_NONMAXSUPPRESSION;

    context.params.borders    = borders->mode;
    context.params.row        = width;
    context.params.col        = height;
    context.params.xstep      = 1;

    status = gcfVX_Kernel(&context);

    return status;
}
#endif

#if VIV_EUCLIDEAN_NONMAX
vx_status vxEuclideanNonMaxSuppression(vx_image src, vx_scalar thr, vx_scalar rad, vx_image dst)
{
    vx_status status = VX_SUCCESS;
    gcoVX_Kernel_Context context = {0};
    vx_float32 radius = 0.0f, thresh = 0.0f;
    vx_df_image format = VX_DF_IMAGE_VIRT;

    status |= vxQueryImage(src, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));

    status |= vxAccessScalarValue(rad, &radius);
    status |= vxAccessScalarValue(thr, &thresh);

    if (format == VX_DF_IMAGE_F32)
    {
		/*index = 0*/
		gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, src, GC_VX_INDEX_AUTO);

		/*index = 1*/
		gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, dst, GC_VX_INDEX_AUTO);

        context.params.scale      = thresh;
        context.params.volume     = (gctUINT32)radius;

        context.params.kernel     = gcvVX_KERNEL_EUCLIDEAN_NONMAXSUPPRESSION;

        context.params.xstep      = 1;

        status = gcfVX_Kernel(&context);
    }

    return status;
}
#endif