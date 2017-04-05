/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include <gc_vxk_common.h>

/*#define COMPARE_TO_REF*/
#define TP_SCALING      0

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

extern vx_int16   F32toF16(vx_float32 val);
extern vx_float32 Fp16toFp32(const vx_int16 in);
extern vx_uint16  Fp32toFp16(const vx_float32 in);
extern void fillInCmmdBuffer(void *inputInfo, vx_array cmdBuffer, vx_bool isTP);

vx_status _gcfVX_Morphology(vx_node node, gceVX_KERNEL kernel, vx_image src, vx_image dst, vx_border_mode_t *bordermode)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 height;

    gcoVX_Kernel_Context * kernelContext = gcvNULL;
    vx_df_image inputFormat;

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

    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_HEIGHT, &height, sizeof(height));
    vxQueryImage(src, VX_IMAGE_ATTRIBUTE_FORMAT, &inputFormat, sizeof(inputFormat));

    /*index = 0*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_INPUT, src, GC_VX_INDEX_AUTO);

    /*index = 1*/
    gcoVX_AddObject(kernelContext, GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT, dst, GC_VX_INDEX_AUTO);

    kernelContext->params.kernel       = kernel;

    if (node->base.context->evisNoInst.isVX2)
    {
        if (inputFormat == VX_DF_IMAGE_S16 || inputFormat == VX_DF_IMAGE_U16)
            kernelContext->params.xstep        =  6;
        else
            kernelContext->params.xstep        =  14;
    }
    else if (node->base.context->evisNoInst.noFilter)
    {
        kernelContext->params.xstep        = 6;
    }
    else
    {
        kernelContext->params.xstep        = 8;
    }
    kernelContext->params.ystep        = height;
#if gcdVX_OPTIMIZER
    kernelContext->borders             = bordermode->mode;
#else
    kernelContext->params.borders      = bordermode->mode;
#endif

    if (node->base.context->evisNoInst.noFilter)
    {
        vx_uint32 rect[2];
        rect[0] = height;
        rect[1] = FV(1);
        gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, rect, sizeof(rect));
        kernelContext->uniforms[0].index       = 2;
        kernelContext->uniforms[0].num         = 4 * 2;
    }
    else
    {
        vx_uint32 rect[1];
        rect[0] = height;
        gcoOS_MemCopy(&kernelContext->uniforms[0].uniform, rect, sizeof(rect));
        kernelContext->uniforms[0].index       = 2;
        kernelContext->uniforms[0].num         = 4;
    }

    kernelContext->uniform_num             = 1;

    if(bordermode->mode == VX_BORDER_MODE_CONSTANT)
    {
        vx_uint32 bin[4];

        bin[0] =
        bin[1] =
        bin[2] =
        bin[3] = FORMAT_VALUE(bordermode->constant_value);

        gcoOS_MemCopy(&kernelContext->uniforms[1].uniform, bin, sizeof(bin));
        kernelContext->uniforms[1].num = 4 * 4;
        kernelContext->uniforms[1].index = 3;
        kernelContext->uniform_num = 2;
    }

    if (node->base.context->evisNoInst.noFilter)
    {
        vx_uint8 bin[16] = {0, 32, 64, 96, 0, 0, 0, 0, 8, 8, 8, 8, 0, 0, 0, 0};
        gcoOS_MemCopy(&kernelContext->uniforms[2].uniform, bin, sizeof(bin));
        kernelContext->uniforms[2].num = sizeof(bin) / sizeof(vx_uint8);
        kernelContext->uniforms[2].index = 4;
        kernelContext->uniform_num = 3;

        kernelContext->evisNoInst = node->base.context->evisNoInst;
    }

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

/* nodeless version of the Erode3x3 kernel*/
vx_status vxErode3x3(vx_node node, vx_image src, vx_image dst, vx_border_mode_t *bordermode)
{
    return _gcfVX_Morphology(node, gcvVX_KERNEL_ERODE_3x3, src, dst, bordermode);
}

/* nodeless version of the Dilate3x3 kernel*/
vx_status vxDilate3x3(vx_node node, vx_image src, vx_image dst, vx_border_mode_t *bordermode)
{
    return _gcfVX_Morphology(node, gcvVX_KERNEL_DILATE_3x3, src, dst, bordermode);
}


#define FP32_MAX 3.402823466e+38F

static void AddObject(vx_uint32 index, vx_array src, gcoVX_Kernel_Context * kernelContext, vx_uint32 width, vx_int32 height, vx_uint32 shift)
{
    gctUINT32 data[4];
    gctUINT32_PTR p = &data[0];
    vx_nn_cmd_info_u info;

    info.vx_nn_image_cmd_info.width  = width;
    info.vx_nn_image_cmd_info.height = height;
    info.vx_nn_image_cmd_info.shift  = shift;
    info.vx_nn_image_cmd_info.physical = src->memory.physicals[0];

    gcoVX_SetNNImage((void*)&info, &p);

    gcoOS_MemCopy(&kernelContext->uniforms[kernelContext->uniform_num].uniform, data, sizeof(data));
    kernelContext->uniforms[kernelContext->uniform_num].index       = index;
    kernelContext->uniforms[kernelContext->uniform_num].num         = sizeof(data);
    kernelContext->uniform_num ++;
}

#define CPU_CACHE_MEORMY 0

vx_status vxMaxPool3x3(vx_node node, vx_array src, vx_scalar format, vx_scalar _width, vx_scalar _height, vx_scalar _depth, vx_scalar batch, vx_scalar _width_o, vx_scalar _height_o, vx_scalar kernel, vx_scalar stride, vx_scalar pad, vx_array dst)
{

    vx_status status = VX_SUCCESS;
    vx_int32 stride_v = 2, kernel_v = 3, pad_v = 0;
    vx_int32 width = 0, height = 0, depth = 0, b = 0, width_o = 0, height_o = 0;
    vx_enum format_v = VX_TYPE_FLOAT32;

#if defined(__linux__)
    struct timeval start = gcfVX_PerfStart((vx_reference)node);
#endif

    vxReadScalarValue(format, &format_v);
    vxReadScalarValue(_width, &width);
    vxReadScalarValue(_height, &height);
    vxReadScalarValue(_depth, &depth);
    vxReadScalarValue(batch, &b);
    vxReadScalarValue(_width_o, &width_o);
    vxReadScalarValue(_height_o,&height_o);
    vxReadScalarValue(kernel, &kernel_v);
    vxReadScalarValue(stride, &stride_v);
    vxReadScalarValue(pad, &pad_v);


#if NN_TP_ENGINE
    if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_TP_ENGINE) && 1)
    {
        vx_array            cmdBuffer;
        vx_tp_coomandInfo_s tpCommand;
        vx_context          context = vxGetContext((vx_reference)node);
        vx_uint32           outTileXsize = 32;
        vx_uint32           outTileYsize = 16;

        /* Fill in tpCommand */
        memset(&tpCommand, 0, sizeof(vx_tp_coomandInfo_s));
        tpCommand.inImageXSize = width;
        tpCommand.inImageYSize = height;
        tpCommand.inImageZSize = depth;
        tpCommand.inImageStride = width;
        tpCommand.inImageSlice = width * height;
        tpCommand.inWindowXStart = -pad_v;
        tpCommand.inWindowYStart = -pad_v;
        tpCommand.inWindowXEnd = (width_o - 1) * stride_v + kernel_v - stride_v;
        tpCommand.inWindowYEnd = (height_o - 1) * stride_v + kernel_v - stride_v;
        tpCommand.inTileSequence = 0x0;
        tpCommand.inImageGlobalMem = 1;
        tpCommand.inImageBaseAddress = src->memory.physicals[0];
        tpCommand.inTileXSize = outTileXsize * stride_v + kernel_v - stride_v;
        tpCommand.inTileYSize = outTileYsize * stride_v + kernel_v - stride_v;
        tpCommand.inTileXInc = outTileXsize * stride_v;
        tpCommand.inTileYInc = outTileYsize * stride_v;
        tpCommand.aluSquarePreshift = 0;
        tpCommand.aluSquareEnable = 0;
        tpCommand.aluHorzProcessing = 0x3;
        tpCommand.aluHorzProcCount = 2;
        tpCommand.aluHorzProcStride = 0x1;
        tpCommand.aluVertProcessing = 0x3;
        tpCommand.aluVertProcCount = 2;
        tpCommand.aluVertProcStride = 0x1;
        tpCommand.aluPwlEnable = 0;
        tpCommand.aluMultEnable = 0;
        tpCommand.aluLoadPwlLUT = 0;
        tpCommand.aluLoadPwlLUTAddress = 0;
        tpCommand.aluLoadPwlLUTGlobalMem = 1;
        tpCommand.outBaseAddress = dst->memory.physicals[0];
        tpCommand.outGlobalMem  = 1;
        tpCommand.outTileSkipAtborder = 0;
        tpCommand.outBrickMode  = 0;
        tpCommand.outLoop0Inc   = 0;
        tpCommand.outLoop0Count = 1;
        tpCommand.outLoop1Inc   = 1;
        tpCommand.outLoop1Count = 0;
        tpCommand.outLoop1Reset = 0x1;
        tpCommand.outLoop2Inc   = width_o;
        tpCommand.outLoop2Count = 0;
        tpCommand.outLoop2Reset = 0x1;
        tpCommand.outLoop3Inc   = outTileXsize;
        tpCommand.outLoop3Count = (width_o + outTileXsize - 1) / outTileXsize;
        tpCommand.outLoop3Reset = 0x0;
        tpCommand.outLoop4Inc   = outTileYsize * width_o;
        tpCommand.outLoop4Count = (height_o + outTileYsize - 1) / outTileYsize;
        tpCommand.outLoop5Inc   = 0;
        tpCommand.outLoop5Count = 1;
        tpCommand.outLoop6Inc   = width_o * height_o;
        tpCommand.last          = 1;

        /* Create tp command buffer*/
        cmdBuffer = vxCreateArray(context, VX_TYPE_CHAR, 128);
        if (vxoArray_AllocateMemory(cmdBuffer))
        {
            gctUINT32 cmdBufferAddress;

            fillInCmmdBuffer((void*)&tpCommand, cmdBuffer, vx_true_e);

            /* Execute tp command buffer. */
            cmdBufferAddress = (gctUINT32)cmdBuffer->memory.physicals[0];
            status = gcfVX_Accel((gctUINT32)cmdBufferAddress, gcvVX_ACCELERATOR_TP, 0, gcvFALSE);
        }
        else
        {
            status |= VX_ERROR_NO_MEMORY;
        }
        vxReleaseArray(&cmdBuffer);
    }
    else
