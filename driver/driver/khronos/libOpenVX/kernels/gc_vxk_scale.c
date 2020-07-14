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

static vx_status vxVivScale(vx_node node, vx_image src_image, vx_image dst_image, vx_scalar stype,
                               const vx_border_t *borders)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 width = 0, height = 0;
    vx_uint32 dst_width, dst_height;
    vx_int32 uniformNum = 0;
    vx_int32 index = 0;
    vx_df_image format = 0;

    vx_float32 m[16] = {0};
    vx_enum type = 0;
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

    vxQueryImage(src_image, VX_IMAGE_WIDTH, &width, sizeof(width));
    vxQueryImage(src_image, VX_IMAGE_HEIGHT, &height, sizeof(height));
    vxQueryImage(src_image, VX_IMAGE_FORMAT, &format, sizeof(format));

    vxQueryImage(dst_image, VX_IMAGE_WIDTH, &dst_width, sizeof(dst_width));
    vxQueryImage(dst_image, VX_IMAGE_HEIGHT, &dst_height, sizeof(dst_height));

    status |= vxReadScalarValue(stype, &type);

    /*index = 0*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, src_image, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, dst_image, GC_VX_INDEX_AUTO);

    kernelContext->params.kernel           = gcvVX_KERNEL_SCALE_IMAGE;

    kernelContext->params.xstep            = (format == VX_DF_IMAGE_U8) ? 16 : 8;
    kernelContext->params.policy           = (type == VX_INTERPOLATION_NEAREST_NEIGHBOR)?gcvVX_INTERPOLATION_TYPE_NEAREST_NEIGHBOR:(type == VX_INTERPOLATION_BILINEAR?gcvVX_INTERPOLATION_TYPE_BILINEAR:gcvVX_INTERPOLATION_TYPE_NEAREST_NEIGHBOR);
#if gcdVX_OPTIMIZER
    kernelContext->borders                 = borders->mode;
#else
    kernelContext->params.borders          = borders->mode;
#endif
    kernelContext->params.constant_value   = borders->constant_value.U32;

    kernelContext->params.xmax             = dst_width;
    kernelContext->params.ymax             = dst_height;

    if(borders->mode == VX_BORDER_CONSTANT || borders->mode == VX_BORDER_UNDEFINED)
    {
        vx_uint32 bin[4];
        bin[0] =
        bin[1] =
        bin[2] =
        bin[3] = FORMAT_VALUE(borders->mode == VX_BORDER_CONSTANT?borders->constant_value.U32:0xcd);

        gcoOS_MemCopy(&kernelContext->uniforms[index].uniform, bin, sizeof(bin));
        kernelContext->uniforms[index].num     = 4 * 4;
        kernelContext->uniforms[index++].index   = 4;
        uniformNum++;
    }
    m[0] = (vx_float32)width/(vx_float32)dst_width;
    m[1] = (vx_float32)height/(vx_float32)dst_height;
    m[2] = (vx_float32)width/(vx_float32)dst_width;
    m[3] = (vx_float32)height/(vx_float32)dst_height;

    /* upload matrix */
    gcoOS_MemCopy(&kernelContext->uniforms[index].uniform, m, sizeof(m));

    kernelContext->uniforms[index].num         = 4 * 4;
    kernelContext->uniforms[index++].index       = 8;
    uniformNum++;

    if (node->base.context->evisNoInst.isVX2 || node->base.context->evisNoInst.noBilinear)
    {
        vx_uint8 constantData0[16] = {0, 0, 8, 8, 64, 64, 72, 72, 0, 8, 0, 8, 0, 8, 0, 8};
        vx_uint8 constantData1[16] = {0, 0, 16, 0, 0, 0, 0, 0, 16, 0, 16, 0, 0, 0, 0, 0};
        vx_uint8 constantData2[16] = {32, 32, 40, 40, 96, 96, 104, 104, 0, 8, 0, 8, 0, 8, 0, 8};

        gcoOS_MemCopy(&kernelContext->uniforms[index].uniform, constantData0, sizeof(constantData0));
        kernelContext->uniforms[index].num = vxmLENGTH_OF(constantData0);
        kernelContext->uniforms[index++].index = 12;
        uniformNum++;

        gcoOS_MemCopy(&kernelContext->uniforms[index].uniform, constantData1, sizeof(constantData1));
        kernelContext->uniforms[index].num = vxmLENGTH_OF(constantData1);
        kernelContext->uniforms[index++].index = 13;
        uniformNum++;

        gcoOS_MemCopy(&kernelContext->uniforms[index].uniform, constantData2, sizeof(constantData2));
        kernelContext->uniforms[index].num = vxmLENGTH_OF(constantData1);
        kernelContext->uniforms[index++].index = 14;
        uniformNum++;
    }

    kernelContext->uniform_num = uniformNum;

    kernelContext->params.evisNoInst = node->base.context->evisNoInst;

    vxWriteScalarValue(stype, &type);

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

vx_status vxScaleImage(vx_node node, vx_image src_image, vx_image dst_image, vx_scalar stype, vx_border_t *bordermode, vx_float64 *interm, vx_size size)
{
    vx_status status = VX_FAILURE;
    vx_enum type = 0;

    vxReadScalarValue(stype, &type);
    if (interm && size)
    {
        status = vxVivScale(node, src_image, dst_image, stype, bordermode);
    }
    else
    {
        status = VX_ERROR_NO_RESOURCES;
    }

    return status;
}

