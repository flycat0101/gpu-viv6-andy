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


#include "gc_gl_context.h"
#include "chip_context.h"

/***************************************************************************/
/* Implementation for internal functions                                   */
/***************************************************************************/
static gceSTATUS _clearRect(__GLcontext *gc, IN gcoSURF surface, IN GLuint mask, IN gcsRECT *prect)
{
    gceSTATUS   status = gcvSTATUS_OK;
    gctUINT     depthFlags;
    gcsSURF_CLEAR_ARGS  colorClearArgs, dsClearArgs;
    gctUINT8 colorMask;
    gcsSURF_VIEW surfView = {surface, 0, 1};

    if (surface && (mask & (GL_COLOR_BUFFER_BIT |GL_ACCUM_BUFFER_BIT)) )
    {
        gcoOS_ZeroMemory(&colorClearArgs, sizeof(colorClearArgs));

        colorClearArgs.clearRect = prect;
        if ( mask & GL_ACCUM_BUFFER_BIT) {
            colorClearArgs.color.r.floatValue = gc->state.accum.clear.r;
            colorClearArgs.color.g.floatValue = gc->state.accum.clear.g;
            colorClearArgs.color.b.floatValue = gc->state.accum.clear.b;
            colorClearArgs.color.a.floatValue = gc->state.accum.clear.a;
        } else {
            colorClearArgs.color.r.floatValue = gc->state.raster.clear.r;
            colorClearArgs.color.g.floatValue = gc->state.raster.clear.g;
            colorClearArgs.color.b.floatValue = gc->state.raster.clear.b;
            colorClearArgs.color.a.floatValue = gc->state.raster.clear.a;
        }
        colorClearArgs.color.valueType = gcvVALUE_FLOAT;
        colorClearArgs.flags = gcvCLEAR_COLOR;

        colorMask = ((gc->state.raster.colorMask[0].redMask ? 0x1 : 0x0) |
                           (gc->state.raster.colorMask[0].greenMask ? 0x2 : 0x0) |
                           (gc->state.raster.colorMask[0].blueMask ? 0x4 : 0x0) |
                           (gc->state.raster.colorMask[0].alphaMask ? 0x8 : 0x0) );

        colorClearArgs.colorMask = colorMask;
        status = gcoSURF_Clear(&surfView, &colorClearArgs);
        if (gcmIS_ERROR(status))
        {
            if ( mask & (GL_COLOR_BUFFER_BIT | GL_ACCUM_BUFFER_BIT) )
            {
                /* Clear color buffers using software. */
                colorClearArgs.flags = (colorClearArgs.flags & ~gcvCLEAR_WITH_GPU_ONLY) | gcvCLEAR_WITH_CPU_ONLY;
                if (gcmIS_ERROR(gcoSURF_Clear(&surfView, &colorClearArgs)))
                {
                    return GL_INVALID_OPERATION;
                }
            }
        }

        return GL_NO_ERROR;
    }

    depthFlags = 0;

    if (mask & GL_DEPTH_BUFFER_BIT)
    {
        depthFlags |= gcvCLEAR_DEPTH;
    }

    if (mask & GL_STENCIL_BUFFER_BIT)
    {
        depthFlags |= gcvCLEAR_STENCIL;
    }

    if ( surface && (depthFlags != 0) )
    {
        gcoOS_ZeroMemory(&dsClearArgs, sizeof(dsClearArgs));

        dsClearArgs.clearRect = prect;
        dsClearArgs.depth.floatValue = gc->state.depth.clear;
        dsClearArgs.depthMask = (gctBOOL)gc->state.depth.writeEnable;
        dsClearArgs.stencil = gc->state.stencil.clear;
        dsClearArgs.stencilMask = (gctUINT8)(gc->state.stencil.current.front.writeMask & 0x00FF);
        dsClearArgs.flags = depthFlags;
        status = gcoSURF_Clear(&surfView, &dsClearArgs);
        if (gcmIS_ERROR(status))
        {
            /* Clear depth/stencil buffers using software. */
            dsClearArgs.flags = (dsClearArgs.flags & ~gcvCLEAR_WITH_GPU_ONLY) | gcvCLEAR_WITH_CPU_ONLY;
            if (gcmIS_ERROR(gcoSURF_Clear(&surfView, &dsClearArgs)))
            {
                return GL_INVALID_OPERATION;
            }
        }
    }

    return GL_NO_ERROR;
}