#endif
    {
#if VX_SHADER_TP
    vx_int32 rect[4];

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

    AddObject(0, src, kernelContext, width, height * depth, 1);

    AddObject(1, dst, kernelContext, width_o, height_o * depth, 1);

    kernelContext->params.kernel       = gcvVX_KERNEL_MAXPOOL;

    kernelContext->params.borders       = VX_BORDER_MODE_CONSTANT;

    kernelContext->params.xstep        = 6;
    kernelContext->params.ystep        = height;
    kernelContext->params.xmax         = width;
    kernelContext->params.ymax         = height;
    kernelContext->params.col          = kernel_v;

    kernelContext->params.row           = pad_v;

    kernelContext->node = node;

    rect[0] = height;
    rect[1] = height * depth;
    rect[2] = -height_o;
    rect[3] = -1;
    gcoOS_MemCopy(&kernelContext->uniforms[kernelContext->uniform_num].uniform, rect, sizeof(rect));
    kernelContext->uniforms[kernelContext->uniform_num].index       = 2;
    kernelContext->uniforms[kernelContext->uniform_num++].num       = 4 * 4;

    rect[0] = 2;
    rect[1] = 1;
    rect[2] = 0xfc00fc00;
    rect[3] = height_o;
    gcoOS_MemCopy(&kernelContext->uniforms[kernelContext->uniform_num].uniform, rect, sizeof(rect));
    kernelContext->uniforms[kernelContext->uniform_num].index       = 3;
    kernelContext->uniforms[kernelContext->uniform_num++].num       = 4 * 4;

    {
        vx_uint32 bin[16] = {
                                0x00010101, 0x00000000, 0x00020000, 0x00060004, 0xaaaaaaaa, 0x00000000, 0x00000000, 0x00003300,
                                0x00000001, 0x00000000, 0x00000001, 0x00000000, 0x00000001, 0x00000000, 0x00000000, 0x00000000,
                            };

        gcoOS_MemCopy(&kernelContext->uniforms[kernelContext->uniform_num].uniform, bin, sizeof(bin));
        kernelContext->uniforms[kernelContext->uniform_num].num         = 16 * 4;
        kernelContext->uniforms[kernelContext->uniform_num].index       = 4;
        kernelContext->uniform_num ++;
    }

    kernelContext->params.evisNoInst = node->base.context->evisNoInst;

    status = gcfVX_Kernel(kernelContext);

    gcfVX_Flush(gcvTRUE);

#if gcdVX_OPTIMIZER
    if (!node || !node->kernelContext)
    {
        vxFree(kernelContext);
    }
#endif
#else

#if CPU_CACHE_MEORMY
    void *data_p = NULL, *data_dp = NULL;
#endif
    void *data = 0;
    void *data_d = 0;
    vx_int32 i = 0, j = 0, k = 0, p = 0;

#if defined(__linux__)
    start = gcfVX_PerfStart((vx_reference)node);
#endif

    src->itemCount = width * height * depth;
    dst->itemCount = width_o * height_o * depth;

#if CPU_CACHE_MEORMY
    data_p = data = (vx_float32*)malloc(height * width * depth * b * sizeof(vx_float32));
    data_dp = data_d = (vx_float32*)malloc(height_o * width_o * depth * b * sizeof(vx_float32));

    memcpy(data, src->memory.logicals[0], height * width * depth * b * sizeof(vx_float32));
    memcpy(data_d, dst->memory.logicals[0], height_o * width_o * depth * b * ((format_v == VX_TYPE_FLOAT32)?sizeof(vx_float32):sizeof(vx_int16)));
#else
    data = (vx_float32*)src->memory.logicals[0];
    data_d = (vx_float32*)dst->memory.logicals[0];
#endif

    switch(format_v)
    {
    case VX_TYPE_FLOAT32:
        {
            for (k = 0; k < b; k ++)
            {
                for (p = 0; p < depth; p ++)
                {
                    for (j = 0; j < height_o; j ++)
                    {
                        for (i = 0; i < width_o; i ++)
                        {
                            vx_int32 pad_h = pad_v, pad_w = pad_v;
                            vx_int32 hstart = j * stride_v - pad_h;
                            vx_int32 wstart = i * stride_v - pad_w;
                            vx_int32 hend = min(hstart + kernel_v, height);
                            vx_int32 wend = min(wstart + kernel_v, width);
                            vx_int32 pool_index = 0;
                            vx_int32 h, w = 0;
                            vx_float32 *d_f32;

                            hstart = max(hstart, 0);
                            wstart = max(wstart, 0);

                            pool_index = j * (width_o) + i;

                            d_f32 = (vx_float32*)data_d + pool_index;
                            *d_f32 = -FP32_MAX;
                            for (h = hstart; h < hend; ++ h)
                            {
                                for (w = wstart; w < wend; ++ w)
                                {
                                    const vx_int32 index = h * (width) + w;
                                    vx_float32 * d = (vx_float32*)data + index;
                                    if (*d > *d_f32)
                                    {
                                        *d_f32 = *d;
                                    }
                                }
                            }
                        }
                    }

                    data = (vx_float32*)(data) + width * height;
                    data_d = (vx_float32*)(data_d) + width_o * height_o;
                }
            }
        }
        break;
    case VX_TYPE_INT16:
        {
            for (k = 0; k < b; k ++)
            {
                for (p = 0; p < depth; p ++)
                {
                    for (j = 0; j < height_o; j ++)
                    {
                        for (i = 0; i < width_o; i ++)
                        {
                            vx_int32 pad_h = pad_v, pad_w = pad_v;
                            vx_int32 hstart = j * stride_v - pad_h;
                            vx_int32 wstart = i * stride_v - pad_w;
                            vx_int32 hend = min(hstart + kernel_v, height);
                            vx_int32 wend = min(wstart + kernel_v, width);
                            vx_int32 pool_index = 0;
                            vx_int32 h, w = 0;
                            vx_int16 *d_f16;

                            hstart = max(hstart, 0);
                            wstart = max(wstart, 0);

                            pool_index = j * (width_o) + i;
                            d_f16 = (vx_int16*)data_d + pool_index;
                            *d_f16 = 0xfc00;//F32toF16(-FP32_MAX);

                            for (h = hstart; h < hend; ++ h)
                            {
                                for (w = wstart; w < wend; ++ w)
                                {
                                    const vx_int32 index = h * (width) + w;
                                    vx_int16 *d = (vx_int16*)data + index;
                                    if ((*d) > (*d_f16))
                                    {
                                        *d_f16 = *d;
                                    }
                                }
                            }
                        }
                    }
                    data = (vx_int16*)(data) + width * height;
                    data_d = (vx_int16*)(data_d) + width_o * height_o;
                }
            }
        }
        break;
    }

#if CPU_CACHE_MEORMY
    memcpy(dst->memory.logicals[0], data_dp, height_o * width_o * depth * b * ((format_v == VX_TYPE_FLOAT32)?sizeof(vx_float32):sizeof(vx_int16)));
    free(data_p);
    free(data_dp);
#endif
#endif
    }


