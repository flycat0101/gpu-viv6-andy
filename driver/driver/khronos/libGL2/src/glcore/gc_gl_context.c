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
#include "gl/gl_device.h"
#include "g_asmoff.h"
#include "dri/viv_lock.h"
#include "gc_gl_debug.h"

/* Mutex lock for drawable change */
__GLlock drawableChangeLock;


extern GLvoid __glInitExtensions(__GLcontext *gc);
extern GLuint __glCopyContext(__GLcontext *dst, __GLcontext *src, GLuint mask);
extern GLvoid __glSwpCreateContext(__GLcontext *gc);
extern GLvoid __glShareDlists(__GLcontext *dst, __GLcontext *src);
extern GLvoid __glShareTextureObjects(__GLcontext *dst, __GLcontext *src);
extern GLvoid __glShareBufferObjects(__GLcontext *dst, __GLcontext *src);
extern GLvoid __glShareProgramObjects(__GLcontext *dst, __GLcontext *src);
extern GLvoid __glShareShaderProgramObjects(__GLcontext *dst, __GLcontext *src);
extern GLvoid __glFreeAttribStackState(__GLcontext *gc);
extern GLvoid __glFreeTransformState(__GLcontext *gc);
extern GLvoid __glFreeSelectState(__GLcontext *gc);
extern GLuint __glInitVertexInputState(__GLcontext *gc);
extern GLvoid __glFreeImmedVertexCacheBuffer(__GLcontext *gc);
extern GLvoid __glFreeVertexInputState(__GLcontext *gc);
extern GLvoid __glFreeProgramState(__GLcontext * gc);
extern GLvoid __glFreeShaderProgramState(__GLcontext * gc);
extern GLvoid __glInitNopDispatchFuncTable(GLvoid);
extern GLvoid __glOverWriteListCompileTable(GLvoid);
extern GLuint __glInitVertexOutputState(__GLcontext *gc);
extern GLvoid __glFreeVertexOutputState(__GLcontext *gc);
extern GLvoid __glDispatchDrawableChange(__GLcontext *gc);
extern GLvoid __glFreePixelMapState(__GLcontext *gc);
extern GLvoid __glFreeEvaluatorState(__GLcontext *gc);
extern GLvoid __glFreeTextureState(__GLcontext *gc);
extern GLvoid __glFreeDlistVertexCache(__GLcontext *gc);
extern GLvoid __glFreeConcatDlistCache(__GLcontext *gc);
extern GLvoid __glFreeDlistState(__GLcontext *gc);
extern GLvoid __glFreeBufferObjectState(__GLcontext *gc);
extern GLvoid __glFreeQueryState(__GLcontext *gc);
extern GLvoid __glInitPixelMachine(__GLcontext *gc);
extern GLvoid __glFreePixelSpanInfo( __GLcontext *gc );
extern GLuint __glInitPixelSpanInfo( __GLcontext *gc );
extern GLvoid __glInitTexObjTemplate(__GLcontext * gc);
extern GLvoid __glInitBufObjTemplate(__GLcontext * gc);

#if GL_EXT_framebuffer_object
extern GLvoid __glInitFramebufferStates(__GLcontext *gc);
extern GLvoid __glFreeFramebufferStates(__GLcontext *gc);
extern GLvoid __glShareFrameBufferObjects(__GLcontext * dst, __GLcontext * src);
#endif

extern GLvoid __glUpdateViewport(__GLcontext *gc, GLint x, GLint y, GLsizei w, GLsizei h);
extern GLvoid __glUpdateScissor(__GLcontext *gc, GLint x, GLint y, GLint w, GLint h);

extern GLboolean __glBindTexImageARB(__GLcontext *gc, __GLdrawablePrivate *srcDrawable, GLvoid*,GLenum iBuffer);
extern GLboolean __glReleaseTexImageARB(__GLcontext *gc, __GLdrawablePrivate *srcDrawable, GLenum iBuffer);

extern __GLdispatchState __glListCompileFuncTable;
extern __GLdispatchState __glNopDispatchFuncTable;

#define __GL_NOOP(fp) *(GLvoid (**)())&(fp) = __glNoop;
#define __GL_NOOP_RET0(fp) *(GLuint (**)())&(fp) = __glNoop_Return0;
#define __GL_NOOP_RET1(fp) *(GLuint (**)())&(fp) = __glNoop_Return1;


GLuint  g_initGlobals = GL_TRUE;
GLfloat g_uByteToFloat[256];

#ifndef _LINUX_
extern GLint currentContextNum;
#else
extern GLint currentContextNum;
#endif


GLvoid __glNoop()
{
}

GLuint __glNoop_Return0()
{
    return 0;
}

GLuint __glNoop_Return1()
{
    return 1;
}

GLvoid __glCoreNopDispatch(GLvoid)
{
    __GL_SETUP();
    gc->immediateDispatchTable = __glNopDispatchFuncTable;
    __GL_SET_API_DISPATCH(__GL_IMMEDIATE_TABLE_OFFSET);
}

GLvoid __glRestoreDispatch(__GLcontext *gc)
{
    gc->immediateDispatchTable = __glImmediateFuncTable;
    __GL_SET_API_DISPATCH(gc->savedDispatchOffset);
}

