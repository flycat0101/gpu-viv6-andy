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


#include "gc_gl_context.h"
#include "gl/gl_device.h"
#include "chip_context.h"

extern glsDEVICEPIPELINEGLOBAL dpGlobalInfo;

#define _GC_OBJ_ZONE    gcvZONE_API_GL

gceSURF_FORMAT __glVIVDevFormatToHWFormat [__GL_DEVFMT_MAX + 1]=
{
    gcvSURF_A8,                                /* __GL_DEVFMT_ALPHA8 */
    gcvSURF_A16,                               /* __GL_DEVFMT_ALPHA16 */
    gcvSURF_A32,                               /* __GL_DEVFMT_ALPHA24 */
    gcvSURF_A32F,                              /* __GL_DEVFMT_ALPHA32F */

    gcvSURF_L8,                                /* __GL_DEVFMT_LUMINANCE8 */
    gcvSURF_L16,                               /* __GL_DEVFMT_LUMINANCE16 */
    gcvSURF_L32,                               /* __GL_DEVFMT_LUMINANCE24 */
    gcvSURF_L32F,                              /* __GL_DEVFMT_LUMINANCE32F */

    gcvSURF_A8R8G8B8,                          /* __GL_DEVFMT_INTENSITY8 */
    gcvSURF_A16R16G16B16,                      /* __GL_DEVFMT_INTENSITY16 */
    gcvSURF_A32R32G32B32,                      /* __GL_DEVFMT_INTENSITY24 */
    gcvSURF_A32B32G32R32F,                     /* __GL_DEVFMT_INTENSITY32F */

    gcvSURF_A4L4,                              /*__GL_DEVFMT_LUMINANCE_ALPHA4 */
    gcvSURF_A8L8,                              /*__GL_DEVFMT_LUMINANCE_ALPHA8 */
    gcvSURF_A16L16,                            /*__GL_DEVFMT_LUMINANCE_ALPHA16 */

    gcvSURF_R5G6B5,                            /*__GL_DEVFMT_BGR565*/
    gcvSURF_A4R4G4B4,                          /*__GL_DEVFMT_BGRA444*/
    gcvSURF_A1R5G5B5,                          /*__GL_DEVFMT_BGRA5551*/
    gcvSURF_A8R8G8B8,                          /*__GL_DEVFMT_BGRA8888*/
    gcvSURF_A16R16G16B16,                      /*__GL_DEVFMT_RGBA16*/
    gcvSURF_X8R8G8B8,                          /*__GL_DEVFMT_BGRX8888*/
    gcvSURF_A2R10G10B10,                       /*__GL_DEVFMT_BGRA1010102*/
    gcvSURF_D16,                               /*__GL_DEVFMT_Z16*/
    gcvSURF_D24X8,                             /*__GL_DEVFMT_Z24*/
    gcvSURF_D24S8,                             /*__GL_DEVFMT_Z24_STENCIL*/
    gcvSURF_UNKNOWN,                           /*__GL_DEVFMT_Z32F*/
    gcvSURF_DXT1,                              /*__GL_DEVFMT_COMPRESSED_RGB_DXT1*/
    gcvSURF_DXT1,                              /*__GL_DEVFMT_COMPRESSED_RGBA_DXT1*/
    gcvSURF_DXT3,                              /*__GL_DEVFMT_COMPRESSED_RGBA_DXT3*/
    gcvSURF_DXT5,                              /*__GL_DEVFMT_COMPRESSED_RGBA_DXT5*/
    gcvSURF_UNKNOWN,                           /*__GL_DEVFMT_ZHIGH24*/
    gcvSURF_UNKNOWN,                           /*__GL_DEVFMT_ZHIGH24_STENCIL*/
    gcvSURF_UNKNOWN,                           /*__GL_DEVFMT_COMPRESSED_LUMINANCE_LATC1*/
    gcvSURF_UNKNOWN,                           /*__GL_DEVFMT_COMPRESSED_SIGNED_LUMINANCE_LATC1*/
    gcvSURF_UNKNOWN,                           /*__GL_DEVFMT_COMPRESSED_LUMINANCE_ALPHA_LATC2*/
    gcvSURF_UNKNOWN,                           /*__GL_DEVFMT_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2*/
    gcvSURF_UNKNOWN,                           /*__GL_DEVFMT_COMPRESSED_RED_RGTC1*/
    gcvSURF_UNKNOWN,                           /*__GL_DEVFMT_COMPRESSED_SIGNED_RED_RGTC1*/
    gcvSURF_UNKNOWN,                           /*__GL_DEVFMT_COMPRESSED_RED_GREEN_RGTC2*/
    gcvSURF_UNKNOWN,                           /*__GL_DEVFMT_COMPRESSED_SIGNED_RED_GREEN_RGTC2*/
    gcvSURF_A8R8G8B8,                          /*__GL_DEVFMT_RGBA8888*/
    gcvSURF_UNKNOWN,                           /*__GL_DEVFMT_RGBA8888_SIGNED*/
    gcvSURF_A32R32G32B32,                      /*__GL_DEVFMT_RGBA32UI*/
    gcvSURF_UNKNOWN,                           /*__GL_DEVFMT_RGBA32I*/
    gcvSURF_A16R16G16B16,                      /*__GL_DEVFMT_RGBA16UI*/
    gcvSURF_UNKNOWN,                           /*__GL_DEVFMT_RGBA16I*/
    gcvSURF_A8R8G8B8,                          /*__GL_DEVFMT_RGBA8UI*/
    gcvSURF_UNKNOWN,                           /*__GL_DEVFMT_RGBA8I*/
    gcvSURF_B32G32R32,                         /*__GL_DEVFMT_RGB32UI*/
    gcvSURF_UNKNOWN,                           /*__GL_DEVFMT_RGB32I*/
    gcvSURF_A16B16G16R16F,                     /*__GL_DEVFMT_RGBA16F*/
    gcvSURF_A32B32G32R32F,                     /*__GL_DEVFMT_RGBA32F*/
    gcvSURF_UNKNOWN,                           /*__GL_DEVFMT_SRGB_ALPHA */
    gcvSURF_UNKNOWN,                           /*__GL_DEVFMT_COMPRESSED_SRGB_S3TC_DXT1*/
    gcvSURF_UNKNOWN,                           /*__GL_DEVFMT_COMPRESSED_SRGB_ALPHA_S3TC_DXT1*/
    gcvSURF_UNKNOWN,                           /*__GL_DEVFMT_COMPRESSED_SRGB_ALPHA_S3TC_DXT3*/
    gcvSURF_UNKNOWN,                           /*__GL_DEVFMT_COMPRESSED_SRGB_ALPHA_S3TC_DXT5*/
    gcvSURF_UNKNOWN,                           /*__GL_DEVFMT_RGB9_E5 */
    gcvSURF_B32G32R32F,                        /*__GL_DEVFMT_RGB32F*/
    gcvSURF_UNKNOWN,                           /*__GL_DEVFMT_R11G11B10F*/
    gcvSURF_UNKNOWN,                           /*__GL_DEVFMT_STENCIL*/
    gcvSURF_UNKNOWN,                           /*__GL_DEVFMT_MAX*/
};