#ifdef COMPARE_TO_REF
    {
        vx_uint32 layerIndex = CURR_LAYER;
        FILE *outputFile = NULL;
        vx_uint32 itemCount = width_o * height_o * depth * b;
        vx_float32 *pfNN_out, *pfRef_out;
        vx_float32 fDelta, fMax_delta, fPortion_BigDelta;
        vx_uint32 i, count_big_delta, offset_max_delta;
        vx_char *fileName[] = { "fasterRcnn_resource\\000456-proposal\\layer3_pool1\\output_1_96_151_201.dat",
                                "fasterRcnn_resource\\000456-proposal\\layer7_pool2\\output_1_256_39_51.dat"};
        vx_char fullFileName[256] = {'\0'};
        pfNN_out =  (vx_float32*)malloc(itemCount * sizeof(vx_float32));
        pfRef_out = (vx_float32*)malloc(itemCount * sizeof(vx_float32));

        sprintf(fullFileName, "%s", fileName[layerIndex]);
        outputFile = fopen(fullFileName,"rb");            /* compare to ref output file of each layer */
        if (outputFile == NULL)
        {
            gcmPRINT("can't find file %s", fullFileName);
            return VX_ERROR_INVALID_PARAMETERS;
        }

        if(fread(pfRef_out, 1, itemCount*sizeof(vx_float32), outputFile) != itemCount*sizeof(vx_float32) )
        {
            gcmPRINT("fread failure");
            return VX_ERROR_INVALID_PARAMETERS;
        }
        fclose(outputFile);


        fMax_delta =0.0f;
        count_big_delta = 0;
        offset_max_delta = 0;
        for (i = 0; i < itemCount; i++)
        {
            pfNN_out[i] = Fp16toFp32(*((vx_int16*) dst->memory.logicals[0] + i));
            //pfNN_out[i] = *((vx_float32*) dst->memory.logicals[0] + i);
            if (pfRef_out[i] != 0.0f)
            {
                fDelta = (vx_float32)fabs((pfRef_out[i] - pfNN_out[i]) / pfRef_out[i]);
            }
            else if (pfNN_out[i] != 0.0f)
            {
                fDelta = (vx_float32)fabs((pfRef_out[i] - pfNN_out[i]) / pfNN_out[i]);
            }
            else
            {
                continue;
            }

            if (fDelta > 0.01f)
            {
                count_big_delta++;
            }
            if (fDelta > fMax_delta)
            {
                fMax_delta = fDelta;
                offset_max_delta = i;
            }
        }

        fPortion_BigDelta = (float)count_big_delta/(float)itemCount;
        printf("Portion_BigDelta = %9.7f, Max delta = %10.6f, count_big_delta= %8d \n", fPortion_BigDelta, fMax_delta, count_big_delta);
        free(pfRef_out);
        free(pfNN_out);
    }
#endif

    vxWriteScalarValue(format, &format_v);
    vxWriteScalarValue(_width, &width);
    vxWriteScalarValue(_height, &height);
    vxWriteScalarValue(_depth, &depth);
    vxWriteScalarValue(batch, &b);
    vxWriteScalarValue(_width_o, &width_o);
    vxWriteScalarValue(_height_o,&height_o);
    vxWriteScalarValue(kernel, &kernel_v);
    vxWriteScalarValue(stride, &stride_v);
    vxWriteScalarValue(pad, &pad_v);

#if defined(__linux__)
    if (node->base.context->perfEnable)
        printf("fast rcnn max pool        CPU  time:%10d us\n", gcfVX_PerfEnd((vx_reference)node, start));
#endif

#if VX_SHADER_TP
#if NN_MULTI_THREAD2
    if ((node->base.context->cnnEvent != VX_NULL))
        vxSetEvent(node->base.context->cnnEvent);
#endif
#endif

    return status;
}

#ifdef WIN32
static vx_float32 round(vx_float32 x)
{
    return (floorf((vx_float32)(fabs(x) + 0.5f)));
}
#endif

