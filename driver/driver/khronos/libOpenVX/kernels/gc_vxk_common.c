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

#define _GC_OBJ_ZONE            gcdZONE_VX_OTHERS

VX_INTERNAL_API gctUINT64 gcfVX_PerfStart(vx_reference ref)
{
    gctUINT64 start = 0;
    if(ref->context->options.enableCNNPerf)
    {
        gcoOS_GetTime(&start);
    }

    return start;
}

VX_INTERNAL_API vx_uint64 gcfVX_PerfEnd(vx_reference ref, gctUINT64 start)
{
    vx_uint64 interval = 0;

    if(ref->context->options.enableCNNPerf)
    {
        gctUINT64 end;
        gcoOS_GetTime(&end);
        interval = (vx_uint64)(end - start);
    }

    return interval;
}

#define GCREG_SH_INSTRUCTION_TYPE_FLOAT32 0x0
#define GCREG_SH_INSTRUCTION_TYPE_FLOAT16 0x1
#define GCREG_SH_INSTRUCTION_TYPE_SIGNED32 0x2
#define GCREG_SH_INSTRUCTION_TYPE_SIGNED16 0x3
#define GCREG_SH_INSTRUCTION_TYPE_SIGNED8 0x4
#define GCREG_SH_INSTRUCTION_TYPE_UNSIGNED32 0x5
#define GCREG_SH_INSTRUCTION_TYPE_UNSIGNED16 0x6
#define GCREG_SH_INSTRUCTION_TYPE_UNSIGNED8 0x7
#define   GCREG_SH_INSTRUCTION_TYPE_BFLOAT16                                 0x8
#define   GCREG_SH_INSTRUCTION_TYPE_SIGNED64                                 0xA