GLvoid __glUnSupportModeEnter(GLvoid)
{
    __GL_SETUP();
    gc->flags |= __GL_DISCARD_FOLLOWING_DRAWS_UNSPPORTED_MODE;
}

GLvoid __glUnSupportModeExit(__GLcontext *gc)
{
    gc->flags &= ~__GL_DISCARD_FOLLOWING_DRAWS_UNSPPORTED_MODE;
}

GLvoid __glSaveDispatch(__GLcontext *gc)
{
    gc->savedDispatchOffset = gc->currentDispatchOffset;
}

/*WGL_EXT_swap_control*/
GLboolean __glSwapInterval( __GLcontext *gc, GLint interval, GLuint fSet )
{
    return (*gc->dp.swapInterval)(gc, interval, fSet);
}

GLvoid __glInitConstantDefault(__GLdeviceConstants *constants)
{
    /* Specific size limits */
    constants->numberOfLights = 32;
    constants->numberOfClipPlanes = 6;
    constants->numberOfTextureUnits = 8;
    constants->numberofQueryCounterBits = 32;
    constants->maxTextureSize = 1 <<(12-1) ;
    constants->maxNumTextureLevels  = 13;
    constants->maxTextureLodBias = 12;
    constants->maxTextureMaxAnisotropy = 16;
    constants->subpixelBits = 3;
    constants->maxListNesting = 64;
    constants->maxEvalOrder = 30;
    constants->maxAttribStackDepth = 16;
    constants->maxClientAttribStackDepth = 16;
    constants->maxNameStackDepth = 128;
    constants->maxModelViewStackDepth = 32;
    constants->maxProjectionStackDepth = 16;
    constants->maxTextureStackDepth = 16;
    constants->maxProgramStackDepth = 16;
    constants->maxColorStackDepth = 16;
    constants->maxElementsVertices = 32;
    constants->maxElementsIndices = 65535;
    constants->maxConvolution1DWidth = 65535;
    constants->maxConvolution2DWidth = 65535;
    constants->maxConvolution2DHeight = 65535;
    constants->maxSeparable2DWidth = 65535;
    constants->maxSeparable2DHeight = 65535;

    constants->pointSizeMinimum = 0.5;
    constants->pointSizeMaximum = 100.0;
    constants->pointSizeGranularity = 0.125;
    constants->lineWidthMinimum = 0.1f;
    constants->lineWidthMaximum = 100.0;
    constants->lineWidthGranularity = 0.125;

    constants->maxProgErrStrLen = 256;

    constants->pixelPipelineCopySimulation  = GL_FALSE;
    constants->pixelPipelineDrawSimulation  = GL_FALSE;

    /*set conservative values*/
    constants->maxRenderBufferWidth         = 2048;
    constants->maxRenderBufferHeight        = 2048;
    constants->maxViewportWidth             = 2048;
    constants->maxViewportHeight            = 2048;
    constants->maxDrawBuffers               = 1;
    constants->maxSamples                   = 16;
}

