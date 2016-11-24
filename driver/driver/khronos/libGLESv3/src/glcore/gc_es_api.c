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


#include "gc_es_context.h"


#define __GLES_APINAME(apiname) gl##apiname

#if gcdPATTERN_FAST_PATH
#define __GL_GET_CONTEXT  __GLcontext *gc = __glGetGLcontext(); if (gc == gcvNULL) { return; }; gc->pattern.apiCount++;
#define __GL_GET_CONTEXT_RET(_value_)  __GLcontext *gc = __glGetGLcontext(); if (gc == gcvNULL) { return _value_; }; gc->pattern.apiCount++;

#define __GL_PATTERN_MATCH_NAME(name) \
{\
    if (gc->pattern.enable && gc->pattern.patternMatchMask != 0) \
    {\
        GLuint i = 0;\
        if (gc->pattern.matchCount++ == gc->pattern.apiCount - 1)\
        {\
            for (i = 0; i < GLES_PATTERN_COUNT; i++)\
            {\
                if (gc->pattern.patternMatchMask & (1 << i))\
                {\
                    if (gc->pattern.patterns[i]->apiCount < gc->pattern.apiCount ||\
                        gc->pattern.patterns[i]->api[gc->pattern.apiCount - 1].apiEnum != name)\
                    {\
                        gc->pattern.patternMatchMask &= ~(1 <<  i);\
                        /*gcmPRINT("match fail");*/\
                    }\
                    else\
                    {\
                        if (gc->pattern.state == GLES_PATTERN_STATE_MATCHED)\
                        {\
                            /*gcmPRINT("replaying match");*/\
                        }\
                        else\
                        {\
                            /*gcmPRINT("do match");*/\
                        }\
                    }\
                }\
            }\
        }\
        else\
        {\
            gc->pattern.patternMatchMask = 0;\
        }\
    }\
}

#define __GL_PATTERN_MATCH_NAME_WITH_ARG(name, arg) \
{\
    if (gc->pattern.enable && gc->pattern.patternMatchMask != 0) \
    {\
        GLuint i = 0;\
        if (gc->pattern.matchCount++ == gc->pattern.apiCount - 1)\
        {\
            for (i = 0; i < GLES_PATTERN_COUNT; i++)\
            {\
                if (gc->pattern.patternMatchMask & (1 << i))\
                {\
                    if (gc->pattern.patterns[i]->apiCount < gc->pattern.apiCount ||\
                        gc->pattern.patterns[i]->api[gc->pattern.apiCount - 1].apiEnum != name ||\
                        gc->pattern.patterns[i]->api[gc->pattern.apiCount - 1].param[0] != arg)\
                    {\
                        gc->pattern.patternMatchMask &= ~(1 <<  i);\
                        /*gcmPRINT("match fail");*/\
                    }\
                    else\
                    {\
                        if (gc->pattern.state == GLES_PATTERN_STATE_MATCHED)\
                        {\
                            /*gcmPRINT("replaying match");*/\
                        }\
                        else\
                        {\
                            /*gcmPRINT("do match");*/\
                        }\
                    }\
                }\
            }\
        }\
        else\
        {\
            gc->pattern.patternMatchMask = 0;\
        }\
    }\
}

#define __GL_PATTERN_MATCH_NAME_WITH_ARG2(name, arg, arg1) \
{\
    if (gc->pattern.enable && gc->pattern.patternMatchMask != 0) \
    {\
        GLuint i = 0;\
        if (gc->pattern.matchCount++ == gc->pattern.apiCount - 1)\
        {\
            for (i = 0; i < GLES_PATTERN_COUNT; i++)\
            {\
                if (gc->pattern.patternMatchMask & (1 << i))\
                {\
                    if (gc->pattern.patterns[i]->apiCount < gc->pattern.apiCount ||\
                        gc->pattern.patterns[i]->api[gc->pattern.apiCount - 1].apiEnum != name ||\
                        gc->pattern.patterns[i]->api[gc->pattern.apiCount - 1].param[0] != arg||\
                        gc->pattern.patterns[i]->api[gc->pattern.apiCount - 1].param[1] != arg1)\
                    {\
                        gc->pattern.patternMatchMask &= ~(1 <<  i);\
                        /*gcmPRINT("match fail");*/\
                    }\
                    else\
                    {\
                        if (gc->pattern.state == GLES_PATTERN_STATE_MATCHED)\
                        {\
                            /*gcmPRINT("replaying match");*/\
                        }\
                        else\
                        {\
                            /*gcmPRINT("do match");*/\
                        }\
                    }\
                }\
            }\
        }\
        else\
        {\
            gc->pattern.patternMatchMask = 0;\
        }\
    }\
}

#else
#define __GL_GET_CONTEXT  __GLcontext *gc = __glGetGLcontext(); if (gc == gcvNULL) { return; };
#define __GL_GET_CONTEXT_RET(_value_)  __GLcontext *gc = __glGetGLcontext(); if (gc == gcvNULL) { return _value_; };
#define __GL_PATTERN_MATCH_NAME(name)
#define __GL_PATTERN_MATCH_NAME_WITH_ARG(name,arg)
#define __GL_PATTERN_MATCH_NAME_WITH_ARG2(name, arg, arg1)
#endif
/*
** OpenGL ES 2.0
*/

GLvoid GL_APIENTRY __GLES_APINAME(ActiveTexture)(GLenum texture)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ActiveTexture(gc, texture);
}

GLvoid GL_APIENTRY __GLES_APINAME(AttachShader)(GLuint program, GLuint shader)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.AttachShader(gc, program, shader);
}

GLvoid GL_APIENTRY __GLES_APINAME(BindAttribLocation)(GLuint program, GLuint index, const GLchar* name)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BindAttribLocation(gc, program, index, name);
}

GLvoid GL_APIENTRY __GLES_APINAME(BindBuffer)(GLenum target, GLuint buffer)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BindBuffer(gc, target, buffer);
    __GL_PATTERN_MATCH_NAME(__glesApiEnum(BindBuffer));
}

GLvoid GL_APIENTRY __GLES_APINAME(BindFramebuffer)(GLenum target, GLuint framebuffer)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BindFramebuffer(gc, target, framebuffer);
}

GLvoid GL_APIENTRY __GLES_APINAME(BindRenderbuffer)(GLenum target, GLuint renderbuffer)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BindRenderbuffer(gc, target, renderbuffer);
}

GLvoid GL_APIENTRY __GLES_APINAME(BindTexture)(GLenum target, GLuint texture)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BindTexture(gc, target, texture);
}

GLvoid GL_APIENTRY __GLES_APINAME(BlendColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BlendColor(gc, red, green, blue, alpha);
    __GL_PATTERN_MATCH_NAME(__glesApiEnum(BlendColor));
}

GLvoid GL_APIENTRY __GLES_APINAME(BlendEquation)(GLenum mode)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BlendEquation(gc, mode);
}

GLvoid GL_APIENTRY __GLES_APINAME(BlendEquationSeparate)(GLenum modeRGB, GLenum modeAlpha)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BlendEquationSeparate(gc, modeRGB, modeAlpha);
}

GLvoid GL_APIENTRY __GLES_APINAME(BlendFunc)(GLenum sfactor, GLenum dfactor)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BlendFunc(gc, sfactor, dfactor);
    __GL_PATTERN_MATCH_NAME(__glesApiEnum(BlendFunc));
}

GLvoid GL_APIENTRY __GLES_APINAME(BlendFuncSeparate)(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BlendFuncSeparate(gc, srcRGB, dstRGB, srcAlpha, dstAlpha);
}

GLvoid GL_APIENTRY __GLES_APINAME(BufferData)(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BufferData(gc, target, size, data, usage);
}

GLvoid GL_APIENTRY __GLES_APINAME(BufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BufferSubData(gc, target, offset, size, data);
}

GLenum GL_APIENTRY __GLES_APINAME(CheckFramebufferStatus)(GLenum target)
{
    __GL_GET_CONTEXT_RET(0);
    return gc->apiDispatchTable.CheckFramebufferStatus(gc, target);
}

GLvoid GL_APIENTRY __GLES_APINAME(Clear)(GLbitfield mask)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Clear(gc, mask);
}

GLvoid GL_APIENTRY __GLES_APINAME(ClearColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ClearColor(gc, red, green, blue, alpha);
}

GLvoid GL_APIENTRY __GLES_APINAME(ClearDepthf)(GLfloat depth)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ClearDepthf(gc, depth);
}

GLvoid GL_APIENTRY __GLES_APINAME(ClearStencil)(GLint s)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ClearStencil(gc, s);
}

GLvoid GL_APIENTRY __GLES_APINAME(ColorMask)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ColorMask(gc, red, green, blue, alpha);
}

GLvoid GL_APIENTRY __GLES_APINAME(CompileShader)(GLuint shader)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.CompileShader(gc, shader);
}

GLvoid GL_APIENTRY __GLES_APINAME(CompressedTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.CompressedTexImage2D(gc, target, level, internalformat, width, height, border, imageSize, data);
}

