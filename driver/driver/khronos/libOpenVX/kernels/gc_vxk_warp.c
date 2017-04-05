/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include <gc_vxk_common.h>

static vx_status vxVivWarpGeneric(vx_node node, vx_image src_image, vx_matrix matrix, vx_scalar stype, vx_image dst_image,
                               const vx_border_mode_t *borders, vx_uint32 kernel)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 dst_width, dst_height;
    vx_float32 m[9];
    vx_enum type = 0;
    gcoVX_Kernel_Context * kernelContext = gcvNULL;
    vx_int32 uniformNum = 0;
    vx_int32 index = 0;

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

    vxQueryImage(dst_image, VX_IMAGE_ATTRIBUTE_WIDTH, &dst_width, sizeof(dst_width));
    vxQueryImage(dst_image, VX_IMAGE_ATTRIBUTE_HEIGHT, &dst_height, sizeof(dst_height));

    status |= vxReadMatrix(matrix, m);
    status |= vxReadScalarValue(stype, &type);

    /*index = 0*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, src_image, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, dst_image, GC_VX_INDEX_AUTO);

    kernelContext->params.kernel           = kernel;

    kernelContext->params.xstep            = 16;
    kernelContext->params.policy           = (type == VX_INTERPOLATION_TYPE_NEAREST_NEIGHBOR)?gcvVX_INTERPOLATION_TYPE_NEAREST_NEIGHBOR:(type == VX_INTERPOLATION_TYPE_BILINEAR?gcvVX_INTERPOLATION_TYPE_BILINEAR:gcvVX_INTERPOLATION_TYPE_NEAREST_NEIGHBOR);
#if gcdVX_OPTIMIZER
    kernelContext->borders                 = borders->mode;
#else
    kernelContext->params.borders          = borders->mode;
#endif
    kernelContext->params.constant_value   = borders->constant_value;

    kernelContext->params.xmax             = dst_width;
    kernelContext->params.ymax             = dst_height;
    kernelContext->params.evisNoInst       = node->base.context->evisNoInst;

    if(borders->mode == VX_BORDER_MODE_CONSTANT || borders->mode == VX_BORDER_MODE_UNDEFINED)
    {
        vx_uint32 bin[4];
        bin[0] =
        bin[1] =
        bin[2] =
        bin[3] = FORMAT_VALUE(borders->mode == VX_BORDER_MODE_CONSTANT?borders->constant_value:0xcd);

        gcoOS_MemCopy(&kernelContext->uniforms[index].uniform, bin, sizeof(bin));
        kernelContext->uniforms[index].num     = 4 * 4;
        kernelContext->uniforms[index++].index   = 4;
        uniformNum++;
    }

    /* upload matrix */
    if(kernel == gcvVX_KERNEL_WARP_PERSPECTIVE)
    {
        gcoOS_MemCopy(&kernelContext->uniforms[index].uniform, m, 3*sizeof(vx_float32));
        gcoOS_MemCopy(&kernelContext->uniforms[index].uniform.termConfig + 4, &m[3], 3*sizeof(vx_float32));
        gcoOS_MemCopy(&kernelContext->uniforms[index].uniform.termConfig + 8, &m[6], 3*sizeof(vx_float32));
    }
    else
        gcoOS_MemCopy(&kernelContext->uniforms[index].uniform, m, sizeof(m));

    kernelContext->uniforms[index].num         = 4 * 4;
    kernelContext->uniforms[index++].index       = 8;
    uniformNum++;

    if (node->base.context->evisNoInst.isVX2 || node->base.context->evisNoInst.noBilinear)
    {
        vx_uint32 constantData[8] = {0, 8, 16, 24, 0, 0, 0, 0};
        gcoOS_MemCopy(&kernelContext->uniforms[2].uniform, constantData, sizeof(constantData));
        kernelContext->uniforms[index].num = vxmLENGTH_OF(constantData);
        kernelContext->uniforms[index++].index = 12;
        uniformNum++;
    }

    kernelContext->uniform_num = uniformNum;

    kernelContext->node = node;

    status |= gcfVX_Kernel(kernelContext);

#if gcdVX_OPTIMIZER
    if (!node || !node->kernelContext)
    {
        vxFree(kernelContext);
    }
#endif

    return status;
}

vx_status vxWarpAffine(vx_node node, vx_image src_image, vx_matrix matrix, vx_scalar stype, vx_image dst_image, const vx_border_mode_t *borders)
{
    return vxVivWarpGeneric(node, src_image, matrix, stype, dst_image, borders, gcvVX_KERNEL_WARP_AFFINE);
}

vx_status vxWarpPerspective(vx_node node, vx_image src_image, vx_matrix matrix, vx_scalar stype, vx_image dst_image, const vx_border_mode_t *borders)
{
    return vxVivWarpGeneric(node, src_image, matrix, stype, dst_image, borders, gcvVX_KERNEL_WARP_PERSPECTIVE);
}

