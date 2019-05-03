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


#include "gc_es_context.h"
#include "gc_chip_context.h"

#define _GC_OBJ_ZONE    gcdZONE_GL40_CLEAR
#define glmUSE_3D_CLEAR 0


#if gcdFRAMEINFO_STATISTIC
extern GLboolean g_dbgPerDrawKickOff;
extern GLbitfield g_dbgDumpImagePerDraw;
extern GLboolean g_dbgSkipDraw;
#endif

/***************************************************************************/
/* Implementation for internal functions                                   */
/***************************************************************************/
__GL_INLINE gceSTATUS
gcChipGetClearRect(
    __GLcontext *gc,
    gcoSURF      surf,
    gcsRECT     *pClearRect,
    GLboolean   *pbFullClear
    )
{
    GLint width  = 0;
    GLint height = 0;
    gceSTATUS status = gcvSTATUS_OK;

    __GLchipContext  *chipCtx = CHIP_CTXINFO(gc);

    gcmHEADER_ARG("gc=0x%x surf=0x%x pClearRect=0x%x pbFullClear=0x%x",
                   gc, surf, pClearRect, pbFullClear);

    GL_ASSERT(surf);
    GL_ASSERT(pClearRect);
    GL_ASSERT(pbFullClear);

    __GL_MEMZERO(pClearRect, sizeof(gcsRECT));

    gcmONERROR(gcoSURF_GetSize(surf, (gctUINT*)&width, (gctUINT*)&height, gcvNULL));

    /* make compiler happy */
#ifndef __clang__
    chipCtx = chipCtx;
#endif

    if (gc->state.enables.scissorTest)
    {
        __GLscissor *pScissor = &(gc->state.scissor);

        /* ClearRect is the Intersection of scissor and RT size */
        pClearRect->left    = __GL_MIN(__GL_MAX(0, pScissor->scissorX), width);
        pClearRect->top     = __GL_MIN(__GL_MAX(0, pScissor->scissorY), height);
        pClearRect->right   = __GL_MIN(__GL_MAX(0, pScissor->scissorX + pScissor->scissorWidth), width);
        pClearRect->bottom  = __GL_MIN(__GL_MAX(0, pScissor->scissorY + pScissor->scissorHeight), height);

        if (chipCtx->drawYInverted)
        {
            gctINT32 temp = pClearRect->top;
            pClearRect->top = height - pClearRect->bottom;
            pClearRect->bottom = height - temp;
        }
    }
    else
    {
        pClearRect->left    = 0;
        pClearRect->right   = width;
        pClearRect->top     = 0;
        pClearRect->bottom  = height;
    }

    if (((pClearRect->right - pClearRect->left) == width) &&
        ((pClearRect->bottom - pClearRect->top) == height))
    {
        *pbFullClear = GL_TRUE;
    }
    else
    {
        *pbFullClear = GL_FALSE;
    }

OnError:
    gcmFOOTER();
    return status;
}

