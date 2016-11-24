#include "gc_es_context.h"

/* All next APIs are only used to set _GLdispathTable(glVVT_DispatchTable)
** g_api_entry.c is used to export gl Functions.
** Some users write the GL AP as the next:
** PFNGLGENBUFFERSPROC    glGenBuffers    = NULL;
** glGenBuffers = (PFNGLGENBUFFERSPROC)glXGetProcAddress("glGenBuffers");
** call glGenBuffers(...) somewhere.
** When Ap loads libGL.so, glVVT_DispatchTable->GenBuffers will be set
** as &glGenBuffers(the variant defined in Ap) or glGenBuffers(defined in
** libGL.so), if it is set as the address of variant, it will failed to run
** Ap cause glXGetProcAddress returns the address of variant(glGenBuffers).
** So glVVT_DispatchTable is set by these _vvvvvvv_gl##func to avoid this
** issue.
** For the Ap developers, they'd better not name the same as what libGL.so exports. Some benchmarks already did this and we have to support.
** Perhaps better solution, Fix me??
*/

#define __GLES_APINAME(apiname) _vvvvvvv_gl##apiname

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

GLvoid GLAPIENTRY __GLES_APINAME(ActiveTexture)(GLenum texture)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ActiveTexture(gc, texture);
}

GLvoid GLAPIENTRY __GLES_APINAME(AttachShader)(GLuint program, GLuint shader)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.AttachShader(gc, program, shader);
}

GLvoid GLAPIENTRY __GLES_APINAME(BindAttribLocation)(GLuint program, GLuint index, const GLchar* name)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BindAttribLocation(gc, program, index, name);
}

GLvoid GLAPIENTRY __GLES_APINAME(BindBuffer)(GLenum target, GLuint buffer)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BindBuffer(gc, target, buffer);
    __GL_PATTERN_MATCH_NAME(__glesApiEnum(BindBuffer));
}

GLvoid GLAPIENTRY __GLES_APINAME(BindFramebuffer)(GLenum target, GLuint framebuffer)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BindFramebuffer(gc, target, framebuffer);
}

GLvoid GLAPIENTRY __GLES_APINAME(BindRenderbuffer)(GLenum target, GLuint renderbuffer)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BindRenderbuffer(gc, target, renderbuffer);
}

GLvoid GLAPIENTRY __GLES_APINAME(BindTexture)(GLenum target, GLuint texture)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BindTexture(gc, target, texture);
}

GLvoid GLAPIENTRY __GLES_APINAME(BlendColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BlendColor(gc, red, green, blue, alpha);
    __GL_PATTERN_MATCH_NAME(__glesApiEnum(BlendColor));
}

GLvoid GLAPIENTRY __GLES_APINAME(BlendEquation)(GLenum mode)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BlendEquation(gc, mode);
}

GLvoid GLAPIENTRY __GLES_APINAME(BlendEquationSeparate)(GLenum modeRGB, GLenum modeAlpha)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BlendEquationSeparate(gc, modeRGB, modeAlpha);
}

GLvoid GLAPIENTRY __GLES_APINAME(BlendFunc)(GLenum sfactor, GLenum dfactor)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BlendFunc(gc, sfactor, dfactor);
    __GL_PATTERN_MATCH_NAME(__glesApiEnum(BlendFunc));
}

GLvoid GLAPIENTRY __GLES_APINAME(BlendFuncSeparate)(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BlendFuncSeparate(gc, srcRGB, dstRGB, srcAlpha, dstAlpha);
}

GLvoid GLAPIENTRY __GLES_APINAME(BufferData)(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BufferData(gc, target, size, data, usage);
}

GLvoid GLAPIENTRY __GLES_APINAME(BufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BufferSubData(gc, target, offset, size, data);
}

GLenum GLAPIENTRY __GLES_APINAME(CheckFramebufferStatus)(GLenum target)
{
    __GL_GET_CONTEXT_RET(0);
    return gc->apiDispatchTable.CheckFramebufferStatus(gc, target);
}

GLvoid GLAPIENTRY __GLES_APINAME(Clear)(GLbitfield mask)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Clear(gc, mask);
}

GLvoid GLAPIENTRY __GLES_APINAME(ClearColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ClearColor(gc, red, green, blue, alpha);
}

GLvoid GLAPIENTRY __GLES_APINAME(ClearDepthf)(GLfloat depth)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ClearDepthf(gc, depth);
}

GLvoid GLAPIENTRY __GLES_APINAME(ClearStencil)(GLint s)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ClearStencil(gc, s);
}

GLvoid GLAPIENTRY __GLES_APINAME(ColorMask)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ColorMask(gc, red, green, blue, alpha);
}

GLvoid GLAPIENTRY __GLES_APINAME(CompileShader)(GLuint shader)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.CompileShader(gc, shader);
}

GLvoid GLAPIENTRY __GLES_APINAME(CompressedTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.CompressedTexImage2D(gc, target, level, internalformat, width, height, border, imageSize, data);
}

GLvoid GLAPIENTRY __GLES_APINAME(CompressedTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid* data)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.CompressedTexSubImage2D(gc, target, level, xoffset, yoffset, width, height, format, imageSize, data);
}

GLvoid GLAPIENTRY __GLES_APINAME(CopyTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.CopyTexImage2D(gc, target, level, internalformat, x, y, width, height, border);
}

GLvoid GLAPIENTRY __GLES_APINAME(CopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.CopyTexSubImage2D(gc, target, level, xoffset, yoffset, x, y, width, height);
}

GLuint GLAPIENTRY __GLES_APINAME(CreateProgram)(GLvoid)
{
    __GL_GET_CONTEXT_RET(0);
    return gc->apiDispatchTable.CreateProgram(gc);
}

GLuint GLAPIENTRY __GLES_APINAME(CreateShader)(GLenum type)
{
    __GL_GET_CONTEXT_RET(0);
    return gc->apiDispatchTable.CreateShader(gc, type);
}

GLvoid GLAPIENTRY __GLES_APINAME(CullFace)(GLenum mode)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.CullFace(gc, mode);
}

GLvoid GLAPIENTRY __GLES_APINAME(DeleteBuffers)(GLsizei n, const GLuint* buffers)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DeleteBuffers(gc, n, buffers);
}

GLvoid GLAPIENTRY __GLES_APINAME(DeleteFramebuffers)(GLsizei n, const GLuint* framebuffers)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DeleteFramebuffers(gc, n, framebuffers);
}

GLvoid GLAPIENTRY __GLES_APINAME(DeleteProgram)(GLuint program)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DeleteProgram(gc, program);
}

GLvoid GLAPIENTRY __GLES_APINAME(DeleteRenderbuffers)(GLsizei n, const GLuint* renderbuffers)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DeleteRenderbuffers(gc, n, renderbuffers);
}

GLvoid GLAPIENTRY __GLES_APINAME(DeleteShader)(GLuint shader)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DeleteShader(gc, shader);
}

GLvoid GLAPIENTRY __GLES_APINAME(DeleteTextures)(GLsizei n, const GLuint* textures)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DeleteTextures(gc, n, textures);
}

GLvoid GLAPIENTRY __GLES_APINAME(DepthFunc)(GLenum func)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DepthFunc(gc, func);
    __GL_PATTERN_MATCH_NAME(__glesApiEnum(DepthFunc));
}

GLvoid GLAPIENTRY __GLES_APINAME(DepthMask)(GLboolean flag)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DepthMask(gc, flag);
}

GLvoid GLAPIENTRY __GLES_APINAME(DepthRangef)(GLfloat n, GLfloat f)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DepthRangef(gc, n, f);
}

GLvoid GLAPIENTRY __GLES_APINAME(DetachShader)(GLuint program, GLuint shader)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DetachShader(gc, program, shader);
}

GLvoid GLAPIENTRY __GLES_APINAME(Disable)(GLenum cap)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Disable(gc, cap);
    __GL_PATTERN_MATCH_NAME_WITH_ARG(__glesApiEnum(Disable), cap);
}

GLvoid GLAPIENTRY __GLES_APINAME(DisableVertexAttribArray)(GLuint index)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DisableVertexAttribArray(gc, index);
}

GLvoid GLAPIENTRY __GLES_APINAME(DrawArrays)(GLenum mode, GLint first, GLsizei count)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DrawArrays(gc, mode, first, count);
}

GLvoid GLAPIENTRY __GLES_APINAME(DrawElements)(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices)
{
    __GL_GET_CONTEXT;
    __GL_PATTERN_MATCH_NAME_WITH_ARG2(__glesApiEnum(DrawElements),mode, type);
    gc->apiDispatchTable.DrawElements(gc, mode, count, type, indices);
}

GLvoid GLAPIENTRY __GLES_APINAME(Enable)(GLenum cap)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Enable(gc, cap);
    __GL_PATTERN_MATCH_NAME_WITH_ARG(__glesApiEnum(Enable),cap);
}

GLvoid GLAPIENTRY __GLES_APINAME(EnableVertexAttribArray)(GLuint index)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.EnableVertexAttribArray(gc, index);
}

GLvoid GLAPIENTRY __GLES_APINAME(Finish)(GLvoid)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Finish(gc);
}

GLvoid GLAPIENTRY __GLES_APINAME(Flush)(GLvoid)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Flush(gc);
}

GLvoid GLAPIENTRY __GLES_APINAME(FramebufferRenderbuffer)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.FramebufferRenderbuffer(gc, target, attachment, renderbuffertarget, renderbuffer);
}

GLvoid GLAPIENTRY __GLES_APINAME(FramebufferTexture2D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.FramebufferTexture2D(gc, target, attachment, textarget, texture, level);
}

GLvoid GLAPIENTRY __GLES_APINAME(FrontFace)(GLenum mode)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.FrontFace(gc, mode);
}

GLvoid GLAPIENTRY __GLES_APINAME(GenBuffers)(GLsizei n, GLuint* buffers)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GenBuffers(gc, n, buffers);
}

GLvoid GLAPIENTRY __GLES_APINAME(GenerateMipmap)(GLenum target)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GenerateMipmap(gc, target);
}

GLvoid GLAPIENTRY __GLES_APINAME(GenFramebuffers)(GLsizei n, GLuint* framebuffers)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GenFramebuffers(gc, n, framebuffers);
}

GLvoid GLAPIENTRY __GLES_APINAME(GenRenderbuffers)(GLsizei n, GLuint* renderbuffers)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GenRenderbuffers(gc, n, renderbuffers);
}

GLvoid GLAPIENTRY __GLES_APINAME(GenTextures)(GLsizei n, GLuint* textures)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GenTextures(gc, n, textures);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetActiveAttrib)(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetActiveAttrib(gc, program, index, bufsize, length, size, type, name);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetActiveUniform)(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetActiveUniform(gc, program, index, bufsize, length, size, type, name);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetAttachedShaders)(GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetAttachedShaders(gc, program, maxcount, count, shaders);
}

GLint GLAPIENTRY __GLES_APINAME(GetAttribLocation)(GLuint program, const GLchar* name)
{
    __GL_GET_CONTEXT_RET(GL_INVALID_OPERATION);
    return gc->apiDispatchTable.GetAttribLocation(gc, program, name);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetBooleanv)(GLenum pname, GLboolean* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetBooleanv(gc, pname, params);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetBufferParameteriv)(GLenum target, GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetBufferParameteriv(gc, target, pname, params);
}

GLenum GLAPIENTRY __GLES_APINAME(GetError)(GLvoid)
{
    __GL_GET_CONTEXT_RET(GL_NO_ERROR);
    return gc->apiDispatchTable.GetError(gc);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetFloatv)(GLenum pname, GLfloat* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetFloatv(gc, pname, params);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetFramebufferAttachmentParameteriv)(GLenum target, GLenum attachment, GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetFramebufferAttachmentParameteriv(gc, target, attachment, pname, params);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetIntegerv)(GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetIntegerv(gc, pname, params);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetProgramiv)(GLuint program, GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetProgramiv(gc, program, pname, params);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetProgramInfoLog)(GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetProgramInfoLog(gc, program, bufsize, length, infolog);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetRenderbufferParameteriv)(GLenum target, GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetRenderbufferParameteriv(gc, target, pname, params);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetShaderiv)(GLuint shader, GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetShaderiv(gc, shader, pname, params);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetShaderInfoLog)(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetShaderInfoLog(gc, shader, bufsize, length, infolog);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetShaderPrecisionFormat)(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetShaderPrecisionFormat(gc, shadertype, precisiontype, range, precision);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetShaderSource)(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetShaderSource(gc, shader, bufsize, length, source);
}

const GLubyte* GLAPIENTRY __GLES_APINAME(GetString)(GLenum name)
{
    __GLcontext *gc = (__GLcontext *)_glapi_get_context();

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

GLvoid GLAPIENTRY __GLES_APINAME(GetTexParameterfv)(GLenum target, GLenum pname, GLfloat* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetTexParameterfv(gc, target, pname, params);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetTexParameteriv)(GLenum target, GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetTexParameteriv(gc, target, pname, params);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetUniformfv)(GLuint program, GLint location, GLfloat* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetUniformfv(gc, program, location, params);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetUniformiv)(GLuint program, GLint location, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetUniformiv(gc, program, location, params);
}

GLint GLAPIENTRY __GLES_APINAME(GetUniformLocation)(GLuint program, const GLchar* name)
{
    __GL_GET_CONTEXT_RET(GL_INVALID_OPERATION);
    return gc->apiDispatchTable.GetUniformLocation(gc, program, name);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetVertexAttribfv)(GLuint index, GLenum pname, GLfloat* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetVertexAttribfv(gc, index, pname, params);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetVertexAttribiv)(GLuint index, GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetVertexAttribiv(gc, index, pname, params);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetVertexAttribPointerv)(GLuint index, GLenum pname, GLvoid** pointer)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetVertexAttribPointerv(gc, index, pname, pointer);
}

GLvoid GLAPIENTRY __GLES_APINAME(Hint)(GLenum target, GLenum mode)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Hint(gc, target, mode);
}

GLboolean GLAPIENTRY __GLES_APINAME(IsBuffer)(GLuint buffer)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->apiDispatchTable.IsBuffer(gc, buffer);
}

GLboolean GLAPIENTRY __GLES_APINAME(IsEnabled)(GLenum cap)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->apiDispatchTable.IsEnabled(gc, cap);
}

GLboolean GLAPIENTRY __GLES_APINAME(IsFramebuffer)(GLuint framebuffer)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->apiDispatchTable.IsFramebuffer(gc, framebuffer);
}

GLboolean GLAPIENTRY __GLES_APINAME(IsProgram)(GLuint program)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->apiDispatchTable.IsProgram(gc, program);
}

GLboolean GLAPIENTRY __GLES_APINAME(IsRenderbuffer)(GLuint renderbuffer)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->apiDispatchTable.IsRenderbuffer(gc, renderbuffer);
}

GLboolean GLAPIENTRY __GLES_APINAME(IsShader)(GLuint shader)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->apiDispatchTable.IsShader(gc, shader);
}

GLboolean GLAPIENTRY __GLES_APINAME(IsTexture)(GLuint texture)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->apiDispatchTable.IsTexture(gc, texture);
}

GLvoid GLAPIENTRY __GLES_APINAME(LineWidth)(GLfloat width)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.LineWidth(gc, width);
}

GLvoid GLAPIENTRY __GLES_APINAME(LinkProgram)(GLuint program)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.LinkProgram(gc, program);
}

GLvoid GLAPIENTRY __GLES_APINAME(PixelStorei)(GLenum pname, GLint param)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.PixelStorei(gc, pname, param);
}

GLvoid GLAPIENTRY __GLES_APINAME(PolygonOffset)(GLfloat factor, GLfloat units)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.PolygonOffset(gc, factor, units);
}

GLvoid GLAPIENTRY __GLES_APINAME(ReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ReadPixels(gc, x, y, width, height, format, type, pixels);
}

GLvoid GLAPIENTRY __GLES_APINAME(ReleaseShaderCompiler)(GLvoid)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ReleaseShaderCompiler(gc);
}

GLvoid GLAPIENTRY __GLES_APINAME(RenderbufferStorage)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.RenderbufferStorage(gc, target, internalformat, width, height);
}

GLvoid GLAPIENTRY __GLES_APINAME(SampleCoverage)(GLfloat value, GLboolean invert)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.SampleCoverage(gc, value, invert);
}

