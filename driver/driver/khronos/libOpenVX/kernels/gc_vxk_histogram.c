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
#include <stdio.h>

static vx_status _vxHistogram(vx_image src, vx_distribution dist, vx_size offset)
{
    gcoVX_Kernel_Context context = {{0}};
    gcoVX_Kernel_Context_Reg bin0[4] = {{{0}}}, bin1[4] = {{{0}}};
    gctUINT32 bytes = sizeof(bin0), i = 0;
    vx_size range = 0;
    vx_size numBins = 0;
    vx_uint32 window_size = 0;

    vxQueryDistribution(dist, VX_DISTRIBUTION_ATTRIBUTE_BINS, &numBins, sizeof(numBins));
    vxQueryDistribution(dist, VX_DISTRIBUTION_ATTRIBUTE_RANGE, &range, sizeof(range));
    vxQueryDistribution(dist, VX_DISTRIBUTION_ATTRIBUTE_WINDOW, &window_size, sizeof(window_size));

	/*index = 0*/
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, src, GC_VX_INDEX_AUTO);

	/*index = 1*/
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_DISTRIBUTION, dist, GC_VX_INDEX_AUTO);

    /*
    *   |                            numBins                          |
    *          /           /                    \             \       |
    *   | window_size | window_size | ... | window_size | window_size |
    *   |<--------------------------- range ------------------------->|
    * offset      window_size
    */

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
    bin0[0].bin16[0] = (vx_uint16)offset;
    bin0[1].bin16[0] = (vx_uint16)(window_size + bin0[0].bin16[0] - 1);

    /* init min1 */
    if(numBins > 8)
    {
        bin1[0].bin16[0] = bin0[0].bin16[0] + (vx_uint16)(window_size * 8);
        bin1[1].bin16[0] = bin1[0].bin16[0] + (vx_uint16)(window_size - 1);
    }

    /* set non-use value as the max to avoid assertion */
    for(i = 1; i < 16; i++)
    {
        if(i < 8)
            bin0[1].bin16[i] = (i < numBins)?(bin0[1].bin16[i - 1] + (vx_uint16)window_size):(bin0[1].bin16[numBins - 1] + (vx_uint16)window_size);          /*c4*/
        else if(i != 8)
            bin1[1].bin16[i%8] = (i < numBins)?(bin1[1].bin16[i%8 - 1] + (vx_uint16)window_size):(bin1[1].bin16[numBins%8 - 1] + (vx_uint16)window_size);      /*c8*/
    }

    bin1[3].bin32[0] =
    bin1[3].bin32[1] =
    bin1[3].bin32[2] =
    bin1[3].bin32[3] = 0x01010101;
	gcoOS_MemCopy(&context.uniforms[0].uniform, bin0, bytes);
    gcoOS_MemCopy(&context.uniforms[1].uniform, bin1, bytes);

    context.uniforms[0].index = 4;
    context.uniforms[0].num	= 4 * 4;

    context.uniforms[1].index = 8;
    context.uniforms[1].num	= 4 * 4;

    context.uniform_num		= 2;

    context.params.kernel		= gcvVX_KERNEL_HISTOGRAM;
    context.params.xstep		= 16;

    return gcfVX_Kernel(&context);
}

vx_status vxHistogram(vx_image src, vx_distribution dist, vx_reference* staging)
{
    vx_status status = VX_SUCCESS;
    vx_size offset = 0;
    vx_size numBins = 0;
    vx_uint32 window_size = 0, i = 0, count = 0;
    vx_uint32* dist_ptr = NULL;
    vx_uint32* dist_src = NULL;
    vx_uint32 offsets = 0;
	vx_distribution distribution;
    vx_size size = 0;

    vxQueryDistribution(dist, VX_DISTRIBUTION_ATTRIBUTE_BINS, &numBins, sizeof(numBins));
    vxQueryDistribution(dist, VX_DISTRIBUTION_ATTRIBUTE_OFFSET, &offset, sizeof(offset));
    vxQueryDistribution(dist, VX_DISTRIBUTION_ATTRIBUTE_WINDOW, &window_size, sizeof(window_size));

    count = (vx_uint32)ceil(numBins/16.0f);

    status |= vxAccessDistribution(dist, (void **)&dist_ptr, VX_WRITE_ONLY);

    distribution = (vx_distribution)staging[0];

    status |= vxAccessDistribution(distribution, (void **)&dist_src, VX_READ_ONLY);
    status |= vxQueryDistribution(distribution, VX_DISTRIBUTION_ATTRIBUTE_SIZE, &size, sizeof(size));

    for(i = 0; i < count; i ++)
    {
		memset(dist_src, 0, size);

        status |= _vxHistogram(src, distribution, offset + 16 * i * window_size);
        /* To clean up */
        status |= gcfVX_Flush(gcvTRUE);
        memcpy(dist_ptr + offsets, dist_src, size);

        offsets +=  (vx_uint32)size/4;
    }
    status |= vxCommitDistribution(distribution, dist_src);

    status |= vxCommitDistribution(dist, dist_ptr);

    return status;
}


#define EQUAL_HISTOGRAM_HIST 0
#define EQUAL_HISTOGRAM_CDF  1
#define EQUAL_HISTOGRAM_LUT  2

