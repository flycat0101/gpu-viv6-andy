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

vx_status _gcfVX_BitwiseOpration(gceVX_KERNEL kernel, vx_image in0, vx_image in1, vx_image output)
{
    gcoVX_Kernel_Context context = {{0}};

	/*index = 0*/
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, in0, GC_VX_INDEX_AUTO);

	/*index = 1*/
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, in1, GC_VX_INDEX_AUTO);

	/*index = 2*/
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

    context.params.kernel = kernel;
    context.params.xstep = 16;

    return gcfVX_Kernel(&context);
}


vx_status vxAnd(vx_image in1, vx_image in2, vx_image output)
{
    return _gcfVX_BitwiseOpration(gcvVX_KERNEL_AND, in1, in2, output);
}

vx_status vxOr(vx_image in1, vx_image in2, vx_image output)
{
     return _gcfVX_BitwiseOpration(gcvVX_KERNEL_OR, in1, in2, output);
}

vx_status vxXor(vx_image in1, vx_image in2, vx_image output)
{
     return _gcfVX_BitwiseOpration(gcvVX_KERNEL_XOR, in1, in2, output);
}

vx_status vxNot(vx_image input, vx_image output)
{
    gcoVX_Kernel_Context context = {{0}};

	/*index = 0*/
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

	/*index = 1*/
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

    context.params.kernel = gcvVX_KERNEL_NOT;
    context.params.xstep = 16;

    return gcfVX_Kernel(&context);
}