#ifdef OPENGL40
gceSTATUS gcChipclearAccumBuffer(__GLcontext* gc,
                                 glsCHIPACCUMBUFFER *chipAccumBuffer)
{
    __GLchipContext     *chipCtx = CHIP_CTXINFO(gc);
    gcsRECT             clearRect = {0};
    GLboolean           bFullClear = GL_TRUE;
    GLuint              i = 0;
    gcsSURF_CLEAR_ARGS  clearArg;
    __GLrasterState     *pRasterState = &gc->state.raster;
    gceSTATUS           status = gcvSTATUS_OK;
    gctBOOL             tsEnabled = gcvFALSE;
    gcsSURF_VIEW        accumView = {gcvNULL, 0, 1};
    gcoSURF             accumSurf = NULL;

    gcmHEADER_ARG("gc=0x%x",gc);

    accumSurf = chipAccumBuffer->renderTarget;
    accumView.surf = accumSurf;

    if (accumSurf)
    {
        __GL_MEMZERO(&clearArg, sizeof(clearArg));

        gcmONERROR(gcChipGetClearRect(gc, accumSurf, &clearRect, &bFullClear));

        clearArg.color.r.floatValue = gc->state.accum.clear.r;
        clearArg.color.g.floatValue = gc->state.accum.clear.g;
        clearArg.color.b.floatValue = gc->state.accum.clear.b;
        clearArg.color.a.floatValue = gc->state.accum.clear.a;
        clearArg.color.valueType = gcvVALUE_FLOAT;
        clearArg.colorMask = (gctUINT8) pRasterState->colorMask[i].redMask          |
                            ((gctUINT8) pRasterState->colorMask[i].greenMask << 1)  |
                            ((gctUINT8) pRasterState->colorMask[i].blueMask  << 2)  |
                            ((gctUINT8) pRasterState->colorMask[i].alphaMask << 3);

        if((!gcoSURF_QueryFlags(accumView.surf, gcvSURF_FLAG_CONTENT_PRESERVED)) &&
            (!gcoSURF_QueryFlags(accumView.surf, gcvSURF_FLAG_CONTENT_UPDATED))
            )
        {
            clearArg.colorMask = (clearArg.colorMask) ? 0xF: 0x0;
        }

        clearArg.flags = gcvCLEAR_COLOR;
//        clearArg.flags |= chipCtx->drawLayered ? gcvCLEAR_MULTI_SLICES : 0;

        if (bFullClear)
        {
            clearArg.clearRect = GL_NONE;
        }
        else
        {
            clearArg.clearRect = &clearRect;
        }

        tsEnabled = gcoSURF_IsTileStatusEnabled(&(accumView));

        gcmONERROR(gcoSURF_Clear(&accumView, &clearArg));

        /* TS from disable to enable */
        if (!tsEnabled &&
            gcoSURF_IsTileStatusEnabled(&(accumView)))
        {
            chipCtx->chipDirty.uBuffer.sBuffer.rtSurfDirty = gcvTRUE;
        }
    }

OnError:

    gcmFOOTER();
    return status;
}

#endif


__GL_INLINE gceSTATUS
gcChipClearRenderTarget(
    __GLcontext *gc
    )
{
    __GLchipContext     *chipCtx = CHIP_CTXINFO(gc);
    gcsRECT             clearRect = {0};
    GLboolean           bFullClear = GL_TRUE;
    GLuint              i = 0;
    gcsSURF_CLEAR_ARGS  clearArg;
    __GLrasterState     *pRasterState = &gc->state.raster;
    gceSTATUS           status = gcvSTATUS_OK;
    gctBOOL             tsEnabled = gcvFALSE;

    gcmHEADER_ARG("gc=0x%x",gc);

    for (i = 0; i < gc->constants.shaderCaps.maxDrawBuffers; ++i)
    {
        if (chipCtx->drawRtViews[i].surf)
        {
            __GL_MEMZERO(&clearArg, sizeof(clearArg));

            gcmONERROR(gcChipGetClearRect(gc, chipCtx->drawRtViews[i].surf, &clearRect, &bFullClear));

            clearArg.color.r.floatValue = pRasterState->clearColor.clear.r;
            clearArg.color.g.floatValue = pRasterState->clearColor.clear.g;
            clearArg.color.b.floatValue = pRasterState->clearColor.clear.b;
            clearArg.color.a.floatValue = pRasterState->clearColor.clear.a;
            clearArg.color.valueType = gcvVALUE_FLOAT;
            clearArg.colorMask = (gctUINT8) pRasterState->colorMask[i].redMask          |
                                ((gctUINT8) pRasterState->colorMask[i].greenMask << 1)  |
                                ((gctUINT8) pRasterState->colorMask[i].blueMask  << 2)  |
                                ((gctUINT8) pRasterState->colorMask[i].alphaMask << 3);

            if((!gcoSURF_QueryFlags(chipCtx->drawRtViews[i].surf, gcvSURF_FLAG_CONTENT_PRESERVED)) &&
               (!gcoSURF_QueryFlags(chipCtx->drawRtViews[i].surf, gcvSURF_FLAG_CONTENT_UPDATED))
              )
            {
                clearArg.colorMask = (clearArg.colorMask) ? 0xF: 0x0;
            }

            clearArg.flags = gcvCLEAR_COLOR;
            clearArg.flags |= chipCtx->drawLayered ? gcvCLEAR_MULTI_SLICES : 0;

            if (bFullClear)
            {
                clearArg.clearRect = GL_NONE;
            }
            else
            {
                clearArg.clearRect = &clearRect;
            }

            tsEnabled = gcoSURF_IsTileStatusEnabled(&chipCtx->drawRtViews[i]);

            gcmONERROR(gcoSURF_Clear(&chipCtx->drawRtViews[i], &clearArg));

            /* TS from disable to enable */
            if (!tsEnabled &&
                gcoSURF_IsTileStatusEnabled(&chipCtx->drawRtViews[i]))
            {
                chipCtx->chipDirty.uBuffer.sBuffer.rtSurfDirty = gcvTRUE;
            }
        }
    }

OnError:

    gcmFOOTER();
    return status;
}

