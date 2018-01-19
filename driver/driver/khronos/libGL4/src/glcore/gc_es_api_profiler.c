/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include <stdio.h>
#include "gc_es_context.h"

#define __GLES_LOG_API(...)  gcmPRINT(__VA_ARGS__)

__GLesTracerDispatchTableStruct __glesTracerDispatchTable = {0};

#define __glesApiNameStr(func)  #func

GLchar *__glesTracerFuncNames[] = {
    __GLES_API_ENTRIES(__glesApiNameStr)
};

extern GLint __glesApiTraceMode;

#if VIVANTE_PROFILER
GLint __glesApiProfileMode = -1;

#define __GLES_PROFILE_VARS() \
    gctHANDLE tid = gcoOS_GetCurrentThreadID(); \
    gctUINT64 startTimeusec = 0, endTimeusec = 0

#define __GLES_PROFILE_HEADER() \
    if (__glesApiProfileMode > 0) \
    {\
        gcoOS_GetTime(&startTimeusec);\
    }\


#define __GLES_PROFILE_FOOTER(api_id) \
    if (__glesApiProfileMode > 0 && api_id >= 100) \
    { \
        gc->profiler.apiCalls[api_id - 100]++; \
        gcoOS_GetTime(&endTimeusec); \
        gc->profiler.totalDriverTime+=(endTimeusec-startTimeusec); \
        gc->profiler.apiTimes[api_id - 100]+=(endTimeusec-startTimeusec); \
    }\

#else

#define __GLES_PROFILE_VARS() \
    gctHANDLE tid = gcoOS_GetCurrentThreadID()

#define __GLES_PROFILE_HEADER()

#define __GLES_PROFILE_FOOTER(api_id)

#endif

GLboolean __glInitTracerDispatchTable(GLint trmode, __GLApiVersion apiVersion)
{
    if (trmode == gcvTRACEMODE_LOGGER)
    {
        gctHANDLE trlib = gcvNULL;
        gctPOINTER funcPtr = gcvNULL;
        gceSTATUS status;
        char trApiName[80];
        GLsizei tableSize;
        GLsizei i;

#if defined(_WIN32) || defined(_WIN32_WCE)
        gcoOS_LoadLibrary(gcvNULL, "libGLES_vlogger.dll", &trlib);
#else
        gcoOS_LoadLibrary(gcvNULL, "libGLES_vlogger.so", &trlib);
#endif

        if (trlib  == gcvNULL)
        {
#if defined(_WIN32) || defined(_WIN32_WCE)
            gcoOS_Print("Failed to open libGLES_vlogger.dll!\n");
#else
            gcoOS_Print("Failed to open libGLES_vlogger.so!\n");
#endif

            /* Clear __glesTracerDispatchTable[] */
            __GL_MEMZERO(&__glesTracerDispatchTable, sizeof(__GLesTracerDispatchTableStruct));

            return GL_FALSE;
        }

        switch (apiVersion)
        {
        case __GL_API_VERSION_ES20:
            tableSize = (GLsizei)(offsetof(__GLesTracerDispatchTableStruct, ReadBuffer)              / sizeof(GLvoid*));
            break;
        case __GL_API_VERSION_ES30:
            tableSize = (GLsizei)(offsetof(__GLesTracerDispatchTableStruct, DispatchCompute)         / sizeof(GLvoid*));
            break;
        case __GL_API_VERSION_ES31:
            tableSize = (GLsizei)(offsetof(__GLesTracerDispatchTableStruct, TexStorage3DMultisample) / sizeof(GLvoid*));
            break;
        case __GL_API_VERSION_ES32:
            tableSize = (GLsizei)(sizeof(__GLesTracerDispatchTableStruct)                            / sizeof(GLvoid*));
            break;
        /* vivTracer does NOT support OGL yet */
        case __GL_API_VERSION_OGL10:
        case __GL_API_VERSION_OGL11:
        case __GL_API_VERSION_OGL12:
        case __GL_API_VERSION_OGL13:
        case __GL_API_VERSION_OGL14:
        case __GL_API_VERSION_OGL15:
        case __GL_API_VERSION_OGL20:
        case __GL_API_VERSION_OGL21:
        case __GL_API_VERSION_OGL30:
        case __GL_API_VERSION_OGL31:
        case __GL_API_VERSION_OGL32:
        case __GL_API_VERSION_OGL33:
        case __GL_API_VERSION_OGL40:
        default:
            GL_ASSERT(0);
            return GL_FALSE;
        }

        for (i = 0; i < tableSize; ++i)
        {
            trApiName[0] = '\0';
            gcoOS_StrCatSafe(trApiName, 80, "TR_gl");
            gcoOS_StrCatSafe(trApiName, 80, __glesTracerFuncNames[i]);
            status =  gcoOS_GetProcAddress(gcvNULL, trlib, trApiName, &funcPtr);

            if (status == gcvSTATUS_OK)
            {
                ((void *(*))(&__glesTracerDispatchTable))[i] = funcPtr;
            }
            else
            {
                gcoOS_Print("Failed to initialize __glesTracerDispatchTable: gl%s!\n", __glesTracerFuncNames[i]);

                /* Clear __glesTracerDispatchTable[] */
                __GL_MEMZERO(&__glesTracerDispatchTable, sizeof(__GLesTracerDispatchTableStruct));

                gcoOS_FreeLibrary(gcvNULL, trlib);
                return GL_FALSE;
            }
        }
    }
    else
    {
        /* Clear __glesTracerDispatchTable to enable simple GLES API log function */
        __GL_MEMZERO(&__glesTracerDispatchTable, sizeof(__GLesTracerDispatchTableStruct));
    }

    return GL_TRUE;
}


/* GLES API profiler wrapper functions that can do more API profiling functions */

/* OpenGL ES 2.0 */

#define __GL_PTRVALUE(ptr)  ((ptr) ? *(ptr) : 0)

__GL_INLINE GLvoid __glesLogArrayData(__GLcontext *gc, GLsizei n, const GLuint *array)
{
    __GLES_LOG_API("{");
    if (n > 0 && array)
    {
        GLsizei i;
        __GLES_LOG_API("%d", array[0]);
        for (i = 1; i < n; ++i)
        {
            __GLES_LOG_API(", %d", array[i]);
        }
    }
    __GLES_LOG_API("}\n");
}

__GL_INLINE GLvoid __glesLogSourceStrings(__GLcontext *gc, GLsizei count, const GLchar* const* string)
{
    GLint i, j;
    GLchar tmpbuf[256], *chptr;

    __GLES_LOG_API("####\n");
    for (i = 0; i < count; i++)
    {
        chptr = (GLchar *)string[i];
        while (*chptr != '\0')
        {
            for (j = 0; (j < 255 && *chptr != '\n' && *chptr != '\0'); j++)
            {
                tmpbuf[j] = *chptr++;
            }
            while (*chptr == '\n') chptr++;
            tmpbuf[j] = '\0';
            __GLES_LOG_API("%s\n", tmpbuf);
        }
    }
    __GLES_LOG_API("####\n");
}

GLvoid GLAPIENTRY __glesProfile_ActiveTexture(__GLcontext *gc, GLenum texture)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glActiveTexture 0x%04X\n",
                        gc, tid, texture);
    }

    __GLES_PROFILE_HEADER();
    __gles_ActiveTexture(gc, texture);
    __GLES_PROFILE_FOOTER(GLES3_ACTIVETEXTURE);

    if (__glesTracerDispatchTable.ActiveTexture)
    {
        (*__glesTracerDispatchTable.ActiveTexture)(texture);
    }
}

GLvoid GLAPIENTRY __glesProfile_AttachShader(__GLcontext *gc, GLuint program, GLuint shader)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glAttachShader %d %d\n",
                        gc, tid, program, shader);
    }

    __GLES_PROFILE_HEADER();
    __gles_AttachShader(gc, program, shader);
    __GLES_PROFILE_FOOTER(GLES3_ATTACHSHADER);

    if (__glesTracerDispatchTable.AttachShader)
    {
        (*__glesTracerDispatchTable.AttachShader)(program, shader);
    }
}

GLvoid GLAPIENTRY __glesProfile_BindAttribLocation(__GLcontext *gc, GLuint program, GLuint index, const GLchar* name)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glBindAttribLocation %d %d %s\n",
                        gc, tid, program, index, name);
    }

    __GLES_PROFILE_HEADER();
    __gles_BindAttribLocation(gc, program, index, name);
    __GLES_PROFILE_FOOTER(GLES3_BINDATTRIBLOCATION);

    if (__glesTracerDispatchTable.BindAttribLocation)
    {
        (*__glesTracerDispatchTable.BindAttribLocation)(program, index, name);
    }
}

GLvoid GLAPIENTRY __glesProfile_BindBuffer(__GLcontext *gc, GLenum target, GLuint buffer)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glBindBuffer 0x%04X %d\n",
                        gc, tid, target, buffer);
    }

    __GLES_PROFILE_HEADER();
    __gles_BindBuffer(gc, target, buffer);
    __GLES_PROFILE_FOOTER(GLES3_BINDBUFFER);

    if (__glesTracerDispatchTable.BindBuffer)
    {
        (*__glesTracerDispatchTable.BindBuffer)(target, buffer);
    }
}

GLvoid GLAPIENTRY __glesProfile_BindFramebuffer(__GLcontext *gc, GLenum target, GLuint framebuffer)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glBindFramebuffer 0x%04X %d\n",
                        gc, tid, target, framebuffer);
    }

    __GLES_PROFILE_HEADER();
    __gles_BindFramebuffer(gc, target, framebuffer);
    __GLES_PROFILE_FOOTER(GLES3_BINDFRAMEBUFFER);

    if (__glesTracerDispatchTable.BindFramebuffer)
    {
        (*__glesTracerDispatchTable.BindFramebuffer)(target, framebuffer);
    }
}

GLvoid GLAPIENTRY __glesProfile_BindRenderbuffer(__GLcontext *gc, GLenum target, GLuint renderbuffer)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glBindRenderbuffer 0x%04X %d\n",
                        gc, tid, target, renderbuffer);
    }

    __GLES_PROFILE_HEADER();
    __gles_BindRenderbuffer(gc, target, renderbuffer);
    __GLES_PROFILE_FOOTER(GLES3_BINDRENDERBUFFER);

    if (__glesTracerDispatchTable.BindRenderbuffer)
    {
        (*__glesTracerDispatchTable.BindRenderbuffer)(target, renderbuffer);
    }
}

GLvoid GLAPIENTRY __glesProfile_BindTexture(__GLcontext *gc, GLenum target, GLuint texture)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glBindTexture 0x%04X %d\n",
                        gc, tid, target, texture);
    }

    __GLES_PROFILE_HEADER();
    __gles_BindTexture(gc, target, texture);
    __GLES_PROFILE_FOOTER(GLES3_BINDTEXTURE);

    if (__glesTracerDispatchTable.BindTexture)
    {
        (*__glesTracerDispatchTable.BindTexture)(target, texture);
    }
}

GLvoid GLAPIENTRY __glesProfile_BlendColor(__GLcontext *gc, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glBlendColor %f %f %f %f\n",
                        gc, tid, red, green, blue, alpha);
    }

    __GLES_PROFILE_HEADER();
    __gles_BlendColor(gc, red, green, blue, alpha);
    __GLES_PROFILE_FOOTER(GLES3_BLENDCOLOR);

    if (__glesTracerDispatchTable.BlendColor)
    {
        (*__glesTracerDispatchTable.BlendColor)(red, green, blue, alpha);
    }
}

GLvoid GLAPIENTRY __glesProfile_BlendEquation(__GLcontext *gc, GLenum mode)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glBlendEquation 0x%04X\n",
                        gc, tid, mode);
    }

    __GLES_PROFILE_HEADER();
    __gles_BlendEquation(gc, mode);
    __GLES_PROFILE_FOOTER(GLES3_BLENDEQUATION);

    if (__glesTracerDispatchTable.BlendEquation)
    {
        (*__glesTracerDispatchTable.BlendEquation)(mode);
    }
}

GLvoid GLAPIENTRY __glesProfile_BlendEquationSeparate(__GLcontext *gc, GLenum modeRGB, GLenum modeAlpha)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glBlendEquationSeparate 0x%04X 0x%04X\n",
                        gc, tid, modeRGB, modeAlpha);
    }

    __GLES_PROFILE_HEADER();
    __gles_BlendEquationSeparate(gc, modeRGB, modeAlpha);
    __GLES_PROFILE_FOOTER(GLES3_BLENDEQUATIONSEPARATE);

    if (__glesTracerDispatchTable.BlendEquationSeparate)
    {
        (*__glesTracerDispatchTable.BlendEquationSeparate)(modeRGB, modeAlpha);
    }
}

GLvoid GLAPIENTRY __glesProfile_BlendFunc(__GLcontext *gc, GLenum sfactor, GLenum dfactor)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glBlendFunc 0x%04X 0x%04X\n",
                        gc, tid, sfactor, dfactor);
    }

    __GLES_PROFILE_HEADER();
    __gles_BlendFunc(gc, sfactor, dfactor);
    __GLES_PROFILE_FOOTER(GLES3_BLENDFUNC);

    if (__glesTracerDispatchTable.BlendFunc)
    {
        (*__glesTracerDispatchTable.BlendFunc)(sfactor, dfactor);
    }
}

GLvoid GLAPIENTRY __glesProfile_BlendFuncSeparate(__GLcontext *gc, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glBlendFuncSeparate 0x%04X 0x%04X 0x%04X 0x%04X\n",
                        gc, tid, srcRGB, dstRGB, srcAlpha, dstAlpha);
    }

    __GLES_PROFILE_HEADER();
    __gles_BlendFuncSeparate(gc, srcRGB, dstRGB, srcAlpha, dstAlpha);
    __GLES_PROFILE_FOOTER(GLES3_BLENDFUNCSEPARATE);

    if (__glesTracerDispatchTable.BlendFuncSeparate)
    {
        (*__glesTracerDispatchTable.BlendFuncSeparate)(srcRGB, dstRGB, srcAlpha, dstAlpha);
    }
}

GLvoid GLAPIENTRY __glesProfile_BufferData(__GLcontext *gc, GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glBufferData 0x%04X 0x%08X 0x%08X 0x%04X\n",
                        gc, tid, target, __GL_PTR2UINT(size), __GL_PTR2UINT(data), usage);
    }

    __GLES_PROFILE_HEADER();
    __gles_BufferData(gc, target, size, data, usage);
    __GLES_PROFILE_FOOTER(GLES3_BUFFERDATA);

    if (__glesTracerDispatchTable.BufferData)
    {
        (*__glesTracerDispatchTable.BufferData)(target, size, data, usage);
    }
}

GLvoid GLAPIENTRY __glesProfile_BufferSubData(__GLcontext *gc, GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glBufferSubData 0x%04X 0x%08X 0x%08X 0x%08X\n",
                        gc, tid, target, __GL_PTR2UINT(offset), __GL_PTR2UINT(size), __GL_PTR2UINT(data));
    }

    __GLES_PROFILE_HEADER();
    __gles_BufferSubData(gc, target, offset, size, data);
    __GLES_PROFILE_FOOTER(GLES3_BUFFERSUBDATA);

    if (__glesTracerDispatchTable.BufferSubData)
    {
        (*__glesTracerDispatchTable.BufferSubData)(target, offset, size, data);
    }
}

GLenum GLAPIENTRY __glesProfile_CheckFramebufferStatus(__GLcontext *gc, GLenum target)
{
    GLenum status;
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glCheckFramebufferStatus 0x%04X\n",
                        gc, tid, target);
    }

    __GLES_PROFILE_HEADER();
    status = __gles_CheckFramebufferStatus(gc, target);
    __GLES_PROFILE_FOOTER(GLES3_CHECKFRAMEBUFFERSTATUS);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glCheckFramebufferStatus => 0x%04X\n", status);
    }

    if (__glesTracerDispatchTable.CheckFramebufferStatus)
    {
        (*__glesTracerDispatchTable.CheckFramebufferStatus)(target);
    }

    return status;
}

GLvoid GLAPIENTRY __glesProfile_Clear(__GLcontext *gc, GLbitfield mask)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glClear 0x%08X\n",
                        gc, tid, mask);
    }

    __GLES_PROFILE_HEADER();
    __gles_Clear(gc, mask);
    __GLES_PROFILE_FOOTER(GLES3_CLEAR);

    if (__glesTracerDispatchTable.Clear)
    {
        (*__glesTracerDispatchTable.Clear)(mask);
    }
}

GLvoid GLAPIENTRY __glesProfile_ClearColor(__GLcontext *gc, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glClearColor %f %f %f %f\n",
                        gc, tid, red, green, blue, alpha);
    }

    __GLES_PROFILE_HEADER();
    __gles_ClearColor(gc, red, green, blue, alpha);
    __GLES_PROFILE_FOOTER(GLES3_CLEARCOLOR);

    if (__glesTracerDispatchTable.ClearColor)
    {
        (*__glesTracerDispatchTable.ClearColor)(red, green, blue, alpha);
    }
}

GLvoid GLAPIENTRY __glesProfile_ClearDepthf(__GLcontext *gc, GLfloat depth)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glClearDepthf %f\n",
                        gc, tid, depth);
    }

    __GLES_PROFILE_HEADER();
    __gles_ClearDepthf(gc, depth);
    __GLES_PROFILE_FOOTER(GLES3_CLEARDEPTHF);

    if (__glesTracerDispatchTable.ClearDepthf)
    {
        (*__glesTracerDispatchTable.ClearDepthf)(depth);
    }
}

GLvoid GLAPIENTRY __glesProfile_ClearStencil(__GLcontext *gc, GLint s)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glClearStencil %d\n",
                        gc, tid, s);
    }

    __GLES_PROFILE_HEADER();
    __gles_ClearStencil(gc, s);
    __GLES_PROFILE_FOOTER(GLES3_CLEARSTENCIL);

    if (__glesTracerDispatchTable.ClearStencil)
    {
        (*__glesTracerDispatchTable.ClearStencil)(s);
    }
}

GLvoid GLAPIENTRY __glesProfile_ColorMask(__GLcontext *gc, GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glColorMask %d %d %d %d\n",
                        gc, tid, red, green, blue, alpha);
    }

    __GLES_PROFILE_HEADER();
    __gles_ColorMask(gc, red, green, blue, alpha);
    __GLES_PROFILE_FOOTER(GLES3_COLORMASK);

    if (__glesTracerDispatchTable.ColorMask)
    {
        (*__glesTracerDispatchTable.ColorMask)(red, green, blue, alpha);
    }
}

GLvoid GLAPIENTRY __glesProfile_CompileShader(__GLcontext *gc, GLuint shader)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glCompileShader %d\n",
                        gc, tid, shader);
    }

    __GLES_PROFILE_HEADER();
    __gles_CompileShader(gc, shader);
    __GLES_PROFILE_FOOTER(GLES3_COMPILESHADER);

    if (__glesTracerDispatchTable.CompileShader)
    {
        (*__glesTracerDispatchTable.CompileShader)(shader);
    }
}

GLvoid GLAPIENTRY __glesProfile_CompressedTexImage2D(__GLcontext *gc, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glCompressedTexImage2D 0x%04X %d 0x%04X %d %d %d %d 0x%08X\n",
                        gc, tid, target, level, internalformat, width, height, border, imageSize, __GL_PTR2UINT(data));
    }

    __GLES_PROFILE_HEADER();
    __gles_CompressedTexImage2D(gc, target, level, internalformat, width, height, border, imageSize, data);
    __GLES_PROFILE_FOOTER(GLES3_COMPRESSEDTEXIMAGE2D);

    if (__glesTracerDispatchTable.CompressedTexImage2D)
    {
        (*__glesTracerDispatchTable.CompressedTexImage2D)(target, level, internalformat, width, height, border, imageSize, data);
    }
}

GLvoid GLAPIENTRY __glesProfile_CompressedTexSubImage2D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid* data)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glCompressedTexSubImage2D 0x%04X %d %d %d %d %d 0x%04X %d 0x%08X\n",
                        gc, tid, target, level, xoffset, yoffset, width, height, format, imageSize, __GL_PTR2UINT(data));
    }

    __GLES_PROFILE_HEADER();
    __gles_CompressedTexSubImage2D(gc, target, level, xoffset, yoffset, width, height, format, imageSize, data);
    __GLES_PROFILE_FOOTER(GLES3_COMPRESSEDTEXSUBIMAGE2D);

    if (__glesTracerDispatchTable.CompressedTexSubImage2D)
    {
        (*__glesTracerDispatchTable.CompressedTexSubImage2D)(target, level, xoffset, yoffset, width, height, format, imageSize, data);
    }
}

GLvoid GLAPIENTRY __glesProfile_CopyTexImage2D(__GLcontext *gc, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glCopyTexImage2D 0x%04X %d 0x%04X %d %d %d %d %d\n",
                        gc, tid, target, level, internalformat, x, y, width, height, border);
    }

    __GLES_PROFILE_HEADER();
    __gles_CopyTexImage2D(gc, target, level, internalformat, x, y, width, height, border);
    __GLES_PROFILE_FOOTER(GLES3_COPYTEXIMAGE2D);

    if (__glesTracerDispatchTable.CopyTexImage2D)
    {
        (*__glesTracerDispatchTable.CopyTexImage2D)(target, level, internalformat, x, y, width, height, border);
    }
}

GLvoid GLAPIENTRY __glesProfile_CopyTexSubImage2D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glCopyTexSubImage2D 0x%04X %d %d %d %d %d %d %d\n",
                        gc, tid, target, level, xoffset, yoffset, x, y, width, height);
    }

    __GLES_PROFILE_HEADER();
    __gles_CopyTexSubImage2D(gc, target, level, xoffset, yoffset, x, y, width, height);
    __GLES_PROFILE_FOOTER(GLES3_COPYTEXSUBIMAGE2D);

    if (__glesTracerDispatchTable.CopyTexSubImage2D)
    {
        (*__glesTracerDispatchTable.CopyTexSubImage2D)(target, level, xoffset, yoffset, x, y, width, height);
    }
}

GLuint GLAPIENTRY __glesProfile_CreateProgram(__GLcontext *gc)
{
    __GLES_PROFILE_VARS();
    GLuint program;

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glCreateProgram\n",
                        gc, tid);
    }

    __GLES_PROFILE_HEADER();
    program = __gles_CreateProgram(gc);
    __GLES_PROFILE_FOOTER(GLES3_CREATEPROGRAM);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glCreateProgram => %d\n", program);
    }

    if (__glesTracerDispatchTable.CreateProgram)
    {
        (*__glesTracerDispatchTable.CreateProgram)(program);
    }

    return program;
}

GLuint GLAPIENTRY __glesProfile_CreateShader(__GLcontext *gc, GLenum type)
{
    __GLES_PROFILE_VARS();
    GLuint shader;

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glCreateShader 0x%04X\n",
                        gc, tid, type);
    }

    __GLES_PROFILE_HEADER();
    shader = __gles_CreateShader(gc, type);
    __GLES_PROFILE_FOOTER(GLES3_CREATESHADER);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glCreateShader => %d\n", shader);
    }

    if (__glesTracerDispatchTable.CreateShader)
    {
        (*__glesTracerDispatchTable.CreateShader)(type, shader);
    }

    return shader;
}

GLvoid GLAPIENTRY __glesProfile_CullFace(__GLcontext *gc, GLenum mode)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glCullFace 0x%04X\n",
                        gc, tid, mode);
    }

    __GLES_PROFILE_HEADER();
    __gles_CullFace(gc, mode);
    __GLES_PROFILE_FOOTER(GLES3_CULLFACE);

    if (__glesTracerDispatchTable.CullFace)
    {
        (*__glesTracerDispatchTable.CullFace)(mode);
    }
}

GLvoid GLAPIENTRY __glesProfile_DeleteBuffers(__GLcontext *gc, GLsizei n, const GLuint* buffers)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glDeleteBuffers %d ",
                        gc, tid, n);
        __glesLogArrayData(gc, n, buffers);
    }

    __GLES_PROFILE_HEADER();
    __gles_DeleteBuffers(gc, n, buffers);
    __GLES_PROFILE_FOOTER(GLES3_DELETEBUFFERS);

    if (__glesTracerDispatchTable.DeleteBuffers)
    {
        (*__glesTracerDispatchTable.DeleteBuffers)(n, buffers);
    }
}

GLvoid GLAPIENTRY __glesProfile_DeleteFramebuffers(__GLcontext *gc, GLsizei n, const GLuint* framebuffers)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glDeleteFramebuffers %d ",
                        gc, tid, n);
        __glesLogArrayData(gc, n, framebuffers);
    }

    __GLES_PROFILE_HEADER();
    __gles_DeleteFramebuffers(gc, n, framebuffers);
    __GLES_PROFILE_FOOTER(GLES3_DELETEFRAMEBUFFERS);

    if (__glesTracerDispatchTable.DeleteFramebuffers)
    {
        (*__glesTracerDispatchTable.DeleteFramebuffers)(n, framebuffers);
    }
}

GLvoid GLAPIENTRY __glesProfile_DeleteProgram(__GLcontext *gc, GLuint program)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glDeleteProgram %d\n",
                        gc, tid, program);
    }

    __GLES_PROFILE_HEADER();
    __gles_DeleteProgram(gc, program);
    __GLES_PROFILE_FOOTER(GLES3_DELETEPROGRAM);

    if (__glesTracerDispatchTable.DeleteProgram)
    {
        (*__glesTracerDispatchTable.DeleteProgram)(program);
    }
}

GLvoid GLAPIENTRY __glesProfile_DeleteRenderbuffers(__GLcontext *gc, GLsizei n, const GLuint* renderbuffers)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glDeleteRenderbuffers %d ",
                        gc, tid, n);
        __glesLogArrayData(gc, n, renderbuffers);
    }

    __GLES_PROFILE_HEADER();
    __gles_DeleteRenderbuffers(gc, n, renderbuffers);
    __GLES_PROFILE_FOOTER(GLES3_DELETERENDERBUFFERS);

    if (__glesTracerDispatchTable.DeleteRenderbuffers)
    {
        (*__glesTracerDispatchTable.DeleteRenderbuffers)(n, renderbuffers);
    }
}

GLvoid GLAPIENTRY __glesProfile_DeleteShader(__GLcontext *gc, GLuint shader)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glDeleteShader %d\n",
                        gc, tid, shader);
    }

    __GLES_PROFILE_HEADER();
    __gles_DeleteShader(gc, shader);
    __GLES_PROFILE_FOOTER(GLES3_DELETESHADER);

    if (__glesTracerDispatchTable.DeleteShader)
    {
        (*__glesTracerDispatchTable.DeleteShader)(shader);
    }
}

GLvoid GLAPIENTRY __glesProfile_DeleteTextures(__GLcontext *gc, GLsizei n, const GLuint* textures)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glDeleteTextures %d ",
                        gc, tid, n);
        __glesLogArrayData(gc, n, textures);
    }

    __GLES_PROFILE_HEADER();
    __gles_DeleteTextures(gc, n, textures);
    __GLES_PROFILE_FOOTER(GLES3_DELETETEXTURES);

    if (__glesTracerDispatchTable.DeleteTextures)
    {
        (*__glesTracerDispatchTable.DeleteTextures)(n, textures);
    }
}

GLvoid GLAPIENTRY __glesProfile_DepthFunc(__GLcontext *gc, GLenum func)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glDepthFunc 0x%04X\n",
                        gc, tid, func);
    }

    __GLES_PROFILE_HEADER();
    __gles_DepthFunc(gc, func);
    __GLES_PROFILE_FOOTER(GLES3_DEPTHFUNC);

    if (__glesTracerDispatchTable.DepthFunc)
    {
        (*__glesTracerDispatchTable.DepthFunc)(func);
    }
}

GLvoid GLAPIENTRY __glesProfile_DepthMask(__GLcontext *gc, GLboolean flag)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glDepthMask %d\n",
                        gc, tid, flag);
    }

    __GLES_PROFILE_HEADER();
    __gles_DepthMask(gc, flag);
    __GLES_PROFILE_FOOTER(GLES3_DEPTHMASK);

    if (__glesTracerDispatchTable.DepthMask)
    {
        (*__glesTracerDispatchTable.DepthMask)(flag);
    }
}

GLvoid GLAPIENTRY __glesProfile_DepthRangef(__GLcontext *gc, GLfloat n, GLfloat f)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glDepthRangef %f %f\n",
                        gc, tid, n, f);
    }

    __GLES_PROFILE_HEADER();
    __gles_DepthRangef(gc, n, f);
    __GLES_PROFILE_FOOTER(GLES3_DEPTHRANGEF);

    if (__glesTracerDispatchTable.DepthRangef)
    {
        (*__glesTracerDispatchTable.DepthRangef)(n, f);
    }
}

GLvoid GLAPIENTRY __glesProfile_DetachShader(__GLcontext *gc, GLuint program, GLuint shader)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glDetachShader %d %d\n",
                        gc, tid, program, shader);
    }

    __GLES_PROFILE_HEADER();
    __gles_DetachShader(gc, program, shader);
    __GLES_PROFILE_FOOTER(GLES3_DETACHSHADER);

    if (__glesTracerDispatchTable.DetachShader)
    {
        (*__glesTracerDispatchTable.DetachShader)(program, shader);
    }
}

GLvoid GLAPIENTRY __glesProfile_Disable(__GLcontext *gc, GLenum cap)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glDisable 0x%04X\n",
                        gc, tid, cap);
    }

    __GLES_PROFILE_HEADER();
    __gles_Disable(gc, cap);
    __GLES_PROFILE_FOOTER(GLES3_DISABLE);

    if (__glesTracerDispatchTable.Disable)
    {
        (*__glesTracerDispatchTable.Disable)(cap);
    }
}

GLvoid GLAPIENTRY __glesProfile_DisableVertexAttribArray(__GLcontext *gc, GLuint index)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glDisableVertexAttribArray %d\n",
                        gc, tid, index);
    }

    __GLES_PROFILE_HEADER();
    __gles_DisableVertexAttribArray(gc, index);
    __GLES_PROFILE_FOOTER(GLES3_DISABLEVERTEXATTRIBARRAY);

    if (__glesTracerDispatchTable.DisableVertexAttribArray)
    {
        (*__glesTracerDispatchTable.DisableVertexAttribArray)(index);
    }
}

GLvoid GLAPIENTRY __glesProfile_DrawArrays(__GLcontext *gc, GLenum mode, GLint first, GLsizei count)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glDrawArrays 0x%04X %d %d\n",
                         gc, tid, mode, first, count);
    }

    __GLES_PROFILE_HEADER();
    __gles_DrawArrays(gc, mode, first, count);
    __GLES_PROFILE_FOOTER(GLES3_DRAWARRAYS);

    if (__glesTracerDispatchTable.DrawArrays)
    {
        (*__glesTracerDispatchTable.DrawArrays)(mode, first, count);
    }
}

GLvoid GLAPIENTRY __glesProfile_DrawElements(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const GLvoid* indices)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glDrawElements 0x%04X %d 0x%04X 0x%08X\n",
                        gc, tid, mode, count, type, __GL_PTR2UINT(indices));
    }

    __GLES_PROFILE_HEADER();
    __gles_DrawElements(gc, mode, count, type, indices);
    __GLES_PROFILE_FOOTER(GLES3_DRAWELEMENTS);

    if (__glesTracerDispatchTable.DrawElements)
    {
        (*__glesTracerDispatchTable.DrawElements)(mode, count, type, indices);
    }
}

GLvoid GLAPIENTRY __glesProfile_Enable(__GLcontext *gc, GLenum cap)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glEnable 0x%04X\n",
                        gc, tid, cap);
    }

    __GLES_PROFILE_HEADER();
    __gles_Enable(gc, cap);
    __GLES_PROFILE_FOOTER(GLES3_ENABLE);

    if (__glesTracerDispatchTable.Enable)
    {
        (*__glesTracerDispatchTable.Enable)(cap);
    }
}

GLvoid GLAPIENTRY __glesProfile_EnableVertexAttribArray(__GLcontext *gc, GLuint index)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glEnableVertexAttribArray %d\n",
                        gc, tid, index);
    }

    __GLES_PROFILE_HEADER();
    __gles_EnableVertexAttribArray(gc, index);
    __GLES_PROFILE_FOOTER(GLES3_ENABLEVERTEXATTRIBARRAY);

    if (__glesTracerDispatchTable.EnableVertexAttribArray)
    {
        (*__glesTracerDispatchTable.EnableVertexAttribArray)(index);
    }
}

GLvoid GLAPIENTRY __glesProfile_Finish(__GLcontext *gc)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glFinish\n",
                        gc, tid);
    }

    __GLES_PROFILE_HEADER();
    __gles_Finish(gc);
    __GLES_PROFILE_FOOTER(GLES3_FINISH);

    if (__glesTracerDispatchTable.Finish)
    {
        (*__glesTracerDispatchTable.Finish)();
    }
}

GLvoid GLAPIENTRY __glesProfile_Flush(__GLcontext *gc)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glFlush\n",
                        gc, tid);
    }

    __GLES_PROFILE_HEADER();
    __gles_Flush(gc);
    __GLES_PROFILE_FOOTER(GLES3_FLUSH);

    if (__glesTracerDispatchTable.Flush)
    {
        (*__glesTracerDispatchTable.Flush)();
    }
}

GLvoid GLAPIENTRY __glesProfile_FramebufferRenderbuffer(__GLcontext *gc, GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glFramebufferRenderbuffer 0x%04X 0x%04X 0x%04X %d\n",
                        gc, tid, target, attachment, renderbuffertarget, renderbuffer);
    }

    __GLES_PROFILE_HEADER();
    __gles_FramebufferRenderbuffer(gc, target, attachment, renderbuffertarget, renderbuffer);
    __GLES_PROFILE_FOOTER(GLES3_FRAMEBUFFERRENDERBUFFER);

    if (__glesTracerDispatchTable.FramebufferRenderbuffer)
    {
        (*__glesTracerDispatchTable.FramebufferRenderbuffer)(target, attachment, renderbuffertarget, renderbuffer);
    }
}

GLvoid GLAPIENTRY __glesProfile_FramebufferTexture2D(__GLcontext *gc, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glFramebufferTexture2D 0x%04X 0x%04X 0x%04X %d %d\n",
                        gc, tid, target, attachment, textarget, texture, level);
    }

    __GLES_PROFILE_HEADER();
    __gles_FramebufferTexture2D(gc, target, attachment, textarget, texture, level);
    __GLES_PROFILE_FOOTER(GLES3_FRAMEBUFFERTEXTURE2D);

    if (__glesTracerDispatchTable.FramebufferTexture2D)
    {
        (*__glesTracerDispatchTable.FramebufferTexture2D)(target, attachment, textarget, texture, level);
    }
}

GLvoid GLAPIENTRY __glesProfile_FrontFace(__GLcontext *gc, GLenum mode)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glFrontFace 0x%04X\n",
                        gc, tid, mode);
    }

    __GLES_PROFILE_HEADER();
    __gles_FrontFace(gc, mode);
    __GLES_PROFILE_FOOTER(GLES3_FRONTFACE);

    if (__glesTracerDispatchTable.FrontFace)
    {
        (*__glesTracerDispatchTable.FrontFace)(mode);
    }
}

GLvoid GLAPIENTRY __glesProfile_GenBuffers(__GLcontext *gc, GLsizei n, GLuint* buffers)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGenBuffers %d\n",
                        gc, tid, n);
    }

    __GLES_PROFILE_HEADER();
    __gles_GenBuffers(gc, n, buffers);
    __GLES_PROFILE_FOOTER(GLES3_GENBUFFERS);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGenBuffers => ");
        __glesLogArrayData(gc, n, buffers);
    }

    if (__glesTracerDispatchTable.GenBuffers)
    {
        (*__glesTracerDispatchTable.GenBuffers)(n, buffers);
    }
}

GLvoid GLAPIENTRY __glesProfile_GenerateMipmap(__GLcontext *gc, GLenum target)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGenerateMipmap 0x%04X\n",
                        gc, tid, target);
    }

    __GLES_PROFILE_HEADER();
    __gles_GenerateMipmap(gc, target);
    __GLES_PROFILE_FOOTER(GLES3_GENERATEMIPMAP);

    if (__glesTracerDispatchTable.GenerateMipmap)
    {
        (*__glesTracerDispatchTable.GenerateMipmap)(target);
    }
}

GLvoid GLAPIENTRY __glesProfile_GenFramebuffers(__GLcontext *gc, GLsizei n, GLuint* framebuffers)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGenFramebuffers %d\n",
                        gc, tid, n);
    }

    __GLES_PROFILE_HEADER();
    __gles_GenFramebuffers(gc, n, framebuffers);
    __GLES_PROFILE_FOOTER(GLES3_GENFRAMEBUFFERS);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGenFramebuffers => ");
        __glesLogArrayData(gc, n, framebuffers);
    }

    if (__glesTracerDispatchTable.GenFramebuffers)
    {
        (*__glesTracerDispatchTable.GenFramebuffers)(n, framebuffers);
    }
}

GLvoid GLAPIENTRY __glesProfile_GenRenderbuffers(__GLcontext *gc, GLsizei n, GLuint* renderbuffers)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGenRenderbuffers %d\n",
                        gc, tid, n);
    }

    __GLES_PROFILE_HEADER();
    __gles_GenRenderbuffers(gc, n, renderbuffers);
    __GLES_PROFILE_FOOTER(GLES3_GENRENDERBUFFERS);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGenRenderbuffers => ");
        __glesLogArrayData(gc, n, renderbuffers);
    }

    if (__glesTracerDispatchTable.GenRenderbuffers)
    {
        (*__glesTracerDispatchTable.GenRenderbuffers)(n, renderbuffers);
    }
}

GLvoid GLAPIENTRY __glesProfile_GenTextures(__GLcontext *gc, GLsizei n, GLuint* textures)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGenTextures %d\n",
                        gc, tid, n);
    }

    __GLES_PROFILE_HEADER();
    __gles_GenTextures(gc, n, textures);
    __GLES_PROFILE_FOOTER(GLES3_GENTEXTURES);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGenTextures => ");
        __glesLogArrayData(gc, n, textures);
    }

    if (__glesTracerDispatchTable.GenTextures)
    {
        (*__glesTracerDispatchTable.GenTextures)(n, textures);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetActiveAttrib(__GLcontext *gc, GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetActiveAttrib %d %d %d\n",
                        gc, tid, program, index, bufsize);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetActiveAttrib(gc, program, index, bufsize, length, size, type, name);
    __GLES_PROFILE_FOOTER(GLES3_GETACTIVEATTRIB);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetActiveAttrib => %d %d 0x%04X %s\n",
                        __GL_PTRVALUE(length), __GL_PTRVALUE(size), __GL_PTRVALUE(type), name);
    }

    if (__glesTracerDispatchTable.GetActiveAttrib)
    {
        (*__glesTracerDispatchTable.GetActiveAttrib)(program, index, bufsize, length, size, type, name);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetActiveUniform(__GLcontext *gc, GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetActiveUniform %d %d %d\n",
            gc, tid, program, index, bufsize);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetActiveUniform(gc, program, index, bufsize, length, size, type, name);
    __GLES_PROFILE_FOOTER(GLES3_GETACTIVEUNIFORM);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetActiveUniform => %d %d 0x%04X %s\n",
                        __GL_PTRVALUE(length), __GL_PTRVALUE(size), __GL_PTRVALUE(type), name);
    }

    if (__glesTracerDispatchTable.GetActiveUniform)
    {
        (*__glesTracerDispatchTable.GetActiveUniform)(program, index, bufsize, length, size, type, name);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetAttachedShaders(__GLcontext *gc, GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetAttachedShaders %d %d\n",
                        gc, tid, program, maxcount);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetAttachedShaders(gc, program, maxcount, count, shaders);
    __GLES_PROFILE_FOOTER(GLES3_GETATTACHEDSHADERS);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetAttachedShaders => %d 0x%08X\n",
                        __GL_PTRVALUE(count), __GL_PTR2UINT(shaders));
    }

    if (__glesTracerDispatchTable.GetAttachedShaders)
    {
        (*__glesTracerDispatchTable.GetAttachedShaders)(program, maxcount, count, shaders);
    }
}

