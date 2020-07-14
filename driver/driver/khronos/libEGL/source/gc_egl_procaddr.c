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


#ifndef _GNU_SOURCE
#  define _GNU_SOURCE
#endif


#include "gc_egl_precomp.h"

/* Zone used for header/footer. */
#define _GC_OBJ_ZONE    gcdZONE_EGL_API

EGLAPI EGLBoolean EGLAPIENTRY
eglPatchID(
    EGLenum *PatchID,
    EGLBoolean Set
    )
{
#if gcdENABLE_3D

    gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D);

    if (Set)
    {
        gcoHAL_SetGlobalPatchID(gcvNULL, *PatchID);
    }
    else
    {
        gcoHAL_GetPatchID(gcvNULL, (gcePATCH_ID*)PatchID);
    }
#endif

    return EGL_TRUE;
}

/*
 * XXX: Following typedefs and definitions are copied from GLES header file.
 * Maybe include GLES header file directly is better.
 *
 * Define data types here because 'gc_egl.h' is incorrectly included by GLES
 * driver. Duplicated typedefs may cause problems on some compilers.
 */
typedef void GLvoid;
typedef unsigned int GLenum;
typedef khronos_float_t GLfloat;
typedef khronos_int32_t GLfixed;
typedef unsigned int GLuint;
typedef khronos_ssize_t GLsizeiptr;
typedef khronos_intptr_t GLintptr;
typedef unsigned int GLbitfield;
typedef int GLint;
typedef khronos_uint8_t GLubyte;
typedef unsigned char GLboolean;
typedef int GLsizei;
typedef khronos_int32_t GLclampx;
typedef unsigned short   GLushort;
typedef char GLchar;
typedef struct __GLsync *GLsync;
typedef khronos_int64_t GLint64;
typedef khronos_uint64_t GLuint64;
typedef float        GLclampf;

typedef void *GLeglImageOES;

#ifndef GL_API
#define GL_API      KHRONOS_APICALL
#endif

#ifndef GL_APIENTRY
#define GL_APIENTRY KHRONOS_APIENTRY
#endif

typedef void  (GL_APIENTRY  *GLDEBUGPROCKHR)(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam);
/*
 * OpenGL/OpenGL ES Client API entries for eglGetProcAddress.
 * This table only includes APIs with same name in es11 and es3x.
 */
