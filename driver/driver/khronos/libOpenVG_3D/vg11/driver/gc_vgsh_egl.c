/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_vgsh_precomp.h"
#include "gc_egl_common.h"

static void * (* veglGetCurVG11CtxFunc)(EGLenum) = gcvNULL;

#if VIVANTE_PROFILER
static void _ConstructChipName(
    _VGContext *Context
    )
{
    gctSTRING ChipName = Context->chipName;
    gctSTRING productName = gcvNULL;
    gceSTATUS status;

    gcmHEADER_ARG("Context=0x%x", Context);

    gcoOS_ZeroMemory(ChipName, vgdCHIP_NAME_LEN);

    /* Append Vivante GC to the string. */
    *ChipName++ = 'V';
    *ChipName++ = 'i';
    *ChipName++ = 'v';
    *ChipName++ = 'a';
    *ChipName++ = 'n';
    *ChipName++ = 't';
    *ChipName++ = 'e';
    *ChipName++ = ' ';

    gcmONERROR(gcoHAL_GetProductName(Context->hal, &productName));
    gcoOS_StrCatSafe(Context->chipName, vgdCHIP_NAME_LEN, productName);

    gcmOS_SAFE_FREE(Context->os, productName);

OnError:

    gcmFOOTER_NO();
    return;

}
#endif

#ifdef _DUMP_FILE
static void
_AddSurface(
    gcoDUMP Dump,
    gcoSURF Surface
    )
{
    gctUINT32 address;
    gctPOINTER memory[3];
    gctUINT width, height;
    gctINT stride;
    gceSURF_FORMAT format;

    /* Lock the surface. */
    gcmVERIFY_OK(gcoSURF_Lock(Surface, &address, memory));

    /* Get the size of the surface. */
    gcmVERIFY_OK(gcoSURF_GetAlignedSize(Surface, &width, &height, &stride));

    /* Get the format of the surface. */
    gcmVERIFY_OK(gcoSURF_GetFormat(Surface, gcvNULL, &format));

    /* Dump the surface. */
    gcmVERIFY_OK(gcoDUMP_AddSurface(Dump,
                                    width, height,
                                    format,
                                    address,
                                    stride * height));

    /* Unlock the surface. */
    gcmVERIFY_OK(gcoSURF_Unlock(Surface, memory[0]));
}
#endif

static gceSTATUS
_LoadCompiler(
    _VGContext * Context
    )
{
    gceSTATUS status;
    VSC_HW_CONFIG hwCfg;

    gcmHEADER();
    gcQueryShaderCompilerHwCfg(gcvNULL, &hwCfg);

    do
    {
        union __GLinitializerUnion
        {
            gctGLSLInitCompiler initGLSL;
            gctPOINTER          ptr;
        } intializer;

        union __GLfinalizerUnion
        {
            gctGLSLFinalizeCompiler finalizeGLSL;
            gctPOINTER ptr;
        } finalizer;

        status = gcoOS_LoadLibrary(gcvNULL,
#if defined(__APPLE__)
                                   "libGLSLC.dylib",
#elif defined(LINUXEMULATOR)
                                   "libGLSLC.so",
#elif defined(__QNXNTO__)
                                   "glesv2-sc-dlls",
#else
                                   "libGLSLC",
#endif
                                   &Context->dll);

        if (gcmIS_ERROR(status))
        {
            break;
        }

        gcmERR_BREAK(gcoOS_GetProcAddress(gcvNULL, Context->dll, "gcInitializeCompiler", &intializer.ptr));
        gcmERR_BREAK(gcoOS_GetProcAddress(gcvNULL, Context->dll, "gcFinalizeCompiler", &finalizer.ptr));

        Context->pfInitCompiler = intializer.initGLSL;
        Context->pfFinalizeCompiler = finalizer.finalizeGLSL;

        gcmERR_BREAK(Context->pfInitCompiler(gcvPATCH_INVALID, &hwCfg, gcvNULL));
    } while (gcvFALSE);

    gcmFOOTER_NO();
    return status;
}

static gceSTATUS
_ReleaseCompiler(
    _VGContext * Context
    )
{
    gceSTATUS status;

    gcmHEADER();

    gcmONERROR(Context->pfFinalizeCompiler());

OnError:
    gcmFOOTER_NO();
    return status;
}