GLvoid __glInitDevPipeDefault(__GLdevicePipeline *dp)
{

    /* DP context management */
    __GL_NOOP_RET1(dp->ctx.makeCurrent);
    __GL_NOOP_RET1(dp->ctx.loseCurrent);
    __GL_NOOP(dp->ctx.destroyPrivateData);
    __GL_NOOP(dp->ctx.drawPrimitive);
    __GL_NOOP(dp->ctx.tnLAndAccumInPipeline);
    __GL_NOOP(dp->ctx.drawPixels);
    __GL_NOOP(dp->ctx.copyPixels);
    __GL_NOOP(dp->ctx.readPixels);
    __GL_NOOP(dp->ctx.bitmaps);
    __GL_NOOP(dp->ctx.rasterPos);
    __GL_NOOP(dp->ctx.notifyChangeBufferSize);
    __GL_NOOP(dp->ctx.notifyDestroyBuffers);
    __GL_NOOP(dp->ctx.notifySwapBuffers);
    __GL_NOOP(dp->ctx.notifyDrawableSwitch);
    __GL_NOOP(dp->ctx.notifyWinBuffersResident);
    __GL_NOOP(dp->ctx.deviceConfigurationChanged);

    /* Draw primitive begin, end */
    __GL_NOOP(dp->begin);
    __GL_NOOP(dp->end);

    /* Raster function begin, end */
    __GL_NOOP(dp->rasterBegin);
    __GL_NOOP(dp->rasterEnd);

    /* Raster position begin, end */
    __GL_NOOP(dp->rasterPosBegin);
    __GL_NOOP(dp->rasterPosEnd);

    /* Used to notify DP of GL attribute changes */
    __GL_NOOP(dp->attributeChanged);

    /* Used to delete cached immediate mode primitive data */
    __GL_NOOP(dp->deletePrimData);

    /* Attributes */
    __GL_NOOP(dp->drawBuffers);
    __GL_NOOP(dp->drawBuffer);
    __GL_NOOP(dp->readBuffer);
    __GL_NOOP(dp->colorMaterial);
    __GL_NOOP(dp->pixelStore);
    __GL_NOOP(dp->pixelMap);
    __GL_NOOP(dp->pixelTransfer);
    __GL_NOOP(dp->pixelZoom);

    /* Attribute Enable/Disable */
    __GL_NOOP(dp->colorMaterialEndisable);
    __GL_NOOP(dp->colorTableEndisable);
    __GL_NOOP(dp->postConvColorTableEndisable);
    __GL_NOOP(dp->postColorMatrixColorTableEndisable);
    __GL_NOOP(dp->convolution1DEndisable);
    __GL_NOOP(dp->convolution2DEndisable);
    __GL_NOOP(dp->minmaxEndisable);
    __GL_NOOP(dp->histogramEndisable);
    __GL_NOOP(dp->separable2DEndisable);

    /* Color table functions */
    __GL_NOOP(dp->colorTable);
    __GL_NOOP(dp->texColorTable);
    __GL_NOOP(dp->convColorTable);
    __GL_NOOP(dp->postColorMatrixColorTable);

    __GL_NOOP(dp->colorSubTable);
    __GL_NOOP(dp->texColorSubTable);
    __GL_NOOP(dp->convColorSubTable);
    __GL_NOOP(dp->postColorMatrixColorSubTable);

    __GL_NOOP(dp->copyColorTable);
    __GL_NOOP(dp->copyTexColorTable);
    __GL_NOOP(dp->copyConvColorTable);
    __GL_NOOP(dp->copyPostColorMatrixColorTable);

    __GL_NOOP(dp->copyColorSubTable);
    __GL_NOOP(dp->copyTexColorSubTable);
    __GL_NOOP(dp->copyConvColorSubTable);
    __GL_NOOP(dp->copyPostColorMatrixColorSubTable);

    /* Texture functions */
    __GL_NOOP(dp->bindTexture);
    __GL_NOOP(dp->deleteTexture);
    __GL_NOOP_RET0(dp->isTextureResident);

    __GL_NOOP(dp->getTexImage);
    __GL_NOOP(dp->getCompressedTexImage);

    __GL_NOOP(dp->texImage1D);
    __GL_NOOP(dp->texImage2D);
    __GL_NOOP(dp->texImage3D);

    __GL_NOOP(dp->compressedTexImage1D);
    __GL_NOOP(dp->compressedTexImage2D);
    __GL_NOOP(dp->compressedTexImage3D);

    __GL_NOOP(dp->copyTexImage1D);
    __GL_NOOP(dp->copyTexImage2D);

    __GL_NOOP(dp->texSubImage1D);
    __GL_NOOP(dp->texSubImage2D);
    __GL_NOOP(dp->texSubImage3D);

    __GL_NOOP(dp->compressedTexSubImage1D);
    __GL_NOOP(dp->compressedTexSubImage2D);
    __GL_NOOP(dp->compressedTexSubImage3D);

    __GL_NOOP(dp->copyTexSubImage1D);
    __GL_NOOP(dp->copyTexSubImage2D);

    __GL_NOOP(dp->activeTexture);
    __GL_NOOP(dp->clientActiveTexture);

    __GL_NOOP(dp->syncTextureFromDeviceMemory);

    __GL_NOOP(dp->texParameter);


    /* Get functions */
    __GL_NOOP(dp->get);
    __GL_NOOP(dp->getTexLevelParameter);
    __GL_NOOP(dp->getTexParameter);

    /* Imaging extensions */
    __GL_NOOP(dp->histogram);
    __GL_NOOP(dp->resetHistogram);
    __GL_NOOP(dp->minmax);
    __GL_NOOP(dp->resetMinmax);
    __GL_NOOP(dp->convolutionFilter1D);
    __GL_NOOP(dp->convolutionFilter2D);
    __GL_NOOP(dp->separableFilter2D);
    __GL_NOOP(dp->copyConvolutionFilter1D);
    __GL_NOOP(dp->copyConvolutionFilter2D);

    /* Vertex buffer object extension */
    __GL_NOOP(dp->bufferData);
    __GL_NOOP(dp->bindBuffer);
    __GL_NOOP(dp->bufferSubData);
    __GL_NOOP(dp->mapBuffer);
    __GL_NOOP_RET0(dp->unmapBuffer);
    __GL_NOOP(dp->deleteBuffer);
    __GL_NOOP(dp->getBufferSubData);

    /* Occlusion query extension */
    __GL_NOOP(dp->beginQuery);
    __GL_NOOP(dp->endQuery);
    __GL_NOOP(dp->getQueryObject);

    /* Flush, Finish */
    __GL_NOOP(dp->flush);
    __GL_NOOP(dp->finish);
    __GL_NOOP(dp->clear);

    /* Accumulation functions */
    __GL_NOOP(dp->accum);

    /* Program functions  */
    __GL_NOOP_RET0(dp->ProgramStringARB);
    __GL_NOOP(dp->DeleteProgramARB);
    __GL_NOOP(dp->BindProgramARB);

    /* OpenGL 2.0 GLSL APIs */
    __GL_NOOP(dp->deleteShader);
    __GL_NOOP(dp->compileShader);
    __GL_NOOP(dp->useProgram);
    __GL_NOOP(dp->linkProgram);
    __GL_NOOP(dp->useProgram);
    __GL_NOOP(dp->validateShaderProgram);
    __GL_NOOP(dp->deleteShaderProgram);
    __GL_NOOP(dp->getActiveUniform);
    __GL_NOOP(dp->getUniformLocation);
    __GL_NOOP(dp->bindAttributeLocation);
    __GL_NOOP(dp->getAttributeLocation);

    __GL_NOOP(dp->createConstantBuffer);
    __GL_NOOP(dp->destroyConstantBuffer);
    __GL_NOOP(dp->syncConstantBuffer);
    __GL_NOOP(dp->constantBufferMakeResident);

    /* GL_EXT_framebuffer_object */
    __GL_NOOP(dp->bindDrawFramebuffer);
    __GL_NOOP(dp->bindReadFramebuffer);
    __GL_NOOP(dp->bindRenderbuffer);
    __GL_NOOP(dp->deleteRenderbuffer);
    __GL_NOOP_RET0(dp->renderbufferStorage);
    __GL_NOOP(dp->blitFramebuffer);
    __GL_NOOP(dp->frameBufferTexture);
    __GL_NOOP(dp->framebufferRenderbuffer);
    __GL_NOOP_RET1(dp->isFramebufferComplete);

    /* WGL_EXT_swap_control */
    __GL_NOOP_RET0(dp->swapInterval);

    /* Render texture */
    __GL_NOOP_RET0(dp->bindTexImageARB);
    __GL_NOOP_RET0(dp->releaseTexImageARB);

    /* Free vertex data (IM or DL) cached in video memory */
    __GL_NOOP_RET0(dp->updatePrivateData);

    /* Sync the front buffer to the fake front buffer */
    __GL_NOOP(dp->syncFrontToFakeFront);

    /* Update shading mode on hash table. */
    __GL_NOOP(dp->updateShadingMode);
}