GLvoid GLAPIENTRY __GLES_APINAME(Scissor)(GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Scissor(gc, x, y, width, height);
}

GLvoid GLAPIENTRY __GLES_APINAME(ShaderBinary)(GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ShaderBinary(gc, n, shaders, binaryformat, binary, length);
}

GLvoid GLAPIENTRY __GLES_APINAME(ShaderSource)(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ShaderSource(gc, shader, count, string, length);
}

GLvoid GLAPIENTRY __GLES_APINAME(StencilFunc)(GLenum func, GLint ref, GLuint mask)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.StencilFunc(gc, func, ref, mask);
}

GLvoid GLAPIENTRY __GLES_APINAME(StencilFuncSeparate)(GLenum face, GLenum func, GLint ref, GLuint mask)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.StencilFuncSeparate(gc, face, func, ref, mask);
}

GLvoid GLAPIENTRY __GLES_APINAME(StencilMask)(GLuint mask)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.StencilMask(gc, mask);
}

GLvoid GLAPIENTRY __GLES_APINAME(StencilMaskSeparate)(GLenum face, GLuint mask)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.StencilMaskSeparate(gc, face, mask);
}

GLvoid GLAPIENTRY __GLES_APINAME(StencilOp)(GLenum fail, GLenum zfail, GLenum zpass)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.StencilOp(gc, fail, zfail, zpass);
}

GLvoid GLAPIENTRY __GLES_APINAME(StencilOpSeparate)(GLenum face, GLenum fail, GLenum zfail, GLenum zpass)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.StencilOpSeparate(gc, face, fail, zfail, zpass);
}

GLvoid GLAPIENTRY __GLES_APINAME(TexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexImage2D(gc, target, level, internalformat, width, height, border, format, type, pixels);
}

GLvoid GLAPIENTRY __GLES_APINAME(TexParameterf)(GLenum target, GLenum pname, GLfloat param)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexParameterf(gc, target, pname, param);
}

GLvoid GLAPIENTRY __GLES_APINAME(TexParameterfv)(GLenum target, GLenum pname, const GLfloat* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexParameterfv(gc, target, pname, params);
}

GLvoid GLAPIENTRY __GLES_APINAME(TexParameteri)(GLenum target, GLenum pname, GLint param)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexParameteri(gc, target, pname, param);
}

GLvoid GLAPIENTRY __GLES_APINAME(TexParameteriv)(GLenum target, GLenum pname, const GLint* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexParameteriv(gc, target, pname, params);
}

GLvoid GLAPIENTRY __GLES_APINAME(TexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexSubImage2D(gc, target, level, xoffset, yoffset, width, height, format, type, pixels);
}

GLvoid GLAPIENTRY __GLES_APINAME(Uniform1f)(GLint location, GLfloat x)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform1f(gc, location, x);
    __GL_PATTERN_MATCH_NAME(__glesApiEnum(Uniform1f));
}

GLvoid GLAPIENTRY __GLES_APINAME(Uniform1fv)(GLint location, GLsizei count, const GLfloat* v)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform1fv(gc, location, count, v);
}

GLvoid GLAPIENTRY __GLES_APINAME(Uniform1i)(GLint location, GLint x)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform1i(gc, location, x);
}

GLvoid GLAPIENTRY __GLES_APINAME(Uniform1iv)(GLint location, GLsizei count, const GLint* v)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform1iv(gc, location, count, v);
}

GLvoid GLAPIENTRY __GLES_APINAME(Uniform2f)(GLint location, GLfloat x, GLfloat y)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform2f(gc, location, x, y);
    __GL_PATTERN_MATCH_NAME(__glesApiEnum(Uniform2f));
}

GLvoid GLAPIENTRY __GLES_APINAME(Uniform2fv)(GLint location, GLsizei count, const GLfloat* v)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform2fv(gc, location, count, v);
}

GLvoid GLAPIENTRY __GLES_APINAME(Uniform2i)(GLint location, GLint x, GLint y)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform2i(gc, location, x, y);
}

GLvoid GLAPIENTRY __GLES_APINAME(Uniform2iv)(GLint location, GLsizei count, const GLint* v)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform2iv(gc, location, count, v);
}

GLvoid GLAPIENTRY __GLES_APINAME(Uniform3f)(GLint location, GLfloat x, GLfloat y, GLfloat z)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform3f(gc, location, x, y, z);
}

GLvoid GLAPIENTRY __GLES_APINAME(Uniform3fv)(GLint location, GLsizei count, const GLfloat* v)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform3fv(gc, location, count, v);
}

GLvoid GLAPIENTRY __GLES_APINAME(Uniform3i)(GLint location, GLint x, GLint y, GLint z)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform3i(gc, location, x, y, z);
}

GLvoid GLAPIENTRY __GLES_APINAME(Uniform3iv)(GLint location, GLsizei count, const GLint* v)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform3iv(gc, location, count, v);
}

GLvoid GLAPIENTRY __GLES_APINAME(Uniform4f)(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform4f(gc, location, x, y, z, w);
    __GL_PATTERN_MATCH_NAME(__glesApiEnum(Uniform4f));
}

GLvoid GLAPIENTRY __GLES_APINAME(Uniform4fv)(GLint location, GLsizei count, const GLfloat* v)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform4fv(gc, location, count, v);
}

GLvoid GLAPIENTRY __GLES_APINAME(Uniform4i)(GLint location, GLint x, GLint y, GLint z, GLint w)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform4i(gc, location, x, y, z, w);
}

GLvoid GLAPIENTRY __GLES_APINAME(Uniform4iv)(GLint location, GLsizei count, const GLint* v)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform4iv(gc, location, count, v);
}

GLvoid GLAPIENTRY __GLES_APINAME(UniformMatrix2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.UniformMatrix2fv(gc, location, count, transpose, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(UniformMatrix3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.UniformMatrix3fv(gc, location, count, transpose, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(UniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.UniformMatrix4fv(gc, location, count, transpose, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(UseProgram)(GLuint program)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.UseProgram(gc, program);
}

GLvoid GLAPIENTRY __GLES_APINAME(ValidateProgram)(GLuint program)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ValidateProgram(gc, program);
}

GLvoid GLAPIENTRY __GLES_APINAME(VertexAttrib1f)(GLuint indx, GLfloat x)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.VertexAttrib1f(gc, indx, x);
}

GLvoid GLAPIENTRY __GLES_APINAME(VertexAttrib1fv)(GLuint indx, const GLfloat* values)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.VertexAttrib1fv(gc, indx, values);
}

GLvoid GLAPIENTRY __GLES_APINAME(VertexAttrib2f)(GLuint indx, GLfloat x, GLfloat y)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.VertexAttrib2f(gc, indx, x, y);
}

GLvoid GLAPIENTRY __GLES_APINAME(VertexAttrib2fv)(GLuint indx, const GLfloat* values)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.VertexAttrib2fv(gc, indx, values);
}

GLvoid GLAPIENTRY __GLES_APINAME(VertexAttrib3f)(GLuint indx, GLfloat x, GLfloat y, GLfloat z)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.VertexAttrib3f(gc, indx, x, y, z);
}

GLvoid GLAPIENTRY __GLES_APINAME(VertexAttrib3fv)(GLuint indx, const GLfloat* values)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.VertexAttrib3fv(gc, indx, values);
}

GLvoid GLAPIENTRY __GLES_APINAME(VertexAttrib4f)(GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.VertexAttrib4f(gc, indx, x, y, z, w);
}

GLvoid GLAPIENTRY __GLES_APINAME(VertexAttrib4fv)(GLuint indx, const GLfloat* values)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.VertexAttrib4fv(gc, indx, values);
}

GLvoid GLAPIENTRY __GLES_APINAME(VertexAttribPointer)(GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.VertexAttribPointer(gc, indx, size, type, normalized, stride, ptr);
    __GL_PATTERN_MATCH_NAME_WITH_ARG2(__glesApiEnum(VertexAttribPointer), type, normalized);
}

GLvoid GLAPIENTRY __GLES_APINAME(Viewport)(GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Viewport(gc, x, y, width, height);
}


/*
** OpenGL ES 3.0
*/

GLvoid GLAPIENTRY __GLES_APINAME(ReadBuffer)(GLenum mode)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ReadBuffer(gc, mode);
}

GLvoid GLAPIENTRY __GLES_APINAME(DrawRangeElements)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DrawRangeElements(gc, mode, start, end, count, type, indices);
}

GLvoid GLAPIENTRY __GLES_APINAME(TexImage3D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexImage3D(gc, target, level, internalformat, width, height, depth, border, format, type, pixels);
}

GLvoid GLAPIENTRY __GLES_APINAME(TexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexSubImage3D(gc, target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
}

GLvoid GLAPIENTRY __GLES_APINAME(CopyTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.CopyTexSubImage3D(gc, target, level, xoffset, yoffset, zoffset, x, y, width, height);
}

GLvoid GLAPIENTRY __GLES_APINAME(CompressedTexImage3D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.CompressedTexImage3D(gc, target, level, internalformat, width, height, depth, border, imageSize, data);
}

GLvoid GLAPIENTRY __GLES_APINAME(CompressedTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.CompressedTexSubImage3D(gc, target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
}

GLvoid GLAPIENTRY __GLES_APINAME(GenQueries)(GLsizei n, GLuint* ids)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GenQueries(gc, n, ids);
}

GLvoid GLAPIENTRY __GLES_APINAME(DeleteQueries)(GLsizei n, const GLuint* ids)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DeleteQueries(gc, n, ids);
}

GLboolean GLAPIENTRY __GLES_APINAME(IsQuery)(GLuint id)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->apiDispatchTable.IsQuery(gc, id);
}

GLvoid GLAPIENTRY __GLES_APINAME(BeginQuery)(GLenum target, GLuint id)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BeginQuery(gc, target, id);
}

GLvoid GLAPIENTRY __GLES_APINAME(EndQuery)(GLenum target)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.EndQuery(gc, target);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetQueryiv)(GLenum target, GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetQueryiv(gc, target, pname, params);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetQueryObjectuiv)(GLuint id, GLenum pname, GLuint* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetQueryObjectuiv(gc, id, pname, params);
}

GLvoid* GLAPIENTRY __GLES_APINAME(MapBuffer)(GLenum target, GLenum access)
{
    __GL_GET_CONTEXT;
    return gc->currentImmediateTable->MapBuffer(gc, target, access);
}

GLboolean GLAPIENTRY __GLES_APINAME(UnmapBuffer)(GLenum target)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->apiDispatchTable.UnmapBuffer(gc, target);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetBufferPointerv)(GLenum target, GLenum pname, GLvoid** params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetBufferPointerv(gc, target, pname, params);
}

GLvoid GLAPIENTRY __GLES_APINAME(DrawBuffers)(GLsizei n, const GLenum* bufs)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DrawBuffers(gc, n, bufs);
}

GLvoid GLAPIENTRY __GLES_APINAME(UniformMatrix2x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.UniformMatrix2x3fv(gc, location, count, transpose, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(UniformMatrix3x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.UniformMatrix3x2fv(gc, location, count, transpose, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(UniformMatrix2x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.UniformMatrix2x4fv(gc, location, count, transpose, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(UniformMatrix4x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.UniformMatrix4x2fv(gc, location, count, transpose, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(UniformMatrix3x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.UniformMatrix3x4fv(gc, location, count, transpose, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(UniformMatrix4x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.UniformMatrix4x3fv(gc, location, count, transpose, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(BlitFramebuffer)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BlitFramebuffer(gc, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
}

GLvoid GLAPIENTRY __GLES_APINAME(RenderbufferStorageMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.RenderbufferStorageMultisample(gc, target, samples, internalformat, width, height);
}

GLvoid GLAPIENTRY __GLES_APINAME(FramebufferTextureLayer)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.FramebufferTextureLayer(gc, target, attachment, texture, level, layer);
}

GLvoid* GLAPIENTRY __GLES_APINAME(MapBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
    __GL_GET_CONTEXT_RET(gcvNULL);
    return gc->apiDispatchTable.MapBufferRange(gc, target, offset, length, access);
}

GLvoid GLAPIENTRY __GLES_APINAME(FlushMappedBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.FlushMappedBufferRange(gc, target, offset, length);
}

GLvoid GLAPIENTRY __GLES_APINAME(BindVertexArray)(GLuint array)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BindVertexArray(gc, array);
}

GLvoid GLAPIENTRY __GLES_APINAME(DeleteVertexArrays)(GLsizei n, const GLuint* arrays)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DeleteVertexArrays(gc, n, arrays);
}

GLvoid GLAPIENTRY __GLES_APINAME(GenVertexArrays)(GLsizei n, GLuint* arrays)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GenVertexArrays(gc, n, arrays);
}

GLboolean GLAPIENTRY __GLES_APINAME(IsVertexArray)(GLuint array)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->apiDispatchTable.IsVertexArray(gc, array);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetIntegeri_v)(GLenum target, GLuint index, GLint* data)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetIntegeri_v(gc, target, index, data);
}

GLvoid GLAPIENTRY __GLES_APINAME(BeginTransformFeedback)(GLenum primitiveMode)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BeginTransformFeedback(gc, primitiveMode);
}

GLvoid GLAPIENTRY __GLES_APINAME(EndTransformFeedback)(GLvoid)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.EndTransformFeedback(gc);
}

GLvoid GLAPIENTRY __GLES_APINAME(BindBufferRange)(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BindBufferRange(gc, target, index, buffer, offset, size);
}

GLvoid GLAPIENTRY __GLES_APINAME(BindBufferBase)(GLenum target, GLuint index, GLuint buffer)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BindBufferBase(gc, target, index, buffer);
}

GLvoid GLAPIENTRY __GLES_APINAME(TransformFeedbackVaryings)(GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TransformFeedbackVaryings(gc, program, count, varyings, bufferMode);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetTransformFeedbackVarying)(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetTransformFeedbackVarying(gc, program, index, bufSize, length, size, type, name);
}

GLvoid GLAPIENTRY __GLES_APINAME(VertexAttribIPointer)(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.VertexAttribIPointer(gc, index, size, type, stride, pointer);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetVertexAttribIiv)(GLuint index, GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetVertexAttribIiv(gc, index, pname, params);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetVertexAttribIuiv)(GLuint index, GLenum pname, GLuint* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetVertexAttribIuiv(gc, index, pname, params);
}

GLvoid GLAPIENTRY __GLES_APINAME(VertexAttribI4i)(GLuint index, GLint x, GLint y, GLint z, GLint w)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.VertexAttribI4i(gc, index, x, y, z, w);
}

GLvoid GLAPIENTRY __GLES_APINAME(VertexAttribI4ui)(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.VertexAttribI4ui(gc, index, x, y, z, w);
}

GLvoid GLAPIENTRY __GLES_APINAME(VertexAttribI4iv)(GLuint index, const GLint* v)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.VertexAttribI4iv(gc, index, v);
}

GLvoid GLAPIENTRY __GLES_APINAME(VertexAttribI4uiv)(GLuint index, const GLuint* v)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.VertexAttribI4uiv(gc, index, v);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetUniformuiv)(GLuint program, GLint location, GLuint* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetUniformuiv(gc, program, location, params);
}

GLint GLAPIENTRY __GLES_APINAME(GetFragDataLocation)(GLuint program, const GLchar *name)
{
    __GL_GET_CONTEXT_RET(GL_INVALID_OPERATION);
    return gc->apiDispatchTable.GetFragDataLocation(gc, program, name);
}

GLvoid GLAPIENTRY __GLES_APINAME(Uniform1ui)(GLint location, GLuint v0)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform1ui(gc, location, v0);
}

GLvoid GLAPIENTRY __GLES_APINAME(Uniform2ui)(GLint location, GLuint v0, GLuint v1)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform2ui(gc, location, v0, v1);
}

GLvoid GLAPIENTRY __GLES_APINAME(Uniform3ui)(GLint location, GLuint v0, GLuint v1, GLuint v2)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform3ui(gc, location, v0, v1, v2);
}

GLvoid GLAPIENTRY __GLES_APINAME(Uniform4ui)(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform4ui(gc, location, v0, v1, v2, v3);
}