vx_status vxROIPool(vx_node node, vx_array input1, vx_array input2, vx_scalar kernel, vx_scalar stride, vx_scalar pad, vx_array dst)
{
    vx_status status = VX_SUCCESS;
#ifdef USE_REF_INPUT
    vx_int32 stride_w = 5;
#else
    vx_int32 stride_w = 4;
#endif
    vx_int32 stride_v = 2, kernel_v = 3, pad_v = 0;
    vx_float32 *rois = NULL, *data = NULL, *dest = NULL;
    vx_int32 num_rois = 256, width = 51, height = 39;
    vx_int32 pooled_width = 6, pooled_height = 6, channels = 256;
    vx_float32 spatial_scale_ = 0.0625;
    vx_int32 n, c, ph, pw, h, w;

#if defined(__linux__)
    struct timeval start = gcfVX_PerfStart((vx_reference)node);
#endif

    data = (vx_float32*)input1->memory.logicals[0];
    rois = (vx_float32*)input2->memory.logicals[0];
    dest = (vx_float32*)dst->memory.logicals[0];

    vxReadScalarValue(kernel, &kernel_v);
    vxReadScalarValue(stride, &stride_v);
    vxReadScalarValue(pad, &pad_v);


#if NN_TP_ENGINE
    if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_TP_ENGINE) && 1)
    {
        typedef struct _vxsTP_ROI_Pool
        {
#if NN_TP_NEW_ROI
            vx_uint32 xcoord        :  8;   // [7:0]
            vx_uint32 ycoord        :  8;   // [15:8]
            vx_uint32 last          :  1;   // [16]
            vx_uint32 _pad0         : 15;   // [31:17]
            vx_uint32 poolingHInc   : 14;   // [13:0]
            vx_uint32 _pad1         :  2;   // [15:14]
            vx_uint32 poolingVInc   : 14;   // [29:16]
            vx_uint32 _pad2         :  2;   // [31:30]
#else
            vx_uint32 poolingHsize :  6;   // [5:0]
            vx_uint32 hstride      :  1;   // [6]
            vx_uint32 poolingVsize :  6;   // [12:7]
            vx_uint32 vstride      :  1;   // [13]
            vx_uint32 last         :  1;   // [15]
            vx_uint32 xcoord       :  8;   // [23:16]
            vx_uint32 ycoord       :  8;   // [31:24]
#endif
        }
        vxsTP_ROI_Pool;

        vx_array            cmdBuffer;
        vx_tp_coomandInfo_s tpCommand;
        gctUINT32           cmdBufferAddress;
        vx_context          context = vxGetContext((vx_reference)node);
        vx_array            roiHPoolBuffer = VX_NULL;
        vx_uint32           maxPoolSize = (width + pooled_width - 1) / pooled_width;
        vx_array            roiListBuffer = VX_NULL;
        vxsTP_ROI_Pool *    roiPool;
        vx_int32            offset = stride_w == 5 ? 1 : 0;
        vx_int32            roiIter;
        vx_uint32           hPoolStride = 1 << (vx_uint32) ceil(log(width)    / log(2));
        vx_uint32           hPoolYsize  = 1 << (vx_uint32) ceil(log(height)   / log(2));
        vx_uint32           hPoolZsize  = 1 << (vx_uint32) ceil(log(channels) / log(2));
        vx_uint32           hPoolSlice  = hPoolStride * hPoolYsize;
        vx_uint32           hPoolInTileXsize = width;
        vx_uint32           hPoolInTileYsize = 16;
        vx_uint32           proposalsInterleaved = 1;
        vx_uint32           zTogether = 1;
#ifdef USE_REF_INPUT
        vx_uint16           *f16Rois = (vx_uint16 *) rois;
#endif

        /* Create tp command buffer*/
        cmdBuffer = vxCreateArray(context, VX_TYPE_CHAR, 128);
        if (!vxoArray_AllocateMemory(cmdBuffer))
        {
            status |= VX_ERROR_NO_MEMORY;
            return status;
        }

        /* Create ROI HPool output buffer. */
        roiHPoolBuffer = vxCreateArray(context, VX_TYPE_UINT16, hPoolSlice * hPoolZsize * maxPoolSize);
        if (!vxoArray_AllocateMemory(roiHPoolBuffer))
        {
            status |= VX_ERROR_NO_MEMORY;
            return status;
        }

        /* Create ROI Pool list buffer. */
        roiListBuffer = vxCreateArray(context, VX_TYPE_UINT32, num_rois * sizeof(vxsTP_ROI_Pool) / 4);
        if (!vxoArray_AllocateMemory(roiListBuffer))
        {
            status |= VX_ERROR_NO_MEMORY;
            return status;
        }

        /* Pass 1: create ROI horizontal pool with transpose output. */
        roiHPoolBuffer->itemCount = hPoolSlice * hPoolZsize * maxPoolSize;
        roiHPoolBuffer->itemSize = 2;
        roiHPoolBuffer->base.isStage = vx_true_e;

        /* Fill in tpCommand */
        memset(&tpCommand, 0, sizeof(vx_tp_coomandInfo_s));
        tpCommand.inImageXSize = width;
        tpCommand.inImageYSize = height;
        tpCommand.inImageZSize = channels;
        tpCommand.inImageStride = width;
        tpCommand.inImageSlice = width * height;
        tpCommand.inWindowXStart = 0;
        tpCommand.inWindowYStart = 0;
        tpCommand.inWindowXEnd = width - 1 + maxPoolSize - 1;
        tpCommand.inWindowYEnd = height - 1;
        tpCommand.inTileSequence = 0x0;
        tpCommand.inImageGlobalMem = 1;
        tpCommand.inImageBaseAddress = input1->memory.physicals[0];
        tpCommand.inTileListAddress = 0;
        tpCommand.inTileListGlobalMem = 1;
        tpCommand.inTileXSize = hPoolInTileXsize;
        tpCommand.inTileYSize = hPoolInTileYsize;
        tpCommand.inTileXInc = 1;
        tpCommand.inTileYInc = hPoolInTileYsize;
        tpCommand.aluSquarePreshift = 0;
        tpCommand.aluSquareEnable = 0;
        tpCommand.aluHorzProcessing = 0x2;
        tpCommand.aluHorzProcCount = maxPoolSize - 1;
        tpCommand.aluHorzProcStride = 0;
        tpCommand.aluVertProcessing = 0;
        tpCommand.aluVertProcCount = 0;
        tpCommand.aluVertProcStride = 0;
        tpCommand.aluPwlEnable = 0;
        tpCommand.aluMultEnable = 0;
        tpCommand.aluLoadPwlLUT = 0;
        tpCommand.aluLoadPwlLUTAddress = 0;
        tpCommand.aluLoadPwlLUTGlobalMem = 1;
        tpCommand.outBaseAddress = roiHPoolBuffer->memory.physicals[0];
        tpCommand.outGlobalMem  = 1;
        tpCommand.outTileSkipAtborder = 1;
        tpCommand.outBrickMode  = 0;
        tpCommand.outLoop0Inc   = hPoolSlice * hPoolZsize;
        tpCommand.outLoop0Count = maxPoolSize;
        tpCommand.outLoop1Inc   = hPoolStride * maxPoolSize;
        tpCommand.outLoop1Count = 0;
        tpCommand.outLoop1Reset = 0x1;
        tpCommand.outLoop2Inc   = 1;
        tpCommand.outLoop2Count = 0;
        tpCommand.outLoop2Reset = 0x1;
        tpCommand.outLoop3Inc   = hPoolStride;
        tpCommand.outLoop3Count = maxPoolSize;
        tpCommand.outLoop3Reset = 0x0;
        tpCommand.outLoop4Inc   = hPoolInTileYsize;
        tpCommand.outLoop4Count = (height + hPoolInTileYsize - 1) / hPoolInTileYsize;
        tpCommand.outLoop5Inc   = 0;
        tpCommand.outLoop5Count = 1;
        tpCommand.outLoop6Inc   = hPoolSlice;
        tpCommand.last          = 1;

        fillInCmmdBuffer((void*)&tpCommand, cmdBuffer, vx_true_e);

        /* Execute tp command buffer. */
        cmdBufferAddress = (gctUINT32)cmdBuffer->memory.physicals[0];
        status = gcfVX_Accel((gctUINT32)cmdBufferAddress, gcvVX_ACCELERATOR_TP, 0, gcvFALSE);


        /* Pass 2: process ROI pool list on ROI horizontal pool. */

        /* Prepare ROI Pool List. */
        roiListBuffer->itemCount = num_rois;
        roiListBuffer->itemSize = 4;
        roiListBuffer->base.isStage = vx_true_e;
        roiPool = (vxsTP_ROI_Pool *) roiListBuffer->memory.logicals[0];
#ifdef USE_REF_INPUT
        for (roiIter = 0; roiIter < num_rois; roiIter++, f16Rois += stride_w, roiPool++)
#else
        for (roiIter = 0; roiIter < num_rois; roiIter++, rois += stride_w, roiPool++)
#endif
        {
#ifdef USE_REF_INPUT
            vx_uint32 roi_start_w = (vx_uint32)round(Fp16toFp32(f16Rois[offset])     * spatial_scale_);
            vx_uint32 roi_start_h = (vx_uint32)round(Fp16toFp32(f16Rois[offset + 1]) * spatial_scale_);
            vx_uint32 roi_end_w   = (vx_uint32)round(Fp16toFp32(f16Rois[offset + 2]) * spatial_scale_);
            vx_uint32 roi_end_h   = (vx_uint32)round(Fp16toFp32(f16Rois[offset + 3]) * spatial_scale_);
#else
            vx_uint32 roi_start_w = (vx_uint32)round(rois[offset]     * spatial_scale_);
            vx_uint32 roi_start_h = (vx_uint32)round(rois[offset + 1] * spatial_scale_);
            vx_uint32 roi_end_w   = (vx_uint32)round(rois[offset + 2] * spatial_scale_);
            vx_uint32 roi_end_h   = (vx_uint32)round(rois[offset + 3] * spatial_scale_);
#endif
            vx_uint32 roi_width   = (vx_uint32)max(roi_end_w - roi_start_w + 1, 1);
            vx_uint32 roi_height  = (vx_uint32)max(roi_end_h - roi_start_h + 1, 1);

            roiPool->xcoord = roi_start_w;
            roiPool->ycoord = roi_start_h;
#if NN_TP_NEW_ROI
            roiPool->poolingHInc = (vx_uint32)round((float)roi_width  * 256.0f / pooled_width);
            roiPool->poolingVInc = (vx_uint32)round((float)roi_height * 256.0f / pooled_height);
#else
            roiPool->poolingHsize = (roi_width + pooled_width - 1) / pooled_width;
            roiPool->poolingVsize = (roi_height + pooled_height - 1) / pooled_height;
            roiPool->hstride = (roiPool->poolingHsize * pooled_width  == roi_width)  ? 0 : 1;
            roiPool->vstride = (roiPool->poolingVsize * pooled_height == roi_height) ? 0 : 1;
#endif
            roiPool->last = 0;
        }
        roiPool--;
        roiPool->last = 1;


        /* Fill in tpCommand */
        memset(&tpCommand, 0, sizeof(vx_tp_coomandInfo_s));
        tpCommand.inImageXSize = height;
        tpCommand.inImageYSize = width;
        tpCommand.inImageZSize = channels;
        tpCommand.inImageStride = hPoolStride;
        tpCommand.inImageSlice = hPoolSlice;
        tpCommand.inWindowXStart = 0;
        tpCommand.inWindowYStart = 0;
        tpCommand.inWindowXEnd = height - 1 + maxPoolSize - 1;
        tpCommand.inWindowYEnd = width - 1;
        tpCommand.inTileSequence = 0x2;
        tpCommand.inImageGlobalMem = 1;
        tpCommand.inImageBaseAddress = roiHPoolBuffer->memory.physicals[0];
        tpCommand.inTileListGlobalMem = 1;
        tpCommand.inTileListAddress = roiListBuffer->memory.physicals[0];
        tpCommand.inTileXSize = pooled_height;
        tpCommand.inTileYSize = pooled_width;
        tpCommand.inTileXInc = zTogether;
        tpCommand.inTileYInc = 0;
        tpCommand.aluSquarePreshift = 0;
        tpCommand.aluSquareEnable = 0;
        tpCommand.aluHorzProcessing = 0x3;
        tpCommand.aluHorzProcCount = 1;
        tpCommand.aluHorzProcStride = 0;
        tpCommand.aluVertProcessing = 0;
        tpCommand.aluVertProcCount = 0;
        tpCommand.aluVertProcStride = 0;
        tpCommand.aluPwlEnable = 0;
        tpCommand.aluMultEnable = 0;
        tpCommand.aluLoadPwlLUT = 0;
        tpCommand.aluLoadPwlLUTAddress = 0;
        tpCommand.aluLoadPwlLUTGlobalMem = 1;
        tpCommand.outBaseAddress = dst->memory.physicals[0];
        tpCommand.outGlobalMem  = 1;
        tpCommand.outTileSkipAtborder = 0;
        tpCommand.outBrickMode  = 0;
        tpCommand.outLoop0Inc   = pooled_width * proposalsInterleaved;
        tpCommand.outLoop0Count = pooled_height;
        tpCommand.outLoop1Inc   = proposalsInterleaved;
        tpCommand.outLoop1Count = pooled_width;
        tpCommand.outLoop1Reset = 0x0;
        tpCommand.outLoop2Inc   = pooled_width * pooled_height * proposalsInterleaved;
        tpCommand.outLoop2Count = zTogether;
        tpCommand.outLoop2Reset = 0x0;
        tpCommand.outLoop3Inc   = 1;
        tpCommand.outLoop3Count = proposalsInterleaved;
        tpCommand.outLoop3Reset = 0x0;
        tpCommand.outLoop4Inc   = pooled_width * pooled_height * channels;
        tpCommand.outLoop4Count = num_rois;
        tpCommand.outLoop5Inc   = 0;
        tpCommand.outLoop5Count = 1;
        tpCommand.outLoop6Inc   = pooled_width * pooled_height * proposalsInterleaved * zTogether;
        tpCommand.last          = 1;

        fillInCmmdBuffer((void*)&tpCommand, cmdBuffer, vx_true_e);

        /* Execute tp command buffer. */
        cmdBufferAddress = (gctUINT32)cmdBuffer->memory.physicals[0];
        status = gcfVX_Accel((gctUINT32)cmdBufferAddress, gcvVX_ACCELERATOR_TP, 0, gcvFALSE);

        vxReleaseArray(&cmdBuffer);
        vxReleaseArray(&roiHPoolBuffer);
        vxReleaseArray(&roiListBuffer);
    }
    else
