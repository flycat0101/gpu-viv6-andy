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

vx_status vxConvertColor(vx_image input, vx_image output)
{
    gcoVX_Kernel_Context context = {{0}};

    vx_df_image inputFormat = 0;
    vx_df_image outputFormat = 0;

    vx_uint32 inputWidth = 0;

    vxQueryImage(input, VX_IMAGE_ATTRIBUTE_FORMAT, &inputFormat, sizeof(vx_df_image));

    vxQueryImage(output, VX_IMAGE_ATTRIBUTE_FORMAT, &outputFormat, sizeof(vx_df_image));

    vxQueryImage(input, VX_IMAGE_ATTRIBUTE_WIDTH, &inputWidth, sizeof(vx_uint32));

	if (inputFormat == VX_DF_IMAGE_RGB)
    {
        if (outputFormat == VX_DF_IMAGE_RGBX)
        {
            vx_uint32 constantData0[8] = {0, 8, 16, 24, 0, 0, 0, 0};
            vx_uint8 constantData1[16] = {0, 24, 48, 72, 8, 32, 56, 80, 8, 8, 8, 8, 8, 8, 8, 8}; /*get rg*/
            vx_uint8 constantData2[16] = {16, 40, 64, 88, 0, 0, 0, 0, 8, 8, 8, 8, 0, 0, 0, 0}; /*get b*/


			/*index = 0*/
			gcoVX_AddObject(&context,GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

			/*index = 1*/
			gcoVX_AddObject(&context,GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

            context.params.kernel = gcvVX_KERNEL_COLOR_CONVERT;
            context.params.xstep = 12;
            context.params.inputFormat = gcvSURF_R8G8B8;
            context.params.outputFormat = gcvSURF_R8G8B8X8;

            context.params.inputMultipleWidth = 3;
            context.params.xmax = inputWidth * 3;

            gcoOS_MemCopy(&context.uniforms[0].uniform, constantData0, sizeof(constantData0));
	        context.uniforms[0].index = 4;
            context.uniforms[0].num = sizeof(constantData0) / sizeof(vx_uint32);

            gcoOS_MemCopy(&context.uniforms[1].uniform, constantData1, sizeof(constantData1));
	        context.uniforms[1].index = 5;
            context.uniforms[1].num = sizeof(constantData1) / sizeof(vx_uint8);

            gcoOS_MemCopy(&context.uniforms[2].uniform, constantData2, sizeof(constantData2));
	        context.uniforms[2].index = 6;
            context.uniforms[2].num = sizeof(constantData2) / sizeof(vx_uint8);

            context.uniform_num = 3;
        }
        else if ((outputFormat == VX_DF_IMAGE_YUV4) ||
                 (outputFormat == VX_DF_IMAGE_NV12) ||
                 (outputFormat == VX_DF_IMAGE_IYUV))
        {
            vx_uint32 constantData[8] = {0, 8, 16, 24, 0, 0, 0, 0};
            vx_float32 constantData1[4] = { 0.2126f,  0.7152f,  0.0722f, 0.5f};
            vx_float32 constantData2[4] = {-0.1146f, -0.3854f,   0.5f,    128.5f};
            vx_float32 constantData3[4] = { 0.5f,    -0.4542f, -0.0458f, 128.5f};
            vx_uint8 constantData4[16] = {24, 32, 40, 0, 0, 0, 0, 0, 8, 8, 8, 0, 0, 0, 0, 0};
            vx_uint8 constantData5[16] = {48, 56, 64, 0, 0, 0, 0, 0, 8, 8, 8, 0, 0, 0, 0, 0};
            vx_uint8 constantData6[16] = {72, 80, 88, 0, 0, 0, 0, 0, 8, 8, 8, 0, 0, 0, 0, 0};
            vx_uint8 constantData7[16] = {0, 32, 64, 96, 0, 0, 0, 0, 8, 8, 8, 8, 0, 0, 0, 0};


			/*index = 0*/
			gcoVX_AddObject(&context,GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

			/*index = 1*/
			gcoVX_AddObject(&context,GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

            context.params.kernel = gcvVX_KERNEL_COLOR_CONVERT;
            context.params.xstep = 12;
            context.params.inputMultipleWidth = 3;
            context.params.xmax = inputWidth * 3;
            context.params.inputFormat = gcvSURF_R8G8B8;
            if (outputFormat == VX_DF_IMAGE_YUV4)
            {
                context.params.outputFormat = gcvSURF_AYUV;
            }else if (outputFormat == VX_DF_IMAGE_NV12)
            {
                context.params.ystep = 2;
                context.params.outputFormat = gcvSURF_NV12;
            }else
            {
                context.params.ystep = 2;
                context.params.outputFormat = gcvSURF_I420;
            }

            gcoOS_MemCopy(&context.uniforms[0].uniform, constantData, sizeof(constantData));
	        context.uniforms[0].index = 4;
            context.uniforms[0].num = sizeof(constantData) / sizeof(vx_uint32);

            gcoOS_MemCopy(&context.uniforms[1].uniform, constantData1, sizeof(constantData1));
	        context.uniforms[1].index = 5;
            context.uniforms[1].num = sizeof(constantData1) / sizeof(vx_float32);

            gcoOS_MemCopy(&context.uniforms[2].uniform, constantData2, sizeof(constantData2));
	        context.uniforms[2].index = 6;
            context.uniforms[2].num = sizeof(constantData2) / sizeof(vx_float32);

            gcoOS_MemCopy(&context.uniforms[3].uniform, constantData3, sizeof(constantData3));
	        context.uniforms[3].index = 7;
            context.uniforms[3].num = sizeof(constantData3) / sizeof(vx_float32);

            gcoOS_MemCopy(&context.uniforms[4].uniform, constantData4, sizeof(constantData4));
	        context.uniforms[4].index = 8;
            context.uniforms[4].num = sizeof(constantData4) / sizeof(vx_uint8);

            gcoOS_MemCopy(&context.uniforms[5].uniform, constantData5, sizeof(constantData5));
	        context.uniforms[5].index = 9;
            context.uniforms[5].num = sizeof(constantData5) / sizeof(vx_uint8);

            gcoOS_MemCopy(&context.uniforms[6].uniform, constantData6, sizeof(constantData6));
	        context.uniforms[6].index = 10;
            context.uniforms[6].num = sizeof(constantData6) / sizeof(vx_uint8);

            gcoOS_MemCopy(&context.uniforms[7].uniform, constantData7, sizeof(constantData7));
	        context.uniforms[7].index = 11;
            context.uniforms[7].num = sizeof(constantData7) / sizeof(vx_uint8);

            context.uniform_num = 8;
        }
    }

    if (inputFormat == VX_DF_IMAGE_RGBX)
    {
        if (outputFormat == VX_DF_IMAGE_RGB)
        {
            vx_uint8 constantData0[16] = {0, 8, 16, 32, 40, 48, 64, 72, 8, 8, 8, 8, 8, 8, 8, 8};
            vx_uint8 constantData1[16] = {80, 96, 104, 112, 0, 0, 0, 0, 8, 8, 8, 8, 0, 0, 0, 0};


			/*index = 0*/
			gcoVX_AddObject(&context,GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

			/*index = 1*/
			gcoVX_AddObject(&context,GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

            context.params.kernel = gcvVX_KERNEL_COLOR_CONVERT;
            context.params.xstep = 4;
            context.params.outputMultipleWidth = 3;
            context.params.inputFormat = gcvSURF_R8G8B8X8;
            context.params.outputFormat = gcvSURF_R8G8B8;

            gcoOS_MemCopy(&context.uniforms[0].uniform, constantData0, sizeof(constantData0));
	        context.uniforms[0].index = 4;
            context.uniforms[0].num = sizeof(constantData0) / sizeof(vx_uint8);

            gcoOS_MemCopy(&context.uniforms[1].uniform, constantData1, sizeof(constantData1));
	        context.uniforms[1].index = 5;
            context.uniforms[1].num = sizeof(constantData1) / sizeof(vx_uint8);

            context.uniform_num = 2;
        }
        else if ((outputFormat == VX_DF_IMAGE_YUV4) ||
                 (outputFormat == VX_DF_IMAGE_NV12) ||
                 (outputFormat == VX_DF_IMAGE_IYUV))
        {
            vx_uint32 constantData0[8] = {0, 8, 16, 24, 0, 0, 0, 0};
            vx_float32 constantData1[4] = { 0.2126f,  0.7152f,  0.0722f, 0.5f};
            vx_float32 constantData2[4] = {-0.1146f, -0.3854f,   0.5f,    128.5f};
            vx_float32 constantData3[4] = { 0.5f,    -0.4542f, -0.0458f, 128.5f};


			/*index = 0*/
			gcoVX_AddObject(&context,GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

			/*index = 1*/
			gcoVX_AddObject(&context,GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

            context.params.kernel = gcvVX_KERNEL_COLOR_CONVERT;
            context.params.xstep = 4;
            context.params.inputFormat = gcvSURF_R8G8B8X8;
            if (outputFormat == VX_DF_IMAGE_YUV4)
            {
                context.params.outputFormat = gcvSURF_AYUV;
            }
            else if (outputFormat == VX_DF_IMAGE_NV12)
            {
                context.params.ystep = 2;
                context.params.outputFormat = gcvSURF_NV12;
            }else
            {
                context.params.ystep = 2;
                context.params.outputFormat = gcvSURF_I420;
            }

            gcoOS_MemCopy(&context.uniforms[0].uniform, constantData0, sizeof(constantData0));
	        context.uniforms[0].index = 4;
            context.uniforms[0].num = sizeof(constantData0) / sizeof(vx_uint8);

            gcoOS_MemCopy(&context.uniforms[1].uniform, constantData1, sizeof(constantData1));
	        context.uniforms[1].index = 5;
            context.uniforms[1].num = sizeof(constantData1) / sizeof(vx_float32);

            gcoOS_MemCopy(&context.uniforms[2].uniform, constantData2, sizeof(constantData2));
	        context.uniforms[2].index = 6;
            context.uniforms[2].num = sizeof(constantData2) / sizeof(vx_float32);

            gcoOS_MemCopy(&context.uniforms[3].uniform, constantData3, sizeof(constantData3));
	        context.uniforms[3].index = 7;
            context.uniforms[3].num = sizeof(constantData3) / sizeof(vx_float32);

            context.uniform_num = 4;
        }
    }

    if (inputFormat == VX_DF_IMAGE_YUYV || inputFormat == VX_DF_IMAGE_UYVY)
    {
        if ((outputFormat == VX_DF_IMAGE_RGBX) || (outputFormat == VX_DF_IMAGE_RGB))
        {
            vx_uint32 constantData0[8] = {0, 8, 16, 24, 0, 0, 0, 0};
            vx_float32 constantData1[3] = {1.0f, 0.0f, 1.5748f};
            vx_float32 constantData2[3] = {1.0f, -0.1873f, -0.4681f};
            vx_float32 constantData3[3] = {1.0f, 1.8556f, 0.0};


			/*index = 0*/
			gcoVX_AddObject(&context,GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

			/*index = 1*/
			gcoVX_AddObject(&context,GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

            context.params.xstep = 2;
            context.params.kernel = gcvVX_KERNEL_COLOR_CONVERT;

            if (inputFormat == VX_DF_IMAGE_YUYV)
            {
                context.params.inputFormat = gcvSURF_YUY2;
            }
            else
            {
                context.params.inputFormat = gcvSURF_UYVY;
            }

            if (outputFormat == VX_DF_IMAGE_RGBX)
            {
                context.params.outputFormat = gcvSURF_R8G8B8X8;
            }
            else
            {
                context.params.outputMultipleWidth = 3;
                context.params.outputFormat = gcvSURF_R8G8B8;
            }

            gcoOS_MemCopy(&context.uniforms[0].uniform, constantData0, sizeof(constantData0));
	        context.uniforms[0].index = 4;
            context.uniforms[0].num = sizeof(constantData0) / sizeof(vx_uint8);

            gcoOS_MemCopy(&context.uniforms[1].uniform, constantData1, sizeof(constantData1));
	        context.uniforms[1].index = 5;
            context.uniforms[1].num = sizeof(constantData1) / sizeof(vx_float32);

            gcoOS_MemCopy(&context.uniforms[2].uniform, constantData2, sizeof(constantData2));
	        context.uniforms[2].index = 6;
            context.uniforms[2].num = sizeof(constantData2) / sizeof(vx_float32);

            gcoOS_MemCopy(&context.uniforms[3].uniform, constantData3, sizeof(constantData3));
	        context.uniforms[3].index = 7;
            context.uniforms[3].num = sizeof(constantData3) / sizeof(vx_float32);

            context.uniform_num = 4;
        }
        else if (outputFormat == VX_DF_IMAGE_NV12 || outputFormat == VX_DF_IMAGE_IYUV)
        {
            vx_uint8 constantData0[16] = {0, 16, 32, 48, 64, 80, 96, 112, 8, 8, 8, 8, 8, 8, 8, 8};  /*get y*/
            vx_uint8 constantData1[16] = {8, 24, 40, 56, 72, 88, 104, 120, 8, 8, 8, 8, 8, 8, 8, 8}; /*get uv*/
            vx_uint8 constantData2[16] = {0, 16, 32, 48, 0, 0, 0, 0, 8, 8, 8, 8, 0, 0, 0, 0}; /*get u*/
            vx_uint8 constantData3[16] = {8, 24, 40, 56, 0, 0, 0, 0, 8, 8, 8, 8, 0, 0, 0, 0}; /*get v*/
            vx_uint32 constantData4[8] = {0, 8, 16, 24, 0, 0, 0, 0};
            vx_uint8 constantData5[16] = {0, 8, 32, 40, 64, 72, 96, 104, 8, 8, 8, 8, 8, 8, 8, 8};
            vx_uint8 constantData6[16] = {0, 32, 64, 96, 0, 0, 0, 0, 8, 8, 8, 8, 0, 0, 0, 0};  /*only for VX_DF_IMAGE_IYUV*/


			/*index = 0*/
			gcoVX_AddObject(&context,GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

			/*index = 1*/
			gcoVX_AddObject(&context,GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

            context.params.kernel = gcvVX_KERNEL_COLOR_CONVERT;
            context.params.ystep = 2;
            if (inputFormat == VX_DF_IMAGE_YUYV)
            {
                context.params.inputFormat = gcvSURF_YUY2;
            }
            else
            {
                context.params.inputFormat = gcvSURF_UYVY;
            }

            if (outputFormat == VX_DF_IMAGE_NV12)
            {
                context.params.xstep = 8;
                context.params.outputFormat = gcvSURF_NV12;
            }
            else
            {
                context.params.xstep = 16;
                context.params.outputFormat = gcvSURF_I420;
                context.params.inputMultipleWidth = 2;  /*magnify width to width * 2*/
                context.params.xmax = inputWidth * 2;
            }

            gcoOS_MemCopy(&context.uniforms[0].uniform, constantData0, sizeof(constantData0));
	        context.uniforms[0].index = 4;
            context.uniforms[0].num = sizeof(constantData0) / sizeof(vx_uint8);

            gcoOS_MemCopy(&context.uniforms[1].uniform, constantData1, sizeof(constantData1));
	        context.uniforms[1].index = 5;
            context.uniforms[1].num = sizeof(constantData1) / sizeof(vx_uint8);

            gcoOS_MemCopy(&context.uniforms[2].uniform, constantData2, sizeof(constantData2));
	        context.uniforms[2].index = 6;
            context.uniforms[2].num = sizeof(constantData2) / sizeof(vx_uint8);

            gcoOS_MemCopy(&context.uniforms[3].uniform, constantData3, sizeof(constantData3));
	        context.uniforms[3].index = 7;
            context.uniforms[3].num = sizeof(constantData3) / sizeof(vx_uint8);

            gcoOS_MemCopy(&context.uniforms[4].uniform, constantData4, sizeof(constantData4));
	        context.uniforms[4].index = 8;
            context.uniforms[4].num = sizeof(constantData4) / sizeof(vx_uint32);

            gcoOS_MemCopy(&context.uniforms[5].uniform, constantData5, sizeof(constantData5));
	        context.uniforms[5].index = 9;
            context.uniforms[5].num = sizeof(constantData5) / sizeof(vx_uint8);

            if (outputFormat == VX_DF_IMAGE_NV12)
            {
                context.uniform_num = 6;
            }
            else
            {
                gcoOS_MemCopy(&context.uniforms[6].uniform, constantData6, sizeof(constantData6));
	            context.uniforms[6].index = 10;
                context.uniforms[6].num = sizeof(constantData6) / sizeof(vx_uint8);

                context.uniform_num = 7;
            }
        }
    }

    if (inputFormat == VX_DF_IMAGE_IYUV)
    {
        if ((outputFormat == VX_DF_IMAGE_NV12) || (outputFormat == VX_DF_IMAGE_YUV4))
        {
            vx_uint32 constantData0[8] = {0, 8, 16, 24, 0, 0, 0, 0};


			/*index = 0*/
			gcoVX_AddObject(&context,GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

			/*index = 1*/
			gcoVX_AddObject(&context,GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, 3);

            context.params.kernel = gcvVX_KERNEL_COLOR_CONVERT;
            if (inputWidth < 16)
            {
                context.params.xstep = 8;
            }
            else
            {
                context.params.xstep = 16;
            }

            context.params.inputFormat = gcvSURF_I420;
            if (outputFormat == VX_DF_IMAGE_NV12)
            {
                context.params.ystep = 2;
                context.params.outputFormat = gcvSURF_NV12;
            }
            else
            {
                context.params.outputFormat = gcvSURF_AYUV;
            }

            gcoOS_MemCopy(&context.uniforms[0].uniform, constantData0, sizeof(constantData0));
	        context.uniforms[0].index = 6;
            context.uniforms[0].num = sizeof(constantData0) / sizeof(vx_uint32);

            context.uniform_num = 1;
        }
        else if (outputFormat == VX_DF_IMAGE_RGBX || outputFormat == VX_DF_IMAGE_RGB)
        {
            vx_uint32 constantData0[8] = {0, 8, 16, 24, 0, 0, 0, 0};
            vx_float32 constantData1[3] = {1.0f, 0.0f, 1.5748f};
            vx_float32 constantData2[3] = {1.0f, -0.1873f, -0.4681f};
            vx_float32 constantData3[3] = {1.0f, 1.8556f, 0.0};
            vx_uint8 constantData4[16] = {0, 32, 64, 0, 0, 0, 0, 0, 8, 8, 8, 0, 0, 0, 0, 0};


			/*index = 0*/
			gcoVX_AddObject(&context,GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

			/*index = 1*/
			gcoVX_AddObject(&context,GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, 3);

            context.params.xstep = 4;
            context.params.ystep = 2;
            context.params.kernel = gcvVX_KERNEL_COLOR_CONVERT;

            context.params.inputFormat = gcvSURF_I420;
            if (outputFormat == VX_DF_IMAGE_RGBX)
            {

                context.params.outputFormat = gcvSURF_R8G8B8X8;
            }
            else
            {
                context.params.outputMultipleWidth = 3;
                context.params.outputFormat = gcvSURF_R8G8B8;
            }

            gcoOS_MemCopy(&context.uniforms[0].uniform, constantData0, sizeof(constantData0));
	        context.uniforms[0].index = 4;
            context.uniforms[0].num = sizeof(constantData0) / sizeof(vx_uint32);

            gcoOS_MemCopy(&context.uniforms[1].uniform, constantData1, sizeof(constantData1));
	        context.uniforms[1].index = 5;
            context.uniforms[1].num = sizeof(constantData1) / sizeof(vx_float32);

            gcoOS_MemCopy(&context.uniforms[2].uniform, constantData2, sizeof(constantData2));
	        context.uniforms[2].index = 6;
            context.uniforms[2].num = sizeof(constantData2) / sizeof(vx_float32);

            gcoOS_MemCopy(&context.uniforms[3].uniform, constantData3, sizeof(constantData3));
	        context.uniforms[3].index = 7;
            context.uniforms[3].num = sizeof(constantData3) / sizeof(vx_float32);

            gcoOS_MemCopy(&context.uniforms[4].uniform, constantData4, sizeof(constantData4));
	        context.uniforms[4].index = 8;
            context.uniforms[4].num = sizeof(constantData4) / sizeof(vx_uint8);

            context.uniform_num = 5;
        }
    }

    if (inputFormat == VX_DF_IMAGE_NV12 || inputFormat == VX_DF_IMAGE_NV21)
    {
        if (outputFormat == VX_DF_IMAGE_RGBX || outputFormat == VX_DF_IMAGE_RGB)
        {
            vx_uint32 constantData0[8] = {0, 8, 16, 24, 0, 0, 0, 0};
            vx_float32 constantData1[3] = {1.0f, 0.0f, 1.5748f};
            vx_float32 constantData2[3] = {1.0f, -0.1873f, -0.4681f};
            vx_float32 constantData3[3] = {1.0f, 1.8556f, 0.0};
            vx_uint8 constantData4[16] = {0, 32, 64, 0, 0, 0, 0, 0, 8, 8, 8, 0, 0, 0, 0, 0};


			/*index = 0*/
			gcoVX_AddObject(&context,GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

			/*index = 1*/
			gcoVX_AddObject(&context,GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, 3);

            context.params.kernel = gcvVX_KERNEL_COLOR_CONVERT;
            context.params.xstep = 4;
            context.params.ystep = 2;
            if (inputFormat == VX_DF_IMAGE_NV12)
            {
                context.params.inputFormat = gcvSURF_NV12;
            }
            else
            {
                context.params.inputFormat = gcvSURF_NV21;
            }

            if (outputFormat == VX_DF_IMAGE_RGBX)
            {

                context.params.outputFormat = gcvSURF_R8G8B8X8;
            }
            else
            {
                context.params.outputMultipleWidth = 3;
                context.params.outputFormat = gcvSURF_R8G8B8;
            }

            gcoOS_MemCopy(&context.uniforms[0].uniform, constantData0, sizeof(constantData0));
	        context.uniforms[0].index = 4;
            context.uniforms[0].num = sizeof(constantData0) / sizeof(vx_uint32);

            gcoOS_MemCopy(&context.uniforms[1].uniform, constantData1, sizeof(constantData1));
	        context.uniforms[1].index = 5;
            context.uniforms[1].num = sizeof(constantData1) / sizeof(vx_float32);

            gcoOS_MemCopy(&context.uniforms[2].uniform, constantData2, sizeof(constantData2));
	        context.uniforms[2].index = 6;
            context.uniforms[2].num = sizeof(constantData2) / sizeof(vx_float32);

            gcoOS_MemCopy(&context.uniforms[3].uniform, constantData3, sizeof(constantData3));
	        context.uniforms[3].index = 7;
            context.uniforms[3].num = sizeof(constantData3) / sizeof(vx_float32);

            gcoOS_MemCopy(&context.uniforms[4].uniform, constantData4, sizeof(constantData4));
	        context.uniforms[4].index = 8;
            context.uniforms[4].num = sizeof(constantData4) / sizeof(vx_uint8);

            context.uniform_num = 5;
        }
        else if (outputFormat == VX_DF_IMAGE_YUV4 || outputFormat == VX_DF_IMAGE_IYUV)
        {
            vx_uint32 constantData0[8] = {0, 8, 16, 24, 0, 0, 0, 0};
            vx_uint8 constantData1[16] = {0, 16, 32, 48, 64, 80, 96, 112, 8, 8, 8, 8, 8, 8, 8, 8}; /*get u*/
            vx_uint8 constantData2[16] = {8, 24, 40, 56, 72, 88, 104, 120, 8, 8, 8, 8, 8, 8, 8, 8}; /*get v*/


			/*index = 0*/
			gcoVX_AddObject(&context,GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

			/*index = 1*/
			gcoVX_AddObject(&context,GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, 3);

            context.params.kernel = gcvVX_KERNEL_COLOR_CONVERT;

            if (inputFormat == VX_DF_IMAGE_NV12)
            {
                context.params.inputFormat = gcvSURF_NV12;
            }
            else
            {
                context.params.inputFormat = gcvSURF_NV21;
            }

            if (outputFormat == VX_DF_IMAGE_YUV4)
            {
                context.params.xstep = 16;
                context.params.outputFormat = gcvSURF_AYUV;
            }
            else
            {
                if (inputWidth % 16 != 0)
                {
                    context.params.xstep = 8;
                }
                else
                {
                    context.params.xstep = 16;
                }
                context.params.outputFormat = gcvSURF_I420;
            }

            gcoOS_MemCopy(&context.uniforms[0].uniform, constantData0, sizeof(constantData0));
	        context.uniforms[0].index = 6;
            context.uniforms[0].num = sizeof(constantData0) / sizeof(vx_uint32);

            gcoOS_MemCopy(&context.uniforms[1].uniform, constantData1, sizeof(constantData1));
	        context.uniforms[1].index = 7;
            context.uniforms[1].num = sizeof(constantData1) / sizeof(vx_uint8);

            gcoOS_MemCopy(&context.uniforms[2].uniform, constantData2, sizeof(constantData2));
	        context.uniforms[2].index = 8;
            context.uniforms[2].num = sizeof(constantData2) / sizeof(vx_uint8);

            context.uniform_num = 3;
        }
    }

    return gcfVX_Kernel(&context);
}