#define GL_API_ENTRIES(GL_ENTRY) \
    GL_ENTRY(void, glActiveTexture, GLenum texture) \
    GL_ENTRY(void, glBindBuffer, GLenum target, GLuint buffer) \
    GL_ENTRY(void, glBindTexture, GLenum target, GLuint texture) \
    GL_ENTRY(void, glBlendFunc, GLenum sfactor, GLenum dfactor) \
    GL_ENTRY(void, glBufferData, GLenum target, GLsizeiptr size, const void * data, GLenum usage) \
    GL_ENTRY(void, glBufferSubData, GLenum target, GLintptr offset, GLsizeiptr size, const void * data) \
    GL_ENTRY(void, glClear, GLbitfield mask) \
    GL_ENTRY(void, glClearColor, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) \
    GL_ENTRY(void, glClearDepthf, GLfloat d) \
    GL_ENTRY(void, glClearStencil, GLint s) \
    GL_ENTRY(void, glColorMask, GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) \
    GL_ENTRY(void, glCompressedTexImage2D, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void * data) \
    GL_ENTRY(void, glCompressedTexSubImage2D, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void * data) \
    GL_ENTRY(void, glCopyTexImage2D, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) \
    GL_ENTRY(void, glCopyTexSubImage2D, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) \
    GL_ENTRY(void, glCullFace, GLenum mode) \
    GL_ENTRY(void, glDeleteBuffers, GLsizei n, const GLuint * buffers) \
    GL_ENTRY(void, glDeleteTextures, GLsizei n, const GLuint * textures) \
    GL_ENTRY(void, glDepthFunc, GLenum func) \
    GL_ENTRY(void, glDepthMask, GLboolean flag) \
    GL_ENTRY(void, glDepthRangef, GLfloat n, GLfloat f) \
    GL_ENTRY(void, glDisable, GLenum cap) \
    GL_ENTRY(void, glDrawArrays, GLenum mode, GLint first, GLsizei count) \
    GL_ENTRY(void, glDrawElements, GLenum mode, GLsizei count, GLenum type, const void * indices) \
    GL_ENTRY(void, glEnable, GLenum cap) \
    GL_ENTRY(void, glFinish, void) \
    GL_ENTRY(void, glFlush, void) \
    GL_ENTRY(void, glFrontFace, GLenum mode) \
    GL_ENTRY(void, glGenBuffers, GLsizei n, GLuint * buffers) \
    GL_ENTRY(void, glGenTextures, GLsizei n, GLuint * textures) \
    GL_ENTRY(void, glGetBooleanv, GLenum pname, GLboolean * data) \
    GL_ENTRY(void, glGetBufferParameteriv, GLenum target, GLenum pname, GLint * params) \
    GL_ENTRY(GLenum, glGetError, void) \
    GL_ENTRY(void, glGetFloatv, GLenum pname, GLfloat * data) \
    GL_ENTRY(void, glGetIntegerv, GLenum pname, GLint * data) \
    GL_ENTRY(void, glGetPointerv, GLenum pname, void ** params) \
    GL_ENTRY(const GLubyte *, glGetString, GLenum name) \
    GL_ENTRY(void, glGetTexParameterfv, GLenum target, GLenum pname, GLfloat * params) \
    GL_ENTRY(void, glGetTexParameteriv, GLenum target, GLenum pname, GLint * params) \
    GL_ENTRY(void, glHint, GLenum target, GLenum mode) \
    GL_ENTRY(GLboolean, glIsBuffer, GLuint buffer) \
    GL_ENTRY(GLboolean, glIsEnabled, GLenum cap) \
    GL_ENTRY(GLboolean, glIsTexture, GLuint texture) \
    GL_ENTRY(void, glLineWidth, GLfloat width) \
    GL_ENTRY(void, glPixelStorei, GLenum pname, GLint param) \
    GL_ENTRY(void, glPolygonOffset, GLfloat factor, GLfloat units) \
    GL_ENTRY(void, glReadPixels, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void * pixels) \
    GL_ENTRY(void, glSampleCoverage, GLfloat value, GLboolean invert) \
    GL_ENTRY(void, glScissor, GLint x, GLint y, GLsizei width, GLsizei height) \
    GL_ENTRY(void, glStencilFunc, GLenum func, GLint ref, GLuint mask) \
    GL_ENTRY(void, glStencilMask, GLuint mask) \
    GL_ENTRY(void, glStencilOp, GLenum fail, GLenum zfail, GLenum zpass) \
    GL_ENTRY(void, glTexImage2D, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void * pixels) \
    GL_ENTRY(void, glTexParameterf, GLenum target, GLenum pname, GLfloat param) \
    GL_ENTRY(void, glTexParameterfv, GLenum target, GLenum pname, const GLfloat * params) \
    GL_ENTRY(void, glTexParameteri, GLenum target, GLenum pname, GLint param) \
    GL_ENTRY(void, glTexParameteriv, GLenum target, GLenum pname, const GLint * params) \
    GL_ENTRY(void, glTexSubImage2D, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void * pixels) \
    GL_ENTRY(void, glViewport, GLint x, GLint y, GLsizei width, GLsizei height) \
    GL_ENTRY(void, glEGLImageTargetRenderbufferStorageOES, GLenum target, GLeglImageOES image) \
    GL_ENTRY(void, glEGLImageTargetTexture2DOES, GLenum target, GLeglImageOES image) \
    GL_ENTRY(void, glGetBufferPointervOES, GLenum target, GLenum pname, void ** params) \
    GL_ENTRY(void *, glMapBufferOES, GLenum target, GLenum access) \
    GL_ENTRY(GLboolean, glUnmapBufferOES, GLenum target) \
    GL_ENTRY(void, glMultiDrawArraysEXT, GLenum mode, const GLint * first, const GLsizei * count, GLsizei primcount) \
    GL_ENTRY(void, glMultiDrawElementsEXT, GLenum mode, const GLsizei * count, GLenum type, const void *const* indices, GLsizei primcount) \
    GL_ENTRY(void, glDiscardFramebufferEXT, GLenum target, GLsizei numAttachments, const GLenum *attachments) \
    GL_ENTRY(void, glRenderbufferStorageMultisampleEXT, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height) \
    GL_ENTRY(void, glFramebufferTexture2DMultisampleEXT, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples) \
    GL_ENTRY(void, glGetProgramBinaryOES, GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary) \
    GL_ENTRY(void, glProgramBinaryOES, GLuint program, GLenum binaryFormat, const void *binary, GLint length) \
    GL_ENTRY(void, glTexDirectVIV, GLenum Target, GLsizei Width, GLsizei Height, GLenum Format, GLvoid ** Pixels) \
    GL_ENTRY(void, glTexDirectVIVMap, GLenum Target, GLsizei Width, GLsizei Height, GLenum Format, GLvoid ** Logical, const GLuint * Physical) \
    GL_ENTRY(void, glTexDirectMapVIV, GLenum Target, GLsizei Width, GLsizei Height, GLenum Format, GLvoid ** Logical, const GLuint * Physical) \
    GL_ENTRY(void, glTexDirectTiledMapVIV, GLenum Target, GLsizei Width, GLsizei Height, GLenum Format, GLvoid ** Logical, const GLuint * Physical) \
    GL_ENTRY(void, glTexDirectInvalidateVIV, GLenum Target) \
    GL_ENTRY(void, glAttachShader, GLuint program, GLuint shader) \
    GL_ENTRY(void, glBindAttribLocation, GLuint program, GLuint index, const GLchar* name) \
    GL_ENTRY(void, glBindFramebuffer, GLenum target, GLuint framebuffer) \
    GL_ENTRY(void, glBindRenderbuffer, GLenum target, GLuint renderbuffer) \
    GL_ENTRY(void, glBlendColor, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) \
    GL_ENTRY(void, glBlendEquation, GLenum mode) \
    GL_ENTRY(void, glBlendEquationSeparate, GLenum modeRGB, GLenum modeAlpha) \
    GL_ENTRY(void, glBlendFuncSeparate, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) \
    GL_ENTRY(GLenum, glCheckFramebufferStatus, GLenum target) \
    GL_ENTRY(void, glCompileShader, GLuint shader) \
    GL_ENTRY(GLuint, glCreateProgram, void) \
    GL_ENTRY(GLuint, glCreateShader, GLenum type) \
    GL_ENTRY(void, glDeleteFramebuffers, GLsizei n, const GLuint* framebuffers) \
    GL_ENTRY(void, glDeleteProgram, GLuint program) \
    GL_ENTRY(void, glDeleteRenderbuffers, GLsizei n, const GLuint* renderbuffers) \
    GL_ENTRY(void, glDeleteShader, GLuint shader) \
    GL_ENTRY(void, glDetachShader, GLuint program, GLuint shader) \
    GL_ENTRY(void, glDisableVertexAttribArray, GLuint index) \
    GL_ENTRY(void, glEnableVertexAttribArray, GLuint index) \
    GL_ENTRY(void, glFramebufferRenderbuffer, GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) \
    GL_ENTRY(void, glFramebufferTexture2D, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) \
    GL_ENTRY(void, glGenerateMipmap, GLenum target) \
    GL_ENTRY(void, glGenFramebuffers, GLsizei n, GLuint* framebuffers) \
    GL_ENTRY(void, glGenRenderbuffers, GLsizei n, GLuint* renderbuffers) \
    GL_ENTRY(void, glGetActiveAttrib, GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name) \
    GL_ENTRY(void, glGetActiveUniform, GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name) \
    GL_ENTRY(void, glGetAttachedShaders, GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders) \
    GL_ENTRY(GLint, glGetAttribLocation, GLuint program, const GLchar* name) \
    GL_ENTRY(void, glGetFramebufferAttachmentParameteriv, GLenum target, GLenum attachment, GLenum pname, GLint* params) \
    GL_ENTRY(void, glGetProgramiv, GLuint program, GLenum pname, GLint* params) \
    GL_ENTRY(void, glGetProgramInfoLog, GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog) \
    GL_ENTRY(void, glGetRenderbufferParameteriv, GLenum target, GLenum pname, GLint* params) \
    GL_ENTRY(void, glGetShaderiv, GLuint shader, GLenum pname, GLint* params) \
    GL_ENTRY(void, glGetShaderInfoLog, GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog) \
    GL_ENTRY(void, glGetShaderPrecisionFormat, GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision) \
    GL_ENTRY(void, glGetShaderSource, GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source) \
    GL_ENTRY(void, glGetUniformfv, GLuint program, GLint location, GLfloat* params) \
    GL_ENTRY(void, glGetUniformiv, GLuint program, GLint location, GLint* params) \
    GL_ENTRY(GLint, glGetUniformLocation, GLuint program, const GLchar* name) \
    GL_ENTRY(void, glGetVertexAttribfv, GLuint index, GLenum pname, GLfloat* params) \
    GL_ENTRY(void, glGetVertexAttribiv, GLuint index, GLenum pname, GLint* params) \
    GL_ENTRY(void, glGetVertexAttribPointerv, GLuint index, GLenum pname, GLvoid** pointer) \
    GL_ENTRY(GLboolean, glIsFramebuffer, GLuint framebuffer) \
    GL_ENTRY(GLboolean, glIsProgram, GLuint program) \
    GL_ENTRY(GLboolean, glIsRenderbuffer, GLuint renderbuffer) \
    GL_ENTRY(GLboolean, glIsShader, GLuint shader) \
    GL_ENTRY(void, glLinkProgram, GLuint program) \
    GL_ENTRY(void, glReleaseShaderCompiler, void) \
    GL_ENTRY(void, glRenderbufferStorage, GLenum target, GLenum internalformat, GLsizei width, GLsizei height) \
    GL_ENTRY(void, glShaderBinary, GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length) \
    GL_ENTRY(void, glShaderSource, GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length) \
    GL_ENTRY(void, glStencilFuncSeparate, GLenum face, GLenum func, GLint ref, GLuint mask) \
    GL_ENTRY(void, glStencilMaskSeparate, GLenum face, GLuint mask) \
    GL_ENTRY(void, glStencilOpSeparate, GLenum face, GLenum fail, GLenum zfail, GLenum zpass) \
    GL_ENTRY(void, glUniform1f, GLint location, GLfloat x) \
    GL_ENTRY(void, glUniform1fv, GLint location, GLsizei count, const GLfloat* v) \
    GL_ENTRY(void, glUniform1i, GLint location, GLint x) \
    GL_ENTRY(void, glUniform1iv, GLint location, GLsizei count, const GLint* v) \
    GL_ENTRY(void, glUniform2f, GLint location, GLfloat x, GLfloat y) \
    GL_ENTRY(void, glUniform2fv, GLint location, GLsizei count, const GLfloat* v) \
    GL_ENTRY(void, glUniform2i, GLint location, GLint x, GLint y) \
    GL_ENTRY(void, glUniform2iv, GLint location, GLsizei count, const GLint* v) \
    GL_ENTRY(void, glUniform3f, GLint location, GLfloat x, GLfloat y, GLfloat z) \
    GL_ENTRY(void, glUniform3fv, GLint location, GLsizei count, const GLfloat* v) \
    GL_ENTRY(void, glUniform3i, GLint location, GLint x, GLint y, GLint z) \
    GL_ENTRY(void, glUniform3iv, GLint location, GLsizei count, const GLint* v) \
    GL_ENTRY(void, glUniform4f, GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w) \
    GL_ENTRY(void, glUniform4fv, GLint location, GLsizei count, const GLfloat* v) \
    GL_ENTRY(void, glUniform4i, GLint location, GLint x, GLint y, GLint z, GLint w) \
    GL_ENTRY(void, glUniform4iv, GLint location, GLsizei count, const GLint* v) \
    GL_ENTRY(void, glUniformMatrix2fv, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) \
    GL_ENTRY(void, glUniformMatrix3fv, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) \
    GL_ENTRY(void, glUniformMatrix4fv, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) \
    GL_ENTRY(void, glUseProgram, GLuint program) \
    GL_ENTRY(void, glValidateProgram, GLuint program) \
    GL_ENTRY(void, glVertexAttrib1f, GLuint indx, GLfloat x) \
    GL_ENTRY(void, glVertexAttrib1fv, GLuint indx, const GLfloat* values) \
    GL_ENTRY(void, glVertexAttrib2f, GLuint indx, GLfloat x, GLfloat y) \
    GL_ENTRY(void, glVertexAttrib2fv, GLuint indx, const GLfloat* values) \
    GL_ENTRY(void, glVertexAttrib3f, GLuint indx, GLfloat x, GLfloat y, GLfloat z) \
    GL_ENTRY(void, glVertexAttrib3fv, GLuint indx, const GLfloat* values) \
    GL_ENTRY(void, glVertexAttrib4f, GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w) \
    GL_ENTRY(void, glVertexAttrib4fv, GLuint indx, const GLfloat* values) \
    GL_ENTRY(void, glVertexAttribPointer, GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr) \
    /* OpenGL ES 3.0 */ \
    GL_ENTRY(void, glReadBuffer, GLenum mode) \
    GL_ENTRY(void, glDrawRangeElements, GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices) \
    GL_ENTRY(void, glTexImage3D, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels) \
    GL_ENTRY(void, glTexSubImage3D, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels) \
    GL_ENTRY(void, glCopyTexSubImage3D, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height) \
    GL_ENTRY(void, glCompressedTexImage3D, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data) \
    GL_ENTRY(void, glCompressedTexSubImage3D, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data) \
    GL_ENTRY(void, glGenQueries, GLsizei n, GLuint* ids) \
    GL_ENTRY(void, glDeleteQueries, GLsizei n, const GLuint* ids) \
    GL_ENTRY(GLboolean, glIsQuery, GLuint id) \
    GL_ENTRY(void, glBeginQuery, GLenum target, GLuint id) \
    GL_ENTRY(void, glEndQuery, GLenum target) \
    GL_ENTRY(void, glGetQueryiv, GLenum target, GLenum pname, GLint* params) \
    GL_ENTRY(void, glGetQueryObjectuiv, GLuint id, GLenum pname, GLuint* params) \
    GL_ENTRY(GLboolean, glUnmapBuffer, GLenum target) \
    GL_ENTRY(void, glGetBufferPointerv, GLenum target, GLenum pname, GLvoid** params) \
    GL_ENTRY(void, glDrawBuffers, GLsizei n, const GLenum* bufs) \
    GL_ENTRY(void, glUniformMatrix2x3fv, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) \
    GL_ENTRY(void, glUniformMatrix3x2fv, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) \
    GL_ENTRY(void, glUniformMatrix2x4fv, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) \
    GL_ENTRY(void, glUniformMatrix4x2fv, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) \
    GL_ENTRY(void, glUniformMatrix3x4fv, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) \
    GL_ENTRY(void, glUniformMatrix4x3fv, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) \
    GL_ENTRY(void, glBlitFramebuffer, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter) \
    GL_ENTRY(void, glRenderbufferStorageMultisample, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height) \
    GL_ENTRY(void, glFramebufferTextureLayer, GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer) \
    GL_ENTRY(GLvoid*, glMapBufferRange, GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access) \
    GL_ENTRY(void, glFlushMappedBufferRange, GLenum target, GLintptr offset, GLsizeiptr length) \
    GL_ENTRY(void, glBindVertexArray, GLuint array) \
    GL_ENTRY(void, glDeleteVertexArrays, GLsizei n, const GLuint* arrays) \
    GL_ENTRY(void, glGenVertexArrays, GLsizei n, GLuint* arrays) \
    GL_ENTRY(GLboolean, glIsVertexArray, GLuint array) \
    GL_ENTRY(void, glGetIntegeri_v, GLenum target, GLuint index, GLint* data) \
    GL_ENTRY(void, glBeginTransformFeedback, GLenum primitiveMode) \
    GL_ENTRY(void, glEndTransformFeedback, void) \
    GL_ENTRY(void, glBindBufferRange, GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size) \
    GL_ENTRY(void, glBindBufferBase, GLenum target, GLuint index, GLuint buffer) \
    GL_ENTRY(void, glTransformFeedbackVaryings, GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode) \
    GL_ENTRY(void, glGetTransformFeedbackVarying, GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name) \
    GL_ENTRY(void, glVertexAttribIPointer, GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer) \
    GL_ENTRY(void, glGetVertexAttribIiv, GLuint index, GLenum pname, GLint* params) \
    GL_ENTRY(void, glGetVertexAttribIuiv, GLuint index, GLenum pname, GLuint* params) \
    GL_ENTRY(void, glVertexAttribI4i, GLuint index, GLint x, GLint y, GLint z, GLint w) \
    GL_ENTRY(void, glVertexAttribI4ui, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w) \
    GL_ENTRY(void, glVertexAttribI4iv, GLuint index, const GLint* v) \
    GL_ENTRY(void, glVertexAttribI4uiv, GLuint index, const GLuint* v) \
    GL_ENTRY(void, glGetUniformuiv, GLuint program, GLint location, GLuint* params) \
    GL_ENTRY(GLint, glGetFragDataLocation, GLuint program, const GLchar *name) \
    GL_ENTRY(void, glUniform1ui, GLint location, GLuint v0) \
    GL_ENTRY(void, glUniform2ui, GLint location, GLuint v0, GLuint v1) \
    GL_ENTRY(void, glUniform3ui, GLint location, GLuint v0, GLuint v1, GLuint v2) \
    GL_ENTRY(void, glUniform4ui, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3) \
    GL_ENTRY(void, glUniform1uiv, GLint location, GLsizei count, const GLuint* value) \
    GL_ENTRY(void, glUniform2uiv, GLint location, GLsizei count, const GLuint* value) \
    GL_ENTRY(void, glUniform3uiv, GLint location, GLsizei count, const GLuint* value) \
    GL_ENTRY(void, glUniform4uiv, GLint location, GLsizei count, const GLuint* value) \
    GL_ENTRY(void, glClearBufferiv, GLenum buffer, GLint drawbuffer, const GLint* value) \
    GL_ENTRY(void, glClearBufferuiv, GLenum buffer, GLint drawbuffer, const GLuint* value) \
    GL_ENTRY(void, glClearBufferfv, GLenum buffer, GLint drawbuffer, const GLfloat* value) \
    GL_ENTRY(void, glClearBufferfi, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil) \
    GL_ENTRY(const GLubyte*, glGetStringi, GLenum name, GLuint index) \
    GL_ENTRY(void, glCopyBufferSubData, GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size) \
    GL_ENTRY(void, glGetUniformIndices, GLuint program, GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices) \
    GL_ENTRY(void, glGetActiveUniformsiv, GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params) \
    GL_ENTRY(GLuint, glGetUniformBlockIndex, GLuint program, const GLchar* uniformBlockName) \
    GL_ENTRY(void, glGetActiveUniformBlockiv, GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params) \
    GL_ENTRY(void, glGetActiveUniformBlockName, GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName) \
    GL_ENTRY(void, glUniformBlockBinding, GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding) \
    GL_ENTRY(void, glDrawArraysInstanced, GLenum mode, GLint first, GLsizei count, GLsizei instanceCount) \
    GL_ENTRY(void, glDrawElementsInstanced, GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei instanceCount) \
    GL_ENTRY(GLsync, glFenceSync, GLenum condition, GLbitfield flags) \
    GL_ENTRY(GLboolean, glIsSync, GLsync sync) \
    GL_ENTRY(void, glDeleteSync, GLsync sync) \
    GL_ENTRY(GLenum, glClientWaitSync, GLsync sync, GLbitfield flags, GLuint64 timeout) \
    GL_ENTRY(void, glWaitSync, GLsync sync, GLbitfield flags, GLuint64 timeout) \
    GL_ENTRY(void, glGetInteger64v, GLenum pname, GLint64* params) \
    GL_ENTRY(void, glGetSynciv, GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values) \
    GL_ENTRY(void, glGetInteger64i_v, GLenum target, GLuint index, GLint64* data) \
    GL_ENTRY(void, glGetBufferParameteri64v, GLenum target, GLenum pname, GLint64* params) \
    GL_ENTRY(void, glGenSamplers, GLsizei count, GLuint* samplers) \
    GL_ENTRY(void, glDeleteSamplers, GLsizei count, const GLuint* samplers) \
    GL_ENTRY(GLboolean, glIsSampler, GLuint sampler) \
    GL_ENTRY(void, glBindSampler, GLuint unit, GLuint sampler) \
    GL_ENTRY(void, glSamplerParameteri, GLuint sampler, GLenum pname, GLint param) \
    GL_ENTRY(void, glSamplerParameteriv, GLuint sampler, GLenum pname, const GLint* param) \
    GL_ENTRY(void, glSamplerParameterf, GLuint sampler, GLenum pname, GLfloat param) \
    GL_ENTRY(void, glSamplerParameterfv, GLuint sampler, GLenum pname, const GLfloat* param) \
    GL_ENTRY(void, glGetSamplerParameteriv, GLuint sampler, GLenum pname, GLint* params) \
    GL_ENTRY(void, glGetSamplerParameterfv, GLuint sampler, GLenum pname, GLfloat* params) \
    GL_ENTRY(void, glVertexAttribDivisor, GLuint index, GLuint divisor) \
    GL_ENTRY(void, glBindTransformFeedback, GLenum target, GLuint id) \
    GL_ENTRY(void, glDeleteTransformFeedbacks, GLsizei n, const GLuint* ids) \
    GL_ENTRY(void, glGenTransformFeedbacks, GLsizei n, GLuint* ids) \
    GL_ENTRY(GLboolean, glIsTransformFeedback, GLuint id) \
    GL_ENTRY(void, glPauseTransformFeedback, void) \
    GL_ENTRY(void, glResumeTransformFeedback, void) \
    GL_ENTRY(void, glGetProgramBinary, GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, GLvoid* binary) \
    GL_ENTRY(void, glProgramBinary, GLuint program, GLenum binaryFormat, const GLvoid* binary, GLsizei length) \
    GL_ENTRY(void, glProgramParameteri, GLuint program, GLenum pname, GLint value) \
    GL_ENTRY(void, glInvalidateFramebuffer, GLenum target, GLsizei numAttachments, const GLenum* attachments) \
    GL_ENTRY(void, glInvalidateSubFramebuffer, GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height) \
    GL_ENTRY(void, glTexStorage2D, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height) \
    GL_ENTRY(void, glTexStorage3D, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth) \
    GL_ENTRY(void, glGetInternalformativ, GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params) \
    /* OpenGL ES 3.1 */ \
    GL_ENTRY(void, glDispatchCompute, GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z) \
    GL_ENTRY(void, glDispatchComputeIndirect, GLintptr indirect) \
    GL_ENTRY(void, glDrawArraysIndirect, GLenum mode, const void *indirect) \
    GL_ENTRY(void, glDrawElementsIndirect, GLenum mode, GLenum type, const void *indirect) \
    GL_ENTRY(void, glFramebufferParameteri, GLenum target, GLenum pname, GLint param) \
    GL_ENTRY(void, glGetFramebufferParameteriv, GLenum target, GLenum pname, GLint *params) \
    GL_ENTRY(void, glGetProgramInterfaceiv, GLuint program, GLenum programInterface, GLenum pname, GLint *params) \
    GL_ENTRY(GLuint, glGetProgramResourceIndex, GLuint program, GLenum programInterface, const GLchar *name) \
    GL_ENTRY(void, glGetProgramResourceName, GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name) \
    GL_ENTRY(void, glGetProgramResourceiv, GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params) \
    GL_ENTRY(GLint, glGetProgramResourceLocation, GLuint program, GLenum programInterface, const GLchar *name) \
    GL_ENTRY(void, glUseProgramStages, GLuint pipeline, GLbitfield stages, GLuint program) \
    GL_ENTRY(void, glActiveShaderProgram, GLuint pipeline, GLuint program) \
    GL_ENTRY(GLuint, glCreateShaderProgramv, GLenum type, GLsizei count, const GLchar *const*strings) \
    GL_ENTRY(void, glBindProgramPipeline, GLuint pipeline) \
    GL_ENTRY(void, glDeleteProgramPipelines, GLsizei n, const GLuint *pipelines) \
    GL_ENTRY(void, glGenProgramPipelines, GLsizei n, GLuint *pipelines) \
    GL_ENTRY(GLboolean, glIsProgramPipeline, GLuint pipeline) \
    GL_ENTRY(void, glGetProgramPipelineiv, GLuint pipeline, GLenum pname, GLint *params) \
    GL_ENTRY(void, glProgramUniform1i, GLuint program, GLint location, GLint v0) \
    GL_ENTRY(void, glProgramUniform2i, GLuint program, GLint location, GLint v0, GLint v1) \
    GL_ENTRY(void, glProgramUniform3i, GLuint program, GLint location, GLint v0, GLint v1, GLint v2) \
    GL_ENTRY(void, glProgramUniform4i, GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3) \
    GL_ENTRY(void, glProgramUniform1ui, GLuint program, GLint location, GLuint v0) \
    GL_ENTRY(void, glProgramUniform2ui, GLuint program, GLint location, GLuint v0, GLuint v1) \
    GL_ENTRY(void, glProgramUniform3ui, GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2) \
    GL_ENTRY(void, glProgramUniform4ui, GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3) \
    GL_ENTRY(void, glProgramUniform1f, GLuint program, GLint location, GLfloat v0) \
    GL_ENTRY(void, glProgramUniform2f, GLuint program, GLint location, GLfloat v0, GLfloat v1) \
    GL_ENTRY(void, glProgramUniform3f, GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2) \
    GL_ENTRY(void, glProgramUniform4f, GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) \
    GL_ENTRY(void, glProgramUniform1iv, GLuint program, GLint location, GLsizei count, const GLint *value) \
    GL_ENTRY(void, glProgramUniform2iv, GLuint program, GLint location, GLsizei count, const GLint *value) \
    GL_ENTRY(void, glProgramUniform3iv, GLuint program, GLint location, GLsizei count, const GLint *value) \
    GL_ENTRY(void, glProgramUniform4iv, GLuint program, GLint location, GLsizei count, const GLint *value) \
    GL_ENTRY(void, glProgramUniform1uiv, GLuint program, GLint location, GLsizei count, const GLuint *value) \
    GL_ENTRY(void, glProgramUniform2uiv, GLuint program, GLint location, GLsizei count, const GLuint *value) \
    GL_ENTRY(void, glProgramUniform3uiv, GLuint program, GLint location, GLsizei count, const GLuint *value) \
    GL_ENTRY(void, glProgramUniform4uiv, GLuint program, GLint location, GLsizei count, const GLuint *value) \
    GL_ENTRY(void, glProgramUniform1fv, GLuint program, GLint location, GLsizei count, const GLfloat *value) \
    GL_ENTRY(void, glProgramUniform2fv, GLuint program, GLint location, GLsizei count, const GLfloat *value) \
    GL_ENTRY(void, glProgramUniform3fv, GLuint program, GLint location, GLsizei count, const GLfloat *value) \
    GL_ENTRY(void, glProgramUniform4fv, GLuint program, GLint location, GLsizei count, const GLfloat *value) \
    GL_ENTRY(void, glProgramUniformMatrix2fv, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
    GL_ENTRY(void, glProgramUniformMatrix3fv, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
    GL_ENTRY(void, glProgramUniformMatrix4fv, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
    GL_ENTRY(void, glProgramUniformMatrix2x3fv, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
    GL_ENTRY(void, glProgramUniformMatrix3x2fv, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
    GL_ENTRY(void, glProgramUniformMatrix2x4fv, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
    GL_ENTRY(void, glProgramUniformMatrix4x2fv, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
    GL_ENTRY(void, glProgramUniformMatrix3x4fv, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
    GL_ENTRY(void, glProgramUniformMatrix4x3fv, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
    GL_ENTRY(void, glValidateProgramPipeline, GLuint pipeline) \
    GL_ENTRY(void, glGetProgramPipelineInfoLog, GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog) \
    GL_ENTRY(void, glBindImageTexture, GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format) \
    GL_ENTRY(void, glGetBooleani_v, GLenum target, GLuint index, GLboolean *data) \
    GL_ENTRY(void, glMemoryBarrier, GLbitfield barriers) \
    GL_ENTRY(void, glMemoryBarrierByRegion, GLbitfield barriers) \
    GL_ENTRY(void, glTexStorage2DMultisample, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations) \
    GL_ENTRY(void, glGetMultisamplefv, GLenum pname, GLuint index, GLfloat *val) \
    GL_ENTRY(void, glSampleMaski, GLuint maskNumber, GLbitfield mask) \
    GL_ENTRY(void, glGetTexLevelParameteriv, GLenum target, GLint level, GLenum pname, GLint *params) \
    GL_ENTRY(void, glGetTexLevelParameterfv, GLenum target, GLint level, GLenum pname, GLfloat *params) \
    GL_ENTRY(void, glBindVertexBuffer, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride) \
    GL_ENTRY(void, glVertexAttribFormat, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset) \
    GL_ENTRY(void, glVertexAttribIFormat, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset) \
    GL_ENTRY(void, glVertexAttribBinding, GLuint attribindex, GLuint bindingindex) \
    GL_ENTRY(void, glVertexBindingDivisor, GLuint bindingindex, GLuint divisor) \
    /* OpenGL ES 3.2 */ \
    GL_ENTRY(void, glTexStorage3DMultisample, GLenum target, GLsizei samples, GLenum sizedinternalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations) \
    GL_ENTRY(void, glBlendBarrier, void) \
    GL_ENTRY(void, glDebugMessageControl, GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled) \
    GL_ENTRY(void, glDebugMessageInsert, GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* buf) \
    GL_ENTRY(void, glDebugMessageCallback, GLDEBUGPROCKHR callback, const GLvoid* userParam) \
    GL_ENTRY(GLuint, glGetDebugMessageLog, GLuint count, GLsizei bufSize, GLenum* sources, GLenum* types, GLuint* ids, GLenum* severities, GLsizei* lengths, GLchar* messageLog) \
    GL_ENTRY(void, glPushDebugGroup, GLenum source, GLuint id, GLsizei length, const GLchar * message) \
    GL_ENTRY(void, glPopDebugGroup, void) \
    GL_ENTRY(void, glObjectLabel, GLenum identifier, GLuint name, GLsizei length, const GLchar *label) \
    GL_ENTRY(void, glGetObjectLabel, GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label) \
    GL_ENTRY(void, glObjectPtrLabel, const GLvoid* ptr, GLsizei length, const GLchar *label) \
    GL_ENTRY(void, glGetObjectPtrLabel, const GLvoid* ptr, GLsizei bufSize, GLsizei *length, GLchar *label) \
    GL_ENTRY(GLenum, glGetGraphicsResetStatus, void) \
    GL_ENTRY(void, glReadnPixels, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, GLvoid *data) \
    GL_ENTRY(void, glGetnUniformfv, GLuint program, GLint location, GLsizei bufSize, GLfloat *params) \
    GL_ENTRY(void, glGetnUniformiv, GLuint program, GLint location, GLsizei bufSize, GLint *params) \
    GL_ENTRY(void, glGetnUniformuiv, GLuint program, GLint location, GLsizei bufSize, GLuint *params) \
    GL_ENTRY(void, glBlendEquationi, GLuint buf, GLenum mode) \
    GL_ENTRY(void, glBlendEquationSeparatei, GLuint buf, GLenum modeRGB, GLenum modeAlpha) \
    GL_ENTRY(void, glBlendFunci, GLuint buf, GLenum sfactor, GLenum dfactor) \
    GL_ENTRY(void, glBlendFuncSeparatei, GLuint buf, GLenum sfactorRGB,GLenum dfactorRGB,GLenum sfactorAlpha,GLenum dfactorAlpha) \
    GL_ENTRY(void, glColorMaski, GLuint buf, GLboolean r, GLboolean g, GLboolean b, GLboolean a) \
    GL_ENTRY(void, glEnablei, GLenum target, GLuint index) \
    GL_ENTRY(void, glDisablei, GLenum target, GLuint index) \
    GL_ENTRY(GLboolean, glIsEnabledi, GLenum target, GLuint index) \
    GL_ENTRY(void, glTexParameterIiv, GLenum target, GLenum pname, const GLint *params) \
    GL_ENTRY(void, glTexParameterIuiv, GLenum target, GLenum pname, const GLuint *params) \
    GL_ENTRY(void, glGetTexParameterIiv, GLenum target, GLenum pname, GLint *params) \
    GL_ENTRY(void, glGetTexParameterIuiv, GLenum target, GLenum pname, GLuint *params) \
    GL_ENTRY(void, glSamplerParameterIiv, GLuint sampler, GLenum pname, const GLint *param) \
    GL_ENTRY(void, glSamplerParameterIuiv, GLuint sampler, GLenum pname, const GLuint *param) \
    GL_ENTRY(void, glGetSamplerParameterIiv, GLuint sampler, GLenum pname, GLint *params) \
    GL_ENTRY(void, glGetSamplerParameterIuiv, GLuint sampler, GLenum pname, GLuint *params) \
    GL_ENTRY(void, glTexBuffer, GLenum target, GLenum internalformat, GLuint buffer) \
    GL_ENTRY(void, glTexBufferRange, GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size) \
    GL_ENTRY(void, glPatchParameteri, GLenum pname, GLint value) \
    GL_ENTRY(void, glFramebufferTexture, GLenum target, GLenum attachment, GLuint texture, GLint level) \
    GL_ENTRY(void, glMinSampleShading, GLfloat value) \
    GL_ENTRY(void, glCopyImageSubData, GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, \
                                                       GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ,  \
                                                       GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth) \
    GL_ENTRY(void, glDrawElementsBaseVertex, GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex) \
    GL_ENTRY(void, glDrawRangeElementsBaseVertex, GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex) \
    GL_ENTRY(void, glDrawElementsInstancedBaseVertex, GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex) \
    GL_ENTRY(void, glPrimitiveBoundingBox, GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW, GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW) \
    /* OpenGL ES extensions */ \
    /* GL_EXT_multi_draw_arrays && GL_EXT_draw_elements_base_vertex */ \
    GL_ENTRY(void, glMultiDrawElementsBaseVertexEXT, GLenum mode, const GLsizei *count, GLenum type, const void *const *indices, GLsizei drawcount, const GLint * basevertex) \
    /* GL_EXT_multi_draw_indirect */ \
    GL_ENTRY(void, glMultiDrawArraysIndirectEXT, GLenum mode, const void *indirect, GLsizei drawcount, GLsizei stride) \
    GL_ENTRY(void, glMultiDrawElementsIndirectEXT, GLenum mode, GLenum type, const void *indirect, GLsizei drawcount, GLsizei stride) \
    GL_ENTRY(void, glGetTexImage, GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels) \
    GL_ENTRY(void, glAlphaFunc, GLenum func, GLfloat ref) \
    GL_ENTRY(void, glClientActiveTexture, GLenum texture) \
    GL_ENTRY(void, glColor4f, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) \
    GL_ENTRY(void, glColor4ub, GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha) \
    GL_ENTRY(void, glColorPointer, GLint size, GLenum type, GLsizei stride, const void *pointer) \
    GL_ENTRY(void, glDisableClientState, GLenum array) \
    GL_ENTRY(void, glEnableClientState, GLenum array) \
    GL_ENTRY(void, glFogf, GLenum pname, GLfloat param) \
    GL_ENTRY(void, glFogfv, GLenum pname, const GLfloat *params) \
    GL_ENTRY(void, glGetLightfv, GLenum light, GLenum pname, GLfloat *params) \
    GL_ENTRY(void, glGetMaterialfv, GLenum face, GLenum pname, GLfloat *params) \
    GL_ENTRY(void, glGetTexEnvfv, GLenum target, GLenum pname, GLfloat *params) \
    GL_ENTRY(void, glGetTexEnviv, GLenum target, GLenum pname, GLint *params) \
    GL_ENTRY(void, glLightModelf, GLenum pname, GLfloat param) \
    GL_ENTRY(void, glLightModelfv, GLenum pname, const GLfloat *params) \
    GL_ENTRY(void, glLightf, GLenum light, GLenum pname, GLfloat param) \
    GL_ENTRY(void, glLightfv, GLenum light, GLenum pname, const GLfloat *params) \
    GL_ENTRY(void, glLoadIdentity, void) \
    GL_ENTRY(void, glLoadMatrixf, const GLfloat *m) \
    GL_ENTRY(void, glLogicOp, GLenum opcode) \
    GL_ENTRY(void, glMaterialf, GLenum face, GLenum pname, GLfloat param) \
    GL_ENTRY(void, glMaterialfv, GLenum face, GLenum pname, const GLfloat *params) \
    GL_ENTRY(void, glMatrixMode, GLenum mode) \
    GL_ENTRY(void, glMultMatrixf, const GLfloat *m) \
    GL_ENTRY(void, glMultiTexCoord4f, GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q) \
    GL_ENTRY(void, glNormal3f, GLfloat nx, GLfloat ny, GLfloat nz) \
    GL_ENTRY(void, glNormalPointer, GLenum type, GLsizei stride, const void *pointer) \
    GL_ENTRY(void, glPointParameterf, GLenum pname, GLfloat param) \
    GL_ENTRY(void, glPointParameterfv, GLenum pname, const GLfloat *params) \
    GL_ENTRY(void, glPointSize, GLfloat size) \
    GL_ENTRY(void, glPopMatrix, void) \
    GL_ENTRY(void, glPushMatrix, void) \
    GL_ENTRY(void, glRotatef, GLfloat angle, GLfloat x, GLfloat y, GLfloat z) \
    GL_ENTRY(void, glScalef, GLfloat x, GLfloat y, GLfloat z) \
    GL_ENTRY(void, glShadeModel, GLenum mode) \
    GL_ENTRY(void, glTexCoordPointer, GLint size, GLenum type, GLsizei stride, const void *pointer) \
    GL_ENTRY(void, glTexEnvf, GLenum target, GLenum pname, GLfloat param) \
    GL_ENTRY(void, glTexEnvfv, GLenum target, GLenum pname, const GLfloat *params) \
    GL_ENTRY(void, glTexEnvi, GLenum target, GLenum pname, GLint param) \
    GL_ENTRY(void, glTexEnviv, GLenum target, GLenum pname, const GLint *params) \
    GL_ENTRY(void, glTranslatef, GLfloat x, GLfloat y, GLfloat z) \
    GL_ENTRY(void, glVertexPointer, GLint size, GLenum type, GLsizei stride, const void *pointer)


#define apiNameEntry(__ret, __api, ...) #__api,

const char * glApiNames[] =
{
    GL_API_ENTRIES(apiNameEntry)
};

#define apiHookEntry(__ret, __api, ...) __ret (GL_APIENTRY * __api)(__VA_ARGS__);

typedef struct _veglGLAPIHook
{
    GL_API_ENTRIES(apiHookEntry)
}
veglGLAPIHook;

/* Global gl hooks. Do not need to be thread specific. */
static veglGLAPIHook glHooks[vegl_OPENVG - vegl_OPENGL_ES11];
static EGLBoolean    glHoolInitialized[vegl_OPENVG - vegl_OPENGL_ES11];

veglGLAPIHook *
_GetGLAPIHook(
    void
    )
{
    gctSIZE_T i;
    EGLint index = 0;
    gctHANDLE library;
    veglDISPATCH *dispatch = gcvNULL;
    __eglMustCastToProperFunctionPointerType *api;

    VEGLThreadData thread = veglGetThreadData();

    if (thread == gcvNULL)
    {
        return gcvNULL;
    }

    if (thread->esContext)
    {
        if (MAJOR_API_VER(thread->esContext->client) == 1)
        {
            index = (vegl_OPENGL_ES11 - vegl_OPENGL_ES11);
        }
        else
        {
            index = (vegl_OPENGL_ES20 - vegl_OPENGL_ES11);
        }
    }
    else if (thread->glContext)
    {
        index = (vegl_OPENGL - vegl_OPENGL_ES11);
    }

    if (glHoolInitialized[index])
    {
        return &glHooks[index];
    }

    /* To avoid multiple initializations. */
    gcoOS_LockPLS();

    /* Check again. */
    if (glHoolInitialized[index])
    {
        gcoOS_UnLockPLS();
        return &glHooks[index];
    }

    /* Get client driver. */
    library  = thread->clientHandles[vegl_OPENGL_ES11 + index];
    dispatch = thread->dispatchTables[vegl_OPENGL_ES11 + index];

    if (dispatch == gcvNULL && index == 0)
    {
        /* Try commit-line profile. */
        library  = thread->clientHandles[vegl_OPENGL_ES11_CL];
        dispatch = thread->dispatchTables[vegl_OPENGL_ES11_CL];
    }

    /* Dispatch should not be null. */
    gcmASSERT(dispatch != gcvNULL);

    /* Set as initialized. */
    glHoolInitialized[index] = EGL_TRUE;

    /* Cast glHook as function pointer array. */
    api = (__eglMustCastToProperFunctionPointerType *) &glHooks[index];

    /* Now load the symbols. */
    for (i = 0; i < gcmCOUNTOF(glApiNames); i++)
    {
        __eglMustCastToProperFunctionPointerType func = gcvNULL;

        if (dispatch->getProcAddr)
        {
            func = dispatch->getProcAddr(glApiNames[i]);
        }

        if (!func && library)
        {
            gcoOS_GetProcAddress(gcvNULL, library, glApiNames[i], (gctPOINTER *) &func);
        }

        *api++ = func;
    }

    gcoOS_UnLockPLS();
    return &glHooks[index];
}


/*
 * define forward functions, like
 * GLvoid forward_##glXXX(...)
 */
#define API_ENTRY(__api) forward_##__api

#define CALL_GL_API(__api, ...) \
    veglGLAPIHook *hook = _GetGLAPIHook(); \
    if (hook && hook->__api) \
    { \
        hook->__api(__VA_ARGS__); \
    }

#define CALL_GL_API_RETURN(__api, ...) \
    veglGLAPIHook *hook = _GetGLAPIHook(); \
    if (hook && hook->__api) \
    { \
        return hook->__api(__VA_ARGS__); \
    } \
    return 0;

static void GL_APIENTRY API_ENTRY(glActiveTexture)(GLenum texture)
{
    CALL_GL_API(glActiveTexture, texture);
}

static void GL_APIENTRY API_ENTRY(glBindBuffer)(GLenum target, GLuint buffer)
{
    CALL_GL_API(glBindBuffer, target, buffer);
}

static void GL_APIENTRY API_ENTRY(glBindTexture)(GLenum target, GLuint texture)
{
    CALL_GL_API(glBindTexture, target, texture);
}

static void GL_APIENTRY API_ENTRY(glBlendFunc)(GLenum sfactor, GLenum dfactor)
{
    CALL_GL_API(glBlendFunc, sfactor, dfactor);
}

static void GL_APIENTRY API_ENTRY(glBufferData)(GLenum target, GLsizeiptr size, const void * data, GLenum usage)
{
    CALL_GL_API(glBufferData, target, size, data, usage);
}

static void GL_APIENTRY API_ENTRY(glBufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, const void * data)
{
    CALL_GL_API(glBufferSubData, target, offset, size, data);
}

static void GL_APIENTRY API_ENTRY(glClear)(GLbitfield mask)
{
    CALL_GL_API(glClear, mask);
}

static void GL_APIENTRY API_ENTRY(glClearColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    CALL_GL_API(glClearColor, red, green, blue, alpha);
}

static void GL_APIENTRY API_ENTRY(glClearDepthf)(GLfloat d)
{
    CALL_GL_API(glClearDepthf, d);
}

static void GL_APIENTRY API_ENTRY(glClearStencil)(GLint s)
{
    CALL_GL_API(glClearStencil, s);
}

static void GL_APIENTRY API_ENTRY(glColorMask)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    CALL_GL_API(glColorMask, red, green, blue, alpha);
}

static void GL_APIENTRY API_ENTRY(glCompressedTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void * data)
{
    CALL_GL_API(glCompressedTexImage2D, target, level, internalformat, width, height, border, imageSize, data);
}

static void GL_APIENTRY API_ENTRY(glCompressedTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void * data)
{
    CALL_GL_API(glCompressedTexSubImage2D, target, level, xoffset, yoffset, width, height, format, imageSize, data);
}

static void GL_APIENTRY API_ENTRY(glCopyTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
    CALL_GL_API(glCopyTexImage2D, target, level, internalformat, x, y, width, height, border);
}

static void GL_APIENTRY API_ENTRY(glCopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    CALL_GL_API(glCopyTexSubImage2D, target, level, xoffset, yoffset, x, y, width, height);
}

static void GL_APIENTRY API_ENTRY(glCullFace)(GLenum mode)
{
    CALL_GL_API(glCullFace, mode);
}

static void GL_APIENTRY API_ENTRY(glDeleteBuffers)(GLsizei n, const GLuint * buffers)
{
    CALL_GL_API(glDeleteBuffers, n, buffers);
}

static void GL_APIENTRY API_ENTRY(glDeleteTextures)(GLsizei n, const GLuint * textures)
{
    CALL_GL_API(glDeleteTextures, n, textures);
}

static void GL_APIENTRY API_ENTRY(glDepthFunc)(GLenum func)
{
    CALL_GL_API(glDepthFunc, func);
}

static void GL_APIENTRY API_ENTRY(glDepthMask)(GLboolean flag)
{
    CALL_GL_API(glDepthMask, flag);
}

static void GL_APIENTRY API_ENTRY(glDepthRangef)(GLfloat n, GLfloat f)
{
    CALL_GL_API(glDepthRangef, n, f);
}

static void GL_APIENTRY API_ENTRY(glDisable)(GLenum cap)
{
    CALL_GL_API(glDisable, cap);
}

static void GL_APIENTRY API_ENTRY(glDrawArrays)(GLenum mode, GLint first, GLsizei count)
{
    CALL_GL_API(glDrawArrays, mode, first, count);
}

static void GL_APIENTRY API_ENTRY(glDrawElements)(GLenum mode, GLsizei count, GLenum type, const void * indices)
{
    CALL_GL_API(glDrawElements, mode, count, type, indices);
}

static void GL_APIENTRY API_ENTRY(glEnable)(GLenum cap)
{
    CALL_GL_API(glEnable, cap);
}

static void GL_APIENTRY API_ENTRY(glFinish)(void)
{
    CALL_GL_API(glFinish);
}

static void GL_APIENTRY API_ENTRY(glFlush)(void)
{
    CALL_GL_API(glFlush);
}

static void GL_APIENTRY API_ENTRY(glFrontFace)(GLenum mode)
{
    CALL_GL_API(glFrontFace, mode);
}

static void GL_APIENTRY API_ENTRY(glGenBuffers)(GLsizei n, GLuint * buffers)
{
    CALL_GL_API(glGenBuffers, n, buffers);
}

static void GL_APIENTRY API_ENTRY(glGenTextures)(GLsizei n, GLuint * textures)
{
    CALL_GL_API(glGenTextures, n, textures);
}

static void GL_APIENTRY API_ENTRY(glGetBooleanv)(GLenum pname, GLboolean * data)
{
    CALL_GL_API(glGetBooleanv, pname, data);
}

static void GL_APIENTRY API_ENTRY(glGetBufferParameteriv)(GLenum target, GLenum pname, GLint * params)
{
    CALL_GL_API(glGetBufferParameteriv, target, pname, params);
}

static GLenum GL_APIENTRY API_ENTRY(glGetError)(void)
{
    CALL_GL_API_RETURN(glGetError);
}

static void GL_APIENTRY API_ENTRY(glGetFloatv)(GLenum pname, GLfloat * data)
{
    CALL_GL_API(glGetFloatv, pname, data);
}

static void GL_APIENTRY API_ENTRY(glGetIntegerv)(GLenum pname, GLint * data)
{
    CALL_GL_API(glGetIntegerv, pname, data);
}

static void GL_APIENTRY API_ENTRY(glGetPointerv)(GLenum pname, void ** params)
{
    CALL_GL_API(glGetPointerv, pname, params);
}

const GLubyte * GL_APIENTRY API_ENTRY(glGetString)(GLenum name)
{
    CALL_GL_API_RETURN(glGetString, name);
}

static void GL_APIENTRY API_ENTRY(glGetTexParameterfv)(GLenum target, GLenum pname, GLfloat * params)
{
    CALL_GL_API(glGetTexParameterfv, target, pname, params);
}

static void GL_APIENTRY API_ENTRY(glGetTexParameteriv)(GLenum target, GLenum pname, GLint * params)
{
    CALL_GL_API(glGetTexParameteriv, target, pname, params);
}

static void GL_APIENTRY API_ENTRY(glHint)(GLenum target, GLenum mode)
{
    CALL_GL_API(glHint, target, mode);
}

static GLboolean GL_APIENTRY API_ENTRY(glIsBuffer)(GLuint buffer)
{
    CALL_GL_API_RETURN(glIsBuffer, buffer);
}

static GLboolean GL_APIENTRY API_ENTRY(glIsEnabled)(GLenum cap)
{
    CALL_GL_API_RETURN(glIsEnabled, cap);
}

static GLboolean GL_APIENTRY API_ENTRY(glIsTexture)(GLuint texture)
{
    CALL_GL_API_RETURN(glIsTexture, texture);
}

static void GL_APIENTRY API_ENTRY(glLineWidth)(GLfloat width)
{
    CALL_GL_API(glLineWidth, width);
}

static void GL_APIENTRY API_ENTRY(glPixelStorei)(GLenum pname, GLint param)
{
    CALL_GL_API(glPixelStorei, pname, param);
}

static void GL_APIENTRY API_ENTRY(glPolygonOffset)(GLfloat factor, GLfloat units)
{
    CALL_GL_API(glPolygonOffset, factor, units);
}

static void GL_APIENTRY API_ENTRY(glReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void * pixels)
{
    CALL_GL_API(glReadPixels, x, y, width, height, format, type, pixels);
}

static void GL_APIENTRY API_ENTRY(glSampleCoverage)(GLfloat value, GLboolean invert)
{
    CALL_GL_API(glSampleCoverage, value, invert);
}

static void GL_APIENTRY API_ENTRY(glScissor)(GLint x, GLint y, GLsizei width, GLsizei height)
{
    CALL_GL_API(glScissor, x, y, width, height);
}

static void GL_APIENTRY API_ENTRY(glStencilFunc)(GLenum func, GLint ref, GLuint mask)
{
    CALL_GL_API(glStencilFunc, func, ref, mask);
}

static void GL_APIENTRY API_ENTRY(glStencilMask)(GLuint mask)
{
    CALL_GL_API(glStencilMask, mask);
}

static void GL_APIENTRY API_ENTRY(glStencilOp)(GLenum fail, GLenum zfail, GLenum zpass)
{
    CALL_GL_API(glStencilOp, fail, zfail, zpass);
}

static void GL_APIENTRY API_ENTRY(glTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void * pixels)
{
    CALL_GL_API(glTexImage2D, target, level, internalformat, width, height, border, format, type, pixels);
}

static void GL_APIENTRY API_ENTRY(glTexParameterf)(GLenum target, GLenum pname, GLfloat param)
{
    CALL_GL_API(glTexParameterf, target, pname, param);
}

static void GL_APIENTRY API_ENTRY(glTexParameterfv)(GLenum target, GLenum pname, const GLfloat * params)
{
    CALL_GL_API(glTexParameterfv, target, pname, params);
}

static void GL_APIENTRY API_ENTRY(glTexParameteri)(GLenum target, GLenum pname, GLint param)
{
    CALL_GL_API(glTexParameteri, target, pname, param);
}

static void GL_APIENTRY API_ENTRY(glTexParameteriv)(GLenum target, GLenum pname, const GLint * params)
{
    CALL_GL_API(glTexParameteriv, target, pname, params);
}

static void GL_APIENTRY API_ENTRY(glTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void * pixels)
{
    CALL_GL_API(glTexSubImage2D, target, level, xoffset, yoffset, width, height, format, type, pixels);
}

static void GL_APIENTRY API_ENTRY(glViewport)(GLint x, GLint y, GLsizei width, GLsizei height)
{
    CALL_GL_API(glViewport, x, y, width, height);
}

static void GL_APIENTRY API_ENTRY(glEGLImageTargetRenderbufferStorageOES)(GLenum target, GLeglImageOES image)
{
    CALL_GL_API(glEGLImageTargetRenderbufferStorageOES, target, image);
}

static void GL_APIENTRY API_ENTRY(glEGLImageTargetTexture2DOES)(GLenum target, GLeglImageOES image)
{
    CALL_GL_API(glEGLImageTargetTexture2DOES, target, image);
}

static void GL_APIENTRY API_ENTRY(glGetBufferPointervOES)(GLenum target, GLenum pname, void ** params)
{
    CALL_GL_API(glGetBufferPointervOES, target, pname, params);
}

static void * GL_APIENTRY API_ENTRY(glMapBufferOES)(GLenum target, GLenum access)
{
    CALL_GL_API_RETURN(glMapBufferOES, target, access);
}

static GLboolean GL_APIENTRY API_ENTRY(glUnmapBufferOES)(GLenum target)
{
    CALL_GL_API_RETURN(glUnmapBufferOES, target);
}

static void GL_APIENTRY API_ENTRY(glMultiDrawArraysEXT)(GLenum mode, const GLint * first, const GLsizei * count, GLsizei primcount)
{
    CALL_GL_API(glMultiDrawArraysEXT, mode, first, count, primcount);
}

static void GL_APIENTRY API_ENTRY(glMultiDrawElementsEXT)(GLenum mode, const GLsizei * count, GLenum type, const void *const* indices, GLsizei primcount)
{
    CALL_GL_API(glMultiDrawElementsEXT, mode, count, type, indices, primcount);
}

static void GL_APIENTRY API_ENTRY(glDiscardFramebufferEXT)(GLenum target, GLsizei numAttachments, const GLenum *attachments)
{
    CALL_GL_API(glDiscardFramebufferEXT, target, numAttachments, attachments);
}

static void GL_APIENTRY API_ENTRY(glRenderbufferStorageMultisampleEXT)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
    CALL_GL_API(glRenderbufferStorageMultisampleEXT, target, samples, internalformat, width, height);
}

static void GL_APIENTRY API_ENTRY(glFramebufferTexture2DMultisampleEXT)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples)
{
    CALL_GL_API(glFramebufferTexture2DMultisampleEXT, target, attachment, textarget, texture, level, samples);
}

static void GL_APIENTRY API_ENTRY(glGetProgramBinaryOES)(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary)
{
    CALL_GL_API(glGetProgramBinaryOES, program, bufSize, length, binaryFormat, binary);
}

static void GL_APIENTRY API_ENTRY(glProgramBinaryOES)(GLuint program, GLenum binaryFormat, const void *binary, GLint length)
{
    CALL_GL_API(glProgramBinaryOES, program, binaryFormat, binary, length);
}

static void GL_APIENTRY API_ENTRY(glTexDirectVIV)(GLenum Target, GLsizei Width, GLsizei Height, GLenum Format, GLvoid ** Pixels)
{
    CALL_GL_API(glTexDirectVIV, Target, Width, Height, Format, Pixels);
}

static void GL_APIENTRY API_ENTRY(glTexDirectVIVMap)(GLenum Target, GLsizei Width, GLsizei Height, GLenum Format, GLvoid ** Logical, const GLuint * Physical)
{
    CALL_GL_API(glTexDirectVIVMap, Target, Width, Height, Format, Logical, Physical);
}

static void GL_APIENTRY API_ENTRY(glTexDirectMapVIV)(GLenum Target, GLsizei Width, GLsizei Height, GLenum Format, GLvoid ** Logical, const GLuint * Physical)
{
    CALL_GL_API(glTexDirectMapVIV, Target, Width, Height, Format, Logical, Physical);
}

static void GL_APIENTRY API_ENTRY(glTexDirectTiledMapVIV)(GLenum Target, GLsizei Width, GLsizei Height, GLenum Format, GLvoid ** Logical, const GLuint * Physical)
{
    CALL_GL_API(glTexDirectTiledMapVIV, Target, Width, Height, Format, Logical, Physical);
}

static void GL_APIENTRY API_ENTRY(glTexDirectInvalidateVIV)(GLenum Target)
{
    CALL_GL_API(glTexDirectInvalidateVIV, Target);
}

static void GL_APIENTRY API_ENTRY(glAttachShader)(GLuint program, GLuint shader)
{
    CALL_GL_API(glAttachShader, program, shader);
}

static void GL_APIENTRY API_ENTRY(glBindAttribLocation)(GLuint program, GLuint index, const GLchar* name)
{
    CALL_GL_API(glBindAttribLocation, program, index, name);
}

static void GL_APIENTRY API_ENTRY(glBindFramebuffer)(GLenum target, GLuint framebuffer)
{
    CALL_GL_API(glBindFramebuffer, target, framebuffer);
}

static void GL_APIENTRY API_ENTRY(glBindRenderbuffer)(GLenum target, GLuint renderbuffer)
{
    CALL_GL_API(glBindRenderbuffer, target, renderbuffer);
}

static void GL_APIENTRY API_ENTRY(glBlendColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    CALL_GL_API(glBlendColor, red, green, blue, alpha);
}

static void GL_APIENTRY API_ENTRY(glBlendEquation)(GLenum mode)
{
    CALL_GL_API(glBlendEquation, mode);
}

static void GL_APIENTRY API_ENTRY(glBlendEquationSeparate)(GLenum modeRGB, GLenum modeAlpha)
{
    CALL_GL_API(glBlendEquationSeparate, modeRGB, modeAlpha);
}

static void GL_APIENTRY API_ENTRY(glBlendFuncSeparate)(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
    CALL_GL_API(glBlendFuncSeparate, srcRGB, dstRGB, srcAlpha, dstAlpha);
}

static GLenum GL_APIENTRY API_ENTRY(glCheckFramebufferStatus)(GLenum target)
{
    CALL_GL_API_RETURN(glCheckFramebufferStatus, target);
}

static void GL_APIENTRY API_ENTRY(glCompileShader)(GLuint shader)
{
    CALL_GL_API(glCompileShader, shader);
}

static GLuint GL_APIENTRY API_ENTRY(glCreateProgram)(GLvoid)
{
    CALL_GL_API_RETURN(glCreateProgram);
}

static GLuint GL_APIENTRY API_ENTRY(glCreateShader)(GLenum type)
{
    CALL_GL_API_RETURN(glCreateShader, type);
}

static void GL_APIENTRY API_ENTRY(glDeleteFramebuffers)(GLsizei n, const GLuint* framebuffers)
{
    CALL_GL_API(glDeleteFramebuffers, n, framebuffers);
}

static void GL_APIENTRY API_ENTRY(glDeleteProgram)(GLuint program)
{
    CALL_GL_API(glDeleteProgram, program);
}

static void GL_APIENTRY API_ENTRY(glDeleteRenderbuffers)(GLsizei n, const GLuint* renderbuffers)
{
    CALL_GL_API(glDeleteRenderbuffers, n, renderbuffers);
}

static void GL_APIENTRY API_ENTRY(glDeleteShader)(GLuint shader)
{
    CALL_GL_API(glDeleteShader, shader);
}

static void GL_APIENTRY API_ENTRY(glDetachShader)(GLuint program, GLuint shader)
{
    CALL_GL_API(glDetachShader, program, shader);
}

static void GL_APIENTRY API_ENTRY(glDisableVertexAttribArray)(GLuint index)
{
    CALL_GL_API(glDisableVertexAttribArray, index);
}

static void GL_APIENTRY API_ENTRY(glEnableVertexAttribArray)(GLuint index)
{
    CALL_GL_API(glEnableVertexAttribArray, index);
}

static void GL_APIENTRY API_ENTRY(glFramebufferRenderbuffer)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
    CALL_GL_API(glFramebufferRenderbuffer, target, attachment, renderbuffertarget, renderbuffer);
}

static void GL_APIENTRY API_ENTRY(glFramebufferTexture2D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    CALL_GL_API(glFramebufferTexture2D, target, attachment, textarget, texture, level);
}

static void GL_APIENTRY API_ENTRY(glGenerateMipmap)(GLenum target)
{
    CALL_GL_API(glGenerateMipmap, target);
}

static void GL_APIENTRY API_ENTRY(glGenFramebuffers)(GLsizei n, GLuint* framebuffers)
{
    CALL_GL_API(glGenFramebuffers, n, framebuffers);
}

static void GL_APIENTRY API_ENTRY(glGenRenderbuffers)(GLsizei n, GLuint* renderbuffers)
{
    CALL_GL_API(glGenRenderbuffers, n, renderbuffers);
}

static void GL_APIENTRY API_ENTRY(glGetActiveAttrib)(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
    CALL_GL_API(glGetActiveAttrib, program, index, bufsize, length, size, type, name);
}

static void GL_APIENTRY API_ENTRY(glGetActiveUniform)(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
    CALL_GL_API(glGetActiveUniform, program, index, bufsize, length, size, type, name);
}

static void GL_APIENTRY API_ENTRY(glGetAttachedShaders)(GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders)
{
    CALL_GL_API(glGetAttachedShaders, program, maxcount, count, shaders);
}

static GLint GL_APIENTRY API_ENTRY(glGetAttribLocation)(GLuint program, const GLchar* name)
{
    CALL_GL_API_RETURN(glGetAttribLocation, program, name);
}

static void GL_APIENTRY API_ENTRY(glGetFramebufferAttachmentParameteriv)(GLenum target, GLenum attachment, GLenum pname, GLint* params)
{
    CALL_GL_API(glGetFramebufferAttachmentParameteriv, target, attachment, pname, params);
}

static void GL_APIENTRY API_ENTRY(glGetProgramiv)(GLuint program, GLenum pname, GLint* params)
{
    CALL_GL_API(glGetProgramiv, program, pname, params);
}

static void GL_APIENTRY API_ENTRY(glGetProgramInfoLog)(GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog)
{
    CALL_GL_API(glGetProgramInfoLog, program, bufsize, length, infolog);
}

static void GL_APIENTRY API_ENTRY(glGetRenderbufferParameteriv)(GLenum target, GLenum pname, GLint* params)
{
    CALL_GL_API(glGetRenderbufferParameteriv, target, pname, params);
}

static void GL_APIENTRY API_ENTRY(glGetShaderiv)(GLuint shader, GLenum pname, GLint* params)
{
    CALL_GL_API(glGetShaderiv, shader, pname, params);
}

static void GL_APIENTRY API_ENTRY(glGetShaderInfoLog)(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog)
{
    CALL_GL_API(glGetShaderInfoLog, shader, bufsize, length, infolog);
}

static void GL_APIENTRY API_ENTRY(glGetShaderPrecisionFormat)(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision)
{
    CALL_GL_API(glGetShaderPrecisionFormat, shadertype, precisiontype, range, precision);
}

static void GL_APIENTRY API_ENTRY(glGetShaderSource)(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source)
{
    CALL_GL_API(glGetShaderSource, shader, bufsize, length, source);
}

static void GL_APIENTRY API_ENTRY(glGetUniformfv)(GLuint program, GLint location, GLfloat* params)
{
    CALL_GL_API(glGetUniformfv, program, location, params);
}

static void GL_APIENTRY API_ENTRY(glGetUniformiv)(GLuint program, GLint location, GLint* params)
{
    CALL_GL_API(glGetUniformiv, program, location, params);
}

static GLint GL_APIENTRY API_ENTRY(glGetUniformLocation)(GLuint program, const GLchar* name)
{
    CALL_GL_API_RETURN(glGetUniformLocation, program, name);
}

static void GL_APIENTRY API_ENTRY(glGetVertexAttribfv)(GLuint index, GLenum pname, GLfloat* params)
{
    CALL_GL_API(glGetVertexAttribfv, index, pname, params);
}

static void GL_APIENTRY API_ENTRY(glGetVertexAttribiv)(GLuint index, GLenum pname, GLint* params)
{
    CALL_GL_API(glGetVertexAttribiv, index, pname, params);
}

static void GL_APIENTRY API_ENTRY(glGetVertexAttribPointerv)(GLuint index, GLenum pname, GLvoid** pointer)
{
    CALL_GL_API(glGetVertexAttribPointerv, index, pname, pointer);
}

static GLboolean GL_APIENTRY API_ENTRY(glIsFramebuffer)(GLuint framebuffer)
{
    CALL_GL_API_RETURN(glIsFramebuffer, framebuffer);
}

static GLboolean GL_APIENTRY API_ENTRY(glIsProgram)(GLuint program)
{
    CALL_GL_API_RETURN(glIsProgram, program);
}

static GLboolean GL_APIENTRY API_ENTRY(glIsRenderbuffer)(GLuint renderbuffer)
{
    CALL_GL_API_RETURN(glIsRenderbuffer, renderbuffer);
}

static GLboolean GL_APIENTRY API_ENTRY(glIsShader)(GLuint shader)
{
    CALL_GL_API_RETURN(glIsShader, shader);
}

static void GL_APIENTRY API_ENTRY(glLinkProgram)(GLuint program)
{
    CALL_GL_API(glLinkProgram, program);
}

static void GL_APIENTRY API_ENTRY(glReleaseShaderCompiler)(GLvoid)
{
    CALL_GL_API(glReleaseShaderCompiler);
}

static void GL_APIENTRY API_ENTRY(glRenderbufferStorage)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
    CALL_GL_API(glRenderbufferStorage, target, internalformat, width, height);
}

static void GL_APIENTRY API_ENTRY(glShaderBinary)(GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length)
{
    CALL_GL_API(glShaderBinary, n, shaders, binaryformat, binary, length);
}

static void GL_APIENTRY API_ENTRY(glShaderSource)(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length)
{
    CALL_GL_API(glShaderSource, shader, count, string, length);
}

static void GL_APIENTRY API_ENTRY(glStencilFuncSeparate)(GLenum face, GLenum func, GLint ref, GLuint mask)
{
    CALL_GL_API(glStencilFuncSeparate, face, func, ref, mask);
}

static void GL_APIENTRY API_ENTRY(glStencilMaskSeparate)(GLenum face, GLuint mask)
{
    CALL_GL_API(glStencilMaskSeparate, face, mask);
}

static void GL_APIENTRY API_ENTRY(glStencilOpSeparate)(GLenum face, GLenum fail, GLenum zfail, GLenum zpass)
{
    CALL_GL_API(glStencilOpSeparate, face, fail, zfail, zpass);
}

static void GL_APIENTRY API_ENTRY(glUniform1f)(GLint location, GLfloat x)
{
    CALL_GL_API(glUniform1f, location, x);
}

static void GL_APIENTRY API_ENTRY(glUniform1fv)(GLint location, GLsizei count, const GLfloat* v)
{
    CALL_GL_API(glUniform1fv, location, count, v);
}

static void GL_APIENTRY API_ENTRY(glUniform1i)(GLint location, GLint x)
{
    CALL_GL_API(glUniform1i, location, x);
}

static void GL_APIENTRY API_ENTRY(glUniform1iv)(GLint location, GLsizei count, const GLint* v)
{
    CALL_GL_API(glUniform1iv, location, count, v);
}

static void GL_APIENTRY API_ENTRY(glUniform2f)(GLint location, GLfloat x, GLfloat y)
{
    CALL_GL_API(glUniform2f, location, x, y);
}

static void GL_APIENTRY API_ENTRY(glUniform2fv)(GLint location, GLsizei count, const GLfloat* v)
{
    CALL_GL_API(glUniform2fv, location, count, v);
}

static void GL_APIENTRY API_ENTRY(glUniform2i)(GLint location, GLint x, GLint y)
{
    CALL_GL_API(glUniform2i, location, x, y);
}

static void GL_APIENTRY API_ENTRY(glUniform2iv)(GLint location, GLsizei count, const GLint* v)
{
    CALL_GL_API(glUniform2iv, location, count, v);
}

static void GL_APIENTRY API_ENTRY(glUniform3f)(GLint location, GLfloat x, GLfloat y, GLfloat z)
{
    CALL_GL_API(glUniform3f, location, x, y, z);
}

static void GL_APIENTRY API_ENTRY(glUniform3fv)(GLint location, GLsizei count, const GLfloat* v)
{
    CALL_GL_API(glUniform3fv, location, count, v);
}

static void GL_APIENTRY API_ENTRY(glUniform3i)(GLint location, GLint x, GLint y, GLint z)
{
    CALL_GL_API(glUniform3i, location, x, y, z);
}

static void GL_APIENTRY API_ENTRY(glUniform3iv)(GLint location, GLsizei count, const GLint* v)
{
    CALL_GL_API(glUniform3iv, location, count, v);
}

static void GL_APIENTRY API_ENTRY(glUniform4f)(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    CALL_GL_API(glUniform4f, location, x, y, z, w);
}

static void GL_APIENTRY API_ENTRY(glUniform4fv)(GLint location, GLsizei count, const GLfloat* v)
{
    CALL_GL_API(glUniform4fv, location, count, v);
}

static void GL_APIENTRY API_ENTRY(glUniform4i)(GLint location, GLint x, GLint y, GLint z, GLint w)
{
    CALL_GL_API(glUniform4i, location, x, y, z, w);
}

static void GL_APIENTRY API_ENTRY(glUniform4iv)(GLint location, GLsizei count, const GLint* v)
{
    CALL_GL_API(glUniform4iv, location, count, v);
}

static void GL_APIENTRY API_ENTRY(glUniformMatrix2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    CALL_GL_API(glUniformMatrix2fv, location, count, transpose, value);
}

static void GL_APIENTRY API_ENTRY(glUniformMatrix3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    CALL_GL_API(glUniformMatrix3fv, location, count, transpose, value);
}

static void GL_APIENTRY API_ENTRY(glUniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    CALL_GL_API(glUniformMatrix4fv, location, count, transpose, value);
}

static void GL_APIENTRY API_ENTRY(glUseProgram)(GLuint program)
{
    CALL_GL_API(glUseProgram, program);
}

static void GL_APIENTRY API_ENTRY(glValidateProgram)(GLuint program)
{
    CALL_GL_API(glValidateProgram, program);
}

static void GL_APIENTRY API_ENTRY(glVertexAttrib1f)(GLuint indx, GLfloat x)
{
    CALL_GL_API(glVertexAttrib1f, indx, x);
}

static void GL_APIENTRY API_ENTRY(glVertexAttrib1fv)(GLuint indx, const GLfloat* values)
{
    CALL_GL_API(glVertexAttrib1fv, indx, values);
}

static void GL_APIENTRY API_ENTRY(glVertexAttrib2f)(GLuint indx, GLfloat x, GLfloat y)
{
    CALL_GL_API(glVertexAttrib2f, indx, x, y);
}

static void GL_APIENTRY API_ENTRY(glVertexAttrib2fv)(GLuint indx, const GLfloat* values)
{
    CALL_GL_API(glVertexAttrib2fv, indx, values);
}

static void GL_APIENTRY API_ENTRY(glVertexAttrib3f)(GLuint indx, GLfloat x, GLfloat y, GLfloat z)
{
    CALL_GL_API(glVertexAttrib3f, indx, x, y, z);
}

static void GL_APIENTRY API_ENTRY(glVertexAttrib3fv)(GLuint indx, const GLfloat* values)
{
    CALL_GL_API(glVertexAttrib3fv, indx, values);
}

static void GL_APIENTRY API_ENTRY(glVertexAttrib4f)(GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    CALL_GL_API(glVertexAttrib4f, indx, x, y, z, w);
}

static void GL_APIENTRY API_ENTRY(glVertexAttrib4fv)(GLuint indx, const GLfloat* values)
{
    CALL_GL_API(glVertexAttrib4fv, indx, values);
}

static void GL_APIENTRY API_ENTRY(glVertexAttribPointer)(GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr)
{
    CALL_GL_API(glVertexAttribPointer, indx, size, type, normalized, stride, ptr);
}

/*
** OpenGL ES 3.0
*/

static void GL_APIENTRY API_ENTRY(glReadBuffer)(GLenum mode)
{
    CALL_GL_API(glReadBuffer, mode);
}

static void GL_APIENTRY API_ENTRY(glDrawRangeElements)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices)
{
    CALL_GL_API(glDrawRangeElements, mode, start, end, count, type, indices);
}

static void GL_APIENTRY API_ENTRY(glTexImage3D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
{
    CALL_GL_API(glTexImage3D, target, level, internalformat, width, height, depth, border, format, type, pixels);
}

static void GL_APIENTRY API_ENTRY(glTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels)
{
    CALL_GL_API(glTexSubImage3D, target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
}

static void GL_APIENTRY API_ENTRY(glCopyTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    CALL_GL_API(glCopyTexSubImage3D, target, level, xoffset, yoffset, zoffset, x, y, width, height);
}

static void GL_APIENTRY API_ENTRY(glCompressedTexImage3D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data)
{
    CALL_GL_API(glCompressedTexImage3D, target, level, internalformat, width, height, depth, border, imageSize, data);
}

static void GL_APIENTRY API_ENTRY(glCompressedTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data)
{
    CALL_GL_API(glCompressedTexSubImage3D, target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
}

static void GL_APIENTRY API_ENTRY(glGenQueries)(GLsizei n, GLuint* ids)
{
    CALL_GL_API(glGenQueries, n, ids);
}

static void GL_APIENTRY API_ENTRY(glDeleteQueries)(GLsizei n, const GLuint* ids)
{
    CALL_GL_API(glDeleteQueries, n, ids);
}

static GLboolean GL_APIENTRY API_ENTRY(glIsQuery)(GLuint id)
{
    CALL_GL_API_RETURN(glIsQuery, id);
}

static void GL_APIENTRY API_ENTRY(glBeginQuery)(GLenum target, GLuint id)
{
    CALL_GL_API(glBeginQuery, target, id);
}

static void GL_APIENTRY API_ENTRY(glEndQuery)(GLenum target)
{
    CALL_GL_API(glEndQuery, target);
}

static void GL_APIENTRY API_ENTRY(glGetQueryiv)(GLenum target, GLenum pname, GLint* params)
{
    CALL_GL_API(glGetQueryiv, target, pname, params);
}

static void GL_APIENTRY API_ENTRY(glGetQueryObjectuiv)(GLuint id, GLenum pname, GLuint* params)
{
    CALL_GL_API(glGetQueryObjectuiv, id, pname, params);
}

static GLboolean GL_APIENTRY API_ENTRY(glUnmapBuffer)(GLenum target)
{
    CALL_GL_API_RETURN(glUnmapBuffer, target);
}

static void GL_APIENTRY API_ENTRY(glGetBufferPointerv)(GLenum target, GLenum pname, GLvoid** params)
{
    CALL_GL_API(glGetBufferPointerv, target, pname, params);
}

static void GL_APIENTRY API_ENTRY(glDrawBuffers)(GLsizei n, const GLenum* bufs)
{
    CALL_GL_API(glDrawBuffers, n, bufs);
}

static void GL_APIENTRY API_ENTRY(glUniformMatrix2x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    CALL_GL_API(glUniformMatrix2x3fv, location, count, transpose, value);
}

static void GL_APIENTRY API_ENTRY(glUniformMatrix3x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    CALL_GL_API(glUniformMatrix3x2fv, location, count, transpose, value);
}

static void GL_APIENTRY API_ENTRY(glUniformMatrix2x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    CALL_GL_API(glUniformMatrix2x4fv, location, count, transpose, value);
}

static void GL_APIENTRY API_ENTRY(glUniformMatrix4x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    CALL_GL_API(glUniformMatrix4x2fv, location, count, transpose, value);
}

static void GL_APIENTRY API_ENTRY(glUniformMatrix3x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    CALL_GL_API(glUniformMatrix3x4fv, location, count, transpose, value);
}

static void GL_APIENTRY API_ENTRY(glUniformMatrix4x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    CALL_GL_API(glUniformMatrix4x3fv, location, count, transpose, value);
}

static void GL_APIENTRY API_ENTRY(glBlitFramebuffer)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
    CALL_GL_API(glBlitFramebuffer, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
}

static void GL_APIENTRY API_ENTRY(glRenderbufferStorageMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
    CALL_GL_API(glRenderbufferStorageMultisample, target, samples, internalformat, width, height);
}

static void GL_APIENTRY API_ENTRY(glFramebufferTextureLayer)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
    CALL_GL_API(glFramebufferTextureLayer, target, attachment, texture, level, layer);
}

static GLvoid* GL_APIENTRY API_ENTRY(glMapBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
    CALL_GL_API_RETURN(glMapBufferRange, target, offset, length, access);
}

static void GL_APIENTRY API_ENTRY(glFlushMappedBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length)
{
    CALL_GL_API(glFlushMappedBufferRange, target, offset, length);
}

static void GL_APIENTRY API_ENTRY(glBindVertexArray)(GLuint array)
{
    CALL_GL_API(glBindVertexArray, array);
}

static void GL_APIENTRY API_ENTRY(glDeleteVertexArrays)(GLsizei n, const GLuint* arrays)
{
    CALL_GL_API(glDeleteVertexArrays, n, arrays);
}

static void GL_APIENTRY API_ENTRY(glGenVertexArrays)(GLsizei n, GLuint* arrays)
{
    CALL_GL_API(glGenVertexArrays, n, arrays);
}

static GLboolean GL_APIENTRY API_ENTRY(glIsVertexArray)(GLuint array)
{
    CALL_GL_API_RETURN(glIsVertexArray, array);
}

static void GL_APIENTRY API_ENTRY(glGetIntegeri_v)(GLenum target, GLuint index, GLint* data)
{
    CALL_GL_API(glGetIntegeri_v, target, index, data);
}

static void GL_APIENTRY API_ENTRY(glBeginTransformFeedback)(GLenum primitiveMode)
{
    CALL_GL_API(glBeginTransformFeedback, primitiveMode);
}

static void GL_APIENTRY API_ENTRY(glEndTransformFeedback)(void)
{
    CALL_GL_API(glEndTransformFeedback);
}

static void GL_APIENTRY API_ENTRY(glBindBufferRange)(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    CALL_GL_API(glBindBufferRange, target, index, buffer, offset, size);
}

static void GL_APIENTRY API_ENTRY(glBindBufferBase)(GLenum target, GLuint index, GLuint buffer)
{
    CALL_GL_API(glBindBufferBase, target, index, buffer);
}

static void GL_APIENTRY API_ENTRY(glTransformFeedbackVaryings)(GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode)
{
    CALL_GL_API(glTransformFeedbackVaryings, program, count, varyings, bufferMode);
}

static void GL_APIENTRY API_ENTRY(glGetTransformFeedbackVarying)(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name)
{
    CALL_GL_API(glGetTransformFeedbackVarying, program, index, bufSize, length, size, type, name);
}

static void GL_APIENTRY API_ENTRY(glVertexAttribIPointer)(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
    CALL_GL_API(glVertexAttribIPointer, index, size, type, stride, pointer);
}

static void GL_APIENTRY API_ENTRY(glGetVertexAttribIiv)(GLuint index, GLenum pname, GLint* params)
{
    CALL_GL_API(glGetVertexAttribIiv, index, pname, params);
}

static void GL_APIENTRY API_ENTRY(glGetVertexAttribIuiv)(GLuint index, GLenum pname, GLuint* params)
{
    CALL_GL_API(glGetVertexAttribIuiv, index, pname, params);
}

static void GL_APIENTRY API_ENTRY(glVertexAttribI4i)(GLuint index, GLint x, GLint y, GLint z, GLint w)
{
    CALL_GL_API(glVertexAttribI4i, index, x, y, z, w);
}

static void GL_APIENTRY API_ENTRY(glVertexAttribI4ui)(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
    CALL_GL_API(glVertexAttribI4ui, index, x, y, z, w);
}

static void GL_APIENTRY API_ENTRY(glVertexAttribI4iv)(GLuint index, const GLint* v)
{
    CALL_GL_API(glVertexAttribI4iv, index, v);
}

static void GL_APIENTRY API_ENTRY(glVertexAttribI4uiv)(GLuint index, const GLuint* v)
{
    CALL_GL_API(glVertexAttribI4uiv, index, v);
}

static void GL_APIENTRY API_ENTRY(glGetUniformuiv)(GLuint program, GLint location, GLuint* params)
{
    CALL_GL_API(glGetUniformuiv, program, location, params);
}

static GLint GL_APIENTRY API_ENTRY(glGetFragDataLocation)(GLuint program, const GLchar *name)
{
    CALL_GL_API_RETURN(glGetFragDataLocation, program, name);
}

static void GL_APIENTRY API_ENTRY(glUniform1ui)(GLint location, GLuint v0)
{
    CALL_GL_API(glUniform1ui, location, v0);
}

static void GL_APIENTRY API_ENTRY(glUniform2ui)(GLint location, GLuint v0, GLuint v1)
{
    CALL_GL_API(glUniform2ui, location, v0, v1);
}

static void GL_APIENTRY API_ENTRY(glUniform3ui)(GLint location, GLuint v0, GLuint v1, GLuint v2)
{
    CALL_GL_API(glUniform3ui, location, v0, v1, v2);
}

static void GL_APIENTRY API_ENTRY(glUniform4ui)(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
    CALL_GL_API(glUniform4ui, location, v0, v1, v2, v3);
}

static void GL_APIENTRY API_ENTRY(glUniform1uiv)(GLint location, GLsizei count, const GLuint* value)
{
    CALL_GL_API(glUniform1uiv, location, count, value);
}

static void GL_APIENTRY API_ENTRY(glUniform2uiv)(GLint location, GLsizei count, const GLuint* value)
{
    CALL_GL_API(glUniform2uiv, location, count, value);
}

static void GL_APIENTRY API_ENTRY(glUniform3uiv)(GLint location, GLsizei count, const GLuint* value)
{
    CALL_GL_API(glUniform3uiv, location, count, value);
}

static void GL_APIENTRY API_ENTRY(glUniform4uiv)(GLint location, GLsizei count, const GLuint* value)
{
    CALL_GL_API(glUniform4uiv, location, count, value);
}

static void GL_APIENTRY API_ENTRY(glClearBufferiv)(GLenum buffer, GLint drawbuffer, const GLint* value)
{
    CALL_GL_API(glClearBufferiv, buffer, drawbuffer, value);
}

static void GL_APIENTRY API_ENTRY(glClearBufferuiv)(GLenum buffer, GLint drawbuffer, const GLuint* value)
{
    CALL_GL_API(glClearBufferuiv, buffer, drawbuffer, value);
}

static void GL_APIENTRY API_ENTRY(glClearBufferfv)(GLenum buffer, GLint drawbuffer, const GLfloat* value)
{
    CALL_GL_API(glClearBufferfv, buffer, drawbuffer, value);
}

static void GL_APIENTRY API_ENTRY(glClearBufferfi)(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)
{
    CALL_GL_API(glClearBufferfi, buffer, drawbuffer, depth, stencil);
}

static const GLubyte* GL_APIENTRY API_ENTRY(glGetStringi)(GLenum name, GLuint index)
{
    CALL_GL_API_RETURN(glGetStringi, name, index);
}

static void GL_APIENTRY API_ENTRY(glCopyBufferSubData)(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
    CALL_GL_API(glCopyBufferSubData, readTarget, writeTarget, readOffset, writeOffset, size);
}

static void GL_APIENTRY API_ENTRY(glGetUniformIndices)(GLuint program, GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices)
{
    CALL_GL_API(glGetUniformIndices, program, uniformCount, uniformNames, uniformIndices);
}

static void GL_APIENTRY API_ENTRY(glGetActiveUniformsiv)(GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params)
{
    CALL_GL_API(glGetActiveUniformsiv, program, uniformCount, uniformIndices, pname, params);
}

static GLuint GL_APIENTRY API_ENTRY(glGetUniformBlockIndex)(GLuint program, const GLchar* uniformBlockName)
{
    CALL_GL_API_RETURN(glGetUniformBlockIndex, program, uniformBlockName);
}

static void GL_APIENTRY API_ENTRY(glGetActiveUniformBlockiv)(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params)
{
    CALL_GL_API(glGetActiveUniformBlockiv, program, uniformBlockIndex, pname, params);
}

static void GL_APIENTRY API_ENTRY(glGetActiveUniformBlockName)(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName)
{
    CALL_GL_API(glGetActiveUniformBlockName, program, uniformBlockIndex, bufSize, length, uniformBlockName);
}

static void GL_APIENTRY API_ENTRY(glUniformBlockBinding)(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
    CALL_GL_API(glUniformBlockBinding, program, uniformBlockIndex, uniformBlockBinding);
}

static void GL_APIENTRY API_ENTRY(glDrawArraysInstanced)(GLenum mode, GLint first, GLsizei count, GLsizei instanceCount)
{
    CALL_GL_API(glDrawArraysInstanced, mode, first, count, instanceCount);
}

static void GL_APIENTRY API_ENTRY(glDrawElementsInstanced)(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei instanceCount)
{
    CALL_GL_API(glDrawElementsInstanced, mode, count, type, indices, instanceCount);
}

static GLsync GL_APIENTRY API_ENTRY(glFenceSync)(GLenum condition, GLbitfield flags)
{
    CALL_GL_API_RETURN(glFenceSync, condition, flags);
}

static GLboolean GL_APIENTRY API_ENTRY(glIsSync)(GLsync sync)
{
    CALL_GL_API_RETURN(glIsSync, sync);
}

static void GL_APIENTRY API_ENTRY(glDeleteSync)(GLsync sync)
{
    CALL_GL_API(glDeleteSync, sync);
}

static GLenum GL_APIENTRY API_ENTRY(glClientWaitSync)(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    CALL_GL_API_RETURN(glClientWaitSync, sync, flags, timeout);
}

static void GL_APIENTRY API_ENTRY(glWaitSync)(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    CALL_GL_API(glWaitSync, sync, flags, timeout);
}

static void GL_APIENTRY API_ENTRY(glGetInteger64v)(GLenum pname, GLint64* params)
{
    CALL_GL_API(glGetInteger64v, pname, params);
}

static void GL_APIENTRY API_ENTRY(glGetSynciv)(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values)
{
    CALL_GL_API(glGetSynciv, sync, pname, bufSize, length, values);
}

static void GL_APIENTRY API_ENTRY(glGetInteger64i_v)(GLenum target, GLuint index, GLint64* data)
{
    CALL_GL_API(glGetInteger64i_v, target, index, data);
}

static void GL_APIENTRY API_ENTRY(glGetBufferParameteri64v)(GLenum target, GLenum pname, GLint64* params)
{
    CALL_GL_API(glGetBufferParameteri64v, target, pname, params);
}

static void GL_APIENTRY API_ENTRY(glGenSamplers)(GLsizei count, GLuint* samplers)
{
    CALL_GL_API(glGenSamplers, count, samplers);
}

static void GL_APIENTRY API_ENTRY(glDeleteSamplers)(GLsizei count, const GLuint* samplers)
{
    CALL_GL_API(glDeleteSamplers, count, samplers);
}

static GLboolean GL_APIENTRY API_ENTRY(glIsSampler)(GLuint sampler)
{
    CALL_GL_API_RETURN(glIsSampler, sampler);
}

static void GL_APIENTRY API_ENTRY(glBindSampler)(GLuint unit, GLuint sampler)
{
    CALL_GL_API(glBindSampler, unit, sampler);
}

static void GL_APIENTRY API_ENTRY(glSamplerParameteri)(GLuint sampler, GLenum pname, GLint param)
{
    CALL_GL_API(glSamplerParameteri, sampler, pname, param);
}

static void GL_APIENTRY API_ENTRY(glSamplerParameteriv)(GLuint sampler, GLenum pname, const GLint* param)
{
    CALL_GL_API(glSamplerParameteriv, sampler, pname, param);
}

static void GL_APIENTRY API_ENTRY(glSamplerParameterf)(GLuint sampler, GLenum pname, GLfloat param)
{
    CALL_GL_API(glSamplerParameterf, sampler, pname, param);
}

static void GL_APIENTRY API_ENTRY(glSamplerParameterfv)(GLuint sampler, GLenum pname, const GLfloat* param)
{
    CALL_GL_API(glSamplerParameterfv, sampler, pname, param);
}

static void GL_APIENTRY API_ENTRY(glGetSamplerParameteriv)(GLuint sampler, GLenum pname, GLint* params)
{
    CALL_GL_API(glGetSamplerParameteriv, sampler, pname, params);
}

static void GL_APIENTRY API_ENTRY(glGetSamplerParameterfv)(GLuint sampler, GLenum pname, GLfloat* params)
{
    CALL_GL_API(glGetSamplerParameterfv, sampler, pname, params);
}

static void GL_APIENTRY API_ENTRY(glVertexAttribDivisor)(GLuint index, GLuint divisor)
{
    CALL_GL_API(glVertexAttribDivisor, index, divisor);
}

static void GL_APIENTRY API_ENTRY(glBindTransformFeedback)(GLenum target, GLuint id)
{
    CALL_GL_API(glBindTransformFeedback, target, id);
}

static void GL_APIENTRY API_ENTRY(glDeleteTransformFeedbacks)(GLsizei n, const GLuint* ids)
{
    CALL_GL_API(glDeleteTransformFeedbacks, n, ids);
}

static void GL_APIENTRY API_ENTRY(glGenTransformFeedbacks)(GLsizei n, GLuint* ids)
{
    CALL_GL_API(glGenTransformFeedbacks, n, ids);
}

static GLboolean GL_APIENTRY API_ENTRY(glIsTransformFeedback)(GLuint id)
{
    CALL_GL_API_RETURN(glIsTransformFeedback, id);
}

static void GL_APIENTRY API_ENTRY(glPauseTransformFeedback)(GLvoid)
{
    CALL_GL_API(glPauseTransformFeedback);
}

static void GL_APIENTRY API_ENTRY(glResumeTransformFeedback)(GLvoid)
{
    CALL_GL_API(glResumeTransformFeedback);
}

static void GL_APIENTRY API_ENTRY(glGetProgramBinary)(GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, GLvoid* binary)
{
    CALL_GL_API(glGetProgramBinary, program, bufSize, length, binaryFormat, binary);
}

static void GL_APIENTRY API_ENTRY(glProgramBinary)(GLuint program, GLenum binaryFormat, const GLvoid* binary, GLsizei length)
{
    CALL_GL_API(glProgramBinary, program, binaryFormat, binary, length);
}

static void GL_APIENTRY API_ENTRY(glProgramParameteri)(GLuint program, GLenum pname, GLint value)
{
    CALL_GL_API(glProgramParameteri, program, pname, value);
}

static void GL_APIENTRY API_ENTRY(glInvalidateFramebuffer)(GLenum target, GLsizei numAttachments, const GLenum* attachments)
{
    CALL_GL_API(glInvalidateFramebuffer, target, numAttachments, attachments);
}

static void GL_APIENTRY API_ENTRY(glInvalidateSubFramebuffer)(GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height)
{
    CALL_GL_API(glInvalidateSubFramebuffer, target, numAttachments, attachments, x, y, width, height);
}

static void GL_APIENTRY API_ENTRY(glTexStorage2D)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
    CALL_GL_API(glTexStorage2D, target, levels, internalformat, width, height);
}

static void GL_APIENTRY API_ENTRY(glTexStorage3D)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
    CALL_GL_API(glTexStorage3D, target, levels, internalformat, width, height, depth);
}

static void GL_APIENTRY API_ENTRY(glGetInternalformativ)(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params)
{
    CALL_GL_API(glGetInternalformativ, target, internalformat, pname, bufSize, params);
}

/*
** OpenGL ES 3.1
*/
static void GL_APIENTRY API_ENTRY(glDispatchCompute)(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z)
{
    CALL_GL_API(glDispatchCompute, num_groups_x, num_groups_y, num_groups_z);
}

static void GL_APIENTRY API_ENTRY(glDispatchComputeIndirect)(GLintptr indirect)
{
    CALL_GL_API(glDispatchComputeIndirect, indirect);
}

static void GL_APIENTRY API_ENTRY(glDrawArraysIndirect)(GLenum mode, const void *indirect)
{
    CALL_GL_API(glDrawArraysIndirect, mode, indirect);
}

static void GL_APIENTRY API_ENTRY(glDrawElementsIndirect)(GLenum mode, GLenum type, const void *indirect)
{
    CALL_GL_API(glDrawElementsIndirect, mode, type, indirect);
}

static void GL_APIENTRY API_ENTRY(glFramebufferParameteri)(GLenum target, GLenum pname, GLint param)
{
    CALL_GL_API(glFramebufferParameteri, target, pname, param);
}

static void GL_APIENTRY API_ENTRY(glGetFramebufferParameteriv)(GLenum target, GLenum pname, GLint *params)
{
    CALL_GL_API(glGetFramebufferParameteriv, target, pname, params);
}

static void GL_APIENTRY API_ENTRY(glGetProgramInterfaceiv)(GLuint program, GLenum programInterface, GLenum pname, GLint *params)
{
    CALL_GL_API(glGetProgramInterfaceiv, program, programInterface, pname, params);
}

static GLuint GL_APIENTRY API_ENTRY(glGetProgramResourceIndex)(GLuint program, GLenum programInterface, const GLchar *name)
{
    CALL_GL_API_RETURN(glGetProgramResourceIndex, program, programInterface, name);
}

static void GL_APIENTRY API_ENTRY(glGetProgramResourceName)(GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name)
{
    CALL_GL_API(glGetProgramResourceName, program, programInterface, index, bufSize, length, name);
}

static void GL_APIENTRY API_ENTRY(glGetProgramResourceiv)(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params)
{
    CALL_GL_API(glGetProgramResourceiv, program, programInterface, index, propCount, props, bufSize, length, params);
}

static GLint GL_APIENTRY API_ENTRY(glGetProgramResourceLocation)(GLuint program, GLenum programInterface, const GLchar *name)
{
    CALL_GL_API_RETURN(glGetProgramResourceLocation, program, programInterface, name);
}

static void GL_APIENTRY API_ENTRY(glUseProgramStages)(GLuint pipeline, GLbitfield stages, GLuint program)
{
    CALL_GL_API(glUseProgramStages, pipeline, stages, program);
}

static void GL_APIENTRY API_ENTRY(glActiveShaderProgram)(GLuint pipeline, GLuint program)
{
    CALL_GL_API(glActiveShaderProgram, pipeline, program);
}

static GLuint GL_APIENTRY API_ENTRY(glCreateShaderProgramv)(GLenum type, GLsizei count, const GLchar *const*strings)
{
    CALL_GL_API_RETURN(glCreateShaderProgramv, type, count, strings);
}

static void GL_APIENTRY API_ENTRY(glBindProgramPipeline)(GLuint pipeline)
{
    CALL_GL_API(glBindProgramPipeline, pipeline);
}

static void GL_APIENTRY API_ENTRY(glDeleteProgramPipelines)(GLsizei n, const GLuint *pipelines)
{
    CALL_GL_API(glDeleteProgramPipelines, n , pipelines);
}

static void GL_APIENTRY API_ENTRY(glGenProgramPipelines)(GLsizei n, GLuint *pipelines)
{
    CALL_GL_API(glGenProgramPipelines, n , pipelines);
}

static GLboolean GL_APIENTRY API_ENTRY(glIsProgramPipeline)(GLuint pipeline)
{
    CALL_GL_API_RETURN(glIsProgramPipeline, pipeline);
}

static void GL_APIENTRY API_ENTRY(glGetProgramPipelineiv)(GLuint pipeline, GLenum pname, GLint *params)
{
    CALL_GL_API(glGetProgramPipelineiv, pipeline, pname, params);
}

static void GL_APIENTRY API_ENTRY(glProgramUniform1i)(GLuint program, GLint location, GLint v0)
{
    CALL_GL_API(glProgramUniform1i, program, location, v0);
}

static void GL_APIENTRY API_ENTRY(glProgramUniform2i)(GLuint program, GLint location, GLint v0, GLint v1)
{
    CALL_GL_API(glProgramUniform2i, program, location, v0, v1);
}

static void GL_APIENTRY API_ENTRY(glProgramUniform3i)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2)
{
    CALL_GL_API(glProgramUniform3i, program, location, v0, v1, v2);
}

static void GL_APIENTRY API_ENTRY(glProgramUniform4i)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
    CALL_GL_API(glProgramUniform4i, program, location, v0, v1, v2, v3);
}

static void GL_APIENTRY API_ENTRY(glProgramUniform1ui)(GLuint program, GLint location, GLuint v0)
{
    CALL_GL_API(glProgramUniform1ui, program, location, v0);
}

static void GL_APIENTRY API_ENTRY(glProgramUniform2ui)(GLuint program, GLint location, GLuint v0, GLuint v1)
{
    CALL_GL_API(glProgramUniform2ui, program, location, v0, v1);
}

static void GL_APIENTRY API_ENTRY(glProgramUniform3ui)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2)
{
    CALL_GL_API(glProgramUniform3ui, program, location, v0, v1, v2);
}

