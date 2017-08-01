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
#include <stdio.h>

extern vx_int16 Fp32toFp16(vx_float32 val);

vx_status vxHistogram(vx_node node, vx_image src, vx_distribution dist, vx_reference* staging)
{
    vx_status status = VX_SUCCESS;
    vx_int32 offset = 0;
    vx_size numBins = 0;
    vx_uint32 window_size = 0, count = 0;
    vx_uint32* dist_ptr = NULL;
    vx_size size = 0;
    gcoVX_Kernel_Context * kernelContext = gcvNULL;
    vx_size rang = 0;
    vx_uint16 min = 0;

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

    status = vxQueryDistribution(dist, VX_DISTRIBUTION_BINS, &numBins, sizeof(numBins));
    status |= vxQueryDistribution(dist, VX_DISTRIBUTION_OFFSET, &offset, sizeof(offset));
    status |= vxQueryDistribution(dist, VX_DISTRIBUTION_WINDOW, &window_size, sizeof(window_size));
    status |= vxQueryDistribution(dist, VX_DISTRIBUTION_RANGE, &rang, sizeof(vx_uint32));
    status |= vxQueryDistribution(dist, VX_DISTRIBUTION_SIZE, &size, sizeof(size));

    count = (vx_uint32)ceil(numBins/16.0f);
    min = (vx_uint16)(offset);

    status |= vxAccessDistribution(dist, (void **)&dist_ptr, VX_WRITE_ONLY);
    gcoOS_ZeroMemory(dist_ptr, size);

    if (node->base.context->evisNoInst.isVX2)
    {
        vx_uint32 height = 0;
        vxQueryImage(src, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
        count = (vx_uint32)numBins;

        /*index = 0*/
        gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, src, GC_VX_INDEX_AUTO);

        /*index = 1*/
        gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_DISTRIBUTION, dist, GC_VX_INDEX_AUTO);

        kernelContext->uniform_num = 0;
        {
            gcoVX_Kernel_Context_Reg bin;
            vx_float32 offset_obj = (0.125f - offset)/window_size;
            vx_float32 inv_win = 1.0f/window_size;

            gcoOS_ZeroMemory(&bin, sizeof(gcoVX_Kernel_Context_Reg));
            bin.bin16[0] = Fp32toFp16(1.0f);
            bin.bin16[1] = 0;
            bin.bin16[2] = Fp32toFp16(inv_win);
            bin.bin16[3] = Fp32toFp16(offset_obj);

            gcoOS_MemCopy(&kernelContext->uniforms[kernelContext->uniform_num].uniform, &bin, sizeof(gcoVX_Kernel_Context_Reg));
            kernelContext->uniforms[kernelContext->uniform_num].num = 4 * 4;
            kernelContext->uniforms[kernelContext->uniform_num++].index = 2;
        }
        {
            vx_uint8 constantData[16] = {0, 32, 64, 96, 0, 0, 0, 0, 16, 16, 16, 16, 0, 0, 0, 0};

            gcoOS_MemCopy(&kernelContext->uniforms[kernelContext->uniform_num].uniform, constantData, sizeof(constantData));
            kernelContext->uniforms[kernelContext->uniform_num].num = sizeof(constantData) / sizeof(vx_uint8);
            kernelContext->uniforms[kernelContext->uniform_num++].index = 3;
        }
        {

            vx_int32 bin[4];

            bin[0] =
            bin[1] =
            bin[2] =
            bin[3] = FV2(offset);

            gcoOS_MemCopy(&kernelContext->uniforms[kernelContext->uniform_num].uniform, bin, sizeof(bin));
            kernelContext->uniforms[kernelContext->uniform_num].num = 4 * 4;
            kernelContext->uniforms[kernelContext->uniform_num++].index = 4;
        }

        {


            vx_int32 bin[4];
            bin[0] =
            bin[1] =
            bin[2] =
            bin[3] = 0x00010001;

            gcoOS_MemCopy(&kernelContext->uniforms[kernelContext->uniform_num].uniform, bin, sizeof(bin));
            kernelContext->uniforms[kernelContext->uniform_num].index = 5;
            kernelContext->uniforms[kernelContext->uniform_num++].num = 4 * 4;

        }

        kernelContext->params.kernel        = gcvVX_KERNEL_HISTOGRAM;
        kernelContext->params.xstep         = 8;
        kernelContext->params.ystep         = height;
        kernelContext->params.volume        = count;
        kernelContext->params.col           = height;
        kernelContext->params.scale         = 1.0f/window_size;

        kernelContext->params.evisNoInst = node->base.context->evisNoInst;
    }
    else if (node->base.context->evisNoInst.noSelectAdd)
    {
        /*index = 0*/
        gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, src, GC_VX_INDEX_AUTO);

        /*index = 1*/
        gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_DISTRIBUTION, dist, GC_VX_INDEX_AUTO);

        {
            vx_float32 bin[4];

            bin[0] = (vx_float32)offset;
            bin[1] = (vx_float32)(offset + rang);
            bin[2] = 1.0f/window_size;
            bin[3] = 0;

            gcoOS_MemCopy(&kernelContext->uniforms[kernelContext->uniform_num].uniform, bin, sizeof(bin));
            kernelContext->uniforms[kernelContext->uniform_num].num = 4 * 4;
            kernelContext->uniforms[kernelContext->uniform_num++].index = 2;
        }
        {
            vx_int32 bin[4];
            bin[0] =
            bin[1] =
            bin[2] =
            bin[3] = 0x01010101;

            gcoOS_MemCopy(&kernelContext->uniforms[kernelContext->uniform_num].uniform, bin, sizeof(bin));
            kernelContext->uniforms[kernelContext->uniform_num].num = 4 * 4;
            kernelContext->uniforms[kernelContext->uniform_num++].index = 3;
        }

        /*
        *   |                            numBins                          |
        *          /           /                    \             \       |
        *   | window_size | window_size | ... | window_size | window_size |
        *   |<--------------------------- range ------------------------->|
        * offset      window_size
        */

        kernelContext->params.kernel        = gcvVX_KERNEL_HISTOGRAM;
        kernelContext->params.xstep         = 16;
        kernelContext->params.ystep         = 1;
        kernelContext->params.policy        = (gctUINT32)offset;

        kernelContext->params.evisNoInst = node->base.context->evisNoInst;

    }
    else
    {
        gcoVX_Kernel_Context_Reg bin0[4] = {{{0}}}, bin1[4] = {{{0}}};
        gctUINT32 bytes = sizeof(bin0), i = 0, j = 0;
        vx_uint32 height = 0;

        vx_uint32 index = 0, position = 0;

        vxQueryImage(src, VX_IMAGE_HEIGHT, &height, sizeof(height));

        /*index = 0*/
        gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, src, GC_VX_INDEX_AUTO);

        /*index = 1*/
        gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_DISTRIBUTION, dist, GC_VX_INDEX_AUTO);

        /*
        *   |                            numBins                          |
        *          /           /                    \             \       |
        *   | window_size | window_size | ... | window_size | window_size |
        *   |<--------------------------- range ------------------------->|
        * offset      window_size
        */

        kernelContext->uniform_num = 0;

        for(j = 0; j < count; j++)
        {
            vx_int32 n[2] = {0};
            gcoVX_Kernel_Context_Reg * bin = bin0;
            gcoOS_ZeroMemory(bin0, sizeof(gcoVX_Kernel_Context_Reg) * 4);
            gcoOS_ZeroMemory(bin1, sizeof(gcoVX_Kernel_Context_Reg) * 4);
            /*
            * bin0     x            y            z            w
            * 0: |       min0 |            |            |            |
            * 1: | max1  max0 | max3  max2 | max3  max4 | max7  max6 |
            * 2: |            |            |            |            |
            * 3: |            |            |            |            |
            *
            * bin1      x           y            z            w
            * 0: |       min1 |            |            |            |
            * 1: | max9  max8 | max11 max10| max13 max12| max15 max14|
            * 2: |            |            |            |            |
            * 3: | 0x01010101 | 0x01010101 | 0x01010101 | 0x01010101 |
            *
            */

            bin[0].bin16[0] = min;

            for(i = bin0[0].bin16[0]; i < 256; i++)
            {
                n[0] = (vx_int32)((i - offset)*numBins/rang);
                n[1] = (vx_int32)((i - offset + 1)*numBins/rang);

                if (index % 8 == 0)
                {
                    if (n[0] == (vx_int32)index && n[0] != n[1])
                    {
                        bin[1].bin16[position ++ ]  = (vx_uint16)(i);

                        index ++;
                    }
                }

                else
                {
                    if (n[0] == (vx_int32)index && n[1] > (vx_int32)index)
                    {
                        bin[1].bin16[position ++ ] = (vx_uint16)i;
                        index ++;
                    }
                }

                if ((position > 0) && (position < 8) && (index >= numBins))
                {
                    vx_uint32 c = 0;
                    vx_uint16 max = (vx_uint16)(offset + rang);
                    for(c = position; c < 8; c++)
                    {
                        if(bin[1].bin16[position -1] > max)
                        {
                            bin[1].bin16[position -1] =
                            bin[1].bin16[c] = max;
                        }

                        else
                            bin[1].bin16[c] = bin[1].bin16[position -1];
                    }
                    break;
                }

                if (position > 0 && position % 8 == 0)
                {
                    min = bin[1].bin16[7] + 1;

                    if (((index %16) / 8) == 1)
                        bin = bin1;
                    else
                        bin = bin0;
                }

                if (position == 8)
                {
                    if (index % 16 == 0)
                    {
                        position = 0;
                        break;
                    }
                    else if (position % 8 == 0)
                    {
                        bin[0].bin16[0] = min;
                        position = 0;
                    }
                }
            }

            bin1[3].bin32[0] =
            bin1[3].bin32[1] =
            bin1[3].bin32[2] =
            bin1[3].bin32[3] = 0x01010101;

            gcoOS_MemCopy(&kernelContext->uniforms[kernelContext->uniform_num].uniform, bin0, bytes);
            kernelContext->uniforms[kernelContext->uniform_num].index = 4 * (2 * j + 1);
            kernelContext->uniforms[kernelContext->uniform_num++].num = 4 * 4;

            gcoOS_MemCopy(&kernelContext->uniforms[kernelContext->uniform_num].uniform, bin1, bytes);
            kernelContext->uniforms[kernelContext->uniform_num].index = kernelContext->uniforms[kernelContext->uniform_num - 1].index + 4;
            kernelContext->uniforms[kernelContext->uniform_num++].num = 4 * 4;

        }

        gcoOS_MemCopy(&kernelContext->uniforms[kernelContext->uniform_num].uniform, &height, sizeof(height));
        kernelContext->uniforms[kernelContext->uniform_num].index = 3;
        kernelContext->uniforms[kernelContext->uniform_num++].num = sizeof(height) / sizeof(vx_uint32);

        kernelContext->params.kernel        = gcvVX_KERNEL_HISTOGRAM;
        kernelContext->params.xstep         = 16;
        kernelContext->params.ystep         = height;
        kernelContext->params.volume        = count;

        kernelContext->params.evisNoInst = node->base.context->evisNoInst;
    }

    kernelContext->node = node;

    status = gcfVX_Kernel(kernelContext);

