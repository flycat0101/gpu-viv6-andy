/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include <gc_vxk_common.h>

vx_status vxConvertColor(vx_node node, vx_image input, vx_image output)
{
    vx_status status = VX_SUCCESS;
    vx_df_image inputFormat = 0;
    vx_df_image outputFormat = 0;

    vx_uint32 inputWidth = 0;
    gcoVX_Kernel_Context * kernelContext = gcvNULL;

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
        kernelContext->uniform_num = 0;
    }

    vxQueryImage(input, VX_IMAGE_FORMAT, &inputFormat, sizeof(vx_df_image));

    vxQueryImage(output, VX_IMAGE_FORMAT, &outputFormat, sizeof(vx_df_image));

    vxQueryImage(input, VX_IMAGE_WIDTH, &inputWidth, sizeof(vx_uint32));

    if (inputFormat == VX_DF_IMAGE_RGB)
    {
        if (outputFormat == VX_DF_IMAGE_RGBX)
        {
            vx_uint8 constantData1[16] = {0, 8, 16, 0, 24, 32, 40, 0, 8, 8, 8, 0, 8, 8, 8, 0};

            vx_uint8 constantData2[16] = {48, 56, 64, 0, 72, 80, 88, 0, 8, 8, 8, 0, 8, 8, 8, 0};

            /*index = 0*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

            /*index = 1*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

            kernelContext->params.kernel = gcvVX_KERNEL_COLOR_CONVERT;
            kernelContext->params.xstep = 12;
            kernelContext->params.inputFormat = gcvSURF_R8G8B8;
            kernelContext->params.outputFormat = gcvSURF_R8G8B8X8;

            kernelContext->params.inputMultipleWidth = 3;
            kernelContext->params.xmax = inputWidth * 3;

            gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, constantData1, sizeof(constantData1));
            kernelContext->uniforms[0].index = 4;
            kernelContext->uniforms[0].num = vxmLENGTH_OF(constantData1);

            gcoOS_MemCopy(&kernelContext->uniforms[1].uniform, constantData2, sizeof(constantData2));
            kernelContext->uniforms[1].index = 5;
            kernelContext->uniforms[1].num = vxmLENGTH_OF(constantData2);

            kernelContext->uniform_num = 2;
        }
        else if ((outputFormat == VX_DF_IMAGE_YUV4) ||
                 (outputFormat == VX_DF_IMAGE_NV12) ||
                 (outputFormat == VX_DF_IMAGE_IYUV))
        {
            vx_uint8 constantData[16] = {0, 8, 16, 24, 0, 0, 0, 0, 8, 8, 8, 0, 0, 0, 0, 0};
            vx_float32 constantData1[4] = { 0.2126f, 0.7152f, 0.0722f, 0.5f};
            vx_float32 constantData2[4] = {-0.1146f, -0.3854f, 0.5f, 128.5f};
            vx_float32 constantData3[4] = { 0.5f, -0.4542f, -0.0458f, 128.5f};
            vx_uint8 constantData4[16] = {24, 32, 40, 48, 0, 0, 0, 0, 8, 8, 8, 0, 0, 0, 0, 0};
            vx_uint8 constantData5[16] = {48, 56, 64, 72, 0, 0, 0, 0, 8, 8, 8, 0, 0, 0, 0, 0};
            vx_uint8 constantData6[16] = {72, 80, 88, 96, 0, 0, 0, 0, 8, 8, 8, 0, 0, 0, 0, 0};
            vx_uint8 constantData7[16] = {0, 32, 64, 96, 0, 0, 0, 0, 8, 8, 8, 8, 0, 0, 0, 0};

            /*index = 0*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

            /*index = 1*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

            kernelContext->params.kernel = gcvVX_KERNEL_COLOR_CONVERT;
            kernelContext->params.xstep = 12;
            kernelContext->params.inputMultipleWidth = 3;
            kernelContext->params.xmax = inputWidth * 3;
            kernelContext->params.inputFormat = gcvSURF_R8G8B8;
            if (outputFormat == VX_DF_IMAGE_YUV4)
            {
                kernelContext->params.outputFormat = gcvSURF_AYUV;
            }else if (outputFormat == VX_DF_IMAGE_NV12)
            {
                kernelContext->params.ystep = 2;
                kernelContext->params.outputFormat = gcvSURF_NV12;
            }else
            {
                kernelContext->params.ystep = 2;
                kernelContext->params.outputFormat = gcvSURF_I420;
            }

            gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, constantData, sizeof(constantData));
            kernelContext->uniforms[0].index = 4;
            kernelContext->uniforms[0].num = vxmLENGTH_OF(constantData);

            gcoOS_MemCopy(&kernelContext->uniforms[1].uniform, constantData1, sizeof(constantData1));
            kernelContext->uniforms[1].index = 5;
            kernelContext->uniforms[1].num = vxmLENGTH_OF(constantData1);

            gcoOS_MemCopy(&kernelContext->uniforms[2].uniform, constantData2, sizeof(constantData2));
            kernelContext->uniforms[2].index = 6;
            kernelContext->uniforms[2].num = vxmLENGTH_OF(constantData2);

            gcoOS_MemCopy(&kernelContext->uniforms[3].uniform, constantData3, sizeof(constantData3));
            kernelContext->uniforms[3].index = 7;
            kernelContext->uniforms[3].num = vxmLENGTH_OF(constantData3);

            gcoOS_MemCopy(&kernelContext->uniforms[4].uniform, constantData4, sizeof(constantData4));
            kernelContext->uniforms[4].index = 8;
            kernelContext->uniforms[4].num = vxmLENGTH_OF(constantData4);

            gcoOS_MemCopy(&kernelContext->uniforms[5].uniform, constantData5, sizeof(constantData5));
            kernelContext->uniforms[5].index = 9;
            kernelContext->uniforms[5].num = vxmLENGTH_OF(constantData5);

            gcoOS_MemCopy(&kernelContext->uniforms[6].uniform, constantData6, sizeof(constantData6));
            kernelContext->uniforms[6].index = 10;
            kernelContext->uniforms[6].num = vxmLENGTH_OF(constantData6);

            gcoOS_MemCopy(&kernelContext->uniforms[7].uniform, constantData7, sizeof(constantData7));
            kernelContext->uniforms[7].index = 11;
            kernelContext->uniforms[7].num = vxmLENGTH_OF(constantData7);

            kernelContext->uniform_num = 8;
        }
    }

    if (inputFormat == VX_DF_IMAGE_RGBX)
    {
        if (outputFormat == VX_DF_IMAGE_RGB)
        {
            vx_uint8 constantData0[16] = {0, 8, 16, 32, 40, 48, 64, 72, 8, 8, 8, 8, 8, 8, 8, 8};
            vx_uint8 constantData1[16] = {80, 96, 104, 112, 0, 0, 0, 0, 8, 8, 8, 8, 0, 0, 0, 0};

            /*index = 0*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

            /*index = 1*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

            kernelContext->params.kernel = gcvVX_KERNEL_COLOR_CONVERT;
            kernelContext->params.xstep = 4;
            kernelContext->params.outputMultipleWidth = 3;
            kernelContext->params.inputFormat = gcvSURF_R8G8B8X8;
            kernelContext->params.outputFormat = gcvSURF_R8G8B8;

            gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, constantData0, sizeof(constantData0));
            kernelContext->uniforms[0].index = 4;
            kernelContext->uniforms[0].num = vxmLENGTH_OF(constantData0);

            gcoOS_MemCopy(&kernelContext->uniforms[1].uniform, constantData1, sizeof(constantData1));
            kernelContext->uniforms[1].index = 5;
            kernelContext->uniforms[1].num = vxmLENGTH_OF(constantData1);

            kernelContext->uniform_num = 2;
        }
        else if ((outputFormat == VX_DF_IMAGE_YUV4) ||
                 (outputFormat == VX_DF_IMAGE_NV12) ||
                 (outputFormat == VX_DF_IMAGE_IYUV))
        {
            vx_uint32 constantData0[8] = {0, 8, 16, 24, 0, 0, 0, 0};
            vx_float32 constantData1[4] = { 0.2126f, 0.7152f, 0.0722f, 0.5f};
            vx_float32 constantData2[4] = {-0.1146f, -0.3854f, 0.5f, 128.5f};
            vx_float32 constantData3[4] = { 0.5f, -0.4542f, -0.0458f, 128.5f};

            /*index = 0*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

            /*index = 1*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

            kernelContext->params.kernel = gcvVX_KERNEL_COLOR_CONVERT;
            kernelContext->params.xstep = 4;
            kernelContext->params.inputFormat = gcvSURF_R8G8B8X8;
            if (outputFormat == VX_DF_IMAGE_YUV4)
            {
                kernelContext->params.outputFormat = gcvSURF_AYUV;
            }
            else if (outputFormat == VX_DF_IMAGE_NV12)
            {
                kernelContext->params.ystep = 2;
                kernelContext->params.outputFormat = gcvSURF_NV12;
            }else
            {
                kernelContext->params.ystep = 2;
                kernelContext->params.outputFormat = gcvSURF_I420;
            }

            gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, constantData0, sizeof(constantData0));
            kernelContext->uniforms[0].index = 4;
            kernelContext->uniforms[0].num = vxmLENGTH_OF(constantData0);

            gcoOS_MemCopy(&kernelContext->uniforms[1].uniform, constantData1, sizeof(constantData1));
            kernelContext->uniforms[1].index = 5;
            kernelContext->uniforms[1].num = vxmLENGTH_OF(constantData1);

            gcoOS_MemCopy(&kernelContext->uniforms[2].uniform, constantData2, sizeof(constantData2));
            kernelContext->uniforms[2].index = 6;
            kernelContext->uniforms[2].num = vxmLENGTH_OF(constantData2);

            gcoOS_MemCopy(&kernelContext->uniforms[3].uniform, constantData3, sizeof(constantData3));
            kernelContext->uniforms[3].index = 7;
            kernelContext->uniforms[3].num = vxmLENGTH_OF(constantData3);

            kernelContext->uniform_num = 4;
        }
    }

    if (inputFormat == VX_DF_IMAGE_YUYV || inputFormat == VX_DF_IMAGE_UYVY)
    {
        if ((outputFormat == VX_DF_IMAGE_RGBX) || (outputFormat == VX_DF_IMAGE_RGB))
        {
            /*for YUYV*/
            vx_uint8 dataYUYV0[16] = {0, 8, 24, 0, 0, 0, 0, 0, 8, 8, 8, 0, 0, 0, 0, 0};
            vx_uint8 dataYUYV1[16] = {16, 8, 24, 0, 0, 0, 0, 0, 8, 8, 8, 0, 0, 0, 0, 0};
            vx_uint8 dataYUYV2[16] = {32, 40, 56, 0, 0, 0, 0, 0, 8, 8, 8, 0, 0, 0, 0, 0};
            vx_uint8 dataYUYV3[16] = {48, 40, 56, 0, 0, 0, 0, 0, 8, 8, 8, 0, 0, 0, 0, 0};
            /*for UYVY*/
            vx_uint8 dataUYUV0[16] = {8, 0, 16, 0, 0, 0, 0, 0, 8, 8, 8, 0, 0, 0, 0, 0};
            vx_uint8 dataUYUV1[16] = {24, 0, 16, 0, 0, 0, 0, 0, 8, 8, 8, 0, 0, 0, 0, 0};
            vx_uint8 dataUYUV2[16] = {40, 32, 48, 0, 0, 0, 0, 0, 8, 8, 8, 0, 0, 0, 0, 0};
            vx_uint8 dataUYUV3[16] = {56, 32, 48, 0, 0, 0, 0, 0, 8, 8, 8, 0, 0, 0, 0, 0};

            vx_uint8 constantData0[16] = {0};
            vx_uint8 constantData4[16] = {0};
            vx_uint8 constantData5[16] = {0};
            vx_uint8 constantData6[16] = {0};
            vx_float32 constantData1[4] = {1.0f, 0.0f, 1.5748f, -201.0744f};
            vx_float32 constantData2[4] = {1.0f, -0.1873f, -0.4681f, 84.3912f};
            vx_float32 constantData3[4] = {1.0f, 1.8556f, 0.0, -237.0168f};

            vx_uint8 constantData7[16] = {0, 8, 16, 32, 40, 48, 64, 72, 8, 8, 8, 8, 8, 8, 8, 8};
            vx_uint8 constantData8[16] = {80, 96, 104, 112, 0, 0, 0, 0, 8, 8, 8, 8, 0, 0, 0, 0};

            /*index = 0*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

            /*index = 1*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

            kernelContext->params.xstep = 8;
            kernelContext->params.kernel = gcvVX_KERNEL_COLOR_CONVERT;

            kernelContext->params.inputMultipleWidth = 2;

            if (inputFormat == VX_DF_IMAGE_YUYV)
            {
                gcoOS_MemCopy(constantData0, dataYUYV0, sizeof(constantData0));
                gcoOS_MemCopy(constantData4, dataYUYV1, sizeof(constantData4));
                gcoOS_MemCopy(constantData5, dataYUYV2, sizeof(constantData5));
                gcoOS_MemCopy(constantData6, dataYUYV3, sizeof(constantData6));

                kernelContext->params.inputFormat = gcvSURF_YUY2;
            }
            else
            {
                gcoOS_MemCopy(constantData0, dataUYUV0, sizeof(constantData0));
                gcoOS_MemCopy(constantData4, dataUYUV1, sizeof(constantData4));
                gcoOS_MemCopy(constantData5, dataUYUV2, sizeof(constantData5));
                gcoOS_MemCopy(constantData6, dataUYUV3, sizeof(constantData6));

                kernelContext->params.inputFormat = gcvSURF_UYVY;
            }

            gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, constantData0, sizeof(constantData0));
            kernelContext->uniforms[0].index = 4;
            kernelContext->uniforms[0].num = vxmLENGTH_OF(constantData0);

            gcoOS_MemCopy(&kernelContext->uniforms[1].uniform, constantData1, sizeof(constantData1));
            kernelContext->uniforms[1].index = 5;
            kernelContext->uniforms[1].num = vxmLENGTH_OF(constantData1);

            gcoOS_MemCopy(&kernelContext->uniforms[2].uniform, constantData2, sizeof(constantData2));
            kernelContext->uniforms[2].index = 6;
            kernelContext->uniforms[2].num = vxmLENGTH_OF(constantData2);

            gcoOS_MemCopy(&kernelContext->uniforms[3].uniform, constantData3, sizeof(constantData3));
            kernelContext->uniforms[3].index = 7;
            kernelContext->uniforms[3].num = vxmLENGTH_OF(constantData3);

            gcoOS_MemCopy(&kernelContext->uniforms[4].uniform, constantData4, sizeof(constantData4));
            kernelContext->uniforms[4].index = 8;
            kernelContext->uniforms[4].num = vxmLENGTH_OF(constantData4);

            gcoOS_MemCopy(&kernelContext->uniforms[5].uniform, constantData5, sizeof(constantData5));
            kernelContext->uniforms[5].index = 9;
            kernelContext->uniforms[5].num = vxmLENGTH_OF(constantData5);

            gcoOS_MemCopy(&kernelContext->uniforms[6].uniform, constantData6, sizeof(constantData6));
            kernelContext->uniforms[6].index = 10;
            kernelContext->uniforms[6].num = vxmLENGTH_OF(constantData6);

            if (outputFormat == VX_DF_IMAGE_RGBX)
            {
                kernelContext->uniform_num = 7;
                kernelContext->params.outputFormat = gcvSURF_R8G8B8X8;
            }
            else
            {
                gcoOS_MemCopy(&kernelContext->uniforms[7].uniform, constantData7, sizeof(constantData7));
                kernelContext->uniforms[7].index = 11;
                kernelContext->uniforms[7].num = vxmLENGTH_OF(constantData7);

                gcoOS_MemCopy(&kernelContext->uniforms[8].uniform, constantData8, sizeof(constantData8));
                kernelContext->uniforms[8].index = 12;
                kernelContext->uniforms[8].num = vxmLENGTH_OF(constantData8);

                kernelContext->uniform_num = 9;

                kernelContext->params.outputMultipleWidth = 3;
                kernelContext->params.outputFormat = gcvSURF_R8G8B8;
            }


        }
        else if (outputFormat == VX_DF_IMAGE_NV12 || outputFormat == VX_DF_IMAGE_IYUV)
        {
            vx_uint8 dataYUYV0[16] = {0, 16, 32, 48, 64, 80, 96, 112, 8, 8, 8, 8, 8, 8, 8, 8};  /*get y for YUYV*/
            vx_uint8 dataYUYV1[16] = {8, 40, 72, 104, 0, 0, 0, 0, 8, 8, 8, 8, 0, 0, 0, 0}; /*get u*/
            vx_uint8 dataYUYV2[16] = {24, 56, 88, 120, 0, 0, 0, 0, 8, 8, 8, 8, 0, 0, 0, 0}; /*get v*/

            vx_uint8 dataUYVY0[16] = {8, 24, 40, 56, 72, 88, 104, 120, 8, 8, 8, 8, 8, 8, 8, 8}; /*get y for UYVY*/
            vx_uint8 dataUYVY1[16] = {0, 32, 64, 96, 0, 0, 0, 0, 8, 8, 8, 8, 0, 0, 0, 0}; /*get u*/
            vx_uint8 dataUYVY2[16] = {16, 48, 80, 112, 0, 0, 0, 0, 8, 8, 8, 8, 0, 0, 0, 0}; /*get v*/

            vx_uint8 constantData0[16] = {0};
            vx_uint8 constantData1[16] = {0};
            vx_uint8 constantData2[16] = {0};
            vx_uint8 constantData3[16] = {0, 8, 32, 40, 64, 72, 96, 104, 8, 8, 8, 8, 8, 8, 8, 8};
            vx_uint8 constantData4[16] = {0, 32, 64, 96, 0, 0, 0, 0, 8, 8, 8, 8, 0, 0, 0, 0};  /*only for VX_DF_IMAGE_IYUV*/

            /*index = 0*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

            /*index = 1*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, GC_VX_INDEX_AUTO);

            kernelContext->params.kernel = gcvVX_KERNEL_COLOR_CONVERT;
            kernelContext->params.ystep = 2;
            if (inputFormat == VX_DF_IMAGE_YUYV)
            {
                gcoOS_MemCopy(constantData0, dataYUYV0, sizeof(constantData0));
                gcoOS_MemCopy(constantData1, dataYUYV1, sizeof(constantData1));
                gcoOS_MemCopy(constantData2, dataYUYV2, sizeof(constantData2));
                kernelContext->params.inputFormat = gcvSURF_YUY2;
            }
            else
            {
                gcoOS_MemCopy(constantData0, dataUYVY0, sizeof(constantData0));
                gcoOS_MemCopy(constantData1, dataUYVY1, sizeof(constantData1));
                gcoOS_MemCopy(constantData2, dataUYVY2, sizeof(constantData2));
                kernelContext->params.inputFormat = gcvSURF_UYVY;
            }

            if (outputFormat == VX_DF_IMAGE_NV12)
            {
                kernelContext->params.xstep = 8;
                kernelContext->params.outputFormat = gcvSURF_NV12;
            }
            else
            {
                kernelContext->params.xstep = 16;
                kernelContext->params.outputFormat = gcvSURF_I420;
                kernelContext->params.inputMultipleWidth = 2;  /*magnify width to width * 2*/
                kernelContext->params.xmax = inputWidth * 2;
            }

            gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, constantData0, sizeof(constantData0));
            kernelContext->uniforms[0].index = 4;
            kernelContext->uniforms[0].num = vxmLENGTH_OF(constantData0);

            gcoOS_MemCopy(&kernelContext->uniforms[1].uniform, constantData1, sizeof(constantData1));
            kernelContext->uniforms[1].index = 5;
            kernelContext->uniforms[1].num = vxmLENGTH_OF(constantData1);

            gcoOS_MemCopy(&kernelContext->uniforms[2].uniform, constantData2, sizeof(constantData2));
            kernelContext->uniforms[2].index = 6;
            kernelContext->uniforms[2].num = vxmLENGTH_OF(constantData2);

            gcoOS_MemCopy(&kernelContext->uniforms[3].uniform, constantData3, sizeof(constantData3));
            kernelContext->uniforms[3].index = 7;
            kernelContext->uniforms[3].num = vxmLENGTH_OF(constantData3);

            if (outputFormat == VX_DF_IMAGE_NV12)
            {
                kernelContext->uniform_num = 4;
            }
            else
            {
                gcoOS_MemCopy(&kernelContext->uniforms[4].uniform, constantData4, sizeof(constantData4));
                kernelContext->uniforms[4].index = 8;
                kernelContext->uniforms[4].num = vxmLENGTH_OF(constantData4);

                kernelContext->uniform_num = 5;
            }
        }
    }

    if (inputFormat == VX_DF_IMAGE_IYUV)
    {
        if ((outputFormat == VX_DF_IMAGE_NV12) || (outputFormat == VX_DF_IMAGE_YUV4))
        {
            vx_uint8 dataNV12_0[16] = {0, 0, 8, 0, 16, 0, 24, 0, 8, 0, 8, 0, 8, 0, 8, 0};
            vx_uint8 dataNV12_1[16] = {32, 0, 40, 0, 48, 0, 56, 0, 8, 0, 8, 0, 8, 0, 8, 0};
            vx_uint8 dataYUV4_0[16] = {0, 0, 8, 8, 16, 16, 24, 24, 8, 8, 8, 8, 8, 8, 8, 8};
            vx_uint8 dataYUV4_1[16] = {32, 32, 40, 40, 48, 48, 56, 56, 8, 8, 8, 8, 8, 8, 8, 8};

            vx_uint8 constantData0[16] = {0};
            vx_uint8 constantData1[16] = {0};

            /*index = 0*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

            /*index = 1*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, 3);

            kernelContext->params.kernel = gcvVX_KERNEL_COLOR_CONVERT;
            if (inputWidth < 16)
            {
                kernelContext->params.xstep = 8;
            }
            else
            {
                kernelContext->params.xstep = 16;
            }

            kernelContext->params.inputFormat = gcvSURF_I420;
            if (outputFormat == VX_DF_IMAGE_NV12)
            {
                kernelContext->params.ystep = 2;
                kernelContext->params.outputFormat = gcvSURF_NV12;
                gcoOS_MemCopy(constantData0, dataNV12_0, sizeof(constantData0));
                gcoOS_MemCopy(constantData1, dataNV12_1, sizeof(constantData0));
            }
            else
            {
                kernelContext->params.outputFormat = gcvSURF_AYUV;
                gcoOS_MemCopy(constantData0, dataYUV4_0, sizeof(constantData0));
                gcoOS_MemCopy(constantData1, dataYUV4_1, sizeof(constantData0));
            }

            gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, constantData0, sizeof(constantData0));
            kernelContext->uniforms[0].index = 6;
            kernelContext->uniforms[0].num = vxmLENGTH_OF(constantData0);

            gcoOS_MemCopy(&kernelContext->uniforms[1].uniform, constantData1, sizeof(constantData1));
            kernelContext->uniforms[1].index = 7;
            kernelContext->uniforms[1].num = vxmLENGTH_OF(constantData1);

            kernelContext->uniform_num = 2;
        }
        else if (outputFormat == VX_DF_IMAGE_RGBX || outputFormat == VX_DF_IMAGE_RGB)
        {
            vx_uint32 constantData0[8] = {0, 8, 16, 24, 0, 0, 0, 0};
            vx_float32 constantData1[4] = {1.0f, 0.0f, 1.5748f, -201.0744f};
            vx_float32 constantData2[4] = {1.0f, -0.1873f, -0.4681f, 84.3912f};
            vx_float32 constantData3[4] = {1.0f, 1.8556f, 0.0, -237.0168f};
            vx_uint8 constantData4[16] = {0, 32, 64, 0, 0, 0, 0, 0, 8, 8, 8, 0, 0, 0, 0, 0};
            vx_uint8 constantData5[16] = {0, 8, 16, 32, 40, 48, 64, 72, 8, 8, 8, 8, 8, 8, 8, 8};
            vx_uint8 constantData6[16] = {80, 96, 104, 112, 0, 0, 0, 0, 8, 8, 8, 8, 0, 0, 0, 0};

            /*index = 0*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

            /*index = 1*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, 3);

            kernelContext->params.xstep = 4;
            kernelContext->params.ystep = 2;
            kernelContext->params.kernel = gcvVX_KERNEL_COLOR_CONVERT;

            kernelContext->params.inputFormat = gcvSURF_I420;

            gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, constantData0, sizeof(constantData0));
            kernelContext->uniforms[0].index = 4;
            kernelContext->uniforms[0].num = vxmLENGTH_OF(constantData0);

            gcoOS_MemCopy(&kernelContext->uniforms[1].uniform, constantData1, sizeof(constantData1));
            kernelContext->uniforms[1].index = 5;
            kernelContext->uniforms[1].num = vxmLENGTH_OF(constantData1);

            gcoOS_MemCopy(&kernelContext->uniforms[2].uniform, constantData2, sizeof(constantData2));
            kernelContext->uniforms[2].index = 6;
            kernelContext->uniforms[2].num = vxmLENGTH_OF(constantData2);

            gcoOS_MemCopy(&kernelContext->uniforms[3].uniform, constantData3, sizeof(constantData3));
            kernelContext->uniforms[3].index = 7;
            kernelContext->uniforms[3].num = vxmLENGTH_OF(constantData3);

            gcoOS_MemCopy(&kernelContext->uniforms[4].uniform, constantData4, sizeof(constantData4));
            kernelContext->uniforms[4].index = 8;
            kernelContext->uniforms[4].num = vxmLENGTH_OF(constantData4);

            if (outputFormat == VX_DF_IMAGE_RGBX)
            {
                kernelContext->uniform_num = 5;
                kernelContext->params.outputFormat = gcvSURF_R8G8B8X8;
            }
            else
            {
                gcoOS_MemCopy(&kernelContext->uniforms[5].uniform, constantData5, sizeof(constantData5));
                kernelContext->uniforms[5].index = 9;
                kernelContext->uniforms[5].num = vxmLENGTH_OF(constantData5);

                gcoOS_MemCopy(&kernelContext->uniforms[6].uniform, constantData6, sizeof(constantData6));
                kernelContext->uniforms[6].index = 10;
                kernelContext->uniforms[6].num = vxmLENGTH_OF(constantData6);

                kernelContext->uniform_num = 7;
                kernelContext->params.outputMultipleWidth = 3;
                kernelContext->params.outputFormat = gcvSURF_R8G8B8;
            }
        }
    }

    if (inputFormat == VX_DF_IMAGE_NV12 || inputFormat == VX_DF_IMAGE_NV21)
    {
        if (outputFormat == VX_DF_IMAGE_RGBX || outputFormat == VX_DF_IMAGE_RGB)
        {
            vx_uint32 constantData0[8] = {0, 8, 16, 24, 0, 0, 0, 0};
            vx_float32 constantData1[4] = {1.0f, 0.0f, 1.5748f, -201.0744f};
            vx_float32 constantData2[4] = {1.0f, -0.1873f, -0.4681f, 84.3912f};
            vx_float32 constantData3[4] = {1.0f, 1.8556f, 0.0, -237.0168f};
            vx_uint8 constantData4[16] = {0, 32, 64, 0, 0, 0, 0, 0, 8, 8, 8, 0, 0, 0, 0, 0};
            vx_uint8 constantData5[16] = {0, 8, 16, 32, 40, 48, 64, 72, 8, 8, 8, 8, 8, 8, 8, 8};
            vx_uint8 constantData6[16] = {80, 96, 104, 112, 0, 0, 0, 0, 8, 8, 8, 8, 0, 0, 0, 0};

            /*index = 0*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

            /*index = 1*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, 3);

            kernelContext->params.kernel = gcvVX_KERNEL_COLOR_CONVERT;
            kernelContext->params.xstep = 4;
            kernelContext->params.ystep = 2;
            if (inputFormat == VX_DF_IMAGE_NV12)
            {
                kernelContext->params.inputFormat = gcvSURF_NV12;
            }
            else
            {
                kernelContext->params.inputFormat = gcvSURF_NV21;
            }

            gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, constantData0, sizeof(constantData0));
            kernelContext->uniforms[0].index = 4;
            kernelContext->uniforms[0].num = vxmLENGTH_OF(constantData0);

            gcoOS_MemCopy(&kernelContext->uniforms[1].uniform, constantData1, sizeof(constantData1));
            kernelContext->uniforms[1].index = 5;
            kernelContext->uniforms[1].num = vxmLENGTH_OF(constantData1);

            gcoOS_MemCopy(&kernelContext->uniforms[2].uniform, constantData2, sizeof(constantData2));
            kernelContext->uniforms[2].index = 6;
            kernelContext->uniforms[2].num = vxmLENGTH_OF(constantData2);

            gcoOS_MemCopy(&kernelContext->uniforms[3].uniform, constantData3, sizeof(constantData3));
            kernelContext->uniforms[3].index = 7;
            kernelContext->uniforms[3].num = vxmLENGTH_OF(constantData3);

            gcoOS_MemCopy(&kernelContext->uniforms[4].uniform, constantData4, sizeof(constantData4));
            kernelContext->uniforms[4].index = 8;
            kernelContext->uniforms[4].num = vxmLENGTH_OF(constantData4);

            if (outputFormat == VX_DF_IMAGE_RGBX)
            {
                kernelContext->uniform_num = 5;
                kernelContext->params.outputFormat = gcvSURF_R8G8B8X8;
            }
            else
            {
                gcoOS_MemCopy(&kernelContext->uniforms[5].uniform, constantData5, sizeof(constantData5));
                kernelContext->uniforms[5].index = 9;
                kernelContext->uniforms[5].num = vxmLENGTH_OF(constantData5);

                gcoOS_MemCopy(&kernelContext->uniforms[6].uniform, constantData6, sizeof(constantData6));
                kernelContext->uniforms[6].index = 10;
                kernelContext->uniforms[6].num = vxmLENGTH_OF(constantData6);

                kernelContext->uniform_num = 7;
                kernelContext->params.outputMultipleWidth = 3;
                kernelContext->params.outputFormat = gcvSURF_R8G8B8;
            }
        }
        else if (outputFormat == VX_DF_IMAGE_YUV4 || outputFormat == VX_DF_IMAGE_IYUV)
        {
            /*for VX_DF_IMAGE_YUV4*/
            vx_uint8 dataNV12_0[16] = {0, 0, 16, 16, 32, 32, 48, 48, 8, 8, 8, 8, 8, 8, 8, 8};
            vx_uint8 dataNV12_1[16] = {64, 64, 80, 80, 96, 96, 112, 112, 8, 8, 8, 8, 8, 8, 8, 8};
            vx_uint8 dataNV12_2[16] = {8, 8, 24, 24, 40, 40, 56, 56, 8, 8, 8, 8, 8, 8, 8, 8};
            vx_uint8 dataNV12_3[16] = {72, 72, 88, 88, 104, 104, 120, 120, 8, 8, 8, 8, 8, 8, 8, 8};

            vx_uint8 dataNV21_0[16] = {8, 8, 24, 24, 40, 40, 56, 56, 8, 8, 8, 8, 8, 8, 8, 8};
            vx_uint8 dataNV21_1[16] = {72, 72, 88, 88, 104, 104, 120, 120, 8, 8, 8, 8, 8, 8, 8, 8};
            vx_uint8 dataNV21_2[16] = {0, 0, 16, 16, 32, 32, 48, 48, 8, 8, 8, 8, 8, 8, 8, 8};
            vx_uint8 dataNV21_3[16] = {64, 64, 80, 80, 96, 96, 112, 112, 8, 8, 8, 8, 8, 8, 8, 8};

            /*for VX_DF_IMAGE_IYUV*/
            vx_uint8 dataNV12_IYUV0[16] = {0, 16, 32, 48, 64, 80, 96, 112, 8, 8, 8, 8, 8, 8, 8, 8}; /*get u*/
            vx_uint8 dataNV12_IYUV1[16] = {8, 24, 40, 56, 72, 88, 104, 120, 8, 8, 8, 8, 8, 8, 8, 8}; /*get v*/

            vx_uint8 constantData0[16] = {0};
            vx_uint8 constantData1[16] = {0};
            vx_uint8 constantData2[16] = {0};
            vx_uint8 constantData3[16] = {0};

            /*index = 0*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

            /*index = 1*/
            gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, output, 3);

            kernelContext->params.kernel = gcvVX_KERNEL_COLOR_CONVERT;

            if (inputFormat == VX_DF_IMAGE_NV12)
            {
                if (outputFormat == VX_DF_IMAGE_YUV4)
                {
                    gcoOS_MemCopy(constantData0, dataNV12_0, sizeof(constantData0));
                    gcoOS_MemCopy(constantData1, dataNV12_1, sizeof(constantData1));
                    gcoOS_MemCopy(constantData2, dataNV12_2, sizeof(constantData2));
                    gcoOS_MemCopy(constantData3, dataNV12_3, sizeof(constantData3));
                }
                else
                {
                    gcoOS_MemCopy(constantData0, dataNV12_IYUV0, sizeof(constantData0));
                    gcoOS_MemCopy(constantData1, dataNV12_IYUV1, sizeof(constantData1));
                }
                kernelContext->params.inputFormat = gcvSURF_NV12;
            }
            else
            {
                if (outputFormat == VX_DF_IMAGE_YUV4)
                {
                    gcoOS_MemCopy(constantData0, dataNV21_0, sizeof(constantData0));
                    gcoOS_MemCopy(constantData1, dataNV21_1, sizeof(constantData1));
                    gcoOS_MemCopy(constantData2, dataNV21_2, sizeof(constantData2));
                    gcoOS_MemCopy(constantData3, dataNV21_3, sizeof(constantData3));
                }
                else
                {
                    gcoOS_MemCopy(constantData0, dataNV12_IYUV1, sizeof(constantData0));
                    gcoOS_MemCopy(constantData1, dataNV12_IYUV0, sizeof(constantData1));
                }
                kernelContext->params.inputFormat = gcvSURF_NV21;
            }

            if (outputFormat == VX_DF_IMAGE_YUV4)
            {
                kernelContext->params.xstep = 16;
                kernelContext->params.outputFormat = gcvSURF_AYUV;
            }
            else
            {
                if (inputWidth % 16 != 0)
                {
                    kernelContext->params.xstep = 8;
                }
                else
                {
                    kernelContext->params.xstep = 16;
                }
                kernelContext->params.outputFormat = gcvSURF_I420;
            }

            gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, constantData0, sizeof(constantData0));
            kernelContext->uniforms[0].index = 6;
            kernelContext->uniforms[0].num = vxmLENGTH_OF(constantData0);

            gcoOS_MemCopy(&kernelContext->uniforms[1].uniform, constantData1, sizeof(constantData1));
            kernelContext->uniforms[1].index = 7;
            kernelContext->uniforms[1].num = vxmLENGTH_OF(constantData1);

            if (outputFormat == VX_DF_IMAGE_YUV4)
            {
                gcoOS_MemCopy(&kernelContext->uniforms[2].uniform, constantData2, sizeof(constantData2));
                kernelContext->uniforms[2].index = 8;
                kernelContext->uniforms[2].num = vxmLENGTH_OF(constantData2);

                gcoOS_MemCopy(&kernelContext->uniforms[3].uniform, constantData3, sizeof(constantData3));
                kernelContext->uniforms[3].index = 9;
                kernelContext->uniforms[3].num = vxmLENGTH_OF(constantData3);

                kernelContext->uniform_num = 4;
            }
            else
            {
                kernelContext->uniform_num = 2;
            }
        }
    }

    kernelContext->node = node;

    status = gcfVX_Kernel(kernelContext);

#if gcdVX_OPTIMIZER
    if (!node || !node->kernelContext)
    {
        vxFree(kernelContext);
    }
#endif

    return status;
}

