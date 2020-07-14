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

vx_status Convolve(vx_node node, vx_image src, vx_image dst, vx_int16* matrix, vx_uint32 scale, vx_bool clamp,
                   vx_size conv_width, vx_size conv_height, vx_border_mode_t *bordermode)
{
    vx_status status  = VX_SUCCESS;
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

    /*index = 0*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, src, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, dst, GC_VX_INDEX_AUTO);

    kernelContext->params.kernel = gcvVX_KERNEL_CUSTOM_CONVOLUTION;

    kernelContext->params.xstep = (((conv_width) < 8) && (conv_height / 2 + 2 + conv_width)  > 8) ? 2 :4;
    if(conv_height == 5 && conv_width == 5) kernelContext->params.xstep = 4;

    kernelContext->params.clamp = clamp;
    kernelContext->params.col = (vx_uint32)conv_width;
    kernelContext->params.row = (vx_uint32)conv_height;
    kernelContext->params.matrix = matrix;
    kernelContext->params.scale = gcoMATH_Ceiling(gcoMATH_Log2((vx_float32)scale));
#if gcdVX_OPTIMIZER
    kernelContext->borders = bordermode->mode;
#else
    kernelContext->params.borders = bordermode->mode;
#endif

    if(bordermode->mode == VX_BORDER_CONSTANT || bordermode->mode == VX_BORDER_UNDEFINED)
    {
        vx_uint32 bin[4];

        bin[0] =
        bin[1] =
        bin[2] =
        bin[3] = FORMAT_VALUE((bordermode->mode == VX_BORDER_UNDEFINED)?0xcd:bordermode->constant_value.U32);

        gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, bin, sizeof(bin));
        kernelContext->uniforms[0].num = 4 * 4;
        kernelContext->uniforms[0].index = 2;
        kernelContext->uniform_num = 1;
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

vx_status vxConvolve(vx_node node, vx_image src, vx_convolution conv, vx_image dst, vx_border_t *bordermode)
{
    vx_status status  = VX_SUCCESS;
    vx_size conv_width, conv_height;
    vx_uint32 scale = 1;
    vx_int16 conv_mat[C_MAX_CONVOLUTION_DIM * C_MAX_CONVOLUTION_DIM] = {0};

    status |= vxQueryConvolution(conv, VX_CONVOLUTION_COLUMNS, &conv_width, sizeof(conv_width));
    status |= vxQueryConvolution(conv, VX_CONVOLUTION_ROWS, &conv_height, sizeof(conv_height));
    status |= vxQueryConvolution(conv, VX_CONVOLUTION_SCALE, &scale, sizeof(scale));

    status |= vxReadConvolutionCoefficients(conv, conv_mat);
    status |= vxWriteConvolutionCoefficients(conv, NULL);

    status = Convolve(node, src, dst, conv_mat, (vx_uint32)scale, vx_true_e, conv_width, conv_height, bordermode);
    return status;
}