__GL_INLINE GLvoid __glCopyAttributeState(__GLattribute *dst, __GLattribute *src)
{
    __GL_MEMCOPY(dst, src, sizeof(__GLattribute));
}

GLvoid __glFreeDataCacheInVideoMemory(__GLcontext *gc)
{
    /* Free DL vertex cache if it is in video memory */
    __glFreeDlistVertexCache(gc);
    __glFreeConcatDlistCache(gc);

}

GLuint __glLoseCurrent(__GLcontext *gc, GLvoid **thrArea)
{
    GLuint status = 0;
    GLboolean bkickoffcmd = GL_TRUE;

    __GL_VERTEX_BUFFER_FLUSH(gc);

    /* Get the latest drawable information */
    LINUX_LOCK_FRAMEBUFFER(gc);

    /* Flush all GL attributes to device pipeline before lose current.
    ** Now gc->commitState contents are exactly the same as gc->state.
    */
    __glDispatchDrawableChange(gc);

    if (gc->globalDirtyState[__GL_ALL_ATTRS])
    {
        __glEvaluateAttributeChange(gc);
    }

    /* thrArea is NULL means that this function is called by detachProcess,
    ** so we can discard commands in command buffer to avoid screen saver image remain on screen
    */
    if (!thrArea)
    {
        bkickoffcmd = GL_FALSE;
    }

    /* Notify DP the context is not current anymore */
    status = (*gc->dp.ctx.loseCurrent)(gc, bkickoffcmd);
    if (!status) {
        LINUX_UNLOCK_FRAMEBUFFER(gc);
        return status;
    }

#if SWP_PIPELINE_ENABLED
    /* Notify software pipeline the context is not current anymore */
    status = (*gc->swp.ctx.loseCurrent)(gc, bkickoffcmd);
    if (!status) {
        LINUX_UNLOCK_FRAMEBUFFER(gc);
        return status;
    }
#endif

    LINUX_UNLOCK_FRAMEBUFFER(gc);

    /* Free all vertex data caches in video memory */
    __glFreeDataCacheInVideoMemory(gc);
    __glFreeVertexInputState(gc);
    __glFreeVertexOutputState(gc);
    /* The context is not the current context of the thread any longer */
    gc->thrHashId = __GL_INVALID_THREAD_HASH_ID;

    return status;
}

GLuint __glMakeCurrent(__GLcontext *gc, GLuint thrHashId, GLvoid **thrArea)
{
    GLuint status = 0;
    GLsizei width, height;

    __GL_MEMCOPY(&gc->modes, &gc->drawablePrivate->modes, sizeof(__GLcontextModes));

    __glInitVertexInputState(gc);
    __glInitVertexOutputState(gc);

    /* Cache thrHashId in gc */
    gc->thrHashId = thrHashId;

    if (gc->flags & __GL_CONTEXT_UNINITIALIZED) {
        width = gc->drawablePrivate->width;
        height = gc->drawablePrivate->height;

        /* Initialize the viewport and scissor to the whole window */
        __glUpdateViewport(gc, 0, 0, width, height);
        __glUpdateScissor(gc, 0, 0, width, height);

        /*
        ** These flags should be initialized once.
        */

        gc->input.inputMaskChanged = GL_TRUE;

        /* for draw instanced ext */
        gc->input.primCount = 1;


        /* Must reset drawables/RTs for newly created gc */
        gc->changeMask |= __GL_DRAWABLE_PENDING_RESIZE;

        gc->flags &= ~(__GL_CONTEXT_UNINITIALIZED);
    }

    /* Get the latest drawable information */
    LINUX_LOCK_FRAMEBUFFER(gc);

    /* Notify the DP of the new context drawable pair */
    status = (*gc->dp.ctx.makeCurrent)(gc);
    if (!status) {
        LINUX_UNLOCK_FRAMEBUFFER(gc);
        return status;
    }

#if SWP_PIPELINE_ENABLED
    /* Notify the software pipeline of the new context drawable pair */
    status = (*gc->swp.ctx.makeCurrent)(gc);
    if (!status) {
        LINUX_UNLOCK_FRAMEBUFFER(gc);
        return status;
    }
#endif

    /* Make sure the drawable is allocated and updated */
    __glDispatchDrawableChange(gc);
    if (gc->transform.clipSeqNum != gc->drawablePrivate->clipSeqNum)
    {
        __glComputeClipBox(gc);
    }

    LINUX_UNLOCK_FRAMEBUFFER(gc);

    /* Make sure the readable is allocated and updated
    ** __glDispatchReadableChange(gc);
    */

    width = gc->drawablePrivate->width;
    height = gc->drawablePrivate->height;

    /* Renitialize the viewport and scissor to the whole window */
    /* for Linux driver drawable width and height are initialized after here*/
    __glUpdateViewport(gc, 0, 0, width, height);
    __glUpdateScissor(gc, 0, 0, width, height);

    /* Validate states here to guarantee no state change event is lost.
    ** States derived from the basic gl states need these state change events.
    */
    __glEvaluateAttributeChange(gc);

    /* Reset immediate mode vertex buffer */
    __glResetImmedVertexBuffer(gc);

    return status;
}

