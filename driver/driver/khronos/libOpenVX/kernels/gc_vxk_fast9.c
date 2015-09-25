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

#define F9C_STRENGTH 0
#define F9C_NONMAX   1

vx_status vxViv_Fast9Corners_Strength(vx_image src, vx_uint8 t, vx_image output, vx_bool do_nonmax)
{
    gcoVX_Kernel_Context context = {{0}};
    vx_status status                    = vx_true_e;
    vx_uint32                         i = 0, height = 0;
    gcoVX_Index indexs[]                = {
        /* index,  num,             shift0,         shift1,      mask0,    mask1 */
        {    3,   4 * 4, {(vx_uint32)FV4(2*8,3*8,4*8,(2+16)*8),     (vx_uint32)FV4((3+16)*8,(4+16)*8,0, 0),     (vx_uint32)FV4(8,8,8,8),    (vx_uint32)FV4(8,8,0,0)}  }, /* 16, 1, 2, 10, 9, 8 */
        {    4,   4 * 4, {(vx_uint32)FV4(8,5*8,(1+16)*8,(5+16)*8),  (vx_uint32)FV4(0,0,0,0),                    (vx_uint32)FV4(8,8,8,8),    (vx_uint32)(vx_uint32)FV4(0,0,0,0)}  }, /* 15,3,11,7 */
        {    5,   4 * 4, {(vx_uint32)FV4(0,6*8,16*8,(6+16)*8),      (vx_uint32)FV4(0,6*8,0,0),                  (vx_uint32)FV4(8,8,8,8),    (vx_uint32)FV4(8,8,0,0)}  }, /* 14,4,12,6  -- 13,5 */
        {    6,   4 * 4, {           FV(3 * 8),                                FV(3 * 8),                                  FV(8),                      FV(8)       }  }, /* p */

        {    7,   4 * 4, {(vx_uint32)FV4(3*8,4*8,5*8,(3+16)*8),     (vx_uint32)FV4((4+16)*8,(5+16)*8,0, 0), (vx_uint32)FV4(8,8,8,8),    (vx_uint32)FV4(8,8,0,0)}  }, /* 16, 1, 2, 10, 9, 8 */
        {    8,   4 * 4, {(vx_uint32)FV4(2*8,6*8,(2+16)*8,(6+16)*8),(vx_uint32)FV4(0,0,0,0),                (vx_uint32)FV4(8,8,8,8),    (vx_uint32)FV4(0,0,0,0)}  }, /* 15,3,11,7 */
        {    9,   4 * 4, {(vx_uint32)FV4(8,7*8,(16+1)*8,(7+16)*8),  (vx_uint32)FV4(8,7*8,0,0),              (vx_uint32)FV4(8,8,8,8),    (vx_uint32)FV4(8,8,0,0)}  }, /* 14,4,12,6  -- 13,5 */
        {   10,   4 * 4, {(vx_uint32)FV(4 * 8),                     (vx_uint32)FV(4 * 8),                   (vx_uint32)FV(8),           (vx_uint32)FV(8)       }  }, /* p */

        {   11,   4 * 4, {(vx_uint32)FV4(4*8,5*8,6*8,(4+16)*8),     (vx_uint32)FV4((5+16)*8,(6+16)*8,0, 0), (vx_uint32)FV4(8,8,8,8),    (vx_uint32)FV4(8,8,0,0)}  }, /* 16, 1, 2, 10, 9, 8 */
        {   12,   4 * 4, {(vx_uint32)FV4(3*8,7*8,(3+16)*8,(7+16)*8),(vx_uint32)FV4(0,0,0,0),                (vx_uint32)FV4(8,8,8,8),    (vx_uint32)FV4(0,0,0,0)}  }, /* 15,3,11,7 */
        {   13,   4 * 4, {(vx_uint32)FV4(2*8,8*8,(16+2)*8,(8+16)*8),(vx_uint32)FV4(2*8,8*8,0,0),            (vx_uint32)FV4(8,8,8,8),    (vx_uint32)FV4(8,8,0,0)}  }, /* 14,4,12,6  -- 13,5 */
        {   14,   4 * 4, {           FV(5 * 8),                                FV(5 * 8),                              FV(8),                      FV(8)       }  }, /* p */

        {   15,   4 * 4, {(vx_uint32)FV4(5*8,6*8,7*8,(5+16)*8),     (vx_uint32)FV4((6+16)*8,(7+16)*8,0, 0), (vx_uint32)FV4(8,8,8,8),    (vx_uint32)FV4(8,8,0,0)}  }, /* 16, 1, 2, 10, 9, 8 */
        {   16,   4 * 4, {(vx_uint32)FV4(4*8,8*8,(4+16)*8,(8+16)*8),(vx_uint32)FV4(0,0,0,0),                (vx_uint32)FV4(8,8,8,8),    (vx_uint32)FV4(0,0,0,0)}  }, /* 15,3,11,7 */
        {   17,   4 * 4, {(vx_uint32)FV4(3*8,9*8,(16+3)*8,(9+16)*8),(vx_uint32)FV4(3*8,9*8,0,0),            (vx_uint32)FV4(8,8,8,8),    (vx_uint32)FV4(8,8,0,0)}  }, /* 14,4,12,6  -- 13,5 */
        {   18,   4 * 4, {           FV(6 * 8),                                FV(6 * 8),                              FV(8),                      FV(8)       }  }, /* p */

        {   19,   4 * 4, {(vx_uint32)FV4(6*8,7*8,8*8,(6+16)*8),       (vx_uint32)FV4((7+16)*8,(8+16)*8,0, 0),       (vx_uint32)FV4(8,8,8,8),    (vx_uint32)FV4(8,8,0,0)}  }, /* 16, 1, 2, 10, 9, 8 */
        {   20,   4 * 4, {(vx_uint32)FV4(5*8,9*8,(5+16)*8,(9+16)*8),  (vx_uint32)FV4(0,0,0,0),                      (vx_uint32)FV4(8,8,8,8),    (vx_uint32)FV4(0,0,0,0)}  }, /* 15,3,11,7 */
        {   21,   4 * 4, {(vx_uint32)FV4(4*8,10*8,(16+4)*8,(10+16)*8),(vx_uint32)FV4(4*8,10*8,0,0),                 (vx_uint32)FV4(8,8,8,8),    (vx_uint32)FV4(8,8,0,0)}  }, /* 14,4,12,6  -- 13,5 */
        {   22,   4 * 4, {           FV(7 * 8),                                  FV(7 * 8),                                    FV(8),                      FV(8)       }  }, /* p */

        {   23,   4 * 4, {(vx_uint32)FV4(7*8,8*8,9*8,(7+16)*8),         (vx_uint32)FV4((8+16)*8,(9+16)*8,0, 0),     (vx_uint32)FV4(8,8,8,8),    (vx_uint32)FV4(8,8,0,0)}  }, /* 16, 1, 2, 10, 9, 8 */
        {   24,   4 * 4, {(vx_uint32)FV4(6*8,10*8,(6+16)*8,(10+16)*8),  (vx_uint32)FV4(0,0,0,0),                    (vx_uint32)FV4(8,8,8,8),    (vx_uint32)FV4(0,0,0,0)}  }, /* 15,3,11,7 */
        {   25,   4 * 4, {(vx_uint32)FV4(5*8,11*8,(16+5)*8,(11+16)*8),  (vx_uint32)FV4(5*8,11*8,0,0),               (vx_uint32)FV4(8,8,8,8),    (vx_uint32)FV4(8,8,0,0)}  }, /* 14,4,12,6  -- 13,5 */
        {   26,   4 * 4, {           FV(8 * 8),                                    FV(8 * 8),                                  FV(8),                      FV(8)       }  }, /* p */

        {   27,   4 * 4, {(vx_uint32)FV4(8*8,9*8,10*8,(8+16)*8),        (vx_uint32)FV4((9+16)*8,(10+16)*8,0, 0),        (vx_uint32)FV4(8,8,8,8),    (vx_uint32)FV4(8,8,0,0)}  }, /* 16, 1, 2, 10, 9, 8 */
        {   28,   4 * 4, {(vx_uint32)FV4(7*8,11*8,(7+16)*8,(11+16)*8),  (vx_uint32)FV4(0,0,0,0),                        (vx_uint32)FV4(8,8,8,8),    (vx_uint32)FV4(0,0,0,0)}  }, /* 15,3,11,7 */
        {   29,   4 * 4, {(vx_uint32)FV4(6*8,12*8,(16+6)*8,(12+16)*8),  (vx_uint32)FV4(6*8,12*8,0,0),                   (vx_uint32)FV4(8,8,8,8),    (vx_uint32)FV4(8,8,0,0)}  }, /* 14,4,12,6  -- 13,5 */
        {   30,   4 * 4, {           FV(9 * 8),                                    FV(9 * 8),                                      FV(8),                      FV(8)       }  }, /* p */

        {   31,   4 * 4, {(vx_uint32)FV4(9*8,10*8,11*8,(9+16)*8),       (vx_uint32)FV4((10+16)*8,(11+16)*8,0, 0),       (vx_uint32)FV4(8,8,8,8),    (vx_uint32)FV4(8,8,0,0)}  }, /* 16, 1, 2, 10, 9, 8 */
        {   32,   4 * 4, {(vx_uint32)FV4(8*8,12*8,(8+16)*8,(12+16)*8),  (vx_uint32)FV4(0,0,0,0),                        (vx_uint32)FV4(8,8,8,8),    (vx_uint32)FV4(0,0,0,0)}  }, /* 15,3,11,7 */
        {   33,   4 * 4, {(vx_uint32)FV4(7*8,13*8,(16+7)*8,(13+16)*8),  (vx_uint32)FV4(7*8,13*8,0,0),                   (vx_uint32)FV4(8,8,8,8),    (vx_uint32)FV4(8,8,0,0)}  }, /* 14,4,12,6  -- 13,5 */
        {   34,   4 * 4, {           FV(10 * 8),                                   FV(10 * 8),                                     FV(8),                      FV(8)       }  }, /* p */

        {   35,   4 * 4, {        FV2(t),        FV2(255),       FV2(t),           0    }  }, /* a, b, c, 0 */
        {   36,   4 * 4, {        FV2(1),          FV2(1),            0,      FV2(1)    }  }, /* constant */
        {   37,   4 * 4, {  FV4(0,1,2,3),    FV4(4,5,6,7), FV4(9,9,9,9), FV4(9,9,9,9)   }  }, /*  */
        {   38,   4 * 4, {FV4(8,9,10,11),FV4(12,13,14,15), FV4(9,9,9,9), FV4(9,9,9,9)   }  }, /*  */
        {   39,   4 * 4, {  0x01ff01ff,        0x01ff01ff,   0x01ff01ff,   0x01ff01ff   }  }, /*  */
        /* 40 - 41 sort */
        {   40,   4 * 4, {(vx_uint32)FV4(16,2*16,(8+1)*16,(8+5)*16),    (vx_uint32)FV4(7*16,(8+7)*16,(8+3)*16, 5*16),   (vx_uint32)FV4(16,16,16,16),(vx_uint32)FV4(16,16,16,16)}  }, /* 1, 2, 3, 4, 5, 6, 7, 8 */
        {   41,   4 * 4, {(vx_uint32)FV4(4*16,3*16,(8+2)*16,(8+6)*16),  (vx_uint32)FV4(6*16,(8+4)*16,8*16, 0),          (vx_uint32)FV4(16,16,16,16),(vx_uint32)FV4(16,16,16,16)}  }, /* 1, 2, 3, 4, 5, 6, 7, 8 */
        {   42,   4 * 4, {(vx_uint32)FV4(3*8,7*8,11*8, 15*8),   (vx_uint32)FV4(0, 0, 0, 0),         (vx_uint32)FV4(8*4,8*4,8*4,8*4),FV4(0, 0, 0, 0)}  }, /* 4, 5, 6, 7, 8, 9, a, b, c, d, e, f, 0, 1, 2, 3 */
        {   43,   4 * 4, {(vx_uint32)FV4(6*8,10*8,14*8, 18*8),  (vx_uint32)FV4(0, 0, 0, 0),         (vx_uint32)FV4(8*4,8*4,8*4,8*4),FV4(0, 0, 0, 0)}  }, /* 7, 8, 9, a, b, c, d, e, f, 0, 1, 2, 3, 4, 5, 6 */
        {   44,   4 * 4, {(vx_uint32)FV4(8*8,12*8,0, 4*8),      (vx_uint32)FV4(0, 0, 0, 0),         (vx_uint32)FV4(8*4,8*4,8*4,8*4),FV4(0, 0, 0, 0)}  }, /* 7, 8, 9, a, b, c, d, e, f, 0, 1, 2, 3, 4, 5, 6 */
    };

    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(vx_uint32));

    /*index = 0*/
    gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, src, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

    context.params.step                 = F9C_STRENGTH;
    context.params.kernel               = gcvVX_KERNEL_FAST_CORNERS;
    context.params.borders              = VX_BORDER_MODE_UNDEFINED;
    context.params.row                  = t;
    context.params.xstep                = 8;
    context.params.ystep                = height;
    context.params.volume               = height; /* save height to check border */
    context.uniform_num                 = sizeof(indexs)/sizeof(indexs[0]);
    context.params.constant_value       = do_nonmax;

    for(i = 0; i < context.uniform_num; i++)
    {
        gcoOS_MemCopy(&context.uniforms[i].uniform, indexs[i].bin, sizeof(indexs[i].bin));
        context.uniforms[i].num = indexs[i].num;
        context.uniforms[i].index = indexs[i].index;
    }

    {
        vx_uint32 bin[16];

        /* DP16x1 */
        bin[0] = 0x55555555;
        bin[1] = 0x55550000;
        bin[2] = 0x76543210;
        bin[3] = 0x76543210;

        bin[4] = 0xaaaaaaaa;
        bin[5] = 0x76543210;
        bin[6] = 0xfedcba98;
        bin[7] = 0x00006600;

        bin[8] =  0x00020001;
        bin[9] =  0x00080004;
        bin[10] = 0x00200010;
        bin[11] = 0x00800040;

        bin[12] = 0x02000100;
        bin[13] = 0x08000400;
        bin[14] = 0x20001000;
        bin[15] = 0x80004000;

        gcoOS_MemCopy(&context.uniforms[context.uniform_num].uniform, bin, sizeof(bin));
        context.uniforms[context.uniform_num].num         = 16 * 4;
        context.uniforms[context.uniform_num].index       = 45;

        context.uniform_num ++;
    }

    status = gcfVX_Kernel(&context);

    return status;
}