GLvoid GL_APIENTRY __GLES_APINAME(CompressedTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid* data)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.CompressedTexSubImage2D(gc, target, level, xoffset, yoffset, width, height, format, imageSize, data);
}

GLvoid GL_APIENTRY __GLES_APINAME(CopyTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.CopyTexImage2D(gc, target, level, internalformat, x, y, width, height, border);
}

GLvoid GL_APIENTRY __GLES_APINAME(CopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.CopyTexSubImage2D(gc, target, level, xoffset, yoffset, x, y, width, height);
}

GLuint GL_APIENTRY __GLES_APINAME(CreateProgram)(GLvoid)
{
    __GL_GET_CONTEXT_RET(0);
    return gc->apiDispatchTable.CreateProgram(gc);
}

GLuint GL_APIENTRY __GLES_APINAME(CreateShader)(GLenum type)
{
    __GL_GET_CONTEXT_RET(0);
    return gc->apiDispatchTable.CreateShader(gc, type);
}

GLvoid GL_APIENTRY __GLES_APINAME(CullFace)(GLenum mode)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.CullFace(gc, mode);
}

GLvoid GL_APIENTRY __GLES_APINAME(DeleteBuffers)(GLsizei n, const GLuint* buffers)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DeleteBuffers(gc, n, buffers);
}

GLvoid GL_APIENTRY __GLES_APINAME(DeleteFramebuffers)(GLsizei n, const GLuint* framebuffers)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DeleteFramebuffers(gc, n, framebuffers);
}

GLvoid GL_APIENTRY __GLES_APINAME(DeleteProgram)(GLuint program)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DeleteProgram(gc, program);
}

GLvoid GL_APIENTRY __GLES_APINAME(DeleteRenderbuffers)(GLsizei n, const GLuint* renderbuffers)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DeleteRenderbuffers(gc, n, renderbuffers);
}

GLvoid GL_APIENTRY __GLES_APINAME(DeleteShader)(GLuint shader)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DeleteShader(gc, shader);
}

GLvoid GL_APIENTRY __GLES_APINAME(DeleteTextures)(GLsizei n, const GLuint* textures)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DeleteTextures(gc, n, textures);
}

GLvoid GL_APIENTRY __GLES_APINAME(DepthFunc)(GLenum func)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DepthFunc(gc, func);
    __GL_PATTERN_MATCH_NAME(__glesApiEnum(DepthFunc));
}

GLvoid GL_APIENTRY __GLES_APINAME(DepthMask)(GLboolean flag)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DepthMask(gc, flag);
}

GLvoid GL_APIENTRY __GLES_APINAME(DepthRangef)(GLfloat n, GLfloat f)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DepthRangef(gc, n, f);
}

GLvoid GL_APIENTRY __GLES_APINAME(DetachShader)(GLuint program, GLuint shader)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DetachShader(gc, program, shader);
}

GLvoid GL_APIENTRY __GLES_APINAME(Disable)(GLenum cap)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Disable(gc, cap);
    __GL_PATTERN_MATCH_NAME_WITH_ARG(__glesApiEnum(Disable), cap);
}

GLvoid GL_APIENTRY __GLES_APINAME(DisableVertexAttribArray)(GLuint index)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DisableVertexAttribArray(gc, index);
}

GLvoid GL_APIENTRY __GLES_APINAME(DrawArrays)(GLenum mode, GLint first, GLsizei count)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DrawArrays(gc, mode, first, count);
}

GLvoid GL_APIENTRY __GLES_APINAME(DrawElements)(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices)
{
    __GL_GET_CONTEXT;
    __GL_PATTERN_MATCH_NAME_WITH_ARG2(__glesApiEnum(DrawElements),mode, type);
    gc->apiDispatchTable.DrawElements(gc, mode, count, type, indices);
}

GLvoid GL_APIENTRY __GLES_APINAME(Enable)(GLenum cap)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Enable(gc, cap);
    __GL_PATTERN_MATCH_NAME_WITH_ARG(__glesApiEnum(Enable),cap);
}

GLvoid GL_APIENTRY __GLES_APINAME(EnableVertexAttribArray)(GLuint index)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.EnableVertexAttribArray(gc, index);
}

GLvoid GL_APIENTRY __GLES_APINAME(Finish)(GLvoid)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Finish(gc);
}

GLvoid GL_APIENTRY __GLES_APINAME(Flush)(GLvoid)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Flush(gc);
}

GLvoid GL_APIENTRY __GLES_APINAME(FramebufferRenderbuffer)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.FramebufferRenderbuffer(gc, target, attachment, renderbuffertarget, renderbuffer);
}

GLvoid GL_APIENTRY __GLES_APINAME(FramebufferTexture2D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.FramebufferTexture2D(gc, target, attachment, textarget, texture, level);
}

GLvoid GL_APIENTRY __GLES_APINAME(FrontFace)(GLenum mode)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.FrontFace(gc, mode);
}

GLvoid GL_APIENTRY __GLES_APINAME(GenBuffers)(GLsizei n, GLuint* buffers)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GenBuffers(gc, n, buffers);
}

GLvoid GL_APIENTRY __GLES_APINAME(GenerateMipmap)(GLenum target)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GenerateMipmap(gc, target);
}

GLvoid GL_APIENTRY __GLES_APINAME(GenFramebuffers)(GLsizei n, GLuint* framebuffers)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GenFramebuffers(gc, n, framebuffers);
}

GLvoid GL_APIENTRY __GLES_APINAME(GenRenderbuffers)(GLsizei n, GLuint* renderbuffers)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GenRenderbuffers(gc, n, renderbuffers);
}

GLvoid GL_APIENTRY __GLES_APINAME(GenTextures)(GLsizei n, GLuint* textures)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GenTextures(gc, n, textures);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetActiveAttrib)(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetActiveAttrib(gc, program, index, bufsize, length, size, type, name);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetActiveUniform)(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetActiveUniform(gc, program, index, bufsize, length, size, type, name);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetAttachedShaders)(GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetAttachedShaders(gc, program, maxcount, count, shaders);
}

GLint GL_APIENTRY __GLES_APINAME(GetAttribLocation)(GLuint program, const GLchar* name)
{
    __GL_GET_CONTEXT_RET(GL_INVALID_OPERATION);
    return gc->apiDispatchTable.GetAttribLocation(gc, program, name);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetBooleanv)(GLenum pname, GLboolean* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetBooleanv(gc, pname, params);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetBufferParameteriv)(GLenum target, GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetBufferParameteriv(gc, target, pname, params);
}

GLenum GL_APIENTRY __GLES_APINAME(GetError)(GLvoid)
{
    __GL_GET_CONTEXT_RET(GL_NO_ERROR);
    return gc->apiDispatchTable.GetError(gc);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetFloatv)(GLenum pname, GLfloat* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetFloatv(gc, pname, params);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetFramebufferAttachmentParameteriv)(GLenum target, GLenum attachment, GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetFramebufferAttachmentParameteriv(gc, target, attachment, pname, params);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetIntegerv)(GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetIntegerv(gc, pname, params);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetProgramiv)(GLuint program, GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetProgramiv(gc, program, pname, params);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetProgramInfoLog)(GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetProgramInfoLog(gc, program, bufsize, length, infolog);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetRenderbufferParameteriv)(GLenum target, GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetRenderbufferParameteriv(gc, target, pname, params);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetShaderiv)(GLuint shader, GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetShaderiv(gc, shader, pname, params);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetShaderInfoLog)(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetShaderInfoLog(gc, shader, bufsize, length, infolog);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetShaderPrecisionFormat)(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetShaderPrecisionFormat(gc, shadertype, precisiontype, range, precision);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetShaderSource)(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetShaderSource(gc, shader, bufsize, length, source);
}

const GLubyte* GL_APIENTRY __GLES_APINAME(GetString)(GLenum name)
{
    __GLcontext *gc = __glGetGLcontext();

    if (gc == gcvNULL)
    {
        return gcvNULL;
    }

    if (gc->magic != ES3X_MAGIC)
    {
        switch (name)
        {
        case GL_VERSION:
            return (GLubyte*)"OpenGL ES 1.1";
        default:
            return(GLubyte*)" ";
        }
    }
    gc->pattern.apiCount++;

    return gc->apiDispatchTable.GetString(gc, name);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetTexParameterfv)(GLenum target, GLenum pname, GLfloat* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetTexParameterfv(gc, target, pname, params);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetTexParameteriv)(GLenum target, GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetTexParameteriv(gc, target, pname, params);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetUniformfv)(GLuint program, GLint location, GLfloat* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetUniformfv(gc, program, location, params);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetUniformiv)(GLuint program, GLint location, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetUniformiv(gc, program, location, params);
}

