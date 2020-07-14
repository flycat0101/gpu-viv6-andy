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

vx_status _gcfVX_Filter_Halfevis(vx_node node, gceVX_KERNEL kernel, vx_image src, vx_image dst, vx_border_t *bordermode)
{
    vx_status status  = VX_SUCCESS;
    gcoVX_Kernel_Context * kernelContext = gcvNULL;
    vx_uint32 height;
    vx_uint8 constantData[16] = {0, 32, 64, 96, 0, 0, 0, 0, 8, 8, 8, 8, 0, 0, 0, 0};

    vxQueryImage(src, VX_IMAGE_HEIGHT, &height, sizeof(height));

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

    if(bordermode->mode == VX_BORDER_CONSTANT || bordermode->mode == VX_BORDER_UNDEFINED)
    {
        vx_uint32 bin[4];

        bin[0] =
        bin[1] =
        bin[2] =
        bin[3] = FORMAT_VALUE((bordermode->mode == VX_BORDER_UNDEFINED)?0xcd:bordermode->constant_value.U32);

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
                     const vx_border_t *borders,
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
    if(borders->mode == VX_BORDER_REPLICATE || borders->mode == VX_BORDER_UNDEFINED )
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
    else if(borders->mode == VX_BORDER_CONSTANT )
    {
        vx_uint32 cval = borders->constant_value.U32;
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
                        if(!ccase )
                            ((vx_uint8*)destination)[dest_index] = *(vx_uint8*)(ptr + y*stride_y + x*stride_x);
                        else
                            ((vx_uint8*)destination)[dest_index] = (vx_uint8)cval;
                        break;
                    case VX_DF_IMAGE_S16:
                    case VX_DF_IMAGE_U16:
                        if(!ccase )
                            ((vx_uint16*)destination)[dest_index] = *(vx_uint16*)(ptr + y*stride_y + x*stride_x);
                        else
                            ((vx_uint16*)destination)[dest_index] = (vx_uint16)cval;
                        break;
                    case VX_DF_IMAGE_S32:
                    case VX_DF_IMAGE_U32:
                        if(!ccase )
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

vx_status _gcfVX_Median3x3_Cpu(vx_image src, vx_image dst, vx_border_t *borders)
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

    if (borders->mode == VX_BORDER_UNDEFINED)
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

vx_status _gcfVX_Filter(vx_node node, gceVX_KERNEL kernel, vx_image src, vx_image dst, vx_border_t borders)
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
        kernelContext->uniform_num = 0;
    }

    vxQueryImage(src, VX_IMAGE_HEIGHT, &height, sizeof(height));
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

    if(borders.mode == VX_BORDER_CONSTANT || borders.mode == VX_BORDER_UNDEFINED)
    {
        vx_uint32 bin[4];

        bin[0] =
        bin[1] =
        bin[2] =
        bin[3] = FORMAT_VALUE((borders.mode == VX_BORDER_UNDEFINED) ? 0xcd : borders.constant_value.U32);

        gcoOS_MemCopy(&kernelContext->uniforms[1].uniform, bin, sizeof(bin));
        kernelContext->uniforms[1].num    = 4 * 4;
        kernelContext->uniforms[1].index  = 3;
        kernelContext->uniform_num        = 2;
    }

    kernelContext->params.evisNoInst = node->base.context->evisNoInst;

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

vx_status vxMedian3x3(vx_node node, vx_image src, vx_image dst, vx_border_t *borders)
{
    if (!node->base.context->evisNoInst.isVX2 && node->base.context->evisNoInst.noFilter)
    {
        return _gcfVX_Median3x3_Cpu(src, dst, borders);
    }
    else
    {
        return _gcfVX_Filter(node, gcvVX_KERNEL_MEDIAN_3x3, src, dst, *borders);
    }
}

vx_status vxBox3x3(vx_node node, vx_image src, vx_image dst, vx_border_t *bordermode)
{
    if (node->base.context->evisNoInst.isVX2 || node->base.context->evisNoInst.noBoxFilter)
    {
        return _gcfVX_Filter_Halfevis(node, gcvVX_KERNEL_BOX_3x3, src, dst, bordermode);
    }
    else
    {
        return _gcfVX_Filter(node, gcvVX_KERNEL_BOX_3x3, src, dst, *bordermode);
    }
}

vx_status vxGaussian3x3(vx_node node, vx_image src, vx_image dst, vx_border_t *bordermode)
{
    if (node->base.context->evisNoInst.isVX2 || node->base.context->evisNoInst.noFilter)
    {
        return _gcfVX_Filter_Halfevis(node, gcvVX_KERNEL_GAUSSIAN_3x3, src, dst, bordermode);
    }
    else
    {
        return _gcfVX_Filter(node, gcvVX_KERNEL_GAUSSIAN_3x3, src, dst, *bordermode);
    }
}

#define SGM_COST    0
#define SGM_PATH90  1
#define SGM_PATH45  2
#define SGM_PATH135 3
#define SGM_PATH0   4

#ifndef SHRT_MAX
#define SHRT_MAX 32767
#endif

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

vx_status vxSGMCost(vx_node node, vx_image right, vx_image left, vx_image cost, vx_uint32 width, vx_uint32 height, vx_uint32 range)
{
    vx_status status = VX_SUCCESS;
    vx_int32 i, j, k;
    vx_uint32 window_size = 7;
    vx_uint16 v_cost;
    void *right_base = NULL;
    void *left_base = NULL;
    void *cost_base = NULL;
    vx_imagepatch_addressing_t right_addr, left_addr, cost_addr;
    vx_rectangle_t rect;
    vx_uint8 *v_left;
    vx_uint8 *v_right;
    vx_uint16 *u_cost;

    vx_int32 wk, wi, vk, vi, vj;
    vx_int32 wsize = window_size/2;
    vx_uint16 scost[48];

    status |= vxGetValidRegionImage(right, &rect);
    status |= vxAccessImagePatch(right, &rect, 0, &right_addr, &right_base, VX_READ_ONLY);
    status |= vxAccessImagePatch(left, &rect, 0, &left_addr, &left_base, VX_READ_ONLY);
    status |= vxGetValidRegionImage(cost, &rect);
    status |= vxAccessImagePatch(cost, &rect, 0, &cost_addr, &cost_base, VX_WRITE_ONLY);
    /* k, i central position */
    for (k = 0; k < (vx_int32)height; k++)
    {
        for (i = 0; i < (vx_int32)width ; i++)
        {
            for (j = 0; j < (vx_int32)range; j++)
            {
                scost[j] = 0;
            }

            /* wk, wi position inside windows*/
            for (wk = k-wsize; wk<=k+wsize; wk++)
            {

                for (wi=i-wsize; wi<=i+wsize; wi++)
                {
                    vk = max(min(wk, (vx_int32)height - 1), 0);
                    vi = max(min(wi, (vx_int32)width - 1), 0);

                    v_left = (vx_uint8 *)vxFormatImagePatchAddress2d(left_base, vi, vk, &left_addr);

                    for (j = 0; j < (vx_int32)range; j++)
                    {
                        vj = max(min(wi - j, (vx_int32)width - 1), 0);
                        v_right = (vx_uint8 *)vxFormatImagePatchAddress2d(right_base, vj, vk, &right_addr);

                        if (*v_left > *v_right)
                            v_cost = *v_left - *v_right;
                        else
                            v_cost = *v_right - *v_left;
                        scost[j] += v_cost;
                    }

                }
            }

            for (j = 0; j < (vx_int32)range; j++)
            {
                u_cost = (vx_uint16 *)vxFormatImagePatchAddress2d(cost_base, i * range + j, k, &cost_addr);
                *u_cost = (vx_uint16)(scost[j]/window_size/window_size);
            }
        }
    }
    status |= vxCommitImagePatch(right, NULL, 0, &right_addr, right_base);
    status |= vxCommitImagePatch(left, NULL, 0, &right_addr, left_base);
    status |= vxCommitImagePatch(cost, &rect, 0, &cost_addr, cost_base);

    return status;
}

vx_status vxPathCost_90(vx_node node, vx_image cost, vx_image output, vx_uint32 disp_range, vx_uint32 width, vx_uint32 height)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 x, y, d;
    vx_uint16 pre_lr[48];
    vx_uint16 cur_lr[48];
    const vx_uint16 p1=2;
    const vx_uint16 p2=5;
    void *dst_base = NULL;
    void *cost_base = NULL;
    vx_rectangle_t rect;
    vx_imagepatch_addressing_t dst_addr, cost_addr;
    vx_uint16 *v_cost;
    vx_uint16 *dst;

    status |= vxGetValidRegionImage(cost, &rect);
    status |= vxAccessImagePatch(cost, &rect, 0, &cost_addr, &cost_base, VX_READ_ONLY);
    status |= vxGetValidRegionImage(output, &rect);
    status |= vxAccessImagePatch(output, &rect, 0, &dst_addr, &dst_base, VX_WRITE_ONLY);
    /* 90 degree*/
    for (d = 0; d < disp_range; d++)
    {
        pre_lr[d] =0;
    }

    for (x = 0; x < width; x++)
    {
        for (y = 0; y < height; y++)
        {
            vx_uint16 min_lr= SHRT_MAX;
            for (d = 0; d < disp_range; d++)
            {
                /* min */
                if (pre_lr[d]<min_lr)
                    min_lr=pre_lr[d];
            }

            for (d = 0; d < disp_range; d++)
            {
                vx_uint16 l_dminus, l_dplus, l_dcur, l_dmin;

                if (d==0)
                {
                    l_dminus=p1;
                }
                else
                {
                    l_dminus = pre_lr[d-1]+p1;
                }

                if (d == disp_range-1)
                {
                    l_dplus=p1;
                }
                else
                {
                    l_dplus = pre_lr[d+1]+p1;
                }

                l_dcur = pre_lr[d];

                l_dmin = (l_dminus < l_dplus)? l_dminus: l_dplus;
                l_dmin = (l_dmin < l_dcur)? l_dmin:l_dcur;
                l_dmin = (l_dmin < (min_lr +p2)) ? l_dmin:(min_lr+p2);

                v_cost = (vx_uint16 *)vxFormatImagePatchAddress2d(cost_base, x * disp_range + d, y, &cost_addr);
                dst = (vx_uint16 *)vxFormatImagePatchAddress2d(dst_base, x * disp_range + d, y, &dst_addr);
                cur_lr[d] = *v_cost + l_dmin - min_lr;
                *dst = cur_lr[d];
            }

            for (d = 0; d < disp_range; d++)
            {
                /* rotate*/
                pre_lr[d] = cur_lr[d];
            }

        }
    }

    status |= vxCommitImagePatch(cost, NULL, 0, &cost_addr, cost_base);
    status |= vxCommitImagePatch(output, &rect, 0, &dst_addr, dst_base);
    return status;
}

vx_status vxPathCost_45(vx_node node, vx_image cost, vx_image lr, vx_uint32 disp_range, vx_uint32 width, vx_uint32 height)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 x, y, d;
    vx_uint16 pre_lr[48];
    vx_uint16 cur_lr[48];
    const vx_uint16 p1=2;
    const vx_uint16 p2=5;
    void *dst_base = NULL;
    void *cost_base = NULL;
    vx_rectangle_t rect;
    vx_imagepatch_addressing_t dst_addr, cost_addr;
    vx_uint16 *v_cost;
    vx_uint16 *dst;

    status |= vxGetValidRegionImage(cost, &rect);
    status |= vxAccessImagePatch(cost, &rect, 0, &cost_addr, &cost_base, VX_READ_ONLY);
    status |= vxGetValidRegionImage(lr, &rect);
    status |= vxAccessImagePatch(lr, &rect, 0, &dst_addr, &dst_base, VX_WRITE_ONLY);

    for (d=0; d<disp_range; d++)
    {
        pre_lr[d] =0;
    }

    for (x = 0; x < width; x++)
    {
        for (y = 0; y < height; y++)
        {
            vx_uint16 min_lr= SHRT_MAX;
            vx_uint32 tx;
            tx = (y + x) % width;

            for (d=0; d<disp_range; d++)
            {
                /* min */
                if (pre_lr[d]<min_lr)
                    min_lr=pre_lr[d];
            }

            for (d=0; d<disp_range; d++)
            {
                vx_uint16 l_dminus, l_dplus, l_dcur, l_dmin;

                if (d==0)
                {
                    l_dminus=p1;
                }
                else
                {
                    l_dminus = pre_lr[d-1]+p1;
                }

                if (d==disp_range-1)
                {
                    l_dplus=p1;
                }
                else
                {
                    l_dplus = pre_lr[d+1]+p1;
                }

                l_dcur = pre_lr[d];

                l_dmin = (l_dminus < l_dplus)? l_dminus: l_dplus;
                l_dmin = (l_dmin < l_dcur)? l_dmin:l_dcur;
                l_dmin = (l_dmin < (min_lr +p2)) ? l_dmin:(min_lr+p2);

                v_cost = (vx_uint16 *)vxFormatImagePatchAddress2d(cost_base, tx * disp_range + d, y, &cost_addr);
                dst = (vx_uint16 *)vxFormatImagePatchAddress2d(dst_base, tx * disp_range + d, y, &dst_addr);
                cur_lr[d] = *v_cost + l_dmin - min_lr;
                *dst += cur_lr[d];
            }

            for (d=0; d<disp_range; d++)
            {
                pre_lr[d] = (tx==(width - 1))? 0 : cur_lr[d];
            }

        }
    }

    status |= vxCommitImagePatch(cost, NULL, 0, &cost_addr, cost_base);
    status |= vxCommitImagePatch(lr, &rect, 0, &dst_addr, dst_base);
    return status;
}

