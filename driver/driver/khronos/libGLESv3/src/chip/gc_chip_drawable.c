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


#include "gc_es_context.h"
#include "gc_es_device.h"
#include "gc_chip_context.h"
#define _GC_OBJ_ZONE    gcdZONE_ES30_TRACE

/***************************************************************************/
/* Implementation for internal functions                                   */
/***************************************************************************/

__GL_INLINE gceSTATUS
gcChipPickDrawBuffersForDrawable(
    __GLcontext *gc
    )
{
    __GLchipContext     *chipCtx = CHIP_CTXINFO(gc);
    __GLdrawablePrivate *drawable = gc->drawablePrivate;
    gcsSURF_VIEW         rtViews[__GL_MAX_DRAW_BUFFERS];
    gcsSURF_VIEW         dView = {gcvNULL, 0, 1};
    gcsSURF_VIEW         sView = {gcvNULL, 0, 1};
    GLint               sBpp = 0;
    GLboolean           yInverted = GL_FALSE;
    GLuint              samples = 0;
    GLuint              i;
    gceSTATUS           status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x", gc);

    for (i = 0; i < gc->constants.shaderCaps.maxDrawBuffers; ++i)
    {
        /* Init rtViews array to nullView first: dView now is nullView */
        rtViews[i] = dView;
    }

    if (drawable)
    {
        rtViews[0].surf = (gcoSURF)drawable->rtHandles[0];
        dView.surf      = (gcoSURF)drawable->depthHandle;
        sView.surf      = (gcoSURF)drawable->stencilHandle;

        if (rtViews[0].surf && drawable->prevRtHandles[0] &&
            !gcoSURF_QueryFlags(rtViews[0].surf, gcvSURF_FLAG_CONTENT_UPDATED) &&
            gcoSURF_QueryFlags(rtViews[0].surf, gcvSURF_FLAG_CONTENT_PRESERVED))
        {
            gcsRECT clearRect = {0, };
            gctBOOL fullClear = gcvFALSE;

            if (gc->flags & __GL_CONTEXT_SKIP_PRESERVE_CLEAR_RECT)
            {
                if (gc->state.enables.scissorTest)
                {
                    GLint width, height;
                    __GLscissor *pScissor = &gc->state.scissor;
                    gcmVERIFY_OK(gcoSURF_GetSize(rtViews[0].surf, (gctUINT *) &width, (gctUINT *) &height, gcvNULL));

                    clearRect.left   = __GL_MIN(__GL_MAX(0, pScissor->scissorX), width);
                    clearRect.top    = __GL_MIN(__GL_MAX(0, pScissor->scissorY), height);
                    clearRect.right  = __GL_MIN(__GL_MAX(0, pScissor->scissorX + pScissor->scissorWidth), width);
                    clearRect.bottom = __GL_MIN(__GL_MAX(0, pScissor->scissorY + pScissor->scissorHeight), height);

                    if ((clearRect.left   == 0) &&
                        (clearRect.top    == 0) &&
                        (clearRect.right  == (gctINT) width) &&
                        (clearRect.bottom == (gctINT) height))
                    {
                        fullClear = gcvTRUE;
                    }
                }
                else
                {
                    fullClear = gcvTRUE;
                }
            }

            if (!fullClear)
            {
                gcoSURF prev = (gcoSURF)drawable->prevRtHandles[0];
                gcmVERIFY_OK(gcoSURF_Preserve(prev, rtViews[0].surf, &clearRect));
            }

            gcmVERIFY_OK(gcoSURF_SetFlags(rtViews[0].surf, gcvSURF_FLAG_CONTENT_PRESERVED, gcvFALSE));
        }

        if (rtViews[0].surf)
        {
            yInverted = (gcoSURF_QueryFlags(rtViews[0].surf, gcvSURF_FLAG_CONTENT_YINVERTED) == gcvSTATUS_TRUE);
            gcmONERROR(gcoSURF_GetSamples(rtViews[0].surf, &samples));
        }
        else if (dView.surf)
        {
            yInverted = (gcoSURF_QueryFlags(dView.surf, gcvSURF_FLAG_CONTENT_YINVERTED) == gcvSTATUS_TRUE);
            gcmONERROR(gcoSURF_GetSamples(dView.surf, &samples));
        }
        else if (sView.surf)
        {
            yInverted = (gcoSURF_QueryFlags(dView.surf, gcvSURF_FLAG_CONTENT_YINVERTED) == gcvSTATUS_TRUE);
            gcmONERROR(gcoSURF_GetSamples(dView.surf, &samples));
        }

        if (sView.surf)
        {
            sBpp = drawable->dsFormatInfo->stencilSize;
        }
    }

    if (chipCtx->drawStencilMask != (GLuint) ((1 << sBpp) - 1))
    {
        chipCtx->drawStencilMask = (1 << sBpp) - 1;
        chipCtx->chipDirty.uDefer.sDefer.stencilRef = 1;
    }

    gc->state.raster.mrtEnable = GL_FALSE;

    gcmONERROR(gcChipSetDrawBuffers(gc,
                                    0,
                                    (GLboolean)gc->modes.rgbFloatMode,
                                    rtViews,
                                    &dView,
                                    &sView,
                                    yInverted,
                                    samples,
                                    GL_FALSE,
                                    0,
                                    0,
                                    GL_FALSE,
                                    1));

OnError:
    gcmFOOTER();
    return status;
}