static vx_status _vxVivEqualizeHist_hist(vx_image src, vx_image hist)
{
    gcoVX_Kernel_Context context = {{0}};

	/*index = 0*/
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, src, GC_VX_INDEX_AUTO);

	/*index = 1*/
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, hist, GC_VX_INDEX_AUTO);

    /*step is step index*/
    context.params.step             = EQUAL_HISTOGRAM_HIST;

    context.params.kernel           = gcvVX_KERNEL_EQUALIZE_HISTOGRAM;
    context.params.xstep            = 8;

    return gcfVX_Kernel(&context);
}

static vx_status _vxVivEqualizeHist_cdf(vx_image cdf, vx_image hist, vx_uint32 div)
{
    gcoVX_Kernel_Context context = {{0}};

	/*index = 0*/
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, cdf, GC_VX_INDEX_AUTO);

	/*index = 1*/
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, hist, GC_VX_INDEX_AUTO);

    context.params.scale    = (gctFLOAT)div;

	if(div <= 0)
	{
        vx_uint32 bin[4];

        bin[0] = 0;
        bin[1] = 1;
        bin[2] = 2;
        bin[3] = 3;

		gcoOS_MemCopy(&context.uniforms[context.uniform_num].uniform, bin, sizeof(bin));
        context.uniforms[context.uniform_num].num = 4 * 4;
        context.uniforms[context.uniform_num++].index = 4;
    }

    /*step is step index*/
    context.params.step	  = EQUAL_HISTOGRAM_CDF;

    context.params.kernel   = gcvVX_KERNEL_EQUALIZE_HISTOGRAM;
    context.params.xstep    = 4;

    return gcfVX_Kernel(&context);
}

vx_status _vxVivEqualizeHist_lut(vx_image src, vx_image hist, vx_image dst)
{
    gcoVX_Kernel_Context context = {{0}};
    vx_status status = VX_SUCCESS;

	/*index = 0*/
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, src, GC_VX_INDEX_AUTO);

	/*index = 1*/
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, hist, GC_VX_INDEX_AUTO);

	/*index = 2*/
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, dst, GC_VX_INDEX_AUTO);

    /*step is step index*/
    context.params.step         = EQUAL_HISTOGRAM_LUT;

    context.params.kernel       = gcvVX_KERNEL_EQUALIZE_HISTOGRAM;
    context.params.xstep        = 8;

    status = gcfVX_Kernel(&context);

    return status;
}

#define NUM_BINS 256
static vx_status _generateCDF(vx_image hist, vx_image cdf, vx_uint32 * min, vx_uint32 * min_cdf)
{
    vx_status status = VX_SUCCESS;
    void *hist_base = NULL, *cdf_base = NULL;
    vx_imagepatch_addressing_t hist_addr, cdf_addr;
    vx_rectangle_t rect;
    vx_uint32 x = 0, sum = 0;
    vx_uint32 *hists;
    vx_float32 *cdfs;

    *min = 0;

    status = vxGetValidRegionImage(hist, &rect);
    status |= vxAccessImagePatch(hist, &rect, 0, &hist_addr, &hist_base, VX_READ_ONLY);
    status |= vxAccessImagePatch(cdf, &rect, 0, &cdf_addr, &cdf_base, VX_WRITE_ONLY);

    /* Find min */
    hists = (vx_uint32 *)vxFormatImagePatchAddress2d(hist_base, 0, 0, &hist_addr);
    while(hists[x++] == 0)
        (*min)++;

    /* Generate cdf from histogram */
    cdfs = (vx_float32 *)vxFormatImagePatchAddress2d(cdf_base, 0, 0, &cdf_addr);

    /* Get min cdf value */
    *min_cdf = hists[*min];

    for (x = 0; x < NUM_BINS; x++)
    {
        sum += hists[x];
        cdfs[x] = (vx_float32)(sum - *min_cdf);
    }

    status |= vxCommitImagePatch(cdf, NULL, 0, &cdf_addr, cdf_base);
    status |= vxCommitImagePatch(hist, NULL, 0, &hist_addr, hist_base);

    return status;
}

vx_status vxEqualizeHist(vx_image src, vx_image dst, vx_reference* staging)
{
    vx_status status = VX_SUCCESS;
    void *base = NULL;
    vx_uint32 width = 0, height = 0, min = 0xff, min_cdf = 0, div = 0;
    vx_imagepatch_addressing_t addr;
    vx_rectangle_t rect;
    vx_image hist = (vx_image)staging[0];
    vx_image cdf = (vx_image)staging[1];

    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));

    status |= vxGetValidRegionImage(hist, &rect);
    status |= vxAccessImagePatch(hist, &rect, 0, &addr, &base, VX_WRITE_ONLY);

	gcoOS_ZeroMemory(base, 256 * 2 * 1 * 2);

    status |= vxCommitImagePatch(hist, NULL, 0, &addr, base);

    status |= _vxVivEqualizeHist_hist(src, hist);
    /* To Clean up */
    status |= gcfVX_Flush(gcvTRUE);
    status |= _generateCDF(hist, cdf, &min, &min_cdf);

    div = width * height - min_cdf;
	status |= _vxVivEqualizeHist_cdf(cdf, hist, div);

    status |= _vxVivEqualizeHist_lut(src, hist, dst);
    /* To Clean up */
    status |= gcfVX_Flush(gcvTRUE);


    return status;
}