gceSURF_FORMAT getHWFormat(GLuint devFmt)
{
    return __glVIVDevFormatToHWFormat[devFmt];
}

#if DIRECT_TO_FB
GLboolean createDisplayBuffer(__GLcontext * gc, GLboolean bOnScreen,  gcoSURF *renderTarget)
{
    __GLdrawablePrivate *draw = gc->drawablePrivate;
    glsCHIPDRAWABLE_PTR  chipDraw = (glsCHIPDRAWABLE_PTR)(draw->dp.privateData);
    __GLdeviceFormat devFmt;
    gceSURF_FORMAT hwFormat;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("GC context=0x%x", gc);

    devFmt = (__GLdeviceFormat)(*__glDevice->devQueryDeviceFormat)(draw->internalFormatDisplayBuffer, GL_FALSE, 1);

    hwFormat = __glVIVDevFormatToHWFormat[devFmt];

    /* drawable surface */
    if (bOnScreen) {
        if (dpGlobalInfo.gpuAddress) {
            do {
                gcmERR_BREAK(gcoSURF_ConstructWrapper(gcvNULL,
                    &chipDraw->displayBuffer.renderTarget));
                *renderTarget = chipDraw->displayBuffer.renderTarget;

                gcmERR_BREAK(gcoSURF_SetBuffer(chipDraw->displayBuffer.renderTarget,
                    gcvSURF_BITMAP,
                    hwFormat,
                    dpGlobalInfo.stride,
                    dpGlobalInfo.logicalAddress,
                    dpGlobalInfo.gpuAddress
                    ));

                gcmERR_BREAK(gcoSURF_SetWindow(chipDraw->displayBuffer.renderTarget,
                    0,
                    0,
                    dpGlobalInfo.width,
                    dpGlobalInfo.height));
            } while(GL_FALSE);
        }
    } else {
        if (draw->backBufferPhysAddr) {
            do {
                gcmERR_BREAK(gcoSURF_ConstructWrapper(gcvNULL,
                    &chipDraw->displayBuffer.renderTarget));

                gcmERR_BREAK(gcoSURF_SetBuffer(chipDraw->displayBuffer.renderTarget,
                    gcvSURF_BITMAP,
                    hwFormat,
                    draw->wWidth * dpGlobalInfo.bpp,
                    draw->backBufferLogicalAddr,
                    draw->backBufferPhysAddr
                    ));

                gcmERR_BREAK(gcoSURF_SetWindow(chipDraw->displayBuffer.renderTarget,
                    0,
                    0,
                    draw->wWidth,
                    draw->wHeight));
            } while(GL_FALSE);
        }
    }

    gcmFOOTER_NO();
    return GL_TRUE;
}
#endif
extern GLvoid clearAccumBuffer(__GLcontext* gc, __GLdrawableBuffer* buffer);

