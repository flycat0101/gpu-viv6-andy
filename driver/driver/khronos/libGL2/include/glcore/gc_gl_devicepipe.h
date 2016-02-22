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


#ifndef __gc_gl_devicepipe_h_
#define __gc_gl_devicepipe_h_

#include "gc_gl_context.h"

/* Macro to check if privateData.resource.handle is NULL */
#define __GL_DP_RESOURCE_HANDLE(privateData)   (*(GLuint*)(privateData))

enum {
    __GL_DP_GENERIC_PATH = 0,
    __GL_SWP_GENERIC_PATH,
    __GL_DP_PATH_NUM
};

typedef struct __GLpipelineRec {

    /* Private context management functions */
    GLuint (*makeCurrent)(__GLcontext*);
    GLuint (*loseCurrent)(__GLcontext*, GLboolean);
    GLuint (*destroyPrivateData)(__GLcontext*);

    /* Draw primitive function with vertex input from "gc->vertexStreams" */
    GLvoid (*drawPrimitive)(__GLcontext*);

    GLvoid (*tnLAndAccumInPipeline)(__GLcontext*, GLboolean);

    /* Raster functions */
    GLboolean (*drawPixels)(__GLcontext*, GLsizei, GLsizei, GLenum, GLenum, GLubyte*);
    GLboolean (*readPixels)(__GLcontext*, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLubyte*);
    GLboolean (*copyPixels)(__GLcontext*, GLint, GLint, GLsizei, GLsizei, GLenum);
    GLboolean (*bitmaps)(__GLcontext*, GLsizei, GLsizei, GLfloat, GLfloat, GLfloat, GLfloat, const GLubyte*, __GLbufferObject* bufObj);

    /* Raster position function */
    GLvoid (*rasterPos)(__GLcontext*, GLfloat*);

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

typedef struct __GLdevicePipelineRec {
    /* Device private context management functions */
    __GLpipeline ctx;

    /* Draw primitive begin, end */
    GLvoid (*begin)(__GLcontext*, GLenum);
    GLvoid (*end)(__GLcontext*);
    GLuint beginIndex;

    /* Raster function begin, end */
    GLvoid (*rasterBegin)(__GLcontext*, GLenum, GLenum, GLint , GLint);
    GLvoid (*rasterEnd)(__GLcontext*, GLenum);

    /* Raster position begin, end */
    GLvoid (*rasterPosBegin)(__GLcontext*);
    GLvoid (*rasterPosEnd)(__GLcontext*);

    /* Used to notify DP of GL attribute changes */
    GLvoid (*attributeChanged)(__GLcontext*);

    /* Used to delete cached primitive data (vertex buffers) */
    GLvoid (*deletePrimData)(__GLcontext*, GLvoid*);
    GLvoid (*deleteStreamInfo)(__GLcontext*, GLvoid*);

    /* Attributes */
    GLvoid (*drawBuffers)(__GLcontext*);
    GLvoid (*drawBuffer)(__GLcontext*);
    GLvoid (*readBuffer)(__GLcontext*);
    GLvoid (*colorMaterial)(__GLcontext*);
    GLvoid (*pixelStore)(__GLcontext*);
    GLvoid (*pixelTransfer)(__GLcontext*);
    GLvoid (*pixelMap)(__GLcontext*, GLenum);
    GLvoid (*pixelZoom)(__GLcontext*);

    /* Attribute Enable/Disable */
    GLvoid (*colorMaterialEndisable)(__GLcontext*);
    GLvoid (*colorTableEndisable)(__GLcontext*);
    GLvoid (*postConvColorTableEndisable)(__GLcontext*);
    GLvoid (*postColorMatrixColorTableEndisable)(__GLcontext*);
    GLvoid (*convolution1DEndisable)(__GLcontext*);
    GLvoid (*convolution2DEndisable)(__GLcontext*);
    GLvoid (*minmaxEndisable)(__GLcontext*);
    GLvoid (*histogramEndisable)(__GLcontext*);
    GLvoid (*separable2DEndisable)(__GLcontext*);

    /* Color table functions */
    GLvoid (*colorTable)(__GLcontext*, GLenum, GLenum, GLsizei, GLenum, GLenum, GLubyte*);
    GLvoid (*texColorTable)(__GLcontext*, GLenum, GLenum, GLsizei, GLenum, GLenum, GLubyte*, GLint);
    GLvoid (*convColorTable)(__GLcontext*, GLenum, GLenum, GLsizei, GLenum, GLenum, GLubyte*);
    GLvoid (*postColorMatrixColorTable)(__GLcontext*, GLenum, GLenum, GLsizei, GLenum, GLenum, GLubyte*);

    GLvoid (*colorSubTable)(__GLcontext*, GLenum, GLsizei, GLsizei, GLenum, GLenum, GLubyte*);
    GLvoid (*texColorSubTable)(__GLcontext*, GLenum, GLsizei, GLsizei, GLenum, GLenum, GLubyte*, GLint);
    GLvoid (*convColorSubTable)(__GLcontext*, GLenum, GLsizei, GLsizei, GLenum, GLenum, GLubyte*);
    GLvoid (*postColorMatrixColorSubTable)(__GLcontext*, GLenum, GLsizei, GLsizei, GLenum, GLenum, GLubyte*);

    GLvoid (*copyColorTable)(__GLcontext*, GLenum, GLenum, GLint, GLint, GLsizei);
    GLvoid (*copyTexColorTable)(__GLcontext*, GLenum, GLenum, GLint, GLint, GLsizei, GLint);
    GLvoid (*copyConvColorTable)(__GLcontext*, GLenum, GLenum, GLint, GLint, GLsizei);
    GLvoid (*copyPostColorMatrixColorTable)(__GLcontext*, GLenum, GLenum, GLint, GLint, GLsizei);

    GLvoid (*copyColorSubTable)(__GLcontext*, GLenum, GLsizei, GLint, GLint, GLsizei);
    GLvoid (*copyTexColorSubTable)(__GLcontext*, GLenum, GLsizei, GLint, GLint, GLsizei, GLint);
    GLvoid (*copyConvColorSubTable)(__GLcontext*, GLenum, GLsizei, GLint, GLint, GLsizei);
    GLvoid (*copyPostColorMatrixColorSubTable)(__GLcontext*, GLenum, GLsizei, GLint, GLint, GLsizei);

    /* Texture functions */
    GLvoid (*bindTexture)(__GLcontext*, __GLtextureObject*);
    GLvoid (*deleteTexture)(__GLcontext*, __GLtextureObject*);
    GLboolean (*isTextureResident)(__GLcontext*, __GLtextureObject*);

    GLvoid (*getTexImage)(__GLcontext*, __GLtextureObject*, GLint, GLenum, GLenum, GLvoid*, GLint, GLenum);
    GLvoid (*getCompressedTexImage)(__GLcontext*, __GLtextureObject*, GLint, GLvoid*, GLint);

    /* Query HW supported texture format */
    GLint (*querySupportedCompressedTextureFormats)(__GLcontext*, GLint*);

    /* Generate a level from the base level */
    GLboolean (*generateMipmaps)(__GLcontext*, __GLtextureObject*, GLint, GLint);

    GLvoid (*texImage1D)(__GLcontext*, __GLtextureObject*, GLint, const GLvoid*);
    GLvoid (*texImage2D)(__GLcontext*, __GLtextureObject*, GLint, GLint, const GLvoid*);
    GLvoid (*texImage3D)(__GLcontext*, __GLtextureObject*, GLint, const GLvoid*);

    GLvoid (*compressedTexImage1D)(__GLcontext*, __GLtextureObject*, GLint, const GLvoid*);
    GLvoid (*compressedTexImage2D)(__GLcontext*, __GLtextureObject*, GLint, GLint, const GLvoid*);
    GLvoid (*compressedTexImage3D)(__GLcontext*, __GLtextureObject*, GLint, const GLvoid*);

    GLboolean (*copyTexImage1D)(__GLcontext*, __GLtextureObject*, GLint, GLint, GLint);
    GLboolean (*copyTexImage2D)(__GLcontext*, __GLtextureObject*, GLint, GLint, GLint, GLint);

    GLvoid (*texSubImage1D)(__GLcontext*, __GLtextureObject*, GLint, GLint, GLint, const GLvoid*);
    GLvoid (*texSubImage2D)(__GLcontext*, __GLtextureObject*, GLint, GLint, GLint, GLint, GLint, GLint, const GLvoid*);
    GLvoid (*texSubImage3D)(__GLcontext*, __GLtextureObject*, GLint, GLint, GLint, GLint, GLint, GLint, GLint, const GLvoid*);

    GLvoid (*compressedTexSubImage1D)(__GLcontext*, __GLtextureObject*, GLint, GLint, GLint, GLint, const GLvoid*);
    GLvoid (*compressedTexSubImage2D)(__GLcontext*, __GLtextureObject*, GLint, GLint, GLint, GLint, GLint, GLint, const GLvoid*);
    GLvoid (*compressedTexSubImage3D)(__GLcontext*, __GLtextureObject*, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, const GLvoid*);

    GLboolean (*copyTexSubImage1D)(__GLcontext*, __GLtextureObject*, GLint, GLint, GLint, GLint, GLint);
    GLboolean (*copyTexSubImage2D)(__GLcontext*, __GLtextureObject*, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint);
    GLboolean (*copyTexSubImage3D)(__GLcontext*, __GLtextureObject*, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint);

    GLvoid (*activeTexture)(__GLcontext*, GLuint);
    GLvoid (*clientActiveTexture)(__GLcontext*, GLuint);
    GLboolean (*syncTextureFromDeviceMemory)(__GLcontext*, __GLtextureObject*, GLuint);

    GLvoid (*texParameter)(__GLcontext*, __GLtextureObject*, GLenum, GLfloat*);

    /* Get functions */
    GLvoid (*get)(__GLcontext*, GLenum, GLint*);
    GLvoid (*getTexLevelParameter)(__GLcontext*, __GLtextureObject*, GLint, GLenum, GLenum, GLfloat*);
    GLvoid (*getTexParameter)(__GLcontext*, __GLtextureObject*, GLenum, GLfloat*);

    /* Imaging extensions */
    GLvoid (*histogram)(__GLcontext*, GLenum, GLsizei, GLenum, GLboolean);
    GLvoid (*resetHistogram)(__GLcontext*, GLenum);
    GLvoid (*minmax)(__GLcontext*, GLenum, GLenum, GLboolean);
    GLvoid (*resetMinmax)(__GLcontext*, GLenum);
    GLvoid (*convolutionFilter1D)(__GLcontext*, GLenum, GLenum, GLsizei, GLenum, GLenum, GLvoid*);
    GLvoid (*convolutionFilter2D)(__GLcontext*, GLenum, GLenum, GLsizei, GLenum, GLenum, GLvoid*);
    GLvoid (*separableFilter2D)(__GLcontext*, GLenum, GLenum, GLsizei, GLenum, GLenum, GLvoid*, GLvoid*);
    GLvoid (*copyConvolutionFilter1D)(__GLcontext*, GLenum, GLenum, GLint, GLint, GLsizei);
    GLvoid (*copyConvolutionFilter2D)(__GLcontext*, GLenum, GLenum, GLint, GLint, GLsizei, GLsizei);

    /* Vertex buffer object */
    GLvoid (*bindBuffer)(__GLcontext*, __GLbufferObject*, GLuint);
    GLboolean (*bufferData)(__GLcontext*, __GLbufferObject*, GLuint, const GLvoid*);
    GLboolean (*bufferSubData)(__GLcontext*, __GLbufferObject*, GLuint, GLintptr, GLsizeiptr, const GLvoid*);
    GLvoid * (*mapBuffer)(__GLcontext*, __GLbufferObject*);
    GLboolean (*unmapBuffer)(__GLcontext*, __GLbufferObject*);
    GLvoid (*deleteBuffer)(__GLcontext*, __GLbufferObject*);
    GLvoid (*getBufferSubData)(__GLcontext*, __GLbufferObject*, GLintptr, GLsizeiptr, GLvoid*);

    /* Occlusion query */
    GLvoid (*beginQuery)(__GLcontext*, __GLqueryObject*);
    GLvoid (*endQuery)(__GLcontext*, __GLqueryObject*);
    GLvoid (*getQueryObject)(__GLcontext*, GLenum, __GLqueryObject*);
    GLvoid (*deleteQuery)(__GLcontext*, __GLqueryObject*);

    /* Flush, Finish */
    GLvoid (*flush)(__GLcontext*);
    GLvoid (*finish)(__GLcontext*);
    GLvoid (*clear)(__GLcontext*, GLbitfield);

    /* Accumulation functions */
    GLvoid (*accum)(__GLcontext*, GLenum , GLfloat);

    /* Program */
    GLboolean (*ProgramStringARB)(__GLcontext*, __GLProgramObject*);
    GLboolean (*BindProgramARB)(__GLcontext*, __GLProgramObject*, GLvoid**);
    GLvoid (*DeleteProgramARB)(__GLcontext*, GLvoid**);

    /* OpenGL 2.0 GLSL APIs */
    GLvoid (*deleteShader)(__GLcontext*, __GLshaderObject*);
    GLboolean (*compileShader)(__GLcontext*, __GLshaderObject*);
    GLboolean (*linkProgram)(__GLcontext*, __GLshaderProgramObject*);
    GLboolean (*useProgram)(__GLcontext*, __GLshaderProgramObject*, GLboolean*);
    GLboolean (*validateShaderProgram)(__GLcontext*, __GLshaderProgramObject*);
    GLvoid (*deleteShaderProgram)(__GLcontext*, GLvoid**);
    GLvoid (*getActiveUniform)(__GLcontext *, __GLshaderProgramObject *, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, char *);
    GLboolean (*getUniformLocation)(__GLcontext *, __GLshaderProgramObject *, const GLchar *, GLuint, GLuint, GLboolean, GLint *);
    GLboolean (*bindAttributeLocation)(__GLcontext *, __GLshaderProgramObject *, GLuint, const GLchar *);
    GLint (*getAttributeLocation)(__GLcontext *, __GLshaderProgramObject *, const GLchar *);
    GLvoid (*getActiveAttribute)(__GLcontext *, __GLshaderProgramObject *, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, char *);
    GLint (*uniforms)(__GLcontext *, __GLshaderProgramObject *, GLint, GLint, GLsizei, const void *, GLboolean);
    GLboolean (*getUniforms)(__GLcontext *, __GLshaderProgramObject *, GLint, GLint, GLvoid *);
    GLvoid (*buildTextureEnableDim)(__GLcontext *);
    GLboolean (*checkTextureConflict)(__GLcontext *, __GLshaderProgramObject *);
    GLvoid * (*createConstantBuffer)(__GLcontext*, GLuint);
    GLvoid (*destroyConstantBuffer)(__GLcontext*, GLvoid*);
    GLvoid (*syncConstantBuffer)(__GLcontext*, __GLconstantBuffer*);
    GLboolean (*constantBufferMakeResident)(__GLcontext*, __GLconstantBuffer*);

    /* GL_EXT_framebuffer_object */
    union {
    GLvoid (*bindFramebuffer)(__GLcontext*,  __GLframebufferObject*, __GLframebufferObject*);
    GLvoid (*bindDrawFramebuffer)(__GLcontext*, __GLframebufferObject*, __GLframebufferObject*);
    };
    GLvoid (*bindReadFramebuffer)(__GLcontext*, __GLframebufferObject*, __GLframebufferObject*);
    GLvoid (*bindRenderbuffer)(__GLcontext*, __GLrenderbufferObject*);
    GLvoid (*deleteRenderbuffer)(__GLcontext*, __GLrenderbufferObject*);
    GLboolean (*renderbufferStorage)(__GLcontext*, __GLrenderbufferObject*);
    GLvoid (*blitFramebuffer)(__GLcontext*, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum);
    GLvoid (*frameBufferTexture)(__GLcontext *, __GLframebufferObject*, GLint, __GLtextureObject *, GLint, GLint, GLint, GLboolean);
    GLvoid (*framebufferRenderbuffer)(__GLcontext *, __GLframebufferObject*, GLint, __GLrenderbufferObject *);
    GLboolean (*isFramebufferComplete)(__GLcontext *, __GLframebufferObject *);

    /* WGL_EXT_swap_control */
    GLboolean (*swapInterval)(__GLcontext*, GLint, GLuint);

    /* Render texture */
    GLboolean (*bindTexImageARB)(__GLcontext*, __GLdrawablePrivate*, __GLdrawableBuffer*, __GLtextureObject*);
    GLboolean (*releaseTexImageARB)(__GLcontext*, __GLtextureObject*);

    /* EXT_texture_buffer_object */
    GLvoid (*texBufferEXT)(__GLcontext*, __GLtextureObject *, GLint);

    /* These two interfaces are used by SWP to lock/unlock surface */
    GLvoid (*lockBuffer)(__GLcontext*, GLvoid*, GLuint, GLuint**, GLuint*);
    GLvoid (*unlockBuffer)(__GLcontext*, GLvoid*, GLuint);

    /* Free vertex data (IM or DL) cached in video memory */
    GLboolean (*updatePrivateData)(__GLcontext*, GLvoid*, GLuint);

    /* Sync the front buffer to the fake front buffer */
    GLvoid (*syncFrontToFakeFront)(__GLcontext*);

    GLvoid (*unBindDpDrawable)(__GLcontext*);

    /* Query whether hw support pixel buffer format */
    GLboolean (*queryPixelBufferFormat)(__GLcontext *, GLenum, GLenum, GLuint*, GLuint *);

    /* Update shading mode on hash table. */
    GLvoid (*updateShadingMode)(__GLcontext*, GLenum);

} __GLdevicePipeline;


typedef struct __GLswPipelineRec {

    /* SW private context management functions */
    __GLpipeline ctx;

    /* Draw primitive begin */
    GLvoid (*begin)(__GLcontext*, GLenum);

    /* Clear sw raster infos for this texObj. */
    GLvoid (*deleteTexture)(__GLcontext*, __GLtextureObject*);

} __GLswPipeline;

#endif /* __gc_gl_devicepipe_h_ */