GLvoid GLAPIENTRY __GLES_APINAME(Uniform1uiv)(GLint location, GLsizei count, const GLuint* value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform1uiv(gc, location, count, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(Uniform2uiv)(GLint location, GLsizei count, const GLuint* value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform2uiv(gc, location, count, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(Uniform3uiv)(GLint location, GLsizei count, const GLuint* value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform3uiv(gc, location, count, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(Uniform4uiv)(GLint location, GLsizei count, const GLuint* value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Uniform4uiv(gc, location, count, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(ClearBufferiv)(GLenum buffer, GLint drawbuffer, const GLint* value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ClearBufferiv(gc, buffer, drawbuffer, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(ClearBufferuiv)(GLenum buffer, GLint drawbuffer, const GLuint* value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ClearBufferuiv(gc, buffer, drawbuffer, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(ClearBufferfv)(GLenum buffer, GLint drawbuffer, const GLfloat* value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ClearBufferfv(gc, buffer, drawbuffer, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(ClearBufferfi)(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ClearBufferfi(gc, buffer, drawbuffer, depth, stencil);
}

const GLubyte* GLAPIENTRY __GLES_APINAME(GetStringi)(GLenum name, GLuint index)
{
    __GL_GET_CONTEXT_RET(gcvNULL);
    return gc->apiDispatchTable.GetStringi(gc, name, index);
}

GLvoid GLAPIENTRY __GLES_APINAME(CopyBufferSubData)(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.CopyBufferSubData(gc, readTarget, writeTarget, readOffset, writeOffset, size);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetUniformIndices)(GLuint program, GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetUniformIndices(gc, program, uniformCount, uniformNames, uniformIndices);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetActiveUniformsiv)(GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetActiveUniformsiv(gc, program, uniformCount, uniformIndices, pname, params);
}

GLuint GLAPIENTRY __GLES_APINAME(GetUniformBlockIndex)(GLuint program, const GLchar* uniformBlockName)
{
    __GL_GET_CONTEXT_RET(GL_INVALID_INDEX);
    return gc->apiDispatchTable.GetUniformBlockIndex(gc, program, uniformBlockName);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetActiveUniformBlockiv)(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetActiveUniformBlockiv(gc, program, uniformBlockIndex, pname, params);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetActiveUniformBlockName)(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetActiveUniformBlockName(gc, program, uniformBlockIndex, bufSize, length, uniformBlockName);
}

GLvoid GLAPIENTRY __GLES_APINAME(UniformBlockBinding)(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.UniformBlockBinding(gc, program, uniformBlockIndex, uniformBlockBinding);
}

GLvoid GLAPIENTRY __GLES_APINAME(DrawArraysInstanced)(GLenum mode, GLint first, GLsizei count, GLsizei instanceCount)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DrawArraysInstanced(gc, mode, first, count, instanceCount);
}

GLvoid GLAPIENTRY __GLES_APINAME(DrawElementsInstanced)(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei instanceCount)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DrawElementsInstanced(gc, mode, count, type, indices, instanceCount);
}

GLsync GLAPIENTRY __GLES_APINAME(FenceSync)(GLenum condition, GLbitfield flags)
{
    __GL_GET_CONTEXT_RET(0);
    return gc->apiDispatchTable.FenceSync(gc, condition, flags);
}

GLboolean GLAPIENTRY __GLES_APINAME(IsSync)(GLsync sync)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->apiDispatchTable.IsSync(gc, sync);
}

GLvoid GLAPIENTRY __GLES_APINAME(DeleteSync)(GLsync sync)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DeleteSync(gc, sync);
}

GLenum GLAPIENTRY __GLES_APINAME(ClientWaitSync)(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    __GL_GET_CONTEXT_RET(GL_WAIT_FAILED);
    return gc->apiDispatchTable.ClientWaitSync(gc, sync, flags, timeout);
}

GLvoid GLAPIENTRY __GLES_APINAME(WaitSync)(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.WaitSync(gc, sync, flags, timeout);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetInteger64v)(GLenum pname, GLint64* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetInteger64v(gc, pname, params);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetSynciv)(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetSynciv(gc, sync, pname, bufSize, length, values);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetInteger64i_v)(GLenum target, GLuint index, GLint64* data)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetInteger64i_v(gc, target, index, data);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetBufferParameteri64v)(GLenum target, GLenum pname, GLint64* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetBufferParameteri64v(gc, target, pname, params);
}

GLvoid GLAPIENTRY __GLES_APINAME(GenSamplers)(GLsizei count, GLuint* samplers)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GenSamplers(gc, count, samplers);
}

GLvoid GLAPIENTRY __GLES_APINAME(DeleteSamplers)(GLsizei count, const GLuint* samplers)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DeleteSamplers(gc, count, samplers);
}

GLboolean GLAPIENTRY __GLES_APINAME(IsSampler)(GLuint sampler)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->apiDispatchTable.IsSampler(gc, sampler);
}

GLvoid GLAPIENTRY __GLES_APINAME(BindSampler)(GLuint unit, GLuint sampler)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BindSampler(gc, unit, sampler);
}

GLvoid GLAPIENTRY __GLES_APINAME(SamplerParameteri)(GLuint sampler, GLenum pname, GLint param)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.SamplerParameteri(gc, sampler, pname, param);
}

GLvoid GLAPIENTRY __GLES_APINAME(SamplerParameteriv)(GLuint sampler, GLenum pname, const GLint* param)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.SamplerParameteriv(gc, sampler, pname, param);
}

GLvoid GLAPIENTRY __GLES_APINAME(SamplerParameterf)(GLuint sampler, GLenum pname, GLfloat param)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.SamplerParameterf(gc, sampler, pname, param);
}

GLvoid GLAPIENTRY __GLES_APINAME(SamplerParameterfv)(GLuint sampler, GLenum pname, const GLfloat* param)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.SamplerParameterfv(gc, sampler, pname, param);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetSamplerParameteriv)(GLuint sampler, GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetSamplerParameteriv(gc, sampler, pname, params);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetSamplerParameterfv)(GLuint sampler, GLenum pname, GLfloat* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetSamplerParameterfv(gc, sampler, pname, params);
}

GLvoid GLAPIENTRY __GLES_APINAME(VertexAttribDivisor)(GLuint index, GLuint divisor)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.VertexAttribDivisor(gc, index, divisor);
}

GLvoid GLAPIENTRY __GLES_APINAME(BindTransformFeedback)(GLenum target, GLuint id)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BindTransformFeedback(gc, target, id);
}

GLvoid GLAPIENTRY __GLES_APINAME(DeleteTransformFeedbacks)(GLsizei n, const GLuint* ids)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DeleteTransformFeedbacks(gc, n, ids);
}

GLvoid GLAPIENTRY __GLES_APINAME(GenTransformFeedbacks)(GLsizei n, GLuint* ids)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GenTransformFeedbacks(gc, n, ids);
}

GLboolean GLAPIENTRY __GLES_APINAME(IsTransformFeedback)(GLuint id)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->apiDispatchTable.IsTransformFeedback(gc, id);
}

GLvoid GLAPIENTRY __GLES_APINAME(PauseTransformFeedback)(GLvoid)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.PauseTransformFeedback(gc);
}

GLvoid GLAPIENTRY __GLES_APINAME(ResumeTransformFeedback)(GLvoid)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ResumeTransformFeedback(gc);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetProgramBinary)(GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, GLvoid* binary)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetProgramBinary(gc, program, bufSize, length, binaryFormat, binary);
}

GLvoid GLAPIENTRY __GLES_APINAME(ProgramBinary)(GLuint program, GLenum binaryFormat, const GLvoid* binary, GLsizei length)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramBinary(gc, program, binaryFormat, binary, length);
}

GLvoid GLAPIENTRY __GLES_APINAME(ProgramParameteri)(GLuint program, GLenum pname, GLint value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramParameteri(gc, program, pname, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(InvalidateFramebuffer)(GLenum target, GLsizei numAttachments, const GLenum* attachments)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.InvalidateFramebuffer(gc, target, numAttachments, attachments);
}

GLvoid GLAPIENTRY __GLES_APINAME(InvalidateSubFramebuffer)(GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.InvalidateSubFramebuffer(gc, target, numAttachments, attachments, x, y, width, height);
}

GLvoid GLAPIENTRY __GLES_APINAME(TexStorage2D)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexStorage2D(gc, target, levels, internalformat, width, height);
}

GLvoid GLAPIENTRY __GLES_APINAME(TexStorage3D)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexStorage3D(gc, target, levels, internalformat, width, height, depth);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetInternalformativ)(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetInternalformativ(gc, target, internalformat, pname, bufSize, params);
}

/*
** OpenGL ES 3.1
*/
GLvoid GLAPIENTRY __GLES_APINAME(DispatchCompute)(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DispatchCompute(gc, num_groups_x, num_groups_y, num_groups_z);
}

GLvoid GLAPIENTRY __GLES_APINAME(DispatchComputeIndirect)(GLintptr indirect)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DispatchComputeIndirect(gc, indirect);
}

GLvoid GLAPIENTRY __GLES_APINAME(DrawArraysIndirect)(GLenum mode, const void *indirect)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DrawArraysIndirect(gc, mode, indirect);
}

GLvoid GLAPIENTRY __GLES_APINAME(DrawElementsIndirect)(GLenum mode, GLenum type, const void *indirect)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DrawElementsIndirect(gc, mode, type, indirect);
}

GLvoid GLAPIENTRY __GLES_APINAME(FramebufferParameteri)(GLenum target, GLenum pname, GLint param)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.FramebufferParameteri(gc, target, pname, param);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetFramebufferParameteriv)(GLenum target, GLenum pname, GLint *params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetFramebufferParameteriv(gc, target, pname, params);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetProgramInterfaceiv)(GLuint program, GLenum programInterface, GLenum pname, GLint *params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetProgramInterfaceiv(gc, program, programInterface, pname, params);
}

GLuint GLAPIENTRY __GLES_APINAME(GetProgramResourceIndex)(GLuint program, GLenum programInterface, const GLchar *name)
{
    __GL_GET_CONTEXT_RET(0xffffffff);
    return gc->apiDispatchTable.GetProgramResourceIndex(gc, program, programInterface, name);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetProgramResourceName)(GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetProgramResourceName(gc, program, programInterface, index, bufSize, length, name);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetProgramResourceiv)(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetProgramResourceiv(gc, program, programInterface, index, propCount, props, bufSize, length, params);
}

GLint GLAPIENTRY __GLES_APINAME(GetProgramResourceLocation)(GLuint program, GLenum programInterface, const GLchar *name)
{
    __GL_GET_CONTEXT_RET(-1);
    return gc->apiDispatchTable.GetProgramResourceLocation(gc, program, programInterface, name);
}

GLvoid GLAPIENTRY __GLES_APINAME(UseProgramStages)(GLuint pipeline, GLbitfield stages, GLuint program)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.UseProgramStages(gc, pipeline, stages, program);
}

GLvoid GLAPIENTRY __GLES_APINAME(ActiveShaderProgram)(GLuint pipeline, GLuint program)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ActiveShaderProgram(gc, pipeline, program);
}

GLuint GLAPIENTRY __GLES_APINAME(CreateShaderProgramv)(GLenum type, GLsizei count, const GLchar *const*strings)
{
    __GL_GET_CONTEXT_RET(0);
    return gc->apiDispatchTable.CreateShaderProgramv(gc, type, count, strings);
}

GLvoid GLAPIENTRY __GLES_APINAME(BindProgramPipeline)(GLuint pipeline)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BindProgramPipeline(gc, pipeline);
}

GLvoid GLAPIENTRY __GLES_APINAME(DeleteProgramPipelines)(GLsizei n, const GLuint *pipelines)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DeleteProgramPipelines(gc, n , pipelines);
}

GLvoid GLAPIENTRY __GLES_APINAME(GenProgramPipelines)(GLsizei n, GLuint *pipelines)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GenProgramPipelines(gc, n , pipelines);
}

GLboolean GLAPIENTRY __GLES_APINAME(IsProgramPipeline)(GLuint pipeline)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->apiDispatchTable.IsProgramPipeline(gc, pipeline);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetProgramPipelineiv)(GLuint pipeline, GLenum pname, GLint *params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetProgramPipelineiv(gc, pipeline, pname, params);
}

GLvoid GLAPIENTRY __GLES_APINAME(ProgramUniform1i)(GLuint program, GLint location, GLint v0)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform1i(gc, program, location, v0);
}

GLvoid GLAPIENTRY __GLES_APINAME(ProgramUniform2i)(GLuint program, GLint location, GLint v0, GLint v1)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform2i(gc, program, location, v0, v1);
}

GLvoid GLAPIENTRY __GLES_APINAME(ProgramUniform3i)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform3i(gc, program, location, v0, v1, v2);
}

GLvoid GLAPIENTRY __GLES_APINAME(ProgramUniform4i)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform4i(gc, program, location, v0, v1, v2, v3);
}

GLvoid GLAPIENTRY __GLES_APINAME(ProgramUniform1ui)(GLuint program, GLint location, GLuint v0)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform1ui(gc, program, location, v0);
}

GLvoid GLAPIENTRY __GLES_APINAME(ProgramUniform2ui)(GLuint program, GLint location, GLuint v0, GLuint v1)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform2ui(gc, program, location, v0, v1);
}

GLvoid GLAPIENTRY __GLES_APINAME(ProgramUniform3ui)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform3ui(gc, program, location, v0, v1, v2);
}

GLvoid GLAPIENTRY __GLES_APINAME(ProgramUniform4ui)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform4ui(gc, program, location, v0, v1, v2, v3);
}

GLvoid GLAPIENTRY __GLES_APINAME(ProgramUniform1f)(GLuint program, GLint location, GLfloat v0)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform1f(gc, program, location, v0);
}

GLvoid GLAPIENTRY __GLES_APINAME(ProgramUniform2f)(GLuint program, GLint location, GLfloat v0, GLfloat v1)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform2f(gc, program, location, v0, v1);
}

GLvoid GLAPIENTRY __GLES_APINAME(ProgramUniform3f)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform3f(gc, program, location, v0, v1, v2);
}

GLvoid GLAPIENTRY __GLES_APINAME(ProgramUniform4f)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform4f(gc, program, location, v0, v1, v2, v3);
}