__GL_INLINE gceSTATUS
gcChipPickReadBufferForDrawable(
    __GLcontext *gc
    )
{
    __GLdrawablePrivate *readable = gc->readablePrivate;
    gcsSURF_VIEW         rtView = {gcvNULL, 0, 1};
    gcsSURF_VIEW         dView  = {gcvNULL, 0, 1};
    gcsSURF_VIEW         sView  = {gcvNULL, 0, 1};
    GLboolean           yInverted = GL_FALSE;
    gceSTATUS           status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x", gc);

    if (readable)
    {
        rtView.surf = (gcoSURF)readable->rtHandles[0];
        dView.surf  = (gcoSURF)readable->depthHandle;
        sView.surf  = (gcoSURF)readable->stencilHandle;

#if gcdENABLE_BLIT_BUFFER_PRESERVE
        if (rtView.surf && readable->prevRtHandles[0] &&
            !gcoSURF_QueryFlags(rtView.surf, gcvSURF_FLAG_CONTENT_UPDATED) &&
            gcoSURF_QueryFlags(rtView.surf, gcvSURF_FLAG_CONTENT_PRESERVED))
        {
            gcoSURF prev = (gcoSURF) readable->prevRtHandles[0];
            gcmVERIFY_OK(gcoSURF_Preserve(prev, rtView.surf, gcvNULL));
            gcmVERIFY_OK(gcoSURF_SetFlags(rtView.surf, gcvSURF_FLAG_CONTENT_PRESERVED, gcvFALSE));
        }
#endif

        if (rtView.surf)
        {
            yInverted = (gcoSURF_QueryFlags(rtView.surf, gcvSURF_FLAG_CONTENT_YINVERTED) == gcvSTATUS_TRUE);
        }
        else if (dView.surf)
        {
            yInverted = (gcoSURF_QueryFlags(dView.surf, gcvSURF_FLAG_CONTENT_YINVERTED) == gcvSTATUS_TRUE);
        }
        else if (sView.surf)
        {
            yInverted = (gcoSURF_QueryFlags(sView.surf, gcvSURF_FLAG_CONTENT_YINVERTED) == gcvSTATUS_TRUE);
        }
    }

    /*
    **  Set read buffer states
    */
    gcmONERROR(gcChipSetReadBuffers(gc, 0, &rtView, &dView, &sView, yInverted, GL_FALSE));


OnError:
    gcmFOOTER();
    return status;
}

/***************************************************************************/
/* Implementation for EXPORTED FUNCTIONS                                   */
/***************************************************************************/

GLboolean
__glChipChangeDrawBuffers(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;

    gcmHEADER_ARG("gc=0x%x", gc);
    if (!gc->frameBuffer.drawFramebufObj->name)
    {
        gcmONERROR(gcChipPickDrawBuffersForDrawable(gc));
    }
    else
    {
        gcmONERROR(gcChipPickDrawBufferForFBO(gc));
    }

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;

}

GLboolean
__glChipChangeReadBuffers(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;
    gcmHEADER_ARG("gc=0x%x", gc);
    if (!gc->frameBuffer.readFramebufObj->name)
    {
        gcmONERROR(gcChipPickReadBufferForDrawable(gc));
    }
    else
    {
        gcmONERROR(gcChipPickReadBufferForFBO(gc));
    }

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}