GLint GLAPIENTRY __glesProfile_GetAttribLocation(__GLcontext *gc, GLuint program, const GLchar* name)
{
    __GLES_PROFILE_VARS();
    GLint location;

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetAttribLocation %d %s\n",
                        gc, tid, program, name);
    }

    __GLES_PROFILE_HEADER();
    location = __gles_GetAttribLocation(gc, program, name);
    __GLES_PROFILE_FOOTER(GLES3_GETATTRIBLOCATION);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetAttribLocation => %d\n", location);
    }

    if (__glesTracerDispatchTable.GetAttribLocation)
    {
        (*__glesTracerDispatchTable.GetAttribLocation)(program, name, location);
    }

    return location;
}

GLvoid GLAPIENTRY __glesProfile_GetBooleanv(__GLcontext *gc, GLenum pname, GLboolean* params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetBooleanv 0x%04X\n",
                        gc, tid, pname);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetBooleanv(gc, pname, params);
    __GLES_PROFILE_FOOTER(GLES3_GETBOOLEANV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetBooleanv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glesTracerDispatchTable.GetBooleanv)
    {
        (*__glesTracerDispatchTable.GetBooleanv)(pname, params);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetBufferParameteriv(__GLcontext *gc, GLenum target, GLenum pname, GLint* params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetBufferParameteriv 0x%04X 0x%04X\n",
                        gc, tid, target, pname);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetBufferParameteriv(gc, target, pname, params);
    __GLES_PROFILE_FOOTER(GLES3_GETBUFFERPARAMETERIV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetBufferParameteriv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glesTracerDispatchTable.GetBufferParameteriv)
    {
        (*__glesTracerDispatchTable.GetBufferParameteriv)(target, pname, params);
    }
}

GLenum GLAPIENTRY __glesProfile_GetError(__GLcontext *gc)
{
    __GLES_PROFILE_VARS();
    GLenum error;

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetError\n",
                        gc, tid);
    }

    __GLES_PROFILE_HEADER();
    error = __gles_GetError(gc);
    __GLES_PROFILE_FOOTER(GLES3_GETERROR);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetError => 0x%04X\n", error);
    }

    if (__glesTracerDispatchTable.GetError)
    {
        (*__glesTracerDispatchTable.GetError)();
    }

    return error;
}

GLvoid GLAPIENTRY __glesProfile_GetFloatv(__GLcontext *gc, GLenum pname, GLfloat* params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetFloatv 0x%04X\n",
                        gc, tid, pname);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetFloatv(gc, pname, params);
    __GLES_PROFILE_FOOTER(GLES3_GETFLOATV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetFloatv => %f\n", __GL_PTRVALUE(params));
    }

    if (__glesTracerDispatchTable.GetFloatv)
    {
        (*__glesTracerDispatchTable.GetFloatv)(pname, params);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetFramebufferAttachmentParameteriv(__GLcontext *gc, GLenum target, GLenum attachment, GLenum pname, GLint* params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetFramebufferAttachmentParameteriv 0x%04X 0x%04X 0x%04X\n",
                        gc, tid, target, attachment, pname);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetFramebufferAttachmentParameteriv(gc, target, attachment, pname, params);
    __GLES_PROFILE_FOOTER(GLES3_GETFRAMEBUFFERATTACHMENTPARAMETERIV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetFramebufferAttachmentParameteriv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glesTracerDispatchTable.GetFramebufferAttachmentParameteriv)
    {
        (*__glesTracerDispatchTable.GetFramebufferAttachmentParameteriv)(target, attachment, pname, params);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetIntegerv(__GLcontext *gc, GLenum pname, GLint* params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetIntegerv 0x%04X\n",
                        gc, tid, pname);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetIntegerv(gc, pname, params);
    __GLES_PROFILE_FOOTER(GLES3_GETINTEGERV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetIntegerv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glesTracerDispatchTable.GetIntegerv)
    {
        (*__glesTracerDispatchTable.GetIntegerv)(pname, params);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetProgramiv(__GLcontext *gc, GLuint program, GLenum pname, GLint* params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetProgramiv %d 0x%04X\n",
                        gc, tid, program, pname);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetProgramiv(gc, program, pname, params);
    __GLES_PROFILE_FOOTER(GLES3_GETPROGRAMIV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetProgramiv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glesTracerDispatchTable.GetProgramiv)
    {
        (*__glesTracerDispatchTable.GetProgramiv)(program, pname, params);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetProgramInfoLog(__GLcontext *gc, GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetProgramInfoLog %d %d\n",
                        gc, tid, program, bufsize);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetProgramInfoLog(gc, program, bufsize, length, infolog);
    __GLES_PROFILE_FOOTER(GLES3_GETPROGRAMINFOLOG);

    if (bufsize && (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST))
    {
        __GLES_LOG_API("        glGetProgramInfoLog => %d %s\n", __GL_PTRVALUE(length), infolog);
    }

    if (__glesTracerDispatchTable.GetProgramInfoLog)
    {
        (*__glesTracerDispatchTable.GetProgramInfoLog)(program, bufsize, length, infolog);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetRenderbufferParameteriv(__GLcontext *gc, GLenum target, GLenum pname, GLint* params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetRenderbufferParameteriv 0x%04X 0x%04X\n",
                        gc, tid, target, pname);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetRenderbufferParameteriv(gc, target, pname, params);
    __GLES_PROFILE_FOOTER(GLES3_GETRENDERBUFFERPARAMETERIV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetRenderbufferParameteriv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glesTracerDispatchTable.GetRenderbufferParameteriv)
    {
        (*__glesTracerDispatchTable.GetRenderbufferParameteriv)(target, pname, params);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetShaderiv(__GLcontext *gc, GLuint shader, GLenum pname, GLint* params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetShaderiv 0x%04X 0x%04X\n",
                         gc, tid, shader, pname);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetShaderiv(gc, shader, pname, params);
    __GLES_PROFILE_FOOTER(GLES3_GETSHADERIV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetShaderiv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glesTracerDispatchTable.GetShaderiv)
    {
        (*__glesTracerDispatchTable.GetShaderiv)(shader, pname, params);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetShaderInfoLog(__GLcontext *gc, GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetShaderInfoLog %d %d\n",
                        gc, tid, shader, bufsize);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetShaderInfoLog(gc, shader, bufsize, length, infolog);
    __GLES_PROFILE_FOOTER(GLES3_GETSHADERINFOLOG);

    if (bufsize && (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST))
    {
        __GLES_LOG_API("        glGetShaderInfoLog => %d %s\n", __GL_PTRVALUE(length), infolog);
    }

    if (__glesTracerDispatchTable.GetShaderInfoLog)
    {
        (*__glesTracerDispatchTable.GetShaderInfoLog)(shader, bufsize, length, infolog);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetShaderPrecisionFormat(__GLcontext *gc, GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetShaderPrecisionFormat 0x%04X 0x%04X\n",
                        gc, tid, shadertype, precisiontype);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetShaderPrecisionFormat(gc, shadertype, precisiontype, range, precision);
    __GLES_PROFILE_FOOTER(GLES3_GETSHADERPRECISIONFORMAT);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetShaderPrecisionFormat => %d %d\n", __GL_PTRVALUE(range), __GL_PTRVALUE(precision));
    }

    if (__glesTracerDispatchTable.GetShaderPrecisionFormat)
    {
        (*__glesTracerDispatchTable.GetShaderPrecisionFormat)(shadertype, precisiontype, range, precision);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetShaderSource(__GLcontext *gc, GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetShaderSource %d %d\n",
                        gc, tid, shader, bufsize);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetShaderSource(gc, shader, bufsize, length, source);
    __GLES_PROFILE_FOOTER(GLES3_GETSHADERSOURCE);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetShaderSource => %d\n####\n%s\n####\n", __GL_PTRVALUE(length), source);
    }

    if (__glesTracerDispatchTable.GetShaderSource)
    {
        (*__glesTracerDispatchTable.GetShaderSource)(shader, bufsize, length, source);
    }
}

const GLubyte* GLAPIENTRY __glesProfile_GetString(__GLcontext *gc, GLenum name)
{
    __GLES_PROFILE_VARS();
    const GLubyte *string;

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetString 0x%04X\n",
                        gc, tid, name);
    }

    __GLES_PROFILE_HEADER();
    string = __gles_GetString(gc, name);
    __GLES_PROFILE_FOOTER(GLES3_GETSTRING);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetString => %s\n", string);
    }

    if (__glesTracerDispatchTable.GetString)
    {
        (*__glesTracerDispatchTable.GetString)(name);
    }

    return string;
}

GLvoid GLAPIENTRY __glesProfile_GetTexParameterfv(__GLcontext *gc, GLenum target, GLenum pname, GLfloat* params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetTexParameterfv 0x%04X 0x%04X\n",
                        gc, tid, target, pname);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetTexParameterfv(gc, target, pname, params);
    __GLES_PROFILE_FOOTER(GLES3_GETTEXPARAMETERFV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetTexParameterfv => %f\n", __GL_PTRVALUE(params));
    }

    if (__glesTracerDispatchTable.GetTexParameterfv)
    {
        (*__glesTracerDispatchTable.GetTexParameterfv)(target, pname, params);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetTexParameteriv(__GLcontext *gc, GLenum target, GLenum pname, GLint* params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetTexParameteriv 0x%04X 0x%04X\n",
                        gc, tid, target, pname);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetTexParameteriv(gc, target, pname, params);
    __GLES_PROFILE_FOOTER(GLES3_GETTEXPARAMETERIV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetTexParameteriv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glesTracerDispatchTable.GetTexParameteriv)
    {
        (*__glesTracerDispatchTable.GetTexParameteriv)(target, pname, params);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetUniformfv(__GLcontext *gc, GLuint program, GLint location, GLfloat* params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetUniformfv %d %d\n",
                        gc, tid, program, location);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetUniformfv(gc, program, location, params);
    __GLES_PROFILE_FOOTER(GLES3_GETUNIFORMFV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetUniformfv => %f\n", __GL_PTRVALUE(params));
    }

    if (__glesTracerDispatchTable.GetUniformfv)
    {
        (*__glesTracerDispatchTable.GetUniformfv)(program, location, params);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetUniformiv(__GLcontext *gc, GLuint program, GLint location, GLint* params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetUniformiv %d %d\n",
                        gc, tid, program, location);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetUniformiv(gc, program, location, params);
    __GLES_PROFILE_FOOTER(GLES3_GETUNIFORMIV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetUniformiv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glesTracerDispatchTable.GetUniformiv)
    {
        (*__glesTracerDispatchTable.GetUniformiv)(program, location, params);
    }
}

GLint GLAPIENTRY __glesProfile_GetUniformLocation(__GLcontext *gc, GLuint program, const GLchar* name)
{
    __GLES_PROFILE_VARS();
    GLint location;

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetUniformLocation %d %s\n",
                        gc, tid, program, name);
    }

    __GLES_PROFILE_HEADER();
    location = __gles_GetUniformLocation(gc, program, name);
    __GLES_PROFILE_FOOTER(GLES3_GETUNIFORMLOCATION);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetUniformLocation => %d\n", location);
    }

    if (__glesTracerDispatchTable.GetUniformLocation)
    {
        (*__glesTracerDispatchTable.GetUniformLocation)(program, name, location);
    }

    return location;
}

GLvoid GLAPIENTRY __glesProfile_GetVertexAttribfv(__GLcontext *gc, GLuint index, GLenum pname, GLfloat* params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetVertexAttribfv %d 0x%04X\n",
                        gc, tid, index, pname);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetVertexAttribfv(gc, index, pname, params);
    __GLES_PROFILE_FOOTER(GLES3_GETVERTEXATTRIBFV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetVertexAttribfv => %f\n", __GL_PTRVALUE(params));
    }

    if (__glesTracerDispatchTable.GetVertexAttribfv)
    {
        (*__glesTracerDispatchTable.GetVertexAttribfv)(index, pname, params);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetVertexAttribiv(__GLcontext *gc, GLuint index, GLenum pname, GLint* params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetVertexAttribiv %d 0x%04X\n",
                        gc, tid, index, pname);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetVertexAttribiv(gc, index, pname, params);
    __GLES_PROFILE_FOOTER(GLES3_GETVERTEXATTRIBIV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetVertexAttribiv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glesTracerDispatchTable.GetVertexAttribiv)
    {
        (*__glesTracerDispatchTable.GetVertexAttribiv)(index, pname, params);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetVertexAttribPointerv(__GLcontext *gc, GLuint index, GLenum pname, GLvoid** pointer)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetVertexAttribPointerv %d 0x%04X\n",
                        gc, tid, index, pname);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetVertexAttribPointerv(gc, index, pname, pointer);
    __GLES_PROFILE_FOOTER(GLES3_GETVERTEXATTRIBPOINTERV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetVertexAttribPointerv => 0x%08X\n", __GL_PTR2UINT(__GL_PTRVALUE(pointer)));
    }

    if (__glesTracerDispatchTable.GetVertexAttribPointerv)
    {
        (*__glesTracerDispatchTable.GetVertexAttribPointerv)(index, pname, pointer);
    }
}

GLvoid GLAPIENTRY __glesProfile_Hint(__GLcontext *gc, GLenum target, GLenum mode)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glHint 0x%04X 0x%04X\n",
                        gc, tid, target, mode);
    }

    __GLES_PROFILE_HEADER();
    __gles_Hint(gc, target, mode);
    __GLES_PROFILE_FOOTER(GLES3_HINT);

    if (__glesTracerDispatchTable.Hint)
    {
        (*__glesTracerDispatchTable.Hint)(target, mode);
    }
}

GLboolean GLAPIENTRY __glesProfile_IsBuffer(__GLcontext *gc, GLuint buffer)
{
    __GLES_PROFILE_VARS();
    GLboolean is;

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glIsBuffer %d\n",
                        gc, tid, buffer);
    }

    __GLES_PROFILE_HEADER();
    is = __gles_IsBuffer(gc, buffer);
    __GLES_PROFILE_FOOTER(GLES3_ISBUFFER);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glIsBuffer => %d\n", is);
    }

    if (__glesTracerDispatchTable.IsBuffer)
    {
        (*__glesTracerDispatchTable.IsBuffer)(buffer);
    }

    return is;
}

GLboolean GLAPIENTRY __glesProfile_IsEnabled(__GLcontext *gc, GLenum cap)
{
    __GLES_PROFILE_VARS();
    GLboolean is;

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glIsEnabled 0x%04X\n",
                        gc, tid, cap);
    }

    __GLES_PROFILE_HEADER();
    is = __gles_IsEnabled(gc, cap);
    __GLES_PROFILE_FOOTER(GLES3_ISENABLED);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glIsEnabled => %d\n", is);
    }

    if (__glesTracerDispatchTable.IsEnabled)
    {
        (*__glesTracerDispatchTable.IsEnabled)(cap);
    }

    return is;
}

GLboolean GLAPIENTRY __glesProfile_IsFramebuffer(__GLcontext *gc, GLuint framebuffer)
{
    __GLES_PROFILE_VARS();
    GLboolean is;

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glIsFramebuffer %d\n",
                        gc, tid, framebuffer);
    }

    __GLES_PROFILE_HEADER();
    is = __gles_IsFramebuffer(gc, framebuffer);
    __GLES_PROFILE_FOOTER(GLES3_ISFRAMEBUFFER);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glIsFramebuffer => %d\n", is);
    }

    if (__glesTracerDispatchTable.IsFramebuffer)
    {
        (*__glesTracerDispatchTable.IsFramebuffer)(framebuffer);
    }

    return is;
}

GLboolean GLAPIENTRY __glesProfile_IsProgram(__GLcontext *gc, GLuint program)
{
    __GLES_PROFILE_VARS();
    GLboolean is;

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glIsProgram %d\n",
                        gc, tid, program);
    }

    __GLES_PROFILE_HEADER();
    is = __gles_IsProgram(gc, program);
    __GLES_PROFILE_FOOTER(GLES3_ISPROGRAM);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glIsProgram => %d\n", is);
    }

    if (__glesTracerDispatchTable.IsProgram)
    {
        (*__glesTracerDispatchTable.IsProgram)(program);
    }

    return is;
}

GLboolean GLAPIENTRY __glesProfile_IsRenderbuffer(__GLcontext *gc, GLuint renderbuffer)
{
    __GLES_PROFILE_VARS();
    GLboolean is;

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glIsRenderbuffer %d\n",
                        gc, tid, renderbuffer);
    }

    __GLES_PROFILE_HEADER();
    is = __gles_IsRenderbuffer(gc, renderbuffer);
    __GLES_PROFILE_FOOTER(GLES3_ISRENDERBUFFER);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glIsRenderbuffer => %d\n", is);
    }

    if (__glesTracerDispatchTable.IsRenderbuffer)
    {
        (*__glesTracerDispatchTable.IsRenderbuffer)(renderbuffer);
    }

    return is;
}

GLboolean GLAPIENTRY __glesProfile_IsShader(__GLcontext *gc, GLuint shader)
{
    __GLES_PROFILE_VARS();
    GLboolean is;

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glIsShader %d\n",
                        gc, tid, shader);
    }

    __GLES_PROFILE_HEADER();
    is = __gles_IsShader(gc, shader);
    __GLES_PROFILE_FOOTER(GLES3_ISSHADER);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glIsShader => %d\n", is);
    }

    if (__glesTracerDispatchTable.IsShader)
    {
        (*__glesTracerDispatchTable.IsShader)(shader);
    }

    return is;
}

GLboolean GLAPIENTRY __glesProfile_IsTexture(__GLcontext *gc, GLuint texture)
{
    __GLES_PROFILE_VARS();
    GLboolean is;

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glIsTexture %d\n",
                        gc, tid, texture);
    }

    __GLES_PROFILE_HEADER();
    is = __gles_IsTexture(gc, texture);
    __GLES_PROFILE_FOOTER(GLES3_ISTEXTURE);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glIsTexture => %d\n", is);
    }

    if (__glesTracerDispatchTable.IsTexture)
    {
        (*__glesTracerDispatchTable.IsTexture)(texture);
    }

    return is;
}

GLvoid GLAPIENTRY __glesProfile_LineWidth(__GLcontext *gc, GLfloat width)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glLineWidth %f\n",
                        gc, tid, width);
    }

    __GLES_PROFILE_HEADER();
    __gles_LineWidth(gc, width);
    __GLES_PROFILE_FOOTER(GLES3_LINEWIDTH);

    if (__glesTracerDispatchTable.LineWidth)
    {
        (*__glesTracerDispatchTable.LineWidth)(width);
    }
}

GLvoid GLAPIENTRY __glesProfile_LinkProgram(__GLcontext *gc, GLuint program)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glLinkProgram %d\n",
                        gc, tid, program);
    }

    __GLES_PROFILE_HEADER();
    __gles_LinkProgram(gc, program);
    __GLES_PROFILE_FOOTER(GLES3_LINKPROGRAM);

    if (__glesTracerDispatchTable.LinkProgram)
    {
        (*__glesTracerDispatchTable.LinkProgram)(program);
    }
}

GLvoid GLAPIENTRY __glesProfile_PixelStorei(__GLcontext *gc, GLenum pname, GLint param)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glPixelStorei 0x%04X %d\n",
                        gc, tid, pname, param);
    }

    __GLES_PROFILE_HEADER();
    __gles_PixelStorei(gc, pname, param);
    __GLES_PROFILE_FOOTER(GLES3_PIXELSTOREI);

    if (__glesTracerDispatchTable.PixelStorei)
    {
        (*__glesTracerDispatchTable.PixelStorei)(pname, param);
    }
}

GLvoid GLAPIENTRY __glesProfile_PolygonOffset(__GLcontext *gc, GLfloat factor, GLfloat units)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glPolygonOffset %f %f\n",
                        gc, tid, factor, units);
    }

    __GLES_PROFILE_HEADER();
    __gles_PolygonOffset(gc, factor, units);
    __GLES_PROFILE_FOOTER(GLES3_POLYGONOFFSET);

    if (__glesTracerDispatchTable.PolygonOffset)
    {
        (*__glesTracerDispatchTable.PolygonOffset)(factor, units);
    }
}

GLvoid GLAPIENTRY __glesProfile_ReadPixels(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glReadPixels %d %d %d %d 0x%04X 0x%04X 0x%08X\n",
                        gc, tid, x, y, width, height, format, type, __GL_PTR2UINT(pixels));
    }

    __GLES_PROFILE_HEADER();
    __gles_ReadPixels(gc, x, y, width, height, format, type, pixels);
    __GLES_PROFILE_FOOTER(GLES3_READPIXELS);

    if (__glesTracerDispatchTable.ReadPixels)
    {
        (*__glesTracerDispatchTable.ReadPixels)(x, y, width, height, format, type, pixels);
    }
}

GLvoid GLAPIENTRY __glesProfile_ReleaseShaderCompiler(__GLcontext *gc)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glReleaseShaderCompiler\n", gc, tid);
    }

    __GLES_PROFILE_HEADER();
    __gles_ReleaseShaderCompiler(gc);
    __GLES_PROFILE_FOOTER(GLES3_RELEASESHADERCOMPILER);

    if (__glesTracerDispatchTable.ReleaseShaderCompiler)
    {
        (*__glesTracerDispatchTable.ReleaseShaderCompiler)();
    }
}

GLvoid GLAPIENTRY __glesProfile_RenderbufferStorage(__GLcontext *gc, GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glRenderbufferStorage 0x%04X 0x%04X %d %d\n",
                        gc, tid, target, internalformat, width, height);
    }

    __GLES_PROFILE_HEADER();
    __gles_RenderbufferStorage(gc, target, internalformat, width, height);
    __GLES_PROFILE_FOOTER(GLES3_RENDERBUFFERSTORAGE);

    if (__glesTracerDispatchTable.RenderbufferStorage)
    {
        (*__glesTracerDispatchTable.RenderbufferStorage)(target, internalformat, width, height);
    }
}

GLvoid GLAPIENTRY __glesProfile_SampleCoverage(__GLcontext *gc, GLfloat value, GLboolean invert)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glSampleCoverage %f %d\n",
                        gc, tid, value, invert);
    }

    __GLES_PROFILE_HEADER();
    __gles_SampleCoverage(gc, value, invert);
    __GLES_PROFILE_FOOTER(GLES3_SAMPLECOVERAGE);

    if (__glesTracerDispatchTable.SampleCoverage)
    {
        (*__glesTracerDispatchTable.SampleCoverage)(value, invert);
    }
}

GLvoid GLAPIENTRY __glesProfile_Scissor(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glScissor %d %d %d %d\n",
                        gc, tid, x, y, width, height);
    }

    __GLES_PROFILE_HEADER();
    __gles_Scissor(gc, x, y, width, height);
    __GLES_PROFILE_FOOTER(GLES3_SCISSOR);

    if (__glesTracerDispatchTable.Scissor)
    {
        (*__glesTracerDispatchTable.Scissor)(x, y, width, height);
    }
}

GLvoid GLAPIENTRY __glesProfile_ShaderBinary(__GLcontext *gc, GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glShaderBinary %d 0x%08X 0x%04X 0x%08X %d\n",
                        gc, tid, n, __GL_PTR2UINT(shaders), binaryformat, __GL_PTR2UINT(binary), length);
    }

    __GLES_PROFILE_HEADER();
    __gles_ShaderBinary(gc, n, shaders, binaryformat, binary, length);
    __GLES_PROFILE_FOOTER(GLES3_SHADERBINARY);

    if (__glesTracerDispatchTable.ShaderBinary)
    {
        (*__glesTracerDispatchTable.ShaderBinary)(n, shaders, binaryformat, binary, length);
    }
}

GLvoid GLAPIENTRY __glesProfile_ShaderSource(__GLcontext *gc, GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glShaderSource %d %d %d 0x%08X \n",
                        gc, tid, shader, count, __GL_PTR2UINT(string), __GL_PTRVALUE(length));

        __glesLogSourceStrings(gc, count, string);
    }

    __GLES_PROFILE_HEADER();
    __gles_ShaderSource(gc, shader, count, string, length);
    __GLES_PROFILE_FOOTER(GLES3_SHADERSOURCE);

    if (__glesTracerDispatchTable.ShaderSource)
    {
        (*__glesTracerDispatchTable.ShaderSource)(shader, count, string, length);
    }
}

GLvoid GLAPIENTRY __glesProfile_StencilFunc(__GLcontext *gc, GLenum func, GLint ref, GLuint mask)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glStencilFunc 0x%04X %d 0x%08X\n",
                        gc, tid, func, ref, mask);
    }

    __GLES_PROFILE_HEADER();
    __gles_StencilFunc(gc, func, ref, mask);
    __GLES_PROFILE_FOOTER(GLES3_STENCILFUNC);

    if (__glesTracerDispatchTable.StencilFunc)
    {
        (*__glesTracerDispatchTable.StencilFunc)(func, ref, mask);
    }
}

GLvoid GLAPIENTRY __glesProfile_StencilFuncSeparate(__GLcontext *gc, GLenum face, GLenum func, GLint ref, GLuint mask)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glStencilFuncSeparate 0x%04X 0x%04X %d 0x%08X\n",
                        gc, tid, face, func, ref, mask);
    }

    __GLES_PROFILE_HEADER();
    __gles_StencilFuncSeparate(gc, face, func, ref, mask);
    __GLES_PROFILE_FOOTER(GLES3_STENCILFUNCSEPARATE);

    if (__glesTracerDispatchTable.StencilFuncSeparate)
    {
        (*__glesTracerDispatchTable.StencilFuncSeparate)(face, func, ref, mask);
    }
}

GLvoid GLAPIENTRY __glesProfile_StencilMask(__GLcontext *gc, GLuint mask)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glStencilMask 0x%08X\n",
                        gc, tid, mask);
    }

    __GLES_PROFILE_HEADER();
    __gles_StencilMask(gc, mask);
    __GLES_PROFILE_FOOTER(GLES3_STENCILMASK);

    if (__glesTracerDispatchTable.StencilMask)
    {
        (*__glesTracerDispatchTable.StencilMask)(mask);
    }
}

GLvoid GLAPIENTRY __glesProfile_StencilMaskSeparate(__GLcontext *gc, GLenum face, GLuint mask)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glStencilMaskSeparate 0x%04X 0x%08X\n",
                        gc, tid, face, mask);
    }

    __GLES_PROFILE_HEADER();
    __gles_StencilMaskSeparate(gc, face, mask);
    __GLES_PROFILE_FOOTER(GLES3_STENCILMASKSEPARATE);

    if (__glesTracerDispatchTable.StencilMaskSeparate)
    {
        (*__glesTracerDispatchTable.StencilMaskSeparate)(face, mask);
    }
}

GLvoid GLAPIENTRY __glesProfile_StencilOp(__GLcontext *gc, GLenum fail, GLenum zfail, GLenum zpass)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glStencilOp 0x%04X 0x%04X 0x%04X\n",
                        gc, tid, fail, zfail, zpass);
    }

    __GLES_PROFILE_HEADER();
    __gles_StencilOp(gc, fail, zfail, zpass);
    __GLES_PROFILE_FOOTER(GLES3_STENCILOP);

    if (__glesTracerDispatchTable.StencilOp)
    {
        (*__glesTracerDispatchTable.StencilOp)(fail, zfail, zpass);
    }
}

GLvoid GLAPIENTRY __glesProfile_StencilOpSeparate(__GLcontext *gc, GLenum face, GLenum fail, GLenum zfail, GLenum zpass)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glStencilOpSeparate 0x%04X 0x%04X 0x%04X 0x%04X\n",
                        gc, tid, face, fail, zfail, zpass);
    }

    __GLES_PROFILE_HEADER();
    __gles_StencilOpSeparate(gc, face, fail, zfail, zpass);
    __GLES_PROFILE_FOOTER(GLES3_STENCILOPSEPARATE);

    if (__glesTracerDispatchTable.StencilOpSeparate)
    {
        (*__glesTracerDispatchTable.StencilOpSeparate)(face, fail, zfail, zpass);
    }
}

GLvoid GLAPIENTRY __glesProfile_TexImage2D(__GLcontext *gc, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glTexImage2D 0x%04X %d 0x%04X %d %d %d 0x%04X 0x%04X 0x%08X\n",
                        gc, tid, target, level, internalformat, width, height, border, format, type, __GL_PTR2UINT(pixels));
    }

    __GLES_PROFILE_HEADER();
    __gles_TexImage2D(gc, target, level, internalformat, width, height, border, format, type, pixels);
    __GLES_PROFILE_FOOTER(GLES3_TEXIMAGE2D);

    if (__glesTracerDispatchTable.TexImage2D)
    {
        (*__glesTracerDispatchTable.TexImage2D)(target, level, internalformat, width, height, border, format, type, pixels);
    }
}

GLvoid GLAPIENTRY __glesProfile_TexParameterf(__GLcontext *gc, GLenum target, GLenum pname, GLfloat param)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glTexParameterf 0x%04X 0x%04X %f\n",
                        gc, tid, target, pname, param);
    }

    __GLES_PROFILE_HEADER();
    __gles_TexParameterf(gc, target, pname, param);
    __GLES_PROFILE_FOOTER(GLES3_TEXPARAMETERF);

    if (__glesTracerDispatchTable.TexParameterf)
    {
        (*__glesTracerDispatchTable.TexParameterf)(target, pname, param);
    }
}

GLvoid GLAPIENTRY __glesProfile_TexParameterfv(__GLcontext *gc, GLenum target, GLenum pname, const GLfloat* params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glTexParameterfv 0x%04X 0x%04X %f\n",
                        gc, tid, target, pname, __GL_PTRVALUE(params));
    }

    __GLES_PROFILE_HEADER();
    __gles_TexParameterfv(gc, target, pname, params);
    __GLES_PROFILE_FOOTER(GLES3_TEXPARAMETERFV);

    if (__glesTracerDispatchTable.TexParameterfv)
    {
        (*__glesTracerDispatchTable.TexParameterfv)(target, pname, params);
    }
}

GLvoid GLAPIENTRY __glesProfile_TexParameteri(__GLcontext *gc, GLenum target, GLenum pname, GLint param)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glTexParameteri 0x%04X 0x%04X %d\n",
                        gc, tid, target, pname, param);
    }

    __GLES_PROFILE_HEADER();
    __gles_TexParameteri(gc, target, pname, param);
    __GLES_PROFILE_FOOTER(GLES3_TEXPARAMETERI);

    if (__glesTracerDispatchTable.TexParameteri)
    {
        (*__glesTracerDispatchTable.TexParameteri)(target, pname, param);
    }
}

GLvoid GLAPIENTRY __glesProfile_TexParameteriv(__GLcontext *gc, GLenum target, GLenum pname, const GLint* params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glTexParameteriv 0x%04X 0x%04X %d\n",
                        gc, tid, target, pname, __GL_PTRVALUE(params));
    }

    __GLES_PROFILE_HEADER();
    __gles_TexParameteriv(gc, target, pname, params);
    __GLES_PROFILE_FOOTER(GLES3_TEXPARAMETERIV);

    if (__glesTracerDispatchTable.TexParameteriv)
    {
        (*__glesTracerDispatchTable.TexParameteriv)(target, pname, params);
    }
}

GLvoid GLAPIENTRY __glesProfile_TexSubImage2D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glTexSubImage2D 0x%04X %d %d %d %d %d 0x%04X 0x%04X 0x%08X\n",
                        gc, tid, target, level, xoffset, yoffset, width, height, format, type, __GL_PTR2UINT(pixels));
    }

    __GLES_PROFILE_HEADER();
    __gles_TexSubImage2D(gc, target, level, xoffset, yoffset, width, height, format, type, pixels);
    __GLES_PROFILE_FOOTER(GLES3_TEXSUBIMAGE2D);

    if (__glesTracerDispatchTable.TexSubImage2D)
    {
        (*__glesTracerDispatchTable.TexSubImage2D)(target, level, xoffset, yoffset, width, height, format, type, pixels);
    }
}

GLvoid GLAPIENTRY __glesProfile_Uniform1f(__GLcontext *gc, GLint location, GLfloat x)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glUniform1f %d %f\n",
                        gc, tid, location, x);
    }

    __GLES_PROFILE_HEADER();
    __gles_Uniform1f(gc, location, x);
    __GLES_PROFILE_FOOTER(GLES3_UNIFORM1F);

    if (__glesTracerDispatchTable.Uniform1f)
    {
        (*__glesTracerDispatchTable.Uniform1f)(location, x);
    }
}

GLvoid GLAPIENTRY __glesProfile_Uniform1fv(__GLcontext *gc, GLint location, GLsizei count, const GLfloat* v)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glUniform1fv %d %d 0x%08X\n",
                        gc, tid, location, count, __GL_PTR2UINT(v));
    }

    __GLES_PROFILE_HEADER();
    __gles_Uniform1fv(gc, location, count, v);
    __GLES_PROFILE_FOOTER(GLES3_UNIFORM1FV);

    if (__glesTracerDispatchTable.Uniform1fv)
    {
        (*__glesTracerDispatchTable.Uniform1fv)(location, count, v);
    }
}

GLvoid GLAPIENTRY __glesProfile_Uniform1i(__GLcontext *gc, GLint location, GLint x)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glUniform1i %d %d\n",
                        gc, tid, location, x);
    }

    __GLES_PROFILE_HEADER();
    __gles_Uniform1i(gc, location, x);
    __GLES_PROFILE_FOOTER(GLES3_UNIFORM1I);

    if (__glesTracerDispatchTable.Uniform1i)
    {
        (*__glesTracerDispatchTable.Uniform1i)(location, x);
    }
}

GLvoid GLAPIENTRY __glesProfile_Uniform1iv(__GLcontext *gc, GLint location, GLsizei count, const GLint* v)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glUniform1iv %d %d 0x%08X\n",
                        gc, tid, location, count, __GL_PTR2UINT(v));
    }

    __GLES_PROFILE_HEADER();
    __gles_Uniform1iv(gc, location, count, v);
    __GLES_PROFILE_FOOTER(GLES3_UNIFORM1IV);

    if (__glesTracerDispatchTable.Uniform1iv)
    {
        (*__glesTracerDispatchTable.Uniform1iv)(location, count, v);
    }
}

GLvoid GLAPIENTRY __glesProfile_Uniform2f(__GLcontext *gc, GLint location, GLfloat x, GLfloat y)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glUniform2f %d %f %f\n",
                        gc, tid, location, x, y);
    }

    __GLES_PROFILE_HEADER();
    __gles_Uniform2f(gc, location, x, y);
    __GLES_PROFILE_FOOTER(GLES3_UNIFORM2F);

    if (__glesTracerDispatchTable.Uniform2f)
    {
        (*__glesTracerDispatchTable.Uniform2f)(location, x, y);
    }
}

GLvoid GLAPIENTRY __glesProfile_Uniform2fv(__GLcontext *gc, GLint location, GLsizei count, const GLfloat* v)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glUniform2fv %d %d 0x%08X\n",
                        gc, tid, location, count, __GL_PTR2UINT(v));
    }

    __GLES_PROFILE_HEADER();
    __gles_Uniform2fv(gc, location, count, v);
    __GLES_PROFILE_FOOTER(GLES3_UNIFORM2FV);

    if (__glesTracerDispatchTable.Uniform2fv)
    {
        (*__glesTracerDispatchTable.Uniform2fv)(location, count, v);
    }
}

GLvoid GLAPIENTRY __glesProfile_Uniform2i(__GLcontext *gc, GLint location, GLint x, GLint y)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glUniform2i %d %d %d\n",
                        gc, tid, location, x, y);
    }

    __GLES_PROFILE_HEADER();
    __gles_Uniform2i(gc, location, x, y);
    __GLES_PROFILE_FOOTER(GLES3_UNIFORM2I);

    if (__glesTracerDispatchTable.Uniform2i)
    {
        (*__glesTracerDispatchTable.Uniform2i)(location, x, y);
    }
}

GLvoid GLAPIENTRY __glesProfile_Uniform2iv(__GLcontext *gc, GLint location, GLsizei count, const GLint* v)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glUniform2iv %d %d 0x%08X\n",
                        gc, tid, location, count, __GL_PTR2UINT(v));
    }

    __GLES_PROFILE_HEADER();
    __gles_Uniform2iv(gc, location, count, v);
    __GLES_PROFILE_FOOTER(GLES3_UNIFORM2IV);

    if (__glesTracerDispatchTable.Uniform2iv)
    {
        (*__glesTracerDispatchTable.Uniform2iv)(location, count, v);
    }
}

GLvoid GLAPIENTRY __glesProfile_Uniform3f(__GLcontext *gc, GLint location, GLfloat x, GLfloat y, GLfloat z)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glUniform3f %d %f %f %f\n",
                        gc, tid, location, x, y, z);
    }

    __GLES_PROFILE_HEADER();
    __gles_Uniform3f(gc, location, x, y, z);
    __GLES_PROFILE_FOOTER(GLES3_UNIFORM3F);

    if (__glesTracerDispatchTable.Uniform3f)
    {
        (*__glesTracerDispatchTable.Uniform3f)(location, x, y, z);
    }
}

GLvoid GLAPIENTRY __glesProfile_Uniform3fv(__GLcontext *gc, GLint location, GLsizei count, const GLfloat* v)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glUniform3fv %d %d 0x%08X\n",
                        gc, tid, location, count, __GL_PTR2UINT(v));
    }

    __GLES_PROFILE_HEADER();
    __gles_Uniform3fv(gc, location, count, v);
    __GLES_PROFILE_FOOTER(GLES3_UNIFORM3FV);

    if (__glesTracerDispatchTable.Uniform3fv)
    {
        (*__glesTracerDispatchTable.Uniform3fv)(location, count, v);
    }
}

GLvoid GLAPIENTRY __glesProfile_Uniform3i(__GLcontext *gc, GLint location, GLint x, GLint y, GLint z)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glUniform3i %d %d %d %d\n",
                        gc, tid, location, x, y, z);
    }

    __GLES_PROFILE_HEADER();
    __gles_Uniform3i(gc, location, x, y, z);
    __GLES_PROFILE_FOOTER(GLES3_UNIFORM3I);

    if (__glesTracerDispatchTable.Uniform3i)
    {
        (*__glesTracerDispatchTable.Uniform3i)(location, x, y, z);
    }
}