GLint GL_APIENTRY __GLES_APINAME(GetUniformLocation)(GLuint program, const GLchar* name)
{
    __GL_GET_CONTEXT_RET(GL_INVALID_OPERATION);
    return gc->apiDispatchTable.GetUniformLocation(gc, program, name);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetVertexAttribfv)(GLuint index, GLenum pname, GLfloat* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetVertexAttribfv(gc, index, pname, params);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetVertexAttribiv)(GLuint index, GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetVertexAttribiv(gc, index, pname, params);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetVertexAttribPointerv)(GLuint index, GLenum pname, GLvoid** pointer)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetVertexAttribPointerv(gc, index, pname, pointer);
}

GLvoid GL_APIENTRY __GLES_APINAME(Hint)(GLenum target, GLenum mode)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Hint(gc, target, mode);
}

GLboolean GL_APIENTRY __GLES_APINAME(IsBuffer)(GLuint buffer)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->apiDispatchTable.IsBuffer(gc, buffer);
}

GLboolean GL_APIENTRY __GLES_APINAME(IsEnabled)(GLenum cap)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->apiDispatchTable.IsEnabled(gc, cap);
}

GLboolean GL_APIENTRY __GLES_APINAME(IsFramebuffer)(GLuint framebuffer)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->apiDispatchTable.IsFramebuffer(gc, framebuffer);
}

GLboolean GL_APIENTRY __GLES_APINAME(IsProgram)(GLuint program)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->apiDispatchTable.IsProgram(gc, program);
}

GLboolean GL_APIENTRY __GLES_APINAME(IsRenderbuffer)(GLuint renderbuffer)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->apiDispatchTable.IsRenderbuffer(gc, renderbuffer);
}

GLboolean GL_APIENTRY __GLES_APINAME(IsShader)(GLuint shader)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->apiDispatchTable.IsShader(gc, shader);
}

GLboolean GL_APIENTRY __GLES_APINAME(IsTexture)(GLuint texture)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->apiDispatchTable.IsTexture(gc, texture);
}

GLvoid GL_APIENTRY __GLES_APINAME(LineWidth)(GLfloat width)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.LineWidth(gc, width);
}

GLvoid GL_APIENTRY __GLES_APINAME(LinkProgram)(GLuint program)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.LinkProgram(gc, program);
}

GLvoid GL_APIENTRY __GLES_APINAME(PixelStorei)(GLenum pname, GLint param)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.PixelStorei(gc, pname, param);
}

GLvoid GL_APIENTRY __GLES_APINAME(PolygonOffset)(GLfloat factor, GLfloat units)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.PolygonOffset(gc, factor, units);
}

GLvoid GL_APIENTRY __GLES_APINAME(ReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ReadPixels(gc, x, y, width, height, format, type, pixels);
}

GLvoid GL_APIENTRY __GLES_APINAME(ReleaseShaderCompiler)(GLvoid)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ReleaseShaderCompiler(gc);
}

GLvoid GL_APIENTRY __GLES_APINAME(RenderbufferStorage)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.RenderbufferStorage(gc, target, internalformat, width, height);
}

GLvoid GL_APIENTRY __GLES_APINAME(SampleCoverage)(GLfloat value, GLboolean invert)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.SampleCoverage(gc, value, invert);
}

GLvoid GL_APIENTRY __GLES_APINAME(Scissor)(GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Scissor(gc, x, y, width, height);
}

GLvoid GL_APIENTRY __GLES_APINAME(ShaderBinary)(GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ShaderBinary(gc, n, shaders, binaryformat, binary, length);
}

GLvoid GL_APIENTRY __GLES_APINAME(ShaderSource)(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ShaderSource(gc, shader, count, string, length);
}

GLvoid GL_APIENTRY __GLES_APINAME(StencilFunc)(GLenum func, GLint ref, GLuint mask)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.StencilFunc(gc, func, ref, mask);
}

GLvoid GL_APIENTRY __GLES_APINAME(StencilFuncSeparate)(GLenum face, GLenum func, GLint ref, GLuint mask)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.StencilFuncSeparate(gc, face, func, ref, mask);
}

GLvoid GL_APIENTRY __GLES_APINAME(StencilMask)(GLuint mask)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.StencilMask(gc, mask);
}

GLvoid GL_APIENTRY __GLES_APINAME(StencilMaskSeparate)(GLenum face, GLuint mask)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.StencilMaskSeparate(gc, face, mask);
}

GLvoid GL_APIENTRY __GLES_APINAME(StencilOp)(GLenum fail, GLenum zfail, GLenum zpass)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.StencilOp(gc, fail, zfail, zpass);
}

GLvoid GL_APIENTRY __GLES_APINAME(StencilOpSeparate)(GLenum face, GLenum fail, GLenum zfail, GLenum zpass)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.StencilOpSeparate(gc, face, fail, zfail, zpass);
}

GLvoid GL_APIENTRY __GLES_APINAME(TexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexImage2D(gc, target, level, internalformat, width, height, border, format, type, pixels);
}

GLvoid GL_APIENTRY __GLES_APINAME(TexParameterf)(GLenum target, GLenum pname, GLfloat param)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexParameterf(gc, target, pname, param);
}

GLvoid GL_APIENTRY __GLES_APINAME(TexParameterfv)(GLenum target, GLenum pname, const GLfloat* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexParameterfv(gc, target, pname, params);
}

GLvoid GL_APIENTRY __GLES_APINAME(TexParameteri)(GLenum target, GLenum pname, GLint param)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexParameteri(gc, target, pname, param);
}

GLvoid GL_APIENTRY __GLES_APINAME(TexParameteriv)(GLenum target, GLenum pname, const GLint* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexParameteriv(gc, target, pname, params);
}

GLvoid GL_APIENTRY __GLES_APINAME(TexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexSubImage2D(gc, target, level, xoffset, yoffset, width, height, format, type, pixels);
}

GLvoid GL_APIENTRY __GLES_APINAME(Uniform1f)(GLint location, GLfloat x)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform1f(gc, location, x);
    __GL_PATTERN_MATCH_NAME(__glesApiEnum(Uniform1f));
}

GLvoid GL_APIENTRY __GLES_APINAME(Uniform1fv)(GLint location, GLsizei count, const GLfloat* v)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform1fv(gc, location, count, v);
}

GLvoid GL_APIENTRY __GLES_APINAME(Uniform1i)(GLint location, GLint x)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform1i(gc, location, x);
}

GLvoid GL_APIENTRY __GLES_APINAME(Uniform1iv)(GLint location, GLsizei count, const GLint* v)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform1iv(gc, location, count, v);
}

GLvoid GL_APIENTRY __GLES_APINAME(Uniform2f)(GLint location, GLfloat x, GLfloat y)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform2f(gc, location, x, y);
    __GL_PATTERN_MATCH_NAME(__glesApiEnum(Uniform2f));
}

GLvoid GL_APIENTRY __GLES_APINAME(Uniform2fv)(GLint location, GLsizei count, const GLfloat* v)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform2fv(gc, location, count, v);
}

GLvoid GL_APIENTRY __GLES_APINAME(Uniform2i)(GLint location, GLint x, GLint y)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform2i(gc, location, x, y);
}

GLvoid GL_APIENTRY __GLES_APINAME(Uniform2iv)(GLint location, GLsizei count, const GLint* v)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform2iv(gc, location, count, v);
}

GLvoid GL_APIENTRY __GLES_APINAME(Uniform3f)(GLint location, GLfloat x, GLfloat y, GLfloat z)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform3f(gc, location, x, y, z);
}

GLvoid GL_APIENTRY __GLES_APINAME(Uniform3fv)(GLint location, GLsizei count, const GLfloat* v)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform3fv(gc, location, count, v);
}

GLvoid GL_APIENTRY __GLES_APINAME(Uniform3i)(GLint location, GLint x, GLint y, GLint z)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform3i(gc, location, x, y, z);
}

GLvoid GL_APIENTRY __GLES_APINAME(Uniform3iv)(GLint location, GLsizei count, const GLint* v)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform3iv(gc, location, count, v);
}

GLvoid GL_APIENTRY __GLES_APINAME(Uniform4f)(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform4f(gc, location, x, y, z, w);
    __GL_PATTERN_MATCH_NAME(__glesApiEnum(Uniform4f));
}

GLvoid GL_APIENTRY __GLES_APINAME(Uniform4fv)(GLint location, GLsizei count, const GLfloat* v)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform4fv(gc, location, count, v);
}

GLvoid GL_APIENTRY __GLES_APINAME(Uniform4i)(GLint location, GLint x, GLint y, GLint z, GLint w)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform4i(gc, location, x, y, z, w);
}

GLvoid GL_APIENTRY __GLES_APINAME(Uniform4iv)(GLint location, GLsizei count, const GLint* v)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform4iv(gc, location, count, v);
}

