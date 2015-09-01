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

static vx_status vxVivMeanStdDev(vx_image input, vx_scalar mean, vx_scalar stddev)
{
    vx_status status = VX_SUCCESS;
    gcoVX_Kernel_Context context = {{0}};
    vx_uint32 inputWidth = 0;
    vx_uint32 inputHeight = 0;
    vx_uint32 rect[1];
    vx_uint64 maxSize = 0;

    vxQueryImage(input, VX_IMAGE_ATTRIBUTE_WIDTH, &inputWidth, sizeof(vx_uint32));

    vxQueryImage(input, VX_IMAGE_ATTRIBUTE_HEIGHT, &inputHeight, sizeof(vx_uint32));
    rect[0] = inputHeight;

	/*index = 0*/
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

	/*index = 1*/
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_SCALAR, mean, GC_VX_INDEX_AUTO);

	/* index = 2 */
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_SCALAR, stddev, GC_VX_INDEX_AUTO);

    context.params.kernel = gcvVX_KERNEL_MEAN_STDDEV;
    context.params.xstep  = 16;

    maxSize = (vx_uint64)inputWidth * inputHeight * 255 * 255;

    /*check size of sqsum is larger than 32-bit, then do shift into 32-bit*/
    if (maxSize >= 0xffffffff)
    {
        context.params.ystep        = inputHeight;

        gcoOS_MemCopy(&context.uniforms[0].uniform, rect, sizeof(rect));
        context.uniforms[0].index       = 3;
        context.uniforms[0].num         = sizeof(rect) / sizeof(vx_uint32);
        context.uniform_num             = 1;
    }

    status = gcfVX_Kernel(&context);

    return status;
}

vx_status vxMeanStdDev(vx_image input, vx_scalar mean, vx_scalar stddev)
{
    vx_status status  = VX_SUCCESS;
    vx_float32 fmean = 0.0f, fstddev = 0.0f, npix = 0.0f;
    vx_uint64 sum = 0, sqsum = 0, maxsize;
    vx_imagepatch_addressing_t addrs;
    void *base_ptr = NULL;
    vx_rectangle_t rect ;
    vx_uint32 width = 0, height = 0;

    /* clean init value */
    status |= vxCommitScalarValue(mean, &fmean);
    status |= vxCommitScalarValue(stddev, &fstddev);

    status |= vxGetValidRegionImage(input, &rect);
    status |= vxAccessImagePatch(input, &rect, 0, &addrs, &base_ptr, VX_READ_ONLY);

    status |= vxVivMeanStdDev(input, mean, stddev);
    /* To Clean up */
    status |= gcfVX_Flush(gcvTRUE);

    status |= vxAccessScalarValue(mean, &sum);
    status |= vxAccessScalarValue(stddev, &sqsum);

    width = rect.end_x - rect.start_x;
    height = rect.end_y - rect.start_x;

    maxsize = (vx_uint64)width * height * 255 * 255;

    /*check size of sqsum is larger than 32-bit*/
    if (maxsize >= 0xffffffff)
    {
        sum = sum << 1;
        sqsum = sqsum << 1;
    }

    /*
     * fmean = sum / (width * height)
     * fstddev = sqsum/ (width * height) - (fmean * fmean)
     */

    npix = addrs.dim_x*addrs.dim_y * 1.0f;
    fmean = (sum)/npix;

    fstddev = sqsum/npix - fmean * fmean;
    fstddev = (vx_float32)sqrt(fstddev);

    status |= vxCommitScalarValue(mean, &fmean);
    status |= vxCommitScalarValue(stddev, &fstddev);
    status |= vxCommitImagePatch(input, &rect, 0, &addrs, base_ptr);

    return status;
}

#define MML_FILTER 0
#define MML_LOC   1