vx_status vxViv_Fast9Corners_NonMax(vx_image src, vx_image output, vx_uint8 t, vx_bool do_nonmax)
{
    gcoVX_Kernel_Context context = {{0}};
    vx_status status = vx_true_e;
    vx_uint32 i = 0;
    gcoVX_Index indexs[] = {
        /* index,  num,             shift0,         shift1,      mask0,       mask1 */
        {    3,   4 * 4, {       FV4(8,8,8,8),         0,     FV4(8,8,8,8),     0       }  }, /* pixel p(x,y) */
        {    4,   4 * 4, {       FV4(0,0,0,0),         0,     FV4(8,0,0,0),     0       }  }, /* pixel p(x-1,y) */
        {    5,   4 * 4, {     FV4(2*8,0,0,0),         0,     FV4(8,0,0,0),     0       }  }, /* pixel p(x+1,y) */
        {    6,   4 * 4, {              FV(1),     FV(1),            FV(1), FV(1)       }  }, /* pixel p(x+1,y) */
        {    7,   4 * 4, {     FV4(0,4*8,0,0),         0, FV4(2*8,2*8,0,0),     0       }  }, /* u32->u16 */
    };

    /*index = 0*/
    gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, src, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

    context.params.step     = F9C_NONMAX;
    context.params.kernel     = gcvVX_KERNEL_FAST_CORNERS;
    context.params.borders    = VX_BORDER_MODE_UNDEFINED;
    context.params.row        = t;
    context.params.col        = do_nonmax;
    context.params.xstep      = 1;
    context.uniform_num       = sizeof(indexs)/sizeof(indexs[0]);

    for(i = 0; i < context.uniform_num; i++)
    {
        gcoOS_MemCopy(&context.uniforms[i].uniform, indexs[i].bin, sizeof(indexs[i].bin));
        context.uniforms[i].num = indexs[i].num;
        context.uniforms[i].index = indexs[i].index;
    }

    status = gcfVX_Kernel(&context);

    return status;
}