static void GL_APIENTRY API_ENTRY(glProgramUniform4ui)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
    CALL_GL_API(glProgramUniform4ui, program, location, v0, v1, v2, v3);
}

static void GL_APIENTRY API_ENTRY(glProgramUniform1f)(GLuint program, GLint location, GLfloat v0)
{
    CALL_GL_API(glProgramUniform1f, program, location, v0);
}

static void GL_APIENTRY API_ENTRY(glProgramUniform2f)(GLuint program, GLint location, GLfloat v0, GLfloat v1)
{
    CALL_GL_API(glProgramUniform2f, program, location, v0, v1);
}

static void GL_APIENTRY API_ENTRY(glProgramUniform3f)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
    CALL_GL_API(glProgramUniform3f, program, location, v0, v1, v2);
}

static void GL_APIENTRY API_ENTRY(glProgramUniform4f)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    CALL_GL_API(glProgramUniform4f, program, location, v0, v1, v2, v3);
}

static void GL_APIENTRY API_ENTRY(glProgramUniform1iv)(GLuint program, GLint location, GLsizei count, const GLint *value)
{
    CALL_GL_API(glProgramUniform1iv, program, location, count, value);
}

static void GL_APIENTRY API_ENTRY(glProgramUniform2iv)(GLuint program, GLint location, GLsizei count, const GLint *value)
{
    CALL_GL_API(glProgramUniform2iv, program, location, count, value);
}