static vx_status packLocation(vx_image img, vx_array src_array, vx_uint32 src_row_items, vx_uint32 src_col_items,vx_int32 *ptr_count, vx_array dst_array )
{
    vx_status status  = VX_SUCCESS;
    void *count_base = NULL;
    void *src_list_base = NULL, *dst_list_base = NULL;
    vx_uint32 src_list_offset =0, dst_list_offset =0;
    vx_size cap = 0, itemsize = 0, itemcount = 0, size = 0, m_count = 0;
    vx_uint32 i;

    if (img == NULL)
    {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    if (img->memory.logicals[0] == NULL)
    {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    if(dst_array)
    {
        vxTruncateArray(dst_array, 0);
        vxQueryArray(dst_array, VX_ARRAY_ATTRIBUTE_ITEMSIZE, &itemsize, sizeof(itemsize));
        vxQueryArray(dst_array, VX_ARRAY_ATTRIBUTE_CAPACITY, &cap, sizeof(cap));
        dst_list_base = dst_array->memory.logicals[0];
    }
    if (src_array)
    {
        src_list_base = src_array->memory.logicals[0];
    }

    vxQueryImage(img, VX_IMAGE_ATTRIBUTE_SIZE, &size, sizeof(size));

    count_base = malloc(size);

    memcpy(count_base, img->memory.logicals[0], size);

    for(i = 0; i < src_col_items; i ++)
    {
        vx_uint32 count = *((vx_uint32*)(count_base) + i);
        if(count > 0)
        {
            if (src_array && dst_array && (count + itemcount <= cap))
            {
                src_list_offset = i * src_row_items * (vx_uint32)itemsize;
                memcpy((void*)((vx_uint8*)dst_list_base + dst_list_offset), (void*)((vx_uint8*)src_list_base + src_list_offset), count * itemsize);
                itemcount += count;
                dst_list_offset += count * (vx_uint32)itemsize;
            }
            m_count += count;
        }
    }

    if (dst_array)
    {
        dst_array->itemCount = itemcount;
    }
    *ptr_count = (vx_int32)m_count;
    free(count_base);

    return status;
}

static vx_status getLocation(vx_image img, vx_image countImg, vx_int32 value, vx_df_image format, vx_int32 *count, vx_array arrays, vx_array tempArray)
{
    vx_status status  = VX_SUCCESS;
    gcoVX_Kernel_Context   context  = {{0}};
    vx_int32 width, height;
    vx_size itemSize = 0;
    vx_uint32 bin[4];
    vx_uint32 constantData[8] = {0, 8, 16, 24, 0, 0, 0, 0};
    vx_bool return_loc = vx_false_e;

    vxQueryImage(img, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
    vxQueryImage(img, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
    if(arrays && tempArray)
    {
        vxQueryArray(arrays, VX_ARRAY_ATTRIBUTE_ITEMSIZE, &itemSize, sizeof(itemSize));
        return_loc = vx_true_e;
	}

    /*index = 0*/
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, img, GC_VX_INDEX_AUTO);
    /*index = 1*/
    gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, countImg, GC_VX_INDEX_AUTO);
    if (tempArray)
    {
        /*index = 2*/
        gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_ARRAY, tempArray, GC_VX_INDEX_AUTO);
    }

    bin[0] = (format == VX_DF_IMAGE_S16) ? FV2(0xffff & value) : FV((vx_uint8)value);
    bin[1] = (format == VX_DF_IMAGE_S16) ? FV2(1) : FV(1);
    bin[2] = width;
    bin[3] = height;
    gcoOS_MemCopy(&context.uniforms[0].uniform, bin, sizeof(bin));
    context.uniforms[0].index       = 3;
    context.uniforms[0].num         = 16;

    if (format == VX_DF_IMAGE_S16)
    {
        constantData[1] = 16;
        constantData[2] = 32;
        constantData[3] = 32;
    }

    gcoOS_MemCopy(&context.uniforms[1].uniform, constantData, sizeof(constantData));
    context.uniforms[1].index       = 4;
    context.uniforms[1].num         = sizeof(constantData) / sizeof(vx_uint32);
    context.uniform_num             = 2;


    context.params.kernel           = gcvVX_KERNEL_MINMAXLOC;
    context.params.step             = MML_LOC;
    context.params.xstep            = width;
    context.params.ystep            = 1;
    context.params.volume           = (vx_uint32)itemSize;
    context.params.clamp            = (vx_uint32)itemSize * width;
    context.params.policy           = return_loc;

    status = gcfVX_Kernel(&context);

    status |= gcfVX_Flush(gcvTRUE);

    status |= packLocation(countImg, tempArray, width, height, count, arrays);

    return status;
}

static vx_status vxVivMinMaxLoc_filter(vx_image input, vx_scalar filter_min, vx_scalar filter_max,
                                       vx_uint32 xstep, vx_uint32 ystep)
{
    vx_status                status = VX_SUCCESS;
    gcoVX_Kernel_Context   context  = {{0}};
	gctUINT32 i = 0;
    gcoVX_Index indexs[]                = {
        /* index,  num,             shift0,         shift1,      mask0,    mask1 */
        {    3,   4 * 4, {FV4(3*8,(16+3)*8,0,0),		FV4(0,0,0, 0),		FV4(4*8,4*8,0,0),	FV4(0,0,0,0)}  }, /*  */
        {    4,   4 * 4, {FV4(6*8,(16+6)*8,0,0),		FV4(0,0,0, 0),		FV4(4*8,4*8,0,0),	FV4(0,0,0,0)}  }, /*  */
	};

	/*index = 0*/
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

	/*index = 1*/
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_SCALAR, filter_min, GC_VX_INDEX_AUTO);

	/* index = 2 */
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_SCALAR, filter_max, GC_VX_INDEX_AUTO);

    context.params.step             = MML_FILTER;
    context.params.borders          = VX_BORDER_MODE_REPLICATE;
    context.params.kernel           = gcvVX_KERNEL_MINMAXLOC;
    context.params.xstep            = xstep;
    context.params.ystep            = ystep;
    context.uniform_num             = sizeof(indexs)/sizeof(indexs[0]);

    for(i = 0; i < context.uniform_num; i++)
    {
        gcoOS_MemCopy(&context.uniforms[i].uniform, indexs[i].bin, sizeof(indexs[i].bin));
        context.uniforms[i].num = indexs[i].num;
        context.uniforms[i].index = indexs[i].index;
    }

    status = gcfVX_Kernel(&context);

    return status;
}