GLvoid GL_APIENTRY __GLES_APINAME(UniformMatrix2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.UniformMatrix2fv(gc, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(UniformMatrix3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.UniformMatrix3fv(gc, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(UniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.UniformMatrix4fv(gc, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(UseProgram)(GLuint program)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.UseProgram(gc, program);
}

GLvoid GL_APIENTRY __GLES_APINAME(ValidateProgram)(GLuint program)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ValidateProgram(gc, program);
}

GLvoid GL_APIENTRY __GLES_APINAME(VertexAttrib1f)(GLuint indx, GLfloat x)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.VertexAttrib1f(gc, indx, x);
}

GLvoid GL_APIENTRY __GLES_APINAME(VertexAttrib1fv)(GLuint indx, const GLfloat* values)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.VertexAttrib1fv(gc, indx, values);
}

GLvoid GL_APIENTRY __GLES_APINAME(VertexAttrib2f)(GLuint indx, GLfloat x, GLfloat y)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.VertexAttrib2f(gc, indx, x, y);
}

GLvoid GL_APIENTRY __GLES_APINAME(VertexAttrib2fv)(GLuint indx, const GLfloat* values)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.VertexAttrib2fv(gc, indx, values);
}

GLvoid GL_APIENTRY __GLES_APINAME(VertexAttrib3f)(GLuint indx, GLfloat x, GLfloat y, GLfloat z)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.VertexAttrib3f(gc, indx, x, y, z);
}

GLvoid GL_APIENTRY __GLES_APINAME(VertexAttrib3fv)(GLuint indx, const GLfloat* values)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.VertexAttrib3fv(gc, indx, values);
}

GLvoid GL_APIENTRY __GLES_APINAME(VertexAttrib4f)(GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.VertexAttrib4f(gc, indx, x, y, z, w);
}

GLvoid GL_APIENTRY __GLES_APINAME(VertexAttrib4fv)(GLuint indx, const GLfloat* values)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.VertexAttrib4fv(gc, indx, values);
}

GLvoid GL_APIENTRY __GLES_APINAME(VertexAttribPointer)(GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.VertexAttribPointer(gc, indx, size, type, normalized, stride, ptr);
    __GL_PATTERN_MATCH_NAME_WITH_ARG2(__glesApiEnum(VertexAttribPointer), type, normalized);
}

GLvoid GL_APIENTRY __GLES_APINAME(Viewport)(GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Viewport(gc, x, y, width, height);
}


/*
** OpenGL ES 3.0
*/

GLvoid GL_APIENTRY __GLES_APINAME(ReadBuffer)(GLenum mode)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ReadBuffer(gc, mode);
}

GLvoid GL_APIENTRY __GLES_APINAME(DrawRangeElements)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DrawRangeElements(gc, mode, start, end, count, type, indices);
}

GLvoid GL_APIENTRY __GLES_APINAME(TexImage3D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexImage3D(gc, target, level, internalformat, width, height, depth, border, format, type, pixels);
}

GLvoid GL_APIENTRY __GLES_APINAME(TexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexSubImage3D(gc, target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
}

GLvoid GL_APIENTRY __GLES_APINAME(CopyTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.CopyTexSubImage3D(gc, target, level, xoffset, yoffset, zoffset, x, y, width, height);
}

GLvoid GL_APIENTRY __GLES_APINAME(CompressedTexImage3D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.CompressedTexImage3D(gc, target, level, internalformat, width, height, depth, border, imageSize, data);
}

GLvoid GL_APIENTRY __GLES_APINAME(CompressedTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.CompressedTexSubImage3D(gc, target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
}

GLvoid GL_APIENTRY __GLES_APINAME(GenQueries)(GLsizei n, GLuint* ids)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GenQueries(gc, n, ids);
}

GLvoid GL_APIENTRY __GLES_APINAME(DeleteQueries)(GLsizei n, const GLuint* ids)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DeleteQueries(gc, n, ids);
}

GLboolean GL_APIENTRY __GLES_APINAME(IsQuery)(GLuint id)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->apiDispatchTable.IsQuery(gc, id);
}

GLvoid GL_APIENTRY __GLES_APINAME(BeginQuery)(GLenum target, GLuint id)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BeginQuery(gc, target, id);
}

GLvoid GL_APIENTRY __GLES_APINAME(EndQuery)(GLenum target)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.EndQuery(gc, target);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetQueryiv)(GLenum target, GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetQueryiv(gc, target, pname, params);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetQueryObjectuiv)(GLuint id, GLenum pname, GLuint* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetQueryObjectuiv(gc, id, pname, params);
}

GLboolean GL_APIENTRY __GLES_APINAME(UnmapBuffer)(GLenum target)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->apiDispatchTable.UnmapBuffer(gc, target);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetBufferPointerv)(GLenum target, GLenum pname, GLvoid** params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetBufferPointerv(gc, target, pname, params);
}

GLvoid GL_APIENTRY __GLES_APINAME(DrawBuffers)(GLsizei n, const GLenum* bufs)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DrawBuffers(gc, n, bufs);
}

GLvoid GL_APIENTRY __GLES_APINAME(UniformMatrix2x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.UniformMatrix2x3fv(gc, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(UniformMatrix3x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.UniformMatrix3x2fv(gc, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(UniformMatrix2x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.UniformMatrix2x4fv(gc, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(UniformMatrix4x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.UniformMatrix4x2fv(gc, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(UniformMatrix3x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.UniformMatrix3x4fv(gc, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(UniformMatrix4x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.UniformMatrix4x3fv(gc, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(BlitFramebuffer)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BlitFramebuffer(gc, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
}

GLvoid GL_APIENTRY __GLES_APINAME(RenderbufferStorageMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.RenderbufferStorageMultisample(gc, target, samples, internalformat, width, height);
}

GLvoid GL_APIENTRY __GLES_APINAME(FramebufferTextureLayer)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.FramebufferTextureLayer(gc, target, attachment, texture, level, layer);
}

GLvoid* GL_APIENTRY __GLES_APINAME(MapBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
    __GL_GET_CONTEXT_RET(gcvNULL);
    return gc->apiDispatchTable.MapBufferRange(gc, target, offset, length, access);
}

GLvoid GL_APIENTRY __GLES_APINAME(FlushMappedBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.FlushMappedBufferRange(gc, target, offset, length);
}

GLvoid GL_APIENTRY __GLES_APINAME(BindVertexArray)(GLuint array)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BindVertexArray(gc, array);
}

GLvoid GL_APIENTRY __GLES_APINAME(DeleteVertexArrays)(GLsizei n, const GLuint* arrays)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DeleteVertexArrays(gc, n, arrays);
}

GLvoid GL_APIENTRY __GLES_APINAME(GenVertexArrays)(GLsizei n, GLuint* arrays)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GenVertexArrays(gc, n, arrays);
}

GLboolean GL_APIENTRY __GLES_APINAME(IsVertexArray)(GLuint array)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->apiDispatchTable.IsVertexArray(gc, array);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetIntegeri_v)(GLenum target, GLuint index, GLint* data)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetIntegeri_v(gc, target, index, data);
}

GLvoid GL_APIENTRY __GLES_APINAME(BeginTransformFeedback)(GLenum primitiveMode)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BeginTransformFeedback(gc, primitiveMode);
}

GLvoid GL_APIENTRY __GLES_APINAME(EndTransformFeedback)(GLvoid)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.EndTransformFeedback(gc);
}

GLvoid GL_APIENTRY __GLES_APINAME(BindBufferRange)(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BindBufferRange(gc, target, index, buffer, offset, size);
}

GLvoid GL_APIENTRY __GLES_APINAME(BindBufferBase)(GLenum target, GLuint index, GLuint buffer)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BindBufferBase(gc, target, index, buffer);
}

GLvoid GL_APIENTRY __GLES_APINAME(TransformFeedbackVaryings)(GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TransformFeedbackVaryings(gc, program, count, varyings, bufferMode);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetTransformFeedbackVarying)(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetTransformFeedbackVarying(gc, program, index, bufSize, length, size, type, name);
}

GLvoid GL_APIENTRY __GLES_APINAME(VertexAttribIPointer)(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.VertexAttribIPointer(gc, index, size, type, stride, pointer);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetVertexAttribIiv)(GLuint index, GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetVertexAttribIiv(gc, index, pname, params);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetVertexAttribIuiv)(GLuint index, GLenum pname, GLuint* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetVertexAttribIuiv(gc, index, pname, params);
}

GLvoid GL_APIENTRY __GLES_APINAME(VertexAttribI4i)(GLuint index, GLint x, GLint y, GLint z, GLint w)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.VertexAttribI4i(gc, index, x, y, z, w);
}

GLvoid GL_APIENTRY __GLES_APINAME(VertexAttribI4ui)(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.VertexAttribI4ui(gc, index, x, y, z, w);
}

GLvoid GL_APIENTRY __GLES_APINAME(VertexAttribI4iv)(GLuint index, const GLint* v)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.VertexAttribI4iv(gc, index, v);
}

GLvoid GL_APIENTRY __GLES_APINAME(VertexAttribI4uiv)(GLuint index, const GLuint* v)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.VertexAttribI4uiv(gc, index, v);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetUniformuiv)(GLuint program, GLint location, GLuint* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetUniformuiv(gc, program, location, params);
}

