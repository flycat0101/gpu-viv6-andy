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

vx_status vxPhase(vx_image grad_x, vx_image grad_y, vx_image output)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 width = 0, height = 0;
    gcoVX_Kernel_Context context = {{0}};

    vxQueryImage(grad_x, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
    vxQueryImage(grad_x, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));

    /*index = 0*/
    gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, grad_x, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, grad_y, GC_VX_INDEX_AUTO);

    /*index = 2*/
    gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

    context.params.kernel = gcvVX_KERNEL_PHASE;
    context.params.xstep = 8;

    context.params.xmax = width;
    context.params.ymax = height;

    status = gcfVX_Kernel(&context);

    return status;
}

