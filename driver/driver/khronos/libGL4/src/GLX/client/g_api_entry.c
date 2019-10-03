/******************************************************************************\
|* Copyright (c) 2011-2015 by Vivante Corporation.  All Rights Reserved.      *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of Vivante Corporation.  This is proprietary information owned by Vivante  *|
|* Corporation.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of Vivante Corporation.                                 *|
|*                                                                            *|
\******************************************************************************/

#include "gc_es_context.h"

#define __GL_APINAME(apiname) gl##apiname

#if gcdPATTERN_FAST_PATH
/* For GL, gc will never be NULL, If no context, gc will be glNopContext for Nop function return */
#define __GL_GET_CONTEXT  __GLcontext *gc = (__GLcontext *)_glapi_get_context(); gc->pattern.apiCount++;
#define __GL_GET_CONTEXT_RET(_value_)  __GLcontext *gc = (__GLcontext *)_glapi_get_context(); gc->pattern.apiCount++;

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
/* For GL, gc will never be NULL, If no context, gc will be glNopContext for Nop function return */
#define __GL_GET_CONTEXT  __GLcontext *gc = (__GLcontext *)_glapi_get_context();
#define __GL_GET_CONTEXT_RET(_value_)  __GLcontext *gc = (__GLcontext *)_glapi_get_context();
#define __GL_PATTERN_MATCH_NAME(name)
#define __GL_PATTERN_MATCH_NAME_WITH_ARG(name,arg)
#define __GL_PATTERN_MATCH_NAME_WITH_ARG2(name, arg, arg1)
#endif

/*
** OpenGL ES 2.0
*/

GLvoid GL_APIENTRY __GL_APINAME(ActiveTexture)(GLenum texture)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ActiveTexture(gc, texture);
}

GLvoid GL_APIENTRY __GL_APINAME(AttachShader)(GLuint program, GLuint shader)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->AttachShader(gc, program, shader);
}

GLvoid GL_APIENTRY __GL_APINAME(BindAttribLocation)(GLuint program, GLuint index, const GLchar* name)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->BindAttribLocation(gc, program, index, name);
}

GLvoid GL_APIENTRY __GL_APINAME(BindBuffer)(GLenum target, GLuint buffer)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->BindBuffer(gc, target, buffer);
    __GL_PATTERN_MATCH_NAME(__glApiEnum(BindBuffer));
}

GLvoid GL_APIENTRY __GL_APINAME(BindFramebuffer)(GLenum target, GLuint framebuffer)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->BindFramebuffer(gc, target, framebuffer);
}

GLvoid GL_APIENTRY __GL_APINAME(BindRenderbuffer)(GLenum target, GLuint renderbuffer)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->BindRenderbuffer(gc, target, renderbuffer);
}

GLvoid GL_APIENTRY __GL_APINAME(BindTexture)(GLenum target, GLuint texture)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->BindTexture(gc, target, texture);
}

GLvoid GL_APIENTRY __GL_APINAME(BlendColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->BlendColor(gc, red, green, blue, alpha);
    __GL_PATTERN_MATCH_NAME(__glApiEnum(BlendColor));
}

GLvoid GL_APIENTRY __GL_APINAME(BlendEquation)(GLenum mode)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->BlendEquation(gc, mode);
}

GLvoid GL_APIENTRY __GL_APINAME(BlendEquationSeparate)(GLenum modeRGB, GLenum modeAlpha)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->BlendEquationSeparate(gc, modeRGB, modeAlpha);
}

GLvoid GL_APIENTRY __GL_APINAME(BlendFunc)(GLenum sfactor, GLenum dfactor)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->BlendFunc(gc, sfactor, dfactor);
    __GL_PATTERN_MATCH_NAME(__glApiEnum(BlendFunc));
}

GLvoid GL_APIENTRY __GL_APINAME(BlendFuncSeparate)(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->BlendFuncSeparate(gc, srcRGB, dstRGB, srcAlpha, dstAlpha);
}

GLvoid GL_APIENTRY __GL_APINAME(BufferData)(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->BufferData(gc, target, size, data, usage);
}

GLvoid GL_APIENTRY __GL_APINAME(BufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->BufferSubData(gc, target, offset, size, data);
}

GLenum GL_APIENTRY __GL_APINAME(CheckFramebufferStatus)(GLenum target)
{
    __GL_GET_CONTEXT_RET(0);
    return gc->pEntryDispatch->CheckFramebufferStatus(gc, target);
}

GLvoid GL_APIENTRY __GL_APINAME(Clear)(GLbitfield mask)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Clear(gc, mask);
}

GLvoid GL_APIENTRY __GL_APINAME(ClearColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ClearColor(gc, red, green, blue, alpha);
}

GLvoid GL_APIENTRY __GL_APINAME(ClearDepthf)(GLfloat depth)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ClearDepthf(gc, depth);
}

GLvoid GL_APIENTRY __GL_APINAME(ClearStencil)(GLint s)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ClearStencil(gc, s);
}

GLvoid GL_APIENTRY __GL_APINAME(ColorMask)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ColorMask(gc, red, green, blue, alpha);
}

GLvoid GL_APIENTRY __GL_APINAME(CompileShader)(GLuint shader)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->CompileShader(gc, shader);
}

GLvoid GL_APIENTRY __GL_APINAME(CompressedTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->CompressedTexImage2D(gc, target, level, internalformat, width, height, border, imageSize, data);
}

GLvoid GL_APIENTRY __GL_APINAME(CompressedTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid* data)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->CompressedTexSubImage2D(gc, target, level, xoffset, yoffset, width, height, format, imageSize, data);
}

GLvoid GL_APIENTRY __GL_APINAME(CopyTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->CopyTexImage2D(gc, target, level, internalformat, x, y, width, height, border);
}

GLvoid GL_APIENTRY __GL_APINAME(CopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->CopyTexSubImage2D(gc, target, level, xoffset, yoffset, x, y, width, height);
}

GLuint GL_APIENTRY __GL_APINAME(CreateProgram)(GLvoid)
{
    __GL_GET_CONTEXT_RET(0);
    return gc->pEntryDispatch->CreateProgram(gc);
}

GLuint GL_APIENTRY __GL_APINAME(CreateShader)(GLenum type)
{
    __GL_GET_CONTEXT_RET(0);
    return gc->pEntryDispatch->CreateShader(gc, type);
}

GLvoid GL_APIENTRY __GL_APINAME(CullFace)(GLenum mode)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->CullFace(gc, mode);
}

GLvoid GL_APIENTRY __GL_APINAME(DeleteBuffers)(GLsizei n, const GLuint* buffers)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DeleteBuffers(gc, n, buffers);
}

GLvoid GL_APIENTRY __GL_APINAME(DeleteFramebuffers)(GLsizei n, const GLuint* framebuffers)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DeleteFramebuffers(gc, n, framebuffers);
}

GLvoid GL_APIENTRY __GL_APINAME(DeleteProgram)(GLuint program)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DeleteProgram(gc, program);
}

GLvoid GL_APIENTRY __GL_APINAME(DeleteRenderbuffers)(GLsizei n, const GLuint* renderbuffers)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DeleteRenderbuffers(gc, n, renderbuffers);
}

GLvoid GL_APIENTRY __GL_APINAME(DeleteShader)(GLuint shader)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DeleteShader(gc, shader);
}

GLvoid GL_APIENTRY __GL_APINAME(DeleteTextures)(GLsizei n, const GLuint* textures)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DeleteTextures(gc, n, textures);
}

GLvoid GL_APIENTRY __GL_APINAME(DepthFunc)(GLenum func)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DepthFunc(gc, func);
    __GL_PATTERN_MATCH_NAME(__glApiEnum(DepthFunc));
}

GLvoid GL_APIENTRY __GL_APINAME(DepthMask)(GLboolean flag)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DepthMask(gc, flag);
}

GLvoid GL_APIENTRY __GL_APINAME(DepthRangef)(GLfloat n, GLfloat f)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DepthRangef(gc, n, f);
}

GLvoid GL_APIENTRY __GL_APINAME(DetachShader)(GLuint program, GLuint shader)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DetachShader(gc, program, shader);
}

GLvoid GL_APIENTRY __GL_APINAME(Disable)(GLenum cap)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Disable(gc, cap);
    __GL_PATTERN_MATCH_NAME_WITH_ARG(__glApiEnum(Disable), cap);
}

GLvoid GL_APIENTRY __GL_APINAME(DisableVertexAttribArray)(GLuint index)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DisableVertexAttribArray(gc, index);
}

GLvoid GL_APIENTRY __GL_APINAME(DrawArrays)(GLenum mode, GLint first, GLsizei count)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DrawArrays(gc, mode, first, count);
}

GLvoid GL_APIENTRY __GL_APINAME(DrawElements)(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices)
{
    __GL_GET_CONTEXT;
    __GL_PATTERN_MATCH_NAME_WITH_ARG2(__glApiEnum(DrawElements),mode, type);
    gc->pEntryDispatch->DrawElements(gc, mode, count, type, indices);
}

GLvoid GL_APIENTRY __GL_APINAME(Enable)(GLenum cap)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Enable(gc, cap);
    __GL_PATTERN_MATCH_NAME_WITH_ARG(__glApiEnum(Enable),cap);
}

GLvoid GL_APIENTRY __GL_APINAME(EnableVertexAttribArray)(GLuint index)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->EnableVertexAttribArray(gc, index);
}

GLvoid GL_APIENTRY __GL_APINAME(Finish)(GLvoid)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Finish(gc);
}

GLvoid GL_APIENTRY __GL_APINAME(Flush)(GLvoid)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Flush(gc);
}

GLvoid GL_APIENTRY __GL_APINAME(FramebufferRenderbuffer)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->FramebufferRenderbuffer(gc, target, attachment, renderbuffertarget, renderbuffer);
}

GLvoid GL_APIENTRY __GL_APINAME(FramebufferTexture2D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->FramebufferTexture2D(gc, target, attachment, textarget, texture, level);
}

GLvoid GL_APIENTRY __GL_APINAME(FrontFace)(GLenum mode)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->FrontFace(gc, mode);
}

GLvoid GL_APIENTRY __GL_APINAME(GenBuffers)(GLsizei n, GLuint* buffers)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GenBuffers(gc, n, buffers);
}

GLvoid GL_APIENTRY __GL_APINAME(GenerateMipmap)(GLenum target)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GenerateMipmap(gc, target);
}

GLvoid GL_APIENTRY __GL_APINAME(GenFramebuffers)(GLsizei n, GLuint* framebuffers)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GenFramebuffers(gc, n, framebuffers);
}

GLvoid GL_APIENTRY __GL_APINAME(GenRenderbuffers)(GLsizei n, GLuint* renderbuffers)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GenRenderbuffers(gc, n, renderbuffers);
}

GLvoid GL_APIENTRY __GL_APINAME(GenTextures)(GLsizei n, GLuint* textures)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GenTextures(gc, n, textures);
}

GLvoid GL_APIENTRY __GL_APINAME(GetActiveAttrib)(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetActiveAttrib(gc, program, index, bufsize, length, size, type, name);
}

GLvoid GL_APIENTRY __GL_APINAME(GetActiveUniform)(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetActiveUniform(gc, program, index, bufsize, length, size, type, name);
}

GLvoid GL_APIENTRY __GL_APINAME(GetAttachedShaders)(GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetAttachedShaders(gc, program, maxcount, count, shaders);
}

GLint GL_APIENTRY __GL_APINAME(GetAttribLocation)(GLuint program, const GLchar* name)
{
    __GL_GET_CONTEXT_RET(GL_INVALID_OPERATION);
    return gc->pEntryDispatch->GetAttribLocation(gc, program, name);
}

GLvoid GL_APIENTRY __GL_APINAME(GetBooleanv)(GLenum pname, GLboolean* params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetBooleanv(gc, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GetBufferParameteriv)(GLenum target, GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetBufferParameteriv(gc, target, pname, params);
}

GLenum GL_APIENTRY __GL_APINAME(GetError)(GLvoid)
{
    __GL_GET_CONTEXT_RET(GL_NO_ERROR);
    return gc->pEntryDispatch->GetError(gc);
}

GLvoid GL_APIENTRY __GL_APINAME(GetFloatv)(GLenum pname, GLfloat* params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetFloatv(gc, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GetFramebufferAttachmentParameteriv)(GLenum target, GLenum attachment, GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetFramebufferAttachmentParameteriv(gc, target, attachment, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GetIntegerv)(GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetIntegerv(gc, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GetProgramiv)(GLuint program, GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetProgramiv(gc, program, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GetProgramInfoLog)(GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetProgramInfoLog(gc, program, bufsize, length, infolog);
}

GLvoid GL_APIENTRY __GL_APINAME(GetRenderbufferParameteriv)(GLenum target, GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetRenderbufferParameteriv(gc, target, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GetShaderiv)(GLuint shader, GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetShaderiv(gc, shader, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GetShaderInfoLog)(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetShaderInfoLog(gc, shader, bufsize, length, infolog);
}

GLvoid GL_APIENTRY __GL_APINAME(GetShaderPrecisionFormat)(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetShaderPrecisionFormat(gc, shadertype, precisiontype, range, precision);
}

GLvoid GL_APIENTRY __GL_APINAME(GetShaderSource)(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetShaderSource(gc, shader, bufsize, length, source);
}

const GLubyte* GL_APIENTRY __GL_APINAME(GetString)(GLenum name)
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

    return gc->pEntryDispatch->GetString(gc, name);
}

GLvoid GL_APIENTRY __GL_APINAME(GetTexParameterfv)(GLenum target, GLenum pname, GLfloat* params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetTexParameterfv(gc, target, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GetTexParameteriv)(GLenum target, GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetTexParameteriv(gc, target, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GetUniformfv)(GLuint program, GLint location, GLfloat* params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetUniformfv(gc, program, location, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GetUniformiv)(GLuint program, GLint location, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetUniformiv(gc, program, location, params);
}

GLint GL_APIENTRY __GL_APINAME(GetUniformLocation)(GLuint program, const GLchar* name)
{
    __GL_GET_CONTEXT_RET(GL_INVALID_OPERATION);
    return gc->pEntryDispatch->GetUniformLocation(gc, program, name);
}

GLvoid GL_APIENTRY __GL_APINAME(GetVertexAttribfv)(GLuint index, GLenum pname, GLfloat* params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetVertexAttribfv(gc, index, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GetVertexAttribiv)(GLuint index, GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetVertexAttribiv(gc, index, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GetVertexAttribPointerv)(GLuint index, GLenum pname, GLvoid** pointer)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetVertexAttribPointerv(gc, index, pname, pointer);
}

GLvoid GL_APIENTRY __GL_APINAME(Hint)(GLenum target, GLenum mode)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Hint(gc, target, mode);
}

GLboolean GL_APIENTRY __GL_APINAME(IsBuffer)(GLuint buffer)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->pEntryDispatch->IsBuffer(gc, buffer);
}

GLboolean GL_APIENTRY __GL_APINAME(IsEnabled)(GLenum cap)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->pEntryDispatch->IsEnabled(gc, cap);
}

GLboolean GL_APIENTRY __GL_APINAME(IsFramebuffer)(GLuint framebuffer)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->pEntryDispatch->IsFramebuffer(gc, framebuffer);
}

GLboolean GL_APIENTRY __GL_APINAME(IsProgram)(GLuint program)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->pEntryDispatch->IsProgram(gc, program);
}

GLboolean GL_APIENTRY __GL_APINAME(IsRenderbuffer)(GLuint renderbuffer)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->pEntryDispatch->IsRenderbuffer(gc, renderbuffer);
}

GLboolean GL_APIENTRY __GL_APINAME(IsShader)(GLuint shader)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->pEntryDispatch->IsShader(gc, shader);
}

GLboolean GL_APIENTRY __GL_APINAME(IsTexture)(GLuint texture)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->pEntryDispatch->IsTexture(gc, texture);
}

GLvoid GL_APIENTRY __GL_APINAME(LineWidth)(GLfloat width)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->LineWidth(gc, width);
}

GLvoid GL_APIENTRY __GL_APINAME(LinkProgram)(GLuint program)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->LinkProgram(gc, program);
}

GLvoid GL_APIENTRY __GL_APINAME(PixelStorei)(GLenum pname, GLint param)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->PixelStorei(gc, pname, param);
}

GLvoid GL_APIENTRY __GL_APINAME(PolygonOffset)(GLfloat factor, GLfloat units)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->PolygonOffset(gc, factor, units);
}

GLvoid GL_APIENTRY __GL_APINAME(ReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ReadPixels(gc, x, y, width, height, format, type, pixels);
}

GLvoid GL_APIENTRY __GL_APINAME(ReleaseShaderCompiler)(GLvoid)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ReleaseShaderCompiler(gc);
}

GLvoid GL_APIENTRY __GL_APINAME(RenderbufferStorage)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->RenderbufferStorage(gc, target, internalformat, width, height);
}

GLvoid GL_APIENTRY __GL_APINAME(SampleCoverage)(GLfloat value, GLboolean invert)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->SampleCoverage(gc, value, invert);
}

GLvoid GL_APIENTRY __GL_APINAME(Scissor)(GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Scissor(gc, x, y, width, height);
}

GLvoid GL_APIENTRY __GL_APINAME(ShaderBinary)(GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ShaderBinary(gc, n, shaders, binaryformat, binary, length);
}

GLvoid GL_APIENTRY __GL_APINAME(ShaderSource)(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ShaderSource(gc, shader, count, string, length);
}

GLvoid GL_APIENTRY __GL_APINAME(StencilFunc)(GLenum func, GLint ref, GLuint mask)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->StencilFunc(gc, func, ref, mask);
}

