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


#include "gc_es_context.h"
#include "gc_es_device.h"


#define _GC_OBJ_ZONE __GLES3_ZONE_CORE
/*******************************************************************************
***** Version Signature *******************************************************/

#define _gcmTXT2STR(t) #t
#define gcmTXT2STR(t) _gcmTXT2STR(t)
const char * _GLESV2_VERSION = "\n\0$VERSION$"
                                gcmTXT2STR(gcvVERSION_MAJOR) "."
                                gcmTXT2STR(gcvVERSION_MINOR) "."
                                gcmTXT2STR(gcvVERSION_PATCH) ":"
                                gcmTXT2STR(gcvVERSION_BUILD)
                               "$\n";

extern GLvoid *__eglMalloc(size_t size);
extern GLvoid __eglFree(GLvoid *ptr);

extern GLboolean __glDpInitialize(__GLdeviceStruct deviceEntry[]);

extern GLvoid __glInitPixelState(__GLcontext *gc);

extern GLvoid __glInitBufferObjectState(__GLcontext *gc);
extern GLvoid __glFreeBufferObjectState(__GLcontext *gc);

extern GLvoid __glInitTextureState(__GLcontext *gc);
extern GLvoid __glFreeTextureState(__GLcontext *gc);

extern GLvoid __glInitSamplerState(__GLcontext *gc);
extern GLvoid __glFreeSamplerState(__GLcontext *gc);

extern GLvoid __glInitShaderProgramState(__GLcontext *gc);
extern GLvoid __glFreeShaderProgramState(__GLcontext * gc);

extern GLvoid __glInitFramebufferStates(__GLcontext *gc);
extern GLvoid __glFreeFramebufferStates(__GLcontext *gc);

extern GLvoid __glInitSyncState(__GLcontext *gc);
extern GLvoid __glFreeSyncState(__GLcontext *gc);

extern GLvoid __glInitXfbState(__GLcontext *gc);
extern GLvoid __glFreeXfbState(__GLcontext *gc);

extern GLvoid __glInitQueryState(__GLcontext *gc);
extern GLvoid __glFreeQueryState(__GLcontext *gc);

extern GLvoid __glInitVertexArrayState(__GLcontext *gc);
extern GLvoid __glFreeVertexArrayState(__GLcontext * gc);

extern GLvoid __glUpdateViewport(__GLcontext *gc, GLint x, GLint y, GLsizei w, GLsizei h);
extern GLvoid __glUpdateScissor(__GLcontext *gc, GLint x, GLint y, GLint w, GLint h);

extern GLvoid __glInitImageState(__GLcontext *gc);

extern GLvoid __glInitDebugState(__GLcontext *gc);
extern GLvoid __glFreeDebugState(__GLcontext *gc);

extern GLboolean __glInitTracerDispatchTable(GLint trmode, __GLApiVersion apiVersion);
extern __GLesDispatchTable __glesNopDispatchFuncTable;
extern __GLesDispatchTable __glesApiProfileDispatchTable;
extern GLvoid __glSetDrawable(__GLcontext* gc, __GLdrawablePrivate* drawable, __GLdrawablePrivate* readable);

extern __GLformatInfo __glFormatInfoTable[];

gceTRACEMODE __glesApiTraceMode = gcvTRACEMODE_NONE;
GLint __glesApiProfileMode = -1;


#define __GL_NOOP(fp)      *(GLvoid (**)())&(fp) = __glNoop;
#define __GL_NOOP_RET0(fp) *(GLuint (**)())&(fp) = __glNoop_Return0;
#define __GL_NOOP_RET1(fp) *(GLuint (**)())&(fp) = __glNoop_Return1;

#if gcdPATTERN_FAST_PATH
__GLapiPattern gfxPattern0 = {
    {
        { __glesApiEnum(Uniform2f) , {0, 0, 0, 0}},
        { __glesApiEnum(Uniform2f) , {0, 0, 0, 0}},
        { __glesApiEnum(Uniform1f) , {0, 0, 0, 0}},
        { __glesApiEnum(Uniform2f) , {0, 0, 0, 0}},
        { __glesApiEnum(Uniform2f) , {0, 0, 0, 0}},
        { __glesApiEnum(Uniform4f) , {0, 0, 0, 0}},
        { __glesApiEnum(Uniform4f) , {0, 0, 0, 0}},
        { __glesApiEnum(Uniform4f) , {0, 0, 0, 0}},
        { __glesApiEnum(Uniform4f) , {0, 0, 0, 0}},
        { __glesApiEnum(BlendColor) , {0, 0, 0, 0}},
        { __glesApiEnum(Disable) , {0x0B71, 0, 0, 0}},
        { __glesApiEnum(DepthFunc) , {0, 0, 0, 0}},
        { __glesApiEnum(BlendFunc) , {0, 0, 0, 0}},
        { __glesApiEnum(Enable) , {0x0BE2, 0, 0, 0}},
        { __glesApiEnum(BindBuffer) , {0, 0, 0, 0}},
        { __glesApiEnum(BindBuffer) , {0, 0, 0, 0}},
        { __glesApiEnum(VertexAttribPointer) , {0x1406, 0, 0, 0}},
        { __glesApiEnum(VertexAttribPointer) , {0x1406, 0, 0, 0}},
        { __glesApiEnum(DrawElements) , {0x4, 0x1403, 0, 0}},
    },
    19
};

