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

gceSTATUS gcfVX_Kernel_ConvertFormat(
    IN vx_df_image Format,
    OUT gcsVX_IMAGE_INFO_PTR Info
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Format=%d Info=%d", Format, Info);

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
#define   GCREG_SH_INSTRUCTION_TYPE_FLOAT32                                  0x0
#define   GCREG_SH_INSTRUCTION_TYPE_FLOAT16                                  0x1
#define   GCREG_SH_INSTRUCTION_TYPE_SIGNED32                                 0x2
#define   GCREG_SH_INSTRUCTION_TYPE_SIGNED16                                 0x3
#define   GCREG_SH_INSTRUCTION_TYPE_SIGNED8                                  0x4
#define   GCREG_SH_INSTRUCTION_TYPE_UNSIGNED32                               0x5
#define   GCREG_SH_INSTRUCTION_TYPE_UNSIGNED16                               0x6
#define   GCREG_SH_INSTRUCTION_TYPE_UNSIGNED8                                0x7

gctUINT32 gcfVX_ConvertFormat(vx_df_image vx_format)
{
    gctUINT32 format = 0;
    switch(vx_format)
    {
    case VX_DF_IMAGE_U8:
    case VX_DF_IMAGE_YUV4:
    case VX_DF_IMAGE_NV21:
    case VX_DF_IMAGE_NV12:
        format = GCREG_SH_INSTRUCTION_TYPE_UNSIGNED8;
        break;
    case VX_DF_IMAGE_U16:
    case VX_DF_IMAGE_UYVY:
    case VX_DF_IMAGE_YUYV:
        format = GCREG_SH_INSTRUCTION_TYPE_UNSIGNED16;
        break;
    case VX_DF_IMAGE_S16:
        format = GCREG_SH_INSTRUCTION_TYPE_SIGNED16;
        break;
    case VX_DF_IMAGE_U32:
    case VX_DF_IMAGE_RGB:
    case VX_DF_IMAGE_RGBX:
        format = GCREG_SH_INSTRUCTION_TYPE_UNSIGNED32;
        break;
    case VX_DF_IMAGE_S32:
        format = GCREG_SH_INSTRUCTION_TYPE_SIGNED32;
        break;
    case VX_DF_IMAGE_F32:
        format = GCREG_SH_INSTRUCTION_TYPE_FLOAT32;
        break;
    default:
        format = GCREG_SH_INSTRUCTION_TYPE_UNSIGNED8;
        break;
    }

    return format;
}

static vx_status _SetBorderMode(vx_enum border, gctUINT32 *viv_border)
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("border=%d viv_border=%d", border, viv_border);

    switch(border)
    {
    case VX_BORDER_MODE_UNDEFINED:
        *viv_border = gcvVX_BORDER_MODE_UNDEFINED;
        break;
    case VX_BORDER_MODE_CONSTANT:
        *viv_border = gcvVX_BORDER_MODE_CONSTANT;
        break;
    case VX_BORDER_MODE_REPLICATE:
        *viv_border = gcvVX_BORDER_MODE_REPLACEMENT;
        break;
    default:
        *viv_border = gcvVX_BORDER_MODE_UNDEFINED;
        break;
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

gceSTATUS
gcfVX_BindInstructions(
    IN gcoVX_Kernel_Context          *C
    )
{
#if gcdVX_OPTIMIZER > 1
    gceSTATUS                       status = gcvSTATUS_OK;
    gcoVX_Hardware_Context          *hwContext = &C->params;

    gcmHEADER_ARG("C=%d", C);

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

    gcmHEADER_ARG("C=%d", C);

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

                /*check if hardwareContext exist*/
                if ((hardwareContext.borders != Context->borders) || (hardwareContext.clamp != Context->clamp) ||
                    (hardwareContext.col != Context->col) || (hardwareContext.constant_value != Context->constant_value) ||
                    (hardwareContext.inputFormat != Context->inputFormat) || (hardwareContext.outputFormat != Context->outputFormat) ||
                    (hardwareContext.inputMultipleWidth != Context->inputMultipleWidth) || (hardwareContext.outputMultipleWidth != Context->outputMultipleWidth) ||
                    (hardwareContext.kernel != Context->kernel) || (hardwareContext.maxLevel != Context->maxLevel) ||
                    (hardwareContext.policy != Context->policy) || (hardwareContext.rounding != Context->rounding) ||
                    (hardwareContext.row != Context->row) || (hardwareContext.scale != Context->scale) ||
                    (hardwareContext.step != Context->step) || (hardwareContext.volume != Context->volume) || (hardwareContext.winSize != Context->winSize) ||
                    (hardwareContext.xstep != Context->xstep) || (hardwareContext.ystep != Context->ystep) ||
                    (hardwareContext.input_count != Context->input_count) || (*hardwareContext.unifor_num != *Num))
                {
                    continue;
                }
                else if((gcoOS_MemCmp(hardwareContext.input_type, Context->input_type, Context->input_count * sizeof(Context->input_type[0])) == gcvSTATUS_MISMATCH) ||
                        (gcoOS_MemCmp(hardwareContext.output_type, Context->output_type, Context->input_count * sizeof(Context->output_type[0])) == gcvSTATUS_MISMATCH) ||
                        (gcoOS_MemCmp(hardwareContext.uniform, Uniform, (*Num) *sizeof(gcoVX_Kernel_Context_Uniform))))
                {
                    continue;
                }

                hwContext = C->hwContext[i];
                gcoOS_MemFill(&Context->instructions, 0, sizeof(gcoVX_Instructions));
                break;
            }
        }

        if (hwContext == gcvNULL)
        {
            /* Allocal a local hwContext for execution. */
            C->hwContext[i] = (gcoVX_Hardware_Context *) vxAllocate(sizeof(gcoVX_Hardware_Context));
            if (C->hwContext[i] == gcvNULL) return VX_ERROR_NO_MEMORY;
            hwContext = C->hwContext[i];
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

    hwContext->isUseInitialEstimate = Context->isUseInitialEstimate;
    hwContext->maxLevel = Context->maxLevel;
    hwContext->winSize = Context->winSize;

    hwContext->evisNoInst = Context->evisNoInst;

    gcmONERROR(gcoVX_KernelConstruct(hwContext));
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
            for(i = 0; i < C->uniform_num; i++)
            {
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

gceSTATUS
gcfVX_GetImageInfo(
    IN gcoVX_Kernel_Context Context,
    IN vx_image Image,
    IN gcsVX_IMAGE_INFO_PTR Info,
    IN vx_uint32 Multiply
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    vx_uint32 plane = 0;
    vx_df_image format;
    gcsSURF_NODE_PTR node = gcvNULL;

    gcmHEADER_ARG("Context=%d", Context);

#if gcdVX_OPTIMIZER
    _SetBorderMode(Context.borders, &Info->border);
#else
    _SetBorderMode(Context.params.borders, &Info->border);
#endif

    vxQueryImage(Image, VX_IMAGE_ATTRIBUTE_WIDTH, &Info->width, sizeof(vx_uint32));
    vxQueryImage(Image, VX_IMAGE_ATTRIBUTE_HEIGHT, &Info->height, sizeof(vx_uint32));
    vxQueryImage(Image, VX_IMAGE_ATTRIBUTE_FORMAT, &format, sizeof(format));

    /* Initialize information structure.*/
    Info->isFloat   = gcvFALSE;
    Info->format    = gcfVX_ConvertFormat(format);

    gcmONERROR(gcfVX_Kernel_ConvertFormat(format, Info));

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

        node = gcvNULL;
        vxQuerySurfaceNode((vx_reference)Image, plane, (void**)&node);

        Info->physicals[plane] = (node->hardwareAddresses[1] != ~0) ? (node->hardwareAddresses[1]) : (node->hardwareAddresses[2]);
        Info->logicals[plane] = (gctUINT32)node->logical;
    }

    if (Multiply)
    {
        Info->bpp = 8;
        Info->width *= Multiply;
    }

OnError:
    gcmFOOTER_ARG("%d", status);

    return status;
}

gceSTATUS gcfVX_GetInfo(
    IN gcoVX_Kernel_Context *Context
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    vx_uint32 i = 0;
    gcsVX_IMAGE_INFO_PTR globel = gcvNULL;
    gcsSURF_NODE_PTR node = gcvNULL;

    gcmHEADER_ARG("Context=%d", Context);

    Context->params.input_count = 0;
    Context->params.output_count = 0;

    for(i = 0; i < Context->objects_num; i++)
    {
        gcoVX_Kernel_Context_Object* object = &Context->obj[i];
        gcsVX_IMAGE_INFO_PTR info = &object->info;

        switch(object->type)
        {
        case GC_VX_CONTEXT_OBJECT_IMAGE_INPUT:

            gcfVX_GetImageInfo(*Context, (vx_image)object->obj, info, Context->params.inputMultipleWidth);

            Context->params.input_type[Context->params.input_count++] = info->format;

            gcmONERROR(gcoVX_BindImage(object->index, info));

            if(globel == gcvNULL)
                globel = info;

            break;

        case GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT:

            gcfVX_GetImageInfo(*Context, (vx_image)object->obj, info, Context->params.outputMultipleWidth);

            Context->params.output_type[Context->params.output_count++] = info->format;

            gcmONERROR(gcoVX_BindImage(object->index, info));

            break;

        case GC_VX_CONTEXT_OBJECT_REMAP:

            vxQueryRemap(((vx_remap)object->obj), VX_REMAP_ATTRIBUTE_DESTINATION_WIDTH, &info->width, sizeof(vx_uint32));
            vxQueryRemap(((vx_remap)object->obj), VX_REMAP_ATTRIBUTE_DESTINATION_HEIGHT, &info->height, sizeof(vx_uint32));

            info->bpp               = 64;
            info->planes            = 1;
            info->componentCount    = 4;
            info->stride[0]         = info->width * 8;
            info->isFloat           = gcvTRUE;
            info->bytes             = gcmALIGN(info->stride[0] * info->height, 64);

            vxQuerySurfaceNode((vx_reference)object->obj, 0, (void**)&node);

            info->physicals[0] = (node->hardwareAddresses[1] != ~0) ? (node->hardwareAddresses[1]) : (node->hardwareAddresses[2]);
            info->logicals[0] = (gctUINT32)node->logical;

            gcmONERROR(gcoVX_BindImage(object->index, info));

            break;

        case GC_VX_CONTEXT_OBJECT_DISTRIBUTION:

            node = gcvNULL;
            vxQuerySurfaceNode((vx_reference)object->obj, 0, (void**)&node);

            info->physicals[0] = (node->hardwareAddresses[1] != ~0) ? (node->hardwareAddresses[1]) : (node->hardwareAddresses[2]);
            info->logicals[0] = (gctUINT32)node->logical;

#if ! gcdVX_OPTIMIZER
            gcmONERROR(gcoVX_BindUniform(GC_VX_UNIFORM_PIXEL, object->index, &info->physicals[0], 1));
#endif

            break;

        case GC_VX_CONTEXT_OBJECT_LUT:

            vxQueryLUT((vx_lut)object->obj, VX_LUT_ATTRIBUTE_COUNT, &info->width, sizeof(vx_uint32));

            gcfVX_Kernel_ConvertFormat(VX_DF_IMAGE_U8, info);

            info->height = 1;
            vxQuerySurfaceNode((vx_reference)object->obj, 0, (void**)&node);

            info->physicals[0] = (node->hardwareAddresses[1] != ~0) ? (node->hardwareAddresses[1]) : (node->hardwareAddresses[2]);
            info->logicals[0] = (gctUINT32)node->logical;

            gcmONERROR(gcoVX_BindImage(object->index, info));

            break;

        case GC_VX_CONTEXT_OBJECT_SCALAR:

            gcfVX_Kernel_ConvertFormat(VX_DF_IMAGE_S16, info);

            info->width = 16;
            info->height = 1;
            info->bytes = 4 * 4;

            vxQuerySurfaceNode((vx_reference)object->obj, 0, (void**)&node);

            info->physicals[0] = (node->hardwareAddresses[1] != ~0) ? (node->hardwareAddresses[1]) : (node->hardwareAddresses[2]);
            info->logicals[0] = (gctUINT32)node->logical;

            gcmONERROR(gcoVX_BindImage(object->index, info));

            break;

        case GC_VX_CONTEXT_OBJECT_ARRAY:
            {
                vx_size capacity = 0, itemsize = 0, numItems = 0;
                status = (gceSTATUS)vxQueryArray((vx_array)object->obj, VX_ARRAY_ATTRIBUTE_CAPACITY, &capacity, sizeof(capacity));
                status = (gceSTATUS)vxQueryArray((vx_array)object->obj, VX_ARRAY_ATTRIBUTE_ITEMSIZE, &itemsize, sizeof(itemsize));
                status = (gceSTATUS)vxQueryArray((vx_array)object->obj, VX_ARRAY_ATTRIBUTE_NUMITEMS, &numItems, sizeof(numItems));

                gcfVX_Kernel_ConvertFormat(VX_DF_IMAGE_S16, info);

                info->width = (vx_uint32)capacity;
                info->height = 1;
                info->bytes = (numItems > 0) ? (vx_uint32)(numItems * itemsize) : (vx_uint32)(capacity * itemsize);

                vxQuerySurfaceNode((vx_reference)object->obj, 0, (void**)&node);

                info->physicals[0] = (node->hardwareAddresses[1] != ~0) ? (node->hardwareAddresses[1]) : (node->hardwareAddresses[2]);
                info->logicals[0] = (gctUINT32)node->logical;

                gcmONERROR(gcoVX_BindImage(object->index, info));
            }
            break;
        }
    }

    Context->params.xmax = (Context->params.xmax > 0) ? Context->params.xmax : globel->width;
    Context->params.xstep = Context->params.xstep;
    Context->params.ystep = (Context->params.ystep > 0) ?Context->params.ystep : 0;

    Context->params.ymax = (Context->params.ymax > 0) ? Context->params.ymax : globel->height;
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

    gcmHEADER_ARG("Context=%d", Context);

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

    gcmHEADER_ARG("Context=%d", Context);

    for(i = 0; i < Context->objects_num; i++)
    {
        vxCommitSurfaceNode((vx_reference)Context->obj[i].obj);
#if gcdDUMP
        /* Todo: Add distribution and arry*/
        if (Context->obj[i].type == GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT)
        {
            /* Dump memory */
            gcmDUMP_BUFFER(gcvNULL,
                        "verify",
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

vx_status
gcfVX_Kernel(
    IN gcoVX_Kernel_Context *Context
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Context=%d", Context);

    gcmONERROR(gcoVX_Initialize(NULL));

    /* get infos from objects*/
    gcmONERROR(gcfVX_GetInfo(Context));

#if gcdVX_OPTIMIZER
    if (!Context->codeGenOnly)
    {
        gcmONERROR(gcfVX_BindObjects(Context));
    }
#endif

    gcmONERROR(gcfVX_BindInstructions(Context));

#if gcdVX_OPTIMIZER
    if (!Context->codeGenOnly)
#endif
    {
        gcmONERROR(gcoVX_InvokeKernel(&Context->params));
    }

#if gcdDUMP
    /* Need dump output of every node while gcdDUMP is set                              */
    /* While measuring perf for each kernel inside driver,                              */
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
gcfVX_Flush(
    IN gctBOOL      Stall
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Stall=%d", Stall);

    /* stall */
    gcmONERROR(gcoVX_Commit(gcvTRUE, Stall, NULL, NULL));

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}
