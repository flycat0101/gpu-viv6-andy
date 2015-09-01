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

vx_status vxTableLookup(vx_image src, vx_lut lut, vx_image dst)
{
    vx_status status = VX_SUCCESS;

    gcoVX_Kernel_Context context = {{0}};

	/*index = 0*/
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, src, GC_VX_INDEX_AUTO);

	/*index = 1*/
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_LUT, lut, GC_VX_INDEX_AUTO);

	/*index = 2*/
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, dst, GC_VX_INDEX_AUTO);

    context.params.kernel = gcvVX_KERNEL_TABLE_LOOKUP;
    context.params.xstep = 1;

    status = gcfVX_Kernel(&context);

    return status;
}