GLvoid GL_APIENTRY __GL_APINAME(StencilFuncSeparate)(GLenum face, GLenum func, GLint ref, GLuint mask)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->StencilFuncSeparate(gc, face, func, ref, mask);
}

GLvoid GL_APIENTRY __GL_APINAME(StencilMask)(GLuint mask)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->StencilMask(gc, mask);
}

GLvoid GL_APIENTRY __GL_APINAME(StencilMaskSeparate)(GLenum face, GLuint mask)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->StencilMaskSeparate(gc, face, mask);
}

GLvoid GL_APIENTRY __GL_APINAME(StencilOp)(GLenum fail, GLenum zfail, GLenum zpass)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->StencilOp(gc, fail, zfail, zpass);
}

GLvoid GL_APIENTRY __GL_APINAME(StencilOpSeparate)(GLenum face, GLenum fail, GLenum zfail, GLenum zpass)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->StencilOpSeparate(gc, face, fail, zfail, zpass);
}

GLvoid GL_APIENTRY __GL_APINAME(TexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexImage2D(gc, target, level, internalformat, width, height, border, format, type, pixels);
}

GLvoid GL_APIENTRY __GL_APINAME(TexParameterf)(GLenum target, GLenum pname, GLfloat param)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexParameterf(gc, target, pname, param);
}

GLvoid GL_APIENTRY __GL_APINAME(TexParameterfv)(GLenum target, GLenum pname, const GLfloat* params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexParameterfv(gc, target, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(TexParameteri)(GLenum target, GLenum pname, GLint param)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexParameteri(gc, target, pname, param);
}

GLvoid GL_APIENTRY __GL_APINAME(TexParameteriv)(GLenum target, GLenum pname, const GLint* params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexParameteriv(gc, target, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(TexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexSubImage2D(gc, target, level, xoffset, yoffset, width, height, format, type, pixels);
}

GLvoid GL_APIENTRY __GL_APINAME(Uniform1f)(GLint location, GLfloat x)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Uniform1f(gc, location, x);
    __GL_PATTERN_MATCH_NAME(__glApiEnum(Uniform1f));
}

GLvoid GL_APIENTRY __GL_APINAME(Uniform1fv)(GLint location, GLsizei count, const GLfloat* v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Uniform1fv(gc, location, count, v);
}

GLvoid GL_APIENTRY __GL_APINAME(Uniform1i)(GLint location, GLint x)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Uniform1i(gc, location, x);
}

GLvoid GL_APIENTRY __GL_APINAME(Uniform1iv)(GLint location, GLsizei count, const GLint* v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Uniform1iv(gc, location, count, v);
}

GLvoid GL_APIENTRY __GL_APINAME(Uniform2f)(GLint location, GLfloat x, GLfloat y)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Uniform2f(gc, location, x, y);
    __GL_PATTERN_MATCH_NAME(__glApiEnum(Uniform2f));
}

GLvoid GL_APIENTRY __GL_APINAME(Uniform2fv)(GLint location, GLsizei count, const GLfloat* v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Uniform2fv(gc, location, count, v);
}

GLvoid GL_APIENTRY __GL_APINAME(Uniform2i)(GLint location, GLint x, GLint y)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Uniform2i(gc, location, x, y);
}

GLvoid GL_APIENTRY __GL_APINAME(Uniform2iv)(GLint location, GLsizei count, const GLint* v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Uniform2iv(gc, location, count, v);
}

GLvoid GL_APIENTRY __GL_APINAME(Uniform3f)(GLint location, GLfloat x, GLfloat y, GLfloat z)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Uniform3f(gc, location, x, y, z);
}

GLvoid GL_APIENTRY __GL_APINAME(Uniform3fv)(GLint location, GLsizei count, const GLfloat* v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Uniform3fv(gc, location, count, v);
}

GLvoid GL_APIENTRY __GL_APINAME(Uniform3i)(GLint location, GLint x, GLint y, GLint z)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Uniform3i(gc, location, x, y, z);
}

GLvoid GL_APIENTRY __GL_APINAME(Uniform3iv)(GLint location, GLsizei count, const GLint* v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Uniform3iv(gc, location, count, v);
}

GLvoid GL_APIENTRY __GL_APINAME(Uniform4f)(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Uniform4f(gc, location, x, y, z, w);
    __GL_PATTERN_MATCH_NAME(__glApiEnum(Uniform4f));
}

GLvoid GL_APIENTRY __GL_APINAME(Uniform4fv)(GLint location, GLsizei count, const GLfloat* v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Uniform4fv(gc, location, count, v);
}

GLvoid GL_APIENTRY __GL_APINAME(Uniform4i)(GLint location, GLint x, GLint y, GLint z, GLint w)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Uniform4i(gc, location, x, y, z, w);
}

GLvoid GL_APIENTRY __GL_APINAME(Uniform4iv)(GLint location, GLsizei count, const GLint* v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Uniform4iv(gc, location, count, v);
}

GLvoid GL_APIENTRY __GL_APINAME(UniformMatrix2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->UniformMatrix2fv(gc, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GL_APINAME(UniformMatrix3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->UniformMatrix3fv(gc, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GL_APINAME(UniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->UniformMatrix4fv(gc, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GL_APINAME(UseProgram)(GLuint program)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->UseProgram(gc, program);
}

GLvoid GL_APIENTRY __GL_APINAME(ValidateProgram)(GLuint program)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ValidateProgram(gc, program);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttrib1f)(GLuint indx, GLfloat x)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttrib1f(gc, indx, x);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttrib1fv)(GLuint indx, const GLfloat* values)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttrib1fv(gc, indx, values);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttrib2f)(GLuint indx, GLfloat x, GLfloat y)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttrib2f(gc, indx, x, y);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttrib2fv)(GLuint indx, const GLfloat* values)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttrib2fv(gc, indx, values);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttrib3f)(GLuint indx, GLfloat x, GLfloat y, GLfloat z)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttrib3f(gc, indx, x, y, z);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttrib3fv)(GLuint indx, const GLfloat* values)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttrib3fv(gc, indx, values);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttrib4f)(GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttrib4f(gc, indx, x, y, z, w);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttrib4fv)(GLuint indx, const GLfloat* values)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttrib4fv(gc, indx, values);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttribPointer)(GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttribPointer(gc, indx, size, type, normalized, stride, ptr);
    __GL_PATTERN_MATCH_NAME_WITH_ARG2(__glApiEnum(VertexAttribPointer), type, normalized);
}

GLvoid GL_APIENTRY __GL_APINAME(Viewport)(GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Viewport(gc, x, y, width, height);
}


/*
** OpenGL ES 3.0
*/

GLvoid GL_APIENTRY __GL_APINAME(ReadBuffer)(GLenum mode)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ReadBuffer(gc, mode);
}

GLvoid GL_APIENTRY __GL_APINAME(DrawRangeElements)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DrawRangeElements(gc, mode, start, end, count, type, indices);
}

GLvoid GL_APIENTRY __GL_APINAME(TexImage3D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexImage3D(gc, target, level, internalformat, width, height, depth, border, format, type, pixels);
}

GLvoid GL_APIENTRY __GL_APINAME(TexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexSubImage3D(gc, target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
}

GLvoid GL_APIENTRY __GL_APINAME(CopyTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->CopyTexSubImage3D(gc, target, level, xoffset, yoffset, zoffset, x, y, width, height);
}

GLvoid GL_APIENTRY __GL_APINAME(CompressedTexImage3D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->CompressedTexImage3D(gc, target, level, internalformat, width, height, depth, border, imageSize, data);
}

GLvoid GL_APIENTRY __GL_APINAME(CompressedTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->CompressedTexSubImage3D(gc, target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
}

GLvoid GL_APIENTRY __GL_APINAME(GenQueries)(GLsizei n, GLuint* ids)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GenQueries(gc, n, ids);
}

GLvoid GL_APIENTRY __GL_APINAME(DeleteQueries)(GLsizei n, const GLuint* ids)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DeleteQueries(gc, n, ids);
}

GLboolean GL_APIENTRY __GL_APINAME(IsQuery)(GLuint id)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->pEntryDispatch->IsQuery(gc, id);
}

GLvoid GL_APIENTRY __GL_APINAME(BeginQuery)(GLenum target, GLuint id)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->BeginQuery(gc, target, id);
}

GLvoid GL_APIENTRY __GL_APINAME(EndQuery)(GLenum target)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->EndQuery(gc, target);
}

GLvoid GL_APIENTRY __GL_APINAME(GetQueryiv)(GLenum target, GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetQueryiv(gc, target, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GetQueryObjectuiv)(GLuint id, GLenum pname, GLuint* params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetQueryObjectuiv(gc, id, pname, params);
}

GLboolean GL_APIENTRY __GL_APINAME(UnmapBuffer)(GLenum target)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->pEntryDispatch->UnmapBuffer(gc, target);
}

GLvoid GL_APIENTRY __GL_APINAME(GetBufferPointerv)(GLenum target, GLenum pname, GLvoid** params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetBufferPointerv(gc, target, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(DrawBuffers)(GLsizei n, const GLenum* bufs)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DrawBuffers(gc, n, bufs);
}

GLvoid GL_APIENTRY __GL_APINAME(UniformMatrix2x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->UniformMatrix2x3fv(gc, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GL_APINAME(UniformMatrix3x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->UniformMatrix3x2fv(gc, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GL_APINAME(UniformMatrix2x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->UniformMatrix2x4fv(gc, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GL_APINAME(UniformMatrix4x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->UniformMatrix4x2fv(gc, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GL_APINAME(UniformMatrix3x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->UniformMatrix3x4fv(gc, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GL_APINAME(UniformMatrix4x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->UniformMatrix4x3fv(gc, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GL_APINAME(BlitFramebuffer)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->BlitFramebuffer(gc, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
}

GLvoid GL_APIENTRY __GL_APINAME(RenderbufferStorageMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->RenderbufferStorageMultisample(gc, target, samples, internalformat, width, height);
}

GLvoid GL_APIENTRY __GL_APINAME(FramebufferTextureLayer)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->FramebufferTextureLayer(gc, target, attachment, texture, level, layer);
}

GLvoid* GL_APIENTRY __GL_APINAME(MapBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
    __GL_GET_CONTEXT_RET(gcvNULL);
    return gc->pEntryDispatch->MapBufferRange(gc, target, offset, length, access);
}

GLvoid GL_APIENTRY __GL_APINAME(FlushMappedBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->FlushMappedBufferRange(gc, target, offset, length);
}

GLvoid GL_APIENTRY __GL_APINAME(BindVertexArray)(GLuint array)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->BindVertexArray(gc, array);
}

GLvoid GL_APIENTRY __GL_APINAME(DeleteVertexArrays)(GLsizei n, const GLuint* arrays)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DeleteVertexArrays(gc, n, arrays);
}

GLvoid GL_APIENTRY __GL_APINAME(GenVertexArrays)(GLsizei n, GLuint* arrays)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GenVertexArrays(gc, n, arrays);
}

GLboolean GL_APIENTRY __GL_APINAME(IsVertexArray)(GLuint array)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->pEntryDispatch->IsVertexArray(gc, array);
}

GLvoid GL_APIENTRY __GL_APINAME(GetIntegeri_v)(GLenum target, GLuint index, GLint* data)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetIntegeri_v(gc, target, index, data);
}

GLvoid GL_APIENTRY __GL_APINAME(BeginTransformFeedback)(GLenum primitiveMode)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->BeginTransformFeedback(gc, primitiveMode);
}

GLvoid GL_APIENTRY __GL_APINAME(EndTransformFeedback)(GLvoid)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->EndTransformFeedback(gc);
}

GLvoid GL_APIENTRY __GL_APINAME(BindBufferRange)(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->BindBufferRange(gc, target, index, buffer, offset, size);
}

GLvoid GL_APIENTRY __GL_APINAME(BindBufferBase)(GLenum target, GLuint index, GLuint buffer)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->BindBufferBase(gc, target, index, buffer);
}

GLvoid GL_APIENTRY __GL_APINAME(TransformFeedbackVaryings)(GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TransformFeedbackVaryings(gc, program, count, varyings, bufferMode);
}

GLvoid GL_APIENTRY __GL_APINAME(GetTransformFeedbackVarying)(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetTransformFeedbackVarying(gc, program, index, bufSize, length, size, type, name);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttribIPointer)(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttribIPointer(gc, index, size, type, stride, pointer);
}

GLvoid GL_APIENTRY __GL_APINAME(GetVertexAttribIiv)(GLuint index, GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetVertexAttribIiv(gc, index, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GetVertexAttribIuiv)(GLuint index, GLenum pname, GLuint* params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetVertexAttribIuiv(gc, index, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttribI4i)(GLuint index, GLint x, GLint y, GLint z, GLint w)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttribI4i(gc, index, x, y, z, w);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttribI4ui)(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttribI4ui(gc, index, x, y, z, w);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttribI4iv)(GLuint index, const GLint* v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttribI4iv(gc, index, v);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttribI4uiv)(GLuint index, const GLuint* v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttribI4uiv(gc, index, v);
}

GLvoid GL_APIENTRY __GL_APINAME(GetUniformuiv)(GLuint program, GLint location, GLuint* params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetUniformuiv(gc, program, location, params);
}

GLint GL_APIENTRY __GL_APINAME(GetFragDataLocation)(GLuint program, const GLchar *name)
{
    __GL_GET_CONTEXT_RET(GL_INVALID_OPERATION);
    return gc->pEntryDispatch->GetFragDataLocation(gc, program, name);
}

GLvoid GL_APIENTRY __GL_APINAME(Uniform1ui)(GLint location, GLuint v0)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Uniform1ui(gc, location, v0);
}

GLvoid GL_APIENTRY __GL_APINAME(Uniform2ui)(GLint location, GLuint v0, GLuint v1)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Uniform2ui(gc, location, v0, v1);
}

GLvoid GL_APIENTRY __GL_APINAME(Uniform3ui)(GLint location, GLuint v0, GLuint v1, GLuint v2)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Uniform3ui(gc, location, v0, v1, v2);
}

GLvoid GL_APIENTRY __GL_APINAME(Uniform4ui)(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Uniform4ui(gc, location, v0, v1, v2, v3);
}

GLvoid GL_APIENTRY __GL_APINAME(Uniform1uiv)(GLint location, GLsizei count, const GLuint* value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Uniform1uiv(gc, location, count, value);
}

GLvoid GL_APIENTRY __GL_APINAME(Uniform2uiv)(GLint location, GLsizei count, const GLuint* value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Uniform2uiv(gc, location, count, value);
}

GLvoid GL_APIENTRY __GL_APINAME(Uniform3uiv)(GLint location, GLsizei count, const GLuint* value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Uniform3uiv(gc, location, count, value);
}

GLvoid GL_APIENTRY __GL_APINAME(Uniform4uiv)(GLint location, GLsizei count, const GLuint* value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Uniform4uiv(gc, location, count, value);
}

GLvoid GL_APIENTRY __GL_APINAME(ClearBufferiv)(GLenum buffer, GLint drawbuffer, const GLint* value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ClearBufferiv(gc, buffer, drawbuffer, value);
}

GLvoid GL_APIENTRY __GL_APINAME(ClearBufferuiv)(GLenum buffer, GLint drawbuffer, const GLuint* value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ClearBufferuiv(gc, buffer, drawbuffer, value);
}

GLvoid GL_APIENTRY __GL_APINAME(ClearBufferfv)(GLenum buffer, GLint drawbuffer, const GLfloat* value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ClearBufferfv(gc, buffer, drawbuffer, value);
}

GLvoid GL_APIENTRY __GL_APINAME(ClearBufferfi)(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ClearBufferfi(gc, buffer, drawbuffer, depth, stencil);
}

const GLubyte* GL_APIENTRY __GL_APINAME(GetStringi)(GLenum name, GLuint index)
{
    __GL_GET_CONTEXT_RET(gcvNULL);
    return gc->pEntryDispatch->GetStringi(gc, name, index);
}

GLvoid GL_APIENTRY __GL_APINAME(CopyBufferSubData)(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->CopyBufferSubData(gc, readTarget, writeTarget, readOffset, writeOffset, size);
}

GLvoid GL_APIENTRY __GL_APINAME(GetUniformIndices)(GLuint program, GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetUniformIndices(gc, program, uniformCount, uniformNames, uniformIndices);
}

GLvoid GL_APIENTRY __GL_APINAME(GetActiveUniformsiv)(GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetActiveUniformsiv(gc, program, uniformCount, uniformIndices, pname, params);
}

GLuint GL_APIENTRY __GL_APINAME(GetUniformBlockIndex)(GLuint program, const GLchar* uniformBlockName)
{
    __GL_GET_CONTEXT_RET(GL_INVALID_INDEX);
    return gc->pEntryDispatch->GetUniformBlockIndex(gc, program, uniformBlockName);
}

GLvoid GL_APIENTRY __GL_APINAME(GetActiveUniformBlockiv)(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetActiveUniformBlockiv(gc, program, uniformBlockIndex, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GetActiveUniformBlockName)(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetActiveUniformBlockName(gc, program, uniformBlockIndex, bufSize, length, uniformBlockName);
}

GLvoid GL_APIENTRY __GL_APINAME(UniformBlockBinding)(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->UniformBlockBinding(gc, program, uniformBlockIndex, uniformBlockBinding);
}

GLvoid GL_APIENTRY __GL_APINAME(DrawArraysInstanced)(GLenum mode, GLint first, GLsizei count, GLsizei instanceCount)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DrawArraysInstanced(gc, mode, first, count, instanceCount);
}

GLvoid GL_APIENTRY __GL_APINAME(DrawElementsInstanced)(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei instanceCount)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DrawElementsInstanced(gc, mode, count, type, indices, instanceCount);
}

GLsync GL_APIENTRY __GL_APINAME(FenceSync)(GLenum condition, GLbitfield flags)
{
    __GL_GET_CONTEXT_RET(0);
    return gc->pEntryDispatch->FenceSync(gc, condition, flags);
}

GLboolean GL_APIENTRY __GL_APINAME(IsSync)(GLsync sync)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->pEntryDispatch->IsSync(gc, sync);
}