GLboolean  createRenderBuffer(__GLcontext *gc, glsCHIPBUFFERCREATE * chipCreateInfo)
{
    __GLdrawableBuffer  *drawableBuf;
    gceSTATUS status = gcvSTATUS_OK;
    GLvoid   *chipRenderBuffer = NULL;
    gceSURF_FORMAT hwFormat;
    gcoSURF     renderTarget;
    GLuint bufSize = 0;

    gcmHEADER_ARG("chipCreateInfo=0x%x",
        chipCreateInfo);

    gcmVERIFY_ARGUMENT(chipCreateInfo != gcvNULL);

    switch(chipCreateInfo->flags)
    {
        case __GL_FRONT_BUFFER:
        case __GL_BACK_BUFFER:
        case __GL_RESOLVE_BUFFER_FLAG:
            bufSize = sizeof(glsCHIPRENDERBUFFER);
            break;
        case __GL_DEPTH_BUFFER:
            bufSize = sizeof(glsCHIPDEPTHBUFFER);
            break;
        case __GL_STENCIL_BUFFER:
            bufSize = sizeof(glsCHIPSTENCILBUFFER);
            break;
        case __GL_PBUFFERTEX_BUFFER:
            break;
        case __GL_ACCUM_BUFFER:
            bufSize = sizeof(glsCHIPACCUMBUFFER);
            break;
    }

    drawableBuf = (__GLdrawableBuffer *)chipCreateInfo->bufInfo;

    gcmONERROR(gcoOS_Allocate(gcvNULL,
               bufSize,
              (gctPOINTER *) &chipRenderBuffer));

    gcoOS_ZeroMemory(chipRenderBuffer, bufSize);

    hwFormat = __glVIVDevFormatToHWFormat[drawableBuf->deviceFormatInfo->devfmt];

    if (chipCreateInfo->flags != __GL_STENCIL_BUFFER) {
#if DIRECT_TO_FB
        if ((chipCreateInfo->flags == __GL_RESOLVE_BUFFER_FLAG) &&
           (chipCreateInfo->subFlags != 0)) {
            if (!createDisplayBuffer(gc, GL_TRUE, &renderTarget)) {
                goto OnError;
            }
        } else
#endif
        {
            /* create render target */
            status = gcoSURF_Construct(gcvNULL, drawableBuf->width, drawableBuf->height,
                1,   /* number of planes */
                chipCreateInfo->surfType,
                hwFormat,
                chipCreateInfo->poolType,
                &renderTarget
            );

            if (!gcmIS_SUCCESS(status))
            {
                goto OnError;
            }

            do {
                /* Set multi-sampling size. */
                gcmERR_BREAK(gcoSURF_SetSamples(
                         renderTarget,
                        chipCreateInfo->samples
                        ));
            } while (GL_FALSE);
        }
    }

    drawableBuf->privateData = chipRenderBuffer;

    switch(chipCreateInfo->flags) {
        case __GL_FRONT_BUFFER:
        case __GL_BACK_BUFFER:
        case __GL_RESOLVE_BUFFER_FLAG:
             gcmERR_BREAK(gcoSURF_GetAlignedSize(
                    renderTarget,
                    &((glsCHIPRENDERBUFFER *)chipRenderBuffer)->alignedWidth,
                    &((glsCHIPRENDERBUFFER *)chipRenderBuffer)->alignedHeight,
                    gcvNULL
                    ));
            ((glsCHIPRENDERBUFFER *)chipRenderBuffer)->renderTarget = renderTarget;
            ((glsCHIPRENDERBUFFER *)chipRenderBuffer)->renderTargetFormat = hwFormat;
            break;
        case __GL_ACCUM_BUFFER:
            ((glsCHIPACCUMBUFFER *)chipRenderBuffer)->renderTarget = renderTarget;
            ((glsCHIPACCUMBUFFER *)chipRenderBuffer)->renderTargetFormat = hwFormat;
            clearAccumBuffer(gc, drawableBuf);
            break;
        case __GL_DEPTH_BUFFER:
            ((glsCHIPDEPTHBUFFER *)chipRenderBuffer)->depthBuffer = renderTarget;
            ((glsCHIPDEPTHBUFFER *)chipRenderBuffer)->depthFormat = hwFormat;
            break;
        case __GL_STENCIL_BUFFER:
            break;
        case __GL_PBUFFERTEX_BUFFER:
            break;
    }

    gcmFOOTER_NO();
    return GL_TRUE;

OnError:
    if (chipRenderBuffer) {
        gcoOS_Free(gcvNULL, chipRenderBuffer);
    }
    gcmFOOTER_NO();
    return GL_FALSE;
}