GLvoid GLAPIENTRY __GLES_APINAME(ProgramUniform1iv)(GLuint program, GLint location, GLsizei count, const GLint *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform1iv(gc, program, location, count, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(ProgramUniform2iv)(GLuint program, GLint location, GLsizei count, const GLint *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform2iv(gc, program, location, count, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(ProgramUniform3iv)(GLuint program, GLint location, GLsizei count, const GLint *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform3iv(gc, program, location, count, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(ProgramUniform4iv)(GLuint program, GLint location, GLsizei count, const GLint *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform4iv(gc, program, location, count, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(ProgramUniform1uiv)(GLuint program, GLint location, GLsizei count, const GLuint *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform1uiv(gc, program, location, count, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(ProgramUniform2uiv)(GLuint program, GLint location, GLsizei count, const GLuint *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform2uiv(gc, program, location, count, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(ProgramUniform3uiv)(GLuint program, GLint location, GLsizei count, const GLuint *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform3uiv(gc, program, location, count, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(ProgramUniform4uiv)(GLuint program, GLint location, GLsizei count, const GLuint *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform4uiv(gc, program, location, count, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(ProgramUniform1fv)(GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform1fv(gc, program, location, count, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(ProgramUniform2fv)(GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform2fv(gc, program, location, count, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(ProgramUniform3fv)(GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform3fv(gc, program, location, count, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(ProgramUniform4fv)(GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniform4fv(gc, program, location, count, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(ProgramUniformMatrix2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniformMatrix2fv(gc, program, location, count, transpose, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(ProgramUniformMatrix3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniformMatrix3fv(gc, program, location, count, transpose, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(ProgramUniformMatrix4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniformMatrix4fv(gc, program, location, count, transpose, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(ProgramUniformMatrix2x3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniformMatrix2x3fv(gc, program, location, count, transpose, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(ProgramUniformMatrix3x2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniformMatrix3x2fv(gc, program, location, count, transpose, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(ProgramUniformMatrix2x4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniformMatrix2x4fv(gc, program, location, count, transpose, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(ProgramUniformMatrix4x2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniformMatrix4x2fv(gc, program, location, count, transpose, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(ProgramUniformMatrix3x4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniformMatrix3x4fv(gc, program, location, count, transpose, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(ProgramUniformMatrix4x3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramUniformMatrix4x3fv(gc, program, location, count, transpose, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(ValidateProgramPipeline)(GLuint pipeline)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ValidateProgramPipeline(gc, pipeline);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetProgramPipelineInfoLog)(GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetProgramPipelineInfoLog(gc, pipeline, bufSize, length, infoLog);
}

GLvoid GLAPIENTRY __GLES_APINAME(BindImageTexture)(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BindImageTexture(gc, unit, texture, level, layered, layer, access, format);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetBooleani_v)(GLenum target, GLuint index, GLboolean *data)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetBooleani_v(gc, target, index, data);
}

GLvoid GLAPIENTRY __GLES_APINAME(MemoryBarrier)(GLbitfield barriers)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.MemoryBarrier(gc, barriers);
}

GLvoid GLAPIENTRY __GLES_APINAME(MemoryBarrierByRegion)(GLbitfield barriers)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.MemoryBarrierByRegion(gc, barriers);
}

GLvoid GLAPIENTRY __GLES_APINAME(TexStorage2DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexStorage2DMultisample(gc, target, samples, internalformat, width, height, fixedsamplelocations);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetMultisamplefv)(GLenum pname, GLuint index, GLfloat *val)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetMultisamplefv(gc, pname, index, val);
}

GLvoid GLAPIENTRY __GLES_APINAME(SampleMaski)(GLuint maskNumber, GLbitfield mask)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.SampleMaski(gc, maskNumber, mask);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetTexLevelParameteriv)(GLenum target, GLint level, GLenum pname, GLint *params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetTexLevelParameteriv(gc, target, level, pname, params);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetTexLevelParameterfv)(GLenum target, GLint level, GLenum pname, GLfloat *params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetTexLevelParameterfv(gc, target, level, pname, params);
}

GLvoid GLAPIENTRY __GLES_APINAME(BindVertexBuffer)(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BindVertexBuffer(gc, bindingindex, buffer, offset, stride);
}

GLvoid GLAPIENTRY __GLES_APINAME(VertexAttribFormat)(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.VertexAttribFormat(gc, attribindex, size, type, normalized, relativeoffset);
}

GLvoid GLAPIENTRY __GLES_APINAME(VertexAttribIFormat)(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.VertexAttribIFormat(gc, attribindex, size, type, relativeoffset);
}

GLvoid GLAPIENTRY __GLES_APINAME(VertexAttribBinding)(GLuint attribindex, GLuint bindingindex)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.VertexAttribBinding(gc, attribindex, bindingindex);
}

GLvoid GLAPIENTRY __GLES_APINAME(VertexBindingDivisor)(GLuint bindingindex, GLuint divisor)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.VertexBindingDivisor(gc, bindingindex, divisor);
}

/* OpenGL ES 3.2 */

GLvoid GLAPIENTRY __GLES_APINAME(TexStorage3DMultisample)(GLenum target, GLsizei samples, GLenum sizedinternalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexStorage3DMultisample(gc, target, samples, sizedinternalformat, width, height, depth, fixedsamplelocations);
}

GLvoid GLAPIENTRY __GLES_APINAME(BlendBarrier)(void)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BlendBarrier(gc);
}

GLvoid GLAPIENTRY __GLES_APINAME(DebugMessageControl)(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DebugMessageControl(gc, source, type, severity, count, ids, enabled);
}

GLvoid GLAPIENTRY __GLES_APINAME(DebugMessageInsert)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* buf)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DebugMessageInsert(gc, source, type, id, severity, length, buf);
}

GLvoid GLAPIENTRY __GLES_APINAME(DebugMessageCallback)(GLDEBUGPROCKHR callback, const GLvoid* userParam)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DebugMessageCallback(gc, callback, userParam);
}

GLuint GLAPIENTRY __GLES_APINAME(GetDebugMessageLog)(GLuint count, GLsizei bufSize, GLenum* sources, GLenum* types, GLuint* ids, GLenum* severities, GLsizei* lengths, GLchar* messageLog)
{
    __GL_GET_CONTEXT_RET(0);
    return gc->apiDispatchTable.GetDebugMessageLog(gc, count, bufSize, sources, types, ids, severities, lengths, messageLog);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetPointerv)(GLenum pname, GLvoid** params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetPointerv(gc, pname, params);
}

GLvoid GLAPIENTRY __GLES_APINAME(PushDebugGroup)(GLenum source, GLuint id, GLsizei length, const GLchar * message)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.PushDebugGroup(gc, source, id, length, message);
}

GLvoid GLAPIENTRY __GLES_APINAME(PopDebugGroup)(void)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.PopDebugGroup(gc);
}

GLvoid GLAPIENTRY __GLES_APINAME(ObjectLabel)(GLenum identifier, GLuint name, GLsizei length, const GLchar *label)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ObjectLabel(gc, identifier, name, length, label);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetObjectLabel)(GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetObjectLabel(gc, identifier, name, bufSize, length, label);
}

GLvoid GLAPIENTRY __GLES_APINAME(ObjectPtrLabel)(const GLvoid* ptr, GLsizei length, const GLchar *label)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ObjectPtrLabel(gc, ptr, length, label);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetObjectPtrLabel)(const GLvoid* ptr, GLsizei bufSize, GLsizei *length, GLchar *label)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetObjectPtrLabel(gc, ptr, bufSize, length, label);
}

GLenum GLAPIENTRY __GLES_APINAME(GetGraphicsResetStatus)()
{
    __GL_GET_CONTEXT_RET(GL_NO_ERROR);
    return gc->apiDispatchTable.GetGraphicsResetStatus(gc);
}

GLvoid GLAPIENTRY __GLES_APINAME(ReadnPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, GLvoid *data)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ReadnPixels(gc, x, y, width, height, format, type, bufSize, data);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetnUniformfv)(GLuint program, GLint location, GLsizei bufSize, GLfloat *params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetnUniformfv(gc, program, location, bufSize, params);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetnUniformiv)(GLuint program, GLint location, GLsizei bufSize, GLint *params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetnUniformiv(gc, program, location, bufSize, params);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetnUniformuiv)(GLuint program, GLint location, GLsizei bufSize, GLuint *params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetnUniformuiv(gc, program, location, bufSize, params);
}

GLvoid GLAPIENTRY __GLES_APINAME(BlendEquationi)(GLuint buf, GLenum mode)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BlendEquationi(gc, buf, mode);
}

GLvoid GLAPIENTRY __GLES_APINAME(BlendEquationSeparatei)(GLuint buf, GLenum modeRGB, GLenum modeAlpha)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BlendEquationSeparatei(gc, buf, modeRGB, modeAlpha);
}

GLvoid GLAPIENTRY __GLES_APINAME(BlendFunci)(GLuint buf, GLenum sfactor, GLenum dfactor)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BlendFunci(gc, buf, sfactor, dfactor);
}

GLvoid GLAPIENTRY __GLES_APINAME(BlendFuncSeparatei)(GLuint buf, GLenum sfactorRGB,GLenum dfactorRGB,GLenum sfactorAlpha,GLenum dfactorAlpha)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BlendFuncSeparatei(gc, buf, sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
}

GLvoid GLAPIENTRY __GLES_APINAME(ColorMaski)(GLuint buf, GLboolean r, GLboolean g, GLboolean b, GLboolean a)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ColorMaski(gc, buf, r, g, b, a);
}

GLvoid GLAPIENTRY __GLES_APINAME(Enablei)(GLenum target, GLuint index)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Enablei(gc, target, index);
}

GLvoid GLAPIENTRY  __GLES_APINAME(Disablei)( GLenum target, GLuint index)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.Disablei(gc, target, index);
}

GLboolean GLAPIENTRY  __GLES_APINAME(IsEnabledi)( GLenum target, GLuint index)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->apiDispatchTable.IsEnabledi(gc, target, index);
}

GLvoid GLAPIENTRY __GLES_APINAME(TexParameterIiv)(GLenum target, GLenum pname, const GLint *params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexParameterIiv(gc, target, pname, params);
}

GLvoid GLAPIENTRY __GLES_APINAME(TexParameterIuiv)(GLenum target, GLenum pname, const GLuint *params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexParameterIuiv(gc, target, pname, params);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetTexParameterIiv)(GLenum target, GLenum pname, GLint *params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetTexParameterIiv(gc, target, pname, params);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetTexParameterIuiv)(GLenum target, GLenum pname, GLuint *params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetTexParameterIuiv(gc, target, pname, params);
}

GLvoid GLAPIENTRY __GLES_APINAME(SamplerParameterIiv)(GLuint sampler, GLenum pname, const GLint *param)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.SamplerParameterIiv(gc, sampler, pname, param);
}

GLvoid GLAPIENTRY __GLES_APINAME(SamplerParameterIuiv)(GLuint sampler, GLenum pname, const GLuint *param)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.SamplerParameterIuiv(gc, sampler, pname, param);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetSamplerParameterIiv)(GLuint sampler, GLenum pname, GLint *params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetSamplerParameterIiv(gc, sampler, pname, params);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetSamplerParameterIuiv)(GLuint sampler, GLenum pname, GLuint *params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetSamplerParameterIuiv(gc, sampler, pname, params);
}

GLvoid GLAPIENTRY __GLES_APINAME(TexBuffer)(GLenum target, GLenum internalformat, GLuint buffer)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexBuffer(gc, target, internalformat, buffer);
}

GLvoid GLAPIENTRY __GLES_APINAME(TexBufferRange)(GLenum target, GLenum internalformat, GLuint buffer,
                                                  GLintptr offset, GLsizeiptr size)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexBufferRange(gc, target, internalformat, buffer, offset, size);
}

GLvoid GLAPIENTRY __GLES_APINAME(PatchParameteri)(GLenum pname, GLint value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.PatchParameteri(gc, pname, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(FramebufferTexture)(GLenum target, GLenum attachment, GLuint texture, GLint level)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.FramebufferTexture(gc, target, attachment, texture, level);
}

GLvoid GLAPIENTRY __GLES_APINAME(MinSampleShading)(GLfloat value)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.MinSampleShading(gc, value);
}

GLvoid GLAPIENTRY __GLES_APINAME(CopyImageSubData)(GLuint srcName, GLenum srcTarget, GLint srcLevel,
                                                    GLint srcX, GLint srcY, GLint srcZ,
                                                    GLuint dstName, GLenum dstTarget, GLint dstLevel,
                                                    GLint dstX, GLint dstY, GLint dstZ,
                                                    GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.CopyImageSubData(gc, srcName, srcTarget, srcLevel, srcX, srcY, srcZ, dstName, dstTarget, dstLevel,
                                          dstX, dstY, dstZ, srcWidth, srcHeight, srcDepth);
}


GLvoid GLAPIENTRY __GLES_APINAME(DrawElementsBaseVertex)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DrawElementsBaseVertex(gc, mode, count, type, indices, basevertex);
}

GLvoid GLAPIENTRY __GLES_APINAME(DrawRangeElementsBaseVertex)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DrawRangeElementsBaseVertex(gc, mode, start, end, count, type, indices, basevertex);
}

GLvoid GLAPIENTRY __GLES_APINAME(DrawElementsInstancedBaseVertex)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DrawElementsInstancedBaseVertex(gc, mode, count, type, indices, instancecount, basevertex);
}

GLvoid GLAPIENTRY __GLES_APINAME(PrimitiveBoundingBox)(GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW,
                                                        GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.PrimitiveBoundingBox(gc, minX, minY, minZ, minW, maxX, maxY, maxZ, maxW);
}
/* add GL api */
GLboolean GLAPIENTRY __GLES_APINAME(IsList)( GLuint list )
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->currentImmediateTable->IsList(gc, list);
}
GLvoid GLAPIENTRY __GLES_APINAME(DeleteLists)( GLuint list, GLsizei range )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->DeleteLists(gc, list, range);
}
GLuint GLAPIENTRY __GLES_APINAME(GenLists)( GLsizei range )
{
    __GL_GET_CONTEXT_RET(0);
    return gc->currentImmediateTable->GenLists(gc, range);
}
GLvoid GLAPIENTRY __GLES_APINAME(NewList)( GLuint list, GLenum mode )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->NewList(gc, list, mode);
}
GLvoid GLAPIENTRY __GLES_APINAME(EndList)()
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->EndList(gc);
}
GLvoid GLAPIENTRY __GLES_APINAME(CallList)( GLuint list )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->CallList(gc, list);
}
GLvoid GLAPIENTRY __GLES_APINAME(CallLists)( GLsizei n, GLenum type, const GLvoid *lists )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->CallLists(gc, n, type, lists);
}
GLvoid GLAPIENTRY __GLES_APINAME(ListBase)( GLuint base )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->ListBase(gc, base);
}
GLvoid GLAPIENTRY __GLES_APINAME(Begin)( GLenum mode )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Begin(gc, mode);
}
GLvoid GLAPIENTRY __GLES_APINAME(Bitmap)( GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Bitmap(gc, width, height, xorig, yorig, xmove, ymove, bitmap);
}
GLvoid GLAPIENTRY __GLES_APINAME(Color3b)( GLbyte red, GLbyte green, GLbyte blue )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Color3b(gc, red, green, blue);
}
GLvoid GLAPIENTRY __GLES_APINAME(Color3d)( GLdouble red, GLdouble green, GLdouble blue )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Color3d(gc, red, green, blue);
}
GLvoid GLAPIENTRY __GLES_APINAME(Color3f)( GLfloat red, GLfloat green, GLfloat blue )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Color3f(gc, red, green, blue);
}
GLvoid GLAPIENTRY __GLES_APINAME(Color3i)( GLint red, GLint green, GLint blue )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Color3i(gc, red, green, blue);
}
GLvoid GLAPIENTRY __GLES_APINAME(Color3s)( GLshort red, GLshort green, GLshort blue )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Color3s(gc, red, green, blue);
}
GLvoid GLAPIENTRY __GLES_APINAME(Color3ub)( GLubyte red, GLubyte green, GLubyte blue )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Color3ub(gc, red, green, blue);
}
GLvoid GLAPIENTRY __GLES_APINAME(Color3ui)( GLuint red, GLuint green, GLuint blue )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Color3ui(gc, red, green, blue);
}
GLvoid GLAPIENTRY __GLES_APINAME(Color3us)( GLushort red, GLushort green, GLushort blue )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Color3us(gc, red, green, blue);
}
GLvoid GLAPIENTRY __GLES_APINAME(Color3bv)( const GLbyte *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Color3bv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(Color3dv)( const GLdouble *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Color3dv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(Color3fv)( const GLfloat *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Color3fv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(Color3iv)( const GLint *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Color3iv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(Color3sv)( const GLshort *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Color3sv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(Color3ubv)( const GLubyte *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Color3ubv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(Color3uiv)( const GLuint *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Color3uiv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(Color3usv)( const GLushort *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Color3usv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(Color4b)( GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Color4b(gc, red, green, blue, alpha);
}
GLvoid GLAPIENTRY __GLES_APINAME(Color4d)( GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Color4d(gc, red, green, blue, alpha);
}
GLvoid GLAPIENTRY __GLES_APINAME(Color4f)( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Color4f(gc, red, green, blue, alpha);
}
GLvoid GLAPIENTRY __GLES_APINAME(Color4i)( GLint red, GLint green, GLint blue, GLint alpha )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Color4i(gc, red, green, blue, alpha);
}
GLvoid GLAPIENTRY __GLES_APINAME(Color4s)( GLshort red, GLshort green, GLshort blue, GLshort alpha )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Color4s(gc, red, green, blue, alpha);
}
GLvoid GLAPIENTRY __GLES_APINAME(Color4ub)( GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Color4ub(gc, red, green, blue, alpha);
}
GLvoid GLAPIENTRY __GLES_APINAME(Color4ui)( GLuint red, GLuint green, GLuint blue, GLuint alpha )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Color4ui(gc, red, green, blue, alpha);
}
GLvoid GLAPIENTRY __GLES_APINAME(Color4us)( GLushort red, GLushort green, GLushort blue, GLushort alpha )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Color4us(gc, red, green, blue, alpha);
}
GLvoid GLAPIENTRY __GLES_APINAME(Color4bv)( const GLbyte *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Color4bv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(Color4dv)( const GLdouble *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Color4dv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(Color4fv)( const GLfloat *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Color4fv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(Color4iv)( const GLint *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Color4iv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(Color4sv)( const GLshort *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Color4sv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(Color4ubv)( const GLubyte *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Color4ubv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(Color4uiv)( const GLuint *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Color4uiv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(Color4usv)( const GLushort *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Color4usv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(EdgeFlag)( GLboolean flag )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->EdgeFlag(gc, flag);
}
GLvoid GLAPIENTRY __GLES_APINAME(EdgeFlagv)( const GLboolean *flag )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->EdgeFlagv(gc, flag);
}
GLvoid GLAPIENTRY __GLES_APINAME(End)()
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->End(gc);
}
GLvoid GLAPIENTRY __GLES_APINAME(Indexd)( GLdouble c )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Indexd(gc, c);
}
GLvoid GLAPIENTRY __GLES_APINAME(Indexf)( GLfloat c )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Indexf(gc, c);
}
GLvoid GLAPIENTRY __GLES_APINAME(Indexi)( GLint c )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Indexi(gc, c);
}
GLvoid GLAPIENTRY __GLES_APINAME(Indexs)( GLshort c )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Indexs(gc, c);
}
GLvoid GLAPIENTRY __GLES_APINAME(Indexdv)( const GLdouble *c )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Indexdv(gc, c);
}
GLvoid GLAPIENTRY __GLES_APINAME(Indexfv)( const GLfloat *c )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Indexfv(gc, c);
}
GLvoid GLAPIENTRY __GLES_APINAME(Indexiv)( const GLint *c )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Indexiv(gc, c);
}
GLvoid GLAPIENTRY __GLES_APINAME(Indexsv)( const GLshort *c )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Indexsv(gc, c);
}
GLvoid GLAPIENTRY __GLES_APINAME(Normal3b)( GLbyte nx, GLbyte ny, GLbyte nz )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Normal3b(gc, nx, ny, nz);
}
GLvoid GLAPIENTRY __GLES_APINAME(Normal3d)( GLdouble nx, GLdouble ny, GLdouble nz )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Normal3d(gc, nx, ny, nz);
}
GLvoid GLAPIENTRY __GLES_APINAME(Normal3f)( GLfloat nx, GLfloat ny, GLfloat nz )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Normal3f(gc, nx, ny, nz);
}
GLvoid GLAPIENTRY __GLES_APINAME(Normal3i)( GLint nx, GLint ny, GLint nz )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Normal3i(gc, nx, ny, nz);
}
GLvoid GLAPIENTRY __GLES_APINAME(Normal3s)( GLshort nx, GLshort ny, GLshort nz )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Normal3s(gc, nx, ny, nz);
}
GLvoid GLAPIENTRY __GLES_APINAME(Normal3bv)( const GLbyte *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Normal3bv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(Normal3dv)( const GLdouble *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Normal3dv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(Normal3fv)( const GLfloat *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Normal3fv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(Normal3iv)( const GLint *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Normal3iv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(Normal3sv)( const GLshort *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Normal3sv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(RasterPos2d)( GLdouble x, GLdouble y )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->RasterPos2d(gc, x, y);
}
GLvoid GLAPIENTRY __GLES_APINAME(RasterPos2f)( GLfloat x, GLfloat y )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->RasterPos2f(gc, x, y);
}
GLvoid GLAPIENTRY __GLES_APINAME(RasterPos2i)( GLint x, GLint y )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->RasterPos2i(gc, x, y);
}
GLvoid GLAPIENTRY __GLES_APINAME(RasterPos2s)( GLshort x, GLshort y )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->RasterPos2s(gc, x, y);
}
GLvoid GLAPIENTRY __GLES_APINAME(RasterPos3d)( GLdouble x, GLdouble y, GLdouble z )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->RasterPos3d(gc, x, y, z);
}
GLvoid GLAPIENTRY __GLES_APINAME(RasterPos3f)( GLfloat x, GLfloat y, GLfloat z )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->RasterPos3f(gc, x, y, z);
}
GLvoid GLAPIENTRY __GLES_APINAME(RasterPos3i)( GLint x, GLint y, GLint z )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->RasterPos3i(gc, x, y, z);
}
GLvoid GLAPIENTRY __GLES_APINAME(RasterPos3s)( GLshort x, GLshort y, GLshort z )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->RasterPos3s(gc, x, y, z);
}
GLvoid GLAPIENTRY __GLES_APINAME(RasterPos4d)( GLdouble x, GLdouble y, GLdouble z, GLdouble w )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->RasterPos4d(gc, x, y, z, w);
}
GLvoid GLAPIENTRY __GLES_APINAME(RasterPos4f)( GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->RasterPos4f(gc, x, y, z, w);
}
GLvoid GLAPIENTRY __GLES_APINAME(RasterPos4i)( GLint x, GLint y, GLint z, GLint w )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->RasterPos4i(gc, x, y, z, w);
}
GLvoid GLAPIENTRY __GLES_APINAME(RasterPos4s)( GLshort x, GLshort y, GLshort z, GLshort w )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->RasterPos4s(gc, x, y, z, w);
}
GLvoid GLAPIENTRY __GLES_APINAME(RasterPos2dv)( const GLdouble *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->RasterPos2dv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(RasterPos2fv)( const GLfloat *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->RasterPos2fv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(RasterPos2iv)( const GLint *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->RasterPos2iv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(RasterPos2sv)( const GLshort *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->RasterPos2sv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(RasterPos3dv)( const GLdouble *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->RasterPos3dv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(RasterPos3fv)( const GLfloat *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->RasterPos3fv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(RasterPos3iv)( const GLint *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->RasterPos3iv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(RasterPos3sv)( const GLshort *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->RasterPos3sv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(RasterPos4dv)( const GLdouble *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->RasterPos4dv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(RasterPos4fv)( const GLfloat *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->RasterPos4fv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(RasterPos4iv)( const GLint *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->RasterPos4iv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(RasterPos4sv)( const GLshort *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->RasterPos4sv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(Rectd)( GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2 )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Rectd(gc, x1, y1, x2, y2);
}
GLvoid GLAPIENTRY __GLES_APINAME(Rectf)( GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2 )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Rectf(gc, x1, y1, x2, y2);
}
GLvoid GLAPIENTRY __GLES_APINAME(Recti)( GLint x1, GLint y1, GLint x2, GLint y2 )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Recti(gc, x1, y1, x2, y2);
}
GLvoid GLAPIENTRY __GLES_APINAME(Rects)( GLshort x1, GLshort y1, GLshort x2, GLshort y2 )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Rects(gc, x1, y1, x2, y2);
}
GLvoid GLAPIENTRY __GLES_APINAME(Rectdv)( const GLdouble *v1, const GLdouble *v2 )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Rectdv(gc, v1, v2);
}
GLvoid GLAPIENTRY __GLES_APINAME(Rectfv)( const GLfloat *v1, const GLfloat *v2 )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Rectfv(gc, v1, v2);
}
GLvoid GLAPIENTRY __GLES_APINAME(Rectiv)( const GLint *v1, const GLint *v2 )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Rectiv(gc, v1, v2);
}
GLvoid GLAPIENTRY __GLES_APINAME(Rectsv)( const GLshort *v1, const GLshort *v2 )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Rectsv(gc, v1, v2);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexCoord1d)( GLdouble s )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexCoord1d(gc, s);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexCoord1f)( GLfloat s )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexCoord1f(gc, s);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexCoord1i)( GLint s )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexCoord1i(gc, s);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexCoord1s)( GLshort s )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexCoord1s(gc, s);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexCoord2d)( GLdouble s, GLdouble t )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexCoord2d(gc, s, t);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexCoord2f)( GLfloat s, GLfloat t )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexCoord2f(gc, s, t);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexCoord2i)( GLint s, GLint t )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexCoord2i(gc, s, t);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexCoord2s)( GLshort s, GLshort t )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexCoord2s(gc, s, t);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexCoord3d)( GLdouble s, GLdouble t, GLdouble r )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexCoord3d(gc, s, t, r);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexCoord3f)( GLfloat s, GLfloat t, GLfloat r )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexCoord3f(gc, s, t, r);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexCoord3i)( GLint s, GLint t, GLint r )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexCoord3i(gc, s, t, r);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexCoord3s)( GLshort s, GLshort t, GLshort r )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexCoord3s(gc, s, t, r);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexCoord4d)( GLdouble s, GLdouble t, GLdouble r, GLdouble q )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexCoord4d(gc, s, t, r, q);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexCoord4f)( GLfloat s, GLfloat t, GLfloat r, GLfloat q )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexCoord4f(gc, s, t, r, q);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexCoord4i)( GLint s, GLint t, GLint r, GLint q )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexCoord4i(gc, s, t, r, q);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexCoord4s)( GLshort s, GLshort t, GLshort r, GLshort q )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexCoord4s(gc, s, t, r, q);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexCoord1dv)( const GLdouble *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexCoord1dv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexCoord1fv)( const GLfloat *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexCoord1fv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexCoord1iv)( const GLint *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexCoord1iv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexCoord1sv)( const GLshort *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexCoord1sv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexCoord2dv)( const GLdouble *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexCoord2dv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexCoord2fv)( const GLfloat *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexCoord2fv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexCoord2iv)( const GLint *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexCoord2iv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexCoord2sv)( const GLshort *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexCoord2sv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexCoord3dv)( const GLdouble *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexCoord3dv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexCoord3fv)( const GLfloat *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexCoord3fv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexCoord3iv)( const GLint *v )
{
   __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexCoord3iv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexCoord3sv)( const GLshort *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexCoord3sv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexCoord4dv)( const GLdouble *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexCoord4dv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexCoord4fv)( const GLfloat *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexCoord4fv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexCoord4iv)( const GLint *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexCoord4iv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexCoord4sv)( const GLshort *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexCoord4sv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(Vertex2d)( GLdouble x, GLdouble y )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Vertex2d(gc, x, y);
}
GLvoid GLAPIENTRY __GLES_APINAME(Vertex2f)( GLfloat x, GLfloat y )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Vertex2f(gc, x, y);
}
GLvoid GLAPIENTRY __GLES_APINAME(Vertex2i)( GLint x, GLint y )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Vertex2i(gc, x, y);
}
GLvoid GLAPIENTRY __GLES_APINAME(Vertex2s)( GLshort x, GLshort y )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Vertex2s(gc, x, y);
}
GLvoid GLAPIENTRY __GLES_APINAME(Vertex3d)( GLdouble x, GLdouble y, GLdouble z )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Vertex3d(gc, x, y, z);
}
GLvoid GLAPIENTRY __GLES_APINAME(Vertex3f)( GLfloat x, GLfloat y, GLfloat z )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Vertex3f(gc, x, y, z);
}
GLvoid GLAPIENTRY __GLES_APINAME(Vertex3i)( GLint x, GLint y, GLint z )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Vertex3i(gc, x, y, z);
}
GLvoid GLAPIENTRY __GLES_APINAME(Vertex3s)( GLshort x, GLshort y, GLshort z )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Vertex3s(gc, x, y, z);
}
GLvoid GLAPIENTRY __GLES_APINAME(Vertex4d)( GLdouble x, GLdouble y, GLdouble z, GLdouble w )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Vertex4d(gc, x, y, z, w);
}
GLvoid GLAPIENTRY __GLES_APINAME(Vertex4f)( GLfloat x, GLfloat y, GLfloat z, GLfloat w )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Vertex4f(gc, x, y, z, w);
}
GLvoid GLAPIENTRY __GLES_APINAME(Vertex4i)( GLint x, GLint y, GLint z, GLint w )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Vertex4i(gc, x, y, z, w);
}
GLvoid GLAPIENTRY __GLES_APINAME(Vertex4s)( GLshort x, GLshort y, GLshort z, GLshort w )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Vertex4s(gc, x, y, z, w);
}
GLvoid GLAPIENTRY __GLES_APINAME(Vertex2dv)( const GLdouble *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Vertex2dv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(Vertex2fv)( const GLfloat *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Vertex2fv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(Vertex2iv)( const GLint *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Vertex2iv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(Vertex2sv)( const GLshort *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Vertex2sv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(Vertex3dv)( const GLdouble *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Vertex3dv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(Vertex3fv)( const GLfloat *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Vertex3fv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(Vertex3iv)( const GLint *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Vertex3iv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(Vertex3sv)( const GLshort *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Vertex3sv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(Vertex4dv)( const GLdouble *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Vertex4dv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(Vertex4fv)( const GLfloat *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Vertex4fv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(Vertex4iv)( const GLint *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Vertex4iv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(Vertex4sv)( const GLshort *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Vertex4sv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(ClipPlane)( GLenum plane, const GLdouble *equation )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->ClipPlane(gc, plane, equation);
}
GLvoid GLAPIENTRY __GLES_APINAME(ColorMaterial)( GLenum face, GLenum mode )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->ColorMaterial(gc, face, mode);
}
GLvoid GLAPIENTRY __GLES_APINAME(Fogf)( GLenum pname, GLfloat param )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Fogf(gc, pname, param);
}
GLvoid GLAPIENTRY __GLES_APINAME(Fogi)( GLenum pname, GLint param )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Fogi(gc, pname, param);
}
GLvoid GLAPIENTRY __GLES_APINAME(Fogfv)( GLenum pname, const GLfloat *params )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Fogfv(gc, pname, params);
}
GLvoid GLAPIENTRY __GLES_APINAME(Fogiv)( GLenum pname, const GLint *params )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Fogiv(gc, pname, params);
}
GLvoid GLAPIENTRY __GLES_APINAME(Lightf)( GLenum light, GLenum pname, GLfloat param )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Lightf(gc, light, pname, param);
}
GLvoid GLAPIENTRY __GLES_APINAME(Lighti)( GLenum light, GLenum pname, GLint param )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Lighti(gc, light, pname, param);
}
GLvoid GLAPIENTRY __GLES_APINAME(Lightfv)( GLenum light, GLenum pname, const GLfloat *params )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Lightfv(gc, light, pname, params);
}
GLvoid GLAPIENTRY __GLES_APINAME(Lightiv)( GLenum light, GLenum pname, const GLint *params )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Lightiv(gc, light, pname, params);
}
GLvoid GLAPIENTRY __GLES_APINAME(LightModelf)( GLenum pname, GLfloat param )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->LightModelf(gc, pname, param);
}
GLvoid GLAPIENTRY __GLES_APINAME(LightModeli)( GLenum pname, GLint param )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->LightModeli(gc, pname, param);
}
GLvoid GLAPIENTRY __GLES_APINAME(LightModelfv)( GLenum pname, const GLfloat *params )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->LightModelfv(gc, pname, params);
}
GLvoid GLAPIENTRY __GLES_APINAME(LightModeliv)( GLenum pname, const GLint *params )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->LightModeliv(gc, pname, params);
}
GLvoid GLAPIENTRY __GLES_APINAME(LineStipple)( GLint factor, GLushort pattern )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->LineStipple(gc, factor, pattern);
}
GLvoid GLAPIENTRY __GLES_APINAME(Materialf)( GLenum face, GLenum pname, GLfloat param )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Materialf(gc, face, pname, param);
}
GLvoid GLAPIENTRY __GLES_APINAME(Materiali)( GLenum face, GLenum pname, GLint param )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Materiali(gc, face, pname, param);
}
GLvoid GLAPIENTRY __GLES_APINAME(Materialfv)( GLenum face, GLenum pname, const GLfloat *params )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Materialfv(gc, face, pname, params);
}
GLvoid GLAPIENTRY __GLES_APINAME(Materialiv)( GLenum face, GLenum pname, const GLint *params )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Materialiv(gc, face, pname, params);
}
GLvoid GLAPIENTRY __GLES_APINAME(PointSize)( GLfloat size )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->PointSize(gc, size);
}
GLvoid GLAPIENTRY __GLES_APINAME(PolygonMode)( GLenum face, GLenum mode )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->PolygonMode(gc, face, mode);
}
GLvoid GLAPIENTRY __GLES_APINAME(PolygonStipple)( const GLubyte *mask )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->PolygonStipple(gc, mask);
}
GLvoid GLAPIENTRY __GLES_APINAME(ShadeModel)( GLenum mode )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->ShadeModel(gc, mode);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexImage1D)( GLenum target, GLint level, GLint internalFormat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexImage1D(gc, target, level, internalFormat, width, border, format, type, pixels);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexEnvf)( GLenum target, GLenum pname, GLfloat param )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexEnvf(gc, target, pname, param);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexEnvi)( GLenum target, GLenum pname, GLint param )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexEnvi(gc, target, pname, param);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexEnvfv)( GLenum target, GLenum pname, const GLfloat *params )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexEnvfv(gc, target, pname, params);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexEnviv)( GLenum target, GLenum pname, const GLint *params )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexEnviv(gc, target, pname, params);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexGend)( GLenum coord, GLenum pname, GLdouble param )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexGend(gc, coord, pname, param);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexGenf)( GLenum coord, GLenum pname, GLfloat param )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexGenf(gc, coord, pname, param);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexGeni)( GLenum coord, GLenum pname, GLint param )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexGeni(gc, coord, pname, param);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexGendv)( GLenum coord, GLenum pname, const GLdouble *params )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexGendv(gc, coord, pname, params);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexGenfv)( GLenum coord, GLenum pname, const GLfloat *params )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexGenfv(gc, coord, pname, params);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexGeniv)( GLenum coord, GLenum pname, const GLint *params )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexGeniv(gc, coord, pname, params);
}
GLvoid GLAPIENTRY __GLES_APINAME(FeedbackBuffer)( GLsizei size, GLenum type, GLfloat *buffer )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->FeedbackBuffer(gc, size, type, buffer);
}
GLvoid GLAPIENTRY __GLES_APINAME(SelectBuffer)( GLsizei size, GLuint *buffer )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->SelectBuffer(gc, size, buffer);
}
GLint GLAPIENTRY __GLES_APINAME(RenderMode)( GLenum mode )
{
    __GL_GET_CONTEXT_RET(0);
    return gc->currentImmediateTable->RenderMode(gc, mode);
}
GLvoid GLAPIENTRY __GLES_APINAME(InitNames)( void )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->InitNames(gc);
}
GLvoid GLAPIENTRY __GLES_APINAME(LoadName)( GLuint name )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->LoadName(gc, name);
}
GLvoid GLAPIENTRY __GLES_APINAME(PushName)( GLuint name )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->PushName(gc, name);
}
GLvoid GLAPIENTRY __GLES_APINAME(PopName)( void )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->PopName(gc);
}
GLvoid GLAPIENTRY __GLES_APINAME(PassThrough)( GLfloat token )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->PassThrough(gc, token);
}
GLvoid GLAPIENTRY __GLES_APINAME(DrawBuffer)( GLenum mode )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->DrawBuffer(gc, mode);
}