GLvoid GL_APIENTRY __GL_APINAME(DeleteSync)(GLsync sync)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DeleteSync(gc, sync);
}

GLenum GL_APIENTRY __GL_APINAME(ClientWaitSync)(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    __GL_GET_CONTEXT_RET(GL_WAIT_FAILED);
    return gc->pEntryDispatch->ClientWaitSync(gc, sync, flags, timeout);
}

GLvoid GL_APIENTRY __GL_APINAME(WaitSync)(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->WaitSync(gc, sync, flags, timeout);
}

GLvoid GL_APIENTRY __GL_APINAME(GetInteger64v)(GLenum pname, GLint64* params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetInteger64v(gc, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GetSynciv)(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetSynciv(gc, sync, pname, bufSize, length, values);
}

GLvoid GL_APIENTRY __GL_APINAME(GetInteger64i_v)(GLenum target, GLuint index, GLint64* data)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetInteger64i_v(gc, target, index, data);
}

GLvoid GL_APIENTRY __GL_APINAME(GetBufferParameteri64v)(GLenum target, GLenum pname, GLint64* params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetBufferParameteri64v(gc, target, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GenSamplers)(GLsizei count, GLuint* samplers)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GenSamplers(gc, count, samplers);
}

GLvoid GL_APIENTRY __GL_APINAME(DeleteSamplers)(GLsizei count, const GLuint* samplers)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DeleteSamplers(gc, count, samplers);
}

GLboolean GL_APIENTRY __GL_APINAME(IsSampler)(GLuint sampler)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->pEntryDispatch->IsSampler(gc, sampler);
}

GLvoid GL_APIENTRY __GL_APINAME(BindSampler)(GLuint unit, GLuint sampler)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->BindSampler(gc, unit, sampler);
}

GLvoid GL_APIENTRY __GL_APINAME(SamplerParameteri)(GLuint sampler, GLenum pname, GLint param)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->SamplerParameteri(gc, sampler, pname, param);
}

GLvoid GL_APIENTRY __GL_APINAME(SamplerParameteriv)(GLuint sampler, GLenum pname, const GLint* param)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->SamplerParameteriv(gc, sampler, pname, param);
}

GLvoid GL_APIENTRY __GL_APINAME(SamplerParameterf)(GLuint sampler, GLenum pname, GLfloat param)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->SamplerParameterf(gc, sampler, pname, param);
}

GLvoid GL_APIENTRY __GL_APINAME(SamplerParameterfv)(GLuint sampler, GLenum pname, const GLfloat* param)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->SamplerParameterfv(gc, sampler, pname, param);
}

GLvoid GL_APIENTRY __GL_APINAME(GetSamplerParameteriv)(GLuint sampler, GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetSamplerParameteriv(gc, sampler, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GetSamplerParameterfv)(GLuint sampler, GLenum pname, GLfloat* params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetSamplerParameterfv(gc, sampler, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttribDivisor)(GLuint index, GLuint divisor)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttribDivisor(gc, index, divisor);
}

GLvoid GL_APIENTRY __GL_APINAME(BindTransformFeedback)(GLenum target, GLuint id)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->BindTransformFeedback(gc, target, id);
}

GLvoid GL_APIENTRY __GL_APINAME(DeleteTransformFeedbacks)(GLsizei n, const GLuint* ids)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DeleteTransformFeedbacks(gc, n, ids);
}

GLvoid GL_APIENTRY __GL_APINAME(GenTransformFeedbacks)(GLsizei n, GLuint* ids)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GenTransformFeedbacks(gc, n, ids);
}

GLboolean GL_APIENTRY __GL_APINAME(IsTransformFeedback)(GLuint id)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->pEntryDispatch->IsTransformFeedback(gc, id);
}

GLvoid GL_APIENTRY __GL_APINAME(PauseTransformFeedback)(GLvoid)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->PauseTransformFeedback(gc);
}

GLvoid GL_APIENTRY __GL_APINAME(ResumeTransformFeedback)(GLvoid)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ResumeTransformFeedback(gc);
}

GLvoid GL_APIENTRY __GL_APINAME(GetProgramBinary)(GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, GLvoid* binary)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetProgramBinary(gc, program, bufSize, length, binaryFormat, binary);
}

GLvoid GL_APIENTRY __GL_APINAME(ProgramBinary)(GLuint program, GLenum binaryFormat, const GLvoid* binary, GLsizei length)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ProgramBinary(gc, program, binaryFormat, binary, length);
}

GLvoid GL_APIENTRY __GL_APINAME(ProgramParameteri)(GLuint program, GLenum pname, GLint value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ProgramParameteri(gc, program, pname, value);
}

GLvoid GL_APIENTRY __GL_APINAME(InvalidateFramebuffer)(GLenum target, GLsizei numAttachments, const GLenum* attachments)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->InvalidateFramebuffer(gc, target, numAttachments, attachments);
}

GLvoid GL_APIENTRY __GL_APINAME(InvalidateSubFramebuffer)(GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->InvalidateSubFramebuffer(gc, target, numAttachments, attachments, x, y, width, height);
}

GLvoid GL_APIENTRY __GL_APINAME(TexStorage2D)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexStorage2D(gc, target, levels, internalformat, width, height);
}

GLvoid GL_APIENTRY __GL_APINAME(TexStorage3D)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexStorage3D(gc, target, levels, internalformat, width, height, depth);
}

GLvoid GL_APIENTRY __GL_APINAME(GetInternalformativ)(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetInternalformativ(gc, target, internalformat, pname, bufSize, params);
}

/*
** OpenGL ES 3.1
*/
GLvoid GL_APIENTRY __GL_APINAME(DispatchCompute)(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DispatchCompute(gc, num_groups_x, num_groups_y, num_groups_z);
}

GLvoid GL_APIENTRY __GL_APINAME(DispatchComputeIndirect)(GLintptr indirect)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DispatchComputeIndirect(gc, indirect);
}

GLvoid GL_APIENTRY __GL_APINAME(DrawArraysIndirect)(GLenum mode, const void *indirect)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DrawArraysIndirect(gc, mode, indirect);
}

GLvoid GL_APIENTRY __GL_APINAME(DrawElementsIndirect)(GLenum mode, GLenum type, const void *indirect)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DrawElementsIndirect(gc, mode, type, indirect);
}

GLvoid GL_APIENTRY __GL_APINAME(FramebufferParameteri)(GLenum target, GLenum pname, GLint param)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->FramebufferParameteri(gc, target, pname, param);
}

GLvoid GL_APIENTRY __GL_APINAME(GetFramebufferParameteriv)(GLenum target, GLenum pname, GLint *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetFramebufferParameteriv(gc, target, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GetProgramInterfaceiv)(GLuint program, GLenum programInterface, GLenum pname, GLint *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetProgramInterfaceiv(gc, program, programInterface, pname, params);
}

GLuint GL_APIENTRY __GL_APINAME(GetProgramResourceIndex)(GLuint program, GLenum programInterface, const GLchar *name)
{
    __GL_GET_CONTEXT_RET(0xffffffff);
    return gc->pEntryDispatch->GetProgramResourceIndex(gc, program, programInterface, name);
}

GLvoid GL_APIENTRY __GL_APINAME(GetProgramResourceName)(GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetProgramResourceName(gc, program, programInterface, index, bufSize, length, name);
}

GLvoid GL_APIENTRY __GL_APINAME(GetProgramResourceiv)(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetProgramResourceiv(gc, program, programInterface, index, propCount, props, bufSize, length, params);
}

GLint GL_APIENTRY __GL_APINAME(GetProgramResourceLocation)(GLuint program, GLenum programInterface, const GLchar *name)
{
    __GL_GET_CONTEXT_RET(-1);
    return gc->pEntryDispatch->GetProgramResourceLocation(gc, program, programInterface, name);
}

GLvoid GL_APIENTRY __GL_APINAME(UseProgramStages)(GLuint pipeline, GLbitfield stages, GLuint program)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->UseProgramStages(gc, pipeline, stages, program);
}

GLvoid GL_APIENTRY __GL_APINAME(ActiveShaderProgram)(GLuint pipeline, GLuint program)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ActiveShaderProgram(gc, pipeline, program);
}

GLuint GL_APIENTRY __GL_APINAME(CreateShaderProgramv)(GLenum type, GLsizei count, const GLchar *const*strings)
{
    __GL_GET_CONTEXT_RET(0);
    return gc->pEntryDispatch->CreateShaderProgramv(gc, type, count, strings);
}

GLvoid GL_APIENTRY __GL_APINAME(BindProgramPipeline)(GLuint pipeline)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->BindProgramPipeline(gc, pipeline);
}

GLvoid GL_APIENTRY __GL_APINAME(DeleteProgramPipelines)(GLsizei n, const GLuint *pipelines)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DeleteProgramPipelines(gc, n , pipelines);
}

GLvoid GL_APIENTRY __GL_APINAME(GenProgramPipelines)(GLsizei n, GLuint *pipelines)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GenProgramPipelines(gc, n , pipelines);
}

GLboolean GL_APIENTRY __GL_APINAME(IsProgramPipeline)(GLuint pipeline)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->pEntryDispatch->IsProgramPipeline(gc, pipeline);
}

GLvoid GL_APIENTRY __GL_APINAME(GetProgramPipelineiv)(GLuint pipeline, GLenum pname, GLint *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetProgramPipelineiv(gc, pipeline, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(ProgramUniform1i)(GLuint program, GLint location, GLint v0)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ProgramUniform1i(gc, program, location, v0);
}

GLvoid GL_APIENTRY __GL_APINAME(ProgramUniform2i)(GLuint program, GLint location, GLint v0, GLint v1)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ProgramUniform2i(gc, program, location, v0, v1);
}

GLvoid GL_APIENTRY __GL_APINAME(ProgramUniform3i)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ProgramUniform3i(gc, program, location, v0, v1, v2);
}

GLvoid GL_APIENTRY __GL_APINAME(ProgramUniform4i)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ProgramUniform4i(gc, program, location, v0, v1, v2, v3);
}

GLvoid GL_APIENTRY __GL_APINAME(ProgramUniform1ui)(GLuint program, GLint location, GLuint v0)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ProgramUniform1ui(gc, program, location, v0);
}

GLvoid GL_APIENTRY __GL_APINAME(ProgramUniform2ui)(GLuint program, GLint location, GLuint v0, GLuint v1)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ProgramUniform2ui(gc, program, location, v0, v1);
}

GLvoid GL_APIENTRY __GL_APINAME(ProgramUniform3ui)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ProgramUniform3ui(gc, program, location, v0, v1, v2);
}

GLvoid GL_APIENTRY __GL_APINAME(ProgramUniform4ui)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ProgramUniform4ui(gc, program, location, v0, v1, v2, v3);
}

GLvoid GL_APIENTRY __GL_APINAME(ProgramUniform1f)(GLuint program, GLint location, GLfloat v0)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ProgramUniform1f(gc, program, location, v0);
}

GLvoid GL_APIENTRY __GL_APINAME(ProgramUniform2f)(GLuint program, GLint location, GLfloat v0, GLfloat v1)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ProgramUniform2f(gc, program, location, v0, v1);
}

GLvoid GL_APIENTRY __GL_APINAME(ProgramUniform3f)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ProgramUniform3f(gc, program, location, v0, v1, v2);
}

GLvoid GL_APIENTRY __GL_APINAME(ProgramUniform4f)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ProgramUniform4f(gc, program, location, v0, v1, v2, v3);
}

GLvoid GL_APIENTRY __GL_APINAME(ProgramUniform1iv)(GLuint program, GLint location, GLsizei count, const GLint *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ProgramUniform1iv(gc, program, location, count, value);
}

GLvoid GL_APIENTRY __GL_APINAME(ProgramUniform2iv)(GLuint program, GLint location, GLsizei count, const GLint *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ProgramUniform2iv(gc, program, location, count, value);
}

GLvoid GL_APIENTRY __GL_APINAME(ProgramUniform3iv)(GLuint program, GLint location, GLsizei count, const GLint *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ProgramUniform3iv(gc, program, location, count, value);
}

GLvoid GL_APIENTRY __GL_APINAME(ProgramUniform4iv)(GLuint program, GLint location, GLsizei count, const GLint *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ProgramUniform4iv(gc, program, location, count, value);
}

GLvoid GL_APIENTRY __GL_APINAME(ProgramUniform1uiv)(GLuint program, GLint location, GLsizei count, const GLuint *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ProgramUniform1uiv(gc, program, location, count, value);
}

GLvoid GL_APIENTRY __GL_APINAME(ProgramUniform2uiv)(GLuint program, GLint location, GLsizei count, const GLuint *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ProgramUniform2uiv(gc, program, location, count, value);
}

GLvoid GL_APIENTRY __GL_APINAME(ProgramUniform3uiv)(GLuint program, GLint location, GLsizei count, const GLuint *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ProgramUniform3uiv(gc, program, location, count, value);
}

GLvoid GL_APIENTRY __GL_APINAME(ProgramUniform4uiv)(GLuint program, GLint location, GLsizei count, const GLuint *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ProgramUniform4uiv(gc, program, location, count, value);
}

GLvoid GL_APIENTRY __GL_APINAME(ProgramUniform1fv)(GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ProgramUniform1fv(gc, program, location, count, value);
}

GLvoid GL_APIENTRY __GL_APINAME(ProgramUniform2fv)(GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ProgramUniform2fv(gc, program, location, count, value);
}

GLvoid GL_APIENTRY __GL_APINAME(ProgramUniform3fv)(GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ProgramUniform3fv(gc, program, location, count, value);
}

GLvoid GL_APIENTRY __GL_APINAME(ProgramUniform4fv)(GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ProgramUniform4fv(gc, program, location, count, value);
}

