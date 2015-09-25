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

vx_status _gcfVX_Filter(gceVX_KERNEL kernel, vx_image src, vx_image dst, vx_border_mode_t borders)
{
    gcoVX_Kernel_Context context = {{0}};
    vx_uint32 height;
    vx_uint32 rect[1];

    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
    rect[0] = height;

    /*index = 0*/
    gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, src, context.objects_num);

    /*index = 1*/
    gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, dst, context.objects_num);

    context.params.kernel = kernel;


    /* RTL/CMODEl requst box3x3 (end - start) < 6, so xstep must < 6 */
    context.params.xstep            = (kernel == gcvVX_KERNEL_BOX_3x3) ? 4: 8;
    context.params.ystep            = height;

    context.params.borders          = borders.mode;

    gcoOS_MemCopy(&context.uniforms[0].uniform, rect, sizeof(rect));
    context.uniforms[0].index       = 2;
    context.uniforms[0].num         = 4;
    context.uniform_num             = 1;

    if(borders.mode == VX_BORDER_MODE_CONSTANT)
    {
        vx_uint32 bin[4];

        bin[0] =
        bin[1] =
        bin[2] =
        bin[3] = FORMAT_VALUE(borders.constant_value);

        gcoOS_MemCopy(&context.uniforms[1].uniform, bin, sizeof(bin));
        context.uniforms[1].num    = 4 * 4;
        context.uniforms[1].index  = 3;
        context.uniform_num        = 2;
    }

    return gcfVX_Kernel(&context);
}

vx_status vxMedian3x3(vx_image src, vx_image dst, vx_border_mode_t *borders)
{
    return _gcfVX_Filter(gcvVX_KERNEL_MEDIAN_3x3, src, dst, *borders);
}


vx_status vxBox3x3(vx_image src, vx_image dst, vx_border_mode_t *bordermode)
{
    return _gcfVX_Filter(gcvVX_KERNEL_BOX_3x3, src, dst, *bordermode);
}


vx_status vxGaussian3x3(vx_image src, vx_image dst, vx_border_mode_t *bordermode)
{
    return _gcfVX_Filter(gcvVX_KERNEL_GAUSSIAN_3x3, src, dst, *bordermode);
}