gceSTATUS __glChipDestroyRenderBuffer(glCHIPBUFFERDESSTROY* chipDestroyInfo)
{
    __GLdrawableBuffer *drawableBuf;
    glsCHIPRENDERBUFFER *chipRenderBuffer;

    gcmHEADER_ARG("chipDestroyInfo=0x%x",
        chipDestroyInfo);

    gcmVERIFY_ARGUMENT(chipDestroyInfo != gcvNULL);

    drawableBuf = (__GLdrawableBuffer*)(chipDestroyInfo->bufInfo);

    if(drawableBuf->privateData == gcvNULL)
    {
        gcmFOOTER_NO();
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    if ((chipDestroyInfo->flags == __GL_FRONT_BUFFER) || (chipDestroyInfo->flags == __GL_BACK_BUFFER)) {
        chipRenderBuffer = (glsCHIPRENDERBUFFER*)(drawableBuf->privateData);
        if (chipRenderBuffer->resolveBits) {
              gcoSURF_Unlock(chipRenderBuffer->renderTarget, chipRenderBuffer->resolveBits);
        }
    }


    chipRenderBuffer = (glsCHIPRENDERBUFFER*)(drawableBuf->privateData);

    if (chipDestroyInfo->flags != __GL_STENCIL_BUFFER) {
        gcoSURF_Destroy(chipRenderBuffer->renderTarget);
    }

    gcoOS_Free(gcvNULL, drawableBuf->privateData);
    drawableBuf->privateData = NULL;

    switch(chipDestroyInfo->flags){
        case __GL_FRONT_BUFFER:
        case __GL_BACK_BUFFER:
            break;

        case __GL_DEPTH_BUFFER:
            break;

        case __GL_STENCIL_BUFFER:
            break;

        case __GL_PBUFFERTEX_BUFFER:
            break;

        case __GL_ACCUM_BUFFER:
            break;
    }

    gcmFOOTER_NO();
    return gcvSTATUS_OK;
}

GLvoid __glChipLockBuffer(__GLcontext *gc,
    void *buffer,
    GLuint format,
    GLuint **base,
    GLuint *pitch)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    __GLdrawableBuffer  *drawBuffer = NULL;
    gcoSURF     *chipBuffer;
    gctUINT32   alignheight;
    gceSTATUS status = gcvSTATUS_OK;
    gctPOINTER memoryResolve[3] = {gcvNULL};
    gceSURF_FORMAT hwFormat;
    gcoSURF     *target;

    drawBuffer = (__GLdrawableBuffer*)buffer;

    hwFormat = __glVIVDevFormatToHWFormat[drawBuffer->deviceFormatInfo->devfmt];
    switch(format)
    {
        case GL_STENCIL_INDEX:
            chipBuffer = &(((glsCHIPSTENCILBUFFER*)(drawBuffer->privateData))->lockTarget);
            target = &(((glsCHIPSTENCILBUFFER*)(drawBuffer->privateData))->stencilBuffer);
            break;

        case GL_DEPTH_COMPONENT:
            chipBuffer = &(((glsCHIPDEPTHBUFFER*)(drawBuffer->privateData))->lockTarget);
            target = &(((glsCHIPDEPTHBUFFER*)(drawBuffer->privateData))->depthBuffer);
            break;

        default:
            chipBuffer = &(((glsCHIPRENDERBUFFER*)(drawBuffer->privateData))->lockTarget);
            target = &(((glsCHIPRENDERBUFFER*)(drawBuffer->privateData))->renderTarget);
            break;
    }
    /* create render target */
    if(*chipBuffer)
    {
        GL_ASSERT(GL_FALSE);
    }
    else
    {
        gcmONERROR(gcoSURF_Construct(gcvNULL, drawBuffer->width, drawBuffer->height,
                1,   /* number of planes */
                gcvSURF_BITMAP,
                hwFormat,
                gcvPOOL_UNIFIED,
                chipBuffer));
        gcmONERROR(gcoSURF_Lock(*chipBuffer, gcvNULL, memoryResolve));
    }

    if (*chipBuffer) {
        gcsSURF_VIEW tgtView = {*target, 0, 1};
        gcsSURF_VIEW lockView = {*chipBuffer, 0, 1};
        gcsSURF_RESOLVE_ARGS rlvArgs = {0};

        rlvArgs.version = gcvHAL_ARG_VERSION_V2;
        rlvArgs.uArgs.v2.yInverted = gcvTRUE;
        rlvArgs.uArgs.v2.rectSize.x = drawBuffer->width;
        rlvArgs.uArgs.v2.rectSize.y = drawBuffer->height;
        rlvArgs.uArgs.v2.numSlices  = 1;
        gcoSURF_ResolveRect_v2(&tgtView, &lockView, &rlvArgs);
    }

    gcoSURF_Flush(*chipBuffer);
    gcoHAL_Commit(chipCtx->hal, gcvTRUE);

    /* Lock the surface. */
    gcoSURF_GetAlignedSize(*chipBuffer, NULL, &alignheight, (gctINT *)pitch);
    *base = memoryResolve[0];
    return;

OnError:
    if (*chipBuffer != NULL)
    {
        gcoSURF_Destroy(*chipBuffer);
        *chipBuffer = gcvNULL;
    }
    return;
}