__GLapiPattern gfxPattern1 = {
    {
        { __glesApiEnum(Uniform2f) , {0, 0, 0, 0}},
        { __glesApiEnum(Uniform2f) , {0, 0, 0, 0}},
        { __glesApiEnum(Uniform1f) , {0, 0, 0, 0}},
        { __glesApiEnum(Uniform2f) , {0, 0, 0, 0}},
        { __glesApiEnum(Uniform2f) , {0, 0, 0, 0}},
        { __glesApiEnum(Uniform4f) , {0, 0, 0, 0}},
        { __glesApiEnum(Uniform4f) , {0, 0, 0, 0}},
        { __glesApiEnum(Uniform4f) , {0, 0, 0, 0}},
        { __glesApiEnum(Uniform4f) , {0, 0, 0, 0}},
        { __glesApiEnum(BlendColor) , {0, 0, 0, 0}},
        { __glesApiEnum(Enable) , {0x0B71, 0, 0, 0}},
        { __glesApiEnum(DepthFunc) , {0, 0, 0, 0}},
        { __glesApiEnum(BlendFunc) , {0, 0, 0, 0}},
        { __glesApiEnum(Enable) , {0x0BE2, 0, 0, 0}},
        { __glesApiEnum(BindBuffer) , {0, 0, 0, 0}},
        { __glesApiEnum(BindBuffer) , {0, 0, 0, 0}},
        { __glesApiEnum(VertexAttribPointer) , {0x1406, 0, 0, 0}},
        { __glesApiEnum(VertexAttribPointer) , {0x1406, 0, 0, 0}},
        { __glesApiEnum(DrawElements) , {0x4, 0x1403, 0, 0}},
    },
    19
};
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

GLvoid __glInitConstantDefault(__GLdeviceConstants *constants)
{

    /* Specific size limits */
    constants->numberofQueryCounterBits = 32;
    constants->maxTextureSize = 8192;
    constants->maxNumTextureLevels = 14;
    constants->maxTextureLodBias = 16;
    constants->subpixelBits = 4;
    constants->maxElementsVertices = 32;
    constants->maxElementsIndices = 65535;

    constants->pointSizeMin = 0.5;
    constants->pointSizeMax = 100.0;

    constants->lineWidthMin = 1.0f;
    constants->lineWidthMax = 1.0f;
    constants->lineWidthGranularity = 0.125;

    constants->shaderCaps.maxProgErrStrLen = 256;

    /*set conservative values*/
    constants->maxRenderBufferSize  = 2048;
    constants->maxViewportWidth     = 2048;
    constants->maxViewportHeight    = 2048;
    constants->shaderCaps.maxDrawBuffers = 1;
    constants->maxSamples           = 4;
    constants->maxSamplesInteger    = 1;
    constants->maxSampleMaskWords   = 1;

    constants->maxServerWaitTimeout = 500000; /* nanoseconds */

    /* Those arrays need to be retrieved in Chip layer */
    constants->pCompressedTexturesFormats = gcvNULL;
    constants->pShaderBinaryFormats = gcvNULL;
    constants->pProgramBinaryFormats = gcvNULL;

    constants->maxTextureBufferSize = 65536;
    constants->textureBufferOffsetAlignment = 256;
}