__GL_INLINE gceSTATUS
gcChipClearDepthAndStencil(
    __GLcontext *gc,
    GLuint mask
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gcsRECT clearRect;
    gcsSURF_VIEW *dsView = gcvNULL;
    GLboolean bFullClear = GL_TRUE;
    gcsSURF_CLEAR_ARGS  clearArg;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x mask=%u",gc, mask);

    __GL_MEMZERO(&clearArg, sizeof(clearArg));

    /* mask out color buffer bit as we need pass it down to 3d clear in some cases */
    mask &= ~GL_COLOR_BUFFER_BIT;

    /* Compute masks for clearing. */
    if (chipCtx->drawDepthView.surf && (mask & GL_DEPTH_BUFFER_BIT))
    {
        dsView = &chipCtx->drawDepthView;
        clearArg.flags |= gcvCLEAR_DEPTH;
        clearArg.depth.floatValue = gc->state.depth.clear;
        clearArg.depthMask = gc->state.depth.writeEnable;
    }

    if (chipCtx->drawStencilView.surf && (mask & GL_STENCIL_BUFFER_BIT))
    {
        /* If both depth and stencil buffer are set, they must be same.
        ** We only support stencil buffer packed with depth buffer.
        */
        GL_ASSERT(!dsView || !dsView->surf || dsView->surf == chipCtx->drawStencilView.surf);

        dsView = &chipCtx->drawStencilView;
        clearArg.flags |= gcvCLEAR_STENCIL;
        clearArg.stencil = (gc->state.stencil.clear & chipCtx->drawStencilMask);
        clearArg.stencilMask = (gc->state.stencil.front.writeMask & 0xFF);

        if((!gcoSURF_QueryFlags(dsView->surf, gcvSURF_FLAG_CONTENT_PRESERVED)) &&
           (!gcoSURF_QueryFlags(dsView->surf, gcvSURF_FLAG_CONTENT_UPDATED))
          )
        {
            clearArg.stencilMask = clearArg.stencilMask ? 0xFF : 0x0;
        }
    }

    if (dsView && dsView->surf)
    {
        gcmONERROR(gcChipGetClearRect(gc, dsView->surf, &clearRect, &bFullClear));

        if (bFullClear)
        {
            clearArg.clearRect = GL_NONE;
        }
        else
        {
            clearArg.clearRect = &clearRect;
        }

        clearArg.flags |= chipCtx->drawLayered ? gcvCLEAR_MULTI_SLICES : 0;

        gcmONERROR(gcoSURF_Clear(dsView, &clearArg));
    }

OnError:

    gcmFOOTER();
    return status;
}