GLuint __glShareContext(__GLcontext *gc, __GLcontext *gcShare)
{
    __glShareDlists(gc, gcShare);
    __glShareTextureObjects(gc, gcShare);
    __glShareBufferObjects(gc, gcShare);
    __glShareProgramObjects(gc, gcShare);
    __glShareShaderProgramObjects(gc, gcShare);

#if GL_EXT_framebuffer_object
    __glShareFrameBufferObjects(gc, gcShare);
#endif

    return GL_TRUE;
}

GLvoid __glAssociateContext(__GLcontext *gc, __GLdrawablePrivate *drawable, __GLdrawablePrivate *readable)
{
    gc->drawablePrivate = drawable;
    gc->readablePrivate = readable;
}

GLvoid __glDeassociateContext(__GLcontext *gc)
{
    if(gc->dp.unBindDpDrawable)
    {
        gc->dp.unBindDpDrawable(gc);
    }

    gc->drawablePrivate = NULL;
    gc->readablePrivate = NULL;
}

GLvoid __glDeviceConfigurationChanged(__GLcontext *gc)
{
    if (gc->drawablePrivate) {
        __GL_MEMCOPY(&gc->modes, &gc->drawablePrivate->modes, sizeof(__GLcontextModes));
    }

    /* Notify dp display configuration changed */
    (*gc->dp.ctx.deviceConfigurationChanged)(gc);

#if SWP_PIPELINE_ENABLED
    /* Notify swp display configuration changed */
    (*gc->swp.ctx.deviceConfigurationChanged)(gc);
#endif

    /* Free all vertex data caches in video memory */
    __glFreeDataCacheInVideoMemory(gc);

    /* Set all attributes dirty */
    __glSetAttributeStatesDirty(gc);

    /* Dirty current FBO*/
    gc->frameBuffer.drawFramebufObj->seqNumber++;
    if(gc->frameBuffer.readFramebufObj != gc->frameBuffer.drawFramebufObj)
    {
        gc->frameBuffer.readFramebufObj->seqNumber++;
    }
}

GLvoid __glNotifyRTMakeResident(__GLcontext *gc)
{
    __GLpipeline *ctx;
    ctx = &gc->dp.ctx;
    if(ctx->notifyWinBuffersResident)
    {
        ctx->notifyWinBuffersResident(gc, gc->drawablePrivate);
    }

#if SWP_PIPELINE_ENABLED
    ctx = &gc->swp.ctx;
    if(ctx->notifyWinBuffersResident)
    {
        ctx->notifyWinBuffersResident(gc, gc->drawablePrivate);
    }
#endif
}

GLvoid __glNotifySwapBuffers(__GLcontext *gc)
{
    __GLpipeline *ctx;
    ctx = &gc->dp.ctx;
    if(ctx->notifySwapBuffers)
    {
        ctx->notifySwapBuffers(gc);
    }

#if SWP_PIPELINE_ENABLED
    ctx = &gc->swp.ctx;
    if(ctx->notifySwapBuffers)
    {
        ctx->notifySwapBuffers(gc);
    }
#endif
}

GLvoid __glNotifyChangeBufferSize(__GLcontext *gc)
{
    __GLpipeline *ctx;
    __GLdrawablePrivate *draw = gc->drawablePrivate;
    GLint yInvert = (DRAW_FRAMEBUFFER_BINDING_NAME == 0)? draw->yInverted :GL_FALSE;

    ctx = &gc->dp.ctx;
    if(ctx->notifyChangeBufferSize)
    {
        /* Release exclusive mode first, then create new RTs */
        if((draw->fullScreenMode) && (draw->type == __GL_WINDOW) && (__glDevice->IsEXCLUSIVE_MODE))
        {
            (*draw->dp.setExclusiveDisplay)(GL_FALSE);
            draw->flipOn = GL_FALSE;
        }

        ctx->notifyChangeBufferSize(gc);
    }

#if SWP_PIPELINE_ENABLED
    ctx = &gc->swp.ctx;
    if(ctx->notifyChangeBufferSize)
    {
        ctx->notifyChangeBufferSize(gc);
    }
#endif

    /* Reset raster pos to default value */
    if (yInvert)
        gc->state.rasterPos.rPos.winPos.y = (GLfloat)gc->drawablePrivate->height;
    else
        gc->state.rasterPos.rPos.winPos.y = 0;

    /* Compute clip box */
    __glComputeClipBox(gc);
}

