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
#include "gc_es_device.h"
#ifdef OPENGL40
#include "gc_es_attrib.h"
#include "viv_lock.h"
#endif


#define _GC_OBJ_ZONE gcdZONE_GL40_CORE
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
extern __GLthreadPriv* __eglGetThreadEsPrivData(void* thrData);

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
extern __GLdispatchTable __glProfileFuncTable;
extern GLvoid __glSetDrawable(__GLcontext* gc, __GLdrawablePrivate* drawable, __GLdrawablePrivate* readable);
#ifdef OPENGL40
extern GLuint __glInitVertexInputState(__GLcontext *gc);
extern GLvoid __glFreeVertexInputState(__GLcontext *gc);
extern GLvoid __glFreeTransformState(__GLcontext *gc);
extern GLvoid __glFreeEvaluatorState(__GLcontext *gc);
extern GLvoid __glFreeDlistState(__GLcontext *gc);
extern GLvoid __glFreeAttribStackState(__GLcontext *gc);
extern GLvoid __glInitEvaluatorState(__GLcontext *gc);
extern GLfloat __glClampWidth(GLfloat width, __GLdeviceConstants *constants);
extern GLvoid __glComputeClipBox(__GLcontext *gc);
extern GLvoid __glOverWriteListCompileTable();
#endif

extern __GLformatInfo __glFormatInfoTable[];

gceTRACEMODE __glApiTraceMode = gcvTRACEMODE_NONE;


#define __GL_NOOP(fp)      *(GLvoid (**)())&(fp) = __glNoop;
#define __GL_NOOP_RET0(fp) *(GLuint (**)())&(fp) = __glNoop_Return0;
#define __GL_NOOP_RET1(fp) *(GLuint (**)())&(fp) = __glNoop_Return1;

#if gcdPATTERN_FAST_PATH
__GLapiPattern gfxPattern0 = {
    {
        { __glApiEnum(Uniform2f) , {0, 0, 0, 0}},
        { __glApiEnum(Uniform2f) , {0, 0, 0, 0}},
        { __glApiEnum(Uniform1f) , {0, 0, 0, 0}},
        { __glApiEnum(Uniform2f) , {0, 0, 0, 0}},
        { __glApiEnum(Uniform2f) , {0, 0, 0, 0}},
        { __glApiEnum(Uniform4f) , {0, 0, 0, 0}},
        { __glApiEnum(Uniform4f) , {0, 0, 0, 0}},
        { __glApiEnum(Uniform4f) , {0, 0, 0, 0}},
        { __glApiEnum(Uniform4f) , {0, 0, 0, 0}},
        { __glApiEnum(BlendColor) , {0, 0, 0, 0}},
        { __glApiEnum(Disable) , {0x0B71, 0, 0, 0}},
        { __glApiEnum(DepthFunc) , {0, 0, 0, 0}},
        { __glApiEnum(BlendFunc) , {0, 0, 0, 0}},
        { __glApiEnum(Enable) , {0x0BE2, 0, 0, 0}},
        { __glApiEnum(BindBuffer) , {0, 0, 0, 0}},
        { __glApiEnum(BindBuffer) , {0, 0, 0, 0}},
        { __glApiEnum(VertexAttribPointer) , {0x1406, 0, 0, 0}},
        { __glApiEnum(VertexAttribPointer) , {0x1406, 0, 0, 0}},
        { __glApiEnum(DrawElements) , {0x4, 0x1403, 0, 0}},
    },
    19
};