GLvoid getClearRect(__GLcontext *gc,
                    gcoSURF      pView,
                    gcsRECT     *pClearRect,
                    GLboolean   *pbFullClear)
{
    GLint viewWidth = 0;
    GLint viewHeight = 0;

    GL_ASSERT(pView);
    GL_ASSERT(pClearRect);
    GL_ASSERT(pbFullClear);

    __GL_MEMZERO(pClearRect, sizeof(gcsRECT));

    gcoSURF_GetSize(pView, (gctUINT *)&viewWidth, (gctUINT *)&viewHeight, gcvNULL);

    if(gc->state.enables.scissor)
    {
        pClearRect->left    = min(max(0, gc->state.scissor.scissorX), viewWidth);
        pClearRect->right   = min(viewWidth, max(0, gc->state.scissor.scissorX + gc->state.scissor.scissorWidth));

        pClearRect->top     = min(max(0, gc->state.scissor.scissorY), viewHeight);
        pClearRect->bottom  = min(viewHeight, max(0, gc->state.scissor.scissorY + gc->state.scissor.scissorHeight));
    }
    else
    {
        pClearRect->left    = 0;
        pClearRect->right   = viewWidth;
        pClearRect->top     = 0;
        pClearRect->bottom  = viewHeight;
    }

    if( ((pClearRect->right - pClearRect->left) == viewWidth) &&
        ((pClearRect->bottom - pClearRect->top) == viewHeight) )
    {
        *pbFullClear = GL_TRUE;
    }
    else
    {
        *pbFullClear = GL_FALSE;
    }
}

extern GLenum setClearColor( glsCHIPCONTEXT_PTR chipCtx, GLvoid* ClearColor, gleTYPE Type);

GLvoid clearAccumBuffer(__GLcontext* gc, __GLdrawableBuffer* buffer)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    glsCHIPACCUMBUFFER* chipAccumBuffer = (glsCHIPACCUMBUFFER*)(buffer->privateData);
    gcsRECT      rect;
    GLboolean  fullScreen;
    GLfloat clearColor[4];

    if( (buffer->width == 0) || (buffer->height == 0) )
    {
        return;
    }

    GL_ASSERT(chipAccumBuffer);

    /* set accumulation buffer clear color */
    clearColor[0] = gc->state.accum.clear.r;
    clearColor[1] = gc->state.accum.clear.g;
    clearColor[2] = gc->state.accum.clear.b;
    clearColor[3] = gc->state.accum.clear.a;
    setClearColor(chipCtx, clearColor, glvFLOAT);

    getClearRect(gc, chipAccumBuffer->renderTarget, &rect, &fullScreen);

    if(fullScreen)
    {
        _clearRect(gc, chipAccumBuffer->renderTarget ,GL_ACCUM_BUFFER_BIT, NULL);
    }
    else
    {
        _clearRect(gc, chipAccumBuffer->renderTarget ,GL_ACCUM_BUFFER_BIT, &rect);
    }

    /* Restore clear color */
    clearColor[0] = gc->state.raster.clear.r;
    clearColor[1] = gc->state.raster.clear.g;
    clearColor[2] = gc->state.raster.clear.b;
    clearColor[3] = gc->state.raster.clear.a;
    setClearColor(chipCtx, clearColor, glvFLOAT);
}

GLvoid clearRenderTarget(__GLcontext *gc)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gcsRECT             clearRect = {0};
    GLboolean           bFullClear = GL_TRUE;
    GLuint              i = 0;

    for (i = 0; i < __GL_MAX_DRAW_BUFFERS; i++)
    {
        if (chipCtx->drawRT[i])
        {
            getClearRect(gc, chipCtx->drawRT[i], &clearRect, &bFullClear);
            if (bFullClear)
            {
                _clearRect(gc, chipCtx->drawRT[i] ,GL_COLOR_BUFFER_BIT, NULL);
            }
            else
            {
                _clearRect(gc, chipCtx->drawRT[i] ,GL_COLOR_BUFFER_BIT, &clearRect);
            }
        }
    }
}