#endif
    {
    for (n = 0; n < num_rois; ++n) {
        vx_int32 offset = 0, roi_batch_ind = 0, roi_start_w = 0, roi_start_h = 0, roi_end_w = 0, roi_end_h = 0;
        vx_int32 roi_height = 0, roi_width = 0;
        vx_float32 bin_size_h = 0, bin_size_w = 0;
        vx_float32* batch_data = VX_NULL;

        if (stride_w == 5)
        {
            offset = 1;
            roi_batch_ind = (vx_int32)rois[0];
        }

        roi_start_w = (vx_int32)(round(rois[offset] * spatial_scale_));
        roi_start_h = (vx_int32)round(rois[offset + 1] * spatial_scale_);
        roi_end_w = (vx_int32)round(rois[offset + 2] * spatial_scale_);
        roi_end_h = (vx_int32)round(rois[offset + 3] * spatial_scale_);

        roi_height = (vx_int32)max(roi_end_h - roi_start_h + 1, 1);
        roi_width = (vx_int32)max(roi_end_w - roi_start_w + 1, 1);
        bin_size_h = (vx_float32)(roi_height) / (vx_float32)(pooled_height);
        bin_size_w = (vx_float32)(roi_width) / (vx_float32)(pooled_width);

        batch_data = data + roi_batch_ind * channels * width * height;

        for (c = 0; c < channels; ++c) {
            for (ph = 0; ph < pooled_height; ++ph) {
                for (pw = 0; pw < pooled_width; ++pw) {
                    /*
                     *  Compute pooling region for this output unit:
                     *  start (included) = floor(ph * roi_height / pooled_height_)
                     *  end (excluded) = ceil((ph + 1) * roi_height / pooled_height_)
                     */
                    vx_int32 hstart = (vx_int32)(floor((vx_float32)(ph) * bin_size_h));
                    vx_int32 wstart = (vx_int32)(floor((vx_float32)(pw) * bin_size_w));
                    vx_int32 hend = (vx_int32)(ceil((vx_float32)(ph + 1) * bin_size_h));
                    vx_int32 wend = (vx_int32)(ceil((vx_float32)(pw + 1) * bin_size_w));

                    vx_int32 pool_index = 0;
                    vx_bool is_empty = vx_false_e;

                    hstart = min(max(hstart + roi_start_h, 0), height);
                    hend = min(max(hend + roi_start_h, 0), height);
                    wstart = min(max(wstart + roi_start_w, 0), width);
                    wend = min(max(wend + roi_start_w, 0), width);

                    is_empty = (vx_bool)((hend <= hstart) || (wend <= wstart));

                    pool_index = ph * pooled_width + pw;
                    if (is_empty)
                        dest[pool_index] = 0;
                    else
                        dest[pool_index] = -FP32_MAX;

                    for (h = hstart; h < hend; ++h) {
                        for (w = wstart; w < wend; ++w) {
                            const vx_int32 index = h * width + w;
                            if (batch_data[index] > dest[pool_index]) {
                                dest[pool_index] = batch_data[index];
                            }
                        }
                    }
                }
            }

            /* Increment all data pointers by one channel*/
            batch_data += width * height;
            dest += pooled_width * pooled_height;
        }

        /* Increment ROI data pointer */
        if (stride_w == 5)
            rois += 5;
        else
            rois += 4;
    }
    }

    vxWriteScalarValue(kernel, &kernel_v);
    vxWriteScalarValue(stride, &stride_v);
    vxWriteScalarValue(pad, &pad_v);

    dst->itemCount = num_rois * pooled_width * pooled_height * channels;
    dst->itemSize = 4;

#if defined(__linux__)
    if (node->base.context->perfEnable)
        printf("fast rcnn roi pool       CPU  time:%10d us\n", gcfVX_PerfEnd((vx_reference)node, start));
#endif


