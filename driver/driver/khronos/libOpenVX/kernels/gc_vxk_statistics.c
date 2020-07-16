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

vx_status vxCopyImage(vx_node node, vx_image src, vx_image dst)
{
    vx_status status = VX_SUCCESS;
    gcoVX_Kernel_Context * kernelContext = gcvNULL;
    vx_df_image format;

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
    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));
    /*index = 0*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, src, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, dst, GC_VX_INDEX_AUTO);

    kernelContext->params.kernel = gcvVX_KERNEL_COPY_IMAGE;
    if (format == VX_DF_IMAGE_S16 || format == VX_DF_IMAGE_U16)
        kernelContext->params.xstep = 8;
    else if (format == VX_DF_IMAGE_U8)
        kernelContext->params.xstep = 16;
    else if (format == VX_DF_IMAGE_U32 || format == VX_DF_IMAGE_S32)
        kernelContext->params.xstep = 4;
    else
        return VX_ERROR_INVALID_PARAMETERS;

    kernelContext->params.clamp = vx_false_e;
    kernelContext->params.evisNoInst = node->base.context->evisNoInst;


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

static vx_status vxVivMeanStdDev(vx_node node, vx_image input, vx_scalar mean, vx_scalar stddev)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 inputWidth = 0;
    vx_uint32 inputHeight = 0;
    vx_uint64 maxSize = 0;
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

    vxQueryImage(input, VX_IMAGE_WIDTH, &inputWidth, sizeof(vx_uint32));

    vxQueryImage(input, VX_IMAGE_HEIGHT, &inputHeight, sizeof(vx_uint32));

    /*index = 0*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_SCALAR, mean, GC_VX_INDEX_AUTO);

    /* index = 2 */
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_SCALAR, stddev, GC_VX_INDEX_AUTO);

    kernelContext->params.kernel = gcvVX_KERNEL_MEAN_STDDEV;
    kernelContext->params.xstep  = 16;

    maxSize = (vx_uint64)inputWidth * inputHeight * 255 * 255;

    /*check size of sqsum is larger than 32-bit, then do shift into 32-bit*/
    if (maxSize >= 0xffffffff)
    {
        kernelContext->params.ystep        = 255;
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

vx_status vxMeanStdDev(vx_node node, vx_image input, vx_scalar mean, vx_scalar stddev)
{
    vx_status status  = VX_SUCCESS;
    vx_float32 fmean = 0.0f, fstddev = 0.0f, npix = 0.0f;
    vx_uint64 sum = 0, sqsum = 0, maxsize;
    vx_imagepatch_addressing_t addrs;
    void *base_ptr = NULL;
    vx_rectangle_t rect ;
    vx_uint32 width = 0, height = 0;

    /* clean init value */
    status |= vxWriteScalarValue(mean, &fmean);
    status |= vxWriteScalarValue(stddev, &fstddev);

    status |= vxGetValidRegionImage(input, &rect);
    status |= vxAccessImagePatch(input, &rect, 0, &addrs, &base_ptr, VX_READ_ONLY);

    status |= vxVivMeanStdDev(node, input, mean, stddev);
    /* To Clean up */
    status |= gcfVX_Flush(gcvTRUE);

    status |= vxReadScalarValue(mean, &sum);
    status |= vxReadScalarValue(stddev, &sqsum);

    width = rect.end_x - rect.start_x;
    height = rect.end_y - rect.start_x;

    maxsize = (vx_uint64)width * height * 255 * 255;

    /*check size of sqsum is larger than 32-bit*/
    if (maxsize >= 0xffffffff)
    {
        sum = sum << 1;
        sqsum = sqsum << 1;
    }

    /*
     * fmean = sum / (width * height)
     * fstddev = sqsum/ (width * height) - (fmean * fmean)
     */

    npix = addrs.dim_x*addrs.dim_y * 1.0f;
    fmean = (sum)/npix;

    fstddev = sqsum/npix - fmean * fmean;
    fstddev = (fstddev < 0) ? 0 : (vx_float32)sqrt(fstddev);

    status |= vxWriteScalarValue(mean, &fmean);
    status |= vxWriteScalarValue(stddev, &fstddev);
    status |= vxCommitImagePatch(input, &rect, 0, &addrs, base_ptr);

    return status;
}

#define MML_FILTER 0
#define MML_LOC   1
#define MML_PACK   2

vx_status vxMinMaxPackLocation(vx_node node, vx_image inputImage, vx_array inputArray, vx_scalar widthScalar, vx_scalar heightScalar, vx_scalar countScalar, vx_size itemSize, vx_size cap, vx_array outputArray)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 bin[4];
    gcoVX_Kernel_Context * kernelContext = gcvNULL;
    vx_uint32 width, height;
    vx_uint32 count = 0;

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
        bin[1] = (vx_uint32)(itemSize * height * width);
        gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, bin, sizeof(bin));
        kernelContext->uniforms[0].index       = 3;
        kernelContext->uniforms[0].num         = 16;
        kernelContext->uniform_num             = 1;
    }

    kernelContext->params.kernel           = gcvVX_KERNEL_MINMAXLOC;
    kernelContext->params.step             = MML_PACK;
    kernelContext->params.xstep            = width;
    kernelContext->params.ystep            = 1;
    kernelContext->params.volume           = (vx_uint32)itemSize;
    kernelContext->params.clamp            = (vx_uint32)itemSize * width;
    kernelContext->params.col              = height;

    kernelContext->node = node;

    status = gcfVX_Kernel(kernelContext);

    status = gcfVX_Flush(gcvTRUE);

    status |= vxReadScalarValue(countScalar, &count);

    if (outputArray)
    {
        if (count > cap)
        {
            outputArray->itemCount = cap;
        }
        else
        {
            outputArray->itemCount = count;
        }
    }