GLvoid GL_APIENTRY __GL_APINAME(ProgramUniformMatrix2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ProgramUniformMatrix2fv(gc, program, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GL_APINAME(ProgramUniformMatrix3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ProgramUniformMatrix3fv(gc, program, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GL_APINAME(ProgramUniformMatrix4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ProgramUniformMatrix4fv(gc, program, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GL_APINAME(ProgramUniformMatrix2x3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ProgramUniformMatrix2x3fv(gc, program, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GL_APINAME(ProgramUniformMatrix3x2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ProgramUniformMatrix3x2fv(gc, program, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GL_APINAME(ProgramUniformMatrix2x4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ProgramUniformMatrix2x4fv(gc, program, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GL_APINAME(ProgramUniformMatrix4x2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ProgramUniformMatrix4x2fv(gc, program, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GL_APINAME(ProgramUniformMatrix3x4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ProgramUniformMatrix3x4fv(gc, program, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GL_APINAME(ProgramUniformMatrix4x3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ProgramUniformMatrix4x3fv(gc, program, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GL_APINAME(ValidateProgramPipeline)(GLuint pipeline)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ValidateProgramPipeline(gc, pipeline);
}

GLvoid GL_APIENTRY __GL_APINAME(GetProgramPipelineInfoLog)(GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetProgramPipelineInfoLog(gc, pipeline, bufSize, length, infoLog);
}

GLvoid GL_APIENTRY __GL_APINAME(BindImageTexture)(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->BindImageTexture(gc, unit, texture, level, layered, layer, access, format);
}

GLvoid GL_APIENTRY __GL_APINAME(GetBooleani_v)(GLenum target, GLuint index, GLboolean *data)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetBooleani_v(gc, target, index, data);
}

GLvoid GL_APIENTRY __GL_APINAME(MemoryBarrier)(GLbitfield barriers)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MemoryBarrier(gc, barriers);
}

GLvoid GL_APIENTRY __GL_APINAME(MemoryBarrierByRegion)(GLbitfield barriers)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MemoryBarrierByRegion(gc, barriers);
}

GLvoid GL_APIENTRY __GL_APINAME(TexStorage2DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexStorage2DMultisample(gc, target, samples, internalformat, width, height, fixedsamplelocations);
}

GLvoid GL_APIENTRY __GL_APINAME(GetMultisamplefv)(GLenum pname, GLuint index, GLfloat *val)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetMultisamplefv(gc, pname, index, val);
}

GLvoid GL_APIENTRY __GL_APINAME(SampleMaski)(GLuint maskNumber, GLbitfield mask)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->SampleMaski(gc, maskNumber, mask);
}

GLvoid GL_APIENTRY __GL_APINAME(GetTexLevelParameteriv)(GLenum target, GLint level, GLenum pname, GLint *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetTexLevelParameteriv(gc, target, level, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GetTexLevelParameterfv)(GLenum target, GLint level, GLenum pname, GLfloat *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetTexLevelParameterfv(gc, target, level, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(BindVertexBuffer)(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->BindVertexBuffer(gc, bindingindex, buffer, offset, stride);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttribFormat)(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttribFormat(gc, attribindex, size, type, normalized, relativeoffset);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttribIFormat)(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttribIFormat(gc, attribindex, size, type, relativeoffset);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttribBinding)(GLuint attribindex, GLuint bindingindex)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttribBinding(gc, attribindex, bindingindex);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexBindingDivisor)(GLuint bindingindex, GLuint divisor)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexBindingDivisor(gc, bindingindex, divisor);
}

/*
** OpenGL ES 3.2
*/

GLvoid GL_APIENTRY __GL_APINAME(TexStorage3DMultisample)(GLenum target, GLsizei samples, GLenum sizedinternalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexStorage3DMultisample(gc, target, samples, sizedinternalformat, width, height, depth, fixedsamplelocations);
}

GLvoid GL_APIENTRY __GL_APINAME(BlendBarrier)(void)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->BlendBarrier(gc);
}

GLvoid GL_APIENTRY __GL_APINAME(DebugMessageControl)(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DebugMessageControl(gc, source, type, severity, count, ids, enabled);
}

GLvoid GL_APIENTRY __GL_APINAME(DebugMessageInsert)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* buf)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DebugMessageInsert(gc, source, type, id, severity, length, buf);
}

GLvoid GL_APIENTRY __GL_APINAME(DebugMessageCallback)(GLDEBUGPROCKHR callback, const GLvoid* userParam)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DebugMessageCallback(gc, callback, userParam);
}

GLuint GL_APIENTRY __GL_APINAME(GetDebugMessageLog)(GLuint count, GLsizei bufSize, GLenum* sources, GLenum* types, GLuint* ids, GLenum* severities, GLsizei* lengths, GLchar* messageLog)
{
    __GL_GET_CONTEXT_RET(0);
    return gc->pEntryDispatch->GetDebugMessageLog(gc, count, bufSize, sources, types, ids, severities, lengths, messageLog);
}

GLvoid GL_APIENTRY __GL_APINAME(GetPointerv)(GLenum pname, GLvoid** params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetPointerv(gc, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(PushDebugGroup)(GLenum source, GLuint id, GLsizei length, const GLchar * message)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->PushDebugGroup(gc, source, id, length, message);
}

GLvoid GL_APIENTRY __GL_APINAME(PopDebugGroup)(void)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->PopDebugGroup(gc);
}

GLvoid GL_APIENTRY __GL_APINAME(ObjectLabel)(GLenum identifier, GLuint name, GLsizei length, const GLchar *label)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ObjectLabel(gc, identifier, name, length, label);
}

GLvoid GL_APIENTRY __GL_APINAME(GetObjectLabel)(GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetObjectLabel(gc, identifier, name, bufSize, length, label);
}

GLvoid GL_APIENTRY __GL_APINAME(ObjectPtrLabel)(const GLvoid* ptr, GLsizei length, const GLchar *label)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ObjectPtrLabel(gc, ptr, length, label);
}

GLvoid GL_APIENTRY __GL_APINAME(GetObjectPtrLabel)(const GLvoid* ptr, GLsizei bufSize, GLsizei *length, GLchar *label)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetObjectPtrLabel(gc, ptr, bufSize, length, label);
}

GLenum GL_APIENTRY __GL_APINAME(GetGraphicsResetStatus)()
{
    __GL_GET_CONTEXT_RET(GL_NO_ERROR);
    return gc->pEntryDispatch->GetGraphicsResetStatus(gc);
}

GLvoid GL_APIENTRY __GL_APINAME(ReadnPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, GLvoid *data)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ReadnPixels(gc, x, y, width, height, format, type, bufSize, data);
}

GLvoid GL_APIENTRY __GL_APINAME(GetnUniformfv)(GLuint program, GLint location, GLsizei bufSize, GLfloat *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetnUniformfv(gc, program, location, bufSize, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GetnUniformiv)(GLuint program, GLint location, GLsizei bufSize, GLint *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetnUniformiv(gc, program, location, bufSize, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GetnUniformuiv)(GLuint program, GLint location, GLsizei bufSize, GLuint *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetnUniformuiv(gc, program, location, bufSize, params);
}

GLvoid GL_APIENTRY __GL_APINAME(BlendEquationi)(GLuint buf, GLenum mode)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->BlendEquationi(gc, buf, mode);
}

GLvoid GL_APIENTRY __GL_APINAME(BlendEquationSeparatei)(GLuint buf, GLenum modeRGB, GLenum modeAlpha)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->BlendEquationSeparatei(gc, buf, modeRGB, modeAlpha);
}

GLvoid GL_APIENTRY __GL_APINAME(BlendFunci)(GLuint buf, GLenum sfactor, GLenum dfactor)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->BlendFunci(gc, buf, sfactor, dfactor);
}

GLvoid GL_APIENTRY __GL_APINAME(BlendFuncSeparatei)(GLuint buf, GLenum sfactorRGB,GLenum dfactorRGB,GLenum sfactorAlpha,GLenum dfactorAlpha)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->BlendFuncSeparatei(gc, buf, sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
}

GLvoid GL_APIENTRY __GL_APINAME(ColorMaski)(GLuint buf, GLboolean r, GLboolean g, GLboolean b, GLboolean a)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ColorMaski(gc, buf, r, g, b, a);
}

GLvoid GL_APIENTRY __GL_APINAME(Enablei)(GLenum target, GLuint index)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Enablei(gc, target, index);
}

GLvoid GL_APIENTRY  __GL_APINAME(Disablei)( GLenum target, GLuint index)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Disablei(gc, target, index);
}

GLboolean GL_APIENTRY  __GL_APINAME(IsEnabledi)( GLenum target, GLuint index)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->pEntryDispatch->IsEnabledi(gc, target, index);
}

GLvoid GL_APIENTRY __GL_APINAME(TexParameterIiv)(GLenum target, GLenum pname, const GLint *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexParameterIiv(gc, target, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(TexParameterIuiv)(GLenum target, GLenum pname, const GLuint *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexParameterIuiv(gc, target, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GetTexParameterIiv)(GLenum target, GLenum pname, GLint *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetTexParameterIiv(gc, target, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GetTexParameterIuiv)(GLenum target, GLenum pname, GLuint *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetTexParameterIuiv(gc, target, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(SamplerParameterIiv)(GLuint sampler, GLenum pname, const GLint *param)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->SamplerParameterIiv(gc, sampler, pname, param);
}

GLvoid GL_APIENTRY __GL_APINAME(SamplerParameterIuiv)(GLuint sampler, GLenum pname, const GLuint *param)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->SamplerParameterIuiv(gc, sampler, pname, param);
}

GLvoid GL_APIENTRY __GL_APINAME(GetSamplerParameterIiv)(GLuint sampler, GLenum pname, GLint *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetSamplerParameterIiv(gc, sampler, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GetSamplerParameterIuiv)(GLuint sampler, GLenum pname, GLuint *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetSamplerParameterIuiv(gc, sampler, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(TexBuffer)(GLenum target, GLenum internalformat, GLuint buffer)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexBuffer(gc, target, internalformat, buffer);
}

GLvoid GL_APIENTRY __GL_APINAME(TexBufferRange)(GLenum target, GLenum internalformat, GLuint buffer,
                                                  GLintptr offset, GLsizeiptr size)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexBufferRange(gc, target, internalformat, buffer, offset, size);
}

GLvoid GL_APIENTRY __GL_APINAME(PatchParameteri)(GLenum pname, GLint value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->PatchParameteri(gc, pname, value);
}

GLvoid GL_APIENTRY __GL_APINAME(FramebufferTexture)(GLenum target, GLenum attachment, GLuint texture, GLint level)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->FramebufferTexture(gc, target, attachment, texture, level);
}

GLvoid GL_APIENTRY __GL_APINAME(MinSampleShading)(GLfloat value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MinSampleShading(gc, value);
}

GLvoid GL_APIENTRY __GL_APINAME(CopyImageSubData)(GLuint srcName, GLenum srcTarget, GLint srcLevel,
                                                    GLint srcX, GLint srcY, GLint srcZ,
                                                    GLuint dstName, GLenum dstTarget, GLint dstLevel,
                                                    GLint dstX, GLint dstY, GLint dstZ,
                                                    GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->CopyImageSubData(gc, srcName, srcTarget, srcLevel, srcX, srcY, srcZ, dstName, dstTarget, dstLevel,
                                          dstX, dstY, dstZ, srcWidth, srcHeight, srcDepth);
}


GLvoid GL_APIENTRY __GL_APINAME(DrawElementsBaseVertex)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DrawElementsBaseVertex(gc, mode, count, type, indices, basevertex);
}

GLvoid GL_APIENTRY __GL_APINAME(DrawRangeElementsBaseVertex)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DrawRangeElementsBaseVertex(gc, mode, start, end, count, type, indices, basevertex);
}

GLvoid GL_APIENTRY __GL_APINAME(DrawElementsInstancedBaseVertex)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DrawElementsInstancedBaseVertex(gc, mode, count, type, indices, instancecount, basevertex);
}

GLvoid GL_APIENTRY __GL_APINAME(PrimitiveBoundingBox)(GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW,
                                                        GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->PrimitiveBoundingBox(gc, minX, minY, minZ, minW, maxX, maxY, maxZ, maxW);
}


/*
** OpenGL defines as core but ES as extension APIs
*/
GLvoid* GL_APIENTRY __GL_APINAME(MapBuffer)(GLenum target, GLenum access)
{
    __GL_GET_CONTEXT_RET(gcvNULL);
    return gc->pEntryDispatch->MapBuffer(gc, target, access);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiDrawArrays)(GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiDrawArrays(gc, mode, first, count, primcount);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiDrawElements)(GLenum mode, const GLsizei *count, GLenum type, const GLvoid*const*indices, GLsizei primcount)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiDrawElements(gc, mode, count, type, indices, primcount);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiDrawElementsBaseVertex)(GLenum mode, const GLsizei * count, GLenum type, const void *const*indices, GLsizei drawcount, const GLint *basevertex)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiDrawElementsBaseVertex(gc, mode, count, type, indices, drawcount, basevertex);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiDrawArraysIndirect)(GLenum mode, const void *indirect, GLsizei drawcount, GLsizei stride)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiDrawArraysIndirect(gc, mode, indirect, drawcount, stride);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiDrawElementsIndirect)(GLenum mode, GLenum type, const void *indirect, GLsizei drawcount, GLsizei stride)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiDrawElementsIndirect(gc, mode, type, indirect, drawcount, stride);
}


/*********************************************
**
** OpenGL only APIs
**
*********************************************/

#ifdef OPENGL40
/*
** GL_VERSION_1_0
*/
GLvoid GL_APIENTRY __GL_APINAME(NewList)(GLuint list, GLenum mode)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->NewList(gc, list, mode);
}

GLvoid GL_APIENTRY __GL_APINAME(EndList)(GLvoid)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->EndList(gc);
}

GLvoid GL_APIENTRY __GL_APINAME(CallList)(GLuint list)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->CallList(gc, list);
}

GLvoid GL_APIENTRY __GL_APINAME(CallLists)(GLsizei n, GLenum type, const GLvoid *lists)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->CallLists(gc, n, type, lists);
}

GLvoid GL_APIENTRY __GL_APINAME(DeleteLists)(GLuint list, GLsizei range)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DeleteLists(gc, list, range);
}

GLuint GL_APIENTRY __GL_APINAME(GenLists)(GLsizei range)
{
    __GL_GET_CONTEXT_RET(0);
    return gc->pEntryDispatch->GenLists(gc, range);
}

GLvoid GL_APIENTRY __GL_APINAME(ListBase)(GLuint base)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ListBase(gc, base);
}

GLvoid GL_APIENTRY __GL_APINAME(Begin)(GLenum mode)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Begin(gc, mode);
}

GLvoid GL_APIENTRY __GL_APINAME(Bitmap)(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Bitmap(gc, width, height, xorig, yorig, xmove, ymove, bitmap);
}

GLvoid GL_APIENTRY __GL_APINAME(Color3b)(GLbyte red, GLbyte green, GLbyte blue)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Color3b(gc, red, green, blue);
}

GLvoid GL_APIENTRY __GL_APINAME(Color3bv)(const GLbyte *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Color3bv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(Color3d)(GLdouble red, GLdouble green, GLdouble blue)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Color3d(gc, red, green, blue);
}

GLvoid GL_APIENTRY __GL_APINAME(Color3dv)(const GLdouble *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Color3dv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(Color3f)(GLfloat red, GLfloat green, GLfloat blue)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Color3f(gc, red, green, blue);
}

GLvoid GL_APIENTRY __GL_APINAME(Color3fv)(const GLfloat *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Color3fv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(Color3i)(GLint red, GLint green, GLint blue)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Color3i(gc, red, green, blue);
}

GLvoid GL_APIENTRY __GL_APINAME(Color3iv)(const GLint *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Color3iv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(Color3s)(GLshort red, GLshort green, GLshort blue)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Color3s(gc, red, green, blue);
}

GLvoid GL_APIENTRY __GL_APINAME(Color3sv)(const GLshort *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Color3sv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(Color3ub)(GLubyte red, GLubyte green, GLubyte blue)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Color3ub(gc, red, green, blue);
}

GLvoid GL_APIENTRY __GL_APINAME(Color3ubv)(const GLubyte *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Color3ubv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(Color3ui)(GLuint red, GLuint green, GLuint blue)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Color3ui(gc, red, green, blue);
}

GLvoid GL_APIENTRY __GL_APINAME(Color3uiv)(const GLuint *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Color3uiv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(Color3us)(GLushort red, GLushort green, GLushort blue)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Color3us(gc, red, green, blue);
}

GLvoid GL_APIENTRY __GL_APINAME(Color3usv)(const GLushort *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Color3usv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(Color4b)(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Color4b(gc, red, green, blue, alpha);
}

GLvoid GL_APIENTRY __GL_APINAME(Color4bv)(const GLbyte *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Color4bv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(Color4d)(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Color4d(gc, red, green, blue, alpha);
}

GLvoid GL_APIENTRY __GL_APINAME(Color4dv)(const GLdouble *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Color4dv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(Color4f)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Color4f(gc, red, green, blue, alpha);
}

GLvoid GL_APIENTRY __GL_APINAME(Color4fv)(const GLfloat *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Color4fv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(Color4i)(GLint red, GLint green, GLint blue, GLint alpha)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Color4i(gc, red, green, blue, alpha);
}

GLvoid GL_APIENTRY __GL_APINAME(Color4iv)(const GLint *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Color4iv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(Color4s)(GLshort red, GLshort green, GLshort blue, GLshort alpha)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Color4s(gc, red, green, blue, alpha);
}

GLvoid GL_APIENTRY __GL_APINAME(Color4sv)(const GLshort *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Color4sv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(Color4ub)(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Color4ub(gc, red, green, blue, alpha);
}

GLvoid GL_APIENTRY __GL_APINAME(Color4ubv)(const GLubyte *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Color4ubv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(Color4ui)(GLuint red, GLuint green, GLuint blue, GLuint alpha)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Color4ui(gc, red, green, blue, alpha);
}

GLvoid GL_APIENTRY __GL_APINAME(Color4uiv)(const GLuint *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Color4uiv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(Color4us)(GLushort red, GLushort green, GLushort blue, GLushort alpha)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Color4us(gc, red, green, blue, alpha);
}

GLvoid GL_APIENTRY __GL_APINAME(Color4usv)(const GLushort *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Color4usv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(EdgeFlag)(GLboolean flag)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->EdgeFlag(gc, flag);
}

GLvoid GL_APIENTRY __GL_APINAME(EdgeFlagv)(const GLboolean *flag)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->EdgeFlagv(gc, flag);
}

GLvoid GL_APIENTRY __GL_APINAME(End)(GLvoid)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->End(gc);
}

GLvoid GL_APIENTRY __GL_APINAME(Indexd)(GLdouble c)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Indexd(gc, c);
}

GLvoid GL_APIENTRY __GL_APINAME(Indexdv)(const GLdouble *c)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Indexdv(gc, c);
}

GLvoid GL_APIENTRY __GL_APINAME(Indexf)(GLfloat c)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Indexf(gc, c);
}

GLvoid GL_APIENTRY __GL_APINAME(Indexfv)(const GLfloat *c)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Indexfv(gc, c);
}

GLvoid GL_APIENTRY __GL_APINAME(Indexi)(GLint c)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Indexi(gc, c);
}

GLvoid GL_APIENTRY __GL_APINAME(Indexiv)(const GLint *c)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Indexiv(gc, c);
}

GLvoid GL_APIENTRY __GL_APINAME(Indexs)(GLshort c)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Indexs(gc, c);
}

GLvoid GL_APIENTRY __GL_APINAME(Indexsv)(const GLshort *c)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Indexsv(gc, c);
}

GLvoid GL_APIENTRY __GL_APINAME(Normal3b)(GLbyte nx, GLbyte ny, GLbyte nz)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Normal3b(gc, nx, ny, nz);
}

GLvoid GL_APIENTRY __GL_APINAME(Normal3bv)(const GLbyte *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Normal3bv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(Normal3d)(GLdouble nx, GLdouble ny, GLdouble nz)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Normal3d(gc, nx, ny, nz);
}

GLvoid GL_APIENTRY __GL_APINAME(Normal3dv)(const GLdouble *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Normal3dv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(Normal3f)(GLfloat nx, GLfloat ny, GLfloat nz)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Normal3f(gc, nx, ny, nz);
}

GLvoid GL_APIENTRY __GL_APINAME(Normal3fv)(const GLfloat *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Normal3fv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(Normal3i)(GLint nx, GLint ny, GLint nz)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Normal3i(gc, nx, ny, nz);
}

GLvoid GL_APIENTRY __GL_APINAME(Normal3iv)(const GLint *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Normal3iv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(Normal3s)(GLshort nx, GLshort ny, GLshort nz)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Normal3s(gc, nx, ny, nz);
}