static void *
veglCreateContext(
    void * Thread,
    gctINT ClientVersion,
    VEGLimports *Imports,
    void * SharedContext
    )
{
    _VGContext *context = gcvNULL;
    gcoOS os = gcvNULL;
    gcoHAL hal = gcvNULL;
    gco3D engine = gcvNULL;

    gcmHEADER_ARG("SharedContext=0x%x", SharedContext);

    do
    {
        gceSTATUS   status;

        gcmERR_BREAK(gcoOS_Construct(gcvNULL, &os));

        gcmERR_BREAK(gcoHAL_Construct(gcvNULL, os, &hal));

        gcmERR_BREAK(gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D));

        NEWOBJ(_VGContext, os, context);

        if (context == gcvNULL)
        {
            break;
        }

        /* Set veglGetCurAPICtxFunc function pointer. */
        if (veglGetCurVG11CtxFunc == gcvNULL && Imports)
        {
            veglGetCurVG11CtxFunc = Imports->getCurContext;
        }

        /* Create gco3D object pointer. */
        gcmERR_BREAK(gco3D_Construct(hal, gcvFALSE, &engine));

        context->os     = os;
        context->hal    = hal;
        context->engine = engine;
        context->phal   = gcvNULL;

        /* Query chip model. */
        gcmERR_BREAK(gcoHAL_QueryChipIdentity(context->hal,
                                              &context->model,
                                              &context->revision,
                                              gcvNULL, gcvNULL));

#if WALK600
        if (context->model == 0x0600 && context->revision == 0x19)
        {
            context->hardware.isGC600_19 = gcvTRUE;
        }
#endif
        context->hardware.featureVAA = gcvFALSE;

        context->hardware.hasTxDescriptor = gcoHAL_IsFeatureAvailable(
            hal, gcvFEATURE_TX_DESCRIPTOR
            ) == gcvSTATUS_TRUE;

        gcmERR_BREAK(_CreateSharedData(context, SharedContext));

#if VIVANTE_PROFILER
        /* Construct chip name. */
        _ConstructChipName(context);
        _vgshProfilerInitialize(context);
#endif

        /* Create gcoVERTEX object. */
        gcmERR_BREAK(gcoVERTEX_Construct(context->hal, &context->vertex));

        context->hardware.core.os       = os;
        context->hardware.core.hal      = hal;
        context->hardware.core.engine   = engine;
        context->hardware.context       = context;
        context->hardware.core.vertex   = context->vertex;

        /* Set HAL API to OpenGL. */
        gcmERR_BREAK(gco3D_SetAPI(engine, gcvAPI_OPENVG));

            /* Initialize states. */
        gcmERR_BREAK(gco3D_SetBlendFunction(context->engine,
                                        gcvBLEND_SOURCE,
                                        gcvBLEND_SOURCE_ALPHA, gcvBLEND_ONE));

        gcmERR_BREAK(gco3D_SetBlendFunction(context->engine,
                                        gcvBLEND_TARGET,
                                        gcvBLEND_INV_SOURCE_ALPHA, gcvBLEND_INV_SOURCE_ALPHA));


        gcmERR_BREAK(gco3D_EnableBlending(context->engine, gcvTRUE));

        gcmERR_BREAK(gco3D_SetBlendMode(context->engine, gcvBLEND_ADD, gcvBLEND_ADD));

        gcmERR_BREAK(gco3D_SetBlendColorF(context->engine, 0.0f, 0.0f, 0.0f, 0.0f));

        gcmERR_BREAK(gco3D_SetAlphaTest(context->engine, gcvFALSE));

        gcmERR_BREAK(gco3D_SetCulling(context->engine, gcvCULL_NONE));



        gcmERR_BREAK(gco3D_EnableDither(context->engine, gcvFALSE));


        gcmERR_BREAK(gco3D_SetScissors(context->engine,
                                   0,
                                   0,
                                   800,
                                   600));


        gcmERR_BREAK(gco3D_SetColorWrite(context->engine, 0xF));

        /* Initialize PA. */
        gcmERR_BREAK(gco3D_SetFill(context->engine, gcvFILL_SOLID));

        /*always turn the multisample*/
        gcmERR_BREAK(gco3D_SetAntiAlias(context->engine, gcvTRUE));

        gcmERR_BREAK(gco3D_SetShading(context->engine, gcvSHADING_SMOOTH));

        if (context->model > gcv2000 && gcoHAL_IsFeatureAvailable(context->hal, gcvFEATURE_WIDE_LINE) == gcvSTATUS_TRUE)
        {
            gcmERR_BREAK(gco3D_SetAntiAliasLine(context->engine, gcvTRUE));

            gcmERR_BREAK(gco3D_SetAALineWidth(context->engine, 2));
        }
        else
        {
            gcmERR_BREAK(gco3D_SetAntiAliasLine(context->engine, gcvFALSE));
        }

        gcmERR_BREAK(gco3D_SetLastPixelEnable(context->engine, gcvTRUE));

        /* Load compiler. */
        gcmERR_BREAK(_LoadCompiler(context));

        /* succeed to create context */
        gcmFOOTER_ARG("return=0x%x", context);
        return context;

    }while (gcvFALSE);

    /*  roll back the resources */
    if (engine)
    {
        gcmVERIFY_OK(gco3D_Destroy(engine));
    }

    if (hal)
    {
        gcmVERIFY_OK(gcoHAL_Destroy(hal));
    }

    if (os)
    {
        gcmVERIFY_OK(gcoOS_Destroy(os));
    }

    if (context != gcvNULL)
    {
        DELETEOBJ(_VGContext, os, context);
    }

    /* failed to create context */
    gcmFOOTER_ARG("return=0x%x", gcvNULL);
    return gcvNULL;
}

static EGLBoolean
veglDestroyContext(
    void * Thread,
    void * Context
    )
{
    _VGContext *context = (_VGContext*) Context;

    gcmHEADER_ARG("Context=0x%x", Context);

    if (context->engine)
    {
        gcmVERIFY_OK(gco3D_Destroy(context->engine));
    }

    if (context->hal)
    {
        gcmVERIFY_OK(gcoHAL_Destroy(context->hal));
    }

    if (context->os)
    {
        gcmVERIFY_OK(gcoOS_Destroy(context->os));
    }

    if (context->dll)
    {
        gcmVERIFY_OK(_ReleaseCompiler(context));
    }

    if (context != gcvNULL)
    {
        DELETEOBJ(_VGContext, context->os, context);

        gcmFOOTER_ARG("return=%s", "EGL_TRUE");
        return EGL_TRUE;
    }

    gcmFOOTER_ARG("return=%s", "EGL_FALSE");
    return EGL_FALSE;
}