#if gcdVX_OPTIMIZER
    if (!node || !node->kernelContext)
    {
        vxFree(kernelContext);
    }
#endif

    return status;
}

vx_status vxMinMaxGetLocation(vx_node node, vx_image img, vx_scalar minVal, vx_scalar maxVal, vx_df_image format, vx_image minImg, vx_image maxImg,
                             vx_scalar minCount, vx_scalar maxCount, vx_array minArray, vx_array maxArray)
{
    vx_status status  = VX_SUCCESS;
    vx_int32 width, height;
    vx_size itemSize = 0;
    vx_uint32 bin[4];
    vx_uint32 constantData[8] = {0, 8, 16, 24, 0, 0, 0, 0};
    vx_int32 minValue = 0, maxValue = 0;
    vx_bool return_loc = vx_false_e;
    vx_int32 count = 0;
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

    vxQueryImage(img, VX_IMAGE_HEIGHT, &height, sizeof(height));
    vxQueryImage(img, VX_IMAGE_WIDTH, &width, sizeof(width));
    if(minArray && maxArray)
    {
        vxQueryArray(minArray, VX_ARRAY_ITEMSIZE, &itemSize, sizeof(itemSize));
        return_loc = vx_true_e;
    }

    vxWriteScalarValue(minCount, &count);
    vxWriteScalarValue(maxCount, &count);

    /*index = 0*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, img, GC_VX_INDEX_AUTO);
    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, minImg, GC_VX_INDEX_AUTO);
    /*index = 2*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, maxImg, GC_VX_INDEX_AUTO);
    /*index = 3*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_SCALAR, minCount, GC_VX_INDEX_AUTO);
    /*index = 4*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_SCALAR, maxCount, GC_VX_INDEX_AUTO);

    if (minArray && maxArray)
    {
        /*index = 5*/
        gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_ARRAY, minArray, GC_VX_INDEX_AUTO);
        /*index = 6*/
        gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_ARRAY, maxArray, GC_VX_INDEX_AUTO);
    }

    vxReadScalarValue(minVal, &minValue);
    vxReadScalarValue(maxVal, &maxValue);

    bin[0] = (format == VX_DF_IMAGE_S16) ? FV2(0xffff & minValue) : FV((vx_uint8)minValue);
    bin[1] = (format == VX_DF_IMAGE_S16) ? FV2(1) : FV(1);
    bin[2] = (format == VX_DF_IMAGE_S16) ? FV2(0xffff & maxValue) : FV((vx_uint8)maxValue);
    bin[3] = width;
    gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, bin, sizeof(bin));
    kernelContext->uniforms[0].index       = 7;
    kernelContext->uniforms[0].num         = 16;

    if (format == VX_DF_IMAGE_S16)
    {
        constantData[1] = 16;
        constantData[2] = 32;
        constantData[3] = 32;
    }

    gcoOS_MemCopy(&kernelContext->uniforms[1].uniform, constantData, sizeof(constantData));
    kernelContext->uniforms[1].index       = 8;
    kernelContext->uniforms[1].num         = vxmLENGTH_OF(constantData);
    kernelContext->uniform_num             = 2;


    kernelContext->params.kernel           = gcvVX_KERNEL_MINMAXLOC;
    kernelContext->params.step             = MML_LOC;
    kernelContext->params.xstep            = width;
    kernelContext->params.ystep            = 1;
    kernelContext->params.volume           = (vx_uint32)itemSize;
    kernelContext->params.clamp            = (vx_uint32)itemSize * width;
    kernelContext->params.policy           = return_loc;
    kernelContext->params.col              = height;

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