GLvoid GL_APIENTRY __GL_APINAME(Normal3sv)(const GLshort *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Normal3sv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(RasterPos2d)(GLdouble x, GLdouble y)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->RasterPos2d(gc, x, y);
}

GLvoid GL_APIENTRY __GL_APINAME(RasterPos2dv)(const GLdouble *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->RasterPos2dv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(RasterPos2f)(GLfloat x, GLfloat y)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->RasterPos2f(gc, x, y);
}

GLvoid GL_APIENTRY __GL_APINAME(RasterPos2fv)(const GLfloat *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->RasterPos2fv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(RasterPos2i)(GLint x, GLint y)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->RasterPos2i(gc, x, y);
}

GLvoid GL_APIENTRY __GL_APINAME(RasterPos2iv)(const GLint *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->RasterPos2iv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(RasterPos2s)(GLshort x, GLshort y)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->RasterPos2s(gc, x, y);
}

GLvoid GL_APIENTRY __GL_APINAME(RasterPos2sv)(const GLshort *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->RasterPos2sv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(RasterPos3d)(GLdouble x, GLdouble y, GLdouble z)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->RasterPos3d(gc, x, y, z);
}

GLvoid GL_APIENTRY __GL_APINAME(RasterPos3dv)(const GLdouble *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->RasterPos3dv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(RasterPos3f)(GLfloat x, GLfloat y, GLfloat z)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->RasterPos3f(gc, x, y, z);
}

GLvoid GL_APIENTRY __GL_APINAME(RasterPos3fv)(const GLfloat *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->RasterPos3fv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(RasterPos3i)(GLint x, GLint y, GLint z)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->RasterPos3i(gc, x, y, z);
}

GLvoid GL_APIENTRY __GL_APINAME(RasterPos3iv)(const GLint *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->RasterPos3iv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(RasterPos3s)(GLshort x, GLshort y, GLshort z)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->RasterPos3s(gc, x, y, z);
}

GLvoid GL_APIENTRY __GL_APINAME(RasterPos3sv)(const GLshort *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->RasterPos3sv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(RasterPos4d)(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->RasterPos4d(gc, x, y, z, w);
}

GLvoid GL_APIENTRY __GL_APINAME(RasterPos4dv)(const GLdouble *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->RasterPos4dv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(RasterPos4f)(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->RasterPos4f(gc, x, y, z, w);
}

GLvoid GL_APIENTRY __GL_APINAME(RasterPos4fv)(const GLfloat *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->RasterPos4fv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(RasterPos4i)(GLint x, GLint y, GLint z, GLint w)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->RasterPos4i(gc, x, y, z, w);
}

GLvoid GL_APIENTRY __GL_APINAME(RasterPos4iv)(const GLint *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->RasterPos4iv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(RasterPos4s)(GLshort x, GLshort y, GLshort z, GLshort w)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->RasterPos4s(gc, x, y, z, w);
}

GLvoid GL_APIENTRY __GL_APINAME(RasterPos4sv)(const GLshort *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->RasterPos4sv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(Rectd)(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Rectd(gc, x1, y1, x2, y2);
}

GLvoid GL_APIENTRY __GL_APINAME(Rectdv)(const GLdouble *v1, const GLdouble *v2)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Rectdv(gc, v1, v2);
}

GLvoid GL_APIENTRY __GL_APINAME(Rectf)(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Rectf(gc, x1, y1, x2, y2);
}

GLvoid GL_APIENTRY __GL_APINAME(Rectfv)(const GLfloat *v1, const GLfloat *v2)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Rectfv(gc, v1, v2);
}

GLvoid GL_APIENTRY __GL_APINAME(Recti)(GLint x1, GLint y1, GLint x2, GLint y2)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Recti(gc, x1, y1, x2, y2);
}

GLvoid GL_APIENTRY __GL_APINAME(Rectiv)(const GLint *v1, const GLint *v2)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Rectiv(gc, v1, v2);
}

GLvoid GL_APIENTRY __GL_APINAME(Rects)(GLshort x1, GLshort y1, GLshort x2, GLshort y2)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Rects(gc, x1, y1, x2, y2);
}

GLvoid GL_APIENTRY __GL_APINAME(Rectsv)(const GLshort *v1, const GLshort *v2)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Rectsv(gc, v1, v2);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoord1d)(GLdouble s)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoord1d(gc, s);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoord1dv)(const GLdouble *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoord1dv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoord1f)(GLfloat s)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoord1f(gc, s);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoord1fv)(const GLfloat *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoord1fv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoord1i)(GLint s)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoord1i(gc, s);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoord1iv)(const GLint *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoord1iv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoord1s)(GLshort s)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoord1s(gc, s);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoord1sv)(const GLshort *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoord1sv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoord2d)(GLdouble s, GLdouble t)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoord2d(gc, s, t);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoord2dv)(const GLdouble *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoord2dv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoord2f)(GLfloat s, GLfloat t)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoord2f(gc, s, t);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoord2fv)(const GLfloat *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoord2fv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoord2i)(GLint s, GLint t)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoord2i(gc, s, t);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoord2iv)(const GLint *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoord2iv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoord2s)(GLshort s, GLshort t)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoord2s(gc, s, t);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoord2sv)(const GLshort *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoord2sv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoord3d)(GLdouble s, GLdouble t, GLdouble r)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoord3d(gc, s, t, r);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoord3dv)(const GLdouble *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoord3dv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoord3f)(GLfloat s, GLfloat t, GLfloat r)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoord3f(gc, s, t, r);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoord3fv)(const GLfloat *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoord3fv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoord3i)(GLint s, GLint t, GLint r)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoord3i(gc, s, t, r);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoord3iv)(const GLint *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoord3iv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoord3s)(GLshort s, GLshort t, GLshort r)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoord3s(gc, s, t, r);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoord3sv)(const GLshort *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoord3sv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoord4d)(GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoord4d(gc, s, t, r, q);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoord4dv)(const GLdouble *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoord4dv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoord4f)(GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoord4f(gc, s, t, r, q);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoord4fv)(const GLfloat *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoord4fv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoord4i)(GLint s, GLint t, GLint r, GLint q)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoord4i(gc, s, t, r, q);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoord4iv)(const GLint *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoord4iv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoord4s)(GLshort s, GLshort t, GLshort r, GLshort q)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoord4s(gc, s, t, r, q);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoord4sv)(const GLshort *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoord4sv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(Vertex2d)(GLdouble x, GLdouble y)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Vertex2d(gc, x, y);
}

GLvoid GL_APIENTRY __GL_APINAME(Vertex2dv)(const GLdouble *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Vertex2dv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(Vertex2f)(GLfloat x, GLfloat y)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Vertex2f(gc, x, y);
}

GLvoid GL_APIENTRY __GL_APINAME(Vertex2fv)(const GLfloat *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Vertex2fv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(Vertex2i)(GLint x, GLint y)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Vertex2i(gc, x, y);
}

GLvoid GL_APIENTRY __GL_APINAME(Vertex2iv)(const GLint *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Vertex2iv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(Vertex2s)(GLshort x, GLshort y)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Vertex2s(gc, x, y);
}

GLvoid GL_APIENTRY __GL_APINAME(Vertex2sv)(const GLshort *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Vertex2sv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(Vertex3d)(GLdouble x, GLdouble y, GLdouble z)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Vertex3d(gc, x, y, z);
}

GLvoid GL_APIENTRY __GL_APINAME(Vertex3dv)(const GLdouble *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Vertex3dv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(Vertex3f)(GLfloat x, GLfloat y, GLfloat z)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Vertex3f(gc, x, y, z);
}

GLvoid GL_APIENTRY __GL_APINAME(Vertex3fv)(const GLfloat *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Vertex3fv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(Vertex3i)(GLint x, GLint y, GLint z)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Vertex3i(gc, x, y, z);
}

GLvoid GL_APIENTRY __GL_APINAME(Vertex3iv)(const GLint *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Vertex3iv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(Vertex3s)(GLshort x, GLshort y, GLshort z)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Vertex3s(gc, x, y, z);
}

GLvoid GL_APIENTRY __GL_APINAME(Vertex3sv)(const GLshort *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Vertex3sv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(Vertex4d)(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Vertex4d(gc, x, y, z, w);
}

GLvoid GL_APIENTRY __GL_APINAME(Vertex4dv)(const GLdouble *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Vertex4dv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(Vertex4f)(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Vertex4f(gc, x, y, z, w);
}

GLvoid GL_APIENTRY __GL_APINAME(Vertex4fv)(const GLfloat *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Vertex4fv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(Vertex4i)(GLint x, GLint y, GLint z, GLint w)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Vertex4i(gc, x, y, z, w);
}

GLvoid GL_APIENTRY __GL_APINAME(Vertex4iv)(const GLint *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Vertex4iv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(Vertex4s)(GLshort x, GLshort y, GLshort z, GLshort w)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Vertex4s(gc, x, y, z, w);
}

GLvoid GL_APIENTRY __GL_APINAME(Vertex4sv)(const GLshort *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Vertex4sv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(ClipPlane)(GLenum plane, const GLdouble *equation)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ClipPlane(gc, plane, equation);
}

GLvoid GL_APIENTRY __GL_APINAME(ColorMaterial)(GLenum face, GLenum mode)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ColorMaterial(gc, face, mode);
}

GLvoid GL_APIENTRY __GL_APINAME(Fogf)(GLenum pname, GLfloat param)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Fogf(gc, pname, param);
}

GLvoid GL_APIENTRY __GL_APINAME(Fogfv)(GLenum pname, const GLfloat *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Fogfv(gc, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(Fogi)(GLenum pname, GLint param)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Fogi(gc, pname, param);
}

GLvoid GL_APIENTRY __GL_APINAME(Fogiv)(GLenum pname, const GLint *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Fogiv(gc, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(Lightf)(GLenum light, GLenum pname, GLfloat param)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Lightf(gc, light, pname, param);
}

GLvoid GL_APIENTRY __GL_APINAME(Lightfv)(GLenum light, GLenum pname, const GLfloat *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Lightfv(gc, light, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(Lighti)(GLenum light, GLenum pname, GLint param)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Lighti(gc, light, pname, param);
}

GLvoid GL_APIENTRY __GL_APINAME(Lightiv)(GLenum light, GLenum pname, const GLint *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Lightiv(gc, light, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(LightModelf)(GLenum pname, GLfloat param)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->LightModelf(gc, pname, param);
}

GLvoid GL_APIENTRY __GL_APINAME(LightModelfv)(GLenum pname, const GLfloat *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->LightModelfv(gc, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(LightModeli)(GLenum pname, GLint param)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->LightModeli(gc, pname, param);
}

GLvoid GL_APIENTRY __GL_APINAME(LightModeliv)(GLenum pname, const GLint *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->LightModeliv(gc, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(LineStipple)(GLint factor, GLushort pattern)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->LineStipple(gc, factor, pattern);
}

GLvoid GL_APIENTRY __GL_APINAME(Materialf)(GLenum face, GLenum pname, GLfloat param)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Materialf(gc, face, pname, param);
}

GLvoid GL_APIENTRY __GL_APINAME(Materialfv)(GLenum face, GLenum pname, const GLfloat *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Materialfv(gc, face, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(Materiali)(GLenum face, GLenum pname, GLint param)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Materiali(gc, face, pname, param);
}

GLvoid GL_APIENTRY __GL_APINAME(Materialiv)(GLenum face, GLenum pname, const GLint *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Materialiv(gc, face, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(PointSize)(GLfloat size)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->PointSize(gc, size);
}

GLvoid GL_APIENTRY __GL_APINAME(PolygonMode)(GLenum face, GLenum mode)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->PolygonMode(gc, face, mode);
}

GLvoid GL_APIENTRY __GL_APINAME(PolygonStipple)(const GLubyte *mask)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->PolygonStipple(gc, mask);
}

GLvoid GL_APIENTRY __GL_APINAME(ShadeModel)(GLenum mode)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ShadeModel(gc, mode);
}

GLvoid GL_APIENTRY __GL_APINAME(TexImage1D)(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexImage1D(gc, target, level, internalFormat, width, border, format, type, pixels);
}

GLvoid GL_APIENTRY __GL_APINAME(TexEnvf)(GLenum target, GLenum pname, GLfloat param)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexEnvf(gc, target, pname, param);
}

GLvoid GL_APIENTRY __GL_APINAME(TexEnvfv)(GLenum target, GLenum pname, const GLfloat *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexEnvfv(gc, target, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(TexEnvi)(GLenum target, GLenum pname, GLint param)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexEnvi(gc, target, pname, param);
}

GLvoid GL_APIENTRY __GL_APINAME(TexEnviv)(GLenum target, GLenum pname, const GLint *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexEnviv(gc, target, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(TexGend)(GLenum coord, GLenum pname, GLdouble param)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexGend(gc, coord, pname, param);
}

GLvoid GL_APIENTRY __GL_APINAME(TexGendv)(GLenum coord, GLenum pname, const GLdouble *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexGendv(gc, coord, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(TexGenf)(GLenum coord, GLenum pname, GLfloat param)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexGenf(gc, coord, pname, param);
}

GLvoid GL_APIENTRY __GL_APINAME(TexGenfv)(GLenum coord, GLenum pname, const GLfloat *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexGenfv(gc, coord, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(TexGeni)(GLenum coord, GLenum pname, GLint param)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexGeni(gc, coord, pname, param);
}

GLvoid GL_APIENTRY __GL_APINAME(TexGeniv)(GLenum coord, GLenum pname, const GLint *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexGeniv(gc, coord, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(FeedbackBuffer)(GLsizei size, GLenum type, GLfloat *buffer)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->FeedbackBuffer(gc, size, type, buffer);
}

GLvoid GL_APIENTRY __GL_APINAME(SelectBuffer)(GLsizei size, GLuint *buffer)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->SelectBuffer(gc, size, buffer);
}

GLint GL_APIENTRY __GL_APINAME(RenderMode)(GLenum mode)
{
    __GL_GET_CONTEXT_RET(-1);
    return gc->pEntryDispatch->RenderMode(gc, mode);
}

GLvoid GL_APIENTRY __GL_APINAME(InitNames)(GLvoid)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->InitNames(gc);
}

GLvoid GL_APIENTRY __GL_APINAME(LoadName)(GLuint name)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->LoadName(gc, name);
}

GLvoid GL_APIENTRY __GL_APINAME(PassThrough)(GLfloat token)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->PassThrough(gc, token);
}

GLvoid GL_APIENTRY __GL_APINAME(PopName)(GLvoid)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->PopName(gc);
}

GLvoid GL_APIENTRY __GL_APINAME(PushName)(GLuint name)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->PushName(gc, name);
}

GLvoid GL_APIENTRY __GL_APINAME(DrawBuffer)(GLenum mode)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DrawBuffer(gc, mode);
}

GLvoid GL_APIENTRY __GL_APINAME(ClearAccum)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ClearAccum(gc, red, green, blue, alpha);
}

GLvoid GL_APIENTRY __GL_APINAME(ClearIndex)(GLfloat c)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ClearIndex(gc, c);
}

GLvoid GL_APIENTRY __GL_APINAME(ClearDepth)(GLclampd depth)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ClearDepth(gc, depth);
}

GLvoid GL_APIENTRY __GL_APINAME(IndexMask)(GLuint mask)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->IndexMask(gc, mask);
}

GLvoid GL_APIENTRY __GL_APINAME(Accum)(GLenum op, GLfloat value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Accum(gc, op, value);
}

GLvoid GL_APIENTRY __GL_APINAME(PopAttrib)(GLvoid)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->PopAttrib(gc);
}

GLvoid GL_APIENTRY __GL_APINAME(PushAttrib)(GLbitfield mask)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->PushAttrib(gc, mask);
}

GLvoid GL_APIENTRY __GL_APINAME(Map1d)(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Map1d(gc, target, u1, u2, stride, order, points);
}

GLvoid GL_APIENTRY __GL_APINAME(Map1f)(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Map1f(gc, target, u1, u2, stride, order, points);
}

GLvoid GL_APIENTRY __GL_APINAME(Map2d)(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Map2d(gc, target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
}

GLvoid GL_APIENTRY __GL_APINAME(Map2f)(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Map2f(gc, target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
}

GLvoid GL_APIENTRY __GL_APINAME(MapGrid1d)(GLint un, GLdouble u1, GLdouble u2)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MapGrid1d(gc, un, u1, u2);
}

GLvoid GL_APIENTRY __GL_APINAME(MapGrid1f)(GLint un, GLfloat u1, GLfloat u2)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MapGrid1f(gc, un, u1, u2);
}

GLvoid GL_APIENTRY __GL_APINAME(MapGrid2d)(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MapGrid2d(gc, un, u1, u2, vn, v1, v2);
}

GLvoid GL_APIENTRY __GL_APINAME(MapGrid2f)(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MapGrid2f(gc, un, u1, u2, vn, v1, v2);
}

GLvoid GL_APIENTRY __GL_APINAME(EvalCoord1d)(GLdouble u)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->EvalCoord1d(gc, u);
}

GLvoid GL_APIENTRY __GL_APINAME(EvalCoord1dv)(const GLdouble *u)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->EvalCoord1dv(gc, u);
}

GLvoid GL_APIENTRY __GL_APINAME(EvalCoord1f)(GLfloat u)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->EvalCoord1f(gc, u);
}

GLvoid GL_APIENTRY __GL_APINAME(EvalCoord1fv)(const GLfloat *u)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->EvalCoord1fv(gc, u);
}

GLvoid GL_APIENTRY __GL_APINAME(EvalCoord2d)(GLdouble u, GLdouble v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->EvalCoord2d(gc, u, v);
}

GLvoid GL_APIENTRY __GL_APINAME(EvalCoord2dv)(const GLdouble *u)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->EvalCoord2dv(gc, u);
}

