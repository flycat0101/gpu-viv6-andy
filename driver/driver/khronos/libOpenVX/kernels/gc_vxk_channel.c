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

vx_status vxChannelCombine(vx_image inputs[4], vx_image output)
{
	vx_status status = VX_SUCCESS;
	gcoVX_Kernel_Context context = {{0}};
	vx_df_image format = 0;
    vx_uint32 constantData[8] = {0, 8, 16, 24, 0, 0, 0, 0};
    vx_uint32 width;
    vx_uint32 height;
    vxQueryImage(output, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
    vxQueryImage(output, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
    vxQueryImage(output, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));

	switch (format)
	{
	    case VX_DF_IMAGE_RGB:
			/*index = 0*/
			gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[0], GC_VX_INDEX_AUTO);

			/*index = 1*/
			gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[1], GC_VX_INDEX_AUTO);

			/*index = 2*/
			gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[2], GC_VX_INDEX_AUTO);

			/*index = 3*/
			gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

			gcoOS_MemCopy(&context.uniforms[0].uniform, constantData, sizeof(constantData));
	        context.uniforms[0].index = 4;
            context.uniforms[0].num = sizeof(constantData) / sizeof(vx_uint32);
            context.uniform_num = 1;
			context.params.outputFormat = gcvSURF_R8G8B8;
            context.params.xstep = 4;
            break;
        case VX_DF_IMAGE_RGBX:
			/*index = 0*/
			gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[0], GC_VX_INDEX_AUTO);

			/*index = 1*/
			gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[1], GC_VX_INDEX_AUTO);

			/*index = 2*/
			gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[2], GC_VX_INDEX_AUTO);

			/*index = 3*/
			gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[3], GC_VX_INDEX_AUTO);

			/*index = 4*/
			gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

			gcoOS_MemCopy(&context.uniforms[0].uniform, constantData, sizeof(constantData));
	        context.uniforms[0].index = 5;
            context.uniforms[0].num = sizeof(constantData) / sizeof(vx_uint32);
            context.uniform_num = 1;
			context.params.outputFormat = gcvSURF_X8R8G8B8;
            context.params.xstep = 4;
			break;
		case VX_DF_IMAGE_UYVY:
			/*index = 0*/
			gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[0], GC_VX_INDEX_AUTO);

			/*index = 1*/
			gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[1], GC_VX_INDEX_AUTO);

			/*index = 2*/
			gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[2], GC_VX_INDEX_AUTO);

			/*index = 3*/
			gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

            gcoOS_MemCopy(&context.uniforms[0].uniform, constantData, sizeof(constantData));
	        context.uniforms[0].index = 4;
            context.uniforms[0].num = sizeof(constantData) / sizeof(vx_uint32);
            context.uniform_num = 1;
            context.params.outputFormat = gcvSURF_UYVY;
            context.params.xstep = 8;
			break;
        case VX_DF_IMAGE_YUYV:
			/*index = 0*/
			gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[0], GC_VX_INDEX_AUTO);

			/*index = 1*/
			gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[1], GC_VX_INDEX_AUTO);

			/*index = 2*/
			gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[2], GC_VX_INDEX_AUTO);

			/*index = 3*/
			gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

            gcoOS_MemCopy(&context.uniforms[0].uniform, constantData, sizeof(constantData));
	        context.uniforms[0].index = 4;
            context.uniforms[0].num = sizeof(constantData) / sizeof(vx_uint32);
            context.uniform_num = 1;

            context.params.outputFormat = gcvSURF_YUY2;
            context.params.xstep = 8;
            break;
        case VX_DF_IMAGE_NV12:
			/*index = 0*/
			gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[0], GC_VX_INDEX_AUTO);

			/*index = 1*/
			gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[1], GC_VX_INDEX_AUTO);

			/*index = 2*/
			gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[2], GC_VX_INDEX_AUTO);

			/*index = 3*/
			gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

            gcoOS_MemCopy(&context.uniforms[0].uniform, constantData, sizeof(constantData));
	        context.uniforms[0].index = 5;
            context.uniforms[0].num = sizeof(constantData) / sizeof(vx_uint32);
            context.uniform_num = 1;
            context.params.outputFormat = gcvSURF_NV12;
            context.params.xstep = 16;
            break;
        case VX_DF_IMAGE_NV21:
			/*index = 0*/
			gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[0], GC_VX_INDEX_AUTO);

			/*index = 1*/
			gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[1], GC_VX_INDEX_AUTO);

			/*index = 2*/
			gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[2], GC_VX_INDEX_AUTO);

			/*index = 3*/
			gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

            gcoOS_MemCopy(&context.uniforms[0].uniform, constantData, sizeof(constantData));
	        context.uniforms[0].index = 5;
            context.uniforms[0].num = sizeof(constantData) / sizeof(vx_uint32);
            context.uniform_num = 1;
            context.params.outputFormat = gcvSURF_NV21;
            context.params.xstep = 16;
            break;
        case VX_DF_IMAGE_IYUV:
			/*index = 0*/
			gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[0], GC_VX_INDEX_AUTO);

			/*index = 1*/
			gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[1], GC_VX_INDEX_AUTO);

			/*index = 2*/
			gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[2], GC_VX_INDEX_AUTO);

			/*index = 3*/
			gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

            context.params.outputFormat = gcvSURF_I420;
            context.params.xstep = 16;
            break;
        case VX_DF_IMAGE_YUV4:
			/*index = 0*/
			gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[0], GC_VX_INDEX_AUTO);

			/*index = 1*/
			gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[1], GC_VX_INDEX_AUTO);

			/*index = 2*/
			gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputs[2], GC_VX_INDEX_AUTO);

			/*index = 3*/
			gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

            context.params.outputFormat = gcvSURF_AYUV;
            context.params.xstep = 16;
            break;
	    default:
		    status = VX_ERROR_INVALID_FORMAT;
		    break;
	}
	if (status != VX_SUCCESS)
	    return status;


	context.params.kernel = gcvVX_KERNEL_CHANNEL_COMBINE;
	context.params.borders = VX_BORDER_MODE_CONSTANT;
	context.params.constant_value = 0;
    context.params.row = width;
    context.params.col = height;

	status = gcfVX_Kernel(&context);
	return status;

}