__GL_INLINE gceSTATUS
gcChipClearDrawable(
    __GLcontext *gc,
    GLuint mask
    )
{
    gceSTATUS status;
    gcePATCH_ID patchId = gcvPATCH_INVALID;

    gcmHEADER_ARG("gc=0x%x mask=%u",gc, mask);

    gcoHAL_GetPatchID(gcvNULL, &patchId);

    if ((patchId == gcvPATCH_NENAMARK2) && !gc->state.enables.scissorTest)
    {
        mask &= ~GL_COLOR_BUFFER_BIT;
    }

    if (mask & GL_COLOR_BUFFER_BIT)
    {
        gcmONERROR(gcChipClearRenderTarget(gc));
    }

    gcmONERROR(gcChipClearDepthAndStencil(gc, mask));

#ifdef OPENGL40
    if (mask & GL_ACCUM_BUFFER_BIT)
    {
        gcmONERROR(gcChipclearAccumBuffer(gc, (glsCHIPACCUMBUFFER *)(gc->drawablePrivate->accumBuffer.privateData)));
    }
#endif

OnError:
    gcmFOOTER();
    return status;
}

__GL_INLINE gceSTATUS
gcChipClearFBO(
    __GLcontext *gc,
    GLuint mask
    )
{
    gceSTATUS status;

    gcmHEADER_ARG("gc=0x%x mask=%u",gc, mask);
    if (mask & GL_COLOR_BUFFER_BIT)
    {
        gcmONERROR(gcChipClearRenderTarget(gc));
    }

    gcmONERROR(gcChipClearDepthAndStencil(gc, mask));

OnError:

    gcmFOOTER();
    return status;

}

__GL_INLINE GLvoid
gcChipClearValidateRenderTarget(
    __GLcontext *gc,
    __GLchipContext *chipCtx
    )
{
    /*TBD, maybe we needn't this function*/
}
/************************************************************************/
/* Implementation for EXPORTED FUNCTIONS                                */
/************************************************************************/
GLboolean
__glChipClearBegin(
    __GLcontext *gc,
    GLbitfield *mask
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);

    if (gc->state.enables.rasterizerDiscard)
    {
        return GL_FALSE;
    }

#if gcdFRAMEINFO_STATISTIC
    gcoHAL_FrameInfoOps(chipCtx->hal,
                        gcvFRAMEINFO_DRAW_NUM,
                        gcvFRAMEINFO_OP_INC,
                        gcvNULL);
    if (g_dbgSkipDraw)
    {
        return GL_FALSE;
    }
#endif

    /* Clear invalid mask */
    if (*mask & GL_COLOR_BUFFER_BIT)
    {
        GLuint i;
        GLboolean hasRT = GL_FALSE;
        for (i = 0; i < gc->constants.shaderCaps.maxDrawBuffers; ++i)
        {
            if (chipCtx->drawRtViews[i].surf)
            {
                hasRT = GL_TRUE;
                break;
            }
        }

        if (!hasRT)
        {
            *mask &= ~GL_COLOR_BUFFER_BIT;
        }
    }

    if ((*mask & GL_DEPTH_BUFFER_BIT) && !chipCtx->drawDepthView.surf)
    {
        *mask &= ~GL_DEPTH_BUFFER_BIT;
    }

    if ((*mask & GL_STENCIL_BUFFER_BIT) && !chipCtx->drawStencilView.surf)
    {
        *mask &= ~GL_STENCIL_BUFFER_BIT;
    }

    if (chipCtx->needStencilOpt && (*mask & GL_STENCIL_BUFFER_BIT))
    {
        __GLchipStencilOpt *stencilOpt = gcChipPatchStencilOptGetInfo(gc, GL_FALSE);

        if (stencilOpt)
        {
            gcsRECT rect;
            GLint width  = (GLint)chipCtx->drawRTWidth;
            GLint height = (GLint)chipCtx->drawRTHeight;

            if (gc->state.enables.scissorTest)
            {
                __GLscissor *pScissor = &(gc->state.scissor);

                /* ClearRect is the Intersection of scissor and RT size */
                rect.left    = __GL_MIN(__GL_MAX(0, pScissor->scissorX), width  - 1);
                rect.top     = __GL_MIN(__GL_MAX(0, pScissor->scissorY), height - 1);
                rect.right   = __GL_MIN(__GL_MAX(0, pScissor->scissorX + pScissor->scissorWidth  - 1), width  - 1);
                rect.bottom  = __GL_MIN(__GL_MAX(0, pScissor->scissorY + pScissor->scissorHeight - 1), height - 1);

                if (chipCtx->drawYInverted)
                {
                    gctINT32 temp = rect.top;
                    rect.top = height - rect.bottom - 1 ;
                    rect.bottom = height - temp - 1;
                }
            }
            else
            {
                rect.left   = 0;
                rect.top    = 0;
                rect.right  = width  - 1;
                rect.bottom = height - 1;
            }

            gcChipPatchStencilOptWrite(gc,
                                       stencilOpt,
                                       &rect,
                                       gc->state.stencil.clear,
                                       (GLuint)gc->state.stencil.front.writeMask,
                                       GL_FALSE);
        }
    }

    return *mask ? GL_TRUE : GL_FALSE;
}