GLvoid GLAPIENTRY __GLES_APINAME(ClearAccum)( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->ClearAccum(gc, red, green, blue, alpha);
}
GLvoid GLAPIENTRY __GLES_APINAME(ClearIndex)( GLfloat c )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->ClearIndex(gc, c);
}
GLvoid GLAPIENTRY __GLES_APINAME(ClearDepth)( GLclampd depth )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->ClearDepth(gc, depth);
}
GLvoid GLAPIENTRY __GLES_APINAME(IndexMask)( GLuint mask )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->IndexMask(gc, mask);
}
GLvoid GLAPIENTRY __GLES_APINAME(Accum)( GLenum op, GLfloat value )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Accum(gc, op, value);
}
GLvoid GLAPIENTRY __GLES_APINAME(PushAttrib)( GLbitfield mask )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->PushAttrib(gc, mask);
}
GLvoid GLAPIENTRY __GLES_APINAME(PopAttrib)( void )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->PopAttrib(gc);
}
GLvoid GLAPIENTRY __GLES_APINAME(Map1d)( GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Map1d(gc, target, u1, u2, stride, order, points);
}
GLvoid GLAPIENTRY __GLES_APINAME(Map1f)( GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Map1f(gc, target, u1, u2, stride, order, points);
}
GLvoid GLAPIENTRY __GLES_APINAME(Map2d)( GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Map2d(gc, target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
}
GLvoid GLAPIENTRY __GLES_APINAME(Map2f)( GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Map2f(gc, target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
}
GLvoid GLAPIENTRY __GLES_APINAME(MapGrid1d)( GLint un, GLdouble u1, GLdouble u2 )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MapGrid1d(gc, un, u1, u2);
}
GLvoid GLAPIENTRY __GLES_APINAME(MapGrid1f)( GLint un, GLfloat u1, GLfloat u2 )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MapGrid1f(gc, un, u1, u2);
}
GLvoid GLAPIENTRY __GLES_APINAME(MapGrid2d)( GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2 )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MapGrid2d(gc, un, u1, u2, vn, v1, v2);
}
GLvoid GLAPIENTRY __GLES_APINAME(MapGrid2f)( GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2 )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MapGrid2f(gc, un, u1, u2, vn, v1, v2);
}
GLvoid GLAPIENTRY __GLES_APINAME(EvalCoord1d)( GLdouble u )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->EvalCoord1d(gc, u);
}
GLvoid GLAPIENTRY __GLES_APINAME(EvalCoord1f)( GLfloat u )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->EvalCoord1f(gc, u);
}
GLvoid GLAPIENTRY __GLES_APINAME(EvalCoord1dv)( const GLdouble *u )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->EvalCoord1dv(gc, u);
}
GLvoid GLAPIENTRY __GLES_APINAME(EvalCoord1fv)( const GLfloat *u )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->EvalCoord1fv(gc, u);
}
GLvoid GLAPIENTRY __GLES_APINAME(EvalCoord2d)( GLdouble u, GLdouble v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->EvalCoord2d(gc, u, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(EvalCoord2f)( GLfloat u, GLfloat v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->EvalCoord2f(gc, u, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(EvalCoord2dv)( const GLdouble *u )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->EvalCoord2dv(gc, u);
}
GLvoid GLAPIENTRY __GLES_APINAME(EvalCoord2fv)( const GLfloat *u )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->EvalCoord2fv(gc, u);
}
GLvoid GLAPIENTRY __GLES_APINAME(EvalPoint1)( GLint i )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->EvalPoint1(gc, i);
}
GLvoid GLAPIENTRY __GLES_APINAME(EvalPoint2)( GLint i, GLint j )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->EvalPoint2(gc, i, j);
}
GLvoid GLAPIENTRY __GLES_APINAME(EvalMesh1)( GLenum mode, GLint i1, GLint i2 )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->EvalMesh1(gc, mode, i1, i2);
}
GLvoid GLAPIENTRY __GLES_APINAME(EvalMesh2)( GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2 )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->EvalMesh2(gc, mode, i1, i2, j1, j2);
}
GLvoid GLAPIENTRY __GLES_APINAME(AlphaFunc)( GLenum func, GLclampf ref )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->AlphaFunc(gc, func, ref);
}
GLvoid GLAPIENTRY __GLES_APINAME(LogicOp)( GLenum opcode )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->LogicOp(gc, opcode);
}
GLvoid GLAPIENTRY __GLES_APINAME(PixelZoom)( GLfloat xfactor, GLfloat yfactor ){}
GLvoid GLAPIENTRY __GLES_APINAME(PixelTransferf)( GLenum pname, GLfloat param ){}
GLvoid GLAPIENTRY __GLES_APINAME(PixelTransferi)( GLenum pname, GLint param ){}
GLvoid GLAPIENTRY __GLES_APINAME(PixelStoref)( GLenum pname, GLfloat param ){}
GLvoid GLAPIENTRY __GLES_APINAME(PixelMapfv)( GLenum map, GLsizei mapsize, const GLfloat *values ){}
GLvoid GLAPIENTRY __GLES_APINAME(PixelMapuiv)( GLenum map, GLsizei mapsize, const GLuint *values ){}
GLvoid GLAPIENTRY __GLES_APINAME(PixelMapusv)( GLenum map, GLsizei mapsize, const GLushort *values ){}
GLvoid GLAPIENTRY __GLES_APINAME(GetClipPlane)( GLenum plane, GLdouble *equation )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->GetClipPlane(gc, plane, equation);
}
GLvoid GLAPIENTRY __GLES_APINAME(GetDoublev)( GLenum pname, GLdouble *params ){}
GLvoid GLAPIENTRY __GLES_APINAME(GetLightfv)( GLenum light, GLenum pname, GLfloat *params )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->GetLightfv(gc, light, pname, params);
}
GLvoid GLAPIENTRY __GLES_APINAME(GetLightiv)( GLenum light, GLenum pname, GLint *params )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->GetLightiv(gc, light, pname, params);
}
GLvoid GLAPIENTRY __GLES_APINAME(GetMapdv)( GLenum target, GLenum query, GLdouble *v ){}
GLvoid GLAPIENTRY __GLES_APINAME(GetMapfv)( GLenum target, GLenum query, GLfloat *v ){}
GLvoid GLAPIENTRY __GLES_APINAME(GetMapiv)( GLenum target, GLenum query, GLint *v ){}
GLvoid GLAPIENTRY __GLES_APINAME(GetMaterialfv)( GLenum face, GLenum pname, GLfloat *params )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->GetMaterialfv(gc, face, pname, params);
}
GLvoid GLAPIENTRY __GLES_APINAME(GetMaterialiv)( GLenum face, GLenum pname, GLint *params )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->GetMaterialiv(gc, face, pname, params);
}
GLvoid GLAPIENTRY __GLES_APINAME(GetPixelMapfv)( GLenum map, GLfloat *values ){}
GLvoid GLAPIENTRY __GLES_APINAME(GetPixelMapuiv)( GLenum map, GLuint *values ){}
GLvoid GLAPIENTRY __GLES_APINAME(GetPixelMapusv)( GLenum map, GLushort *values ){}
GLvoid GLAPIENTRY __GLES_APINAME(GetPolygonStipple)( GLubyte *mask )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->GetPolygonStipple(gc, mask);
}
GLvoid GLAPIENTRY __GLES_APINAME(GetTexEnvfv)( GLenum target, GLenum pname, GLfloat *params )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->GetTexEnvfv(gc, target, pname, params);
}
GLvoid GLAPIENTRY __GLES_APINAME(GetTexEnviv)( GLenum target, GLenum pname, GLint *params )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->GetTexEnviv(gc, target, pname, params);
}
GLvoid GLAPIENTRY __GLES_APINAME(GetTexGendv)( GLenum coord, GLenum pname, GLdouble *params )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->GetTexGendv(gc, coord, pname, params);
}
GLvoid GLAPIENTRY __GLES_APINAME(GetTexGenfv)( GLenum coord, GLenum pname, GLfloat *params )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->GetTexGenfv(gc, coord, pname, params);
}
GLvoid GLAPIENTRY __GLES_APINAME(GetTexGeniv)( GLenum coord, GLenum pname, GLint *params )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->GetTexGeniv(gc, coord, pname, params);
}
GLvoid GLAPIENTRY __GLES_APINAME(GetTexImage)( GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels ){}
GLvoid GLAPIENTRY __GLES_APINAME(DepthRange)( GLclampd near_val, GLclampd far_val )
{
     __GL_GET_CONTEXT;
    gc->currentImmediateTable->DepthRange(gc, near_val, far_val);
}
GLvoid GLAPIENTRY __GLES_APINAME(Frustum)( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Frustum(gc, left, right, bottom, top, near_val, far_val);
}
GLvoid GLAPIENTRY __GLES_APINAME(LoadIdentity)( void )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->LoadIdentity(gc);
}
GLvoid GLAPIENTRY __GLES_APINAME(LoadMatrixd)( const GLdouble *m )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->LoadMatrixd(gc, m);
}
GLvoid GLAPIENTRY __GLES_APINAME(LoadMatrixf)( const GLfloat *m )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->LoadMatrixf(gc, m);
}
GLvoid GLAPIENTRY __GLES_APINAME(MatrixMode)( GLenum mode )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MatrixMode(gc, mode);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultMatrixd)( const GLdouble *m )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultMatrixd(gc, m);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultMatrixf)( const GLfloat *m )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultMatrixf(gc, m);
}
GLvoid GLAPIENTRY __GLES_APINAME(Ortho)( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Ortho(gc, left, right, bottom, top, near_val, far_val);
}
GLvoid GLAPIENTRY __GLES_APINAME(PushMatrix)( void )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->PushMatrix(gc);
}
GLvoid GLAPIENTRY __GLES_APINAME(PopMatrix)( void )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->PopMatrix(gc);
}
GLvoid GLAPIENTRY __GLES_APINAME(Rotated)( GLdouble angle, GLdouble x, GLdouble y, GLdouble z )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Rotated(gc, angle, x, y, z);
}
GLvoid GLAPIENTRY __GLES_APINAME(Rotatef)( GLfloat angle, GLfloat x, GLfloat y, GLfloat z )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Rotatef(gc, angle, x, y, z);
}
GLvoid GLAPIENTRY __GLES_APINAME(Scaled)( GLdouble x, GLdouble y, GLdouble z )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Scaled(gc, x, y, z);
}
GLvoid GLAPIENTRY __GLES_APINAME(Scalef)( GLfloat x, GLfloat y, GLfloat z )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Scalef(gc, x, y, z);
}
GLvoid GLAPIENTRY __GLES_APINAME(Translated)( GLdouble x, GLdouble y, GLdouble z )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Translated(gc, x, y, z);
}
GLvoid GLAPIENTRY __GLES_APINAME(Translatef)( GLfloat x, GLfloat y, GLfloat z )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Translatef(gc, x, y, z);
}
GLvoid GLAPIENTRY __GLES_APINAME(ArrayElement)( GLint i )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->ArrayElement(gc, i);
}
GLvoid GLAPIENTRY __GLES_APINAME(ColorPointer)( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->ColorPointer(gc, size, type, stride, ptr);
}
GLvoid GLAPIENTRY __GLES_APINAME(EnableClientState)( GLenum cap )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->EnableClientState(gc, cap);
}
GLvoid GLAPIENTRY __GLES_APINAME(DisableClientState)( GLenum cap )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->DisableClientState(gc, cap);
}
GLvoid GLAPIENTRY __GLES_APINAME(EdgeFlagPointer)( GLsizei stride, const GLvoid *ptr )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->EdgeFlagPointer(gc, stride, ptr);
}
GLvoid GLAPIENTRY __GLES_APINAME(IndexPointer)( GLenum type, GLsizei stride, const GLvoid *ptr )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->IndexPointer(gc, type, stride, ptr);
}
GLvoid GLAPIENTRY __GLES_APINAME(InterleavedArrays)( GLenum format, GLsizei stride, const GLvoid *pointer )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->InterleavedArrays(gc, format, stride, pointer);
}
GLvoid GLAPIENTRY __GLES_APINAME(NormalPointer)( GLenum type, GLsizei stride,  const GLvoid *ptr )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->NormalPointer(gc, type, stride, ptr);
}
GLvoid GLAPIENTRY __GLES_APINAME(VertexPointer)( GLint size, GLenum type, GLsizei stride, const GLvoid *ptr )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->VertexPointer(gc, size, type, stride, ptr);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexCoordPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *ptr)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexCoordPointer(gc, size, type, stride, ptr);
}
GLboolean GLAPIENTRY __GLES_APINAME(AreTexturesResident)( GLsizei n, const GLuint *textures, GLboolean *residences )
{
    __GL_GET_CONTEXT_RET(0);
    return gc->currentImmediateTable->AreTexturesResident(gc, n, textures, residences);
}
GLvoid GLAPIENTRY __GLES_APINAME(CopyTexImage1D)( GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->CopyTexImage1D(gc, target, level, internalformat, x, y, width, border);
}
GLvoid GLAPIENTRY __GLES_APINAME(CopyTexSubImage1D)( GLenum target, GLint level,  GLint xoffset, GLint x, GLint y,  GLsizei width )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->CopyTexSubImage1D(gc, target, level, xoffset, x, y, width);
}
GLvoid GLAPIENTRY __GLES_APINAME(PrioritizeTextures)( GLsizei n,  const GLuint *textures, const GLclampf *priorities )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->PrioritizeTextures(gc, n, textures, priorities);
}
GLvoid GLAPIENTRY __GLES_APINAME(TexSubImage1D)( GLenum target, GLint level,  GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->TexSubImage1D(gc, target, level, xoffset, width, format, type, pixels);
}
GLvoid GLAPIENTRY __GLES_APINAME(PushClientAttrib)( GLbitfield mask )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->PushClientAttrib(gc, mask);
}
GLvoid GLAPIENTRY __GLES_APINAME(PopClientAttrib)( void )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->PopClientAttrib(gc);
}
GLvoid GLAPIENTRY __GLES_APINAME(ColorTable)( GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table ){}
GLvoid GLAPIENTRY __GLES_APINAME(ColorTableParameteriv)(GLenum target, GLenum pname, const GLint *params){}
GLvoid GLAPIENTRY __GLES_APINAME(ColorTableParameterfv)(GLenum target, GLenum pname, const GLfloat *params){}
GLvoid GLAPIENTRY __GLES_APINAME(CopyColorTable)( GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width ){}
GLvoid GLAPIENTRY __GLES_APINAME(GetColorTable)( GLenum target, GLenum format,  GLenum type, GLvoid *table ){}
GLvoid GLAPIENTRY __GLES_APINAME(GetColorTableParameterfv)( GLenum target, GLenum pname, GLfloat *params ){}
GLvoid GLAPIENTRY __GLES_APINAME(GetColorTableParameteriv)( GLenum target, GLenum pname,  GLint *params ){}
GLvoid GLAPIENTRY __GLES_APINAME(ColorSubTable)( GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data ){}
GLvoid GLAPIENTRY __GLES_APINAME(CopyColorSubTable)( GLenum target, GLsizei start, GLint x, GLint y, GLsizei width ){}
GLvoid GLAPIENTRY __GLES_APINAME(ConvolutionFilter1D)( GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *image ){}
GLvoid GLAPIENTRY __GLES_APINAME(ConvolutionFilter2D)( GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image ){}
GLvoid GLAPIENTRY __GLES_APINAME(ConvolutionParameterf)( GLenum target, GLenum pname, GLfloat params ){}
GLvoid GLAPIENTRY __GLES_APINAME(ConvolutionParameterfv)( GLenum target, GLenum pname, const GLfloat *params ){}
GLvoid GLAPIENTRY __GLES_APINAME(ConvolutionParameteri)( GLenum target, GLenum pname, GLint params ){}
GLvoid GLAPIENTRY __GLES_APINAME(ConvolutionParameteriv)( GLenum target, GLenum pname, const GLint *params ){}
GLvoid GLAPIENTRY __GLES_APINAME(CopyConvolutionFilter1D)( GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width ){}
GLvoid GLAPIENTRY __GLES_APINAME(CopyConvolutionFilter2D)( GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height){}
GLvoid GLAPIENTRY __GLES_APINAME(GetConvolutionFilter)( GLenum target, GLenum format, GLenum type, GLvoid *image ){}
GLvoid GLAPIENTRY __GLES_APINAME(GetConvolutionParameterfv)( GLenum target, GLenum pname, GLfloat *params ){}
GLvoid GLAPIENTRY __GLES_APINAME(GetConvolutionParameteriv)( GLenum target, GLenum pname, GLint *params ){}
GLvoid GLAPIENTRY __GLES_APINAME(SeparableFilter2D)( GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *row, const GLvoid *column ){}
GLvoid GLAPIENTRY __GLES_APINAME(GetSeparableFilter)( GLenum target, GLenum format, GLenum type, GLvoid *row, GLvoid *column, GLvoid *span ){}
GLvoid GLAPIENTRY __GLES_APINAME(Histogram)( GLenum target, GLsizei width, GLenum internalformat, GLboolean sink ){}
GLvoid GLAPIENTRY __GLES_APINAME(ResetHistogram)( GLenum target ){}
GLvoid GLAPIENTRY __GLES_APINAME(GetHistogram)( GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values ){}
GLvoid GLAPIENTRY __GLES_APINAME(GetHistogramParameterfv)( GLenum target, GLenum pname, GLfloat *params ){}
GLvoid GLAPIENTRY __GLES_APINAME(GetHistogramParameteriv)( GLenum target, GLenum pname, GLint *params ){}
GLvoid GLAPIENTRY __GLES_APINAME(Minmax)( GLenum target, GLenum internalformat, GLboolean sink ){}
GLvoid GLAPIENTRY __GLES_APINAME(ResetMinmax)( GLenum target ){}
GLvoid GLAPIENTRY __GLES_APINAME(GetMinmax)( GLenum target, GLboolean reset, GLenum format, GLenum types, GLvoid *values ){}
GLvoid GLAPIENTRY __GLES_APINAME(GetMinmaxParameterfv)( GLenum target, GLenum pname,  GLfloat *params ){}
GLvoid GLAPIENTRY __GLES_APINAME(GetMinmaxParameteriv)( GLenum target, GLenum pname, GLint *params ){}
GLvoid GLAPIENTRY __GLES_APINAME(ClientActiveTexture)( GLenum texture )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->ClientActiveTexture(gc, texture);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord1d)( GLenum target, GLdouble s )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord1d(gc, target, s);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord1dv)( GLenum target, const GLdouble *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord1dv(gc, target, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord1f)( GLenum target, GLfloat s )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord1f(gc, target, s);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord1fv)( GLenum target, const GLfloat *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord1fv(gc, target, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord1i)( GLenum target, GLint s )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord1i(gc, target, s);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord1iv)( GLenum target, const GLint *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord1iv(gc, target, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord1s)( GLenum target, GLshort s )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord1s(gc, target, s);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord1sv)( GLenum target, const GLshort *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord1sv(gc, target, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord2d)( GLenum target, GLdouble s, GLdouble t )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord2d(gc, target, s, t);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord2dv)( GLenum target, const GLdouble *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord2dv(gc, target, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord2f)( GLenum target, GLfloat s, GLfloat t )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord2f(gc, target, s, t);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord2fv)( GLenum target, const GLfloat *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord2fv(gc, target, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord2i)( GLenum target, GLint s, GLint t )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord2i(gc, target, s, t);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord2iv)( GLenum target, const GLint *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord2iv(gc, target, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord2s)( GLenum target, GLshort s, GLshort t )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord2s(gc, target, s, t);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord2sv)( GLenum target, const GLshort *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord2sv(gc, target, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord3d)( GLenum target, GLdouble s, GLdouble t, GLdouble r )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord3d(gc, target, s, t, r);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord3dv)( GLenum target, const GLdouble *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord3dv(gc, target, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord3f)( GLenum target, GLfloat s, GLfloat t, GLfloat r )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord3f(gc, target, s, t, r);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord3fv)( GLenum target, const GLfloat *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord3fv(gc, target, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord3i)( GLenum target, GLint s, GLint t, GLint r )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord3i(gc, target, s, t, r);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord3iv)( GLenum target, const GLint *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord3iv(gc, target, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord3s)( GLenum target, GLshort s, GLshort t, GLshort r )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord3s(gc, target, s, t, r);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord3sv)( GLenum target, const GLshort *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord3sv(gc, target, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord4d)( GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord4d(gc, target, s, t, r, q);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord4dv)( GLenum target, const GLdouble *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord4dv(gc, target, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord4f)( GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord4f(gc, target, s, t, r, q);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord4fv)( GLenum target, const GLfloat *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord4fv(gc, target, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord4i)( GLenum target, GLint s, GLint t, GLint r, GLint q )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord4i(gc, target, s, t, r, q);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord4iv)( GLenum target, const GLint *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord4iv(gc, target, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord4s)( GLenum target, GLshort s, GLshort t, GLshort r, GLshort q )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord4s(gc, target, s, t, r, q);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord4sv)( GLenum target, const GLshort *v )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord4sv(gc, target, v);
}

GLvoid GLAPIENTRY __GLES_APINAME(LoadTransposeMatrixd)( const GLdouble m[16] )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->LoadTransposeMatrixd(gc, m);
}
GLvoid GLAPIENTRY __GLES_APINAME(LoadTransposeMatrixf)( const GLfloat m[16] )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->LoadTransposeMatrixf(gc, m);
}