#define IMGLST_FIND 0
#define IMGLST_PACK 1
static vx_status packFast9(vx_image src, vx_image countImg, vx_array src_array, vx_array dst_array, vx_scalar num_scalar)
{
    vx_status status  = VX_SUCCESS;
    gcoVX_Kernel_Context context = {{0}};
    void *count_base = NULL;
    vx_size cap = 0, itemsize = 0;
    vx_uint32 width, height, numCorners;
    vx_uint32 bin[4];

    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
    if(dst_array)
    {
        vxTruncateArray(dst_array, 0);
        vxQueryArray(dst_array, VX_ARRAY_ATTRIBUTE_ITEMSIZE, &itemsize, sizeof(itemsize));
        vxQueryArray(dst_array, VX_ARRAY_ATTRIBUTE_CAPACITY, &cap, sizeof(cap));
    }

    /*index = 0*/
    gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, countImg, GC_VX_INDEX_AUTO);

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
    context.params.xstep            = width;
    context.params.ystep            = 1;
    context.params.volume           = (vx_uint32)itemsize;
    context.params.clamp            = (vx_uint32)itemsize * width;
    context.params.col              = height;

    status = gcfVX_Kernel(&context);

    status = gcfVX_Flush(gcvTRUE);
    count_base = countImg->memory.logicals[0];
    numCorners = *((vx_uint32*)(count_base) + height - 1);

    if(dst_array)
    {
        if (numCorners > cap)
        {
            dst_array->itemCount = cap;
        }
        else
        {
            dst_array->itemCount = numCorners;
        }
    }

    if (num_scalar)
        status = vxCommitScalarValue(num_scalar, &numCorners);

    return status;
}