GLvoid GL_APIENTRY __GL_APINAME(EvalCoord2f)(GLfloat u, GLfloat v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->EvalCoord2f(gc, u, v);
}

GLvoid GL_APIENTRY __GL_APINAME(EvalCoord2fv)(const GLfloat *u)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->EvalCoord2fv(gc, u);
}

GLvoid GL_APIENTRY __GL_APINAME(EvalMesh1)(GLenum mode, GLint i1, GLint i2)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->EvalMesh1(gc, mode, i1, i2);
}

GLvoid GL_APIENTRY __GL_APINAME(EvalPoint1)(GLint i)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->EvalPoint1(gc, i);
}

GLvoid GL_APIENTRY __GL_APINAME(EvalMesh2)(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->EvalMesh2(gc, mode, i1, i2, j1, j2);
}

GLvoid GL_APIENTRY __GL_APINAME(EvalPoint2)(GLint i, GLint j)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->EvalPoint2(gc, i, j);
}

GLvoid GL_APIENTRY __GL_APINAME(AlphaFunc)(GLenum func, GLclampf ref)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->AlphaFunc(gc, func, ref);
}

GLvoid GL_APIENTRY __GL_APINAME(LogicOp)(GLenum opcode)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->LogicOp(gc, opcode);
}

GLvoid GL_APIENTRY __GL_APINAME(PixelZoom)(GLfloat xfactor, GLfloat yfactor)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->PixelZoom(gc, xfactor, yfactor);
}

GLvoid GL_APIENTRY __GL_APINAME(PixelTransferf)(GLenum pname, GLfloat param)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->PixelTransferf(gc, pname, param);
}

GLvoid GL_APIENTRY __GL_APINAME(PixelTransferi)(GLenum pname, GLint param)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->PixelTransferi(gc, pname, param);
}

GLvoid GL_APIENTRY __GL_APINAME(PixelStoref)(GLenum pname, GLfloat param)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->PixelStoref(gc, pname, param);
}

GLvoid GL_APIENTRY __GL_APINAME(PixelMapfv)(GLenum map, GLsizei mapsize, const GLfloat *values)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->PixelMapfv(gc, map, mapsize, values);
}

GLvoid GL_APIENTRY __GL_APINAME(PixelMapuiv)(GLenum map, GLsizei mapsize, const GLuint *values)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->PixelMapuiv(gc, map, mapsize, values);
}

GLvoid GL_APIENTRY __GL_APINAME(PixelMapusv)(GLenum map, GLsizei mapsize, const GLushort *values)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->PixelMapusv(gc, map, mapsize, values);
}

GLvoid GL_APIENTRY __GL_APINAME(CopyPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->CopyPixels(gc, x, y, width, height, type);
}

GLvoid GL_APIENTRY __GL_APINAME(DrawPixels)(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DrawPixels(gc, width, height, format, type, pixels);
}

GLvoid GL_APIENTRY __GL_APINAME(GetClipPlane)(GLenum plane, GLdouble *equation)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetClipPlane(gc, plane, equation);
}

GLvoid GL_APIENTRY __GL_APINAME(GetDoublev)(GLenum pname, GLdouble *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetDoublev(gc, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GetLightfv)(GLenum light, GLenum pname, GLfloat *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetLightfv(gc, light, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GetLightiv)(GLenum light, GLenum pname, GLint *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetLightiv(gc, light, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GetMapdv)(GLenum target, GLenum query, GLdouble *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetMapdv(gc, target, query, v);
}

GLvoid GL_APIENTRY __GL_APINAME(GetMapfv)(GLenum target, GLenum query, GLfloat *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetMapfv(gc, target, query, v);
}

GLvoid GL_APIENTRY __GL_APINAME(GetMapiv)(GLenum target, GLenum query, GLint *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetMapiv(gc, target, query, v);
}

GLvoid GL_APIENTRY __GL_APINAME(GetMaterialfv)(GLenum face, GLenum pname, GLfloat *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetMaterialfv(gc, face, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GetMaterialiv)(GLenum face, GLenum pname, GLint *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetMaterialiv(gc, face, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GetPixelMapfv)(GLenum map, GLfloat *values)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetPixelMapfv(gc, map, values);
}

GLvoid GL_APIENTRY __GL_APINAME(GetPixelMapuiv)(GLenum map, GLuint *values)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetPixelMapuiv(gc, map, values);
}

GLvoid GL_APIENTRY __GL_APINAME(GetPixelMapusv)(GLenum map, GLushort *values)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetPixelMapusv(gc, map, values);
}

GLvoid GL_APIENTRY __GL_APINAME(GetPolygonStipple)(GLubyte *mask)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetPolygonStipple(gc, mask);
}

GLvoid GL_APIENTRY __GL_APINAME(GetTexEnvfv)(GLenum target, GLenum pname, GLfloat *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetTexEnvfv(gc, target, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GetTexEnviv)(GLenum target, GLenum pname, GLint *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetTexEnviv(gc, target, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GetTexGendv)(GLenum coord, GLenum pname, GLdouble *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetTexGendv(gc, coord, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GetTexGenfv)(GLenum coord, GLenum pname, GLfloat *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetTexGenfv(gc, coord, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GetTexGeniv)(GLenum coord, GLenum pname, GLint *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetTexGeniv(gc, coord, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GetTexImage)(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetTexImage(gc, target, level, format, type, pixels);
}

GLboolean GL_APIENTRY __GL_APINAME(IsList)(GLuint list)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->pEntryDispatch->IsList(gc, list);
}

GLvoid GL_APIENTRY __GL_APINAME(DepthRange)(GLclampd near_val, GLclampd far_val)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DepthRange(gc, near_val, far_val);
}

GLvoid GL_APIENTRY __GL_APINAME(Frustum)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Frustum(gc, left, right, bottom, top, near_val, far_val);
}

GLvoid GL_APIENTRY __GL_APINAME(LoadIdentity)(GLvoid)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->LoadIdentity(gc);
}

GLvoid GL_APIENTRY __GL_APINAME(LoadMatrixf)(const GLfloat *m)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->LoadMatrixf(gc, m);
}

GLvoid GL_APIENTRY __GL_APINAME(LoadMatrixd)(const GLdouble *m)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->LoadMatrixd(gc, m);
}

GLvoid GL_APIENTRY __GL_APINAME(MatrixMode)(GLenum mode)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MatrixMode(gc, mode);
}

GLvoid GL_APIENTRY __GL_APINAME(MultMatrixf)(const GLfloat *m)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultMatrixf(gc, m);
}

GLvoid GL_APIENTRY __GL_APINAME(MultMatrixd)(const GLdouble *m)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultMatrixd(gc, m);
}

GLvoid GL_APIENTRY __GL_APINAME(Ortho)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Ortho(gc, left, right, bottom, top, near_val, far_val);
}

GLvoid GL_APIENTRY __GL_APINAME(PopMatrix)(GLvoid)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->PopMatrix(gc);
}

GLvoid GL_APIENTRY __GL_APINAME(PushMatrix)(GLvoid)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->PushMatrix(gc);
}

GLvoid GL_APIENTRY __GL_APINAME(Rotated)(GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Rotated(gc, angle, x, y, z);
}

GLvoid GL_APIENTRY __GL_APINAME(Rotatef)(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Rotatef(gc, angle, x, y, z);
}

GLvoid GL_APIENTRY __GL_APINAME(Scaled)(GLdouble x, GLdouble y, GLdouble z)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Scaled(gc, x, y, z);
}

GLvoid GL_APIENTRY __GL_APINAME(Scalef)(GLfloat x, GLfloat y, GLfloat z)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Scalef(gc, x, y, z);
}

GLvoid GL_APIENTRY __GL_APINAME(Translated)(GLdouble x, GLdouble y, GLdouble z)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Translated(gc, x, y, z);
}

GLvoid GL_APIENTRY __GL_APINAME(Translatef)(GLfloat x, GLfloat y, GLfloat z)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Translatef(gc, x, y, z);
}


/*
** GL_VERSION_1_1
*/
GLvoid GL_APIENTRY __GL_APINAME(ArrayElement)(GLint i)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ArrayElement(gc, i);
}

GLvoid GL_APIENTRY __GL_APINAME(ColorPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ColorPointer(gc, size, type, stride, ptr);
}

GLvoid GL_APIENTRY __GL_APINAME(DisableClientState)(GLenum cap)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DisableClientState(gc, cap);
}

GLvoid GL_APIENTRY __GL_APINAME(EdgeFlagPointer)(GLsizei stride, const GLvoid *ptr)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->EdgeFlagPointer(gc, stride, ptr);
}

GLvoid GL_APIENTRY __GL_APINAME(EnableClientState)(GLenum cap)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->EnableClientState(gc, cap);
}

GLvoid GL_APIENTRY __GL_APINAME(IndexPointer)(GLenum type, GLsizei stride, const GLvoid *ptr)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->IndexPointer(gc, type, stride, ptr);
}

GLvoid GL_APIENTRY __GL_APINAME(Indexub)(GLubyte c)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Indexub(gc, c);
}

GLvoid GL_APIENTRY __GL_APINAME(Indexubv)(const GLubyte *c)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Indexubv(gc, c);
}

GLvoid GL_APIENTRY __GL_APINAME(InterleavedArrays)(GLenum format, GLsizei stride, const GLvoid *pointer)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->InterleavedArrays(gc, format, stride, pointer);
}

GLvoid GL_APIENTRY __GL_APINAME(NormalPointer)(GLenum type, GLsizei stride, const GLvoid *ptr)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->NormalPointer(gc, type, stride, ptr);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoordPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoordPointer(gc, size, type, stride, ptr);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexPointer(gc, size, type, stride, ptr);
}

GLboolean GL_APIENTRY __GL_APINAME(AreTexturesResident)(GLsizei n, const GLuint *textures, GLboolean *residences)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->pEntryDispatch->AreTexturesResident(gc, n, textures, residences);
}

GLvoid GL_APIENTRY __GL_APINAME(CopyTexImage1D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->CopyTexImage1D(gc, target, level, internalformat, x, y, width, border);
}

GLvoid GL_APIENTRY __GL_APINAME(CopyTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->CopyTexSubImage1D(gc, target, level, xoffset, x, y, width);
}

GLvoid GL_APIENTRY __GL_APINAME(PrioritizeTextures)(GLsizei n, const GLuint *textures, const GLclampf *priorities)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->PrioritizeTextures(gc, n, textures, priorities);
}

GLvoid GL_APIENTRY __GL_APINAME(TexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexSubImage1D(gc, target, level, xoffset, width, format, type, pixels);
}

GLvoid GL_APIENTRY __GL_APINAME(PopClientAttrib)(GLvoid)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->PopClientAttrib(gc);
}

GLvoid GL_APIENTRY __GL_APINAME(PushClientAttrib)(GLbitfield mask)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->PushClientAttrib(gc, mask);
}

/*
** GL_VERSION_1_2
*/
GLvoid GL_APIENTRY __GL_APINAME(ActiveTextureARB)(GLenum texture)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ActiveTexture(gc, texture);
}

GLvoid GL_APIENTRY __GL_APINAME(ClientActiveTextureARB)(GLenum texture)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ClientActiveTexture(gc, texture);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord1dARB)(GLenum target, GLdouble s)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord1d(gc, target, s);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord1dvARB)(GLenum target, const GLdouble *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord1dv(gc, target, v);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord1fARB)(GLenum target, GLfloat s)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord1f(gc, target, s);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord1fvARB)(GLenum target, const GLfloat *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord1fv(gc, target, v);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord1iARB)(GLenum target, GLint s)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord1i(gc, target, s);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord1ivARB)(GLenum target, const GLint *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord1iv(gc, target, v);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord1sARB)(GLenum target, GLshort s)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord1s(gc, target, s);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord1svARB)(GLenum target, const GLshort *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord1sv(gc, target, v);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord2dARB)(GLenum target, GLdouble s, GLdouble t)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord2d(gc, target, s, t);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord2dvARB)(GLenum target, const GLdouble *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord2dv(gc, target, v);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord2fARB)(GLenum target, GLfloat s, GLfloat t)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord2f(gc, target, s, t);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord2fvARB)(GLenum target, const GLfloat *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord2fv(gc, target, v);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord2iARB)(GLenum target, GLint s, GLint t)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord2i(gc, target, s, t);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord2ivARB)(GLenum target, const GLint *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord2iv(gc, target, v);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord2sARB)(GLenum target, GLshort s, GLshort t)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord2s(gc, target, s, t);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord2svARB)(GLenum target, const GLshort *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord2sv(gc, target, v);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord3dARB)(GLenum target, GLdouble s, GLdouble t, GLdouble r)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord3d(gc, target, s, t, r);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord3dvARB)(GLenum target, const GLdouble *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord3dv(gc, target, v);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord3fARB)(GLenum target, GLfloat s, GLfloat t, GLfloat r)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord3f(gc, target, s, t, r);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord3fvARB)(GLenum target, const GLfloat *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord3fv(gc, target, v);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord3iARB)(GLenum target, GLint s, GLint t, GLint r)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord3i(gc, target, s, t, r);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord3ivARB)(GLenum target, const GLint *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord3iv(gc, target, v);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord3sARB)(GLenum target, GLshort s, GLshort t, GLshort r)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord3s(gc, target, s, t, r);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord3svARB)(GLenum target, const GLshort *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord3sv(gc, target, v);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord4dARB)(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord4d(gc, target, s, t, r, q);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord4dvARB)(GLenum target, const GLdouble *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord4dv(gc, target, v);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord4fARB)(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord4f(gc, target, s, t, r, q);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord4fvARB)(GLenum target, const GLfloat *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord4fv(gc, target, v);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord4iARB)(GLenum target, GLint s, GLint t, GLint r, GLint q)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord4i(gc, target, s, t, r, q);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord4ivARB)(GLenum target, const GLint *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord4iv(gc, target, v);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord4sARB)(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord4s(gc, target, s, t, r, q);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord4svARB)(GLenum target, const GLshort *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord4sv(gc, target, v);
}

/*
** GL_VERSION_1_3
*/
GLvoid GL_APIENTRY __GL_APINAME(CompressedTexImage1D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->CompressedTexImage1D(gc, target, level, internalformat, width, border, imageSize, data);
}

GLvoid GL_APIENTRY __GL_APINAME(CompressedTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->CompressedTexSubImage1D(gc, target, level, xoffset, width, format, imageSize, data);
}

GLvoid GL_APIENTRY __GL_APINAME(GetCompressedTexImage)(GLenum target, GLint lod, GLvoid *img)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetCompressedTexImage(gc, target, lod, img);
}

GLvoid GLAPIENTRY __GL_APINAME(ColorTable)( GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table ){}
GLvoid GLAPIENTRY __GL_APINAME(ColorTableParameteriv)(GLenum target, GLenum pname, const GLint *params){}
GLvoid GLAPIENTRY __GL_APINAME(ColorTableParameterfv)(GLenum target, GLenum pname, const GLfloat *params){}
GLvoid GLAPIENTRY __GL_APINAME(CopyColorTable)( GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width ){}
GLvoid GLAPIENTRY __GL_APINAME(GetColorTable)( GLenum target, GLenum format,  GLenum type, GLvoid *table ){}
GLvoid GLAPIENTRY __GL_APINAME(GetColorTableParameterfv)( GLenum target, GLenum pname, GLfloat *params ){}
GLvoid GLAPIENTRY __GL_APINAME(GetColorTableParameteriv)( GLenum target, GLenum pname,  GLint *params ){}
GLvoid GLAPIENTRY __GL_APINAME(ColorSubTable)( GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data ){}
GLvoid GLAPIENTRY __GL_APINAME(CopyColorSubTable)( GLenum target, GLsizei start, GLint x, GLint y, GLsizei width ){}
GLvoid GLAPIENTRY __GL_APINAME(ConvolutionFilter1D)( GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *image ){}
GLvoid GLAPIENTRY __GL_APINAME(ConvolutionFilter2D)( GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image ){}
GLvoid GLAPIENTRY __GL_APINAME(ConvolutionParameterf)( GLenum target, GLenum pname, GLfloat params ){}
GLvoid GLAPIENTRY __GL_APINAME(ConvolutionParameterfv)( GLenum target, GLenum pname, const GLfloat *params ){}
GLvoid GLAPIENTRY __GL_APINAME(ConvolutionParameteri)( GLenum target, GLenum pname, GLint params ){}
GLvoid GLAPIENTRY __GL_APINAME(ConvolutionParameteriv)( GLenum target, GLenum pname, const GLint *params ){}
GLvoid GLAPIENTRY __GL_APINAME(CopyConvolutionFilter1D)( GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width ){}
GLvoid GLAPIENTRY __GL_APINAME(CopyConvolutionFilter2D)( GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height){}
GLvoid GLAPIENTRY __GL_APINAME(GetConvolutionFilter)( GLenum target, GLenum format, GLenum type, GLvoid *image ){}
GLvoid GLAPIENTRY __GL_APINAME(GetConvolutionParameterfv)( GLenum target, GLenum pname, GLfloat *params ){}
GLvoid GLAPIENTRY __GL_APINAME(GetConvolutionParameteriv)( GLenum target, GLenum pname, GLint *params ){}
GLvoid GLAPIENTRY __GL_APINAME(SeparableFilter2D)( GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *row, const GLvoid *column ){}
GLvoid GLAPIENTRY __GL_APINAME(GetSeparableFilter)( GLenum target, GLenum format, GLenum type, GLvoid *row, GLvoid *column, GLvoid *span ){}
GLvoid GLAPIENTRY __GL_APINAME(Histogram)( GLenum target, GLsizei width, GLenum internalformat, GLboolean sink ){}
GLvoid GLAPIENTRY __GL_APINAME(ResetHistogram)( GLenum target ){}
GLvoid GLAPIENTRY __GL_APINAME(GetHistogram)( GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values ){}
GLvoid GLAPIENTRY __GL_APINAME(GetHistogramParameterfv)( GLenum target, GLenum pname, GLfloat *params ){}
GLvoid GLAPIENTRY __GL_APINAME(GetHistogramParameteriv)( GLenum target, GLenum pname, GLint *params ){}
GLvoid GLAPIENTRY __GL_APINAME(Minmax)( GLenum target, GLenum internalformat, GLboolean sink ){}
GLvoid GLAPIENTRY __GL_APINAME(ResetMinmax)( GLenum target ){}
GLvoid GLAPIENTRY __GL_APINAME(GetMinmax)( GLenum target, GLboolean reset, GLenum format, GLenum types, GLvoid *values ){}
GLvoid GLAPIENTRY __GL_APINAME(GetMinmaxParameterfv)( GLenum target, GLenum pname,  GLfloat *params ){}
GLvoid GLAPIENTRY __GL_APINAME(GetMinmaxParameteriv)( GLenum target, GLenum pname, GLint *params ){}