vx_status vxPathCost_135(vx_node node, vx_image cost, vx_image lr, vx_uint32 disp_range, vx_uint32 width, vx_uint32 height)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 x, y, d;
    vx_uint16 pre_lr[48];
    vx_uint16 cur_lr[48];
    const vx_uint16 p1=2;
    const vx_uint16 p2=5;
    void *dst_base = NULL;
    void *cost_base = NULL;
    vx_rectangle_t rect;
    vx_imagepatch_addressing_t dst_addr, cost_addr;
    vx_uint16 *v_cost;
    vx_uint16 *dst;

    status |= vxGetValidRegionImage(cost, &rect);
    status |= vxAccessImagePatch(cost, &rect, 0, &cost_addr, &cost_base, VX_READ_ONLY);
    status |= vxGetValidRegionImage(lr, &rect);
    status |= vxAccessImagePatch(lr, &rect, 0, &dst_addr, &dst_base, VX_WRITE_ONLY);

    for (d=0; d<disp_range; d++)
    {
        pre_lr[d] =0;
    }

    for (x = 0; x < width; x++)
    {
        for (y = 0; y < height; y++)
        {
            vx_uint16 min_lr= SHRT_MAX;
            vx_int32 tx;
            tx = x-y;
            if (tx <0)
            {
                tx = width + tx;
            }
            for (d=0; d<disp_range; d++)
            {
                /* min */
                if (pre_lr[d]<min_lr)
                    min_lr=pre_lr[d];
            }

            for (d=0; d<disp_range; d++)
            {
                vx_uint16 l_dminus, l_dplus, l_dcur, l_dmin;

                if (d==0)
                {
                    l_dminus=p1;
                }
                else
                {
                    l_dminus = pre_lr[d-1]+p1;
                }

                if (d==disp_range-1)
                {
                    l_dplus=p1;
                }
                else
                {
                    l_dplus = pre_lr[d+1]+p1;
                }

                l_dcur = pre_lr[d];

                l_dmin = (l_dminus < l_dplus)? l_dminus: l_dplus;
                l_dmin = (l_dmin < l_dcur)? l_dmin:l_dcur;
                l_dmin = (l_dmin < (min_lr +p2)) ? l_dmin:(min_lr+p2);

                v_cost = (vx_uint16 *)vxFormatImagePatchAddress2d(cost_base, tx * disp_range + d, y, &cost_addr);
                dst = (vx_uint16 *)vxFormatImagePatchAddress2d(dst_base, tx * disp_range + d, y, &dst_addr);
                cur_lr[d] = *v_cost + l_dmin - min_lr;
                *dst += cur_lr[d];
            }

            for (d=0; d<disp_range; d++)
            {
                pre_lr[d] = (tx == (vx_int32)(width - 1)) ? 0 : cur_lr[d];
            }
        }
    }

    status |= vxCommitImagePatch(cost, NULL, 0, &cost_addr, cost_base);
    status |= vxCommitImagePatch(lr, &rect, 0, &dst_addr, dst_base);
    return status;
}

