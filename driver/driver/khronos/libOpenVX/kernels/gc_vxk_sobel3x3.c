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

vx_status vxSobel3x3(vx_image input, vx_image grad_x, vx_image grad_y, vx_border_mode_t *bordermode)
{
    gcoVX_Kernel_Context context = {{0}};

    /*index = 0*/
    gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, grad_x, GC_VX_INDEX_AUTO);

    /* index = 2 */
    gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, grad_y, GC_VX_INDEX_AUTO);

    context.params.kernel = gcvVX_KERNEL_SOBEL_3x3;
    /* RTL limit. Output bin# <= 6 */
    context.params.xstep = 6;
    context.params.borders = bordermode->mode;

    if(bordermode->mode == VX_BORDER_MODE_CONSTANT)
    {
        vx_uint32 bin[4];

        bin[0] =
        bin[1] =
        bin[2] =
        bin[3] = FORMAT_VALUE(bordermode->constant_value);

        gcoOS_MemCopy(&context.uniforms[0].uniform, bin, sizeof(bin));
        context.uniforms[0].num         = 4 * 4;
        context.uniforms[0].index       = 4;
        context.uniform_num             = 1;
    }

    return gcfVX_Kernel(&context);
}

vx_status vxScharr3x3(vx_image input, vx_image grad_x, vx_image grad_y, vx_border_mode_t *bordermode)
{
    gcoVX_Kernel_Context context = {{0}};
    vx_uint32 bin[4] = {0, 0, 0, 0};
    vx_uint32 width = 0, height = 0;
    vxQueryImage(input, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
    vxQueryImage(input, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));

    /*index = 0*/
    gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, grad_x, GC_VX_INDEX_AUTO);

    /*index = 2*/
    gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, grad_y, GC_VX_INDEX_AUTO);

    context.params.kernel = gcvVX_KERNEL_SCHARR_3x3;

    /* RTL limit. Output bin# <= 6 */
    context.params.xstep = 6;
    context.params.ystep = height;
    context.params.borders = bordermode->mode;

    // c3 : width,  height
    {
        bin[0] = width;
        bin[1] = height;

        gcoOS_MemCopy(&context.uniforms[0].uniform, bin, sizeof(bin));
        context.uniforms[0].num         = 4 * 4;
        context.uniforms[0].index       = 3;
        context.uniform_num             = 1;
    }

    return gcfVX_Kernel(&context);
}

#if VIV_SOBEL_MXN
static vx_int16 ops3_x[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1},
};

static vx_int16 ops3_y[3][3] = {
        { 1, 2, 1},
        { 0, 0, 0},
        {-1,-2,-1},
};

static vx_int16 ops5_x[5][5] = {
        {-1, -2, 0, 2, 1},
        {-4, -8, 0, 8, 4},
        {-6,-12, 0,12, 6},
        {-4, -8, 0, 8, 4},
        {-1, -2, 0, 2, 1},
};

static vx_int16 ops5_y[5][5] = {
        { 1, 4,  6, 4, 1},
        { 2, 8, 12, 8, 2},
        { 0, 0,  0, 0, 0},
        {-2,-8,-12,-8,-2},
        {-1,-4, -6,-4,-1},
};

static vx_int16 ops7_x[7][7] = {
        { -1, -4, -5, 0,    5,   4,   1},
        { -6,-24,-30, 0,   30,  24,   6},
        {-15,-60,-75, 0,   75,  60,  15},
        {-20,-80,-100,0,  100,  80,  20},
        {-15,-60,-75, 0,   75,  60,  15},
        { -6,-24,-30, 0,   30,  24,   6},
        { -1, -4, -5, 0,    5,   4,   1},
};
static vx_int16 ops7_y[7][7] = {
        { 1,  6, 15,  20, 15,  6, 1},
        { 4, 24, 60,  80, 60, 24, 4},
        { 5, 30, 75, 100, 75, 30, 5},
        { 0,  0,  0,   0,  0,  0, 0},
        {-5,-30,-75,-100,-75,-30,-5},
        {-4,-24,-60, -80,-60,-24,-4},
        {-1, -6,-15, -20,-15, -6,-1},
};

vx_status vxSobelMxN(vx_image input, vx_scalar win, vx_image grad_x,
                     vx_image grad_y, vx_border_mode_t *bordermode)
{
    vx_status status = vx_false_e;
    vx_int16 * ops[] = {
                        (vx_int16 *)ops3_x, (vx_int16 *)ops3_y,
                        (vx_int16 *)ops5_x, (vx_int16 *)ops5_y,
                        (vx_int16 *)ops7_x, (vx_int16 *)ops7_y
                    };
    vx_int32    wins = 0;

    status |= vxAccessScalarValue(win, &wins);

    status |= Convolve(input, grad_x, ops[wins - 3], 0, vx_false_e, wins, wins, bordermode);

    status |= Convolve(input, grad_y, ops[wins - 2], 0, vx_false_e, wins, wins, bordermode);

    status |= vxCommitScalarValue(win, &wins);

    return status;
}
#endif