GLvoid GL_APIENTRY __GL_APINAME(ClientActiveTexture)(GLenum texture)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ClientActiveTexture(gc, texture);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord1d)(GLenum target, GLdouble s)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord1d(gc, target, s);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord1dv)(GLenum target, const GLdouble *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord1dv(gc, target, v);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord1f)(GLenum target, GLfloat s)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord1f(gc, target, s);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord1fv)(GLenum target, const GLfloat *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord1fv(gc, target, v);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord1i)(GLenum target, GLint s)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord1i(gc, target, s);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord1iv)(GLenum target, const GLint *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord1iv(gc, target, v);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord1s)(GLenum target, GLshort s)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord1s(gc, target, s);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord1sv)(GLenum target, const GLshort *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord1sv(gc, target, v);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord2d)(GLenum target, GLdouble s, GLdouble t)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord2d(gc, target, s, t);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord2dv)(GLenum target, const GLdouble *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord2dv(gc, target, v);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord2f)(GLenum target, GLfloat s, GLfloat t)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord2f(gc, target, s, t);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord2fv)(GLenum target, const GLfloat *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord2fv(gc, target, v);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord2i)(GLenum target, GLint s, GLint t)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord2i(gc, target, s, t);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord2iv)(GLenum target, const GLint *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord2iv(gc, target, v);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord2s)(GLenum target, GLshort s, GLshort t)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord2s(gc, target, s, t);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord2sv)(GLenum target, const GLshort *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord2sv(gc, target, v);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord3d)(GLenum target, GLdouble s, GLdouble t, GLdouble r)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord3d(gc, target, s, t, r);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord3dv)(GLenum target, const GLdouble *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord3dv(gc, target, v);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord3f)(GLenum target, GLfloat s, GLfloat t, GLfloat r)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord3f(gc, target, s, t, r);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord3fv)(GLenum target, const GLfloat *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord3fv(gc, target, v);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord3i)(GLenum target, GLint s, GLint t, GLint r)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord3i(gc, target, s, t, r);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord3iv)(GLenum target, const GLint *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord3iv(gc, target, v);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord3s)(GLenum target, GLshort s, GLshort t, GLshort r)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord3s(gc, target, s, t, r);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord3sv)(GLenum target, const GLshort *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord3sv(gc, target, v);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord4d)(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord4d(gc, target, s, t, r, q);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord4dv)(GLenum target, const GLdouble *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord4dv(gc, target, v);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord4f)(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord4f(gc, target, s, t, r, q);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord4fv)(GLenum target, const GLfloat *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord4fv(gc, target, v);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord4i)(GLenum target, GLint s, GLint t, GLint r, GLint q)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord4i(gc, target, s, t, r, q);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord4iv)(GLenum target, const GLint *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord4iv(gc, target, v);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord4s)(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord4s(gc, target, s, t, r, q);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoord4sv)(GLenum target, const GLshort *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoord4sv(gc, target, v);
}

GLvoid GL_APIENTRY __GL_APINAME(LoadTransposeMatrixf)(const GLfloat m[16])
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->LoadTransposeMatrixf(gc, m);
}

GLvoid GL_APIENTRY __GL_APINAME(LoadTransposeMatrixd)(const GLdouble m[16])
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->LoadTransposeMatrixd(gc, m);
}

GLvoid GL_APIENTRY __GL_APINAME(MultTransposeMatrixf)(const GLfloat m[16])
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultTransposeMatrixf(gc, m);
}

GLvoid GL_APIENTRY __GL_APINAME(MultTransposeMatrixd)(const GLdouble m[16])
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultTransposeMatrixd(gc, m);
}


/*
** GL_VERSION_1_4
*/
GLvoid GL_APIENTRY __GL_APINAME(PointParameterf)(GLenum pname, GLfloat param)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->PointParameterf(gc, pname, param);
}

GLvoid GL_APIENTRY __GL_APINAME(PointParameterfv)(GLenum pname, const GLfloat *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->PointParameterfv(gc, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(PointParameteri)(GLenum pname, GLint param)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->PointParameteri(gc, pname, param);
}

GLvoid GL_APIENTRY __GL_APINAME(PointParameteriv)(GLenum pname, const GLint *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->PointParameteriv(gc, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(FogCoordf)(GLfloat coord)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->FogCoordf(gc, coord);
}

GLvoid GL_APIENTRY __GL_APINAME(FogCoordfv)(const GLfloat *coord)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->FogCoordfv(gc, coord);
}

GLvoid GL_APIENTRY __GL_APINAME(FogCoordd)(GLdouble coord)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->FogCoordd(gc, coord);
}

GLvoid GL_APIENTRY __GL_APINAME(FogCoorddv)(const GLdouble *coord)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->FogCoorddv(gc, coord);
}

GLvoid GL_APIENTRY __GL_APINAME(FogCoordPointer)(GLenum type, GLsizei stride, const GLvoid *pointer)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->FogCoordPointer(gc, type, stride, pointer);
}

GLvoid GL_APIENTRY __GL_APINAME(SecondaryColor3b)(GLbyte red, GLbyte green, GLbyte blue)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->SecondaryColor3b(gc, red, green, blue);
}

GLvoid GL_APIENTRY __GL_APINAME(SecondaryColor3bv)(const GLbyte *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->SecondaryColor3bv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(SecondaryColor3d)(GLdouble red, GLdouble green, GLdouble blue)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->SecondaryColor3d(gc, red, green, blue);
}

GLvoid GL_APIENTRY __GL_APINAME(SecondaryColor3dv)(const GLdouble *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->SecondaryColor3dv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(SecondaryColor3f)(GLfloat red, GLfloat green, GLfloat blue)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->SecondaryColor3f(gc, red, green, blue);
}

GLvoid GL_APIENTRY __GL_APINAME(SecondaryColor3fv)(const GLfloat *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->SecondaryColor3fv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(SecondaryColor3i)(GLint red, GLint green, GLint blue)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->SecondaryColor3i(gc, red, green, blue);
}

GLvoid GL_APIENTRY __GL_APINAME(SecondaryColor3iv)(const GLint *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->SecondaryColor3iv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(SecondaryColor3s)(GLshort red, GLshort green, GLshort blue)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->SecondaryColor3s(gc, red, green, blue);
}

GLvoid GL_APIENTRY __GL_APINAME(SecondaryColor3sv)(const GLshort *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->SecondaryColor3sv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(SecondaryColor3ub)(GLubyte red, GLubyte green, GLubyte blue)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->SecondaryColor3ub(gc, red, green, blue);
}

GLvoid GL_APIENTRY __GL_APINAME(SecondaryColor3ubv)(const GLubyte *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->SecondaryColor3ubv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(SecondaryColor3ui)(GLuint red, GLuint green, GLuint blue)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->SecondaryColor3ui(gc, red, green, blue);
}

GLvoid GL_APIENTRY __GL_APINAME(SecondaryColor3uiv)(const GLuint *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->SecondaryColor3uiv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(SecondaryColor3us)(GLushort red, GLushort green, GLushort blue)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->SecondaryColor3us(gc, red, green, blue);
}

GLvoid GL_APIENTRY __GL_APINAME(SecondaryColor3usv)(const GLushort *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->SecondaryColor3usv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(SecondaryColorPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->SecondaryColorPointer(gc, size, type, stride, pointer);
}

GLvoid GL_APIENTRY __GL_APINAME(WindowPos2d)(GLdouble x, GLdouble y)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->WindowPos2d(gc, x, y);
}

GLvoid GL_APIENTRY __GL_APINAME(WindowPos2dv)(const GLdouble *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->WindowPos2dv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(WindowPos2f)(GLfloat x, GLfloat y)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->WindowPos2f(gc, x, y);
}

GLvoid GL_APIENTRY __GL_APINAME(WindowPos2fv)(const GLfloat *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->WindowPos2fv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(WindowPos2i)(GLint x, GLint y)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->WindowPos2i(gc, x, y);
}

GLvoid GL_APIENTRY __GL_APINAME(WindowPos2iv)(const GLint *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->WindowPos2iv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(WindowPos2s)(GLshort x, GLshort y)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->WindowPos2s(gc, x, y);
}

GLvoid GL_APIENTRY __GL_APINAME(WindowPos2sv)(const GLshort *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->WindowPos2sv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(WindowPos3d)(GLdouble x, GLdouble y, GLdouble z)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->WindowPos3d(gc, x, y, z);
}

GLvoid GL_APIENTRY __GL_APINAME(WindowPos3dv)(const GLdouble *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->WindowPos3dv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(WindowPos3f)(GLfloat x, GLfloat y, GLfloat z)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->WindowPos3f(gc, x, y, z);
}

GLvoid GL_APIENTRY __GL_APINAME(WindowPos3fv)(const GLfloat *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->WindowPos3fv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(WindowPos3i)(GLint x, GLint y, GLint z)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->WindowPos3i(gc, x, y, z);
}

GLvoid GL_APIENTRY __GL_APINAME(WindowPos3iv)(const GLint *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->WindowPos3iv(gc, v);
}

GLvoid GL_APIENTRY __GL_APINAME(WindowPos3s)(GLshort x, GLshort y, GLshort z)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->WindowPos3s(gc, x, y, z);
}

GLvoid GL_APIENTRY __GL_APINAME(WindowPos3sv)(const GLshort *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->WindowPos3sv(gc, v);
}


/*
** GL_VERSION_1_5
*/
GLvoid GL_APIENTRY __GL_APINAME(GetQueryObjectiv)(GLuint id, GLenum pname, GLint *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetQueryObjectiv(gc, id, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GetBufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, GLvoid *data)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetBufferSubData(gc, target, offset, size, data);
}


/*
** GL_VERSION_2_0
*/
GLvoid GL_APIENTRY __GL_APINAME(GetVertexAttribdv)(GLuint index, GLenum pname, GLdouble *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetVertexAttribdv(gc, index, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttrib1d)(GLuint index, GLdouble x)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttrib1d(gc, index, x);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttrib1dv)(GLuint index, const GLdouble *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttrib1dv(gc, index, v);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttrib1s)(GLuint index, GLshort x)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttrib1s(gc, index, x);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttrib1sv)(GLuint index, const GLshort *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttrib1sv(gc, index, v);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttrib2d)(GLuint index, GLdouble x, GLdouble y)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttrib2d(gc, index, x, y);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttrib2dv)(GLuint index, const GLdouble *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttrib2dv(gc, index, v);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttrib2s)(GLuint index, GLshort x, GLshort y)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttrib2s(gc, index, x, y);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttrib2sv)(GLuint index, const GLshort *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttrib2sv(gc, index, v);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttrib3d)(GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttrib3d(gc, index, x, y, z);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttrib3dv)(GLuint index, const GLdouble *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttrib3dv(gc, index, v);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttrib3s)(GLuint index, GLshort x, GLshort y, GLshort z)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttrib3s(gc, index, x, y, z);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttrib3sv)(GLuint index, const GLshort *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttrib3sv(gc, index, v);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttrib4Nbv)(GLuint index, const GLbyte *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttrib4Nbv(gc, index, v);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttrib4Niv)(GLuint index, const GLint *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttrib4Niv(gc, index, v);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttrib4Nsv)(GLuint index, const GLshort *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttrib4Nsv(gc, index, v);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttrib4Nub)(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttrib4Nub(gc, index, x, y, z, w);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttrib4Nubv)(GLuint index, const GLubyte *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttrib4Nubv(gc, index, v);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttrib4Nuiv)(GLuint index, const GLuint *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttrib4Nuiv(gc, index, v);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttrib4Nusv)(GLuint index, const GLushort *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttrib4Nusv(gc, index, v);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttrib4bv)(GLuint index, const GLbyte *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttrib4bv(gc, index, v);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttrib4d)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttrib4d(gc, index, x, y, z, w);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttrib4dv)(GLuint index, const GLdouble *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttrib4dv(gc, index, v);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttrib4iv)(GLuint index, const GLint *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttrib4iv(gc, index, v);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttrib4s)(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttrib4s(gc, index, x, y, z, w);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttrib4sv)(GLuint index, const GLshort *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttrib4sv(gc, index, v);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttrib4ubv)(GLuint index, const GLubyte *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttrib4ubv(gc, index, v);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttrib4uiv)(GLuint index, const GLuint *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttrib4uiv(gc, index, v);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttrib4usv)(GLuint index, const GLushort *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttrib4usv(gc, index, v);
}

/*
** GL_VERSION_2_1
*/

/*
** GL_VERSION_3_0
*/
GLvoid GL_APIENTRY __GL_APINAME(ClampColor)(GLenum target, GLenum clamp)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ClampColor(gc, target, clamp);
}

GLvoid GL_APIENTRY __GL_APINAME(BeginConditionalRender)(GLuint id, GLenum mode)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->BeginConditionalRender(gc, id, mode);
}

GLvoid GL_APIENTRY __GL_APINAME(EndConditionalRender)(GLvoid)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->EndConditionalRender(gc);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttribI1i)(GLuint index, GLint x)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttribI1i(gc, index, x);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttribI2i)(GLuint index, GLint x, GLint y)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttribI2i(gc, index, x, y);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttribI3i)(GLuint index, GLint x, GLint y, GLint z)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttribI3i(gc, index, x, y, z);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttribI1ui)(GLuint index, GLuint x)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttribI1ui(gc, index, x);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttribI2ui)(GLuint index, GLuint x, GLuint y)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttribI2ui(gc, index, x, y);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttribI3ui)(GLuint index, GLuint x, GLuint y, GLuint z)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttribI3ui(gc, index, x, y, z);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttribI1iv)(GLuint index, const GLint *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttribI1iv(gc, index, v);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttribI2iv)(GLuint index, const GLint *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttribI2iv(gc, index, v);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttribI3iv)(GLuint index, const GLint *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttribI3iv(gc, index, v);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttribI1uiv)(GLuint index, const GLuint *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttribI1uiv(gc, index, v);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttribI2uiv)(GLuint index, const GLuint *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttribI2uiv(gc, index, v);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttribI3uiv)(GLuint index, const GLuint *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttribI3uiv(gc, index, v);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttribI4bv)(GLuint index, const GLbyte *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttribI4bv(gc, index, v);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttribI4sv)(GLuint index, const GLshort *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttribI4sv(gc, index, v);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttribI4ubv)(GLuint index, const GLubyte *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttribI4ubv(gc, index, v);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttribI4usv)(GLuint index, const GLushort *v)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttribI4usv(gc, index, v);
}

GLvoid GL_APIENTRY __GL_APINAME(BindFragDataLocation)(GLuint program, GLuint color, const GLchar *name)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->BindFragDataLocation(gc, program, color, name);
}

GLvoid GL_APIENTRY __GL_APINAME(FramebufferTexture1D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->FramebufferTexture1D(gc, target, attachment, textarget, texture, level);
}

GLvoid GL_APIENTRY __GL_APINAME(FramebufferTexture3D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->FramebufferTexture3D(gc, target, attachment, textarget, texture, level, zoffset);
}

/*
** GL_VERSION_3_1
*/
GLvoid GL_APIENTRY __GL_APINAME(PrimitiveRestartIndex)(GLuint index)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->PrimitiveRestartIndex(gc, index);
}

GLvoid GL_APIENTRY __GL_APINAME(GetActiveUniformName)(GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformName)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetActiveUniformName(gc, program, uniformIndex, bufSize, length, uniformName);
}

/*
** GL_VERSION_3_2
*/
GLvoid GL_APIENTRY __GL_APINAME(ProvokingVertex)(GLenum mode)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ProvokingVertex(gc, mode);
}

GLvoid GL_APIENTRY __GL_APINAME(TexImage2DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexImage2DMultisample(gc, target, samples, internalformat, width, height, fixedsamplelocations);
}

GLvoid GL_APIENTRY __GL_APINAME(TexImage3DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexImage3DMultisample(gc, target, samples, internalformat, width, height, depth, fixedsamplelocations);
}