#ifdef COMPARE_TO_REF
    {
        vx_uint32 i, itemCount = num_rois * channels * pooled_height * pooled_width;
        vx_float32 fDelta, fMax_delta, fPortion_BigDelta;
        vx_uint32 count_big_delta, offset_max_delta;
        vx_float32 * ref = (vx_float32*)malloc(itemCount * sizeof(vx_float32));
        FILE* ref_f = fopen("fasterRcnn_resource\\000456-detection\\layer0_roi_pool5\\output_256_256_6_6.dat", "rb");
        fread(ref, itemCount, sizeof(vx_float32), ref_f);
        fclose(ref_f);


        fMax_delta =0.0f;
        count_big_delta = 0;
        offset_max_delta = 0;
        for (i = 0; i < itemCount; i ++)
        {
            vx_float32 pfOut = Fp16toFp32(*((vx_int16*) dst->memory.logicals[0] + i));
            //vx_float32 pfOut = *((vx_float32*) dst->memory.logicals[0] + i);
            if (ref[i] != 0.0f)
            {
                fDelta = (vx_float32)fabs((ref[i] - pfOut) / ref[i]);
            }
            else if (pfOut != 0.0f)
            {
                fDelta = (vx_float32)fabs((ref[i] - pfOut) / pfOut);
            }
            else
            {
                continue;
            }

            if (fDelta > 0.01f)
            {
                count_big_delta++;
            }
            if (fDelta > fMax_delta)
            {
                fMax_delta = fDelta;
                offset_max_delta = i;
            }
        }

        fPortion_BigDelta = (float)count_big_delta/(float)itemCount;
        printf("Portion_BigDelta = %9.7f, Max delta = %10.6f, count_big_delta= %8d \n", fPortion_BigDelta, fMax_delta, count_big_delta);
        free(ref);
    }
#endif

    return status;
}

typedef enum _CNN_LRN_Type
{
    CNN_LRN_Type_WITHIN_CHANNELS = 0,
    CNN_LRN_Type_ACROSS_CHANNELS,
}
CNN_LRN_Type;

#define COUNT_TIME 0

#define FAST_ALGORITHM 0

#if FAST_ALGORITHM
float fast4throot(float x)
{
    /*
     *  fast sqrt : *(int*)&x = (((*(int*)&x)-0x3f800000)>>1)+0x3f800000 - 0x4c000
     *  sqrt(sqrt(x)): *(int*)&x = ((((((*(int*)&x) - 0x3f800000)>>1) + 0x3f800000 - 0x4c000) - 0x3f800000) >> 1) + 0x3f800000 - 0x4c000;
     */
    *(int*)&x = (((((*(int*)&x) - 0x3f800000)>>1) - 0x4c000) >> 1) + 0x3F7B4000;
    return x;
}
#endif

vx_status vxLRN(vx_node node, vx_array src, vx_scalar _width, vx_scalar _height, vx_scalar _depth, vx_scalar batch, vx_scalar type,
                vx_scalar kernel, vx_scalar stride, vx_scalar pad, vx_scalar sf, vx_scalar ap, vx_scalar bt, vx_array dst)
{
    vx_status   status = VX_SUCCESS;
    vx_int32    width = 0, height = 0, depth = 0, b = 0;
    vx_int32    stride_v = 2, kernel_v = 3, pad_v = 0 ,type_v= 0;
    vx_float32  shift = 1.0f, alpha = 0.00005f, beta = 0.75f;
#if NN_TP_ENGINE
    vx_float32  pwlLUT[1024];
#endif

    vxReadScalarValue(_width, &width);
    vxReadScalarValue(_height, &height);
    vxReadScalarValue(_depth, &depth);
    vxReadScalarValue(batch, &b);
    vxReadScalarValue(type, &type_v);
    vxReadScalarValue(kernel, &kernel_v);
    vxReadScalarValue(stride, &stride_v);
    vxReadScalarValue(pad, &pad_v);
    vxReadScalarValue(sf, &shift);
    vxReadScalarValue(ap, &alpha);
    vxReadScalarValue(bt, &beta);

#if NN_TP_ENGINE
    /* Create PWL LUT. */
    {
        vx_uint16  base;
        vx_uint16  baseF16;
        vx_float32 baseF32;
        vx_uint32  baseU32;
        vx_float32 pwlValue;
        for (base = 0; base < 0x20; base++)
        {
            pwlLUT[base] = 1.0f;
        }
        for (base = 0x20; base < 0x3E0; base++)
        {
            baseF16 = base << 5;
            baseF32 = Fp16toFp32(baseF16);
            pwlValue = shift + alpha * (baseF32 * 65536.0f / 9.0f);
            pwlValue = 1.0f / (sqrtf(sqrtf(pwlValue * pwlValue * pwlValue)));
            pwlLUT[base] = pwlValue;
        }
        baseU32 = (31 - 15 + 127) << 23;
        baseF32 = *((vx_float32*) &baseU32);
        pwlValue = shift + alpha * (baseF32 * 65536.0f / 9.0f);
        pwlValue = 1.0f / (sqrtf(sqrtf(pwlValue * pwlValue * pwlValue)));
        for (base = 0x3E0; base < 0x400; base++)
        {
            pwlLUT[base] = pwlValue;
        }
    }


    if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_TP_ENGINE) && 1)
    {
        vx_array            cmdBuffer;
        vx_tp_coomandInfo_s tpCommand;
        vx_context          context = vxGetContext((vx_reference)node);
        vx_array            pwlLUTBuffer = VX_NULL;
        vx_uint16 *         pwlLUTBase;
        vx_uint16           base;
        vx_uint32           outTileXsize = 64;
        vx_uint32           outTileYsize = 16;

        pwlLUTBuffer = vxCreateArray(context, VX_TYPE_UINT16, 1024);
        if (!vxoArray_AllocateMemory(pwlLUTBuffer))
        {
            status |= VX_ERROR_NO_MEMORY;
        }
        pwlLUTBuffer->itemCount = 1024;
        pwlLUTBuffer->base.isStage = vx_true_e;
        pwlLUTBase = (vx_uint16 *) pwlLUTBuffer->memory.logicals[0];
        for (base = 0; base < 1024; base++)
        {
            pwlLUTBase[base] = F32toF16(pwlLUT[base]);
        }

        /* Fill in tpCommand */
        memset(&tpCommand, 0, sizeof(vx_tp_coomandInfo_s));
        tpCommand.inImageXSize = width;
        tpCommand.inImageYSize = height;
        tpCommand.inImageZSize = depth;
        tpCommand.inImageStride = width;
        tpCommand.inImageSlice = width * height;
        tpCommand.inWindowXStart = -1;
        tpCommand.inWindowYStart = -1;
        tpCommand.inWindowXEnd = width;
        tpCommand.inWindowYEnd = height;
        tpCommand.inTileSequence = 0;
        tpCommand.inImageGlobalMem = 1;
        tpCommand.inImageBaseAddress = src->memory.physicals[0];
        tpCommand.inTileXSize = outTileXsize + 3 - 1;
        tpCommand.inTileYSize = outTileYsize + 3 - 1;
        tpCommand.inTileXInc = outTileXsize;
        tpCommand.inTileYInc = outTileYsize;
        tpCommand.aluSquarePreshift = 8;
        tpCommand.aluSquareEnable = 1;
        tpCommand.aluHorzProcessing = 0;
        tpCommand.aluHorzProcCount = 2;
        tpCommand.aluHorzProcStride = 0;
        tpCommand.aluVertProcessing = 0;
        tpCommand.aluVertProcCount = 2;
        tpCommand.aluVertProcStride = 0;
        tpCommand.aluPwlEnable = 1;
        tpCommand.aluMultEnable = 1;
        tpCommand.aluLoadPwlLUT = 1;
        tpCommand.aluLoadPwlLUTAddress = pwlLUTBuffer->memory.physicals[0];
        tpCommand.aluLoadPwlLUTGlobalMem = 1;
        tpCommand.outBaseAddress = dst->memory.physicals[0];
        tpCommand.outGlobalMem  = 1;
        tpCommand.outTileSkipAtborder = 0;
        tpCommand.outBrickMode  = 0;
        tpCommand.outLoop0Inc   = 0;
        tpCommand.outLoop0Count = 1;
        tpCommand.outLoop1Inc   = 1;
        tpCommand.outLoop1Count = 0;
        tpCommand.outLoop1Reset = 1;
        tpCommand.outLoop2Inc   = width;
        tpCommand.outLoop2Count = 0;
        tpCommand.outLoop2Reset = 1;
        tpCommand.outLoop3Inc   = outTileXsize;   /* outtile_xsize */
        tpCommand.outLoop3Count = (width + outTileXsize - 1) / outTileXsize;
        tpCommand.outLoop3Reset = 0;
        tpCommand.outLoop4Inc   = outTileYsize * width;
        tpCommand.outLoop4Count = (height + outTileYsize - 1) / outTileYsize;
        tpCommand.outLoop5Inc   = 0;
        tpCommand.outLoop5Count = 1;
        tpCommand.outLoop6Inc   = width * height;
        tpCommand.last          = 1;

        /* Create tp command buffer*/
        cmdBuffer = vxCreateArray(context, VX_TYPE_CHAR, 128);
        if (vxoArray_AllocateMemory(cmdBuffer))
        {
            gctUINT32 cmdBufferAddress;

            fillInCmmdBuffer((void*)&tpCommand, cmdBuffer, vx_true_e);

            /* Execute tp command buffer. */
            cmdBufferAddress = (gctUINT32)cmdBuffer->memory.physicals[0];
            status = gcfVX_Accel((gctUINT32)cmdBufferAddress, gcvVX_ACCELERATOR_TP, 0, gcvFALSE);
        }
        else
        {
            status |= VX_ERROR_NO_MEMORY;
        }
        vxReleaseArray(&cmdBuffer);
        vxReleaseArray(&pwlLUTBuffer);
    }
    else