static EGLBoolean
veglFlushContext(
    void * Context
    )
{
    gcmHEADER_ARG("Context=0x%x", Context);
    gcmFOOTER_ARG("return=%s", "EGL_TRUE");
    return EGL_TRUE;
}

static EGLBoolean
veglSetContext(
    void       * Thread,
    void       * Context,
    VEGLDrawable Drawable,
    VEGLDrawable Readable
    )
{
    gcoSURF Draw  = Drawable ? (gcoSURF)Drawable->rtHandles[0] : gcvNULL;
    gcoSURF Read  = Readable ? (gcoSURF)Readable->rtHandles[0] : gcvNULL;
    gcoSURF Depth = Drawable ? (gcoSURF)Drawable->depthHandle : gcvNULL;
    _VGContext *context = (_VGContext*) Context;

    gcmHEADER_ARG("Context=0x%x Draw=0x%x Read=0x%x Depth=0x%x", Context, Draw, Read, Depth);
#if gcmIS_DEBUG(gcdDEBUG_TRACE)
    gcoOS_SetDebugZone(gcvZONE_NONE);
#endif

    do
    {
        gceSTATUS   status;
        gcsSURF_CLEAR_ARGS clearArgs;

        gcmASSERT(context);

        /* Release context case */
        if ((Drawable == gcvNULL) && (Readable == gcvNULL))
        {

            gcmVERIFY_OK(gco3D_SetTarget(context->engine, 0, gcvNULL, 0));
            gcmVERIFY_OK(gco3D_SetDepth(context->engine, gcvNULL));
            gcmVERIFY_OK(gco3D_UnSet3DEngine(context->engine));
            gcmFOOTER_NO();
            return EGL_TRUE;
        }

        /* Start to set new context to this thread */
        gcmERR_BREAK(gco3D_Set3DEngine(context->engine));

        gcmERR_BREAK(gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D));

        if  (context->targetImage.surface == Draw)
        {
            gcmERR_BREAK(SetTarget(context, Draw, Read, Depth));

            context->hardware.core.programState.stateBuffer = gcvNULL;
            context->hardware.core.invalidCache     = gcvTRUE;

        }
        else
        {
            gcsSURF_VIEW dView = {Depth, 0, 1};

            gcmERR_BREAK(SetTarget(context, Draw, Read, Depth));

            context->depth = Depth;

            if (Draw == gcvNULL || Depth == gcvNULL)
            {
                /* succeed to set context */
                gcmFOOTER_ARG("return=%s", "EGL_TRUE");
                return EGL_TRUE;
            }

            if (context->targetImage.surface != gcvNULL)
            {
                _VGImageDtor(context->os, &context->targetImage);
            }

            if (context->maskImage.surface != gcvNULL)
            {
                _VGImageDtor(context->os, &context->maskImage);
            }

            _VGImageCtor(context->os, &context->targetImage);

            gcmERR_BREAK(vgshIMAGE_WrapFromSurface(context, &context->targetImage, Draw));

#if WALK600
            if (context->targetImage.width <= 64 && context->targetImage.height <= 64)
            {
                context->hardware.isConformance = gcvTRUE;
            }
#endif
            if (context->targetImage.internalColorDesc.colorFormat & PREMULTIPLIED)
            {
                gcmERR_BREAK(gco3D_SetBlendFunction(context->engine,
                                                gcvBLEND_SOURCE,
                                                gcvBLEND_ONE, gcvBLEND_ONE));

                gcmERR_BREAK(gco3D_SetBlendFunction(context->engine,
                                                gcvBLEND_TARGET,
                                                gcvBLEND_INV_SOURCE_ALPHA, gcvBLEND_INV_SOURCE_ALPHA));
            }
            else
            {
                gcmERR_BREAK(gco3D_SetBlendFunction(context->engine,
                                                gcvBLEND_SOURCE,
                                                gcvBLEND_SOURCE_ALPHA, gcvBLEND_ONE));

                gcmERR_BREAK(gco3D_SetBlendFunction(context->engine,
                                                gcvBLEND_TARGET,
                                                gcvBLEND_INV_SOURCE_ALPHA, gcvBLEND_INV_SOURCE_ALPHA));
            }


            context->hardware.core.programState.stateBuffer   = gcvNULL;
            context->hardware.core.invalidCache = gcvTRUE;
            context->hardware.core.draw         = Draw;

            gcmERR_BREAK(gco3D_SetClearColor(context->engine, 0, 0, 0, 0));
            gcmERR_BREAK(gco3D_SetClearDepthF(context->engine, 0.0f));
            gcmERR_BREAK(gco3D_SetClearStencil(context->engine, 0));

            gcoOS_ZeroMemory(&clearArgs, sizeof(clearArgs));
            clearArgs.depth.floatValue = 0.0f;
            clearArgs.depthMask = gcvTRUE;
            clearArgs.stencil = 0;
            clearArgs.stencilMask = 0xFF;
            clearArgs.clearRect = gcvNULL;
            clearArgs.flags = gcvCLEAR_DEPTH | gcvCLEAR_STENCIL;


            gcmERR_BREAK(gcoSURF_Clear(&dView, &clearArgs));
        }

        /* succeed to set context */
        gcmFOOTER_ARG("return=%s", "EGL_TRUE");
        return EGL_TRUE;
    }
    while(gcvFALSE);

    /* Error handling */

    gcmVERIFY_OK(gco3D_UnSet3DEngine(context->engine));

    /* failed to set context */
    gcmFOOTER_ARG("return=%s", "EGL_FALSE");
    return EGL_FALSE;
}