GLvoid GLAPIENTRY __glesProfile_Uniform3iv(__GLcontext *gc, GLint location, GLsizei count, const GLint* v)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glUniform3iv %d %d 0x%08X\n",
                        gc, tid, location, count, __GL_PTR2UINT(v));
    }

    __GLES_PROFILE_HEADER();
    __gles_Uniform3iv(gc, location, count, v);
    __GLES_PROFILE_FOOTER(GLES3_UNIFORM3IV);

    if (__glesTracerDispatchTable.Uniform3iv)
    {
        (*__glesTracerDispatchTable.Uniform3iv)(location, count, v);
    }
}

GLvoid GLAPIENTRY __glesProfile_Uniform4f(__GLcontext *gc, GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glUniform4f %d %f %f %f %f\n",
                        gc, tid, location, x, y, z, w);
    }

    __GLES_PROFILE_HEADER();
    __gles_Uniform4f(gc, location, x, y, z, w);
    __GLES_PROFILE_FOOTER(GLES3_UNIFORM4F);

    if (__glesTracerDispatchTable.Uniform4f)
    {
        (*__glesTracerDispatchTable.Uniform4f)(location, x, y, z, w);
    }
}

GLvoid GLAPIENTRY __glesProfile_Uniform4fv(__GLcontext *gc, GLint location, GLsizei count, const GLfloat* v)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glUniform4fv %d %d 0x%08X\n",
                        gc, tid, location, count, __GL_PTR2UINT(v));
    }

    __GLES_PROFILE_HEADER();
    __gles_Uniform4fv(gc, location, count, v);
    __GLES_PROFILE_FOOTER(GLES3_UNIFORM4FV);

    if (__glesTracerDispatchTable.Uniform4fv)
    {
        (*__glesTracerDispatchTable.Uniform4fv)(location, count, v);
    }
}

GLvoid GLAPIENTRY __glesProfile_Uniform4i(__GLcontext *gc, GLint location, GLint x, GLint y, GLint z, GLint w)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glUniform4i %d %d %d %d %d\n",
                        gc, tid, location, x, y, z, w);
    }

    __GLES_PROFILE_HEADER();
    __gles_Uniform4i(gc, location, x, y, z, w);
    __GLES_PROFILE_FOOTER(GLES3_UNIFORM4I);

    if (__glesTracerDispatchTable.Uniform4i)
    {
        (*__glesTracerDispatchTable.Uniform4i)(location, x, y, z, w);
    }
}

GLvoid GLAPIENTRY __glesProfile_Uniform4iv(__GLcontext *gc, GLint location, GLsizei count, const GLint* v)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glUniform4iv %d %d 0x%08X\n",
                        gc, tid, location, count, __GL_PTR2UINT(v));
    }

    __GLES_PROFILE_HEADER();
    __gles_Uniform4iv(gc, location, count, v);
    __GLES_PROFILE_FOOTER(GLES3_UNIFORM4IV);

    if (__glesTracerDispatchTable.Uniform4iv)
    {
        (*__glesTracerDispatchTable.Uniform4iv)(location, count, v);
    }
}

GLvoid GLAPIENTRY __glesProfile_UniformMatrix2fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glUniformMatrix2fv %d %d %d 0x%08X\n",
                        gc, tid, location, count, transpose, __GL_PTR2UINT(value));
    }

    __GLES_PROFILE_HEADER();
    __gles_UniformMatrix2fv(gc, location, count, transpose, value);
    __GLES_PROFILE_FOOTER(GLES3_UNIFORMMATRIX2FV);

    if (__glesTracerDispatchTable.UniformMatrix2fv)
    {
        (*__glesTracerDispatchTable.UniformMatrix2fv)(location, count, transpose, value);
    }
}

GLvoid GLAPIENTRY __glesProfile_UniformMatrix3fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glUniformMatrix3fv %d %d %d 0x%08X\n",
                        gc, tid, location, count, transpose, __GL_PTR2UINT(value));
    }

    __GLES_PROFILE_HEADER();
    __gles_UniformMatrix3fv(gc, location, count, transpose, value);
    __GLES_PROFILE_FOOTER(GLES3_UNIFORMMATRIX3FV);

    if (__glesTracerDispatchTable.UniformMatrix3fv)
    {
        (*__glesTracerDispatchTable.UniformMatrix3fv)(location, count, transpose, value);
    }
}

GLvoid GLAPIENTRY __glesProfile_UniformMatrix4fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glUniformMatrix4fv %d %d %d 0x%08X\n",
                        gc, tid, location, count, transpose, __GL_PTR2UINT(value));
    }

    __GLES_PROFILE_HEADER();
    __gles_UniformMatrix4fv(gc, location, count, transpose, value);
    __GLES_PROFILE_FOOTER(GLES3_UNIFORMMATRIX4FV);

    if (__glesTracerDispatchTable.UniformMatrix4fv)
    {
        (*__glesTracerDispatchTable.UniformMatrix4fv)(location, count, transpose, value);
    }
}

GLvoid GLAPIENTRY __glesProfile_UseProgram(__GLcontext *gc, GLuint program)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glUseProgram %d\n",
                        gc, tid, program);
    }

    __GLES_PROFILE_HEADER();
    __gles_UseProgram(gc, program);
    __GLES_PROFILE_FOOTER(GLES3_USEPROGRAM);

    if (__glesTracerDispatchTable.UseProgram)
    {
        (*__glesTracerDispatchTable.UseProgram)(program);
    }
}

GLvoid GLAPIENTRY __glesProfile_ValidateProgram(__GLcontext *gc, GLuint program)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glValidateProgram %d\n",
                        gc, tid, program);
    }

    __GLES_PROFILE_HEADER();
    __gles_ValidateProgram(gc, program);
    __GLES_PROFILE_FOOTER(GLES3_VALIDATEPROGRAM);

    if (__glesTracerDispatchTable.ValidateProgram)
    {
        (*__glesTracerDispatchTable.ValidateProgram)(program);
    }
}

GLvoid GLAPIENTRY __glesProfile_VertexAttrib1f(__GLcontext *gc, GLuint indx, GLfloat x)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glVertexAttrib1f %d %f\n",
                        gc, tid, indx, x);
    }

    __GLES_PROFILE_HEADER();
    __gles_VertexAttrib1f(gc, indx, x);
    __GLES_PROFILE_FOOTER(GLES3_VERTEXATTRIB1F);

    if (__glesTracerDispatchTable.VertexAttrib1f)
    {
        (*__glesTracerDispatchTable.VertexAttrib1f)(indx, x);
    }
}

GLvoid GLAPIENTRY __glesProfile_VertexAttrib1fv(__GLcontext *gc, GLuint indx, const GLfloat* values)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glVertexAttrib1fv %d 0x%08X\n",
                        gc, tid, indx, __GL_PTR2UINT(values));
    }

    __GLES_PROFILE_HEADER();
    __gles_VertexAttrib1fv(gc, indx, values);
    __GLES_PROFILE_FOOTER(GLES3_VERTEXATTRIB1FV);

    if (__glesTracerDispatchTable.VertexAttrib1fv)
    {
        (*__glesTracerDispatchTable.VertexAttrib1fv)(indx, values);
    }
}

GLvoid GLAPIENTRY __glesProfile_VertexAttrib2f(__GLcontext *gc, GLuint indx, GLfloat x, GLfloat y)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glVertexAttrib2f %d %f %f\n",
                        gc, tid, indx, x, y);
    }

    __GLES_PROFILE_HEADER();
    __gles_VertexAttrib2f(gc, indx, x, y);
    __GLES_PROFILE_FOOTER(GLES3_VERTEXATTRIB2F);

    if (__glesTracerDispatchTable.VertexAttrib2f)
    {
        (*__glesTracerDispatchTable.VertexAttrib2f)(indx, x, y);
    }
}

GLvoid GLAPIENTRY __glesProfile_VertexAttrib2fv(__GLcontext *gc, GLuint indx, const GLfloat* values)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glVertexAttrib2fv %d 0x%08X\n",
                        gc, tid, indx, __GL_PTR2UINT(values));
    }

    __GLES_PROFILE_HEADER();
    __gles_VertexAttrib2fv(gc, indx, values);
    __GLES_PROFILE_FOOTER(GLES3_VERTEXATTRIB2FV);

    if (__glesTracerDispatchTable.VertexAttrib2fv)
    {
        (*__glesTracerDispatchTable.VertexAttrib2fv)(indx, values);
    }
}

GLvoid GLAPIENTRY __glesProfile_VertexAttrib3f(__GLcontext *gc, GLuint indx, GLfloat x, GLfloat y, GLfloat z)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glVertexAttrib3f %d %f %f %f\n",
                        gc, tid, indx, x, y, z);
    }

    __GLES_PROFILE_HEADER();
    __gles_VertexAttrib3f(gc, indx, x, y, z);
    __GLES_PROFILE_FOOTER(GLES3_VERTEXATTRIB3F);

    if (__glesTracerDispatchTable.VertexAttrib3f)
    {
        (*__glesTracerDispatchTable.VertexAttrib3f)(indx, x, y, z);
    }
}

GLvoid GLAPIENTRY __glesProfile_VertexAttrib3fv(__GLcontext *gc, GLuint indx, const GLfloat* values)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glVertexAttrib3fv %d 0x%08X\n",
                        gc, tid, indx, __GL_PTR2UINT(values));
    }

    __GLES_PROFILE_HEADER();
    __gles_VertexAttrib3fv(gc, indx, values);
    __GLES_PROFILE_FOOTER(GLES3_VERTEXATTRIB3FV);

    if (__glesTracerDispatchTable.VertexAttrib3fv)
    {
        (*__glesTracerDispatchTable.VertexAttrib3fv)(indx, values);
    }
}

GLvoid GLAPIENTRY __glesProfile_VertexAttrib4f(__GLcontext *gc, GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glVertexAttrib4f %d %f %f %f %f\n",
                        gc, tid, indx, x, y, z, w);
    }

    __GLES_PROFILE_HEADER();
    __gles_VertexAttrib4f(gc, indx, x, y, z, w);
    __GLES_PROFILE_FOOTER(GLES3_VERTEXATTRIB4F);

    if (__glesTracerDispatchTable.VertexAttrib4f)
    {
        (*__glesTracerDispatchTable.VertexAttrib4f)(indx, x, y, z, w);
    }
}

GLvoid GLAPIENTRY __glesProfile_VertexAttrib4fv(__GLcontext *gc, GLuint indx, const GLfloat* values)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glVertexAttrib4fv %d 0x%08X\n",
                        gc, tid, indx, __GL_PTR2UINT(values));
    }

    __GLES_PROFILE_HEADER();
    __gles_VertexAttrib4fv(gc, indx, values);
    __GLES_PROFILE_FOOTER(GLES3_VERTEXATTRIB4FV);

    if (__glesTracerDispatchTable.VertexAttrib4fv)
    {
        (*__glesTracerDispatchTable.VertexAttrib4fv)(indx, values);
    }
}

GLvoid GLAPIENTRY __glesProfile_VertexAttribPointer(__GLcontext *gc, GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glVertexAttribPointer %d %d 0x%04X %d %d 0x%08X\n",
                        gc, tid, indx, size, type, normalized, stride, __GL_PTR2UINT(ptr));
    }

    __GLES_PROFILE_HEADER();
    __gles_VertexAttribPointer(gc, indx, size, type, normalized, stride, ptr);
    __GLES_PROFILE_FOOTER(GLES3_VERTEXATTRIBPOINTER);

    if (__glesTracerDispatchTable.VertexAttribPointer)
    {
        (*__glesTracerDispatchTable.VertexAttribPointer)(indx, size, type, normalized, stride, ptr);
    }
}

GLvoid GLAPIENTRY __glesProfile_Viewport(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glViewport %d %d %d %d\n",
                        gc, tid, x, y, width, height);
    }

    __GLES_PROFILE_HEADER();
    __gles_Viewport(gc, x, y, width, height);
    __GLES_PROFILE_FOOTER(GLES3_VIEWPORT);

    if (__glesTracerDispatchTable.Viewport)
    {
        (*__glesTracerDispatchTable.Viewport)(x, y, width, height);
    }
}

/* OpenGL ES 3.0 */

GLvoid GLAPIENTRY __glesProfile_ReadBuffer(__GLcontext *gc, GLenum mode)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glReadBuffer 0x%04X\n",
                        gc, tid, mode);
    }

    __GLES_PROFILE_HEADER();
    __gles_ReadBuffer(gc, mode);
    __GLES_PROFILE_FOOTER(GLES3_READBUFFER);

    if (__glesTracerDispatchTable.ReadBuffer)
    {
        (*__glesTracerDispatchTable.ReadBuffer)(mode);
    }
}

GLvoid GLAPIENTRY __glesProfile_DrawRangeElements(__GLcontext *gc, GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glDrawRangeElements 0x%04X %d %d %d %d 0x%08X\n",
                        gc, tid, mode, start, end, count, type, __GL_PTR2UINT(indices));
    }

    __GLES_PROFILE_HEADER();
    __gles_DrawRangeElements(gc, mode, start, end, count, type, indices);
    __GLES_PROFILE_FOOTER(GLES3_DRAWRANGEELEMENTS);

    if (__glesTracerDispatchTable.DrawRangeElements)
    {
        (*__glesTracerDispatchTable.DrawRangeElements)(mode, start, end, count, type, indices);
    }
}

GLvoid GLAPIENTRY __glesProfile_TexImage3D(__GLcontext *gc, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glTexImage3D 0x%04X %d 0x%04X %d %d %d %d 0x%04X 0x%04X 0x%08X\n",
                        gc, tid, target, level, internalformat, width, height, depth, border, format, type, __GL_PTR2UINT(pixels));
    }

    __GLES_PROFILE_HEADER();
    __gles_TexImage3D(gc, target, level, internalformat, width, height, depth, border, format, type, pixels);
    __GLES_PROFILE_FOOTER(GLES3_TEXIMAGE3D);

    if (__glesTracerDispatchTable.TexImage3D)
    {
        (*__glesTracerDispatchTable.TexImage3D)(target, level, internalformat, width, height, depth, border, format, type, pixels);
    }
}

GLvoid GLAPIENTRY __glesProfile_TexSubImage3D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glTexSubImage3D 0x%04X %d %d %d %d %d %d %d 0x%04X 0x%04X 0x%08X\n",
                        gc, tid, target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, __GL_PTR2UINT(pixels));
    }

    __GLES_PROFILE_HEADER();
    __gles_TexSubImage3D(gc, target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
    __GLES_PROFILE_FOOTER(GLES3_TEXSUBIMAGE3D);

    if (__glesTracerDispatchTable.TexSubImage3D)
    {
        (*__glesTracerDispatchTable.TexSubImage3D)(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
    }
}

GLvoid GLAPIENTRY __glesProfile_CopyTexSubImage3D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glCopyTexSubImage3D 0x%04X %d %d %d %d %d %d %d %d\n",
                        gc, tid, target, level, xoffset, yoffset, zoffset, x, y, width, height);
    }

    __GLES_PROFILE_HEADER();
    __gles_CopyTexSubImage3D(gc, target, level, xoffset, yoffset, zoffset, x, y, width, height);
    __GLES_PROFILE_FOOTER(GLES3_COPYTEXSUBIMAGE3D);

    if (__glesTracerDispatchTable.CopyTexSubImage3D)
    {
        (*__glesTracerDispatchTable.CopyTexSubImage3D)(target, level, xoffset, yoffset, zoffset, x, y, width, height);
    }
}

GLvoid GLAPIENTRY __glesProfile_CompressedTexImage3D(__GLcontext *gc, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glCompressedTexImage3D 0x%04X %d 0x%04X %d %d %d %d %d 0x%08X\n",
                        gc, tid, target, level, internalformat, width, height, depth, border, imageSize, __GL_PTR2UINT(data));
    }

    __GLES_PROFILE_HEADER();
    __gles_CompressedTexImage3D(gc, target, level, internalformat, width, height, depth, border, imageSize, data);
    __GLES_PROFILE_FOOTER(GLES3_COMPRESSEDTEXIMAGE3D);

    if (__glesTracerDispatchTable.CompressedTexImage3D)
    {
        (*__glesTracerDispatchTable.CompressedTexImage3D)(target, level, internalformat, width, height, depth, border, imageSize, data);
    }
}

GLvoid GLAPIENTRY __glesProfile_CompressedTexSubImage3D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glCompressedTexSubImage3D 0x%04X %d %d %d %d %d %d %d 0x%04X %d 0x%08X\n",
                        gc, tid, target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, __GL_PTR2UINT(data));
    }

    __GLES_PROFILE_HEADER();
    __gles_CompressedTexSubImage3D(gc, target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
    __GLES_PROFILE_FOOTER(GLES3_COMPRESSEDTEXSUBIMAGE3D);

    if (__glesTracerDispatchTable.CompressedTexSubImage3D)
    {
        (*__glesTracerDispatchTable.CompressedTexSubImage3D)(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
    }
}

GLvoid GLAPIENTRY __glesProfile_GenQueries(__GLcontext *gc, GLsizei n, GLuint* ids)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGenQueries %d\n",
                        gc, tid, n);
    }

    __GLES_PROFILE_HEADER();
    __gles_GenQueries(gc, n, ids);
    __GLES_PROFILE_FOOTER(GLES3_GENQUERIES);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGenQueries => ");
        __glesLogArrayData(gc, n, ids);
    }

    if (__glesTracerDispatchTable.GenQueries)
    {
        (*__glesTracerDispatchTable.GenQueries)(n, ids);
    }
}

GLvoid GLAPIENTRY __glesProfile_DeleteQueries(__GLcontext *gc, GLsizei n, const GLuint* ids)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glDeleteQueries %d ",
                        gc, tid, n);
        __glesLogArrayData(gc, n, ids);
    }

    __GLES_PROFILE_HEADER();
    __gles_DeleteQueries(gc, n, ids);
    __GLES_PROFILE_FOOTER(GLES3_DELETEQUERIES);

    if (__glesTracerDispatchTable.DeleteQueries)
    {
        (*__glesTracerDispatchTable.DeleteQueries)(n, ids);
    }
}

GLboolean GLAPIENTRY __glesProfile_IsQuery(__GLcontext *gc, GLuint id)
{
    __GLES_PROFILE_VARS();
    GLboolean is;

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glIsQuery %d\n",
                        gc, tid, id);
    }

    __GLES_PROFILE_HEADER();
    is = __gles_IsQuery(gc, id);
    __GLES_PROFILE_FOOTER(GLES3_ISQUERY);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glIsQuery => %d\n", is);
    }

    if (__glesTracerDispatchTable.IsQuery)
    {
        (*__glesTracerDispatchTable.IsQuery)(id);
    }

    return is;
}

GLvoid GLAPIENTRY __glesProfile_BeginQuery(__GLcontext *gc, GLenum target, GLuint id)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glBeginQuery 0x%04X %d\n",
                        gc, tid, target, id);
    }

    __GLES_PROFILE_HEADER();
    __gles_BeginQuery(gc, target, id);
    __GLES_PROFILE_FOOTER(GLES3_BEGINQUERY);

    if (__glesTracerDispatchTable.BeginQuery)
    {
        (*__glesTracerDispatchTable.BeginQuery)(target, id);
    }
}

GLvoid GLAPIENTRY __glesProfile_EndQuery(__GLcontext *gc, GLenum target)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glEndQuery 0x%04X\n",
                        gc, tid, target);
    }

    __GLES_PROFILE_HEADER();
    __gles_EndQuery(gc, target);
    __GLES_PROFILE_FOOTER(GLES3_ENDQUERY);

    if (__glesTracerDispatchTable.EndQuery)
    {
        (*__glesTracerDispatchTable.EndQuery)(target);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetQueryiv(__GLcontext *gc, GLenum target, GLenum pname, GLint* params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetQueryiv 0x%04X 0x%04X\n",
                        gc, tid, target, pname);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetQueryiv(gc, target, pname, params);
    __GLES_PROFILE_FOOTER(GLES3_GETQUERYIV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetQueryiv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glesTracerDispatchTable.GetQueryiv)
    {
        (*__glesTracerDispatchTable.GetQueryiv)(target, pname, params);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetQueryObjectuiv(__GLcontext *gc, GLuint id, GLenum pname, GLuint* params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetQueryObjectuiv %d 0x%04X\n",
                        gc, tid, id, pname);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetQueryObjectuiv(gc, id, pname, params);
    __GLES_PROFILE_FOOTER(GLES3_GETQUERYOBJECTUIV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetQueryObjectuiv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glesTracerDispatchTable.GetQueryObjectuiv)
    {
        (*__glesTracerDispatchTable.GetQueryObjectuiv)(id, pname, params);
    }
}

GLboolean GLAPIENTRY __glesProfile_UnmapBuffer(__GLcontext *gc, GLenum target)
{
    __GLES_PROFILE_VARS();
    GLboolean success;

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glUnmapBuffer 0x%04X\n",
                        gc, tid, target);
    }

    __GLES_PROFILE_HEADER();
    success = __gles_UnmapBuffer(gc, target);
    __GLES_PROFILE_FOOTER(GLES3_UNMAPBUFFER);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glUnmapBuffer => %d\n", success);
    }

    if (__glesTracerDispatchTable.UnmapBuffer)
    {
        (*__glesTracerDispatchTable.UnmapBuffer)(target);
    }

    return success;
}

GLvoid GLAPIENTRY __glesProfile_GetBufferPointerv(__GLcontext *gc, GLenum target, GLenum pname, GLvoid** params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetBufferPointerv 0x%04X 0x%04X\n",
                        gc, tid, target, pname);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetBufferPointerv(gc, target, pname, params);
    __GLES_PROFILE_FOOTER(GLES3_GETBUFFERPOINTERV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetBufferPointerv => 0x%08X\n", __GL_PTR2UINT(__GL_PTRVALUE(params)));
    }

    if (__glesTracerDispatchTable.GetBufferPointerv)
    {
        (*__glesTracerDispatchTable.GetBufferPointerv)(target, pname, params);
    }
}

GLvoid GLAPIENTRY __glesProfile_DrawBuffers(__GLcontext *gc, GLsizei n, const GLenum* bufs)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glDrawBuffers %d ",
                        gc, tid, n);
        __glesLogArrayData(gc, n, bufs);
    }

    __GLES_PROFILE_HEADER();
    __gles_DrawBuffers(gc, n, bufs);
    __GLES_PROFILE_FOOTER(GLES3_DRAWBUFFERS);

    if (__glesTracerDispatchTable.DrawBuffers)
    {
        (*__glesTracerDispatchTable.DrawBuffers)(n, bufs);
    }
}

GLvoid GLAPIENTRY __glesProfile_UniformMatrix2x3fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glUniformMatrix2x3fv %d %d %d 0x%08X\n",
                        gc, tid, location, count, transpose, __GL_PTR2UINT(value));
    }

    __GLES_PROFILE_HEADER();
    __gles_UniformMatrix2x3fv(gc, location, count, transpose, value);
    __GLES_PROFILE_FOOTER(GLES3_UNIFORMMATRIX2X3FV);

    if (__glesTracerDispatchTable.UniformMatrix2x3fv)
    {
        (*__glesTracerDispatchTable.UniformMatrix2x3fv)(location, count, transpose, value);
    }
}

GLvoid GLAPIENTRY __glesProfile_UniformMatrix3x2fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glUniformMatrix3x2fv %d %d %d 0x%08X\n",
                        gc, tid, location, count, transpose, __GL_PTR2UINT(value));
    }

    __GLES_PROFILE_HEADER();
    __gles_UniformMatrix3x2fv(gc, location, count, transpose, value);
    __GLES_PROFILE_FOOTER(GLES3_UNIFORMMATRIX3X2FV);

    if (__glesTracerDispatchTable.UniformMatrix3x2fv)
    {
        (*__glesTracerDispatchTable.UniformMatrix3x2fv)(location, count, transpose, value);
    }
}

GLvoid GLAPIENTRY __glesProfile_UniformMatrix2x4fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glUniformMatrix2x4fv %d %d %d 0x%08X\n",
                        gc, tid, location, count, transpose, __GL_PTR2UINT(value));
    }

    __GLES_PROFILE_HEADER();
    __gles_UniformMatrix2x4fv(gc, location, count, transpose, value);
    __GLES_PROFILE_FOOTER(GLES3_UNIFORMMATRIX2X4FV);

    if (__glesTracerDispatchTable.UniformMatrix2x4fv)
    {
        (*__glesTracerDispatchTable.UniformMatrix2x4fv)(location, count, transpose, value);
    }
}

GLvoid GLAPIENTRY __glesProfile_UniformMatrix4x2fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glUniformMatrix4x2fv %d %d %d 0x%08X\n",
                        gc, tid, location, count, transpose, __GL_PTR2UINT(value));
    }

    __GLES_PROFILE_HEADER();
    __gles_UniformMatrix4x2fv(gc, location, count, transpose, value);
    __GLES_PROFILE_FOOTER(GLES3_UNIFORMMATRIX4X2FV);

    if (__glesTracerDispatchTable.UniformMatrix4x2fv)
    {
        (*__glesTracerDispatchTable.UniformMatrix4x2fv)(location, count, transpose, value);
    }
}

GLvoid GLAPIENTRY __glesProfile_UniformMatrix3x4fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glUniformMatrix3x4fv %d %d %d 0x%08X\n",
                        gc, tid, location, count, transpose, __GL_PTR2UINT(value));
    }

    __GLES_PROFILE_HEADER();
    __gles_UniformMatrix3x4fv(gc, location, count, transpose, value);
    __GLES_PROFILE_FOOTER(GLES3_UNIFORMMATRIX3X4FV);

    if (__glesTracerDispatchTable.UniformMatrix3x4fv)
    {
        (*__glesTracerDispatchTable.UniformMatrix3x4fv)(location, count, transpose, value);
    }
}

GLvoid GLAPIENTRY __glesProfile_UniformMatrix4x3fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glUniformMatrix4x3fv %d %d %d 0x%08X\n",
                        gc, tid, location, count, transpose, __GL_PTR2UINT(value));
    }

    __GLES_PROFILE_HEADER();
    __gles_UniformMatrix4x3fv(gc, location, count, transpose, value);
    __GLES_PROFILE_FOOTER(GLES3_UNIFORMMATRIX4X3FV);

    if (__glesTracerDispatchTable.UniformMatrix4x3fv)
    {
        (*__glesTracerDispatchTable.UniformMatrix4x3fv)(location, count, transpose, value);
    }
}

GLvoid GLAPIENTRY __glesProfile_BlitFramebuffer(__GLcontext *gc, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glBlitFramebuffer %d %d %d %d %d %d %d %d 0x%08X 0x%04X\n",
                        gc, tid, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
    }

    __GLES_PROFILE_HEADER();
    __gles_BlitFramebuffer(gc, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
    __GLES_PROFILE_FOOTER(GLES3_BLITFRAMEBUFFER);

    if (__glesTracerDispatchTable.BlitFramebuffer)
    {
        (*__glesTracerDispatchTable.BlitFramebuffer)(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
    }
}

GLvoid GLAPIENTRY __glesProfile_RenderbufferStorageMultisample(__GLcontext *gc, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glRenderbufferStorageMultisample 0x%04X %d 0x%04X %d %d\n",
                        gc, tid, target, samples, internalformat, width, height);
    }

    __GLES_PROFILE_HEADER();
    __gles_RenderbufferStorageMultisample(gc, target, samples, internalformat, width, height);
    __GLES_PROFILE_FOOTER(GLES3_RENDERBUFFERSTORAGEMULTISAMPLE);

    if (__glesTracerDispatchTable.RenderbufferStorageMultisample)
    {
        (*__glesTracerDispatchTable.RenderbufferStorageMultisample)(target, samples, internalformat, width, height);
    }
}

GLvoid GLAPIENTRY __glesProfile_FramebufferTextureLayer(__GLcontext *gc, GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glFramebufferTextureLayer 0x%04X 0x%04X %d %d %d\n",
                        gc, tid, target, attachment, texture, level, layer);
    }

    __GLES_PROFILE_HEADER();
    __gles_FramebufferTextureLayer(gc, target, attachment, texture, level, layer);
    __GLES_PROFILE_FOOTER(GLES3_FRAMEBUFFERTEXTURELAYER);

    if (__glesTracerDispatchTable.FramebufferTextureLayer)
    {
        (*__glesTracerDispatchTable.FramebufferTextureLayer)(target, attachment, texture, level, layer);
    }
}

GLvoid* GLAPIENTRY __glesProfile_MapBufferRange(__GLcontext *gc, GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
    __GLES_PROFILE_VARS();
    GLvoid *buf;

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glMapBufferRange 0x%04X %d %d 0x%08X\n",
                        gc, tid, target, (GLuint)offset, (GLuint)length, access);
    }

    __GLES_PROFILE_HEADER();
    buf = __gles_MapBufferRange(gc, target, offset, length, access);
    __GLES_PROFILE_FOOTER(GLES3_MAPBUFFERRANGE);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glMapBufferRange => 0x%08X\n", __GL_PTR2UINT(buf));
    }

    if (__glesTracerDispatchTable.MapBufferRange)
    {
        (*__glesTracerDispatchTable.MapBufferRange)(target, offset, length, access, buf);
    }

    return buf;
}

GLvoid GLAPIENTRY __glesProfile_FlushMappedBufferRange(__GLcontext *gc, GLenum target, GLintptr offset, GLsizeiptr length)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glFlushMappedBufferRange 0x%04X %d %d\n",
                        gc, tid, target, (GLuint)offset, (GLuint)length);
    }

    __GLES_PROFILE_HEADER();
    __gles_FlushMappedBufferRange(gc, target, offset, length);
    __GLES_PROFILE_FOOTER(GLES3_FLUSHMAPPEDBUFFERRANGE);

    if (__glesTracerDispatchTable.FlushMappedBufferRange)
    {
        (*__glesTracerDispatchTable.FlushMappedBufferRange)(target, offset, length);
    }
}

GLvoid GLAPIENTRY __glesProfile_BindVertexArray(__GLcontext *gc, GLuint array)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glBindVertexArray %d\n",
                        gc, tid, array);
    }

    __GLES_PROFILE_HEADER();
    __gles_BindVertexArray(gc, array);
    __GLES_PROFILE_FOOTER(GLES3_BINDVERTEXARRAY);

    if (__glesTracerDispatchTable.BindVertexArray)
    {
        (*__glesTracerDispatchTable.BindVertexArray)(array);
    }
}

GLvoid GLAPIENTRY __glesProfile_DeleteVertexArrays(__GLcontext *gc, GLsizei n, const GLuint* arrays)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glDeleteVertexArrays %d ",
                        gc, tid, n);
        __glesLogArrayData(gc, n, arrays);
    }

    __GLES_PROFILE_HEADER();
    __gles_DeleteVertexArrays(gc, n, arrays);
    __GLES_PROFILE_FOOTER(GLES3_DELETEVERTEXARRAYS);

    if (__glesTracerDispatchTable.DeleteVertexArrays)
    {
        (*__glesTracerDispatchTable.DeleteVertexArrays)(n , arrays);
    }
}

GLvoid GLAPIENTRY __glesProfile_GenVertexArrays(__GLcontext *gc, GLsizei n, GLuint* arrays)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGenVertexArrays %d\n",
                        gc, tid, n);
    }

    __GLES_PROFILE_HEADER();
    __gles_GenVertexArrays(gc, n, arrays);
    __GLES_PROFILE_FOOTER(GLES3_GENVERTEXARRAYS);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGenVertexArrays => ");
        __glesLogArrayData(gc, n, arrays);
    }

    if (__glesTracerDispatchTable.GenVertexArrays)
    {
        (*__glesTracerDispatchTable.GenVertexArrays)(n , arrays);
    }
}

GLboolean GLAPIENTRY __glesProfile_IsVertexArray(__GLcontext *gc, GLuint array)
{
    __GLES_PROFILE_VARS();
    GLboolean is;

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glIsVertexArray %d\n",
                        gc, tid, array);
    }

    __GLES_PROFILE_HEADER();
    is = __gles_IsVertexArray(gc, array);
    __GLES_PROFILE_FOOTER(GLES3_ISVERTEXARRAY);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glIsVertexArray => %d\n", is);
    }

    if (__glesTracerDispatchTable.IsVertexArray)
    {
        (*__glesTracerDispatchTable.IsVertexArray)(array);
    }

    return is;
}

GLvoid GLAPIENTRY __glesProfile_GetIntegeri_v(__GLcontext *gc, GLenum target, GLuint index, GLint* data)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetIntegeri_v 0x%04X %d\n",
                        gc, tid, target, index);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetIntegeri_v(gc, target, index, data);
    __GLES_PROFILE_FOOTER(GLES3_GETINTEGERI_V);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetIntegeri_v => %d\n", __GL_PTRVALUE(data));
    }

    if (__glesTracerDispatchTable.GetIntegeri_v)
    {
        (*__glesTracerDispatchTable.GetIntegeri_v)(target, index, data);
    }
}

GLvoid GLAPIENTRY __glesProfile_BeginTransformFeedback(__GLcontext *gc, GLenum primitiveMode)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glBeginTransformFeedback 0x%04X\n",
                        gc, tid, primitiveMode);
    }

    __GLES_PROFILE_HEADER();
    __gles_BeginTransformFeedback(gc, primitiveMode);
    __GLES_PROFILE_FOOTER(GLES3_BEGINTRANSFORMFEEDBACK);

    if (__glesTracerDispatchTable.BeginTransformFeedback)
    {
        (*__glesTracerDispatchTable.BeginTransformFeedback)(primitiveMode);
    }
}

GLvoid GLAPIENTRY __glesProfile_EndTransformFeedback(__GLcontext *gc)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glEndTransformFeedback\n", gc, tid);
    }

    __GLES_PROFILE_HEADER();
    __gles_EndTransformFeedback(gc);
    __GLES_PROFILE_FOOTER(GLES3_ENDTRANSFORMFEEDBACK);

    if (__glesTracerDispatchTable.EndTransformFeedback)
    {
        (*__glesTracerDispatchTable.EndTransformFeedback)();
    }
}

GLvoid GLAPIENTRY __glesProfile_BindBufferRange(__GLcontext *gc, GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glBindBufferRange 0x%04X %d %d %d %d\n",
                        gc, tid, target, index, buffer, (GLuint)offset, (GLuint)size);
    }

    __GLES_PROFILE_HEADER();
    __gles_BindBufferRange(gc, target, index, buffer, offset, size);
    __GLES_PROFILE_FOOTER(GLES3_BINDBUFFERRANGE);

    if (__glesTracerDispatchTable.BindBufferRange)
    {
        (*__glesTracerDispatchTable.BindBufferRange)(target, index, buffer, offset, size);
    }
}

GLvoid GLAPIENTRY __glesProfile_BindBufferBase(__GLcontext *gc, GLenum target, GLuint index, GLuint buffer)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glBindBufferBase 0x%04X %d %d\n",
                        gc, tid, target, index, buffer);
    }

    __GLES_PROFILE_HEADER();
    __gles_BindBufferBase(gc, target, index, buffer);
    __GLES_PROFILE_FOOTER(GLES3_BINDBUFFERBASE);

    if (__glesTracerDispatchTable.BindBufferBase)
    {
        (*__glesTracerDispatchTable.BindBufferBase)(target, index, buffer);
    }
}

GLvoid GLAPIENTRY __glesProfile_TransformFeedbackVaryings(__GLcontext *gc, GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glTransformFeedbackVaryings %d %d 0x%08X 0x%08X\n",
                        gc, tid, program, count, __GL_PTR2UINT(varyings), bufferMode);
    }

    __GLES_PROFILE_HEADER();
    __gles_TransformFeedbackVaryings(gc, program, count, varyings, bufferMode);
    __GLES_PROFILE_FOOTER(GLES3_TRANSFORMFEEDBACKVARYINGS);

    if (__glesTracerDispatchTable.TransformFeedbackVaryings)
    {
        (*__glesTracerDispatchTable.TransformFeedbackVaryings)(program, count, varyings, bufferMode);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetTransformFeedbackVarying(__GLcontext *gc, GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetTransformFeedbackVarying %d %d %d\n",
                        gc, tid, program, index, bufSize);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetTransformFeedbackVarying(gc, program, index, bufSize, length, size, type, name);
    __GLES_PROFILE_FOOTER(GLES3_GETTRANSFORMFEEDBACKVARYING);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetTransformFeedbackVarying => %d %d 0x%04X %s\n",
                        __GL_PTRVALUE(length), __GL_PTRVALUE(size), __GL_PTR2UINT(type), name);
    }

    if (__glesTracerDispatchTable.GetTransformFeedbackVarying)
    {
        (*__glesTracerDispatchTable.GetTransformFeedbackVarying)(program, index, bufSize, length, size, type, name);
    }
}

GLvoid GLAPIENTRY __glesProfile_VertexAttribIPointer(__GLcontext *gc, GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glVertexAttribIPointer %d %d 0x%04X %d 0x%08X\n",
                        gc, tid, index, size, type, stride, __GL_PTR2UINT(pointer));
    }

    __GLES_PROFILE_HEADER();
    __gles_VertexAttribIPointer(gc, index, size, type, stride, pointer);
    __GLES_PROFILE_FOOTER(GLES3_VERTEXATTRIBIPOINTER);

    if (__glesTracerDispatchTable.VertexAttribIPointer)
    {
        (*__glesTracerDispatchTable.VertexAttribIPointer)(index, size, type, stride, pointer);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetVertexAttribIiv(__GLcontext *gc, GLuint index, GLenum pname, GLint* params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetVertexAttribIiv %d 0x%04X\n",
                        gc, tid, index, pname);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetVertexAttribIiv(gc, index, pname, params);
    __GLES_PROFILE_FOOTER(GLES3_GETVERTEXATTRIBIIV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetVertexAttribIiv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glesTracerDispatchTable.GetVertexAttribIiv)
    {
        (*__glesTracerDispatchTable.GetVertexAttribIiv)(index, pname, params);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetVertexAttribIuiv(__GLcontext *gc, GLuint index, GLenum pname, GLuint* params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetVertexAttribIuiv %d 0x%04X\n",
                        gc, tid, index, pname);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetVertexAttribIuiv(gc, index, pname, params);
    __GLES_PROFILE_FOOTER(GLES3_GETVERTEXATTRIBIUIV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetVertexAttribIuiv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glesTracerDispatchTable.GetVertexAttribIuiv)
    {
        (*__glesTracerDispatchTable.GetVertexAttribIuiv)(index, pname, params);
    }
}

GLvoid GLAPIENTRY __glesProfile_VertexAttribI4i(__GLcontext *gc, GLuint index, GLint x, GLint y, GLint z, GLint w)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glVertexAttribI4i %d %d %d %d %d\n",
                        gc, tid, index, x, y, z, w);
    }

    __GLES_PROFILE_HEADER();
    __gles_VertexAttribI4i(gc, index, x, y, z, w);
    __GLES_PROFILE_FOOTER(GLES3_VERTEXATTRIBI4I);

    if (__glesTracerDispatchTable.VertexAttribI4i)
    {
        (*__glesTracerDispatchTable.VertexAttribI4i)(index, x, y, z, w);
    }
}