GLvoid __glInitDevPipeDefault(__GLdevicePipeline *dp)
{
    /* DP context management */
    __GL_NOOP_RET1(dp->makeCurrent);
    __GL_NOOP_RET1(dp->loseCurrent);
    __GL_NOOP(dp->destroyPrivateData);
    __GL_NOOP(dp->drawPrimitive);
    __GL_NOOP(dp->readPixelsBegin);
    __GL_NOOP(dp->readPixelsValidateState);
    __GL_NOOP(dp->readPixelsEnd);
    __GL_NOOP(dp->readPixels);

    __GL_NOOP(dp->drawBegin);
    __GL_NOOP(dp->drawValidateState);
    __GL_NOOP(dp->drawEnd);

    /* Read/draw buffers */
    __GL_NOOP(dp->changeDrawBuffers);
    __GL_NOOP(dp->changeReadBuffers);
    __GL_NOOP(dp->detachDrawable);

    /* Texture functions */
    __GL_NOOP(dp->bindTexture);
    __GL_NOOP(dp->deleteTexture);
    __GL_NOOP(dp->detachTexture);
    __GL_NOOP(dp->texImage2D);
    __GL_NOOP(dp->texImage3D);
    __GL_NOOP(dp->texSubImage2D);
    __GL_NOOP(dp->texSubImage3D);
    __GL_NOOP(dp->copyTexImage2D);
    __GL_NOOP(dp->copyTexSubImage2D);
    __GL_NOOP(dp->copyTexSubImage3D);
    __GL_NOOP(dp->texDirectVIV);
    __GL_NOOP(dp->texDirectInvalidateVIV);
    __GL_NOOP(dp->texDirectVIVMap);
    __GL_NOOP(dp->compressedTexImage2D);
    __GL_NOOP(dp->compressedTexImage3D);
    __GL_NOOP(dp->compressedTexSubImage2D);
    __GL_NOOP(dp->compressedTexSubImage3D);
    __GL_NOOP(dp->generateMipmaps);

    __GL_NOOP(dp->copyTexBegin);
    __GL_NOOP(dp->copyTexValidateState);
    __GL_NOOP(dp->copyTexEnd);

    __GL_NOOP(dp->copyImageSubData);

    /* Texture functions */
    __GL_NOOP(dp->freeTexImage);
    __GL_NOOP(dp->createEglImageTexture);
    __GL_NOOP(dp->createEglImageRenderbuffer);
    __GL_NOOP(dp->eglImageTargetTexture2DOES);
    __GL_NOOP(dp->eglImageTargetRenderbufferStorageOES);
    __GL_NOOP(dp->getTextureAttribFromImage);

    /* Vertex buffer object extension */
    __GL_NOOP(dp->bufferData);
    __GL_NOOP(dp->mapBufferRange);
    __GL_NOOP(dp->flushMappedBufferRange);
    __GL_NOOP_RET0(dp->unmapBuffer);
    __GL_NOOP(dp->deleteBuffer);
    __GL_NOOP(dp->bufferData);
    __GL_NOOP(dp->bufferSubData);
    __GL_NOOP(dp->copyBufferSubData);

    /* Occlusion query extension */
    __GL_NOOP(dp->beginQuery);
    __GL_NOOP(dp->endQuery);
    __GL_NOOP(dp->getQueryObject);

    /* Flush, Finish */
    __GL_NOOP(dp->flush);
    __GL_NOOP(dp->finish);
    __GL_NOOP(dp->clearBegin);
    __GL_NOOP(dp->clearValidateState);
    __GL_NOOP(dp->clearEnd);
    __GL_NOOP(dp->clear);
    __GL_NOOP(dp->clearBuffer);
    __GL_NOOP(dp->clearBufferfi);

    /* GLSL APIs */
    __GL_NOOP(dp->deleteShader);
    __GL_NOOP(dp->compileShader);
    __GL_NOOP(dp->createProgram);
    __GL_NOOP(dp->deleteProgram);
    __GL_NOOP(dp->linkProgram);
    __GL_NOOP(dp->useProgram);
    __GL_NOOP_RET0(dp->validateProgram);
    __GL_NOOP(dp->bindAttributeLocation);
    __GL_NOOP(dp->getAttributeLocation);
    __GL_NOOP(dp->getActiveAttribute);
    __GL_NOOP_RET0(dp->getFragDataLocation);
    __GL_NOOP_RET0(dp->getUniformLocation);
    __GL_NOOP(dp->getActiveUniform);
    __GL_NOOP(dp->getActiveUniformsiv);
    __GL_NOOP(dp->getUniformIndices);
    __GL_NOOP_RET0(dp->getUniformBlockIndex);
    __GL_NOOP(dp->getActiveUniformBlockiv);
    __GL_NOOP(dp->getActiveUniformBlockName);
    __GL_NOOP(dp->uniformBlockBinding);
    __GL_NOOP_RET0(dp->setUniformData);
    __GL_NOOP_RET0(dp->getUniformData);
    __GL_NOOP(dp->buildTexEnableDim);
    __GL_NOOP_RET0(dp->getProgramResourceIndex);
    __GL_NOOP(dp->getProgramResourceName);
    __GL_NOOP(dp->getProgramResourceiv);
    __GL_NOOP_RET0(dp->validateProgramPipeline);

    /* RBO & FBO */
    __GL_NOOP(dp->bindDrawFramebuffer);
    __GL_NOOP(dp->bindReadFramebuffer);
    __GL_NOOP(dp->bindRenderbuffer);
    __GL_NOOP(dp->deleteRenderbuffer);
    __GL_NOOP(dp->detachRenderbuffer);
    __GL_NOOP_RET0(dp->renderbufferStorage);
    __GL_NOOP(dp->blitFramebufferBegin);
    __GL_NOOP(dp->blitFramebufferValidateState);
    __GL_NOOP(dp->blitFramebufferEnd);
    __GL_NOOP(dp->blitFramebuffer);
    __GL_NOOP(dp->frameBufferTexture);
    __GL_NOOP(dp->framebufferRenderbuffer);
    __GL_NOOP_RET0(dp->isFramebufferComplete);
    __GL_NOOP(dp->invalidateFramebuffer);
    __GL_NOOP(dp->invalidateDrawable);
    __GL_NOOP(dp->cleanTextureShadow);
    __GL_NOOP(dp->cleanRenderbufferShadow);

    /* Sync */
    __GL_NOOP(dp->createSync);
    __GL_NOOP(dp->deleteSync);
    __GL_NOOP_RET0(dp->waitSync);
    __GL_NOOP(dp->syncImage);

    /* Transform feedback */
    __GL_NOOP(dp->bindXFB);
    __GL_NOOP(dp->deleteXFB);
    __GL_NOOP(dp->beginXFB);
    __GL_NOOP(dp->endXFB);
    __GL_NOOP(dp->pauseXFB);
    __GL_NOOP(dp->resumeXFB);
    __GL_NOOP(dp->getXfbVarying);
    __GL_NOOP_RET0(dp->checkXFBBufSizes);


    /* GL_EXT_robustness */
    __GL_NOOP_RET0(dp->getGraphicsResetStatus);

    /* Multisample */
    __GL_NOOP(dp->getSampleLocation);

    /* Compute shader */
    __GL_NOOP_RET0(dp->computeBegin);
    __GL_NOOP_RET0(dp->computeValidateState);
    __GL_NOOP(dp->dispatchCompute);
    __GL_NOOP(dp->computeEnd);

    __GL_NOOP(dp->memoryBarrier);
}

GLvoid __glInitCurrentState(__GLcontext *gc)
{
    GLint i;

    for (i=0; i<__GL_MAX_VERTEX_ATTRIBUTES; ++i)
    {
        gc->state.current.attribute[i].f.x = 0.0;
        gc->state.current.attribute[i].f.y = 0.0;
        gc->state.current.attribute[i].f.z = 0.0;
        gc->state.current.attribute[i].f.w = 1.0;
    }
}

GLvoid __glInitHintState(__GLcontext *gc)
{
    __GLhintState *hs = &gc->state.hints;

    hs->generateMipmap = GL_DONT_CARE;
    hs->fsDerivative   = GL_DONT_CARE;
}

GLvoid __glInitRasterState(__GLcontext *gc)
{
    __GLrasterState *fs = &gc->state.raster;
    GLint i;

    fs->blendColor.r = 0.0;
    fs->blendColor.g = 0.0;
    fs->blendColor.b = 0.0;
    fs->blendColor.a = 0.0;
    fs->clearColor.clear.r = 0.0;
    fs->clearColor.clear.g = 0.0;
    fs->clearColor.clear.b = 0.0;
    fs->clearColor.clear.a = 0.0;
    fs->mrtEnable = GL_FALSE;

    for (i = 0; i < __GL_MAX_DRAW_BUFFERS; ++i)
    {
        fs->blendSrcRGB[i] = GL_ONE;
        fs->blendSrcAlpha[i] = GL_ONE;
        fs->blendDstRGB[i] = GL_ZERO;
        fs->blendDstAlpha[i] = GL_ZERO;
        fs->blendEquationRGB[i] = GL_FUNC_ADD;
        fs->blendEquationAlpha[i] = GL_FUNC_ADD;
        fs->colorMask[i].redMask = GL_TRUE;
        fs->colorMask[i].greenMask = GL_TRUE;
        fs->colorMask[i].blueMask = GL_TRUE;
        fs->colorMask[i].alphaMask = GL_TRUE;
    }

    gc->flags &= ~__GL_CONTEXT_DRAW_TO_FRONT;
    fs->drawBuffers[0] = GL_BACK;
    fs->readBuffer = GL_BACK;
}

