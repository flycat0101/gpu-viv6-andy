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
#include <stdio.h>

vx_status vxHistogram(vx_node node, vx_image src, vx_distribution dist, vx_reference* staging)
{
    vx_status status = VX_SUCCESS;
    vx_size offset = 0;
    vx_size numBins = 0;
    vx_uint32 window_size = 0, count = 0;
    vx_uint32* dist_ptr = NULL;
    vx_size size = 0;
    gcoVX_Kernel_Context * kernelContext = gcvNULL;
	vx_uint32 rang = 0;

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

    status = vxQueryDistribution(dist, VX_DISTRIBUTION_ATTRIBUTE_BINS, &numBins, sizeof(numBins));
    status |= vxQueryDistribution(dist, VX_DISTRIBUTION_ATTRIBUTE_OFFSET, &offset, sizeof(offset));
    status |= vxQueryDistribution(dist, VX_DISTRIBUTION_ATTRIBUTE_WINDOW, &window_size, sizeof(window_size));
    status |= vxQueryDistribution(dist, VX_DISTRIBUTION_ATTRIBUTE_RANGE, &rang, sizeof(rang));
    status |= vxQueryDistribution(dist, VX_DISTRIBUTION_ATTRIBUTE_SIZE, &size, sizeof(size));

    count = (vx_uint32)ceil(numBins/16.0f);

    status |= vxAccessDistribution(dist, (void **)&dist_ptr, VX_WRITE_ONLY);
	gcoOS_ZeroMemory(dist_ptr, size);

    if (node->base.context->evisNoInst.noSelectAdd)
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

        kernelContext->params.kernel		= gcvVX_KERNEL_HISTOGRAM;
        kernelContext->params.xstep		    = 16;
        kernelContext->params.ystep         = 1;
        kernelContext->params.policy		= (gctUINT32)offset;

        kernelContext->params.evisNoInst = node->base.context->evisNoInst;

    }
    else
    {
		gcoVX_Kernel_Context_Reg bin0[4] = {{{0}}}, bin1[4] = {{{0}}};
		gctUINT32 bytes = sizeof(bin0), i = 0, j = 0;
		vx_uint32 height = 0;

		vxQueryImage(src, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));

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

			/* init min0 */
			bin0[0].bin16[0] = (vx_uint16)(offset + 16 * j * window_size);
			bin0[1].bin16[0] = (vx_uint16)(window_size + bin0[0].bin16[0] - 1);

			/* init min1 */
			if(numBins > 8)
			{
				bin1[0].bin16[0] = (vx_uint16)(bin0[0].bin16[0] + window_size * 8);
				bin1[1].bin16[0] = (vx_uint16)(bin1[0].bin16[0] + window_size - 1);
			}

			/* set non-use value as the max to avoid assertion */
			for(i = 1; i < 16; i++)
			{
				if(i < 8)
					bin0[1].bin16[i] = (i < numBins)?(vx_uint16)(bin0[1].bin16[i - 1] + window_size):(vx_uint16)(bin0[1].bin16[numBins - 1] + window_size);          /*c4*/
				else if(i != 8)
					bin1[1].bin16[i%8] = (i < numBins)?(vx_uint16)(bin1[1].bin16[i%8 - 1] + window_size):(vx_uint16)(bin1[1].bin16[numBins%8 - 1] + window_size);      /*c8*/
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

		kernelContext->params.kernel		= gcvVX_KERNEL_HISTOGRAM;
		kernelContext->params.xstep		    = 16;
		kernelContext->params.ystep         = height;
		kernelContext->params.volume        = count;

		kernelContext->params.evisNoInst = node->base.context->evisNoInst;
    }

    status = gcfVX_Kernel(kernelContext);

#if gcdVX_OPTIMIZER
    if (!node || !node->kernelContext)
    {
        vxFree(kernelContext);
    }
#endif

    status |= vxCommitDistribution(dist, dist_ptr);

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
    }

    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));

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

    kernelContext->uniform_num		= 2;

    status = gcfVX_Kernel(kernelContext);

#if gcdVX_OPTIMIZER
    if (!node || !node->kernelContext)
    {
        vxFree(kernelContext);
    }
#endif

    return status;
}

vx_status vxEqualizeHist_cdf(vx_node node, vx_image cdf, vx_uint32 wxh,  vx_scalar min, vx_image hist)
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
    kernelContext->params.step	  = EQUAL_HISTOGRAM_CDF;

    kernelContext->params.kernel  = gcvVX_KERNEL_EQUALIZE_HISTOGRAM;
    kernelContext->params.xstep   = 4;
	kernelContext->params.volume  = wxh;

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
    }

    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));

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

    kernelContext->uniform_num		= 2;

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
    }

    vxQueryImage(hist, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
    vxQueryImage(hist, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));

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

    status = gcfVX_Kernel(kernelContext);

#if gcdVX_OPTIMIZER
    if (!node || !node->kernelContext)
    {
        vxFree(kernelContext);
    }
#endif

    return status;
}