static void GL_APIENTRY API_ENTRY(glProgramUniform3iv)(GLuint program, GLint location, GLsizei count, const GLint *value)
{
    CALL_GL_API(glProgramUniform3iv, program, location, count, value);
}

static void GL_APIENTRY API_ENTRY(glProgramUniform4iv)(GLuint program, GLint location, GLsizei count, const GLint *value)
{
    CALL_GL_API(glProgramUniform4iv, program, location, count, value);
}

static void GL_APIENTRY API_ENTRY(glProgramUniform1uiv)(GLuint program, GLint location, GLsizei count, const GLuint *value)
{
    CALL_GL_API(glProgramUniform1uiv, program, location, count, value);
}

static void GL_APIENTRY API_ENTRY(glProgramUniform2uiv)(GLuint program, GLint location, GLsizei count, const GLuint *value)
{
    CALL_GL_API(glProgramUniform2uiv, program, location, count, value);
}

static void GL_APIENTRY API_ENTRY(glProgramUniform3uiv)(GLuint program, GLint location, GLsizei count, const GLuint *value)
{
    CALL_GL_API(glProgramUniform3uiv, program, location, count, value);
}

static void GL_APIENTRY API_ENTRY(glProgramUniform4uiv)(GLuint program, GLint location, GLsizei count, const GLuint *value)
{
    CALL_GL_API(glProgramUniform4uiv, program, location, count, value);
}