GLvoid __glInitStencilState(__GLcontext *gc)
{
    __GLstencilState *ss = &gc->state.stencil;

    ss->front.testFunc = GL_ALWAYS;
    ss->front.fail = GL_KEEP;
    ss->front.depthFail = GL_KEEP;
    ss->front.depthPass = GL_KEEP;
    ss->front.mask = 0xff;
    ss->front.reference = 0;
    ss->front.writeMask = 0xffffffff;

    ss->back.testFunc = GL_ALWAYS;
    ss->back.fail = GL_KEEP;
    ss->back.depthFail = GL_KEEP;
    ss->back.depthPass = GL_KEEP;
    ss->back.mask = 0xff;
    ss->back.reference = 0;
    ss->back.writeMask = 0xffffffff;

    gc->state.stencil.clear = 0;
}

GLvoid __glInitDepthState(__GLcontext *gc)
{
    __GLdepthState *ds = &gc->state.depth;

    ds->writeEnable = GL_TRUE;
    ds->testFunc = GL_LESS;
    ds->clear = 1.0;
    ds->zNear = 0.0;
    ds->zFar = 1.0;
}

GLvoid __glInitLineState(__GLcontext *gc)
{
    __GLlineState *ls = &gc->state.line;

    ls->requestedWidth = 1.0;
    ls->aliasedWidth = 1;
}

GLvoid __glInitPolygonState(__GLcontext *gc)
{
    __GLpolygonState *ps = &gc->state.polygon;

    ps->cullFace  = GL_BACK;
    ps->frontFace = GL_CCW;
    ps->factor    = 0.0;
    ps->units     = 0.0;
}

GLvoid __glInitMultisampleState(__GLcontext *gc)
{
    gc->state.multisample.coverageValue = 1.0;
    gc->state.multisample.coverageInvert = GL_FALSE;
    gc->state.multisample.sampleMaskValue = 0xffffffff;
}

GLvoid __glInitEnableState(__GLcontext *gc)
{
    GLuint i;
    __GLenableState *enables = &gc->state.enables;

    for (i = 0; i < __GL_MAX_DRAW_BUFFERS; ++i)
    {
        enables->colorBuffer.blend[i] = GL_FALSE;
    }
    enables->colorBuffer.dither = GL_TRUE;
    enables->polygon.cullFace = GL_FALSE;
    enables->polygon.polygonOffsetFill = GL_FALSE;
    enables->multisample.alphaToCoverage = GL_FALSE;
    enables->multisample.coverage = GL_FALSE;
    enables->multisample.sampleMask = GL_FALSE;
    enables->scissorTest = GL_FALSE;
    enables->depthTest = GL_FALSE;
    enables->stencilTest = GL_FALSE;
    enables->primitiveRestart = GL_FALSE;
    enables->rasterizerDiscard = GL_FALSE;
}

__GL_INLINE GLvoid __glInitPrimBoundingState(__GLcontext *gc)
{
    gc->state.primBound.minX = -1.0f;
    gc->state.primBound.minY = -1.0f;
    gc->state.primBound.minZ = -1.0f;
    gc->state.primBound.minW = 1.0f;
    gc->state.primBound.maxX = 1.0f;
    gc->state.primBound.maxY = 1.0f;
    gc->state.primBound.maxZ = 1.0f;
    gc->state.primBound.maxW = 1.0f;
}


__GL_INLINE GLvoid __glCopyAttributeState(__GLattribute *dst, __GLattribute *src)
{
    __GL_MEMCOPY(dst, src, sizeof(__GLattribute));
}

GLvoid __glFormatGLModes(__GLcontextModes *modes, VEGLConfig pConfigs)
{
    __GL_MEMZERO(modes, sizeof(__GLcontextModes));
    modes->rgbMode = 1;
    modes->rgbFloatMode = 0;
    modes->doubleBufferMode = 1;
    modes->tripleBufferMode = 0;
    modes->stereoMode = 0;
    if (pConfigs)
    {
        modes->haveDepthBuffer = pConfigs->depthSize != 0;
        modes->haveStencilBuffer = pConfigs->stencilSize != 0;
        modes->sampleBuffers = pConfigs->sampleBuffers;
        modes->samples = pConfigs->samples;

        modes->redBits = pConfigs->redSize;
        modes->greenBits = pConfigs->greenSize;
        modes->blueBits = pConfigs->blueSize;
        modes->alphaBits = pConfigs->alphaSize;

        modes->rgbaBits = pConfigs->bufferSize;
        modes->depthBits = pConfigs->depthSize;
        modes->stencilBits = pConfigs->stencilSize;
    }
}

GLvoid __glDestroyDrawable(void* drawable)
{
    VEGLDrawable eglDrawable = (VEGLDrawable)drawable;

    if (eglDrawable && eglDrawable->private)
    {
        __GLdrawablePrivate *glDrawable = (__GLdrawablePrivate*)(eglDrawable->private);
        /*
        ** Here we have two assumuptions which are true on ES3.1
        ** 1. one drawable is only made current by one gc, on GL, it could be multiple gc.
        ** 2. drawable and readable are always same, on GL, it could be different.
        */
        if (glDrawable->gc)
        {
            __glSetDrawable(glDrawable->gc, NULL, NULL);
        }

        /* The drawable must be detached from any context */
        GL_ASSERT(!glDrawable->gc);

        __glDevicePipe.devDestroyDrawable(glDrawable);

        __eglFree(glDrawable);
        eglDrawable->private = gcvNULL;
    }
}