GLvoid __glChipUnlockBuffer(__GLcontext *gc, void *buffer, GLuint format)
{
    __GLdrawableBuffer  *drawBuffer = (__GLdrawableBuffer*)buffer;
    gcoSURF     *chipBuffer;
    gcoSURF     *target;

    switch(format)
    {
        case GL_STENCIL_INDEX:
            chipBuffer = &(((glsCHIPSTENCILBUFFER*)(drawBuffer->privateData))->lockTarget);
            target = &(((glsCHIPSTENCILBUFFER*)(drawBuffer->privateData))->stencilBuffer);
            break;

        case GL_DEPTH_COMPONENT:
            chipBuffer = &(((glsCHIPDEPTHBUFFER*)(drawBuffer->privateData))->lockTarget);
            target = &(((glsCHIPDEPTHBUFFER*)(drawBuffer->privateData))->depthBuffer);
            break;

        default:
            chipBuffer = &(((glsCHIPRENDERBUFFER*)(drawBuffer->privateData))->lockTarget);
            target = &(((glsCHIPRENDERBUFFER*)(drawBuffer->privateData))->renderTarget);
            break;
    }

    if (*chipBuffer)
    {
        gceSURF_TYPE surftype = gcvSURF_TYPE_UNKNOWN;
        gcsSURF_VIEW tgtView = {*target, 0, 1};
        gcsSURF_VIEW lockView = {*chipBuffer, 0, 1};
        gcsSURF_RESOLVE_ARGS rlvArgs = {0};

        gcoSURF_GetFormat(*target, &surftype, gcvNULL);

        rlvArgs.version = gcvHAL_ARG_VERSION_V2;
        rlvArgs.uArgs.v2.yInverted = (surftype != gcvSURF_BITMAP) ? gcvTRUE : gcvFALSE;
        rlvArgs.uArgs.v2.rectSize.x = drawBuffer->width;
        rlvArgs.uArgs.v2.rectSize.y = drawBuffer->height;
        rlvArgs.uArgs.v2.numSlices  = 1;
        gcoSURF_ResolveRect_v2(&lockView, &tgtView, &rlvArgs);
    }

    gcoSURF_Unlock(*chipBuffer, NULL);
    gcoSURF_Destroy(*chipBuffer);
    *chipBuffer = NULL;
}
