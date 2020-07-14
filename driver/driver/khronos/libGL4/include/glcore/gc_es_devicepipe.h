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


#ifndef __gc_es_devicepipe_h__
#define __gc_es_devicepipe_h__

#if defined(OPENGL40) && defined(DRI_PIXMAPRENDER_GL)
typedef struct __GLpipelineRec {

    /* Notify the pipeline that the drawable has changed*/
    GLvoid (*notifyChangeBufferSize)(__GLcontext*);
    GLvoid (*notifyDestroyBuffers)(__GLcontext*);
    GLvoid (*notifySwapBuffers)(__GLcontext*);
    GLvoid (*notifyDrawableSwitch)(__GLcontext*);
    GLvoid (*notifyWinBuffersResident)(__GLcontext*, __GLdrawablePrivate*);

    /* Display device configuration change notification */
    /* such as mode change, samm enabled/disabled, rotation enabled/disabled and etc. */
    GLvoid (*deviceConfigurationChanged)(__GLcontext*);

    /* Pointer to pipeline private data area */
    GLvoid *privateData;

} __GLpipeline;
#endif

typedef struct __GLdevicePipelineRec
{
    /* Pointer to chip private data area */
    GLvoid *privateData;

    /* Private context management functions */
    GLboolean (*makeCurrent)(__GLcontext*);
    GLboolean (*loseCurrent)(__GLcontext*, GLboolean);
    GLboolean (*destroyPrivateData)(__GLcontext*);
    GLvoid (*queryFormatInfo)(__GLcontext *, __GLformat, GLint *, GLint *, GLint);

    /* Draw primitive functions */
    GLboolean (*drawBegin)(__GLcontext*, GLenum);
    GLboolean (*drawValidateState)(__GLcontext*);
    GLboolean (*drawEnd)(__GLcontext*);
    GLboolean (*drawPrimitive)(__GLcontext*);

    /* Raster functions */
    GLboolean (*readPixelsBegin)(__GLcontext*);
    GLvoid (*readPixelsValidateState)(__GLcontext *);
    GLboolean (*readPixelsEnd)(__GLcontext *);
    GLboolean (*readPixels)(__GLcontext*, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLubyte*);
#ifdef OPENGL40
    /* Raster function begin, end */
    GLvoid (*rasterBegin)(__GLcontext*, GLenum, GLenum, GLint , GLint);
    GLvoid (*rasterEnd)(__GLcontext*, GLenum);
    GLboolean (*bitmaps)(__GLcontext*, GLsizei, GLsizei, GLfloat, GLfloat, GLfloat, GLfloat, const GLubyte*, __GLbufferObject* bufObj);
    GLboolean (*drawPixels)(__GLcontext *gc, GLsizei width, GLsizei height, GLenum format, GLenum type, GLubyte *pixels);
    GLboolean (*copyPixels)(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format);
#endif
    /* Read/draw buffers */
    GLboolean (*changeDrawBuffers)(__GLcontext*);
    GLboolean (*changeReadBuffers)(__GLcontext*);
    GLvoid (*detachDrawable)(__GLcontext*);

    /* Texture functions */
    GLvoid (*bindTexture)(__GLcontext*, __GLtextureObject*);
    GLvoid (*deleteTexture)(__GLcontext*, __GLtextureObject*);
    GLvoid (*detachTexture)(__GLcontext*,  __GLtextureObject*);

#ifdef OPENGL40
    GLboolean (*texImage1D)(__GLcontext*, __GLtextureObject*,  GLint, const GLvoid *);
#endif
    GLboolean (*texImage2D)(__GLcontext*, __GLtextureObject*, GLint, GLint, const GLvoid*);
    GLboolean (*texImage3D)(__GLcontext*, __GLtextureObject*, GLint, const GLvoid*);
#ifdef OPENGL40
    GLboolean (*texSubImage1D)(__GLcontext*, __GLtextureObject*, GLint, GLint, GLint, const GLvoid*);
#endif
    GLboolean (*texSubImage2D)(__GLcontext*, __GLtextureObject*, GLint, GLint, GLint, GLint, GLint, GLint, const GLvoid*);
    GLboolean (*texSubImage3D)(__GLcontext*, __GLtextureObject*, GLint, GLint, GLint, GLint, GLint, GLint, GLint, const GLvoid*);
#ifdef OPENGL40
    GLboolean (*copyTexImage1D)(__GLcontext*, __GLtextureObject*, GLint, GLint, GLint);
#endif
    GLboolean (*copyTexImage2D)(__GLcontext*, __GLtextureObject*, GLint, GLint, GLint, GLint);
#ifdef OPENGL40
    GLboolean (*copyTexSubImage1D)(__GLcontext*, __GLtextureObject*, GLint, GLint, GLint, GLint, GLint);
#endif
    GLboolean (*copyTexSubImage2D)(__GLcontext*, __GLtextureObject*, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint);
    GLboolean (*copyTexSubImage3D)(__GLcontext*, __GLtextureObject*, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint);
    GLboolean (*texDirectVIV)(__GLcontext*, __GLtextureObject*, GLint, GLint, GLenum, GLvoid**);
    GLboolean (*texDirectInvalidateVIV)(__GLcontext*, __GLtextureObject*);
    GLboolean (*texDirectVIVMap)(__GLcontext*, __GLtextureObject*, GLenum, GLsizei, GLsizei, GLenum, GLvoid **, const GLuint *, GLboolean);
#ifdef OPENGL40
    GLboolean (*compressedTexImage1D)(__GLcontext*, __GLtextureObject*, GLint, const GLvoid*);
#endif
    GLboolean (*compressedTexImage2D)(__GLcontext*, __GLtextureObject*, GLint, GLint, const GLvoid*);
    GLboolean (*compressedTexImage3D)(__GLcontext*, __GLtextureObject*, GLint, const GLvoid*);
#ifdef OPENGL40
    GLboolean (*compressedTexSubImage1D)(__GLcontext*, __GLtextureObject*, GLint, GLint, GLint, GLint, const GLvoid*);
#endif
    GLboolean (*compressedTexSubImage2D)(__GLcontext*, __GLtextureObject*, GLint, GLint, GLint, GLint, GLint, GLint, const GLvoid*, GLsizei);
    GLboolean (*compressedTexSubImage3D)(__GLcontext*, __GLtextureObject*, GLint, GLint, GLint, GLint, GLint, GLint, GLint, const GLvoid*, GLsizei);
    GLboolean (*generateMipmaps)(__GLcontext*, __GLtextureObject*, GLint, GLint*);
    GLboolean (*getTexImage)(__GLcontext*, __GLtextureObject*, GLint, GLint, GLenum, GLenum, GLvoid*);
#ifdef OPENGL40
    GLboolean (*getCompressedTexImage)(__GLcontext*, __GLtextureObject*, __GLmipMapLevel *, GLint , GLvoid *);
#endif


    GLboolean (*copyTexBegin)(__GLcontext*);
    GLvoid (*copyTexValidateState)(__GLcontext *);
    GLvoid (*copyTexEnd)(__GLcontext *);

    GLboolean (*copyImageSubData)(__GLcontext *, GLvoid *, GLint, GLint, GLint, GLint, GLint, GLvoid *, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei);

    /* EGL image */
    GLboolean (*bindTexImage)(__GLcontext*, __GLtextureObject*, GLint, void*, void**);
    GLvoid (*freeTexImage)(__GLcontext*, __GLtextureObject*, GLint, GLint);
    GLboolean (*eglImageTargetTexture2DOES)(__GLcontext*, __GLtextureObject*, GLenum, GLvoid*);
    GLenum (*createEglImageTexture)(__GLcontext*, __GLtextureObject*, GLint, GLint, GLint, GLvoid*);
    GLenum (*createEglImageRenderbuffer)(__GLcontext*, __GLrenderbufferObject*, GLvoid*);
    GLboolean (*eglImageTargetRenderbufferStorageOES)(__GLcontext*, __GLrenderbufferObject*, GLenum, GLvoid*);
    GLboolean (*getTextureAttribFromImage)(__GLcontext*, GLvoid*, GLint*, GLint*, GLint*, gceSURF_FORMAT*, GLint*, GLint*, GLint*, GLint*, GLuint*, GLvoid**);

    /* Vertex buffer object */
    GLboolean (*bindBuffer)(__GLcontext*, __GLbufferObject*, GLuint);
    GLboolean (*bufferSubData)(__GLcontext*, __GLbufferObject*, GLuint, GLintptr, GLsizeiptr, const GLvoid*);
    GLvoid *  (*mapBufferRange)(__GLcontext*, __GLbufferObject*, GLuint, GLintptr, GLsizeiptr, GLbitfield);
    GLboolean (*flushMappedBufferRange)(__GLcontext*, __GLbufferObject*, GLuint, GLintptr, GLsizeiptr);
    GLboolean (*unmapBuffer)(__GLcontext*, __GLbufferObject*, GLuint);
    GLboolean (*deleteBuffer)(__GLcontext*, __GLbufferObject*);
    GLboolean (*bufferData)(__GLcontext*, __GLbufferObject*, GLuint, const GLvoid*);
    GLboolean (*copyBufferSubData)(__GLcontext*, GLuint, __GLbufferObject*, GLuint, __GLbufferObject*, GLintptr, GLintptr, GLsizeiptr);
    GLboolean (*getBufferSubData)(__GLcontext*, GLuint, __GLbufferObject*, GLintptr, GLsizeiptr, GLvoid *);

    /* Occlusion query */
    GLboolean (*beginQuery)(__GLcontext*, __GLqueryObject*);
    GLboolean (*endQuery)(__GLcontext*, __GLqueryObject*);
    GLboolean (*getQueryObject)(__GLcontext*, GLenum, __GLqueryObject*);
    GLvoid (*deleteQuery)(__GLcontext*, __GLqueryObject*);

    /* Flush, Finish */
    GLboolean (*flush)(__GLcontext*);
    GLboolean (*finish)(__GLcontext*);
    GLboolean (*clear)(__GLcontext*, GLbitfield);
    GLboolean (*clearBuffer)(__GLcontext*, GLenum, GLint, GLvoid*, GLenum);
    GLboolean (*clearBufferfi)(__GLcontext*, GLfloat, GLint);
    GLboolean (*clearBegin)(__GLcontext*, GLbitfield*);
    GLboolean (*clearValidateState)(__GLcontext *, GLbitfield);
    GLboolean (*clearEnd)(__GLcontext *, GLbitfield);
#ifdef OPENGL40
    GLvoid  (*createAccumBufferInfo)(__GLcontext *, gcoSURF, __GLdrawablePrivate *);
    GLvoid (*bindFragDataLocation)(__GLcontext*, __GLprogramObject*, GLuint,  const GLchar*);
#endif

    /* GLSL APIs */
    GLvoid (*deleteShader)(__GLcontext*, __GLshaderObject*);
    GLboolean (*compileShader)(__GLcontext*, __GLshaderObject*);
    GLboolean (*createProgram)(__GLcontext*, __GLprogramObject*);
    GLvoid (*deleteProgram)(__GLcontext*, __GLprogramObject*);
    GLboolean (*linkProgram)(__GLcontext*, __GLprogramObject*);
    GLboolean (*useProgram)(__GLcontext*, __GLprogramObject*, GLboolean*);
    GLboolean (*validateProgram)(__GLcontext*, __GLprogramObject*, GLboolean);
    GLboolean (*getProgramBinary)(__GLcontext*, __GLprogramObject*, GLsizei, GLsizei*, GLenum*, GLvoid*);
    GLboolean (*programBinary)(__GLcontext*, __GLprogramObject*, const GLvoid*, GLsizei);
    GLboolean (*shaderBinary)(__GLcontext*, GLsizei, __GLshaderObject**, GLenum, const GLvoid*, GLsizei);
    GLboolean (*bindAttributeLocation)(__GLcontext*, __GLprogramObject*, GLuint, const GLchar*);
    GLboolean (*getActiveAttribute)(__GLcontext*, __GLprogramObject*, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar*);
    GLint (*getAttributeLocation)(__GLcontext*, __GLprogramObject*, const GLchar*);
    GLint (*getFragDataLocation)(__GLcontext*, __GLprogramObject*, const GLchar*);
    GLint (*getUniformLocation)(__GLcontext*, __GLprogramObject*, const GLchar*);
    GLvoid (*getActiveUniform)(__GLcontext*, __GLprogramObject*, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar*);
    GLvoid (*getActiveUniformsiv)(__GLcontext*, __GLprogramObject*, GLsizei, const GLuint*, GLenum, GLint*);
    GLvoid (*getUniformIndices)(__GLcontext*, __GLprogramObject*, GLsizei, const GLchar* const*, GLuint*);
    GLuint (*getUniformBlockIndex)(__GLcontext*, __GLprogramObject*, const GLchar*);
    GLvoid (*getActiveUniformBlockiv)(__GLcontext*, __GLprogramObject*, GLuint, GLenum, GLint*);
    GLvoid (*getActiveUniformBlockName)(__GLcontext*, __GLprogramObject*, GLuint, GLsizei, GLsizei*, GLchar*);
    GLvoid (*uniformBlockBinding)(__GLcontext*, __GLprogramObject*, GLuint, GLuint);
    GLboolean (*setUniformData)(__GLcontext*, __GLprogramObject*, GLint, GLint, GLsizei, const void*, GLboolean);
    GLboolean (*getUniformData)(__GLcontext*, __GLprogramObject*, GLint, GLint, GLvoid*);
    GLsizei (*getUniformSize)(__GLcontext*, __GLprogramObject*, GLint);
    GLvoid (*buildTexEnableDim)(__GLcontext*);
    GLuint (*getProgramResourceIndex)(__GLcontext*, __GLprogramObject*, GLenum, const GLchar*);
    GLvoid (*getProgramResourceName)(__GLcontext*, __GLprogramObject*, GLenum, GLuint, GLsizei, GLsizei*, GLchar*);
    GLvoid (*getProgramResourceiv)(__GLcontext*, __GLprogramObject*, GLenum, GLuint, GLsizei, const GLenum*, GLsizei, GLsizei*, GLint*);
    GLboolean (*validateProgramPipeline)(__GLcontext*, __GLprogramPipelineObject*, GLboolean);

    /* RBO & FBO */
    GLboolean (*bindDrawFramebuffer)(__GLcontext*, __GLframebufferObject*, __GLframebufferObject*);
    GLvoid (*bindReadFramebuffer)(__GLcontext*, __GLframebufferObject*, __GLframebufferObject*);
    GLvoid (*bindRenderbuffer)(__GLcontext*, __GLrenderbufferObject*);
    GLvoid (*deleteRenderbuffer)(__GLcontext*, __GLrenderbufferObject*);
    GLvoid (*detachRenderbuffer)(__GLcontext*, __GLrenderbufferObject*);
    GLboolean (*renderbufferStorage)(__GLcontext*, __GLrenderbufferObject*);
    GLvoid (*blitFramebuffer)(__GLcontext*, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLboolean, GLboolean, GLenum);
    GLboolean (*frameBufferTexture)(__GLcontext*, __GLframebufferObject*, GLint, __GLtextureObject*, GLint, GLint, GLsizei, GLint, GLboolean, __GLfboAttachPoint*);
    GLvoid (*framebufferRenderbuffer)(__GLcontext*, __GLframebufferObject*, GLint, __GLrenderbufferObject*, __GLfboAttachPoint*);
    GLboolean (*isFramebufferComplete)(__GLcontext*, __GLframebufferObject*);
    GLvoid (*invalidateFramebuffer)(__GLcontext*, __GLframebufferObject*, __GLfboAttachPoint*, GLint, GLint, GLsizei, GLsizei);
    GLvoid (*invalidateDrawable)(__GLcontext*, GLint, GLint, GLsizei, GLsizei);
    GLboolean (*blitFramebufferBegin)(__GLcontext *);
    GLboolean (*blitFramebufferValidateState)(__GLcontext *, GLbitfield);
    GLboolean (*blitFramebufferEnd)(__GLcontext *);
    GLvoid (*cleanTextureShadow)(__GLcontext*, __GLtextureObject*);
    GLvoid (*cleanRenderbufferShadow)(__GLcontext*, __GLrenderbufferObject*);

    /* Sync */
    GLboolean (*createSync)(__GLcontext*, __GLsyncObject*);
    GLboolean (*deleteSync)(__GLcontext*, __GLsyncObject*);
    GLenum (*waitSync)(__GLcontext*, __GLsyncObject*, GLuint64);
    GLboolean (*syncImage)(__GLcontext*);

    /* Transform feedback */
    GLvoid (*bindXFB)(__GLcontext*, __GLxfbObject*);
    GLvoid (*deleteXFB)(__GLcontext*, __GLxfbObject*);
    GLvoid (*beginXFB)(__GLcontext*, __GLxfbObject*);
    GLvoid (*pauseXFB)(__GLcontext *);
    GLvoid (*resumeXFB)(__GLcontext *);
    GLvoid (*endXFB)(__GLcontext*, __GLxfbObject*);
    GLvoid (*getXfbVarying)(__GLcontext*, __GLprogramObject*, GLuint, GLsizei, GLsizei*, GLsizei*, GLenum*, GLchar*);
    GLboolean (*checkXFBBufSizes)(__GLcontext *, __GLxfbObject *, GLsizei);

    /* GL_EXT_robustness */
    GLenum (*getGraphicsResetStatus)(__GLcontext *);

#if VIVANTE_PROFILER
    /* profiler */
    GLboolean (*profiler)(__GLcontext *, GLuint, gctHANDLE);
#endif

    /* Patches. */
    GLvoid  (*patchBlend)(__GLcontext *, gctBOOL);

    /* Get error from chip layer*/
    GLenum (*getError)(__GLcontext *);

    /* Get sample location*/
    GLvoid (*getSampleLocation)(__GLcontext *, GLuint, GLfloat*);

    /* Compute shader */
    GLboolean  (*computeBegin)(__GLcontext *);
    GLboolean  (*computeValidateState)(__GLcontext *);
    GLvoid (*computeEnd)(__GLcontext *);
    GLboolean (*dispatchCompute)(__GLcontext *);

    /* Memory barrier */
    GLvoid (*memoryBarrier)(__GLcontext *, GLbitfield);

    /* Advanced blend barrier */
    GLvoid (*blendBarrier)(__GLcontext *);
    GLboolean (*drawPattern)(__GLcontext*);
    /* Accumulation functions */
    GLvoid (*accum)(__GLcontext*, GLenum , GLfloat);

#if defined(OPENGL40) && defined(DRI_PIXMAPRENDER_GL)
    /* Device private context management functions */
    __GLpipeline ctx;
#endif

} __GLdevicePipeline;

#endif /* __gc_es_devicepipe_h__ */