GLvoid GLAPIENTRY __glesProfile_VertexAttribI4ui(__GLcontext *gc, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glVertexAttribI4ui %d %d %d %d %d\n",
                        gc, tid, index, x, y, z, w);
    }

    __GLES_PROFILE_HEADER();
    __gles_VertexAttribI4ui(gc, index, x, y, z, w);
    __GLES_PROFILE_FOOTER(GLES3_VERTEXATTRIBI4UI);

    if (__glesTracerDispatchTable.VertexAttribI4ui)
    {
        (*__glesTracerDispatchTable.VertexAttribI4ui)(index, x, y, z, w);
    }
}

GLvoid GLAPIENTRY __glesProfile_VertexAttribI4iv(__GLcontext *gc, GLuint index, const GLint* v)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glVertexAttribI4iv %d 0x%08X\n",
                        gc, tid, index, __GL_PTR2UINT(v));
    }

    __GLES_PROFILE_HEADER();
    __gles_VertexAttribI4iv(gc, index, v);
    __GLES_PROFILE_FOOTER(GLES3_VERTEXATTRIBI4IV);

    if (__glesTracerDispatchTable.VertexAttribI4iv)
    {
        (*__glesTracerDispatchTable.VertexAttribI4iv)(index, v);
    }
}

GLvoid GLAPIENTRY __glesProfile_VertexAttribI4uiv(__GLcontext *gc, GLuint index, const GLuint* v)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glVertexAttribI4uiv %d 0x%08X\n",
                        gc, tid, index, __GL_PTR2UINT(v));
    }

    __GLES_PROFILE_HEADER();
    __gles_VertexAttribI4uiv(gc, index, v);
    __GLES_PROFILE_FOOTER(GLES3_VERTEXATTRIBI4UIV);

    if (__glesTracerDispatchTable.VertexAttribI4uiv)
    {
        (*__glesTracerDispatchTable.VertexAttribI4uiv)(index, v);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetUniformuiv(__GLcontext *gc, GLuint program, GLint location, GLuint* params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetUniformuiv %d %d\n",
                        gc, tid, program, location);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetUniformuiv(gc, program, location, params);
    __GLES_PROFILE_FOOTER(GLES3_GETUNIFORMUIV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetUniformuiv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glesTracerDispatchTable.GetUniformuiv)
    {
        (*__glesTracerDispatchTable.GetUniformuiv)(program, location, params);
    }
}

GLint GLAPIENTRY __glesProfile_GetFragDataLocation(__GLcontext *gc, GLuint program, const GLchar *name)
{
    __GLES_PROFILE_VARS();
    GLint location;

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetFragDataLocation %d %s\n",
                        gc, tid, program, name);
    }

    __GLES_PROFILE_HEADER();
    location = __gles_GetFragDataLocation(gc, program, name);
    __GLES_PROFILE_FOOTER(GLES3_GETFRAGDATALOCATION);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetFragDataLocation => %d\n", location);
    }

    if (__glesTracerDispatchTable.GetFragDataLocation)
    {
        (*__glesTracerDispatchTable.GetFragDataLocation)(program, name, location);
    }

    return location;
}

GLvoid GLAPIENTRY __glesProfile_Uniform1ui(__GLcontext *gc, GLint location, GLuint v0)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glUniform1ui %d %d\n",
                        gc, tid, location, v0);
    }

    __GLES_PROFILE_HEADER();
    __gles_Uniform1ui(gc, location, v0);
    __GLES_PROFILE_FOOTER(GLES3_UNIFORM1UI);

    if (__glesTracerDispatchTable.Uniform1ui)
    {
        (*__glesTracerDispatchTable.Uniform1ui)(location, v0);
    }
}

GLvoid GLAPIENTRY __glesProfile_Uniform2ui(__GLcontext *gc, GLint location, GLuint v0, GLuint v1)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glUniform2ui %d %d %d\n",
                        gc, tid, location, v0, v1);
    }

    __GLES_PROFILE_HEADER();
    __gles_Uniform2ui(gc, location, v0, v1);
    __GLES_PROFILE_FOOTER(GLES3_UNIFORM2UI);

    if (__glesTracerDispatchTable.Uniform2ui)
    {
        (*__glesTracerDispatchTable.Uniform2ui)(location, v0, v1);
    }
}

GLvoid GLAPIENTRY __glesProfile_Uniform3ui(__GLcontext *gc, GLint location, GLuint v0, GLuint v1, GLuint v2)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glUniform3ui %d %d %d %d\n",
                        gc, tid, location, v0, v1, v2);
    }

    __GLES_PROFILE_HEADER();
    __gles_Uniform3ui(gc, location, v0, v1, v2);
    __GLES_PROFILE_FOOTER(GLES3_UNIFORM3UI);

    if (__glesTracerDispatchTable.Uniform3ui)
    {
        (*__glesTracerDispatchTable.Uniform3ui)(location, v0, v1, v2);
    }
}

GLvoid GLAPIENTRY __glesProfile_Uniform4ui(__GLcontext *gc, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glUniform4ui %d %d %d %d %d\n",
                        gc, tid, location, v0, v1, v2, v3);
    }

    __GLES_PROFILE_HEADER();
    __gles_Uniform4ui(gc, location, v0, v1, v2, v3);
    __GLES_PROFILE_FOOTER(GLES3_UNIFORM4UI);

    if (__glesTracerDispatchTable.Uniform4ui)
    {
        (*__glesTracerDispatchTable.Uniform4ui)(location, v0, v1, v2, v3);
    }
}

GLvoid GLAPIENTRY __glesProfile_Uniform1uiv(__GLcontext *gc, GLint location, GLsizei count, const GLuint* value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glUniform1uiv %d %d 0x%08X\n",
                        gc, tid, location, count, __GL_PTR2UINT(value));
    }

    __GLES_PROFILE_HEADER();
    __gles_Uniform1uiv(gc, location, count, value);
    __GLES_PROFILE_FOOTER(GLES3_UNIFORM1UIV);

    if (__glesTracerDispatchTable.Uniform1uiv)
    {
        (*__glesTracerDispatchTable.Uniform1uiv)(location, count, value);
    }
}

GLvoid GLAPIENTRY __glesProfile_Uniform2uiv(__GLcontext *gc, GLint location, GLsizei count, const GLuint* value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glUniform2uiv %d %d 0x%08X\n",
                        gc, tid, location, count, __GL_PTR2UINT(value));
    }

    __GLES_PROFILE_HEADER();
    __gles_Uniform2uiv(gc, location, count, value);
    __GLES_PROFILE_FOOTER(GLES3_UNIFORM2UIV);

    if (__glesTracerDispatchTable.Uniform2uiv)
    {
        (*__glesTracerDispatchTable.Uniform2uiv)(location, count, value);
    }
}

GLvoid GLAPIENTRY __glesProfile_Uniform3uiv(__GLcontext *gc, GLint location, GLsizei count, const GLuint* value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glUniform3uiv %d %d 0x%08X\n",
                        gc, tid, location, count, __GL_PTR2UINT(value));
    }

    __GLES_PROFILE_HEADER();
    __gles_Uniform3uiv(gc, location, count, value);
    __GLES_PROFILE_FOOTER(GLES3_UNIFORM3UIV);

    if (__glesTracerDispatchTable.Uniform3uiv)
    {
        (*__glesTracerDispatchTable.Uniform3uiv)(location, count, value);
    }
}

GLvoid GLAPIENTRY __glesProfile_Uniform4uiv(__GLcontext *gc, GLint location, GLsizei count, const GLuint* value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glUniform4uiv %d %d 0x%08X\n",
                        gc, tid, location, count, __GL_PTR2UINT(value));
    }

    __GLES_PROFILE_HEADER();
    __gles_Uniform4uiv(gc, location, count, value);
    __GLES_PROFILE_FOOTER(GLES3_UNIFORM4UIV);

    if (__glesTracerDispatchTable.Uniform4uiv)
    {
        (*__glesTracerDispatchTable.Uniform4uiv)(location, count, value);
    }
}

GLvoid GLAPIENTRY __glesProfile_ClearBufferiv(__GLcontext *gc, GLenum buffer, GLint drawbuffer, const GLint* value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glClearBufferiv 0x%04X %d 0x%08X\n",
                        gc, tid, buffer, drawbuffer, __GL_PTR2UINT(value));
    }

    __GLES_PROFILE_HEADER();
    __gles_ClearBufferiv(gc, buffer, drawbuffer, value);
    __GLES_PROFILE_FOOTER(GLES3_CLEARBUFFERIV);

    if (__glesTracerDispatchTable.ClearBufferiv)
    {
        (*__glesTracerDispatchTable.ClearBufferiv)(buffer, drawbuffer, value);
    }
}

GLvoid GLAPIENTRY __glesProfile_ClearBufferuiv(__GLcontext *gc, GLenum buffer, GLint drawbuffer, const GLuint* value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glClearBufferuiv 0x%04X %d 0x%08X\n",
                        gc, tid, buffer, drawbuffer, __GL_PTR2UINT(value));
    }

    __GLES_PROFILE_HEADER();
    __gles_ClearBufferuiv(gc, buffer, drawbuffer, value);
    __GLES_PROFILE_FOOTER(GLES3_CLEARBUFFERUIV);

    if (__glesTracerDispatchTable.ClearBufferuiv)
    {
        (*__glesTracerDispatchTable.ClearBufferuiv)(buffer, drawbuffer, value);
    }
}

GLvoid GLAPIENTRY __glesProfile_ClearBufferfv(__GLcontext *gc, GLenum buffer, GLint drawbuffer, const GLfloat* value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glClearBufferfv 0x%04X %d 0x%08X\n",
                        gc, tid, buffer, drawbuffer, __GL_PTR2UINT(value));
    }

    __GLES_PROFILE_HEADER();
    __gles_ClearBufferfv(gc, buffer, drawbuffer, value);
    __GLES_PROFILE_FOOTER(GLES3_CLEARBUFFERFV);

    if (__glesTracerDispatchTable.ClearBufferfv)
    {
        (*__glesTracerDispatchTable.ClearBufferfv)(buffer, drawbuffer, value);
    }
}

GLvoid GLAPIENTRY __glesProfile_ClearBufferfi(__GLcontext *gc, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glClearBufferfi 0x%04X %d %f %d\n",
                        gc, tid, buffer, drawbuffer, depth, stencil);
    }

    __GLES_PROFILE_HEADER();
    __gles_ClearBufferfi(gc, buffer, drawbuffer, depth, stencil);
    __GLES_PROFILE_FOOTER(GLES3_CLEARBUFFERFI);

    if (__glesTracerDispatchTable.ClearBufferfi)
    {
        (*__glesTracerDispatchTable.ClearBufferfi)(buffer, drawbuffer, depth, stencil);
    }
}

const GLubyte* GLAPIENTRY __glesProfile_GetStringi(__GLcontext *gc, GLenum name, GLuint index)
{
    __GLES_PROFILE_VARS();
    const GLubyte* string;

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetStringi 0x%04X %d\n",
                        gc, tid, name, index);
    }

    __GLES_PROFILE_HEADER();
    string = __gles_GetStringi(gc, name, index);
    __GLES_PROFILE_FOOTER(GLES3_GETSTRINGI);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetStringi => %s\n", string);
    }

    if (__glesTracerDispatchTable.GetStringi)
    {
        (*__glesTracerDispatchTable.GetStringi)(name, index);
    }

    return string;
}

GLvoid GLAPIENTRY __glesProfile_CopyBufferSubData(__GLcontext *gc, GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glCopyBufferSubData 0x%04X 0x%04X %d %d %d\n",
                        gc, tid, readTarget, writeTarget, (GLuint)readOffset, (GLuint)writeOffset, (GLuint)size);
    }

    __GLES_PROFILE_HEADER();
    __gles_CopyBufferSubData(gc, readTarget, writeTarget, readOffset, writeOffset, size);
    __GLES_PROFILE_FOOTER(GLES3_COPYBUFFERSUBDATA);

    if (__glesTracerDispatchTable.CopyBufferSubData)
    {
        (*__glesTracerDispatchTable.CopyBufferSubData)(readTarget, writeTarget, readOffset, writeOffset, size);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetUniformIndices(__GLcontext *gc, GLuint program, GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetUniformIndices %d %d 0x%08X\n",
                        gc, tid, program, uniformCount, __GL_PTR2UINT(uniformNames));
    }

    __GLES_PROFILE_HEADER();
    __gles_GetUniformIndices(gc, program, uniformCount, uniformNames, uniformIndices);
    __GLES_PROFILE_FOOTER(GLES3_GETUNIFORMINDICES);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetUniformIndices => { ");
        if (uniformCount > 0)
        {
            GLsizei i;
            __GLES_LOG_API("uniform[%d] %s", uniformIndices[0], uniformNames[0]);
            for (i = 1; i < uniformCount; ++i)
            {
                __GLES_LOG_API(", uniform[%d] %s", uniformIndices[i], uniformNames[i]);
            }
        }
        __GLES_LOG_API(" }\n");
    }

    if (__glesTracerDispatchTable.GetUniformIndices)
    {
        (*__glesTracerDispatchTable.GetUniformIndices)(program, uniformCount, uniformNames, uniformIndices);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetActiveUniformsiv(__GLcontext *gc, GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetActiveUniformsiv %d %d 0x%08X 0x%04X 0x%08X\n",
                        gc, tid, program, uniformCount, __GL_PTR2UINT(uniformIndices), pname, __GL_PTR2UINT(params));
    }

    __GLES_PROFILE_HEADER();
    __gles_GetActiveUniformsiv(gc, program, uniformCount, uniformIndices, pname, params);
    __GLES_PROFILE_FOOTER(GLES3_GETACTIVEUNIFORMSIV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetActiveUniformsiv => { ");
        if (uniformCount > 0)
        {
            GLsizei i;
            __GLES_LOG_API("uniform[%d] %d", uniformIndices[0], params[0]);
            for (i = 1; i < uniformCount; ++i)
            {
                __GLES_LOG_API(", uniform[%d] %d", uniformIndices[i], params[i]);
            }
        }
        __GLES_LOG_API(" }\n");
    }

    if (__glesTracerDispatchTable.GetActiveUniformsiv)
    {
        (*__glesTracerDispatchTable.GetActiveUniformsiv)(program, uniformCount, uniformIndices, pname, params);
    }
}

GLuint GLAPIENTRY __glesProfile_GetUniformBlockIndex(__GLcontext *gc, GLuint program, const GLchar* uniformBlockName)
{
    __GLES_PROFILE_VARS();
    GLuint index;

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetUniformBlockIndex %d %s\n",
                        gc, tid, program, uniformBlockName);
    }

    __GLES_PROFILE_HEADER();
    index = __gles_GetUniformBlockIndex(gc, program, uniformBlockName);
    __GLES_PROFILE_FOOTER(GLES3_GETUNIFORMBLOCKINDEX);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetUniformBlockIndex => %d\n", index);
    }

    if (__glesTracerDispatchTable.GetUniformBlockIndex)
    {
        (*__glesTracerDispatchTable.GetUniformBlockIndex)(program, uniformBlockName, index);
    }

    return index;
}

GLvoid GLAPIENTRY __glesProfile_GetActiveUniformBlockiv(__GLcontext *gc, GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetActiveUniformBlockiv %d %d 0x%04X\n",
                        gc, tid, program, uniformBlockIndex, pname);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetActiveUniformBlockiv(gc, program, uniformBlockIndex, pname, params);
    __GLES_PROFILE_FOOTER(GLES3_GETACTIVEUNIFORMBLOCKIV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetActiveUniformBlockiv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glesTracerDispatchTable.GetActiveUniformBlockiv)
    {
        (*__glesTracerDispatchTable.GetActiveUniformBlockiv)(program, uniformBlockIndex, pname, params);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetActiveUniformBlockName(__GLcontext *gc, GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetActiveUniformBlockName %d %d %d\n",
                        gc, tid, program, uniformBlockIndex, bufSize);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetActiveUniformBlockName(gc, program, uniformBlockIndex, bufSize, length, uniformBlockName);
    __GLES_PROFILE_FOOTER(GLES3_GETACTIVEUNIFORMBLOCKNAME);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetActiveUniformBlockName => %d %s\n", __GL_PTRVALUE(length), uniformBlockName);
    }

    if (__glesTracerDispatchTable.GetActiveUniformBlockName)
    {
        (*__glesTracerDispatchTable.GetActiveUniformBlockName)(program, uniformBlockIndex, bufSize, length, uniformBlockName);
    }
}

GLvoid GLAPIENTRY __glesProfile_UniformBlockBinding(__GLcontext *gc, GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glUniformBlockBinding %d %d %d\n",
                        gc, tid, program, uniformBlockIndex, uniformBlockBinding);
    }

    __GLES_PROFILE_HEADER();
    __gles_UniformBlockBinding(gc, program, uniformBlockIndex, uniformBlockBinding);
    __GLES_PROFILE_FOOTER(GLES3_UNIFORMBLOCKBINDING);

    if (__glesTracerDispatchTable.UniformBlockBinding)
    {
        (*__glesTracerDispatchTable.UniformBlockBinding)(program, uniformBlockIndex, uniformBlockBinding);
    }
}

GLvoid GLAPIENTRY __glesProfile_DrawArraysInstanced(__GLcontext *gc, GLenum mode, GLint first, GLsizei count, GLsizei instanceCount)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glDrawArraysInstanced 0x%04X %d %d %d\n",
                        gc, tid, mode, first, count, instanceCount);
    }

    __GLES_PROFILE_HEADER();
    __gles_DrawArraysInstanced(gc, mode, first, count, instanceCount);
    __GLES_PROFILE_FOOTER(GLES3_DRAWARRAYSINSTANCED);

    if (__glesTracerDispatchTable.DrawArraysInstanced)
    {
        (*__glesTracerDispatchTable.DrawArraysInstanced)(mode, first, count, instanceCount);
    }
}

GLvoid GLAPIENTRY __glesProfile_DrawElementsInstanced(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei instanceCount)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glDrawElementsInstanced 0x%04X %d 0x%04X 0x%08X %d\n",
                        gc, tid, mode, count, type, __GL_PTR2UINT(indices), instanceCount);
    }

    __GLES_PROFILE_HEADER();
    __gles_DrawElementsInstanced(gc, mode, count, type, indices, instanceCount);
    __GLES_PROFILE_FOOTER(GLES3_DRAWELEMENTSINSTANCED);

    if (__glesTracerDispatchTable.DrawElementsInstanced)
    {
        (*__glesTracerDispatchTable.DrawElementsInstanced)(mode, count, type, indices, instanceCount);
    }
}

GLsync GLAPIENTRY __glesProfile_FenceSync(__GLcontext *gc, GLenum condition, GLbitfield flags)
{
    __GLES_PROFILE_VARS();
    GLsync sync;

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glFenceSync 0x%04X 0x%08X\n",
                        gc, tid, condition, flags);
    }

    __GLES_PROFILE_HEADER();
    sync = __gles_FenceSync(gc, condition, flags);
    __GLES_PROFILE_FOOTER(GLES3_FENCESYNC);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glFenceSync => %d\n", __GL_PTR2UINT(sync));
    }

    if (__glesTracerDispatchTable.FenceSync)
    {
        (*__glesTracerDispatchTable.FenceSync)(condition, flags, sync);
    }

    return sync;
}

GLboolean GLAPIENTRY __glesProfile_IsSync(__GLcontext *gc, GLsync sync)
{
    __GLES_PROFILE_VARS();
    GLboolean is;

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glIsSync 0x%08X\n",
                        gc, tid, __GL_PTR2UINT(sync));
    }

    __GLES_PROFILE_HEADER();
    is = __gles_IsSync(gc, sync);
    __GLES_PROFILE_FOOTER(GLES3_ISSYNC);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glIsSync => %d\n", is);
    }

    if (__glesTracerDispatchTable.IsSync)
    {
        (*__glesTracerDispatchTable.IsSync)(sync);
    }

    return is;
}

GLvoid GLAPIENTRY __glesProfile_DeleteSync(__GLcontext *gc, GLsync sync)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glDeleteSync 0x%08X\n",
                        gc, tid, __GL_PTR2UINT(sync));
    }

    __GLES_PROFILE_HEADER();
    __gles_DeleteSync(gc, sync);
    __GLES_PROFILE_FOOTER(GLES3_DELETESYNC);

    if (__glesTracerDispatchTable.DeleteSync)
    {
        (*__glesTracerDispatchTable.DeleteSync)(sync);
    }
}

GLenum GLAPIENTRY __glesProfile_ClientWaitSync(__GLcontext *gc, GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    __GLES_PROFILE_VARS();
    GLenum status;

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glClientWaitSync 0x%08X 0x%08X 0x%16llX\n",
                        gc, tid, __GL_PTR2UINT(sync), flags, (long long unsigned int)timeout);
    }

    __GLES_PROFILE_HEADER();
    status = __gles_ClientWaitSync(gc, sync, flags, timeout);
    __GLES_PROFILE_FOOTER(GLES3_CLIENTWAITSYNC);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glClientWaitSync => 0x%04X\n", status);
    }

    if (__glesTracerDispatchTable.ClientWaitSync)
    {
        (*__glesTracerDispatchTable.ClientWaitSync)(sync, flags, timeout);
    }

    return status;
}

GLvoid GLAPIENTRY __glesProfile_WaitSync(__GLcontext *gc, GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glWaitSync 0x%08X 0x%08X 0x%16llX\n",
                        gc, tid, __GL_PTR2UINT(sync), flags, (long long unsigned int)timeout);
    }

    __GLES_PROFILE_HEADER();
    __gles_WaitSync(gc, sync, flags, timeout);
    __GLES_PROFILE_FOOTER(GLES3_WAITSYNC);

    if (__glesTracerDispatchTable.WaitSync)
    {
        (*__glesTracerDispatchTable.WaitSync)(sync, flags, timeout);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetInteger64v(__GLcontext *gc, GLenum pname, GLint64* params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetInteger64v 0x%04X\n",
                        gc, tid, pname);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetInteger64v(gc, pname, params);
    __GLES_PROFILE_FOOTER(GLES3_GETINTEGER64V);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetInteger64v => 0x%16llX\n", (long long unsigned int)__GL_PTRVALUE(params));
    }

    if (__glesTracerDispatchTable.GetInteger64v)
    {
        (*__glesTracerDispatchTable.GetInteger64v)(pname, params);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetSynciv(__GLcontext *gc, GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetSynciv 0x%08X 0x%04X %d\n",
                        gc, tid, __GL_PTR2UINT(sync), pname, bufSize);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetSynciv(gc, sync, pname, bufSize, length, values);
    __GLES_PROFILE_FOOTER(GLES3_GETSYNCIV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetSynciv => %d %d\n", __GL_PTRVALUE(length), __GL_PTRVALUE(values));
    }

    if (__glesTracerDispatchTable.GetSynciv)
    {
        (*__glesTracerDispatchTable.GetSynciv)(sync, pname, bufSize, length, values);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetInteger64i_v(__GLcontext *gc, GLenum target, GLuint index, GLint64* data)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetInteger64i_v 0x%04X %d\n",
                        gc, tid, target, index);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetInteger64i_v(gc, target, index, data);
    __GLES_PROFILE_FOOTER(GLES3_GETINTEGER64I_V);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetInteger64i_v => 0x%16llX\n", (long long unsigned int)__GL_PTRVALUE(data));
    }

    if (__glesTracerDispatchTable.GetInteger64i_v)
    {
        (*__glesTracerDispatchTable.GetInteger64i_v)(target, index, data);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetBufferParameteri64v(__GLcontext *gc, GLenum target, GLenum pname, GLint64* params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetBufferParameteri64v 0x%04X 0x%04X\n",
                        gc, tid, target, pname);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetBufferParameteri64v(gc, target, pname, params);
    __GLES_PROFILE_FOOTER(GLES3_GETBUFFERPARAMETERI64V);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetBufferParameteri64v => 0x%16llX\n", (long long unsigned int)__GL_PTRVALUE(params));
    }

    if (__glesTracerDispatchTable.GetBufferParameteri64v)
    {
        (*__glesTracerDispatchTable.GetBufferParameteri64v)(target, pname, params);
    }
}

GLvoid GLAPIENTRY __glesProfile_GenSamplers(__GLcontext *gc, GLsizei count, GLuint* samplers)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGenSamplers %d\n",
                        gc, tid, count);
    }

    __GLES_PROFILE_HEADER();
    __gles_GenSamplers(gc, count, samplers);
    __GLES_PROFILE_FOOTER(GLES3_GENSAMPLERS);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGenSamplers => ");
        __glesLogArrayData(gc, count, samplers);
    }

    if (__glesTracerDispatchTable.GenSamplers)
    {
        (*__glesTracerDispatchTable.GenSamplers)(count, samplers);
    }
}

GLvoid GLAPIENTRY __glesProfile_DeleteSamplers(__GLcontext *gc, GLsizei count, const GLuint* samplers)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glDeleteSamplers %d ",
                        gc, tid, count);
        __glesLogArrayData(gc, count, samplers);
    }

    __GLES_PROFILE_HEADER();
    __gles_DeleteSamplers(gc, count, samplers);
    __GLES_PROFILE_FOOTER(GLES3_DELETESAMPLERS);

    if (__glesTracerDispatchTable.DeleteSamplers)
    {
        (*__glesTracerDispatchTable.DeleteSamplers)(count, samplers);
    }
}

GLboolean GLAPIENTRY __glesProfile_IsSampler(__GLcontext *gc, GLuint sampler)
{
    __GLES_PROFILE_VARS();
    GLboolean is;

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glIsSampler %d\n",
                        gc, tid, sampler);
    }

    __GLES_PROFILE_HEADER();
    is = __gles_IsSampler(gc, sampler);
    __GLES_PROFILE_FOOTER(GLES3_ISSAMPLER);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glIsSampler => %d\n", is);
    }

    if (__glesTracerDispatchTable.IsSampler)
    {
        (*__glesTracerDispatchTable.IsSampler)(sampler);
    }

    return is;
}

GLvoid GLAPIENTRY __glesProfile_BindSampler(__GLcontext *gc, GLuint unit, GLuint sampler)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glBindSampler %d %d\n",
                        gc, tid, unit, sampler);
    }

    __GLES_PROFILE_HEADER();
    __gles_BindSampler(gc, unit, sampler);
    __GLES_PROFILE_FOOTER(GLES3_BINDSAMPLER);

    if (__glesTracerDispatchTable.BindSampler)
    {
        (*__glesTracerDispatchTable.BindSampler)(unit, sampler);
    }
}

GLvoid GLAPIENTRY __glesProfile_SamplerParameteri(__GLcontext *gc, GLuint sampler, GLenum pname, GLint param)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glSamplerParameteri %d 0x%04X %d\n",
                        gc, tid, sampler, pname, param);
    }

    __GLES_PROFILE_HEADER();
    __gles_SamplerParameteri(gc, sampler, pname, param);
    __GLES_PROFILE_FOOTER(GLES3_SAMPLERPARAMETERI);

    if (__glesTracerDispatchTable.SamplerParameteri)
    {
        (*__glesTracerDispatchTable.SamplerParameteri)(sampler, pname, param);
    }
}

GLvoid GLAPIENTRY __glesProfile_SamplerParameteriv(__GLcontext *gc, GLuint sampler, GLenum pname, const GLint* param)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glSamplerParameteriv %d 0x%04X 0x%08X\n",
                        gc, tid, sampler, pname, __GL_PTR2UINT(param));
    }

    __GLES_PROFILE_HEADER();
    __gles_SamplerParameteriv(gc, sampler, pname, param);
    __GLES_PROFILE_FOOTER(GLES3_SAMPLERPARAMETERIV);

    if (__glesTracerDispatchTable.SamplerParameteriv)
    {
        (*__glesTracerDispatchTable.SamplerParameteriv)(sampler, pname, param);
    }
}

GLvoid GLAPIENTRY __glesProfile_SamplerParameterf(__GLcontext *gc, GLuint sampler, GLenum pname, GLfloat param)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glSamplerParameterf %d 0x%04X %f\n",
                        gc, tid, sampler, pname, param);
    }

    __GLES_PROFILE_HEADER();
    __gles_SamplerParameterf(gc, sampler, pname, param);
    __GLES_PROFILE_FOOTER(GLES3_SAMPLERPARAMETERF);

    if (__glesTracerDispatchTable.SamplerParameterf)
    {
        (*__glesTracerDispatchTable.SamplerParameterf)(sampler, pname, param);
    }
}

GLvoid GLAPIENTRY __glesProfile_SamplerParameterfv(__GLcontext *gc, GLuint sampler, GLenum pname, const GLfloat* param)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glSamplerParameterfv %d 0x%04X 0x%08X\n",
                        gc, tid, sampler, pname, __GL_PTR2UINT(param));
    }

    __GLES_PROFILE_HEADER();
    __gles_SamplerParameterfv(gc, sampler, pname, param);
    __GLES_PROFILE_FOOTER(GLES3_SAMPLERPARAMETERFV);

    if (__glesTracerDispatchTable.SamplerParameterfv)
    {
        (*__glesTracerDispatchTable.SamplerParameterfv)(sampler, pname, param);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetSamplerParameteriv(__GLcontext *gc, GLuint sampler, GLenum pname, GLint* params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetSamplerParameteriv %d 0x%04X 0x%08X\n",
                        gc, tid, sampler, pname, __GL_PTR2UINT(params));
    }

    __GLES_PROFILE_HEADER();
    __gles_GetSamplerParameteriv(gc, sampler, pname, params);
    __GLES_PROFILE_FOOTER(GLES3_GETSAMPLERPARAMETERIV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetSamplerParameteriv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glesTracerDispatchTable.GetSamplerParameteriv)
    {
        (*__glesTracerDispatchTable.GetSamplerParameteriv)(sampler, pname, params);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetSamplerParameterfv(__GLcontext *gc, GLuint sampler, GLenum pname, GLfloat* params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetSamplerParameterfv %d 0x%04X\n",
                        gc, tid, sampler, pname);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetSamplerParameterfv(gc, sampler, pname, params);
    __GLES_PROFILE_FOOTER(GLES3_GETSAMPLERPARAMETERFV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetSamplerParameterfv => %f\n", __GL_PTRVALUE(params));
    }

    if (__glesTracerDispatchTable.GetSamplerParameterfv)
    {
        (*__glesTracerDispatchTable.GetSamplerParameterfv)(sampler, pname, params);
    }
}

GLvoid GLAPIENTRY __glesProfile_VertexAttribDivisor(__GLcontext *gc, GLuint index, GLuint divisor)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glVertexAttribDivisor %d %d\n",
                        gc, tid, index, divisor);
    }

    __GLES_PROFILE_HEADER();
    __gles_VertexAttribDivisor(gc, index, divisor);
    __GLES_PROFILE_FOOTER(GLES3_VERTEXATTRIBDIVISOR);

    if (__glesTracerDispatchTable.VertexAttribDivisor)
    {
        (*__glesTracerDispatchTable.VertexAttribDivisor)(index, divisor);
    }
}

GLvoid GLAPIENTRY __glesProfile_BindTransformFeedback(__GLcontext *gc, GLenum target, GLuint id)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glBindTransformFeedback 0x%04X %d\n",
                        gc, tid, target, id);
    }

    __GLES_PROFILE_HEADER();
    __gles_BindTransformFeedback(gc, target, id);
    __GLES_PROFILE_FOOTER(GLES3_BINDTRANSFORMFEEDBACK);

    if (__glesTracerDispatchTable.BindTransformFeedback)
    {
        (*__glesTracerDispatchTable.BindTransformFeedback)(target, id);
    }
}

GLvoid GLAPIENTRY __glesProfile_DeleteTransformFeedbacks(__GLcontext *gc, GLsizei n, const GLuint* ids)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glDeleteTransformFeedbacks %d ",
                        gc, tid, n);
        __glesLogArrayData(gc, n, ids);
    }

    __GLES_PROFILE_HEADER();
    __gles_DeleteTransformFeedbacks(gc, n, ids);
    __GLES_PROFILE_FOOTER(GLES3_DELETETRANSFORMFEEDBACKS);

    if (__glesTracerDispatchTable.DeleteTransformFeedbacks)
    {
        (*__glesTracerDispatchTable.DeleteTransformFeedbacks)(n, ids);
    }
}

GLvoid GLAPIENTRY __glesProfile_GenTransformFeedbacks(__GLcontext *gc, GLsizei n, GLuint* ids)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGenTransformFeedbacks %d\n",
                        gc, tid, n);
    }

    __GLES_PROFILE_HEADER();
    __gles_GenTransformFeedbacks(gc, n, ids);
    __GLES_PROFILE_FOOTER(GLES3_GENTRANSFORMFEEDBACKS);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGenTransformFeedbacks => ");
        __glesLogArrayData(gc, n, ids);
    }

    if (__glesTracerDispatchTable.GenTransformFeedbacks)
    {
        (*__glesTracerDispatchTable.GenTransformFeedbacks)(n, ids);
    }
}

GLboolean GLAPIENTRY __glesProfile_IsTransformFeedback(__GLcontext *gc, GLuint id)
{
    __GLES_PROFILE_VARS();
    GLboolean is;

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glIsTransformFeedback %d\n",
                        gc, tid, id);
    }

    __GLES_PROFILE_HEADER();
    is = __gles_IsTransformFeedback(gc, id);
    __GLES_PROFILE_FOOTER(GLES3_ISTRANSFORMFEEDBACK);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glIsTransformFeedback => %d\n", is);
    }

    if (__glesTracerDispatchTable.IsTransformFeedback)
    {
        (*__glesTracerDispatchTable.IsTransformFeedback)(id);
    }

    return is;
}

GLvoid GLAPIENTRY __glesProfile_PauseTransformFeedback(__GLcontext *gc)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glPauseTransformFeedback\n", gc, tid);
    }

    __GLES_PROFILE_HEADER();
    __gles_PauseTransformFeedback(gc);
    __GLES_PROFILE_FOOTER(GLES3_PAUSETRANSFORMFEEDBACK);

    if (__glesTracerDispatchTable.PauseTransformFeedback)
    {
        (*__glesTracerDispatchTable.PauseTransformFeedback)();
    }
}

GLvoid GLAPIENTRY __glesProfile_ResumeTransformFeedback(__GLcontext *gc)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glResumeTransformFeedback\n", gc, tid);
    }

    __GLES_PROFILE_HEADER();
    __gles_ResumeTransformFeedback(gc);
    __GLES_PROFILE_FOOTER(GLES3_RESUMETRANSFORMFEEDBACK);

    if (__glesTracerDispatchTable.ResumeTransformFeedback)
    {
        (*__glesTracerDispatchTable.ResumeTransformFeedback)();
    }
}

GLvoid GLAPIENTRY __glesProfile_GetProgramBinary(__GLcontext *gc, GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, GLvoid* binary)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetProgramBinary %d %d\n",
                        gc, tid, program, bufSize);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetProgramBinary(gc, program, bufSize, length, binaryFormat, binary);
    __GLES_PROFILE_FOOTER(GLES3_GETPROGRAMBINARY);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetProgramBinary => %d 0x%04X 0x%08X\n",
                       __GL_PTRVALUE(length), __GL_PTRVALUE(binaryFormat), __GL_PTR2UINT(binary));
    }

    if (__glesTracerDispatchTable.GetProgramBinary)
    {
        (*__glesTracerDispatchTable.GetProgramBinary)(program, bufSize, length, binaryFormat, binary);
    }
}

GLvoid GLAPIENTRY __glesProfile_ProgramBinary(__GLcontext *gc, GLuint program, GLenum binaryFormat, const GLvoid* binary, GLsizei length)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glProgramBinary %d 0x%04X 0x%08X %d\n",
                        gc, tid, program, binaryFormat, __GL_PTR2UINT(binary), length);
    }

    __GLES_PROFILE_HEADER();
    __gles_ProgramBinary(gc, program, binaryFormat, binary, length);
    __GLES_PROFILE_FOOTER(GLES3_PROGRAMBINARY);

    if (__glesTracerDispatchTable.ProgramBinary)
    {
        (*__glesTracerDispatchTable.ProgramBinary)(program, binaryFormat, binary, length);
    }
}

GLvoid GLAPIENTRY __glesProfile_ProgramParameteri(__GLcontext *gc, GLuint program, GLenum pname, GLint value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glProgramParameteri %d 0x%04X %d\n",
                        gc, tid, program, pname, value);
    }

    __GLES_PROFILE_HEADER();
    __gles_ProgramParameteri(gc, program, pname, value);
    __GLES_PROFILE_FOOTER(GLES3_PROGRAMPARAMETERI);

    if (__glesTracerDispatchTable.ProgramParameteri)
    {
        (*__glesTracerDispatchTable.ProgramParameteri)(program, pname, value);
    }
}

GLvoid GLAPIENTRY __glesProfile_InvalidateFramebuffer(__GLcontext *gc, GLenum target, GLsizei numAttachments, const GLenum* attachments)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glInvalidateFramebuffer 0x%04X %d 0x%08X\n",
                        gc, tid, target, numAttachments, __GL_PTR2UINT(attachments));
    }

    __GLES_PROFILE_HEADER();
    __gles_InvalidateFramebuffer(gc, target, numAttachments, attachments);
    __GLES_PROFILE_FOOTER(GLES3_INVALIDATEFRAMEBUFFER);

    if (__glesTracerDispatchTable.InvalidateFramebuffer)
    {
        (*__glesTracerDispatchTable.InvalidateFramebuffer)(target, numAttachments, attachments);
    }
}

GLvoid GLAPIENTRY __glesProfile_InvalidateSubFramebuffer(__GLcontext *gc, GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glInvalidateSubFramebuffer 0x%04X %d 0x%08X %d %d %d %d\n",
                        gc, tid, target, numAttachments, __GL_PTR2UINT(attachments), x, y, width, height);
    }

    __GLES_PROFILE_HEADER();
    __gles_InvalidateSubFramebuffer(gc, target, numAttachments, attachments, x, y, width, height);
    __GLES_PROFILE_FOOTER(GLES3_INVALIDATESUBFRAMEBUFFER);

    if (__glesTracerDispatchTable.InvalidateSubFramebuffer)
    {
        (*__glesTracerDispatchTable.InvalidateSubFramebuffer)(target, numAttachments, attachments, x, y, width, height);
    }
}

GLvoid GLAPIENTRY __glesProfile_TexStorage2D(__GLcontext *gc, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glTexStorage2D 0x%04X %d 0x%04X %d %d\n",
                        gc, tid, target, levels, internalformat, width, height);
    }

    __GLES_PROFILE_HEADER();
    __gles_TexStorage2D(gc, target, levels, internalformat, width, height);
    __GLES_PROFILE_FOOTER(GLES3_TEXSTORAGE2D);

    if (__glesTracerDispatchTable.TexStorage2D)
    {
        (*__glesTracerDispatchTable.TexStorage2D)(target, levels, internalformat, width, height);
    }
}

