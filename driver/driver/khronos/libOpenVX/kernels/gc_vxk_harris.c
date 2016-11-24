/****************************************************************************
*
*    Copyright (c) 2005 - 2016 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include <gc_vxk_common.h>

#if VIV_HARRIS_SCORE
vx_status vxHarrisScore(vx_node node, vx_image grad_x, vx_image grad_y, vx_image dst,
                        vx_scalar scales, vx_scalar winds, vx_scalar sens, vx_border_mode_t borders)
{
    vx_status status = VX_SUCCESS;
    if (borders.mode == VX_BORDER_MODE_UNDEFINED)
    {
        vx_uint32 block_size = 0, i = 0;
        vx_float32 k = 0.0f;
        vx_uint32 width = 0, height = 0, scale = 0;
        vx_float64 s;
        gcoVX_Index indexs[]            = {
            /* index, num, shift0, shift1, mask0, mask1 */
            {    3, 4 * 4, {             1, 0, 0xffffffff, 0xfffffffe  }  }, /* y start */
            {    4, 4 * 4, {             1, 0, 0xffffffff, 0xfffffffe  }  }, /* y end */
            {    5, 4 * 4, {             1, 0, 0xffffffff, 0xfffffffe  }  }, /* x start */
            {    6, 4 * 4, {             1, 0, 0xffffffff, 0xfffffffe  }  }, /* x end */
        };
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

        status |= vxReadScalarValue(winds, &block_size);
        status |= vxReadScalarValue(sens, &k);
        status |= vxReadScalarValue(scales, &scale);

        s = (1 / ((1 << (scale - 1)) * block_size * 255.0));

        s = s * s * s * s;

        status |= vxQueryImage(grad_x, VX_IMAGE_ATTRIBUTE_WIDTH, &width, sizeof(width));
        status |= vxQueryImage(grad_x, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));

        /*index = 0*/
        gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, grad_x, GC_VX_INDEX_AUTO);

        /*index = 1*/
        gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, grad_y, GC_VX_INDEX_AUTO);

        /*index = 2*/
        gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, dst, GC_VX_INDEX_AUTO);

        kernelContext->params.kernel               = gcvVX_KERNEL_HARRIS_CORNERS;

#if gcdVX_OPTIMIZER
        kernelContext->borders                     = VX_BORDER_MODE_UNDEFINED;
#else
        kernelContext->params.borders              = VX_BORDER_MODE_UNDEFINED;
#endif

        kernelContext->params.volume               = block_size;
        kernelContext->params.factor               = k;
        kernelContext->params.scale                = (vx_float32)s;

        kernelContext->params.col                  = width;
        kernelContext->params.row                  = height;

        kernelContext->params.xstep                = 4;

        kernelContext->uniform_num                 = sizeof(indexs)/sizeof(indexs[0]);

        for(i = 0; i < kernelContext->uniform_num; i++)
        {
                                /* y start, y end, x start, x end */
            vx_int32 base[4] = {block_size/2 + 1, height - (block_size/2 + 1), (block_size/2 + 1), width - (block_size/2 + 1)};
            vx_int32 offset = (i>1)?1:0;
            indexs[i].bin[0] = base[i];                                  /* y start */
            indexs[i].bin[1] = indexs[i].bin[0] - offset;                /* y end */
            indexs[i].bin[2] = ((vx_int32)indexs[i].bin[1] - offset);    /* x start */
            indexs[i].bin[3] = ((vx_int32)indexs[i].bin[2] - offset);    /* x end */

            gcoOS_MemCopy(&kernelContext->uniforms[i].uniform, indexs[i].bin, sizeof(indexs[i].bin));
            kernelContext->uniforms[i].num = indexs[i].num;
            kernelContext->uniforms[i].index = indexs[i].index;
        }

        kernelContext->node = node;

        status = gcfVX_Kernel(kernelContext);

#if gcdVX_OPTIMIZER
        if (!node || !node->kernelContext)
        {
            vxFree(kernelContext);
        }