vx_status vxPathCost_0(vx_node node, vx_image cost, vx_image lr, vx_uint32 disp_range, vx_uint32 width, vx_uint32 height)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 x, y, d;
    vx_uint16 pre_lr[48];
    vx_uint16 cur_lr[48];
    const vx_uint16 p1=2;
    const vx_uint16 p2=5;
    void *dst_base = NULL;
    void *cost_base = NULL;
    vx_rectangle_t rect;
    vx_imagepatch_addressing_t dst_addr, cost_addr;
    vx_uint16 *v_cost;
    vx_uint16 *dst;

    status |= vxGetValidRegionImage(cost, &rect);
    status |= vxAccessImagePatch(cost, &rect, 0, &cost_addr, &cost_base, VX_READ_ONLY);
    status |= vxGetValidRegionImage(lr, &rect);
    status |= vxAccessImagePatch(lr, &rect, 0, &dst_addr, &dst_base, VX_WRITE_ONLY);

    for (d=0; d<disp_range; d++)
    {
        pre_lr[d] =0;
    }

     for (x = 0; x < width; x++)
    {
        for (y = 0; y < height; y++)
        {
            vx_uint16 min_lr= SHRT_MAX;
            for (d=0; d<disp_range; d++)
            {
                /* min */
                if (pre_lr[d]<min_lr)
                    min_lr=pre_lr[d];
            }

            for (d=0; d<disp_range; d++)
            {
                vx_uint16 l_dminus, l_dplus, l_dcur, l_dmin;

                if (d==0)
                {
                    l_dminus=p1;
                }
                else
                {
                    l_dminus = pre_lr[d-1]+p1;
                }

                if (d==disp_range-1)
                {
                    l_dplus=p1;
                }
                else
                {
                    l_dplus = pre_lr[d+1]+p1;
                }

                l_dcur = pre_lr[d];

                l_dmin = (l_dminus < l_dplus)? l_dminus: l_dplus;
                l_dmin = (l_dmin < l_dcur)? l_dmin:l_dcur;
                l_dmin = (l_dmin < (min_lr +p2)) ? l_dmin:(min_lr+p2);

                v_cost = (vx_uint16 *)vxFormatImagePatchAddress2d(cost_base, x * disp_range + d, y, &cost_addr);
                dst = (vx_uint16 *)vxFormatImagePatchAddress2d(dst_base, x * disp_range + d, y, &dst_addr);
                cur_lr[d] = *v_cost + l_dmin - min_lr;
                *dst += cur_lr[d];
            }

            for (d=0; d<disp_range; d++)
            {
                pre_lr[d] =  cur_lr[d];
            }
        }
    }
    status |= vxCommitImagePatch(cost, NULL, 0, &cost_addr, cost_base);
    status |= vxCommitImagePatch(lr, &rect, 0, &dst_addr, dst_base);
    return status;
}