GLint GL_APIENTRY __GLES_APINAME(GetFragDataLocation)(GLuint program, const GLchar *name)
{
    __GL_GET_CONTEXT_RET(GL_INVALID_OPERATION);
    return gc->apiDispatchTable.GetFragDataLocation(gc, program, name);
}

GLvoid GL_APIENTRY __GLES_APINAME(Uniform1ui)(GLint location, GLuint v0)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform1ui(gc, location, v0);
}

GLvoid GL_APIENTRY __GLES_APINAME(Uniform2ui)(GLint location, GLuint v0, GLuint v1)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform2ui(gc, location, v0, v1);
}

GLvoid GL_APIENTRY __GLES_APINAME(Uniform3ui)(GLint location, GLuint v0, GLuint v1, GLuint v2)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform3ui(gc, location, v0, v1, v2);
}

GLvoid GL_APIENTRY __GLES_APINAME(Uniform4ui)(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform4ui(gc, location, v0, v1, v2, v3);
}

GLvoid GL_APIENTRY __GLES_APINAME(Uniform1uiv)(GLint location, GLsizei count, const GLuint* value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform1uiv(gc, location, count, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(Uniform2uiv)(GLint location, GLsizei count, const GLuint* value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform2uiv(gc, location, count, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(Uniform3uiv)(GLint location, GLsizei count, const GLuint* value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform3uiv(gc, location, count, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(Uniform4uiv)(GLint location, GLsizei count, const GLuint* value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform4uiv(gc, location, count, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(ClearBufferiv)(GLenum buffer, GLint drawbuffer, const GLint* value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ClearBufferiv(gc, buffer, drawbuffer, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(ClearBufferuiv)(GLenum buffer, GLint drawbuffer, const GLuint* value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ClearBufferuiv(gc, buffer, drawbuffer, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(ClearBufferfv)(GLenum buffer, GLint drawbuffer, const GLfloat* value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ClearBufferfv(gc, buffer, drawbuffer, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(ClearBufferfi)(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ClearBufferfi(gc, buffer, drawbuffer, depth, stencil);
}

const GLubyte* GL_APIENTRY __GLES_APINAME(GetStringi)(GLenum name, GLuint index)
{
    __GL_GET_CONTEXT_RET(gcvNULL);
    return gc->apiDispatchTable.GetStringi(gc, name, index);
}

GLvoid GL_APIENTRY __GLES_APINAME(CopyBufferSubData)(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.CopyBufferSubData(gc, readTarget, writeTarget, readOffset, writeOffset, size);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetUniformIndices)(GLuint program, GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetUniformIndices(gc, program, uniformCount, uniformNames, uniformIndices);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetActiveUniformsiv)(GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetActiveUniformsiv(gc, program, uniformCount, uniformIndices, pname, params);
}

GLuint GL_APIENTRY __GLES_APINAME(GetUniformBlockIndex)(GLuint program, const GLchar* uniformBlockName)
{
    __GL_GET_CONTEXT_RET(GL_INVALID_INDEX);
    return gc->apiDispatchTable.GetUniformBlockIndex(gc, program, uniformBlockName);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetActiveUniformBlockiv)(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetActiveUniformBlockiv(gc, program, uniformBlockIndex, pname, params);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetActiveUniformBlockName)(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetActiveUniformBlockName(gc, program, uniformBlockIndex, bufSize, length, uniformBlockName);
}

GLvoid GL_APIENTRY __GLES_APINAME(UniformBlockBinding)(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.UniformBlockBinding(gc, program, uniformBlockIndex, uniformBlockBinding);
}

GLvoid GL_APIENTRY __GLES_APINAME(DrawArraysInstanced)(GLenum mode, GLint first, GLsizei count, GLsizei instanceCount)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DrawArraysInstanced(gc, mode, first, count, instanceCount);
}

GLvoid GL_APIENTRY __GLES_APINAME(DrawElementsInstanced)(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei instanceCount)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DrawElementsInstanced(gc, mode, count, type, indices, instanceCount);
}

GLsync GL_APIENTRY __GLES_APINAME(FenceSync)(GLenum condition, GLbitfield flags)
{
    __GL_GET_CONTEXT_RET(0);
    return gc->apiDispatchTable.FenceSync(gc, condition, flags);
}

GLboolean GL_APIENTRY __GLES_APINAME(IsSync)(GLsync sync)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->apiDispatchTable.IsSync(gc, sync);
}

GLvoid GL_APIENTRY __GLES_APINAME(DeleteSync)(GLsync sync)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DeleteSync(gc, sync);
}

GLenum GL_APIENTRY __GLES_APINAME(ClientWaitSync)(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    __GL_GET_CONTEXT_RET(GL_WAIT_FAILED);
    return gc->apiDispatchTable.ClientWaitSync(gc, sync, flags, timeout);
}

GLvoid GL_APIENTRY __GLES_APINAME(WaitSync)(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.WaitSync(gc, sync, flags, timeout);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetInteger64v)(GLenum pname, GLint64* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetInteger64v(gc, pname, params);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetSynciv)(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetSynciv(gc, sync, pname, bufSize, length, values);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetInteger64i_v)(GLenum target, GLuint index, GLint64* data)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetInteger64i_v(gc, target, index, data);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetBufferParameteri64v)(GLenum target, GLenum pname, GLint64* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetBufferParameteri64v(gc, target, pname, params);
}

GLvoid GL_APIENTRY __GLES_APINAME(GenSamplers)(GLsizei count, GLuint* samplers)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GenSamplers(gc, count, samplers);
}

GLvoid GL_APIENTRY __GLES_APINAME(DeleteSamplers)(GLsizei count, const GLuint* samplers)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DeleteSamplers(gc, count, samplers);
}

GLboolean GL_APIENTRY __GLES_APINAME(IsSampler)(GLuint sampler)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->apiDispatchTable.IsSampler(gc, sampler);
}

GLvoid GL_APIENTRY __GLES_APINAME(BindSampler)(GLuint unit, GLuint sampler)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BindSampler(gc, unit, sampler);
}

GLvoid GL_APIENTRY __GLES_APINAME(SamplerParameteri)(GLuint sampler, GLenum pname, GLint param)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.SamplerParameteri(gc, sampler, pname, param);
}

GLvoid GL_APIENTRY __GLES_APINAME(SamplerParameteriv)(GLuint sampler, GLenum pname, const GLint* param)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.SamplerParameteriv(gc, sampler, pname, param);
}

GLvoid GL_APIENTRY __GLES_APINAME(SamplerParameterf)(GLuint sampler, GLenum pname, GLfloat param)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.SamplerParameterf(gc, sampler, pname, param);
}

GLvoid GL_APIENTRY __GLES_APINAME(SamplerParameterfv)(GLuint sampler, GLenum pname, const GLfloat* param)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.SamplerParameterfv(gc, sampler, pname, param);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetSamplerParameteriv)(GLuint sampler, GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetSamplerParameteriv(gc, sampler, pname, params);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetSamplerParameterfv)(GLuint sampler, GLenum pname, GLfloat* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetSamplerParameterfv(gc, sampler, pname, params);
}

GLvoid GL_APIENTRY __GLES_APINAME(VertexAttribDivisor)(GLuint index, GLuint divisor)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.VertexAttribDivisor(gc, index, divisor);
}

GLvoid GL_APIENTRY __GLES_APINAME(BindTransformFeedback)(GLenum target, GLuint id)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BindTransformFeedback(gc, target, id);
}

GLvoid GL_APIENTRY __GLES_APINAME(DeleteTransformFeedbacks)(GLsizei n, const GLuint* ids)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DeleteTransformFeedbacks(gc, n, ids);
}

GLvoid GL_APIENTRY __GLES_APINAME(GenTransformFeedbacks)(GLsizei n, GLuint* ids)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GenTransformFeedbacks(gc, n, ids);
}

GLboolean GL_APIENTRY __GLES_APINAME(IsTransformFeedback)(GLuint id)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->apiDispatchTable.IsTransformFeedback(gc, id);
}

GLvoid GL_APIENTRY __GLES_APINAME(PauseTransformFeedback)(GLvoid)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.PauseTransformFeedback(gc);
}

GLvoid GL_APIENTRY __GLES_APINAME(ResumeTransformFeedback)(GLvoid)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ResumeTransformFeedback(gc);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetProgramBinary)(GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, GLvoid* binary)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetProgramBinary(gc, program, bufSize, length, binaryFormat, binary);
}

GLvoid GL_APIENTRY __GLES_APINAME(ProgramBinary)(GLuint program, GLenum binaryFormat, const GLvoid* binary, GLsizei length)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramBinary(gc, program, binaryFormat, binary, length);
}

GLvoid GL_APIENTRY __GLES_APINAME(ProgramParameteri)(GLuint program, GLenum pname, GLint value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramParameteri(gc, program, pname, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(InvalidateFramebuffer)(GLenum target, GLsizei numAttachments, const GLenum* attachments)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.InvalidateFramebuffer(gc, target, numAttachments, attachments);
}

GLvoid GL_APIENTRY __GLES_APINAME(InvalidateSubFramebuffer)(GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.InvalidateSubFramebuffer(gc, target, numAttachments, attachments, x, y, width, height);
}

GLvoid GL_APIENTRY __GLES_APINAME(TexStorage2D)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexStorage2D(gc, target, levels, internalformat, width, height);
}

