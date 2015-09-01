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

vx_status _gcfVX_AddSubOpration(gceVX_KERNEL kernel, vx_image in0, vx_image in1, vx_scalar policy_param, vx_image output)
{
    vx_enum overflow_policy = -1;
    gcoVX_Kernel_Context context = {{0}};
    vxAccessScalarValue(policy_param, &overflow_policy);

	/*index = 0*/
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, in0, GC_VX_INDEX_AUTO);

	/*index = 1*/
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, in1, GC_VX_INDEX_AUTO);

	/*index = 2*/
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

    context.params.kernel           = kernel;
    context.params.xstep            = 8;

    context.params.policy           = (overflow_policy == VX_CONVERT_POLICY_SATURATE)?gcvTRUE:gcvFALSE;

	vxCommitScalarValue(policy_param, &overflow_policy);

    return gcfVX_Kernel(&context);
}

vx_status vxAddition(vx_image in0, vx_image in1, vx_scalar policy_param, vx_image output)
{
    return _gcfVX_AddSubOpration(gcvVX_KERNEL_ADD, in0, in1, policy_param, output);
}

/* nodeless version of the Subtraction kernel*/
vx_status vxSubtraction(vx_image in0, vx_image in1, vx_scalar policy_param, vx_image output)
{
    return _gcfVX_AddSubOpration(gcvVX_KERNEL_SUBTRACT, in0, in1, policy_param, output);
}