#endif
    {
#if VX_SHADER_TP
        vx_int32 params_int[8];
        vx_float32 params_fp[4];
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
            {            /* Allocate a local copy for old flow. */
                node->kernelContext = (gcoVX_Kernel_Context *) vxAllocate(sizeof(gcoVX_Kernel_Context));
            }
            kernelContext = (gcoVX_Kernel_Context *)node->kernelContext;
            kernelContext->objects_num = 0;
            kernelContext->uniform_num = 0;
        }

        // 1. transform src from array to image, and store in uniform[0]
        AddObject(0, src, kernelContext, width, height * depth, 1);
        // 2. transform dst from array to image, and store in uniform[1]
        AddObject(1, dst, kernelContext, width, height * depth, (dst->itemSize == 4)?2:1);

        kernelContext->params.borders = gcvVX_BORDER_MODE_CONSTANT;
        kernelContext->params.constant_value = 0;

        kernelContext->params.kernel       = gcvVX_KERNEL_LRN;
        kernelContext->params.xstep        = 4;
        kernelContext->params.ystep        = height;
        kernelContext->params.xmax         = width;
        kernelContext->params.ymax         = height;
        kernelContext->params.col          = depth;
        kernelContext->params.row          = (vx_uint32)dst->itemSize;

        kernelContext->node = node;

        params_int[0] = height-1;
        params_int[1] = 0;
        params_int[2] = height-2;
        params_int[3] = height * depth;
        params_int[4] = depth;
        params_int[5] = kernel_v;
        params_int[6] = stride_v;
        params_int[7] = pad_v;
        gcoOS_MemCopy(&kernelContext->uniforms[kernelContext->uniform_num].uniform, params_int, sizeof(params_int));
        kernelContext->uniforms[kernelContext->uniform_num].index       = 2;
        kernelContext->uniforms[kernelContext->uniform_num++].num       = 4*8;

        params_fp[0] = shift;
        params_fp[1] = alpha;
        params_fp[2] = beta;
        params_fp[3] = 0;
        gcoOS_MemCopy(&kernelContext->uniforms[kernelContext->uniform_num].uniform, params_fp, sizeof(params_fp));
        kernelContext->uniforms[kernelContext->uniform_num].index       = 4;
        kernelContext->uniforms[kernelContext->uniform_num++].num       = 4*4;
        {
            vx_uint32 dp_fp16_1[16] =
            {
                0x15151515, // TCfg
                0x00000000, // ASelt
                0x03210210, 0x05430432, // ABin
                0x15151515, // BSelt
                0x03210210, 0x05430432, // BBin
                0x00000400, // AccumType, ConstantType, and PostShift
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
            };

            vx_uint32 dp_fp16tofp32[16] =
            {
                0x01010101, // TCfg
                0x00000000, // ASelt
                0x00020001, 0x00040003, // ABin
                0x02020202, // BSelt
                0x00000000, 0x00000000, // BBin
                0x00000100, // AccumType, ConstantType, and PostShift
                0x00003c00, 0x00000000, 0x00003c00, 0x00000000, 0x00003c00, 0x00000000, 0x00003c00, 0x00000000 // Constant
            };
            vx_uint32 dp_u32tofp16[16] =
            {
                0x01010101, // TCfg
                0x00000000, // ASelt
                0x22220000, 0x66664444, // ABin
                0xaaaaaaaa, // BSelt
                0x00000000, 0x00000000, // BBin
                0x00003300, // AccumType, ConstantType, and PostShift
                0x00000001, 0x00000000, 0x00000001, 0x00000000, 0x00000001, 0x00000000, 0x00000001, 0x00000000 // Constant
            };

            gcoOS_MemCopy(&kernelContext->uniforms[kernelContext->uniform_num].uniform, dp_fp16_1, sizeof(dp_fp16_1));
            kernelContext->uniforms[kernelContext->uniform_num].index       = 5;
            kernelContext->uniforms[kernelContext->uniform_num].num         = 16 * 4;
            kernelContext->uniform_num ++;

            gcoOS_MemCopy(&kernelContext->uniforms[kernelContext->uniform_num].uniform, dp_fp16tofp32, sizeof(dp_fp16tofp32));
            kernelContext->uniforms[kernelContext->uniform_num].index       = 9;
            kernelContext->uniforms[kernelContext->uniform_num].num         = 16 * 4;
            kernelContext->uniform_num ++;

            gcoOS_MemCopy(&kernelContext->uniforms[kernelContext->uniform_num].uniform, dp_u32tofp16, sizeof(dp_u32tofp16));
            kernelContext->uniforms[kernelContext->uniform_num].index       = 13;
            kernelContext->uniforms[kernelContext->uniform_num].num         = 16 * 4;
            kernelContext->uniform_num ++;
        }
        kernelContext->params.evisNoInst = node->base.context->evisNoInst;

        dst->itemCount = src->itemCount;

        status = gcfVX_Kernel(kernelContext);

#if !VX_NN_SH_PARALLEL

        gcoVX_Flush(gcvTRUE);
#endif
#else

#if CPU_CACHE_MEORMY
        void *data_p = NULL, *data_dp = NULL;
#endif

#ifdef LINUX
#if COUNT_TIME
        struct timeval tmsCountStart, tmsCountEnd;
        unsigned int tmpCountVal = 0, tmpCountVal1 = 0;
#endif
#endif

        vx_float32 *data = 0;
        vx_float32 *data_d = 0, *sum_buffer, *sum_buffer2;
        vx_int32 i = 0, j = 0, k = 0, p = 0, avg = 0;
#if defined(__linux__)
        struct timeval start = gcfVX_PerfStart((vx_reference)node);
#endif

        src->itemCount = dst->itemCount = width * height * depth;

#if CPU_CACHE_MEORMY
        data_p = data = (vx_float32*)malloc(height * width * depth * b * sizeof(vx_float32));
        data_dp = data_d = (vx_float32*)dst->memory.logicals[0];

        memcpy(data, src->memory.logicals[0], height * width * depth * b * sizeof(vx_float32));
#else
        data = (vx_float32*)src->memory.logicals[0];
        data_d = (vx_float32*)dst->memory.logicals[0];
#endif

#if VX_C_MEMORY_MANAGE
       vxoMemory_CAllocate(node->base.context, (void**)&sum_buffer, 2 * width * height * sizeof(vx_float32));
#else
        sum_buffer = (vx_float32*)malloc(2 * width * height * sizeof(vx_float32));