GLvoid GL_APIENTRY __GLES_APINAME(TexStorage3D)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexStorage3D(gc, target, levels, internalformat, width, height, depth);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetInternalformativ)(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetInternalformativ(gc, target, internalformat, pname, bufSize, params);
}

/*
** OpenGL ES 3.1
*/
GLvoid GL_APIENTRY __GLES_APINAME(DispatchCompute)(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DispatchCompute(gc, num_groups_x, num_groups_y, num_groups_z);
}

GLvoid GL_APIENTRY __GLES_APINAME(DispatchComputeIndirect)(GLintptr indirect)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DispatchComputeIndirect(gc, indirect);
}

GLvoid GL_APIENTRY __GLES_APINAME(DrawArraysIndirect)(GLenum mode, const void *indirect)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DrawArraysIndirect(gc, mode, indirect);
}

GLvoid GL_APIENTRY __GLES_APINAME(DrawElementsIndirect)(GLenum mode, GLenum type, const void *indirect)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DrawElementsIndirect(gc, mode, type, indirect);
}

GLvoid GL_APIENTRY __GLES_APINAME(FramebufferParameteri)(GLenum target, GLenum pname, GLint param)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.FramebufferParameteri(gc, target, pname, param);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetFramebufferParameteriv)(GLenum target, GLenum pname, GLint *params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetFramebufferParameteriv(gc, target, pname, params);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetProgramInterfaceiv)(GLuint program, GLenum programInterface, GLenum pname, GLint *params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetProgramInterfaceiv(gc, program, programInterface, pname, params);
}

GLuint GL_APIENTRY __GLES_APINAME(GetProgramResourceIndex)(GLuint program, GLenum programInterface, const GLchar *name)
{
    __GL_GET_CONTEXT_RET(0xffffffff);
    return gc->apiDispatchTable.GetProgramResourceIndex(gc, program, programInterface, name);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetProgramResourceName)(GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetProgramResourceName(gc, program, programInterface, index, bufSize, length, name);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetProgramResourceiv)(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetProgramResourceiv(gc, program, programInterface, index, propCount, props, bufSize, length, params);
}

GLint GL_APIENTRY __GLES_APINAME(GetProgramResourceLocation)(GLuint program, GLenum programInterface, const GLchar *name)
{
    __GL_GET_CONTEXT_RET(-1);
    return gc->apiDispatchTable.GetProgramResourceLocation(gc, program, programInterface, name);
}

GLvoid GL_APIENTRY __GLES_APINAME(UseProgramStages)(GLuint pipeline, GLbitfield stages, GLuint program)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.UseProgramStages(gc, pipeline, stages, program);
}

GLvoid GL_APIENTRY __GLES_APINAME(ActiveShaderProgram)(GLuint pipeline, GLuint program)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ActiveShaderProgram(gc, pipeline, program);
}

GLuint GL_APIENTRY __GLES_APINAME(CreateShaderProgramv)(GLenum type, GLsizei count, const GLchar *const*strings)
{
    __GL_GET_CONTEXT_RET(0);
    return gc->apiDispatchTable.CreateShaderProgramv(gc, type, count, strings);
}

GLvoid GL_APIENTRY __GLES_APINAME(BindProgramPipeline)(GLuint pipeline)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BindProgramPipeline(gc, pipeline);
}

GLvoid GL_APIENTRY __GLES_APINAME(DeleteProgramPipelines)(GLsizei n, const GLuint *pipelines)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DeleteProgramPipelines(gc, n , pipelines);
}

GLvoid GL_APIENTRY __GLES_APINAME(GenProgramPipelines)(GLsizei n, GLuint *pipelines)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GenProgramPipelines(gc, n , pipelines);
}

GLboolean GL_APIENTRY __GLES_APINAME(IsProgramPipeline)(GLuint pipeline)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->apiDispatchTable.IsProgramPipeline(gc, pipeline);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetProgramPipelineiv)(GLuint pipeline, GLenum pname, GLint *params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetProgramPipelineiv(gc, pipeline, pname, params);
}

GLvoid GL_APIENTRY __GLES_APINAME(ProgramUniform1i)(GLuint program, GLint location, GLint v0)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform1i(gc, program, location, v0);
}

GLvoid GL_APIENTRY __GLES_APINAME(ProgramUniform2i)(GLuint program, GLint location, GLint v0, GLint v1)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform2i(gc, program, location, v0, v1);
}

GLvoid GL_APIENTRY __GLES_APINAME(ProgramUniform3i)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform3i(gc, program, location, v0, v1, v2);
}

GLvoid GL_APIENTRY __GLES_APINAME(ProgramUniform4i)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform4i(gc, program, location, v0, v1, v2, v3);
}

GLvoid GL_APIENTRY __GLES_APINAME(ProgramUniform1ui)(GLuint program, GLint location, GLuint v0)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform1ui(gc, program, location, v0);
}

GLvoid GL_APIENTRY __GLES_APINAME(ProgramUniform2ui)(GLuint program, GLint location, GLuint v0, GLuint v1)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform2ui(gc, program, location, v0, v1);
}

GLvoid GL_APIENTRY __GLES_APINAME(ProgramUniform3ui)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform3ui(gc, program, location, v0, v1, v2);
}

GLvoid GL_APIENTRY __GLES_APINAME(ProgramUniform4ui)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform4ui(gc, program, location, v0, v1, v2, v3);
}

GLvoid GL_APIENTRY __GLES_APINAME(ProgramUniform1f)(GLuint program, GLint location, GLfloat v0)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform1f(gc, program, location, v0);
}

GLvoid GL_APIENTRY __GLES_APINAME(ProgramUniform2f)(GLuint program, GLint location, GLfloat v0, GLfloat v1)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform2f(gc, program, location, v0, v1);
}

GLvoid GL_APIENTRY __GLES_APINAME(ProgramUniform3f)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform3f(gc, program, location, v0, v1, v2);
}

GLvoid GL_APIENTRY __GLES_APINAME(ProgramUniform4f)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform4f(gc, program, location, v0, v1, v2, v3);
}