static EGLBoolean
veglUnsetContext(
    void * Thread,
    void * Context
    )
{
    return veglSetContext(Thread, Context, gcvNULL, gcvNULL);
}

static void
veglFlush(
    void
    )
{
    _VGContext* context;
    gcmHEADER();

    OVG_GET_CONTEXT(OVG_NO_RETVAL);

    flush(context);

    gcmFOOTER_NO();
}

static void
veglFinish(
    void
    )
{
    _VGContext* context;
    gcmHEADER();

    OVG_GET_CONTEXT(OVG_NO_RETVAL);

    finish(context);

    gcmFOOTER_NO();
}

static gcoSURF
veglGetClientBuffer(
    void * Context,
    EGLClientBuffer Buffer
    )
{
    _VGContext *ctx = (_VGContext*)Context;
    _VGImage   *image;
    gctINT32        referenceCount = 0;

    gcmHEADER_ARG("Context=0x%x Buffer=0x%x", Context, Buffer);

    if (!ctx)
    {
        gcmFOOTER_ARG("return=0x%x", gcvNULL);
        return gcvNULL;
    }

    image = _VGIMAGE(ctx, *(VGHandle*)&Buffer);
    if (!image)
    {
        gcmFOOTER_ARG("return=0x%x", gcvNULL);
        return gcvNULL;
    }

    gcoSURF_QueryReferenceCount(image->surface, &referenceCount);

    /* Test if surface is a sibling of any eglImage. */
    if (referenceCount > 1)
    {
        gcmFOOTER_ARG("return=0x%x", gcvNULL);
        return gcvNULL;
    }

    if (gcmIS_ERROR(gcoSURF_ReferenceSurface(image->surface)))
    {
        gcmFOOTER_ARG("return=0x%x", gcvNULL);
        return gcvNULL;
    }

    image->eglUsed  = gcvTRUE;

    gcmFOOTER_ARG("return=0x%x", image->surface);
    return image->surface;
}

_VGContext*
vgshGetCurrentContext(void)
{
    _VGContext* context = gcvNULL;

    gcmHEADER();

    if (veglGetCurVG11CtxFunc != gcvNULL)
    {
        context = veglGetCurVG11CtxFunc(EGL_OPENVG_API);
    }

    gcmFOOTER_ARG("return=0x%x", context);

    return context;
}

static EGLBoolean
veglQueryHWVG(
    void
    )
{
    return EGL_FALSE;
}

gceSTATUS
SetTarget(_VGContext *context, gcoSURF Draw, gcoSURF Read, gcoSURF Depth)
{
    gceSTATUS status;
    gcsSURF_VIEW drawView = {Draw, 0, 1};
    gcsSURF_VIEW dsView = {Depth, 0, 1};

    gcmHEADER_ARG("context=0x%x Draw=0x%x Read=0x%x Depth=0x%x",
                  context, Draw, Read, Depth);
    do
    {
        gctUINT width, height;
        gcmASSERT(context != gcvNULL);

        gcmERR_BREAK(gco3D_SetTarget(context->engine, 0, &drawView, 0));
        gcmERR_BREAK(gco3D_SetDepth(context->engine, &dsView));
        gcmERR_BREAK(gco3D_SetColorOutCount(context->engine, 1));

        /*gcmERR_BREAK(_SetCentroids(context->engine, Draw));*/

        if (Draw != gcvNULL)
        {
            gcmVERIFY_OK(gcoSURF_GetSize(Draw,
                                     &width,
                                     &height,
                                     gcvNULL));

            gcmERR_BREAK(gco3D_SetViewport(context->engine,
                                       0,
                                       0,
                                       width,
                                       height));

            gcmERR_BREAK(gco3D_SetScissors(context->engine,
                                       0,
                                       0,
                                       width,
                                       height));


            vgmSET_GAL_MATRIX(context->projection,
                2/(VGfloat)width,                  0,                     0,     -1,
                0,                                 2/(VGfloat)height,      0,     -1,
                0,                                 0,                      -1,     0,
                0,                                 0,                      0,      1);

        }

        /*caution: we should do the other depth initialize after the gco3D_SetDepth*/

        if (Depth != gcvNULL)
        {
            gcmERR_BREAK(gco3D_SetDepthMode(context->engine, gcvDEPTH_Z));

            gcmERR_BREAK(gco3D_SetDepthRangeF(context->engine, gcvDEPTH_Z, 0.0f, 1.0f));

            gcmERR_BREAK(gco3D_SetDepthCompare(context->engine, gcvCOMPARE_NOT_EQUAL));

            gcmERR_BREAK(gco3D_EnableDepthWrite(context->engine, gcvTRUE));

            gcmERR_BREAK(gco3D_SetDepthOnly(context->engine, gcvFALSE));

            gcmERR_BREAK(gco3D_SetDepthScaleBiasF(context->engine, 0.0f, 0.0f));

            gcmERR_BREAK(gco3D_SetStencilMode(context->engine, gcvSTENCIL_NONE));
            gcmERR_BREAK(gco3D_SetStencilReference(context->engine, 0, gcvTRUE));
            gcmERR_BREAK(gco3D_SetStencilReference(context->engine, 0, gcvFALSE));
            gcmERR_BREAK(gco3D_SetStencilCompare(context->engine,
                                             gcvSTENCIL_FRONT,
                                             gcvCOMPARE_ALWAYS));
            gcmERR_BREAK(gco3D_SetStencilCompare(context->engine,
                                             gcvSTENCIL_BACK,
                                             gcvCOMPARE_ALWAYS));
            gcmERR_BREAK(gco3D_SetStencilMask(context->engine, 0xFF));
            gcmERR_BREAK(gco3D_SetStencilWriteMask(context->engine, 0xFF));
            gcmERR_BREAK(gco3D_SetStencilFail(context->engine,
                                          gcvSTENCIL_FRONT,
                                          gcvSTENCIL_KEEP));
            gcmERR_BREAK(gco3D_SetStencilFail(context->engine,
                                          gcvSTENCIL_BACK,
                                          gcvSTENCIL_KEEP));
            gcmERR_BREAK(gco3D_SetStencilDepthFail(context->engine,
                                               gcvSTENCIL_FRONT,
                                               gcvSTENCIL_KEEP));
            gcmERR_BREAK(gco3D_SetStencilDepthFail(context->engine,
                                               gcvSTENCIL_BACK,
                                               gcvSTENCIL_KEEP));
            gcmERR_BREAK(gco3D_SetStencilPass(context->engine,
                                          gcvSTENCIL_FRONT,
                                          gcvSTENCIL_KEEP));
            gcmERR_BREAK(gco3D_SetStencilPass(context->engine,
                                          gcvSTENCIL_BACK,
                                          gcvSTENCIL_KEEP));
        }

        gcmFOOTER();
        return gcvSTATUS_OK;
    }
    while(gcvFALSE);

    gcmFOOTER();
    return status;
}