GLvoid GLAPIENTRY __glesProfile_TexStorage3D(__GLcontext *gc, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glTexStorage3D 0x%04X %d 0x%04X %d %d %d\n",
                        gc, tid, target, levels, internalformat, width, height, depth);
    }

    __GLES_PROFILE_HEADER();
    __gles_TexStorage3D(gc, target, levels, internalformat, width, height, depth);
    __GLES_PROFILE_FOOTER(GLES3_TEXSTORAGE3D);

    if (__glesTracerDispatchTable.TexStorage3D)
    {
        (*__glesTracerDispatchTable.TexStorage3D)(target, levels, internalformat, width, height, depth);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetInternalformativ(__GLcontext *gc, GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetInternalformativ 0x%04X 0x%04X 0x%04X %d\n",
                        gc, tid, target, internalformat, pname, bufSize);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetInternalformativ(gc, target, internalformat, pname, bufSize, params);
    __GLES_PROFILE_FOOTER(GLES3_GETINTERNALFORMATIV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetInternalformativ => %d\n", __GL_PTRVALUE(params));
    }

    if (__glesTracerDispatchTable.GetInternalformativ)
    {
        (*__glesTracerDispatchTable.GetInternalformativ)(target, internalformat, pname, bufSize, params);
    }
}

/*
** OpenGL ES 3.1
*/
GLvoid GLAPIENTRY __glesProfile_DispatchCompute(__GLcontext *gc, GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glDispatchCompute %d %d %d\n",
                        gc, tid, num_groups_x, num_groups_y, num_groups_z);
    }

    __GLES_PROFILE_HEADER();
    __gles_DispatchCompute(gc, num_groups_x, num_groups_y, num_groups_z);
    __GLES_PROFILE_FOOTER(GLES31_DISPATCHCOMPUTE);

    if (__glesTracerDispatchTable.DispatchCompute)
    {
        (*__glesTracerDispatchTable.DispatchCompute)(num_groups_x, num_groups_y, num_groups_z);
    }
}

GLvoid GLAPIENTRY __glesProfile_DispatchComputeIndirect(__GLcontext *gc, GLintptr indirect)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glDispatchComputeIndirect 0x%08X\n",
                        gc, tid, indirect);
    }

    __GLES_PROFILE_HEADER();
    __gles_DispatchComputeIndirect(gc, indirect);
    __GLES_PROFILE_FOOTER(GLES31_DISPATCHCOMPUTEINDIRECT);

    if (__glesTracerDispatchTable.DispatchComputeIndirect)
    {
        (*__glesTracerDispatchTable.DispatchComputeIndirect)(indirect);
    }
}

GLvoid GLAPIENTRY __glesProfile_DrawArraysIndirect(__GLcontext *gc, GLenum mode, const void *indirect)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glDrawArraysIndirect 0x%04X 0x%08X\n",
                        gc, tid, mode, indirect);
    }

    __GLES_PROFILE_HEADER();
    __gles_DrawArraysIndirect(gc, mode, indirect);
    __GLES_PROFILE_FOOTER(GLES31_DRAWARRAYSINDIRECT);

    if (__glesTracerDispatchTable.DrawArraysIndirect)
    {
        (*__glesTracerDispatchTable.DrawArraysIndirect)(mode, indirect);
    }
}

GLvoid GLAPIENTRY __glesProfile_DrawElementsIndirect(__GLcontext *gc, GLenum mode, GLenum type, const void *indirect)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glDrawElementsIndirect 0x%04X 0x%04X 0x%08X\n",
                        gc, tid, mode, type, indirect);
    }

    __GLES_PROFILE_HEADER();
    __gles_DrawElementsIndirect(gc, mode, type, indirect);
    __GLES_PROFILE_FOOTER(GLES31_DRAWELEMENTSINDIRECT);

    if (__glesTracerDispatchTable.DrawElementsIndirect)
    {
        (*__glesTracerDispatchTable.DrawElementsIndirect)(mode, type, indirect);
    }
}

GLvoid GLAPIENTRY __glesProfile_FramebufferParameteri(__GLcontext *gc, GLenum target, GLenum pname, GLint param)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glFramebufferParameteri 0x%04X 0x%04X %d\n",
                        gc, tid, target, pname, param);
    }

    __GLES_PROFILE_HEADER();
    __gles_FramebufferParameteri(gc, target, pname, param);
    __GLES_PROFILE_FOOTER(GLES31_FRAMEBUFFERPARAMETERI);

    if (__glesTracerDispatchTable.FramebufferParameteri)
    {
        (*__glesTracerDispatchTable.FramebufferParameteri)(target, pname, param);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetFramebufferParameteriv(__GLcontext *gc, GLenum target, GLenum pname, GLint *params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetFramebufferParameteriv 0x%04X 0x%04X 0x%08X\n",
                        gc, tid, target, pname, params);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetFramebufferParameteriv(gc, target, pname, params);
    __GLES_PROFILE_FOOTER(GLES31_GETFRAMEBUFFERPARAMETERIV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetFramebufferParameteriv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glesTracerDispatchTable.GetFramebufferParameteriv)
    {
        (*__glesTracerDispatchTable.GetFramebufferParameteriv)(target, pname, params);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetProgramInterfaceiv(__GLcontext *gc, GLuint program, GLenum programInterface, GLenum pname, GLint *params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetProgramInterfaceiv %d 0x%04X 0x%04X 0x%08X\n",
                        gc, tid, program, programInterface, pname, params);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetProgramInterfaceiv(gc, program, programInterface, pname, params);
    __GLES_PROFILE_FOOTER(GLES31_GETPROGRAMINTERFACEIV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetProgramInterfaceiv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glesTracerDispatchTable.GetProgramInterfaceiv)
    {
        (*__glesTracerDispatchTable.GetProgramInterfaceiv)(program, programInterface, pname, params);
    }
}

GLuint GLAPIENTRY __glesProfile_GetProgramResourceIndex(__GLcontext *gc, GLuint program, GLenum programInterface, const GLchar *name)
{
    __GLES_PROFILE_VARS();
    GLuint index;

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetProgramResourceIndex %d 0x%04X %s\n",
                        gc, tid, program, programInterface, name);
    }

    __GLES_PROFILE_HEADER();
    index = __gles_GetProgramResourceIndex(gc, program, programInterface, name);
    __GLES_PROFILE_FOOTER(GLES31_GETPROGRAMRESOURCEINDEX);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetProgramResourceIndex => %d\n", index);
    }

    if (__glesTracerDispatchTable.GetProgramResourceIndex)
    {
        (*__glesTracerDispatchTable.GetProgramResourceIndex)(program, programInterface, name);
    }

    return index;
}

GLvoid GLAPIENTRY __glesProfile_GetProgramResourceName(__GLcontext *gc, GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetProgramResourceName %d 0x%04X %d %d\n",
                        gc, tid, program, programInterface, index, bufSize);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetProgramResourceName(gc, program, programInterface, index, bufSize, length, name);
    __GLES_PROFILE_FOOTER(GLES31_GETPROGRAMRESOURCENAME);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetProgramResourceName => %d %s\n", __GL_PTRVALUE(length), name);
    }

    if (__glesTracerDispatchTable.GetProgramResourceName)
    {
        (*__glesTracerDispatchTable.GetProgramResourceName)(program, programInterface, index, bufSize, length, name);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetProgramResourceiv(__GLcontext *gc, GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetProgramResourceiv %d 0x%04X %d %d 0x%08X %d\n",
                        gc, tid, program, programInterface, index, propCount, props, bufSize);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetProgramResourceiv(gc, program, programInterface, index, propCount, props, bufSize, length, params);
    __GLES_PROFILE_FOOTER(GLES31_GETPROGRAMRESOURCEIV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetProgramResourceiv => %d %d\n", __GL_PTRVALUE(length), __GL_PTRVALUE(params));
    }

    if (__glesTracerDispatchTable.GetProgramResourceiv)
    {
        (*__glesTracerDispatchTable.GetProgramResourceiv)(program, programInterface, index, propCount, props, bufSize, length, params);
    }
}

GLint GLAPIENTRY __glesProfile_GetProgramResourceLocation(__GLcontext *gc, GLuint program, GLenum programInterface, const GLchar *name)
{
    __GLES_PROFILE_VARS();
    GLint location;

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetProgramResourceLocation %d 0x%04X %s\n",
                        gc, tid, program, programInterface, name);
    }

    __GLES_PROFILE_HEADER();
    location = __gles_GetProgramResourceLocation(gc, program, programInterface, name);
    __GLES_PROFILE_FOOTER(GLES31_GETPROGRAMRESOURCELOCATION);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetProgramResourceLocation => %d\n", location);
    }

    if (__glesTracerDispatchTable.GetProgramResourceLocation)
    {
        (*__glesTracerDispatchTable.GetProgramResourceLocation)(program, programInterface, name);
    }
    return location;
}

GLvoid GLAPIENTRY __glesProfile_UseProgramStages(__GLcontext *gc, GLuint pipeline, GLbitfield stages, GLuint program)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glUseProgramStages %d 0x%08X %d\n",
                        gc, tid, pipeline, stages, program);
    }

    __GLES_PROFILE_HEADER();
    __gles_UseProgramStages(gc, pipeline, stages, program);
    __GLES_PROFILE_FOOTER(GLES31_USEPROGRAMSTAGES);

    if (__glesTracerDispatchTable.UseProgramStages)
    {
        (*__glesTracerDispatchTable.UseProgramStages)(pipeline, stages, program);
    }
}

GLvoid GLAPIENTRY __glesProfile_ActiveShaderProgram(__GLcontext *gc, GLuint pipeline, GLuint program)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glActiveShaderProgram %d %d\n",
                        gc, tid, pipeline, program);
    }

    __GLES_PROFILE_HEADER();
    __gles_ActiveShaderProgram(gc, pipeline, program);
    __GLES_PROFILE_FOOTER(GLES31_ACTIVESHADERPROGRAM);

    if (__glesTracerDispatchTable.ActiveShaderProgram)
    {
        (*__glesTracerDispatchTable.ActiveShaderProgram)(pipeline, program);
    }
}

GLuint GLAPIENTRY __glesProfile_CreateShaderProgramv(__GLcontext *gc, GLenum type, GLsizei count, const GLchar *const*strings)
{
    __GLES_PROFILE_VARS();
    GLint shader;

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glCreateShaderProgramv 0x%04X %d 0x%08X\n",
                        gc, tid, type, count, __GL_PTR2UINT(strings));

        __glesLogSourceStrings(gc, count, strings);
    }

    __GLES_PROFILE_HEADER();
    shader = __gles_CreateShaderProgramv(gc, type, count, strings);
    __GLES_PROFILE_FOOTER(GLES31_CREATESHADERPROGRAMV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glCreateShaderProgramv => %d\n", shader);
    }

    if (__glesTracerDispatchTable.CreateShaderProgramv)
    {
        (*__glesTracerDispatchTable.CreateShaderProgramv)(type, count, strings);
    }

    return shader;
}

GLvoid GLAPIENTRY __glesProfile_BindProgramPipeline(__GLcontext *gc, GLuint pipeline)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glBindProgramPipeline %d\n",
                        gc, tid, pipeline);
    }

    __GLES_PROFILE_HEADER();
    __gles_BindProgramPipeline(gc, pipeline);
    __GLES_PROFILE_FOOTER(GLES31_BINDPROGRAMPIPELINE);

    if (__glesTracerDispatchTable.BindProgramPipeline)
    {
        (*__glesTracerDispatchTable.BindProgramPipeline)(pipeline);
    }
}

GLvoid GLAPIENTRY __glesProfile_DeleteProgramPipelines(__GLcontext *gc, GLsizei n, const GLuint *pipelines)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glDeleteProgramPipelines %d ",
                        gc, tid, n);
        __glesLogArrayData(gc, n, pipelines);
    }

    __GLES_PROFILE_HEADER();
    __gles_DeleteProgramPipelines(gc, n, pipelines);
    __GLES_PROFILE_FOOTER(GLES31_DELETEPROGRAMPIPELINES);

    if (__glesTracerDispatchTable.DeleteProgramPipelines)
    {
        (*__glesTracerDispatchTable.DeleteProgramPipelines)(n, pipelines);
    }
}

GLvoid GLAPIENTRY __glesProfile_GenProgramPipelines(__GLcontext *gc, GLsizei n, GLuint *pipelines)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGenProgramPipelines %d\n",
                        gc, tid, n);
    }

    __GLES_PROFILE_HEADER();
    __gles_GenProgramPipelines(gc, n, pipelines);
    __GLES_PROFILE_FOOTER(GLES31_GENPROGRAMPIPELINES);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGenProgramPipelines => ");
        __glesLogArrayData(gc, n, pipelines);
    }

    if (__glesTracerDispatchTable.GenProgramPipelines)
    {
        (*__glesTracerDispatchTable.GenProgramPipelines)(n, pipelines);
    }
}

GLboolean GLAPIENTRY __glesProfile_IsProgramPipeline(__GLcontext *gc, GLuint pipeline)
{
    __GLES_PROFILE_VARS();
    GLboolean result;

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glIsProgramPipeline %d\n",
                        gc, tid, pipeline);
    }

    __GLES_PROFILE_HEADER();
    result = __gles_IsProgramPipeline(gc, pipeline);
    __GLES_PROFILE_FOOTER(GLES31_ISPROGRAMPIPELINE);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glIsProgramPipeline => %d\n", result);
    }

    if (__glesTracerDispatchTable.IsProgramPipeline)
    {
        (*__glesTracerDispatchTable.IsProgramPipeline)(pipeline);
    }

    return result;
}

GLvoid GLAPIENTRY __glesProfile_GetProgramPipelineiv(__GLcontext *gc, GLuint pipeline, GLenum pname, GLint *params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetProgramPipelineiv %d 0x%04X 0x%08X\n",
                        gc, tid, pipeline, pname, params);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetProgramPipelineiv(gc, pipeline, pname, params);
    __GLES_PROFILE_FOOTER(GLES31_GETPROGRAMPIPELINEIV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetProgramPipelineiv => %d\n", *params);
    }

    if (__glesTracerDispatchTable.GetProgramPipelineiv)
    {
        (*__glesTracerDispatchTable.GetProgramPipelineiv)(pipeline, pname, params);
    }
}

GLvoid GLAPIENTRY __glesProfile_ProgramUniform1i(__GLcontext *gc, GLuint program, GLint location, GLint v0)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glProgramUniform1i %d %d %d\n",
                        gc, tid, program, location, v0);
    }

    __GLES_PROFILE_HEADER();
    __gles_ProgramUniform1i(gc, program, location, v0);
    __GLES_PROFILE_FOOTER(GLES31_PROGRAMUNIFORM1I);

    if (__glesTracerDispatchTable.ProgramUniform1i)
    {
        (*__glesTracerDispatchTable.ProgramUniform1i)(program, location, v0);
    }
}

GLvoid GLAPIENTRY __glesProfile_ProgramUniform2i(__GLcontext *gc, GLuint program, GLint location, GLint v0, GLint v1)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glProgramUniform2i %d %d %d %d\n",
                        gc, tid, program, location, v0, v1);
    }

    __GLES_PROFILE_HEADER();
    __gles_ProgramUniform2i(gc, program, location, v0, v1);
    __GLES_PROFILE_FOOTER(GLES31_PROGRAMUNIFORM2I);

    if (__glesTracerDispatchTable.ProgramUniform2i)
    {
        (*__glesTracerDispatchTable.ProgramUniform2i)(program, location, v0, v1);
    }
}

GLvoid GLAPIENTRY __glesProfile_ProgramUniform3i(__GLcontext *gc, GLuint program, GLint location, GLint v0, GLint v1, GLint v2)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glProgramUniform3i %d %d %d %d %d\n",
                        gc, tid, program, location, v0, v1, v2);
    }

    __GLES_PROFILE_HEADER();
    __gles_ProgramUniform3i(gc, program, location, v0, v1, v2);
    __GLES_PROFILE_FOOTER(GLES31_PROGRAMUNIFORM3I);

    if (__glesTracerDispatchTable.ProgramUniform3i)
    {
        (*__glesTracerDispatchTable.ProgramUniform3i)(program, location, v0, v1, v2);
    }
}

GLvoid GLAPIENTRY __glesProfile_ProgramUniform4i(__GLcontext *gc, GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glProgramUniform4i %d %d %d %d %d %d\n",
                        gc, tid, program, location, v0, v1, v2, v3);
    }

    __GLES_PROFILE_HEADER();
    __gles_ProgramUniform4i(gc, program, location, v0, v1, v2, v3);
    __GLES_PROFILE_FOOTER(GLES31_PROGRAMUNIFORM4I);

    if (__glesTracerDispatchTable.ProgramUniform4i)
    {
        (*__glesTracerDispatchTable.ProgramUniform4i)(program, location, v0, v1, v2, v3);
    }
}

GLvoid GLAPIENTRY __glesProfile_ProgramUniform1ui(__GLcontext *gc, GLuint program, GLint location, GLuint v0)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glProgramUniform1ui %d %d %d\n",
                        gc, tid, program, location, v0);
    }

    __GLES_PROFILE_HEADER();
    __gles_ProgramUniform1ui(gc, program, location, v0);
    __GLES_PROFILE_FOOTER(GLES31_PROGRAMUNIFORM1UI);

    if (__glesTracerDispatchTable.ProgramUniform1ui)
    {
        (*__glesTracerDispatchTable.ProgramUniform1ui)(program, location, v0);
    }
}

GLvoid GLAPIENTRY __glesProfile_ProgramUniform2ui(__GLcontext *gc, GLuint program, GLint location, GLuint v0, GLuint v1)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glProgramUniform2ui %d %d %d %d\n",
                        gc, tid, program, location, v0, v1);
    }

    __GLES_PROFILE_HEADER();
    __gles_ProgramUniform2ui(gc, program, location, v0, v1);
    __GLES_PROFILE_FOOTER(GLES31_PROGRAMUNIFORM2UI);

    if (__glesTracerDispatchTable.ProgramUniform2ui)
    {
        (*__glesTracerDispatchTable.ProgramUniform2ui)(program, location, v0, v1);
    }
}

GLvoid GLAPIENTRY __glesProfile_ProgramUniform3ui(__GLcontext *gc, GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glProgramUniform3ui %d %d %d %d %d\n",
                        gc, tid, program, location, v0, v1, v2);
    }

    __GLES_PROFILE_HEADER();
    __gles_ProgramUniform3ui(gc, program, location, v0, v1, v2);
    __GLES_PROFILE_FOOTER(GLES31_PROGRAMUNIFORM3UI);

    if (__glesTracerDispatchTable.ProgramUniform3ui)
    {
        (*__glesTracerDispatchTable.ProgramUniform3ui)(program, location, v0, v1, v2);
    }
}

GLvoid GLAPIENTRY __glesProfile_ProgramUniform4ui(__GLcontext *gc, GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glProgramUniform4ui %d %d %d %d %d %d\n",
                        gc, tid, program, location, v0, v1, v2, v3);
    }

    __GLES_PROFILE_HEADER();
    __gles_ProgramUniform4ui(gc, program, location, v0, v1, v2, v3);
    __GLES_PROFILE_FOOTER(GLES31_PROGRAMUNIFORM4UI);

    if (__glesTracerDispatchTable.ProgramUniform4ui)
    {
        (*__glesTracerDispatchTable.ProgramUniform4ui)(program, location, v0, v1, v2, v3);
    }
}

GLvoid GLAPIENTRY __glesProfile_ProgramUniform1f(__GLcontext *gc, GLuint program, GLint location, GLfloat v0)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glProgramUniform1f %d %d %f\n",
                        gc, tid, program, location, v0);
    }

    __GLES_PROFILE_HEADER();
    __gles_ProgramUniform1f(gc, program, location, v0);
    __GLES_PROFILE_FOOTER(GLES31_PROGRAMUNIFORM1F);

    if (__glesTracerDispatchTable.ProgramUniform1f)
    {
        (*__glesTracerDispatchTable.ProgramUniform1f)(program, location, v0);
    }
}

GLvoid GLAPIENTRY __glesProfile_ProgramUniform2f(__GLcontext *gc, GLuint program, GLint location, GLfloat v0, GLfloat v1)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glProgramUniform2f %d %d %f %f\n",
                        gc, tid, program, location, v0, v1);
    }

    __GLES_PROFILE_HEADER();
    __gles_ProgramUniform2f(gc, program, location, v0, v1);
    __GLES_PROFILE_FOOTER(GLES31_PROGRAMUNIFORM2F);

    if (__glesTracerDispatchTable.ProgramUniform2f)
    {
        (*__glesTracerDispatchTable.ProgramUniform2f)(program, location, v0, v1);
    }
}

GLvoid GLAPIENTRY __glesProfile_ProgramUniform3f(__GLcontext *gc, GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glProgramUniform3f %d %d %f %f %f\n",
                        gc, tid, program, location, v0, v1, v2);
    }

    __GLES_PROFILE_HEADER();
    __gles_ProgramUniform3f(gc, program, location, v0, v1, v2);
    __GLES_PROFILE_FOOTER(GLES31_PROGRAMUNIFORM3F);

    if (__glesTracerDispatchTable.ProgramUniform3f)
    {
        (*__glesTracerDispatchTable.ProgramUniform3f)(program, location, v0, v1, v2);
    }
}

GLvoid GLAPIENTRY __glesProfile_ProgramUniform4f(__GLcontext *gc, GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glProgramUniform4f %d %d %f %f %f %f\n",
                        gc, tid, program, location, v0, v1, v2, v3);
    }

    __GLES_PROFILE_HEADER();
    __gles_ProgramUniform4f(gc, program, location, v0, v1, v2, v3);
    __GLES_PROFILE_FOOTER(GLES31_PROGRAMUNIFORM4F);

    if (__glesTracerDispatchTable.ProgramUniform4f)
    {
        (*__glesTracerDispatchTable.ProgramUniform4f)(program, location, v0, v1, v2, v3);
    }
}

GLvoid GLAPIENTRY __glesProfile_ProgramUniform1iv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLint *value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glProgramUniform1iv %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, value);
    }

    __GLES_PROFILE_HEADER();
    __gles_ProgramUniform1iv(gc, program, location, count, value);
    __GLES_PROFILE_FOOTER(GLES31_PROGRAMUNIFORM1IV);

    if (__glesTracerDispatchTable.ProgramUniform1iv)
    {
        (*__glesTracerDispatchTable.ProgramUniform1iv)(program, location, count, value);
    }
}

GLvoid GLAPIENTRY __glesProfile_ProgramUniform2iv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLint *value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glProgramUniform2iv %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, value);
    }

    __GLES_PROFILE_HEADER();
    __gles_ProgramUniform2iv(gc, program, location, count, value);
    __GLES_PROFILE_FOOTER(GLES31_PROGRAMUNIFORM2IV);

    if (__glesTracerDispatchTable.ProgramUniform2iv)
    {
        (*__glesTracerDispatchTable.ProgramUniform2iv)(program, location, count, value);
    }
}

GLvoid GLAPIENTRY __glesProfile_ProgramUniform3iv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLint *value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glProgramUniform3iv %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, value);
    }

    __GLES_PROFILE_HEADER();
    __gles_ProgramUniform3iv(gc, program, location, count, value);
    __GLES_PROFILE_FOOTER(GLES31_PROGRAMUNIFORM3IV);

    if (__glesTracerDispatchTable.ProgramUniform3iv)
    {
        (*__glesTracerDispatchTable.ProgramUniform3iv)(program, location, count, value);
    }
}

GLvoid GLAPIENTRY __glesProfile_ProgramUniform4iv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLint *value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glProgramUniform4iv %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, value);
    }

    __GLES_PROFILE_HEADER();
    __gles_ProgramUniform4iv(gc, program, location, count, value);
    __GLES_PROFILE_FOOTER(GLES31_PROGRAMUNIFORM4IV);

    if (__glesTracerDispatchTable.ProgramUniform4iv)
    {
        (*__glesTracerDispatchTable.ProgramUniform4iv)(program, location, count, value);
    }
}

GLvoid GLAPIENTRY __glesProfile_ProgramUniform1uiv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLuint *value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glProgramUniform1uiv %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, value);
    }

    __GLES_PROFILE_HEADER();
    __gles_ProgramUniform1uiv(gc, program, location, count, value);
    __GLES_PROFILE_FOOTER(GLES31_PROGRAMUNIFORM1UIV);

    if (__glesTracerDispatchTable.ProgramUniform1uiv)
    {
        (*__glesTracerDispatchTable.ProgramUniform1uiv)(program, location, count, value);
    }
}

GLvoid GLAPIENTRY __glesProfile_ProgramUniform2uiv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLuint *value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glProgramUniform2uiv %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, value);
    }

    __GLES_PROFILE_HEADER();
    __gles_ProgramUniform2uiv(gc, program, location, count, value);
    __GLES_PROFILE_FOOTER(GLES31_PROGRAMUNIFORM2UIV);

    if (__glesTracerDispatchTable.ProgramUniform2uiv)
    {
        (*__glesTracerDispatchTable.ProgramUniform2uiv)(program, location, count, value);
    }
}

GLvoid GLAPIENTRY __glesProfile_ProgramUniform3uiv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLuint *value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glProgramUniform3uiv %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, value);
    }

    __GLES_PROFILE_HEADER();
    __gles_ProgramUniform3uiv(gc, program, location, count, value);
    __GLES_PROFILE_FOOTER(GLES31_PROGRAMUNIFORM3UIV);

    if (__glesTracerDispatchTable.ProgramUniform3uiv)
    {
        (*__glesTracerDispatchTable.ProgramUniform3uiv)(program, location, count, value);
    }
}

GLvoid GLAPIENTRY __glesProfile_ProgramUniform4uiv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLuint *value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glProgramUniform4uiv %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, value);
    }

    __GLES_PROFILE_HEADER();
    __gles_ProgramUniform4uiv(gc, program, location, count, value);
    __GLES_PROFILE_FOOTER(GLES31_PROGRAMUNIFORM4UIV);

    if (__glesTracerDispatchTable.ProgramUniform4uiv)
    {
        (*__glesTracerDispatchTable.ProgramUniform4uiv)(program, location, count, value);
    }
}

GLvoid GLAPIENTRY __glesProfile_ProgramUniform1fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glProgramUniform1fv %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, value);
    }

    __GLES_PROFILE_HEADER();
    __gles_ProgramUniform1fv(gc, program, location, count, value);
    __GLES_PROFILE_FOOTER(GLES31_PROGRAMUNIFORM1FV);

    if (__glesTracerDispatchTable.ProgramUniform1fv)
    {
        (*__glesTracerDispatchTable.ProgramUniform1fv)(program, location, count, value);
    }
}

GLvoid GLAPIENTRY __glesProfile_ProgramUniform2fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glProgramUniform2fv %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, value);
    }

    __GLES_PROFILE_HEADER();
    __gles_ProgramUniform2fv(gc, program, location, count, value);
    __GLES_PROFILE_FOOTER(GLES31_PROGRAMUNIFORM2FV);

    if (__glesTracerDispatchTable.ProgramUniform2fv)
    {
        (*__glesTracerDispatchTable.ProgramUniform2fv)(program, location, count, value);
    }
}

GLvoid GLAPIENTRY __glesProfile_ProgramUniform3fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glProgramUniform3fv %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, value);
    }

    __GLES_PROFILE_HEADER();
    __gles_ProgramUniform3fv(gc, program, location, count, value);
    __GLES_PROFILE_FOOTER(GLES31_PROGRAMUNIFORM3FV);

    if (__glesTracerDispatchTable.ProgramUniform3fv)
    {
        (*__glesTracerDispatchTable.ProgramUniform3fv)(program, location, count, value);
    }
}

GLvoid GLAPIENTRY __glesProfile_ProgramUniform4fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glProgramUniform4fv %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, value);
    }

    __GLES_PROFILE_HEADER();
    __gles_ProgramUniform4fv(gc, program, location, count, value);
    __GLES_PROFILE_FOOTER(GLES31_PROGRAMUNIFORM4FV);

    if (__glesTracerDispatchTable.ProgramUniform4fv)
    {
        (*__glesTracerDispatchTable.ProgramUniform4fv)(program, location, count, value);
    }
}

GLvoid GLAPIENTRY __glesProfile_ProgramUniformMatrix2fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glProgramUniformMatrix2fv %d %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, transpose, value);
    }

    __GLES_PROFILE_HEADER();
    __gles_ProgramUniformMatrix2fv(gc, program, location, count, transpose, value);
    __GLES_PROFILE_FOOTER(GLES31_PROGRAMUNIFORMMATRIX2FV);

    if (__glesTracerDispatchTable.ProgramUniformMatrix2fv)
    {
        (*__glesTracerDispatchTable.ProgramUniformMatrix2fv)(program, location, count, transpose, value);
    }
}

GLvoid GLAPIENTRY __glesProfile_ProgramUniformMatrix3fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glProgramUniformMatrix3fv %d %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, transpose, value);
    }

    __GLES_PROFILE_HEADER();
    __gles_ProgramUniformMatrix3fv(gc, program, location, count, transpose, value);
    __GLES_PROFILE_FOOTER(GLES31_PROGRAMUNIFORMMATRIX3FV);

    if (__glesTracerDispatchTable.ProgramUniformMatrix3fv)
    {
        (*__glesTracerDispatchTable.ProgramUniformMatrix3fv)(program, location, count, transpose, value);
    }
}

GLvoid GLAPIENTRY __glesProfile_ProgramUniformMatrix4fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glProgramUniformMatrix4fv %d %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, transpose, value);
    }

    __GLES_PROFILE_HEADER();
    __gles_ProgramUniformMatrix4fv(gc, program, location, count, transpose, value);
    __GLES_PROFILE_FOOTER(GLES31_PROGRAMUNIFORMMATRIX4FV);

    if (__glesTracerDispatchTable.ProgramUniformMatrix4fv)
    {
        (*__glesTracerDispatchTable.ProgramUniformMatrix4fv)(program, location, count, transpose, value);
    }
}

GLvoid GLAPIENTRY __glesProfile_ProgramUniformMatrix2x3fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glProgramUniformMatrix2x3fv %d %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, transpose, value);
    }

    __GLES_PROFILE_HEADER();
    __gles_ProgramUniformMatrix2x3fv(gc, program, location, count, transpose, value);
    __GLES_PROFILE_FOOTER(GLES31_PROGRAMUNIFORMMATRIX2X3FV);

    if (__glesTracerDispatchTable.ProgramUniformMatrix2x3fv)
    {
        (*__glesTracerDispatchTable.ProgramUniformMatrix2x3fv)(program, location, count, transpose, value);
    }
}

GLvoid GLAPIENTRY __glesProfile_ProgramUniformMatrix3x2fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glProgramUniformMatrix3x2fv %d %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, transpose, value);
    }

    __GLES_PROFILE_HEADER();
    __gles_ProgramUniformMatrix3x2fv(gc, program, location, count, transpose, value);
    __GLES_PROFILE_FOOTER(GLES31_PROGRAMUNIFORMMATRIX3X2FV);

    if (__glesTracerDispatchTable.ProgramUniformMatrix3x2fv)
    {
        (*__glesTracerDispatchTable.ProgramUniformMatrix3x2fv)(program, location, count, transpose, value);
    }
}

GLvoid GLAPIENTRY __glesProfile_ProgramUniformMatrix2x4fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glProgramUniformMatrix2x4fv %d %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, transpose, value);
    }

    __GLES_PROFILE_HEADER();
    __gles_ProgramUniformMatrix2x4fv(gc, program, location, count, transpose, value);
    __GLES_PROFILE_FOOTER(GLES31_PROGRAMUNIFORMMATRIX2X4FV);

    if (__glesTracerDispatchTable.ProgramUniformMatrix2x4fv)
    {
        (*__glesTracerDispatchTable.ProgramUniformMatrix2x4fv)(program, location, count, transpose, value);
    }
}

GLvoid GLAPIENTRY __glesProfile_ProgramUniformMatrix4x2fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glProgramUniformMatrix4x2fv %d %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, transpose, value);
    }

    __GLES_PROFILE_HEADER();
    __gles_ProgramUniformMatrix4x2fv(gc, program, location, count, transpose, value);
    __GLES_PROFILE_FOOTER(GLES31_PROGRAMUNIFORMMATRIX4X2FV);

    if (__glesTracerDispatchTable.ProgramUniformMatrix4x2fv)
    {
        (*__glesTracerDispatchTable.ProgramUniformMatrix4x2fv)(program, location, count, transpose, value);
    }
}

GLvoid GLAPIENTRY __glesProfile_ProgramUniformMatrix3x4fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glProgramUniformMatrix3x4fv %d %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, transpose, value);
    }

    __GLES_PROFILE_HEADER();
    __gles_ProgramUniformMatrix3x4fv(gc, program, location, count, transpose, value);
    __GLES_PROFILE_FOOTER(GLES31_PROGRAMUNIFORMMATRIX3X4FV);

    if (__glesTracerDispatchTable.ProgramUniformMatrix3x4fv)
    {
        (*__glesTracerDispatchTable.ProgramUniformMatrix3x4fv)(program, location, count, transpose, value);
    }
}

GLvoid GLAPIENTRY __glesProfile_ProgramUniformMatrix4x3fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glProgramUniformMatrix4x3fv %d %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, transpose, value);
    }

    __GLES_PROFILE_HEADER();
    __gles_ProgramUniformMatrix4x3fv(gc, program, location, count, transpose, value);
    __GLES_PROFILE_FOOTER(GLES31_PROGRAMUNIFORMMATRIX4X3FV);

    if (__glesTracerDispatchTable.ProgramUniformMatrix4x3fv)
    {
        (*__glesTracerDispatchTable.ProgramUniformMatrix4x3fv)(program, location, count, transpose, value);
    }
}

GLvoid GLAPIENTRY __glesProfile_ValidateProgramPipeline(__GLcontext *gc, GLuint pipeline)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glValidateProgramPipeline %d\n",
                        gc, tid, pipeline);
    }

    __GLES_PROFILE_HEADER();
    __gles_ValidateProgramPipeline(gc, pipeline);
    __GLES_PROFILE_FOOTER(GLES31_VALIDATEPROGRAMPIPELINE);

    if (__glesTracerDispatchTable.ValidateProgramPipeline)
    {
        (*__glesTracerDispatchTable.ValidateProgramPipeline)(pipeline);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetProgramPipelineInfoLog(__GLcontext *gc, GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetProgramPipelineInfoLog %d %d 0x%08X 0x%08X\n",
                        gc, tid, pipeline, bufSize, length, infoLog);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetProgramPipelineInfoLog(gc, pipeline, bufSize, length, infoLog);
    __GLES_PROFILE_FOOTER(GLES31_GETPROGRAMPIPELINEINFOLOG);

    if (bufSize && (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST))
    {
        __GLES_LOG_API("        glGetProgramPipelineInfoLog => %s\n", infoLog);
    }

    if (__glesTracerDispatchTable.GetProgramPipelineInfoLog)
    {
        (*__glesTracerDispatchTable.GetProgramPipelineInfoLog)(pipeline, bufSize, length, infoLog);
    }
}

GLvoid GLAPIENTRY __glesProfile_BindImageTexture(__GLcontext *gc, GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glBindImageTexture %d %d %d %d %d 0x%04X 0x%04X\n",
                        gc, tid, unit, texture, level, layered, layer, access, format);
    }

    __GLES_PROFILE_HEADER();
    __gles_BindImageTexture(gc, unit, texture, level, layered, layer, access, format);
    __GLES_PROFILE_FOOTER(GLES31_BINDIMAGETEXTURE);

    if (__glesTracerDispatchTable.BindImageTexture)
    {
        (*__glesTracerDispatchTable.BindImageTexture)(unit, texture, level, layered, layer, access, format);
    }
}

GLvoid GL_APIENTRY __glesProfile_GetTexImage(__GLcontext *gc, GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels)
{
    __gles_GetTexImage(gc, target, level, format, type, pixels);
}

GLvoid GLAPIENTRY __glesProfile_GetBooleani_v(__GLcontext *gc, GLenum target, GLuint index, GLboolean *data)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetBooleani_v 0x%04X %d 0x%08X\n",
                        gc, tid, target, index, data);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetBooleani_v(gc, target, index, data);
    __GLES_PROFILE_FOOTER(GLES31_GETBOOLEANI_V);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetBooleani_v => %d\n", *data);
    }

    if (__glesTracerDispatchTable.GetBooleani_v)
    {
        (*__glesTracerDispatchTable.GetBooleani_v)(target, index, data);
    }
}

GLvoid GLAPIENTRY __glesProfile_MemoryBarrier(__GLcontext *gc, GLbitfield barriers)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glMemoryBarrier 0x%08X\n",
                        gc, tid, barriers);
    }

    __GLES_PROFILE_HEADER();
    __gles_MemoryBarrier(gc, barriers);
    __GLES_PROFILE_FOOTER(GLES31_MEMORYBARRIER);

    if (__glesTracerDispatchTable.MemoryBarrier)
    {
        (*__glesTracerDispatchTable.MemoryBarrier)(barriers);
    }
}

GLvoid GLAPIENTRY __glesProfile_MemoryBarrierByRegion(__GLcontext *gc, GLbitfield barriers)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glMemoryBarrierByRegion 0x%08X\n",
                        gc, tid, barriers);
    }

    __GLES_PROFILE_HEADER();
    __gles_MemoryBarrierByRegion(gc, barriers);
    __GLES_PROFILE_FOOTER(GLES31_MEMORYBARRIERBYREGION);

    if (__glesTracerDispatchTable.MemoryBarrierByRegion)
    {
        (*__glesTracerDispatchTable.MemoryBarrierByRegion)(barriers);
    }
}

GLvoid GLAPIENTRY __glesProfile_TexStorage2DMultisample(__GLcontext *gc, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glTexStorage2DMultisample 0x%04X %d 0x%04X %d %d %d\n",
                        gc, tid, target, samples, internalformat, width, height, fixedsamplelocations);
    }

    __GLES_PROFILE_HEADER();
    __gles_TexStorage2DMultisample(gc, target, samples, internalformat, width, height, fixedsamplelocations);
    __GLES_PROFILE_FOOTER(GLES31_TEXSTORAGE2DMULTISAMPLE);

    if (__glesTracerDispatchTable.TexStorage2DMultisample)
    {
        (*__glesTracerDispatchTable.TexStorage2DMultisample)(target, samples, internalformat, width, height, fixedsamplelocations);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetMultisamplefv(__GLcontext *gc, GLenum pname, GLuint index, GLfloat *val)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetMultisamplefv 0x%04X %d 0x%08X\n",
                        gc, tid, pname, index, val);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetMultisamplefv(gc, pname, index, val);
    __GLES_PROFILE_FOOTER(GLES31_GETMULTISAMPLEFV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetMultisamplefv => %f\n", *val);
    }

    if (__glesTracerDispatchTable.GetMultisamplefv)
    {
        (*__glesTracerDispatchTable.GetMultisamplefv)(pname, index, val);
    }
}