vx_status vxSelectDisp(vx_node node, vx_image path, vx_image depth, vx_uint32 disp_range, vx_uint32 width, vx_uint32 height)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 x, y, d;
    vx_uint8 max_disp = 0;

    void *dst_base = NULL;
    void *path_base = NULL;
    vx_rectangle_t rect;
    vx_imagepatch_addressing_t dst_addr, path_addr;
    vx_uint16 *v_path;
    vx_uint8 *dst;

    status |= vxGetValidRegionImage(path, &rect);
    status |= vxAccessImagePatch(path, &rect, 0, &path_addr, &path_base, VX_READ_ONLY);
    status |= vxGetValidRegionImage(depth, &rect);
    status |= vxAccessImagePatch(depth, &rect, 0, &dst_addr, &dst_base, VX_WRITE_ONLY);

    for (x = 0; x < width; x++)
    {
        for (y = 0; y < height; y++)
        {
            vx_uint16 min_lr = SHRT_MAX;
            vx_uint32 min_id = 0;

            for (d=0; d<disp_range; d++)
            {
                v_path = (vx_uint16 *)vxFormatImagePatchAddress2d(path_base, x * disp_range + d, y, &path_addr);
                if (*v_path < min_lr)
                {
                    min_id = d;
                    min_lr = *v_path;
                }
            }

            dst = (vx_uint8 *)vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);
            *dst = (vx_uint8)min_id;
            if ((vx_uint8)min_id>max_disp)
            {
                max_disp = (vx_uint8)min_id;
            }
        }
    }

    for (x = 0; x < width; x++)
    {
        for (y = 0; y < height; y++)
        {
            dst = (vx_uint8 *)vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);
            *dst = (vx_uint8)(*dst * 255.0 / (vx_float32) max_disp);
        }
    }

    status |= vxCommitImagePatch(path, NULL, 0, &path_addr, path_base);
    status |= vxCommitImagePatch(depth, &rect, 0, &dst_addr, dst_base);
    return status;
}