vx_status vxMinMaxLoc(vx_image input, vx_scalar minVal, vx_scalar maxVal, vx_array minLoc, vx_array maxLoc, vx_scalar minCount, vx_scalar maxCount, vx_reference* staging)
{
    vx_status status  = VX_SUCCESS;
    vx_int32 height, xstep, ystep;
    vx_scalar filter_min, filter_max;
    vx_image count_img;
    vx_array minArray, maxArray;
    vx_df_image format;
    vx_int32 min, max, mincount, maxcount;

    vxQueryImage(input, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
    vxQueryImage(input, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));

    xstep = (format == VX_DF_IMAGE_S16) ? 6 : 14;
    ystep = height;
    height = (vx_int32)ceil(height* 1.0f / ystep);

    filter_min = (vx_scalar)staging[0];
    filter_max = (vx_scalar)staging[1];
    count_img = (vx_image)staging[2];

    status = vxVivMinMaxLoc_filter(input, filter_min, filter_max, xstep, ystep);
    /* To Clean up */
    status |= gcfVX_Flush(gcvTRUE);

	status |= vxAccessScalarValue(filter_min, &min);
	status |= vxAccessScalarValue(filter_max, &max);

    status |= vxCommitScalarValue(filter_min, &min);
    status |= vxCommitScalarValue(filter_max, &max);

    minArray = (vx_array)staging[3];
    status |= getLocation(input, count_img, min, format, &mincount, minLoc, minArray);
    maxArray = (vx_array)staging[4];
    status |= getLocation(input, count_img, max, format, &maxcount, maxLoc, maxArray);

    if(minCount)status |= vxCommitScalarValue(minCount, &mincount);
    if(maxCount)status |= vxCommitScalarValue(maxCount, &maxcount);

    status |= vxCommitScalarValue(minVal, &min);
    status |= vxCommitScalarValue(maxVal, &max);


    return status;

}