GLvoid GLAPIENTRY __glesProfile_SampleMaski(__GLcontext *gc, GLuint maskNumber, GLbitfield mask)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glSampleMaski %d 0x%08X\n",
                        gc, tid, maskNumber, mask);
    }

    __GLES_PROFILE_HEADER();
    __gles_SampleMaski(gc, maskNumber, mask);
    __GLES_PROFILE_FOOTER(GLES31_SAMPLEMASKI);

    if (__glesTracerDispatchTable.SampleMaski)
    {
        (*__glesTracerDispatchTable.SampleMaski)(maskNumber, mask);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetTexLevelParameteriv(__GLcontext *gc, GLenum target, GLint level, GLenum pname, GLint *params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetTexLevelParameteriv 0x%04X %d 0x%04X 0x%08X\n",
                        gc, tid, target, level, pname, params);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetTexLevelParameteriv(gc, target, level, pname, params);
    __GLES_PROFILE_FOOTER(GLES31_GETTEXLEVELPARAMETERIV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetTexLevelParameteriv => %d\n", *params);
    }

    if (__glesTracerDispatchTable.GetTexLevelParameteriv)
    {
        (*__glesTracerDispatchTable.GetTexLevelParameteriv)(target, level, pname, params);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetTexLevelParameterfv(__GLcontext *gc, GLenum target, GLint level, GLenum pname, GLfloat *params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetTexLevelParameterfv 0x%04X %d 0x%04X 0x%08X\n",
                        gc, tid, target, level, pname, params);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetTexLevelParameterfv(gc, target, level, pname, params);
    __GLES_PROFILE_FOOTER(GLES31_GETTEXLEVELPARAMETERFV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetTexLevelParameterfv => %f\n", *params);
    }

    if (__glesTracerDispatchTable.GetTexLevelParameterfv)
    {
        (*__glesTracerDispatchTable.GetTexLevelParameterfv)(target, level, pname, params);
    }
}

GLvoid GLAPIENTRY __glesProfile_BindVertexBuffer(__GLcontext *gc, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glBindVertexBuffer %d %d 0x%08X %d\n",
                        gc, tid, bindingindex, buffer, offset, stride);
    }

    __GLES_PROFILE_HEADER();
    __gles_BindVertexBuffer(gc, bindingindex, buffer, offset, stride);
    __GLES_PROFILE_FOOTER(GLES31_BINDVERTEXBUFFER);

    if (__glesTracerDispatchTable.BindVertexBuffer)
    {
        (*__glesTracerDispatchTable.BindVertexBuffer)(bindingindex, buffer, offset, stride);
    }
}

GLvoid GLAPIENTRY __glesProfile_VertexAttribFormat(__GLcontext *gc, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glVertexAttribFormat %d %d 0x%04X %d %d\n",
                        gc, tid, attribindex, size, type, normalized, relativeoffset);
    }

    __GLES_PROFILE_HEADER();
    __gles_VertexAttribFormat(gc, attribindex, size, type, normalized, relativeoffset);
    __GLES_PROFILE_FOOTER(GLES31_VERTEXATTRIBFORMAT);

    if (__glesTracerDispatchTable.VertexAttribFormat)
    {
        (*__glesTracerDispatchTable.VertexAttribFormat)(attribindex, size, type, normalized, relativeoffset);
    }
}

GLvoid GLAPIENTRY __glesProfile_VertexAttribIFormat(__GLcontext *gc, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glVertexAttribIFormat %d %d 0x%04X %d\n",
                        gc, tid, attribindex, size, type, relativeoffset);
    }

    __GLES_PROFILE_HEADER();
    __gles_VertexAttribIFormat(gc, attribindex, size, type, relativeoffset);
    __GLES_PROFILE_FOOTER(GLES31_VERTEXATTRIBIFORMAT);

    if (__glesTracerDispatchTable.VertexAttribIFormat)
    {
        (*__glesTracerDispatchTable.VertexAttribIFormat)(attribindex, size, type, relativeoffset);
    }
}

GLvoid GLAPIENTRY __glesProfile_VertexAttribBinding(__GLcontext *gc, GLuint attribindex, GLuint bindingindex)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glVertexAttribBinding %d %d\n",
                        gc, tid, attribindex, bindingindex);
    }

    __GLES_PROFILE_HEADER();
    __gles_VertexAttribBinding(gc, attribindex, bindingindex);
    __GLES_PROFILE_FOOTER(GLES31_VERTEXATTRIBBINDING);

    if (__glesTracerDispatchTable.VertexAttribBinding)
    {
        (*__glesTracerDispatchTable.VertexAttribBinding)(attribindex, bindingindex);
    }
}

GLvoid GLAPIENTRY __glesProfile_VertexBindingDivisor(__GLcontext *gc, GLuint bindingindex, GLuint divisor)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glVertexBindingDivisor %d %d\n",
                        gc, tid, bindingindex, divisor);
    }

    __GLES_PROFILE_HEADER();
    __gles_VertexBindingDivisor(gc, bindingindex, divisor);
    __GLES_PROFILE_FOOTER(GLES31_VERTEXBINDINGDIVISOR);

    if (__glesTracerDispatchTable.VertexBindingDivisor)
    {
        (*__glesTracerDispatchTable.VertexBindingDivisor)(bindingindex, divisor);
    }
}

GLvoid GLAPIENTRY __glesProfile_TexStorage3DMultisample(__GLcontext *gc, GLenum target, GLsizei samples, GLenum sizedinternalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glTexStorage3DMultisample 0x%04X %d 0x%04X %d %d %d %d\n",
                        gc, tid, target, samples, sizedinternalformat, width, height, depth, fixedsamplelocations);
    }

    __GLES_PROFILE_HEADER();
    __gles_TexStorage3DMultisample(gc, target, samples, sizedinternalformat, width, height, depth, fixedsamplelocations);
    __GLES_PROFILE_FOOTER(GLES31_TEXSTORAGE3DMULTISAMPLE);

    if (__glesTracerDispatchTable.TexStorage3DMultisample)
    {
        (*__glesTracerDispatchTable.TexStorage3DMultisample)(target, samples, sizedinternalformat, width, height, depth, fixedsamplelocations);
    }
}

GLvoid GLAPIENTRY __glesProfile_BlendBarrier(__GLcontext *gc)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glBlendBarrier\n",
                        gc, tid);
    }

    __GLES_PROFILE_HEADER();
    __gles_BlendBarrier(gc);
    __GLES_PROFILE_FOOTER(GLES31_BLENDBARRIER);

    if (__glesTracerDispatchTable.BlendBarrier)
    {
        (*__glesTracerDispatchTable.BlendBarrier)();
    }
}

GLvoid GLAPIENTRY __glesProfile_DebugMessageControl(__GLcontext *gc, GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glDebugMessageControl(0x%04X, 0x%04X, 0x%04X, %d, 0x%08X, %d)\n",
                        gc, tid, source, type, severity, count, ids, enabled);
    }

    __GLES_PROFILE_HEADER();
    __gles_DebugMessageControl(gc, source, type, severity, count, ids, enabled);
    __GLES_PROFILE_FOOTER(GLES31_DEBUGMESSAGECONTROL);

    if (__glesTracerDispatchTable.DebugMessageControl)
    {
        (*__glesTracerDispatchTable.DebugMessageControl)(source, type, severity, count, ids, enabled);
    }
}

GLvoid GLAPIENTRY __glesProfile_DebugMessageInsert(__GLcontext *gc, GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* buf)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glDebugMessageInsert(0x%04X, 0x%04X, %u, 0x%04X, %d, 0x%08X)\n",
                        gc, tid, source, type, id, severity, length, buf);
    }

    __GLES_PROFILE_HEADER();
    __gles_DebugMessageInsert(gc, source, type, id, severity, length, buf);
    __GLES_PROFILE_FOOTER(GLES31_DEBUGMESSAGEINSERT);

    if (__glesTracerDispatchTable.DebugMessageInsert)
    {
        (*__glesTracerDispatchTable.DebugMessageInsert)(source, type, id, severity, length, buf);
    }
}

GLvoid GLAPIENTRY __glesProfile_DebugMessageCallback(__GLcontext *gc, GLDEBUGPROCKHR callback, const GLvoid* userParam)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glDebugMessageCallback(0x%08X, 0x%08X)\n",
                        gc, tid, callback, userParam);
    }

    __GLES_PROFILE_HEADER();
    __gles_DebugMessageCallback(gc, callback, userParam);
    __GLES_PROFILE_FOOTER(GLES31_DEBUGMESSAGECALLBACK);

    if (__glesTracerDispatchTable.DebugMessageCallback)
    {
        (*__glesTracerDispatchTable.DebugMessageCallback)(callback, userParam);
    }
}

GLuint GLAPIENTRY __glesProfile_GetDebugMessageLog(__GLcontext *gc, GLuint count, GLsizei bufSize, GLenum* sources, GLenum* types, GLuint* ids, GLenum* severities, GLsizei* lengths, GLchar* messageLog)
{
    __GLES_PROFILE_VARS();
    GLuint number;

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetDebugMessageLog(%u, %d, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X)\n",
                        gc, tid, count, bufSize, sources, types, ids, severities, lengths, messageLog);
    }

    __GLES_PROFILE_HEADER();
    number = __gles_GetDebugMessageLog(gc, count, bufSize, sources, types, ids, severities, lengths, messageLog);
    __GLES_PROFILE_FOOTER(GLES31_GETDEBUGMESSAGELOG);

    if (__glesTracerDispatchTable.GetDebugMessageLog)
    {
        (*__glesTracerDispatchTable.GetDebugMessageLog)(count, bufSize, sources, types, ids, severities, lengths, messageLog);
    }
    return number;
}

GLvoid GLAPIENTRY __glesProfile_GetPointerv(__GLcontext *gc, GLenum pname, GLvoid** params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetPointerv(0x%04X, 0x%08X)\n",
                        gc, tid, pname, params);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetPointerv(gc, pname, params);
    __GLES_PROFILE_FOOTER(GLES31_GETPOINTERV);

    if (__glesTracerDispatchTable.GetPointerv)
    {
        (*__glesTracerDispatchTable.GetPointerv)(pname, params);
    }
}

GLvoid GLAPIENTRY __glesProfile_PushDebugGroup(__GLcontext *gc, GLenum source, GLuint id, GLsizei length, const GLchar * message)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glPushDebugGroup(0x%04X, %u, %d, 0x%08X)\n",
                        gc, tid, source, id, length, message);
    }

    __GLES_PROFILE_HEADER();
    __gles_PushDebugGroup(gc, source, id, length, message);
    __GLES_PROFILE_FOOTER(GLES31_PUSHDEBUGGROUP);

    if (__glesTracerDispatchTable.PushDebugGroup)
    {
        (*__glesTracerDispatchTable.PushDebugGroup)(source, id, length, message);
    }
}

GLvoid GLAPIENTRY __glesProfile_PopDebugGroup(__GLcontext *gc)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glPopDebugGroup()\n",
                        gc, tid);
    }

    __GLES_PROFILE_HEADER();
    __gles_PopDebugGroup(gc);
    __GLES_PROFILE_FOOTER(GLES31_POPDEBUGGROUP);

    if (__glesTracerDispatchTable.PopDebugGroup)
    {
        (*__glesTracerDispatchTable.PopDebugGroup)();
    }
}

GLvoid GLAPIENTRY __glesProfile_ObjectLabel(__GLcontext *gc, GLenum identifier, GLuint name, GLsizei length, const GLchar *label)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glObjectLabel(0x%04X, %u, %d, 0x%08X)\n",
                        gc, tid, identifier, name, length, label);
    }

    __GLES_PROFILE_HEADER();
    __gles_ObjectLabel(gc, identifier, name, length, label);
    __GLES_PROFILE_FOOTER(GLES31_OBJECTLABEL);

    if (__glesTracerDispatchTable.ObjectLabel)
    {
        (*__glesTracerDispatchTable.ObjectLabel)(identifier, name, length, label);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetObjectLabel(__GLcontext *gc, GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetObjectLabel(0x%04X, %u, %d, 0x%08X, 0x%08X)\n",
                        gc, tid, identifier, name, bufSize, length, label);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetObjectLabel(gc, identifier, name, bufSize, length, label);
    __GLES_PROFILE_FOOTER(GLES31_GETOBJECTLABEL);

    if (__glesTracerDispatchTable.GetObjectLabel)
    {
        (*__glesTracerDispatchTable.GetObjectLabel)(identifier, name, bufSize, length, label);
    }
}

GLvoid GLAPIENTRY __glesProfile_ObjectPtrLabel(__GLcontext *gc, const GLvoid* ptr, GLsizei length, const GLchar *label)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glObjectPtrLabel(0x%08X, %d, 0x%08X)\n",
                        gc, tid, ptr, length, label);
    }

    __GLES_PROFILE_HEADER();
    __gles_ObjectPtrLabel(gc, ptr, length, label);
    __GLES_PROFILE_FOOTER(GLES31_OBJECTPTRLABEL);

    if (__glesTracerDispatchTable.ObjectPtrLabel)
    {
        (*__glesTracerDispatchTable.ObjectPtrLabel)(ptr, length, label);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetObjectPtrLabel(__GLcontext *gc, const GLvoid* ptr, GLsizei bufSize, GLsizei *length, GLchar *label)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetObjectPtrLabel(0x%08X, %d, 0x%08X, 0x%08X)\n",
                        gc, tid, ptr, bufSize, length, label);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetObjectPtrLabel(gc, ptr, bufSize, length, label);
    __GLES_PROFILE_FOOTER(GLES31_GETOBJECTPTRLABEL);

    if (__glesTracerDispatchTable.GetObjectPtrLabel)
    {
        (*__glesTracerDispatchTable.GetObjectPtrLabel)(ptr, bufSize, length, label);
    }
}

GLenum GLAPIENTRY __glesProfile_GetGraphicsResetStatus(__GLcontext *gc)
{
    __GLES_PROFILE_VARS();
    GLenum retStatus;

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetGraphicsResetStatusEXT\n",
                        gc, tid);
    }

    __GLES_PROFILE_HEADER();
    retStatus = __gles_GetGraphicsResetStatus(gc);
    __GLES_PROFILE_FOOTER(GLES3_GETGRAPHICSRESETSTATUS);

    if (__glesTracerDispatchTable.GetGraphicsResetStatus)
    {
        (*__glesTracerDispatchTable.GetGraphicsResetStatus)();
    }

    return retStatus;
}

GLvoid GLAPIENTRY __glesProfile_ReadnPixels(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, GLvoid *data)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glReadnPixels %d %d %d %d 0x%04X 0x%04X %d 0x%08X\n",
                        gc, tid, x, y, width, height, format, type, bufSize, __GL_PTR2UINT(data));
    }

    __GLES_PROFILE_HEADER();
    __gles_ReadnPixels(gc, x, y, width, height, format, type, bufSize, data);
    __GLES_PROFILE_FOOTER(GLES3_READNPIXELS);

    if (__glesTracerDispatchTable.ReadnPixels)
    {
        (*__glesTracerDispatchTable.ReadnPixels)(x, y, width, height, format, type, bufSize, data);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetnUniformfv(__GLcontext *gc, GLuint program, GLint location, GLsizei bufSize, GLfloat *params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetnUniformfv %d %d %d\n",
                        gc, tid, program, location, bufSize);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetnUniformfv(gc, program, location, bufSize, params);
    __GLES_PROFILE_FOOTER(GLES3_GETNUNIFORMFV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetnUniformfv => %f\n", __GL_PTRVALUE(params));
    }

    if (__glesTracerDispatchTable.GetnUniformfv)
    {
        (*__glesTracerDispatchTable.GetnUniformfv)(program, location, bufSize, params);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetnUniformiv(__GLcontext *gc, GLuint program, GLint location, GLsizei bufSize, GLint *params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetnUniformiv %d %d %d\n",
                        gc, tid, program, location, bufSize);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetnUniformiv(gc, program, location, bufSize, params);
    __GLES_PROFILE_FOOTER(GLES3_GETNUNIFORMIV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetnUniformiv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glesTracerDispatchTable.GetnUniformiv)
    {
        (*__glesTracerDispatchTable.GetnUniformiv)(program, location, bufSize, params);
    }
}

GLvoid GLAPIENTRY __glesProfile_GetnUniformuiv(__GLcontext *gc, GLuint program, GLint location, GLsizei bufSize, GLuint *params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetnUniformuiv %d %d %d\n",
                        gc, tid, program, location, bufSize);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetnUniformuiv(gc, program, location, bufSize, params);
    __GLES_PROFILE_FOOTER(GLES3_GETNUNIFORMUIV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetnUniformuiv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glesTracerDispatchTable.GetnUniformuiv)
    {
        (*__glesTracerDispatchTable.GetnUniformuiv)(program, location, bufSize, params);
    }
}

GLvoid GLAPIENTRY __glesProfile_BlendEquationi(__GLcontext * gc, GLuint buf, GLenum mode)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glBlendEquationi %d 0x%04X\n",
                        gc, tid, buf, mode);
    }

    __GLES_PROFILE_HEADER();
    __gles_BlendEquationi(gc, buf, mode);
    __GLES_PROFILE_FOOTER(GLES31_BLENDEQUATIONI);

    if (__glesTracerDispatchTable.BlendEquationi)
    {
        (*__glesTracerDispatchTable.BlendEquationi)(buf, mode);
    }

}


GLvoid GLAPIENTRY __glesProfile_BlendEquationSeparatei(__GLcontext * gc, GLuint buf, GLenum modeRGB, GLenum modeAlpha)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glBlendEquationSeparatei %d 0x%04X 0x%04X\n",
                        gc, tid, buf, modeRGB, modeAlpha);
    }

    __GLES_PROFILE_HEADER();
    __gles_BlendEquationSeparatei(gc, buf, modeRGB, modeAlpha);
    __GLES_PROFILE_FOOTER(GLES31_BLENDEQUATIONSEPARATEI);

    if (__glesTracerDispatchTable.BlendEquationSeparatei)
    {
        (*__glesTracerDispatchTable.BlendEquationSeparatei)(buf, modeRGB, modeAlpha);
    }
}


GLvoid GLAPIENTRY __glesProfile_BlendFunci(__GLcontext * gc, GLuint buf, GLenum sfactor, GLenum dfactor)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glBlendFunci %d 0x%04X 0x%04X\n",
                        gc, tid, buf, sfactor, dfactor);
    }

    __GLES_PROFILE_HEADER();
    __gles_BlendFunci(gc, buf, sfactor, dfactor);
    __GLES_PROFILE_FOOTER(GLES31_BLENDFUNCI);

    if (__glesTracerDispatchTable.BlendFunci)
    {
        (*__glesTracerDispatchTable.BlendFunci)(buf, sfactor, dfactor);
    }

}

GLvoid GLAPIENTRY __glesProfile_BlendFuncSeparatei(__GLcontext * gc, GLuint buf, GLenum sfactorRGB,GLenum dfactorRGB,GLenum sfactorAlpha,GLenum dfactorAlpha)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glBlendFuncSeparatei %d 0x%04X 0x%04X 0x%04X 0x%04X\n",
                        gc, tid, buf, sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
    }

    __GLES_PROFILE_HEADER();
    __gles_BlendFuncSeparatei(gc, buf, sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
    __GLES_PROFILE_FOOTER(GLES31_BLENDFUNCSEPARATEI);

    if (__glesTracerDispatchTable.BlendFuncSeparatei)
    {
        (*__glesTracerDispatchTable.BlendFuncSeparatei)(buf, sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
    }
}


GLvoid GLAPIENTRY __glesProfile_ColorMaski(__GLcontext * gc,GLuint buf, GLboolean r, GLboolean g, GLboolean b, GLboolean a)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glColorMaski %d %d %d %d %d\n",
                        gc, tid, buf, r, g, b, a);
    }

    __GLES_PROFILE_HEADER();
    __gles_ColorMaski(gc, buf, r, g, b, a);
    __GLES_PROFILE_FOOTER(GLES31_COLORMASKI);

    if (__glesTracerDispatchTable.ColorMaski)
    {
        (*__glesTracerDispatchTable.ColorMaski)(buf, r, g, b, a);
    }
}

GLvoid GLAPIENTRY __glesProfile_Enablei(__GLcontext *gc, GLenum target, GLuint index)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glEnablei 0x%04X %d \n",
                        gc, tid, target, index);
    }

    __GLES_PROFILE_HEADER();
    __gles_Enablei(gc, target, index);
    __GLES_PROFILE_FOOTER(GLES31_ENABLEI);

    if (__glesTracerDispatchTable.Enablei)
    {
        (*__glesTracerDispatchTable.Enablei)(target, index);
    }

}

GLvoid GLAPIENTRY __glesProfile_Disablei(__GLcontext *gc, GLenum target, GLuint index)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glDisablei 0x%04X %d \n",
                        gc, tid, target, index);
    }

    __GLES_PROFILE_HEADER();
    __gles_Disablei(gc, target, index);
    __GLES_PROFILE_FOOTER(GLES31_DISABLEI);

    if (__glesTracerDispatchTable.Disablei)
    {
        (*__glesTracerDispatchTable.Disablei)(target, index);
    }
}

GLboolean GLAPIENTRY __glesProfile_IsEnabledi(__GLcontext * gc, GLenum target, GLuint index)
{
    __GLES_PROFILE_VARS();
    GLboolean is;

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glIsEnablediEXT 0x%04X %d\n",
                        gc, tid, target, index);
    }

    __GLES_PROFILE_HEADER();
    is = __gles_IsEnabledi(gc, target, index);
    __GLES_PROFILE_FOOTER(GLES31_ISENABLEDI);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glIsEnabledi => %d\n", is);
    }

    if (__glesTracerDispatchTable.IsEnabledi)
    {
        (*__glesTracerDispatchTable.IsEnabledi)(target, index);
    }

    return is;

}

GLvoid GLAPIENTRY __glesProfile_TexParameterIiv(__GLcontext *gc, GLenum target, GLenum pname, const GLint *params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glTexParameterIiv 0x%04X 0x%04X %d\n",
                        gc, tid, target, pname, __GL_PTRVALUE(params));
    }

    __GLES_PROFILE_HEADER();
    __gles_TexParameterIiv(gc, target, pname, params);
    __GLES_PROFILE_FOOTER(GLES31_TEXPARAMETERIIV);

    if (__glesTracerDispatchTable.TexParameterIiv)
    {
        (*__glesTracerDispatchTable.TexParameterIiv)(target, pname, params);
    }
}
GLvoid GLAPIENTRY __glesProfile_TexParameterIuiv(__GLcontext *gc, GLenum target, GLenum pname, const GLuint *params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glTexParameterIuiv 0x%04X 0x%04X %d\n",
                        gc, tid, target, pname, __GL_PTRVALUE(params));
    }

    __GLES_PROFILE_HEADER();
    __gles_TexParameterIuiv(gc, target, pname, params);
    __GLES_PROFILE_FOOTER(GLES31_TEXPARAMETERIUIV);

    if (__glesTracerDispatchTable.TexParameterIuiv)
    {
        (*__glesTracerDispatchTable.TexParameterIuiv)(target, pname, params);
    }
}
GLvoid GLAPIENTRY __glesProfile_GetTexParameterIiv(__GLcontext *gc, GLenum target, GLenum pname, GLint *params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetTexParameterIiv 0x%04X 0x%04X 0x%08X\n",
                        gc, tid, target, pname, params);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetTexParameterIiv(gc, target, pname, params);
    __GLES_PROFILE_FOOTER(GLES31_GETTEXPARAMETERIIV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetTexParameterIiv => %d\n", *params);
    }

    if (__glesTracerDispatchTable.GetTexParameterIiv)
    {
        (*__glesTracerDispatchTable.GetTexParameterIiv)(target, pname, params);
    }
}
GLvoid GLAPIENTRY __glesProfile_GetTexParameterIuiv(__GLcontext *gc, GLenum target, GLenum pname, GLuint *params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetTexParameterIuiv 0x%04X 0x%04X 0x%08X\n",
                        gc, tid, target, pname, params);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetTexParameterIuiv(gc, target, pname, params);
    __GLES_PROFILE_FOOTER(GLES31_GETTEXPARAMETERIUIV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetTexParameterIuiv => %d\n", *params);
    }

    if (__glesTracerDispatchTable.GetTexParameterIuiv)
    {
        (*__glesTracerDispatchTable.GetTexParameterIuiv)(target, pname, params);
    }
}
GLvoid GLAPIENTRY __glesProfile_SamplerParameterIiv(__GLcontext *gc, GLuint sampler, GLenum pname, const GLint *param)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glSamplerParameterIiv 0x%04X 0x%04X %d\n",
                        gc, tid, sampler, pname, __GL_PTRVALUE(param));
    }

    __GLES_PROFILE_HEADER();
    __gles_SamplerParameterIiv(gc, sampler, pname, param);
    __GLES_PROFILE_FOOTER(GLES31_SAMPLERPARAMETERIIV);

    if (__glesTracerDispatchTable.SamplerParameterIiv)
    {
        (*__glesTracerDispatchTable.SamplerParameterIiv)(sampler, pname, param);
    }

}
GLvoid GLAPIENTRY __glesProfile_SamplerParameterIuiv(__GLcontext *gc, GLuint sampler, GLenum pname, const GLuint *param)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glSamplerParameterIuiv 0x%04X 0x%04X %d\n",
                        gc, tid, sampler, pname, __GL_PTRVALUE(param));
    }

    __GLES_PROFILE_HEADER();
    __gles_SamplerParameterIuiv(gc, sampler, pname, param);
    __GLES_PROFILE_FOOTER(GLES31_SAMPLERPARAMETERIUIV);

    if (__glesTracerDispatchTable.SamplerParameterIuiv)
    {
        (*__glesTracerDispatchTable.SamplerParameterIuiv)(sampler, pname, param);
    }

}
GLvoid GLAPIENTRY __glesProfile_GetSamplerParameterIiv(__GLcontext *gc, GLuint sampler, GLenum pname, GLint *params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetSamplerParameterIiv 0x%04X 0x%04X 0x%08X\n",
                        gc, tid, sampler, pname, params);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetSamplerParameterIiv(gc, sampler, pname, params);
    __GLES_PROFILE_FOOTER(GLES31_GETSAMPLERPARAMETERIIV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetSamplerParameterIiv => %d\n", *params);
    }

    if (__glesTracerDispatchTable.GetSamplerParameterIiv)
    {
        (*__glesTracerDispatchTable.GetSamplerParameterIiv)(sampler, pname, params);
    }
}
GLvoid GLAPIENTRY __glesProfile_GetSamplerParameterIuiv (__GLcontext *gc, GLuint sampler, GLenum pname, GLuint *params)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glGetSamplerParameterIuiv 0x%04X 0x%04X 0x%08X\n",
                        gc, tid, sampler, pname, params);
    }

    __GLES_PROFILE_HEADER();
    __gles_GetSamplerParameterIuiv(gc, sampler, pname, params);
    __GLES_PROFILE_FOOTER(GLES31_GETSAMPLERPARAMETERIUIV);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glGetSamplerParameterIuiv => %d\n", *params);
    }

    if (__glesTracerDispatchTable.GetSamplerParameterIuiv)
    {
        (*__glesTracerDispatchTable.GetSamplerParameterIuiv)(sampler, pname, params);
    }
}

GLvoid GLAPIENTRY __glesProfile_TexBuffer(__GLcontext *gc, GLenum target, GLenum internalformat, GLuint buffer)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glTexBuffer 0x%04X 0x%04X %d\n",
                        gc, tid, target, internalformat, buffer);
    }

    __GLES_PROFILE_HEADER();
    __gles_TexBuffer(gc, target, internalformat, buffer);
    __GLES_PROFILE_FOOTER(GLES31_TEXBUFFER);

    if (__glesTracerDispatchTable.TexBuffer)
    {
        (*__glesTracerDispatchTable.TexBuffer)(target, internalformat, buffer);
    }

}
GLvoid GLAPIENTRY __glesProfile_TexBufferRange(__GLcontext *gc, GLenum target, GLenum internalformat,
                                                   GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glTexBufferRange 0x%04X 0x%04X %d 0x%08X 0x%08X\n",
                        gc, tid, target, internalformat, buffer, __GL_PTR2UINT(offset), __GL_PTR2UINT(size));
    }

    __GLES_PROFILE_HEADER();
    __gles_TexBufferRange(gc, target, internalformat, buffer, offset, size);
    __GLES_PROFILE_FOOTER(GLES31_TEXBUFFERRANGE);

    if (__glesTracerDispatchTable.TexBufferRange)
    {
        (*__glesTracerDispatchTable.TexBufferRange)(target, internalformat, buffer, offset, size);
    }

}

GLvoid GLAPIENTRY __glesProfile_PatchParameteri(__GLcontext *gc, GLenum pname, GLint value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glPatchParameteri 0x%04X %d\n",
                        gc, tid, pname, value);
    }

    __GLES_PROFILE_HEADER();
    __gles_PatchParameteri(gc, pname, value);
    __GLES_PROFILE_FOOTER(GLES31_PATCHPARAMETERI);

    if (__glesTracerDispatchTable.PatchParameteri)
    {
        (*__glesTracerDispatchTable.PatchParameteri)(pname, value);
    }

}

GLvoid GLAPIENTRY __glesProfile_FramebufferTexture(__GLcontext *gc, GLenum target, GLenum attachment, GLuint texture, GLint level)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glFramebufferTexture 0x%04X 0x%04X %d %d\n",
                        gc, tid, target, attachment, texture, level);
    }

    __GLES_PROFILE_HEADER();
    __gles_FramebufferTexture(gc, target, attachment, texture, level);
    __GLES_PROFILE_FOOTER(GLES31_FRAMEBUFFERTEXTURE);

    if (__glesTracerDispatchTable.FramebufferTexture)
    {
        (*__glesTracerDispatchTable.FramebufferTexture)(target, attachment, texture, level);
    }

}

GLvoid GLAPIENTRY __glesProfile_MinSampleShading(__GLcontext *gc, GLfloat value)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glMinSampleShading %f\n",
                        gc, tid, value);
    }

    __GLES_PROFILE_HEADER();
    __gles_MinSampleShading(gc, value);
    __GLES_PROFILE_FOOTER(GLES31_MINSAMPLESHADING);

    if (__glesTracerDispatchTable.MinSampleShading)
    {
        (*__glesTracerDispatchTable.MinSampleShading)(value);
    }
}

GLvoid GLAPIENTRY __glesProfile_CopyImageSubData(__GLcontext *gc,
                                                  GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ,
                                                  GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ,
                                                  GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glCopyImageSubData %d 0x%04X %d %d %d %d "
                        "%d 0x%04X %d %d %d %d %d %d %d",
                        gc, tid, srcName, srcTarget, srcLevel, srcX, srcY, srcZ,
                        dstName, dstTarget, dstLevel, dstX, dstY, dstZ,
                        srcWidth, srcHeight, srcDepth);
    }

    __GLES_PROFILE_HEADER();
    __gles_CopyImageSubData(gc,
                            srcName, srcTarget, srcLevel, srcX, srcY, srcZ,
                            dstName, dstTarget, dstLevel, dstX, dstY, dstZ,
                            srcWidth, srcHeight, srcDepth);
    __GLES_PROFILE_FOOTER(GLES31_COPYIMAGESUBDATA);

    if (__glesTracerDispatchTable.CopyImageSubData)
    {
        (*__glesTracerDispatchTable.CopyImageSubData)(srcName, srcTarget, srcLevel, srcX, srcY, srcZ,
                                                         dstName, dstTarget, dstLevel, dstX, dstY, dstZ,
                                                         srcWidth, srcHeight, srcDepth);
    }
}


GLvoid GLAPIENTRY __glesProfile_DrawElementsBaseVertex(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glDrawElementsBaseVertex 0x%04X 0x%04X 0x%04X 0x%08X 0x%08X\n",
                        gc, tid, mode, count, type, indices, basevertex);
    }

    __GLES_PROFILE_HEADER();
    __gles_DrawElementsBaseVertex(gc, mode, count, type, indices, basevertex);
    __GLES_PROFILE_FOOTER(GLES31_DRAWELEMENTSBASEVERTEX);

    if (__glesTracerDispatchTable.DrawElementsBaseVertex)
    {
        (*__glesTracerDispatchTable.DrawElementsBaseVertex)(mode, count, type, indices, basevertex);
    }
}

GLvoid GLAPIENTRY __glesProfile_DrawRangeElementsBaseVertex(__GLcontext *gc, GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glDrawRangeElementsBaseVertex 0x%04X 0x%04X 0x%04X 0x%04X 0x%04X 0x%08X 0x%04X\n",
                        gc, tid, mode, start, end, count, type, indices, basevertex);
    }

    __GLES_PROFILE_HEADER();
    __gles_DrawRangeElementsBaseVertex(gc, mode, start, end, count, type, indices, basevertex);
    __GLES_PROFILE_FOOTER(GLES31_DRAWRANGEELEMENTSBASEVERTEX);

    if (__glesTracerDispatchTable.DrawRangeElementsBaseVertex)
    {
        (*__glesTracerDispatchTable.DrawRangeElementsBaseVertex)(mode, start, end, count, type, indices, basevertex);
    }
}

GLvoid GLAPIENTRY __glesProfile_DrawElementsInstancedBaseVertex(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glDrawElementsInstancedBaseVertex 0x%04X 0x%04X 0x%04X 0x%08X 0x%04X 0x%04X\n",
                        gc, tid, mode, count, type, indices, instancecount, basevertex);
    }

    __GLES_PROFILE_HEADER();
    __gles_DrawElementsInstancedBaseVertex(gc, mode, count, type, indices, instancecount, basevertex);
    __GLES_PROFILE_FOOTER(GLES31_DRAWELEMENTSINSTANCEDBASEVERTEX);

    if (__glesTracerDispatchTable.DrawElementsInstancedBaseVertex)
    {
        (*__glesTracerDispatchTable.DrawElementsInstancedBaseVertex)(mode, count, type, indices, instancecount, basevertex);
    }
}

GLvoid GLAPIENTRY __glesProfile_PrimitiveBoundingBox(__GLcontext *gc, GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW,
                                                      GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glPrimitiveBoundingBox 0x%04X 0x%04X 0x%04X 0x%04X 0x%04X 0x%04X 0x%04X\n",
                        gc, tid, minX, minY, minZ, minW, maxX, maxY, maxZ, maxW);
    }

    __GLES_PROFILE_HEADER();
    __gles_PrimitiveBoundingBox(gc, minX, minY, minZ, minW, maxX, maxY, maxZ, maxW);
    __GLES_PROFILE_FOOTER(GLES31_DRAWELEMENTSINSTANCEDBASEVERTEX);

    if (__glesTracerDispatchTable.PrimitiveBoundingBox)
    {
        (*__glesTracerDispatchTable.PrimitiveBoundingBox)(minX, minY, minZ, minW, maxX, maxY, maxZ, maxW);
    }
}

/*
** OpenGL ES extensions
*/
#if GL_OES_EGL_image
GLvoid GLAPIENTRY __glesProfile_EGLImageTargetTexture2DOES(__GLcontext *gc, GLenum target, GLeglImageOES image)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glEGLImageTargetTexture2DOES 0x%04X 0x%08X\n",
                        gc, tid, target, __GL_PTR2UINT(image));
    }

    __GLES_PROFILE_HEADER();
    __gles_EGLImageTargetTexture2DOES(gc, target, image);
    __GLES_PROFILE_FOOTER(GLES3_EGLIMAGETARGETTEXTURE2DOES);

    if (__glesTracerDispatchTable.EGLImageTargetTexture2DOES)
    {
        (*__glesTracerDispatchTable.EGLImageTargetTexture2DOES)(target, image);
    }
}

GLvoid GLAPIENTRY __glesProfile_EGLImageTargetRenderbufferStorageOES(__GLcontext *gc, GLenum target, GLeglImageOES image)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glEGLImageTargetRenderbufferStorageOES 0x%04X 0x%08X\n",
                        gc, tid, target, __GL_PTR2UINT(image));
    }

    __GLES_PROFILE_HEADER();
    __gles_EGLImageTargetRenderbufferStorageOES(gc, target, image);
    __GLES_PROFILE_FOOTER(GLES3_EGLIMAGETARGETRENDERBUFFERSTORAGEOES);

    if (__glesTracerDispatchTable.EGLImageTargetRenderbufferStorageOES)
    {
        (*__glesTracerDispatchTable.EGLImageTargetRenderbufferStorageOES)(target, image);
    }
}
#endif

#if GL_EXT_multi_draw_arrays
GLvoid GLAPIENTRY __glesProfile_MultiDrawArraysEXT(__GLcontext *gc, GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glMultiDrawArraysEXT 0x%04X 0x%08X 0x%08X %d\n",
                        gc, tid, mode, __GL_PTR2UINT(first), __GL_PTR2UINT(count), primcount);
    }

    __GLES_PROFILE_HEADER();
    __gles_MultiDrawArraysEXT(gc, mode, first, count, primcount);
    __GLES_PROFILE_FOOTER(GLES3_MULTIDRAWARRAYSEXT);

    if (__glesTracerDispatchTable.MultiDrawArraysEXT)
    {
        (*__glesTracerDispatchTable.MultiDrawArraysEXT)(mode, first, count, primcount);
    }
}

GLvoid GLAPIENTRY __glesProfile_MultiDrawElementsEXT(__GLcontext *gc, GLenum mode, const GLsizei *count, GLenum type, const GLvoid*const*indices, GLsizei primcount)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glMultiDrawElementsEXT 0x%04X 0x%08X 0x%04X 0x%08X %d\n",
                        gc, tid, mode, __GL_PTR2UINT(count), type, __GL_PTR2UINT(indices), primcount);
    }

    __GLES_PROFILE_HEADER();
    __gles_MultiDrawElementsEXT(gc, mode, count, type, indices, primcount);
    __GLES_PROFILE_FOOTER(GLES3_MULTIDRAWELEMENTSEXT);

    if (__glesTracerDispatchTable.MultiDrawElementsEXT)
    {
        (*__glesTracerDispatchTable.MultiDrawElementsEXT)(mode, count, type, indices, primcount);
    }
}
#if GL_EXT_draw_elements_base_vertex
GLvoid GLAPIENTRY __glesProfile_MultiDrawElementsBaseVertexEXT(__GLcontext *gc, GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei drawcount, const GLint * basevertex)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glMultiDrawElementsBaseVertexEXT 0x%04X 0x%04X 0x%04X 0x%08X 0x%04X 0x%04X\n",
                        gc, tid, mode, count, type, indices, drawcount, basevertex);
    }

    __GLES_PROFILE_HEADER();
    __gles_MultiDrawElementsBaseVertexEXT(gc, mode, count, type, indices, drawcount, basevertex);
    __GLES_PROFILE_FOOTER(GLES31_MULTIDRAWELEMENTSBASEVERTEXEXT);

    if (__glesTracerDispatchTable.MultiDrawElementsBaseVertexEXT)
    {
        (*__glesTracerDispatchTable.MultiDrawElementsBaseVertexEXT)(mode, count, type, indices, drawcount, basevertex);
    }
}
#endif
#endif

#if GL_OES_mapbuffer
GLvoid GLAPIENTRY __glesProfile_GetBufferPointervOES(__GLcontext *gc, GLenum target, GLenum pname, GLvoid** params)
{
    __glesProfile_GetBufferPointerv(gc, target, pname, params);
}

GLvoid* GLAPIENTRY __glesProfile_MapBufferOES(__GLcontext *gc, GLenum target, GLenum access)
{
    __GLES_PROFILE_VARS();
    GLvoid *addr;

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glMapBufferOES 0x%04X 0x%04X\n",
                        gc, tid, target, access);
    }

    __GLES_PROFILE_HEADER();
    addr = __gles_MapBufferOES(gc, target, access);
    __GLES_PROFILE_FOOTER(GLES3_MAPBUFFEROES);

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_POST)
    {
        __GLES_LOG_API("        glMapBufferOES => 0x%08X\n", __GL_PTR2UINT(addr));
    }

    if (__glesTracerDispatchTable.MapBufferOES)
    {
        (*__glesTracerDispatchTable.MapBufferOES)(target, access, addr);
    }

    return addr;
}