#endif
        sum_buffer2 = sum_buffer + width * height;
        avg = kernel_v * kernel_v;

        /*
         * B(i) = A(i)/(shift + a * ((A0*A0 + A1*A2 + ... + An*An)/n))^b
         * shift : k
         * a     : alpha
         * b     : beta
         * kernel: local_size
         */

        switch(type_v)
        {
        case CNN_LRN_Type_WITHIN_CHANNELS:
            for (k = 0; k < b; k ++)
            {
                for (p = 0; p < depth; p ++)
                {
#if COUNT_TIME
                    gettimeofday(&tmsCountStart, 0);
#endif
                    /*   _____________   _ _ _ _ _ _ _   _____________
                     *   |            |  |______0_____|  |            |
                     *   |            |  |            |  |            |
                     *   |            |  |            |  |            |
                     *   |            |+ |            | +|            |
                     *   |            |  |            |  |            |
                     *   |            |  |            |  |____________|
                     *   |____________|  |____________|  |_ _ _ 0 _ _ |
                     *
                     */
                    for (j = 0; j < height; j ++)
                    {
                        const vx_int32 stride_pre = (j - 1) * (width);
                        const vx_int32 stride = j * (width);
                        const vx_int32 stride_next = (j + 1) * (width);
                        for (i = 0; i < width; i ++)
                        {
                            vx_float32 pre  = (j == 0) ? 0 : data[stride_pre + i];
                            vx_float32 self = data[stride + i];
                            vx_float32 next = (j == (height - 1)) ? 0 : data[stride_next + i];

#if TP_SCALING
                            sum_buffer[stride + i] = Fp16toFp32(F32toF16(pre  * pre  / 65536))
                                                   + Fp16toFp32(F32toF16(self * self / 65536))
                                                   + Fp16toFp32(F32toF16(next * next / 65536));
#else
                            sum_buffer[stride + i] = pre * pre + self * self + next * next;
#endif
                        }
                    }
                    /*   ______________   _____________   _____________
                     *   | |           |  |            |  |          | |
                     *   | |           |  |            |  |          | |
                     *   |0|           |+ |            | +|          |0|
                     *   | |           |  |            |  |          | |
                     *   |_|___________|  |____________|  |__________|_|
                     *
                     */

                    for (j = 0; j < height; j ++)
                    {
                        const vx_int32 stride =  j * (width);
                        for (i = 0; i < width; i ++)
                        {
                            vx_float32 pre  = (i == 0) ? 0 :sum_buffer[stride + i - 1];
                            vx_float32 self = sum_buffer[stride + i];
                            vx_float32 next = (i == (width - 1)) ? 0 : sum_buffer[stride + i + 1];

#if TP_SCALING
                            sum_buffer2[stride + i] = pre + self + next;
                            if (sum_buffer2[stride + i] > 65504)
                            {
                                gcmASSERT(sum_buffer2[stride + i] <= 65504);
                            }
#else
                            sum_buffer2[stride + i] = (pre + self + next) / 9;
#endif
                        }
                    }

#if COUNT_TIME
                    gettimeofday(&tmsCountEnd, 0);
                    tmpCountVal += 1000 * (tmsCountEnd.tv_sec - tmsCountStart.tv_sec) + (tmsCountEnd.tv_usec - tmsCountStart.tv_usec) / 1000;

                    gettimeofday(&tmsCountStart, 0);
#endif

                    for (j = 0; j < height; j ++)
                    {
                        const vx_int32 stride =  j * (width);
                        for (i = 0; i < width; i ++)
                        {
                            vx_int32 pool_index = stride + i;
#if TP_SCALING
                            vx_float32 data_org = sum_buffer2[pool_index];
                            vx_uint16 data_f16 = F32toF16(data_org);
                            vx_uint16 base = data_f16 >> 5;
                            vx_uint16 delta = data_f16 & 0x1F;
                            vx_float32 pwl = (pwlLUT[base + 1] - pwlLUT[base]) * (float)delta / 32.0f + pwlLUT[base];
                            gcmASSERT(base < 0x3E0);
                            data_d[pool_index] = data[pool_index] * pwl;
#else
                            vx_float32 data_s = shift + alpha * (sum_buffer2[pool_index]);
#if FAST_ALGORITHM
                            data_d[pool_index] = data[pool_index]/fast4throot(data_s * data_s * data_s);
#else
                            data_d[pool_index] = data[pool_index]/(sqrtf(sqrtf((data_s * data_s * data_s))));
                            /*data_d[pool_index] = data[pool_index]/powf(data_s, 0.75);*/
#endif
#endif
                        }
                    }

#if COUNT_TIME
                    gettimeofday(&tmsCountEnd, 0);
                    tmpCountVal1 += 1000 * (tmsCountEnd.tv_sec - tmsCountStart.tv_sec) + (tmsCountEnd.tv_usec - tmsCountStart.tv_usec) / 1000;
#endif

                    data += width * height;
                    data_d += width * height;
                }
            }
            break;
        }

#if CPU_CACHE_MEORMY
        free(data_p);
#endif
#if VX_C_MEMORY_MANAGE
        vxoMemory_CFree(node->base.context, (void**)&sum_buffer);
#else
        free(sum_buffer);
#endif

#if defined(__linux__)
        if (node->base.context->perfEnable)
            printf("fast rcnn lrn pool        CPU  time:%10d us\n", gcfVX_PerfEnd((vx_reference)node, start));
#endif

#endif
    }

#if NN_TP_ENGINE

#ifdef COMPARE_TO_REF
    {
        vx_uint32 layerIndex = CURR_LAYER;
        FILE *outputFile = NULL;
        vx_uint32 itemCount = width * height * depth * b;
        vx_float32 *pfNN_out, *pfRef_out;
        vx_float32 fDelta, fMax_delta, fPortion_BigDelta;
        vx_uint32 i, count_big_delta, offset_max_delta;
        vx_char *fileName[] = { "fasterRcnn_resource\\000456-proposal\\layer2_norm1\\output_1_96_300_400.dat",
                                "fasterRcnn_resource\\000456-proposal\\layer6_norm2\\output_1_256_76_101.dat"};
        vx_char fullFileName[256] = {'\0'};
        pfNN_out =  (vx_float32*)malloc(itemCount * sizeof(vx_float32));
        pfRef_out = (vx_float32*)malloc(itemCount * sizeof(vx_float32));

        sprintf(fullFileName, "%s", fileName[layerIndex]);
        outputFile = fopen(fullFileName,"rb");            /* compare to ref output file of each layer */
        if (outputFile == NULL)
        {
            gcmPRINT("can't find file %s", fullFileName);
            return VX_ERROR_INVALID_PARAMETERS;
        }

        if(fread(pfRef_out, 1, itemCount*sizeof(vx_float32), outputFile) != itemCount*sizeof(vx_float32) )
        {
            gcmPRINT("fread failure");
            return VX_ERROR_INVALID_PARAMETERS;
        }
        fclose(outputFile);


        fMax_delta =0.0f;
        count_big_delta = 0;
        offset_max_delta = 0;
        for (i = 0; i < itemCount; i++)
        {
            pfNN_out[i] = Fp16toFp32(*((vx_int16*) dst->memory.logicals[0] + i));
            //pfNN_out[i] = *((vx_float32*) dst->memory.logicals[0] + i);
            if (pfRef_out[i] != 0.0f)
            {
                fDelta = (vx_float32)fabs((pfRef_out[i] - pfNN_out[i]) / pfRef_out[i]);
            }
            else if (pfNN_out[i] != 0.0f)
            {
                fDelta = (vx_float32)fabs((pfRef_out[i] - pfNN_out[i]) / pfNN_out[i]);
            }
            else
            {
                continue;
            }

            if (fDelta > 0.01f)
            {
                count_big_delta++;
            }
            if (fDelta > fMax_delta)
            {
                fMax_delta = fDelta;
                offset_max_delta = i;
            }
        }

        fPortion_BigDelta = (float)count_big_delta/(float)itemCount;
        printf("Portion_BigDelta = %9.7f, Max delta = %10.6f, count_big_delta= %8d \n", fPortion_BigDelta, fMax_delta, count_big_delta);
        free(pfRef_out);
        free(pfNN_out);
    }
#endif
#endif

    vxWriteScalarValue(_width, &width);
    vxWriteScalarValue(_height, &height);
    vxWriteScalarValue(_depth, &depth);
    vxWriteScalarValue(batch, &b);
    vxWriteScalarValue(type, &type_v);
    vxWriteScalarValue(kernel, &kernel_v);
    vxWriteScalarValue(stride, &stride_v);
    vxWriteScalarValue(pad, &pad_v);
    vxWriteScalarValue(sf, &shift);
    vxWriteScalarValue(ap, &alpha);
    vxWriteScalarValue(bt, &beta);

    return status;
}

