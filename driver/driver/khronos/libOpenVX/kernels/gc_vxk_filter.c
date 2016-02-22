/****************************************************************************
*
*    Copyright (c) 2005 - 2016 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include <gc_vxk_common.h>
#include <stdlib.h>

static vx_int16 gaussian3x3Matrix[3][3] = {
        {1, 2, 1},
        {2, 4, 2},
        {1, 2, 1},
};

static vx_int16 box3x3Matrix[3][3] = {
        {1, 1, 1},
        {1, 1, 1},
        {1, 1, 1},
};

vx_status _gcfVX_Filter_Halfevis(vx_node node, gceVX_KERNEL kernel, vx_image src, vx_image dst, vx_border_mode_t *bordermode)
{
    vx_status status  = VX_SUCCESS;
    gcoVX_Kernel_Context * kernelContext = gcvNULL;
    vx_uint32 height;
    vx_uint8 constantData[16] = {0, 32, 64, 96, 0, 0, 0, 0, 8, 8, 8, 8, 0, 0, 0, 0};

    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));

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
    }

    /*index = 0*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, src, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, dst, GC_VX_INDEX_AUTO);

    kernelContext->params.kernel = kernel;

    kernelContext->params.xstep = 4;
    kernelContext->params.ystep = height;

    kernelContext->params.clamp = vx_false_e;
    kernelContext->params.col = 3;
    kernelContext->params.row = 3;
    kernelContext->params.matrix = (kernel == gcvVX_KERNEL_BOX_3x3) ? (vx_int16 *)box3x3Matrix : (vx_int16 *)gaussian3x3Matrix;
    kernelContext->params.scale = (kernel == gcvVX_KERNEL_BOX_3x3) ? 9.0f : gcoMATH_Log2(16);
    kernelContext->params.volume = height;
#if gcdVX_OPTIMIZER
    kernelContext->borders = bordermode->mode;
#else
    kernelContext->params.borders = bordermode->mode;
#endif

    if(bordermode->mode == VX_BORDER_MODE_CONSTANT || bordermode->mode == VX_BORDER_MODE_UNDEFINED)
    {
        vx_uint32 bin[4];

        bin[0] =
        bin[1] =
        bin[2] =
        bin[3] = FORMAT_VALUE((bordermode->mode == VX_BORDER_MODE_UNDEFINED)?0xcd:bordermode->constant_value);

        gcoOS_MemCopy(&kernelContext->uniforms[1].uniform, bin, sizeof(bin));
        kernelContext->uniforms[1].num    = 4 * 4;
        kernelContext->uniforms[1].index  = 3;
        kernelContext->uniform_num        = 2;
    }

    if (kernel == gcvVX_KERNEL_BOX_3x3)
    {
        gcoOS_MemCopy(&kernelContext->uniforms[2].uniform, constantData, sizeof(constantData));
        kernelContext->uniforms[2].index = 4;
        kernelContext->uniforms[2].num = sizeof(constantData) / sizeof(vx_uint32);
        kernelContext->uniform_num = 3;
    }

    kernelContext->params.evisNoInst = node->base.context->evisNoInst;

    status = gcfVX_Kernel(kernelContext);

#if gcdVX_OPTIMIZER
    if (!node || !node->kernelContext)
    {
        vxFree(kernelContext);
    }
#endif

    return status;
}

/*cpu implement for Median3x3*/
vx_status vxAlterRectangle(vx_rectangle_t *rect,
                           vx_int32 dsx,
                           vx_int32 dsy,
                           vx_int32 dex,
                           vx_int32 dey)
{
    if (rect)
    {
        rect->start_x += dsx;
        rect->start_y += dsy;
        rect->end_x += dex;
        rect->end_y += dey;
        return VX_SUCCESS;
    }
    return VX_ERROR_INVALID_REFERENCE;
}

