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

#include <VX/vx_types.h>
#include <stdio.h>


vx_status vxConvertDepth(vx_image input, vx_image output, vx_scalar spol, vx_scalar sshf)
{
    vx_status status = VX_SUCCESS;
	gcoVX_Kernel_Context context = {{0}};
	vx_enum policy = 0;
    vx_int32 shift = 0;
    vx_df_image inputFormat, outputFormat;
    vx_uint32 bin[4];

    vxQueryImage(input, VX_IMAGE_ATTRIBUTE_FORMAT, &inputFormat, sizeof(inputFormat));
    vxQueryImage(output, VX_IMAGE_ATTRIBUTE_FORMAT, &outputFormat, sizeof(outputFormat));

	status = vxAccessScalarValue(spol, &policy);
    status = vxAccessScalarValue(sshf, &shift);

	/*index = 0*/
	gcoVX_AddObject(&context,GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

	/*index = 1*/
	gcoVX_AddObject(&context,GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

	context.params.kernel		= gcvVX_KERNEL_CONVERTDEPTH;
	context.params.policy		= (policy == VX_CONVERT_POLICY_SATURATE)?gcvTRUE:gcvFALSE;
    context.params.volume       = shift;

    if (outputFormat == VX_DF_IMAGE_S32 || outputFormat == VX_DF_IMAGE_U32)
    {
        context.params.xstep = 4;
        bin[0] = shift;
    }
    else if (outputFormat == VX_DF_IMAGE_S16 || outputFormat == VX_DF_IMAGE_U16)
    {
        context.params.xstep = 8;
        bin[0] = FV2(shift);
    }
    else
    {
        context.params.xstep = 8;
        bin[0] = FV(shift);
        bin[1] = FV2(1);
    }
    gcoOS_MemCopy(&context.uniforms[0].uniform, bin, sizeof(bin));
    context.uniforms[0].num = 4 * 4;
    context.uniforms[0].index = 2;
    context.uniform_num = 1;

	status = gcfVX_Kernel(&context);

	status = vxCommitScalarValue(spol, &policy);
	status = vxCommitScalarValue(sshf, &shift);

	return status;
}