GLvoid __glNotifyDestroyBuffers(__GLcontext *gc)
{
     __GLpipeline *ctx;

    ctx = &gc->dp.ctx;
    if(ctx->notifyDestroyBuffers)
    {
        ctx->notifyDestroyBuffers(gc);
    }

#if SWP_PIPELINE_ENABLED
    ctx = &gc->swp.ctx;
    if(ctx->notifyDestroyBuffers)
    {
        ctx->notifyDestroyBuffers(gc);
    }
#endif
}

GLvoid __glNotifyDrawableSwitch(__GLcontext *gc)
{
    __GLpipeline *ctx;

    ctx = &gc->dp.ctx;
    if(ctx->notifyDrawableSwitch)
    {
        ctx->notifyDrawableSwitch(gc);
    }

#if SWP_PIPELINE_ENABLED
    ctx = &gc->swp.ctx;
    if(ctx->notifyDrawableSwitch)
    {
        ctx->notifyDrawableSwitch(gc);
    }
#endif

    __glComputeClipBox(gc);
}

/*
** This function is called in __glEvaluateAttributeChange() if drawable change dirty bit is set.
** The purpose of this function is to notify dp that drawable has changed so that dp could
** respond accordingly.
**
** Four kinds of drawable change are processed in this function:
** 1. drawable buffer size changed ---> __GL_DRAWABLE_PENDING_RESIZE
** 2. drawable buffer position changed-->__GL_DRAWABLE_PENDING_MOVE
** 3. drawable buffer destroy ---> __GL_DRAWABLE_PENDING_DESTROY
** 4. drawable buffer swap -----> __GL_DRAWABLE_PENDING_SWAP
** 5. drawable buffer need make resident ---> __GL_DRAWABLE_PENDING_RT_RESIDENT
*/
GLvoid __glDispatchDrawableChange(__GLcontext *gc)
{
    /*
    ** The window message handler may not run in the same thread as the
    ** GL drawing thread, so we must lock the access to "gc->changeMask"
    ** which will be changed by window message handler and the GL drawing thread.
    ** This Lock is also necessary to make sure that at one time only one context is
    ** updating the drawable (We rely on a context to resize the drawable).
    */
    (*gc->imports.lockMutex)(&drawableChangeLock);

    if ((gc->changeMask & (__GL_DRAWABLE_PENDING_RESIZE |
                      __GL_DRAWABLE_PENDING_MOVE |
                      __GL_DRAWABLE_PENDING_DESTROY |
                      __GL_DRAWABLE_PENDING_SWITCH |
                      __GL_DRAWABLE_PENDING_SWAP |
                      __GL_DRAWABLE_PENDING_RT_RESIDENT |
                      __GL_DRAWABLE_PENDING_PRIMARY_LOST)) == 0x0)
    {
        goto DispatchDrawableChange_Exit;
    }

    /* make resident rt which have been swapped out
    ** Note: Handle this pending info before __GL_DRAWABLE_PENDING_RESIZE*/
    if(gc->changeMask & __GL_DRAWABLE_PENDING_RT_RESIDENT)
    {
        __glNotifyRTMakeResident(gc);
        gc->changeMask &= ~ __GL_DRAWABLE_PENDING_RT_RESIDENT;
    }

    /* Drawable primary surface lost is pending. Dispatch appropriately */
    if (gc->changeMask & __GL_DRAWABLE_PENDING_PRIMARY_LOST )
    {
        __glNotifyChangeBufferSize(gc);
        gc->changeMask &= ~__GL_DRAWABLE_PENDING_PRIMARY_LOST;
    }

    /* Drawable resize is pending. Dispatch appropriately */
    if (gc->changeMask & __GL_DRAWABLE_PENDING_RESIZE ) {
        __glNotifyChangeBufferSize(gc);
        gc->changeMask &= ~__GL_DRAWABLE_PENDING_RESIZE;
    }

    /*Drawable position change is pending. Dispatch approprately*/
    if (gc->changeMask & __GL_DRAWABLE_PENDING_MOVE) {
        /*
        ** gc has nothing to do if position changed,
        ** but the bit should be cleared here.
        */
        gc->changeMask &= ~__GL_DRAWABLE_PENDING_MOVE;
    }

    /* Drawable destroy is pending. Dispatch appropriately */
    if (gc->changeMask & __GL_DRAWABLE_PENDING_DESTROY) {
        __glNotifyDestroyBuffers(gc);
        gc->changeMask &= ~__GL_DRAWABLE_PENDING_DESTROY;
    }

    /* Drawable buffer swap is pending. Dispatch appropriately */
    if (gc->changeMask & __GL_DRAWABLE_PENDING_SWAP) {
        __glNotifySwapBuffers(gc);
        gc->changeMask &= ~__GL_DRAWABLE_PENDING_SWAP;
    }

    /* Drawable switch between different type */
    if (gc->changeMask & __GL_DRAWABLE_PENDING_SWITCH) {
        __glNotifyDrawableSwitch(gc);
        gc->changeMask &= ~__GL_DRAWABLE_PENDING_SWITCH;
    }


DispatchDrawableChange_Exit:

    (*gc->imports.unlockMutex)(&drawableChangeLock);
}