GLboolean GLAPIENTRY __glesProfile_UnmapBufferOES(__GLcontext *gc, GLenum target)
{
    return __glesProfile_UnmapBuffer(gc, target);
}
#endif

#if GL_EXT_discard_framebuffer
GLvoid GLAPIENTRY __glesProfile_DiscardFramebufferEXT(__GLcontext *gc, GLenum target, GLsizei numAttachments, const GLenum *attachments)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glDiscardFramebufferEXT 0x%04X %d 0x%08X\n",
                        gc, tid, target, numAttachments, __GL_PTR2UINT(numAttachments));
    }

    __GLES_PROFILE_HEADER();
    __gles_DiscardFramebufferEXT(gc, target, numAttachments, attachments);
    __GLES_PROFILE_FOOTER(GLES3_DISCARDFRAMEBUFFEREXT);

    if (__glesTracerDispatchTable.DiscardFramebufferEXT)
    {
        (*__glesTracerDispatchTable.DiscardFramebufferEXT)(target, numAttachments, attachments);
    }
}
#endif

#if GL_EXT_multisampled_render_to_texture
GLvoid GLAPIENTRY __glesProfile_RenderbufferStorageMultisampleEXT(__GLcontext *gc, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
    __glesProfile_RenderbufferStorageMultisample(gc, target, samples, internalformat, width, height);
}

GLvoid GLAPIENTRY __glesProfile_FramebufferTexture2DMultisampleEXT(__GLcontext *gc, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glFramebufferTexture2DMultisampleEXT 0x%04X 0x%04X 0x%04X %d %d %d\n",
                        gc, tid, target, attachment, textarget, texture, level, samples);
    }

    __GLES_PROFILE_HEADER();
    __gles_FramebufferTexture2DMultisampleEXT(gc, target, attachment, textarget, texture, level, samples);
    __GLES_PROFILE_FOOTER(GLES3_FRAMEBUFFERTEXTURE2DMULTISAMPLEEXT);

    if (__glesTracerDispatchTable.FramebufferTexture2DMultisampleEXT)
    {
        (*__glesTracerDispatchTable.FramebufferTexture2DMultisampleEXT)(target, attachment, textarget, texture, level, samples);
    }
}
#endif

#if GL_VIV_direct_texture
GLvoid GLAPIENTRY __glesProfile_TexDirectVIV(__GLcontext *gc, GLenum target, GLsizei width, GLsizei height, GLenum format, GLvoid ** pixels)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glTexDirectVIV 0x%04X %d %d 0x%04X 0x%08X\n",
                        gc, tid, target, width, height, format, __GL_PTR2UINT(pixels));
    }

    __GLES_PROFILE_HEADER();
    __gles_TexDirectVIV(gc, target, width, height, format, pixels);
    __GLES_PROFILE_FOOTER(GLES3_TEXDIRECTVIV);

    if (__glesTracerDispatchTable.TexDirectVIV)
    {
        (*__glesTracerDispatchTable.TexDirectVIV)(target, width, height, format, pixels);
    }
}

GLvoid GLAPIENTRY __glesProfile_TexDirectInvalidateVIV(__GLcontext *gc, GLenum target)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glTexDirectInvalidateVIV 0x%04X\n",
                        gc, tid, target);
    }

    __GLES_PROFILE_HEADER();
    __gles_TexDirectInvalidateVIV(gc, target);
    __GLES_PROFILE_FOOTER(GLES3_TEXDIRECTINVALIDATEVIV);

    if (__glesTracerDispatchTable.TexDirectInvalidateVIV)
    {
        (*__glesTracerDispatchTable.TexDirectInvalidateVIV)(target);
    }
}

GLvoid GLAPIENTRY __glesProfile_TexDirectVIVMap(__GLcontext *gc, GLenum target, GLsizei width, GLsizei height, GLenum format, GLvoid ** logical, const GLuint * physical)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glTexDirectVIVMap 0x%04X %d %d 0x%04X 0x%08X 0x%08X\n",
                        gc, tid, target, width, height, format, __GL_PTR2UINT(logical), __GL_PTR2UINT(physical));
    }

    __GLES_PROFILE_HEADER();
    __gles_TexDirectVIVMap(gc, target, width, height, format, logical, physical);
    __GLES_PROFILE_FOOTER(GLES3_TEXDIRECTVIVMAP);

    if (__glesTracerDispatchTable.TexDirectVIVMap)
    {
        (*__glesTracerDispatchTable.TexDirectVIVMap)(target, width, height, format, logical, physical);
    }
}

GLvoid GLAPIENTRY __glesProfile_TexDirectTiledMapVIV(__GLcontext *gc, GLenum target, GLsizei width, GLsizei height, GLenum format, GLvoid ** logical, const GLuint * physical)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glTexDirectTiledMapVIV 0x%04X %d %d 0x%04X 0x%08X 0x%08X\n",
                        gc, tid, target, width, height, format, __GL_PTR2UINT(logical), __GL_PTR2UINT(physical));
    }

    __GLES_PROFILE_HEADER();
    __gles_TexDirectTiledMapVIV(gc, target, width, height, format, logical, physical);
    __GLES_PROFILE_FOOTER(GLES3_TEXDIRECTTILEDMAPVIV);

    if (__glesTracerDispatchTable.TexDirectTiledMapVIV)
    {
        (*__glesTracerDispatchTable.TexDirectTiledMapVIV)(target, width, height, format, logical, physical);
    }
}
#endif

#if GL_EXT_multi_draw_indirect
GLvoid GLAPIENTRY __glesProfile_MultiDrawArraysIndirectEXT(__GLcontext *gc, GLenum mode, const void *indirect, GLsizei drawcount, GLsizei stride)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glMultiDrawArraysIndirectEXT 0x%04X 0x%08X 0x%04X 0x%04X\n",
                        gc, tid, mode, indirect, drawcount, stride);
    }

    __GLES_PROFILE_HEADER();
    __gles_MultiDrawArraysIndirectEXT(gc, mode, indirect, drawcount, stride);
    __GLES_PROFILE_FOOTER(GLES31_MULTIDRAWARRAYSINDIRECTEXT);

    if (__glesTracerDispatchTable.MultiDrawArraysIndirectEXT)
    {
        (*__glesTracerDispatchTable.MultiDrawArraysIndirectEXT)(mode, indirect, drawcount, stride);
    }
}

GLvoid GLAPIENTRY __glesProfile_MultiDrawElementsIndirectEXT(__GLcontext *gc, GLenum mode, GLenum type, const void *indirect, GLsizei drawcount, GLsizei stride)
{
    __GLES_PROFILE_VARS();

    if (__glesApiTraceMode == gcvTRACEMODE_FULL || __glesApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GLES_LOG_API("(gc=%p, tid=%p): glMultiDrawElementsIndirectEXT 0x%04X 0x%04X 0x%08X 0x%04X 0x%04X\n",
            gc, tid, mode, type, indirect, drawcount, stride);
    }

    __GLES_PROFILE_HEADER();
    __gles_MultiDrawElementsIndirectEXT(gc, mode, type, indirect, drawcount, stride);
    __GLES_PROFILE_FOOTER(GLES31_MULTIDRAWELEMENTSINDIRECTEXT);

    if (__glesTracerDispatchTable.MultiDrawElementsIndirectEXT)
    {
        (*__glesTracerDispatchTable.MultiDrawElementsIndirectEXT)(mode, type, indirect, drawcount, stride);
    }
}
#endif



/* GLES API Profiler function dispatch table */

#define __glesProfile(func) __glesProfile_##func

