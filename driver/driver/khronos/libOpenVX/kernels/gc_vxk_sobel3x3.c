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

static vx_int16 ops_x[3][3] = {
        {1, 0, -1},
        {2, 0, -2},
        {1, 0, -1},
};

static vx_int16 ops_y[3][3] = {
        { 1, 2, 1},
        { 0, 0, 0},
        {-1,-2,-1},
};

vx_status vxSobel3x3(vx_node node, vx_image input, vx_image grad_x, vx_image grad_y, vx_border_mode_t *bordermode)
{
    vx_status status = VX_SUCCESS;
    gcoVX_Kernel_Context * kernelContext = gcvNULL;
    vx_uint32 height;
    vxQueryImage(input, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));

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
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, grad_x, GC_VX_INDEX_AUTO);

    /* index = 2 */
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, grad_y, GC_VX_INDEX_AUTO);

    kernelContext->params.kernel = gcvVX_KERNEL_SOBEL_3x3;
    /* RTL limit. Output bin# <= 6 */

    kernelContext->params.ystep = height;
    kernelContext->params.col   = height;
#if gcdVX_OPTIMIZER
    kernelContext->borders = bordermode->mode;
#else
    kernelContext->params.borders = bordermode->mode;
#endif

    if (node->base.context->evisNoInst.noFilter)
    {
        kernelContext->params.xstep = 4;
        kernelContext->params.clamp = vx_false_e;
        kernelContext->params.col = 3;
        kernelContext->params.row = 3;
        kernelContext->params.matrix = (vx_int16 *)ops_x;
        kernelContext->params.matrix1 = (vx_int16 *)ops_y;
        kernelContext->params.scale = gcoMATH_Log2(0);
        kernelContext->params.volume = height;
        kernelContext->params.evisNoInst = node->base.context->evisNoInst;
    }
    else
    {
        kernelContext->params.xstep = 6;
    }


    if(bordermode->mode == VX_BORDER_MODE_CONSTANT)
    {
        vx_uint32 bin[4];

        bin[0] =
        bin[1] =
        bin[2] =
        bin[3] = FORMAT_VALUE(bordermode->constant_value);

        gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, bin, sizeof(bin));
        kernelContext->uniforms[0].num         = 4 * 4;
        kernelContext->uniforms[0].index       = 3;
        kernelContext->uniform_num             = 1;
    }

    status = gcfVX_Kernel(kernelContext);

#if gcdVX_OPTIMIZER
    if (!node || !node->kernelContext)
    {
        vxFree(kernelContext);
    }
#endif

    return status;
}

static vx_int16 sch_x[3][3] = {
        {3,  0,  -3},
        {10, 0, -10},
        {3,  0,  -3},
};

static vx_int16 sch_y[3][3] = {
        { 3, 10, 3},
        { 0,  0, 0},
        {-3,-10,-3},
};

vx_status vxScharr3x3(vx_node node, vx_image input, vx_image grad_x, vx_image grad_y)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 bin[4] = {0, 0, 0, 0};
    vx_uint32 width = 0, height = 0;
    gcoVX_Kernel_Context * kernelContext = gcvNULL;
    vx_border_mode_t bordermode = { VX_BORDER_MODE_UNDEFINED, 0 };

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

    vxQueryImage(input, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
    vxQueryImage(input, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));

    /*index = 0*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, grad_x, GC_VX_INDEX_AUTO);

    /*index = 2*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, grad_y, GC_VX_INDEX_AUTO);

    kernelContext->params.kernel = gcvVX_KERNEL_SCHARR_3x3;

    kernelContext->params.ystep = height;
#if gcdVX_OPTIMIZER
    kernelContext->borders = bordermode.mode;
#else
    kernelContext->params.borders = bordermode.mode;
#endif

    /* RTL limit. Output bin# <= 6 */
    if (node->base.context->evisNoInst.noIAdd || node->base.context->evisNoInst.noFilter)
    {
        kernelContext->params.xstep = 4;
        kernelContext->params.clamp = vx_false_e;
        kernelContext->params.col = 3;
        kernelContext->params.row = 3;
        kernelContext->params.matrix = (vx_int16 *)sch_x;
        kernelContext->params.matrix1 = (vx_int16 *)sch_y;
        kernelContext->params.scale = gcoMATH_Log2(0);
        kernelContext->params.volume = height;
        kernelContext->params.evisNoInst = node->base.context->evisNoInst;
    }
    else
    {
        kernelContext->params.xstep = 6;
    }

    // c3 : width,  height
    {
        bin[0] = width;
        bin[1] = height;

        gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, bin, sizeof(bin));
        kernelContext->uniforms[0].num         = 4 * 4;
        kernelContext->uniforms[0].index       = 3;
        kernelContext->uniform_num             = 1;
    }

    status = gcfVX_Kernel(kernelContext);

#if gcdVX_OPTIMIZER
    if (!node || !node->kernelContext)
    {
        vxFree(kernelContext);
    }
#endif

    return status;
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

vx_status vxSobelMxN(vx_node node, vx_image input, vx_scalar win, vx_image grad_x,
                     vx_image grad_y, vx_border_mode_t *bordermode)
{
    gcoVX_Kernel_Context * kernelContext = gcvNULL;
    vx_status status = VX_SUCCESS;
    vx_int16 * ops[] = {
                        (vx_int16 *)ops3_x, (vx_int16 *)ops3_y,
                        (vx_int16 *)ops5_x, (vx_int16 *)ops5_y,
                        (vx_int16 *)ops7_x, (vx_int16 *)ops7_y
                    };
    vx_int32    wins = 0;
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

    vxQueryImage(input, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));

    status = vxAccessScalarValue(win, &wins);

    /*index = 0*/
	gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

	/*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, grad_x, GC_VX_INDEX_AUTO);

    /*index = 2*/
	gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, grad_y, GC_VX_INDEX_AUTO);

    kernelContext->params.kernel = gcvVX_KERNEL_SOBEL_MxN;

    if (wins == 7)
        kernelContext->params.xstep = 2;
    else
        kernelContext->params.xstep = 4;
    kernelContext->params.ystep = height;

    kernelContext->params.clamp = vx_false_e;
    kernelContext->params.col = wins;
    kernelContext->params.row = wins;
    kernelContext->params.matrix = ops[wins - 3];
    kernelContext->params.matrix1 = ops[wins - 2];
    kernelContext->params.scale = gcoMATH_Log2(0);
    kernelContext->params.borders = bordermode->mode;
    kernelContext->params.volume = height;

    if (bordermode->mode == VX_BORDER_MODE_CONSTANT || bordermode->mode == VX_BORDER_MODE_UNDEFINED)
    {
        vx_uint32 bin[4];

        bin[0] =
        bin[1] =
        bin[2] =
        bin[3] = FORMAT_VALUE((bordermode->mode == VX_BORDER_MODE_UNDEFINED)?0xcd:bordermode->constant_value);

        gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, bin, sizeof(bin));
        kernelContext->uniforms[0].num = 4 * 4;
        kernelContext->uniforms[0].index = 3;
        kernelContext->uniform_num = 1;
    }

    kernelContext->params.evisNoInst = node->base.context->evisNoInst;

    status = gcfVX_Kernel(kernelContext);

    status = vxCommitScalarValue(win, &wins);
#if gcdVX_OPTIMIZER
    if (!node || !node->kernelContext)
    {
        vxFree(kernelContext);
    }
#endif
    return status;
}
#endif