GLboolean
__glChipClearValidateState(
    __GLcontext *gc,
    GLbitfield mask
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;
    __GLframebufferObject *fbo = gc->frameBuffer.drawFramebufObj;
    gcmHEADER_ARG("gc=0x%x mask=%u",gc, mask);

    if (fbo && fbo->shadowRender)
    {
        gcmONERROR(gcChipFBOMarkShadowRendered(gc, fbo, mask));
    }

    gcChipClearValidateRenderTarget(gc, chipCtx);

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;

}


GLboolean
__glChipClearEnd(
    __GLcontext *gc,
    GLbitfield mask
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;
    GLboolean ret = GL_TRUE;

    gcmHEADER_ARG("gc=0x%x", gc);

    {   /* Reset flag */
        GLuint i;
        gctUINT8 enable = (gctUINT8) gc->state.raster.colorMask[0].redMask
                        | ((gctUINT8) gc->state.raster.colorMask[0].greenMask << 1)
                        | ((gctUINT8) gc->state.raster.colorMask[0].blueMask  << 2)
                        | ((gctUINT8) gc->state.raster.colorMask[0].alphaMask << 3);

        /* Now, gcvSURF_FLAG_CONTENT_UPDATED almost used for system drawable. The optimization
           just for swaping and clearing system drawable. If fbo need the same optimization
           method, need more code to support.*/
        for (i = 0; i < gc->constants.shaderCaps.maxDrawBuffers; ++i)
        {
            if (chipCtx->drawRtViews[i].surf && enable &&
                (mask & GL_COLOR_BUFFER_BIT))
            {
                gcmONERROR(gcoSURF_SetFlags(chipCtx->drawRtViews[i].surf, gcvSURF_FLAG_CONTENT_UPDATED, gcvTRUE));
            }
        }

        if (chipCtx->drawDepthView.surf && gc->state.depth.writeEnable &&
            (mask & GL_DEPTH_BUFFER_BIT))
        {
            gcmONERROR(gcoSURF_SetFlags(chipCtx->drawDepthView.surf, gcvSURF_FLAG_CONTENT_UPDATED, gcvTRUE));
        }

        if (chipCtx->drawStencilView.surf &&
            (gc->state.stencil.front.writeMask & 0xFF) &&
            (mask & GL_STENCIL_BUFFER_BIT))
        {
            gcmONERROR(gcoSURF_SetFlags(chipCtx->drawStencilView.surf, gcvSURF_FLAG_CONTENT_UPDATED, gcvTRUE));
        }
    }

#if gcdFRAMEINFO_STATISTIC
    if (g_dbgPerDrawKickOff)
    {
        gcmONERROR(gcoSURF_Flush(gcvNULL));
        /* Commit command buffer. */
        gcmONERROR(gcoHAL_Commit(chipCtx->hal, gcvTRUE));
    }

    if (g_dbgDumpImagePerDraw & (__GL_PERDRAW_DUMP_CLEAR_RT | __GL_PERDRAW_DUMP_CLEAR_DS))
    {
        gcmONERROR(gcChipUtilsDumpRT(gc, (__GL_PERDRAW_DUMP_CLEAR_RT | __GL_PERDRAW_DUMP_CLEAR_DS)));
    }
#endif

OnError:
    ret = gcmIS_ERROR(status) ? GL_FALSE : GL_TRUE;
    if (!ret)
    {
        gcChipSetError(chipCtx, status);
    }

    gcmFOOTER_ARG("return=%d", ret);
    return ret;
}