GLvoid GLAPIENTRY __GLES_APINAME(MultTransposeMatrixd)( const GLdouble m[16] )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultTransposeMatrixd(gc, m);
}

GLvoid GLAPIENTRY __GLES_APINAME(MultTransposeMatrixf)( const GLfloat m[16] )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultTransposeMatrixf(gc, m);
}

GLvoid GLAPIENTRY __GLES_APINAME(CompressedTexImage1D)( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->CompressedTexImage1D(gc, target, level, internalformat, width, border, imageSize, data);
}
GLvoid GLAPIENTRY __GLES_APINAME(CompressedTexSubImage1D)( GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->CompressedTexSubImage1D(gc, target, level, xoffset, width, format, imageSize, data);
}
GLvoid GLAPIENTRY __GLES_APINAME(GetCompressedTexImage)( GLenum target, GLint lod, GLvoid *img ){}
GLvoid GLAPIENTRY __GLES_APINAME(FogCoordf)(GLfloat coord)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->FogCoordf(gc, coord);
}
GLvoid GLAPIENTRY __GLES_APINAME(FogCoordfv)(const GLfloat *coord)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->FogCoordfv(gc, coord);
}
GLvoid GLAPIENTRY __GLES_APINAME(FogCoordd)(GLdouble coord)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->FogCoordd(gc, coord);
}
GLvoid GLAPIENTRY __GLES_APINAME(FogCoorddv)(const GLdouble *coord)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->FogCoorddv(gc, coord);
}
GLvoid GLAPIENTRY __GLES_APINAME(FogCoordPointer)(GLenum type, GLsizei stride, const void *pointer)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->FogCoordPointer(gc, type, stride, pointer);
}
GLvoid GLAPIENTRY __GLES_APINAME(PointParameterf)(GLenum pname, GLfloat param)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->PointParameterf(gc, pname, param);
}
GLvoid GLAPIENTRY __GLES_APINAME(PointParameterfv)(GLenum pname, const GLfloat *params)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->PointParameterfv(gc, pname, params);
}
GLvoid GLAPIENTRY __GLES_APINAME(PointParameteri)(GLenum pname, GLint param)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->PointParameteri(gc, pname, param);
}
GLvoid GLAPIENTRY __GLES_APINAME(PointParameteriv)(GLenum pname, const GLint *params)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->PointParameteriv(gc, pname, params);
}
GLvoid GLAPIENTRY __GLES_APINAME(SecondaryColor3b)(GLbyte red, GLbyte green, GLbyte blue)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->SecondaryColor3b(gc, red, green, blue);
}
GLvoid GLAPIENTRY __GLES_APINAME(SecondaryColor3bv)(const GLbyte *v)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->SecondaryColor3bv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(SecondaryColor3d)(GLdouble red, GLdouble green, GLdouble blue)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->SecondaryColor3d(gc, red, green, blue);
}
GLvoid GLAPIENTRY __GLES_APINAME(SecondaryColor3dv)(const GLdouble *v)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->SecondaryColor3dv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(SecondaryColor3f)(GLfloat red, GLfloat green, GLfloat blue)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->SecondaryColor3f(gc, red, green, blue);
}
GLvoid GLAPIENTRY __GLES_APINAME(SecondaryColor3fv)(const GLfloat *v)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->SecondaryColor3fv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(SecondaryColor3i)(GLint red, GLint green, GLint blue)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->SecondaryColor3i(gc, red, green, blue);
}
GLvoid GLAPIENTRY __GLES_APINAME(SecondaryColor3iv)(const GLint *v)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->SecondaryColor3iv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(SecondaryColor3s)(GLshort red, GLshort green, GLshort blue)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->SecondaryColor3s(gc, red, green, blue);
}
GLvoid GLAPIENTRY __GLES_APINAME(SecondaryColor3sv)(const GLshort *v)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->SecondaryColor3sv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(SecondaryColor3ub)(GLubyte red, GLubyte green, GLubyte blue)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->SecondaryColor3ub(gc, red, green, blue);
}
GLvoid GLAPIENTRY __GLES_APINAME(SecondaryColor3ubv)(const GLubyte *v)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->SecondaryColor3ubv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(SecondaryColor3ui)(GLuint red, GLuint green, GLuint blue)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->SecondaryColor3ui(gc, red, green, blue);
}
GLvoid GLAPIENTRY __GLES_APINAME(SecondaryColor3uiv)(const GLuint *v)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->SecondaryColor3uiv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(SecondaryColor3us)(GLushort red, GLushort green, GLushort blue)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->SecondaryColor3us(gc, red, green, blue);
}
GLvoid GLAPIENTRY __GLES_APINAME(SecondaryColor3usv)(const GLushort *v)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->SecondaryColor3usv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(SecondaryColorPointer)(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->SecondaryColorPointer(gc, size, type, stride, pointer);
}
GLvoid GLAPIENTRY __GLES_APINAME(WindowPos2d)(GLdouble x, GLdouble y)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->WindowPos2d(gc, x, y);
}
GLvoid GLAPIENTRY __GLES_APINAME(WindowPos2dv)(const GLdouble *v)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->WindowPos2dv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(WindowPos2f)(GLfloat x, GLfloat y)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->WindowPos2f(gc, x, y);
}
GLvoid GLAPIENTRY __GLES_APINAME(WindowPos2fv)(const GLfloat *v)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->WindowPos2fv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(WindowPos2i)(GLint x, GLint y)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->WindowPos2i(gc, x, y);
}
GLvoid GLAPIENTRY __GLES_APINAME(WindowPos2iv)(const GLint *v)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->WindowPos2iv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(WindowPos2s)(GLshort x, GLshort y)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->WindowPos2s(gc, x, y);
}
GLvoid GLAPIENTRY __GLES_APINAME(WindowPos2sv)(const GLshort *v)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->WindowPos2sv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(WindowPos3d)(GLdouble x, GLdouble y, GLdouble z)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->WindowPos3d(gc, x, y, z);
}
GLvoid GLAPIENTRY __GLES_APINAME(WindowPos3dv)(const GLdouble *v)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->WindowPos3dv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(WindowPos3f)(GLfloat x, GLfloat y, GLfloat z)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->WindowPos3f(gc, x, y, z);
}
GLvoid GLAPIENTRY __GLES_APINAME(WindowPos3fv)(const GLfloat *v)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->WindowPos3fv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(WindowPos3i)(GLint x, GLint y, GLint z)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->WindowPos3i(gc, x, y, z);
}
GLvoid GLAPIENTRY __GLES_APINAME(WindowPos3iv)(const GLint *v)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->WindowPos3iv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(WindowPos3s)(GLshort x, GLshort y, GLshort z)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->WindowPos3s(gc, x, y, z);
}
GLvoid GLAPIENTRY __GLES_APINAME(WindowPos3sv)(const GLshort *v)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->WindowPos3sv(gc, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(DrawPixels)( GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->DrawPixels(gc, width, height, format, type, pixels);
}
GLvoid GLAPIENTRY __GLES_APINAME(CopyPixels)( GLint x, GLint y,  GLsizei width, GLsizei height, GLenum type )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->CopyPixels(gc, x, y, width, height, type);
}
GLvoid GLAPIENTRY __GLES_APINAME(Indexub)( GLubyte c )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Indexub(gc, c);
}
GLvoid GLAPIENTRY __GLES_APINAME(Indexubv)( const GLubyte *c )
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->Indexubv(gc, c);
}
GLvoid GLAPIENTRY __GLES_APINAME(GetVertexAttribdv) (GLuint index, GLenum pname, GLdouble* params){}
GLvoid GLAPIENTRY __GLES_APINAME(GetQueryObjectiv) (GLuint id, GLenum pname, GLint* params){}
GLvoid GLAPIENTRY __GLES_APINAME(GetBufferSubData) (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data){}
GLvoid GLAPIENTRY __GLES_APINAME(DeleteObjectARB) (UINT obj){}
GLvoid GLAPIENTRY __GLES_APINAME(GetInfoLogARB) (UINT obj, GLsizei maxLength, GLsizei *length, char *infoLog){}

