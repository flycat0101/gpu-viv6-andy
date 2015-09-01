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

#if VIV_HARRIS_SCORE
vx_status vxHarrisScore(vx_image grad_x, vx_image grad_y, vx_image dst,
                        vx_scalar winds, vx_scalar sens, vx_border_mode_t borders)
{
    vx_status status                    = vx_true_e;
    if (borders.mode == VX_BORDER_MODE_UNDEFINED)
    {
        gcoVX_Kernel_Context context    = {{0}};
        vx_uint32 block_size = 0, i = 0;
        vx_float32 k = 0.0f;
        vx_uint32 width = 0, height = 0;
        gcoVX_Index indexs[]            = {
            /* index,  num,             shift0,         shift1,      mask0,        mask1 */
            {    3,   4 * 4, {			   1,		       0,      0xffffffff,   0xfffffffe  }  }, /* y start */
            {    4,   4 * 4, {			   1,		       0,      0xffffffff,   0xfffffffe  }  }, /* y end */
            {    5,   4 * 4, {			   1,		       0,      0xffffffff,   0xfffffffe  }  }, /* x start */
            {    6,   4 * 4, {			   1,		       0,      0xffffffff,   0xfffffffe  }  }, /* x end */
        };

        status |= vxAccessScalarValue(winds, &block_size);
        status |= vxAccessScalarValue(sens, &k);

        status |= vxQueryImage(grad_x, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
        status |= vxQueryImage(grad_x, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));

		/*index = 0*/
		gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, grad_x, GC_VX_INDEX_AUTO);

		/*index = 1*/
		gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, grad_y, GC_VX_INDEX_AUTO);

		/*index = 2*/
		gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, dst, GC_VX_INDEX_AUTO);

        context.params.kernel               = gcvVX_KERNEL_HARRIS_CORNERS;

        context.params.borders              = VX_BORDER_MODE_UNDEFINED;

        context.params.volume               = block_size;
        context.params.scale                = k;

        context.params.col                  = width;
        context.params.row                  = height;

        context.params.xstep                = 4;

        context.uniform_num                 = sizeof(indexs)/sizeof(indexs[0]);

        for(i = 0; i < context.uniform_num; i++)
        {
								/* y start,					y end,					x start,				x end */
			vx_int32 base[4] = {block_size/2 + 1, height - (block_size/2 + 1), (block_size/2 + 1), width - (block_size/2 + 1)};
			vx_int32 offset = (i>1)?1:0;
			indexs[i].bin[0] = base[i];									 /* y start */
			indexs[i].bin[1] = indexs[i].bin[0] - offset;                /* y end */
			indexs[i].bin[2] = ((vx_int32)indexs[i].bin[1] - offset);    /* x start */
			indexs[i].bin[3] = ((vx_int32)indexs[i].bin[2] - offset);    /* x end */

            gcoOS_MemCopy(&context.uniforms[i].uniform, indexs[i].bin, sizeof(indexs[i].bin));
            context.uniforms[i].num = indexs[i].num;
            context.uniforms[i].index = indexs[i].index;
        }

        status = gcfVX_Kernel(&context);

    }
    else
    {
        status = VX_ERROR_NOT_IMPLEMENTED;
    }

    return status;
}
#endif