GLvoid
__glChipDetachDrawable(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLdrawablePrivate *drawable = gc->drawablePrivate;
    __GLdrawablePrivate *readable = gc->readablePrivate;

    gcoSURF *surfList;
    GLuint surfCount = 0;
    gcmHEADER_ARG("gc=0x%x", gc);

    if (gcmIS_SUCCESS(gcoOS_Allocate(gcvNULL, __GL_CHIP_SURF_COUNT * sizeof(GLuint*), (gctPOINTER*)&surfList)))
    {
        gcoOS_ZeroMemory(surfList, __GL_CHIP_SURF_COUNT * sizeof(GLuint*));

        if (drawable)
        {
            if (drawable->rtHandles[0])
                surfList[surfCount++] = (gcoSURF)drawable->rtHandles[0];
            if (drawable->depthHandle)
                surfList[surfCount++] = (gcoSURF)drawable->depthHandle;
            if (drawable->stencilHandle)
                surfList[surfCount++] = (gcoSURF)drawable->stencilHandle;
        }

        if (readable)
        {
            if (readable->rtHandles[0])
                surfList[surfCount++] = (gcoSURF)readable->rtHandles[0];
            if (readable->depthHandle)
                surfList[surfCount++] = (gcoSURF)readable->depthHandle;
            if (readable->stencilHandle)
                surfList[surfCount++] = (gcoSURF)readable->stencilHandle;
        }

        GL_ASSERT(surfCount <= __GL_CHIP_SURF_COUNT);

        if (surfCount)
        {
            gcChipDetachSurface(gc, chipCtx, surfList, surfCount);
        }
        gcmOS_SAFE_FREE(gcvNULL, surfList);
    }

    gcmFOOTER_NO();
    return;
}


GLboolean
__glChipUpdateDrawable(
    __GLdrawablePrivate *drawable
    )
{
    gcePATCH_ID patchId = gcvPATCH_INVALID;
    __GLchipDrawable *chipDrawable = (__GLchipDrawable*)drawable->privateData;
    gceSTATUS status = gcvSTATUS_OK;
    GLboolean ret;

    gcmHEADER_ARG("drawable=%p", drawable);

    /* Get PatchID from HAL in the very beginning */
    gcmONERROR(gcoHAL_GetPatchID(gcvNULL, &patchId));

    if (!chipDrawable)
    {
        /* Allocate the array for the vertex attributes. */
        gcmONERROR(gcoOS_Allocate(gcvNULL,
                                  gcmSIZEOF(__GLchipDrawable),
                                  (gctPOINTER*)&chipDrawable));
        gcoOS_ZeroMemory(chipDrawable, gcmSIZEOF(__GLchipDrawable));
        drawable->privateData = chipDrawable;
    }

    /* Only enable stencil opt for those conformance tests */
    if (patchId == gcvPATCH_GTFES30 || patchId == gcvPATCH_DEQP)
    {
        GLint stencilSize = drawable->dsFormatInfo ? (gctSIZE_T)drawable->dsFormatInfo->stencilSize : 0;

        if (stencilSize > 0)
        {
            if (!chipDrawable->stencilOpt)
            {
                gcmONERROR(gcoOS_Allocate(gcvNULL,
                                          gcmSIZEOF(__GLchipStencilOpt),
                                          (gctPOINTER*)&chipDrawable->stencilOpt));
            }

            gcChipPatchStencilOptReset(chipDrawable->stencilOpt,
                                       (gctSIZE_T)drawable->width,
                                       (gctSIZE_T)drawable->height,
                                       (gctSIZE_T)stencilSize);
        }
        else
        {
            if (chipDrawable->stencilOpt)
            {
                gcmONERROR(gcoOS_Free(gcvNULL, (gctPOINTER)chipDrawable->stencilOpt));
                chipDrawable->stencilOpt = gcvNULL;
            }
        }
    }

OnError:
    ret = gcmIS_ERROR(status) ? GL_FALSE : GL_TRUE;
    gcmFOOTER_ARG("return=%d", ret);
    return ret;
}

GLvoid
__glChipDestroyDrawable(
    __GLdrawablePrivate *drawable
    )
{
    __GLchipDrawable *chipDrawable = (__GLchipDrawable*)drawable->privateData;
    gcmHEADER_ARG("drawable=0x%x", drawable);

    if (chipDrawable)
    {
        if (chipDrawable->stencilOpt)
        {
            gcmVERIFY_OK(gcoOS_Free(gcvNULL, (gctPOINTER)chipDrawable->stencilOpt));
            chipDrawable->stencilOpt = gcvNULL;
        }

        gcmVERIFY_OK(gcoOS_Free(gcvNULL, (gctPOINTER)chipDrawable));
        drawable->privateData = gcvNULL;
    }
    gcmFOOTER_NO();
}