/*
** Notify gc that the drawable changes,
** the drawable change dirty bits are set right before this function is called
*/
GLvoid __glNotifyDrawableChange(__GLcontext *gc, GLuint mask)
{
    /*
    ** The window message handler may not run in the same thread as the
    ** GL drawing thread, so we must lock the access to "gc->changeMask"
    ** which will be changed by window message handler and the GL drawing thread.
    ** This Lock is also necessary to make sure that at one time only one context is
    ** updating the drawable (We rely on a context to resize the drawable).
    */
    (*gc->imports.lockMutex)(&drawableChangeLock);

    gc->changeMask |= mask;

    /* If DrvSwapBuffers is called */
    if (mask & __GL_DRAWABLE_PENDING_SWAP) {

        /*
        ** Before swapping buffers, if there is PENDING_RESIZE or PENDING_MOVE,
        ** we must update the drawable first, otherwise swapBuffer will not behave
        ** correctly( most probably there will be garbage)
        */

        /* make resident rt which have been swapped out
        ** Note: Handle this pending info before __GL_DRAWABLE_PENDING_RESIZE*/
        if (gc->changeMask & __GL_DRAWABLE_PENDING_RT_RESIDENT)
        {
            __glNotifyRTMakeResident(gc);
            gc->changeMask &= ~ __GL_DRAWABLE_PENDING_RT_RESIDENT;
        }

        /* Drawable primary surface lost is pending. Dispatch appropriately */
        if (gc->changeMask & __GL_DRAWABLE_PENDING_PRIMARY_LOST )
        {
            __glNotifyChangeBufferSize(gc);
            gc->changeMask &= ~__GL_DRAWABLE_PENDING_PRIMARY_LOST;
        }

        /* Drawable resize is pending. Dispatch appropriately */
        if (gc->changeMask & __GL_DRAWABLE_PENDING_RESIZE ) {
            __glNotifyChangeBufferSize(gc);
            gc->changeMask &= ~__GL_DRAWABLE_PENDING_RESIZE;
            /*
            ** An extra flush is needed here, otherwise the back buffer will be uninitialized
            ** and there will be garbage swapped to front buffer.
            */
            (*gc->dp.flush)(gc);
        }

        /*Drawable position change is pending. Dispatch approprately*/
        if (gc->changeMask & __GL_DRAWABLE_PENDING_MOVE) {
            /*
            ** gc has nothing to do if position changed,
            ** but the bit should be cleared here.
            */
            gc->changeMask &= ~__GL_DRAWABLE_PENDING_MOVE;
        }

        /* Reset immediate mode vertex buffer */
        __glResetImmedVertexBuffer(gc);
    }

    (*gc->imports.unlockMutex)(&drawableChangeLock);
}

/*
** Notify gc that multi-threaded rendering is on or off.
*/
GLvoid __glNotifyMultiThread(__GLcontext *gc, GLuint mtFlag)
{
    gc->multiThreadOn = mtFlag;
}

GLvoid __glInitGlobalVariables(__GLcontext *gc)
{
    GLint i;

    /* Init the lookup table used in macro __GL_UB_TO_FLOAT.
    */
    for (i = 0; i < 256; i++) {
        g_uByteToFloat[i] = i * __glInvMaxUbyte;
    }

    /* Overwrite some ListCompileTable entries with __glim_FuncName.
    */
    __glOverWriteListCompileTable();

    g_initGlobals = GL_FALSE;
}

GLvoid __glInitContextState(__GLcontext *gc)
{
    gc->flags = __GL_CONTEXT_UNINITIALIZED;
    gc->renderMode = GL_RENDER;

    __glInitCurrentState(gc);
    __glInitAttribStackState(gc);
    __glInitHintState(gc);
    __glInitRasterState(gc);
    __glInitStencilState(gc);
    __glInitDepthState(gc);
    __glInitTransformState(gc);
    __glInitFeedback(gc);
    __glInitSelect(gc);
    __glInitFogState(gc);
    __glInitLightState(gc);
    __glInitPointState(gc);
    __glInitLineState(gc);
    __glInitPolygonState(gc);
    __glInitEvaluatorState(gc);
    __glInitVertexArrayState(gc);
    __glInitPixelState(gc);
    __glInitPixelMachine(gc);
    __glInitAccumState(gc);
    __glInitMultisampleState(gc);
    __glInitDlistState(gc);
    __glInitTextureState(gc);
    __glInitBufferObjectState(gc);
    __glInitProgramState(gc);
    __glInitShaderProgramState(gc);
    __glInitQueryState(gc);
    __glInitPixelSpanInfo(gc);

#if GL_EXT_framebuffer_object
    __glInitFramebufferStates(gc);
#endif

    /* Initialize the primMode to an invalid primitive type */
    gc->vertexStreams.primMode = 0;

    __glSetAttributeStatesDirty(gc);
}

GLvoid __glInitObjectTemplate(__GLcontext *gc)
{
    __glInitTexObjTemplate(gc);
    __glInitBufObjTemplate(gc);
}