void vxReadRectangle(const void *base,
                     const vx_imagepatch_addressing_t *addr,
                     const vx_border_mode_t *borders,
                     vx_df_image type,
                     vx_uint32 center_x,
                     vx_uint32 center_y,
                     vx_uint32 radius_x,
                     vx_uint32 radius_y,
                     void *destination)
{
    vx_int32 width = (vx_int32)addr->dim_x, height = (vx_int32)addr->dim_y;
    vx_int32 stride_y = addr->stride_y;
    vx_int32 stride_x = addr->stride_x;
    const vx_uint8 *ptr = (const vx_uint8 *)base;
    vx_int32 ky, kx;
    vx_uint32 dest_index = 0;
    // kx, kx - kernel x and y
    if( borders->mode == VX_BORDER_MODE_REPLICATE || borders->mode == VX_BORDER_MODE_UNDEFINED )
    {
        for (ky = -(int32_t)radius_y; ky <= (int32_t)radius_y; ++ky)
        {
            vx_int32 y = (vx_int32)(center_y + ky);
            y = y < 0 ? 0 : y >= height ? height - 1 : y;

            for (kx = -(int32_t)radius_x; kx <= (int32_t)radius_x; ++kx, ++dest_index)
            {
                vx_int32 x = (int32_t)(center_x + kx);
                x = x < 0 ? 0 : x >= width ? width - 1 : x;

                switch(type)
                {
                case VX_DF_IMAGE_U8:
                    ((vx_uint8*)destination)[dest_index] = *(vx_uint8*)(ptr + y*stride_y + x*stride_x);
                    break;
                case VX_DF_IMAGE_S16:
                case VX_DF_IMAGE_U16:
                    ((vx_uint16*)destination)[dest_index] = *(vx_uint16*)(ptr + y*stride_y + x*stride_x);
                    break;
                case VX_DF_IMAGE_S32:
                case VX_DF_IMAGE_U32:
                    ((vx_uint32*)destination)[dest_index] = *(vx_uint32*)(ptr + y*stride_y + x*stride_x);
                    break;
                default:
                    abort();
                }
            }
        }
    }
    else if( borders->mode == VX_BORDER_MODE_CONSTANT )
    {
        vx_uint32 cval = borders->constant_value;
        for (ky = -(int32_t)radius_y; ky <= (int32_t)radius_y; ++ky)
        {
            vx_int32 y = (vx_int32)(center_y + ky);
            int ccase_y = y < 0 || y >= height;

            for (kx = -(int32_t)radius_x; kx <= (int32_t)radius_x; ++kx, ++dest_index)
            {
                vx_int32 x = (int32_t)(center_x + kx);
                int ccase = ccase_y || x < 0 || x >= width;

                switch(type)
                {
                    case VX_DF_IMAGE_U8:
                        if( !ccase )
                            ((vx_uint8*)destination)[dest_index] = *(vx_uint8*)(ptr + y*stride_y + x*stride_x);
                        else
                            ((vx_uint8*)destination)[dest_index] = (vx_uint8)cval;
                        break;
                    case VX_DF_IMAGE_S16:
                    case VX_DF_IMAGE_U16:
                        if( !ccase )
                            ((vx_uint16*)destination)[dest_index] = *(vx_uint16*)(ptr + y*stride_y + x*stride_x);
                        else
                            ((vx_uint16*)destination)[dest_index] = (vx_uint16)cval;
                        break;
                    case VX_DF_IMAGE_S32:
                    case VX_DF_IMAGE_U32:
                        if( !ccase )
                            ((vx_uint32*)destination)[dest_index] = *(vx_uint32*)(ptr + y*stride_y + x*stride_x);
                        else
                            ((vx_uint32*)destination)[dest_index] = (vx_uint32)cval;
                        break;
                    default:
                        abort();
                }
            }
        }
    }
    else
        abort();
}

static int vx_uint8_compare(const void * a, const void * b)
{
    return *(vx_uint8*)a - *(vx_uint8*)b;
}

vx_status _gcfVX_Median3x3_Cpu(vx_image src, vx_image dst, vx_border_mode_t *borders)
{
    vx_uint32 y, x;
    void *src_base = NULL;
    void *dst_base = NULL;
    vx_imagepatch_addressing_t src_addr, dst_addr;
    vx_rectangle_t rect;
    vx_uint32 low_x = 0, low_y = 0, high_x, high_y;

    vx_status status = vxGetValidRegionImage(src, &rect);
    status |= vxAccessImagePatch(src, &rect, 0, &src_addr, &src_base, VX_READ_ONLY);
    status |= vxAccessImagePatch(dst, &rect, 0, &dst_addr, &dst_base, VX_WRITE_ONLY);

    high_x = src_addr.dim_x;
    high_y = src_addr.dim_y;

    if (borders->mode == VX_BORDER_MODE_UNDEFINED)
    {
        ++low_x; --high_x;
        ++low_y; --high_y;
        vxAlterRectangle(&rect, 1, 1, -1, -1);
    }

    for (y = low_y; (y < high_y) && (status == VX_SUCCESS); y++)
    {
        for (x = low_x; x < high_x; x++)
        {
            vx_uint8 *dst = (vx_uint8_ptr)vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);
            vx_uint8 values[9];

            vxReadRectangle(src_base, &src_addr, borders, VX_DF_IMAGE_U8, x, y, 1, 1, values);

            qsort(values, vxmLENGTH_OF(values), sizeof(vx_uint8), vx_uint8_compare);
            *dst = values[4]; /* pick the middle value */
        }
    }

    status |= vxCommitImagePatch(src, NULL, 0, &src_addr, src_base);
    status |= vxCommitImagePatch(dst, &rect, 0, &dst_addr, dst_base);

    return status;
}