#if gcdVX_OPTIMIZER
    if (!node || !node->kernelContext)
    {
        vxFree(kernelContext);
    }
#endif

    status |= vxCommitDistribution(dist, NULL);

    return status;
}

#define EQUAL_HISTOGRAM_HIST 0
#define EQUAL_HISTOGRAM_GCDF 1
#define EQUAL_HISTOGRAM_CDF  2
#define EQUAL_HISTOGRAM_LUT  3

vx_status vxEqualizeHist_hist(vx_node node, vx_image src, vx_image hist, vx_scalar min)
{
    vx_status status = VX_SUCCESS;
    gcoVX_Kernel_Context * kernelContext = gcvNULL;
    vx_uint32 constantData[8] = {0, 8, 16, 24, 0, 0, 0, 0};
    vx_uint32 height = 0;

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

    /*index = 0*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, src, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, hist, GC_VX_INDEX_AUTO);

    /*index = 2*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_SCALAR, min, GC_VX_INDEX_AUTO);

    /*step is step index*/
    kernelContext->params.step             = EQUAL_HISTOGRAM_HIST;

    kernelContext->params.kernel           = gcvVX_KERNEL_EQUALIZE_HISTOGRAM;
    kernelContext->params.xstep            = 16;
    kernelContext->params.ystep            = height;

    gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, constantData, sizeof(constantData));
    kernelContext->uniforms[0].index = 3;
    kernelContext->uniforms[0].num = sizeof(constantData) / sizeof(vx_uint8);

    gcoOS_MemCopy(&kernelContext->uniforms[1].uniform, &height, sizeof(height));
    kernelContext->uniforms[1].index       = 4;
    kernelContext->uniforms[1].num         = sizeof(height) / sizeof(vx_uint32);

    kernelContext->uniform_num      = 2;

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