vx_status vxCensus3x3(vx_node node, vx_image src, vx_image dst)
{
    vx_status status = VX_SUCCESS;
    gcoVX_Kernel_Context * kernelContext = gcvNULL;
    vx_uint32 height;
    vx_uint32 bin[4];
    vx_uint32 constantData[8] = {0, 8, 16, 24, 0, 0, 0, 0};
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
        kernelContext->objects_num = 0;
        kernelContext->uniform_num = 0;
    }

    /*index = 0*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, src, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, dst, GC_VX_INDEX_AUTO);

    kernelContext->params.kernel = gcvVX_KERNEL_CENSUS_3x3;

    kernelContext->params.ystep = height;
    kernelContext->params.xstep = 4;
    kernelContext->params.clamp = vx_false_e;
    kernelContext->params.col = 3;
    kernelContext->params.row = 3;
    kernelContext->params.volume = height;
    kernelContext->params.evisNoInst = node->base.context->evisNoInst;

    bin[0] = height;
    bin[1] = 1;
    bin[2] = FV(1);
    bin[3] = FV2(255);

    gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, bin, sizeof(bin));
    kernelContext->uniforms[0].num = 4 * 4;
    kernelContext->uniforms[0].index = 2;

    gcoOS_MemCopy(&kernelContext->uniforms[1].uniform, constantData, sizeof(constantData));
    kernelContext->uniforms[1].index       = 3;
    kernelContext->uniforms[1].num         = sizeof(constantData) / sizeof(vx_uint32);
    kernelContext->uniform_num             = 2;


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

static vx_uint32 _gcfVX_ReadMaskedRectangleU8(const void *base,
    const vx_imagepatch_addressing_t *addr,
    const vx_border_t *borders,
    vx_df_image type,
    vx_uint32 center_x,
    vx_uint32 center_y,
    vx_uint32 left,
    vx_uint32 top,
    vx_uint32 right,
    vx_uint32 bottom,
    vx_uint8 *mask,
    vx_uint8 *destination)
{
    vx_int32 width = (vx_int32)addr->dim_x, height = (vx_int32)addr->dim_y;
    vx_int32 stride_y = addr->stride_y;
    vx_int32 stride_x = addr->stride_x;
    const vx_uint8 *ptr = (const vx_uint8 *)base;
    vx_int32 ky, kx;
    vx_uint32 mask_index = 0;
    vx_uint32 dest_index = 0;
    /* kx, kx - kernel x and y */
    if (borders->mode == VX_BORDER_REPLICATE || borders->mode == VX_BORDER_UNDEFINED)
    {
        for (ky = -(int32_t)top; ky <= (int32_t)bottom; ++ky)
        {
            vx_int32 y = (vx_int32)(center_y + ky);
            y = y < 0 ? 0 : y >= height ? height - 1 : y;

            for (kx = -(int32_t)left; kx <= (int32_t)right; ++kx, ++mask_index)
            {
                vx_int32 x = (int32_t)(center_x + kx);
                x = x < 0 ? 0 : x >= width ? width - 1 : x;
                if (mask[mask_index])
                    ((vx_uint8*)destination)[dest_index++] = *(vx_uint8*)(ptr + y*stride_y + x*stride_x);
            }
        }
    }
    else if (borders->mode == VX_BORDER_CONSTANT)
    {
        vx_pixel_value_t cval = borders->constant_value;
        for (ky = -(int32_t)top; ky <= (int32_t)bottom; ++ky)
        {
            vx_int32 y = (vx_int32)(center_y + ky);
            int ccase_y = y < 0 || y >= height;

            for (kx = -(int32_t)left; kx <= (int32_t)right; ++kx, ++mask_index)
            {
                vx_int32 x = (int32_t)(center_x + kx);
                int ccase = ccase_y || x < 0 || x >= width;
                if (mask[mask_index])
                    ((vx_uint8*)destination)[dest_index++] = ccase ? (vx_uint8)cval.U8 : *(vx_uint8*)(ptr + y*stride_y + x*stride_x);
            }
        }
    }

    return dest_index;
}

enum
{
    gceVX_MEDIAN_CROSS_NOCARE = 0,
    gceVX_MEDIAN_CROSS_MUL,
    gceVX_MEDIAN_CROSS_ADD,
}
gceVX_MedianMode;

static vx_uint8 _gcfVX_PatternValue(vx_enum f, vx_uint8 v, vx_enum mode)
{
    vx_uint8 result = 0;

    switch(f)
    {
    case VX_NONLINEAR_FILTER_MAX:
        result = (v != 0)?1:0;
        break;
    case VX_NONLINEAR_FILTER_MIN:
        result = (v != 0)?0:0xff;
        break;
    case VX_NONLINEAR_FILTER_MEDIAN:
        if (mode == gceVX_MEDIAN_CROSS_MUL)
            result = (v != 0)?1:0;
        else
            result = (v != 0)?0:0xff;
        break;
    }

    return result;
}