__GLdrawablePrivate* __glGetDrawable(VEGLDrawable eglDrawable)
{
    VEGLConfig eglConfig = gcvNULL;
    __GLdrawablePrivate *glDrawable = gcvNULL;

    if (!eglDrawable || !eglDrawable->config)
    {
        return gcvNULL;
    }

    if (eglDrawable->private)
    {
        glDrawable = (__GLdrawablePrivate*)(eglDrawable->private);
    }
    else
    {
        glDrawable = (__GLdrawablePrivate*)__eglMalloc(sizeof(__GLdrawablePrivate));
        if (glDrawable != gcvNULL)
        {
            glDrawable->gc = gcvNULL;
            glDrawable->privateData = gcvNULL;

            eglDrawable->private = glDrawable;
            eglDrawable->destroyPrivate = __glDestroyDrawable;
        }
        else
        {
            return gcvNULL;
        }
    }

    eglConfig = (VEGLConfig)eglDrawable->config;

    /* If the drawable was current but resized, need to detach old surface before updating new ones. */
    if (glDrawable->gc &&
        (glDrawable->rtHandle      != eglDrawable->rtHandle    ||
         glDrawable->depthHandle   != eglDrawable->depthHandle ||
         glDrawable->stencilHandle != eglDrawable->stencilHandle
        )
       )
    {
        (*glDrawable->gc->dp.detachDrawable)(glDrawable->gc);
    }

    /* Always update glDrawable info when EGL set drawable */
    __glFormatGLModes(&glDrawable->modes, eglConfig);
    glDrawable->width= eglDrawable->width;
    glDrawable->height = eglDrawable->height;

    /* Get the rt format Info, see EGL::veglGetFormat() */
    switch (eglConfig->greenSize)
    {
    case 4:
        glDrawable->rtFormatInfo = &__glFormatInfoTable[__GL_FMT_RGBA4];
        break;
    case 5:
        glDrawable->rtFormatInfo = &__glFormatInfoTable[__GL_FMT_RGB5_A1];
        break;
    case 6:
        glDrawable->rtFormatInfo = &__glFormatInfoTable[__GL_FMT_RGB565];
        break;
    case 8:
        glDrawable->rtFormatInfo = eglConfig->alphaSize
                                 ? &__glFormatInfoTable[__GL_FMT_RGBA8]
                                 : &__glFormatInfoTable[__GL_FMT_RGB8];
        break;
    default:
        GL_ASSERT(0);
        glDrawable->rtFormatInfo = gcvNULL;
    }

    glDrawable->rtHandle = eglDrawable->rtHandle;
    glDrawable->prevRtHandle = eglDrawable->prevRtHandle;

    /* Get the depth stencil format Info, see EGL::veglGetFormat() */
    if (eglDrawable->depthHandle)
    {
        switch (eglConfig->depthSize)
        {
        case 16:
            glDrawable->dsFormatInfo = &__glFormatInfoTable[__GL_FMT_Z16];
            break;
        case 24:
            glDrawable->dsFormatInfo = eglConfig->stencilSize
                                     ? &__glFormatInfoTable[__GL_FMT_Z24S8]
                                     : &__glFormatInfoTable[__GL_FMT_Z24];
            break;
        default:
            GL_ASSERT(0);
            glDrawable->dsFormatInfo = gcvNULL;
            break;
        }
    }
    else
    {
        if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_S8_ONLY_RENDERING) == gcvSTATUS_TRUE)
        {
            glDrawable->dsFormatInfo = eglConfig->stencilSize ? &__glFormatInfoTable[__GL_FMT_S8] : gcvNULL;
        }
        else
        {
            glDrawable->dsFormatInfo = eglConfig->stencilSize ? &__glFormatInfoTable[__GL_FMT_Z24S8] : gcvNULL;
        }
    }

    glDrawable->depthHandle = eglDrawable->depthHandle;
    glDrawable->stencilHandle = eglDrawable->stencilHandle;

    if (0 == (glDrawable->width * glDrawable->height))
    {
        glDrawable->flags |= __GL_DRAWABLE_FLAG_ZERO_WH;
    }
    else
    {
        glDrawable->flags &= ~__GL_DRAWABLE_FLAG_ZERO_WH;
    }

    __glDevicePipe.devUpdateDrawable(glDrawable, glDrawable->rtHandle, glDrawable->depthHandle, glDrawable->stencilHandle);

    return glDrawable;
}

GLvoid __glSetDrawable(__GLcontext* gc, __GLdrawablePrivate* drawable, __GLdrawablePrivate* readable)
{
    if (gc->drawablePrivate != drawable)
    {
        if (gc->drawablePrivate)
        {
            /* Detach old drawable chip objects from this context before set new drawable,
            ** If rt was only resized, drawable was detach when update the drawable
            */
            (*gc->dp.detachDrawable)(gc);
            gc->drawablePrivate->gc = gcvNULL;
        }

        if (drawable)
        {
            drawable->gc = gc;
            gc->modes = drawable->modes;
        }
        else
        {
            __GL_MEMZERO(&gc->modes, sizeof(__GLcontextModes));
        }

        gc->drawablePrivate = drawable;
    }

    if (gc->readablePrivate != readable)
    {
        if (gc->readablePrivate)
        {
            gc->readablePrivate->gc = gcvNULL;
        }

        if (readable)
        {
            readable->gc = gc;
        }

        gc->readablePrivate = readable;
    }

    gc->drawableDirtyMask |= __GL_BUFFER_DRAW_READ_BITS;
}