/*
** GL_VERSION_3_3
*/
GLvoid GL_APIENTRY __GL_APINAME(BindFragDataLocationIndexed)(GLuint program, GLuint colorNumber, GLuint index, const GLchar *name)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->BindFragDataLocationIndexed(gc, program, colorNumber, index, name);
}

GLint GL_APIENTRY __GL_APINAME(GetFragDataIndex)(GLuint program, const GLchar *name)
{
    __GL_GET_CONTEXT_RET(-1);
    return gc->pEntryDispatch->GetFragDataIndex(gc, program, name);
}

GLvoid GL_APIENTRY __GL_APINAME(QueryCounter)(GLuint id, GLenum target)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->QueryCounter(gc, id, target);
}

GLvoid GL_APIENTRY __GL_APINAME(GetQueryObjecti64v)(GLuint id, GLenum pname, GLint64 *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetQueryObjecti64v(gc, id, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GetQueryObjectui64v)(GLuint id, GLenum pname, GLuint64 *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetQueryObjectui64v(gc, id, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttribP1ui)(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttribP1ui(gc, index, type, normalized, value);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttribP1uiv)(GLuint index, GLenum type, GLboolean normalized, const GLuint *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttribP1uiv(gc, index, type, normalized, value);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttribP2ui)(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttribP2ui(gc, index, type, normalized, value);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttribP2uiv)(GLuint index, GLenum type, GLboolean normalized, const GLuint *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttribP2uiv(gc, index, type, normalized, value);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttribP3ui)(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttribP3ui(gc, index, type, normalized, value);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttribP3uiv)(GLuint index, GLenum type, GLboolean normalized, const GLuint *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttribP3uiv(gc, index, type, normalized, value);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttribP4ui)(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttribP4ui(gc, index, type, normalized, value);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexAttribP4uiv)(GLuint index, GLenum type, GLboolean normalized, const GLuint *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexAttribP4uiv(gc, index, type, normalized, value);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexP2ui)(GLenum type, GLuint value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexP2ui(gc, type, value);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexP2uiv)(GLenum type, const GLuint *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexP2uiv(gc, type, value);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexP3ui)(GLenum type, GLuint value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexP3ui(gc, type, value);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexP3uiv)(GLenum type, const GLuint *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexP3uiv(gc, type, value);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexP4ui)(GLenum type, GLuint value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexP4ui(gc, type, value);
}

GLvoid GL_APIENTRY __GL_APINAME(VertexP4uiv)(GLenum type, const GLuint *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->VertexP4uiv(gc, type, value);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoordP1ui)(GLenum type, GLuint coords)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoordP1ui(gc, type, coords);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoordP1uiv)(GLenum type, const GLuint *coords)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoordP1uiv(gc, type, coords);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoordP2ui)(GLenum type, GLuint coords)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoordP2ui(gc, type, coords);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoordP2uiv)(GLenum type, const GLuint *coords)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoordP2uiv(gc, type, coords);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoordP3ui)(GLenum type, GLuint coords)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoordP3ui(gc, type, coords);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoordP3uiv)(GLenum type, const GLuint *coords)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoordP3uiv(gc, type, coords);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoordP4ui)(GLenum type, GLuint coords)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoordP4ui(gc, type, coords);
}

GLvoid GL_APIENTRY __GL_APINAME(TexCoordP4uiv)(GLenum type, const GLuint *coords)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexCoordP4uiv(gc, type, coords);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoordP1ui)(GLenum texture, GLenum type, GLuint coords)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoordP1ui(gc, texture, type, coords);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoordP1uiv)(GLenum texture, GLenum type, const GLuint *coords)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoordP1uiv(gc, texture, type, coords);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoordP2ui)(GLenum texture, GLenum type, GLuint coords)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoordP2ui(gc, texture, type, coords);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoordP2uiv)(GLenum texture, GLenum type, const GLuint *coords)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoordP2uiv(gc, texture, type, coords);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoordP3ui)(GLenum texture, GLenum type, GLuint coords)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoordP3ui(gc, texture, type, coords);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoordP3uiv)(GLenum texture, GLenum type, const GLuint *coords)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoordP3uiv(gc, texture, type, coords);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoordP4ui)(GLenum texture, GLenum type, GLuint coords)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoordP4ui(gc, texture, type, coords);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiTexCoordP4uiv)(GLenum texture, GLenum type, const GLuint *coords)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiTexCoordP4uiv(gc, texture, type, coords);
}

GLvoid GL_APIENTRY __GL_APINAME(NormalP3ui)(GLenum type, GLuint coords)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->NormalP3ui(gc, type, coords);
}

GLvoid GL_APIENTRY __GL_APINAME(NormalP3uiv)(GLenum type, const GLuint *coords)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->NormalP3uiv(gc, type, coords);
}

GLvoid GL_APIENTRY __GL_APINAME(ColorP3ui)(GLenum type, GLuint color)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ColorP3ui(gc, type, color);
}

GLvoid GL_APIENTRY __GL_APINAME(ColorP3uiv)(GLenum type, const GLuint *color)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ColorP3uiv(gc, type, color);
}

GLvoid GL_APIENTRY __GL_APINAME(ColorP4ui)(GLenum type, GLuint color)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ColorP4ui(gc, type, color);
}

GLvoid GL_APIENTRY __GL_APINAME(ColorP4uiv)(GLenum type, const GLuint *color)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ColorP4uiv(gc, type, color);
}

GLvoid GL_APIENTRY __GL_APINAME(SecondaryColorP3ui)(GLenum type, GLuint color)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->SecondaryColorP3ui(gc, type, color);
}

GLvoid GL_APIENTRY __GL_APINAME(SecondaryColorP3uiv)(GLenum type, const GLuint *color)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->SecondaryColorP3uiv(gc, type, color);
}

/*
** GL_VERSION_4_0
*/
GLvoid GL_APIENTRY __GL_APINAME(Uniform1d)(GLint location, GLdouble x)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Uniform1d(gc, location, x);
}

GLvoid GL_APIENTRY __GL_APINAME(Uniform2d)(GLint location, GLdouble x, GLdouble y)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Uniform2d(gc, location, x, y);
}

GLvoid GL_APIENTRY __GL_APINAME(Uniform3d)(GLint location, GLdouble x, GLdouble y, GLdouble z)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Uniform3d(gc, location, x, y, z);
}

GLvoid GL_APIENTRY __GL_APINAME(Uniform4d)(GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Uniform4d(gc, location, x, y, z, w);
}

GLvoid GL_APIENTRY __GL_APINAME(Uniform1dv)(GLint location, GLsizei count, const GLdouble *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Uniform1dv(gc, location, count, value);
}

GLvoid GL_APIENTRY __GL_APINAME(Uniform2dv)(GLint location, GLsizei count, const GLdouble *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Uniform2dv(gc, location, count, value);
}

GLvoid GL_APIENTRY __GL_APINAME(Uniform3dv)(GLint location, GLsizei count, const GLdouble *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Uniform3dv(gc, location, count, value);
}

GLvoid GL_APIENTRY __GL_APINAME(Uniform4dv)(GLint location, GLsizei count, const GLdouble *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->Uniform4dv(gc, location, count, value);
}

GLvoid GL_APIENTRY __GL_APINAME(UniformMatrix2dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->UniformMatrix2dv(gc, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GL_APINAME(UniformMatrix3dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->UniformMatrix3dv(gc, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GL_APINAME(UniformMatrix4dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->UniformMatrix4dv(gc, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GL_APINAME(UniformMatrix2x3dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->UniformMatrix2x3dv(gc, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GL_APINAME(UniformMatrix2x4dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->UniformMatrix2x4dv(gc, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GL_APINAME(UniformMatrix3x2dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->UniformMatrix3x2dv(gc, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GL_APINAME(UniformMatrix3x4dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->UniformMatrix3x4dv(gc, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GL_APINAME(UniformMatrix4x2dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->UniformMatrix4x2dv(gc, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GL_APINAME(UniformMatrix4x3dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->UniformMatrix4x3dv(gc, location, count, transpose, value);
}

GLvoid GL_APIENTRY __GL_APINAME(GetUniformdv)(GLuint program, GLint location, GLdouble *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetUniformdv(gc, program, location, params);
}

GLint GL_APIENTRY __GL_APINAME(GetSubroutineUniformLocation)(GLuint program, GLenum shadertype, const GLchar *name)
{
    __GL_GET_CONTEXT_RET(-1);
    return gc->pEntryDispatch->GetSubroutineUniformLocation(gc, program, shadertype, name);
}

GLuint GL_APIENTRY __GL_APINAME(GetSubroutineIndex)(GLuint program, GLenum shadertype, const GLchar *name)
{
    __GL_GET_CONTEXT_RET(0);
    return gc->pEntryDispatch->GetSubroutineIndex(gc, program, shadertype, name);
}

GLvoid GL_APIENTRY __GL_APINAME(GetActiveSubroutineUniformiv)(GLuint program, GLenum shadertype, GLuint index, GLenum pname, GLint *values)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetActiveSubroutineUniformiv(gc, program, shadertype, index, pname, values);
}

GLvoid GL_APIENTRY __GL_APINAME(GetActiveSubroutineUniformName)(GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei *length, GLchar *name)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetActiveSubroutineUniformName(gc, program, shadertype, index, bufsize, length, name);
}

GLvoid GL_APIENTRY __GL_APINAME(GetActiveSubroutineName)(GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei *length, GLchar *name)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetActiveSubroutineName(gc, program, shadertype, index, bufsize, length, name);
}

GLvoid GL_APIENTRY __GL_APINAME(UniformSubroutinesuiv)(GLenum shadertype, GLsizei count, const GLuint *indices)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->UniformSubroutinesuiv(gc, shadertype, count, indices);
}

GLvoid GL_APIENTRY __GL_APINAME(GetUniformSubroutineuiv)(GLenum shadertype, GLint location, GLuint *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetUniformSubroutineuiv(gc, shadertype, location, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GetProgramStageiv)(GLuint program, GLenum shadertype, GLenum pname, GLint *values)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetProgramStageiv(gc, program, shadertype, pname, values);
}

GLvoid GL_APIENTRY __GL_APINAME(PatchParameterfv)(GLenum pname, const GLfloat *values)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->PatchParameterfv(gc, pname, values);
}

GLvoid GL_APIENTRY __GL_APINAME(DrawTransformFeedback)(GLenum mode, GLuint id)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DrawTransformFeedback(gc, mode, id);
}

GLvoid GL_APIENTRY __GL_APINAME(DrawTransformFeedbackStream)(GLenum mode, GLuint id, GLuint stream)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DrawTransformFeedbackStream(gc, mode, id, stream);
}

GLvoid GL_APIENTRY __GL_APINAME(BeginQueryIndexed)(GLenum target, GLuint index, GLuint id)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->BeginQueryIndexed(gc, target, index, id);
}

GLvoid GL_APIENTRY __GL_APINAME(EndQueryIndexed)(GLenum target, GLuint index)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->EndQueryIndexed(gc, target, index);
}

GLvoid GL_APIENTRY __GL_APINAME(GetQueryIndexediv)(GLenum target, GLuint index, GLenum pname, GLint *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetQueryIndexediv(gc, target, index, pname, params);
}


/*********************************************
**
** OpenGL only Extensions
**
*********************************************/

/*
** GL_ARB_shader_objects
*/
GLvoid GL_APIENTRY __GL_APINAME(DeleteObjectARB)(GLhandleARB obj)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DeleteObjectARB(gc, obj);
}

GLvoid GL_APIENTRY __GL_APINAME(GetObjectParameterivARB)(GLhandleARB obj, GLenum pname, GLint *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetObjectParameterivARB(gc, obj, pname, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GetInfoLogARB)(GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetInfoLogARB(gc, obj, maxLength, length, infoLog);
}
#endif


/*
** OpenGL ES Extensions
*/

#if GL_OES_EGL_image
GLvoid GL_APIENTRY __GL_APINAME(EGLImageTargetTexture2DOES)(GLenum target, GLeglImageOES image)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->EGLImageTargetTexture2DOES(gc, target, image);
}

GLvoid GL_APIENTRY __GL_APINAME(EGLImageTargetRenderbufferStorageOES)(GLenum target, GLeglImageOES image)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->EGLImageTargetRenderbufferStorageOES(gc, target, image);
}
#endif

#if GL_VIV_direct_texture
GLvoid GL_APIENTRY __GL_APINAME(TexDirectVIV)(GLenum target, GLsizei width, GLsizei height, GLenum format, GLvoid ** pixels)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexDirectVIV(gc, target, width, height, format, pixels);
}

GLvoid GL_APIENTRY __GL_APINAME(TexDirectInvalidateVIV)(GLenum target)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexDirectInvalidateVIV(gc, target);
}

GLvoid GL_APIENTRY __GL_APINAME(TexDirectVIVMap)(GLenum target, GLsizei width, GLsizei height, GLenum format, GLvoid ** logical, const GLuint * physical)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexDirectVIVMap(gc, target, width, height, format, logical, physical);
}

GLvoid GL_APIENTRY __GL_APINAME(TexDirectTiledMapVIV)(GLenum target, GLsizei width, GLsizei height, GLenum format, GLvoid ** logical, const GLuint * physical)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->TexDirectTiledMapVIV(gc, target, width, height, format, logical, physical);
}
#endif

#if GL_EXT_discard_framebuffer
GLvoid GL_APIENTRY __GL_APINAME(DiscardFramebufferEXT)(GLenum target, GLsizei numAttachments, const GLenum *attachments)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DiscardFramebufferEXT(gc, target, numAttachments, attachments);
}
#endif

#if GL_EXT_multisampled_render_to_texture
GLvoid GL_APIENTRY __GL_APINAME(RenderbufferStorageMultisampleEXT)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->RenderbufferStorageMultisample(gc, target, samples, internalformat, width, height);
}

GLvoid GL_APIENTRY __GL_APINAME(FramebufferTexture2DMultisampleEXT)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->FramebufferTexture2DMultisampleEXT(gc, target, attachment, textarget, texture, level, samples);
}
#endif

#if GL_OES_mapbuffer
GLvoid* GL_APIENTRY __GL_APINAME(MapBufferOES)(GLenum target, GLenum access)
{
    __GL_GET_CONTEXT_RET(gcvNULL);
    return gc->pEntryDispatch->MapBuffer(gc, target, access);
}

GLboolean GL_APIENTRY __GL_APINAME(UnmapBufferOES)(GLenum target)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->pEntryDispatch->UnmapBuffer(gc, target);
}

GLvoid GL_APIENTRY __GL_APINAME(GetBufferPointervOES)(GLenum target, GLenum pname, GLvoid** params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetBufferPointerv(gc, target, pname, params);
}
#endif

#if GL_EXT_multi_draw_arrays
GLvoid GL_APIENTRY __GL_APINAME(MultiDrawArraysEXT)(GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiDrawArrays(gc, mode, first, count, primcount);
}

GLvoid GL_APIENTRY __GL_APINAME(MultiDrawElementsEXT)(GLenum mode, const GLsizei *count, GLenum type, const GLvoid*const*indices, GLsizei primcount)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->MultiDrawElements(gc, mode, count, type, indices, primcount);
}
#endif

#if GL_OES_get_program_binary
GLvoid GL_APIENTRY __GL_APINAME(GetProgramBinaryOES)(GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, GLvoid* binary)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetProgramBinary(gc, program, bufSize, length, binaryFormat, binary);
}

GLvoid GL_APIENTRY __GL_APINAME(ProgramBinaryOES)(GLuint program, GLenum binaryFormat, const GLvoid* binary, GLsizei length)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ProgramBinary(gc, program, binaryFormat, binary, length);
}
#endif

#if GL_OES_vertex_array_object
GLvoid GL_APIENTRY __GL_APINAME(BindVertexArrayOES)(GLuint array)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->BindVertexArray(gc, array);
}

GLvoid GL_APIENTRY __GL_APINAME(DeleteVertexArraysOES)(GLsizei n, const GLuint *arrays)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->DeleteVertexArrays(gc, n , arrays);
}

GLvoid GL_APIENTRY __GL_APINAME(GenVertexArraysOES)(GLsizei n, GLuint *arrays)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GenVertexArrays(gc, n , arrays);
}

GLboolean GL_APIENTRY __GL_APINAME(IsVertexArrayOES)(GLuint array)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->pEntryDispatch->IsVertexArray(gc, array);
}
#endif

#if GL_KHR_blend_equation_advanced
GLvoid GL_APIENTRY __GL_APINAME(BlendBarrierKHR)(void)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->BlendBarrier(gc);
}
#endif

#if GL_EXT_robustness
GLenum GL_APIENTRY __GL_APINAME(GetGraphicsResetStatusEXT)()
{
    __GL_GET_CONTEXT_RET(GL_NO_ERROR);
    return gc->pEntryDispatch->GetGraphicsResetStatus(gc);
}

GLvoid GL_APIENTRY __GL_APINAME(ReadnPixelsEXT)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, GLvoid *data)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->ReadnPixels(gc, x, y, width, height, format, type, bufSize, data);
}

GLvoid GL_APIENTRY __GL_APINAME(GetnUniformfvEXT)(GLuint program, GLint location, GLsizei bufSize, GLfloat *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetnUniformfv(gc, program, location, bufSize, params);
}

GLvoid GL_APIENTRY __GL_APINAME(GetnUniformivEXT)(GLuint program, GLint location, GLsizei bufSize, GLint *params)
{
    __GL_GET_CONTEXT;
    gc->pEntryDispatch->GetnUniformiv(gc, program, location, bufSize, params);
}
#endif