__GLapiPattern gfxPattern1 = {
    {
        { __glApiEnum(Uniform2f) , {0, 0, 0, 0}},
        { __glApiEnum(Uniform2f) , {0, 0, 0, 0}},
        { __glApiEnum(Uniform1f) , {0, 0, 0, 0}},
        { __glApiEnum(Uniform2f) , {0, 0, 0, 0}},
        { __glApiEnum(Uniform2f) , {0, 0, 0, 0}},
        { __glApiEnum(Uniform4f) , {0, 0, 0, 0}},
        { __glApiEnum(Uniform4f) , {0, 0, 0, 0}},
        { __glApiEnum(Uniform4f) , {0, 0, 0, 0}},
        { __glApiEnum(Uniform4f) , {0, 0, 0, 0}},
        { __glApiEnum(BlendColor) , {0, 0, 0, 0}},
        { __glApiEnum(Enable) , {0x0B71, 0, 0, 0}},
        { __glApiEnum(DepthFunc) , {0, 0, 0, 0}},
        { __glApiEnum(BlendFunc) , {0, 0, 0, 0}},
        { __glApiEnum(Enable) , {0x0BE2, 0, 0, 0}},
        { __glApiEnum(BindBuffer) , {0, 0, 0, 0}},
        { __glApiEnum(BindBuffer) , {0, 0, 0, 0}},
        { __glApiEnum(VertexAttribPointer) , {0x1406, 0, 0, 0}},
        { __glApiEnum(VertexAttribPointer) , {0x1406, 0, 0, 0}},
        { __glApiEnum(DrawElements) , {0x4, 0x1403, 0, 0}},
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

#ifdef OPENGL40
    constants->numberOfClipPlanes = 8;
    constants->maxModelViewStackDepth = 32;
    constants->maxProjectionStackDepth = 16;
    constants->maxTextureStackDepth = 16;
    constants->maxProgramStackDepth = 16;
    constants->maxAttribStackDepth = 16;
    constants->maxClientAttribStackDepth = 16;
    constants->maxNameStackDepth = 128;
    constants->maxListNesting = 64;
    constants->maxDrawBuffers = 1;
    constants->maxEvalOrder = 30;
    constants->pointSizeGranularity = 0.125;
#endif
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

#ifdef OPENGL40
    __GL_NOOP(dp->bindFragDataLocation);
    __GL_NOOP(dp->createAccumBufferInfo);
#endif
}

GLvoid __glInitCurrentState(__GLcontext *gc)
{
    GLint i;

#ifdef OPENGL40
    if (gc->imports.conformGLSpec)
    {
        gc->state.current.color.r = 1.0;
        gc->state.current.color.g = 1.0;
        gc->state.current.color.b = 1.0;
        gc->state.current.color.a = 1.0;
        gc->state.current.colorIndex = 1.0;

        gc->state.current.color2.r = 0.0;
        gc->state.current.color2.g = 0.0;
        gc->state.current.color2.b = 0.0;
        gc->state.current.color2.a = 1.0;
    }
    else  /* Running OES api */
#endif
    {
        for (i=0; i<__GL_MAX_VERTEX_ATTRIBUTES; ++i)
        {
            gc->state.current.attribute[i].f.x = 0.0;
            gc->state.current.attribute[i].f.y = 0.0;
            gc->state.current.attribute[i].f.z = 0.0;
            gc->state.current.attribute[i].f.w = 1.0;
        }
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
    GLint i;
    __GLrasterState *fs = &gc->state.raster;

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

#ifdef OPENGL40
    if (gc->imports.conformGLSpec && !gc->modes.doubleBufferMode)
    {
        gc->flags |= __GL_CONTEXT_DRAW_TO_FRONT;
        fs->drawBuffers[0] = GL_FRONT;
        fs->readBuffer = GL_FRONT;
    }
    else  /* Running OES api */
#endif
    {
        gc->flags &= ~__GL_CONTEXT_DRAW_TO_FRONT;
        fs->drawBuffers[0] = GL_BACK;
        fs->readBuffer = GL_BACK;
    }

#ifdef OPENGL40
    if (gc->imports.conformGLSpec)
    {
        fs->alphaFunction = GL_ALWAYS;
        fs->alphaReference = 0;

        gc->state.rasterPos.rPos.winPos.f.x = 0.0;
        gc->state.rasterPos.rPos.winPos.f.y = 0.0;
        gc->state.rasterPos.rPos.winPos.f.z = 0.0;

        /* Initialize primary color */
        gc->state.rasterPos.rPos.color[__GL_PRIMARY_COLOR]
                = &gc->state.rasterPos.rPos.colors[__GL_FRONTFACE];
        gc->state.rasterPos.rPos.colors[__GL_FRONTFACE].r = 1.0;
        gc->state.rasterPos.rPos.colors[__GL_FRONTFACE].g = 1.0;
        gc->state.rasterPos.rPos.colors[__GL_FRONTFACE].b = 1.0;
        gc->state.rasterPos.rPos.colors[__GL_FRONTFACE].a = 1.0;
        gc->state.rasterPos.rPos.colors[__GL_BACKFACE].r = 1.0;
        gc->state.rasterPos.rPos.colors[__GL_BACKFACE].g = 1.0;
        gc->state.rasterPos.rPos.colors[__GL_BACKFACE].b = 1.0;
        gc->state.rasterPos.rPos.colors[__GL_BACKFACE].a = 1.0;

        /* Initialize secondary color */
        gc->state.rasterPos.rPos.color[__GL_SECONDARY_COLOR]
                = &gc->state.rasterPos.rPos.colors2[__GL_FRONTFACE];
        gc->state.rasterPos.rPos.colors2[__GL_FRONTFACE].r = 0.0;
        gc->state.rasterPos.rPos.colors2[__GL_FRONTFACE].g = 0.0;
        gc->state.rasterPos.rPos.colors2[__GL_FRONTFACE].b = 0.0;
        gc->state.rasterPos.rPos.colors2[__GL_FRONTFACE].a = 1.0;
        gc->state.rasterPos.rPos.colors2[__GL_BACKFACE].r = 0.0;
        gc->state.rasterPos.rPos.colors2[__GL_BACKFACE].g = 0.0;
        gc->state.rasterPos.rPos.colors2[__GL_BACKFACE].b = 0.0;
        gc->state.rasterPos.rPos.colors2[__GL_BACKFACE].a = 1.0;

        for (i = 0; i < __GL_MAX_TEXTURE_COORDS; i++)
        {
            gc->state.rasterPos.rPos.texture[i].fTex.s = 0.0;
            gc->state.rasterPos.rPos.texture[i].fTex.t = 0.0;
            gc->state.rasterPos.rPos.texture[i].fTex.r = 0.0;
            gc->state.rasterPos.rPos.texture[i].fTex.q = 1.0;
        }
        gc->state.rasterPos.rPos.clipW = 1.0;
        gc->state.rasterPos.validRasterPos = GL_TRUE;
        gc->state.rasterPos.rPos.eyeDistance = 1.0;
        fs->clampFragColor = GL_FIXED_ONLY;
        fs->clampReadColor = GL_FIXED_ONLY;
    }
#endif
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
#ifdef OPENGL40
    if (gc->imports.conformGLSpec)
    {
        ls->smoothWidth = __glClampWidth(1.0, &gc->constants);
        ls->stipple = 0xFFFF;
        ls->stippleRepeat = 1;
        gc->state.enables.line.stippleRequested = GL_FALSE;
    }
#endif
}

GLvoid __glInitPolygonState(__GLcontext *gc)
{
    __GLpolygonState *ps = &gc->state.polygon;

    ps->cullFace  = GL_BACK;
    ps->frontFace = GL_CCW;
    ps->factor    = 0.0;
    ps->units     = 0.0;
#ifdef OPENGL40
    if (gc->imports.conformGLSpec)
    {
        ps->frontMode = GL_FILL;
        ps->backMode = GL_FILL;
        ps->bothFaceFill = GL_TRUE;
        gc->state.current.edgeflag = GL_TRUE;
    }
#endif
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

__GL_INLINE GLvoid __glInitPrimRestartState(__GLcontext *gc)
{
    gc->state.primRestart.restartElement = 0xFFFFFFFF;
}


__GL_INLINE GLvoid __glCopyAttributeState(__GLattribute *dst, __GLattribute *src)
{
    __GL_MEMCOPY(dst, src, sizeof(__GLattribute));
}

#ifdef OPENGL40
GLvoid __glInitPointState(__GLcontext *gc)
{
    __GLpointState *ps = &gc->state.point;

    ps->requestedSize = 1.0;
    ps->sizeMin = 0.0;
    ps->sizeMax = 100.0;
    ps->fadeThresholdSize = 1.0;
    ps->distanceAttenuation[0] = 1.0;
    ps->distanceAttenuation[1] = 0.0;
    ps->distanceAttenuation[2] = 0.0;
    ps->coordOrigin = GL_UPPER_LEFT;
    gc->state.enables.pointSmooth = GL_FALSE;
}
#endif

GLvoid __glFormatGLModes(__GLcontextModes *tmodes, __GLcontextModes *smodes)
{
    __GL_MEMZERO(tmodes, sizeof(__GLcontextModes));
    tmodes->rgbMode = 1;
    tmodes->rgbFloatMode = 0;
    tmodes->doubleBufferMode = smodes->doubleBufferMode;
    tmodes->tripleBufferMode = 0;
    tmodes->stereoMode = 0;

    tmodes->haveDepthBuffer = smodes->depthBits != 0;
    tmodes->haveStencilBuffer = smodes->stencilBits != 0;
    tmodes->sampleBuffers = smodes->sampleBuffers;
    tmodes->samples = smodes->samples;

    tmodes->redBits = smodes->redBits;
    tmodes->greenBits = smodes->greenBits;
    tmodes->blueBits = smodes->blueBits;
    tmodes->alphaBits = smodes->alphaBits;

    tmodes->rgbaBits = smodes->rgbaBits;
    tmodes->depthBits = smodes->depthBits;
    tmodes->stencilBits = smodes->stencilBits;

    tmodes->accumBits = smodes->accumBits;
    tmodes->accumRedBits = smodes->accumRedBits;
    tmodes->accumGreenBits = smodes->accumGreenBits;
    tmodes->accumBlueBits = smodes->accumBlueBits;
    tmodes->accumAlphaBits = smodes->accumAlphaBits;
    tmodes->haveAccumBuffer = (smodes->accumRedBits +
                               smodes->accumGreenBits +
                               smodes->accumBlueBits +
                               smodes->accumAlphaBits) > 0;
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
    __GLcontextModes *pmode = gcvNULL;

    GLuint i;
    __GLdrawablePrivate *glDrawable = gcvNULL;
//    gcoSURF accumSurf;

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
            glDrawable->flags = 0;
#ifdef OPENGL40
            glDrawable->pbufferTex = gcvNULL;
#endif

            eglDrawable->private = glDrawable;
            eglDrawable->destroyPrivate = __glDestroyDrawable;
        }
        else
        {
            return gcvNULL;
        }
    }

    pmode = (__GLcontextModes *)eglDrawable->config;

#ifdef OPENGL40
        /* TODO: EGL not set doubleBufferMode for now, need to add in the future */
        pmode->doubleBufferMode = GL_FALSE;
#else
    pmode->doubleBufferMode = GL_TRUE;
#endif

    /* If the drawable was current but resized, need to detach old surface before updating new ones. */
    if (glDrawable->gc &&
        (glDrawable->rtHandles[0]  != eglDrawable->rtHandles[0] ||
         glDrawable->depthHandle   != eglDrawable->depthHandle ||
         glDrawable->stencilHandle != eglDrawable->stencilHandle)
       )
    {
        (*glDrawable->gc->dp.detachDrawable)(glDrawable->gc);
    }

    /* Always update glDrawable info when EGL set drawable */
    __glFormatGLModes(&glDrawable->modes, pmode);

    glDrawable->width= eglDrawable->width;
    glDrawable->height = eglDrawable->height;

    switch (pmode->greenBits)
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
        glDrawable->rtFormatInfo = pmode->alphaBits
                                 ? &__glFormatInfoTable[__GL_FMT_RGBA8]
                                 : &__glFormatInfoTable[__GL_FMT_RGB8];
        break;
    default:
        GL_ASSERT(0);
        glDrawable->rtFormatInfo = gcvNULL;
    }


    for (i = 0 ; i < __GL_MAX_DRAW_BUFFERS; i++)
    {
        glDrawable->rtHandles[i] = eglDrawable->rtHandles[i];
        glDrawable->prevRtHandles[i] = eglDrawable->prevRtHandles[i];
    }

    /* Get the depth stencil format Info, see EGL::veglGetFormat() */
    if (eglDrawable->depthHandle)
    {
        switch (pmode->depthBits)
        {
        case 16:
            glDrawable->dsFormatInfo = &__glFormatInfoTable[__GL_FMT_Z16];
            break;
        case 24:
            glDrawable->dsFormatInfo = pmode->stencilBits
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
            glDrawable->dsFormatInfo = pmode->stencilBits ? &__glFormatInfoTable[__GL_FMT_S8] : gcvNULL;
        }
        else
        {
            glDrawable->dsFormatInfo = pmode->stencilBits ? &__glFormatInfoTable[__GL_FMT_Z24S8] : gcvNULL;
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

    __glDevicePipe.devUpdateDrawable(glDrawable);

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
    if (gc->drawablePrivate->flags & __GL_DRAWABLE_FLAG_ZERO_WH)
    {
        gc->flags |= __GL_CONTEXT_SKIP_DRAW_INVALID_RENDERBUFFER;
    }
    else
    {
        gc->flags &= ~__GL_CONTEXT_SKIP_DRAW_INVALID_RENDERBUFFER;
    }
}
#if defined(OPENGL40) && defined(DRI_PIXMAPRENDER_GL)

/* Mutex lock for drawable change */
__GLlock drawableChangeLock;

extern GLvoid __glShareDlists(__GLcontext *dst, __GLcontext *src);
extern GLvoid __glFreeDlistVertexCache(__GLcontext *gc);
extern GLvoid __glFreeConcatDlistCache(__GLcontext *gc);


GLuint __glShareContext(__GLcontext *gc, __GLcontext *gcShare)
{
    __glShareDlists(gc, gcShare);
    return GL_TRUE;
}

GLvoid __glFreeDataCacheInVideoMemory(__GLcontext *gc)
{
    /* Free DL vertex cache if it is in video memory */
    __glFreeDlistVertexCache(gc);
    __glFreeConcatDlistCache(gc);

}

GLvoid __glAssociateContext(__GLcontext *gc, __GLdrawablePrivate *drawable, __GLdrawablePrivate *readable)
{
    gc->drawablePrivate = drawable;
    gc->readablePrivate = readable;
}

GLvoid __glDeassociateContext(__GLcontext *gc)
{
    gc->drawablePrivate = NULL;
    gc->readablePrivate = NULL;
}

GLvoid __glDeviceConfigurationChanged(__GLcontext *gc)
{
    if (gc->drawablePrivate) {
        __GL_MEMCOPY(&gc->modes, &gc->drawablePrivate->modes, sizeof(__GLcontextModes));
    }

    /* Free all vertex data caches in video memory */
    __glFreeDataCacheInVideoMemory(gc);

    /* Set all attributes dirty */
    __glSetAttributeStatesDirty(gc);

}

GLvoid __glNotifyRTMakeResident(__GLcontext *gc)
{
    __GLpipeline *ctx;
    ctx = &gc->dp.ctx;
    if (ctx->notifyWinBuffersResident)
    {
        ctx->notifyWinBuffersResident(gc, gc->drawablePrivate);
    }
}

GLvoid __glNotifySwapBuffers(__GLcontext *gc)
{
    __GLpipeline *ctx;
    ctx = &gc->dp.ctx;
    if (ctx->notifySwapBuffers)
    {
        ctx->notifySwapBuffers(gc);
    }
}

GLvoid __glNotifyChangeBufferSize(__GLcontext *gc)
{
    __GLpipeline *ctx;
    __GLdrawablePrivate *draw = gc->drawablePrivate;
    GLint yInvert = (DRAW_FRAMEBUFFER_BINDING_NAME == 0)? draw->yInverted :GL_FALSE;

    ctx = &gc->dp.ctx;
    if (ctx->notifyChangeBufferSize)
    {
        /* Release exclusive mode first, then create new RTs */
        if ((draw->fullScreenMode) && (draw->type == __GL_WINDOW) && (__glDevice->IsEXCLUSIVE_MODE))
        {
            (*draw->dp.setExclusiveDisplay)(GL_FALSE);
            draw->flipOn = GL_FALSE;
        }

        ctx->notifyChangeBufferSize(gc);
    }

    /* Reset raster pos to default value */
    if (yInvert)
        gc->state.rasterPos.rPos.winPos.f.y = (GLfloat)gc->drawablePrivate->height;
    else
        gc->state.rasterPos.rPos.winPos.f.y = 0;

    /* Compute clip box */
    __glComputeClipBox(gc);
}

GLvoid __glNotifyDestroyBuffers(__GLcontext *gc)
{
     __GLpipeline *ctx;

    ctx = &gc->dp.ctx;
    if (ctx->notifyDestroyBuffers)
    {
        ctx->notifyDestroyBuffers(gc);
    }

}

GLvoid __glNotifyDrawableSwitch(__GLcontext *gc)
{
    __GLpipeline *ctx;

    ctx = &gc->dp.ctx;
    if (ctx->notifyDrawableSwitch)
    {
        ctx->notifyDrawableSwitch(gc);
    }
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
    (*gc->imports.lockMutex)((VEGLLock *)&drawableChangeLock);

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
        gc->drawableDirtyMask |= __GL_BUFFER_DRAW_READ_BITS;
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

    (*gc->imports.unlockMutex)((VEGLLock *)&drawableChangeLock);
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
    (*gc->imports.lockMutex)((VEGLLock *)&drawableChangeLock);

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
            gc->drawableDirtyMask |= __GL_BUFFER_DRAW_READ_BITS;
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

    (*gc->imports.unlockMutex)((VEGLLock *)&drawableChangeLock);
}
#endif

GLvoid __glInitContextState(__GLcontext *gc)
{
    gc->flags = __GL_CONTEXT_UNINITIALIZED;
    gc->invalidCommonCommit = gcvTRUE;
    gc->invalidDrawCommit = gcvTRUE;
    gc->conditionalRenderDiscard = gcvFALSE;

    /* Some init functions should be isolated from ES3, TO DO */
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
    __glInitPrimRestartState(gc);

#ifdef OPENGL40
    if (gc->imports.conformGLSpec)
    {
        gc->renderMode = GL_RENDER;
        __glInitAttribStackState(gc);
        __glInitTransformState(gc);
        __glInitFogState(gc);
        __glInitLightState(gc);
        __glInitPointState(gc);
        __glInitEvaluatorState(gc);
        __glInitDlistState(gc);
        __glInitFeedback(gc);
    }
#endif

    __glBitmaskInitAllOne(&gc->texUnitAttrDirtyMask, gc->constants.shaderCaps.maxCombinedTextureImageUnits);
    __glBitmaskInitAllOne(&gc->imageUnitDirtyMask, gc->constants.shaderCaps.maxImageUnit);

    __glSetAttributeStatesDirty(gc);
}

GLvoid __glOverturnCommitStates(__GLcontext *gc)
{
    GLuint i, j;
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
    pCommit->raster.mrtEnable = !pState->raster.mrtEnable;

    for (i = 0; i < __GL_MAX_IMAGE_UNITS; ++i)
    {
        pCommit->image.imageUnit[i].invalid = !pState->image.imageUnit[i].invalid;
        pCommit->image.imageUnit[i].layered = !pState->image.imageUnit[i].layered;
        pCommit->image.imageUnit[i].singleLayered = !pState->image.imageUnit[i].singleLayered;
    }

    pCommit->depth.writeEnable = !pState->depth.writeEnable;

    pCommit->enables.colorBuffer.dither = !pState->enables.colorBuffer.dither;
#ifdef OPENGL40
    pCommit->enables.colorBuffer.alphaTest = !pState->enables.colorBuffer.alphaTest;
    pCommit->enables.colorBuffer.logicOp = !pState->enables.colorBuffer.logicOp;
    pCommit->enables.colorBuffer.colorLogicOp = !pState->enables.colorBuffer.colorLogicOp;
    pCommit->enables.colorBuffer.indexLogicOp = !pState->enables.colorBuffer.indexLogicOp;
#endif

    pCommit->enables.polygon.cullFace = !pState->enables.polygon.cullFace;
    pCommit->enables.polygon.polygonOffsetFill = !pState->enables.polygon.polygonOffsetFill;
#ifdef OPENGL40
    pCommit->enables.polygon.smooth = !pState->enables.polygon.smooth;
    pCommit->enables.polygon.stipple = !pState->enables.polygon.stipple;
    pCommit->enables.polygon.polygonOffsetPoint = !pState->enables.polygon.polygonOffsetPoint;
    pCommit->enables.polygon.polygonOffsetLine = !pState->enables.polygon.polygonOffsetLine;
#endif

    pCommit->enables.multisample.alphaToCoverage = !pState->enables.multisample.alphaToCoverage;
    pCommit->enables.multisample.coverage = !pState->enables.multisample.coverage;
    pCommit->enables.multisample.sampleMask = !pState->enables.multisample.sampleMask;
    pCommit->enables.multisample.sampleShading = !pState->enables.multisample.sampleShading;
#ifdef OPENGL40
    pCommit->enables.multisample.multisampleOn = !pState->enables.multisample.multisampleOn;
    pCommit->enables.multisample.alphaToOne = !pState->enables.multisample.alphaToOne;
#endif

    pCommit->enables.scissorTest = !pState->enables.scissorTest;
    pCommit->enables.depthTest = !pState->enables.depthTest;
    pCommit->enables.stencilTest = !pState->enables.stencilTest;
    pCommit->enables.primitiveRestart = !pState->enables.primitiveRestart;
    pCommit->enables.rasterizerDiscard = !pState->enables.rasterizerDiscard;

#ifdef OPENGL40
    pCommit->enables.transform.normalize = !pState->enables.transform.normalize;
    pCommit->enables.transform.rescaleNormal = !pState->enables.transform.rescaleNormal;

    pCommit->enables.lighting.lighting = !pState->enables.lighting.lighting;
    pCommit->enables.lighting.colorMaterial = !pState->enables.lighting.colorMaterial;
    for (i = 0; i < __GL_MAX_LIGHT_NUMBER; i++)
    {
        pCommit->enables.lighting.light[i] = !pState->enables.lighting.light[i];
    }

    pCommit->enables.eval.autonormal = !pState->enables.eval.autonormal;
    for (i = 0; i < __GL_MAP_RANGE_COUNT; i++)
    {
        pCommit->enables.eval.map1[i] = !pState->enables.eval.map1[i];
        pCommit->enables.eval.map2[i] = !pState->enables.eval.map2[i];
    }

    for (i = 0; i < __GL_MAX_TEXTURE_UNITS; i++)
    {
        pCommit->enables.texUnits[i].texGen[0] = !pState->enables.texUnits[i].texGen[0];
        pCommit->enables.texUnits[i].texGen[1] = !pState->enables.texUnits[i].texGen[1];
        pCommit->enables.texUnits[i].texGen[2] = !pState->enables.texUnits[i].texGen[2];
        pCommit->enables.texUnits[i].texGen[3] = !pState->enables.texUnits[i].texGen[3];
        pCommit->enables.texUnits[i].texture1D = !pState->enables.texUnits[i].texture1D;
        pCommit->enables.texUnits[i].texture2D = !pState->enables.texUnits[i].texture2D;
        pCommit->enables.texUnits[i].texture3D = !pState->enables.texUnits[i].texture3D;
        pCommit->enables.texUnits[i].textureCubeMap = !pState->enables.texUnits[i].textureCubeMap;
        pCommit->enables.texUnits[i].textureRec = !pState->enables.texUnits[i].textureRec;
    }

    pCommit->enables.depthBuffer.test = !pState->enables.depthBuffer.test;

    pCommit->enables.line.smooth = !pState->enables.line.smooth;
    pCommit->enables.line.stipple = !pState->enables.line.stipple;
    pCommit->enables.line.stippleRequested = !pState->enables.line.stippleRequested;

    pCommit->enables.program.fragmentProgram = !pState->enables.program.fragmentProgram;
    pCommit->enables.program.vertexProgram = !pState->enables.program.vertexProgram;
    pCommit->enables.program.vpPointSize = !pState->enables.program.vpPointSize;
    pCommit->enables.program.vpTwoSize = !pState->enables.program.vpTwoSize;

    pCommit->enables.pointSmooth = !pState->enables.pointSmooth;
    pCommit->enables.fog = !pState->enables.fog;
    pCommit->enables.scissor = !pState->enables.scissor;
    pCommit->enables.stencilTestTwoSideExt = !pState->enables.stencilTestTwoSideExt;
    pCommit->enables.colorSum = !pState->enables.colorSum;
    pCommit->enables.depthBoundTest = !pState->enables.depthBoundTest;
    pCommit->enables.pointSprite = !pState->enables.pointSprite;
#endif

    pCommit->multisample.coverageInvert = !pState->multisample.coverageInvert;
#ifdef OPENGL40
    pCommit->multisample.alphaToCoverage = !pState->multisample.alphaToCoverage;
    pCommit->multisample.alphaToOne = !pState->multisample.alphaToOne;
    pCommit->multisample.coverage = !pState->multisample.coverage;
#endif

    for (i = 0; i < __GL_MAX_TEXTURE_UNITS; i++)
    {
        pCommit->texture.texUnits[i].commitParams.contentProtected = !pState->texture.texUnits[i].commitParams.contentProtected;
#ifdef OPENGL40
        pCommit->texture.texUnits[i].commitParams.sampler.generateMipmap = !pState->texture.texUnits[i].commitParams.sampler.generateMipmap;
        pCommit->texture.texUnits[i].env.coordReplace = !pState->texture.texUnits[i].env.coordReplace;
#endif
        for (j = 0; j < __GL_MAX_TEXTURE_BINDINGS; j++)
        {
            pCommit->texture.texUnits[i].texObj[j].params.contentProtected = !pState->texture.texUnits[i].texObj[j].params.contentProtected;
#ifdef OPENGL40
            pCommit->texture.texUnits[i].texObj[j].params.sampler.generateMipmap = !pState->texture.texUnits[i].texObj[j].params.sampler.generateMipmap;
#endif
        }
    }

#ifdef OPENGL40
    pCommit->rasterPos.validRasterPos = !pState->rasterPos.validRasterPos;

    pCommit->pixel.transferMode.mapColor = !pState->pixel.transferMode.mapColor;
    pCommit->pixel.transferMode.mapStencil = !pState->pixel.transferMode.mapStencil;
#endif

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
    gctSTRING mode = gcvNULL;

    /* Initialize dpGlobalInfo and device entry functions */
    if (__glDpInitialize(&__glDevicePipe) == GL_FALSE)
    {
        return;
    }

    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_TRACE", &mode)) && mode)
    {
        if (gcmIS_SUCCESS(gcoOS_StrCmp(mode, "0")))
        {
            __glApiTraceMode = gcvTRACEMODE_NONE;
        }
        else if (gcmIS_SUCCESS(gcoOS_StrCmp(mode, "1")))
        {
            __glApiTraceMode = gcvTRACEMODE_FULL;
        }
        else if (gcmIS_SUCCESS(gcoOS_StrCmp(mode, "2")))
        {
            __glApiTraceMode = gcvTRACEMODE_LOGGER;
        }
        else if (gcmIS_SUCCESS(gcoOS_StrCmp(mode, "3")))
        {
            __glApiTraceMode = gcvTRACEMODE_ALLZONE;
        }
        else if (gcmIS_SUCCESS(gcoOS_StrCmp(mode, "4")))
        {
            __glApiTraceMode = gcvTRACEMODE_PRE;
        }
        else if (gcmIS_SUCCESS(gcoOS_StrCmp(mode, "5")))
        {
            __glApiTraceMode = gcvTRACEMODE_POST;
        }
        else
        {
            gcmPRINT("ES: Unsupported trace mode");
        }

        if (__glApiTraceMode == gcvTRACEMODE_ALLZONE)
        {
            gcoOS_SetDebugLevel(gcvLEVEL_VERBOSE);
            gcoOS_SetDebugZone(gcdZONE_ALL);
        }

        if (!__glInitTracerDispatchTable(__glApiTraceMode, apiVersion))
        {
            /* Reset to regular API entry functions if tracer dispatch table initialization failed */
            __glApiTraceMode = gcvTRACEMODE_NONE;
        }
    }

#if VIVANTE_PROFILER
    mode = gcvNULL;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_PROFILE", &mode)) && mode)
    {
        if (gcmIS_SUCCESS(gcoOS_StrCmp(mode, "0")))
        {
            __glApiProfileMode = 0;
        }
        else if (gcmIS_SUCCESS(gcoOS_StrCmp(mode, "1")))
        {
            __glApiProfileMode = 1;
        }
        else if (gcmIS_SUCCESS(gcoOS_StrCmp(mode, "2")))
        {
            __glApiProfileMode = 2;
        }
        else if (gcmIS_SUCCESS(gcoOS_StrCmp(mode, "3")))
        {
            __glApiProfileMode = 3;
        }
    }
#endif

    __glOverWriteListCompileTable();
}

GLboolean __glLoseCurrent(__GLcontext *gc, __GLdrawablePrivate* drawable, __GLdrawablePrivate* readable)
{
    GLboolean retVal;

    __glSetDrawable(gc, drawable, readable);

    /* Notify DP the context is not current anymore */
    retVal = (*gc->dp.loseCurrent)(gc, GL_FALSE);

#if defined(OPENGL40) && defined(DRI_PIXMAPRENDER_GL)
    if (gc->imports.conformGLSpec)
    {
        /* Free all vertex data caches in video memory */
        __glFreeDataCacheInVideoMemory(gc);
        __glFreeVertexInputState(gc);
    }
#endif

    return retVal;
}

GLboolean __glMakeCurrent(__GLcontext *gc, __GLdrawablePrivate* drawable, __GLdrawablePrivate* readable, GLboolean flushDrawableChange)
{
    GLboolean retVal;
    __GLframebufferObject *defaultDrawFBO = &gc->frameBuffer.defaultDrawFBO;
    __GLframebufferObject *defaultReadFBO = &gc->frameBuffer.defaultReadFBO;

    __glSetDrawable(gc, drawable, readable);
#ifdef OPENGL40
    if (gc->imports.conformGLSpec)
    {
        __glInitVertexInputState(gc);
    }
#endif
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

#ifdef OPENGL40
        if (gc->imports.conformGLSpec)
        {
            gc->input.inputMaskChanged = GL_TRUE;
            /* Must reset drawables/RTs for newly created gc */
            gc->changeMask |= __GL_DRAWABLE_PENDING_RESIZE;
        }
#endif

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
    retVal = (*gc->dp.makeCurrent)(gc);

#if defined(OPENGL40) && defined(DRI_PIXMAPRENDER_GL)
    if (gc->imports.conformGLSpec)
    {
        /* Get the latest drawable information */
        LINUX_LOCK_FRAMEBUFFER(gc);
        /* Make sure the drawable is allocated and updated */
        __glDispatchDrawableChange(gc);
        __glComputeClipBox(gc);
        LINUX_UNLOCK_FRAMEBUFFER(gc);
    }
#endif

    return retVal;
}

GLboolean __glDestroyContext(GLvoid *context)
{
    __GLcontext *gc = (__GLcontext*)context;
    GLboolean retVal = GL_TRUE;

#ifdef OPENGL40
    if (gc->imports.conformGLSpec)
    {
         __glFreeAttribStackState(gc);
         __glFreeTransformState(gc);
         __glFreeVertexInputState(gc);
         __glFreeEvaluatorState(gc);
         __glFreeDlistState(gc);
    }
#endif
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
    gcoOS_SetDriverTLS(gcvTLS_KEY_OPENGL, (gcsDRIVER_TLS_PTR)gcvNULL);

    return retVal;
}

GLvoid *__glCreateContext(GLint clientVersion,
                          VEGLimports *imports,
                          GLvoid* sharedCtx)
{
    __GLcontext *gc = gcvNULL;
    __GLApiVersion apiVersion;
    __GLdeviceStruct *__glDevice = &__glDevicePipe;
    static GLboolean initialized = GL_FALSE;

    __GL_HEADER();

#ifdef OPENGL40
    if (imports->conformGLSpec)
    {
        switch (clientVersion)
        {
        case 0x10:
            apiVersion = __GL_API_VERSION_OGL10;
            break;
        case 0x11:
            apiVersion = __GL_API_VERSION_OGL11;
            break;
        case 0x12:
            apiVersion = __GL_API_VERSION_OGL12;
            break;
        case 0x13:
            apiVersion = __GL_API_VERSION_OGL13;
            break;
        case 0x14:
            apiVersion = __GL_API_VERSION_OGL14;
            break;
        case 0x15:
            apiVersion = __GL_API_VERSION_OGL15;
            break;
        case 0x20:
            apiVersion = __GL_API_VERSION_OGL20;
            break;
        case 0x21:
            apiVersion = __GL_API_VERSION_OGL21;
            break;
        case 0x30:
            apiVersion = __GL_API_VERSION_OGL30;
            break;
        case 0x31:
            apiVersion = __GL_API_VERSION_OGL31;
            break;
        case 0x32:
            apiVersion = __GL_API_VERSION_OGL32;
            break;
        case 0x33:
            apiVersion = __GL_API_VERSION_OGL33;
            break;
        case 0x40:
            apiVersion = __GL_API_VERSION_OGL40;
            break;
        default:
            GL_ASSERT(0);
            __GL_EXIT();
        }
    }
    else  /* Runing OES api */
#endif
    {
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
            __GL_EXIT();
        }
    }

    if (!initialized)
    {
        __glInitGlobals(apiVersion);
        initialized = GL_TRUE;
    }

    /* Allocate memory for core GL context.
    */
    gc = (__GLcontext*)(*imports->calloc)(gc, 1, sizeof(__GLcontext));
    if (!gc)
    {
        __GL_EXIT();
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

    if (imports->config)
    {
        __glFormatGLModes(&gc->modes, (__GLcontextModes *)imports->config);
    }

    /* Fill in the export functions.
    */
    if (gc->imports.conformGLSpec)
    {
        gc->exports.createContext  = __glCreateContext;
        gc->exports.destroyContext = __glDestroyContext;
        gc->exports.setDrawable    = __glSetDrawable;
        gc->exports.makeCurrent    = __glMakeCurrent;
        gc->exports.loseCurrent    = __glLoseCurrent;
        gc->exports.getThreadData  = __eglGetThreadEsPrivData;
        gc->apiVersion   = apiVersion;
        gc->shareCtx     = (__GLcontext*)sharedCtx;
    }

    if (!gc->imports.conformGLSpec)
    {
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

    if ((*__glDevice->devCreateContext)(gc) == GL_FALSE)
    {
        (*imports->free)(gc, gc);
        gc = gcvNULL;
        __GL_EXIT();
    }

#if gcdPATTERN_FAST_PATH
    /* Init Pattern */
    __glInitPattern(gc);
#endif

    /* Initialize API dispatch tables. */
    if (__glApiTraceMode > gcvTRACEMODE_NONE
#if VIVANTE_PROFILER
     || __glApiProfileMode > 0
#endif
       )
    {
        gc->apiProfile = GL_TRUE;
    }
    else
    {
        gc->apiProfile = GL_FALSE;
    }

    /* Initialize API dispatch tables. */
#ifdef OPENGL40
    if (gc->imports.conformGLSpec)
    {
        gc->dlCompileDispatch = __glListCompileFuncTable;
    }
#endif
    gc->immedModeDispatch = __glImmediateFuncTable;
    gc->pModeDispatch = &gc->immedModeDispatch; /* immediate mode by default. */
    gc->pEntryDispatch = gc->apiProfile ? &__glProfileFuncTable : gc->pModeDispatch;

    gc->magic = ES3X_MAGIC;

    /* Do not destructor. */
    gc->base.destructor = gcvNULL;

    __GL_FOOTER();
    return (gc);

OnExit:
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

