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

#define INTEGRAL_IMAGE_P0 0
#define INTEGRAL_IMAGE_P1 1

vx_status vxIntegralImage(vx_image src, vx_image dst)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 width = 0, height = 0;
    gcoVX_Kernel_Context gcContext = {{0}};
    vx_uint32 constantData[8] = {0, 0x8, 0x10, 0x18, 0, 0, 0, 0};

    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));

    constantData[4] = width;
    constantData[5] = height;
    constantData[6] = 0;
    constantData[7] = 1;


    /************* integral step 0 *************/
    /*index = 0*/
    gcoVX_AddObject(&gcContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, src, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(&gcContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, dst, GC_VX_INDEX_AUTO);

    gcoOS_MemCopy(&gcContext.uniforms[0].uniform, constantData, sizeof(constantData));

    gcContext.uniforms[0].index = 2;
    gcContext.uniforms[0].num = sizeof(constantData) / sizeof(vx_uint32);
    gcContext.uniform_num = 1;

    gcContext.params.step = INTEGRAL_IMAGE_P0;
    gcContext.params.kernel = gcvVX_KERNEL_INTEGRAL_IMAGE;
    gcContext.params.xstep = 4;
    gcContext.params.ystep = height;

    status = gcfVX_Kernel(&gcContext);

    /************* integral step 1 *************/
    memset(&gcContext, 0, sizeof(gcoVX_Kernel_Context));

    /*index = 0*/
    gcoVX_AddObject(&gcContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, dst, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(&gcContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, dst, GC_VX_INDEX_AUTO);

    gcContext.params.step = INTEGRAL_IMAGE_P1;
    gcContext.params.kernel = gcvVX_KERNEL_INTEGRAL_IMAGE;
    gcContext.params.xstep = width;
    gcContext.params.ystep = 2;

    status = gcfVX_Kernel(&gcContext);

    return status;
}