static void GL_APIENTRY API_ENTRY(glProgramUniform1fv)(GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
    CALL_GL_API(glProgramUniform1fv, program, location, count, value);
}

static void GL_APIENTRY API_ENTRY(glProgramUniform2fv)(GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
    CALL_GL_API(glProgramUniform2fv, program, location, count, value);
}

static void GL_APIENTRY API_ENTRY(glProgramUniform3fv)(GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
    CALL_GL_API(glProgramUniform3fv, program, location, count, value);
}

static void GL_APIENTRY API_ENTRY(glProgramUniform4fv)(GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
    CALL_GL_API(glProgramUniform4fv, program, location, count, value);
}

static void GL_APIENTRY API_ENTRY(glProgramUniformMatrix2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    CALL_GL_API(glProgramUniformMatrix2fv, program, location, count, transpose, value);
}

static void GL_APIENTRY API_ENTRY(glProgramUniformMatrix3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    CALL_GL_API(glProgramUniformMatrix3fv, program, location, count, transpose, value);
}

static void GL_APIENTRY API_ENTRY(glProgramUniformMatrix4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    CALL_GL_API(glProgramUniformMatrix4fv, program, location, count, transpose, value);
}

static void GL_APIENTRY API_ENTRY(glProgramUniformMatrix2x3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    CALL_GL_API(glProgramUniformMatrix2x3fv, program, location, count, transpose, value);
}