vx_status vxEqualizeHist_cdf(vx_node node, vx_image cdf, vx_uint32 wxh, vx_scalar min, vx_image hist)
{
    vx_status status = VX_SUCCESS;
    gcoVX_Kernel_Context * kernelContext = gcvNULL;
    vx_uint32 bin[4];

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
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, cdf, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, hist, GC_VX_INDEX_AUTO);

    /*index = 2*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_SCALAR, min, GC_VX_INDEX_AUTO);

    bin[0] = 0;
    bin[1] = 1;
    bin[2] = 2;
    bin[3] = 3;

    gcoOS_MemCopy(&kernelContext->uniforms[kernelContext->uniform_num].uniform, bin, sizeof(bin));
    kernelContext->uniforms[kernelContext->uniform_num].num = 4 * 4;
    kernelContext->uniforms[kernelContext->uniform_num++].index = 4;

    /*step is step index*/
    kernelContext->params.step    = EQUAL_HISTOGRAM_CDF;

    kernelContext->params.kernel  = gcvVX_KERNEL_EQUALIZE_HISTOGRAM;
    kernelContext->params.xstep   = 4;
    kernelContext->params.volume  = wxh;

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

vx_status vxEqualizeHist_lut(vx_node node, vx_image src, vx_image hist, vx_image dst)
{
    vx_status status = VX_SUCCESS;
    gcoVX_Kernel_Context * kernelContext = gcvNULL;
    vx_uint32 constantData[8] = {0, 8, 16, 24, 0, 0, 0, 0};
    vx_uint32 height = 0;

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

    /*index = 0*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, src, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, hist, GC_VX_INDEX_AUTO);

    /*index = 2*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, dst, GC_VX_INDEX_AUTO);

    /*step is step index*/
    kernelContext->params.step         = EQUAL_HISTOGRAM_LUT;

    kernelContext->params.kernel       = gcvVX_KERNEL_EQUALIZE_HISTOGRAM;
    kernelContext->params.xstep        = 16;
    kernelContext->params.ystep        = height;

    gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, constantData, sizeof(constantData));
    kernelContext->uniforms[0].index = 3;
    kernelContext->uniforms[0].num = sizeof(constantData) / sizeof(vx_uint8);

    gcoOS_MemCopy(&kernelContext->uniforms[1].uniform, &height, sizeof(height));
    kernelContext->uniforms[1].index       = 4;
    kernelContext->uniforms[1].num         = sizeof(height) / sizeof(vx_uint32);

    kernelContext->uniform_num      = 2;

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

vx_status vxEqualizeHist_gcdf(vx_node node, vx_image hist, vx_scalar minIndex, vx_image cdf, vx_scalar minValue)
{
    vx_status status = VX_SUCCESS;
    gcoVX_Kernel_Context * kernelContext = gcvNULL;
    vx_uint32 width = 0, height = 0;

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

    vxQueryImage(hist, VX_IMAGE_HEIGHT, &height, sizeof(height));
    vxQueryImage(hist, VX_IMAGE_WIDTH, &width, sizeof(width));

    /*index = 0*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, hist, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, cdf, GC_VX_INDEX_AUTO);

    /*index = 2*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_SCALAR, minIndex, GC_VX_INDEX_AUTO);

    /*index = 3*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_SCALAR, minValue, GC_VX_INDEX_AUTO);

    /*step is step index*/
    kernelContext->params.step             = EQUAL_HISTOGRAM_GCDF;

    kernelContext->params.kernel           = gcvVX_KERNEL_EQUALIZE_HISTOGRAM;
    kernelContext->params.xstep            = width;
    kernelContext->params.ystep            = height;

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