/* Find level-1 children images of an image. */
static int
FindChildImages(
    _VGContext  *context,
    _VGImage    *image,
    VGImage     **children)
{
    int count = 0;
    int i;
    _VGObject *object;

    gcmHEADER_ARG("context=0x%x image=0x%x children=0x%x",
                  context, image, children);

    /* Count all image children. */
    for (i = 0; i < NAMED_OBJECTS_HASH; i++)
    {
        object = context->sharedData->namedObjects[i];
        while (object != gcvNULL)
        {
            if (object != (_VGObject*)image &&
                object->type == VGObject_Image &&
                ((_VGImage*)object)->parent == image)
            {
                count++;
            }

            object = object->next;
        }
    }

    /* Capture all image children. */
    if (count > 0 && children != gcvNULL)
    {
        if (gcmIS_ERROR(gcoOS_Allocate(context->os, sizeof(VGImage) * count, (gctPOINTER) children)))
        {
            /* out-of-memory */
            gcmFOOTER_ARG("return=%d", count);
            return count;
        }

        count = 0;
        for (i = 0; i < NAMED_OBJECTS_HASH; i++)
        {
            object = context->sharedData->namedObjects[i];
            while (object != gcvNULL)
            {
                if (object != (_VGObject*)image &&
                    object->type == VGObject_Image &&
                    ((_VGImage*)object)->parent == image)
                {
                    (*children)[count] = (VGImage)object->name;
                    count++;
                }

                object = object->next;
            }
        }
    }

    /* Return the image children count. */
    gcmFOOTER_ARG("return=%d", count);
    return count;
}

/* Find and Count all child images, including child's child's child ... */
static int
FindAllImageDescents(
    _VGContext  *context,
    _VGImage    *root,
    VGImage     **descents
    )
{
    VGImage     *children = gcvNULL;
    VGImage     *currentSet = gcvNULL;
    _VGImage    *child_obj = gcvNULL;
    int         count = 0;
    int         currCount = 0;
    int         currImage = 0;

    gcmHEADER_ARG("context=0x%x root=0x%x descents=0x%x",
                  context, root, descents);

    /* Find the first level children. */
    count = FindChildImages(context, root, &children);
    if (children == gcvNULL)
    {
        *descents = gcvNULL;
        gcmFOOTER_ARG("return=%d", count);
        return count;
    }

    /* Access all found images to search for their children. */
    currImage = 0;
    do{
        child_obj = (_VGImage*)GetVGObject(context, VGObject_Image, children[currImage]);
        currCount = FindChildImages(context, child_obj, &currentSet);

        /* If this image has children, add them to the children array. */
        if (currCount > 0)
        {
            VGImage *temp;
            if (gcmIS_ERROR(gcoOS_Allocate(context->os,
                                        sizeof(VGImage) * (count + currCount),
                                        (gctPOINTER) &temp)))
            {
                gcmOS_SAFE_FREE(context->os, children);
                if (currentSet != gcvNULL)
                {
                    gcmOS_SAFE_FREE(context->os, currentSet);
                }
                gcmFATAL("%s(%d): gcoOS_Allocate failed", __FUNCTION__, __LINE__);
                gcmFOOTER_ARG("return=%d", count);
                return count;
            }
            gcoOS_MemCopy(temp, children, sizeof(VGImage) * count);     /*Copy the existing children.*/
            gcoOS_MemCopy(temp + count, currentSet, sizeof(VGImage) * currCount);   /*Copy the newly found children.*/
            gcmOS_SAFE_FREE(context->os, children);
            children = temp;
            count += currCount;
        }

        /* Move to the next image. */
        currImage++;
    }while( currImage < count);

    /* Copy the result and return the count. */
    if (descents != gcvNULL)
    {
        if (gcmIS_ERROR(gcoOS_Allocate(context->os,
                                       sizeof(VGImage) * count,
                                       (gctPOINTER*) descents)))
        {
            if (currentSet != gcvNULL)
            {
                gcmOS_SAFE_FREE(context->os, currentSet);
            }
            gcmOS_SAFE_FREE(context->os, children);
            gcmFATAL("%s(%d): gcoOS_Allocate failed", __FUNCTION__, __LINE__);
            gcmFOOTER_ARG("return=%d", count);
            return count;
        }

        if (*descents != gcvNULL)
        {
            gcoOS_MemCopy(*descents, children, sizeof(VGImage) * count);
        }
    }
    gcmOS_SAFE_FREE(context->os, children);
    if (currentSet != gcvNULL)
    {
        gcmOS_SAFE_FREE(context->os, currentSet);
    }

    gcmFOOTER_ARG("return=%d", count);
    return count;
}