GLvoid GL_APIENTRY __GLES_APINAME(ProgramUniform1iv)(GLuint program, GLint location, GLsizei count, const GLint *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform1iv(gc, program, location, count, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(ProgramUniform2iv)(GLuint program, GLint location, GLsizei count, const GLint *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform2iv(gc, program, location, count, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(ProgramUniform3iv)(GLuint program, GLint location, GLsizei count, const GLint *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform3iv(gc, program, location, count, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(ProgramUniform4iv)(GLuint program, GLint location, GLsizei count, const GLint *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform4iv(gc, program, location, count, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(ProgramUniform1uiv)(GLuint program, GLint location, GLsizei count, const GLuint *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform1uiv(gc, program, location, count, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(ProgramUniform2uiv)(GLuint program, GLint location, GLsizei count, const GLuint *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform2uiv(gc, program, location, count, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(ProgramUniform3uiv)(GLuint program, GLint location, GLsizei count, const GLuint *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform3uiv(gc, program, location, count, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(ProgramUniform4uiv)(GLuint program, GLint location, GLsizei count, const GLuint *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform4uiv(gc, program, location, count, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(ProgramUniform1fv)(GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform1fv(gc, program, location, count, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(ProgramUniform2fv)(GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform2fv(gc, program, location, count, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(ProgramUniform3fv)(GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform3fv(gc, program, location, count, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(ProgramUniform4fv)(GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform4fv(gc, program, location, count, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(ProgramUniformMatrix2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniformMatrix2fv(gc, program, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(ProgramUniformMatrix3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniformMatrix3fv(gc, program, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(ProgramUniformMatrix4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniformMatrix4fv(gc, program, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(ProgramUniformMatrix2x3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniformMatrix2x3fv(gc, program, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(ProgramUniformMatrix3x2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniformMatrix3x2fv(gc, program, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(ProgramUniformMatrix2x4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniformMatrix2x4fv(gc, program, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(ProgramUniformMatrix4x2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniformMatrix4x2fv(gc, program, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(ProgramUniformMatrix3x4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniformMatrix3x4fv(gc, program, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(ProgramUniformMatrix4x3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniformMatrix4x3fv(gc, program, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(ValidateProgramPipeline)(GLuint pipeline)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ValidateProgramPipeline(gc, pipeline);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetProgramPipelineInfoLog)(GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetProgramPipelineInfoLog(gc, pipeline, bufSize, length, infoLog);
}

GLvoid GL_APIENTRY __GLES_APINAME(BindImageTexture)(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BindImageTexture(gc, unit, texture, level, layered, layer, access, format);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetBooleani_v)(GLenum target, GLuint index, GLboolean *data)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetBooleani_v(gc, target, index, data);
}

GLvoid GL_APIENTRY __GLES_APINAME(MemoryBarrier)(GLbitfield barriers)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.MemoryBarrier(gc, barriers);
}

GLvoid GL_APIENTRY __GLES_APINAME(MemoryBarrierByRegion)(GLbitfield barriers)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.MemoryBarrierByRegion(gc, barriers);
}

GLvoid GL_APIENTRY __GLES_APINAME(TexStorage2DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexStorage2DMultisample(gc, target, samples, internalformat, width, height, fixedsamplelocations);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetMultisamplefv)(GLenum pname, GLuint index, GLfloat *val)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetMultisamplefv(gc, pname, index, val);
}

GLvoid GL_APIENTRY __GLES_APINAME(SampleMaski)(GLuint maskNumber, GLbitfield mask)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.SampleMaski(gc, maskNumber, mask);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetTexLevelParameteriv)(GLenum target, GLint level, GLenum pname, GLint *params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetTexLevelParameteriv(gc, target, level, pname, params);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetTexLevelParameterfv)(GLenum target, GLint level, GLenum pname, GLfloat *params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetTexLevelParameterfv(gc, target, level, pname, params);
}

GLvoid GL_APIENTRY __GLES_APINAME(BindVertexBuffer)(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BindVertexBuffer(gc, bindingindex, buffer, offset, stride);
}

GLvoid GL_APIENTRY __GLES_APINAME(VertexAttribFormat)(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.VertexAttribFormat(gc, attribindex, size, type, normalized, relativeoffset);
}

GLvoid GL_APIENTRY __GLES_APINAME(VertexAttribIFormat)(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.VertexAttribIFormat(gc, attribindex, size, type, relativeoffset);
}

GLvoid GL_APIENTRY __GLES_APINAME(VertexAttribBinding)(GLuint attribindex, GLuint bindingindex)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.VertexAttribBinding(gc, attribindex, bindingindex);
}

GLvoid GL_APIENTRY __GLES_APINAME(VertexBindingDivisor)(GLuint bindingindex, GLuint divisor)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.VertexBindingDivisor(gc, bindingindex, divisor);
}

/* OpenGL ES 3.2 */

GLvoid GL_APIENTRY __GLES_APINAME(TexStorage3DMultisample)(GLenum target, GLsizei samples, GLenum sizedinternalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexStorage3DMultisample(gc, target, samples, sizedinternalformat, width, height, depth, fixedsamplelocations);
}

GLvoid GL_APIENTRY __GLES_APINAME(BlendBarrier)(void)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BlendBarrier(gc);
}

GLvoid GL_APIENTRY __GLES_APINAME(DebugMessageControl)(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DebugMessageControl(gc, source, type, severity, count, ids, enabled);
}

GLvoid GL_APIENTRY __GLES_APINAME(DebugMessageInsert)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* buf)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DebugMessageInsert(gc, source, type, id, severity, length, buf);
}

GLvoid GL_APIENTRY __GLES_APINAME(DebugMessageCallback)(GLDEBUGPROCKHR callback, const GLvoid* userParam)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DebugMessageCallback(gc, callback, userParam);
}

GLuint GL_APIENTRY __GLES_APINAME(GetDebugMessageLog)(GLuint count, GLsizei bufSize, GLenum* sources, GLenum* types, GLuint* ids, GLenum* severities, GLsizei* lengths, GLchar* messageLog)
{
    __GL_GET_CONTEXT_RET(0);
    return gc->apiDispatchTable.GetDebugMessageLog(gc, count, bufSize, sources, types, ids, severities, lengths, messageLog);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetPointerv)(GLenum pname, GLvoid** params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetPointerv(gc, pname, params);
}

GLvoid GL_APIENTRY __GLES_APINAME(PushDebugGroup)(GLenum source, GLuint id, GLsizei length, const GLchar * message)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.PushDebugGroup(gc, source, id, length, message);
}

GLvoid GL_APIENTRY __GLES_APINAME(PopDebugGroup)(void)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.PopDebugGroup(gc);
}

GLvoid GL_APIENTRY __GLES_APINAME(ObjectLabel)(GLenum identifier, GLuint name, GLsizei length, const GLchar *label)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ObjectLabel(gc, identifier, name, length, label);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetObjectLabel)(GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetObjectLabel(gc, identifier, name, bufSize, length, label);
}

GLvoid GL_APIENTRY __GLES_APINAME(ObjectPtrLabel)(const GLvoid* ptr, GLsizei length, const GLchar *label)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ObjectPtrLabel(gc, ptr, length, label);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetObjectPtrLabel)(const GLvoid* ptr, GLsizei bufSize, GLsizei *length, GLchar *label)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetObjectPtrLabel(gc, ptr, bufSize, length, label);
}

GLenum GL_APIENTRY __GLES_APINAME(GetGraphicsResetStatus)()
{
    __GL_GET_CONTEXT_RET(GL_NO_ERROR);
    return gc->apiDispatchTable.GetGraphicsResetStatus(gc);
}

GLvoid GL_APIENTRY __GLES_APINAME(ReadnPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, GLvoid *data)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ReadnPixels(gc, x, y, width, height, format, type, bufSize, data);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetnUniformfv)(GLuint program, GLint location, GLsizei bufSize, GLfloat *params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetnUniformfv(gc, program, location, bufSize, params);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetnUniformiv)(GLuint program, GLint location, GLsizei bufSize, GLint *params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetnUniformiv(gc, program, location, bufSize, params);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetnUniformuiv)(GLuint program, GLint location, GLsizei bufSize, GLuint *params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetnUniformuiv(gc, program, location, bufSize, params);
}

GLvoid GL_APIENTRY __GLES_APINAME(BlendEquationi)(GLuint buf, GLenum mode)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BlendEquationi(gc, buf, mode);
}

GLvoid GL_APIENTRY __GLES_APINAME(BlendEquationSeparatei)(GLuint buf, GLenum modeRGB, GLenum modeAlpha)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BlendEquationSeparatei(gc, buf, modeRGB, modeAlpha);
}

GLvoid GL_APIENTRY __GLES_APINAME(BlendFunci)(GLuint buf, GLenum sfactor, GLenum dfactor)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BlendFunci(gc, buf, sfactor, dfactor);
}

GLvoid GL_APIENTRY __GLES_APINAME(BlendFuncSeparatei)(GLuint buf, GLenum sfactorRGB,GLenum dfactorRGB,GLenum sfactorAlpha,GLenum dfactorAlpha)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BlendFuncSeparatei(gc, buf, sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
}

GLvoid GL_APIENTRY __GLES_APINAME(ColorMaski)(GLuint buf, GLboolean r, GLboolean g, GLboolean b, GLboolean a)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ColorMaski(gc, buf, r, g, b, a);
}

GLvoid GL_APIENTRY __GLES_APINAME(Enablei)(GLenum target, GLuint index)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Enablei(gc, target, index);
}

GLvoid GL_APIENTRY  __GLES_APINAME(Disablei)( GLenum target, GLuint index)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Disablei(gc, target, index);
}

GLboolean GL_APIENTRY  __GLES_APINAME(IsEnabledi)( GLenum target, GLuint index)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->apiDispatchTable.IsEnabledi(gc, target, index);
}

GLvoid GL_APIENTRY __GLES_APINAME(TexParameterIiv)(GLenum target, GLenum pname, const GLint *params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexParameterIiv(gc, target, pname, params);
}

GLvoid GL_APIENTRY __GLES_APINAME(TexParameterIuiv)(GLenum target, GLenum pname, const GLuint *params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexParameterIuiv(gc, target, pname, params);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetTexParameterIiv)(GLenum target, GLenum pname, GLint *params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetTexParameterIiv(gc, target, pname, params);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetTexParameterIuiv)(GLenum target, GLenum pname, GLuint *params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetTexParameterIuiv(gc, target, pname, params);
}

GLvoid GL_APIENTRY __GLES_APINAME(SamplerParameterIiv)(GLuint sampler, GLenum pname, const GLint *param)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.SamplerParameterIiv(gc, sampler, pname, param);
}

GLvoid GL_APIENTRY __GLES_APINAME(SamplerParameterIuiv)(GLuint sampler, GLenum pname, const GLuint *param)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.SamplerParameterIuiv(gc, sampler, pname, param);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetSamplerParameterIiv)(GLuint sampler, GLenum pname, GLint *params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetSamplerParameterIiv(gc, sampler, pname, params);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetSamplerParameterIuiv)(GLuint sampler, GLenum pname, GLuint *params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetSamplerParameterIuiv(gc, sampler, pname, params);
}

GLvoid GL_APIENTRY __GLES_APINAME(TexBuffer)(GLenum target, GLenum internalformat, GLuint buffer)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexBuffer(gc, target, internalformat, buffer);
}

GLvoid GL_APIENTRY __GLES_APINAME(TexBufferRange)(GLenum target, GLenum internalformat, GLuint buffer,
                                                  GLintptr offset, GLsizeiptr size)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexBufferRange(gc, target, internalformat, buffer, offset, size);
}

GLvoid GL_APIENTRY __GLES_APINAME(PatchParameteri)(GLenum pname, GLint value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.PatchParameteri(gc, pname, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(FramebufferTexture)(GLenum target, GLenum attachment, GLuint texture, GLint level)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.FramebufferTexture(gc, target, attachment, texture, level);
}

GLvoid GL_APIENTRY __GLES_APINAME(MinSampleShading)(GLfloat value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.MinSampleShading(gc, value);
}

GLvoid GL_APIENTRY __GLES_APINAME(CopyImageSubData)(GLuint srcName, GLenum srcTarget, GLint srcLevel,
                                                    GLint srcX, GLint srcY, GLint srcZ,
                                                    GLuint dstName, GLenum dstTarget, GLint dstLevel,
                                                    GLint dstX, GLint dstY, GLint dstZ,
                                                    GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.CopyImageSubData(gc, srcName, srcTarget, srcLevel, srcX, srcY, srcZ, dstName, dstTarget, dstLevel,
                                          dstX, dstY, dstZ, srcWidth, srcHeight, srcDepth);
}


GLvoid GL_APIENTRY __GLES_APINAME(DrawElementsBaseVertex)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DrawElementsBaseVertex(gc, mode, count, type, indices, basevertex);
}

GLvoid GL_APIENTRY __GLES_APINAME(DrawRangeElementsBaseVertex)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DrawRangeElementsBaseVertex(gc, mode, start, end, count, type, indices, basevertex);
}

GLvoid GL_APIENTRY __GLES_APINAME(DrawElementsInstancedBaseVertex)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DrawElementsInstancedBaseVertex(gc, mode, count, type, indices, instancecount, basevertex);
}

GLvoid GL_APIENTRY __GLES_APINAME(PrimitiveBoundingBox)(GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW,
                                                        GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.PrimitiveBoundingBox(gc, minX, minY, minZ, minW, maxX, maxY, maxZ, maxW);
}

/*
** OpenGL ES Extensions
*/

#if GL_OES_EGL_image
GLvoid GL_APIENTRY __GLES_APINAME(EGLImageTargetTexture2DOES)(GLenum target, GLeglImageOES image)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.EGLImageTargetTexture2DOES(gc, target, image);
}

GLvoid GL_APIENTRY __GLES_APINAME(EGLImageTargetRenderbufferStorageOES)(GLenum target, GLeglImageOES image)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.EGLImageTargetRenderbufferStorageOES(gc, target, image);
}
#endif

#if GL_EXT_multi_draw_arrays
GLvoid GL_APIENTRY __GLES_APINAME(MultiDrawArraysEXT)(GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.MultiDrawArraysEXT(gc, mode, first, count, primcount);
}

GLvoid GL_APIENTRY __GLES_APINAME(MultiDrawElementsEXT)(GLenum mode, const GLsizei *count, GLenum type, const GLvoid*const*indices, GLsizei primcount)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.MultiDrawElementsEXT(gc, mode, count, type, indices, primcount);
}

#if GL_EXT_draw_elements_base_vertex
GLvoid GL_APIENTRY __GLES_APINAME(MultiDrawElementsBaseVertexEXT)(GLenum mode, const GLsizei * count, GLenum type, const void *const*indices, GLsizei drawcount, const GLint *basevertex)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.MultiDrawElementsBaseVertexEXT(gc, mode, count, type, indices, drawcount, basevertex);
}
#endif
#endif

#if GL_OES_mapbuffer
GLvoid GL_APIENTRY __GLES_APINAME(GetBufferPointervOES)(GLenum target, GLenum pname, GLvoid** params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetBufferPointervOES(gc, target, pname, params);
}

GLvoid* GL_APIENTRY __GLES_APINAME(MapBufferOES)(GLenum target, GLenum access)
{
    __GL_GET_CONTEXT_RET(gcvNULL);
    return gc->apiDispatchTable.MapBufferOES(gc, target, access);
}

GLboolean GL_APIENTRY __GLES_APINAME(UnmapBufferOES)(GLenum target)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->apiDispatchTable.UnmapBufferOES(gc, target);
}
#endif

#if GL_EXT_discard_framebuffer
GLvoid GL_APIENTRY __GLES_APINAME(DiscardFramebufferEXT)(GLenum target, GLsizei numAttachments, const GLenum *attachments)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DiscardFramebufferEXT(gc, target, numAttachments, attachments);
}
#endif

#if GL_EXT_multisampled_render_to_texture
GLvoid GL_APIENTRY __GLES_APINAME(FramebufferTexture2DMultisampleEXT)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.FramebufferTexture2DMultisampleEXT(gc, target, attachment, textarget, texture, level, samples);
}

GLvoid GL_APIENTRY __GLES_APINAME(RenderbufferStorageMultisampleEXT)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.RenderbufferStorageMultisampleEXT(gc, target, samples, internalformat, width, height);
}
#endif



#if GL_VIV_direct_texture
GLvoid GL_APIENTRY __GLES_APINAME(TexDirectVIV)(GLenum target, GLsizei width, GLsizei height, GLenum format, GLvoid ** pixels)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexDirectVIV(gc, target, width, height, format, pixels);
}

GLvoid GL_APIENTRY __GLES_APINAME(TexDirectInvalidateVIV)(GLenum target)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexDirectInvalidateVIV(gc, target);
}

GLvoid GL_APIENTRY __GLES_APINAME(TexDirectVIVMap)(GLenum target, GLsizei width, GLsizei height, GLenum format, GLvoid ** logical, const GLuint * physical)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexDirectVIVMap(gc, target, width, height, format, logical, physical);
}