static void GL_APIENTRY API_ENTRY(glProgramUniformMatrix3x2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    CALL_GL_API(glProgramUniformMatrix3x2fv, program, location, count, transpose, value);
}

static void GL_APIENTRY API_ENTRY(glProgramUniformMatrix2x4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    CALL_GL_API(glProgramUniformMatrix2x4fv, program, location, count, transpose, value);
}

static void GL_APIENTRY API_ENTRY(glProgramUniformMatrix4x2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    CALL_GL_API(glProgramUniformMatrix4x2fv, program, location, count, transpose, value);
}

static void GL_APIENTRY API_ENTRY(glProgramUniformMatrix3x4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    CALL_GL_API(glProgramUniformMatrix3x4fv, program, location, count, transpose, value);
}

static void GL_APIENTRY API_ENTRY(glProgramUniformMatrix4x3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    CALL_GL_API(glProgramUniformMatrix4x3fv, program, location, count, transpose, value);
}

static void GL_APIENTRY API_ENTRY(glValidateProgramPipeline)(GLuint pipeline)
{
    CALL_GL_API(glValidateProgramPipeline, pipeline);
}

static void GL_APIENTRY API_ENTRY(glGetProgramPipelineInfoLog)(GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
    CALL_GL_API(glGetProgramPipelineInfoLog, pipeline, bufSize, length, infoLog);
}