static EGLenum
veglCreateImageParentImage(
    void * Context,
    unsigned int vgImage,
    void ** Images,
    int * Count
    )
{
    VGImage         *vgimages = gcvNULL;
    _VGImage        *vgimage_obj = gcvNULL;
    _VGImage        *child_obj = gcvNULL;
    khrEGL_IMAGE    *khImage = gcvNULL;
    int             childCount;
    int             i;
    gctINT32        referenceCount = 0;

    _VGContext* context = (_VGContext*) Context;
    gcmHEADER_ARG("vgImage=%d Images=0x%x Count=0x%x",
                  vgImage, Images, Count);

    /* Test for VgImage validation. */
    vgimage_obj = (_VGImage*)GetVGObject(context, VGObject_Image, vgImage);

    if  ((vgimage_obj == gcvNULL) ||
         (vgimage_obj->parent != gcvNULL) /*||
         (vgimage_obj->eglUsed)*/ )
    {
        gcmFOOTER_ARG("return=0x%x", EGL_BAD_ACCESS);
        return EGL_BAD_ACCESS;
    }

    gcoSURF_QueryReferenceCount(vgimage_obj->surface, &referenceCount);
    /* Test if surface is a sibling of any eglImage. */
    if (referenceCount > 1)
    {
        gcmFOOTER_ARG("return=0x%x", EGL_BAD_ACCESS);
        return EGL_BAD_ACCESS;
    }

    /* Get all child images. */
    childCount = FindAllImageDescents(context, vgimage_obj, &vgimages);

    /* Fill the results. */
    *Count = childCount + 1;
    if (gcmIS_ERROR(gcoOS_Allocate(context->os, sizeof(khrEGL_IMAGE) * (*Count), (gctPOINTER)Images)))
    {
        if (vgimages != gcvNULL)
        {
            gcmVERIFY_OK(gcmOS_SAFE_FREE(context->os, vgimages));
        }
        gcmFOOTER_ARG("return=0x%x", EGL_BAD_ALLOC);
        return EGL_BAD_ALLOC;
    }

    /* Fill the root image. */
    {
        child_obj = (_VGImage*)vgshFindObject(context, vgImage);
        khImage = (khrEGL_IMAGE *) *Images;

        khImage->magic = KHR_EGL_IMAGE_MAGIC_NUM;
        khImage->type  = KHR_IMAGE_VG_IMAGE;
        khImage->surface = vgimage_obj->surface;
        khImage->u.vgimage.texSurface = vgimage_obj->texSurface;
        /*Inc reference to surface. The egl's caller will do this.*/
        /*gcoSURF_ReferenceSurface(khImage->surface);   */
        khImage->u.vgimage.format = (gctUINT)vgimage_obj->internalColorDesc.format;
        khImage->u.vgimage.allowedQuality = (gctUINT)vgimage_obj->allowedQuality;
        khImage->u.vgimage.dirty = vgimage_obj->dirty;
        khImage->u.vgimage.dirtyPtr =  (khronos_int32_t *)vgimage_obj->dirtyPtr;
        khImage->u.vgimage.rootOffsetX = vgimage_obj->rootOffsetX;
        khImage->u.vgimage.rootOffsetY = vgimage_obj->rootOffsetY;
        khImage->u.vgimage.rootWidth = vgimage_obj->rootWidth;
        khImage->u.vgimage.rootHeight = vgimage_obj->rootHeight;

        if (child_obj != gcvNULL)
        {
            khImage->u.vgimage.width = child_obj->width;
            khImage->u.vgimage.height = child_obj->height;
            khImage->u.vgimage.offset_x = child_obj->parentOffsetX;
            khImage->u.vgimage.offset_y = child_obj->parentOffsetY;
        }
        else
        {
            khImage->u.vgimage.width = 0;
            khImage->u.vgimage.height = 0;
            khImage->u.vgimage.offset_x = 0;
            khImage->u.vgimage.offset_y = 0;
        }
    }

    /* Fill the child images. */
    for (i = 1; i <= childCount; i++)
    {
        child_obj = (_VGImage*)vgshFindObject(context, (VGuint)vgimages[i - 1]);
        khImage = ((khrEGL_IMAGE *) *Images) + i;

        khImage->magic = KHR_EGL_IMAGE_MAGIC_NUM;
        khImage->type  = KHR_IMAGE_VG_IMAGE;
        khImage->surface = vgimage_obj->surface;
        khImage->u.vgimage.texSurface = vgimage_obj->texSurface;
        /*gcoSURF_ReferenceSurface(khImage->surface);*/         /*Inc reference to surface.*/
        khImage->u.vgimage.format = (gctUINT)vgimage_obj->internalColorDesc.format;
        khImage->u.vgimage.allowedQuality = (gctUINT)vgimage_obj->allowedQuality;
        khImage->u.vgimage.dirty = vgimage_obj->dirty;
        khImage->u.vgimage.dirtyPtr = &khImage->u.vgimage.dirty;
        khImage->u.vgimage.rootWidth = vgimage_obj->rootWidth;
        khImage->u.vgimage.rootHeight = vgimage_obj->rootHeight;

        if (child_obj != gcvNULL)
        {
            khImage->u.vgimage.width = child_obj->width;
            khImage->u.vgimage.height = child_obj->height;
            khImage->u.vgimage.offset_x = child_obj->parentOffsetX;
            khImage->u.vgimage.offset_y = child_obj->parentOffsetY;
        }
        else
        {
            khImage->u.vgimage.width = 0;
            khImage->u.vgimage.height = 0;
            khImage->u.vgimage.offset_x = 0;
            khImage->u.vgimage.offset_y = 0;
        }
    }

    /* Free temporary resources. */
    if (vgimages != gcvNULL)
        gcmVERIFY_OK(gcmOS_SAFE_FREE(context->os, vgimages));

    gcmFOOTER_ARG("return=0x%x", EGL_SUCCESS);
    return EGL_SUCCESS;
}