static vx_status _imageLister(vx_image src, vx_array points, vx_image countImg ,vx_array tempArray)
{
    vx_status status = vx_true_e;
    gcoVX_Kernel_Context context = {{0}};
    vx_size   itemSize = 0;
    vx_int32 width, height;
    vx_uint32 bin[4];
    vx_uint32 constantData[8] = {0, 8, 16, 24, 0, 0, 0, 0};

    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
    if (points)
    {
        status = vxTruncateArray(points, 0);
        status = vxQueryArray(points, VX_ARRAY_ATTRIBUTE_ITEMSIZE, &itemSize, sizeof(itemSize));
    }

    /*index = 0*/
    gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, src, GC_VX_INDEX_AUTO);

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


vx_status vxFast9Corners(vx_image src, vx_scalar sens, vx_scalar nonm, vx_array points,
                         vx_scalar s_num_corners, vx_border_mode_t *bordermode, vx_reference* staging)
{
    vx_float32 b = 0.0f;
    vx_uint8 tolerance = 0;
    vx_bool do_nonmax;
    vx_rectangle_t rect;

    vx_status status = vxAccessScalarValue(sens, &b);
    status |= vxAccessScalarValue(nonm, &do_nonmax);
    /* remove any pre-existing points */
    status |= vxTruncateArray(points, 0);
    tolerance = (vx_uint8)b;
    status |= vxGetValidRegionImage(src, &rect);

    if (status == VX_SUCCESS)
    {
        /*! \todo implement other Fast9 Corners border modes */
        if (bordermode->mode == VX_BORDER_MODE_UNDEFINED)
        {
            vx_bool s_num_corners_b = vx_false_e;
            vx_image output = (vx_image)staging[0];
            vx_image output2 = (vx_image)staging[1];
            vx_image countImg = (vx_image)staging[2];
            vx_array tempArray = (vx_array)staging[3];

            if(!s_num_corners)
                s_num_corners_b = vx_true_e;

            if(s_num_corners_b)
                s_num_corners = vxCreateScalar(vxGetContext((vx_reference)src), VX_TYPE_UINT32, 0);

            status |= vxViv_Fast9Corners_Strength(src, tolerance, output, do_nonmax);

            if(do_nonmax)status |= vxViv_Fast9Corners_NonMax(output, output2, tolerance, do_nonmax);

            status |= _imageLister((do_nonmax)?output2:output, points, countImg, tempArray);
            status |= packFast9((do_nonmax)?output2:output, countImg, tempArray, points, s_num_corners);

            if(s_num_corners_b)
                status |= vxReleaseScalar(&s_num_corners);

        }
        else
        {
            status = VX_ERROR_NOT_IMPLEMENTED;
        }
    }

    status |= vxCommitScalarValue(nonm, &do_nonmax);
    status |= vxCommitScalarValue(sens, &b);

    return status;
}