static void GL_APIENTRY API_ENTRY(glBindImageTexture)(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format)
{
    CALL_GL_API(glBindImageTexture, unit, texture, level, layered, layer, access, format);
}

static void GL_APIENTRY API_ENTRY(glGetBooleani_v)(GLenum target, GLuint index, GLboolean *data)
{
    CALL_GL_API(glGetBooleani_v, target, index, data);
}

static void GL_APIENTRY API_ENTRY(glMemoryBarrier)(GLbitfield barriers)
{
    CALL_GL_API(glMemoryBarrier, barriers);
}

static void GL_APIENTRY API_ENTRY(glMemoryBarrierByRegion)(GLbitfield barriers)
{
    CALL_GL_API(glMemoryBarrierByRegion, barriers);
}

static void GL_APIENTRY API_ENTRY(glTexStorage2DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
    CALL_GL_API(glTexStorage2DMultisample, target, samples, internalformat, width, height, fixedsamplelocations);
}

static void GL_APIENTRY API_ENTRY(glGetMultisamplefv)(GLenum pname, GLuint index, GLfloat *val)
{
    CALL_GL_API(glGetMultisamplefv, pname, index, val);
}

static void GL_APIENTRY API_ENTRY(glSampleMaski)(GLuint maskNumber, GLbitfield mask)
{
    CALL_GL_API(glSampleMaski, maskNumber, mask);
}

static void GL_APIENTRY API_ENTRY(glGetTexLevelParameteriv)(GLenum target, GLint level, GLenum pname, GLint *params)
{
    CALL_GL_API(glGetTexLevelParameteriv, target, level, pname, params);
}

static void GL_APIENTRY API_ENTRY(glGetTexLevelParameterfv)(GLenum target, GLint level, GLenum pname, GLfloat *params)
{
    CALL_GL_API(glGetTexLevelParameterfv, target, level, pname, params);
}

static void GL_APIENTRY API_ENTRY(glBindVertexBuffer)(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)
{
    CALL_GL_API(glBindVertexBuffer, bindingindex, buffer, offset, stride);
}

static void GL_APIENTRY API_ENTRY(glVertexAttribFormat)(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)
{
    CALL_GL_API(glVertexAttribFormat, attribindex, size, type, normalized, relativeoffset);
}

static void GL_APIENTRY API_ENTRY(glVertexAttribIFormat)(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
    CALL_GL_API(glVertexAttribIFormat, attribindex, size, type, relativeoffset);
}

static void GL_APIENTRY API_ENTRY(glVertexAttribBinding)(GLuint attribindex, GLuint bindingindex)
{
    CALL_GL_API(glVertexAttribBinding, attribindex, bindingindex);
}

static void GL_APIENTRY API_ENTRY(glVertexBindingDivisor)(GLuint bindingindex, GLuint divisor)
{
    CALL_GL_API(glVertexBindingDivisor, bindingindex, divisor);
}

/* OpenGL ES 3.2 */

static void GL_APIENTRY API_ENTRY(glTexStorage3DMultisample)(GLenum target, GLsizei samples, GLenum sizedinternalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
    CALL_GL_API(glTexStorage3DMultisample, target, samples, sizedinternalformat, width, height, depth, fixedsamplelocations);
}

static void GL_APIENTRY API_ENTRY(glBlendBarrier)(GLvoid)
{
    CALL_GL_API(glBlendBarrier);
}

static void GL_APIENTRY API_ENTRY(glDebugMessageControl)(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled)
{
    CALL_GL_API(glDebugMessageControl, source, type, severity, count, ids, enabled);
}

static void GL_APIENTRY API_ENTRY(glDebugMessageInsert)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* buf)
{
    CALL_GL_API(glDebugMessageInsert, source, type, id, severity, length, buf);
}


static void GL_APIENTRY API_ENTRY(glDebugMessageCallback)(GLDEBUGPROCKHR callback, const GLvoid* userParam)
{
    CALL_GL_API(glDebugMessageCallback, callback, userParam);
}

static GLuint GL_APIENTRY API_ENTRY(glGetDebugMessageLog)(GLuint count, GLsizei bufSize, GLenum* sources, GLenum* types, GLuint* ids, GLenum* severities, GLsizei* lengths, GLchar* messageLog)
{
    CALL_GL_API_RETURN(glGetDebugMessageLog, count, bufSize, sources, types, ids, severities, lengths, messageLog);
}


static void GL_APIENTRY API_ENTRY(glPushDebugGroup)(GLenum source, GLuint id, GLsizei length, const GLchar * message)
{
    CALL_GL_API(glPushDebugGroup, source, id, length, message);
}

static void GL_APIENTRY API_ENTRY(glPopDebugGroup)(void)
{
    CALL_GL_API(glPopDebugGroup);
}

static void GL_APIENTRY API_ENTRY(glObjectLabel)(GLenum identifier, GLuint name, GLsizei length, const GLchar *label)
{
    CALL_GL_API(glObjectLabel, identifier, name, length, label);
}

static void GL_APIENTRY API_ENTRY(glGetObjectLabel)(GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label)
{
    CALL_GL_API(glGetObjectLabel, identifier, name, bufSize, length, label);
}

static void GL_APIENTRY API_ENTRY(glObjectPtrLabel)(const GLvoid* ptr, GLsizei length, const GLchar *label)
{
    CALL_GL_API(glObjectPtrLabel, ptr, length, label);
}

static void GL_APIENTRY API_ENTRY(glGetObjectPtrLabel)(const GLvoid* ptr, GLsizei bufSize, GLsizei *length, GLchar *label)
{
    CALL_GL_API(glGetObjectPtrLabel, ptr, bufSize, length, label);
}

static GLenum GL_APIENTRY API_ENTRY(glGetGraphicsResetStatus)(GLvoid)
{
    CALL_GL_API_RETURN(glGetGraphicsResetStatus);
}

static void GL_APIENTRY API_ENTRY(glReadnPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, GLvoid *data)
{
    CALL_GL_API(glReadnPixels, x, y, width, height, format, type, bufSize, data);
}

static void GL_APIENTRY API_ENTRY(glGetnUniformfv)(GLuint program, GLint location, GLsizei bufSize, GLfloat *params)
{
    CALL_GL_API(glGetnUniformfv, program, location, bufSize, params);
}

static void GL_APIENTRY API_ENTRY(glGetnUniformiv)(GLuint program, GLint location, GLsizei bufSize, GLint *params)
{
    CALL_GL_API(glGetnUniformiv, program, location, bufSize, params);
}

static void GL_APIENTRY API_ENTRY(glGetnUniformuiv)(GLuint program, GLint location, GLsizei bufSize, GLuint *params)
{
    CALL_GL_API(glGetnUniformuiv, program, location, bufSize, params);
}

static void GL_APIENTRY API_ENTRY(glBlendEquationi)(GLuint buf, GLenum mode)
{
    CALL_GL_API(glBlendEquationi, buf, mode);
}

static void GL_APIENTRY API_ENTRY(glBlendEquationSeparatei)(GLuint buf, GLenum modeRGB, GLenum modeAlpha)
{
    CALL_GL_API(glBlendEquationSeparatei, buf, modeRGB, modeAlpha);
}

static void GL_APIENTRY API_ENTRY(glBlendFunci)(GLuint buf, GLenum sfactor, GLenum dfactor)
{
    CALL_GL_API(glBlendFunci, buf, sfactor, dfactor);
}

static void GL_APIENTRY API_ENTRY(glBlendFuncSeparatei)(GLuint buf, GLenum sfactorRGB,GLenum dfactorRGB,GLenum sfactorAlpha,GLenum dfactorAlpha)
{
    CALL_GL_API(glBlendFuncSeparatei, buf, sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
}

static void GL_APIENTRY API_ENTRY(glColorMaski)(GLuint buf, GLboolean r, GLboolean g, GLboolean b, GLboolean a)
{
    CALL_GL_API(glColorMaski, buf, r, g, b, a);
}

static void GL_APIENTRY API_ENTRY(glEnablei)(GLenum target, GLuint index)
{
    CALL_GL_API(glEnablei, target, index);
}

GLvoid GL_APIENTRY  GL_APIENTRY API_ENTRY(glDisablei)( GLenum target, GLuint index)
{
    CALL_GL_API(glDisablei, target, index);
}

static GLboolean  GL_APIENTRY API_ENTRY(glIsEnabledi)( GLenum target, GLuint index)
{
    CALL_GL_API_RETURN(glIsEnabledi, target, index);
}

static void GL_APIENTRY API_ENTRY(glTexParameterIiv)(GLenum target, GLenum pname, const GLint *params)
{
    CALL_GL_API(glTexParameterIiv, target, pname, params);
}

static void GL_APIENTRY API_ENTRY(glTexParameterIuiv)(GLenum target, GLenum pname, const GLuint *params)
{
    CALL_GL_API(glTexParameterIuiv, target, pname, params);
}

static void GL_APIENTRY API_ENTRY(glGetTexParameterIiv)(GLenum target, GLenum pname, GLint *params)
{
    CALL_GL_API(glGetTexParameterIiv, target, pname, params);
}

static void GL_APIENTRY API_ENTRY(glGetTexParameterIuiv)(GLenum target, GLenum pname, GLuint *params)
{
    CALL_GL_API(glGetTexParameterIuiv, target, pname, params);
}

static void GL_APIENTRY API_ENTRY(glSamplerParameterIiv)(GLuint sampler, GLenum pname, const GLint *param)
{
    CALL_GL_API(glSamplerParameterIiv, sampler, pname, param);
}

static void GL_APIENTRY API_ENTRY(glSamplerParameterIuiv)(GLuint sampler, GLenum pname, const GLuint *param)
{
    CALL_GL_API(glSamplerParameterIuiv, sampler, pname, param);
}

static void GL_APIENTRY API_ENTRY(glGetSamplerParameterIiv)(GLuint sampler, GLenum pname, GLint *params)
{
    CALL_GL_API(glGetSamplerParameterIiv, sampler, pname, params);
}

static void GL_APIENTRY API_ENTRY(glGetSamplerParameterIuiv)(GLuint sampler, GLenum pname, GLuint *params)
{
    CALL_GL_API(glGetSamplerParameterIuiv, sampler, pname, params);
}

static void GL_APIENTRY API_ENTRY(glTexBuffer)(GLenum target, GLenum internalformat, GLuint buffer)
{
    CALL_GL_API(glTexBuffer, target, internalformat, buffer);
}

static void GL_APIENTRY API_ENTRY(glTexBufferRange)(GLenum target, GLenum internalformat, GLuint buffer,
                                                  GLintptr offset, GLsizeiptr size)
{
    CALL_GL_API(glTexBufferRange, target, internalformat, buffer, offset, size);
}

static void GL_APIENTRY API_ENTRY(glPatchParameteri)(GLenum pname, GLint value)
{
    CALL_GL_API(glPatchParameteri, pname, value);
}

static void GL_APIENTRY API_ENTRY(glFramebufferTexture)(GLenum target, GLenum attachment, GLuint texture, GLint level)
{
    CALL_GL_API(glFramebufferTexture, target, attachment, texture, level);
}

static void GL_APIENTRY API_ENTRY(glMinSampleShading)(GLfloat value)
{
    CALL_GL_API(glMinSampleShading, value);
}

static void GL_APIENTRY API_ENTRY(glCopyImageSubData)(GLuint srcName, GLenum srcTarget, GLint srcLevel,
                                                    GLint srcX, GLint srcY, GLint srcZ,
                                                    GLuint dstName, GLenum dstTarget, GLint dstLevel,
                                                    GLint dstX, GLint dstY, GLint dstZ,
                                                    GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth)
{
    CALL_GL_API(glCopyImageSubData, srcName, srcTarget, srcLevel, srcX, srcY, srcZ, dstName, dstTarget, dstLevel,
                                          dstX, dstY, dstZ, srcWidth, srcHeight, srcDepth);
}


static void GL_APIENTRY API_ENTRY(glDrawElementsBaseVertex)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex)
{
    CALL_GL_API(glDrawElementsBaseVertex, mode, count, type, indices, basevertex);
}

static void GL_APIENTRY API_ENTRY(glDrawRangeElementsBaseVertex)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex)
{
    CALL_GL_API(glDrawRangeElementsBaseVertex, mode, start, end, count, type, indices, basevertex);
}

static void GL_APIENTRY API_ENTRY(glDrawElementsInstancedBaseVertex)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex)
{
    CALL_GL_API(glDrawElementsInstancedBaseVertex, mode, count, type, indices, instancecount, basevertex);
}

static void GL_APIENTRY API_ENTRY(glPrimitiveBoundingBox)(GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW,
                                                        GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW)
{
    CALL_GL_API(glPrimitiveBoundingBox, minX, minY, minZ, minW, maxX, maxY, maxZ, maxW);
}

/*
** OpenGL ES Extensions
*/
static void GL_APIENTRY API_ENTRY(glMultiDrawElementsBaseVertexEXT)(GLenum mode, const GLsizei * count, GLenum type, const void *const*indices, GLsizei drawcount, const GLint *basevertex)
{
    CALL_GL_API(glMultiDrawElementsBaseVertexEXT, mode, count, type, indices, drawcount, basevertex);
}


static void GL_APIENTRY API_ENTRY(glMultiDrawArraysIndirectEXT)(GLenum mode, const void *indirect, GLsizei drawcount, GLsizei stride)
{
    CALL_GL_API(glMultiDrawArraysIndirectEXT, mode, indirect, drawcount, stride);
}

static void GL_APIENTRY API_ENTRY(glMultiDrawElementsIndirectEXT)(GLenum mode, GLenum type, const void *indirect, GLsizei drawcount, GLsizei stride)
{
    CALL_GL_API(glMultiDrawElementsIndirectEXT, mode, type, indirect, drawcount, stride);
}

static void GL_APIENTRY API_ENTRY(glGetTexImage)( GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels)
{
    CALL_GL_API(glGetTexImage, target, level, format, type, pixels);
}

static void GL_APIENTRY API_ENTRY(glAlphaFunc)(GLenum func, GLclampf ref)
{
    CALL_GL_API(glAlphaFunc, func, ref);
}

static void GL_APIENTRY API_ENTRY(glClientActiveTexture)(GLenum texture)
{
    CALL_GL_API(glClientActiveTexture, texture);
}