#define IMGLST_FIND 0
#define IMGLST_PACK 1
static vx_status packArrays(vx_image img, vx_array src_array, vx_uint32 src_row_items, vx_uint32 src_col_items, vx_array dst_array, vx_size itemsize, vx_size cap)
{
    vx_status status  = VX_SUCCESS;
    gcoVX_Kernel_Context context = {{0}};
    vx_uint32 bin[4];

    /*index = 0*/
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, img, GC_VX_INDEX_AUTO);

    if (src_array && dst_array)
    {
        /*index = 1*/
        gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_ARRAY, src_array, GC_VX_INDEX_AUTO);
        /*index = 2*/
        gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_ARRAY, dst_array, GC_VX_INDEX_AUTO);

        bin[0] = (vx_uint32)itemsize * (vx_uint32)cap;
        gcoOS_MemCopy(&context.uniforms[0].uniform, bin, sizeof(bin));
        context.uniforms[0].index       = 3;
        context.uniforms[0].num         = 16;
        context.uniform_num             = 1;
    }

    context.params.kernel           = gcvVX_KERNEL_IMAGE_LISTER;
    context.params.step             = IMGLST_PACK;
    context.params.xstep            = src_row_items;
    context.params.ystep            = 1;
    context.params.volume           = (vx_uint32)itemsize;
    context.params.clamp            = (vx_uint32)itemsize * src_row_items;
    context.params.col              = src_col_items;

    status = gcfVX_Kernel(&context);

    return status;
}

vx_status createLister(vx_image src, vx_image countImg, vx_array tempArray, vx_int32 width, vx_uint32 height, vx_size itemSize)
{
    vx_status status = vx_true_e;
    gcoVX_Kernel_Context context = {{0}};
    vx_uint32 bin[4];
    vx_uint32 constantData[8] = {0, 8, 16, 24, 0, 0, 0, 0};

    /*index = 0*/
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, src, GC_VX_INDEX_AUTO);
	/*index = 1*/
    gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, countImg, GC_VX_INDEX_AUTO);
    if (tempArray)
    {
        /*index = 2*/
        gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_ARRAY, tempArray, GC_VX_INDEX_AUTO);
    }

    bin[0] = width;
    bin[1] = width-1;
    bin[2] = width-3;
    bin[3] = height-3;
    gcoOS_MemCopy(&context.uniforms[0].uniform, bin, sizeof(bin));
    context.uniforms[0].index       = 3;
    context.uniforms[0].num         = 16;
    gcoOS_MemCopy(&context.uniforms[1].uniform, constantData, sizeof(constantData));
    context.uniforms[1].index       = 4;
    context.uniforms[1].num         = sizeof(constantData) / sizeof(vx_uint32);
    context.uniform_num             = 2;

    context.params.kernel           = gcvVX_KERNEL_IMAGE_LISTER;
    context.params.step             = IMGLST_FIND;
    context.params.xstep            = width;
    context.params.ystep            = 1;
    context.params.volume           = (vx_uint32)itemSize;
    context.params.clamp            = (vx_uint32)itemSize * width;

    status = gcfVX_Kernel(&context);

    return status;
}

vx_status vxImageLister(vx_image src, vx_array arrays, vx_scalar num, vx_reference* staging)
{
    vx_status status = vx_true_e;
    vx_uint32 numCorners  = 0;
    void *base = NULL;
    vx_size cap = 0;
	vx_size itemSize = 0;
    vx_int32 width, height;
    vx_image countImg;
    vx_array tempArray;

    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));

    countImg = (vx_image)staging[0];
    tempArray = (vx_array)staging[1];
    if (arrays)
    {
        vxTruncateArray(arrays, 0);
        vxQueryArray(arrays, VX_ARRAY_ATTRIBUTE_CAPACITY, &cap, sizeof(cap));
        vxQueryArray(arrays, VX_ARRAY_ATTRIBUTE_ITEMSIZE, &itemSize, sizeof(itemSize));
    }

    status = createLister(src, countImg, tempArray, width, height, itemSize);
    status = packArrays(countImg, tempArray, width, height, arrays, itemSize, cap);

    status = gcfVX_Flush(gcvTRUE);
    base = countImg->memory.logicals[0];
    numCorners = *((vx_uint32*)(base) + height - 1);

    if(arrays)
    {
        if (numCorners > cap)
        {
            arrays->itemCount = cap;
        }
        else
        {
            arrays->itemCount = numCorners;
        }
    }

    if (num)
        status = vxCommitScalarValue(num, &numCorners);
    return status;
}