/* Dispatch window system render buffer changes */
GLvoid __glEvaluateSystemDrawableChange(__GLcontext *gc, GLbitfield flags)
{
    GLboolean complete = GL_TRUE;

    if (flags & __GL_BUFFER_DRAW_BIT)
    {
        __GLdrawablePrivate *drawable = gc->drawablePrivate;
        if (!drawable || (drawable->flags & __GL_DRAWABLE_FLAG_ZERO_WH))
        {
            complete = GL_FALSE;
        }
    }

    if (flags & __GL_BUFFER_READ_BIT)
    {
        __GLdrawablePrivate *readable = gc->readablePrivate;
        if (!readable || (readable->flags & __GL_DRAWABLE_FLAG_ZERO_WH))
        {
            complete = GL_FALSE;
        }
    }

    if (complete)
    {
        gc->flags &= ~__GL_CONTEXT_SKIP_DRAW_INVALID_RENDERBUFFER;
    }
    else
    {
        gc->flags |= __GL_CONTEXT_SKIP_DRAW_INVALID_RENDERBUFFER;
    }
}

GLvoid __glInitContextState(__GLcontext *gc)
{
    gc->flags = __GL_CONTEXT_UNINITIALIZED;
    gc->invalidCommonCommit = gcvTRUE;
    gc->invalidDrawCommit = gcvTRUE;

    __glInitCurrentState(gc);
    __glInitHintState(gc);
    __glInitRasterState(gc);
    __glInitStencilState(gc);
    __glInitDepthState(gc);
    __glInitLineState(gc);
    __glInitPolygonState(gc);
    __glInitPixelState(gc);
    __glInitMultisampleState(gc);

    __glInitVertexArrayState(gc);
    __glInitFramebufferStates(gc);
    __glInitTextureState(gc);
    __glInitBufferObjectState(gc);
    __glInitShaderProgramState(gc);
    __glInitSamplerState(gc);
    __glInitXfbState(gc);
    __glInitQueryState(gc);
    __glInitSyncState(gc);

    __glInitEnableState(gc);
    __glInitImageState(gc);
    __glInitDebugState(gc);
    __glInitPrimBoundingState(gc);

    __glBitmaskInitAllOne(&gc->texUnitAttrDirtyMask, gc->constants.shaderCaps.maxCombinedTextureImageUnits);

    __glBitmaskInitAllOne(&gc->imageUnitDirtyMask, gc->constants.shaderCaps.maxImageUnit);

    __glSetAttributeStatesDirty(gc);
}

GLvoid __glOverturnCommitStates(__GLcontext *gc)
{
    GLuint i;
    __GLattribute *pState = &gc->state;
    __GLattribute *pCommit = &gc->commitState;
    GLubyte *puState = (GLubyte*)pState;
    GLubyte *puCommit = (GLubyte*)pCommit;

    for (i = 0; i < sizeof(__GLattribute); ++i)
    {
        GLubyte complement = ~puState[i];

        /* Make commitState different than current ones, so that coming state
        ** evaluations can keep dirty and send all the current states down.
        ** For float states, the complement might generate NAN/INI. But any valid value compared
        ** to NAN/INI should report "not equal".
        */
        while (complement == puState[i])
        {
            complement++;
        }
        puCommit[i] = complement;
    }

    /* For boolean states, the previous algorithm might not make them different.
    ** They need be NOT of current states .
    */
    for (i = 0; i < __GL_MAX_DRAW_BUFFERS; ++i)
    {
        pCommit->raster.colorMask[i].redMask   = !pState->raster.colorMask[i].redMask;
        pCommit->raster.colorMask[i].greenMask = !pState->raster.colorMask[i].greenMask;
        pCommit->raster.colorMask[i].blueMask  = !pState->raster.colorMask[i].blueMask;
        pCommit->raster.colorMask[i].alphaMask = !pState->raster.colorMask[i].alphaMask;
        pCommit->enables.colorBuffer.blend[i]  = !pState->enables.colorBuffer.blend[i];
    }
    for (i = 0; i < __GL_MAX_IMAGE_UNITS; ++i)
    {
        pCommit->image.imageUnit[i].invalid = !pState->image.imageUnit[i].invalid;
        pCommit->image.imageUnit[i].layered = !pState->image.imageUnit[i].layered;
        pCommit->image.imageUnit[i].singleLayered = !pState->image.imageUnit[i].singleLayered;
    }
    pCommit->raster.mrtEnable = !pState->raster.mrtEnable;
    pCommit->depth.writeEnable = !pState->depth.writeEnable;
    pCommit->enables.scissorTest = !pState->enables.scissorTest;
    pCommit->enables.depthTest = !pState->enables.depthTest;
    pCommit->enables.stencilTest = !pState->enables.stencilTest;
    pCommit->enables.primitiveRestart = !pState->enables.primitiveRestart;
    pCommit->enables.rasterizerDiscard = !pState->enables.rasterizerDiscard;
    pCommit->enables.colorBuffer.dither = !pState->enables.colorBuffer.dither;
    pCommit->enables.polygon.cullFace = !pState->enables.polygon.cullFace;
    pCommit->enables.polygon.polygonOffsetFill = !pState->enables.polygon.polygonOffsetFill;
    pCommit->enables.multisample.alphaToCoverage = !pState->enables.multisample.alphaToCoverage;
    pCommit->enables.multisample.coverage = !pState->enables.multisample.coverage;
    pCommit->enables.multisample.sampleMask = !pState->enables.multisample.sampleMask;
}

#if gcdPATTERN_FAST_PATH
GLvoid __glInitPattern(__GLcontext *gc)
{
    gc->pattern.enable = gcvFALSE;
    gc->pattern.patternMatchMask = (1 << GLES_PATTERN_GFX0) | (1 << GLES_PATTERN_GFX1);
    gc->pattern.matchCount = 0;
    gc->pattern.apiCount = 0;
    gc->pattern.patterns[0] = &gfxPattern0;
    gc->pattern.patterns[1] = &gfxPattern1;
    gc->pattern.state = GLES_PATTERN_STATE_CHECK;
    gc->pattern.matchPattern = gcvNULL;
}
#endif