vx_status vxChannelExtract(vx_image src, vx_scalar channel, vx_image dst)
{
	vx_status status = VX_SUCCESS;
	gcoVX_Kernel_Context context = {{0}};
	vx_uint8 constantData[16] = {0, 32, 64, 96, 0, 0, 0, 0, 8, 8, 8, 8, 0, 0, 0, 0};
	vx_df_image format = 0;
    vx_enum chan = -1;
	vx_uint32 width;
    vx_uint32 height;
	vxAccessScalarValue(channel, &chan);
    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));

	if (format == VX_DF_IMAGE_RGB)
	{
        switch (chan)
        {
             case VX_CHANNEL_0:
			    context.params.volume = 0;
                constantData[0] = 0;
                constantData[1] = 24;
                constantData[2] = 48;
                constantData[3] = 72;
                break;
            case VX_CHANNEL_1:
			    context.params.volume = 1;
                constantData[0] = 8;
                constantData[1] = 32;
                constantData[2] = 56;
                constantData[3] = 80;
                break;
            case VX_CHANNEL_2:
                context.params.volume = 2;
                constantData[0] = 16;
                constantData[1] = 40;
                constantData[2] = 64;
                constantData[3] = 88;
                break;
            default:
                return VX_ERROR_INVALID_PARAMETERS;
		}
		context.params.inputFormat = gcvSURF_R8G8B8;
        context.params.inputMultipleWidth = 3;
        context.params.xmax = width * 3;
        context.params.xstep = 12;
    }
	else if (format == VX_DF_IMAGE_RGBX)
	{
        switch (chan)
        {
            case VX_CHANNEL_0:
			    context.params.volume = 0;
                break;
            case VX_CHANNEL_1:
			    context.params.volume = 1;
                break;
            case VX_CHANNEL_2:
                context.params.volume = 2;
                break;
            case VX_CHANNEL_3:
                context.params.volume = 3;
                break;
            default:
                return VX_ERROR_INVALID_PARAMETERS;
		}
        constantData[0] += (vx_uint8)(8*context.params.volume);
        constantData[1] += (vx_uint8)(8*context.params.volume);
        constantData[2] += (vx_uint8)(8*context.params.volume);
        constantData[3] += (vx_uint8)(8*context.params.volume);
		context.params.inputFormat = gcvSURF_X8R8G8B8;
        context.params.xstep = 4;
	}
    else if (format == VX_DF_IMAGE_UYVY)
    {
        switch (chan)
        {
            case VX_CHANNEL_Y:
			    context.params.volume = 0;
                constantData[0] = 8;
                constantData[1] = 24;
                constantData[2] = 40;
                constantData[3] = 56;
                constantData[4] = 72;
                constantData[5] = 88;
                constantData[6] = 104;
                constantData[7] = 120;
                constantData[12] += 8;
                constantData[13] += 8;
                constantData[14] += 8;
                constantData[15] += 8;
                context.params.xstep = 8;
			    break;
            case VX_CHANNEL_U:
			    context.params.volume = 1;
                context.params.xstep = 8;
			    break;
            case VX_CHANNEL_V:
                context.params.volume = 2;
                constantData[0] += 16;
                constantData[1] += 16;
                constantData[2] += 16;
                constantData[3] += 16;
                context.params.xstep = 8;
                break;
            default:
                return VX_ERROR_INVALID_PARAMETERS;
		}
        context.params.inputFormat = gcvSURF_UYVY;
    }
     else if (format == VX_DF_IMAGE_YUYV)
    {
        switch (chan)
        {
            case VX_CHANNEL_Y:
			    context.params.volume = 0;
                constantData[1] = 16;
                constantData[2] = 32;
                constantData[3] = 48;
                constantData[4] = 64;
                constantData[5] = 80;
                constantData[6] = 96;
                constantData[7] = 112;
                constantData[12] += 8;
                constantData[13] += 8;
                constantData[14] += 8;
                constantData[15] += 8;
                context.params.xstep = 8;
			    break;
            case VX_CHANNEL_U:
			    context.params.volume = 1;
                constantData[0] += 8;
                constantData[1] += 8;
                constantData[2] += 8;
                constantData[3] += 8;
                context.params.xstep = 8;
			    break;
            case VX_CHANNEL_V:
                context.params.volume = 2;
                constantData[0] += 24;
                constantData[1] += 24;
                constantData[2] += 24;
                constantData[3] += 24;
                context.params.xstep = 8;
			    break;
            default:
                return VX_ERROR_INVALID_PARAMETERS;
		}
        context.params.inputFormat = gcvSURF_YUY2;
    }
    else if (format == VX_DF_IMAGE_NV12)
    {
        switch (chan)
        {
        case VX_CHANNEL_Y:
            context.params.volume = 0;
            break;
        case VX_CHANNEL_U:
            context.params.volume = 1;
            constantData[1] = 16;
            constantData[2] = 32;
            constantData[3] = 48;
            constantData[4] = 64;
            constantData[5] = 80;
            constantData[6] = 96;
            constantData[7] = 112;
            break;
        case VX_CHANNEL_V:
            context.params.volume = 2;
            constantData[0] = 8;
            constantData[1] = 24;
            constantData[2] = 40;
            constantData[3] = 56;
            constantData[4] = 72;
            constantData[5] = 88;
            constantData[6] = 104;
            constantData[7] = 120;
            break;
        default:
            return VX_ERROR_INVALID_PARAMETERS;
        }
        constantData[12] = 8;
        constantData[13] = 8;
        constantData[14] = 8;
        constantData[15] = 8;
        context.params.xstep = 16;
        context.params.inputFormat = gcvSURF_NV12;
    }
    else if (format == VX_DF_IMAGE_NV21)
    {
        switch (chan)
        {
        case VX_CHANNEL_Y:
            context.params.volume = 0;
            break;
        case VX_CHANNEL_U:
            context.params.volume = 1;
            constantData[0] = 8;
            constantData[1] = 24;
            constantData[2] = 40;
            constantData[3] = 56;
            constantData[4] = 72;
            constantData[5] = 88;
            constantData[6] = 104;
            constantData[7] = 120;
            break;
        case VX_CHANNEL_V:
            context.params.volume = 2;
            constantData[1] = 16;
            constantData[2] = 32;
            constantData[3] = 48;
            constantData[4] = 64;
            constantData[5] = 80;
            constantData[6] = 96;
            constantData[7] = 112;
            break;
        default:
            return VX_ERROR_INVALID_PARAMETERS;
        }
        constantData[12] = 8;
        constantData[13] = 8;
        constantData[14] = 8;
        constantData[15] = 8;
        context.params.xstep = 16;
        context.params.inputFormat = gcvSURF_NV21;
    }
    else if (format == VX_DF_IMAGE_YUV4)
    {
        switch (chan)
        {
        case VX_CHANNEL_Y:
            context.params.volume = 0;
            break;
        case VX_CHANNEL_U:
            context.params.volume = 1;
            break;
        case VX_CHANNEL_V:
            context.params.volume = 2;
            break;
        default:
            return VX_ERROR_INVALID_PARAMETERS;
        }
        context.params.inputFormat = gcvSURF_AYUV;
        context.params.xstep = 16;
    }
    else if (format == VX_DF_IMAGE_IYUV)
    {
        switch (chan)
        {
        case VX_CHANNEL_Y:
            context.params.volume = 0;
            context.params.xstep = 16;
            break;
        case VX_CHANNEL_U:
            context.params.volume = 1;
            context.params.xstep = 8;
            break;
        case VX_CHANNEL_V:
            context.params.volume = 2;
            context.params.xstep = 8;
            break;
        default:
            return VX_ERROR_INVALID_PARAMETERS;
        }
        context.params.inputFormat = gcvSURF_I420;
    }
    else
    {
        return VX_ERROR_INVALID_PARAMETERS;
    }


	/*index = 0*/
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, src, GC_VX_INDEX_AUTO);

	/*index = 3*/
	gcoVX_AddObject(&context, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, dst, 3);

	context.params.kernel = gcvVX_KERNEL_CHANNEL_EXTRACT;

	gcoOS_MemCopy(&context.uniforms[0].uniform, constantData, sizeof(constantData));
	context.uniforms[0].index = 4;
    context.uniforms[0].num = sizeof(constantData) / sizeof(vx_uint8);
    context.uniform_num = 1;

	context.params.borders = VX_BORDER_MODE_CONSTANT;
	context.params.constant_value = 0;
    context.params.row = width;
    context.params.col = height;

	status = gcfVX_Kernel(&context);
	return status;
}