GLboolean
__glChipClear(
    __GLcontext * gc,
    GLuint mask
    )
{
#if __GL_CHIP_PATCH_ENABLED
    GLint savedWriteMask;
    GLboolean changed = GL_FALSE;
#endif

    gceSTATUS status = gcvSTATUS_OK;
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);

    gcmHEADER_ARG("gc=0x%x mask=%u", gc, mask);

#if __GL_CHIP_PATCH_ENABLED
    changed = gcChipPatchClear(gc, &mask, &savedWriteMask);
#endif

    if (!gc->frameBuffer.drawFramebufObj->name)
    {
        gcmONERROR(gcChipClearDrawable(gc, mask));
    }
    else
    {
        gcmONERROR(gcChipClearFBO(gc, mask));
    }

#if __GL_CHIP_PATCH_ENABLED
    /* restore the mask in case it was changed by patch */
    if (changed)
    {
        gc->state.stencil.front.writeMask = savedWriteMask;
    }
#endif

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:

    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}

GLboolean
__glChipClearBuffer(
    __GLcontext *gc,
    GLenum buffer,
    GLint drawbuffer,
    GLvoid *value,
    GLenum type
    )
{
    __GLchipContext     *chipCtx = CHIP_CTXINFO(gc);
    gcsRECT             clearRect = {0};
    GLboolean           bFullClear = GL_TRUE;
    gcsSURF_CLEAR_ARGS  clearArg;
    __GLrasterState    *pRasterState = &gc->state.raster;
    gcsSURF_VIEW        *surfView = gcvNULL;
    gceSTATUS           status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x buffer=%d drawbuffer=%d value=0x%x type=%d",
                   gc, buffer, drawbuffer, value, type);

    __GL_MEMZERO(&clearArg, sizeof(clearArg));

    switch (buffer)
    {
    case GL_COLOR:
        surfView = &chipCtx->drawRtViews[drawbuffer];
        if (surfView->surf)
        {
            switch(type)
            {
            case GL_FLOAT:
                clearArg.color.r.floatValue = (GLfloat)((GLfloat*)value)[0];
                clearArg.color.g.floatValue = (GLfloat)((GLfloat*)value)[1];
                clearArg.color.b.floatValue = (GLfloat)((GLfloat*)value)[2];
                clearArg.color.a.floatValue = (GLfloat)((GLfloat*)value)[3];
                clearArg.color.valueType = gcvVALUE_FLOAT;
                break;

            case GL_INT:
                clearArg.color.r.intValue = (GLint)((GLint*)value)[0];
                clearArg.color.g.intValue = (GLint)((GLint*)value)[1];
                clearArg.color.b.intValue = (GLint)((GLint*)value)[2];
                clearArg.color.a.intValue = (GLint)((GLint*)value)[3];
                clearArg.color.valueType = gcvVALUE_INT;
                break;

            case GL_UNSIGNED_INT:
                clearArg.color.r.uintValue = (GLuint)((GLuint*)value)[0];
                clearArg.color.g.uintValue = (GLuint)((GLuint*)value)[1];
                clearArg.color.b.uintValue = (GLuint)((GLuint*)value)[2];
                clearArg.color.a.uintValue = (GLuint)((GLuint*)value)[3];
                clearArg.color.valueType = gcvVALUE_UINT;
                break;
            }

            clearArg.colorMask =  (gctUINT8) pRasterState->colorMask[drawbuffer].redMask         |
                                 ((gctUINT8) pRasterState->colorMask[drawbuffer].greenMask << 1) |
                                 ((gctUINT8) pRasterState->colorMask[drawbuffer].blueMask  << 2) |
                                 ((gctUINT8) pRasterState->colorMask[drawbuffer].alphaMask << 3);

            clearArg.flags = gcvCLEAR_COLOR;
        }
        break;


    case GL_DEPTH:
        surfView = &chipCtx->drawDepthView;
        if (surfView->surf)
        {
            clearArg.flags |= gcvCLEAR_DEPTH;
            clearArg.depth.floatValue = (GLfloat)((GLfloat*)value)[0];
            clearArg.depthMask = gc->state.depth.writeEnable;
        }
        break;

    case GL_STENCIL:
        surfView = &chipCtx->drawStencilView;
        if (surfView->surf)
        {
            clearArg.flags |= gcvCLEAR_STENCIL;
            clearArg.stencil = (GLint)((GLint*)value)[0];
            clearArg.stencilMask = (gc->state.stencil.front.writeMask & 0xFF);
        }
        break;
    }

    if (surfView && surfView->surf)
    {
        gcmONERROR(gcChipGetClearRect(gc, surfView->surf, &clearRect, &bFullClear));

        if (bFullClear)
        {
            clearArg.clearRect = GL_NONE;
        }
        else
        {
            clearArg.clearRect = &clearRect;
        }

        clearArg.flags |= chipCtx->drawLayered ? gcvCLEAR_MULTI_SLICES : 0;

#ifdef OPENGL40
        {
            GLuint i;
            for (i = 0; i < gc->constants.shaderCaps.maxDrawBuffers; ++i)
            {
                if (chipCtx->drawRtViews[i].surf)
                {
                    gcmONERROR(gcoSURF_Clear(&chipCtx->drawRtViews[i], &clearArg));
                }
            }
        }
#else
        gcmONERROR(gcoSURF_Clear(surfView, &clearArg));
#endif
    }

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}