static gctBOOL
veglProfiler(
    void * Profiler,
    gctUINT32 Enum,
    gctHANDLE Value
    )
{
#if VIVANTE_PROFILER
    _VGContext      *context;
    context = vgshGetCurrentContext();
    if (context == gcvNULL)
    {
        return gcvFALSE;
    }
    return _vgshProfilerSet(context, Enum, Value);
#else
    return gcvFALSE;
#endif
}

typedef struct _veglLOOKUP
{
    const char *                                name;
    __eglMustCastToProperFunctionPointerType    function;
}
veglLOOKUP;

#define eglMAKE_LOOKUP(function) \
    { #function, (__eglMustCastToProperFunctionPointerType) function }

static veglLOOKUP
_glaLookup[] =
{
    eglMAKE_LOOKUP(vgAppendPath),
    eglMAKE_LOOKUP(vgAppendPathData),
    eglMAKE_LOOKUP(vgChildImage),
    eglMAKE_LOOKUP(vgClear),
    eglMAKE_LOOKUP(vgClearGlyph),
    eglMAKE_LOOKUP(vgClearImage),
    eglMAKE_LOOKUP(vgClearPath),
    eglMAKE_LOOKUP(vgColorMatrix),
    eglMAKE_LOOKUP(vgConvolve),
    eglMAKE_LOOKUP(vgCopyImage),
    eglMAKE_LOOKUP(vgCopyMask),
    eglMAKE_LOOKUP(vgCopyPixels),
    eglMAKE_LOOKUP(vgCreateEGLImageTargetKHR),
    eglMAKE_LOOKUP(vgCreateFont),
    eglMAKE_LOOKUP(vgCreateImage),
    eglMAKE_LOOKUP(vgCreateMaskLayer),
    eglMAKE_LOOKUP(vgCreatePaint),
    eglMAKE_LOOKUP(vgCreatePath),
    eglMAKE_LOOKUP(vgDestroyFont),
    eglMAKE_LOOKUP(vgDestroyImage),
    eglMAKE_LOOKUP(vgDestroyMaskLayer),
    eglMAKE_LOOKUP(vgDestroyPaint),
    eglMAKE_LOOKUP(vgDestroyPath),
    eglMAKE_LOOKUP(vgDrawGlyph),
    eglMAKE_LOOKUP(vgDrawGlyphs),
    eglMAKE_LOOKUP(vgDrawImage),
    eglMAKE_LOOKUP(vgDrawPath),
    eglMAKE_LOOKUP(vgFillMaskLayer),
    eglMAKE_LOOKUP(vgFinish),
    eglMAKE_LOOKUP(vgFlush),
    eglMAKE_LOOKUP(vgGaussianBlur),
    eglMAKE_LOOKUP(vgGetColor),
    eglMAKE_LOOKUP(vgGetError),
    eglMAKE_LOOKUP(vgGetImageSubData),
    eglMAKE_LOOKUP(vgGetMatrix),
    eglMAKE_LOOKUP(vgGetPaint),
    eglMAKE_LOOKUP(vgGetParameterVectorSize),
    eglMAKE_LOOKUP(vgGetParameterf),
    eglMAKE_LOOKUP(vgGetParameterfv),
    eglMAKE_LOOKUP(vgGetParameteri),
    eglMAKE_LOOKUP(vgGetParameteriv),
    eglMAKE_LOOKUP(vgGetParent),
    eglMAKE_LOOKUP(vgGetPathCapabilities),
    eglMAKE_LOOKUP(vgGetPixels),
    eglMAKE_LOOKUP(vgGetString),
    eglMAKE_LOOKUP(vgGetVectorSize),
    eglMAKE_LOOKUP(vgGetf),
    eglMAKE_LOOKUP(vgGetfv),
    eglMAKE_LOOKUP(vgGeti),
    eglMAKE_LOOKUP(vgGetiv),
    eglMAKE_LOOKUP(vgHardwareQuery),
    eglMAKE_LOOKUP(vgImageSubData),
    eglMAKE_LOOKUP(vgInterpolatePath),
    /*eglMAKE_LOOKUP(vgIterativeAverageBlurKHR),*/
    eglMAKE_LOOKUP(vgLoadIdentity),
    eglMAKE_LOOKUP(vgLoadMatrix),
    eglMAKE_LOOKUP(vgLookup),
    eglMAKE_LOOKUP(vgLookupSingle),
    eglMAKE_LOOKUP(vgMask),
    eglMAKE_LOOKUP(vgModifyPathCoords),
    eglMAKE_LOOKUP(vgMultMatrix),
    eglMAKE_LOOKUP(vgPaintPattern),
    /*eglMAKE_LOOKUP(vgParametricFilterKHR),*/
    eglMAKE_LOOKUP(vgPathBounds),
    eglMAKE_LOOKUP(vgPathLength),
    eglMAKE_LOOKUP(vgPathTransformedBounds),
    eglMAKE_LOOKUP(vgPointAlongPath),
    /*eglMAKE_LOOKUP(vgProjectiveMatrixNDS),*/
    eglMAKE_LOOKUP(vgReadPixels),
    eglMAKE_LOOKUP(vgRemovePathCapabilities),
    eglMAKE_LOOKUP(vgRenderToMask),
    eglMAKE_LOOKUP(vgRotate),
    eglMAKE_LOOKUP(vgScale),
    eglMAKE_LOOKUP(vgSeparableConvolve),
    eglMAKE_LOOKUP(vgSetColor),
    eglMAKE_LOOKUP(vgSetGlyphToImage),
    eglMAKE_LOOKUP(vgSetGlyphToPath),
    eglMAKE_LOOKUP(vgSetPaint),
    eglMAKE_LOOKUP(vgSetParameterf),
    eglMAKE_LOOKUP(vgSetParameterfv),
    eglMAKE_LOOKUP(vgSetParameteri),
    eglMAKE_LOOKUP(vgSetParameteriv),
    eglMAKE_LOOKUP(vgSetPixels),
    eglMAKE_LOOKUP(vgSetf),
    eglMAKE_LOOKUP(vgSetfv),
    eglMAKE_LOOKUP(vgSeti),
    eglMAKE_LOOKUP(vgSetiv),
    eglMAKE_LOOKUP(vgShear),
    eglMAKE_LOOKUP(vgTransformPath),
    eglMAKE_LOOKUP(vgTranslate),
    eglMAKE_LOOKUP(vgWritePixels),
    eglMAKE_LOOKUP(vguArc),
    /*eglMAKE_LOOKUP(vguBevelKHR),*/
    eglMAKE_LOOKUP(vguComputeWarpQuadToQuad),
    eglMAKE_LOOKUP(vguComputeWarpQuadToSquare),
    eglMAKE_LOOKUP(vguComputeWarpSquareToQuad),
    /*eglMAKE_LOOKUP(vguDropShadowKHR),*/
    eglMAKE_LOOKUP(vguEllipse),
    /*eglMAKE_LOOKUP(vguGlowKHR),*/
    /*eglMAKE_LOOKUP(vguGradientBevelKHR),*/
    /*eglMAKE_LOOKUP(vguGradientGlowKHR),*/
    eglMAKE_LOOKUP(vguLine),
    eglMAKE_LOOKUP(vguPolygon),
    eglMAKE_LOOKUP(vguRect),
    eglMAKE_LOOKUP(vguRoundRect),
    /*eglMAKE_LOOKUP(vguTransformClipLineNDS),*/
    { gcvNULL, gcvNULL }
};

static EGL_PROC
veglGetProcAddr(
    const char *procName
    )
{
    veglLOOKUP *lookup = _glaLookup;

    /* Loop while there are entries in the lookup tabke. */
    while (lookup->name != gcvNULL)
    {
        if (gcmIS_SUCCESS(gcoOS_StrCmp(lookup->name, procName)))
        {
            return lookup->function;
        }

        /* Next lookup entry. */
        ++lookup;
    }

    return gcvNULL;
}

/* Dispatch table. */
#ifdef _WIN32
VG_API_CALL
#endif
veglDISPATCH
OpenVG_DISPATCH_TABLE =
{
    /* createContext            */  veglCreateContext,
    /* destroyContext           */  veglDestroyContext,
    /* makeCurrent              */  veglSetContext,
    /* loseCurrent              */  veglUnsetContext,
    /* setDrawable              */  veglSetContext,
    /* flushContext             */  veglFlushContext,
    /* flush                    */  veglFlush,
    /* finish                   */  veglFinish,
    /* getClientBuffer          */  veglGetClientBuffer,
    /* createImageTexture       */  gcvNULL,
    /* createImageRenderbuffer  */  gcvNULL,
    /* createImageParentImage   */  veglCreateImageParentImage,
    /* bindTexImage             */  gcvNULL,
    /* profiler                 */  veglProfiler,
    /* getProcAddr              */  veglGetProcAddr,
    /* swapBuffers              */  gcvNULL,
    /* queryHWVG                */  veglQueryHWVG,
    /* resolveVG                */  gcvNULL,
};