GLvoid __glInitGlobals(__GLApiVersion apiVersion)
{
    gctSTRING tracemode = gcvNULL;
#if VIVANTE_PROFILER
    gctSTRING profilemode = gcvNULL;
#endif

    /* Initialize dpGlobalInfo and device entry functions */
    if (__glDpInitialize(&__glDevicePipe) == GL_FALSE)
    {
        return;
    }

    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_TRACE", &tracemode)) && tracemode)
    {
        if (gcmIS_SUCCESS(gcoOS_StrCmp(tracemode, "0")))
        {
            __glesApiTraceMode = gcvTRACEMODE_NONE;
        }
        else if (gcmIS_SUCCESS(gcoOS_StrCmp(tracemode, "1")))
        {
            __glesApiTraceMode = gcvTRACEMODE_FULL;
        }
        else if (gcmIS_SUCCESS(gcoOS_StrCmp(tracemode, "2")))
        {
            __glesApiTraceMode = gcvTRACEMODE_LOGGER;
        }
        else if (gcmIS_SUCCESS(gcoOS_StrCmp(tracemode, "3")))
        {
            __glesApiTraceMode = gcvTRACEMODE_PRE;
        }
        else if (gcmIS_SUCCESS(gcoOS_StrCmp(tracemode, "4")))
        {
            __glesApiTraceMode = gcvTRACEMODE_POST;
        }
        else
        {
            gcmPRINT("ES: Unsupported trace mode");
        }

        if (!__glInitTracerDispatchTable(__glesApiTraceMode, apiVersion))
        {
            /* Reset to regular API entry functions if tracer dispatch table initialization failed */
            __glesApiTraceMode = gcvTRACEMODE_NONE;
        }
    }
#if VIVANTE_PROFILER
    __glesApiProfileMode = -1;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_PROFILE", &profilemode)) && profilemode)
    {
        if (gcmIS_SUCCESS(gcoOS_StrCmp(profilemode, "0")))
        {
            __glesApiProfileMode = 0;
        }
        else if (gcmIS_SUCCESS(gcoOS_StrCmp(profilemode, "1")))
        {
            __glesApiProfileMode = 1;
        }
        else if (gcmIS_SUCCESS(gcoOS_StrCmp(profilemode, "2")))
        {
            __glesApiProfileMode = 2;
        }
        else if (gcmIS_SUCCESS(gcoOS_StrCmp(profilemode, "3")))
        {
            __glesApiProfileMode = 3;
        }
    }
#endif
}

GLboolean __glLoseCurrent(__GLcontext *gc, __GLdrawablePrivate* drawable, __GLdrawablePrivate* readable)
{
    __glSetDrawable(gc, drawable, readable);

    /* Notify DP the context is not current anymore */
    return (*gc->dp.loseCurrent)(gc, GL_FALSE);
}

GLboolean __glMakeCurrent(__GLcontext *gc, __GLdrawablePrivate* drawable, __GLdrawablePrivate* readable, GLboolean flushDrawableChange)
{
    __GLframebufferObject *defaultDrawFBO = &gc->frameBuffer.defaultDrawFBO;
    __GLframebufferObject *defaultReadFBO = &gc->frameBuffer.defaultReadFBO;

    __glSetDrawable(gc, drawable, readable);

    if (gc->flags & __GL_CONTEXT_UNINITIALIZED)
    {
        GLsizei width, height;

        if (gc->drawablePrivate)
        {
            width = gc->drawablePrivate->width;
            height = gc->drawablePrivate->height;
        }
        else
        {
            /* GL OES surfaceless context. */
            width  = 0;
            height = 0;
        }

        /* Initialize the viewport and scissor to the whole window */
        __glUpdateViewport(gc, 0, 0, width, height);
        __glUpdateScissor(gc, 0, 0, width, height);

        gc->flags &= ~(__GL_CONTEXT_UNINITIALIZED);
    }

    if (drawable)
    {
        defaultDrawFBO->flag = (__GL_FRAMEBUFFER_IS_CHECKED | __GL_FRAMEBUFFER_IS_COMPLETE);
        defaultDrawFBO->checkCode = GL_FRAMEBUFFER_COMPLETE;
        defaultDrawFBO->fbSamples = drawable->modes.samples;
    }
    else
    {
        defaultDrawFBO->flag = __GL_FRAMEBUFFER_IS_CHECKED;
        defaultDrawFBO->checkCode = GL_FRAMEBUFFER_UNDEFINED;
        defaultDrawFBO->fbSamples = 0;
    }

    if (readable)
    {
        defaultReadFBO->flag = (__GL_FRAMEBUFFER_IS_CHECKED | __GL_FRAMEBUFFER_IS_COMPLETE);
        defaultReadFBO->checkCode = GL_FRAMEBUFFER_COMPLETE;
        defaultReadFBO->fbSamples = readable->modes.samples;
    }
    else
    {
        defaultReadFBO->flag = __GL_FRAMEBUFFER_IS_CHECKED;
        defaultReadFBO->checkCode = GL_FRAMEBUFFER_UNDEFINED;
        defaultReadFBO->fbSamples = 0;
    }

    /*
    ** Some internal switch need flush drawable immediately to restore the chip layer drawbuffer.
    ** Especially for 3Dblit switch, otherwise the only chance to evaluate drawable change is at
    ** core layer, which wouldn't help when switch happened in chip layer.
    */
    if (flushDrawableChange)
    {
        __glEvaluateDrawableChange(gc, __GL_BUFFER_DRAW_READ_BITS);
    }

    /* Notify the DP of the new context drawable pair */
    return (*gc->dp.makeCurrent)(gc);
}

GLboolean __glDestroyContext(GLvoid *context)
{
    __GLcontext *gc = (__GLcontext*)context;
    GLboolean retVal = GL_TRUE;

    __glFreeFramebufferStates(gc);
    __glFreeTextureState(gc);
    __glFreeVertexArrayState(gc);
    __glFreeBufferObjectState(gc);
    __glFreeShaderProgramState(gc);
    __glFreeSamplerState(gc);
    __glFreeXfbState(gc);
    __glFreeQueryState(gc);
    __glFreeSyncState(gc);
    __glFreeDebugState(gc);

    /* Notify DP to destroy its private data */
    retVal = (*gc->dp.destroyPrivateData)(gc);
    if (!retVal)
    {
        return retVal;
    }

    (*gc->imports.free)(gc, gc);

    return retVal;
}