#endif

    }
    else
    {
        status = VX_ERROR_NOT_IMPLEMENTED;
    }

    return status;
}
#endif

#define IMGLST_FIND 0
#define IMGLST_PACK 1
vx_status vxPackArrays(vx_node node, vx_image inputImage, vx_array inputArray, vx_scalar widthScalar, vx_scalar heightScalar, vx_size itemSize, vx_size cap, vx_array outputArray, vx_scalar num)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 bin[4];
    vx_uint32 numCorners  = 0;
    void *base = NULL;
    gcoVX_Kernel_Context * kernelContext = gcvNULL;
    vx_uint32 width, height;

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

    vxReadScalarValue(widthScalar, &width);
    vxReadScalarValue(heightScalar, &height);

    /*index = 0*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, inputImage, GC_VX_INDEX_AUTO);

    if (inputArray && outputArray)
    {
        /*index = 1*/
        gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_ARRAY, inputArray, GC_VX_INDEX_AUTO);
        /*index = 2*/
        gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_ARRAY, outputArray, GC_VX_INDEX_AUTO);

        bin[0] = (vx_uint32)(itemSize * cap);
        gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, bin, sizeof(bin));
        kernelContext->uniforms[0].index       = 3;
        kernelContext->uniforms[0].num         = 16;
        kernelContext->uniform_num             = 1;
    }

    kernelContext->params.kernel           = gcvVX_KERNEL_IMAGE_LISTER;
    kernelContext->params.step             = IMGLST_PACK;
    kernelContext->params.xstep            = width;
    kernelContext->params.ystep            = 1;
    kernelContext->params.volume           = (vx_uint32)itemSize;
    kernelContext->params.clamp            = (vx_uint32)itemSize * width;
    kernelContext->params.col              = height;

    kernelContext->node = node;

    status = gcfVX_Kernel(kernelContext);

    status = gcfVX_Flush(gcvTRUE);
    base = inputImage->memory.logicals[0];
    numCorners = *((vx_uint32*)(base) + height - 1);

    if(outputArray)
    {
        if (numCorners > cap)
        {
            outputArray->itemCount = cap;
        }
        else
        {
            outputArray->itemCount = numCorners;
        }
    }

    if (num)
    {
        status = vxWriteScalarValue(num, &numCorners);
    }

#if gcdVX_OPTIMIZER
    if (!node || !node->kernelContext)
    {
        vxFree(kernelContext);
    }
#endif

    return status;
}

vx_status vxCreateLister(vx_node node, vx_image src, vx_image countImg, vx_array tempArray, vx_int32 width, vx_uint32 height, vx_size itemSize)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 bin[4];
    vx_uint32 constantData[8] = {0, 8, 16, 24, 0, 0, 0, 0};
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

    /*index = 0*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, src, GC_VX_INDEX_AUTO);
    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, countImg, GC_VX_INDEX_AUTO);
    if (tempArray)
    {
        /*index = 2*/
        gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_ARRAY, tempArray, GC_VX_INDEX_AUTO);
    }

    bin[0] = width;
    bin[1] = width-1;
    bin[2] = width-3;
    bin[3] = height-3;
    gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, bin, sizeof(bin));
    kernelContext->uniforms[0].index       = 3;
    kernelContext->uniforms[0].num         = 16;
    gcoOS_MemCopy(&kernelContext->uniforms[1].uniform, constantData, sizeof(constantData));
    kernelContext->uniforms[1].index       = 4;
    kernelContext->uniforms[1].num         = sizeof(constantData) / sizeof(vx_uint32);
    kernelContext->uniform_num             = 2;

    kernelContext->params.kernel           = gcvVX_KERNEL_IMAGE_LISTER;
    kernelContext->params.step             = IMGLST_FIND;
    kernelContext->params.xstep            = width;
    kernelContext->params.ystep            = 1;
    kernelContext->params.volume           = (vx_uint32)itemSize;
    kernelContext->params.clamp            = (vx_uint32)itemSize * width;

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

