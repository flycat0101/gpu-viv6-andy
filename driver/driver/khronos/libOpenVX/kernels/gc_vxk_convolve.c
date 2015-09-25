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

vx_status Convolve(vx_image src, vx_image dst, vx_int16* matrix,  vx_uint32 scale, vx_bool clamp,
                   vx_size conv_width, vx_size conv_height, vx_border_mode_t *bordermode)
{
    vx_status status  = VX_SUCCESS;
    gcoVX_Kernel_Context context = {{0}};

    /*index = 0*/
    gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, src, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, dst, GC_VX_INDEX_AUTO);

    context.params.kernel = gcvVX_KERNEL_CUSTOM_CONVOLUTION;

    context.params.xstep = (((conv_width) < 8) && (conv_height / 2 + 2 + conv_width)  > 8) ? 2 :4;
    if(conv_height == 5 && conv_width == 5) context.params.xstep = 4;

    context.params.clamp = clamp;
    context.params.col = (vx_uint32)conv_width;
    context.params.row = (vx_uint32)conv_height;
    context.params.matrix = matrix;
    context.params.scale = gcoMATH_Log2((gctFLOAT)scale);
    context.params.borders = bordermode->mode;

    if(bordermode->mode == VX_BORDER_MODE_CONSTANT || bordermode->mode == VX_BORDER_MODE_UNDEFINED)
    {
        vx_uint32 bin[4];

        bin[0] =
        bin[1] =
        bin[2] =
        bin[3] = FORMAT_VALUE((bordermode->mode == VX_BORDER_MODE_UNDEFINED)?0xcd:bordermode->constant_value);

        gcoOS_MemCopy(&context.uniforms[0].uniform, bin, sizeof(bin));
        context.uniforms[0].num = 4 * 4;
        context.uniforms[0].index = 2;
        context.uniform_num = 1;
    }
    status = gcfVX_Kernel(&context);

    return status;
}

vx_status vxConvolve(vx_image src, vx_convolution conv, vx_image dst, vx_border_mode_t *bordermode)
{
    vx_status status  = VX_SUCCESS;
    vx_size conv_width, conv_height;
    vx_uint32 scale = 1;
    vx_int16 conv_mat[C_MAX_CONVOLUTION_DIM * C_MAX_CONVOLUTION_DIM] = {0};

    status |= vxQueryConvolution(conv, VX_CONVOLUTION_ATTRIBUTE_COLUMNS, &conv_width, sizeof(conv_width));
    status |= vxQueryConvolution(conv, VX_CONVOLUTION_ATTRIBUTE_ROWS, &conv_height, sizeof(conv_height));
    status |= vxQueryConvolution(conv, VX_CONVOLUTION_ATTRIBUTE_SCALE, &scale, sizeof(scale));

    status |= vxAccessConvolutionCoefficients(conv, conv_mat);
    status |= vxCommitConvolutionCoefficients(conv, NULL);

    status = Convolve(src, dst, conv_mat, scale, vx_true_e, conv_width, conv_height, bordermode);
    return status;
}