GLvoid clearDepthAndStencil(__GLcontext *gc, GLuint mask)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gcsRECT             clearRect;
    GLboolean           bFullClear = GL_TRUE;
    __GLdrawablePrivate *drawable = gc->drawablePrivate;
    GLuint maskds = 0;

    if ((mask & GL_DEPTH_BUFFER_BIT) && (drawable->modes.haveDepthBuffer))
        maskds |= GL_DEPTH_BUFFER_BIT;

    if ((mask & GL_STENCIL_BUFFER_BIT) && (drawable->modes.haveStencilBuffer))
        maskds |= GL_STENCIL_BUFFER_BIT;

    if (chipCtx->drawDepth)
    {
        getClearRect(gc, chipCtx->drawDepth, &clearRect, &bFullClear);

        if (bFullClear)
        {
            _clearRect(gc, chipCtx->drawDepth ,maskds, NULL);
        }
        else
        {
            _clearRect(gc, chipCtx->drawDepth ,maskds, &clearRect);
        }
    }
}

extern GLenum setDepthMask( glsCHIPCONTEXT_PTR  chipCtx, GLboolean depthMask);

GLvoid clearDrawable(__GLcontext *gc, GLuint mask)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    __GLdrawablePrivate *drawable = gc->drawablePrivate;

    if (mask & GL_COLOR_BUFFER_BIT)
    {
        clearRenderTarget(gc);

        if (gc->drawablePrivate->pbufferTex)
        {
            gc->drawablePrivate->pbufferTex->needGenMipmap = GL_TRUE;
        }
    }

    if (((mask & GL_DEPTH_BUFFER_BIT) && (drawable->modes.haveDepthBuffer)) ||
        ((mask & GL_STENCIL_BUFFER_BIT) && (drawable->modes.haveStencilBuffer)))
    {
        if (!gc->state.enables.depthBuffer.test)
        {
            setDepthMask(chipCtx, gc->state.depth.writeEnable);
        }

        clearDepthAndStencil(gc,mask);

        if (!gc->state.enables.depthBuffer.test)
        {
            setDepthMask(chipCtx, GL_FALSE);
        }
    }

    if (mask & GL_ACCUM_BUFFER_BIT)
    {
        clearAccumBuffer(gc, &gc->drawablePrivate->accumBuffer);
    }
}

GLvoid clearFBO(__GLcontext *gc, GLuint mask)
{
    __GLframebufferObject   *fbo = gc->frameBuffer.drawFramebufObj;

    if (mask & GL_COLOR_BUFFER_BIT)
    {
        clearRenderTarget(gc);
    }

    if (((mask & GL_DEPTH_BUFFER_BIT) && (fbo->attachPoint[__GL_MAX_COLOR_ATTACHMENTS].objectType != GL_NONE)) ||
        ((mask & GL_STENCIL_BUFFER_BIT) && (fbo->attachPoint[__GL_MAX_COLOR_ATTACHMENTS + 1].objectType != GL_NONE)))
    {
        clearDepthAndStencil(gc, mask);
    }

    if (mask & GL_ACCUM_BUFFER_BIT)
    {
        GL_ASSERT(0);   /* FBO don't have accum buffer*/
    }

    if (gc->frameBuffer.drawFramebufObj->name)
    {
    }
}

/************************************************************************/
/* Implementation for device clear buffer API                           */
/************************************************************************/
GLvoid __glChipClear(__GLcontext * gc, GLuint mask)
{
    glsCHIPCONTEXT_PTR  chipCtx = CHIP_CTXINFO(gc);
    if (!gc->frameBuffer.drawFramebufObj->name)
    {
        clearDrawable(gc, mask);
    }
    else
    {
        clearFBO(gc, mask);
    }

    if((gc->flags & __GL_DRAW_TO_FRONT) && chipCtx->drawRT[0])
    {
        /* Flush the cache. */
        if (gcmIS_ERROR(gcoSURF_Flush(chipCtx->drawRT[0])))
        {
            gc->error = GL_INVALID_OPERATION;
            return;
        }

        /* Commit command buffer. */
        if (gcmIS_ERROR(gcoHAL_Commit(chipCtx->hal, gcvFALSE)))
        {
            gc->error = GL_INVALID_OPERATION;
            return;
        }

        (*gc->imports.internalSwapBuffers)(gc,GL_TRUE);
    }
}