/*
** OpenGL ES Extensions
*/

#if GL_OES_EGL_image
GLvoid GLAPIENTRY __GLES_APINAME(EGLImageTargetTexture2DOES)(GLenum target, GLeglImageOES image)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.EGLImageTargetTexture2DOES(gc, target, image);
}

GLvoid GLAPIENTRY __GLES_APINAME(EGLImageTargetRenderbufferStorageOES)(GLenum target, GLeglImageOES image)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.EGLImageTargetRenderbufferStorageOES(gc, target, image);
}
#endif

#if GL_EXT_multi_draw_arrays
GLvoid GLAPIENTRY __GLES_APINAME(MultiDrawArraysEXT)(GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.MultiDrawArraysEXT(gc, mode, first, count, primcount);
}

GLvoid GLAPIENTRY __GLES_APINAME(MultiDrawElementsEXT)(GLenum mode, const GLsizei *count, GLenum type, const GLvoid*const*indices, GLsizei primcount)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.MultiDrawElementsEXT(gc, mode, count, type, indices, primcount);
}

#if GL_EXT_draw_elements_base_vertex
GLvoid GLAPIENTRY __GLES_APINAME(MultiDrawElementsBaseVertexEXT)(GLenum mode, const GLsizei * count, GLenum type, const void *const*indices, GLsizei drawcount, const GLint *basevertex)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.MultiDrawElementsBaseVertexEXT(gc, mode, count, type, indices, drawcount, basevertex);
}
#endif
#endif