GLvoid GL_APIENTRY __GLES_APINAME(TexDirectTiledMapVIV)(GLenum target, GLsizei width, GLsizei height, GLenum format, GLvoid ** logical, const GLuint * physical)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexDirectTiledMapVIV(gc, target, width, height, format, logical, physical);
}
#endif

#if GL_OES_get_program_binary
GLvoid GL_APIENTRY __GLES_APINAME(GetProgramBinaryOES)(GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, GLvoid* binary)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetProgramBinary(gc, program, bufSize, length, binaryFormat, binary);
}

GLvoid GL_APIENTRY __GLES_APINAME(ProgramBinaryOES)(GLuint program, GLenum binaryFormat, const GLvoid* binary, GLsizei length)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramBinary(gc, program, binaryFormat, binary, length);
}
#endif

#if GL_OES_vertex_array_object
GLvoid GL_APIENTRY __GLES_APINAME(BindVertexArrayOES)(GLuint array)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BindVertexArray(gc, array);
}

GLvoid GL_APIENTRY __GLES_APINAME(DeleteVertexArraysOES)(GLsizei n, const GLuint *arrays)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DeleteVertexArrays(gc, n , arrays);
}

GLvoid GL_APIENTRY __GLES_APINAME(GenVertexArraysOES)(GLsizei n, GLuint *arrays)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GenVertexArrays(gc, n , arrays);
}

GLboolean GL_APIENTRY __GLES_APINAME(IsVertexArrayOES)(GLuint array)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->apiDispatchTable.IsVertexArray(gc, array);
}
#endif

#if GL_EXT_multi_draw_indirect
GLvoid GL_APIENTRY __GLES_APINAME(MultiDrawArraysIndirectEXT)(GLenum mode, const void *indirect, GLsizei drawcount, GLsizei stride)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.MultiDrawArraysIndirectEXT(gc, mode, indirect, drawcount, stride);
}

GLvoid GL_APIENTRY __GLES_APINAME(MultiDrawElementsIndirectEXT)(GLenum mode, GLenum type, const void *indirect, GLsizei drawcount, GLsizei stride)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.MultiDrawElementsIndirectEXT(gc, mode, type, indirect, drawcount, stride);
}
#endif

#if GL_KHR_blend_equation_advanced
GLvoid GL_APIENTRY __GLES_APINAME(BlendBarrierKHR)(void)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BlendBarrier(gc);
}
#endif

#if GL_EXT_robustness
GLenum GL_APIENTRY __GLES_APINAME(GetGraphicsResetStatusEXT)()
{
    __GL_GET_CONTEXT_RET(GL_NO_ERROR);
    return gc->apiDispatchTable.GetGraphicsResetStatus(gc);
}

GLvoid GL_APIENTRY __GLES_APINAME(ReadnPixelsEXT)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, GLvoid *data)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ReadnPixels(gc, x, y, width, height, format, type, bufSize, data);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetnUniformfvEXT)(GLuint program, GLint location, GLsizei bufSize, GLfloat *params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetnUniformfv(gc, program, location, bufSize, params);
}

GLvoid GL_APIENTRY __GLES_APINAME(GetnUniformivEXT)(GLuint program, GLint location, GLsizei bufSize, GLint *params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetnUniformiv(gc, program, location, bufSize, params);
}
#endif

GLvoid GL_APIENTRY __GLES_APINAME(GetTexImage)( GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetTexImage(gc, target, level, format, type, pixels);
}