GLboolean
__glChipClearBufferfi(
    __GLcontext *gc,
    GLfloat depth,
    GLint stencil
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gcsRECT clearRect;
    gcsSURF_VIEW *dsView = gcvNULL;
    GLboolean bFullClear = GL_TRUE;
    gcsSURF_CLEAR_ARGS  clearArg;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x depth=%f stencil=%d",gc, depth, stencil);

    __GL_MEMZERO(&clearArg, sizeof(clearArg));

    /* Compute masks for clearing. */
    if (chipCtx->drawDepthView.surf)
    {
        dsView = &chipCtx->drawDepthView;
        clearArg.flags |= gcvCLEAR_DEPTH;
        clearArg.depth.floatValue = depth;
        clearArg.depthMask = gc->state.depth.writeEnable;
    }

    if (chipCtx->drawStencilView.surf)
    {
        /* If both depth and stencil buffer are set, they must be same.
        ** We only support stencil buffer packed with depth buffer.
        */
        GL_ASSERT(!dsView || !dsView->surf || dsView->surf == chipCtx->drawStencilView.surf);
        dsView = &chipCtx->drawStencilView;
        clearArg.flags |= gcvCLEAR_STENCIL;
        clearArg.stencil = stencil;
        clearArg.stencilMask = (gc->state.stencil.front.writeMask & 0xFF);
    }

    if (dsView && dsView->surf)
    {
        gcmONERROR(gcChipGetClearRect(gc, dsView->surf, &clearRect, &bFullClear));

        if (bFullClear)
        {
            clearArg.clearRect = GL_NONE;
        }
        else
        {
            clearArg.clearRect = &clearRect;
        }

        clearArg.flags |= chipCtx->drawLayered ? gcvCLEAR_MULTI_SLICES : 0;

        gcmONERROR(gcoSURF_Clear(dsView, &clearArg));
    }

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;

}