/* add GL api */
GLboolean GLAPIENTRY __glesProfile(IsList)(__GLcontext *gc,   GLuint list ) { return GL_FALSE;}
GLvoid GLAPIENTRY __glesProfile(DeleteLists)(__GLcontext *gc,   GLuint list, GLsizei range ) { }
GLuint GLAPIENTRY __glesProfile(GenLists)(__GLcontext *gc,   GLsizei range ) { return 0; }
GLvoid GLAPIENTRY __glesProfile(NewList)(__GLcontext *gc,   GLuint list, GLenum mode ) { }
GLvoid GLAPIENTRY __glesProfile(EndList)(__GLcontext *gc) { }
GLvoid GLAPIENTRY __glesProfile(CallList)(__GLcontext *gc,   GLuint list ) { }
GLvoid GLAPIENTRY __glesProfile(CallLists)(__GLcontext *gc,   GLsizei n, GLenum type, const GLvoid *lists ) { }
GLvoid GLAPIENTRY __glesProfile(ListBase)(__GLcontext *gc,   GLuint base ) { }
GLvoid GLAPIENTRY __glesProfile(Begin)(__GLcontext *gc,   GLenum mode ) {}
GLvoid GLAPIENTRY __glesProfile(Bitmap)(__GLcontext *gc,   GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap ) {}
GLvoid GLAPIENTRY __glesProfile(Color3b)(__GLcontext *gc,   GLbyte red, GLbyte green, GLbyte blue ){}
GLvoid GLAPIENTRY __glesProfile(Color3d)(__GLcontext *gc,   GLdouble red, GLdouble green, GLdouble blue ){}
GLvoid GLAPIENTRY __glesProfile(Color3f)(__GLcontext *gc,   GLfloat red, GLfloat green, GLfloat blue ){}
GLvoid GLAPIENTRY __glesProfile(Color3i)(__GLcontext *gc,   GLint red, GLint green, GLint blue ){}
GLvoid GLAPIENTRY __glesProfile(Color3s)(__GLcontext *gc,   GLshort red, GLshort green, GLshort blue ){}
GLvoid GLAPIENTRY __glesProfile(Color3ub)(__GLcontext *gc,   GLubyte red, GLubyte green, GLubyte blue ){}
GLvoid GLAPIENTRY __glesProfile(Color3ui)(__GLcontext *gc,   GLuint red, GLuint green, GLuint blue ){}
GLvoid GLAPIENTRY __glesProfile(Color3us)(__GLcontext *gc,   GLushort red, GLushort green, GLushort blue ){}
GLvoid GLAPIENTRY __glesProfile(Color3bv)(__GLcontext *gc,   const GLbyte *v ){}
GLvoid GLAPIENTRY __glesProfile(Color3dv)(__GLcontext *gc,   const GLdouble *v ){}
GLvoid GLAPIENTRY __glesProfile(Color3fv)(__GLcontext *gc,   const GLfloat *v ){}
GLvoid GLAPIENTRY __glesProfile(Color3iv)(__GLcontext *gc,   const GLint *v ){}
GLvoid GLAPIENTRY __glesProfile(Color3sv)(__GLcontext *gc,   const GLshort *v ){}
GLvoid GLAPIENTRY __glesProfile(Color3ubv)(__GLcontext *gc,   const GLubyte *v ){}
GLvoid GLAPIENTRY __glesProfile(Color3uiv)(__GLcontext *gc,   const GLuint *v ){}
GLvoid GLAPIENTRY __glesProfile(Color3usv)(__GLcontext *gc,   const GLushort *v ){}
GLvoid GLAPIENTRY __glesProfile(Color4b)(__GLcontext *gc,   GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha ){}
GLvoid GLAPIENTRY __glesProfile(Color4d)(__GLcontext *gc,   GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha ){}
GLvoid GLAPIENTRY __glesProfile(Color4f)(__GLcontext *gc,   GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha ){}
GLvoid GLAPIENTRY __glesProfile(Color4i)(__GLcontext *gc,   GLint red, GLint green, GLint blue, GLint alpha ){}
GLvoid GLAPIENTRY __glesProfile(Color4s)(__GLcontext *gc,   GLshort red, GLshort green, GLshort blue, GLshort alpha ){}
GLvoid GLAPIENTRY __glesProfile(Color4ub)(__GLcontext *gc,   GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha ){}
GLvoid GLAPIENTRY __glesProfile(Color4ui)(__GLcontext *gc,   GLuint red, GLuint green, GLuint blue, GLuint alpha ){}
GLvoid GLAPIENTRY __glesProfile(Color4us)(__GLcontext *gc,   GLushort red, GLushort green, GLushort blue, GLushort alpha ){}
GLvoid GLAPIENTRY __glesProfile(Color4bv)(__GLcontext *gc,   const GLbyte *v ){}
GLvoid GLAPIENTRY __glesProfile(Color4dv)(__GLcontext *gc,   const GLdouble *v ){}
GLvoid GLAPIENTRY __glesProfile(Color4fv)(__GLcontext *gc,   const GLfloat *v ){}
GLvoid GLAPIENTRY __glesProfile(Color4iv)(__GLcontext *gc,   const GLint *v ){}
GLvoid GLAPIENTRY __glesProfile(Color4sv)(__GLcontext *gc,   const GLshort *v ){}
GLvoid GLAPIENTRY __glesProfile(Color4ubv)(__GLcontext *gc,   const GLubyte *v ){}
GLvoid GLAPIENTRY __glesProfile(Color4uiv)(__GLcontext *gc,   const GLuint *v ){}
GLvoid GLAPIENTRY __glesProfile(Color4usv)(__GLcontext *gc,   const GLushort *v ){}
GLvoid GLAPIENTRY __glesProfile(EdgeFlag)(__GLcontext *gc,   GLboolean flag ){}
GLvoid GLAPIENTRY __glesProfile(EdgeFlagv)(__GLcontext *gc,   const GLboolean *flag ){}
GLvoid GLAPIENTRY __glesProfile(End)(__GLcontext *gc){}
GLvoid GLAPIENTRY __glesProfile(Indexd)(__GLcontext *gc,   GLdouble c ){}
GLvoid GLAPIENTRY __glesProfile(Indexf)(__GLcontext *gc,   GLfloat c ){}
GLvoid GLAPIENTRY __glesProfile(Indexi)(__GLcontext *gc,   GLint c ){}
GLvoid GLAPIENTRY __glesProfile(Indexs)(__GLcontext *gc,   GLshort c ){}
GLvoid GLAPIENTRY __glesProfile(Indexdv)(__GLcontext *gc,   const GLdouble *c ){}
GLvoid GLAPIENTRY __glesProfile(Indexfv)(__GLcontext *gc,   const GLfloat *c ){}
GLvoid GLAPIENTRY __glesProfile(Indexiv)(__GLcontext *gc,   const GLint *c ){}
GLvoid GLAPIENTRY __glesProfile(Indexsv)(__GLcontext *gc,   const GLshort *c ){}
GLvoid GLAPIENTRY __glesProfile(Normal3b)(__GLcontext *gc,   GLbyte nx, GLbyte ny, GLbyte nz ){}
GLvoid GLAPIENTRY __glesProfile(Normal3d)(__GLcontext *gc,   GLdouble nx, GLdouble ny, GLdouble nz ){}
GLvoid GLAPIENTRY __glesProfile(Normal3f)(__GLcontext *gc,   GLfloat nx, GLfloat ny, GLfloat nz ){}
GLvoid GLAPIENTRY __glesProfile(Normal3i)(__GLcontext *gc,   GLint nx, GLint ny, GLint nz ){}
GLvoid GLAPIENTRY __glesProfile(Normal3s)(__GLcontext *gc,   GLshort nx, GLshort ny, GLshort nz ){}
GLvoid GLAPIENTRY __glesProfile(Normal3bv)(__GLcontext *gc,   const GLbyte *v ){}
GLvoid GLAPIENTRY __glesProfile(Normal3dv)(__GLcontext *gc,   const GLdouble *v ){}
GLvoid GLAPIENTRY __glesProfile(Normal3fv)(__GLcontext *gc,   const GLfloat *v ){}
GLvoid GLAPIENTRY __glesProfile(Normal3iv)(__GLcontext *gc,   const GLint *v ){}
GLvoid GLAPIENTRY __glesProfile(Normal3sv)(__GLcontext *gc,   const GLshort *v ){}
GLvoid GLAPIENTRY __glesProfile(RasterPos2d)(__GLcontext *gc,   GLdouble x, GLdouble y ){}
GLvoid GLAPIENTRY __glesProfile(RasterPos2f)(__GLcontext *gc,   GLfloat x, GLfloat y ){}
GLvoid GLAPIENTRY __glesProfile(RasterPos2i)(__GLcontext *gc,   GLint x, GLint y ){}
GLvoid GLAPIENTRY __glesProfile(RasterPos2s)(__GLcontext *gc,   GLshort x, GLshort y ){}
GLvoid GLAPIENTRY __glesProfile(RasterPos3d)(__GLcontext *gc,   GLdouble x, GLdouble y, GLdouble z ){}
GLvoid GLAPIENTRY __glesProfile(RasterPos3f)(__GLcontext *gc,   GLfloat x, GLfloat y, GLfloat z ){}
GLvoid GLAPIENTRY __glesProfile(RasterPos3i)(__GLcontext *gc,   GLint x, GLint y, GLint z ){}
GLvoid GLAPIENTRY __glesProfile(RasterPos3s)(__GLcontext *gc,   GLshort x, GLshort y, GLshort z ){}
GLvoid GLAPIENTRY __glesProfile(RasterPos4d)(__GLcontext *gc,   GLdouble x, GLdouble y, GLdouble z, GLdouble w ){}
GLvoid GLAPIENTRY __glesProfile(RasterPos4f)(__GLcontext *gc,   GLfloat x, GLfloat y, GLfloat z, GLfloat w ){}
GLvoid GLAPIENTRY __glesProfile(RasterPos4i)(__GLcontext *gc,   GLint x, GLint y, GLint z, GLint w ){}
GLvoid GLAPIENTRY __glesProfile(RasterPos4s)(__GLcontext *gc,   GLshort x, GLshort y, GLshort z, GLshort w ){}
GLvoid GLAPIENTRY __glesProfile(RasterPos2dv)(__GLcontext *gc,   const GLdouble *v ){}
GLvoid GLAPIENTRY __glesProfile(RasterPos2fv)(__GLcontext *gc,   const GLfloat *v ){}
GLvoid GLAPIENTRY __glesProfile(RasterPos2iv)(__GLcontext *gc,   const GLint *v ){}
GLvoid GLAPIENTRY __glesProfile(RasterPos2sv)(__GLcontext *gc,   const GLshort *v ){}
GLvoid GLAPIENTRY __glesProfile(RasterPos3dv)(__GLcontext *gc,   const GLdouble *v ){}
GLvoid GLAPIENTRY __glesProfile(RasterPos3fv)(__GLcontext *gc,   const GLfloat *v ){}
GLvoid GLAPIENTRY __glesProfile(RasterPos3iv)(__GLcontext *gc,   const GLint *v ){}
GLvoid GLAPIENTRY __glesProfile(RasterPos3sv)(__GLcontext *gc,   const GLshort *v ){}
GLvoid GLAPIENTRY __glesProfile(RasterPos4dv)(__GLcontext *gc,   const GLdouble *v ){}
GLvoid GLAPIENTRY __glesProfile(RasterPos4fv)(__GLcontext *gc,   const GLfloat *v ){}
GLvoid GLAPIENTRY __glesProfile(RasterPos4iv)(__GLcontext *gc,   const GLint *v ){}
GLvoid GLAPIENTRY __glesProfile(RasterPos4sv)(__GLcontext *gc,   const GLshort *v ){}
GLvoid GLAPIENTRY __glesProfile(Rectd)(__GLcontext *gc,   GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2 ){}
GLvoid GLAPIENTRY __glesProfile(Rectf)(__GLcontext *gc,   GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2 ){}
GLvoid GLAPIENTRY __glesProfile(Recti)(__GLcontext *gc,   GLint x1, GLint y1, GLint x2, GLint y2 ){}
GLvoid GLAPIENTRY __glesProfile(Rects)(__GLcontext *gc,   GLshort x1, GLshort y1, GLshort x2, GLshort y2 ){}
GLvoid GLAPIENTRY __glesProfile(Rectdv)(__GLcontext *gc,   const GLdouble *v1, const GLdouble *v2 ){}
GLvoid GLAPIENTRY __glesProfile(Rectfv)(__GLcontext *gc,   const GLfloat *v1, const GLfloat *v2 ){}
GLvoid GLAPIENTRY __glesProfile(Rectiv)(__GLcontext *gc,   const GLint *v1, const GLint *v2 ){}
GLvoid GLAPIENTRY __glesProfile(Rectsv)(__GLcontext *gc,   const GLshort *v1, const GLshort *v2 ){}
GLvoid GLAPIENTRY __glesProfile(TexCoord1d)(__GLcontext *gc,   GLdouble s ){}
GLvoid GLAPIENTRY __glesProfile(TexCoord1f)(__GLcontext *gc,   GLfloat s ){}
GLvoid GLAPIENTRY __glesProfile(TexCoord1i)(__GLcontext *gc,   GLint s ){}
GLvoid GLAPIENTRY __glesProfile(TexCoord1s)(__GLcontext *gc,   GLshort s ){}
GLvoid GLAPIENTRY __glesProfile(TexCoord2d)(__GLcontext *gc,   GLdouble s, GLdouble t ){}
GLvoid GLAPIENTRY __glesProfile(TexCoord2f)(__GLcontext *gc,   GLfloat s, GLfloat t ){}
GLvoid GLAPIENTRY __glesProfile(TexCoord2i)(__GLcontext *gc,   GLint s, GLint t ){}
GLvoid GLAPIENTRY __glesProfile(TexCoord2s)(__GLcontext *gc,   GLshort s, GLshort t ){}
GLvoid GLAPIENTRY __glesProfile(TexCoord3d)(__GLcontext *gc,   GLdouble s, GLdouble t, GLdouble r ){}
GLvoid GLAPIENTRY __glesProfile(TexCoord3f)(__GLcontext *gc,   GLfloat s, GLfloat t, GLfloat r ){}
GLvoid GLAPIENTRY __glesProfile(TexCoord3i)(__GLcontext *gc,   GLint s, GLint t, GLint r ){}
GLvoid GLAPIENTRY __glesProfile(TexCoord3s)(__GLcontext *gc,   GLshort s, GLshort t, GLshort r ){}
GLvoid GLAPIENTRY __glesProfile(TexCoord4d)(__GLcontext *gc,   GLdouble s, GLdouble t, GLdouble r, GLdouble q ){}
GLvoid GLAPIENTRY __glesProfile(TexCoord4f)(__GLcontext *gc,   GLfloat s, GLfloat t, GLfloat r, GLfloat q ){}
GLvoid GLAPIENTRY __glesProfile(TexCoord4i)(__GLcontext *gc,   GLint s, GLint t, GLint r, GLint q ){}
GLvoid GLAPIENTRY __glesProfile(TexCoord4s)(__GLcontext *gc,   GLshort s, GLshort t, GLshort r, GLshort q ){}
GLvoid GLAPIENTRY __glesProfile(TexCoord1dv)(__GLcontext *gc,   const GLdouble *v ){}
GLvoid GLAPIENTRY __glesProfile(TexCoord1fv)(__GLcontext *gc,   const GLfloat *v ){}
GLvoid GLAPIENTRY __glesProfile(TexCoord1iv)(__GLcontext *gc,   const GLint *v ){}
GLvoid GLAPIENTRY __glesProfile(TexCoord1sv)(__GLcontext *gc,   const GLshort *v ){}
GLvoid GLAPIENTRY __glesProfile(TexCoord2dv)(__GLcontext *gc,   const GLdouble *v ){}
GLvoid GLAPIENTRY __glesProfile(TexCoord2fv)(__GLcontext *gc,   const GLfloat *v ){}
GLvoid GLAPIENTRY __glesProfile(TexCoord2iv)(__GLcontext *gc,   const GLint *v ){}
GLvoid GLAPIENTRY __glesProfile(TexCoord2sv)(__GLcontext *gc,   const GLshort *v ){}
GLvoid GLAPIENTRY __glesProfile(TexCoord3dv)(__GLcontext *gc,   const GLdouble *v ){}
GLvoid GLAPIENTRY __glesProfile(TexCoord3fv)(__GLcontext *gc,   const GLfloat *v ){}
GLvoid GLAPIENTRY __glesProfile(TexCoord3iv)(__GLcontext *gc,   const GLint *v ){}
GLvoid GLAPIENTRY __glesProfile(TexCoord3sv)(__GLcontext *gc,   const GLshort *v ){}
GLvoid GLAPIENTRY __glesProfile(TexCoord4dv)(__GLcontext *gc,   const GLdouble *v ){}
GLvoid GLAPIENTRY __glesProfile(TexCoord4fv)(__GLcontext *gc,   const GLfloat *v ){}
GLvoid GLAPIENTRY __glesProfile(TexCoord4iv)(__GLcontext *gc,   const GLint *v ){}
GLvoid GLAPIENTRY __glesProfile(TexCoord4sv)(__GLcontext *gc,   const GLshort *v ){}
GLvoid GLAPIENTRY __glesProfile(Vertex2d)(__GLcontext *gc,   GLdouble x, GLdouble y ){}
GLvoid GLAPIENTRY __glesProfile(Vertex2f)(__GLcontext *gc,   GLfloat x, GLfloat y ){}
GLvoid GLAPIENTRY __glesProfile(Vertex2i)(__GLcontext *gc,   GLint x, GLint y ){}
GLvoid GLAPIENTRY __glesProfile(Vertex2s)(__GLcontext *gc,   GLshort x, GLshort y ){}
GLvoid GLAPIENTRY __glesProfile(Vertex3d)(__GLcontext *gc,   GLdouble x, GLdouble y, GLdouble z ){}
GLvoid GLAPIENTRY __glesProfile(Vertex3f)(__GLcontext *gc,   GLfloat x, GLfloat y, GLfloat z ){}
GLvoid GLAPIENTRY __glesProfile(Vertex3i)(__GLcontext *gc,   GLint x, GLint y, GLint z ){}
GLvoid GLAPIENTRY __glesProfile(Vertex3s)(__GLcontext *gc,   GLshort x, GLshort y, GLshort z ){}
GLvoid GLAPIENTRY __glesProfile(Vertex4d)(__GLcontext *gc,   GLdouble x, GLdouble y, GLdouble z, GLdouble w ){}
GLvoid GLAPIENTRY __glesProfile(Vertex4f)(__GLcontext *gc,   GLfloat x, GLfloat y, GLfloat z, GLfloat w ){}
GLvoid GLAPIENTRY __glesProfile(Vertex4i)(__GLcontext *gc,   GLint x, GLint y, GLint z, GLint w ){}
GLvoid GLAPIENTRY __glesProfile(Vertex4s)(__GLcontext *gc,   GLshort x, GLshort y, GLshort z, GLshort w ){}
GLvoid GLAPIENTRY __glesProfile(Vertex2dv)(__GLcontext *gc,   const GLdouble *v ){}
GLvoid GLAPIENTRY __glesProfile(Vertex2fv)(__GLcontext *gc,   const GLfloat *v ){}
GLvoid GLAPIENTRY __glesProfile(Vertex2iv)(__GLcontext *gc,   const GLint *v ){}
GLvoid GLAPIENTRY __glesProfile(Vertex2sv)(__GLcontext *gc,   const GLshort *v ){}
GLvoid GLAPIENTRY __glesProfile(Vertex3dv)(__GLcontext *gc,   const GLdouble *v ){}
GLvoid GLAPIENTRY __glesProfile(Vertex3fv)(__GLcontext *gc,   const GLfloat *v ){}
GLvoid GLAPIENTRY __glesProfile(Vertex3iv)(__GLcontext *gc,   const GLint *v ){}
GLvoid GLAPIENTRY __glesProfile(Vertex3sv)(__GLcontext *gc,   const GLshort *v ){}
GLvoid GLAPIENTRY __glesProfile(Vertex4dv)(__GLcontext *gc,   const GLdouble *v ){}
GLvoid GLAPIENTRY __glesProfile(Vertex4fv)(__GLcontext *gc,   const GLfloat *v ){}
GLvoid GLAPIENTRY __glesProfile(Vertex4iv)(__GLcontext *gc,   const GLint *v ){}
GLvoid GLAPIENTRY __glesProfile(Vertex4sv)(__GLcontext *gc,   const GLshort *v ){}
GLvoid GLAPIENTRY __glesProfile(ClipPlane)(__GLcontext *gc,   GLenum plane, const GLdouble *equation ){}
GLvoid GLAPIENTRY __glesProfile(ColorMaterial)(__GLcontext *gc,   GLenum face, GLenum mode ){}
GLvoid GLAPIENTRY __glesProfile(Fogf)(__GLcontext *gc,   GLenum pname, GLfloat param ){}
GLvoid GLAPIENTRY __glesProfile(Fogi)(__GLcontext *gc,   GLenum pname, GLint param ){}
GLvoid GLAPIENTRY __glesProfile(Fogfv)(__GLcontext *gc,   GLenum pname, const GLfloat *params ){}
GLvoid GLAPIENTRY __glesProfile(Fogiv)(__GLcontext *gc,   GLenum pname, const GLint *params ){}
GLvoid GLAPIENTRY __glesProfile(Lightf)(__GLcontext *gc,   GLenum light, GLenum pname, GLfloat param ){}
GLvoid GLAPIENTRY __glesProfile(Lighti)(__GLcontext *gc,   GLenum light, GLenum pname, GLint param ){}
GLvoid GLAPIENTRY __glesProfile(Lightfv)(__GLcontext *gc,   GLenum light, GLenum pname, const GLfloat *params ){}
GLvoid GLAPIENTRY __glesProfile(Lightiv)(__GLcontext *gc,   GLenum light, GLenum pname, const GLint *params ){}
GLvoid GLAPIENTRY __glesProfile(LightModelf)(__GLcontext *gc,   GLenum pname, GLfloat param ){}
GLvoid GLAPIENTRY __glesProfile(LightModeli)(__GLcontext *gc,   GLenum pname, GLint param ){}
GLvoid GLAPIENTRY __glesProfile(LightModelfv)(__GLcontext *gc,   GLenum pname, const GLfloat *params ){}
GLvoid GLAPIENTRY __glesProfile(LightModeliv)(__GLcontext *gc,   GLenum pname, const GLint *params ){}
GLvoid GLAPIENTRY __glesProfile(LineStipple)(__GLcontext *gc,   GLint factor, GLushort pattern ){}
GLvoid GLAPIENTRY __glesProfile(Materialf)(__GLcontext *gc,   GLenum face, GLenum pname, GLfloat param ){}
GLvoid GLAPIENTRY __glesProfile(Materiali)(__GLcontext *gc,   GLenum face, GLenum pname, GLint param ){}
GLvoid GLAPIENTRY __glesProfile(Materialfv)(__GLcontext *gc,   GLenum face, GLenum pname, const GLfloat *params ){}
GLvoid GLAPIENTRY __glesProfile(Materialiv)(__GLcontext *gc,   GLenum face, GLenum pname, const GLint *params ){}
GLvoid GLAPIENTRY __glesProfile(PointSize)(__GLcontext *gc,   GLfloat size ){}
GLvoid GLAPIENTRY __glesProfile(PolygonMode)(__GLcontext *gc,   GLenum face, GLenum mode ){}
GLvoid GLAPIENTRY __glesProfile(PolygonStipple)(__GLcontext *gc,   const GLubyte *mask ){}
GLvoid GLAPIENTRY __glesProfile(ShadeModel)(__GLcontext *gc,   GLenum mode ){}
GLvoid GLAPIENTRY __glesProfile(TexImage1D)(__GLcontext *gc,   GLenum target, GLint level, GLint internalFormat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels ){}
GLvoid GLAPIENTRY __glesProfile(TexEnvf)(__GLcontext *gc,   GLenum target, GLenum pname, GLfloat param ){}
GLvoid GLAPIENTRY __glesProfile(TexEnvi)(__GLcontext *gc,   GLenum target, GLenum pname, GLint param ){}
GLvoid GLAPIENTRY __glesProfile(TexEnvfv)(__GLcontext *gc,   GLenum target, GLenum pname, const GLfloat *params ){}
GLvoid GLAPIENTRY __glesProfile(TexEnviv)(__GLcontext *gc,   GLenum target, GLenum pname, const GLint *params ){}
GLvoid GLAPIENTRY __glesProfile(TexGend)(__GLcontext *gc,   GLenum coord, GLenum pname, GLdouble param ){}
GLvoid GLAPIENTRY __glesProfile(TexGenf)(__GLcontext *gc,   GLenum coord, GLenum pname, GLfloat param ){}
GLvoid GLAPIENTRY __glesProfile(TexGeni)(__GLcontext *gc,   GLenum coord, GLenum pname, GLint param ){}
GLvoid GLAPIENTRY __glesProfile(TexGendv)(__GLcontext *gc,   GLenum coord, GLenum pname, const GLdouble *params ){}
GLvoid GLAPIENTRY __glesProfile(TexGenfv)(__GLcontext *gc,   GLenum coord, GLenum pname, const GLfloat *params ){}
GLvoid GLAPIENTRY __glesProfile(TexGeniv)(__GLcontext *gc,   GLenum coord, GLenum pname, const GLint *params ){}
GLvoid GLAPIENTRY __glesProfile(FeedbackBuffer)(__GLcontext *gc,   GLsizei size, GLenum type, GLfloat *buffer ){}
GLvoid GLAPIENTRY __glesProfile(SelectBuffer)(__GLcontext *gc,   GLsizei size, GLuint *buffer ){}
GLint GLAPIENTRY __glesProfile(RenderMode)(__GLcontext *gc,   GLenum mode ) { return 0;}
GLvoid GLAPIENTRY __glesProfile(InitNames)(__GLcontext *gc){}
GLvoid GLAPIENTRY __glesProfile(LoadName)(__GLcontext *gc,   GLuint name ){}
GLvoid GLAPIENTRY __glesProfile(PushName)(__GLcontext *gc,   GLuint name ){}
GLvoid GLAPIENTRY __glesProfile(PopName)(__GLcontext *gc){}
GLvoid GLAPIENTRY __glesProfile(PassThrough)(__GLcontext *gc,   GLfloat token ){}
GLvoid GLAPIENTRY __glesProfile(DrawBuffer)(__GLcontext *gc,   GLenum mode ){}
GLvoid GLAPIENTRY __glesProfile(ClearAccum)(__GLcontext *gc,   GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha ){}
GLvoid GLAPIENTRY __glesProfile(ClearIndex)(__GLcontext *gc,   GLfloat c ){}
GLvoid GLAPIENTRY __glesProfile(ClearDepth)(__GLcontext *gc,   GLclampd depth ){}
GLvoid GLAPIENTRY __glesProfile(IndexMask)(__GLcontext *gc,   GLuint mask ){}
GLvoid GLAPIENTRY __glesProfile(Accum)(__GLcontext *gc,   GLenum op, GLfloat value ){}
GLvoid GLAPIENTRY __glesProfile(PushAttrib)(__GLcontext *gc,   GLbitfield mask ){}
GLvoid GLAPIENTRY __glesProfile(PopAttrib)(__GLcontext *gc){}
GLvoid GLAPIENTRY __glesProfile(Map1d)(__GLcontext *gc,   GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points ){}
GLvoid GLAPIENTRY __glesProfile(Map1f)(__GLcontext *gc,   GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points ){}
GLvoid GLAPIENTRY __glesProfile(Map2d)(__GLcontext *gc,   GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points ){}
GLvoid GLAPIENTRY __glesProfile(Map2f)(__GLcontext *gc,   GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points ){}
GLvoid GLAPIENTRY __glesProfile(MapGrid1d)(__GLcontext *gc,   GLint un, GLdouble u1, GLdouble u2 ){}
GLvoid GLAPIENTRY __glesProfile(MapGrid1f)(__GLcontext *gc,   GLint un, GLfloat u1, GLfloat u2 ){}
GLvoid GLAPIENTRY __glesProfile(MapGrid2d)(__GLcontext *gc,   GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2 ){}
GLvoid GLAPIENTRY __glesProfile(MapGrid2f)(__GLcontext *gc,   GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2 ){}
GLvoid GLAPIENTRY __glesProfile(EvalCoord1d)(__GLcontext *gc,   GLdouble u ){}
GLvoid GLAPIENTRY __glesProfile(EvalCoord1f)(__GLcontext *gc,   GLfloat u ){}
GLvoid GLAPIENTRY __glesProfile(EvalCoord1dv)(__GLcontext *gc,   const GLdouble *u ){}
GLvoid GLAPIENTRY __glesProfile(EvalCoord1fv)(__GLcontext *gc,   const GLfloat *u ){}
GLvoid GLAPIENTRY __glesProfile(EvalCoord2d)(__GLcontext *gc,   GLdouble u, GLdouble v ){}
GLvoid GLAPIENTRY __glesProfile(EvalCoord2f)(__GLcontext *gc,   GLfloat u, GLfloat v ){}
GLvoid GLAPIENTRY __glesProfile(EvalCoord2dv)(__GLcontext *gc,   const GLdouble *u ){}
GLvoid GLAPIENTRY __glesProfile(EvalCoord2fv)(__GLcontext *gc,   const GLfloat *u ){}
GLvoid GLAPIENTRY __glesProfile(EvalPoint1)(__GLcontext *gc,   GLint i ){}
GLvoid GLAPIENTRY __glesProfile(EvalPoint2)(__GLcontext *gc,   GLint i, GLint j ){}
GLvoid GLAPIENTRY __glesProfile(EvalMesh1)(__GLcontext *gc,   GLenum mode, GLint i1, GLint i2 ){}
GLvoid GLAPIENTRY __glesProfile(EvalMesh2)(__GLcontext *gc,   GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2 ){}
GLvoid GLAPIENTRY __glesProfile(AlphaFunc)(__GLcontext *gc,   GLenum func, GLclampf ref ){}
GLvoid GLAPIENTRY __glesProfile(LogicOp)(__GLcontext *gc,   GLenum opcode ){}
GLvoid GLAPIENTRY __glesProfile(PixelZoom)(__GLcontext *gc,   GLfloat xfactor, GLfloat yfactor ){}
GLvoid GLAPIENTRY __glesProfile(PixelTransferf)(__GLcontext *gc,   GLenum pname, GLfloat param ){}
GLvoid GLAPIENTRY __glesProfile(PixelTransferi)(__GLcontext *gc,   GLenum pname, GLint param ){}
GLvoid GLAPIENTRY __glesProfile(PixelStoref)(__GLcontext *gc,   GLenum pname, GLfloat param ){}
GLvoid GLAPIENTRY __glesProfile(PixelMapfv)(__GLcontext *gc,   GLenum map, GLsizei mapsize, const GLfloat *values ){}
GLvoid GLAPIENTRY __glesProfile(PixelMapuiv)(__GLcontext *gc,   GLenum map, GLsizei mapsize, const GLuint *values ){}
GLvoid GLAPIENTRY __glesProfile(PixelMapusv)(__GLcontext *gc,   GLenum map, GLsizei mapsize, const GLushort *values ){}
GLvoid GLAPIENTRY __glesProfile(GetClipPlane)(__GLcontext *gc,   GLenum plane, GLdouble *equation ){}
GLvoid GLAPIENTRY __glesProfile(GetDoublev)(__GLcontext *gc,   GLenum pname, GLdouble *params ){}
GLvoid GLAPIENTRY __glesProfile(GetLightfv)(__GLcontext *gc,   GLenum light, GLenum pname, GLfloat *params ){}
GLvoid GLAPIENTRY __glesProfile(GetLightiv)(__GLcontext *gc,   GLenum light, GLenum pname, GLint *params ){}
GLvoid GLAPIENTRY __glesProfile(GetMapdv)(__GLcontext *gc,   GLenum target, GLenum query, GLdouble *v ){}
GLvoid GLAPIENTRY __glesProfile(GetMapfv)(__GLcontext *gc,   GLenum target, GLenum query, GLfloat *v ){}
GLvoid GLAPIENTRY __glesProfile(GetMapiv)(__GLcontext *gc,   GLenum target, GLenum query, GLint *v ){}
GLvoid GLAPIENTRY __glesProfile(GetMaterialfv)(__GLcontext *gc,   GLenum face, GLenum pname, GLfloat *params ){}
GLvoid GLAPIENTRY __glesProfile(GetMaterialiv)(__GLcontext *gc,   GLenum face, GLenum pname, GLint *params ){}
GLvoid GLAPIENTRY __glesProfile(GetPixelMapfv)(__GLcontext *gc,   GLenum map, GLfloat *values ){}
GLvoid GLAPIENTRY __glesProfile(GetPixelMapuiv)(__GLcontext *gc,   GLenum map, GLuint *values ){}
GLvoid GLAPIENTRY __glesProfile(GetPixelMapusv)(__GLcontext *gc,   GLenum map, GLushort *values ){}
GLvoid GLAPIENTRY __glesProfile(GetPolygonStipple)(__GLcontext *gc,   GLubyte *mask ){}
GLvoid GLAPIENTRY __glesProfile(GetTexEnvfv)(__GLcontext *gc,   GLenum target, GLenum pname, GLfloat *params ){}
GLvoid GLAPIENTRY __glesProfile(GetTexEnviv)(__GLcontext *gc,   GLenum target, GLenum pname, GLint *params ){}
GLvoid GLAPIENTRY __glesProfile(GetTexGendv)(__GLcontext *gc,   GLenum coord, GLenum pname, GLdouble *params ){}
GLvoid GLAPIENTRY __glesProfile(GetTexGenfv)(__GLcontext *gc,   GLenum coord, GLenum pname, GLfloat *params ){}
GLvoid GLAPIENTRY __glesProfile(GetTexGeniv)(__GLcontext *gc,   GLenum coord, GLenum pname, GLint *params ){}
GLvoid GLAPIENTRY __glesProfile(DepthRange)(__GLcontext *gc,   GLclampd near_val, GLclampd far_val ){}
GLvoid GLAPIENTRY __glesProfile(Frustum)(__GLcontext *gc,   GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val ){}
GLvoid GLAPIENTRY __glesProfile(LoadIdentity)(__GLcontext *gc){}
GLvoid GLAPIENTRY __glesProfile(LoadMatrixd)(__GLcontext *gc,   const GLdouble *m ){}
GLvoid GLAPIENTRY __glesProfile(LoadMatrixf)(__GLcontext *gc,   const GLfloat *m ){}
GLvoid GLAPIENTRY __glesProfile(MatrixMode)(__GLcontext *gc,   GLenum mode ){}
GLvoid GLAPIENTRY __glesProfile(MultMatrixd)(__GLcontext *gc,   const GLdouble *m ){}
GLvoid GLAPIENTRY __glesProfile(MultMatrixf)(__GLcontext *gc,   const GLfloat *m ){}
GLvoid GLAPIENTRY __glesProfile(Ortho)(__GLcontext *gc,   GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val ){}
GLvoid GLAPIENTRY __glesProfile(PushMatrix)(__GLcontext *gc ){}
GLvoid GLAPIENTRY __glesProfile(PopMatrix)(__GLcontext *gc){}
GLvoid GLAPIENTRY __glesProfile(Rotated)(__GLcontext *gc,   GLdouble angle, GLdouble x, GLdouble y, GLdouble z ){}
GLvoid GLAPIENTRY __glesProfile(Rotatef)(__GLcontext *gc,   GLfloat angle, GLfloat x, GLfloat y, GLfloat z ){}
GLvoid GLAPIENTRY __glesProfile(Scaled)(__GLcontext *gc,   GLdouble x, GLdouble y, GLdouble z ){}
GLvoid GLAPIENTRY __glesProfile(Scalef)(__GLcontext *gc,   GLfloat x, GLfloat y, GLfloat z ){}
GLvoid GLAPIENTRY __glesProfile(Translated)(__GLcontext *gc,   GLdouble x, GLdouble y, GLdouble z ){}
GLvoid GLAPIENTRY __glesProfile(Translatef)(__GLcontext *gc,   GLfloat x, GLfloat y, GLfloat z ){}
GLvoid GLAPIENTRY __glesProfile(ArrayElement)(__GLcontext *gc,   GLint i ){}
GLvoid GLAPIENTRY __glesProfile(ColorPointer)(__GLcontext *gc,   GLint size, GLenum type, GLsizei stride, const GLvoid *ptr ){}
GLvoid GLAPIENTRY __glesProfile(EnableClientState)(__GLcontext *gc,   GLenum cap ){}
GLvoid GLAPIENTRY __glesProfile(DisableClientState)(__GLcontext *gc,   GLenum cap ){}
GLvoid GLAPIENTRY __glesProfile(EdgeFlagPointer)(__GLcontext *gc,   GLsizei stride, const GLvoid *ptr ){}
GLvoid GLAPIENTRY __glesProfile(IndexPointer)(__GLcontext *gc,   GLenum type, GLsizei stride, const GLvoid *ptr ){}
GLvoid GLAPIENTRY __glesProfile(InterleavedArrays)(__GLcontext *gc,   GLenum format, GLsizei stride, const GLvoid *pointer ){}
GLvoid GLAPIENTRY __glesProfile(NormalPointer)(__GLcontext *gc,   GLenum type, GLsizei stride,  const GLvoid *ptr ){}
GLvoid GLAPIENTRY __glesProfile(VertexPointer)(__GLcontext *gc,   GLint size, GLenum type, GLsizei stride, const GLvoid *ptr ){}
GLboolean GLAPIENTRY __glesProfile(AreTexturesResident)(__GLcontext *gc,   GLsizei n, const GLuint *textures, GLboolean *residences ) { return GL_FALSE;}
GLvoid GLAPIENTRY __glesProfile(CopyTexImage1D)(__GLcontext *gc,   GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border ){}
GLvoid GLAPIENTRY __glesProfile(CopyTexSubImage1D)(__GLcontext *gc,   GLenum target, GLint level,  GLint xoffset, GLint x, GLint y,  GLsizei width ){}
GLvoid GLAPIENTRY __glesProfile(PrioritizeTextures)(__GLcontext *gc,   GLsizei n,  const GLuint *textures, const GLclampf *priorities ){}
GLvoid GLAPIENTRY __glesProfile(TexSubImage1D)(__GLcontext *gc,   GLenum target, GLint level,  GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels ){}
GLvoid GLAPIENTRY __glesProfile(PushClientAttrib)(__GLcontext *gc,   GLbitfield mask ){}
GLvoid GLAPIENTRY __glesProfile(PopClientAttrib)(__GLcontext *gc){}
GLvoid GLAPIENTRY __glesProfile(ColorTable)(__GLcontext *gc,   GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table ){}
GLvoid GLAPIENTRY __glesProfile(ColorTableParameteriv)(__GLcontext *gc,  GLenum target, GLenum pname, const GLint *params){}
GLvoid GLAPIENTRY __glesProfile(ColorTableParameterfv)(__GLcontext *gc,  GLenum target, GLenum pname, const GLfloat *params){}
GLvoid GLAPIENTRY __glesProfile(CopyColorTable)(__GLcontext *gc,   GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width ){}
GLvoid GLAPIENTRY __glesProfile(GetColorTable)(__GLcontext *gc,   GLenum target, GLenum format,  GLenum type, GLvoid *table ){}
GLvoid GLAPIENTRY __glesProfile(GetColorTableParameterfv)(__GLcontext *gc,   GLenum target, GLenum pname, GLfloat *params ){}
GLvoid GLAPIENTRY __glesProfile(GetColorTableParameteriv)(__GLcontext *gc,   GLenum target, GLenum pname,  GLint *params ){}
GLvoid GLAPIENTRY __glesProfile(ColorSubTable)(__GLcontext *gc,   GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data ){}
GLvoid GLAPIENTRY __glesProfile(CopyColorSubTable)(__GLcontext *gc,   GLenum target, GLsizei start, GLint x, GLint y, GLsizei width ){}
GLvoid GLAPIENTRY __glesProfile(ConvolutionFilter1D)(__GLcontext *gc,   GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *image ){}
GLvoid GLAPIENTRY __glesProfile(ConvolutionFilter2D)(__GLcontext *gc,   GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image ){}
GLvoid GLAPIENTRY __glesProfile(ConvolutionParameterf)(__GLcontext *gc,   GLenum target, GLenum pname, GLfloat params ){}
GLvoid GLAPIENTRY __glesProfile(ConvolutionParameterfv)(__GLcontext *gc,   GLenum target, GLenum pname, const GLfloat *params ){}
GLvoid GLAPIENTRY __glesProfile(ConvolutionParameteri)(__GLcontext *gc,   GLenum target, GLenum pname, GLint params ){}
GLvoid GLAPIENTRY __glesProfile(ConvolutionParameteriv)(__GLcontext *gc,   GLenum target, GLenum pname, const GLint *params ){}
GLvoid GLAPIENTRY __glesProfile(CopyConvolutionFilter1D)(__GLcontext *gc,   GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width ){}
GLvoid GLAPIENTRY __glesProfile(CopyConvolutionFilter2D)(__GLcontext *gc,   GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height){}
GLvoid GLAPIENTRY __glesProfile(GetConvolutionFilter)(__GLcontext *gc,   GLenum target, GLenum format, GLenum type, GLvoid *image ){}
GLvoid GLAPIENTRY __glesProfile(GetConvolutionParameterfv)(__GLcontext *gc,   GLenum target, GLenum pname, GLfloat *params ){}
GLvoid GLAPIENTRY __glesProfile(GetConvolutionParameteriv)(__GLcontext *gc,   GLenum target, GLenum pname, GLint *params ){}
GLvoid GLAPIENTRY __glesProfile(SeparableFilter2D)(__GLcontext *gc,   GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *row, const GLvoid *column ){}
GLvoid GLAPIENTRY __glesProfile(GetSeparableFilter)(__GLcontext *gc,   GLenum target, GLenum format, GLenum type, GLvoid *row, GLvoid *column, GLvoid *span ){}
GLvoid GLAPIENTRY __glesProfile(Histogram)(__GLcontext *gc,   GLenum target, GLsizei width, GLenum internalformat, GLboolean sink ){}
GLvoid GLAPIENTRY __glesProfile(ResetHistogram)(__GLcontext *gc,   GLenum target ){}
GLvoid GLAPIENTRY __glesProfile(GetHistogram)(__GLcontext *gc,   GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values ){}
GLvoid GLAPIENTRY __glesProfile(GetHistogramParameterfv)(__GLcontext *gc,   GLenum target, GLenum pname, GLfloat *params ){}
GLvoid GLAPIENTRY __glesProfile(GetHistogramParameteriv)(__GLcontext *gc,   GLenum target, GLenum pname, GLint *params ){}
GLvoid GLAPIENTRY __glesProfile(Minmax)(__GLcontext *gc,   GLenum target, GLenum internalformat, GLboolean sink ){}
GLvoid GLAPIENTRY __glesProfile(ResetMinmax)(__GLcontext *gc,   GLenum target ){}
GLvoid GLAPIENTRY __glesProfile(GetMinmax)(__GLcontext *gc,   GLenum target, GLboolean reset, GLenum format, GLenum types, GLvoid *values ){}
GLvoid GLAPIENTRY __glesProfile(GetMinmaxParameterfv)(__GLcontext *gc,   GLenum target, GLenum pname,  GLfloat *params ){}
GLvoid GLAPIENTRY __glesProfile(GetMinmaxParameteriv)(__GLcontext *gc,   GLenum target, GLenum pname, GLint *params ){}
GLvoid GLAPIENTRY __glesProfile(ClientActiveTexture)(__GLcontext *gc,   GLenum texture ){}
GLvoid GLAPIENTRY __glesProfile(MultiTexCoord1d)(__GLcontext *gc,   GLenum target, GLdouble s ){}
GLvoid GLAPIENTRY __glesProfile(MultiTexCoord1dv)(__GLcontext *gc,   GLenum target, const GLdouble *v ){}
GLvoid GLAPIENTRY __glesProfile(MultiTexCoord1f)(__GLcontext *gc,   GLenum target, GLfloat s ){}
GLvoid GLAPIENTRY __glesProfile(MultiTexCoord1fv)(__GLcontext *gc,   GLenum target, const GLfloat *v ){}
GLvoid GLAPIENTRY __glesProfile(MultiTexCoord1i)(__GLcontext *gc,   GLenum target, GLint s ){}
GLvoid GLAPIENTRY __glesProfile(MultiTexCoord1iv)(__GLcontext *gc,   GLenum target, const GLint *v ){}
GLvoid GLAPIENTRY __glesProfile(MultiTexCoord1s)(__GLcontext *gc,   GLenum target, GLshort s ){}
GLvoid GLAPIENTRY __glesProfile(MultiTexCoord1sv)(__GLcontext *gc,   GLenum target, const GLshort *v ){}
GLvoid GLAPIENTRY __glesProfile(MultiTexCoord2d)(__GLcontext *gc,   GLenum target, GLdouble s, GLdouble t ){}
GLvoid GLAPIENTRY __glesProfile(MultiTexCoord2dv)(__GLcontext *gc,   GLenum target, const GLdouble *v ){}
GLvoid GLAPIENTRY __glesProfile(MultiTexCoord2f)(__GLcontext *gc,   GLenum target, GLfloat s, GLfloat t ){}
GLvoid GLAPIENTRY __glesProfile(MultiTexCoord2fv)(__GLcontext *gc,   GLenum target, const GLfloat *v ){}
GLvoid GLAPIENTRY __glesProfile(MultiTexCoord2i)(__GLcontext *gc,   GLenum target, GLint s, GLint t ){}
GLvoid GLAPIENTRY __glesProfile(MultiTexCoord2iv)(__GLcontext *gc,   GLenum target, const GLint *v ){}
GLvoid GLAPIENTRY __glesProfile(MultiTexCoord2s)(__GLcontext *gc,   GLenum target, GLshort s, GLshort t ){}
GLvoid GLAPIENTRY __glesProfile(MultiTexCoord2sv)(__GLcontext *gc,   GLenum target, const GLshort *v ){}
GLvoid GLAPIENTRY __glesProfile(MultiTexCoord3d)(__GLcontext *gc,   GLenum target, GLdouble s, GLdouble t, GLdouble r ){}
GLvoid GLAPIENTRY __glesProfile(MultiTexCoord3dv)(__GLcontext *gc,   GLenum target, const GLdouble *v ){}
GLvoid GLAPIENTRY __glesProfile(MultiTexCoord3f)(__GLcontext *gc,   GLenum target, GLfloat s, GLfloat t, GLfloat r ){}
GLvoid GLAPIENTRY __glesProfile(MultiTexCoord3fv)(__GLcontext *gc,   GLenum target, const GLfloat *v ){}
GLvoid GLAPIENTRY __glesProfile(MultiTexCoord3i)(__GLcontext *gc,   GLenum target, GLint s, GLint t, GLint r ){}
GLvoid GLAPIENTRY __glesProfile(MultiTexCoord3iv)(__GLcontext *gc,   GLenum target, const GLint *v ){}
GLvoid GLAPIENTRY __glesProfile(MultiTexCoord3s)(__GLcontext *gc,   GLenum target, GLshort s, GLshort t, GLshort r ){}
GLvoid GLAPIENTRY __glesProfile(MultiTexCoord3sv)(__GLcontext *gc,   GLenum target, const GLshort *v ){}
GLvoid GLAPIENTRY __glesProfile(MultiTexCoord4d)(__GLcontext *gc,   GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q ){}
GLvoid GLAPIENTRY __glesProfile(MultiTexCoord4dv)(__GLcontext *gc,   GLenum target, const GLdouble *v ){}
GLvoid GLAPIENTRY __glesProfile(MultiTexCoord4f)(__GLcontext *gc,   GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q ){}
GLvoid GLAPIENTRY __glesProfile(MultiTexCoord4fv)(__GLcontext *gc,   GLenum target, const GLfloat *v ){}
GLvoid GLAPIENTRY __glesProfile(MultiTexCoord4i)(__GLcontext *gc,   GLenum target, GLint s, GLint t, GLint r, GLint q ){}
GLvoid GLAPIENTRY __glesProfile(MultiTexCoord4iv)(__GLcontext *gc,   GLenum target, const GLint *v ){}
GLvoid GLAPIENTRY __glesProfile(MultiTexCoord4s)(__GLcontext *gc,   GLenum target, GLshort s, GLshort t, GLshort r, GLshort q ){}
GLvoid GLAPIENTRY __glesProfile(MultiTexCoord4sv)(__GLcontext *gc,   GLenum target, const GLshort *v ){}
GLvoid GLAPIENTRY __glesProfile(LoadTransposeMatrixd)(__GLcontext *gc,   const GLdouble m[16] ){}
GLvoid GLAPIENTRY __glesProfile(LoadTransposeMatrixf)(__GLcontext *gc,   const GLfloat m[16] ){}
GLvoid GLAPIENTRY __glesProfile(MultTransposeMatrixd)(__GLcontext *gc,   const GLdouble m[16] ){}
GLvoid GLAPIENTRY __glesProfile(MultTransposeMatrixf)(__GLcontext *gc,   const GLfloat m[16] ){}
GLvoid GLAPIENTRY __glesProfile(CompressedTexImage1D)(__GLcontext *gc,   GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data ){}
GLvoid GLAPIENTRY __glesProfile(CompressedTexSubImage1D)(__GLcontext *gc,   GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data ){}
GLvoid GLAPIENTRY __glesProfile(GetCompressedTexImage)(__GLcontext *gc,   GLenum target, GLint lod, GLvoid *img ){}
GLvoid GLAPIENTRY __glesProfile(FogCoordf)(__GLcontext *gc,  GLfloat coord){}
GLvoid GLAPIENTRY __glesProfile(FogCoordfv)(__GLcontext *gc,  const GLfloat *coord){}
GLvoid GLAPIENTRY __glesProfile(FogCoordd)(__GLcontext *gc,  GLdouble coord){}
GLvoid GLAPIENTRY __glesProfile(FogCoorddv)(__GLcontext *gc,  const GLdouble *coord){}
GLvoid GLAPIENTRY __glesProfile(FogCoordPointer)(__GLcontext *gc,  GLenum type, GLsizei stride, const void *pointer){}
GLvoid GLAPIENTRY __glesProfile(PointParameterf)(__GLcontext *gc,  GLenum pname, GLfloat param){}
GLvoid GLAPIENTRY __glesProfile(PointParameterfv)(__GLcontext *gc,  GLenum pname, const GLfloat *params){}
GLvoid GLAPIENTRY __glesProfile(PointParameteri)(__GLcontext *gc,  GLenum pname, GLint param){}
GLvoid GLAPIENTRY __glesProfile(PointParameteriv)(__GLcontext *gc,  GLenum pname, const GLint *params){}
GLvoid GLAPIENTRY __glesProfile(SecondaryColor3b)(__GLcontext *gc,  GLbyte red, GLbyte green, GLbyte blue){}
GLvoid GLAPIENTRY __glesProfile(SecondaryColor3bv)(__GLcontext *gc,  const GLbyte *v){}
GLvoid GLAPIENTRY __glesProfile(SecondaryColor3d)(__GLcontext *gc,  GLdouble red, GLdouble green, GLdouble blue){}
GLvoid GLAPIENTRY __glesProfile(SecondaryColor3dv)(__GLcontext *gc,  const GLdouble *v){}
GLvoid GLAPIENTRY __glesProfile(SecondaryColor3f)(__GLcontext *gc,  GLfloat red, GLfloat green, GLfloat blue){}
GLvoid GLAPIENTRY __glesProfile(SecondaryColor3fv)(__GLcontext *gc,  const GLfloat *v){}
GLvoid GLAPIENTRY __glesProfile(SecondaryColor3i)(__GLcontext *gc,  GLint red, GLint green, GLint blue){}
GLvoid GLAPIENTRY __glesProfile(SecondaryColor3iv)(__GLcontext *gc,  const GLint *v){}
GLvoid GLAPIENTRY __glesProfile(SecondaryColor3s)(__GLcontext *gc,  GLshort red, GLshort green, GLshort blue){}
GLvoid GLAPIENTRY __glesProfile(SecondaryColor3sv)(__GLcontext *gc,  const GLshort *v){}
GLvoid GLAPIENTRY __glesProfile(SecondaryColor3ub)(__GLcontext *gc,  GLubyte red, GLubyte green, GLubyte blue){}
GLvoid GLAPIENTRY __glesProfile(SecondaryColor3ubv)(__GLcontext *gc,  const GLubyte *v){}
GLvoid GLAPIENTRY __glesProfile(SecondaryColor3ui)(__GLcontext *gc,  GLuint red, GLuint green, GLuint blue){}
GLvoid GLAPIENTRY __glesProfile(SecondaryColor3uiv)(__GLcontext *gc,  const GLuint *v){}
GLvoid GLAPIENTRY __glesProfile(SecondaryColor3us)(__GLcontext *gc,  GLushort red, GLushort green, GLushort blue){}
GLvoid GLAPIENTRY __glesProfile(SecondaryColor3usv)(__GLcontext *gc,  const GLushort *v){}
GLvoid GLAPIENTRY __glesProfile(SecondaryColorPointer)(__GLcontext *gc,  GLint size, GLenum type, GLsizei stride, const void *pointer){}
GLvoid GLAPIENTRY __glesProfile(WindowPos2d)(__GLcontext *gc,  GLdouble x, GLdouble y){}
GLvoid GLAPIENTRY __glesProfile(WindowPos2dv)(__GLcontext *gc,  const GLdouble *v){}
GLvoid GLAPIENTRY __glesProfile(WindowPos2f)(__GLcontext *gc,  GLfloat x, GLfloat y){}
GLvoid GLAPIENTRY __glesProfile(WindowPos2fv)(__GLcontext *gc,  const GLfloat *v){}
GLvoid GLAPIENTRY __glesProfile(WindowPos2i)(__GLcontext *gc,  GLint x, GLint y){}
GLvoid GLAPIENTRY __glesProfile(WindowPos2iv)(__GLcontext *gc,  const GLint *v){}
GLvoid GLAPIENTRY __glesProfile(WindowPos2s)(__GLcontext *gc,  GLshort x, GLshort y){}
GLvoid GLAPIENTRY __glesProfile(WindowPos2sv)(__GLcontext *gc,  const GLshort *v){}
GLvoid GLAPIENTRY __glesProfile(WindowPos3d)(__GLcontext *gc,  GLdouble x, GLdouble y, GLdouble z){}
GLvoid GLAPIENTRY __glesProfile(WindowPos3dv)(__GLcontext *gc,  const GLdouble *v){}
GLvoid GLAPIENTRY __glesProfile(WindowPos3f)(__GLcontext *gc,  GLfloat x, GLfloat y, GLfloat z){}
GLvoid GLAPIENTRY __glesProfile(WindowPos3fv)(__GLcontext *gc,  const GLfloat *v){}
GLvoid GLAPIENTRY __glesProfile(WindowPos3i)(__GLcontext *gc,  GLint x, GLint y, GLint z){}
GLvoid GLAPIENTRY __glesProfile(WindowPos3iv)(__GLcontext *gc,  const GLint *v){}
GLvoid GLAPIENTRY __glesProfile(WindowPos3s)(__GLcontext *gc,  GLshort x, GLshort y, GLshort z){}
GLvoid GLAPIENTRY __glesProfile(WindowPos3sv)(__GLcontext *gc,  const GLshort *v){}
GLvoid GLAPIENTRY __glesProfile(DrawPixels)(__GLcontext *gc,   GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels ){}
GLvoid GLAPIENTRY __glesProfile(CopyPixels)(__GLcontext *gc,   GLint x, GLint y,  GLsizei width, GLsizei height, GLenum type ){}
GLvoid GLAPIENTRY __glesProfile(Indexub)(__GLcontext *gc,   GLubyte c ){}
GLvoid GLAPIENTRY __glesProfile(Indexubv)(__GLcontext *gc,   const GLubyte *c ){}
GLvoid GLAPIENTRY __glesProfile(TexCoordPointer)(__GLcontext *gc, GLint size, GLenum type, GLsizei stride, const GLvoid *ptr){}
GLvoid GLAPIENTRY __glesProfile(GetVertexAttribdv) (__GLcontext *gc, GLuint index, GLenum pname, GLdouble* params){}
GLvoid GLAPIENTRY __glesProfile(GetQueryObjectiv) (__GLcontext *gc, GLuint id, GLenum pname, GLint* params){}
GLvoid GLAPIENTRY __glesProfile(GetBufferSubData) (__GLcontext *gc, GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data){}
GLvoid* GLAPIENTRY __glesProfile(MapBuffer)(__GLcontext *gc, GLenum target, GLenum access){return gcvNULL;}
GLvoid GLAPIENTRY __glesProfile(DeleteObjectARB) (__GLcontext *gc, UINT obj){}
GLvoid GLAPIENTRY __glesProfile(GetInfoLogARB) (__GLcontext *gc, UINT obj, GLsizei maxLength, GLsizei *length, char *infoLog){}
GLvoid GLAPIENTRY __glesProfile(GetObjectParameterivARB) (__GLcontext *gc, UINT obj, GLenum pname, GLint *params){}
GLvoid GLAPIENTRY __glesProfile(ClampColorARB)(__GLcontext *gc, GLenum target, GLenum clamp){}
GLboolean GLAPIENTRY __glesProfile(IsRenderbufferEXT)( __GLcontext *gc,  GLuint renderbuffer){return GL_FALSE;}
GLvoid GLAPIENTRY __glesProfile(BindRenderbufferEXT)(__GLcontext *gc,  GLenum target, GLuint renderbuffer){}
GLvoid GLAPIENTRY __glesProfile(DeleteRenderbuffersEXT)(__GLcontext *gc,  GLsizei n, const GLuint *renderbuffers){}
GLvoid GLAPIENTRY __glesProfile(GenRenderbuffersEXT)(__GLcontext *gc,  GLsizei n, GLuint *renderbuffers){}
GLvoid GLAPIENTRY __glesProfile(RenderbufferStorageEXT)(__GLcontext *gc,  GLenum target, GLenum internalformat, GLsizei width, GLsizei height){}
GLvoid GLAPIENTRY __glesProfile(GetRenderbufferParameterivEXT)(__GLcontext *gc,  GLenum target, GLenum pname, GLint* params){}
GLboolean GLAPIENTRY __glesProfile(IsFramebufferEXT)(__GLcontext *gc,  GLuint framebuffer){return GL_FALSE;}
GLvoid GLAPIENTRY __glesProfile(BindFramebufferEXT)(__GLcontext *gc,  GLenum target, GLuint framebuffer){}
GLvoid GLAPIENTRY __glesProfile(DeleteFramebuffersEXT)(__GLcontext *gc,  GLsizei n, const GLuint *framebuffers){}
GLvoid GLAPIENTRY __glesProfile(GenFramebuffersEXT)(__GLcontext *gc,  GLsizei n, GLuint *framebuffers){}
GLenum GLAPIENTRY __glesProfile(CheckFramebufferStatusEXT)(__GLcontext *gc,  GLenum target){return 0;}
GLvoid GLAPIENTRY __glesProfile(FramebufferTexture1DEXT)(__GLcontext *gc,  GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level){}
GLvoid GLAPIENTRY __glesProfile(FramebufferTexture2DEXT)(__GLcontext *gc,  GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level){}
GLvoid GLAPIENTRY __glesProfile(FramebufferTexture3DEXT)(__GLcontext *gc,  GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset){}
GLvoid GLAPIENTRY __glesProfile(FramebufferRenderbufferEXT)(__GLcontext *gc,  GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer){}
GLvoid GLAPIENTRY __glesProfile(GetFramebufferAttachmentParameterivEXT)(__GLcontext *gc,  GLenum target, GLenum attachment, GLenum pname, GLint *params){}
GLvoid GLAPIENTRY __glesProfile(GenerateMipmapEXT)(__GLcontext *gc,  GLenum target){}
GLvoid GLAPIENTRY __glesProfile(BlitFramebufferEXT)(__GLcontext *gc,  GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,GLbitfield mask, GLenum filter){}
GLvoid  GLAPIENTRY __glesProfile(VertexAttrib1d)(__GLcontext *gc, GLuint indx, GLdouble x){}
GLvoid  GLAPIENTRY __glesProfile(VertexAttrib1dv)(__GLcontext *gc, GLuint indx, const GLdouble *values){}
GLvoid  GLAPIENTRY __glesProfile(VertexAttrib1s)(__GLcontext *gc, GLuint indx, GLshort x){}
GLvoid  GLAPIENTRY __glesProfile(VertexAttrib1sv)(__GLcontext *gc, GLuint indx, const GLshort * values){}
GLvoid  GLAPIENTRY __glesProfile(VertexAttrib2d)(__GLcontext *gc, GLuint indx, GLdouble x, GLdouble y){}
GLvoid  GLAPIENTRY __glesProfile(VertexAttrib2dv)(__GLcontext *gc, GLuint indx, const GLdouble * values){}
GLvoid  GLAPIENTRY __glesProfile(VertexAttrib2s)(__GLcontext *gc, GLuint indx, GLshort x, GLshort y){}
GLvoid  GLAPIENTRY __glesProfile(VertexAttrib2sv)(__GLcontext *gc, GLuint indx, const GLshort * values){}
GLvoid  GLAPIENTRY __glesProfile(VertexAttrib3d)(__GLcontext *gc, GLuint indx, GLdouble x, GLdouble y, GLdouble z){}
GLvoid  GLAPIENTRY __glesProfile(VertexAttrib3dv)(__GLcontext *gc, GLuint indx, const GLdouble * values){}
GLvoid  GLAPIENTRY __glesProfile(VertexAttrib3s)(__GLcontext *gc, GLuint indx, GLshort x, GLshort y, GLshort z){}
GLvoid  GLAPIENTRY __glesProfile(VertexAttrib3sv)(__GLcontext *gc, GLuint indx, const GLshort * values){}
GLvoid  GLAPIENTRY __glesProfile(VertexAttrib4Nbv)(__GLcontext *gc, GLuint indx, const GLbyte * values){}
GLvoid  GLAPIENTRY __glesProfile(VertexAttrib4Niv)(__GLcontext *gc, GLuint indx, const GLint * values){}
GLvoid  GLAPIENTRY __glesProfile(VertexAttrib4Nsv)(__GLcontext *gc, GLuint indx, const GLshort * values){}
GLvoid  GLAPIENTRY __glesProfile(VertexAttrib4Nub)(__GLcontext *gc, GLuint indx, GLubyte x, GLubyte y, GLubyte z, GLubyte w){}
GLvoid  GLAPIENTRY __glesProfile(VertexAttrib4Nubv)(__GLcontext *gc, GLuint indx, const GLubyte * values){}
GLvoid  GLAPIENTRY __glesProfile(VertexAttrib4Nuiv)(__GLcontext *gc, GLuint indx, const GLuint * values){}
GLvoid  GLAPIENTRY __glesProfile(VertexAttrib4Nusv)(__GLcontext *gc, GLuint indx, const GLushort * values){}
GLvoid  GLAPIENTRY __glesProfile(VertexAttrib4bv)(__GLcontext *gc, GLuint indx, const GLbyte * values){}
GLvoid  GLAPIENTRY __glesProfile(VertexAttrib4d)(__GLcontext *gc, GLuint indx, GLdouble x, GLdouble y, GLdouble z, GLdouble w){}
GLvoid  GLAPIENTRY __glesProfile(VertexAttrib4dv)(__GLcontext *gc, GLuint indx, const GLdouble * values){}
GLvoid  GLAPIENTRY __glesProfile(VertexAttrib4iv)(__GLcontext *gc, GLuint indx, const GLint * values){}
GLvoid  GLAPIENTRY __glesProfile(VertexAttrib4s)(__GLcontext *gc, GLuint indx, GLshort x, GLshort y, GLshort z, GLshort w){}
GLvoid  GLAPIENTRY __glesProfile(VertexAttrib4sv)(__GLcontext *gc, GLuint indx, const GLshort * values){}
GLvoid  GLAPIENTRY __glesProfile(VertexAttrib4ubv)(__GLcontext *gc, GLuint indx, const GLubyte * values){}
GLvoid  GLAPIENTRY __glesProfile(VertexAttrib4uiv)(__GLcontext *gc, GLuint indx, const GLuint * values){}
GLvoid  GLAPIENTRY __glesProfile(VertexAttrib4usv)(__GLcontext *gc, GLuint indx, const GLushort * values){}
GLvoid  GLAPIENTRY __glesProfile(BindFragDataLocation)(__GLcontext *gc,  GLuint program, GLuint colorNumber, const GLchar *name){}
GLvoid  GLAPIENTRY __glesProfile(GetUniformdv)(__GLcontext *gc,  GLuint program, GLint location, GLdouble * params){}
GLvoid  GLAPIENTRY __glesProfile(Uniform1d)(__GLcontext *gc, GLint location, GLdouble v0){}
GLvoid  GLAPIENTRY __glesProfile(Uniform2d)(__GLcontext *gc, GLint location, GLdouble v0, GLdouble v1){}
GLvoid  GLAPIENTRY __glesProfile(Uniform3d)(__GLcontext *gc, GLint location, GLdouble v0, GLdouble v1, GLdouble v2){}
GLvoid  GLAPIENTRY __glesProfile(Uniform4d)(__GLcontext *gc, GLint location, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3){}
GLvoid  GLAPIENTRY __glesProfile(Uniform1dv)(__GLcontext *gc, GLint location, GLsizei count, const GLdouble* value){}
GLvoid  GLAPIENTRY __glesProfile(Uniform2dv)(__GLcontext *gc, GLint location, GLsizei count, const GLdouble* value){}
GLvoid  GLAPIENTRY __glesProfile(Uniform3dv)(__GLcontext *gc, GLint location, GLsizei count, const GLdouble* value){}
GLvoid  GLAPIENTRY __glesProfile(Uniform4dv)(__GLcontext *gc, GLint location, GLsizei count, const GLdouble* value){}
GLvoid  GLAPIENTRY __glesProfile(UniformMatrix2dv)(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value){}
GLvoid  GLAPIENTRY __glesProfile(UniformMatrix3dv)(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value){}
GLvoid  GLAPIENTRY __glesProfile(UniformMatrix4dv)(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value){}
GLvoid  GLAPIENTRY __glesProfile(UniformMatrix2x3dv)(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value){}
GLvoid  GLAPIENTRY __glesProfile(UniformMatrix3x2dv)(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value){}
GLvoid  GLAPIENTRY __glesProfile(UniformMatrix2x4dv)(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value){}
GLvoid  GLAPIENTRY __glesProfile(UniformMatrix4x2dv)(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value){}
GLvoid  GLAPIENTRY __glesProfile(UniformMatrix3x4dv)(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value){}
GLvoid  GLAPIENTRY __glesProfile(UniformMatrix4x3dv)(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble* value){}
GLvoid GLAPIENTRY __glesProfile(ClampColor)(__GLcontext *gc, GLenum target, GLenum clamp){}
GLvoid GLAPIENTRY __glesProfile(BeginConditionalRender)(__GLcontext *gc, GLuint id, GLenum mode){}
GLvoid GLAPIENTRY __glesProfile(EndConditionalRender)(__GLcontext *gc){}
GLvoid GLAPIENTRY __glesProfile(VertexAttribI1i)(__GLcontext *gc, GLuint index, GLint x){}
GLvoid GLAPIENTRY __glesProfile(VertexAttribI2i)(__GLcontext *gc, GLuint index, GLint x, GLint y){}
GLvoid GLAPIENTRY __glesProfile(VertexAttribI3i)(__GLcontext *gc, GLuint index, GLint x, GLint y, GLint z){}
GLvoid GLAPIENTRY __glesProfile(VertexAttribI1ui)(__GLcontext *gc, GLuint index, GLuint x){}
GLvoid GLAPIENTRY __glesProfile(VertexAttribI2ui)(__GLcontext *gc, GLuint index, GLuint x, GLuint y){}
GLvoid GLAPIENTRY __glesProfile(VertexAttribI3ui)(__GLcontext *gc, GLuint index, GLuint x, GLuint y, GLuint z){}
GLvoid GLAPIENTRY __glesProfile(VertexAttribI1iv)(__GLcontext *gc, GLuint index, const GLint *v){}
GLvoid GLAPIENTRY __glesProfile(VertexAttribI2iv)(__GLcontext *gc, GLuint index, const GLint *v){}
GLvoid GLAPIENTRY __glesProfile(VertexAttribI3iv)(__GLcontext *gc, GLuint index, const GLint *v){}
GLvoid GLAPIENTRY __glesProfile(VertexAttribI1uiv)(__GLcontext *gc, GLuint index, const GLuint *v){}
GLvoid GLAPIENTRY __glesProfile(VertexAttribI2uiv)(__GLcontext *gc, GLuint index, const GLuint *v){}
GLvoid GLAPIENTRY __glesProfile(VertexAttribI3uiv)(__GLcontext *gc, GLuint index, const GLuint *v){}
GLvoid GLAPIENTRY __glesProfile(VertexAttribI4bv)(__GLcontext *gc, GLuint index, const GLbyte *v){}
GLvoid GLAPIENTRY __glesProfile(VertexAttribI4sv)(__GLcontext *gc, GLuint index, const GLshort *v){}
GLvoid GLAPIENTRY __glesProfile(VertexAttribI4ubv)(__GLcontext *gc, GLuint index, const GLubyte *v){}
GLvoid GLAPIENTRY __glesProfile(VertexAttribI4usv)(__GLcontext *gc, GLuint index, const GLushort *v){}
GLvoid GLAPIENTRY __glesProfile(FramebufferTexture1D)(__GLcontext *gc,  GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level){}
GLvoid GLAPIENTRY __glesProfile(FramebufferTexture3D)(__GLcontext *gc,  GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset){}
GLvoid GLAPIENTRY __glesProfile(PrimitiveRestartIndex)(__GLcontext *gc, GLuint index){}
GLvoid GLAPIENTRY __glesProfile(GetActiveUniformName)(__GLcontext *gc, GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformName){}
GLvoid GLAPIENTRY __glesProfile(MultiDrawArrays)(__GLcontext *gc, GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount){}
GLvoid GLAPIENTRY __glesProfile(MultiDrawElements)(__GLcontext *gc, GLenum mode, const GLsizei *count, GLenum type, const GLvoid*const*indices, GLsizei primcount){}
__GLesDispatchTable __glesApiProfileDispatchTable =
{
    __GLES_API_ENTRIES(__glesProfile)
};