GLvoid *__glCreateContext(GLint clientVersion, VEGLimports *imports, GLvoid* sharedCtx)
{
    __GLcontext *gc = gcvNULL;
    __GLApiVersion apiVersion;
    __GLdeviceStruct *__glDevice = &__glDevicePipe;
    static GLboolean initialized = GL_FALSE;

    __GL_HEADER();

    /* Record the context version: sometime es20 and es30 may have different rule
    */
    switch (clientVersion)
    {
    case 0x20:
        apiVersion = __GL_API_VERSION_ES20;
        break;
    case 0x30:
        apiVersion = __GL_API_VERSION_ES30;
        break;
    case 0x31:
        apiVersion = __GL_API_VERSION_ES31;
        break;
    case 0x32:
        apiVersion = __GL_API_VERSION_ES32;
        break;
    default:
        GL_ASSERT(0);
        __GL_ERROR_EXIT2();
    }

    if (!initialized)
    {
        __glInitGlobals(apiVersion);
        initialized = GL_TRUE;
    }
    else
    {
#if VIVANTE_PROFILER
        /* Need to re-init profiler mode
        */
        gctSTRING profilemode = gcvNULL;
        __glesApiProfileMode = -1;
        if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_PROFILE", &profilemode)) && profilemode)
        {
            if (gcmIS_SUCCESS(gcoOS_StrCmp(profilemode, "0")))
            {
                __glesApiProfileMode = 0;
            }
            else if (gcmIS_SUCCESS(gcoOS_StrCmp(profilemode, "1")))
            {
                __glesApiProfileMode = 1;
            }
            else if (gcmIS_SUCCESS(gcoOS_StrCmp(profilemode, "2")))
            {
                __glesApiProfileMode = 2;
            }
            else if (gcmIS_SUCCESS(gcoOS_StrCmp(profilemode, "3")))
            {
                __glesApiProfileMode = 3;
            }
        }
#endif
    }

    /* Allocate memory for core GL context.
    */
    gc = (__GLcontext*)(*imports->calloc)(gc, 1, sizeof(__GLcontext));
    if (!gc)
    {
        __GL_ERROR_EXIT2();
    }

    gc->imports = *imports;

    /* Convert EGL Reset Notification enums to corresponding GL enums.
    */
    switch (gc->imports.resetNotification)
    {
    case EGL_NO_RESET_NOTIFICATION_EXT:
        gc->imports.resetNotification = GL_NO_RESET_NOTIFICATION_EXT;
        break;
    case EGL_LOSE_CONTEXT_ON_RESET_EXT:
        gc->imports.resetNotification = GL_LOSE_CONTEXT_ON_RESET_EXT;
        break;
    }

    __glFormatGLModes(&gc->modes, (VEGLConfig)imports->config);


    gc->apiVersion   = apiVersion;
    gc->shareCtx     = (__GLcontext*)sharedCtx;
    gc->contextFlags = 0;

    if (imports->contextFlags & EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR)
    {
        gc->contextFlags |= GL_CONTEXT_FLAG_DEBUG_BIT_KHR;
    }

    /* Robust buffer access is enabled by creating a context with robust access enabled 
     * through the window system binding APIs.
     * Robust buffer access behavior may be queried by calling  GetIntegerv with pname CONTEXT_FLAGS.
     */
    if ((imports->contextFlags & EGL_CONTEXT_OPENGL_ROBUST_ACCESS_BIT_KHR) ||
        (imports->robustAccess == gcvTRUE))
    {
        gc->contextFlags |= GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT;
    }

    if (imports->protectedContext)
    {
        gc->contextFlags |= GL_CONTEXT_FLAG_PROTECTED_CONTENT_BIT_EXT;
    }

    /* Initialize the device constants with GL defaults.
    */
    __glInitConstantDefault(&gc->constants);

    /* Call device specific interface to over-write the default constant
    */
    (*__glDevice->devGetConstants)(gc, &gc->constants);

    /* Initialize the core GL context states.
    */
    __glInitContextState(gc);

    /* Initialize DP context function pointers with default Noop functions.
    */
    __glInitDevPipeDefault(&gc->dp);

    /* Create the device specific context and attach it to gc->dp.privateData.
    ** And initialize the device pipeline "gc->dp" function pointers.
    */

    if((*__glDevice->devCreateContext)(gc) == GL_FALSE)
    {
        (*imports->free)(gc, gc);
        gc = gcvNULL;
        __GL_ERROR_EXIT2();
    }

#if gcdPATTERN_FAST_PATH
    /* Init Pattern
    */
    __glInitPattern(gc);
#endif

    /* Initialize API dispatch tables. */
#if VIVANTE_PROFILER
    if (__glesApiTraceMode == gcvTRACEMODE_NONE && __glesApiProfileMode < 1)
        gc->apiDispatchTable = __glesApiFuncDispatchTable;
    else
        gc->apiDispatchTable = __glesApiProfileDispatchTable;
#else
    gc->apiDispatchTable = (__glesApiTraceMode != gcvTRACEMODE_NONE) ?
                                __glesApiProfileDispatchTable : __glesApiFuncDispatchTable;
#endif

    gc->magic = ES3X_MAGIC;


    /* Do not need destructor. */
    gc->base.destructor = gcvNULL;


    __GL_FOOTER();
    return (gc);

OnError:
    if (gc)
    {
        (*imports->free)(gc, gc);
        gc = gcvNULL;
    }
    __GL_FOOTER();
    return gcvNULL;
}

GLvoid __glSetError(__GLcontext *gc, GLenum code)
{
    if (!gc->error)
    {
        gc->error = code;
    }

    if (gc->debug.dbgOut)
    {
        __glDebugPrintLogMessage(gc, GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, 0,
                                 GL_DEBUG_SEVERITY_HIGH, "GL error 0x%x was generated", code);
    }
}