#define ENABLE_SOFT_IMPLEMENT_FOR_MEDIAN 1

/* nodeless version of NonLinearFilter kernel*/
vx_status vxNonLinearFilter(vx_node node, vx_scalar function, vx_image src, vx_matrix mask, vx_image dst, vx_border_t *border)
{
    vx_status status = VX_SUCCESS;
    vx_size mrows, mcols;
    vx_enum func = 0;
    vx_enum pattern;

    status |= vxQueryMatrix(mask, VX_MATRIX_PATTERN, &pattern, sizeof(pattern));

    status |= vxQueryMatrix(mask, VX_MATRIX_ROWS, &mrows, sizeof(mrows));
    status |= vxQueryMatrix(mask, VX_MATRIX_COLUMNS, &mcols, sizeof(mcols));

    status |= vxCopyScalar(function, &func, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);

#if ENABLE_SOFT_IMPLEMENT_FOR_MEDIAN

    if ((mrows == 5 && mcols == 5 && func == VX_NONLINEAR_FILTER_MEDIAN && pattern != VX_PATTERN_CROSS))
    {
        vx_uint32 y, x;
        void *src_base = NULL;
        void *dst_base = NULL;
        vx_imagepatch_addressing_t src_addr, dst_addr;
        vx_rectangle_t rect;
        vx_size low_x = 0, low_y = 0, high_x, high_y;
        vx_enum mtype = 0;
        vx_coordinates2d_t origin;

        vx_uint8 m[VX_INT_MAX_NONLINEAR_DIM * VX_INT_MAX_NONLINEAR_DIM];
        vx_uint8 v[VX_INT_MAX_NONLINEAR_DIM * VX_INT_MAX_NONLINEAR_DIM];

        vx_status status = vxGetValidRegionImage(src, &rect);
        status |= vxAccessImagePatch(src, &rect, 0, &src_addr, &src_base, VX_READ_ONLY);
        status |= vxAccessImagePatch(dst, &rect, 0, &dst_addr, &dst_base, VX_WRITE_ONLY);
        status |= vxQueryMatrix(mask, VX_MATRIX_TYPE, &mtype, sizeof(mtype));
        status |= vxQueryMatrix(mask, VX_MATRIX_ORIGIN, &origin, sizeof(origin));

        if ((mtype != VX_TYPE_UINT8) || (sizeof(m) < mrows * mcols))
            status = VX_ERROR_INVALID_PARAMETERS;

        status |= vxReadMatrix(mask, m);

        if (status == VX_SUCCESS)
        {
            vx_size rx0 = origin.x;
            vx_size ry0 = origin.y;
            vx_size rx1 = mcols - origin.x - 1;
            vx_size ry1 = mrows - origin.y - 1;

            high_x = src_addr.dim_x;
            high_y = src_addr.dim_y;

            if (border->mode == VX_BORDER_UNDEFINED)
            {
                low_x += rx0;
                low_y += ry0;
                high_x -= rx1;
                high_y -= ry1;
                vxAlterRectangle(&rect, (vx_int32)rx0, (vx_int32)ry0, -(vx_int32)rx1, -(vx_int32)ry1);
            }

            for (y = (vx_uint32)low_y; y < (vx_uint32)high_y; y++)
            {
                for (x = (vx_uint32)low_x; x < (vx_uint32)high_x; x++)
                {
                    vx_uint8 *dst = (vx_uint8 *)vxFormatImagePatchAddress2d(dst_base, x, y, &dst_addr);
                    vx_int32 count = _gcfVX_ReadMaskedRectangleU8(src_base, &src_addr, border, VX_DF_IMAGE_U8, x, y, (vx_uint32)rx0, (vx_uint32)ry0, (vx_uint32)rx1, (vx_uint32)ry1, m, v);

                    qsort(v, count, sizeof(vx_uint8), vx_uint8_compare);
                    /*func == VX_NONLINEAR_FILTER_MEDIAN*/
                    *dst = v[count / 2];
                }
            }
        }

        status |= vxCommitImagePatch(src, NULL, 0, &src_addr, src_base);
        status |= vxCommitImagePatch(dst, &rect, 0, &dst_addr, dst_base);

        return status;
    }
    else
#endif
    {
        vx_uint32 height;
        gcoVX_Kernel_Context * kernelContext = gcvNULL;
        vx_uint8 m[C_MAX_NONLINEAR_DIM * C_MAX_NONLINEAR_DIM];
        vx_coordinates2d_t origin;

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

        vxQueryImage(src, VX_IMAGE_HEIGHT, &height, sizeof(height));

        /*index = 0*/
        gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, src, GC_VX_INDEX_AUTO);

        /*index = 1*/
        gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, dst, GC_VX_INDEX_AUTO);

        switch (func)
        {
        case VX_NONLINEAR_FILTER_MIN: kernelContext->params.kernel = gcvVX_KERNEL_NONLINEAR_FILTER_MIN; break;
        case VX_NONLINEAR_FILTER_MAX: kernelContext->params.kernel = gcvVX_KERNEL_NONLINEAR_FILTER_MAX; break;
        case VX_NONLINEAR_FILTER_MEDIAN: kernelContext->params.kernel = gcvVX_KERNEL_NONLINEAR_FILTER_MEDIAN; break;
        }

        kernelContext->params.col          = (vx_uint32)mcols;
        kernelContext->params.row          = (vx_uint32)mrows;

        switch (pattern)
        {
        case VX_PATTERN_BOX:
            kernelContext->params.volume   = gcvVX_PARTTERN_MODE_BOX;

            kernelContext->params.xstep    = (mrows == 3 && mcols == 3)?8:6;
            kernelContext->params.ystep    = (mrows == 3 && mcols == 3)?height:1;
            break;
        case VX_PATTERN_CROSS:
        case VX_PATTERN_DISK:
        case VX_PATTERN_OTHER:
            {
                status |= vxCopyMatrix(mask, m, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);

                if (mrows == 3 && mcols == 3)
                {
                    vx_uint32 bin[4];
                    if (func != VX_NONLINEAR_FILTER_MEDIAN)
                    {
                        /*          x        y       z        w
                         * x max:
                         * c4 : 00010000 01010100 00010000 00000000
                         *
                         * + min:
                         * c4 : ff00ff00 00ff0000 ff00ff00 00000000
                         */
                        bin[0] = _gcfVX_PatternValue(func, m[0], gceVX_MEDIAN_CROSS_NOCARE) | (_gcfVX_PatternValue(func, m[1], gceVX_MEDIAN_CROSS_NOCARE) << 8)  | (_gcfVX_PatternValue(func, m[2], gceVX_MEDIAN_CROSS_NOCARE) << 16);
                        bin[1] = _gcfVX_PatternValue(func, m[3], gceVX_MEDIAN_CROSS_NOCARE) | (_gcfVX_PatternValue(func, m[4], gceVX_MEDIAN_CROSS_NOCARE) << 8)  | (_gcfVX_PatternValue(func, m[5], gceVX_MEDIAN_CROSS_NOCARE) << 16);
                        bin[2] = _gcfVX_PatternValue(func, m[6], gceVX_MEDIAN_CROSS_NOCARE) | (_gcfVX_PatternValue(func, m[7], gceVX_MEDIAN_CROSS_NOCARE) << 8)  | (_gcfVX_PatternValue(func, m[8], gceVX_MEDIAN_CROSS_NOCARE) << 16);
                        bin[3] = 0;
                    }
                    else
                    {
                        /*          x        y       z        w
                         * x src0, src1(xy), + src2(z)
                         * c4 : 00010000 01010100 ff00ff00 00000000
                         *
                         */
                        bin[0] = _gcfVX_PatternValue(func, m[0], gceVX_MEDIAN_CROSS_MUL) | (_gcfVX_PatternValue(func, m[1], gceVX_MEDIAN_CROSS_MUL) << 8)  | (_gcfVX_PatternValue(func, m[2], gceVX_MEDIAN_CROSS_MUL) << 16);
                        bin[1] = _gcfVX_PatternValue(func, m[3], gceVX_MEDIAN_CROSS_MUL) | (_gcfVX_PatternValue(func, m[4], gceVX_MEDIAN_CROSS_MUL) << 8)  | (_gcfVX_PatternValue(func, m[5], gceVX_MEDIAN_CROSS_MUL) << 16);
                        bin[2] = _gcfVX_PatternValue(func, m[6], gceVX_MEDIAN_CROSS_ADD) | (_gcfVX_PatternValue(func, m[7], gceVX_MEDIAN_CROSS_ADD) << 8)  | (_gcfVX_PatternValue(func, m[8], gceVX_MEDIAN_CROSS_ADD) << 16);
                        bin[3] = 0;
                    }

                    gcoOS_MemCopy(&kernelContext->uniforms[2].uniform, bin, sizeof(bin));
                    kernelContext->uniforms[2].index       = 4;
                    kernelContext->uniforms[2].num         = 4 * 4;
                    kernelContext->uniform_num ++;


                }
                else if (mrows == 5 && mcols == 5)
                {
                    /* gcvVX_PARTTERN_MODE_CROSS
                     *          x        y       z        w
                     * x max:
                     * c4 : 00000100 00000000 00000100 00000000
                     * c5 : 01010101 01000000 00000100 00000000
                     * c6 : 0000ff00 00000000 00000000 00000000
                     *
                     * + min:
                     * c4 : ffff00ff ff000000 ffff00ff ff000000
                     * c5 : 00000000 00000000 ffff00ff ff000000
                     * c6 : ffff00ff ff000000 00000000 00000000
                     */

                    /* gcvVX_PARTTERN_MODE_DISK
                     *          x        y       z        w
                     * x max:
                     * c4 : 00010101 00000000 01010101 01000000
                     * c5 : 01010101 01000000 01010101 01000000
                     * c6 : 00010101 00000000 00000000 00000000
                     *
                     * + min:
                     * c4 : ff000000 ff000000 00000000 00000000
                     * c5 : 00000000 00000000 00000000 00000000
                     * c6 : ff000000 ff000000 00000000 00000000
                     */

                    vx_uint32 bin[12];
                    /*c4*/
                    bin[0] = _gcfVX_PatternValue(func, m[0], gceVX_MEDIAN_CROSS_NOCARE) | (_gcfVX_PatternValue(func, m[1], gceVX_MEDIAN_CROSS_NOCARE) << 8)  | (_gcfVX_PatternValue(func, m[2], gceVX_MEDIAN_CROSS_NOCARE) << 16)  | (_gcfVX_PatternValue(func, m[3], gceVX_MEDIAN_CROSS_NOCARE) << 24);

                    bin[1] = _gcfVX_PatternValue(func, m[4], gceVX_MEDIAN_CROSS_NOCARE);

                    bin[2] = _gcfVX_PatternValue(func, m[5], gceVX_MEDIAN_CROSS_NOCARE) | (_gcfVX_PatternValue(func, m[6], gceVX_MEDIAN_CROSS_NOCARE) << 8)  | (_gcfVX_PatternValue(func, m[7], gceVX_MEDIAN_CROSS_NOCARE) << 16)  | (_gcfVX_PatternValue(func, m[8], gceVX_MEDIAN_CROSS_NOCARE) << 24);

                    bin[3] = _gcfVX_PatternValue(func, m[9], gceVX_MEDIAN_CROSS_NOCARE);

                    /*c5*/
                    bin[4] = _gcfVX_PatternValue(func, m[10], gceVX_MEDIAN_CROSS_NOCARE) | (_gcfVX_PatternValue(func, m[11], gceVX_MEDIAN_CROSS_NOCARE) << 8)  | (_gcfVX_PatternValue(func, m[12], gceVX_MEDIAN_CROSS_NOCARE) << 16)  | (_gcfVX_PatternValue(func, m[13], gceVX_MEDIAN_CROSS_NOCARE) << 24);

                    bin[5] = _gcfVX_PatternValue(func, m[14], gceVX_MEDIAN_CROSS_NOCARE);

                    bin[6] = _gcfVX_PatternValue(func, m[15], gceVX_MEDIAN_CROSS_NOCARE) | (_gcfVX_PatternValue(func, m[16], gceVX_MEDIAN_CROSS_NOCARE) << 8)  | (_gcfVX_PatternValue(func, m[17], gceVX_MEDIAN_CROSS_NOCARE) << 16)  | (_gcfVX_PatternValue(func, m[18], gceVX_MEDIAN_CROSS_NOCARE) << 24);

                    bin[7] = _gcfVX_PatternValue(func, m[19], gceVX_MEDIAN_CROSS_NOCARE);

                    /*c6*/
                    bin[8] = _gcfVX_PatternValue(func, m[20], gceVX_MEDIAN_CROSS_NOCARE) | (_gcfVX_PatternValue(func, m[21], gceVX_MEDIAN_CROSS_NOCARE) << 8)  | (_gcfVX_PatternValue(func, m[22], gceVX_MEDIAN_CROSS_NOCARE) << 16)  | (_gcfVX_PatternValue(func, m[23], gceVX_MEDIAN_CROSS_NOCARE) << 24);

                    bin[9] = _gcfVX_PatternValue(func, m[24], gceVX_MEDIAN_CROSS_NOCARE);

                    bin[10] =
                    bin[11] = 0;


                    gcoOS_MemCopy(&kernelContext->uniforms[2].uniform, bin, sizeof(bin));
                    kernelContext->uniforms[2].index       = 4;
                    kernelContext->uniforms[2].num         = 4 * 4 * 3;
                    kernelContext->uniform_num += 3;
                }

                kernelContext->params.xstep        = 1;
                kernelContext->params.ystep        = 1;
                kernelContext->params.volume       = (pattern == VX_PATTERN_CROSS) ? gcvVX_PARTTERN_MODE_CROSS: (pattern == VX_PATTERN_DISK) ? gcvVX_PARTTERN_MODE_DISK : gcvVX_PARTTERN_MODE_OTHER;
            }
            break;
        }