vx_status vxMinMaxLocFilter(vx_node node, vx_image input, vx_scalar filter_min, vx_scalar filter_max)
{
    vx_status                status = VX_SUCCESS;
    gcoVX_Kernel_Context * kernelContext = gcvNULL;
    vx_int32 height;
    vx_df_image format;

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

    vxQueryImage(input, VX_IMAGE_HEIGHT, &height, sizeof(height));
    vxQueryImage(input, VX_IMAGE_FORMAT, &format, sizeof(format));

    /*index = 0*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, input, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_SCALAR, filter_min, GC_VX_INDEX_AUTO);

    /* index = 2 */
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_SCALAR, filter_max, GC_VX_INDEX_AUTO);

    kernelContext->params.step             = MML_FILTER;
#if gcdVX_OPTIMIZER
    kernelContext->borders                 = VX_BORDER_REPLICATE;
#else
    kernelContext->params.borders          = VX_BORDER_REPLICATE;
#endif
    kernelContext->params.kernel           = gcvVX_KERNEL_MINMAXLOC;
    if (node->base.context->evisNoInst.noFilter)
    {
        vx_int32 data[4];
        data[0] = (format == VX_DF_IMAGE_S16) ? FV2(1) : FV(1);
        data[1] = (format == VX_DF_IMAGE_S16)? 0x7fff: 0xff;
        data[2] = (format == VX_DF_IMAGE_S16)? -0x7fff: 0;
        data[3] = height;

        kernelContext->params.xstep            = (format == VX_DF_IMAGE_S16) ? 8 : 16;

        gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, data, sizeof(data));
        kernelContext->uniforms[0].num = 4 * 4;
        kernelContext->uniforms[0].index = 3;
        kernelContext->uniform_num = 1;

        kernelContext->params.evisNoInst = node->base.context->evisNoInst;
    }
    else
    {
        if (format != VX_DF_IMAGE_S16)
        {
            gctUINT32 i = 0;
            gcoVX_Index indexs[]                = {
                /* index, num, shift0, shift1, mask0, mask1 */
                {    3, 4 * 4, {FV4(3*8,(16+3)*8,0,0), FV4(0,0,0, 0), FV4(4*8,4*8,0,0), FV4(0,0,0,0)}  }, /*  */
                {    4, 4 * 4, {FV4(6*8,(16+6)*8,0,0), FV4(0,0,0, 0), FV4(4*8,4*8,0,0), FV4(0,0,0,0)}  }, /*  */
            };

            kernelContext->uniform_num             = vxmLENGTH_OF(indexs);

            for(i = 0; i < kernelContext->uniform_num; i++)
            {
                gcoOS_MemCopy(&kernelContext->uniforms[i].uniform, indexs[i].bin, sizeof(indexs[i].bin));
                kernelContext->uniforms[i].num = indexs[i].num;
                kernelContext->uniforms[i].index = indexs[i].index;
            }
        }
        else
        {
            gctUINT32 i = 0;
            gcoVX_Index indexs[]                = {
                /* index, num, shift0, shift1, mask0, mask1 */
                {    3, 4 * 4, {FV4(4*8,(16+4)*8,0,0), FV4(0,0,0, 0), FV4(4*8,4*8,0,0), FV4(0,0,0,0)}  }, /*  */
                {    4, 4 * 4, {FV4(6*8,(16+6)*8,0,0), FV4(0,0,0, 0), FV4(4*8,4*8,0,0), FV4(0,0,0,0)}  }, /*  */
            };

            kernelContext->uniform_num             = vxmLENGTH_OF(indexs);

            for(i = 0; i < kernelContext->uniform_num; i++)
            {
                gcoOS_MemCopy(&kernelContext->uniforms[i].uniform, indexs[i].bin, sizeof(indexs[i].bin));
                kernelContext->uniforms[i].num = indexs[i].num;
                kernelContext->uniforms[i].index = indexs[i].index;
            }
        }
        kernelContext->params.xstep            = (format == VX_DF_IMAGE_S16) ? 6 : 14;;
    }
    kernelContext->params.ystep            = height;

    kernelContext->params.evisNoInst = node->base.context->evisNoInst;

    kernelContext->node = node;

    status = gcfVX_Kernel(kernelContext);

    status |= gcfVX_Flush(gcvTRUE);

#if gcdVX_OPTIMIZER
    if (!node || !node->kernelContext)
    {
        vxFree(kernelContext);
    }
#endif

    return status;
}