GLuint __glDestroyContext(__GLcontext *gc)
{
     GLuint status = 1;

    if(gc->imports.other) {
        if (gc->constants.extensions) {
            (*gc->imports.free)(gc, gc->constants.extensions);
            gc->constants.extensions = NULL;
        }

        __glFreeAttribStackState(gc);
        __glFreeTransformState(gc);
        __glFreeSelectState(gc);
        __glFreeVertexInputState(gc);
        __glFreeVertexOutputState(gc);
        __glFreePixelMapState(gc);
        __glFreePixelSpanInfo( gc );
        __glFreeEvaluatorState(gc);
        __glFreeDlistState(gc);
#if GL_EXT_framebuffer_object
        __glFreeFramebufferStates(gc);
#endif
        __glFreeTextureState(gc);
        __glFreeBufferObjectState(gc);
        __glFreeProgramState(gc);
        __glFreeShaderProgramState(gc);
        __glFreeQueryState(gc);

        /* Notify DP to destroy its private data */
        status = (*gc->dp.ctx.destroyPrivateData)(gc);
        if (!status) {
            return status;
        }

#if SWP_PIPELINE_ENABLED
        /* Notify software pipeline to destroy its private data */
        status = (*gc->swp.ctx.destroyPrivateData)(gc);
        if (!status) {
            return status;
        }
#endif

        /* Destroy the mutex lock for drawable change */
        (*gc->imports.destroyMutex)(&drawableChangeLock);

#ifdef _LINUX_
        /* Free vvtDriMirror structure attached to gc->imports.other */
        (*gc->imports.free)(gc, gc->imports.other);
#endif
    }

    (*gc->imports.free)(gc, gc);

    return status;
}

__GLcontext *__glCreateContext(__GLimports *imports, __GLcontextModes *modes)
{
    __GLdeviceStruct *__glDevice = &((__GLdeviceStruct*)imports->device)[imports->deviceIndex];
    __GLcontext *gc = NULL;

    /* Allocate memory for core GL context.
    */
    gc = (__GLcontext*)(*imports->calloc)(gc, 1, sizeof(__GLcontext) );
    if (!gc) {
        return(0);
    }

    gc->imports = *imports;
    gc->modes = *modes;

    gc->thrHashId = __GL_INVALID_THREAD_HASH_ID;

    /* Fill in the export functions.
    */
    gc->exports.destroyContext = __glDestroyContext;
    gc->exports.loseCurrent = __glLoseCurrent;
    gc->exports.makeCurrent = __glMakeCurrent;
    gc->exports.shareContext = __glShareContext;
    gc->exports.copyContext = __glCopyContext;
    gc->exports.associateContext = __glAssociateContext;
    gc->exports.deassociateContext = __glDeassociateContext;
    gc->exports.deviceConfigurationChanged = __glDeviceConfigurationChanged;
    gc->exports.notifyDrawableChange = __glNotifyDrawableChange;
    gc->exports.notifyMultiThread = __glNotifyMultiThread;
    gc->exports.swapInterval = __glSwapInterval;
    gc->exports.bindTexImageARB = __glBindTexImageARB;
    gc->exports.releaseTexImageARB = __glReleaseTexImageARB;

    /* Initialize global variables just once.
    */
    if (g_initGlobals) {
        __glInitGlobalVariables(gc);
    }

    /* If imports->other is NULL, an invalid gc is being created.
    */
    if (imports->other) {

        /* Initialize the device constants with SI defaults.
        */
        __glInitConstantDefault(&gc->constants);

        /* Call device specific interface to over-write the default constant
        */
        (*__glDevice->devGetConstants)(&gc->constants);

        /* Initialize object Template
        */
        __glInitObjectTemplate(gc);

        /* Initialize the core GL context states.
        */
        __glInitContextState(gc);

#if SWP_PIPELINE_ENABLED
        /* Create the Swp specific context and attach it to gc->swp.ctx.privateData.
        ** And initialize the SW pipeline.
        */
        __glSwpCreateContext(gc);
#endif
        /* Initialize DP context function pointers with default Noop functions.
        */
        __glInitDevPipeDefault(&gc->dp);

        /* Create the device specific context and attach it to gc->dp.ctx.privateData.
        ** And initialize the device pipeline "gc->dp" function pointers.
        */
        (*__glDevice->devCreateContext)(gc);

        /* Generate extension string and proc table.
        */
        __glInitExtensions(gc);

        /* Initialize the mutex lock for drawable change.
        */
        (*gc->imports.createMutex)(&drawableChangeLock);

        /* Initialize dispatch tables.
        */
        gc->immediateDispatchTable = __glImmediateFuncTable;
        gc->listCompileDispatchTable = __glListCompileFuncTable;

        /* Set current immediate mode dispatch table.
        */
        __GL_SET_API_DISPATCH(__GL_IMMEDIATE_TABLE_OFFSET);
        gc->currentImmediateTable = &gc->immediateDispatchTable;

        /* Set pipeline pointer to device pipeline by default.
        */
        gc->pipeline = &gc->dp.ctx;
    }

    return (gc);
}

GLvoid __glSetError(GLenum code)
{
    __GL_SETUP();

#ifdef DEBUG
    if(code)
        dbgError("\n******************  __glSetError( %d )\n", code);
#endif

    if (!gc->error) {
        gc->error = code;
    }
}