static void GL_APIENTRY API_ENTRY(glColor4f)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    CALL_GL_API(glColor4f, red, green, blue, alpha);
}

static void GL_APIENTRY API_ENTRY(glColor4ub)(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
    CALL_GL_API(glColor4ub, red, green, blue, alpha);
}

static void GL_APIENTRY API_ENTRY(glColorPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr)
{
    CALL_GL_API(glColorPointer, size, type, stride, ptr);
}

static void GL_APIENTRY API_ENTRY(glDisableClientState)(GLenum cap)
{
    CALL_GL_API(glDisableClientState, cap);
}

static void GL_APIENTRY API_ENTRY(glEnableClientState)(GLenum cap)
{
    CALL_GL_API(glEnableClientState, cap);
}

static void GL_APIENTRY API_ENTRY(glFogf)(GLenum pname, GLfloat param)
{
    CALL_GL_API(glFogf, pname, param);
}

static void GL_APIENTRY API_ENTRY(glFogfv)(GLenum pname, const GLfloat *params)
{
    CALL_GL_API(glFogfv, pname, params);
}

static void GL_APIENTRY API_ENTRY(glGetLightfv)(GLenum light, GLenum pname, GLfloat *params)
{
    CALL_GL_API(glGetLightfv, light, pname, params);
}

static void GL_APIENTRY API_ENTRY(glGetMaterialfv)(GLenum face, GLenum pname, GLfloat *params)
{
    CALL_GL_API(glGetMaterialfv, face, pname, params);
}

static void GL_APIENTRY API_ENTRY(glGetTexEnvfv)(GLenum target, GLenum pname, GLfloat *params)
{
    CALL_GL_API(glGetTexEnvfv, target, pname, params);
}

static void GL_APIENTRY API_ENTRY(glGetTexEnviv)(GLenum target, GLenum pname, GLint *params)
{
    CALL_GL_API(glGetTexEnviv, target, pname, params);
}

static void GL_APIENTRY API_ENTRY(glLightModelf)(GLenum pname, GLfloat param)
{
    CALL_GL_API(glLightModelf, pname, param);
}

static void GL_APIENTRY API_ENTRY(glLightModelfv)(GLenum pname, const GLfloat *params)
{
    CALL_GL_API(glLightModelfv, pname, params);
}

static void GL_APIENTRY API_ENTRY(glLightf)(GLenum light, GLenum pname, GLfloat param)
{
    CALL_GL_API(glLightf, light, pname, param);
}

static void GL_APIENTRY API_ENTRY(glLightfv)(GLenum light, GLenum pname, const GLfloat *params)
{
    CALL_GL_API(glLightfv, light, pname, params);
}

static void GL_APIENTRY API_ENTRY(glLoadIdentity)(GLvoid)
{
    CALL_GL_API(glLoadIdentity);
}

static void GL_APIENTRY API_ENTRY(glLoadMatrixf)(const GLfloat *m)
{
    CALL_GL_API(glLoadMatrixf, m);
}

static void GL_APIENTRY API_ENTRY(glLogicOp)(GLenum opcode)
{
    CALL_GL_API(glLogicOp, opcode);
}

static void GL_APIENTRY API_ENTRY(glMaterialf)(GLenum face, GLenum pname, GLfloat param)
{
    CALL_GL_API(glMaterialf, face, pname, param);
}

static void GL_APIENTRY API_ENTRY(glMaterialfv)(GLenum face, GLenum pname, const GLfloat *params)
{
    CALL_GL_API(glMaterialfv, face, pname, params);
}

static void GL_APIENTRY API_ENTRY(glMatrixMode)(GLenum mode)
{
    CALL_GL_API(glMatrixMode, mode);
}

static void GL_APIENTRY API_ENTRY(glMultMatrixf)(const GLfloat *m)
{
    CALL_GL_API(glMultMatrixf, m);
}

static void GL_APIENTRY API_ENTRY(glMultiTexCoord4f)(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    CALL_GL_API(glMultiTexCoord4f, target, s, t, r, q);
}

static void GL_APIENTRY API_ENTRY(glNormal3f)(GLfloat nx, GLfloat ny, GLfloat nz)
{
    CALL_GL_API(glNormal3f, nx, ny, nz);
}

static void GL_APIENTRY API_ENTRY(glNormalPointer)(GLenum type, GLsizei stride, const GLvoid *ptr)
{
    CALL_GL_API(glNormalPointer, type, stride, ptr);
}

static void GL_APIENTRY API_ENTRY(glPointParameterf)(GLenum pname, GLfloat param)
{
    CALL_GL_API(glPointParameterf, pname, param);
}

static void GL_APIENTRY API_ENTRY(glPointParameterfv)(GLenum pname, const GLfloat *params)
{
    CALL_GL_API(glPointParameterfv, pname, params);
}

static void GL_APIENTRY API_ENTRY(glPointSize)(GLfloat size)
{
    CALL_GL_API(glPointSize, size);
}

static void GL_APIENTRY API_ENTRY(glPopMatrix)(GLvoid)
{
    CALL_GL_API(glPopMatrix);
}

static void GL_APIENTRY API_ENTRY(glPushMatrix)(GLvoid)
{
    CALL_GL_API(glPushMatrix);
}

static void GL_APIENTRY API_ENTRY(glRotatef)(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    CALL_GL_API(glRotatef, angle, x, y, z);
}

static void GL_APIENTRY API_ENTRY(glScalef)(GLfloat x, GLfloat y, GLfloat z)
{
    CALL_GL_API(glScalef, x, y, z);
}

static void GL_APIENTRY API_ENTRY(glShadeModel)(GLenum mode)
{
    CALL_GL_API(glShadeModel, mode);
}

static void GL_APIENTRY API_ENTRY(glTexCoordPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr)
{
    CALL_GL_API(glTexCoordPointer, size, type, stride, ptr);
}

static void GL_APIENTRY API_ENTRY(glTexEnvf)(GLenum target, GLenum pname, GLfloat param)
{
    CALL_GL_API(glTexEnvf, target, pname, param);
}

static void GL_APIENTRY API_ENTRY(glTexEnvfv)(GLenum target, GLenum pname, const GLfloat *params)
{
    CALL_GL_API(glTexEnvfv, target, pname, params);
}

static void GL_APIENTRY API_ENTRY(glTexEnvi)(GLenum target, GLenum pname, GLint param)
{
    CALL_GL_API(glTexEnvi, target, pname, param);
}

static void GL_APIENTRY API_ENTRY(glTexEnviv)(GLenum target, GLenum pname, const GLint *params)
{
    CALL_GL_API(glTexEnviv, target, pname, params);
}

static void GL_APIENTRY API_ENTRY(glTranslatef)(GLfloat x, GLfloat y, GLfloat z)
{
    CALL_GL_API(glTranslatef, x, y, z);
}

static void GL_APIENTRY API_ENTRY(glVertexPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr)
{
    CALL_GL_API(glVertexPointer, size, type, stride, ptr);
}


#undef API_ENTRY
#undef CALL_GL_API
#undef CALL_GL_API_RETURN

typedef struct _veglLOOKUP
{
    const char *                                name;
    __eglMustCastToProperFunctionPointerType    function;
}
veglLOOKUP;

#define eglMAKE_LOOKUP(function) \
    { #function, (__eglMustCastToProperFunctionPointerType) function }

#define forwardGLFunction(__ret, __api, ...)   \
    {#__api, (__eglMustCastToProperFunctionPointerType) forward_##__api},

static veglLOOKUP _veglLookup[] =
{
    eglMAKE_LOOKUP(eglGetError),
    eglMAKE_LOOKUP(eglGetDisplay),
    eglMAKE_LOOKUP(eglInitialize),
    eglMAKE_LOOKUP(eglTerminate),
    eglMAKE_LOOKUP(eglQueryString),
    eglMAKE_LOOKUP(eglGetConfigs),
    eglMAKE_LOOKUP(eglChooseConfig),
    eglMAKE_LOOKUP(eglGetConfigAttrib),
    eglMAKE_LOOKUP(eglCreateWindowSurface),
    eglMAKE_LOOKUP(eglCreatePbufferSurface),
    eglMAKE_LOOKUP(eglCreatePixmapSurface),
    eglMAKE_LOOKUP(eglDestroySurface),
    eglMAKE_LOOKUP(eglQuerySurface),
    eglMAKE_LOOKUP(eglBindAPI),
    eglMAKE_LOOKUP(eglQueryAPI),
    eglMAKE_LOOKUP(eglWaitClient),
    eglMAKE_LOOKUP(eglReleaseThread),
    eglMAKE_LOOKUP(eglCreatePbufferFromClientBuffer),
    eglMAKE_LOOKUP(eglSurfaceAttrib),
    eglMAKE_LOOKUP(eglBindTexImage),
    eglMAKE_LOOKUP(eglReleaseTexImage),
    eglMAKE_LOOKUP(eglSwapInterval),
    eglMAKE_LOOKUP(eglCreateContext),
    eglMAKE_LOOKUP(eglDestroyContext),
    eglMAKE_LOOKUP(eglMakeCurrent),
    eglMAKE_LOOKUP(eglGetCurrentContext),
    eglMAKE_LOOKUP(eglGetCurrentSurface),
    eglMAKE_LOOKUP(eglGetCurrentDisplay),
    eglMAKE_LOOKUP(eglQueryContext),
    eglMAKE_LOOKUP(eglWaitGL),
    eglMAKE_LOOKUP(eglWaitNative),
    eglMAKE_LOOKUP(eglSwapBuffers),
    eglMAKE_LOOKUP(eglCopyBuffers),
    eglMAKE_LOOKUP(eglGetProcAddress),
    /* EGL 1.5 */
    eglMAKE_LOOKUP(eglCreateSync),
    eglMAKE_LOOKUP(eglDestroySync),
    eglMAKE_LOOKUP(eglClientWaitSync),
    eglMAKE_LOOKUP(eglGetSyncAttrib),
    eglMAKE_LOOKUP(eglCreateImage),
    eglMAKE_LOOKUP(eglDestroyImage),
    eglMAKE_LOOKUP(eglGetPlatformDisplay),
    eglMAKE_LOOKUP(eglCreatePlatformWindowSurface),
    eglMAKE_LOOKUP(eglCreatePlatformPixmapSurface),
    eglMAKE_LOOKUP(eglWaitSync),
    /* EGL_KHR_lock_surface. */
    eglMAKE_LOOKUP(eglLockSurfaceKHR),
    eglMAKE_LOOKUP(eglUnlockSurfaceKHR),
    /* EGL_KHR_image. */
    eglMAKE_LOOKUP(eglCreateImageKHR),
    eglMAKE_LOOKUP(eglDestroyImageKHR),
    /* EGL_KHR_fence_sync. */
    eglMAKE_LOOKUP(eglCreateSyncKHR),
    eglMAKE_LOOKUP(eglDestroySyncKHR),
    eglMAKE_LOOKUP(eglClientWaitSyncKHR),
    eglMAKE_LOOKUP(eglGetSyncAttribKHR),
    /* EGL_KHR_reusable_sync. */
    eglMAKE_LOOKUP(eglSignalSyncKHR),
    /* EGL_KHR_wait_sync. */
    eglMAKE_LOOKUP(eglWaitSyncKHR),
    /* EGL_EXT_platform_base. */
    eglMAKE_LOOKUP(eglGetPlatformDisplayEXT),
    eglMAKE_LOOKUP(eglCreatePlatformWindowSurfaceEXT),
    eglMAKE_LOOKUP(eglCreatePlatformPixmapSurfaceEXT),
#if defined(WL_EGL_PLATFORM)
    /* EGL_WL_bind_wayland_display. */
    eglMAKE_LOOKUP(eglBindWaylandDisplayWL),
    eglMAKE_LOOKUP(eglUnbindWaylandDisplayWL),
    eglMAKE_LOOKUP(eglQueryWaylandBufferWL),
#endif
#if defined(__linux__)
    /* EGL_ANDROID_native_fence_sync. */
    eglMAKE_LOOKUP(eglDupNativeFenceFDANDROID),
#endif
    eglMAKE_LOOKUP(eglSetDamageRegionKHR),
    eglMAKE_LOOKUP(eglSwapBuffersWithDamageKHR),
    eglMAKE_LOOKUP(eglSwapBuffersWithDamageEXT),
    eglMAKE_LOOKUP(eglQueryDmaBufFormatsEXT),
    eglMAKE_LOOKUP(eglQueryDmaBufModifiersEXT),
    eglMAKE_LOOKUP(eglPatchID),

    GL_API_ENTRIES(forwardGLFunction)

    { gcvNULL, gcvNULL }
};

static EGL_PROC
_Lookup(
    veglLOOKUP * Lookup,
    const char * Name,
    const char * Appendix
    )
{
    /* Test for lookup. */
    if (Lookup != gcvNULL)
    {
        /* Loop while there are entries in the lookup tabke. */
        while (Lookup->name != gcvNULL)
        {
            const char *p = Name;
            const char *q = Lookup->name;

            /* Compare the name and the lookup table. */
            while ((*p == *q) && (*p != '\0') && (*q != '\0'))
            {
                ++p;
                ++q;
            }

            /* No match yet, see if it matches if we append the appendix. */
            if ((*p != *q) && (*p == '\0') && (Appendix != gcvNULL))
            {
                p = Appendix;

                /* Compare the appendix and the lookup table. */
                while ((*p == *q) && (*p != '\0') && (*q != '\0'))
                {
                    ++p;
                    ++q;
                }
            }

            /* See if we have a match. */
            if (*p == *q)
            {
                /* Return the function pointer. */
                return Lookup->function;
            }

            /* Next lookup entry. */
            ++Lookup;
        }
    }

    /* No match found. */
    return gcvNULL;
}

static int isInApiTraceMode()
{
#ifndef UNDER_CE
    void *fptr = dlsym(RTLD_DEFAULT, "ApiTraceEnabled");
    return (fptr != NULL);
#else
    return 0;
#endif
}

#define gcmDEF2STRING(def) #def

EGLAPI __eglMustCastToProperFunctionPointerType EGLAPIENTRY
eglGetProcAddress(const char *procname)
{
    __eglMustCastToProperFunctionPointerType func = gcvNULL;
    VEGLThreadData thread;

    static const char * appendix[] =
    {
#ifdef _EGL_APPENDIX
        gcmDEF2STRING(_EGL_APPENDIX),
#else
        gcvNULL,
#endif

        /* ES11_CL and ES11. */
#ifdef _GL_11_APPENDIX
        gcmDEF2STRING(_GL_11_APPENDIX),
        gcmDEF2STRING(_GL_11_APPENDIX),
#else
        gcvNULL,
        gcvNULL,
#endif

#ifdef _GL_2_APPENDIX
        gcmDEF2STRING(_GL_2_APPENDIX),
#else
        gcvNULL,
#endif

#ifdef _GL_3_APPENDIX
        gcmDEF2STRING(_GL_3_APPENDIX),
#else
        gcvNULL,
#endif

#ifdef _VG_APPENDIX
        gcmDEF2STRING(_VG_APPENDIX),
#else
        gcvNULL,
#endif

#ifdef _GL_APPENDIX
        gcmDEF2STRING(_GL_APPENDIX),
#else
        gcvNULL,
#endif
    };

    gcmHEADER_ARG("procname=%s", procname);
    VEGL_TRACE_API_PRE(GetProcAddress)(procname);

    do
    {
        veglAPIINDEX index;
        gctHANDLE library;
        veglDISPATCH * dispatch;

        /* Lookup in EGL API. */
        func = _Lookup(_veglLookup, procname, appendix[vegl_EGL]);

        if (func != gcvNULL)
        {
            break;
        }

        /* If apitrace is enabled, do not return function name */
        if (isInApiTraceMode())
        {
            func = gcvNULL;
        }

        thread = veglGetThreadData();
        if (thread == gcvNULL)
        {
            gcmTRACE(gcvLEVEL_ERROR, "%s(%d): veglGetThreadData failed.",
                    __FUNCTION__, __LINE__);

            break;
        }

        /* Go through drivers. */
        for (index = vegl_EGL; index < vegl_API_LAST; index++)
        {
            char rename[128];
            const char *name;

            if (appendix[index])
            {
                gcoOS_StrCopySafe(rename, 128, procname);
                gcoOS_StrCatSafe(rename, 128, appendix[index]);
                name = rename;
            }
            else
            {
                name = procname;
            }

            library  = thread->clientHandles[index];
            dispatch = thread->dispatchTables[index];

            /* If apitrace is enabled, do not use dispatch to get proc address */
            if (isInApiTraceMode())
            {
                dispatch = gcvNULL;
            }

            if (dispatch && dispatch->getProcAddr)
            {
                func = dispatch->getProcAddr(name);

                if (func != gcvNULL)
                {
                    break;
                }
            }

            if (library)
            {
                if (gcmIS_SUCCESS(gcoOS_GetProcAddress(gcvNULL,
                                                       library,
                                                       name,
                                                       (gctPOINTER *) &func)))
                {
                    break;
                }
            }
        }
    }
    while (gcvFALSE);

    VEGL_TRACE_API_POST(GetProcAddress)(procname, func);
    gcmDUMP_API("${EGL eglGetProcAddress (0x%08X) := 0x%08X",
                procname, func);
    gcmDUMP_API_DATA(procname, 0);
    gcmDUMP_API("$}");
    gcmFOOTER_ARG("0x%x", func);
    return func;
}