#if GL_OES_mapbuffer
GLvoid GLAPIENTRY __GLES_APINAME(GetBufferPointervOES)(GLenum target, GLenum pname, GLvoid** params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetBufferPointervOES(gc, target, pname, params);
}

GLvoid* GLAPIENTRY __GLES_APINAME(MapBufferOES)(GLenum target, GLenum access)
{
    __GL_GET_CONTEXT_RET(gcvNULL);
    return gc->apiDispatchTable.MapBufferOES(gc, target, access);
}

GLboolean GLAPIENTRY __GLES_APINAME(UnmapBufferOES)(GLenum target)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->apiDispatchTable.UnmapBufferOES(gc, target);
}
#endif

#if GL_EXT_discard_framebuffer
GLvoid GLAPIENTRY __GLES_APINAME(DiscardFramebufferEXT)(GLenum target, GLsizei numAttachments, const GLenum *attachments)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DiscardFramebufferEXT(gc, target, numAttachments, attachments);
}
#endif

#if GL_EXT_multisampled_render_to_texture
GLvoid GLAPIENTRY __GLES_APINAME(FramebufferTexture2DMultisampleEXT)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.FramebufferTexture2DMultisampleEXT(gc, target, attachment, textarget, texture, level, samples);
}

GLvoid GLAPIENTRY __GLES_APINAME(RenderbufferStorageMultisampleEXT)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.RenderbufferStorageMultisampleEXT(gc, target, samples, internalformat, width, height);
}
#endif



#if GL_VIV_direct_texture
GLvoid GLAPIENTRY __GLES_APINAME(TexDirectVIV)(GLenum target, GLsizei width, GLsizei height, GLenum format, GLvoid ** pixels)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexDirectVIV(gc, target, width, height, format, pixels);
}

GLvoid GLAPIENTRY __GLES_APINAME(TexDirectInvalidateVIV)(GLenum target)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexDirectInvalidateVIV(gc, target);
}

GLvoid GLAPIENTRY __GLES_APINAME(TexDirectVIVMap)(GLenum target, GLsizei width, GLsizei height, GLenum format, GLvoid ** logical, const GLuint * physical)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexDirectVIVMap(gc, target, width, height, format, logical, physical);
}

GLvoid GLAPIENTRY __GLES_APINAME(TexDirectTiledMapVIV)(GLenum target, GLsizei width, GLsizei height, GLenum format, GLvoid ** logical, const GLuint * physical)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.TexDirectTiledMapVIV(gc, target, width, height, format, logical, physical);
}
#endif

#if GL_OES_get_program_binary
GLvoid GLAPIENTRY __GLES_APINAME(GetProgramBinaryOES)(GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, GLvoid* binary)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetProgramBinary(gc, program, bufSize, length, binaryFormat, binary);
}

GLvoid GLAPIENTRY __GLES_APINAME(ProgramBinaryOES)(GLuint program, GLenum binaryFormat, const GLvoid* binary, GLsizei length)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ProgramBinary(gc, program, binaryFormat, binary, length);
}
#endif

#if GL_OES_vertex_array_object
GLvoid GLAPIENTRY __GLES_APINAME(BindVertexArrayOES)(GLuint array)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BindVertexArray(gc, array);
}

GLvoid GLAPIENTRY __GLES_APINAME(DeleteVertexArraysOES)(GLsizei n, const GLuint *arrays)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DeleteVertexArrays(gc, n , arrays);
}

GLvoid GLAPIENTRY __GLES_APINAME(GenVertexArraysOES)(GLsizei n, GLuint *arrays)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GenVertexArrays(gc, n , arrays);
}

GLboolean GLAPIENTRY __GLES_APINAME(IsVertexArrayOES)(GLuint array)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->apiDispatchTable.IsVertexArray(gc, array);
}
#endif

#if GL_EXT_multi_draw_indirect
GLvoid GLAPIENTRY __GLES_APINAME(MultiDrawArraysIndirectEXT)(GLenum mode, const void *indirect, GLsizei drawcount, GLsizei stride)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.MultiDrawArraysIndirectEXT(gc, mode, indirect, drawcount, stride);
}

GLvoid GLAPIENTRY __GLES_APINAME(MultiDrawElementsIndirectEXT)(GLenum mode, GLenum type, const void *indirect, GLsizei drawcount, GLsizei stride)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.MultiDrawElementsIndirectEXT(gc, mode, type, indirect, drawcount, stride);
}
#endif

#if GL_KHR_blend_equation_advanced
GLvoid GLAPIENTRY __GLES_APINAME(BlendBarrierKHR)(void)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BlendBarrier(gc);
}
#endif

#if GL_EXT_robustness
GLenum GLAPIENTRY __GLES_APINAME(GetGraphicsResetStatusEXT)()
{
    __GL_GET_CONTEXT_RET(GL_NO_ERROR);
    return gc->apiDispatchTable.GetGraphicsResetStatus(gc);
}

GLvoid GLAPIENTRY __GLES_APINAME(ReadnPixelsEXT)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, GLvoid *data)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ReadnPixels(gc, x, y, width, height, format, type, bufSize, data);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetnUniformfvEXT)(GLuint program, GLint location, GLsizei bufSize, GLfloat *params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetnUniformfv(gc, program, location, bufSize, params);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetnUniformivEXT)(GLuint program, GLint location, GLsizei bufSize, GLint *params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetnUniformiv(gc, program, location, bufSize, params);
}
#endif

GLvoid GLAPIENTRY __GLES_APINAME(GetObjectParameterivARB) (UINT obj, GLenum pname, GLint *params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetObjectParameterivARB(gc, obj, pname, params);
}

GLvoid GLAPIENTRY __GLES_APINAME(ClampColorARB) (GLenum target, GLenum clamp)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ClampColorARB(gc, target, clamp);
}

GLvoid GLAPIENTRY __GLES_APINAME(FramebufferRenderbufferEXT)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->FramebufferRenderbufferEXT(gc, target, attachment, renderbuffertarget, renderbuffer);
}

GLenum GLAPIENTRY __GLES_APINAME(CheckFramebufferStatusEXT)(GLenum target)
{
    __GL_GET_CONTEXT_RET(GL_NO_ERROR);
    return gc->currentImmediateTable->CheckFramebufferStatusEXT(gc, target);
}

GLboolean GLAPIENTRY __GLES_APINAME(IsRenderbufferEXT)(GLuint renderbuffer)
{
    __GL_GET_CONTEXT_RET(GL_NO_ERROR);
    return gc->currentImmediateTable->IsRenderbufferEXT(gc, renderbuffer);
}
GLvoid GLAPIENTRY __GLES_APINAME(BindRenderbufferEXT)(GLenum target, GLuint renderbuffer)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->BindRenderbufferEXT(gc, target, renderbuffer);
}
GLvoid GLAPIENTRY __GLES_APINAME(DeleteRenderbuffersEXT)(GLsizei n, const GLuint *renderbuffers)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->DeleteRenderbuffersEXT(gc, n, renderbuffers);
}
GLvoid GLAPIENTRY __GLES_APINAME(GenRenderbuffersEXT)(GLsizei n, GLuint *renderbuffers)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->GenRenderbuffersEXT(gc, n, renderbuffers);
}
GLvoid GLAPIENTRY __GLES_APINAME(RenderbufferStorageEXT)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->RenderbufferStorageEXT(gc, target, internalformat, width, height);
}
GLvoid GLAPIENTRY __GLES_APINAME(GetRenderbufferParameterivEXT)(GLenum target, GLenum pname, GLint* params)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->GetRenderbufferParameterivEXT(gc, target, pname, params);
}
GLboolean GLAPIENTRY __GLES_APINAME(IsFramebufferEXT)(GLuint framebuffer)
{
    __GL_GET_CONTEXT_RET(GL_NO_ERROR);
    return gc->currentImmediateTable->IsFramebufferEXT(gc, framebuffer);
}
GLvoid GLAPIENTRY __GLES_APINAME(BindFramebufferEXT)(GLenum target, GLuint framebuffer)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->BindFramebufferEXT(gc, target, framebuffer);
}
GLvoid GLAPIENTRY __GLES_APINAME(DeleteFramebuffersEXT)(GLsizei n, const GLuint *framebuffers)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->DeleteFramebuffersEXT(gc, n, framebuffers);
}
GLvoid GLAPIENTRY __GLES_APINAME(GenFramebuffersEXT)(GLsizei n, GLuint *framebuffers)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->GenFramebuffersEXT(gc, n, framebuffers);
}

GLvoid GLAPIENTRY __GLES_APINAME(FramebufferTexture1DEXT)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->FramebufferTexture1DEXT(gc, target, attachment, textarget, texture, level);
}
GLvoid GLAPIENTRY __GLES_APINAME(FramebufferTexture2DEXT)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->FramebufferTexture2DEXT(gc, target, attachment, textarget, texture, level);
}
GLvoid GLAPIENTRY __GLES_APINAME(FramebufferTexture3DEXT)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->FramebufferTexture3DEXT(gc, target, attachment, textarget, texture, level, zoffset);
}

GLvoid GLAPIENTRY __GLES_APINAME(GetFramebufferAttachmentParameterivEXT)(GLenum target, GLenum attachment, GLenum pname, GLint *params)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->GetFramebufferAttachmentParameterivEXT(gc, target, attachment, pname, params);
}
GLvoid GLAPIENTRY __GLES_APINAME(GenerateMipmapEXT)(GLenum target)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->GenerateMipmapEXT(gc, target);
}
GLvoid GLAPIENTRY __GLES_APINAME(BlitFramebufferEXT)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,GLbitfield mask, GLenum filter)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->BlitFramebufferEXT(gc, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
}

#if GL_ARB_multitexture
GLvoid GLAPIENTRY __GLES_APINAME(ActiveTextureARB)(GLenum texture)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.ActiveTexture(gc, texture);
}
GLvoid GLAPIENTRY __GLES_APINAME(ClientActiveTextureARB)(GLenum texture)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->ClientActiveTexture(gc, texture);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord1dARB)(GLenum target, GLdouble s)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord1d(gc, target, s);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord1dvARB)(GLenum target, const GLdouble *v)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord1dv(gc, target, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord1fARB)(GLenum target, GLfloat s)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord1f(gc, target, s);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord1fvARB)(GLenum target, const GLfloat *v)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord1fv(gc, target, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord1iARB)(GLenum target, GLint s)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord1i(gc, target, s);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord1ivARB)(GLenum target, const GLint *v)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord1iv(gc, target, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord1sARB)(GLenum target, GLshort s)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord1s(gc, target, s);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord1svARB)(GLenum target, const GLshort *v)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord1sv(gc, target, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord2dARB)(GLenum target, GLdouble s, GLdouble t)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord2d(gc, target, s, t);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord2dvARB)(GLenum target, const GLdouble *v)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord2dv(gc, target, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord2fARB)(GLenum target, GLfloat s, GLfloat t)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord2f(gc, target, s, t);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord2fvARB)(GLenum target, const GLfloat *v)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord2fv(gc, target, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord2iARB)(GLenum target, GLint s, GLint t)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord2i(gc, target, s, t);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord2ivARB)(GLenum target, const GLint *v)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord2iv(gc, target, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord2sARB)(GLenum target, GLshort s, GLshort t)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord2s(gc, target, s, t);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord2svARB)(GLenum target, const GLshort *v)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord2sv(gc, target, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord3dARB)(GLenum target, GLdouble s, GLdouble t, GLdouble r)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord3d(gc, target, s, t, r);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord3dvARB)(GLenum target, const GLdouble *v)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord3dv(gc, target, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord3fARB)(GLenum target, GLfloat s, GLfloat t, GLfloat r)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord3f(gc, target, s, t, r);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord3fvARB)(GLenum target, const GLfloat *v)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord3fv(gc, target, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord3iARB)(GLenum target, GLint s, GLint t, GLint r)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord3i(gc, target, s, t, r);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord3ivARB)(GLenum target, const GLint *v)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord3iv(gc, target, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord3sARB)(GLenum target, GLshort s, GLshort t, GLshort r)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord3s(gc, target, s, t, r);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord3svARB)(GLenum target, const GLshort *v)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord3sv(gc, target, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord4dARB)(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord4d(gc, target, s, t, r, q);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord4dvARB)(GLenum target, const GLdouble *v)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord4dv(gc, target, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord4fARB)(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord4f(gc, target, s, t, r, q);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord4fvARB)(GLenum target, const GLfloat *v)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord4fv(gc, target, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord4iARB)(GLenum target, GLint s, GLint t, GLint r, GLint q)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord4i(gc, target, s, t, r , q);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord4ivARB)(GLenum target, const GLint *v)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord4iv(gc, target, v);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord4sARB)(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord4s(gc, target, s, t, r, q);
}
GLvoid GLAPIENTRY __GLES_APINAME(MultiTexCoord4svARB)(GLenum target, const GLshort *v)
{
    __GL_GET_CONTEXT;
    gc->currentImmediateTable->MultiTexCoord4sv(gc, target, v);
}
#endif

#ifdef GL_GLEXT_PROTOTYPES
GLvoid GLAPIENTRY __GLES_APINAME(BindBufferARB)(GLenum target, GLuint buffer)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BindBuffer(gc, target, buffer);
    __GL_PATTERN_MATCH_NAME(__glesApiEnum(BindBuffer));
}
GLvoid GLAPIENTRY __GLES_APINAME(DeleteBuffersARB)(GLsizei n, const GLuint *buffers)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.DeleteBuffers(gc, n, buffers);
}
GLvoid GLAPIENTRY __GLES_APINAME(GenBuffersARB)(GLsizei n, GLuint *buffers)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GenBuffers(gc, n, buffers);
}
GLboolean GLAPIENTRY __GLES_APINAME(IsBufferARB)(GLuint buffer)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->apiDispatchTable.IsBuffer(gc, buffer);
}
GLvoid GLAPIENTRY __GLES_APINAME(BufferDataARB)(GLenum target, GLsizeiptrARB size, const void *data, GLenum usage)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BufferData(gc, target, size, data, usage);
}
GLvoid GLAPIENTRY __GLES_APINAME(BufferSubDataARB)(GLenum target, GLintptrARB offset, GLsizeiptrARB size, const void *data)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.BufferSubData(gc, target, offset, size, data);
}
GLvoid GLAPIENTRY __GLES_APINAME(GetBufferSubDataARB)(GLenum target, GLintptrARB offset, GLsizeiptrARB size, void *data)
{
}
GLvoid *GLAPIENTRY __GLES_APINAME(MapBufferARB)(GLenum target, GLenum access)
{
    __GL_GET_CONTEXT;
    return gc->currentImmediateTable->MapBuffer(gc, target, access);
}
GLboolean GLAPIENTRY __GLES_APINAME(UnmapBufferARB)(GLenum target)
{
    __GL_GET_CONTEXT_RET(GL_FALSE);
    return gc->apiDispatchTable.UnmapBuffer(gc, target);
}
GLvoid GLAPIENTRY __GLES_APINAME(GetBufferParameterivARB)(GLenum target, GLenum pname, GLint *params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetBufferParameteriv(gc, target, pname, params);
}
GLvoid GLAPIENTRY __GLES_APINAME(GetBufferPointervARB)(GLenum target, GLenum pname, void **params)
{
    __GL_GET_CONTEXT;
    gc->apiDispatchTable.GetBufferPointerv(gc, target, pname, params);
}
#endif


__GLesRawDispatchTableStruct __glVIVV_DispatchFuncTable =
{
    OPENGL_VERSION_110_ENTRIES,
    __GLES_API_ENTRIES(__GLES_APINAME)
};

#define __glProcInfo(func) {#func, (__GLprocAddr)_vvvvvvv_gl##func}

const __GLprocInfo __glProcInfoTable[] =
{
    __GLES_API_ENTRIES(__glProcInfo)
};

const GLuint __glProcTabSize = __GL_TABLE_SIZE(__glProcInfoTable);

