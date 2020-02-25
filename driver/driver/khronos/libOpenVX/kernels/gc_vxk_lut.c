/****************************************************************************
*
*    Copyright (c) 2005 - 2019 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include <gc_vxk_common.h>

vx_status vxTableLookup(vx_node node, vx_image src, vx_lut lut, vx_image dst)
{
    vx_status status = VX_SUCCESS;

    gcoVX_Kernel_Context * kernelContext = gcvNULL;
    vx_df_image inputFormat = 0;

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

    vxQueryImage(src, VX_IMAGE_FORMAT, &inputFormat, sizeof(vx_df_image));

    /*index = 0*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, src, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_LUT, lut, GC_VX_INDEX_AUTO);

    /*index = 2*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, dst, GC_VX_INDEX_AUTO);

    if (inputFormat == VX_DF_IMAGE_U8)
    {
        vx_uint32 constantData[8] = {0, 8, 16, 24, 0, 0, 0, 0};
        vx_uint8 bin[16] = {0, 32, 64, 96, 0, 0, 0, 0, 8, 8, 8, 8, 0, 0, 0, 0};

        gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, constantData, sizeof(constantData));
        kernelContext->uniforms[0].index       = 3;
        kernelContext->uniforms[0].num         = vxmLENGTH_OF(constantData);

        gcoOS_MemCopy(&kernelContext->uniforms[1].uniform, bin, sizeof(bin));
        kernelContext->uniforms[1].index = 4;
        kernelContext->uniforms[1].num = vxmLENGTH_OF(bin);
        kernelContext->uniform_num = 2;

        kernelContext->params.kernel = gcvVX_KERNEL_TABLE_LOOKUP;
        kernelContext->params.xstep = 16;
    }
    else
    {
        vx_uint8 constantData0[16] = {0, 8, 0, 0, 16, 24, 0, 0, 8, 8, 0, 0, 8, 8, 0, 0};
        vx_uint8 constantData1[16] = {32, 40, 0, 0, 48, 56, 0, 0, 8, 8, 0, 0, 8, 8, 0, 0};
        vx_uint8 constantData2[16] = {64, 72, 0, 0, 80, 88, 0, 0, 8, 8, 0, 0, 8, 8, 0, 0};
        vx_uint8 constantData3[16] = {96, 104, 0, 0, 112, 120, 0, 0, 8, 8, 0, 0, 8, 8, 0, 0};
        vx_uint8 bin[16] = {0, 8, 32, 40, 64, 72, 96, 104, 8, 8, 8, 8, 8, 8, 8, 8};

        gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, constantData0, sizeof(constantData0));
        kernelContext->uniforms[0].index       = 3;
        kernelContext->uniforms[0].num         = vxmLENGTH_OF(constantData0);

        gcoOS_MemCopy(&kernelContext->uniforms[1].uniform, constantData1, sizeof(constantData1));
        kernelContext->uniforms[1].index       = 4;
        kernelContext->uniforms[1].num         = vxmLENGTH_OF(constantData1);

        gcoOS_MemCopy(&kernelContext->uniforms[2].uniform, constantData2, sizeof(constantData2));
        kernelContext->uniforms[2].index       = 5;
        kernelContext->uniforms[2].num         = vxmLENGTH_OF(constantData2);

        gcoOS_MemCopy(&kernelContext->uniforms[3].uniform, constantData3, sizeof(constantData3));
        kernelContext->uniforms[3].index       = 6;
        kernelContext->uniforms[3].num         = vxmLENGTH_OF(constantData3);

        gcoOS_MemCopy(&kernelContext->uniforms[4].uniform, bin, sizeof(bin));
        kernelContext->uniforms[4].index = 7;
        kernelContext->uniforms[4].num = vxmLENGTH_OF(bin);
        kernelContext->uniform_num = 5;

        kernelContext->params.xstep = 8;
    }

    kernelContext->node = node;

    kernelContext->params.kernel = gcvVX_KERNEL_TABLE_LOOKUP;

    status = gcfVX_Kernel(kernelContext);

#if gcdVX_OPTIMIZER
    if (!node || !node->kernelContext)
    {
        vxFree(kernelContext);
    }
#endif

    return status;
}