#if gcdVX_OPTIMIZER
        kernelContext->borders             = bordermode->mode;
#else
        kernelContext->params.borders      = border->mode;
#endif

        if (node->base.context->evisNoInst.noFilter)
        {
            vx_uint32 rect[2];
            rect[0] = height;
            rect[1] = FV(1);
            gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, rect, sizeof(rect));
            kernelContext->uniforms[0].index       = 2;
            kernelContext->uniforms[0].num         = 4 * 2;
        }
        else
        {
            vx_uint32 rect[2];
            rect[0] = height;
            rect[1] = FV(1);
            gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, rect, sizeof(rect));
            kernelContext->uniforms[0].index       = 2;
            kernelContext->uniforms[0].num         = 4 * 2;
        }

        kernelContext->uniform_num ++;


        {
            vx_uint32 bin[4];

            bin[0] =
            bin[1] =
            bin[2] =
            bin[3] = FORMAT_VALUE(border->constant_value.U32);

            gcoOS_MemCopy(&kernelContext->uniforms[1].uniform, bin, sizeof(bin));
            kernelContext->uniforms[1].num = 4 * 4;
            kernelContext->uniforms[1].index = 3;
            kernelContext->uniform_num ++;
        }
        status |= vxQueryMatrix(mask, VX_MATRIX_ORIGIN, &origin, sizeof(origin));
        kernelContext->params.policy = origin.x;
        kernelContext->params.rounding = origin.y;
        kernelContext->params.evisNoInst = node->base.context->evisNoInst;

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
}