static gceSTATUS _FillImageInfoFromFormat(vx_df_image Format, gcsVX_IMAGE_INFO_PTR Info)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 format = 0;

    gcmHEADER_ARG("Format=%d Info=%p", Format, Info);

    switch(Format)
    {
    case VX_DF_IMAGE_U1:
    case VX_DF_IMAGE_U8:
    case VX_DF_IMAGE_YUV4:
    case VX_DF_IMAGE_NV21:
    case VX_DF_IMAGE_NV12:
        format = 0x7;
        break;
    case VX_DF_IMAGE_U16:
    case VX_DF_IMAGE_UYVY:
    case VX_DF_IMAGE_YUYV:
        format = 0x6;
        break;
    case VX_DF_IMAGE_S16:
        format = 0x3;
        break;
    case VX_DF_IMAGE_U32:
    case VX_DF_IMAGE_RGB:
    case VX_DF_IMAGE_RGBX:
        format = 0x5;
        break;
    case VX_DF_IMAGE_S32:
        format = 0x2;
        break;
    case VX_DF_IMAGE_F32:
        format = 0x0;
        break;
    default:
        format = 0x7;
        break;
    }

    Info->format = format;

    Info->isFloat = (Info->format == 0x0 ||
                       Info->format == 0x1) ? gcvTRUE : gcvFALSE;

    /* Convert image format into plane count and bits per pixel.*/
    switch (Format)
    {
        case VX_DF_IMAGE_RGB:
            Info->planes = 1;
            Info->bpp = 24;
            Info->componentCount = 3;
            Info->internalFormat = gcvSURF_R8G8B8;
            break;

        case VX_DF_IMAGE_RGBX:
            Info->planes = 1;
            Info->bpp = 32;
            Info->componentCount = 4;
            Info->internalFormat = gcvSURF_X8R8G8B8;
            break;

        case VX_DF_IMAGE_NV12:
            Info->planes = 2;
            Info->bpp = 8;
            Info->uPixels = Info->vPixels = 2;
            Info->componentCount = 3;
            Info->internalFormat = gcvSURF_NV12;
            break;

        case VX_DF_IMAGE_NV21:
            Info->planes = 2;
            Info->bpp = 8;
            Info->uPixels = Info->vPixels = 2;
            Info->componentCount = 3;
            Info->internalFormat = gcvSURF_NV21;
            break;

        case VX_DF_IMAGE_UYVY:
            Info->planes = 1;
            Info->bpp = 16;
            Info->componentCount = 3;
            Info->internalFormat = gcvSURF_UYVY;
            break;

        case VX_DF_IMAGE_YUYV:
            Info->planes = 1;
            Info->bpp = 16;
            Info->componentCount = 3;
            /*BUGBUG: Info->internalFormat = gcvSURF_YUYV;*/
            break;

        case VX_DF_IMAGE_IYUV:
            Info->planes = 3;
            Info->bpp = 8;
            Info->uPixels = Info->vPixels = 2;
            Info->componentCount = 3;
            /*BUGBUG: Info->internalFormat = gcvSURF_IYUV;*/
            break;

        case VX_DF_IMAGE_YUV4:
            Info->planes = 3;
            Info->bpp = 8;
            Info->uPixels = Info->vPixels = 1;
            Info->componentCount = 3;
            /*BUGBUG: Info->internalFormat = gcvSURF_YUY4;*/
            break;

        case VX_DF_IMAGE_U1:
            Info->width = (Info->width + 7) / 8;
            Info->planes = 1;
            Info->bpp = 8;
            Info->componentCount = 1;
            Info->internalFormat = gcvSURF_R8;
            break;

        case VX_DF_IMAGE_U8:
            Info->planes = 1;
            Info->bpp = 8;
            Info->componentCount = 1;
            Info->internalFormat = gcvSURF_R8;
            break;

        case VX_DF_IMAGE_U16:
        case VX_DF_IMAGE_S16:
            Info->planes = 1;
            Info->bpp = 16;
            Info->componentCount = 1;
            Info->internalFormat = gcvSURF_R16;
            break;

        case VX_DF_IMAGE_U32:
        case VX_DF_IMAGE_S32:
            Info->planes = 1;
            Info->bpp = 32;
            Info->componentCount = 1;
            Info->internalFormat = gcvSURF_R32;
            break;
        case VX_DF_IMAGE_F32:
            Info->planes = 1;
            Info->bpp = 32;
            Info->componentCount = 1;
            Info->internalFormat = gcvSURF_R32F;
            break;

        default:
            /* Unsupported format.*/
            gcmPRINT("ERROR: Invalid image format 0x%04X\n", Format);
            status = gcvSTATUS_FALSE;
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

static vx_status _SetBorderMode(vx_enum border, gctUINT32 *viv_border)
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("border=%d viv_border=%p", border, viv_border);

    switch(border)
    {
    case VX_BORDER_UNDEFINED:
        *viv_border = gcvVX_BORDER_MODE_UNDEFINED;
        break;
    case VX_BORDER_CONSTANT:
        *viv_border = gcvVX_BORDER_MODE_CONSTANT;
        break;
    case VX_BORDER_REPLICATE:
        *viv_border = gcvVX_BORDER_MODE_REPLACEMENT;
        break;
    default:
        *viv_border = gcvVX_BORDER_MODE_UNDEFINED;
        break;
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

static gceSTATUS _BindInstructions(
    IN gcoVX_Kernel_Context *C
    )
{
#if gcdVX_OPTIMIZER > 1
    gceSTATUS                       status = gcvSTATUS_OK;
    gcoVX_Hardware_Context          *hwContext = &C->params;

    gcmHEADER_ARG("C=%p", C);

    _SetBorderMode(C->borders, &hwContext->borders);

    hwContext->instructions   = &C->instructions;
    hwContext->uniform        = C->uniforms;
    hwContext->unifor_num     = &C->uniform_num;

    gcmONERROR(gcoVX_KernelConstruct(hwContext));

    if (!C->codeGenOnly)
    {
        gcmONERROR(gcoVX_LockKernel(hwContext));
        gcmONERROR(gcoVX_BindKernel(hwContext));
        if (C->uniform_num > 0)
        {
            gctUINT32 i;

            for(i = 0; i < C->uniform_num; i++)
            {
                gcoVX_BindUniform(GC_VX_UNIFORM_PIXEL, C->uniforms[i].index, (gctUINT32*)&C->uniforms[i].uniform, C->uniforms[i].num);
            }
        }
    }
#else
    gcsVX_KERNEL_PARAMETERS         *Context = &C->params;
    gcoVX_Kernel_Context_Uniform    *Uniform = C->uniforms;
    gctUINT32                       i = 0, *Num = &C->uniform_num;
    gceSTATUS                       status = gcvSTATUS_OK;
    gcoVX_Hardware_Context          *hwContext = gcvNULL;

    gcmHEADER_ARG("C=%p", C);

#if gcdVX_OPTIMIZER
    {
        hwContext = &C->hwContext;
    }
#else
    {
        for (i = 0; i < GC_VX_MAX_HARDWARE_CONTEXT; i++)
        {
            if (C->hwContext[i] == gcvNULL)
            {
                break;
            }
            else
            {
                gcoVX_Hardware_Context hardwareContext = *C->hwContext[i];

                gctUINT32 borders = gcvVX_BORDER_MODE_UNDEFINED;

                _SetBorderMode(Context->borders, &borders);

                /*check if hardwareContext exist*/
                if ((hardwareContext.borders != borders) || (hardwareContext.clamp != Context->clamp) ||
                    (hardwareContext.col != Context->col) || (hardwareContext.constant_value != Context->constant_value) ||
                    (hardwareContext.inputFormat != Context->inputFormat) || (hardwareContext.outputFormat != Context->outputFormat) ||
                    (hardwareContext.inputMultipleWidth != Context->inputMultipleWidth) || (hardwareContext.outputMultipleWidth != Context->outputMultipleWidth) ||
                    (hardwareContext.kernel != Context->kernel) || (hardwareContext.maxLevel != Context->maxLevel) ||
                    (hardwareContext.policy != Context->policy) || (hardwareContext.rounding != Context->rounding) ||
                    (hardwareContext.row != Context->row) || (hardwareContext.scale != Context->scale) || (hardwareContext.factor != Context->factor) ||
                    (hardwareContext.step != Context->step) || (hardwareContext.volume != Context->volume) || (hardwareContext.winSize != Context->winSize) ||
                    (hardwareContext.xstep != Context->xstep) || (hardwareContext.ystep != Context->ystep) ||
                    (hardwareContext.input_count != Context->input_count) || (*hardwareContext.unifor_num != *Num))
                {
                    continue;
                }
                else if((gcoOS_MemCmp(hardwareContext.input_type, Context->input_type, Context->input_count * sizeof(Context->input_type[0])) == gcvSTATUS_MISMATCH) ||
                        (gcoOS_MemCmp(hardwareContext.output_type, Context->output_type, Context->input_count * sizeof(Context->output_type[0])) == gcvSTATUS_MISMATCH))
                {
                    continue;
                }
                else if (*Num != 0)
                {
                    if (gcoOS_MemCmp(hardwareContext.uniform, Uniform, (*Num) *sizeof(gcoVX_Kernel_Context_Uniform)) == gcvSTATUS_MISMATCH)
                    {
                        continue;
                    }
                }

                hwContext = C->hwContext[i];
                gcoOS_MemFill(&Context->instructions, 0, sizeof(gcoVX_Instructions));
                break;
            }
        }

        if (hwContext == gcvNULL)
        {
            if (i == GC_VX_MAX_HARDWARE_CONTEXT)
                i =0;
            if (C->hwContext[i] != gcvNULL)
            {
                vxFree(C->hwContext[i]);
            }
            /* Allocal a local hwContext for execution. */
            C->hwContext[i] = (gcoVX_Hardware_Context *) vxAllocate(sizeof(gcoVX_Hardware_Context));
            if (C->hwContext[i] == gcvNULL) return (gceSTATUS)VX_ERROR_NO_MEMORY;
            hwContext = C->hwContext[i];

#if GC_VX_ASM
            if(Context->instructions.source != gcvNULL)
            {
                gcoOS_MemFill(&Context->instructions.binarys, 0, sizeof(Context->instructions.binarys));
                Context->instructions.count = 0;
            }
            else
#endif
            gcoOS_MemFill(&Context->instructions, 0, sizeof(gcoVX_Instructions));
        }
    }
#endif

#if gcdVX_OPTIMIZER
    _SetBorderMode(C->borders, &hwContext->borders);
#else
    _SetBorderMode(Context->borders, &hwContext->borders);
#endif

    hwContext->instructions    = &Context->instructions;
    hwContext->kernel          = Context->kernel;
    hwContext->step            = Context->step;
    hwContext->xstep           = Context->xstep;
    hwContext->ystep           = Context->ystep;
    hwContext->policy          = Context->policy;
    hwContext->rounding        = Context->rounding;
    hwContext->scale           = Context->scale;
    hwContext->factor          = Context->factor;
    hwContext->volume          = Context->volume;

    hwContext->constant_value  = Context->constant_value;
    hwContext->order           = &Context->order;

    hwContext->input_count     = Context->input_count;
    hwContext->output_count    = Context->output_count;

    for(i = 0; i < Context->input_count; i++)
        hwContext->input_type[i]   = Context->input_type[i];

    for(i = 0; i < Context->output_count; i++)
        hwContext->output_type[i]  = Context->output_type[i];

    hwContext->matrix          = Context->matrix;
    hwContext->matrix1         = Context->matrix1;
    hwContext->uniform         = Uniform;
    hwContext->unifor_num      = Num;

    hwContext->col             = Context->col;
    hwContext->row             = Context->row;

    hwContext->clamp           = Context->clamp;

    hwContext->inputFormat     = Context->inputFormat;
    hwContext->outputFormat    = Context->outputFormat;
    hwContext->inputMultipleWidth  = Context->inputMultipleWidth;
    hwContext->outputMultipleWidth = Context->outputMultipleWidth;

    hwContext->isUseInitialEstimate = Context->isUseInitialEstimate;
    hwContext->maxLevel = Context->maxLevel;
    hwContext->winSize = Context->winSize;

    hwContext->evisNoInst = Context->evisNoInst;

    for (i = 0; i < sizeof(hwContext->optionalOutputs) / sizeof(vx_uint32); i++)
    {
        hwContext->optionalOutputs[i] = Context->optionalOutputs[i];
    }


    gcmONERROR(gcoVX_KernelConstruct(hwContext));
    Context->hasBarrier = hwContext->hasBarrier;
    Context->hasAtomic  = hwContext->hasAtomic;
#if gcdVX_OPTIMIZER
    if (!C->codeGenOnly)
    {
        gcmONERROR(gcoVX_LockKernel(hwContext));
        gcmONERROR(gcoVX_BindKernel(hwContext));
    }

    if (!C->codeGenOnly)
#endif
    {
        if (C->uniform_num > 0)
        {
            for(i = 0; i < (GC_VX_MAX_ARRAY * GC_VX_MAX_ARRAY)/*C->uniform_num*/; i++)
            {
                if (C->uniforms[i].num > 0)
                gcoVX_BindUniform(GC_VX_UNIFORM_PIXEL, C->uniforms[i].index, (gctUINT32*)&C->uniforms[i].uniform, C->uniforms[i].num);
            }
        }
    }

#endif

OnError:
    gcmFOOTER_ARG("%d", status);

    return status;
}

gcoVX_Kernel_Context_Object*
gcoVX_AddObject(
    IN OUT gcoVX_Kernel_Context* context,
    IN gc_vx_object_type_t type,
    IN void* object,
    IN gctUINT32 index
    )
{
    gcoVX_Kernel_Context_Object *obj = &context->obj[context->objects_num];

    obj->type       = type;
    obj->obj        = object;
    obj->index      = (GC_VX_INDEX_AUTO == index)?context->objects_num:index;

    context->objects_num ++;

    return obj;
}

gceSTATUS gcfVX_AllocateMemForImageFromHandle(vx_image image, vx_uint32 planeIndx)
{
    gcsSURF_NODE_PTR imageNode = gcvNULL;
    gceSTATUS status = gcvSTATUS_OK;

    vxQuerySurfaceNode((vx_reference)image, planeIndx, (void**)&imageNode);
    if (imageNode == NULL)
    {
        vx_context context = vxGetContext((vx_reference)image);

        vx_rectangle_t rect;
        vx_size size = 0;
        gctUINT32_PTR logical = 0;

        vxGetValidRegionImage(image, &rect);
        size = vxComputeWholeImageSize(image, &rect, planeIndx);

        status = gcoVX_AllocateMemory((gctUINT32)size, (gctPOINTER*)&logical,
                                    &image->memory.physicals[planeIndx],
                                    &image->memory.nodePtrs[planeIndx]);

        context->memoryCount ++;

        imageNode = image->memory.nodePtrs[planeIndx];

        if (size > 0)
            gcoOS_MemCopy(imageNode->logical, image->memory.logicals[planeIndx], size);
    }

    return status;
}

gceSTATUS
gcfVX_GetImageInfo(
    IN gcoVX_Kernel_Context* Context,
    IN vx_image Image,
    IN gcsVX_IMAGE_INFO_PTR Info,
    IN vx_uint32 Multiply
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    vx_uint32 plane = 0;
    vx_df_image format;
    gcsSURF_NODE_PTR node = gcvNULL;

    gcmHEADER_ARG("Context=%p", Context);

#if gcdVX_OPTIMIZER
    _SetBorderMode(Context->borders, &Info->border);
#else
    _SetBorderMode(Context->params.borders, &Info->border);
#endif

    vxQueryImage(Image, VX_IMAGE_WIDTH, &Info->width, sizeof(vx_uint32));
    vxQueryImage(Image, VX_IMAGE_HEIGHT, &Info->height, sizeof(vx_uint32));
    vxQueryImage(Image, VX_IMAGE_FORMAT, &format, sizeof(format));

    /* Initialize image info.*/
    gcmONERROR(_FillImageInfoFromFormat(format, Info));

    /* Process all planes.*/
    for (plane = 0; plane < Info->planes; plane++)
    {
        vx_uint32 w, h, b;
        /* Get width, height, and bytes per pixel for this plane.*/
        w = (plane > 0) ? Info->width / Info->uPixels : Info->width;
        h = (plane > 0) ? Info->height / Info->vPixels : Info->height;
        if (Info->planes == 2 && plane == 1)
            b = Info->bpp / 4;
        else
            b = Info->bpp / 8;

        /* Compute the size and advance the address.*/
        Info->stride[plane] = gcmALIGN(w * b, (b == 3) ? 4 : b);
        Info->bytes = gcmALIGN(Info->stride[plane] * h, 64);

        /*overwrite the stride for the 1 plane case*/
        /*Stride must larger than 0, Graph.ReplicateNode */
        if (Info->planes == 1 && Image->memory.strides[plane][VX_DIM_Y] > 0)
        {
            Info->stride[plane] = Image->memory.strides[plane][VX_DIM_Y];
        }

        if (Image->importType == VX_MEMORY_TYPE_HOST)
        {
            if(Image->useInternalMem == vx_false_e)
            {
                Info->physicals[plane] = (gctUINT32)Image->memory.physicals[plane];
            }
            else
            {
                gcfVX_AllocateMemForImageFromHandle(Image, plane);

                Info->physicals[plane] = (gctUINT32)Image->memory.physicals[plane];
                Info->logicals[plane] = (gctPOINTER)Image->memory.nodePtrs[plane]->logical;
            }
        }
        else if (Image->importType == VX_MEMORY_TYPE_DMABUF ||
                 Image->importType == VX_MEMORY_TYPE_INTERNAL ||
                 Image->importType == VX_MEMORY_TYPE_HOST_UNCACHED)
        {
            Info->physicals[plane] = (gctUINT32)Image->memory.physicals[plane];
#if gcdDUMP
            Info->logicals[plane] = Image->memory.logicals[plane];
#endif
        }
        else
        {
            node = gcvNULL;
            vxQuerySurfaceNode((vx_reference)Image, plane, (void**)&node);

            Info->physicals[plane] = (gctUINT32)(Image->memory.physicals[plane] + Image->memory.offset[plane]);
            Info->logicals[plane] = Image->memory.logicals[plane];
        }
    }

    if (Multiply)
    {
        Info->bpp = 8;
        Info->width *= Multiply;
    }

    /* for imageArray/image3D  plane count should be 1 */
    Info->arraySize = Image->arraySize;
    Info->sliceSize = Info->height * Info->stride[0];

OnError:
    gcmFOOTER_ARG("%d", status);

    return status;
}

gceSTATUS
gcfVX_GetImageInfoFromTensor(
    IN vx_enum              borderMode,
    IN vx_tensor            tensor,
    vx_uint32               batchID,
    IN gcsVX_IMAGE_INFO_PTR Info
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("borderMode=%d", borderMode);

    if ((tensor->viewRegion.dimCount < 2) || (tensor->viewRegion.dimCount > 4))
    {
        status =  gcvSTATUS_INVALID_ARGUMENT;
        goto OnError;
    }

    _SetBorderMode(borderMode, &Info->border);

    Info->width  = tensor->viewRegion.viewEnds[0] - tensor->viewRegion.viewStarts[0];
    Info->height = tensor->viewRegion.viewEnds[1] - tensor->viewRegion.viewStarts[1];

    Info->isFloat = gcvFALSE;
    /* Initialize information structure.*/
    switch (TENSOR_DATA_TYPE(tensor))
    {
    case VX_TYPE_INT8:
    case VX_TYPE_BOOL8:
        Info->format = 0x4;
        Info->planes = 1;
        Info->bpp = 8;
        Info->componentCount = 1;
        Info->internalFormat = gcvSURF_R8;
        break;
    case VX_TYPE_UINT8:
        Info->format = 0x7;
        Info->planes = 1;
        Info->bpp = 8;
        Info->componentCount = 1;
        Info->internalFormat = gcvSURF_R8;
        break;
    case VX_TYPE_INT16:
        Info->format = 0x3;
        Info->planes = 1;
        Info->bpp = 16;
        Info->componentCount = 1;
        Info->internalFormat = gcvSURF_R16;
        break;
    case VX_TYPE_UINT16:
        Info->format = 0x6;
        Info->planes = 1;
        Info->bpp = 16;
        Info->componentCount = 1;
        Info->internalFormat = gcvSURF_R16;
        break;
    case VX_TYPE_INT32:
        Info->format = 0x2;
        Info->planes = 1;
        Info->bpp = 32;
        Info->componentCount = 1;
        Info->internalFormat = gcvSURF_R32;
        break;
    case VX_TYPE_UINT32:
        Info->format = 0x5;
        Info->planes = 1;
        Info->bpp = 32;
        Info->componentCount = 1;
        Info->internalFormat = gcvSURF_R32;
        break;
    case VX_TYPE_FLOAT32:
        Info->format = 0x0;
        Info->planes = 1;
        Info->bpp = 32;
        Info->componentCount = 1;
        Info->internalFormat = gcvSURF_R32F;
        Info->isFloat = gcvTRUE;
        break;
    case VX_TYPE_FLOAT16:
        Info->format = 0x1;
        Info->planes = 1;
        Info->bpp = 16;
        Info->componentCount = 1;
        Info->internalFormat = gcvSURF_R16F;
        Info->isFloat = gcvTRUE;
        break;
    case VX_TYPE_INT64:
        Info->format = 0x2;
        Info->planes = 1;
        Info->bpp = 32;
        Info->componentCount = 2;
        Info->width *= 2;
        Info->internalFormat = gcvSURF_R32;
        Info->isFloat = gcvFALSE;
        break;
    case VX_TYPE_BFLOAT16:
        Info->format = 0x6;
        Info->planes = 1;
        Info->bpp = 16;
        Info->componentCount = 1;
        Info->internalFormat = gcvSURF_R16;
        Info->isFloat = gcvFALSE;
        break;
    default:
        status = gcvSTATUS_INVALID_ARGUMENT;
        goto OnError;
    }

    vxoTensor_GetTensorBatchArrayViewMemory(tensor, batchID, &(Info->logicals[0]), &(Info->physicals[0]));
    Info->stride[0] = TENSOR_STRIDE_INDEX(tensor, 1);
    Info->sliceSize = TENSOR_STRIDE_INDEX(tensor, 2);

    /* for imageArray/image3D  plane count should be 1 */
    Info->arraySize = (tensor->viewRegion.dimCount >= 3 ? (tensor->viewRegion.viewEnds[2] - tensor->viewRegion.viewStarts[2]) : 1);

    Info->bytes = gcmALIGN(Info->stride[0] * Info->height, 64);

OnError:
    gcmFOOTER_ARG("%d", status);

    return status;
}


static gceSTATUS _GetInfo(gcoVX_Kernel_Context *Context)
{
    gceSTATUS status = gcvSTATUS_OK;
    vx_uint32 i = 0;
    gcsVX_IMAGE_INFO_PTR global = gcvNULL;
    gcsSURF_NODE_PTR node = gcvNULL;

    gcmHEADER_ARG("Context=%p", Context);

    Context->params.input_count = 0;
    Context->params.output_count = 0;

    for(i = 0; i < Context->objects_num; i++)
    {
        gcoVX_Kernel_Context_Object* object = &Context->obj[i];
        gcsVX_IMAGE_INFO_PTR info = &object->info;

        switch(object->type)
        {
        case GC_VX_CONTEXT_OBJECT_IMAGE_INPUT:

            gcfVX_GetImageInfo(Context, (vx_image)object->obj, info, Context->params.inputMultipleWidth);

            Context->params.input_type[Context->params.input_count++] = info->format;

            gcmONERROR(gcoVX_BindImage(object->index, info));

            if(global == gcvNULL)
                global = info;

            break;

        case GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT:
            if (object->obj)
            {
                gcfVX_GetImageInfo(Context, (vx_image)object->obj, info, Context->params.outputMultipleWidth);

                Context->params.output_type[Context->params.output_count++] = info->format;

                gcmONERROR(gcoVX_BindImage(object->index, info));
            }

            break;

        case GC_VX_CONTEXT_OBJECT_REMAP:

            vxQueryRemap(((vx_remap)object->obj), VX_REMAP_DESTINATION_WIDTH, &info->width, sizeof(vx_uint32));
            vxQueryRemap(((vx_remap)object->obj), VX_REMAP_DESTINATION_HEIGHT, &info->height, sizeof(vx_uint32));

            info->bpp               = 64;
            info->planes            = 1;
            info->componentCount    = 4;
            info->stride[0]         = info->width * 8;
            info->isFloat           = gcvTRUE;
            info->bytes             = gcmALIGN(info->stride[0] * info->height, 64);

            vxQuerySurfaceNode((vx_reference)object->obj, 0, (void**)&node);

            info->physicals[0] = (gcsSURF_NODE_GetHWAddress(node) != ~0u) ? (gcsSURF_NODE_GetHWAddress(node)) : (node->hardwareAddresses[2]);
            info->logicals[0] = node->logical;

            gcmONERROR(gcoVX_BindImage(object->index, info));

            break;

        case GC_VX_CONTEXT_OBJECT_DISTRIBUTION:

            node = gcvNULL;
            vxQuerySurfaceNode((vx_reference)object->obj, 0, (void**)&node);

            info->physicals[0] = (gcsSURF_NODE_GetHWAddress(node) != ~0u) ? (gcsSURF_NODE_GetHWAddress(node)) : (node->hardwareAddresses[2]);
            info->logicals[0] = node->logical;

#if !gcdVX_OPTIMIZER
            gcmONERROR(gcoVX_BindUniform(GC_VX_UNIFORM_PIXEL, object->index, &info->physicals[0], 1));
#endif

            break;

        case GC_VX_CONTEXT_OBJECT_LUT:

            vxQueryLUT((vx_lut)object->obj, VX_LUT_COUNT, &info->width, sizeof(vx_uint32));

            _FillImageInfoFromFormat(VX_DF_IMAGE_U8, info);

            info->height = 1;
            vxQuerySurfaceNode((vx_reference)object->obj, 0, (void**)&node);

            info->physicals[0] = (gcsSURF_NODE_GetHWAddress(node) != ~0u) ? (gcsSURF_NODE_GetHWAddress(node)) : (node->hardwareAddresses[2]);
            info->logicals[0] = node->logical;

            gcmONERROR(gcoVX_BindImage(object->index, info));

            break;

        case GC_VX_CONTEXT_OBJECT_SCALAR:

            _FillImageInfoFromFormat(VX_DF_IMAGE_S16, info);

            info->width = 16;
            info->height = 1;
            info->bytes = 4 * 4;

            vxQuerySurfaceNode((vx_reference)object->obj, 0, (void**)&node);

            info->physicals[0] = (gcsSURF_NODE_GetHWAddress(node) != ~0u) ? (gcsSURF_NODE_GetHWAddress(node)) : (node->hardwareAddresses[2]);
            info->logicals[0] = node->logical;

            gcmONERROR(gcoVX_BindImage(object->index, info));

            break;

        case GC_VX_CONTEXT_OBJECT_ARRAY:
            {
                vx_size capacity = 0, itemsize = 0, numItems = 0;
                status = (gceSTATUS)vxQueryArray((vx_array)object->obj, VX_ARRAY_CAPACITY, &capacity, sizeof(capacity));
                status = (gceSTATUS)vxQueryArray((vx_array)object->obj, VX_ARRAY_ITEMSIZE, &itemsize, sizeof(itemsize));
                status = (gceSTATUS)vxQueryArray((vx_array)object->obj, VX_ARRAY_NUMITEMS, &numItems, sizeof(numItems));

                _FillImageInfoFromFormat(VX_DF_IMAGE_S16, info);

                info->width = (vx_uint32)capacity;
                info->height = 1;
                info->bytes = (numItems > 0) ? (vx_uint32)(numItems * itemsize) : (vx_uint32)(capacity * itemsize);

                vxQuerySurfaceNode((vx_reference)object->obj, 0, (void**)&node);

                info->physicals[0] = (gcsSURF_NODE_GetHWAddress(node) != ~0u) ? (gcsSURF_NODE_GetHWAddress(node)) : (node->hardwareAddresses[2]);
                info->logicals[0] = node->logical;

                gcmONERROR(gcoVX_BindImage(object->index, info));
            }
            break;
        }
    }

    Context->params.xmax = (Context->params.xmax > 0) ? Context->params.xmax : (global ? global->width : 0);
    Context->params.xstep = (Context->params.xstep > 0) ?Context->params.xstep : 1;

    Context->params.ymax = (Context->params.ymax > 0) ? Context->params.ymax : (global ? global ->height : 0);
    Context->params.ystep = (Context->params.ystep > 0) ? Context->params.ystep : 1;

    Context->params.threadcount = 1;

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

#if gcdVX_OPTIMIZER
gceSTATUS gcfVX_BindObjects(
    IN gcoVX_Kernel_Context *Context
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    vx_uint32 i = 0;

    gcmHEADER_ARG("Context=%p", Context);

    for(i = 0; i < Context->objects_num; i++)
    {
        gcoVX_Kernel_Context_Object* object = &Context->obj[i];
        gcsVX_IMAGE_INFO_PTR info = &object->info;
        gctUINT32 plane;

        switch(object->type)
        {
        case GC_VX_CONTEXT_OBJECT_IMAGE_INPUT:
        case GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT:
        case GC_VX_CONTEXT_OBJECT_REMAP:
        case GC_VX_CONTEXT_OBJECT_LUT:
        case GC_VX_CONTEXT_OBJECT_SCALAR:
        case GC_VX_CONTEXT_OBJECT_ARRAY:
            for (plane = 0; plane < info->planes; plane++)
            {
                gcmONERROR(gcoVX_BindUniform(GC_VX_UNIFORM_PIXEL, object->index + plane, info->uniformData[plane], 4));
            }
            break;

        case GC_VX_CONTEXT_OBJECT_DISTRIBUTION:
            gcmONERROR(gcoVX_BindUniform(GC_VX_UNIFORM_PIXEL, object->index, &info->physicals[0], 1));
            break;
        }
    }

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}
#endif

gceSTATUS gcfVX_Commit(
    IN gcoVX_Kernel_Context *Context
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    vx_uint32 i = 0;

    gcmHEADER_ARG("Context=%p", Context);

    for(i = 0; i < Context->objects_num; i++)
    {
        vxCommitSurfaceNode((vx_reference)Context->obj[i].obj);
#if gcdDUMP
        /* Todo: Add distribution and arry*/
        if (Context->obj[i].type == GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT)
        {
            /* Dump memory */
            gcmDUMP_BUFFER(gcvNULL,
                        gcvDUMP_BUFFER_VERIFY,
                        Context->obj[i].info.physicals[0],
                        (gctPOINTER)Context->obj[i].info.logicals[0],
                        0,
                        Context->obj[i].info.stride[0] * Context->obj[i].info.height);
        }
#endif
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

static gceSTATUS _SyncMemoryForOutPut(gcoVX_Kernel_Context *Context)
{
    gceSTATUS status = gcvSTATUS_OK;
    vx_uint32 i = 0, plane = 0;

    gcmHEADER_ARG("Context=%p", Context);

    for(i = 0; i < Context->objects_num; i++)
    {
        gcoVX_Kernel_Context_Object* object = &Context->obj[i];

        switch(object->type)
        {
        case GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT:
            {
                vx_rectangle_t rect;
                vx_image image = (vx_image)object->obj;

                if (!object->obj ) continue;
                if(image->importType != VX_MEMORY_TYPE_HOST || image->useInternalMem == vx_false_e)
                    continue;

                gcoVX_Flush(gcvTRUE);
                vxGetValidRegionImage(image, &rect);

                for (plane = 0; plane < image->memory.planeCount; plane++)
                {
                    if (image->memory.nodePtrs[plane] != VX_NULL && image->memory.logicals[plane] != (image->memory.nodePtrs[plane]->logical + image->memory.offset[plane]))
                    {
                        vx_size size = 0;
                        size = vxComputeWholeImageSize(image, &rect, plane);
                        /*Only copy different memory. For CTS GraphROI.Simple */
                        if (size > 0 && (abs((vx_int32)(gcmALL_TO_UINT32(image->memory.logicals[plane]) - gcmALL_TO_UINT32(image->memory.nodePtrs[plane]->logical))) > (vx_int32)size))
                            gcoOS_MemCopy(image->memory.logicals[plane], image->memory.nodePtrs[plane]->logical, size);
                    }
                }
            }
            break;
        default:
            break;
        }
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

static gceSTATUS _RunKernel(gcoVX_Kernel_Context *Context)
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Context=%p", Context);

    gcmASSERT(gcoVX_VerifyHardware());
    /* get infos from objects*/
    gcmONERROR(_GetInfo(Context));

#if gcdVX_OPTIMIZER
    if (!Context->codeGenOnly)
    {
        gcmONERROR(gcfVX_BindObjects(Context));
    }
#endif

    gcmONERROR(_BindInstructions(Context));

#if VX_NN_SH_PARALLEL
    /* sync CNN layer */
    if (Context->node->cnnWaitEventID0 != 0xffffffff)
    {
        if ((Context->node->cnnWaitEventID0 > 0) && (Context->node->cnnWaitEventID0 < 32))
            gcoVX_WaitNNEvent(Context->node->cnnWaitEventID0);
        else
            gcmPRINT("cnnWaitEventID0 = %d\n", Context->node->cnnWaitEventID0);
    }
#endif

#if gcdVX_OPTIMIZER
    if (!Context->codeGenOnly)
#endif
    {
        gcmONERROR(gcoVX_InvokeKernel(&Context->params));
    }

    gcmONERROR(_SyncMemoryForOutPut(Context));

#if VX_NN_SH_PARALLEL
    if (Context->node->cnnWaitEventID1 != 0xffffffff)
    {
        if ((Context->node->cnnWaitEventID1 > 0) && (Context->node->cnnWaitEventID1 < 32))
           gcoVX_WaitNNEvent(Context->node->cnnWaitEventID1);
        else
           gcmPRINT("Error cnnWaitEventID1 = %d\n", Context->node->cnnWaitEventID1);
    }
#endif

#if gcdDUMP
    /* Need dump output of every node while gcdDUMP is set                              */
    /* While measuring perf for each kernel inside driver, */
    /* please enable this part so that the measurement approach can keep consistent.    */
    /* This is a temporary way for now.                                                 */

    /* stall */
    gcmONERROR(gcoVX_Commit(gcvFALSE, gcvTRUE, NULL, NULL));

    /* post semaphore to all objects*/
    gcmONERROR(gcfVX_Commit(Context));
#endif

OnError:
    gcmFOOTER_ARG("%d", status);

    return status;
}


vx_status
gcfVX_Kernel(
    IN gcoVX_Kernel_Context *Context
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("Context=%p", Context);

    gcmONERROR(_RunKernel(Context));

    gcmFOOTER_ARG("%d", status);
    return VX_SUCCESS;

OnError:
    gcmFOOTER_ARG("%d", status);

    return VX_FAILURE;
}

vx_status
gcfVX_Flush(
    IN gctBOOL      Stall
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Stall=%d", Stall);
    gcoVX_Flush(Stall);

    gcmFOOTER_ARG("%d", status);
    return status;
}

vx_status gcfVX_Accel(
    IN gctUINT32                CmdAddress,
    IN gceVX_ACCELERATOR_TYPE   Type,
    IN gctUINT32                EventId,
    IN gctBOOL                  waitEvent,
    IN gctUINT32                gpuId,
    IN gctBOOL                  sync,
    IN gctUINT32                syncEventID
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("CmdAddress=0x%x Type=%d EventId=%d waitEvent=%d", CmdAddress, Type, EventId, waitEvent);

    gcmASSERT(gcoVX_VerifyHardware());
    gcmONERROR(gcoVX_TriggerAccelerator(
        CmdAddress,
        Type,
        EventId,
        waitEvent,
        gpuId,
        sync,
        syncEventID
        ));

OnError:
    gcmFOOTER_ARG("%d", status);
    return (gcmIS_SUCCESS(status) ? VX_SUCCESS : VX_ERROR_NOT_SUPPORTED);

}


vx_status
gcfVX_CaptureState(
    gctUINT8_PTR CaptureBuffer,
    gctUINT32 InputSizeInByte,
    gctUINT32 *pOutputSizeInByte,
    gctBOOL Enabled,
    gctBOOL dropCommandEnabled
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("CaptureBuffer=%p InputSizeInByte=%d pOutputSizeInByte=%p Enabled=%d dropCommandEnabled=%d",
                   CaptureBuffer, InputSizeInByte, pOutputSizeInByte, Enabled, dropCommandEnabled);

    gcmASSERT(gcoVX_VerifyHardware());
#if gcdDEBUG
    if (Enabled)
    {
        gcmASSERT(CaptureBuffer && InputSizeInByte);
    }
    else
    {
        gcmASSERT(pOutputSizeInByte);
    }
#endif
    gcmONERROR(gcoVX_CaptureState(CaptureBuffer, InputSizeInByte, pOutputSizeInByte, Enabled, dropCommandEnabled));

OnError:
    gcmFOOTER_ARG("%d", status);
    return (gcmIS_SUCCESS(status) ? VX_SUCCESS : VX_ERROR_NO_MEMORY);
}