vx_status _gcfVX_Filter(vx_node node, gceVX_KERNEL kernel, vx_image src, vx_image dst, vx_border_mode_t borders)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 height;
    vx_uint32 rect[1];
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
    }

    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
    rect[0] = height;

    /*index = 0*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, src, kernelContext->objects_num);

    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, dst, kernelContext->objects_num);

    kernelContext->params.kernel = kernel;

    /* RTL/CMODEl requst box3x3 (end - start) < 6, so xstep must < 6 */
    kernelContext->params.xstep            = (kernel == gcvVX_KERNEL_BOX_3x3) ? 4: 8;
    kernelContext->params.ystep            = height;

#if gcdVX_OPTIMIZER
    kernelContext->borders                 = borders.mode;
#else
    kernelContext->params.borders          = borders.mode;
#endif

    gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, rect, sizeof(rect));
    kernelContext->uniforms[0].index       = 2;
    kernelContext->uniforms[0].num         = 4;
    kernelContext->uniform_num             = 1;

    if(borders.mode == VX_BORDER_MODE_CONSTANT)
    {
        vx_uint32 bin[4];

        bin[0] =
        bin[1] =
        bin[2] =
        bin[3] = FORMAT_VALUE(borders.constant_value);

        gcoOS_MemCopy(&kernelContext->uniforms[1].uniform, bin, sizeof(bin));
        kernelContext->uniforms[1].num    = 4 * 4;
        kernelContext->uniforms[1].index  = 3;
        kernelContext->uniform_num        = 2;
    }

    kernelContext->params.evisNoInst = node->base.context->evisNoInst;

    status = gcfVX_Kernel(kernelContext);

#if gcdVX_OPTIMIZER
    if (!node || !node->kernelContext)
    {
        vxFree(kernelContext);
    }
#endif

    return status;
}

vx_status vxMedian3x3(vx_node node, vx_image src, vx_image dst, vx_border_mode_t *borders)
{
    if (node->base.context->evisNoInst.noFilter)
    {
        return _gcfVX_Median3x3_Cpu(src, dst, borders);
    }
    else
    {
        return _gcfVX_Filter(node, gcvVX_KERNEL_MEDIAN_3x3, src, dst, *borders);
    }
}

vx_status vxBox3x3(vx_node node, vx_image src, vx_image dst, vx_border_mode_t *bordermode)
{
    if (node->base.context->evisNoInst.noFilter)
    {
        return _gcfVX_Filter_Halfevis(node, gcvVX_KERNEL_BOX_3x3, src, dst, bordermode);
    }
    else
    {
        return _gcfVX_Filter(node, gcvVX_KERNEL_BOX_3x3, src, dst, *bordermode);
    }
}

vx_status vxGaussian3x3(vx_node node, vx_image src, vx_image dst, vx_border_mode_t *bordermode)
{
    if (node->base.context->evisNoInst.noFilter)
    {
        return _gcfVX_Filter_Halfevis(node, gcvVX_KERNEL_GAUSSIAN_3x3, src, dst, bordermode);
    }
    else
    {
        return _gcfVX_Filter(node, gcvVX_KERNEL_GAUSSIAN_3x3, src, dst, *bordermode);
    }
}

vx_status vxExample(vx_node node, vx_image input, vx_image output)
{
    vx_status status = VX_SUCCESS;
    vx_df_image format = 0;
    gcoVX_Kernel_Context * kernelContext = gcvNULL;

#if gcdVX_OPTIMIZER
    if (node && node->kernelContext)
    {
        kernelContext = (gcoVX_Kernel_Context *) node->kernelContext;
    }
    else
#endif
    {
        /* Allocate a local copy for old flow. */
        kernelContext = (gcoVX_Kernel_Context *) vxAllocate(sizeof(gcoVX_Kernel_Context));
    }

    vxQueryImage(output, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));

    /*index = 0*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, kernelContext->objects_num);

    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, kernelContext->objects_num);

    kernelContext->params.kernel           = gcvVX_KERNEL_EXAMPLE;
    kernelContext->params.xstep            = (format == VX_DF_IMAGE_U8) ? 16 : 8;
    kernelContext->params.ystep            = 1;

    kernelContext->params.evisNoInst = node->base.context->evisNoInst;

    status = gcfVX_Kernel(kernelContext);

#if gcdVX_OPTIMIZER
    if (!node || !node->kernelContext)
#else
    if (kernelContext)
#endif
    {
        vxFree(kernelContext);
    }

    return status;
}
