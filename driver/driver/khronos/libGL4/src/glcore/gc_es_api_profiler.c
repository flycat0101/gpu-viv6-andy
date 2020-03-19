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


#include <stdio.h>
#include "gc_es_context.h"

#define __GL_LOG_API(...)  gcmPRINT(__VA_ARGS__)

__GLtraceDispatchTable __glTracerDispatchTable = {0};

#define __glApiNameStr(func)  #func

GLchar *__glTracerFuncNames[] = {
    __GL_API_ENTRIES(__glApiNameStr)
};

extern GLint __glApiTraceMode;

#if VIVANTE_PROFILER
GLint __glApiProfileMode = -1;

#define __GL_PROFILE_VARS() \
    gctHANDLE tid = gcoOS_GetCurrentThreadID(); \
    gctUINT64 startTimeusec = 0, endTimeusec = 0

#define __GL_PROFILE_HEADER() \
    if (__glApiProfileMode > 0) \
    {\
        gcoOS_GetTime(&startTimeusec);\
    }\


#define __GL_PROFILE_FOOTER(api_id) \
    if (__glApiProfileMode > 0 && api_id >= 100) \
    { \
        gc->profiler.apiCalls[api_id - 100]++; \
        gcoOS_GetTime(&endTimeusec); \
        gc->profiler.totalDriverTime+=(endTimeusec-startTimeusec); \
        gc->profiler.apiTimes[api_id - 100]+=(endTimeusec-startTimeusec); \
    }\

#else

#define __GL_PROFILE_VARS() \
    gctHANDLE tid = gcoOS_GetCurrentThreadID()

#define __GL_PROFILE_HEADER()

#define __GL_PROFILE_FOOTER(api_id)

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

            /* Clear __glTracerDispatchTable[] */
            __GL_MEMZERO(&__glTracerDispatchTable, sizeof(__GLtraceDispatchTable));

            return GL_FALSE;
        }

        switch (apiVersion)
        {
        case __GL_API_VERSION_ES20:
            tableSize = (GLsizei)(offsetof(__GLtraceDispatchTable, ReadBuffer)              / sizeof(GLvoid*));
            break;
        case __GL_API_VERSION_ES30:
            tableSize = (GLsizei)(offsetof(__GLtraceDispatchTable, DispatchCompute)         / sizeof(GLvoid*));
            break;
        case __GL_API_VERSION_ES31:
            tableSize = (GLsizei)(offsetof(__GLtraceDispatchTable, TexStorage3DMultisample) / sizeof(GLvoid*));
            break;
        case __GL_API_VERSION_ES32:
            tableSize = (GLsizei)(sizeof(__GLtraceDispatchTable)                            / sizeof(GLvoid*));
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
            gcoOS_StrCatSafe(trApiName, 80, __glTracerFuncNames[i]);
            status =  gcoOS_GetProcAddress(gcvNULL, trlib, trApiName, &funcPtr);

            if (status == gcvSTATUS_OK)
            {
                ((void *(*))(&__glTracerDispatchTable))[i] = funcPtr;
            }
            else
            {
                gcoOS_Print("Failed to initialize __glTracerDispatchTable: gl%s!\n", __glTracerFuncNames[i]);

                /* Clear __glTracerDispatchTable[] */
                __GL_MEMZERO(&__glTracerDispatchTable, sizeof(__GLtraceDispatchTable));

                gcoOS_FreeLibrary(gcvNULL, trlib);
                return GL_FALSE;
            }
        }
    }
    else
    {
        /* Clear __glTracerDispatchTable to enable simple GLES API log function */
        __GL_MEMZERO(&__glTracerDispatchTable, sizeof(__GLtraceDispatchTable));
    }

    return GL_TRUE;
}


/* GLES API profiler wrapper functions that can do more API profiling functions */

/* OpenGL ES 2.0 */

#define __GL_PTRVALUE(ptr)  ((ptr) ? *(ptr) : 0)

__GL_INLINE GLvoid __glLogArrayData(__GLcontext *gc, GLsizei n, const GLuint *array)
{
    __GL_LOG_API("{");
    if (n > 0 && array)
    {
        GLsizei i;
        __GL_LOG_API("%d", array[0]);
        for (i = 1; i < n; ++i)
        {
            __GL_LOG_API(", %d", array[i]);
        }
    }
    __GL_LOG_API("}\n");
}

__GL_INLINE GLvoid __glLogSourceStrings(__GLcontext *gc, GLsizei count, const GLchar* const* string)
{
    GLint i, j;
    GLchar tmpbuf[256], *chptr;

    if ((gcvNULL == string) || (gcvNULL == *string))
    {
        __GL_LOG_API("####\n\n####\n");
        return;
    }

    __GL_LOG_API("####\n");
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
            __GL_LOG_API("%s\n", tmpbuf);
        }
    }
    __GL_LOG_API("####\n");
}

GLvoid GLAPIENTRY __glProfile_ActiveTexture(__GLcontext *gc, GLenum texture)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glActiveTexture 0x%04X\n",
                        gc, tid, texture);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ActiveTexture(gc, texture);
    __GL_PROFILE_FOOTER(GL4_ACTIVETEXTURE);

    if (__glTracerDispatchTable.ActiveTexture)
    {
        (*__glTracerDispatchTable.ActiveTexture)(texture);
    }
}

GLvoid GLAPIENTRY __glProfile_AttachShader(__GLcontext *gc, GLuint program, GLuint shader)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glAttachShader %d %d\n",
                        gc, tid, program, shader);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->AttachShader(gc, program, shader);
    __GL_PROFILE_FOOTER(GL4_ATTACHSHADER);

    if (__glTracerDispatchTable.AttachShader)
    {
        (*__glTracerDispatchTable.AttachShader)(program, shader);
    }
}

GLvoid GLAPIENTRY __glProfile_BindAttribLocation(__GLcontext *gc, GLuint program, GLuint index, const GLchar* name)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glBindAttribLocation %d %d %s\n",
                        gc, tid, program, index, name);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->BindAttribLocation(gc, program, index, name);
    __GL_PROFILE_FOOTER(GL4_BINDATTRIBLOCATION);

    if (__glTracerDispatchTable.BindAttribLocation)
    {
        (*__glTracerDispatchTable.BindAttribLocation)(program, index, name);
    }
}

GLvoid GLAPIENTRY __glProfile_BindBuffer(__GLcontext *gc, GLenum target, GLuint buffer)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glBindBuffer 0x%04X %d\n",
                        gc, tid, target, buffer);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->BindBuffer(gc, target, buffer);
    __GL_PROFILE_FOOTER(GL4_BINDBUFFER);

    if (__glTracerDispatchTable.BindBuffer)
    {
        (*__glTracerDispatchTable.BindBuffer)(target, buffer);
    }
}

GLvoid GLAPIENTRY __glProfile_BindFramebuffer(__GLcontext *gc, GLenum target, GLuint framebuffer)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glBindFramebuffer 0x%04X %d\n",
                        gc, tid, target, framebuffer);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->BindFramebuffer(gc, target, framebuffer);
    __GL_PROFILE_FOOTER(GL4_BINDFRAMEBUFFER);

    if (__glTracerDispatchTable.BindFramebuffer)
    {
        (*__glTracerDispatchTable.BindFramebuffer)(target, framebuffer);
    }
}

GLvoid GLAPIENTRY __glProfile_BindRenderbuffer(__GLcontext *gc, GLenum target, GLuint renderbuffer)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glBindRenderbuffer 0x%04X %d\n",
                        gc, tid, target, renderbuffer);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->BindRenderbuffer(gc, target, renderbuffer);
    __GL_PROFILE_FOOTER(GL4_BINDRENDERBUFFER);

    if (__glTracerDispatchTable.BindRenderbuffer)
    {
        (*__glTracerDispatchTable.BindRenderbuffer)(target, renderbuffer);
    }
}

GLvoid GLAPIENTRY __glProfile_BindTexture(__GLcontext *gc, GLenum target, GLuint texture)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glBindTexture 0x%04X %d\n",
                        gc, tid, target, texture);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->BindTexture(gc, target, texture);
    __GL_PROFILE_FOOTER(GL4_BINDTEXTURE);

    if (__glTracerDispatchTable.BindTexture)
    {
        (*__glTracerDispatchTable.BindTexture)(target, texture);
    }
}

GLvoid GLAPIENTRY __glProfile_BlendColor(__GLcontext *gc, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glBlendColor %f %f %f %f\n",
                        gc, tid, red, green, blue, alpha);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->BlendColor(gc, red, green, blue, alpha);
    __GL_PROFILE_FOOTER(GL4_BLENDCOLOR);

    if (__glTracerDispatchTable.BlendColor)
    {
        (*__glTracerDispatchTable.BlendColor)(red, green, blue, alpha);
    }
}

GLvoid GLAPIENTRY __glProfile_BlendEquation(__GLcontext *gc, GLenum mode)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glBlendEquation 0x%04X\n",
                        gc, tid, mode);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->BlendEquation(gc, mode);
    __GL_PROFILE_FOOTER(GL4_BLENDEQUATION);

    if (__glTracerDispatchTable.BlendEquation)
    {
        (*__glTracerDispatchTable.BlendEquation)(mode);
    }
}

GLvoid GLAPIENTRY __glProfile_BlendEquationSeparate(__GLcontext *gc, GLenum modeRGB, GLenum modeAlpha)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glBlendEquationSeparate 0x%04X 0x%04X\n",
                        gc, tid, modeRGB, modeAlpha);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->BlendEquationSeparate(gc, modeRGB, modeAlpha);
    __GL_PROFILE_FOOTER(GL4_BLENDEQUATIONSEPARATE);

    if (__glTracerDispatchTable.BlendEquationSeparate)
    {
        (*__glTracerDispatchTable.BlendEquationSeparate)(modeRGB, modeAlpha);
    }
}

GLvoid GLAPIENTRY __glProfile_BlendFunc(__GLcontext *gc, GLenum sfactor, GLenum dfactor)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glBlendFunc 0x%04X 0x%04X\n",
                        gc, tid, sfactor, dfactor);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->BlendFunc(gc, sfactor, dfactor);
    __GL_PROFILE_FOOTER(GL4_BLENDFUNC);

    if (__glTracerDispatchTable.BlendFunc)
    {
        (*__glTracerDispatchTable.BlendFunc)(sfactor, dfactor);
    }
}

GLvoid GLAPIENTRY __glProfile_BlendFuncSeparate(__GLcontext *gc, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glBlendFuncSeparate 0x%04X 0x%04X 0x%04X 0x%04X\n",
                        gc, tid, srcRGB, dstRGB, srcAlpha, dstAlpha);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->BlendFuncSeparate(gc, srcRGB, dstRGB, srcAlpha, dstAlpha);
    __GL_PROFILE_FOOTER(GL4_BLENDFUNCSEPARATE);

    if (__glTracerDispatchTable.BlendFuncSeparate)
    {
        (*__glTracerDispatchTable.BlendFuncSeparate)(srcRGB, dstRGB, srcAlpha, dstAlpha);
    }
}

GLvoid GLAPIENTRY __glProfile_BufferData(__GLcontext *gc, GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glBufferData 0x%04X 0x%08X 0x%08X 0x%04X\n",
                        gc, tid, target, __GL_PTR2UINT(size), __GL_PTR2UINT(data), usage);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->BufferData(gc, target, size, data, usage);
    __GL_PROFILE_FOOTER(GL4_BUFFERDATA);

    if (__glTracerDispatchTable.BufferData)
    {
        (*__glTracerDispatchTable.BufferData)(target, size, data, usage);
    }
}

GLvoid GLAPIENTRY __glProfile_BufferSubData(__GLcontext *gc, GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glBufferSubData 0x%04X 0x%08X 0x%08X 0x%08X\n",
                        gc, tid, target, __GL_PTR2UINT(offset), __GL_PTR2UINT(size), __GL_PTR2UINT(data));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->BufferSubData(gc, target, offset, size, data);
    __GL_PROFILE_FOOTER(GL4_BUFFERSUBDATA);

    if (__glTracerDispatchTable.BufferSubData)
    {
        (*__glTracerDispatchTable.BufferSubData)(target, offset, size, data);
    }
}

GLenum GLAPIENTRY __glProfile_CheckFramebufferStatus(__GLcontext *gc, GLenum target)
{
    GLenum status;
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glCheckFramebufferStatus 0x%04X\n",
                        gc, tid, target);
    }

    __GL_PROFILE_HEADER();
    status = gc->pModeDispatch->CheckFramebufferStatus(gc, target);
    __GL_PROFILE_FOOTER(GL4_CHECKFRAMEBUFFERSTATUS);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glCheckFramebufferStatus => 0x%04X\n", status);
    }

    if (__glTracerDispatchTable.CheckFramebufferStatus)
    {
        (*__glTracerDispatchTable.CheckFramebufferStatus)(target);
    }

    return status;
}

GLvoid GLAPIENTRY __glProfile_Clear(__GLcontext *gc, GLbitfield mask)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glClear 0x%08X\n",
                        gc, tid, mask);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Clear(gc, mask);
    __GL_PROFILE_FOOTER(GL4_CLEAR);

    if (__glTracerDispatchTable.Clear)
    {
        (*__glTracerDispatchTable.Clear)(mask);
    }
}

GLvoid GLAPIENTRY __glProfile_ClearColor(__GLcontext *gc, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glClearColor %f %f %f %f\n",
                        gc, tid, red, green, blue, alpha);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ClearColor(gc, red, green, blue, alpha);
    __GL_PROFILE_FOOTER(GL4_CLEARCOLOR);

    if (__glTracerDispatchTable.ClearColor)
    {
        (*__glTracerDispatchTable.ClearColor)(red, green, blue, alpha);
    }
}

GLvoid GLAPIENTRY __glProfile_ClearDepthf(__GLcontext *gc, GLfloat depth)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glClearDepthf %f\n",
                        gc, tid, depth);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ClearDepthf(gc, depth);
    __GL_PROFILE_FOOTER(GL4_CLEARDEPTHF);

    if (__glTracerDispatchTable.ClearDepthf)
    {
        (*__glTracerDispatchTable.ClearDepthf)(depth);
    }
}

GLvoid GLAPIENTRY __glProfile_ClearStencil(__GLcontext *gc, GLint s)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glClearStencil %d\n",
                        gc, tid, s);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ClearStencil(gc, s);
    __GL_PROFILE_FOOTER(GL4_CLEARSTENCIL);

    if (__glTracerDispatchTable.ClearStencil)
    {
        (*__glTracerDispatchTable.ClearStencil)(s);
    }
}

GLvoid GLAPIENTRY __glProfile_ColorMask(__GLcontext *gc, GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColorMask %d %d %d %d\n",
                        gc, tid, red, green, blue, alpha);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ColorMask(gc, red, green, blue, alpha);
    __GL_PROFILE_FOOTER(GL4_COLORMASK);

    if (__glTracerDispatchTable.ColorMask)
    {
        (*__glTracerDispatchTable.ColorMask)(red, green, blue, alpha);
    }
}

GLvoid GLAPIENTRY __glProfile_CompileShader(__GLcontext *gc, GLuint shader)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glCompileShader %d\n",
                        gc, tid, shader);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->CompileShader(gc, shader);
    __GL_PROFILE_FOOTER(GL4_COMPILESHADER);

    if (__glTracerDispatchTable.CompileShader)
    {
        (*__glTracerDispatchTable.CompileShader)(shader);
    }
}

GLvoid GLAPIENTRY __glProfile_CompressedTexImage2D(__GLcontext *gc, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glCompressedTexImage2D 0x%04X %d 0x%04X %d %d %d %d 0x%08X\n",
                        gc, tid, target, level, internalformat, width, height, border, imageSize, __GL_PTR2UINT(data));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->CompressedTexImage2D(gc, target, level, internalformat, width, height, border, imageSize, data);
    __GL_PROFILE_FOOTER(GL4_COMPRESSEDTEXIMAGE2D);

    if (__glTracerDispatchTable.CompressedTexImage2D)
    {
        (*__glTracerDispatchTable.CompressedTexImage2D)(target, level, internalformat, width, height, border, imageSize, data);
    }
}

GLvoid GLAPIENTRY __glProfile_CompressedTexSubImage2D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid* data)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glCompressedTexSubImage2D 0x%04X %d %d %d %d %d 0x%04X %d 0x%08X\n",
                        gc, tid, target, level, xoffset, yoffset, width, height, format, imageSize, __GL_PTR2UINT(data));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->CompressedTexSubImage2D(gc, target, level, xoffset, yoffset, width, height, format, imageSize, data);
    __GL_PROFILE_FOOTER(GL4_COMPRESSEDTEXSUBIMAGE2D);

    if (__glTracerDispatchTable.CompressedTexSubImage2D)
    {
        (*__glTracerDispatchTable.CompressedTexSubImage2D)(target, level, xoffset, yoffset, width, height, format, imageSize, data);
    }
}

GLvoid GLAPIENTRY __glProfile_CopyTexImage2D(__GLcontext *gc, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glCopyTexImage2D 0x%04X %d 0x%04X %d %d %d %d %d\n",
                        gc, tid, target, level, internalformat, x, y, width, height, border);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->CopyTexImage2D(gc, target, level, internalformat, x, y, width, height, border);
    __GL_PROFILE_FOOTER(GL4_COPYTEXIMAGE2D);

    if (__glTracerDispatchTable.CopyTexImage2D)
    {
        (*__glTracerDispatchTable.CopyTexImage2D)(target, level, internalformat, x, y, width, height, border);
    }
}

GLvoid GLAPIENTRY __glProfile_CopyTexSubImage2D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glCopyTexSubImage2D 0x%04X %d %d %d %d %d %d %d\n",
                        gc, tid, target, level, xoffset, yoffset, x, y, width, height);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->CopyTexSubImage2D(gc, target, level, xoffset, yoffset, x, y, width, height);
    __GL_PROFILE_FOOTER(GL4_COPYTEXSUBIMAGE2D);

    if (__glTracerDispatchTable.CopyTexSubImage2D)
    {
        (*__glTracerDispatchTable.CopyTexSubImage2D)(target, level, xoffset, yoffset, x, y, width, height);
    }
}

GLuint GLAPIENTRY __glProfile_CreateProgram(__GLcontext *gc)
{
    __GL_PROFILE_VARS();
    GLuint program;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glCreateProgram\n",
                        gc, tid);
    }

    __GL_PROFILE_HEADER();
    program = gc->pModeDispatch->CreateProgram(gc);
    __GL_PROFILE_FOOTER(GL4_CREATEPROGRAM);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glCreateProgram => %d\n", program);
    }

    if (__glTracerDispatchTable.CreateProgram)
    {
        (*__glTracerDispatchTable.CreateProgram)(program);
    }

    return program;
}

GLuint GLAPIENTRY __glProfile_CreateShader(__GLcontext *gc, GLenum type)
{
    __GL_PROFILE_VARS();
    GLuint shader;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glCreateShader 0x%04X\n",
                        gc, tid, type);
    }

    __GL_PROFILE_HEADER();
    shader = gc->pModeDispatch->CreateShader(gc, type);
    __GL_PROFILE_FOOTER(GL4_CREATESHADER);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glCreateShader => %d\n", shader);
    }

    if (__glTracerDispatchTable.CreateShader)
    {
        (*__glTracerDispatchTable.CreateShader)(type, shader);
    }

    return shader;
}

GLvoid GLAPIENTRY __glProfile_CullFace(__GLcontext *gc, GLenum mode)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glCullFace 0x%04X\n",
                        gc, tid, mode);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->CullFace(gc, mode);
    __GL_PROFILE_FOOTER(GL4_CULLFACE);

    if (__glTracerDispatchTable.CullFace)
    {
        (*__glTracerDispatchTable.CullFace)(mode);
    }
}

GLvoid GLAPIENTRY __glProfile_DeleteBuffers(__GLcontext *gc, GLsizei n, const GLuint* buffers)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDeleteBuffers %d ",
                        gc, tid, n);
        __glLogArrayData(gc, n, buffers);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DeleteBuffers(gc, n, buffers);
    __GL_PROFILE_FOOTER(GL4_DELETEBUFFERS);

    if (__glTracerDispatchTable.DeleteBuffers)
    {
        (*__glTracerDispatchTable.DeleteBuffers)(n, buffers);
    }
}

GLvoid GLAPIENTRY __glProfile_DeleteFramebuffers(__GLcontext *gc, GLsizei n, const GLuint* framebuffers)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDeleteFramebuffers %d ",
                        gc, tid, n);
        __glLogArrayData(gc, n, framebuffers);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DeleteFramebuffers(gc, n, framebuffers);
    __GL_PROFILE_FOOTER(GL4_DELETEFRAMEBUFFERS);

    if (__glTracerDispatchTable.DeleteFramebuffers)
    {
        (*__glTracerDispatchTable.DeleteFramebuffers)(n, framebuffers);
    }
}

GLvoid GLAPIENTRY __glProfile_DeleteProgram(__GLcontext *gc, GLuint program)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDeleteProgram %d\n",
                        gc, tid, program);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DeleteProgram(gc, program);
    __GL_PROFILE_FOOTER(GL4_DELETEPROGRAM);

    if (__glTracerDispatchTable.DeleteProgram)
    {
        (*__glTracerDispatchTable.DeleteProgram)(program);
    }
}

GLvoid GLAPIENTRY __glProfile_DeleteRenderbuffers(__GLcontext *gc, GLsizei n, const GLuint* renderbuffers)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDeleteRenderbuffers %d ",
                        gc, tid, n);
        __glLogArrayData(gc, n, renderbuffers);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DeleteRenderbuffers(gc, n, renderbuffers);
    __GL_PROFILE_FOOTER(GL4_DELETERENDERBUFFERS);

    if (__glTracerDispatchTable.DeleteRenderbuffers)
    {
        (*__glTracerDispatchTable.DeleteRenderbuffers)(n, renderbuffers);
    }
}

GLvoid GLAPIENTRY __glProfile_DeleteShader(__GLcontext *gc, GLuint shader)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDeleteShader %d\n",
                        gc, tid, shader);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DeleteShader(gc, shader);
    __GL_PROFILE_FOOTER(GL4_DELETESHADER);

    if (__glTracerDispatchTable.DeleteShader)
    {
        (*__glTracerDispatchTable.DeleteShader)(shader);
    }
}

GLvoid GLAPIENTRY __glProfile_DeleteTextures(__GLcontext *gc, GLsizei n, const GLuint* textures)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDeleteTextures %d ",
                        gc, tid, n);
        __glLogArrayData(gc, n, textures);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DeleteTextures(gc, n, textures);
    __GL_PROFILE_FOOTER(GL4_DELETETEXTURES);

    if (__glTracerDispatchTable.DeleteTextures)
    {
        (*__glTracerDispatchTable.DeleteTextures)(n, textures);
    }
}

GLvoid GLAPIENTRY __glProfile_DepthFunc(__GLcontext *gc, GLenum func)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDepthFunc 0x%04X\n",
                        gc, tid, func);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DepthFunc(gc, func);
    __GL_PROFILE_FOOTER(GL4_DEPTHFUNC);

    if (__glTracerDispatchTable.DepthFunc)
    {
        (*__glTracerDispatchTable.DepthFunc)(func);
    }
}

GLvoid GLAPIENTRY __glProfile_DepthMask(__GLcontext *gc, GLboolean flag)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDepthMask %d\n",
                        gc, tid, flag);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DepthMask(gc, flag);
    __GL_PROFILE_FOOTER(GL4_DEPTHMASK);

    if (__glTracerDispatchTable.DepthMask)
    {
        (*__glTracerDispatchTable.DepthMask)(flag);
    }
}

GLvoid GLAPIENTRY __glProfile_DepthRangef(__GLcontext *gc, GLfloat n, GLfloat f)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDepthRangef %f %f\n",
                        gc, tid, n, f);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DepthRangef(gc, n, f);
    __GL_PROFILE_FOOTER(GL4_DEPTHRANGEF);

    if (__glTracerDispatchTable.DepthRangef)
    {
        (*__glTracerDispatchTable.DepthRangef)(n, f);
    }
}

GLvoid GLAPIENTRY __glProfile_DetachShader(__GLcontext *gc, GLuint program, GLuint shader)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDetachShader %d %d\n",
                        gc, tid, program, shader);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DetachShader(gc, program, shader);
    __GL_PROFILE_FOOTER(GL4_DETACHSHADER);

    if (__glTracerDispatchTable.DetachShader)
    {
        (*__glTracerDispatchTable.DetachShader)(program, shader);
    }
}

GLvoid GLAPIENTRY __glProfile_Disable(__GLcontext *gc, GLenum cap)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDisable 0x%04X\n",
                        gc, tid, cap);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Disable(gc, cap);
    __GL_PROFILE_FOOTER(GL4_DISABLE);

    if (__glTracerDispatchTable.Disable)
    {
        (*__glTracerDispatchTable.Disable)(cap);
    }
}

GLvoid GLAPIENTRY __glProfile_DisableVertexAttribArray(__GLcontext *gc, GLuint index)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDisableVertexAttribArray %d\n",
                        gc, tid, index);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DisableVertexAttribArray(gc, index);
    __GL_PROFILE_FOOTER(GL4_DISABLEVERTEXATTRIBARRAY);

    if (__glTracerDispatchTable.DisableVertexAttribArray)
    {
        (*__glTracerDispatchTable.DisableVertexAttribArray)(index);
    }
}

GLvoid GLAPIENTRY __glProfile_DrawArrays(__GLcontext *gc, GLenum mode, GLint first, GLsizei count)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDrawArrays 0x%04X %d %d\n",
                         gc, tid, mode, first, count);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DrawArrays(gc, mode, first, count);
    __GL_PROFILE_FOOTER(GL4_DRAWARRAYS);

    if (__glTracerDispatchTable.DrawArrays)
    {
        (*__glTracerDispatchTable.DrawArrays)(mode, first, count);
    }
}

GLvoid GLAPIENTRY __glProfile_DrawElements(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const GLvoid* indices)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDrawElements 0x%04X %d 0x%04X 0x%08X\n",
                        gc, tid, mode, count, type, __GL_PTR2UINT(indices));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DrawElements(gc, mode, count, type, indices);
    __GL_PROFILE_FOOTER(GL4_DRAWELEMENTS);

    if (__glTracerDispatchTable.DrawElements)
    {
        (*__glTracerDispatchTable.DrawElements)(mode, count, type, indices);
    }
}

GLvoid GLAPIENTRY __glProfile_Enable(__GLcontext *gc, GLenum cap)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glEnable 0x%04X\n",
                        gc, tid, cap);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Enable(gc, cap);
    __GL_PROFILE_FOOTER(GL4_ENABLE);

    if (__glTracerDispatchTable.Enable)
    {
        (*__glTracerDispatchTable.Enable)(cap);
    }
}

GLvoid GLAPIENTRY __glProfile_EnableVertexAttribArray(__GLcontext *gc, GLuint index)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glEnableVertexAttribArray %d\n",
                        gc, tid, index);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->EnableVertexAttribArray(gc, index);
    __GL_PROFILE_FOOTER(GL4_ENABLEVERTEXATTRIBARRAY);

    if (__glTracerDispatchTable.EnableVertexAttribArray)
    {
        (*__glTracerDispatchTable.EnableVertexAttribArray)(index);
    }
}

GLvoid GLAPIENTRY __glProfile_Finish(__GLcontext *gc)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glFinish\n",
                        gc, tid);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Finish(gc);
    __GL_PROFILE_FOOTER(GL4_FINISH);

    if (__glTracerDispatchTable.Finish)
    {
        (*__glTracerDispatchTable.Finish)();
    }
}

GLvoid GLAPIENTRY __glProfile_Flush(__GLcontext *gc)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glFlush\n",
                        gc, tid);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Flush(gc);
    __GL_PROFILE_FOOTER(GL4_FLUSH);

    if (__glTracerDispatchTable.Flush)
    {
        (*__glTracerDispatchTable.Flush)();
    }
}

GLvoid GLAPIENTRY __glProfile_FramebufferRenderbuffer(__GLcontext *gc, GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glFramebufferRenderbuffer 0x%04X 0x%04X 0x%04X %d\n",
                        gc, tid, target, attachment, renderbuffertarget, renderbuffer);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->FramebufferRenderbuffer(gc, target, attachment, renderbuffertarget, renderbuffer);
    __GL_PROFILE_FOOTER(GL4_FRAMEBUFFERRENDERBUFFER);

    if (__glTracerDispatchTable.FramebufferRenderbuffer)
    {
        (*__glTracerDispatchTable.FramebufferRenderbuffer)(target, attachment, renderbuffertarget, renderbuffer);
    }
}

GLvoid GLAPIENTRY __glProfile_FramebufferTexture2D(__GLcontext *gc, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glFramebufferTexture2D 0x%04X 0x%04X 0x%04X %d %d\n",
                        gc, tid, target, attachment, textarget, texture, level);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->FramebufferTexture2D(gc, target, attachment, textarget, texture, level);
    __GL_PROFILE_FOOTER(GL4_FRAMEBUFFERTEXTURE2D);

    if (__glTracerDispatchTable.FramebufferTexture2D)
    {
        (*__glTracerDispatchTable.FramebufferTexture2D)(target, attachment, textarget, texture, level);
    }
}

GLvoid GLAPIENTRY __glProfile_FrontFace(__GLcontext *gc, GLenum mode)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glFrontFace 0x%04X\n",
                        gc, tid, mode);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->FrontFace(gc, mode);
    __GL_PROFILE_FOOTER(GL4_FRONTFACE);

    if (__glTracerDispatchTable.FrontFace)
    {
        (*__glTracerDispatchTable.FrontFace)(mode);
    }
}

GLvoid GLAPIENTRY __glProfile_GenBuffers(__GLcontext *gc, GLsizei n, GLuint* buffers)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGenBuffers %d\n",
                        gc, tid, n);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GenBuffers(gc, n, buffers);
    __GL_PROFILE_FOOTER(GL4_GENBUFFERS);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGenBuffers => ");
        __glLogArrayData(gc, n, buffers);
    }

    if (__glTracerDispatchTable.GenBuffers)
    {
        (*__glTracerDispatchTable.GenBuffers)(n, buffers);
    }
}

GLvoid GLAPIENTRY __glProfile_GenerateMipmap(__GLcontext *gc, GLenum target)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGenerateMipmap 0x%04X\n",
                        gc, tid, target);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GenerateMipmap(gc, target);
    __GL_PROFILE_FOOTER(GL4_GENERATEMIPMAP);

    if (__glTracerDispatchTable.GenerateMipmap)
    {
        (*__glTracerDispatchTable.GenerateMipmap)(target);
    }
}

GLvoid GLAPIENTRY __glProfile_GenFramebuffers(__GLcontext *gc, GLsizei n, GLuint* framebuffers)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGenFramebuffers %d\n",
                        gc, tid, n);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GenFramebuffers(gc, n, framebuffers);
    __GL_PROFILE_FOOTER(GL4_GENFRAMEBUFFERS);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGenFramebuffers => ");
        __glLogArrayData(gc, n, framebuffers);
    }

    if (__glTracerDispatchTable.GenFramebuffers)
    {
        (*__glTracerDispatchTable.GenFramebuffers)(n, framebuffers);
    }
}

GLvoid GLAPIENTRY __glProfile_GenRenderbuffers(__GLcontext *gc, GLsizei n, GLuint* renderbuffers)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGenRenderbuffers %d\n",
                        gc, tid, n);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GenRenderbuffers(gc, n, renderbuffers);
    __GL_PROFILE_FOOTER(GL4_GENRENDERBUFFERS);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGenRenderbuffers => ");
        __glLogArrayData(gc, n, renderbuffers);
    }

    if (__glTracerDispatchTable.GenRenderbuffers)
    {
        (*__glTracerDispatchTable.GenRenderbuffers)(n, renderbuffers);
    }
}

GLvoid GLAPIENTRY __glProfile_GenTextures(__GLcontext *gc, GLsizei n, GLuint* textures)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGenTextures %d\n",
                        gc, tid, n);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GenTextures(gc, n, textures);
    __GL_PROFILE_FOOTER(GL4_GENTEXTURES);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGenTextures => ");
        __glLogArrayData(gc, n, textures);
    }

    if (__glTracerDispatchTable.GenTextures)
    {
        (*__glTracerDispatchTable.GenTextures)(n, textures);
    }
}

GLvoid GLAPIENTRY __glProfile_GetActiveAttrib(__GLcontext *gc, GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetActiveAttrib %d %d %d\n",
                        gc, tid, program, index, bufsize);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetActiveAttrib(gc, program, index, bufsize, length, size, type, name);
    __GL_PROFILE_FOOTER(GL4_GETACTIVEATTRIB);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetActiveAttrib => %d %d 0x%04X %s\n",
                        __GL_PTRVALUE(length), __GL_PTRVALUE(size), __GL_PTRVALUE(type), name);
    }

    if (__glTracerDispatchTable.GetActiveAttrib)
    {
        (*__glTracerDispatchTable.GetActiveAttrib)(program, index, bufsize, length, size, type, name);
    }
}

GLvoid GLAPIENTRY __glProfile_GetActiveUniform(__GLcontext *gc, GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetActiveUniform %d %d %d\n",
            gc, tid, program, index, bufsize);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetActiveUniform(gc, program, index, bufsize, length, size, type, name);
    __GL_PROFILE_FOOTER(GL4_GETACTIVEUNIFORM);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetActiveUniform => %d %d 0x%04X %s\n",
                        __GL_PTRVALUE(length), __GL_PTRVALUE(size), __GL_PTRVALUE(type), name);
    }

    if (__glTracerDispatchTable.GetActiveUniform)
    {
        (*__glTracerDispatchTable.GetActiveUniform)(program, index, bufsize, length, size, type, name);
    }
}

GLvoid GLAPIENTRY __glProfile_GetAttachedShaders(__GLcontext *gc, GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetAttachedShaders %d %d\n",
                        gc, tid, program, maxcount);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetAttachedShaders(gc, program, maxcount, count, shaders);
    __GL_PROFILE_FOOTER(GL4_GETATTACHEDSHADERS);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetAttachedShaders => %d 0x%08X\n",
                        __GL_PTRVALUE(count), __GL_PTR2UINT(shaders));
    }

    if (__glTracerDispatchTable.GetAttachedShaders)
    {
        (*__glTracerDispatchTable.GetAttachedShaders)(program, maxcount, count, shaders);
    }
}

GLint GLAPIENTRY __glProfile_GetAttribLocation(__GLcontext *gc, GLuint program, const GLchar* name)
{
    __GL_PROFILE_VARS();
    GLint location;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetAttribLocation %d %s\n",
                        gc, tid, program, name);
    }

    __GL_PROFILE_HEADER();
    location = gc->pModeDispatch->GetAttribLocation(gc, program, name);
    __GL_PROFILE_FOOTER(GL4_GETATTRIBLOCATION);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetAttribLocation => %d\n", location);
    }

    if (__glTracerDispatchTable.GetAttribLocation)
    {
        (*__glTracerDispatchTable.GetAttribLocation)(program, name, location);
    }

    return location;
}

GLvoid GLAPIENTRY __glProfile_GetBooleanv(__GLcontext *gc, GLenum pname, GLboolean* params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetBooleanv 0x%04X\n",
                        gc, tid, pname);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetBooleanv(gc, pname, params);
    __GL_PROFILE_FOOTER(GL4_GETBOOLEANV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetBooleanv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glTracerDispatchTable.GetBooleanv)
    {
        (*__glTracerDispatchTable.GetBooleanv)(pname, params);
    }
}

GLvoid GLAPIENTRY __glProfile_GetBufferParameteriv(__GLcontext *gc, GLenum target, GLenum pname, GLint* params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetBufferParameteriv 0x%04X 0x%04X\n",
                        gc, tid, target, pname);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetBufferParameteriv(gc, target, pname, params);
    __GL_PROFILE_FOOTER(GL4_GETBUFFERPARAMETERIV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetBufferParameteriv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glTracerDispatchTable.GetBufferParameteriv)
    {
        (*__glTracerDispatchTable.GetBufferParameteriv)(target, pname, params);
    }
}

GLenum GLAPIENTRY __glProfile_GetError(__GLcontext *gc)
{
    __GL_PROFILE_VARS();
    GLenum error;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetError\n",
                        gc, tid);
    }

    __GL_PROFILE_HEADER();
    error = gc->pModeDispatch->GetError(gc);
    __GL_PROFILE_FOOTER(GL4_GETERROR);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetError => 0x%04X\n", error);
    }

    if (__glTracerDispatchTable.GetError)
    {
        (*__glTracerDispatchTable.GetError)();
    }

    return error;
}

GLvoid GLAPIENTRY __glProfile_GetFloatv(__GLcontext *gc, GLenum pname, GLfloat* params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetFloatv 0x%04X\n",
                        gc, tid, pname);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetFloatv(gc, pname, params);
    __GL_PROFILE_FOOTER(GL4_GETFLOATV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetFloatv => %f\n", __GL_PTRVALUE(params));
    }

    if (__glTracerDispatchTable.GetFloatv)
    {
        (*__glTracerDispatchTable.GetFloatv)(pname, params);
    }
}

GLvoid GLAPIENTRY __glProfile_GetFramebufferAttachmentParameteriv(__GLcontext *gc, GLenum target, GLenum attachment, GLenum pname, GLint* params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetFramebufferAttachmentParameteriv 0x%04X 0x%04X 0x%04X\n",
                        gc, tid, target, attachment, pname);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetFramebufferAttachmentParameteriv(gc, target, attachment, pname, params);
    __GL_PROFILE_FOOTER(GL4_GETFRAMEBUFFERATTACHMENTPARAMETERIV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetFramebufferAttachmentParameteriv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glTracerDispatchTable.GetFramebufferAttachmentParameteriv)
    {
        (*__glTracerDispatchTable.GetFramebufferAttachmentParameteriv)(target, attachment, pname, params);
    }
}

GLvoid GLAPIENTRY __glProfile_GetIntegerv(__GLcontext *gc, GLenum pname, GLint* params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetIntegerv 0x%04X\n",
                        gc, tid, pname);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetIntegerv(gc, pname, params);
    __GL_PROFILE_FOOTER(GL4_GETINTEGERV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetIntegerv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glTracerDispatchTable.GetIntegerv)
    {
        (*__glTracerDispatchTable.GetIntegerv)(pname, params);
    }
}

GLvoid GLAPIENTRY __glProfile_GetProgramiv(__GLcontext *gc, GLuint program, GLenum pname, GLint* params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetProgramiv %d 0x%04X\n",
                        gc, tid, program, pname);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetProgramiv(gc, program, pname, params);
    __GL_PROFILE_FOOTER(GL4_GETPROGRAMIV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetProgramiv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glTracerDispatchTable.GetProgramiv)
    {
        (*__glTracerDispatchTable.GetProgramiv)(program, pname, params);
    }
}

GLvoid GLAPIENTRY __glProfile_GetProgramInfoLog(__GLcontext *gc, GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetProgramInfoLog %d %d\n",
                        gc, tid, program, bufsize);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetProgramInfoLog(gc, program, bufsize, length, infolog);
    __GL_PROFILE_FOOTER(GL4_GETPROGRAMINFOLOG);

    if (bufsize && (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST))
    {
        __GL_LOG_API("        glGetProgramInfoLog => %d %s\n", __GL_PTRVALUE(length), infolog);
    }

    if (__glTracerDispatchTable.GetProgramInfoLog)
    {
        (*__glTracerDispatchTable.GetProgramInfoLog)(program, bufsize, length, infolog);
    }
}

GLvoid GLAPIENTRY __glProfile_GetRenderbufferParameteriv(__GLcontext *gc, GLenum target, GLenum pname, GLint* params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetRenderbufferParameteriv 0x%04X 0x%04X\n",
                        gc, tid, target, pname);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetRenderbufferParameteriv(gc, target, pname, params);
    __GL_PROFILE_FOOTER(GL4_GETRENDERBUFFERPARAMETERIV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetRenderbufferParameteriv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glTracerDispatchTable.GetRenderbufferParameteriv)
    {
        (*__glTracerDispatchTable.GetRenderbufferParameteriv)(target, pname, params);
    }
}

GLvoid GLAPIENTRY __glProfile_GetShaderiv(__GLcontext *gc, GLuint shader, GLenum pname, GLint* params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetShaderiv 0x%04X 0x%04X\n",
                         gc, tid, shader, pname);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetShaderiv(gc, shader, pname, params);
    __GL_PROFILE_FOOTER(GL4_GETSHADERIV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetShaderiv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glTracerDispatchTable.GetShaderiv)
    {
        (*__glTracerDispatchTable.GetShaderiv)(shader, pname, params);
    }
}

GLvoid GLAPIENTRY __glProfile_GetShaderInfoLog(__GLcontext *gc, GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetShaderInfoLog %d %d\n",
                        gc, tid, shader, bufsize);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetShaderInfoLog(gc, shader, bufsize, length, infolog);
    __GL_PROFILE_FOOTER(GL4_GETSHADERINFOLOG);

    if (bufsize && (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST))
    {
        __GL_LOG_API("        glGetShaderInfoLog => %d %s\n", __GL_PTRVALUE(length), infolog);
    }

    if (__glTracerDispatchTable.GetShaderInfoLog)
    {
        (*__glTracerDispatchTable.GetShaderInfoLog)(shader, bufsize, length, infolog);
    }
}

GLvoid GLAPIENTRY __glProfile_GetShaderPrecisionFormat(__GLcontext *gc, GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetShaderPrecisionFormat 0x%04X 0x%04X\n",
                        gc, tid, shadertype, precisiontype);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetShaderPrecisionFormat(gc, shadertype, precisiontype, range, precision);
    __GL_PROFILE_FOOTER(GL4_GETSHADERPRECISIONFORMAT);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetShaderPrecisionFormat => %d %d\n", __GL_PTRVALUE(range), __GL_PTRVALUE(precision));
    }

    if (__glTracerDispatchTable.GetShaderPrecisionFormat)
    {
        (*__glTracerDispatchTable.GetShaderPrecisionFormat)(shadertype, precisiontype, range, precision);
    }
}

GLvoid GLAPIENTRY __glProfile_GetShaderSource(__GLcontext *gc, GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetShaderSource %d %d\n",
                        gc, tid, shader, bufsize);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetShaderSource(gc, shader, bufsize, length, source);
    __GL_PROFILE_FOOTER(GL4_GETSHADERSOURCE);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetShaderSource => %d\n####\n%s\n####\n", __GL_PTRVALUE(length), source);
    }

    if (__glTracerDispatchTable.GetShaderSource)
    {
        (*__glTracerDispatchTable.GetShaderSource)(shader, bufsize, length, source);
    }
}

const GLubyte* GLAPIENTRY __glProfile_GetString(__GLcontext *gc, GLenum name)
{
    __GL_PROFILE_VARS();
    const GLubyte *string;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetString 0x%04X\n",
                        gc, tid, name);
    }

    __GL_PROFILE_HEADER();
    string = gc->pModeDispatch->GetString(gc, name);
    __GL_PROFILE_FOOTER(GL4_GETSTRING);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetString => %s\n", string);
    }

    if (__glTracerDispatchTable.GetString)
    {
        (*__glTracerDispatchTable.GetString)(name);
    }

    return string;
}

GLvoid GLAPIENTRY __glProfile_GetTexParameterfv(__GLcontext *gc, GLenum target, GLenum pname, GLfloat* params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetTexParameterfv 0x%04X 0x%04X\n",
                        gc, tid, target, pname);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetTexParameterfv(gc, target, pname, params);
    __GL_PROFILE_FOOTER(GL4_GETTEXPARAMETERFV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetTexParameterfv => %f\n", __GL_PTRVALUE(params));
    }

    if (__glTracerDispatchTable.GetTexParameterfv)
    {
        (*__glTracerDispatchTable.GetTexParameterfv)(target, pname, params);
    }
}

GLvoid GLAPIENTRY __glProfile_GetTexParameteriv(__GLcontext *gc, GLenum target, GLenum pname, GLint* params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetTexParameteriv 0x%04X 0x%04X\n",
                        gc, tid, target, pname);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetTexParameteriv(gc, target, pname, params);
    __GL_PROFILE_FOOTER(GL4_GETTEXPARAMETERIV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetTexParameteriv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glTracerDispatchTable.GetTexParameteriv)
    {
        (*__glTracerDispatchTable.GetTexParameteriv)(target, pname, params);
    }
}

GLvoid GLAPIENTRY __glProfile_GetUniformfv(__GLcontext *gc, GLuint program, GLint location, GLfloat* params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetUniformfv %d %d\n",
                        gc, tid, program, location);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetUniformfv(gc, program, location, params);
    __GL_PROFILE_FOOTER(GL4_GETUNIFORMFV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetUniformfv => %f\n", __GL_PTRVALUE(params));
    }

    if (__glTracerDispatchTable.GetUniformfv)
    {
        (*__glTracerDispatchTable.GetUniformfv)(program, location, params);
    }
}

GLvoid GLAPIENTRY __glProfile_GetUniformiv(__GLcontext *gc, GLuint program, GLint location, GLint* params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetUniformiv %d %d\n",
                        gc, tid, program, location);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetUniformiv(gc, program, location, params);
    __GL_PROFILE_FOOTER(GL4_GETUNIFORMIV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetUniformiv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glTracerDispatchTable.GetUniformiv)
    {
        (*__glTracerDispatchTable.GetUniformiv)(program, location, params);
    }
}

GLint GLAPIENTRY __glProfile_GetUniformLocation(__GLcontext *gc, GLuint program, const GLchar* name)
{
    __GL_PROFILE_VARS();
    GLint location;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetUniformLocation %d %s\n",
                        gc, tid, program, name);
    }

    __GL_PROFILE_HEADER();
    location = gc->pModeDispatch->GetUniformLocation(gc, program, name);
    __GL_PROFILE_FOOTER(GL4_GETUNIFORMLOCATION);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetUniformLocation => %d\n", location);
    }

    if (__glTracerDispatchTable.GetUniformLocation)
    {
        (*__glTracerDispatchTable.GetUniformLocation)(program, name, location);
    }

    return location;
}

GLvoid GLAPIENTRY __glProfile_GetVertexAttribfv(__GLcontext *gc, GLuint index, GLenum pname, GLfloat* params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetVertexAttribfv %d 0x%04X\n",
                        gc, tid, index, pname);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetVertexAttribfv(gc, index, pname, params);
    __GL_PROFILE_FOOTER(GL4_GETVERTEXATTRIBFV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetVertexAttribfv => %f\n", __GL_PTRVALUE(params));
    }

    if (__glTracerDispatchTable.GetVertexAttribfv)
    {
        (*__glTracerDispatchTable.GetVertexAttribfv)(index, pname, params);
    }
}

GLvoid GLAPIENTRY __glProfile_GetVertexAttribiv(__GLcontext *gc, GLuint index, GLenum pname, GLint* params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetVertexAttribiv %d 0x%04X\n",
                        gc, tid, index, pname);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetVertexAttribiv(gc, index, pname, params);
    __GL_PROFILE_FOOTER(GL4_GETVERTEXATTRIBIV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetVertexAttribiv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glTracerDispatchTable.GetVertexAttribiv)
    {
        (*__glTracerDispatchTable.GetVertexAttribiv)(index, pname, params);
    }
}

GLvoid GLAPIENTRY __glProfile_GetVertexAttribPointerv(__GLcontext *gc, GLuint index, GLenum pname, GLvoid** pointer)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetVertexAttribPointerv %d 0x%04X\n",
                        gc, tid, index, pname);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetVertexAttribPointerv(gc, index, pname, pointer);
    __GL_PROFILE_FOOTER(GL4_GETVERTEXATTRIBPOINTERV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetVertexAttribPointerv => 0x%08X\n", __GL_PTR2UINT(__GL_PTRVALUE(pointer)));
    }

    if (__glTracerDispatchTable.GetVertexAttribPointerv)
    {
        (*__glTracerDispatchTable.GetVertexAttribPointerv)(index, pname, pointer);
    }
}

GLvoid GLAPIENTRY __glProfile_Hint(__GLcontext *gc, GLenum target, GLenum mode)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glHint 0x%04X 0x%04X\n",
                        gc, tid, target, mode);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Hint(gc, target, mode);
    __GL_PROFILE_FOOTER(GL4_HINT);

    if (__glTracerDispatchTable.Hint)
    {
        (*__glTracerDispatchTable.Hint)(target, mode);
    }
}

GLboolean GLAPIENTRY __glProfile_IsBuffer(__GLcontext *gc, GLuint buffer)
{
    __GL_PROFILE_VARS();
    GLboolean is;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glIsBuffer %d\n",
                        gc, tid, buffer);
    }

    __GL_PROFILE_HEADER();
    is = gc->pModeDispatch->IsBuffer(gc, buffer);
    __GL_PROFILE_FOOTER(GL4_ISBUFFER);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glIsBuffer => %d\n", is);
    }

    if (__glTracerDispatchTable.IsBuffer)
    {
        (*__glTracerDispatchTable.IsBuffer)(buffer);
    }

    return is;
}

GLboolean GLAPIENTRY __glProfile_IsEnabled(__GLcontext *gc, GLenum cap)
{
    __GL_PROFILE_VARS();
    GLboolean is;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glIsEnabled 0x%04X\n",
                        gc, tid, cap);
    }

    __GL_PROFILE_HEADER();
    is = gc->pModeDispatch->IsEnabled(gc, cap);
    __GL_PROFILE_FOOTER(GL4_ISENABLED);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glIsEnabled => %d\n", is);
    }

    if (__glTracerDispatchTable.IsEnabled)
    {
        (*__glTracerDispatchTable.IsEnabled)(cap);
    }

    return is;
}

GLboolean GLAPIENTRY __glProfile_IsFramebuffer(__GLcontext *gc, GLuint framebuffer)
{
    __GL_PROFILE_VARS();
    GLboolean is;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glIsFramebuffer %d\n",
                        gc, tid, framebuffer);
    }

    __GL_PROFILE_HEADER();
    is = gc->pModeDispatch->IsFramebuffer(gc, framebuffer);
    __GL_PROFILE_FOOTER(GL4_ISFRAMEBUFFER);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glIsFramebuffer => %d\n", is);
    }

    if (__glTracerDispatchTable.IsFramebuffer)
    {
        (*__glTracerDispatchTable.IsFramebuffer)(framebuffer);
    }

    return is;
}

GLboolean GLAPIENTRY __glProfile_IsProgram(__GLcontext *gc, GLuint program)
{
    __GL_PROFILE_VARS();
    GLboolean is;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glIsProgram %d\n",
                        gc, tid, program);
    }

    __GL_PROFILE_HEADER();
    is = gc->pModeDispatch->IsProgram(gc, program);
    __GL_PROFILE_FOOTER(GL4_ISPROGRAM);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glIsProgram => %d\n", is);
    }

    if (__glTracerDispatchTable.IsProgram)
    {
        (*__glTracerDispatchTable.IsProgram)(program);
    }

    return is;
}

GLboolean GLAPIENTRY __glProfile_IsRenderbuffer(__GLcontext *gc, GLuint renderbuffer)
{
    __GL_PROFILE_VARS();
    GLboolean is;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glIsRenderbuffer %d\n",
                        gc, tid, renderbuffer);
    }

    __GL_PROFILE_HEADER();
    is = gc->pModeDispatch->IsRenderbuffer(gc, renderbuffer);
    __GL_PROFILE_FOOTER(GL4_ISRENDERBUFFER);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glIsRenderbuffer => %d\n", is);
    }

    if (__glTracerDispatchTable.IsRenderbuffer)
    {
        (*__glTracerDispatchTable.IsRenderbuffer)(renderbuffer);
    }

    return is;
}

GLboolean GLAPIENTRY __glProfile_IsShader(__GLcontext *gc, GLuint shader)
{
    __GL_PROFILE_VARS();
    GLboolean is;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glIsShader %d\n",
                        gc, tid, shader);
    }

    __GL_PROFILE_HEADER();
    is = gc->pModeDispatch->IsShader(gc, shader);
    __GL_PROFILE_FOOTER(GL4_ISSHADER);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glIsShader => %d\n", is);
    }

    if (__glTracerDispatchTable.IsShader)
    {
        (*__glTracerDispatchTable.IsShader)(shader);
    }

    return is;
}

GLboolean GLAPIENTRY __glProfile_IsTexture(__GLcontext *gc, GLuint texture)
{
    __GL_PROFILE_VARS();
    GLboolean is;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glIsTexture %d\n",
                        gc, tid, texture);
    }

    __GL_PROFILE_HEADER();
    is = gc->pModeDispatch->IsTexture(gc, texture);
    __GL_PROFILE_FOOTER(GL4_ISTEXTURE);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glIsTexture => %d\n", is);
    }

    if (__glTracerDispatchTable.IsTexture)
    {
        (*__glTracerDispatchTable.IsTexture)(texture);
    }

    return is;
}

GLvoid GLAPIENTRY __glProfile_LineWidth(__GLcontext *gc, GLfloat width)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glLineWidth %f\n",
                        gc, tid, width);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->LineWidth(gc, width);
    __GL_PROFILE_FOOTER(GL4_LINEWIDTH);

    if (__glTracerDispatchTable.LineWidth)
    {
        (*__glTracerDispatchTable.LineWidth)(width);
    }
}

GLvoid GLAPIENTRY __glProfile_LinkProgram(__GLcontext *gc, GLuint program)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glLinkProgram %d\n",
                        gc, tid, program);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->LinkProgram(gc, program);
    __GL_PROFILE_FOOTER(GL4_LINKPROGRAM);

    if (__glTracerDispatchTable.LinkProgram)
    {
        (*__glTracerDispatchTable.LinkProgram)(program);
    }
}

GLvoid GLAPIENTRY __glProfile_PixelStorei(__GLcontext *gc, GLenum pname, GLint param)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glPixelStorei 0x%04X %d\n",
                        gc, tid, pname, param);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->PixelStorei(gc, pname, param);
    __GL_PROFILE_FOOTER(GL4_PIXELSTOREI);

    if (__glTracerDispatchTable.PixelStorei)
    {
        (*__glTracerDispatchTable.PixelStorei)(pname, param);
    }
}

GLvoid GLAPIENTRY __glProfile_PolygonOffset(__GLcontext *gc, GLfloat factor, GLfloat units)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glPolygonOffset %f %f\n",
                        gc, tid, factor, units);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->PolygonOffset(gc, factor, units);
    __GL_PROFILE_FOOTER(GL4_POLYGONOFFSET);

    if (__glTracerDispatchTable.PolygonOffset)
    {
        (*__glTracerDispatchTable.PolygonOffset)(factor, units);
    }
}

GLvoid GLAPIENTRY __glProfile_ReadPixels(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glReadPixels %d %d %d %d 0x%04X 0x%04X 0x%08X\n",
                        gc, tid, x, y, width, height, format, type, __GL_PTR2UINT(pixels));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ReadPixels(gc, x, y, width, height, format, type, pixels);
    __GL_PROFILE_FOOTER(GL4_READPIXELS);

    if (__glTracerDispatchTable.ReadPixels)
    {
        (*__glTracerDispatchTable.ReadPixels)(x, y, width, height, format, type, pixels);
    }
}

GLvoid GLAPIENTRY __glProfile_ReleaseShaderCompiler(__GLcontext *gc)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glReleaseShaderCompiler\n", gc, tid);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ReleaseShaderCompiler(gc);
    __GL_PROFILE_FOOTER(GL4_RELEASESHADERCOMPILER);

    if (__glTracerDispatchTable.ReleaseShaderCompiler)
    {
        (*__glTracerDispatchTable.ReleaseShaderCompiler)();
    }
}

GLvoid GLAPIENTRY __glProfile_RenderbufferStorage(__GLcontext *gc, GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glRenderbufferStorage 0x%04X 0x%04X %d %d\n",
                        gc, tid, target, internalformat, width, height);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->RenderbufferStorage(gc, target, internalformat, width, height);
    __GL_PROFILE_FOOTER(GL4_RENDERBUFFERSTORAGE);

    if (__glTracerDispatchTable.RenderbufferStorage)
    {
        (*__glTracerDispatchTable.RenderbufferStorage)(target, internalformat, width, height);
    }
}

GLvoid GLAPIENTRY __glProfile_SampleCoverage(__GLcontext *gc, GLfloat value, GLboolean invert)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glSampleCoverage %f %d\n",
                        gc, tid, value, invert);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->SampleCoverage(gc, value, invert);
    __GL_PROFILE_FOOTER(GL4_SAMPLECOVERAGE);

    if (__glTracerDispatchTable.SampleCoverage)
    {
        (*__glTracerDispatchTable.SampleCoverage)(value, invert);
    }
}

GLvoid GLAPIENTRY __glProfile_Scissor(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glScissor %d %d %d %d\n",
                        gc, tid, x, y, width, height);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Scissor(gc, x, y, width, height);
    __GL_PROFILE_FOOTER(GL4_SCISSOR);

    if (__glTracerDispatchTable.Scissor)
    {
        (*__glTracerDispatchTable.Scissor)(x, y, width, height);
    }
}

GLvoid GLAPIENTRY __glProfile_ShaderBinary(__GLcontext *gc, GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glShaderBinary %d 0x%08X 0x%04X 0x%08X %d\n",
                        gc, tid, n, __GL_PTR2UINT(shaders), binaryformat, __GL_PTR2UINT(binary), length);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ShaderBinary(gc, n, shaders, binaryformat, binary, length);
    __GL_PROFILE_FOOTER(GL4_SHADERBINARY);

    if (__glTracerDispatchTable.ShaderBinary)
    {
        (*__glTracerDispatchTable.ShaderBinary)(n, shaders, binaryformat, binary, length);
    }
}

GLvoid GLAPIENTRY __glProfile_ShaderSource(__GLcontext *gc, GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glShaderSource %u %d 0x%p 0x%08X \n",
                        gc, tid, shader, count, __GL_PTR2UINT(string), __GL_PTRVALUE(length));

        __glLogSourceStrings(gc, count, string);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ShaderSource(gc, shader, count, string, length);
    __GL_PROFILE_FOOTER(GL4_SHADERSOURCE);

    if (__glTracerDispatchTable.ShaderSource)
    {
        (*__glTracerDispatchTable.ShaderSource)(shader, count, string, length);
    }
}

GLvoid GLAPIENTRY __glProfile_StencilFunc(__GLcontext *gc, GLenum func, GLint ref, GLuint mask)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glStencilFunc 0x%04X %d 0x%08X\n",
                        gc, tid, func, ref, mask);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->StencilFunc(gc, func, ref, mask);
    __GL_PROFILE_FOOTER(GL4_STENCILFUNC);

    if (__glTracerDispatchTable.StencilFunc)
    {
        (*__glTracerDispatchTable.StencilFunc)(func, ref, mask);
    }
}

GLvoid GLAPIENTRY __glProfile_StencilFuncSeparate(__GLcontext *gc, GLenum face, GLenum func, GLint ref, GLuint mask)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glStencilFuncSeparate 0x%04X 0x%04X %d 0x%08X\n",
                        gc, tid, face, func, ref, mask);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->StencilFuncSeparate(gc, face, func, ref, mask);
    __GL_PROFILE_FOOTER(GL4_STENCILFUNCSEPARATE);

    if (__glTracerDispatchTable.StencilFuncSeparate)
    {
        (*__glTracerDispatchTable.StencilFuncSeparate)(face, func, ref, mask);
    }
}

GLvoid GLAPIENTRY __glProfile_StencilMask(__GLcontext *gc, GLuint mask)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glStencilMask 0x%08X\n",
                        gc, tid, mask);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->StencilMask(gc, mask);
    __GL_PROFILE_FOOTER(GL4_STENCILMASK);

    if (__glTracerDispatchTable.StencilMask)
    {
        (*__glTracerDispatchTable.StencilMask)(mask);
    }
}

GLvoid GLAPIENTRY __glProfile_StencilMaskSeparate(__GLcontext *gc, GLenum face, GLuint mask)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glStencilMaskSeparate 0x%04X 0x%08X\n",
                        gc, tid, face, mask);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->StencilMaskSeparate(gc, face, mask);
    __GL_PROFILE_FOOTER(GL4_STENCILMASKSEPARATE);

    if (__glTracerDispatchTable.StencilMaskSeparate)
    {
        (*__glTracerDispatchTable.StencilMaskSeparate)(face, mask);
    }
}

GLvoid GLAPIENTRY __glProfile_StencilOp(__GLcontext *gc, GLenum fail, GLenum zfail, GLenum zpass)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glStencilOp 0x%04X 0x%04X 0x%04X\n",
                        gc, tid, fail, zfail, zpass);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->StencilOp(gc, fail, zfail, zpass);
    __GL_PROFILE_FOOTER(GL4_STENCILOP);

    if (__glTracerDispatchTable.StencilOp)
    {
        (*__glTracerDispatchTable.StencilOp)(fail, zfail, zpass);
    }
}

GLvoid GLAPIENTRY __glProfile_StencilOpSeparate(__GLcontext *gc, GLenum face, GLenum fail, GLenum zfail, GLenum zpass)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glStencilOpSeparate 0x%04X 0x%04X 0x%04X 0x%04X\n",
                        gc, tid, face, fail, zfail, zpass);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->StencilOpSeparate(gc, face, fail, zfail, zpass);
    __GL_PROFILE_FOOTER(GL4_STENCILOPSEPARATE);

    if (__glTracerDispatchTable.StencilOpSeparate)
    {
        (*__glTracerDispatchTable.StencilOpSeparate)(face, fail, zfail, zpass);
    }
}

GLvoid GLAPIENTRY __glProfile_TexImage2D(__GLcontext *gc, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexImage2D 0x%04X %d 0x%04X %d %d %d 0x%04X 0x%04X 0x%08X\n",
                        gc, tid, target, level, internalformat, width, height, border, format, type, __GL_PTR2UINT(pixels));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexImage2D(gc, target, level, internalformat, width, height, border, format, type, pixels);
    __GL_PROFILE_FOOTER(GL4_TEXIMAGE2D);

    if (__glTracerDispatchTable.TexImage2D)
    {
        (*__glTracerDispatchTable.TexImage2D)(target, level, internalformat, width, height, border, format, type, pixels);
    }
}

GLvoid GLAPIENTRY __glProfile_TexParameterf(__GLcontext *gc, GLenum target, GLenum pname, GLfloat param)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexParameterf 0x%04X 0x%04X %f\n",
                        gc, tid, target, pname, param);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexParameterf(gc, target, pname, param);
    __GL_PROFILE_FOOTER(GL4_TEXPARAMETERF);

    if (__glTracerDispatchTable.TexParameterf)
    {
        (*__glTracerDispatchTable.TexParameterf)(target, pname, param);
    }
}

GLvoid GLAPIENTRY __glProfile_TexParameterfv(__GLcontext *gc, GLenum target, GLenum pname, const GLfloat* params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexParameterfv 0x%04X 0x%04X %f\n",
                        gc, tid, target, pname, __GL_PTRVALUE(params));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexParameterfv(gc, target, pname, params);
    __GL_PROFILE_FOOTER(GL4_TEXPARAMETERFV);

    if (__glTracerDispatchTable.TexParameterfv)
    {
        (*__glTracerDispatchTable.TexParameterfv)(target, pname, params);
    }
}

GLvoid GLAPIENTRY __glProfile_TexParameteri(__GLcontext *gc, GLenum target, GLenum pname, GLint param)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexParameteri 0x%04X 0x%04X %d\n",
                        gc, tid, target, pname, param);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexParameteri(gc, target, pname, param);
    __GL_PROFILE_FOOTER(GL4_TEXPARAMETERI);

    if (__glTracerDispatchTable.TexParameteri)
    {
        (*__glTracerDispatchTable.TexParameteri)(target, pname, param);
    }
}

GLvoid GLAPIENTRY __glProfile_TexParameteriv(__GLcontext *gc, GLenum target, GLenum pname, const GLint* params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexParameteriv 0x%04X 0x%04X %d\n",
                        gc, tid, target, pname, __GL_PTRVALUE(params));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexParameteriv(gc, target, pname, params);
    __GL_PROFILE_FOOTER(GL4_TEXPARAMETERIV);

    if (__glTracerDispatchTable.TexParameteriv)
    {
        (*__glTracerDispatchTable.TexParameteriv)(target, pname, params);
    }
}

GLvoid GLAPIENTRY __glProfile_TexSubImage2D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexSubImage2D 0x%04X %d %d %d %d %d 0x%04X 0x%04X 0x%08X\n",
                        gc, tid, target, level, xoffset, yoffset, width, height, format, type, __GL_PTR2UINT(pixels));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexSubImage2D(gc, target, level, xoffset, yoffset, width, height, format, type, pixels);
    __GL_PROFILE_FOOTER(GL4_TEXSUBIMAGE2D);

    if (__glTracerDispatchTable.TexSubImage2D)
    {
        (*__glTracerDispatchTable.TexSubImage2D)(target, level, xoffset, yoffset, width, height, format, type, pixels);
    }
}

GLvoid GLAPIENTRY __glProfile_Uniform1f(__GLcontext *gc, GLint location, GLfloat x)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniform1f %d %f\n",
                        gc, tid, location, x);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Uniform1f(gc, location, x);
    __GL_PROFILE_FOOTER(GL4_UNIFORM1F);

    if (__glTracerDispatchTable.Uniform1f)
    {
        (*__glTracerDispatchTable.Uniform1f)(location, x);
    }
}

GLvoid GLAPIENTRY __glProfile_Uniform1fv(__GLcontext *gc, GLint location, GLsizei count, const GLfloat* v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniform1fv %d %d 0x%08X\n",
                        gc, tid, location, count, __GL_PTR2UINT(v));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Uniform1fv(gc, location, count, v);
    __GL_PROFILE_FOOTER(GL4_UNIFORM1FV);

    if (__glTracerDispatchTable.Uniform1fv)
    {
        (*__glTracerDispatchTable.Uniform1fv)(location, count, v);
    }
}

GLvoid GLAPIENTRY __glProfile_Uniform1i(__GLcontext *gc, GLint location, GLint x)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniform1i %d %d\n",
                        gc, tid, location, x);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Uniform1i(gc, location, x);
    __GL_PROFILE_FOOTER(GL4_UNIFORM1I);

    if (__glTracerDispatchTable.Uniform1i)
    {
        (*__glTracerDispatchTable.Uniform1i)(location, x);
    }
}

GLvoid GLAPIENTRY __glProfile_Uniform1iv(__GLcontext *gc, GLint location, GLsizei count, const GLint* v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniform1iv %d %d 0x%08X\n",
                        gc, tid, location, count, __GL_PTR2UINT(v));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Uniform1iv(gc, location, count, v);
    __GL_PROFILE_FOOTER(GL4_UNIFORM1IV);

    if (__glTracerDispatchTable.Uniform1iv)
    {
        (*__glTracerDispatchTable.Uniform1iv)(location, count, v);
    }
}

GLvoid GLAPIENTRY __glProfile_Uniform2f(__GLcontext *gc, GLint location, GLfloat x, GLfloat y)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniform2f %d %f %f\n",
                        gc, tid, location, x, y);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Uniform2f(gc, location, x, y);
    __GL_PROFILE_FOOTER(GL4_UNIFORM2F);

    if (__glTracerDispatchTable.Uniform2f)
    {
        (*__glTracerDispatchTable.Uniform2f)(location, x, y);
    }
}

GLvoid GLAPIENTRY __glProfile_Uniform2fv(__GLcontext *gc, GLint location, GLsizei count, const GLfloat* v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniform2fv %d %d 0x%08X\n",
                        gc, tid, location, count, __GL_PTR2UINT(v));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Uniform2fv(gc, location, count, v);
    __GL_PROFILE_FOOTER(GL4_UNIFORM2FV);

    if (__glTracerDispatchTable.Uniform2fv)
    {
        (*__glTracerDispatchTable.Uniform2fv)(location, count, v);
    }
}

GLvoid GLAPIENTRY __glProfile_Uniform2i(__GLcontext *gc, GLint location, GLint x, GLint y)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniform2i %d %d %d\n",
                        gc, tid, location, x, y);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Uniform2i(gc, location, x, y);
    __GL_PROFILE_FOOTER(GL4_UNIFORM2I);

    if (__glTracerDispatchTable.Uniform2i)
    {
        (*__glTracerDispatchTable.Uniform2i)(location, x, y);
    }
}

GLvoid GLAPIENTRY __glProfile_Uniform2iv(__GLcontext *gc, GLint location, GLsizei count, const GLint* v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniform2iv %d %d 0x%08X\n",
                        gc, tid, location, count, __GL_PTR2UINT(v));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Uniform2iv(gc, location, count, v);
    __GL_PROFILE_FOOTER(GL4_UNIFORM2IV);

    if (__glTracerDispatchTable.Uniform2iv)
    {
        (*__glTracerDispatchTable.Uniform2iv)(location, count, v);
    }
}

GLvoid GLAPIENTRY __glProfile_Uniform3f(__GLcontext *gc, GLint location, GLfloat x, GLfloat y, GLfloat z)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniform3f %d %f %f %f\n",
                        gc, tid, location, x, y, z);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Uniform3f(gc, location, x, y, z);
    __GL_PROFILE_FOOTER(GL4_UNIFORM3F);

    if (__glTracerDispatchTable.Uniform3f)
    {
        (*__glTracerDispatchTable.Uniform3f)(location, x, y, z);
    }
}

GLvoid GLAPIENTRY __glProfile_Uniform3fv(__GLcontext *gc, GLint location, GLsizei count, const GLfloat* v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniform3fv %d %d 0x%08X\n",
                        gc, tid, location, count, __GL_PTR2UINT(v));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Uniform3fv(gc, location, count, v);
    __GL_PROFILE_FOOTER(GL4_UNIFORM3FV);

    if (__glTracerDispatchTable.Uniform3fv)
    {
        (*__glTracerDispatchTable.Uniform3fv)(location, count, v);
    }
}

GLvoid GLAPIENTRY __glProfile_Uniform3i(__GLcontext *gc, GLint location, GLint x, GLint y, GLint z)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniform3i %d %d %d %d\n",
                        gc, tid, location, x, y, z);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Uniform3i(gc, location, x, y, z);
    __GL_PROFILE_FOOTER(GL4_UNIFORM3I);

    if (__glTracerDispatchTable.Uniform3i)
    {
        (*__glTracerDispatchTable.Uniform3i)(location, x, y, z);
    }
}

GLvoid GLAPIENTRY __glProfile_Uniform3iv(__GLcontext *gc, GLint location, GLsizei count, const GLint* v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniform3iv %d %d 0x%08X\n",
                        gc, tid, location, count, __GL_PTR2UINT(v));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Uniform3iv(gc, location, count, v);
    __GL_PROFILE_FOOTER(GL4_UNIFORM3IV);

    if (__glTracerDispatchTable.Uniform3iv)
    {
        (*__glTracerDispatchTable.Uniform3iv)(location, count, v);
    }
}

GLvoid GLAPIENTRY __glProfile_Uniform4f(__GLcontext *gc, GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniform4f %d %f %f %f %f\n",
                        gc, tid, location, x, y, z, w);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Uniform4f(gc, location, x, y, z, w);
    __GL_PROFILE_FOOTER(GL4_UNIFORM4F);

    if (__glTracerDispatchTable.Uniform4f)
    {
        (*__glTracerDispatchTable.Uniform4f)(location, x, y, z, w);
    }
}

GLvoid GLAPIENTRY __glProfile_Uniform4fv(__GLcontext *gc, GLint location, GLsizei count, const GLfloat* v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniform4fv %d %d 0x%08X\n",
                        gc, tid, location, count, __GL_PTR2UINT(v));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Uniform4fv(gc, location, count, v);
    __GL_PROFILE_FOOTER(GL4_UNIFORM4FV);

    if (__glTracerDispatchTable.Uniform4fv)
    {
        (*__glTracerDispatchTable.Uniform4fv)(location, count, v);
    }
}

GLvoid GLAPIENTRY __glProfile_Uniform4i(__GLcontext *gc, GLint location, GLint x, GLint y, GLint z, GLint w)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniform4i %d %d %d %d %d\n",
                        gc, tid, location, x, y, z, w);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Uniform4i(gc, location, x, y, z, w);
    __GL_PROFILE_FOOTER(GL4_UNIFORM4I);

    if (__glTracerDispatchTable.Uniform4i)
    {
        (*__glTracerDispatchTable.Uniform4i)(location, x, y, z, w);
    }
}

GLvoid GLAPIENTRY __glProfile_Uniform4iv(__GLcontext *gc, GLint location, GLsizei count, const GLint* v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniform4iv %d %d 0x%08X\n",
                        gc, tid, location, count, __GL_PTR2UINT(v));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Uniform4iv(gc, location, count, v);
    __GL_PROFILE_FOOTER(GL4_UNIFORM4IV);

    if (__glTracerDispatchTable.Uniform4iv)
    {
        (*__glTracerDispatchTable.Uniform4iv)(location, count, v);
    }
}

GLvoid GLAPIENTRY __glProfile_UniformMatrix2fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniformMatrix2fv %d %d %d 0x%08X\n",
                        gc, tid, location, count, transpose, __GL_PTR2UINT(value));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->UniformMatrix2fv(gc, location, count, transpose, value);
    __GL_PROFILE_FOOTER(GL4_UNIFORMMATRIX2FV);

    if (__glTracerDispatchTable.UniformMatrix2fv)
    {
        (*__glTracerDispatchTable.UniformMatrix2fv)(location, count, transpose, value);
    }
}

GLvoid GLAPIENTRY __glProfile_UniformMatrix3fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniformMatrix3fv %d %d %d 0x%08X\n",
                        gc, tid, location, count, transpose, __GL_PTR2UINT(value));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->UniformMatrix3fv(gc, location, count, transpose, value);
    __GL_PROFILE_FOOTER(GL4_UNIFORMMATRIX3FV);

    if (__glTracerDispatchTable.UniformMatrix3fv)
    {
        (*__glTracerDispatchTable.UniformMatrix3fv)(location, count, transpose, value);
    }
}

GLvoid GLAPIENTRY __glProfile_UniformMatrix4fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniformMatrix4fv %d %d %d 0x%08X\n",
                        gc, tid, location, count, transpose, __GL_PTR2UINT(value));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->UniformMatrix4fv(gc, location, count, transpose, value);
    __GL_PROFILE_FOOTER(GL4_UNIFORMMATRIX4FV);

    if (__glTracerDispatchTable.UniformMatrix4fv)
    {
        (*__glTracerDispatchTable.UniformMatrix4fv)(location, count, transpose, value);
    }
}

GLvoid GLAPIENTRY __glProfile_UseProgram(__GLcontext *gc, GLuint program)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUseProgram %d\n",
                        gc, tid, program);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->UseProgram(gc, program);
    __GL_PROFILE_FOOTER(GL4_USEPROGRAM);

    if (__glTracerDispatchTable.UseProgram)
    {
        (*__glTracerDispatchTable.UseProgram)(program);
    }
}

GLvoid GLAPIENTRY __glProfile_ValidateProgram(__GLcontext *gc, GLuint program)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glValidateProgram %d\n",
                        gc, tid, program);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ValidateProgram(gc, program);
    __GL_PROFILE_FOOTER(GL4_VALIDATEPROGRAM);

    if (__glTracerDispatchTable.ValidateProgram)
    {
        (*__glTracerDispatchTable.ValidateProgram)(program);
    }
}

GLvoid GLAPIENTRY __glProfile_VertexAttrib1f(__GLcontext *gc, GLuint indx, GLfloat x)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttrib1f %d %f\n",
                        gc, tid, indx, x);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttrib1f(gc, indx, x);
    __GL_PROFILE_FOOTER(GL4_VERTEXATTRIB1F);

    if (__glTracerDispatchTable.VertexAttrib1f)
    {
        (*__glTracerDispatchTable.VertexAttrib1f)(indx, x);
    }
}

GLvoid GLAPIENTRY __glProfile_VertexAttrib1fv(__GLcontext *gc, GLuint indx, const GLfloat* values)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttrib1fv %d 0x%08X\n",
                        gc, tid, indx, __GL_PTR2UINT(values));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttrib1fv(gc, indx, values);
    __GL_PROFILE_FOOTER(GL4_VERTEXATTRIB1FV);

    if (__glTracerDispatchTable.VertexAttrib1fv)
    {
        (*__glTracerDispatchTable.VertexAttrib1fv)(indx, values);
    }
}

GLvoid GLAPIENTRY __glProfile_VertexAttrib2f(__GLcontext *gc, GLuint indx, GLfloat x, GLfloat y)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttrib2f %d %f %f\n",
                        gc, tid, indx, x, y);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttrib2f(gc, indx, x, y);
    __GL_PROFILE_FOOTER(GL4_VERTEXATTRIB2F);

    if (__glTracerDispatchTable.VertexAttrib2f)
    {
        (*__glTracerDispatchTable.VertexAttrib2f)(indx, x, y);
    }
}

GLvoid GLAPIENTRY __glProfile_VertexAttrib2fv(__GLcontext *gc, GLuint indx, const GLfloat* values)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttrib2fv %d 0x%08X\n",
                        gc, tid, indx, __GL_PTR2UINT(values));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttrib2fv(gc, indx, values);
    __GL_PROFILE_FOOTER(GL4_VERTEXATTRIB2FV);

    if (__glTracerDispatchTable.VertexAttrib2fv)
    {
        (*__glTracerDispatchTable.VertexAttrib2fv)(indx, values);
    }
}

GLvoid GLAPIENTRY __glProfile_VertexAttrib3f(__GLcontext *gc, GLuint indx, GLfloat x, GLfloat y, GLfloat z)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttrib3f %d %f %f %f\n",
                        gc, tid, indx, x, y, z);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttrib3f(gc, indx, x, y, z);
    __GL_PROFILE_FOOTER(GL4_VERTEXATTRIB3F);

    if (__glTracerDispatchTable.VertexAttrib3f)
    {
        (*__glTracerDispatchTable.VertexAttrib3f)(indx, x, y, z);
    }
}

GLvoid GLAPIENTRY __glProfile_VertexAttrib3fv(__GLcontext *gc, GLuint indx, const GLfloat* values)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttrib3fv %d 0x%08X\n",
                        gc, tid, indx, __GL_PTR2UINT(values));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttrib3fv(gc, indx, values);
    __GL_PROFILE_FOOTER(GL4_VERTEXATTRIB3FV);

    if (__glTracerDispatchTable.VertexAttrib3fv)
    {
        (*__glTracerDispatchTable.VertexAttrib3fv)(indx, values);
    }
}

GLvoid GLAPIENTRY __glProfile_VertexAttrib4f(__GLcontext *gc, GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttrib4f %d %f %f %f %f\n",
                        gc, tid, indx, x, y, z, w);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttrib4f(gc, indx, x, y, z, w);
    __GL_PROFILE_FOOTER(GL4_VERTEXATTRIB4F);

    if (__glTracerDispatchTable.VertexAttrib4f)
    {
        (*__glTracerDispatchTable.VertexAttrib4f)(indx, x, y, z, w);
    }
}

GLvoid GLAPIENTRY __glProfile_VertexAttrib4fv(__GLcontext *gc, GLuint indx, const GLfloat* values)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttrib4fv %d 0x%08X\n",
                        gc, tid, indx, __GL_PTR2UINT(values));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttrib4fv(gc, indx, values);
    __GL_PROFILE_FOOTER(GL4_VERTEXATTRIB4FV);

    if (__glTracerDispatchTable.VertexAttrib4fv)
    {
        (*__glTracerDispatchTable.VertexAttrib4fv)(indx, values);
    }
}

GLvoid GLAPIENTRY __glProfile_VertexAttribPointer(__GLcontext *gc, GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttribPointer %d %d 0x%04X %d %d 0x%08X\n",
                        gc, tid, indx, size, type, normalized, stride, __GL_PTR2UINT(ptr));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttribPointer(gc, indx, size, type, normalized, stride, ptr);
    __GL_PROFILE_FOOTER(GL4_VERTEXATTRIBPOINTER);

    if (__glTracerDispatchTable.VertexAttribPointer)
    {
        (*__glTracerDispatchTable.VertexAttribPointer)(indx, size, type, normalized, stride, ptr);
    }
}

GLvoid GLAPIENTRY __glProfile_Viewport(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glViewport %d %d %d %d\n",
                        gc, tid, x, y, width, height);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Viewport(gc, x, y, width, height);
    __GL_PROFILE_FOOTER(GL4_VIEWPORT);

    if (__glTracerDispatchTable.Viewport)
    {
        (*__glTracerDispatchTable.Viewport)(x, y, width, height);
    }
}

/* OpenGL ES 3.0 */

GLvoid GLAPIENTRY __glProfile_ReadBuffer(__GLcontext *gc, GLenum mode)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glReadBuffer 0x%04X\n",
                        gc, tid, mode);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ReadBuffer(gc, mode);
    __GL_PROFILE_FOOTER(GL4_READBUFFER);

    if (__glTracerDispatchTable.ReadBuffer)
    {
        (*__glTracerDispatchTable.ReadBuffer)(mode);
    }
}

GLvoid GLAPIENTRY __glProfile_DrawRangeElements(__GLcontext *gc, GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDrawRangeElements 0x%04X %d %d %d 0x%04X 0x%08X\n",
                        gc, tid, mode, start, end, count, type, __GL_PTR2UINT(indices));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DrawRangeElements(gc, mode, start, end, count, type, indices);
    __GL_PROFILE_FOOTER(GL4_DRAWRANGEELEMENTS);

    if (__glTracerDispatchTable.DrawRangeElements)
    {
        (*__glTracerDispatchTable.DrawRangeElements)(mode, start, end, count, type, indices);
    }
}

GLvoid GLAPIENTRY __glProfile_TexImage3D(__GLcontext *gc, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexImage3D 0x%04X %d 0x%04X %d %d %d %d 0x%04X 0x%04X 0x%08X\n",
                        gc, tid, target, level, internalformat, width, height, depth, border, format, type, __GL_PTR2UINT(pixels));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexImage3D(gc, target, level, internalformat, width, height, depth, border, format, type, pixels);
    __GL_PROFILE_FOOTER(GL4_TEXIMAGE3D);

    if (__glTracerDispatchTable.TexImage3D)
    {
        (*__glTracerDispatchTable.TexImage3D)(target, level, internalformat, width, height, depth, border, format, type, pixels);
    }
}

GLvoid GLAPIENTRY __glProfile_TexSubImage3D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexSubImage3D 0x%04X %d %d %d %d %d %d %d 0x%04X 0x%04X 0x%08X\n",
                        gc, tid, target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, __GL_PTR2UINT(pixels));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexSubImage3D(gc, target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
    __GL_PROFILE_FOOTER(GL4_TEXSUBIMAGE3D);

    if (__glTracerDispatchTable.TexSubImage3D)
    {
        (*__glTracerDispatchTable.TexSubImage3D)(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
    }
}

GLvoid GLAPIENTRY __glProfile_CopyTexSubImage3D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glCopyTexSubImage3D 0x%04X %d %d %d %d %d %d %d %d\n",
                        gc, tid, target, level, xoffset, yoffset, zoffset, x, y, width, height);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->CopyTexSubImage3D(gc, target, level, xoffset, yoffset, zoffset, x, y, width, height);
    __GL_PROFILE_FOOTER(GL4_COPYTEXSUBIMAGE3D);

    if (__glTracerDispatchTable.CopyTexSubImage3D)
    {
        (*__glTracerDispatchTable.CopyTexSubImage3D)(target, level, xoffset, yoffset, zoffset, x, y, width, height);
    }
}

GLvoid GLAPIENTRY __glProfile_CompressedTexImage3D(__GLcontext *gc, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glCompressedTexImage3D 0x%04X %d 0x%04X %d %d %d %d %d 0x%08X\n",
                        gc, tid, target, level, internalformat, width, height, depth, border, imageSize, __GL_PTR2UINT(data));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->CompressedTexImage3D(gc, target, level, internalformat, width, height, depth, border, imageSize, data);
    __GL_PROFILE_FOOTER(GL4_COMPRESSEDTEXIMAGE3D);

    if (__glTracerDispatchTable.CompressedTexImage3D)
    {
        (*__glTracerDispatchTable.CompressedTexImage3D)(target, level, internalformat, width, height, depth, border, imageSize, data);
    }
}

GLvoid GLAPIENTRY __glProfile_CompressedTexSubImage3D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glCompressedTexSubImage3D 0x%04X %d %d %d %d %d %d %d 0x%04X %d 0x%08X\n",
                        gc, tid, target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, __GL_PTR2UINT(data));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->CompressedTexSubImage3D(gc, target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
    __GL_PROFILE_FOOTER(GL4_COMPRESSEDTEXSUBIMAGE3D);

    if (__glTracerDispatchTable.CompressedTexSubImage3D)
    {
        (*__glTracerDispatchTable.CompressedTexSubImage3D)(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
    }
}

GLvoid GLAPIENTRY __glProfile_GenQueries(__GLcontext *gc, GLsizei n, GLuint* ids)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGenQueries %d\n",
                        gc, tid, n);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GenQueries(gc, n, ids);
    __GL_PROFILE_FOOTER(GL4_GENQUERIES);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGenQueries => ");
        __glLogArrayData(gc, n, ids);
    }

    if (__glTracerDispatchTable.GenQueries)
    {
        (*__glTracerDispatchTable.GenQueries)(n, ids);
    }
}

GLvoid GLAPIENTRY __glProfile_DeleteQueries(__GLcontext *gc, GLsizei n, const GLuint* ids)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDeleteQueries %d ",
                        gc, tid, n);
        __glLogArrayData(gc, n, ids);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DeleteQueries(gc, n, ids);
    __GL_PROFILE_FOOTER(GL4_DELETEQUERIES);

    if (__glTracerDispatchTable.DeleteQueries)
    {
        (*__glTracerDispatchTable.DeleteQueries)(n, ids);
    }
}

GLboolean GLAPIENTRY __glProfile_IsQuery(__GLcontext *gc, GLuint id)
{
    __GL_PROFILE_VARS();
    GLboolean is;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glIsQuery %d\n",
                        gc, tid, id);
    }

    __GL_PROFILE_HEADER();
    is = gc->pModeDispatch->IsQuery(gc, id);
    __GL_PROFILE_FOOTER(GL4_ISQUERY);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glIsQuery => %d\n", is);
    }

    if (__glTracerDispatchTable.IsQuery)
    {
        (*__glTracerDispatchTable.IsQuery)(id);
    }

    return is;
}

GLvoid GLAPIENTRY __glProfile_BeginQuery(__GLcontext *gc, GLenum target, GLuint id)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glBeginQuery 0x%04X %d\n",
                        gc, tid, target, id);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->BeginQuery(gc, target, id);
    __GL_PROFILE_FOOTER(GL4_BEGINQUERY);

    if (__glTracerDispatchTable.BeginQuery)
    {
        (*__glTracerDispatchTable.BeginQuery)(target, id);
    }
}

GLvoid GLAPIENTRY __glProfile_EndQuery(__GLcontext *gc, GLenum target)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glEndQuery 0x%04X\n",
                        gc, tid, target);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->EndQuery(gc, target);
    __GL_PROFILE_FOOTER(GL4_ENDQUERY);

    if (__glTracerDispatchTable.EndQuery)
    {
        (*__glTracerDispatchTable.EndQuery)(target);
    }
}

GLvoid GLAPIENTRY __glProfile_GetQueryiv(__GLcontext *gc, GLenum target, GLenum pname, GLint* params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetQueryiv 0x%04X 0x%04X\n",
                        gc, tid, target, pname);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetQueryiv(gc, target, pname, params);
    __GL_PROFILE_FOOTER(GL4_GETQUERYIV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetQueryiv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glTracerDispatchTable.GetQueryiv)
    {
        (*__glTracerDispatchTable.GetQueryiv)(target, pname, params);
    }
}

GLvoid GLAPIENTRY __glProfile_GetQueryObjectuiv(__GLcontext *gc, GLuint id, GLenum pname, GLuint* params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetQueryObjectuiv %d 0x%04X\n",
                        gc, tid, id, pname);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetQueryObjectuiv(gc, id, pname, params);
    __GL_PROFILE_FOOTER(GL4_GETQUERYOBJECTUIV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetQueryObjectuiv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glTracerDispatchTable.GetQueryObjectuiv)
    {
        (*__glTracerDispatchTable.GetQueryObjectuiv)(id, pname, params);
    }
}

GLboolean GLAPIENTRY __glProfile_UnmapBuffer(__GLcontext *gc, GLenum target)
{
    __GL_PROFILE_VARS();
    GLboolean success;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUnmapBuffer 0x%04X\n",
                        gc, tid, target);
    }

    __GL_PROFILE_HEADER();
    success = gc->pModeDispatch->UnmapBuffer(gc, target);
    __GL_PROFILE_FOOTER(GL4_UNMAPBUFFER);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glUnmapBuffer => %d\n", success);
    }

    if (__glTracerDispatchTable.UnmapBuffer)
    {
        (*__glTracerDispatchTable.UnmapBuffer)(target);
    }

    return success;
}

GLvoid GLAPIENTRY __glProfile_GetBufferPointerv(__GLcontext *gc, GLenum target, GLenum pname, GLvoid** params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetBufferPointerv 0x%04X 0x%04X\n",
                        gc, tid, target, pname);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetBufferPointerv(gc, target, pname, params);
    __GL_PROFILE_FOOTER(GL4_GETBUFFERPOINTERV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetBufferPointerv => 0x%08X\n", __GL_PTR2UINT(__GL_PTRVALUE(params)));
    }

    if (__glTracerDispatchTable.GetBufferPointerv)
    {
        (*__glTracerDispatchTable.GetBufferPointerv)(target, pname, params);
    }
}

GLvoid GLAPIENTRY __glProfile_DrawBuffers(__GLcontext *gc, GLsizei n, const GLenum* bufs)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDrawBuffers %d 0x%08X",
                        gc, tid, n, __GL_PTR2UINT(bufs));
        __glLogArrayData(gc, n, bufs);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DrawBuffers(gc, n, bufs);
    __GL_PROFILE_FOOTER(GL4_DRAWBUFFERS);

    if (__glTracerDispatchTable.DrawBuffers)
    {
        (*__glTracerDispatchTable.DrawBuffers)(n, bufs);
    }
}

GLvoid GLAPIENTRY __glProfile_UniformMatrix2x3fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniformMatrix2x3fv %d %d %d 0x%08X\n",
                        gc, tid, location, count, transpose, __GL_PTR2UINT(value));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->UniformMatrix2x3fv(gc, location, count, transpose, value);
    __GL_PROFILE_FOOTER(GL4_UNIFORMMATRIX2X3FV);

    if (__glTracerDispatchTable.UniformMatrix2x3fv)
    {
        (*__glTracerDispatchTable.UniformMatrix2x3fv)(location, count, transpose, value);
    }
}

GLvoid GLAPIENTRY __glProfile_UniformMatrix3x2fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniformMatrix3x2fv %d %d %d 0x%08X\n",
                        gc, tid, location, count, transpose, __GL_PTR2UINT(value));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->UniformMatrix3x2fv(gc, location, count, transpose, value);
    __GL_PROFILE_FOOTER(GL4_UNIFORMMATRIX3X2FV);

    if (__glTracerDispatchTable.UniformMatrix3x2fv)
    {
        (*__glTracerDispatchTable.UniformMatrix3x2fv)(location, count, transpose, value);
    }
}

GLvoid GLAPIENTRY __glProfile_UniformMatrix2x4fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniformMatrix2x4fv %d %d %d 0x%08X\n",
                        gc, tid, location, count, transpose, __GL_PTR2UINT(value));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->UniformMatrix2x4fv(gc, location, count, transpose, value);
    __GL_PROFILE_FOOTER(GL4_UNIFORMMATRIX2X4FV);

    if (__glTracerDispatchTable.UniformMatrix2x4fv)
    {
        (*__glTracerDispatchTable.UniformMatrix2x4fv)(location, count, transpose, value);
    }
}

GLvoid GLAPIENTRY __glProfile_UniformMatrix4x2fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniformMatrix4x2fv %d %d %d 0x%08X\n",
                        gc, tid, location, count, transpose, __GL_PTR2UINT(value));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->UniformMatrix4x2fv(gc, location, count, transpose, value);
    __GL_PROFILE_FOOTER(GL4_UNIFORMMATRIX4X2FV);

    if (__glTracerDispatchTable.UniformMatrix4x2fv)
    {
        (*__glTracerDispatchTable.UniformMatrix4x2fv)(location, count, transpose, value);
    }
}

GLvoid GLAPIENTRY __glProfile_UniformMatrix3x4fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniformMatrix3x4fv %d %d %d 0x%08X\n",
                        gc, tid, location, count, transpose, __GL_PTR2UINT(value));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->UniformMatrix3x4fv(gc, location, count, transpose, value);
    __GL_PROFILE_FOOTER(GL4_UNIFORMMATRIX3X4FV);

    if (__glTracerDispatchTable.UniformMatrix3x4fv)
    {
        (*__glTracerDispatchTable.UniformMatrix3x4fv)(location, count, transpose, value);
    }
}

GLvoid GLAPIENTRY __glProfile_UniformMatrix4x3fv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniformMatrix4x3fv %d %d %d 0x%08X\n",
                        gc, tid, location, count, transpose, __GL_PTR2UINT(value));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->UniformMatrix4x3fv(gc, location, count, transpose, value);
    __GL_PROFILE_FOOTER(GL4_UNIFORMMATRIX4X3FV);

    if (__glTracerDispatchTable.UniformMatrix4x3fv)
    {
        (*__glTracerDispatchTable.UniformMatrix4x3fv)(location, count, transpose, value);
    }
}

GLvoid GLAPIENTRY __glProfile_BlitFramebuffer(__GLcontext *gc, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glBlitFramebuffer %d %d %d %d %d %d %d %d 0x%08X 0x%04X\n",
                        gc, tid, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->BlitFramebuffer(gc, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
    __GL_PROFILE_FOOTER(GL4_BLITFRAMEBUFFER);

    if (__glTracerDispatchTable.BlitFramebuffer)
    {
        (*__glTracerDispatchTable.BlitFramebuffer)(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
    }
}

GLvoid GLAPIENTRY __glProfile_RenderbufferStorageMultisample(__GLcontext *gc, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glRenderbufferStorageMultisample 0x%04X %d 0x%04X %d %d\n",
                        gc, tid, target, samples, internalformat, width, height);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->RenderbufferStorageMultisample(gc, target, samples, internalformat, width, height);
    __GL_PROFILE_FOOTER(GL4_RENDERBUFFERSTORAGEMULTISAMPLE);

    if (__glTracerDispatchTable.RenderbufferStorageMultisample)
    {
        (*__glTracerDispatchTable.RenderbufferStorageMultisample)(target, samples, internalformat, width, height);
    }
}

GLvoid GLAPIENTRY __glProfile_FramebufferTextureLayer(__GLcontext *gc, GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glFramebufferTextureLayer 0x%04X 0x%04X %d %d %d\n",
                        gc, tid, target, attachment, texture, level, layer);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->FramebufferTextureLayer(gc, target, attachment, texture, level, layer);
    __GL_PROFILE_FOOTER(GL4_FRAMEBUFFERTEXTURELAYER);

    if (__glTracerDispatchTable.FramebufferTextureLayer)
    {
        (*__glTracerDispatchTable.FramebufferTextureLayer)(target, attachment, texture, level, layer);
    }
}

GLvoid* GLAPIENTRY __glProfile_MapBufferRange(__GLcontext *gc, GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
    __GL_PROFILE_VARS();
    GLvoid *buf;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMapBufferRange 0x%04X %d %d 0x%08X\n",
                        gc, tid, target, (GLuint)offset, (GLuint)length, access);
    }

    __GL_PROFILE_HEADER();
    buf = gc->pModeDispatch->MapBufferRange(gc, target, offset, length, access);
    __GL_PROFILE_FOOTER(GL4_MAPBUFFERRANGE);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glMapBufferRange => 0x%08X\n", __GL_PTR2UINT(buf));
    }

    if (__glTracerDispatchTable.MapBufferRange)
    {
        (*__glTracerDispatchTable.MapBufferRange)(target, offset, length, access, buf);
    }

    return buf;
}

GLvoid GLAPIENTRY __glProfile_FlushMappedBufferRange(__GLcontext *gc, GLenum target, GLintptr offset, GLsizeiptr length)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glFlushMappedBufferRange 0x%04X %d %d\n",
                        gc, tid, target, (GLuint)offset, (GLuint)length);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->FlushMappedBufferRange(gc, target, offset, length);
    __GL_PROFILE_FOOTER(GL4_FLUSHMAPPEDBUFFERRANGE);

    if (__glTracerDispatchTable.FlushMappedBufferRange)
    {
        (*__glTracerDispatchTable.FlushMappedBufferRange)(target, offset, length);
    }
}

GLvoid GLAPIENTRY __glProfile_BindVertexArray(__GLcontext *gc, GLuint array)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glBindVertexArray %d\n",
                        gc, tid, array);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->BindVertexArray(gc, array);
    __GL_PROFILE_FOOTER(GL4_BINDVERTEXARRAY);

    if (__glTracerDispatchTable.BindVertexArray)
    {
        (*__glTracerDispatchTable.BindVertexArray)(array);
    }
}

GLvoid GLAPIENTRY __glProfile_DeleteVertexArrays(__GLcontext *gc, GLsizei n, const GLuint* arrays)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDeleteVertexArrays %d ",
                        gc, tid, n);
        __glLogArrayData(gc, n, arrays);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DeleteVertexArrays(gc, n, arrays);
    __GL_PROFILE_FOOTER(GL4_DELETEVERTEXARRAYS);

    if (__glTracerDispatchTable.DeleteVertexArrays)
    {
        (*__glTracerDispatchTable.DeleteVertexArrays)(n , arrays);
    }
}

GLvoid GLAPIENTRY __glProfile_GenVertexArrays(__GLcontext *gc, GLsizei n, GLuint* arrays)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGenVertexArrays %d\n",
                        gc, tid, n);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GenVertexArrays(gc, n, arrays);
    __GL_PROFILE_FOOTER(GL4_GENVERTEXARRAYS);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGenVertexArrays => ");
        __glLogArrayData(gc, n, arrays);
    }

    if (__glTracerDispatchTable.GenVertexArrays)
    {
        (*__glTracerDispatchTable.GenVertexArrays)(n , arrays);
    }
}

GLboolean GLAPIENTRY __glProfile_IsVertexArray(__GLcontext *gc, GLuint array)
{
    __GL_PROFILE_VARS();
    GLboolean is;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glIsVertexArray %d\n",
                        gc, tid, array);
    }

    __GL_PROFILE_HEADER();
    is = gc->pModeDispatch->IsVertexArray(gc, array);
    __GL_PROFILE_FOOTER(GL4_ISVERTEXARRAY);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glIsVertexArray => %d\n", is);
    }

    if (__glTracerDispatchTable.IsVertexArray)
    {
        (*__glTracerDispatchTable.IsVertexArray)(array);
    }

    return is;
}

GLvoid GLAPIENTRY __glProfile_GetIntegeri_v(__GLcontext *gc, GLenum target, GLuint index, GLint* data)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetIntegeri_v 0x%04X %d\n",
                        gc, tid, target, index);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetIntegeri_v(gc, target, index, data);
    __GL_PROFILE_FOOTER(GL4_GETINTEGERI_V);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetIntegeri_v => %d\n", __GL_PTRVALUE(data));
    }

    if (__glTracerDispatchTable.GetIntegeri_v)
    {
        (*__glTracerDispatchTable.GetIntegeri_v)(target, index, data);
    }
}

GLvoid GLAPIENTRY __glProfile_BeginTransformFeedback(__GLcontext *gc, GLenum primitiveMode)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glBeginTransformFeedback 0x%04X\n",
                        gc, tid, primitiveMode);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->BeginTransformFeedback(gc, primitiveMode);
    __GL_PROFILE_FOOTER(GL4_BEGINTRANSFORMFEEDBACK);

    if (__glTracerDispatchTable.BeginTransformFeedback)
    {
        (*__glTracerDispatchTable.BeginTransformFeedback)(primitiveMode);
    }
}

GLvoid GLAPIENTRY __glProfile_EndTransformFeedback(__GLcontext *gc)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glEndTransformFeedback\n", gc, tid);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->EndTransformFeedback(gc);
    __GL_PROFILE_FOOTER(GL4_ENDTRANSFORMFEEDBACK);

    if (__glTracerDispatchTable.EndTransformFeedback)
    {
        (*__glTracerDispatchTable.EndTransformFeedback)();
    }
}

GLvoid GLAPIENTRY __glProfile_BindBufferRange(__GLcontext *gc, GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glBindBufferRange 0x%04X %d %d %d %d\n",
                        gc, tid, target, index, buffer, (GLuint)offset, (GLuint)size);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->BindBufferRange(gc, target, index, buffer, offset, size);
    __GL_PROFILE_FOOTER(GL4_BINDBUFFERRANGE);

    if (__glTracerDispatchTable.BindBufferRange)
    {
        (*__glTracerDispatchTable.BindBufferRange)(target, index, buffer, offset, size);
    }
}

GLvoid GLAPIENTRY __glProfile_BindBufferBase(__GLcontext *gc, GLenum target, GLuint index, GLuint buffer)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glBindBufferBase 0x%04X %d %d\n",
                        gc, tid, target, index, buffer);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->BindBufferBase(gc, target, index, buffer);
    __GL_PROFILE_FOOTER(GL4_BINDBUFFERBASE);

    if (__glTracerDispatchTable.BindBufferBase)
    {
        (*__glTracerDispatchTable.BindBufferBase)(target, index, buffer);
    }
}

GLvoid GLAPIENTRY __glProfile_TransformFeedbackVaryings(__GLcontext *gc, GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTransformFeedbackVaryings %d %d 0x%08X 0x%04X\n",
                        gc, tid, program, count, __GL_PTR2UINT(varyings), bufferMode);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TransformFeedbackVaryings(gc, program, count, varyings, bufferMode);
    __GL_PROFILE_FOOTER(GL4_TRANSFORMFEEDBACKVARYINGS);

    if (__glTracerDispatchTable.TransformFeedbackVaryings)
    {
        (*__glTracerDispatchTable.TransformFeedbackVaryings)(program, count, varyings, bufferMode);
    }
}

GLvoid GLAPIENTRY __glProfile_GetTransformFeedbackVarying(__GLcontext *gc, GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetTransformFeedbackVarying %d %d %d\n",
                        gc, tid, program, index, bufSize);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetTransformFeedbackVarying(gc, program, index, bufSize, length, size, type, name);
    __GL_PROFILE_FOOTER(GL4_GETTRANSFORMFEEDBACKVARYING);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetTransformFeedbackVarying => %d %d 0x%04X %s\n",
                        __GL_PTRVALUE(length), __GL_PTRVALUE(size), __GL_PTR2UINT(type), name);
    }

    if (__glTracerDispatchTable.GetTransformFeedbackVarying)
    {
        (*__glTracerDispatchTable.GetTransformFeedbackVarying)(program, index, bufSize, length, size, type, name);
    }
}

GLvoid GLAPIENTRY __glProfile_VertexAttribIPointer(__GLcontext *gc, GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttribIPointer %d %d 0x%04X %d 0x%08X\n",
                        gc, tid, index, size, type, stride, __GL_PTR2UINT(pointer));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttribIPointer(gc, index, size, type, stride, pointer);
    __GL_PROFILE_FOOTER(GL4_VERTEXATTRIBIPOINTER);

    if (__glTracerDispatchTable.VertexAttribIPointer)
    {
        (*__glTracerDispatchTable.VertexAttribIPointer)(index, size, type, stride, pointer);
    }
}

GLvoid GLAPIENTRY __glProfile_GetVertexAttribIiv(__GLcontext *gc, GLuint index, GLenum pname, GLint* params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetVertexAttribIiv %d 0x%04X\n",
                        gc, tid, index, pname);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetVertexAttribIiv(gc, index, pname, params);
    __GL_PROFILE_FOOTER(GL4_GETVERTEXATTRIBIIV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetVertexAttribIiv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glTracerDispatchTable.GetVertexAttribIiv)
    {
        (*__glTracerDispatchTable.GetVertexAttribIiv)(index, pname, params);
    }
}

GLvoid GLAPIENTRY __glProfile_GetVertexAttribIuiv(__GLcontext *gc, GLuint index, GLenum pname, GLuint* params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetVertexAttribIuiv %d 0x%04X\n",
                        gc, tid, index, pname);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetVertexAttribIuiv(gc, index, pname, params);
    __GL_PROFILE_FOOTER(GL4_GETVERTEXATTRIBIUIV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetVertexAttribIuiv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glTracerDispatchTable.GetVertexAttribIuiv)
    {
        (*__glTracerDispatchTable.GetVertexAttribIuiv)(index, pname, params);
    }
}

GLvoid GLAPIENTRY __glProfile_VertexAttribI4i(__GLcontext *gc, GLuint index, GLint x, GLint y, GLint z, GLint w)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttribI4i %d %d %d %d %d\n",
                        gc, tid, index, x, y, z, w);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttribI4i(gc, index, x, y, z, w);
    __GL_PROFILE_FOOTER(GL4_VERTEXATTRIBI4I);

    if (__glTracerDispatchTable.VertexAttribI4i)
    {
        (*__glTracerDispatchTable.VertexAttribI4i)(index, x, y, z, w);
    }
}

GLvoid GLAPIENTRY __glProfile_VertexAttribI4ui(__GLcontext *gc, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttribI4ui %d %d %d %d %d\n",
                        gc, tid, index, x, y, z, w);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttribI4ui(gc, index, x, y, z, w);
    __GL_PROFILE_FOOTER(GL4_VERTEXATTRIBI4UI);

    if (__glTracerDispatchTable.VertexAttribI4ui)
    {
        (*__glTracerDispatchTable.VertexAttribI4ui)(index, x, y, z, w);
    }
}

GLvoid GLAPIENTRY __glProfile_VertexAttribI4iv(__GLcontext *gc, GLuint index, const GLint* v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttribI4iv %d 0x%08X\n",
                        gc, tid, index, __GL_PTR2UINT(v));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttribI4iv(gc, index, v);
    __GL_PROFILE_FOOTER(GL4_VERTEXATTRIBI4IV);

    if (__glTracerDispatchTable.VertexAttribI4iv)
    {
        (*__glTracerDispatchTable.VertexAttribI4iv)(index, v);
    }
}

GLvoid GLAPIENTRY __glProfile_VertexAttribI4uiv(__GLcontext *gc, GLuint index, const GLuint* v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttribI4uiv %d 0x%08X\n",
                        gc, tid, index, __GL_PTR2UINT(v));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttribI4uiv(gc, index, v);
    __GL_PROFILE_FOOTER(GL4_VERTEXATTRIBI4UIV);

    if (__glTracerDispatchTable.VertexAttribI4uiv)
    {
        (*__glTracerDispatchTable.VertexAttribI4uiv)(index, v);
    }
}

GLvoid GLAPIENTRY __glProfile_GetUniformuiv(__GLcontext *gc, GLuint program, GLint location, GLuint* params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetUniformuiv %d %d\n",
                        gc, tid, program, location);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetUniformuiv(gc, program, location, params);
    __GL_PROFILE_FOOTER(GL4_GETUNIFORMUIV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetUniformuiv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glTracerDispatchTable.GetUniformuiv)
    {
        (*__glTracerDispatchTable.GetUniformuiv)(program, location, params);
    }
}

GLint GLAPIENTRY __glProfile_GetFragDataLocation(__GLcontext *gc, GLuint program, const GLchar *name)
{
    __GL_PROFILE_VARS();
    GLint location;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetFragDataLocation %d %s\n",
                        gc, tid, program, name);
    }

    __GL_PROFILE_HEADER();
    location = gc->pModeDispatch->GetFragDataLocation(gc, program, name);
    __GL_PROFILE_FOOTER(GL4_GETFRAGDATALOCATION);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetFragDataLocation => %d\n", location);
    }

    if (__glTracerDispatchTable.GetFragDataLocation)
    {
        (*__glTracerDispatchTable.GetFragDataLocation)(program, name, location);
    }

    return location;
}

GLvoid GLAPIENTRY __glProfile_Uniform1ui(__GLcontext *gc, GLint location, GLuint v0)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniform1ui %d %d\n",
                        gc, tid, location, v0);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Uniform1ui(gc, location, v0);
    __GL_PROFILE_FOOTER(GL4_UNIFORM1UI);

    if (__glTracerDispatchTable.Uniform1ui)
    {
        (*__glTracerDispatchTable.Uniform1ui)(location, v0);
    }
}

GLvoid GLAPIENTRY __glProfile_Uniform2ui(__GLcontext *gc, GLint location, GLuint v0, GLuint v1)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniform2ui %d %d %d\n",
                        gc, tid, location, v0, v1);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Uniform2ui(gc, location, v0, v1);
    __GL_PROFILE_FOOTER(GL4_UNIFORM2UI);

    if (__glTracerDispatchTable.Uniform2ui)
    {
        (*__glTracerDispatchTable.Uniform2ui)(location, v0, v1);
    }
}

GLvoid GLAPIENTRY __glProfile_Uniform3ui(__GLcontext *gc, GLint location, GLuint v0, GLuint v1, GLuint v2)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniform3ui %d %d %d %d\n",
                        gc, tid, location, v0, v1, v2);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Uniform3ui(gc, location, v0, v1, v2);
    __GL_PROFILE_FOOTER(GL4_UNIFORM3UI);

    if (__glTracerDispatchTable.Uniform3ui)
    {
        (*__glTracerDispatchTable.Uniform3ui)(location, v0, v1, v2);
    }
}

GLvoid GLAPIENTRY __glProfile_Uniform4ui(__GLcontext *gc, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniform4ui %d %d %d %d %d\n",
                        gc, tid, location, v0, v1, v2, v3);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Uniform4ui(gc, location, v0, v1, v2, v3);
    __GL_PROFILE_FOOTER(GL4_UNIFORM4UI);

    if (__glTracerDispatchTable.Uniform4ui)
    {
        (*__glTracerDispatchTable.Uniform4ui)(location, v0, v1, v2, v3);
    }
}

GLvoid GLAPIENTRY __glProfile_Uniform1uiv(__GLcontext *gc, GLint location, GLsizei count, const GLuint* value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniform1uiv %d %d 0x%08X\n",
                        gc, tid, location, count, __GL_PTR2UINT(value));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Uniform1uiv(gc, location, count, value);
    __GL_PROFILE_FOOTER(GL4_UNIFORM1UIV);

    if (__glTracerDispatchTable.Uniform1uiv)
    {
        (*__glTracerDispatchTable.Uniform1uiv)(location, count, value);
    }
}

GLvoid GLAPIENTRY __glProfile_Uniform2uiv(__GLcontext *gc, GLint location, GLsizei count, const GLuint* value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniform2uiv %d %d 0x%08X\n",
                        gc, tid, location, count, __GL_PTR2UINT(value));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Uniform2uiv(gc, location, count, value);
    __GL_PROFILE_FOOTER(GL4_UNIFORM2UIV);

    if (__glTracerDispatchTable.Uniform2uiv)
    {
        (*__glTracerDispatchTable.Uniform2uiv)(location, count, value);
    }
}

GLvoid GLAPIENTRY __glProfile_Uniform3uiv(__GLcontext *gc, GLint location, GLsizei count, const GLuint* value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniform3uiv %d %d 0x%08X\n",
                        gc, tid, location, count, __GL_PTR2UINT(value));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Uniform3uiv(gc, location, count, value);
    __GL_PROFILE_FOOTER(GL4_UNIFORM3UIV);

    if (__glTracerDispatchTable.Uniform3uiv)
    {
        (*__glTracerDispatchTable.Uniform3uiv)(location, count, value);
    }
}

GLvoid GLAPIENTRY __glProfile_Uniform4uiv(__GLcontext *gc, GLint location, GLsizei count, const GLuint* value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniform4uiv %d %d 0x%08X\n",
                        gc, tid, location, count, __GL_PTR2UINT(value));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Uniform4uiv(gc, location, count, value);
    __GL_PROFILE_FOOTER(GL4_UNIFORM4UIV);

    if (__glTracerDispatchTable.Uniform4uiv)
    {
        (*__glTracerDispatchTable.Uniform4uiv)(location, count, value);
    }
}

GLvoid GLAPIENTRY __glProfile_ClearBufferiv(__GLcontext *gc, GLenum buffer, GLint drawbuffer, const GLint* value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glClearBufferiv 0x%04X %d 0x%08X\n",
                        gc, tid, buffer, drawbuffer, __GL_PTR2UINT(value));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ClearBufferiv(gc, buffer, drawbuffer, value);
    __GL_PROFILE_FOOTER(GL4_CLEARBUFFERIV);

    if (__glTracerDispatchTable.ClearBufferiv)
    {
        (*__glTracerDispatchTable.ClearBufferiv)(buffer, drawbuffer, value);
    }
}

GLvoid GLAPIENTRY __glProfile_ClearBufferuiv(__GLcontext *gc, GLenum buffer, GLint drawbuffer, const GLuint* value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glClearBufferuiv 0x%04X %d 0x%08X\n",
                        gc, tid, buffer, drawbuffer, __GL_PTR2UINT(value));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ClearBufferuiv(gc, buffer, drawbuffer, value);
    __GL_PROFILE_FOOTER(GL4_CLEARBUFFERUIV);

    if (__glTracerDispatchTable.ClearBufferuiv)
    {
        (*__glTracerDispatchTable.ClearBufferuiv)(buffer, drawbuffer, value);
    }
}

GLvoid GLAPIENTRY __glProfile_ClearBufferfv(__GLcontext *gc, GLenum buffer, GLint drawbuffer, const GLfloat* value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glClearBufferfv 0x%04X %d 0x%08X\n",
                        gc, tid, buffer, drawbuffer, __GL_PTR2UINT(value));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ClearBufferfv(gc, buffer, drawbuffer, value);
    __GL_PROFILE_FOOTER(GL4_CLEARBUFFERFV);

    if (__glTracerDispatchTable.ClearBufferfv)
    {
        (*__glTracerDispatchTable.ClearBufferfv)(buffer, drawbuffer, value);
    }
}

GLvoid GLAPIENTRY __glProfile_ClearBufferfi(__GLcontext *gc, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glClearBufferfi 0x%04X %d %f %d\n",
                        gc, tid, buffer, drawbuffer, depth, stencil);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ClearBufferfi(gc, buffer, drawbuffer, depth, stencil);
    __GL_PROFILE_FOOTER(GL4_CLEARBUFFERFI);

    if (__glTracerDispatchTable.ClearBufferfi)
    {
        (*__glTracerDispatchTable.ClearBufferfi)(buffer, drawbuffer, depth, stencil);
    }
}

const GLubyte* GLAPIENTRY __glProfile_GetStringi(__GLcontext *gc, GLenum name, GLuint index)
{
    __GL_PROFILE_VARS();
    const GLubyte* string;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetStringi 0x%04X %d\n",
                        gc, tid, name, index);
    }

    __GL_PROFILE_HEADER();
    string = gc->pModeDispatch->GetStringi(gc, name, index);
    __GL_PROFILE_FOOTER(GL4_GETSTRINGI);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetStringi => %s\n", string);
    }

    if (__glTracerDispatchTable.GetStringi)
    {
        (*__glTracerDispatchTable.GetStringi)(name, index);
    }

    return string;
}

GLvoid GLAPIENTRY __glProfile_CopyBufferSubData(__GLcontext *gc, GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glCopyBufferSubData 0x%04X 0x%04X %d %d %d\n",
                        gc, tid, readTarget, writeTarget, (GLuint)readOffset, (GLuint)writeOffset, (GLuint)size);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->CopyBufferSubData(gc, readTarget, writeTarget, readOffset, writeOffset, size);
    __GL_PROFILE_FOOTER(GL4_COPYBUFFERSUBDATA);

    if (__glTracerDispatchTable.CopyBufferSubData)
    {
        (*__glTracerDispatchTable.CopyBufferSubData)(readTarget, writeTarget, readOffset, writeOffset, size);
    }
}

GLvoid GLAPIENTRY __glProfile_GetUniformIndices(__GLcontext *gc, GLuint program, GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetUniformIndices %d %d 0x%08X\n",
                        gc, tid, program, uniformCount, __GL_PTR2UINT(uniformNames));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetUniformIndices(gc, program, uniformCount, uniformNames, uniformIndices);
    __GL_PROFILE_FOOTER(GL4_GETUNIFORMINDICES);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetUniformIndices => { ");
        if (uniformCount > 0)
        {
            GLsizei i;
            __GL_LOG_API("uniform[%d] %s", uniformIndices[0], uniformNames[0]);
            for (i = 1; i < uniformCount; ++i)
            {
                __GL_LOG_API(", uniform[%d] %s", uniformIndices[i], uniformNames[i]);
            }
        }
        __GL_LOG_API(" }\n");
    }

    if (__glTracerDispatchTable.GetUniformIndices)
    {
        (*__glTracerDispatchTable.GetUniformIndices)(program, uniformCount, uniformNames, uniformIndices);
    }
}

GLvoid GLAPIENTRY __glProfile_GetActiveUniformsiv(__GLcontext *gc, GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetActiveUniformsiv %d %d 0x%08X 0x%04X 0x%08X\n",
                        gc, tid, program, uniformCount, __GL_PTR2UINT(uniformIndices), pname, __GL_PTR2UINT(params));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetActiveUniformsiv(gc, program, uniformCount, uniformIndices, pname, params);
    __GL_PROFILE_FOOTER(GL4_GETACTIVEUNIFORMSIV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetActiveUniformsiv => { ");
        if (uniformCount > 0)
        {
            GLsizei i;
            __GL_LOG_API("uniform[%d] %d", uniformIndices[0], params[0]);
            for (i = 1; i < uniformCount; ++i)
            {
                __GL_LOG_API(", uniform[%d] %d", uniformIndices[i], params[i]);
            }
        }
        __GL_LOG_API(" }\n");
    }

    if (__glTracerDispatchTable.GetActiveUniformsiv)
    {
        (*__glTracerDispatchTable.GetActiveUniformsiv)(program, uniformCount, uniformIndices, pname, params);
    }
}

GLuint GLAPIENTRY __glProfile_GetUniformBlockIndex(__GLcontext *gc, GLuint program, const GLchar* uniformBlockName)
{
    __GL_PROFILE_VARS();
    GLuint index;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetUniformBlockIndex %d %s\n",
                        gc, tid, program, uniformBlockName);
    }

    __GL_PROFILE_HEADER();
    index = gc->pModeDispatch->GetUniformBlockIndex(gc, program, uniformBlockName);
    __GL_PROFILE_FOOTER(GL4_GETUNIFORMBLOCKINDEX);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetUniformBlockIndex => %d\n", index);
    }

    if (__glTracerDispatchTable.GetUniformBlockIndex)
    {
        (*__glTracerDispatchTable.GetUniformBlockIndex)(program, uniformBlockName, index);
    }

    return index;
}

GLvoid GLAPIENTRY __glProfile_GetActiveUniformBlockiv(__GLcontext *gc, GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetActiveUniformBlockiv %d %d 0x%04X\n",
                        gc, tid, program, uniformBlockIndex, pname);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetActiveUniformBlockiv(gc, program, uniformBlockIndex, pname, params);
    __GL_PROFILE_FOOTER(GL4_GETACTIVEUNIFORMBLOCKIV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetActiveUniformBlockiv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glTracerDispatchTable.GetActiveUniformBlockiv)
    {
        (*__glTracerDispatchTable.GetActiveUniformBlockiv)(program, uniformBlockIndex, pname, params);
    }
}

GLvoid GLAPIENTRY __glProfile_GetActiveUniformBlockName(__GLcontext *gc, GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetActiveUniformBlockName %d %d %d\n",
                        gc, tid, program, uniformBlockIndex, bufSize);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetActiveUniformBlockName(gc, program, uniformBlockIndex, bufSize, length, uniformBlockName);
    __GL_PROFILE_FOOTER(GL4_GETACTIVEUNIFORMBLOCKNAME);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetActiveUniformBlockName => %d %s\n", __GL_PTRVALUE(length), uniformBlockName);
    }

    if (__glTracerDispatchTable.GetActiveUniformBlockName)
    {
        (*__glTracerDispatchTable.GetActiveUniformBlockName)(program, uniformBlockIndex, bufSize, length, uniformBlockName);
    }
}

GLvoid GLAPIENTRY __glProfile_UniformBlockBinding(__GLcontext *gc, GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniformBlockBinding %d %d %d\n",
                        gc, tid, program, uniformBlockIndex, uniformBlockBinding);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->UniformBlockBinding(gc, program, uniformBlockIndex, uniformBlockBinding);
    __GL_PROFILE_FOOTER(GL4_UNIFORMBLOCKBINDING);

    if (__glTracerDispatchTable.UniformBlockBinding)
    {
        (*__glTracerDispatchTable.UniformBlockBinding)(program, uniformBlockIndex, uniformBlockBinding);
    }
}

GLvoid GLAPIENTRY __glProfile_DrawArraysInstanced(__GLcontext *gc, GLenum mode, GLint first, GLsizei count, GLsizei instanceCount)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDrawArraysInstanced 0x%04X %d %d %d\n",
                        gc, tid, mode, first, count, instanceCount);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DrawArraysInstanced(gc, mode, first, count, instanceCount);
    __GL_PROFILE_FOOTER(GL4_DRAWARRAYSINSTANCED);

    if (__glTracerDispatchTable.DrawArraysInstanced)
    {
        (*__glTracerDispatchTable.DrawArraysInstanced)(mode, first, count, instanceCount);
    }
}

GLvoid GLAPIENTRY __glProfile_DrawElementsInstanced(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei instanceCount)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDrawElementsInstanced 0x%04X %d 0x%04X 0x%08X %d\n",
                        gc, tid, mode, count, type, __GL_PTR2UINT(indices), instanceCount);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DrawElementsInstanced(gc, mode, count, type, indices, instanceCount);
    __GL_PROFILE_FOOTER(GL4_DRAWELEMENTSINSTANCED);

    if (__glTracerDispatchTable.DrawElementsInstanced)
    {
        (*__glTracerDispatchTable.DrawElementsInstanced)(mode, count, type, indices, instanceCount);
    }
}

GLsync GLAPIENTRY __glProfile_FenceSync(__GLcontext *gc, GLenum condition, GLbitfield flags)
{
    __GL_PROFILE_VARS();
    GLsync sync;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glFenceSync 0x%04X 0x%08X\n",
                        gc, tid, condition, flags);
    }

    __GL_PROFILE_HEADER();
    sync = gc->pModeDispatch->FenceSync(gc, condition, flags);
    __GL_PROFILE_FOOTER(GL4_FENCESYNC);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glFenceSync => %d\n", __GL_PTR2UINT(sync));
    }

    if (__glTracerDispatchTable.FenceSync)
    {
        (*__glTracerDispatchTable.FenceSync)(condition, flags, sync);
    }

    return sync;
}

GLboolean GLAPIENTRY __glProfile_IsSync(__GLcontext *gc, GLsync sync)
{
    __GL_PROFILE_VARS();
    GLboolean is;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glIsSync 0x%08X\n",
                        gc, tid, __GL_PTR2UINT(sync));
    }

    __GL_PROFILE_HEADER();
    is = gc->pModeDispatch->IsSync(gc, sync);
    __GL_PROFILE_FOOTER(GL4_ISSYNC);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glIsSync => %d\n", is);
    }

    if (__glTracerDispatchTable.IsSync)
    {
        (*__glTracerDispatchTable.IsSync)(sync);
    }

    return is;
}

GLvoid GLAPIENTRY __glProfile_DeleteSync(__GLcontext *gc, GLsync sync)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDeleteSync 0x%08X\n",
                        gc, tid, __GL_PTR2UINT(sync));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DeleteSync(gc, sync);
    __GL_PROFILE_FOOTER(GL4_DELETESYNC);

    if (__glTracerDispatchTable.DeleteSync)
    {
        (*__glTracerDispatchTable.DeleteSync)(sync);
    }
}

GLenum GLAPIENTRY __glProfile_ClientWaitSync(__GLcontext *gc, GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    __GL_PROFILE_VARS();
    GLenum status;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glClientWaitSync 0x%08X 0x%08X 0x%16llX\n",
                        gc, tid, __GL_PTR2UINT(sync), flags, (long long unsigned int)timeout);
    }

    __GL_PROFILE_HEADER();
    status = gc->pModeDispatch->ClientWaitSync(gc, sync, flags, timeout);
    __GL_PROFILE_FOOTER(GL4_CLIENTWAITSYNC);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glClientWaitSync => 0x%04X\n", status);
    }

    if (__glTracerDispatchTable.ClientWaitSync)
    {
        (*__glTracerDispatchTable.ClientWaitSync)(sync, flags, timeout);
    }

    return status;
}

GLvoid GLAPIENTRY __glProfile_WaitSync(__GLcontext *gc, GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glWaitSync 0x%08X 0x%08X 0x%16llX\n",
                        gc, tid, __GL_PTR2UINT(sync), flags, (long long unsigned int)timeout);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->WaitSync(gc, sync, flags, timeout);
    __GL_PROFILE_FOOTER(GL4_WAITSYNC);

    if (__glTracerDispatchTable.WaitSync)
    {
        (*__glTracerDispatchTable.WaitSync)(sync, flags, timeout);
    }
}

GLvoid GLAPIENTRY __glProfile_GetInteger64v(__GLcontext *gc, GLenum pname, GLint64* params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetInteger64v 0x%04X\n",
                        gc, tid, pname);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetInteger64v(gc, pname, params);
    __GL_PROFILE_FOOTER(GL4_GETINTEGER64V);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetInteger64v => 0x%16llX\n", (long long unsigned int)__GL_PTRVALUE(params));
    }

    if (__glTracerDispatchTable.GetInteger64v)
    {
        (*__glTracerDispatchTable.GetInteger64v)(pname, params);
    }
}

GLvoid GLAPIENTRY __glProfile_GetSynciv(__GLcontext *gc, GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetSynciv 0x%08X 0x%04X %d\n",
                        gc, tid, __GL_PTR2UINT(sync), pname, bufSize);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetSynciv(gc, sync, pname, bufSize, length, values);
    __GL_PROFILE_FOOTER(GL4_GETSYNCIV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetSynciv => %d %d\n", __GL_PTRVALUE(length), __GL_PTRVALUE(values));
    }

    if (__glTracerDispatchTable.GetSynciv)
    {
        (*__glTracerDispatchTable.GetSynciv)(sync, pname, bufSize, length, values);
    }
}

GLvoid GLAPIENTRY __glProfile_GetInteger64i_v(__GLcontext *gc, GLenum target, GLuint index, GLint64* data)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetInteger64i_v 0x%04X %d\n",
                        gc, tid, target, index);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetInteger64i_v(gc, target, index, data);
    __GL_PROFILE_FOOTER(GL4_GETINTEGER64I_V);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetInteger64i_v => 0x%16llX\n", (long long unsigned int)__GL_PTRVALUE(data));
    }

    if (__glTracerDispatchTable.GetInteger64i_v)
    {
        (*__glTracerDispatchTable.GetInteger64i_v)(target, index, data);
    }
}

GLvoid GLAPIENTRY __glProfile_GetBufferParameteri64v(__GLcontext *gc, GLenum target, GLenum pname, GLint64* params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetBufferParameteri64v 0x%04X 0x%04X\n",
                        gc, tid, target, pname);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetBufferParameteri64v(gc, target, pname, params);
    __GL_PROFILE_FOOTER(GL4_GETBUFFERPARAMETERI64V);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetBufferParameteri64v => 0x%16llX\n", (long long unsigned int)__GL_PTRVALUE(params));
    }

    if (__glTracerDispatchTable.GetBufferParameteri64v)
    {
        (*__glTracerDispatchTable.GetBufferParameteri64v)(target, pname, params);
    }
}

GLvoid GLAPIENTRY __glProfile_GenSamplers(__GLcontext *gc, GLsizei count, GLuint* samplers)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGenSamplers %d\n",
                        gc, tid, count);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GenSamplers(gc, count, samplers);
    __GL_PROFILE_FOOTER(GL4_GENSAMPLERS);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGenSamplers => ");
        __glLogArrayData(gc, count, samplers);
    }

    if (__glTracerDispatchTable.GenSamplers)
    {
        (*__glTracerDispatchTable.GenSamplers)(count, samplers);
    }
}

GLvoid GLAPIENTRY __glProfile_DeleteSamplers(__GLcontext *gc, GLsizei count, const GLuint* samplers)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDeleteSamplers %d ",
                        gc, tid, count);
        __glLogArrayData(gc, count, samplers);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DeleteSamplers(gc, count, samplers);
    __GL_PROFILE_FOOTER(GL4_DELETESAMPLERS);

    if (__glTracerDispatchTable.DeleteSamplers)
    {
        (*__glTracerDispatchTable.DeleteSamplers)(count, samplers);
    }
}

GLboolean GLAPIENTRY __glProfile_IsSampler(__GLcontext *gc, GLuint sampler)
{
    __GL_PROFILE_VARS();
    GLboolean is;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glIsSampler %d\n",
                        gc, tid, sampler);
    }

    __GL_PROFILE_HEADER();
    is = gc->pModeDispatch->IsSampler(gc, sampler);
    __GL_PROFILE_FOOTER(GL4_ISSAMPLER);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glIsSampler => %d\n", is);
    }

    if (__glTracerDispatchTable.IsSampler)
    {
        (*__glTracerDispatchTable.IsSampler)(sampler);
    }

    return is;
}

GLvoid GLAPIENTRY __glProfile_BindSampler(__GLcontext *gc, GLuint unit, GLuint sampler)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glBindSampler %d %d\n",
                        gc, tid, unit, sampler);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->BindSampler(gc, unit, sampler);
    __GL_PROFILE_FOOTER(GL4_BINDSAMPLER);

    if (__glTracerDispatchTable.BindSampler)
    {
        (*__glTracerDispatchTable.BindSampler)(unit, sampler);
    }
}

GLvoid GLAPIENTRY __glProfile_SamplerParameteri(__GLcontext *gc, GLuint sampler, GLenum pname, GLint param)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glSamplerParameteri %d 0x%04X %d\n",
                        gc, tid, sampler, pname, param);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->SamplerParameteri(gc, sampler, pname, param);
    __GL_PROFILE_FOOTER(GL4_SAMPLERPARAMETERI);

    if (__glTracerDispatchTable.SamplerParameteri)
    {
        (*__glTracerDispatchTable.SamplerParameteri)(sampler, pname, param);
    }
}

GLvoid GLAPIENTRY __glProfile_SamplerParameteriv(__GLcontext *gc, GLuint sampler, GLenum pname, const GLint* param)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glSamplerParameteriv %d 0x%04X 0x%08X\n",
                        gc, tid, sampler, pname, __GL_PTR2UINT(param));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->SamplerParameteriv(gc, sampler, pname, param);
    __GL_PROFILE_FOOTER(GL4_SAMPLERPARAMETERIV);

    if (__glTracerDispatchTable.SamplerParameteriv)
    {
        (*__glTracerDispatchTable.SamplerParameteriv)(sampler, pname, param);
    }
}

GLvoid GLAPIENTRY __glProfile_SamplerParameterf(__GLcontext *gc, GLuint sampler, GLenum pname, GLfloat param)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glSamplerParameterf %d 0x%04X %f\n",
                        gc, tid, sampler, pname, param);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->SamplerParameterf(gc, sampler, pname, param);
    __GL_PROFILE_FOOTER(GL4_SAMPLERPARAMETERF);

    if (__glTracerDispatchTable.SamplerParameterf)
    {
        (*__glTracerDispatchTable.SamplerParameterf)(sampler, pname, param);
    }
}

GLvoid GLAPIENTRY __glProfile_SamplerParameterfv(__GLcontext *gc, GLuint sampler, GLenum pname, const GLfloat* param)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glSamplerParameterfv %d 0x%04X 0x%08X\n",
                        gc, tid, sampler, pname, __GL_PTR2UINT(param));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->SamplerParameterfv(gc, sampler, pname, param);
    __GL_PROFILE_FOOTER(GL4_SAMPLERPARAMETERFV);

    if (__glTracerDispatchTable.SamplerParameterfv)
    {
        (*__glTracerDispatchTable.SamplerParameterfv)(sampler, pname, param);
    }
}

GLvoid GLAPIENTRY __glProfile_GetSamplerParameteriv(__GLcontext *gc, GLuint sampler, GLenum pname, GLint* params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetSamplerParameteriv %d 0x%04X 0x%08X\n",
                        gc, tid, sampler, pname, __GL_PTR2UINT(params));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetSamplerParameteriv(gc, sampler, pname, params);
    __GL_PROFILE_FOOTER(GL4_GETSAMPLERPARAMETERIV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetSamplerParameteriv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glTracerDispatchTable.GetSamplerParameteriv)
    {
        (*__glTracerDispatchTable.GetSamplerParameteriv)(sampler, pname, params);
    }
}

GLvoid GLAPIENTRY __glProfile_GetSamplerParameterfv(__GLcontext *gc, GLuint sampler, GLenum pname, GLfloat* params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetSamplerParameterfv %d 0x%04X\n",
                        gc, tid, sampler, pname);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetSamplerParameterfv(gc, sampler, pname, params);
    __GL_PROFILE_FOOTER(GL4_GETSAMPLERPARAMETERFV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetSamplerParameterfv => %f\n", __GL_PTRVALUE(params));
    }

    if (__glTracerDispatchTable.GetSamplerParameterfv)
    {
        (*__glTracerDispatchTable.GetSamplerParameterfv)(sampler, pname, params);
    }
}

GLvoid GLAPIENTRY __glProfile_VertexAttribDivisor(__GLcontext *gc, GLuint index, GLuint divisor)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttribDivisor %d %d\n",
                        gc, tid, index, divisor);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttribDivisor(gc, index, divisor);
    __GL_PROFILE_FOOTER(GL4_VERTEXATTRIBDIVISOR);

    if (__glTracerDispatchTable.VertexAttribDivisor)
    {
        (*__glTracerDispatchTable.VertexAttribDivisor)(index, divisor);
    }
}

GLvoid GLAPIENTRY __glProfile_BindTransformFeedback(__GLcontext *gc, GLenum target, GLuint id)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glBindTransformFeedback 0x%04X %d\n",
                        gc, tid, target, id);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->BindTransformFeedback(gc, target, id);
    __GL_PROFILE_FOOTER(GL4_BINDTRANSFORMFEEDBACK);

    if (__glTracerDispatchTable.BindTransformFeedback)
    {
        (*__glTracerDispatchTable.BindTransformFeedback)(target, id);
    }
}

GLvoid GLAPIENTRY __glProfile_DeleteTransformFeedbacks(__GLcontext *gc, GLsizei n, const GLuint* ids)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDeleteTransformFeedbacks %d 0x%08X\n",
                        gc, tid, n, __GL_PTR2UINT(ids));
        __glLogArrayData(gc, n, ids);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DeleteTransformFeedbacks(gc, n, ids);
    __GL_PROFILE_FOOTER(GL4_DELETETRANSFORMFEEDBACKS);

    if (__glTracerDispatchTable.DeleteTransformFeedbacks)
    {
        (*__glTracerDispatchTable.DeleteTransformFeedbacks)(n, ids);
    }
}

GLvoid GLAPIENTRY __glProfile_GenTransformFeedbacks(__GLcontext *gc, GLsizei n, GLuint* ids)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGenTransformFeedbacks %d\n",
                        gc, tid, n);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GenTransformFeedbacks(gc, n, ids);
    __GL_PROFILE_FOOTER(GL4_GENTRANSFORMFEEDBACKS);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGenTransformFeedbacks => ");
        __glLogArrayData(gc, n, ids);
    }

    if (__glTracerDispatchTable.GenTransformFeedbacks)
    {
        (*__glTracerDispatchTable.GenTransformFeedbacks)(n, ids);
    }
}

GLboolean GLAPIENTRY __glProfile_IsTransformFeedback(__GLcontext *gc, GLuint id)
{
    __GL_PROFILE_VARS();
    GLboolean is;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glIsTransformFeedback %d\n",
                        gc, tid, id);
    }

    __GL_PROFILE_HEADER();
    is = gc->pModeDispatch->IsTransformFeedback(gc, id);
    __GL_PROFILE_FOOTER(GL4_ISTRANSFORMFEEDBACK);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glIsTransformFeedback => %d\n", is);
    }

    if (__glTracerDispatchTable.IsTransformFeedback)
    {
        (*__glTracerDispatchTable.IsTransformFeedback)(id);
    }

    return is;
}

GLvoid GLAPIENTRY __glProfile_PauseTransformFeedback(__GLcontext *gc)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glPauseTransformFeedback\n", gc, tid);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->PauseTransformFeedback(gc);
    __GL_PROFILE_FOOTER(GL4_PAUSETRANSFORMFEEDBACK);

    if (__glTracerDispatchTable.PauseTransformFeedback)
    {
        (*__glTracerDispatchTable.PauseTransformFeedback)();
    }
}

GLvoid GLAPIENTRY __glProfile_ResumeTransformFeedback(__GLcontext *gc)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glResumeTransformFeedback\n", gc, tid);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ResumeTransformFeedback(gc);
    __GL_PROFILE_FOOTER(GL4_RESUMETRANSFORMFEEDBACK);

    if (__glTracerDispatchTable.ResumeTransformFeedback)
    {
        (*__glTracerDispatchTable.ResumeTransformFeedback)();
    }
}

GLvoid GLAPIENTRY __glProfile_GetProgramBinary(__GLcontext *gc, GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, GLvoid* binary)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetProgramBinary %d %d\n",
                        gc, tid, program, bufSize);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetProgramBinary(gc, program, bufSize, length, binaryFormat, binary);
    __GL_PROFILE_FOOTER(GL4_GETPROGRAMBINARY);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetProgramBinary => %d 0x%04X 0x%08X\n",
                       __GL_PTRVALUE(length), __GL_PTRVALUE(binaryFormat), __GL_PTR2UINT(binary));
    }

    if (__glTracerDispatchTable.GetProgramBinary)
    {
        (*__glTracerDispatchTable.GetProgramBinary)(program, bufSize, length, binaryFormat, binary);
    }
}

GLvoid GLAPIENTRY __glProfile_ProgramBinary(__GLcontext *gc, GLuint program, GLenum binaryFormat, const GLvoid* binary, GLsizei length)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glProgramBinary %d 0x%04X 0x%08X %d\n",
                        gc, tid, program, binaryFormat, __GL_PTR2UINT(binary), length);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ProgramBinary(gc, program, binaryFormat, binary, length);
    __GL_PROFILE_FOOTER(GL4_PROGRAMBINARY);

    if (__glTracerDispatchTable.ProgramBinary)
    {
        (*__glTracerDispatchTable.ProgramBinary)(program, binaryFormat, binary, length);
    }
}

GLvoid GLAPIENTRY __glProfile_ProgramParameteri(__GLcontext *gc, GLuint program, GLenum pname, GLint value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glProgramParameteri %d 0x%04X %d\n",
                        gc, tid, program, pname, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ProgramParameteri(gc, program, pname, value);
    __GL_PROFILE_FOOTER(GL4_PROGRAMPARAMETERI);

    if (__glTracerDispatchTable.ProgramParameteri)
    {
        (*__glTracerDispatchTable.ProgramParameteri)(program, pname, value);
    }
}

GLvoid GLAPIENTRY __glProfile_InvalidateFramebuffer(__GLcontext *gc, GLenum target, GLsizei numAttachments, const GLenum* attachments)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glInvalidateFramebuffer 0x%04X %d 0x%08X\n",
                        gc, tid, target, numAttachments, __GL_PTR2UINT(attachments));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->InvalidateFramebuffer(gc, target, numAttachments, attachments);
    __GL_PROFILE_FOOTER(GL4_INVALIDATEFRAMEBUFFER);

    if (__glTracerDispatchTable.InvalidateFramebuffer)
    {
        (*__glTracerDispatchTable.InvalidateFramebuffer)(target, numAttachments, attachments);
    }
}

GLvoid GLAPIENTRY __glProfile_InvalidateSubFramebuffer(__GLcontext *gc, GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glInvalidateSubFramebuffer 0x%04X %d 0x%08X %d %d %d %d\n",
                        gc, tid, target, numAttachments, __GL_PTR2UINT(attachments), x, y, width, height);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->InvalidateSubFramebuffer(gc, target, numAttachments, attachments, x, y, width, height);
    __GL_PROFILE_FOOTER(GL4_INVALIDATESUBFRAMEBUFFER);

    if (__glTracerDispatchTable.InvalidateSubFramebuffer)
    {
        (*__glTracerDispatchTable.InvalidateSubFramebuffer)(target, numAttachments, attachments, x, y, width, height);
    }
}

GLvoid GLAPIENTRY __glProfile_TexStorage2D(__GLcontext *gc, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexStorage2D 0x%04X %d 0x%04X %d %d\n",
                        gc, tid, target, levels, internalformat, width, height);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexStorage2D(gc, target, levels, internalformat, width, height);
    __GL_PROFILE_FOOTER(GL4_TEXSTORAGE2D);

    if (__glTracerDispatchTable.TexStorage2D)
    {
        (*__glTracerDispatchTable.TexStorage2D)(target, levels, internalformat, width, height);
    }
}

GLvoid GLAPIENTRY __glProfile_TexStorage3D(__GLcontext *gc, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexStorage3D 0x%04X %d 0x%04X %d %d %d\n",
                        gc, tid, target, levels, internalformat, width, height, depth);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexStorage3D(gc, target, levels, internalformat, width, height, depth);
    __GL_PROFILE_FOOTER(GL4_TEXSTORAGE3D);

    if (__glTracerDispatchTable.TexStorage3D)
    {
        (*__glTracerDispatchTable.TexStorage3D)(target, levels, internalformat, width, height, depth);
    }
}

GLvoid GLAPIENTRY __glProfile_GetInternalformativ(__GLcontext *gc, GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetInternalformativ 0x%04X 0x%04X 0x%04X %d\n",
                        gc, tid, target, internalformat, pname, bufSize);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetInternalformativ(gc, target, internalformat, pname, bufSize, params);
    __GL_PROFILE_FOOTER(GL4_GETINTERNALFORMATIV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetInternalformativ => %d\n", __GL_PTRVALUE(params));
    }

    if (__glTracerDispatchTable.GetInternalformativ)
    {
        (*__glTracerDispatchTable.GetInternalformativ)(target, internalformat, pname, bufSize, params);
    }
}

/*
** OpenGL ES 3.1
*/
GLvoid GLAPIENTRY __glProfile_DispatchCompute(__GLcontext *gc, GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDispatchCompute %d %d %d\n",
                        gc, tid, num_groups_x, num_groups_y, num_groups_z);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DispatchCompute(gc, num_groups_x, num_groups_y, num_groups_z);
    __GL_PROFILE_FOOTER(GL4_DISPATCHCOMPUTE);

    if (__glTracerDispatchTable.DispatchCompute)
    {
        (*__glTracerDispatchTable.DispatchCompute)(num_groups_x, num_groups_y, num_groups_z);
    }
}

GLvoid GLAPIENTRY __glProfile_DispatchComputeIndirect(__GLcontext *gc, GLintptr indirect)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDispatchComputeIndirect 0x%08X\n",
                        gc, tid, indirect);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DispatchComputeIndirect(gc, indirect);
    __GL_PROFILE_FOOTER(GL4_DISPATCHCOMPUTEINDIRECT);

    if (__glTracerDispatchTable.DispatchComputeIndirect)
    {
        (*__glTracerDispatchTable.DispatchComputeIndirect)(indirect);
    }
}

GLvoid GLAPIENTRY __glProfile_DrawArraysIndirect(__GLcontext *gc, GLenum mode, const void *indirect)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDrawArraysIndirect 0x%04X 0x%08X\n",
                        gc, tid, mode, indirect);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DrawArraysIndirect(gc, mode, indirect);
    __GL_PROFILE_FOOTER(GL4_DRAWARRAYSINDIRECT);

    if (__glTracerDispatchTable.DrawArraysIndirect)
    {
        (*__glTracerDispatchTable.DrawArraysIndirect)(mode, indirect);
    }
}

GLvoid GLAPIENTRY __glProfile_DrawElementsIndirect(__GLcontext *gc, GLenum mode, GLenum type, const void *indirect)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDrawElementsIndirect 0x%04X 0x%04X 0x%08X\n",
                        gc, tid, mode, type, indirect);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DrawElementsIndirect(gc, mode, type, indirect);
    __GL_PROFILE_FOOTER(GL4_DRAWELEMENTSINDIRECT);

    if (__glTracerDispatchTable.DrawElementsIndirect)
    {
        (*__glTracerDispatchTable.DrawElementsIndirect)(mode, type, indirect);
    }
}

GLvoid GLAPIENTRY __glProfile_FramebufferParameteri(__GLcontext *gc, GLenum target, GLenum pname, GLint param)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glFramebufferParameteri 0x%04X 0x%04X %d\n",
                        gc, tid, target, pname, param);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->FramebufferParameteri(gc, target, pname, param);
    __GL_PROFILE_FOOTER(GL4_FRAMEBUFFERPARAMETERI);

    if (__glTracerDispatchTable.FramebufferParameteri)
    {
        (*__glTracerDispatchTable.FramebufferParameteri)(target, pname, param);
    }
}

GLvoid GLAPIENTRY __glProfile_GetFramebufferParameteriv(__GLcontext *gc, GLenum target, GLenum pname, GLint *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetFramebufferParameteriv 0x%04X 0x%04X 0x%08X\n",
                        gc, tid, target, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetFramebufferParameteriv(gc, target, pname, params);
    __GL_PROFILE_FOOTER(GL4_GETFRAMEBUFFERPARAMETERIV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetFramebufferParameteriv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glTracerDispatchTable.GetFramebufferParameteriv)
    {
        (*__glTracerDispatchTable.GetFramebufferParameteriv)(target, pname, params);
    }
}

GLvoid GLAPIENTRY __glProfile_GetProgramInterfaceiv(__GLcontext *gc, GLuint program, GLenum programInterface, GLenum pname, GLint *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetProgramInterfaceiv %d 0x%04X 0x%04X 0x%08X\n",
                        gc, tid, program, programInterface, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetProgramInterfaceiv(gc, program, programInterface, pname, params);
    __GL_PROFILE_FOOTER(GL4_GETPROGRAMINTERFACEIV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetProgramInterfaceiv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glTracerDispatchTable.GetProgramInterfaceiv)
    {
        (*__glTracerDispatchTable.GetProgramInterfaceiv)(program, programInterface, pname, params);
    }
}

GLuint GLAPIENTRY __glProfile_GetProgramResourceIndex(__GLcontext *gc, GLuint program, GLenum programInterface, const GLchar *name)
{
    __GL_PROFILE_VARS();
    GLuint index;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetProgramResourceIndex %d 0x%04X %s\n",
                        gc, tid, program, programInterface, name);
    }

    __GL_PROFILE_HEADER();
    index = gc->pModeDispatch->GetProgramResourceIndex(gc, program, programInterface, name);
    __GL_PROFILE_FOOTER(GL4_GETPROGRAMRESOURCEINDEX);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetProgramResourceIndex => %d\n", index);
    }

    if (__glTracerDispatchTable.GetProgramResourceIndex)
    {
        (*__glTracerDispatchTable.GetProgramResourceIndex)(program, programInterface, name);
    }

    return index;
}

GLvoid GLAPIENTRY __glProfile_GetProgramResourceName(__GLcontext *gc, GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetProgramResourceName %d 0x%04X %d %d\n",
                        gc, tid, program, programInterface, index, bufSize);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetProgramResourceName(gc, program, programInterface, index, bufSize, length, name);
    __GL_PROFILE_FOOTER(GL4_GETPROGRAMRESOURCENAME);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetProgramResourceName => %d %s\n", __GL_PTRVALUE(length), name);
    }

    if (__glTracerDispatchTable.GetProgramResourceName)
    {
        (*__glTracerDispatchTable.GetProgramResourceName)(program, programInterface, index, bufSize, length, name);
    }
}

GLvoid GLAPIENTRY __glProfile_GetProgramResourceiv(__GLcontext *gc, GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetProgramResourceiv %d 0x%04X %d %d 0x%08X %d\n",
                        gc, tid, program, programInterface, index, propCount, props, bufSize);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetProgramResourceiv(gc, program, programInterface, index, propCount, props, bufSize, length, params);
    __GL_PROFILE_FOOTER(GL4_GETPROGRAMRESOURCEIV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetProgramResourceiv => %d %d\n", __GL_PTRVALUE(length), __GL_PTRVALUE(params));
    }

    if (__glTracerDispatchTable.GetProgramResourceiv)
    {
        (*__glTracerDispatchTable.GetProgramResourceiv)(program, programInterface, index, propCount, props, bufSize, length, params);
    }
}

GLint GLAPIENTRY __glProfile_GetProgramResourceLocation(__GLcontext *gc, GLuint program, GLenum programInterface, const GLchar *name)
{
    __GL_PROFILE_VARS();
    GLint location;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetProgramResourceLocation %d 0x%04X %s\n",
                        gc, tid, program, programInterface, name);
    }

    __GL_PROFILE_HEADER();
    location = gc->pModeDispatch->GetProgramResourceLocation(gc, program, programInterface, name);
    __GL_PROFILE_FOOTER(GL4_GETPROGRAMRESOURCELOCATION);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetProgramResourceLocation => %d\n", location);
    }

    if (__glTracerDispatchTable.GetProgramResourceLocation)
    {
        (*__glTracerDispatchTable.GetProgramResourceLocation)(program, programInterface, name);
    }
    return location;
}

GLvoid GLAPIENTRY __glProfile_UseProgramStages(__GLcontext *gc, GLuint pipeline, GLbitfield stages, GLuint program)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUseProgramStages %d 0x%08X %d\n",
                        gc, tid, pipeline, stages, program);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->UseProgramStages(gc, pipeline, stages, program);
    __GL_PROFILE_FOOTER(GL4_USEPROGRAMSTAGES);

    if (__glTracerDispatchTable.UseProgramStages)
    {
        (*__glTracerDispatchTable.UseProgramStages)(pipeline, stages, program);
    }
}

GLvoid GLAPIENTRY __glProfile_ActiveShaderProgram(__GLcontext *gc, GLuint pipeline, GLuint program)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glActiveShaderProgram %d %d\n",
                        gc, tid, pipeline, program);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ActiveShaderProgram(gc, pipeline, program);
    __GL_PROFILE_FOOTER(GL4_ACTIVESHADERPROGRAM);

    if (__glTracerDispatchTable.ActiveShaderProgram)
    {
        (*__glTracerDispatchTable.ActiveShaderProgram)(pipeline, program);
    }
}

GLuint GLAPIENTRY __glProfile_CreateShaderProgramv(__GLcontext *gc, GLenum type, GLsizei count, const GLchar *const*strings)
{
    __GL_PROFILE_VARS();
    GLint shader;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glCreateShaderProgramv 0x%04X %d 0x%08X\n",
                        gc, tid, type, count, __GL_PTR2UINT(strings));

        __glLogSourceStrings(gc, count, strings);
    }

    __GL_PROFILE_HEADER();
    shader = gc->pModeDispatch->CreateShaderProgramv(gc, type, count, strings);
    __GL_PROFILE_FOOTER(GL4_CREATESHADERPROGRAMV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glCreateShaderProgramv => %d\n", shader);
    }

    if (__glTracerDispatchTable.CreateShaderProgramv)
    {
        (*__glTracerDispatchTable.CreateShaderProgramv)(type, count, strings);
    }

    return shader;
}

GLvoid GLAPIENTRY __glProfile_BindProgramPipeline(__GLcontext *gc, GLuint pipeline)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glBindProgramPipeline %d\n",
                        gc, tid, pipeline);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->BindProgramPipeline(gc, pipeline);
    __GL_PROFILE_FOOTER(GL4_BINDPROGRAMPIPELINE);

    if (__glTracerDispatchTable.BindProgramPipeline)
    {
        (*__glTracerDispatchTable.BindProgramPipeline)(pipeline);
    }
}

GLvoid GLAPIENTRY __glProfile_DeleteProgramPipelines(__GLcontext *gc, GLsizei n, const GLuint *pipelines)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDeleteProgramPipelines %d ",
                        gc, tid, n);
        __glLogArrayData(gc, n, pipelines);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DeleteProgramPipelines(gc, n, pipelines);
    __GL_PROFILE_FOOTER(GL4_DELETEPROGRAMPIPELINES);

    if (__glTracerDispatchTable.DeleteProgramPipelines)
    {
        (*__glTracerDispatchTable.DeleteProgramPipelines)(n, pipelines);
    }
}

GLvoid GLAPIENTRY __glProfile_GenProgramPipelines(__GLcontext *gc, GLsizei n, GLuint *pipelines)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGenProgramPipelines %d\n",
                        gc, tid, n);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GenProgramPipelines(gc, n, pipelines);
    __GL_PROFILE_FOOTER(GL4_GENPROGRAMPIPELINES);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGenProgramPipelines => ");
        __glLogArrayData(gc, n, pipelines);
    }

    if (__glTracerDispatchTable.GenProgramPipelines)
    {
        (*__glTracerDispatchTable.GenProgramPipelines)(n, pipelines);
    }
}

GLboolean GLAPIENTRY __glProfile_IsProgramPipeline(__GLcontext *gc, GLuint pipeline)
{
    __GL_PROFILE_VARS();
    GLboolean result;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glIsProgramPipeline %d\n",
                        gc, tid, pipeline);
    }

    __GL_PROFILE_HEADER();
    result = gc->pModeDispatch->IsProgramPipeline(gc, pipeline);
    __GL_PROFILE_FOOTER(GL4_ISPROGRAMPIPELINE);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glIsProgramPipeline => %d\n", result);
    }

    if (__glTracerDispatchTable.IsProgramPipeline)
    {
        (*__glTracerDispatchTable.IsProgramPipeline)(pipeline);
    }

    return result;
}

GLvoid GLAPIENTRY __glProfile_GetProgramPipelineiv(__GLcontext *gc, GLuint pipeline, GLenum pname, GLint *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetProgramPipelineiv %d 0x%04X 0x%08X\n",
                        gc, tid, pipeline, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetProgramPipelineiv(gc, pipeline, pname, params);
    __GL_PROFILE_FOOTER(GL4_GETPROGRAMPIPELINEIV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetProgramPipelineiv => %d\n", *params);
    }

    if (__glTracerDispatchTable.GetProgramPipelineiv)
    {
        (*__glTracerDispatchTable.GetProgramPipelineiv)(pipeline, pname, params);
    }
}

GLvoid GLAPIENTRY __glProfile_ProgramUniform1i(__GLcontext *gc, GLuint program, GLint location, GLint v0)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glProgramUniform1i %d %d %d\n",
                        gc, tid, program, location, v0);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ProgramUniform1i(gc, program, location, v0);
    __GL_PROFILE_FOOTER(GL4_PROGRAMUNIFORM1I);

    if (__glTracerDispatchTable.ProgramUniform1i)
    {
        (*__glTracerDispatchTable.ProgramUniform1i)(program, location, v0);
    }
}

GLvoid GLAPIENTRY __glProfile_ProgramUniform2i(__GLcontext *gc, GLuint program, GLint location, GLint v0, GLint v1)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glProgramUniform2i %d %d %d %d\n",
                        gc, tid, program, location, v0, v1);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ProgramUniform2i(gc, program, location, v0, v1);
    __GL_PROFILE_FOOTER(GL4_PROGRAMUNIFORM2I);

    if (__glTracerDispatchTable.ProgramUniform2i)
    {
        (*__glTracerDispatchTable.ProgramUniform2i)(program, location, v0, v1);
    }
}

GLvoid GLAPIENTRY __glProfile_ProgramUniform3i(__GLcontext *gc, GLuint program, GLint location, GLint v0, GLint v1, GLint v2)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glProgramUniform3i %d %d %d %d %d\n",
                        gc, tid, program, location, v0, v1, v2);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ProgramUniform3i(gc, program, location, v0, v1, v2);
    __GL_PROFILE_FOOTER(GL4_PROGRAMUNIFORM3I);

    if (__glTracerDispatchTable.ProgramUniform3i)
    {
        (*__glTracerDispatchTable.ProgramUniform3i)(program, location, v0, v1, v2);
    }
}

GLvoid GLAPIENTRY __glProfile_ProgramUniform4i(__GLcontext *gc, GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glProgramUniform4i %d %d %d %d %d %d\n",
                        gc, tid, program, location, v0, v1, v2, v3);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ProgramUniform4i(gc, program, location, v0, v1, v2, v3);
    __GL_PROFILE_FOOTER(GL4_PROGRAMUNIFORM4I);

    if (__glTracerDispatchTable.ProgramUniform4i)
    {
        (*__glTracerDispatchTable.ProgramUniform4i)(program, location, v0, v1, v2, v3);
    }
}

GLvoid GLAPIENTRY __glProfile_ProgramUniform1ui(__GLcontext *gc, GLuint program, GLint location, GLuint v0)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glProgramUniform1ui %d %d %d\n",
                        gc, tid, program, location, v0);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ProgramUniform1ui(gc, program, location, v0);
    __GL_PROFILE_FOOTER(GL4_PROGRAMUNIFORM1UI);

    if (__glTracerDispatchTable.ProgramUniform1ui)
    {
        (*__glTracerDispatchTable.ProgramUniform1ui)(program, location, v0);
    }
}

GLvoid GLAPIENTRY __glProfile_ProgramUniform2ui(__GLcontext *gc, GLuint program, GLint location, GLuint v0, GLuint v1)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glProgramUniform2ui %d %d %d %d\n",
                        gc, tid, program, location, v0, v1);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ProgramUniform2ui(gc, program, location, v0, v1);
    __GL_PROFILE_FOOTER(GL4_PROGRAMUNIFORM2UI);

    if (__glTracerDispatchTable.ProgramUniform2ui)
    {
        (*__glTracerDispatchTable.ProgramUniform2ui)(program, location, v0, v1);
    }
}

GLvoid GLAPIENTRY __glProfile_ProgramUniform3ui(__GLcontext *gc, GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glProgramUniform3ui %d %d %d %d %d\n",
                        gc, tid, program, location, v0, v1, v2);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ProgramUniform3ui(gc, program, location, v0, v1, v2);
    __GL_PROFILE_FOOTER(GL4_PROGRAMUNIFORM3UI);

    if (__glTracerDispatchTable.ProgramUniform3ui)
    {
        (*__glTracerDispatchTable.ProgramUniform3ui)(program, location, v0, v1, v2);
    }
}

GLvoid GLAPIENTRY __glProfile_ProgramUniform4ui(__GLcontext *gc, GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glProgramUniform4ui %d %d %d %d %d %d\n",
                        gc, tid, program, location, v0, v1, v2, v3);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ProgramUniform4ui(gc, program, location, v0, v1, v2, v3);
    __GL_PROFILE_FOOTER(GL4_PROGRAMUNIFORM4UI);

    if (__glTracerDispatchTable.ProgramUniform4ui)
    {
        (*__glTracerDispatchTable.ProgramUniform4ui)(program, location, v0, v1, v2, v3);
    }
}

GLvoid GLAPIENTRY __glProfile_ProgramUniform1f(__GLcontext *gc, GLuint program, GLint location, GLfloat v0)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glProgramUniform1f %d %d %f\n",
                        gc, tid, program, location, v0);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ProgramUniform1f(gc, program, location, v0);
    __GL_PROFILE_FOOTER(GL4_PROGRAMUNIFORM1F);

    if (__glTracerDispatchTable.ProgramUniform1f)
    {
        (*__glTracerDispatchTable.ProgramUniform1f)(program, location, v0);
    }
}

GLvoid GLAPIENTRY __glProfile_ProgramUniform2f(__GLcontext *gc, GLuint program, GLint location, GLfloat v0, GLfloat v1)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glProgramUniform2f %d %d %f %f\n",
                        gc, tid, program, location, v0, v1);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ProgramUniform2f(gc, program, location, v0, v1);
    __GL_PROFILE_FOOTER(GL4_PROGRAMUNIFORM2F);

    if (__glTracerDispatchTable.ProgramUniform2f)
    {
        (*__glTracerDispatchTable.ProgramUniform2f)(program, location, v0, v1);
    }
}

GLvoid GLAPIENTRY __glProfile_ProgramUniform3f(__GLcontext *gc, GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glProgramUniform3f %d %d %f %f %f\n",
                        gc, tid, program, location, v0, v1, v2);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ProgramUniform3f(gc, program, location, v0, v1, v2);
    __GL_PROFILE_FOOTER(GL4_PROGRAMUNIFORM3F);

    if (__glTracerDispatchTable.ProgramUniform3f)
    {
        (*__glTracerDispatchTable.ProgramUniform3f)(program, location, v0, v1, v2);
    }
}

GLvoid GLAPIENTRY __glProfile_ProgramUniform4f(__GLcontext *gc, GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glProgramUniform4f %d %d %f %f %f %f\n",
                        gc, tid, program, location, v0, v1, v2, v3);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ProgramUniform4f(gc, program, location, v0, v1, v2, v3);
    __GL_PROFILE_FOOTER(GL4_PROGRAMUNIFORM4F);

    if (__glTracerDispatchTable.ProgramUniform4f)
    {
        (*__glTracerDispatchTable.ProgramUniform4f)(program, location, v0, v1, v2, v3);
    }
}

GLvoid GLAPIENTRY __glProfile_ProgramUniform1iv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLint *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glProgramUniform1iv %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ProgramUniform1iv(gc, program, location, count, value);
    __GL_PROFILE_FOOTER(GL4_PROGRAMUNIFORM1IV);

    if (__glTracerDispatchTable.ProgramUniform1iv)
    {
        (*__glTracerDispatchTable.ProgramUniform1iv)(program, location, count, value);
    }
}

GLvoid GLAPIENTRY __glProfile_ProgramUniform2iv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLint *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glProgramUniform2iv %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ProgramUniform2iv(gc, program, location, count, value);
    __GL_PROFILE_FOOTER(GL4_PROGRAMUNIFORM2IV);

    if (__glTracerDispatchTable.ProgramUniform2iv)
    {
        (*__glTracerDispatchTable.ProgramUniform2iv)(program, location, count, value);
    }
}

GLvoid GLAPIENTRY __glProfile_ProgramUniform3iv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLint *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glProgramUniform3iv %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ProgramUniform3iv(gc, program, location, count, value);
    __GL_PROFILE_FOOTER(GL4_PROGRAMUNIFORM3IV);

    if (__glTracerDispatchTable.ProgramUniform3iv)
    {
        (*__glTracerDispatchTable.ProgramUniform3iv)(program, location, count, value);
    }
}

GLvoid GLAPIENTRY __glProfile_ProgramUniform4iv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLint *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glProgramUniform4iv %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ProgramUniform4iv(gc, program, location, count, value);
    __GL_PROFILE_FOOTER(GL4_PROGRAMUNIFORM4IV);

    if (__glTracerDispatchTable.ProgramUniform4iv)
    {
        (*__glTracerDispatchTable.ProgramUniform4iv)(program, location, count, value);
    }
}

GLvoid GLAPIENTRY __glProfile_ProgramUniform1uiv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLuint *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glProgramUniform1uiv %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ProgramUniform1uiv(gc, program, location, count, value);
    __GL_PROFILE_FOOTER(GL4_PROGRAMUNIFORM1UIV);

    if (__glTracerDispatchTable.ProgramUniform1uiv)
    {
        (*__glTracerDispatchTable.ProgramUniform1uiv)(program, location, count, value);
    }
}

GLvoid GLAPIENTRY __glProfile_ProgramUniform2uiv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLuint *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glProgramUniform2uiv %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ProgramUniform2uiv(gc, program, location, count, value);
    __GL_PROFILE_FOOTER(GL4_PROGRAMUNIFORM2UIV);

    if (__glTracerDispatchTable.ProgramUniform2uiv)
    {
        (*__glTracerDispatchTable.ProgramUniform2uiv)(program, location, count, value);
    }
}

GLvoid GLAPIENTRY __glProfile_ProgramUniform3uiv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLuint *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glProgramUniform3uiv %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ProgramUniform3uiv(gc, program, location, count, value);
    __GL_PROFILE_FOOTER(GL4_PROGRAMUNIFORM3UIV);

    if (__glTracerDispatchTable.ProgramUniform3uiv)
    {
        (*__glTracerDispatchTable.ProgramUniform3uiv)(program, location, count, value);
    }
}

GLvoid GLAPIENTRY __glProfile_ProgramUniform4uiv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLuint *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glProgramUniform4uiv %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ProgramUniform4uiv(gc, program, location, count, value);
    __GL_PROFILE_FOOTER(GL4_PROGRAMUNIFORM4UIV);

    if (__glTracerDispatchTable.ProgramUniform4uiv)
    {
        (*__glTracerDispatchTable.ProgramUniform4uiv)(program, location, count, value);
    }
}

GLvoid GLAPIENTRY __glProfile_ProgramUniform1fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glProgramUniform1fv %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ProgramUniform1fv(gc, program, location, count, value);
    __GL_PROFILE_FOOTER(GL4_PROGRAMUNIFORM1FV);

    if (__glTracerDispatchTable.ProgramUniform1fv)
    {
        (*__glTracerDispatchTable.ProgramUniform1fv)(program, location, count, value);
    }
}

GLvoid GLAPIENTRY __glProfile_ProgramUniform2fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glProgramUniform2fv %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ProgramUniform2fv(gc, program, location, count, value);
    __GL_PROFILE_FOOTER(GL4_PROGRAMUNIFORM2FV);

    if (__glTracerDispatchTable.ProgramUniform2fv)
    {
        (*__glTracerDispatchTable.ProgramUniform2fv)(program, location, count, value);
    }
}

GLvoid GLAPIENTRY __glProfile_ProgramUniform3fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glProgramUniform3fv %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ProgramUniform3fv(gc, program, location, count, value);
    __GL_PROFILE_FOOTER(GL4_PROGRAMUNIFORM3FV);

    if (__glTracerDispatchTable.ProgramUniform3fv)
    {
        (*__glTracerDispatchTable.ProgramUniform3fv)(program, location, count, value);
    }
}

GLvoid GLAPIENTRY __glProfile_ProgramUniform4fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glProgramUniform4fv %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ProgramUniform4fv(gc, program, location, count, value);
    __GL_PROFILE_FOOTER(GL4_PROGRAMUNIFORM4FV);

    if (__glTracerDispatchTable.ProgramUniform4fv)
    {
        (*__glTracerDispatchTable.ProgramUniform4fv)(program, location, count, value);
    }
}

GLvoid GLAPIENTRY __glProfile_ProgramUniformMatrix2fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glProgramUniformMatrix2fv %d %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, transpose, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ProgramUniformMatrix2fv(gc, program, location, count, transpose, value);
    __GL_PROFILE_FOOTER(GL4_PROGRAMUNIFORMMATRIX2FV);

    if (__glTracerDispatchTable.ProgramUniformMatrix2fv)
    {
        (*__glTracerDispatchTable.ProgramUniformMatrix2fv)(program, location, count, transpose, value);
    }
}

GLvoid GLAPIENTRY __glProfile_ProgramUniformMatrix3fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glProgramUniformMatrix3fv %d %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, transpose, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ProgramUniformMatrix3fv(gc, program, location, count, transpose, value);
    __GL_PROFILE_FOOTER(GL4_PROGRAMUNIFORMMATRIX3FV);

    if (__glTracerDispatchTable.ProgramUniformMatrix3fv)
    {
        (*__glTracerDispatchTable.ProgramUniformMatrix3fv)(program, location, count, transpose, value);
    }
}

GLvoid GLAPIENTRY __glProfile_ProgramUniformMatrix4fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glProgramUniformMatrix4fv %d %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, transpose, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ProgramUniformMatrix4fv(gc, program, location, count, transpose, value);
    __GL_PROFILE_FOOTER(GL4_PROGRAMUNIFORMMATRIX4FV);

    if (__glTracerDispatchTable.ProgramUniformMatrix4fv)
    {
        (*__glTracerDispatchTable.ProgramUniformMatrix4fv)(program, location, count, transpose, value);
    }
}

GLvoid GLAPIENTRY __glProfile_ProgramUniformMatrix2x3fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glProgramUniformMatrix2x3fv %d %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, transpose, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ProgramUniformMatrix2x3fv(gc, program, location, count, transpose, value);
    __GL_PROFILE_FOOTER(GL4_PROGRAMUNIFORMMATRIX2X3FV);

    if (__glTracerDispatchTable.ProgramUniformMatrix2x3fv)
    {
        (*__glTracerDispatchTable.ProgramUniformMatrix2x3fv)(program, location, count, transpose, value);
    }
}

GLvoid GLAPIENTRY __glProfile_ProgramUniformMatrix3x2fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glProgramUniformMatrix3x2fv %d %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, transpose, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ProgramUniformMatrix3x2fv(gc, program, location, count, transpose, value);
    __GL_PROFILE_FOOTER(GL4_PROGRAMUNIFORMMATRIX3X2FV);

    if (__glTracerDispatchTable.ProgramUniformMatrix3x2fv)
    {
        (*__glTracerDispatchTable.ProgramUniformMatrix3x2fv)(program, location, count, transpose, value);
    }
}

GLvoid GLAPIENTRY __glProfile_ProgramUniformMatrix2x4fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glProgramUniformMatrix2x4fv %d %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, transpose, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ProgramUniformMatrix2x4fv(gc, program, location, count, transpose, value);
    __GL_PROFILE_FOOTER(GL4_PROGRAMUNIFORMMATRIX2X4FV);

    if (__glTracerDispatchTable.ProgramUniformMatrix2x4fv)
    {
        (*__glTracerDispatchTable.ProgramUniformMatrix2x4fv)(program, location, count, transpose, value);
    }
}

GLvoid GLAPIENTRY __glProfile_ProgramUniformMatrix4x2fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glProgramUniformMatrix4x2fv %d %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, transpose, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ProgramUniformMatrix4x2fv(gc, program, location, count, transpose, value);
    __GL_PROFILE_FOOTER(GL4_PROGRAMUNIFORMMATRIX4X2FV);

    if (__glTracerDispatchTable.ProgramUniformMatrix4x2fv)
    {
        (*__glTracerDispatchTable.ProgramUniformMatrix4x2fv)(program, location, count, transpose, value);
    }
}

GLvoid GLAPIENTRY __glProfile_ProgramUniformMatrix3x4fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glProgramUniformMatrix3x4fv %d %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, transpose, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ProgramUniformMatrix3x4fv(gc, program, location, count, transpose, value);
    __GL_PROFILE_FOOTER(GL4_PROGRAMUNIFORMMATRIX3X4FV);

    if (__glTracerDispatchTable.ProgramUniformMatrix3x4fv)
    {
        (*__glTracerDispatchTable.ProgramUniformMatrix3x4fv)(program, location, count, transpose, value);
    }
}

GLvoid GLAPIENTRY __glProfile_ProgramUniformMatrix4x3fv(__GLcontext *gc, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glProgramUniformMatrix4x3fv %d %d %d %d 0x%08X\n",
                        gc, tid, program, location, count, transpose, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ProgramUniformMatrix4x3fv(gc, program, location, count, transpose, value);
    __GL_PROFILE_FOOTER(GL4_PROGRAMUNIFORMMATRIX4X3FV);

    if (__glTracerDispatchTable.ProgramUniformMatrix4x3fv)
    {
        (*__glTracerDispatchTable.ProgramUniformMatrix4x3fv)(program, location, count, transpose, value);
    }
}

GLvoid GLAPIENTRY __glProfile_ValidateProgramPipeline(__GLcontext *gc, GLuint pipeline)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glValidateProgramPipeline %d\n",
                        gc, tid, pipeline);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ValidateProgramPipeline(gc, pipeline);
    __GL_PROFILE_FOOTER(GL4_VALIDATEPROGRAMPIPELINE);

    if (__glTracerDispatchTable.ValidateProgramPipeline)
    {
        (*__glTracerDispatchTable.ValidateProgramPipeline)(pipeline);
    }
}

GLvoid GLAPIENTRY __glProfile_GetProgramPipelineInfoLog(__GLcontext *gc, GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetProgramPipelineInfoLog %d %d 0x%08X 0x%08X\n",
                        gc, tid, pipeline, bufSize, length, infoLog);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetProgramPipelineInfoLog(gc, pipeline, bufSize, length, infoLog);
    __GL_PROFILE_FOOTER(GL4_GETPROGRAMPIPELINEINFOLOG);

    if (bufSize && (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST))
    {
        __GL_LOG_API("        glGetProgramPipelineInfoLog => %s\n", infoLog);
    }

    if (__glTracerDispatchTable.GetProgramPipelineInfoLog)
    {
        (*__glTracerDispatchTable.GetProgramPipelineInfoLog)(pipeline, bufSize, length, infoLog);
    }
}

GLvoid GLAPIENTRY __glProfile_BindImageTexture(__GLcontext *gc, GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glBindImageTexture %d %d %d %d %d 0x%04X 0x%04X\n",
                        gc, tid, unit, texture, level, layered, layer, access, format);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->BindImageTexture(gc, unit, texture, level, layered, layer, access, format);
    __GL_PROFILE_FOOTER(GL4_BINDIMAGETEXTURE);

    if (__glTracerDispatchTable.BindImageTexture)
    {
        (*__glTracerDispatchTable.BindImageTexture)(unit, texture, level, layered, layer, access, format);
    }
}

GLvoid GL_APIENTRY __glProfile_GetTexImage(__GLcontext *gc, GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetTexImage 0x%04X %d 0x%04X 0x%04X 0x%08X\n",
                        gc, tid, target, level, format, type, pixels);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetTexImage(gc, target, level, format, type, pixels);
    __GL_PROFILE_FOOTER(GL4_GETTEXIMAGE);

    if (__glTracerDispatchTable.GetTexImage)
    {
        (*__glTracerDispatchTable.GetTexImage)(target, level, format, type, pixels);
    }
}

GLvoid GLAPIENTRY __glProfile_GetBooleani_v(__GLcontext *gc, GLenum target, GLuint index, GLboolean *data)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetBooleani_v 0x%04X %d 0x%08X\n",
                        gc, tid, target, index, data);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetBooleani_v(gc, target, index, data);
    __GL_PROFILE_FOOTER(GL4_GETBOOLEANI_V);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetBooleani_v => %d\n", *data);
    }

    if (__glTracerDispatchTable.GetBooleani_v)
    {
        (*__glTracerDispatchTable.GetBooleani_v)(target, index, data);
    }
}

GLvoid GLAPIENTRY __glProfile_MemoryBarrier(__GLcontext *gc, GLbitfield barriers)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMemoryBarrier 0x%08X\n",
                        gc, tid, barriers);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MemoryBarrier(gc, barriers);
    __GL_PROFILE_FOOTER(GL4_MEMORYBARRIER);

    if (__glTracerDispatchTable.MemoryBarrier)
    {
        (*__glTracerDispatchTable.MemoryBarrier)(barriers);
    }
}

GLvoid GLAPIENTRY __glProfile_MemoryBarrierByRegion(__GLcontext *gc, GLbitfield barriers)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMemoryBarrierByRegion 0x%08X\n",
                        gc, tid, barriers);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MemoryBarrierByRegion(gc, barriers);
    __GL_PROFILE_FOOTER(GL4_MEMORYBARRIERBYREGION);

    if (__glTracerDispatchTable.MemoryBarrierByRegion)
    {
        (*__glTracerDispatchTable.MemoryBarrierByRegion)(barriers);
    }
}

GLvoid GLAPIENTRY __glProfile_TexStorage2DMultisample(__GLcontext *gc, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexStorage2DMultisample 0x%04X %d 0x%04X %d %d %d\n",
                        gc, tid, target, samples, internalformat, width, height, fixedsamplelocations);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexStorage2DMultisample(gc, target, samples, internalformat, width, height, fixedsamplelocations);
    __GL_PROFILE_FOOTER(GL4_TEXSTORAGE2DMULTISAMPLE);

    if (__glTracerDispatchTable.TexStorage2DMultisample)
    {
        (*__glTracerDispatchTable.TexStorage2DMultisample)(target, samples, internalformat, width, height, fixedsamplelocations);
    }
}

GLvoid GLAPIENTRY __glProfile_GetMultisamplefv(__GLcontext *gc, GLenum pname, GLuint index, GLfloat *val)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetMultisamplefv 0x%04X %d 0x%08X\n",
                        gc, tid, pname, index, val);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetMultisamplefv(gc, pname, index, val);
    __GL_PROFILE_FOOTER(GL4_GETMULTISAMPLEFV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetMultisamplefv => %f\n", *val);
    }

    if (__glTracerDispatchTable.GetMultisamplefv)
    {
        (*__glTracerDispatchTable.GetMultisamplefv)(pname, index, val);
    }
}

GLvoid GLAPIENTRY __glProfile_SampleMaski(__GLcontext *gc, GLuint maskNumber, GLbitfield mask)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glSampleMaski %d 0x%08X\n",
                        gc, tid, maskNumber, mask);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->SampleMaski(gc, maskNumber, mask);
    __GL_PROFILE_FOOTER(GL4_SAMPLEMASKI);

    if (__glTracerDispatchTable.SampleMaski)
    {
        (*__glTracerDispatchTable.SampleMaski)(maskNumber, mask);
    }
}

GLvoid GLAPIENTRY __glProfile_GetTexLevelParameteriv(__GLcontext *gc, GLenum target, GLint level, GLenum pname, GLint *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetTexLevelParameteriv 0x%04X %d 0x%04X 0x%08X\n",
                        gc, tid, target, level, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetTexLevelParameteriv(gc, target, level, pname, params);
    __GL_PROFILE_FOOTER(GL4_GETTEXLEVELPARAMETERIV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetTexLevelParameteriv => %d\n", *params);
    }

    if (__glTracerDispatchTable.GetTexLevelParameteriv)
    {
        (*__glTracerDispatchTable.GetTexLevelParameteriv)(target, level, pname, params);
    }
}

GLvoid GLAPIENTRY __glProfile_GetTexLevelParameterfv(__GLcontext *gc, GLenum target, GLint level, GLenum pname, GLfloat *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetTexLevelParameterfv 0x%04X %d 0x%04X 0x%08X\n",
                        gc, tid, target, level, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetTexLevelParameterfv(gc, target, level, pname, params);
    __GL_PROFILE_FOOTER(GL4_GETTEXLEVELPARAMETERFV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetTexLevelParameterfv => %f\n", *params);
    }

    if (__glTracerDispatchTable.GetTexLevelParameterfv)
    {
        (*__glTracerDispatchTable.GetTexLevelParameterfv)(target, level, pname, params);
    }
}

GLvoid GLAPIENTRY __glProfile_BindVertexBuffer(__GLcontext *gc, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glBindVertexBuffer %d %d 0x%08X %d\n",
                        gc, tid, bindingindex, buffer, offset, stride);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->BindVertexBuffer(gc, bindingindex, buffer, offset, stride);
    __GL_PROFILE_FOOTER(GL4_BINDVERTEXBUFFER);

    if (__glTracerDispatchTable.BindVertexBuffer)
    {
        (*__glTracerDispatchTable.BindVertexBuffer)(bindingindex, buffer, offset, stride);
    }
}

GLvoid GLAPIENTRY __glProfile_VertexAttribFormat(__GLcontext *gc, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttribFormat %d %d 0x%04X %d %d\n",
                        gc, tid, attribindex, size, type, normalized, relativeoffset);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttribFormat(gc, attribindex, size, type, normalized, relativeoffset);
    __GL_PROFILE_FOOTER(GL4_VERTEXATTRIBFORMAT);

    if (__glTracerDispatchTable.VertexAttribFormat)
    {
        (*__glTracerDispatchTable.VertexAttribFormat)(attribindex, size, type, normalized, relativeoffset);
    }
}

GLvoid GLAPIENTRY __glProfile_VertexAttribIFormat(__GLcontext *gc, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttribIFormat %d %d 0x%04X %d\n",
                        gc, tid, attribindex, size, type, relativeoffset);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttribIFormat(gc, attribindex, size, type, relativeoffset);
    __GL_PROFILE_FOOTER(GL4_VERTEXATTRIBIFORMAT);

    if (__glTracerDispatchTable.VertexAttribIFormat)
    {
        (*__glTracerDispatchTable.VertexAttribIFormat)(attribindex, size, type, relativeoffset);
    }
}

GLvoid GLAPIENTRY __glProfile_VertexAttribBinding(__GLcontext *gc, GLuint attribindex, GLuint bindingindex)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttribBinding %d %d\n",
                        gc, tid, attribindex, bindingindex);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttribBinding(gc, attribindex, bindingindex);
    __GL_PROFILE_FOOTER(GL4_VERTEXATTRIBBINDING);

    if (__glTracerDispatchTable.VertexAttribBinding)
    {
        (*__glTracerDispatchTable.VertexAttribBinding)(attribindex, bindingindex);
    }
}

GLvoid GLAPIENTRY __glProfile_VertexBindingDivisor(__GLcontext *gc, GLuint bindingindex, GLuint divisor)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexBindingDivisor %d %d\n",
                        gc, tid, bindingindex, divisor);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexBindingDivisor(gc, bindingindex, divisor);
    __GL_PROFILE_FOOTER(GL4_VERTEXBINDINGDIVISOR);

    if (__glTracerDispatchTable.VertexBindingDivisor)
    {
        (*__glTracerDispatchTable.VertexBindingDivisor)(bindingindex, divisor);
    }
}

GLvoid GLAPIENTRY __glProfile_TexStorage3DMultisample(__GLcontext *gc, GLenum target, GLsizei samples, GLenum sizedinternalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexStorage3DMultisample 0x%04X %d 0x%04X %d %d %d %d\n",
                        gc, tid, target, samples, sizedinternalformat, width, height, depth, fixedsamplelocations);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexStorage3DMultisample(gc, target, samples, sizedinternalformat, width, height, depth, fixedsamplelocations);
    __GL_PROFILE_FOOTER(GL4_TEXSTORAGE3DMULTISAMPLE);

    if (__glTracerDispatchTable.TexStorage3DMultisample)
    {
        (*__glTracerDispatchTable.TexStorage3DMultisample)(target, samples, sizedinternalformat, width, height, depth, fixedsamplelocations);
    }
}

GLvoid GLAPIENTRY __glProfile_BlendBarrier(__GLcontext *gc)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glBlendBarrier\n",
                        gc, tid);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->BlendBarrier(gc);
    __GL_PROFILE_FOOTER(GL4_BLENDBARRIER);

    if (__glTracerDispatchTable.BlendBarrier)
    {
        (*__glTracerDispatchTable.BlendBarrier)();
    }
}

GLvoid GLAPIENTRY __glProfile_DebugMessageControl(__GLcontext *gc, GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDebugMessageControl 0x%04X 0x%04X 0x%04X %d 0x%08X %d\n",
                        gc, tid, source, type, severity, count, ids, enabled);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DebugMessageControl(gc, source, type, severity, count, ids, enabled);
    __GL_PROFILE_FOOTER(GL4_DEBUGMESSAGECONTROL);

    if (__glTracerDispatchTable.DebugMessageControl)
    {
        (*__glTracerDispatchTable.DebugMessageControl)(source, type, severity, count, ids, enabled);
    }
}

GLvoid GLAPIENTRY __glProfile_DebugMessageInsert(__GLcontext *gc, GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* buf)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDebugMessageInsert 0x%04X 0x%04X %u 0x%04X %d 0x%08X\n",
                        gc, tid, source, type, id, severity, length, buf);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DebugMessageInsert(gc, source, type, id, severity, length, buf);
    __GL_PROFILE_FOOTER(GL4_DEBUGMESSAGEINSERT);

    if (__glTracerDispatchTable.DebugMessageInsert)
    {
        (*__glTracerDispatchTable.DebugMessageInsert)(source, type, id, severity, length, buf);
    }
}

GLvoid GLAPIENTRY __glProfile_DebugMessageCallback(__GLcontext *gc, GLDEBUGPROCKHR callback, const GLvoid* userParam)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDebugMessageCallback 0x%08X 0x%08X\n",
                        gc, tid, callback, userParam);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DebugMessageCallback(gc, callback, userParam);
    __GL_PROFILE_FOOTER(GL4_DEBUGMESSAGECALLBACK);

    if (__glTracerDispatchTable.DebugMessageCallback)
    {
        (*__glTracerDispatchTable.DebugMessageCallback)(callback, userParam);
    }
}

GLuint GLAPIENTRY __glProfile_GetDebugMessageLog(__GLcontext *gc, GLuint count, GLsizei bufSize, GLenum* sources, GLenum* types, GLuint* ids, GLenum* severities, GLsizei* lengths, GLchar* messageLog)
{
    __GL_PROFILE_VARS();
    GLuint number;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetDebugMessageLog %u %d 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X\n",
                        gc, tid, count, bufSize, sources, types, ids, severities, lengths, messageLog);
    }

    __GL_PROFILE_HEADER();
    number = gc->pModeDispatch->GetDebugMessageLog(gc, count, bufSize, sources, types, ids, severities, lengths, messageLog);
    __GL_PROFILE_FOOTER(GL4_GETDEBUGMESSAGELOG);

    if (__glTracerDispatchTable.GetDebugMessageLog)
    {
        (*__glTracerDispatchTable.GetDebugMessageLog)(count, bufSize, sources, types, ids, severities, lengths, messageLog);
    }
    return number;
}

GLvoid GLAPIENTRY __glProfile_GetPointerv(__GLcontext *gc, GLenum pname, GLvoid** params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetPointerv 0x%04X 0x%08X\n",
                        gc, tid, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetPointerv(gc, pname, params);
    __GL_PROFILE_FOOTER(GL4_GETPOINTERV);

    if (__glTracerDispatchTable.GetPointerv)
    {
        (*__glTracerDispatchTable.GetPointerv)(pname, params);
    }
}

GLvoid GLAPIENTRY __glProfile_PushDebugGroup(__GLcontext *gc, GLenum source, GLuint id, GLsizei length, const GLchar * message)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glPushDebugGroup 0x%04X %u %d 0x%08X\n",
                        gc, tid, source, id, length, message);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->PushDebugGroup(gc, source, id, length, message);
    __GL_PROFILE_FOOTER(GL4_PUSHDEBUGGROUP);

    if (__glTracerDispatchTable.PushDebugGroup)
    {
        (*__glTracerDispatchTable.PushDebugGroup)(source, id, length, message);
    }
}

GLvoid GLAPIENTRY __glProfile_PopDebugGroup(__GLcontext *gc)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glPopDebugGroup()\n",
                        gc, tid);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->PopDebugGroup(gc);
    __GL_PROFILE_FOOTER(GL4_POPDEBUGGROUP);

    if (__glTracerDispatchTable.PopDebugGroup)
    {
        (*__glTracerDispatchTable.PopDebugGroup)();
    }
}

GLvoid GLAPIENTRY __glProfile_ObjectLabel(__GLcontext *gc, GLenum identifier, GLuint name, GLsizei length, const GLchar *label)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glObjectLabel 0x%04X %u %d 0x%08X\n",
                        gc, tid, identifier, name, length, label);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ObjectLabel(gc, identifier, name, length, label);
    __GL_PROFILE_FOOTER(GL4_OBJECTLABEL);

    if (__glTracerDispatchTable.ObjectLabel)
    {
        (*__glTracerDispatchTable.ObjectLabel)(identifier, name, length, label);
    }
}

GLvoid GLAPIENTRY __glProfile_GetObjectLabel(__GLcontext *gc, GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetObjectLabel 0x%04X %u %d 0x%08X 0x%08X\n",
                        gc, tid, identifier, name, bufSize, length, label);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetObjectLabel(gc, identifier, name, bufSize, length, label);
    __GL_PROFILE_FOOTER(GL4_GETOBJECTLABEL);

    if (__glTracerDispatchTable.GetObjectLabel)
    {
        (*__glTracerDispatchTable.GetObjectLabel)(identifier, name, bufSize, length, label);
    }
}

GLvoid GLAPIENTRY __glProfile_ObjectPtrLabel(__GLcontext *gc, const GLvoid* ptr, GLsizei length, const GLchar *label)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glObjectPtrLabel 0x%08X %d 0x%08X\n",
                        gc, tid, ptr, length, label);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ObjectPtrLabel(gc, ptr, length, label);
    __GL_PROFILE_FOOTER(GL4_OBJECTPTRLABEL);

    if (__glTracerDispatchTable.ObjectPtrLabel)
    {
        (*__glTracerDispatchTable.ObjectPtrLabel)(ptr, length, label);
    }
}

GLvoid GLAPIENTRY __glProfile_GetObjectPtrLabel(__GLcontext *gc, const GLvoid* ptr, GLsizei bufSize, GLsizei *length, GLchar *label)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetObjectPtrLabel 0x%08X %d 0x%08X 0x%08X\n",
                        gc, tid, ptr, bufSize, length, label);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetObjectPtrLabel(gc, ptr, bufSize, length, label);
    __GL_PROFILE_FOOTER(GL4_GETOBJECTPTRLABEL);

    if (__glTracerDispatchTable.GetObjectPtrLabel)
    {
        (*__glTracerDispatchTable.GetObjectPtrLabel)(ptr, bufSize, length, label);
    }
}

GLenum GLAPIENTRY __glProfile_GetGraphicsResetStatus(__GLcontext *gc)
{
    __GL_PROFILE_VARS();
    GLenum retStatus;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetGraphicsResetStatusEXT\n",
                        gc, tid);
    }

    __GL_PROFILE_HEADER();
    retStatus = gc->pModeDispatch->GetGraphicsResetStatus(gc);
    __GL_PROFILE_FOOTER(GL4_GETGRAPHICSRESETSTATUS);

    if (__glTracerDispatchTable.GetGraphicsResetStatus)
    {
        (*__glTracerDispatchTable.GetGraphicsResetStatus)();
    }

    return retStatus;
}

GLvoid GLAPIENTRY __glProfile_ReadnPixels(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, GLvoid *data)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glReadnPixels %d %d %d %d 0x%04X 0x%04X %d 0x%08X\n",
                        gc, tid, x, y, width, height, format, type, bufSize, __GL_PTR2UINT(data));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ReadnPixels(gc, x, y, width, height, format, type, bufSize, data);
    __GL_PROFILE_FOOTER(GL4_READNPIXELS);

    if (__glTracerDispatchTable.ReadnPixels)
    {
        (*__glTracerDispatchTable.ReadnPixels)(x, y, width, height, format, type, bufSize, data);
    }
}

GLvoid GLAPIENTRY __glProfile_GetnUniformfv(__GLcontext *gc, GLuint program, GLint location, GLsizei bufSize, GLfloat *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetnUniformfv %d %d %d\n",
                        gc, tid, program, location, bufSize);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetnUniformfv(gc, program, location, bufSize, params);
    __GL_PROFILE_FOOTER(GL4_GETNUNIFORMFV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetnUniformfv => %f\n", __GL_PTRVALUE(params));
    }

    if (__glTracerDispatchTable.GetnUniformfv)
    {
        (*__glTracerDispatchTable.GetnUniformfv)(program, location, bufSize, params);
    }
}

GLvoid GLAPIENTRY __glProfile_GetnUniformiv(__GLcontext *gc, GLuint program, GLint location, GLsizei bufSize, GLint *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetnUniformiv %d %d %d\n",
                        gc, tid, program, location, bufSize);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetnUniformiv(gc, program, location, bufSize, params);
    __GL_PROFILE_FOOTER(GL4_GETNUNIFORMIV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetnUniformiv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glTracerDispatchTable.GetnUniformiv)
    {
        (*__glTracerDispatchTable.GetnUniformiv)(program, location, bufSize, params);
    }
}

GLvoid GLAPIENTRY __glProfile_GetnUniformuiv(__GLcontext *gc, GLuint program, GLint location, GLsizei bufSize, GLuint *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetnUniformuiv %d %d %d\n",
                        gc, tid, program, location, bufSize);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetnUniformuiv(gc, program, location, bufSize, params);
    __GL_PROFILE_FOOTER(GL4_GETNUNIFORMUIV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetnUniformuiv => %d\n", __GL_PTRVALUE(params));
    }

    if (__glTracerDispatchTable.GetnUniformuiv)
    {
        (*__glTracerDispatchTable.GetnUniformuiv)(program, location, bufSize, params);
    }
}

GLvoid GLAPIENTRY __glProfile_BlendEquationi(__GLcontext * gc, GLuint buf, GLenum mode)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glBlendEquationi %d 0x%04X\n",
                        gc, tid, buf, mode);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->BlendEquationi(gc, buf, mode);
    __GL_PROFILE_FOOTER(GL4_BLENDEQUATIONI);

    if (__glTracerDispatchTable.BlendEquationi)
    {
        (*__glTracerDispatchTable.BlendEquationi)(buf, mode);
    }

}


GLvoid GLAPIENTRY __glProfile_BlendEquationSeparatei(__GLcontext * gc, GLuint buf, GLenum modeRGB, GLenum modeAlpha)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glBlendEquationSeparatei %d 0x%04X 0x%04X\n",
                        gc, tid, buf, modeRGB, modeAlpha);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->BlendEquationSeparatei(gc, buf, modeRGB, modeAlpha);
    __GL_PROFILE_FOOTER(GL4_BLENDEQUATIONSEPARATEI);

    if (__glTracerDispatchTable.BlendEquationSeparatei)
    {
        (*__glTracerDispatchTable.BlendEquationSeparatei)(buf, modeRGB, modeAlpha);
    }
}


GLvoid GLAPIENTRY __glProfile_BlendFunci(__GLcontext * gc, GLuint buf, GLenum sfactor, GLenum dfactor)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glBlendFunci %d 0x%04X 0x%04X\n",
                        gc, tid, buf, sfactor, dfactor);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->BlendFunci(gc, buf, sfactor, dfactor);
    __GL_PROFILE_FOOTER(GL4_BLENDFUNCI);

    if (__glTracerDispatchTable.BlendFunci)
    {
        (*__glTracerDispatchTable.BlendFunci)(buf, sfactor, dfactor);
    }

}

GLvoid GLAPIENTRY __glProfile_BlendFuncSeparatei(__GLcontext * gc, GLuint buf, GLenum sfactorRGB,GLenum dfactorRGB,GLenum sfactorAlpha,GLenum dfactorAlpha)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glBlendFuncSeparatei %d 0x%04X 0x%04X 0x%04X 0x%04X\n",
                        gc, tid, buf, sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->BlendFuncSeparatei(gc, buf, sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
    __GL_PROFILE_FOOTER(GL4_BLENDFUNCSEPARATEI);

    if (__glTracerDispatchTable.BlendFuncSeparatei)
    {
        (*__glTracerDispatchTable.BlendFuncSeparatei)(buf, sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
    }
}


GLvoid GLAPIENTRY __glProfile_ColorMaski(__GLcontext * gc,GLuint buf, GLboolean r, GLboolean g, GLboolean b, GLboolean a)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColorMaski %d %d %d %d %d\n",
                        gc, tid, buf, r, g, b, a);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ColorMaski(gc, buf, r, g, b, a);
    __GL_PROFILE_FOOTER(GL4_COLORMASKI);

    if (__glTracerDispatchTable.ColorMaski)
    {
        (*__glTracerDispatchTable.ColorMaski)(buf, r, g, b, a);
    }
}

GLvoid GLAPIENTRY __glProfile_Enablei(__GLcontext *gc, GLenum target, GLuint index)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glEnablei 0x%04X %d \n",
                        gc, tid, target, index);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Enablei(gc, target, index);
    __GL_PROFILE_FOOTER(GL4_ENABLEI);

    if (__glTracerDispatchTable.Enablei)
    {
        (*__glTracerDispatchTable.Enablei)(target, index);
    }

}

GLvoid GLAPIENTRY __glProfile_Disablei(__GLcontext *gc, GLenum target, GLuint index)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDisablei 0x%04X %d \n",
                        gc, tid, target, index);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Disablei(gc, target, index);
    __GL_PROFILE_FOOTER(GL4_DISABLEI);

    if (__glTracerDispatchTable.Disablei)
    {
        (*__glTracerDispatchTable.Disablei)(target, index);
    }
}

GLboolean GLAPIENTRY __glProfile_IsEnabledi(__GLcontext * gc, GLenum target, GLuint index)
{
    __GL_PROFILE_VARS();
    GLboolean is;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glIsEnablediEXT 0x%04X %d\n",
                        gc, tid, target, index);
    }

    __GL_PROFILE_HEADER();
    is = gc->pModeDispatch->IsEnabledi(gc, target, index);
    __GL_PROFILE_FOOTER(GL4_ISENABLEDI);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glIsEnabledi => %d\n", is);
    }

    if (__glTracerDispatchTable.IsEnabledi)
    {
        (*__glTracerDispatchTable.IsEnabledi)(target, index);
    }

    return is;

}

GLvoid GLAPIENTRY __glProfile_TexParameterIiv(__GLcontext *gc, GLenum target, GLenum pname, const GLint *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexParameterIiv 0x%04X 0x%04X %d\n",
                        gc, tid, target, pname, __GL_PTRVALUE(params));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexParameterIiv(gc, target, pname, params);
    __GL_PROFILE_FOOTER(GL4_TEXPARAMETERIIV);

    if (__glTracerDispatchTable.TexParameterIiv)
    {
        (*__glTracerDispatchTable.TexParameterIiv)(target, pname, params);
    }
}
GLvoid GLAPIENTRY __glProfile_TexParameterIuiv(__GLcontext *gc, GLenum target, GLenum pname, const GLuint *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexParameterIuiv 0x%04X 0x%04X %d\n",
                        gc, tid, target, pname, __GL_PTRVALUE(params));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexParameterIuiv(gc, target, pname, params);
    __GL_PROFILE_FOOTER(GL4_TEXPARAMETERIUIV);

    if (__glTracerDispatchTable.TexParameterIuiv)
    {
        (*__glTracerDispatchTable.TexParameterIuiv)(target, pname, params);
    }
}
GLvoid GLAPIENTRY __glProfile_GetTexParameterIiv(__GLcontext *gc, GLenum target, GLenum pname, GLint *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetTexParameterIiv 0x%04X 0x%04X 0x%08X\n",
                        gc, tid, target, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetTexParameterIiv(gc, target, pname, params);
    __GL_PROFILE_FOOTER(GL4_GETTEXPARAMETERIIV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetTexParameterIiv => %d\n", *params);
    }

    if (__glTracerDispatchTable.GetTexParameterIiv)
    {
        (*__glTracerDispatchTable.GetTexParameterIiv)(target, pname, params);
    }
}
GLvoid GLAPIENTRY __glProfile_GetTexParameterIuiv(__GLcontext *gc, GLenum target, GLenum pname, GLuint *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetTexParameterIuiv 0x%04X 0x%04X 0x%08X\n",
                        gc, tid, target, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetTexParameterIuiv(gc, target, pname, params);
    __GL_PROFILE_FOOTER(GL4_GETTEXPARAMETERIUIV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetTexParameterIuiv => %d\n", *params);
    }

    if (__glTracerDispatchTable.GetTexParameterIuiv)
    {
        (*__glTracerDispatchTable.GetTexParameterIuiv)(target, pname, params);
    }
}
GLvoid GLAPIENTRY __glProfile_SamplerParameterIiv(__GLcontext *gc, GLuint sampler, GLenum pname, const GLint *param)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glSamplerParameterIiv 0x%04X 0x%04X %d\n",
                        gc, tid, sampler, pname, __GL_PTRVALUE(param));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->SamplerParameterIiv(gc, sampler, pname, param);
    __GL_PROFILE_FOOTER(GL4_SAMPLERPARAMETERIIV);

    if (__glTracerDispatchTable.SamplerParameterIiv)
    {
        (*__glTracerDispatchTable.SamplerParameterIiv)(sampler, pname, param);
    }

}
GLvoid GLAPIENTRY __glProfile_SamplerParameterIuiv(__GLcontext *gc, GLuint sampler, GLenum pname, const GLuint *param)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glSamplerParameterIuiv 0x%04X 0x%04X %d\n",
                        gc, tid, sampler, pname, __GL_PTRVALUE(param));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->SamplerParameterIuiv(gc, sampler, pname, param);
    __GL_PROFILE_FOOTER(GL4_SAMPLERPARAMETERIUIV);

    if (__glTracerDispatchTable.SamplerParameterIuiv)
    {
        (*__glTracerDispatchTable.SamplerParameterIuiv)(sampler, pname, param);
    }

}
GLvoid GLAPIENTRY __glProfile_GetSamplerParameterIiv(__GLcontext *gc, GLuint sampler, GLenum pname, GLint *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetSamplerParameterIiv 0x%04X 0x%04X 0x%08X\n",
                        gc, tid, sampler, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetSamplerParameterIiv(gc, sampler, pname, params);
    __GL_PROFILE_FOOTER(GL4_GETSAMPLERPARAMETERIIV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetSamplerParameterIiv => %d\n", *params);
    }

    if (__glTracerDispatchTable.GetSamplerParameterIiv)
    {
        (*__glTracerDispatchTable.GetSamplerParameterIiv)(sampler, pname, params);
    }
}
GLvoid GLAPIENTRY __glProfile_GetSamplerParameterIuiv (__GLcontext *gc, GLuint sampler, GLenum pname, GLuint *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetSamplerParameterIuiv 0x%04X 0x%04X 0x%08X\n",
                        gc, tid, sampler, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetSamplerParameterIuiv(gc, sampler, pname, params);
    __GL_PROFILE_FOOTER(GL4_GETSAMPLERPARAMETERIUIV);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetSamplerParameterIuiv => %d\n", *params);
    }

    if (__glTracerDispatchTable.GetSamplerParameterIuiv)
    {
        (*__glTracerDispatchTable.GetSamplerParameterIuiv)(sampler, pname, params);
    }
}

GLvoid GLAPIENTRY __glProfile_TexBuffer(__GLcontext *gc, GLenum target, GLenum internalformat, GLuint buffer)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexBuffer 0x%04X 0x%04X %d\n",
                        gc, tid, target, internalformat, buffer);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexBuffer(gc, target, internalformat, buffer);
    __GL_PROFILE_FOOTER(GL4_TEXBUFFER);

    if (__glTracerDispatchTable.TexBuffer)
    {
        (*__glTracerDispatchTable.TexBuffer)(target, internalformat, buffer);
    }

}
GLvoid GLAPIENTRY __glProfile_TexBufferRange(__GLcontext *gc, GLenum target, GLenum internalformat,
                                                   GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexBufferRange 0x%04X 0x%04X %d 0x%08X 0x%08X\n",
                        gc, tid, target, internalformat, buffer, __GL_PTR2UINT(offset), __GL_PTR2UINT(size));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexBufferRange(gc, target, internalformat, buffer, offset, size);
    __GL_PROFILE_FOOTER(GL4_TEXBUFFERRANGE);

    if (__glTracerDispatchTable.TexBufferRange)
    {
        (*__glTracerDispatchTable.TexBufferRange)(target, internalformat, buffer, offset, size);
    }

}

GLvoid GLAPIENTRY __glProfile_PatchParameteri(__GLcontext *gc, GLenum pname, GLint value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glPatchParameteri 0x%04X %d\n",
                        gc, tid, pname, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->PatchParameteri(gc, pname, value);
    __GL_PROFILE_FOOTER(GL4_PATCHPARAMETERI);

    if (__glTracerDispatchTable.PatchParameteri)
    {
        (*__glTracerDispatchTable.PatchParameteri)(pname, value);
    }

}

GLvoid GLAPIENTRY __glProfile_FramebufferTexture(__GLcontext *gc, GLenum target, GLenum attachment, GLuint texture, GLint level)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glFramebufferTexture 0x%04X 0x%04X %d %d\n",
                        gc, tid, target, attachment, texture, level);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->FramebufferTexture(gc, target, attachment, texture, level);
    __GL_PROFILE_FOOTER(GL4_FRAMEBUFFERTEXTURE);

    if (__glTracerDispatchTable.FramebufferTexture)
    {
        (*__glTracerDispatchTable.FramebufferTexture)(target, attachment, texture, level);
    }

}

GLvoid GLAPIENTRY __glProfile_MinSampleShading(__GLcontext *gc, GLfloat value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMinSampleShading %f\n",
                        gc, tid, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MinSampleShading(gc, value);
    __GL_PROFILE_FOOTER(GL4_MINSAMPLESHADING);

    if (__glTracerDispatchTable.MinSampleShading)
    {
        (*__glTracerDispatchTable.MinSampleShading)(value);
    }
}

GLvoid GLAPIENTRY __glProfile_CopyImageSubData(__GLcontext *gc,
                                                  GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ,
                                                  GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ,
                                                  GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glCopyImageSubData %d 0x%04X %d %d %d %d "
                        "%d 0x%04X %d %d %d %d %d %d %d",
                        gc, tid, srcName, srcTarget, srcLevel, srcX, srcY, srcZ,
                        dstName, dstTarget, dstLevel, dstX, dstY, dstZ,
                        srcWidth, srcHeight, srcDepth);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->CopyImageSubData(gc,
                            srcName, srcTarget, srcLevel, srcX, srcY, srcZ,
                            dstName, dstTarget, dstLevel, dstX, dstY, dstZ,
                            srcWidth, srcHeight, srcDepth);
    __GL_PROFILE_FOOTER(GL4_COPYIMAGESUBDATA);

    if (__glTracerDispatchTable.CopyImageSubData)
    {
        (*__glTracerDispatchTable.CopyImageSubData)(srcName, srcTarget, srcLevel, srcX, srcY, srcZ,
                                                         dstName, dstTarget, dstLevel, dstX, dstY, dstZ,
                                                         srcWidth, srcHeight, srcDepth);
    }
}


GLvoid GLAPIENTRY __glProfile_DrawElementsBaseVertex(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDrawElementsBaseVertex 0x%04X %d 0x%04X 0x%08X %d\n",
                        gc, tid, mode, count, type, indices, basevertex);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DrawElementsBaseVertex(gc, mode, count, type, indices, basevertex);
    __GL_PROFILE_FOOTER(GL4_DRAWELEMENTSBASEVERTEX);

    if (__glTracerDispatchTable.DrawElementsBaseVertex)
    {
        (*__glTracerDispatchTable.DrawElementsBaseVertex)(mode, count, type, indices, basevertex);
    }
}

GLvoid GLAPIENTRY __glProfile_DrawRangeElementsBaseVertex(__GLcontext *gc, GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDrawRangeElementsBaseVertex 0x%04X %d %d %d 0x%04X 0x%08X %d\n",
                        gc, tid, mode, start, end, count, type, indices, basevertex);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DrawRangeElementsBaseVertex(gc, mode, start, end, count, type, indices, basevertex);
    __GL_PROFILE_FOOTER(GL4_DRAWRANGEELEMENTSBASEVERTEX);

    if (__glTracerDispatchTable.DrawRangeElementsBaseVertex)
    {
        (*__glTracerDispatchTable.DrawRangeElementsBaseVertex)(mode, start, end, count, type, indices, basevertex);
    }
}

GLvoid GLAPIENTRY __glProfile_DrawElementsInstancedBaseVertex(__GLcontext *gc, GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDrawElementsInstancedBaseVertex 0x%04X %d 0x%04X 0x%08X %d %d\n",
                        gc, tid, mode, count, type, indices, instancecount, basevertex);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DrawElementsInstancedBaseVertex(gc, mode, count, type, indices, instancecount, basevertex);
    __GL_PROFILE_FOOTER(GL4_DRAWELEMENTSINSTANCEDBASEVERTEX);

    if (__glTracerDispatchTable.DrawElementsInstancedBaseVertex)
    {
        (*__glTracerDispatchTable.DrawElementsInstancedBaseVertex)(mode, count, type, indices, instancecount, basevertex);
    }
}

GLvoid GLAPIENTRY __glProfile_PrimitiveBoundingBox(__GLcontext *gc, GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW,
                                                      GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glPrimitiveBoundingBox %f %f %f %f %f %f %f %f\n",
                        gc, tid, minX, minY, minZ, minW, maxX, maxY, maxZ, maxW);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->PrimitiveBoundingBox(gc, minX, minY, minZ, minW, maxX, maxY, maxZ, maxW);
    __GL_PROFILE_FOOTER(GL4_DRAWELEMENTSINSTANCEDBASEVERTEX);

    if (__glTracerDispatchTable.PrimitiveBoundingBox)
    {
        (*__glTracerDispatchTable.PrimitiveBoundingBox)(minX, minY, minZ, minW, maxX, maxY, maxZ, maxW);
    }
}

/*
** OpenGL defines as core but ES as extension APIs
*/

GLvoid* GL_APIENTRY __glProfile_MapBuffer(__GLcontext *gc, GLenum target, GLenum access)
{
    __GL_PROFILE_VARS();
    GLvoid *buf;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMapBuffer 0x%04X 0x%04X\n",
            gc, tid, target, access);
    }

    __GL_PROFILE_HEADER();
    buf = gc->pModeDispatch->MapBuffer(gc, target, access);
    __GL_PROFILE_FOOTER(GL4_MAPBUFFERRANGE);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glMapBuffer => 0x%08X\n", __GL_PTR2UINT(buf));
    }

    if (__glTracerDispatchTable.MapBuffer)
    {
        (*__glTracerDispatchTable.MapBuffer)(target, access, buf);
    }

    return buf;
}

GLvoid GLAPIENTRY __glProfile_MultiDrawArrays(__GLcontext *gc, GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiDrawArrays 0x%04X 0x%08X 0x%08X %d\n",
                        gc, tid, mode, __GL_PTR2UINT(first), __GL_PTR2UINT(count), primcount);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiDrawArrays(gc, mode, first, count, primcount);
    __GL_PROFILE_FOOTER(GL4_MULTIDRAWARRAYS);

    if (__glTracerDispatchTable.MultiDrawArrays)
    {
        (*__glTracerDispatchTable.MultiDrawArrays)(mode, first, count, primcount);
    }
}

GLvoid GLAPIENTRY __glProfile_MultiDrawElements(__GLcontext *gc, GLenum mode, const GLsizei *count, GLenum type, const GLvoid*const*indices, GLsizei primcount)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiDrawElements 0x%04X 0x%08X 0x%04X 0x%08X %d\n",
                        gc, tid, mode, __GL_PTR2UINT(count), type, __GL_PTR2UINT(indices), primcount);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiDrawElements(gc, mode, count, type, indices, primcount);
    __GL_PROFILE_FOOTER(GL4_MULTIDRAWELEMENTS);

    if (__glTracerDispatchTable.MultiDrawElements)
    {
        (*__glTracerDispatchTable.MultiDrawElements)(mode, count, type, indices, primcount);
    }
}

GLvoid GLAPIENTRY __glProfile_MultiDrawElementsBaseVertex(__GLcontext *gc, GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei drawcount, const GLint * basevertex)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiDrawElementsBaseVertex 0x%04X 0x%08X 0x%04X 0x%08X %d 0x%08X\n",
                        gc, tid, mode, count, type, indices, drawcount, basevertex);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiDrawElementsBaseVertex(gc, mode, count, type, indices, drawcount, basevertex);
    __GL_PROFILE_FOOTER(GL4_MULTIDRAWELEMENTSBASEVERTEX);

    if (__glTracerDispatchTable.MultiDrawElementsBaseVertex)
    {
        (*__glTracerDispatchTable.MultiDrawElementsBaseVertex)(mode, count, type, indices, drawcount, basevertex);
    }
}

GLvoid GLAPIENTRY __glProfile_MultiDrawArraysIndirect(__GLcontext *gc, GLenum mode, const void *indirect, GLsizei drawcount, GLsizei stride)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiDrawArraysIndirect 0x%04X 0x%08X %d %d\n",
                        gc, tid, mode, indirect, drawcount, stride);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiDrawArraysIndirect(gc, mode, indirect, drawcount, stride);
    __GL_PROFILE_FOOTER(GL4_MULTIDRAWARRAYSINDIRECT);

    if (__glTracerDispatchTable.MultiDrawArraysIndirect)
    {
        (*__glTracerDispatchTable.MultiDrawArraysIndirect)(mode, indirect, drawcount, stride);
    }
}

GLvoid GLAPIENTRY __glProfile_MultiDrawElementsIndirect(__GLcontext *gc, GLenum mode, GLenum type, const void *indirect, GLsizei drawcount, GLsizei stride)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiDrawElementsIndirect 0x%04X 0x%04X 0x%08X %d %d\n",
            gc, tid, mode, type, indirect, drawcount, stride);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiDrawElementsIndirect(gc, mode, type, indirect, drawcount, stride);
    __GL_PROFILE_FOOTER(GL4_MULTIDRAWELEMENTSINDIRECT);

    if (__glTracerDispatchTable.MultiDrawElementsIndirect)
    {
        (*__glTracerDispatchTable.MultiDrawElementsIndirect)(mode, type, indirect, drawcount, stride);
    }
}


#ifdef OPENGL40
/* GL_VERSION_1_0 */

GLvoid GL_APIENTRY __glProfile_NewList(__GLcontext *gc, GLuint list, GLenum mode)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glNewList(list=%u, mode=0x%04X)\n",
                     gc, tid, list, mode);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->NewList(gc, list, mode);
    __GL_PROFILE_FOOTER(enum_glNewList);

    if (__glTracerDispatchTable.NewList)
    {
        (*__glTracerDispatchTable.NewList)(list, mode);
    }
}

GLvoid GL_APIENTRY __glProfile_EndList(__GLcontext *gc)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glEndList()\n",
                     gc, tid);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->EndList(gc);
    __GL_PROFILE_FOOTER(enum_glEndList);

    if (__glTracerDispatchTable.EndList)
    {
        (*__glTracerDispatchTable.EndList)();
    }
}

GLvoid GL_APIENTRY __glProfile_CallList(__GLcontext *gc, GLuint list)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glCallList(list=%u)\n",
                     gc, tid, list);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->CallList(gc, list);
    __GL_PROFILE_FOOTER(enum_glCallList);

    if (__glTracerDispatchTable.CallList)
    {
        (*__glTracerDispatchTable.CallList)(list);
    }
}

GLvoid GL_APIENTRY __glProfile_CallLists(__GLcontext *gc, GLsizei n, GLenum type, const GLvoid *lists)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glCallLists(n=%d, type=0x%04X, lists=0x%08X)\n",
                     gc, tid, n, type, lists);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->CallLists(gc, n, type, lists);
    __GL_PROFILE_FOOTER(enum_glCallLists);

    if (__glTracerDispatchTable.CallLists)
    {
        (*__glTracerDispatchTable.CallLists)(n, type, lists);
    }
}

GLvoid GL_APIENTRY __glProfile_DeleteLists(__GLcontext *gc, GLuint list, GLsizei range)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDeleteLists(list=%u, range=%d)\n",
                     gc, tid, list, range);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DeleteLists(gc, list, range);
    __GL_PROFILE_FOOTER(enum_glDeleteLists);

    if (__glTracerDispatchTable.DeleteLists)
    {
        (*__glTracerDispatchTable.DeleteLists)(list, range);
    }
}

GLuint GL_APIENTRY __glProfile_GenLists(__GLcontext *gc, GLsizei range)
{
    __GL_PROFILE_VARS();
    GLuint ret;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGenLists(range=%d)\n",
                     gc, tid, range);
    }

    __GL_PROFILE_HEADER();
    ret = gc->pModeDispatch->GenLists(gc, range);
    __GL_PROFILE_FOOTER(enum_glGenLists);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGenLists => %u\n", ret);
    }

    if (__glTracerDispatchTable.GenLists)
    {
        (*__glTracerDispatchTable.GenLists)(range);
    }

    return ret;
}

GLvoid GL_APIENTRY __glProfile_ListBase(__GLcontext *gc, GLuint base)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glListBase(base=%u)\n",
                     gc, tid, base);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ListBase(gc, base);
    __GL_PROFILE_FOOTER(enum_glListBase);

    if (__glTracerDispatchTable.ListBase)
    {
        (*__glTracerDispatchTable.ListBase)(base);
    }
}

GLvoid GL_APIENTRY __glProfile_Begin(__GLcontext *gc, GLenum mode)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glBegin(mode=0x%04X)\n",
                     gc, tid, mode);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Begin(gc, mode);
    __GL_PROFILE_FOOTER(enum_glBegin);

    if (__glTracerDispatchTable.Begin)
    {
        (*__glTracerDispatchTable.Begin)(mode);
    }
}

GLvoid GL_APIENTRY __glProfile_Bitmap(__GLcontext *gc, GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glBitmap(width=%d, height=%d, xorig=%f, yorig=%f, xmove=%f, ymove=%f, bitmap=0x%p)\n",
                     gc, tid, width, height, xorig, yorig, xmove, ymove, bitmap);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Bitmap(gc, width, height, xorig, yorig, xmove, ymove, bitmap);
    __GL_PROFILE_FOOTER(enum_glBitmap);

    if (__glTracerDispatchTable.Bitmap)
    {
        (*__glTracerDispatchTable.Bitmap)(width, height, xorig, yorig, xmove, ymove, bitmap);
    }
}

GLvoid GL_APIENTRY __glProfile_Color3b(__GLcontext *gc, GLbyte red, GLbyte green, GLbyte blue)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColor3b(red=%hhd, green=%hhd, blue=%hhd)\n",
                     gc, tid, red, green, blue);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Color3b(gc, red, green, blue);
    __GL_PROFILE_FOOTER(enum_glColor3b);

    if (__glTracerDispatchTable.Color3b)
    {
        (*__glTracerDispatchTable.Color3b)(red, green, blue);
    }
}

GLvoid GL_APIENTRY __glProfile_Color3bv(__GLcontext *gc, const GLbyte *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColor3bv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Color3bv(gc, v);
    __GL_PROFILE_FOOTER(enum_glColor3bv);

    if (__glTracerDispatchTable.Color3bv)
    {
        (*__glTracerDispatchTable.Color3bv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_Color3d(__GLcontext *gc, GLdouble red, GLdouble green, GLdouble blue)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColor3d(red=%lf, green=%lf, blue=%lf)\n",
                     gc, tid, red, green, blue);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Color3d(gc, red, green, blue);
    __GL_PROFILE_FOOTER(enum_glColor3d);

    if (__glTracerDispatchTable.Color3d)
    {
        (*__glTracerDispatchTable.Color3d)(red, green, blue);
    }
}

GLvoid GL_APIENTRY __glProfile_Color3dv(__GLcontext *gc, const GLdouble *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColor3dv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Color3dv(gc, v);
    __GL_PROFILE_FOOTER(enum_glColor3dv);

    if (__glTracerDispatchTable.Color3dv)
    {
        (*__glTracerDispatchTable.Color3dv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_Color3f(__GLcontext *gc, GLfloat red, GLfloat green, GLfloat blue)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColor3f(red=%f, green=%f, blue=%f)\n",
                     gc, tid, red, green, blue);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Color3f(gc, red, green, blue);
    __GL_PROFILE_FOOTER(enum_glColor3f);

    if (__glTracerDispatchTable.Color3f)
    {
        (*__glTracerDispatchTable.Color3f)(red, green, blue);
    }
}

GLvoid GL_APIENTRY __glProfile_Color3fv(__GLcontext *gc, const GLfloat *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColor3fv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Color3fv(gc, v);
    __GL_PROFILE_FOOTER(enum_glColor3fv);

    if (__glTracerDispatchTable.Color3fv)
    {
        (*__glTracerDispatchTable.Color3fv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_Color3i(__GLcontext *gc, GLint red, GLint green, GLint blue)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColor3i(red=%d, green=%d, blue=%d)\n",
                     gc, tid, red, green, blue);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Color3i(gc, red, green, blue);
    __GL_PROFILE_FOOTER(enum_glColor3i);

    if (__glTracerDispatchTable.Color3i)
    {
        (*__glTracerDispatchTable.Color3i)(red, green, blue);
    }
}

GLvoid GL_APIENTRY __glProfile_Color3iv(__GLcontext *gc, const GLint *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColor3iv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Color3iv(gc, v);
    __GL_PROFILE_FOOTER(enum_glColor3iv);

    if (__glTracerDispatchTable.Color3iv)
    {
        (*__glTracerDispatchTable.Color3iv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_Color3s(__GLcontext *gc, GLshort red, GLshort green, GLshort blue)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColor3s(red=%hd, green=%hd, blue=%hd)\n",
                     gc, tid, red, green, blue);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Color3s(gc, red, green, blue);
    __GL_PROFILE_FOOTER(enum_glColor3s);

    if (__glTracerDispatchTable.Color3s)
    {
        (*__glTracerDispatchTable.Color3s)(red, green, blue);
    }
}

GLvoid GL_APIENTRY __glProfile_Color3sv(__GLcontext *gc, const GLshort *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColor3sv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Color3sv(gc, v);
    __GL_PROFILE_FOOTER(enum_glColor3sv);

    if (__glTracerDispatchTable.Color3sv)
    {
        (*__glTracerDispatchTable.Color3sv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_Color3ub(__GLcontext *gc, GLubyte red, GLubyte green, GLubyte blue)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColor3ub(red=%hhu, green=%hhu, blue=%hhu)\n",
                     gc, tid, red, green, blue);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Color3ub(gc, red, green, blue);
    __GL_PROFILE_FOOTER(enum_glColor3ub);

    if (__glTracerDispatchTable.Color3ub)
    {
        (*__glTracerDispatchTable.Color3ub)(red, green, blue);
    }
}

GLvoid GL_APIENTRY __glProfile_Color3ubv(__GLcontext *gc, const GLubyte *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColor3ubv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Color3ubv(gc, v);
    __GL_PROFILE_FOOTER(enum_glColor3ubv);

    if (__glTracerDispatchTable.Color3ubv)
    {
        (*__glTracerDispatchTable.Color3ubv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_Color3ui(__GLcontext *gc, GLuint red, GLuint green, GLuint blue)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColor3ui(red=%u, green=%u, blue=%u)\n",
                     gc, tid, red, green, blue);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Color3ui(gc, red, green, blue);
    __GL_PROFILE_FOOTER(enum_glColor3ui);

    if (__glTracerDispatchTable.Color3ui)
    {
        (*__glTracerDispatchTable.Color3ui)(red, green, blue);
    }
}

GLvoid GL_APIENTRY __glProfile_Color3uiv(__GLcontext *gc, const GLuint *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColor3uiv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Color3uiv(gc, v);
    __GL_PROFILE_FOOTER(enum_glColor3uiv);

    if (__glTracerDispatchTable.Color3uiv)
    {
        (*__glTracerDispatchTable.Color3uiv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_Color3us(__GLcontext *gc, GLushort red, GLushort green, GLushort blue)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColor3us(red=%hu, green=%hu, blue=%hu)\n",
                     gc, tid, red, green, blue);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Color3us(gc, red, green, blue);
    __GL_PROFILE_FOOTER(enum_glColor3us);

    if (__glTracerDispatchTable.Color3us)
    {
        (*__glTracerDispatchTable.Color3us)(red, green, blue);
    }
}

GLvoid GL_APIENTRY __glProfile_Color3usv(__GLcontext *gc, const GLushort *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColor3usv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Color3usv(gc, v);
    __GL_PROFILE_FOOTER(enum_glColor3usv);

    if (__glTracerDispatchTable.Color3usv)
    {
        (*__glTracerDispatchTable.Color3usv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_Color4b(__GLcontext *gc, GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColor4b(red=%hhd, green=%hhd, blue=%hhd, alpha=%hhd)\n",
                     gc, tid, red, green, blue, alpha);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Color4b(gc, red, green, blue, alpha);
    __GL_PROFILE_FOOTER(enum_glColor4b);

    if (__glTracerDispatchTable.Color4b)
    {
        (*__glTracerDispatchTable.Color4b)(red, green, blue, alpha);
    }
}

GLvoid GL_APIENTRY __glProfile_Color4bv(__GLcontext *gc, const GLbyte *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColor4bv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Color4bv(gc, v);
    __GL_PROFILE_FOOTER(enum_glColor4bv);

    if (__glTracerDispatchTable.Color4bv)
    {
        (*__glTracerDispatchTable.Color4bv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_Color4d(__GLcontext *gc, GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColor4d(red=%lf, green=%lf, blue=%lf, alpha=%lf)\n",
                     gc, tid, red, green, blue, alpha);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Color4d(gc, red, green, blue, alpha);
    __GL_PROFILE_FOOTER(enum_glColor4d);

    if (__glTracerDispatchTable.Color4d)
    {
        (*__glTracerDispatchTable.Color4d)(red, green, blue, alpha);
    }
}

GLvoid GL_APIENTRY __glProfile_Color4dv(__GLcontext *gc, const GLdouble *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColor4dv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Color4dv(gc, v);
    __GL_PROFILE_FOOTER(enum_glColor4dv);

    if (__glTracerDispatchTable.Color4dv)
    {
        (*__glTracerDispatchTable.Color4dv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_Color4f(__GLcontext *gc, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColor4f(red=%f, green=%f, blue=%f, alpha=%f)\n",
                     gc, tid, red, green, blue, alpha);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Color4f(gc, red, green, blue, alpha);
    __GL_PROFILE_FOOTER(enum_glColor4f);

    if (__glTracerDispatchTable.Color4f)
    {
        (*__glTracerDispatchTable.Color4f)(red, green, blue, alpha);
    }
}

GLvoid GL_APIENTRY __glProfile_Color4fv(__GLcontext *gc, const GLfloat *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColor4fv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Color4fv(gc, v);
    __GL_PROFILE_FOOTER(enum_glColor4fv);

    if (__glTracerDispatchTable.Color4fv)
    {
        (*__glTracerDispatchTable.Color4fv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_Color4i(__GLcontext *gc, GLint red, GLint green, GLint blue, GLint alpha)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColor4i(red=%d, green=%d, blue=%d, alpha=%d)\n",
                     gc, tid, red, green, blue, alpha);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Color4i(gc, red, green, blue, alpha);
    __GL_PROFILE_FOOTER(enum_glColor4i);

    if (__glTracerDispatchTable.Color4i)
    {
        (*__glTracerDispatchTable.Color4i)(red, green, blue, alpha);
    }
}

GLvoid GL_APIENTRY __glProfile_Color4iv(__GLcontext *gc, const GLint *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColor4iv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Color4iv(gc, v);
    __GL_PROFILE_FOOTER(enum_glColor4iv);

    if (__glTracerDispatchTable.Color4iv)
    {
        (*__glTracerDispatchTable.Color4iv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_Color4s(__GLcontext *gc, GLshort red, GLshort green, GLshort blue, GLshort alpha)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColor4s(red=%hd, green=%hd, blue=%hd, alpha=%hd)\n",
                     gc, tid, red, green, blue, alpha);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Color4s(gc, red, green, blue, alpha);
    __GL_PROFILE_FOOTER(enum_glColor4s);

    if (__glTracerDispatchTable.Color4s)
    {
        (*__glTracerDispatchTable.Color4s)(red, green, blue, alpha);
    }
}

GLvoid GL_APIENTRY __glProfile_Color4sv(__GLcontext *gc, const GLshort *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColor4sv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Color4sv(gc, v);
    __GL_PROFILE_FOOTER(enum_glColor4sv);

    if (__glTracerDispatchTable.Color4sv)
    {
        (*__glTracerDispatchTable.Color4sv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_Color4ub(__GLcontext *gc, GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColor4ub(red=%hhu, green=%hhu, blue=%hhu, alpha=%hhu)\n",
                     gc, tid, red, green, blue, alpha);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Color4ub(gc, red, green, blue, alpha);
    __GL_PROFILE_FOOTER(enum_glColor4ub);

    if (__glTracerDispatchTable.Color4ub)
    {
        (*__glTracerDispatchTable.Color4ub)(red, green, blue, alpha);
    }
}

GLvoid GL_APIENTRY __glProfile_Color4ubv(__GLcontext *gc, const GLubyte *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColor4ubv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Color4ubv(gc, v);
    __GL_PROFILE_FOOTER(enum_glColor4ubv);

    if (__glTracerDispatchTable.Color4ubv)
    {
        (*__glTracerDispatchTable.Color4ubv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_Color4ui(__GLcontext *gc, GLuint red, GLuint green, GLuint blue, GLuint alpha)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColor4ui(red=%u, green=%u, blue=%u, alpha=%u)\n",
                     gc, tid, red, green, blue, alpha);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Color4ui(gc, red, green, blue, alpha);
    __GL_PROFILE_FOOTER(enum_glColor4ui);

    if (__glTracerDispatchTable.Color4ui)
    {
        (*__glTracerDispatchTable.Color4ui)(red, green, blue, alpha);
    }
}

GLvoid GL_APIENTRY __glProfile_Color4uiv(__GLcontext *gc, const GLuint *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColor4uiv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Color4uiv(gc, v);
    __GL_PROFILE_FOOTER(enum_glColor4uiv);

    if (__glTracerDispatchTable.Color4uiv)
    {
        (*__glTracerDispatchTable.Color4uiv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_Color4us(__GLcontext *gc, GLushort red, GLushort green, GLushort blue, GLushort alpha)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColor4us(red=%hu, green=%hu, blue=%hu, alpha=%hu)\n",
                     gc, tid, red, green, blue, alpha);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Color4us(gc, red, green, blue, alpha);
    __GL_PROFILE_FOOTER(enum_glColor4us);

    if (__glTracerDispatchTable.Color4us)
    {
        (*__glTracerDispatchTable.Color4us)(red, green, blue, alpha);
    }
}

GLvoid GL_APIENTRY __glProfile_Color4usv(__GLcontext *gc, const GLushort *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColor4usv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Color4usv(gc, v);
    __GL_PROFILE_FOOTER(enum_glColor4usv);

    if (__glTracerDispatchTable.Color4usv)
    {
        (*__glTracerDispatchTable.Color4usv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_EdgeFlag(__GLcontext *gc, GLboolean flag)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glEdgeFlag(flag=%hhu)\n",
                     gc, tid, flag);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->EdgeFlag(gc, flag);
    __GL_PROFILE_FOOTER(enum_glEdgeFlag);

    if (__glTracerDispatchTable.EdgeFlag)
    {
        (*__glTracerDispatchTable.EdgeFlag)(flag);
    }
}

GLvoid GL_APIENTRY __glProfile_EdgeFlagv(__GLcontext *gc, const GLboolean *flag)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glEdgeFlagv(flag=0x%p)\n",
                     gc, tid, flag);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->EdgeFlagv(gc, flag);
    __GL_PROFILE_FOOTER(enum_glEdgeFlagv);

    if (__glTracerDispatchTable.EdgeFlagv)
    {
        (*__glTracerDispatchTable.EdgeFlagv)(flag);
    }
}

GLvoid GL_APIENTRY __glProfile_End(__GLcontext *gc)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glEnd()\n",
                     gc, tid);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->End(gc);
    __GL_PROFILE_FOOTER(enum_glEnd);

    if (__glTracerDispatchTable.End)
    {
        (*__glTracerDispatchTable.End)();
    }
}

GLvoid GL_APIENTRY __glProfile_Indexd(__GLcontext *gc, GLdouble c)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glIndexd(c=%lf)\n",
                     gc, tid, c);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Indexd(gc, c);
    __GL_PROFILE_FOOTER(enum_glIndexd);

    if (__glTracerDispatchTable.Indexd)
    {
        (*__glTracerDispatchTable.Indexd)(c);
    }
}

GLvoid GL_APIENTRY __glProfile_Indexdv(__GLcontext *gc, const GLdouble *c)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glIndexdv(c=0x%p)\n",
                     gc, tid, c);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Indexdv(gc, c);
    __GL_PROFILE_FOOTER(enum_glIndexdv);

    if (__glTracerDispatchTable.Indexdv)
    {
        (*__glTracerDispatchTable.Indexdv)(c);
    }
}

GLvoid GL_APIENTRY __glProfile_Indexf(__GLcontext *gc, GLfloat c)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glIndexf(c=%f)\n",
                     gc, tid, c);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Indexf(gc, c);
    __GL_PROFILE_FOOTER(enum_glIndexf);

    if (__glTracerDispatchTable.Indexf)
    {
        (*__glTracerDispatchTable.Indexf)(c);
    }
}

GLvoid GL_APIENTRY __glProfile_Indexfv(__GLcontext *gc, const GLfloat *c)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glIndexfv(c=0x%p)\n",
                     gc, tid, c);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Indexfv(gc, c);
    __GL_PROFILE_FOOTER(enum_glIndexfv);

    if (__glTracerDispatchTable.Indexfv)
    {
        (*__glTracerDispatchTable.Indexfv)(c);
    }
}

GLvoid GL_APIENTRY __glProfile_Indexi(__GLcontext *gc, GLint c)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glIndexi(c=%d)\n",
                     gc, tid, c);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Indexi(gc, c);
    __GL_PROFILE_FOOTER(enum_glIndexi);

    if (__glTracerDispatchTable.Indexi)
    {
        (*__glTracerDispatchTable.Indexi)(c);
    }
}

GLvoid GL_APIENTRY __glProfile_Indexiv(__GLcontext *gc, const GLint *c)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glIndexiv(c=0x%p)\n",
                     gc, tid, c);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Indexiv(gc, c);
    __GL_PROFILE_FOOTER(enum_glIndexiv);

    if (__glTracerDispatchTable.Indexiv)
    {
        (*__glTracerDispatchTable.Indexiv)(c);
    }
}

GLvoid GL_APIENTRY __glProfile_Indexs(__GLcontext *gc, GLshort c)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glIndexs(c=%hd)\n",
                     gc, tid, c);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Indexs(gc, c);
    __GL_PROFILE_FOOTER(enum_glIndexs);

    if (__glTracerDispatchTable.Indexs)
    {
        (*__glTracerDispatchTable.Indexs)(c);
    }
}

GLvoid GL_APIENTRY __glProfile_Indexsv(__GLcontext *gc, const GLshort *c)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glIndexsv(c=0x%p)\n",
                     gc, tid, c);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Indexsv(gc, c);
    __GL_PROFILE_FOOTER(enum_glIndexsv);

    if (__glTracerDispatchTable.Indexsv)
    {
        (*__glTracerDispatchTable.Indexsv)(c);
    }
}

GLvoid GL_APIENTRY __glProfile_Normal3b(__GLcontext *gc, GLbyte nx, GLbyte ny, GLbyte nz)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glNormal3b(nx=%hhd, ny=%hhd, nz=%hhd)\n",
                     gc, tid, nx, ny, nz);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Normal3b(gc, nx, ny, nz);
    __GL_PROFILE_FOOTER(enum_glNormal3b);

    if (__glTracerDispatchTable.Normal3b)
    {
        (*__glTracerDispatchTable.Normal3b)(nx, ny, nz);
    }
}

GLvoid GL_APIENTRY __glProfile_Normal3bv(__GLcontext *gc, const GLbyte *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glNormal3bv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Normal3bv(gc, v);
    __GL_PROFILE_FOOTER(enum_glNormal3bv);

    if (__glTracerDispatchTable.Normal3bv)
    {
        (*__glTracerDispatchTable.Normal3bv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_Normal3d(__GLcontext *gc, GLdouble nx, GLdouble ny, GLdouble nz)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glNormal3d(nx=%lf, ny=%lf, nz=%lf)\n",
                     gc, tid, nx, ny, nz);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Normal3d(gc, nx, ny, nz);
    __GL_PROFILE_FOOTER(enum_glNormal3d);

    if (__glTracerDispatchTable.Normal3d)
    {
        (*__glTracerDispatchTable.Normal3d)(nx, ny, nz);
    }
}

GLvoid GL_APIENTRY __glProfile_Normal3dv(__GLcontext *gc, const GLdouble *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glNormal3dv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Normal3dv(gc, v);
    __GL_PROFILE_FOOTER(enum_glNormal3dv);

    if (__glTracerDispatchTable.Normal3dv)
    {
        (*__glTracerDispatchTable.Normal3dv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_Normal3f(__GLcontext *gc, GLfloat nx, GLfloat ny, GLfloat nz)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glNormal3f(nx=%f, ny=%f, nz=%f)\n",
                     gc, tid, nx, ny, nz);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Normal3f(gc, nx, ny, nz);
    __GL_PROFILE_FOOTER(enum_glNormal3f);

    if (__glTracerDispatchTable.Normal3f)
    {
        (*__glTracerDispatchTable.Normal3f)(nx, ny, nz);
    }
}

GLvoid GL_APIENTRY __glProfile_Normal3fv(__GLcontext *gc, const GLfloat *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glNormal3fv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Normal3fv(gc, v);
    __GL_PROFILE_FOOTER(enum_glNormal3fv);

    if (__glTracerDispatchTable.Normal3fv)
    {
        (*__glTracerDispatchTable.Normal3fv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_Normal3i(__GLcontext *gc, GLint nx, GLint ny, GLint nz)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glNormal3i(nx=%d, ny=%d, nz=%d)\n",
                     gc, tid, nx, ny, nz);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Normal3i(gc, nx, ny, nz);
    __GL_PROFILE_FOOTER(enum_glNormal3i);

    if (__glTracerDispatchTable.Normal3i)
    {
        (*__glTracerDispatchTable.Normal3i)(nx, ny, nz);
    }
}

GLvoid GL_APIENTRY __glProfile_Normal3iv(__GLcontext *gc, const GLint *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glNormal3iv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Normal3iv(gc, v);
    __GL_PROFILE_FOOTER(enum_glNormal3iv);

    if (__glTracerDispatchTable.Normal3iv)
    {
        (*__glTracerDispatchTable.Normal3iv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_Normal3s(__GLcontext *gc, GLshort nx, GLshort ny, GLshort nz)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glNormal3s(nx=%hd, ny=%hd, nz=%hd)\n",
                     gc, tid, nx, ny, nz);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Normal3s(gc, nx, ny, nz);
    __GL_PROFILE_FOOTER(enum_glNormal3s);

    if (__glTracerDispatchTable.Normal3s)
    {
        (*__glTracerDispatchTable.Normal3s)(nx, ny, nz);
    }
}

GLvoid GL_APIENTRY __glProfile_Normal3sv(__GLcontext *gc, const GLshort *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glNormal3sv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Normal3sv(gc, v);
    __GL_PROFILE_FOOTER(enum_glNormal3sv);

    if (__glTracerDispatchTable.Normal3sv)
    {
        (*__glTracerDispatchTable.Normal3sv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_RasterPos2d(__GLcontext *gc, GLdouble x, GLdouble y)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glRasterPos2d(x=%lf, y=%lf)\n",
                     gc, tid, x, y);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->RasterPos2d(gc, x, y);
    __GL_PROFILE_FOOTER(enum_glRasterPos2d);

    if (__glTracerDispatchTable.RasterPos2d)
    {
        (*__glTracerDispatchTable.RasterPos2d)(x, y);
    }
}

GLvoid GL_APIENTRY __glProfile_RasterPos2dv(__GLcontext *gc, const GLdouble *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glRasterPos2dv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->RasterPos2dv(gc, v);
    __GL_PROFILE_FOOTER(enum_glRasterPos2dv);

    if (__glTracerDispatchTable.RasterPos2dv)
    {
        (*__glTracerDispatchTable.RasterPos2dv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_RasterPos2f(__GLcontext *gc, GLfloat x, GLfloat y)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glRasterPos2f(x=%f, y=%f)\n",
                     gc, tid, x, y);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->RasterPos2f(gc, x, y);
    __GL_PROFILE_FOOTER(enum_glRasterPos2f);

    if (__glTracerDispatchTable.RasterPos2f)
    {
        (*__glTracerDispatchTable.RasterPos2f)(x, y);
    }
}

GLvoid GL_APIENTRY __glProfile_RasterPos2fv(__GLcontext *gc, const GLfloat *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glRasterPos2fv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->RasterPos2fv(gc, v);
    __GL_PROFILE_FOOTER(enum_glRasterPos2fv);

    if (__glTracerDispatchTable.RasterPos2fv)
    {
        (*__glTracerDispatchTable.RasterPos2fv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_RasterPos2i(__GLcontext *gc, GLint x, GLint y)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glRasterPos2i(x=%d, y=%d)\n",
                     gc, tid, x, y);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->RasterPos2i(gc, x, y);
    __GL_PROFILE_FOOTER(enum_glRasterPos2i);

    if (__glTracerDispatchTable.RasterPos2i)
    {
        (*__glTracerDispatchTable.RasterPos2i)(x, y);
    }
}

GLvoid GL_APIENTRY __glProfile_RasterPos2iv(__GLcontext *gc, const GLint *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glRasterPos2iv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->RasterPos2iv(gc, v);
    __GL_PROFILE_FOOTER(enum_glRasterPos2iv);

    if (__glTracerDispatchTable.RasterPos2iv)
    {
        (*__glTracerDispatchTable.RasterPos2iv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_RasterPos2s(__GLcontext *gc, GLshort x, GLshort y)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glRasterPos2s(x=%hd, y=%hd)\n",
                     gc, tid, x, y);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->RasterPos2s(gc, x, y);
    __GL_PROFILE_FOOTER(enum_glRasterPos2s);

    if (__glTracerDispatchTable.RasterPos2s)
    {
        (*__glTracerDispatchTable.RasterPos2s)(x, y);
    }
}

GLvoid GL_APIENTRY __glProfile_RasterPos2sv(__GLcontext *gc, const GLshort *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glRasterPos2sv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->RasterPos2sv(gc, v);
    __GL_PROFILE_FOOTER(enum_glRasterPos2sv);

    if (__glTracerDispatchTable.RasterPos2sv)
    {
        (*__glTracerDispatchTable.RasterPos2sv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_RasterPos3d(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glRasterPos3d(x=%lf, y=%lf, z=%lf)\n",
                     gc, tid, x, y, z);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->RasterPos3d(gc, x, y, z);
    __GL_PROFILE_FOOTER(enum_glRasterPos3d);

    if (__glTracerDispatchTable.RasterPos3d)
    {
        (*__glTracerDispatchTable.RasterPos3d)(x, y, z);
    }
}

GLvoid GL_APIENTRY __glProfile_RasterPos3dv(__GLcontext *gc, const GLdouble *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glRasterPos3dv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->RasterPos3dv(gc, v);
    __GL_PROFILE_FOOTER(enum_glRasterPos3dv);

    if (__glTracerDispatchTable.RasterPos3dv)
    {
        (*__glTracerDispatchTable.RasterPos3dv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_RasterPos3f(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glRasterPos3f(x=%f, y=%f, z=%f)\n",
                     gc, tid, x, y, z);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->RasterPos3f(gc, x, y, z);
    __GL_PROFILE_FOOTER(enum_glRasterPos3f);

    if (__glTracerDispatchTable.RasterPos3f)
    {
        (*__glTracerDispatchTable.RasterPos3f)(x, y, z);
    }
}

GLvoid GL_APIENTRY __glProfile_RasterPos3fv(__GLcontext *gc, const GLfloat *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glRasterPos3fv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->RasterPos3fv(gc, v);
    __GL_PROFILE_FOOTER(enum_glRasterPos3fv);

    if (__glTracerDispatchTable.RasterPos3fv)
    {
        (*__glTracerDispatchTable.RasterPos3fv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_RasterPos3i(__GLcontext *gc, GLint x, GLint y, GLint z)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glRasterPos3i(x=%d, y=%d, z=%d)\n",
                     gc, tid, x, y, z);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->RasterPos3i(gc, x, y, z);
    __GL_PROFILE_FOOTER(enum_glRasterPos3i);

    if (__glTracerDispatchTable.RasterPos3i)
    {
        (*__glTracerDispatchTable.RasterPos3i)(x, y, z);
    }
}

GLvoid GL_APIENTRY __glProfile_RasterPos3iv(__GLcontext *gc, const GLint *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glRasterPos3iv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->RasterPos3iv(gc, v);
    __GL_PROFILE_FOOTER(enum_glRasterPos3iv);

    if (__glTracerDispatchTable.RasterPos3iv)
    {
        (*__glTracerDispatchTable.RasterPos3iv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_RasterPos3s(__GLcontext *gc, GLshort x, GLshort y, GLshort z)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glRasterPos3s(x=%hd, y=%hd, z=%hd)\n",
                     gc, tid, x, y, z);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->RasterPos3s(gc, x, y, z);
    __GL_PROFILE_FOOTER(enum_glRasterPos3s);

    if (__glTracerDispatchTable.RasterPos3s)
    {
        (*__glTracerDispatchTable.RasterPos3s)(x, y, z);
    }
}

GLvoid GL_APIENTRY __glProfile_RasterPos3sv(__GLcontext *gc, const GLshort *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glRasterPos3sv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->RasterPos3sv(gc, v);
    __GL_PROFILE_FOOTER(enum_glRasterPos3sv);

    if (__glTracerDispatchTable.RasterPos3sv)
    {
        (*__glTracerDispatchTable.RasterPos3sv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_RasterPos4d(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glRasterPos4d(x=%lf, y=%lf, z=%lf, w=%lf)\n",
                     gc, tid, x, y, z, w);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->RasterPos4d(gc, x, y, z, w);
    __GL_PROFILE_FOOTER(enum_glRasterPos4d);

    if (__glTracerDispatchTable.RasterPos4d)
    {
        (*__glTracerDispatchTable.RasterPos4d)(x, y, z, w);
    }
}

GLvoid GL_APIENTRY __glProfile_RasterPos4dv(__GLcontext *gc, const GLdouble *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glRasterPos4dv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->RasterPos4dv(gc, v);
    __GL_PROFILE_FOOTER(enum_glRasterPos4dv);

    if (__glTracerDispatchTable.RasterPos4dv)
    {
        (*__glTracerDispatchTable.RasterPos4dv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_RasterPos4f(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glRasterPos4f(x=%f, y=%f, z=%f, w=%f)\n",
                     gc, tid, x, y, z, w);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->RasterPos4f(gc, x, y, z, w);
    __GL_PROFILE_FOOTER(enum_glRasterPos4f);

    if (__glTracerDispatchTable.RasterPos4f)
    {
        (*__glTracerDispatchTable.RasterPos4f)(x, y, z, w);
    }
}

GLvoid GL_APIENTRY __glProfile_RasterPos4fv(__GLcontext *gc, const GLfloat *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glRasterPos4fv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->RasterPos4fv(gc, v);
    __GL_PROFILE_FOOTER(enum_glRasterPos4fv);

    if (__glTracerDispatchTable.RasterPos4fv)
    {
        (*__glTracerDispatchTable.RasterPos4fv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_RasterPos4i(__GLcontext *gc, GLint x, GLint y, GLint z, GLint w)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glRasterPos4i(x=%d, y=%d, z=%d, w=%d)\n",
                     gc, tid, x, y, z, w);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->RasterPos4i(gc, x, y, z, w);
    __GL_PROFILE_FOOTER(enum_glRasterPos4i);

    if (__glTracerDispatchTable.RasterPos4i)
    {
        (*__glTracerDispatchTable.RasterPos4i)(x, y, z, w);
    }
}

GLvoid GL_APIENTRY __glProfile_RasterPos4iv(__GLcontext *gc, const GLint *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glRasterPos4iv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->RasterPos4iv(gc, v);
    __GL_PROFILE_FOOTER(enum_glRasterPos4iv);

    if (__glTracerDispatchTable.RasterPos4iv)
    {
        (*__glTracerDispatchTable.RasterPos4iv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_RasterPos4s(__GLcontext *gc, GLshort x, GLshort y, GLshort z, GLshort w)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glRasterPos4s(x=%hd, y=%hd, z=%hd, w=%hd)\n",
                     gc, tid, x, y, z, w);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->RasterPos4s(gc, x, y, z, w);
    __GL_PROFILE_FOOTER(enum_glRasterPos4s);

    if (__glTracerDispatchTable.RasterPos4s)
    {
        (*__glTracerDispatchTable.RasterPos4s)(x, y, z, w);
    }
}

GLvoid GL_APIENTRY __glProfile_RasterPos4sv(__GLcontext *gc, const GLshort *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glRasterPos4sv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->RasterPos4sv(gc, v);
    __GL_PROFILE_FOOTER(enum_glRasterPos4sv);

    if (__glTracerDispatchTable.RasterPos4sv)
    {
        (*__glTracerDispatchTable.RasterPos4sv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_Rectd(__GLcontext *gc, GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glRectd(x1=%lf, y1=%lf, x2=%lf, y2=%lf)\n",
                     gc, tid, x1, y1, x2, y2);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Rectd(gc, x1, y1, x2, y2);
    __GL_PROFILE_FOOTER(enum_glRectd);

    if (__glTracerDispatchTable.Rectd)
    {
        (*__glTracerDispatchTable.Rectd)(x1, y1, x2, y2);
    }
}

GLvoid GL_APIENTRY __glProfile_Rectdv(__GLcontext *gc, const GLdouble *v1, const GLdouble *v2)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glRectdv(v1=0x%p, v2=0x%p)\n",
                     gc, tid, v1, v2);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Rectdv(gc, v1, v2);
    __GL_PROFILE_FOOTER(enum_glRectdv);

    if (__glTracerDispatchTable.Rectdv)
    {
        (*__glTracerDispatchTable.Rectdv)(v1, v2);
    }
}

GLvoid GL_APIENTRY __glProfile_Rectf(__GLcontext *gc, GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glRectf(x1=%f, y1=%f, x2=%f, y2=%f)\n",
                     gc, tid, x1, y1, x2, y2);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Rectf(gc, x1, y1, x2, y2);
    __GL_PROFILE_FOOTER(enum_glRectf);

    if (__glTracerDispatchTable.Rectf)
    {
        (*__glTracerDispatchTable.Rectf)(x1, y1, x2, y2);
    }
}

GLvoid GL_APIENTRY __glProfile_Rectfv(__GLcontext *gc, const GLfloat *v1, const GLfloat *v2)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glRectfv(v1=0x%p, v2=0x%p)\n",
                     gc, tid, v1, v2);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Rectfv(gc, v1, v2);
    __GL_PROFILE_FOOTER(enum_glRectfv);

    if (__glTracerDispatchTable.Rectfv)
    {
        (*__glTracerDispatchTable.Rectfv)(v1, v2);
    }
}

GLvoid GL_APIENTRY __glProfile_Recti(__GLcontext *gc, GLint x1, GLint y1, GLint x2, GLint y2)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glRecti(x1=%d, y1=%d, x2=%d, y2=%d)\n",
                     gc, tid, x1, y1, x2, y2);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Recti(gc, x1, y1, x2, y2);
    __GL_PROFILE_FOOTER(enum_glRecti);

    if (__glTracerDispatchTable.Recti)
    {
        (*__glTracerDispatchTable.Recti)(x1, y1, x2, y2);
    }
}

GLvoid GL_APIENTRY __glProfile_Rectiv(__GLcontext *gc, const GLint *v1, const GLint *v2)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glRectiv(v1=0x%p, v2=0x%p)\n",
                     gc, tid, v1, v2);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Rectiv(gc, v1, v2);
    __GL_PROFILE_FOOTER(enum_glRectiv);

    if (__glTracerDispatchTable.Rectiv)
    {
        (*__glTracerDispatchTable.Rectiv)(v1, v2);
    }
}

GLvoid GL_APIENTRY __glProfile_Rects(__GLcontext *gc, GLshort x1, GLshort y1, GLshort x2, GLshort y2)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glRects(x1=%hd, y1=%hd, x2=%hd, y2=%hd)\n",
                     gc, tid, x1, y1, x2, y2);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Rects(gc, x1, y1, x2, y2);
    __GL_PROFILE_FOOTER(enum_glRects);

    if (__glTracerDispatchTable.Rects)
    {
        (*__glTracerDispatchTable.Rects)(x1, y1, x2, y2);
    }
}

GLvoid GL_APIENTRY __glProfile_Rectsv(__GLcontext *gc, const GLshort *v1, const GLshort *v2)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glRectsv(v1=0x%p, v2=0x%p)\n",
                     gc, tid, v1, v2);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Rectsv(gc, v1, v2);
    __GL_PROFILE_FOOTER(enum_glRectsv);

    if (__glTracerDispatchTable.Rectsv)
    {
        (*__glTracerDispatchTable.Rectsv)(v1, v2);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoord1d(__GLcontext *gc, GLdouble s)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoord1d(s=%lf)\n",
                     gc, tid, s);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoord1d(gc, s);
    __GL_PROFILE_FOOTER(enum_glTexCoord1d);

    if (__glTracerDispatchTable.TexCoord1d)
    {
        (*__glTracerDispatchTable.TexCoord1d)(s);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoord1dv(__GLcontext *gc, const GLdouble *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoord1dv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoord1dv(gc, v);
    __GL_PROFILE_FOOTER(enum_glTexCoord1dv);

    if (__glTracerDispatchTable.TexCoord1dv)
    {
        (*__glTracerDispatchTable.TexCoord1dv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoord1f(__GLcontext *gc, GLfloat s)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoord1f(s=%f)\n",
                     gc, tid, s);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoord1f(gc, s);
    __GL_PROFILE_FOOTER(enum_glTexCoord1f);

    if (__glTracerDispatchTable.TexCoord1f)
    {
        (*__glTracerDispatchTable.TexCoord1f)(s);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoord1fv(__GLcontext *gc, const GLfloat *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoord1fv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoord1fv(gc, v);
    __GL_PROFILE_FOOTER(enum_glTexCoord1fv);

    if (__glTracerDispatchTable.TexCoord1fv)
    {
        (*__glTracerDispatchTable.TexCoord1fv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoord1i(__GLcontext *gc, GLint s)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoord1i(s=%d)\n",
                     gc, tid, s);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoord1i(gc, s);
    __GL_PROFILE_FOOTER(enum_glTexCoord1i);

    if (__glTracerDispatchTable.TexCoord1i)
    {
        (*__glTracerDispatchTable.TexCoord1i)(s);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoord1iv(__GLcontext *gc, const GLint *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoord1iv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoord1iv(gc, v);
    __GL_PROFILE_FOOTER(enum_glTexCoord1iv);

    if (__glTracerDispatchTable.TexCoord1iv)
    {
        (*__glTracerDispatchTable.TexCoord1iv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoord1s(__GLcontext *gc, GLshort s)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoord1s(s=%hd)\n",
                     gc, tid, s);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoord1s(gc, s);
    __GL_PROFILE_FOOTER(enum_glTexCoord1s);

    if (__glTracerDispatchTable.TexCoord1s)
    {
        (*__glTracerDispatchTable.TexCoord1s)(s);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoord1sv(__GLcontext *gc, const GLshort *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoord1sv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoord1sv(gc, v);
    __GL_PROFILE_FOOTER(enum_glTexCoord1sv);

    if (__glTracerDispatchTable.TexCoord1sv)
    {
        (*__glTracerDispatchTable.TexCoord1sv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoord2d(__GLcontext *gc, GLdouble s, GLdouble t)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoord2d(s=%lf, t=%lf)\n",
                     gc, tid, s, t);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoord2d(gc, s, t);
    __GL_PROFILE_FOOTER(enum_glTexCoord2d);

    if (__glTracerDispatchTable.TexCoord2d)
    {
        (*__glTracerDispatchTable.TexCoord2d)(s, t);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoord2dv(__GLcontext *gc, const GLdouble *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoord2dv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoord2dv(gc, v);
    __GL_PROFILE_FOOTER(enum_glTexCoord2dv);

    if (__glTracerDispatchTable.TexCoord2dv)
    {
        (*__glTracerDispatchTable.TexCoord2dv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoord2f(__GLcontext *gc, GLfloat s, GLfloat t)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoord2f(s=%f, t=%f)\n",
                     gc, tid, s, t);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoord2f(gc, s, t);
    __GL_PROFILE_FOOTER(enum_glTexCoord2f);

    if (__glTracerDispatchTable.TexCoord2f)
    {
        (*__glTracerDispatchTable.TexCoord2f)(s, t);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoord2fv(__GLcontext *gc, const GLfloat *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoord2fv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoord2fv(gc, v);
    __GL_PROFILE_FOOTER(enum_glTexCoord2fv);

    if (__glTracerDispatchTable.TexCoord2fv)
    {
        (*__glTracerDispatchTable.TexCoord2fv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoord2i(__GLcontext *gc, GLint s, GLint t)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoord2i(s=%d, t=%d)\n",
                     gc, tid, s, t);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoord2i(gc, s, t);
    __GL_PROFILE_FOOTER(enum_glTexCoord2i);

    if (__glTracerDispatchTable.TexCoord2i)
    {
        (*__glTracerDispatchTable.TexCoord2i)(s, t);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoord2iv(__GLcontext *gc, const GLint *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoord2iv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoord2iv(gc, v);
    __GL_PROFILE_FOOTER(enum_glTexCoord2iv);

    if (__glTracerDispatchTable.TexCoord2iv)
    {
        (*__glTracerDispatchTable.TexCoord2iv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoord2s(__GLcontext *gc, GLshort s, GLshort t)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoord2s(s=%hd, t=%hd)\n",
                     gc, tid, s, t);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoord2s(gc, s, t);
    __GL_PROFILE_FOOTER(enum_glTexCoord2s);

    if (__glTracerDispatchTable.TexCoord2s)
    {
        (*__glTracerDispatchTable.TexCoord2s)(s, t);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoord2sv(__GLcontext *gc, const GLshort *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoord2sv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoord2sv(gc, v);
    __GL_PROFILE_FOOTER(enum_glTexCoord2sv);

    if (__glTracerDispatchTable.TexCoord2sv)
    {
        (*__glTracerDispatchTable.TexCoord2sv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoord3d(__GLcontext *gc, GLdouble s, GLdouble t, GLdouble r)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoord3d(s=%lf, t=%lf, r=%lf)\n",
                     gc, tid, s, t, r);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoord3d(gc, s, t, r);
    __GL_PROFILE_FOOTER(enum_glTexCoord3d);

    if (__glTracerDispatchTable.TexCoord3d)
    {
        (*__glTracerDispatchTable.TexCoord3d)(s, t, r);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoord3dv(__GLcontext *gc, const GLdouble *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoord3dv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoord3dv(gc, v);
    __GL_PROFILE_FOOTER(enum_glTexCoord3dv);

    if (__glTracerDispatchTable.TexCoord3dv)
    {
        (*__glTracerDispatchTable.TexCoord3dv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoord3f(__GLcontext *gc, GLfloat s, GLfloat t, GLfloat r)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoord3f(s=%f, t=%f, r=%f)\n",
                     gc, tid, s, t, r);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoord3f(gc, s, t, r);
    __GL_PROFILE_FOOTER(enum_glTexCoord3f);

    if (__glTracerDispatchTable.TexCoord3f)
    {
        (*__glTracerDispatchTable.TexCoord3f)(s, t, r);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoord3fv(__GLcontext *gc, const GLfloat *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoord3fv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoord3fv(gc, v);
    __GL_PROFILE_FOOTER(enum_glTexCoord3fv);

    if (__glTracerDispatchTable.TexCoord3fv)
    {
        (*__glTracerDispatchTable.TexCoord3fv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoord3i(__GLcontext *gc, GLint s, GLint t, GLint r)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoord3i(s=%d, t=%d, r=%d)\n",
                     gc, tid, s, t, r);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoord3i(gc, s, t, r);
    __GL_PROFILE_FOOTER(enum_glTexCoord3i);

    if (__glTracerDispatchTable.TexCoord3i)
    {
        (*__glTracerDispatchTable.TexCoord3i)(s, t, r);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoord3iv(__GLcontext *gc, const GLint *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoord3iv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoord3iv(gc, v);
    __GL_PROFILE_FOOTER(enum_glTexCoord3iv);

    if (__glTracerDispatchTable.TexCoord3iv)
    {
        (*__glTracerDispatchTable.TexCoord3iv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoord3s(__GLcontext *gc, GLshort s, GLshort t, GLshort r)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoord3s(s=%hd, t=%hd, r=%hd)\n",
                     gc, tid, s, t, r);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoord3s(gc, s, t, r);
    __GL_PROFILE_FOOTER(enum_glTexCoord3s);

    if (__glTracerDispatchTable.TexCoord3s)
    {
        (*__glTracerDispatchTable.TexCoord3s)(s, t, r);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoord3sv(__GLcontext *gc, const GLshort *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoord3sv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoord3sv(gc, v);
    __GL_PROFILE_FOOTER(enum_glTexCoord3sv);

    if (__glTracerDispatchTable.TexCoord3sv)
    {
        (*__glTracerDispatchTable.TexCoord3sv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoord4d(__GLcontext *gc, GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoord4d(s=%lf, t=%lf, r=%lf, q=%lf)\n",
                     gc, tid, s, t, r, q);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoord4d(gc, s, t, r, q);
    __GL_PROFILE_FOOTER(enum_glTexCoord4d);

    if (__glTracerDispatchTable.TexCoord4d)
    {
        (*__glTracerDispatchTable.TexCoord4d)(s, t, r, q);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoord4dv(__GLcontext *gc, const GLdouble *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoord4dv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoord4dv(gc, v);
    __GL_PROFILE_FOOTER(enum_glTexCoord4dv);

    if (__glTracerDispatchTable.TexCoord4dv)
    {
        (*__glTracerDispatchTable.TexCoord4dv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoord4f(__GLcontext *gc, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoord4f(s=%f, t=%f, r=%f, q=%f)\n",
                     gc, tid, s, t, r, q);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoord4f(gc, s, t, r, q);
    __GL_PROFILE_FOOTER(enum_glTexCoord4f);

    if (__glTracerDispatchTable.TexCoord4f)
    {
        (*__glTracerDispatchTable.TexCoord4f)(s, t, r, q);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoord4fv(__GLcontext *gc, const GLfloat *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoord4fv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoord4fv(gc, v);
    __GL_PROFILE_FOOTER(enum_glTexCoord4fv);

    if (__glTracerDispatchTable.TexCoord4fv)
    {
        (*__glTracerDispatchTable.TexCoord4fv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoord4i(__GLcontext *gc, GLint s, GLint t, GLint r, GLint q)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoord4i(s=%d, t=%d, r=%d, q=%d)\n",
                     gc, tid, s, t, r, q);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoord4i(gc, s, t, r, q);
    __GL_PROFILE_FOOTER(enum_glTexCoord4i);

    if (__glTracerDispatchTable.TexCoord4i)
    {
        (*__glTracerDispatchTable.TexCoord4i)(s, t, r, q);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoord4iv(__GLcontext *gc, const GLint *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoord4iv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoord4iv(gc, v);
    __GL_PROFILE_FOOTER(enum_glTexCoord4iv);

    if (__glTracerDispatchTable.TexCoord4iv)
    {
        (*__glTracerDispatchTable.TexCoord4iv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoord4s(__GLcontext *gc, GLshort s, GLshort t, GLshort r, GLshort q)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoord4s(s=%hd, t=%hd, r=%hd, q=%hd)\n",
                     gc, tid, s, t, r, q);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoord4s(gc, s, t, r, q);
    __GL_PROFILE_FOOTER(enum_glTexCoord4s);

    if (__glTracerDispatchTable.TexCoord4s)
    {
        (*__glTracerDispatchTable.TexCoord4s)(s, t, r, q);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoord4sv(__GLcontext *gc, const GLshort *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoord4sv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoord4sv(gc, v);
    __GL_PROFILE_FOOTER(enum_glTexCoord4sv);

    if (__glTracerDispatchTable.TexCoord4sv)
    {
        (*__glTracerDispatchTable.TexCoord4sv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_Vertex2d(__GLcontext *gc, GLdouble x, GLdouble y)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertex2d(x=%lf, y=%lf)\n",
                     gc, tid, x, y);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Vertex2d(gc, x, y);
    __GL_PROFILE_FOOTER(enum_glVertex2d);

    if (__glTracerDispatchTable.Vertex2d)
    {
        (*__glTracerDispatchTable.Vertex2d)(x, y);
    }
}

GLvoid GL_APIENTRY __glProfile_Vertex2dv(__GLcontext *gc, const GLdouble *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertex2dv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Vertex2dv(gc, v);
    __GL_PROFILE_FOOTER(enum_glVertex2dv);

    if (__glTracerDispatchTable.Vertex2dv)
    {
        (*__glTracerDispatchTable.Vertex2dv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_Vertex2f(__GLcontext *gc, GLfloat x, GLfloat y)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertex2f(x=%f, y=%f)\n",
                     gc, tid, x, y);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Vertex2f(gc, x, y);
    __GL_PROFILE_FOOTER(enum_glVertex2f);

    if (__glTracerDispatchTable.Vertex2f)
    {
        (*__glTracerDispatchTable.Vertex2f)(x, y);
    }
}

GLvoid GL_APIENTRY __glProfile_Vertex2fv(__GLcontext *gc, const GLfloat *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertex2fv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Vertex2fv(gc, v);
    __GL_PROFILE_FOOTER(enum_glVertex2fv);

    if (__glTracerDispatchTable.Vertex2fv)
    {
        (*__glTracerDispatchTable.Vertex2fv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_Vertex2i(__GLcontext *gc, GLint x, GLint y)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertex2i(x=%d, y=%d)\n",
                     gc, tid, x, y);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Vertex2i(gc, x, y);
    __GL_PROFILE_FOOTER(enum_glVertex2i);

    if (__glTracerDispatchTable.Vertex2i)
    {
        (*__glTracerDispatchTable.Vertex2i)(x, y);
    }
}

GLvoid GL_APIENTRY __glProfile_Vertex2iv(__GLcontext *gc, const GLint *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertex2iv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Vertex2iv(gc, v);
    __GL_PROFILE_FOOTER(enum_glVertex2iv);

    if (__glTracerDispatchTable.Vertex2iv)
    {
        (*__glTracerDispatchTable.Vertex2iv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_Vertex2s(__GLcontext *gc, GLshort x, GLshort y)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertex2s(x=%hd, y=%hd)\n",
                     gc, tid, x, y);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Vertex2s(gc, x, y);
    __GL_PROFILE_FOOTER(enum_glVertex2s);

    if (__glTracerDispatchTable.Vertex2s)
    {
        (*__glTracerDispatchTable.Vertex2s)(x, y);
    }
}

GLvoid GL_APIENTRY __glProfile_Vertex2sv(__GLcontext *gc, const GLshort *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertex2sv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Vertex2sv(gc, v);
    __GL_PROFILE_FOOTER(enum_glVertex2sv);

    if (__glTracerDispatchTable.Vertex2sv)
    {
        (*__glTracerDispatchTable.Vertex2sv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_Vertex3d(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertex3d(x=%lf, y=%lf, z=%lf)\n",
                     gc, tid, x, y, z);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Vertex3d(gc, x, y, z);
    __GL_PROFILE_FOOTER(enum_glVertex3d);

    if (__glTracerDispatchTable.Vertex3d)
    {
        (*__glTracerDispatchTable.Vertex3d)(x, y, z);
    }
}

GLvoid GL_APIENTRY __glProfile_Vertex3dv(__GLcontext *gc, const GLdouble *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertex3dv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Vertex3dv(gc, v);
    __GL_PROFILE_FOOTER(enum_glVertex3dv);

    if (__glTracerDispatchTable.Vertex3dv)
    {
        (*__glTracerDispatchTable.Vertex3dv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_Vertex3f(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertex3f(x=%f, y=%f, z=%f)\n",
                     gc, tid, x, y, z);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Vertex3f(gc, x, y, z);
    __GL_PROFILE_FOOTER(enum_glVertex3f);

    if (__glTracerDispatchTable.Vertex3f)
    {
        (*__glTracerDispatchTable.Vertex3f)(x, y, z);
    }
}

GLvoid GL_APIENTRY __glProfile_Vertex3fv(__GLcontext *gc, const GLfloat *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertex3fv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Vertex3fv(gc, v);
    __GL_PROFILE_FOOTER(enum_glVertex3fv);

    if (__glTracerDispatchTable.Vertex3fv)
    {
        (*__glTracerDispatchTable.Vertex3fv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_Vertex3i(__GLcontext *gc, GLint x, GLint y, GLint z)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertex3i(x=%d, y=%d, z=%d)\n",
                     gc, tid, x, y, z);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Vertex3i(gc, x, y, z);
    __GL_PROFILE_FOOTER(enum_glVertex3i);

    if (__glTracerDispatchTable.Vertex3i)
    {
        (*__glTracerDispatchTable.Vertex3i)(x, y, z);
    }
}

GLvoid GL_APIENTRY __glProfile_Vertex3iv(__GLcontext *gc, const GLint *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertex3iv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Vertex3iv(gc, v);
    __GL_PROFILE_FOOTER(enum_glVertex3iv);

    if (__glTracerDispatchTable.Vertex3iv)
    {
        (*__glTracerDispatchTable.Vertex3iv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_Vertex3s(__GLcontext *gc, GLshort x, GLshort y, GLshort z)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertex3s(x=%hd, y=%hd, z=%hd)\n",
                     gc, tid, x, y, z);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Vertex3s(gc, x, y, z);
    __GL_PROFILE_FOOTER(enum_glVertex3s);

    if (__glTracerDispatchTable.Vertex3s)
    {
        (*__glTracerDispatchTable.Vertex3s)(x, y, z);
    }
}

GLvoid GL_APIENTRY __glProfile_Vertex3sv(__GLcontext *gc, const GLshort *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertex3sv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Vertex3sv(gc, v);
    __GL_PROFILE_FOOTER(enum_glVertex3sv);

    if (__glTracerDispatchTable.Vertex3sv)
    {
        (*__glTracerDispatchTable.Vertex3sv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_Vertex4d(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertex4d(x=%lf, y=%lf, z=%lf, w=%lf)\n",
                     gc, tid, x, y, z, w);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Vertex4d(gc, x, y, z, w);
    __GL_PROFILE_FOOTER(enum_glVertex4d);

    if (__glTracerDispatchTable.Vertex4d)
    {
        (*__glTracerDispatchTable.Vertex4d)(x, y, z, w);
    }
}

GLvoid GL_APIENTRY __glProfile_Vertex4dv(__GLcontext *gc, const GLdouble *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertex4dv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Vertex4dv(gc, v);
    __GL_PROFILE_FOOTER(enum_glVertex4dv);

    if (__glTracerDispatchTable.Vertex4dv)
    {
        (*__glTracerDispatchTable.Vertex4dv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_Vertex4f(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertex4f(x=%f, y=%f, z=%f, w=%f)\n",
                     gc, tid, x, y, z, w);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Vertex4f(gc, x, y, z, w);
    __GL_PROFILE_FOOTER(enum_glVertex4f);

    if (__glTracerDispatchTable.Vertex4f)
    {
        (*__glTracerDispatchTable.Vertex4f)(x, y, z, w);
    }
}

GLvoid GL_APIENTRY __glProfile_Vertex4fv(__GLcontext *gc, const GLfloat *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertex4fv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Vertex4fv(gc, v);
    __GL_PROFILE_FOOTER(enum_glVertex4fv);

    if (__glTracerDispatchTable.Vertex4fv)
    {
        (*__glTracerDispatchTable.Vertex4fv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_Vertex4i(__GLcontext *gc, GLint x, GLint y, GLint z, GLint w)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertex4i(x=%d, y=%d, z=%d, w=%d)\n",
                     gc, tid, x, y, z, w);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Vertex4i(gc, x, y, z, w);
    __GL_PROFILE_FOOTER(enum_glVertex4i);

    if (__glTracerDispatchTable.Vertex4i)
    {
        (*__glTracerDispatchTable.Vertex4i)(x, y, z, w);
    }
}

GLvoid GL_APIENTRY __glProfile_Vertex4iv(__GLcontext *gc, const GLint *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertex4iv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Vertex4iv(gc, v);
    __GL_PROFILE_FOOTER(enum_glVertex4iv);

    if (__glTracerDispatchTable.Vertex4iv)
    {
        (*__glTracerDispatchTable.Vertex4iv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_Vertex4s(__GLcontext *gc, GLshort x, GLshort y, GLshort z, GLshort w)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertex4s(x=%hd, y=%hd, z=%hd, w=%hd)\n",
                     gc, tid, x, y, z, w);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Vertex4s(gc, x, y, z, w);
    __GL_PROFILE_FOOTER(enum_glVertex4s);

    if (__glTracerDispatchTable.Vertex4s)
    {
        (*__glTracerDispatchTable.Vertex4s)(x, y, z, w);
    }
}

GLvoid GL_APIENTRY __glProfile_Vertex4sv(__GLcontext *gc, const GLshort *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertex4sv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Vertex4sv(gc, v);
    __GL_PROFILE_FOOTER(enum_glVertex4sv);

    if (__glTracerDispatchTable.Vertex4sv)
    {
        (*__glTracerDispatchTable.Vertex4sv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_ClipPlane(__GLcontext *gc, GLenum plane, const GLdouble *equation)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glClipPlane(plane=0x%04X, equation=0x%p)\n",
                     gc, tid, plane, equation);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ClipPlane(gc, plane, equation);
    __GL_PROFILE_FOOTER(enum_glClipPlane);

    if (__glTracerDispatchTable.ClipPlane)
    {
        (*__glTracerDispatchTable.ClipPlane)(plane, equation);
    }
}

GLvoid GL_APIENTRY __glProfile_ColorMaterial(__GLcontext *gc, GLenum face, GLenum mode)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColorMaterial(face=0x%04X, mode=0x%04X)\n",
                     gc, tid, face, mode);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ColorMaterial(gc, face, mode);
    __GL_PROFILE_FOOTER(enum_glColorMaterial);

    if (__glTracerDispatchTable.ColorMaterial)
    {
        (*__glTracerDispatchTable.ColorMaterial)(face, mode);
    }
}

GLvoid GL_APIENTRY __glProfile_Fogf(__GLcontext *gc, GLenum pname, GLfloat param)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glFogf(pname=0x%04X, param=%f)\n",
                     gc, tid, pname, param);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Fogf(gc, pname, param);
    __GL_PROFILE_FOOTER(enum_glFogf);

    if (__glTracerDispatchTable.Fogf)
    {
        (*__glTracerDispatchTable.Fogf)(pname, param);
    }
}

GLvoid GL_APIENTRY __glProfile_Fogfv(__GLcontext *gc, GLenum pname, const GLfloat *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glFogfv(pname=0x%04X, params=0x%p)\n",
                     gc, tid, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Fogfv(gc, pname, params);
    __GL_PROFILE_FOOTER(enum_glFogfv);

    if (__glTracerDispatchTable.Fogfv)
    {
        (*__glTracerDispatchTable.Fogfv)(pname, params);
    }
}

GLvoid GL_APIENTRY __glProfile_Fogi(__GLcontext *gc, GLenum pname, GLint param)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glFogi(pname=0x%04X, param=%d)\n",
                     gc, tid, pname, param);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Fogi(gc, pname, param);
    __GL_PROFILE_FOOTER(enum_glFogi);

    if (__glTracerDispatchTable.Fogi)
    {
        (*__glTracerDispatchTable.Fogi)(pname, param);
    }
}

GLvoid GL_APIENTRY __glProfile_Fogiv(__GLcontext *gc, GLenum pname, const GLint *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glFogiv(pname=0x%04X, params=0x%p)\n",
                     gc, tid, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Fogiv(gc, pname, params);
    __GL_PROFILE_FOOTER(enum_glFogiv);

    if (__glTracerDispatchTable.Fogiv)
    {
        (*__glTracerDispatchTable.Fogiv)(pname, params);
    }
}

GLvoid GL_APIENTRY __glProfile_Lightf(__GLcontext *gc, GLenum light, GLenum pname, GLfloat param)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glLightf(light=0x%04X, pname=0x%04X, param=%f)\n",
                     gc, tid, light, pname, param);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Lightf(gc, light, pname, param);
    __GL_PROFILE_FOOTER(enum_glLightf);

    if (__glTracerDispatchTable.Lightf)
    {
        (*__glTracerDispatchTable.Lightf)(light, pname, param);
    }
}

GLvoid GL_APIENTRY __glProfile_Lightfv(__GLcontext *gc, GLenum light, GLenum pname, const GLfloat *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glLightfv(light=0x%04X, pname=0x%04X, params=0x%p)\n",
                     gc, tid, light, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Lightfv(gc, light, pname, params);
    __GL_PROFILE_FOOTER(enum_glLightfv);

    if (__glTracerDispatchTable.Lightfv)
    {
        (*__glTracerDispatchTable.Lightfv)(light, pname, params);
    }
}

GLvoid GL_APIENTRY __glProfile_Lighti(__GLcontext *gc, GLenum light, GLenum pname, GLint param)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glLighti(light=0x%04X, pname=0x%04X, param=%d)\n",
                     gc, tid, light, pname, param);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Lighti(gc, light, pname, param);
    __GL_PROFILE_FOOTER(enum_glLighti);

    if (__glTracerDispatchTable.Lighti)
    {
        (*__glTracerDispatchTable.Lighti)(light, pname, param);
    }
}

GLvoid GL_APIENTRY __glProfile_Lightiv(__GLcontext *gc, GLenum light, GLenum pname, const GLint *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glLightiv(light=0x%04X, pname=0x%04X, params=0x%p)\n",
                     gc, tid, light, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Lightiv(gc, light, pname, params);
    __GL_PROFILE_FOOTER(enum_glLightiv);

    if (__glTracerDispatchTable.Lightiv)
    {
        (*__glTracerDispatchTable.Lightiv)(light, pname, params);
    }
}

GLvoid GL_APIENTRY __glProfile_LightModelf(__GLcontext *gc, GLenum pname, GLfloat param)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glLightModelf(pname=0x%04X, param=%f)\n",
                     gc, tid, pname, param);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->LightModelf(gc, pname, param);
    __GL_PROFILE_FOOTER(enum_glLightModelf);

    if (__glTracerDispatchTable.LightModelf)
    {
        (*__glTracerDispatchTable.LightModelf)(pname, param);
    }
}

GLvoid GL_APIENTRY __glProfile_LightModelfv(__GLcontext *gc, GLenum pname, const GLfloat *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glLightModelfv(pname=0x%04X, params=0x%p)\n",
                     gc, tid, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->LightModelfv(gc, pname, params);
    __GL_PROFILE_FOOTER(enum_glLightModelfv);

    if (__glTracerDispatchTable.LightModelfv)
    {
        (*__glTracerDispatchTable.LightModelfv)(pname, params);
    }
}

GLvoid GL_APIENTRY __glProfile_LightModeli(__GLcontext *gc, GLenum pname, GLint param)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glLightModeli(pname=0x%04X, param=%d)\n",
                     gc, tid, pname, param);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->LightModeli(gc, pname, param);
    __GL_PROFILE_FOOTER(enum_glLightModeli);

    if (__glTracerDispatchTable.LightModeli)
    {
        (*__glTracerDispatchTable.LightModeli)(pname, param);
    }
}

GLvoid GL_APIENTRY __glProfile_LightModeliv(__GLcontext *gc, GLenum pname, const GLint *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glLightModeliv(pname=0x%04X, params=0x%p)\n",
                     gc, tid, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->LightModeliv(gc, pname, params);
    __GL_PROFILE_FOOTER(enum_glLightModeliv);

    if (__glTracerDispatchTable.LightModeliv)
    {
        (*__glTracerDispatchTable.LightModeliv)(pname, params);
    }
}

GLvoid GL_APIENTRY __glProfile_LineStipple(__GLcontext *gc, GLint factor, GLushort pattern)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glLineStipple(factor=%d, pattern=%hu)\n",
                     gc, tid, factor, pattern);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->LineStipple(gc, factor, pattern);
    __GL_PROFILE_FOOTER(enum_glLineStipple);

    if (__glTracerDispatchTable.LineStipple)
    {
        (*__glTracerDispatchTable.LineStipple)(factor, pattern);
    }
}

GLvoid GL_APIENTRY __glProfile_Materialf(__GLcontext *gc, GLenum face, GLenum pname, GLfloat param)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMaterialf(face=0x%04X, pname=0x%04X, param=%f)\n",
                     gc, tid, face, pname, param);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Materialf(gc, face, pname, param);
    __GL_PROFILE_FOOTER(enum_glMaterialf);

    if (__glTracerDispatchTable.Materialf)
    {
        (*__glTracerDispatchTable.Materialf)(face, pname, param);
    }
}

GLvoid GL_APIENTRY __glProfile_Materialfv(__GLcontext *gc, GLenum face, GLenum pname, const GLfloat *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMaterialfv(face=0x%04X, pname=0x%04X, params=0x%p)\n",
                     gc, tid, face, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Materialfv(gc, face, pname, params);
    __GL_PROFILE_FOOTER(enum_glMaterialfv);

    if (__glTracerDispatchTable.Materialfv)
    {
        (*__glTracerDispatchTable.Materialfv)(face, pname, params);
    }
}

GLvoid GL_APIENTRY __glProfile_Materiali(__GLcontext *gc, GLenum face, GLenum pname, GLint param)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMateriali(face=0x%04X, pname=0x%04X, param=%d)\n",
                     gc, tid, face, pname, param);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Materiali(gc, face, pname, param);
    __GL_PROFILE_FOOTER(enum_glMateriali);

    if (__glTracerDispatchTable.Materiali)
    {
        (*__glTracerDispatchTable.Materiali)(face, pname, param);
    }
}

GLvoid GL_APIENTRY __glProfile_Materialiv(__GLcontext *gc, GLenum face, GLenum pname, const GLint *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMaterialiv(face=0x%04X, pname=0x%04X, params=0x%p)\n",
                     gc, tid, face, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Materialiv(gc, face, pname, params);
    __GL_PROFILE_FOOTER(enum_glMaterialiv);

    if (__glTracerDispatchTable.Materialiv)
    {
        (*__glTracerDispatchTable.Materialiv)(face, pname, params);
    }
}

GLvoid GL_APIENTRY __glProfile_PointSize(__GLcontext *gc, GLfloat size)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glPointSize(size=%f)\n",
                     gc, tid, size);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->PointSize(gc, size);
    __GL_PROFILE_FOOTER(enum_glPointSize);

    if (__glTracerDispatchTable.PointSize)
    {
        (*__glTracerDispatchTable.PointSize)(size);
    }
}

GLvoid GL_APIENTRY __glProfile_PolygonMode(__GLcontext *gc, GLenum face, GLenum mode)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glPolygonMode(face=0x%04X, mode=0x%04X)\n",
                     gc, tid, face, mode);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->PolygonMode(gc, face, mode);
    __GL_PROFILE_FOOTER(enum_glPolygonMode);

    if (__glTracerDispatchTable.PolygonMode)
    {
        (*__glTracerDispatchTable.PolygonMode)(face, mode);
    }
}

GLvoid GL_APIENTRY __glProfile_PolygonStipple(__GLcontext *gc, const GLubyte *mask)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glPolygonStipple(mask=0x%p)\n",
                     gc, tid, mask);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->PolygonStipple(gc, mask);
    __GL_PROFILE_FOOTER(enum_glPolygonStipple);

    if (__glTracerDispatchTable.PolygonStipple)
    {
        (*__glTracerDispatchTable.PolygonStipple)(mask);
    }
}

GLvoid GL_APIENTRY __glProfile_ShadeModel(__GLcontext *gc, GLenum mode)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glShadeModel(mode=0x%04X)\n",
                     gc, tid, mode);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ShadeModel(gc, mode);
    __GL_PROFILE_FOOTER(enum_glShadeModel);

    if (__glTracerDispatchTable.ShadeModel)
    {
        (*__glTracerDispatchTable.ShadeModel)(mode);
    }
}

GLvoid GL_APIENTRY __glProfile_TexImage1D(__GLcontext *gc, GLenum target, GLint level, GLint internalFormat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexImage1D(target=0x%04X, level=%d, internalFormat=0x%04X, width=%d, border=%d, format=0x%04X, type=0x%04X, pixels=0x%08X)\n",
                     gc, tid, target, level, internalFormat, width, border, format, type, pixels);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexImage1D(gc, target, level, internalFormat, width, border, format, type, pixels);
    __GL_PROFILE_FOOTER(enum_glTexImage1D);

    if (__glTracerDispatchTable.TexImage1D)
    {
        (*__glTracerDispatchTable.TexImage1D)(target, level, internalFormat, width, border, format, type, pixels);
    }
}

GLvoid GL_APIENTRY __glProfile_TexEnvf(__GLcontext *gc, GLenum target, GLenum pname, GLfloat param)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexEnvf(target=0x%04X, pname=0x%04X, param=%f)\n",
                     gc, tid, target, pname, param);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexEnvf(gc, target, pname, param);
    __GL_PROFILE_FOOTER(enum_glTexEnvf);

    if (__glTracerDispatchTable.TexEnvf)
    {
        (*__glTracerDispatchTable.TexEnvf)(target, pname, param);
    }
}

GLvoid GL_APIENTRY __glProfile_TexEnvfv(__GLcontext *gc, GLenum target, GLenum pname, const GLfloat *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexEnvfv(target=0x%04X, pname=0x%04X, params=0x%p)\n",
                     gc, tid, target, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexEnvfv(gc, target, pname, params);
    __GL_PROFILE_FOOTER(enum_glTexEnvfv);

    if (__glTracerDispatchTable.TexEnvfv)
    {
        (*__glTracerDispatchTable.TexEnvfv)(target, pname, params);
    }
}

GLvoid GL_APIENTRY __glProfile_TexEnvi(__GLcontext *gc, GLenum target, GLenum pname, GLint param)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexEnvi(target=0x%04X, pname=0x%04X, param=%d)\n",
                     gc, tid, target, pname, param);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexEnvi(gc, target, pname, param);
    __GL_PROFILE_FOOTER(enum_glTexEnvi);

    if (__glTracerDispatchTable.TexEnvi)
    {
        (*__glTracerDispatchTable.TexEnvi)(target, pname, param);
    }
}

GLvoid GL_APIENTRY __glProfile_TexEnviv(__GLcontext *gc, GLenum target, GLenum pname, const GLint *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexEnviv(target=0x%04X, pname=0x%04X, params=0x%p)\n",
                     gc, tid, target, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexEnviv(gc, target, pname, params);
    __GL_PROFILE_FOOTER(enum_glTexEnviv);

    if (__glTracerDispatchTable.TexEnviv)
    {
        (*__glTracerDispatchTable.TexEnviv)(target, pname, params);
    }
}

GLvoid GL_APIENTRY __glProfile_TexGend(__GLcontext *gc, GLenum coord, GLenum pname, GLdouble param)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexGend(coord=0x%04X, pname=0x%04X, param=%lf)\n",
                     gc, tid, coord, pname, param);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexGend(gc, coord, pname, param);
    __GL_PROFILE_FOOTER(enum_glTexGend);

    if (__glTracerDispatchTable.TexGend)
    {
        (*__glTracerDispatchTable.TexGend)(coord, pname, param);
    }
}

GLvoid GL_APIENTRY __glProfile_TexGendv(__GLcontext *gc, GLenum coord, GLenum pname, const GLdouble *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexGendv(coord=0x%04X, pname=0x%04X, params=0x%p)\n",
                     gc, tid, coord, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexGendv(gc, coord, pname, params);
    __GL_PROFILE_FOOTER(enum_glTexGendv);

    if (__glTracerDispatchTable.TexGendv)
    {
        (*__glTracerDispatchTable.TexGendv)(coord, pname, params);
    }
}

GLvoid GL_APIENTRY __glProfile_TexGenf(__GLcontext *gc, GLenum coord, GLenum pname, GLfloat param)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexGenf(coord=0x%04X, pname=0x%04X, param=%f)\n",
                     gc, tid, coord, pname, param);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexGenf(gc, coord, pname, param);
    __GL_PROFILE_FOOTER(enum_glTexGenf);

    if (__glTracerDispatchTable.TexGenf)
    {
        (*__glTracerDispatchTable.TexGenf)(coord, pname, param);
    }
}

GLvoid GL_APIENTRY __glProfile_TexGenfv(__GLcontext *gc, GLenum coord, GLenum pname, const GLfloat *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexGenfv(coord=0x%04X, pname=0x%04X, params=0x%p)\n",
                     gc, tid, coord, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexGenfv(gc, coord, pname, params);
    __GL_PROFILE_FOOTER(enum_glTexGenfv);

    if (__glTracerDispatchTable.TexGenfv)
    {
        (*__glTracerDispatchTable.TexGenfv)(coord, pname, params);
    }
}

GLvoid GL_APIENTRY __glProfile_TexGeni(__GLcontext *gc, GLenum coord, GLenum pname, GLint param)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexGeni(coord=0x%04X, pname=0x%04X, param=%d)\n",
                     gc, tid, coord, pname, param);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexGeni(gc, coord, pname, param);
    __GL_PROFILE_FOOTER(enum_glTexGeni);

    if (__glTracerDispatchTable.TexGeni)
    {
        (*__glTracerDispatchTable.TexGeni)(coord, pname, param);
    }
}

GLvoid GL_APIENTRY __glProfile_TexGeniv(__GLcontext *gc, GLenum coord, GLenum pname, const GLint *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexGeniv(coord=0x%04X, pname=0x%04X, params=0x%p)\n",
                     gc, tid, coord, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexGeniv(gc, coord, pname, params);
    __GL_PROFILE_FOOTER(enum_glTexGeniv);

    if (__glTracerDispatchTable.TexGeniv)
    {
        (*__glTracerDispatchTable.TexGeniv)(coord, pname, params);
    }
}

GLvoid GL_APIENTRY __glProfile_FeedbackBuffer(__GLcontext *gc, GLsizei size, GLenum type, GLfloat *buffer)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glFeedbackBuffer(size=0x%08X, type=0x%04X, buffer=0x%p)\n",
                     gc, tid, size, type, buffer);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->FeedbackBuffer(gc, size, type, buffer);
    __GL_PROFILE_FOOTER(enum_glFeedbackBuffer);

    if (__glTracerDispatchTable.FeedbackBuffer)
    {
        (*__glTracerDispatchTable.FeedbackBuffer)(size, type, buffer);
    }
}

GLvoid GL_APIENTRY __glProfile_SelectBuffer(__GLcontext *gc, GLsizei size, GLuint *buffer)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glSelectBuffer(size=0x%08X, buffer=0x%p)\n",
                     gc, tid, size, buffer);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->SelectBuffer(gc, size, buffer);
    __GL_PROFILE_FOOTER(enum_glSelectBuffer);

    if (__glTracerDispatchTable.SelectBuffer)
    {
        (*__glTracerDispatchTable.SelectBuffer)(size, buffer);
    }
}

GLint GL_APIENTRY __glProfile_RenderMode(__GLcontext *gc, GLenum mode)
{
    __GL_PROFILE_VARS();
    GLint ret;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glRenderMode(mode=0x%04X)\n",
                     gc, tid, mode);
    }

    __GL_PROFILE_HEADER();
    ret = gc->pModeDispatch->RenderMode(gc, mode);
    __GL_PROFILE_FOOTER(enum_glRenderMode);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glRenderMode => %d\n", ret);
    }

    if (__glTracerDispatchTable.RenderMode)
    {
        (*__glTracerDispatchTable.RenderMode)(mode);
    }

    return ret;
}

GLvoid GL_APIENTRY __glProfile_InitNames(__GLcontext *gc)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glInitNames()\n",
                     gc, tid);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->InitNames(gc);
    __GL_PROFILE_FOOTER(enum_glInitNames);

    if (__glTracerDispatchTable.InitNames)
    {
        (*__glTracerDispatchTable.InitNames)();
    }
}

GLvoid GL_APIENTRY __glProfile_LoadName(__GLcontext *gc, GLuint name)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glLoadName(name=%u)\n",
                     gc, tid, name);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->LoadName(gc, name);
    __GL_PROFILE_FOOTER(enum_glLoadName);

    if (__glTracerDispatchTable.LoadName)
    {
        (*__glTracerDispatchTable.LoadName)(name);
    }
}

GLvoid GL_APIENTRY __glProfile_PassThrough(__GLcontext *gc, GLfloat token)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glPassThrough(token=%f)\n",
                     gc, tid, token);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->PassThrough(gc, token);
    __GL_PROFILE_FOOTER(enum_glPassThrough);

    if (__glTracerDispatchTable.PassThrough)
    {
        (*__glTracerDispatchTable.PassThrough)(token);
    }
}

GLvoid GL_APIENTRY __glProfile_PopName(__GLcontext *gc)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glPopName()\n",
                     gc, tid);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->PopName(gc);
    __GL_PROFILE_FOOTER(enum_glPopName);

    if (__glTracerDispatchTable.PopName)
    {
        (*__glTracerDispatchTable.PopName)();
    }
}

GLvoid GL_APIENTRY __glProfile_PushName(__GLcontext *gc, GLuint name)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glPushName(name=%u)\n",
                     gc, tid, name);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->PushName(gc, name);
    __GL_PROFILE_FOOTER(enum_glPushName);

    if (__glTracerDispatchTable.PushName)
    {
        (*__glTracerDispatchTable.PushName)(name);
    }
}

GLvoid GL_APIENTRY __glProfile_DrawBuffer(__GLcontext *gc, GLenum mode)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDrawBuffer(mode=0x%04X)\n",
                     gc, tid, mode);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DrawBuffer(gc, mode);
    __GL_PROFILE_FOOTER(enum_glDrawBuffer);

    if (__glTracerDispatchTable.DrawBuffer)
    {
        (*__glTracerDispatchTable.DrawBuffer)(mode);
    }
}

GLvoid GL_APIENTRY __glProfile_ClearAccum(__GLcontext *gc, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glClearAccum(red=%f, green=%f, blue=%f, alpha=%f)\n",
                     gc, tid, red, green, blue, alpha);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ClearAccum(gc, red, green, blue, alpha);
    __GL_PROFILE_FOOTER(enum_glClearAccum);

    if (__glTracerDispatchTable.ClearAccum)
    {
        (*__glTracerDispatchTable.ClearAccum)(red, green, blue, alpha);
    }
}

GLvoid GL_APIENTRY __glProfile_ClearIndex(__GLcontext *gc, GLfloat c)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glClearIndex(c=%f)\n",
                     gc, tid, c);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ClearIndex(gc, c);
    __GL_PROFILE_FOOTER(enum_glClearIndex);

    if (__glTracerDispatchTable.ClearIndex)
    {
        (*__glTracerDispatchTable.ClearIndex)(c);
    }
}

GLvoid GL_APIENTRY __glProfile_ClearDepth(__GLcontext *gc, GLclampd depth)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glClearDepth(depth=%lf)\n",
                     gc, tid, depth);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ClearDepth(gc, depth);
    __GL_PROFILE_FOOTER(enum_glClearDepth);

    if (__glTracerDispatchTable.ClearDepth)
    {
        (*__glTracerDispatchTable.ClearDepth)(depth);
    }
}

GLvoid GL_APIENTRY __glProfile_IndexMask(__GLcontext *gc, GLuint mask)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glIndexMask(mask=%u)\n",
                     gc, tid, mask);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->IndexMask(gc, mask);
    __GL_PROFILE_FOOTER(enum_glIndexMask);

    if (__glTracerDispatchTable.IndexMask)
    {
        (*__glTracerDispatchTable.IndexMask)(mask);
    }
}

GLvoid GL_APIENTRY __glProfile_Accum(__GLcontext *gc, GLenum op, GLfloat value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glAccum(op=0x%04X, value=%f)\n",
                     gc, tid, op, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Accum(gc, op, value);
    __GL_PROFILE_FOOTER(enum_glAccum);

    if (__glTracerDispatchTable.Accum)
    {
        (*__glTracerDispatchTable.Accum)(op, value);
    }
}

GLvoid GL_APIENTRY __glProfile_PopAttrib(__GLcontext *gc)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glPopAttrib()\n",
                     gc, tid);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->PopAttrib(gc);
    __GL_PROFILE_FOOTER(enum_glPopAttrib);

    if (__glTracerDispatchTable.PopAttrib)
    {
        (*__glTracerDispatchTable.PopAttrib)();
    }
}

GLvoid GL_APIENTRY __glProfile_PushAttrib(__GLcontext *gc, GLbitfield mask)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glPushAttrib(mask=0x%08X)\n",
                     gc, tid, mask);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->PushAttrib(gc, mask);
    __GL_PROFILE_FOOTER(enum_glPushAttrib);

    if (__glTracerDispatchTable.PushAttrib)
    {
        (*__glTracerDispatchTable.PushAttrib)(mask);
    }
}

GLvoid GL_APIENTRY __glProfile_Map1d(__GLcontext *gc, GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMap1d(target=0x%04X, u1=%lf, u2=%lf, stride=%d, order=%d, points=0x%p)\n",
                     gc, tid, target, u1, u2, stride, order, points);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Map1d(gc, target, u1, u2, stride, order, points);
    __GL_PROFILE_FOOTER(enum_glMap1d);

    if (__glTracerDispatchTable.Map1d)
    {
        (*__glTracerDispatchTable.Map1d)(target, u1, u2, stride, order, points);
    }
}

GLvoid GL_APIENTRY __glProfile_Map1f(__GLcontext *gc, GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMap1f(target=0x%04X, u1=%f, u2=%f, stride=%d, order=%d, points=0x%p)\n",
                     gc, tid, target, u1, u2, stride, order, points);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Map1f(gc, target, u1, u2, stride, order, points);
    __GL_PROFILE_FOOTER(enum_glMap1f);

    if (__glTracerDispatchTable.Map1f)
    {
        (*__glTracerDispatchTable.Map1f)(target, u1, u2, stride, order, points);
    }
}

GLvoid GL_APIENTRY __glProfile_Map2d(__GLcontext *gc, GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMap2d(target=0x%04X, u1=%lf, u2=%lf, ustride=%d, uorder=%d, v1=%lf, v2=%lf, vstride=%d, vorder=%d, points=0x%p)\n",
                     gc, tid, target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Map2d(gc, target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
    __GL_PROFILE_FOOTER(enum_glMap2d);

    if (__glTracerDispatchTable.Map2d)
    {
        (*__glTracerDispatchTable.Map2d)(target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
    }
}

GLvoid GL_APIENTRY __glProfile_Map2f(__GLcontext *gc, GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMap2f(target=0x%04X, u1=%f, u2=%f, ustride=%d, uorder=%d, v1=%f, v2=%f, vstride=%d, vorder=%d, points=0x%p)\n",
                     gc, tid, target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Map2f(gc, target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
    __GL_PROFILE_FOOTER(enum_glMap2f);

    if (__glTracerDispatchTable.Map2f)
    {
        (*__glTracerDispatchTable.Map2f)(target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
    }
}

GLvoid GL_APIENTRY __glProfile_MapGrid1d(__GLcontext *gc, GLint un, GLdouble u1, GLdouble u2)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMapGrid1d(un=%d, u1=%lf, u2=%lf)\n",
                     gc, tid, un, u1, u2);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MapGrid1d(gc, un, u1, u2);
    __GL_PROFILE_FOOTER(enum_glMapGrid1d);

    if (__glTracerDispatchTable.MapGrid1d)
    {
        (*__glTracerDispatchTable.MapGrid1d)(un, u1, u2);
    }
}

GLvoid GL_APIENTRY __glProfile_MapGrid1f(__GLcontext *gc, GLint un, GLfloat u1, GLfloat u2)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMapGrid1f(un=%d, u1=%f, u2=%f)\n",
                     gc, tid, un, u1, u2);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MapGrid1f(gc, un, u1, u2);
    __GL_PROFILE_FOOTER(enum_glMapGrid1f);

    if (__glTracerDispatchTable.MapGrid1f)
    {
        (*__glTracerDispatchTable.MapGrid1f)(un, u1, u2);
    }
}

GLvoid GL_APIENTRY __glProfile_MapGrid2d(__GLcontext *gc, GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMapGrid2d(un=%d, u1=%lf, u2=%lf, vn=%d, v1=%lf, v2=%lf)\n",
                     gc, tid, un, u1, u2, vn, v1, v2);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MapGrid2d(gc, un, u1, u2, vn, v1, v2);
    __GL_PROFILE_FOOTER(enum_glMapGrid2d);

    if (__glTracerDispatchTable.MapGrid2d)
    {
        (*__glTracerDispatchTable.MapGrid2d)(un, u1, u2, vn, v1, v2);
    }
}

GLvoid GL_APIENTRY __glProfile_MapGrid2f(__GLcontext *gc, GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMapGrid2f(un=%d, u1=%f, u2=%f, vn=%d, v1=%f, v2=%f)\n",
                     gc, tid, un, u1, u2, vn, v1, v2);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MapGrid2f(gc, un, u1, u2, vn, v1, v2);
    __GL_PROFILE_FOOTER(enum_glMapGrid2f);

    if (__glTracerDispatchTable.MapGrid2f)
    {
        (*__glTracerDispatchTable.MapGrid2f)(un, u1, u2, vn, v1, v2);
    }
}

GLvoid GL_APIENTRY __glProfile_EvalCoord1d(__GLcontext *gc, GLdouble u)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glEvalCoord1d(u=%lf)\n",
                     gc, tid, u);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->EvalCoord1d(gc, u);
    __GL_PROFILE_FOOTER(enum_glEvalCoord1d);

    if (__glTracerDispatchTable.EvalCoord1d)
    {
        (*__glTracerDispatchTable.EvalCoord1d)(u);
    }
}

GLvoid GL_APIENTRY __glProfile_EvalCoord1dv(__GLcontext *gc, const GLdouble *u)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glEvalCoord1dv(u=0x%p)\n",
                     gc, tid, u);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->EvalCoord1dv(gc, u);
    __GL_PROFILE_FOOTER(enum_glEvalCoord1dv);

    if (__glTracerDispatchTable.EvalCoord1dv)
    {
        (*__glTracerDispatchTable.EvalCoord1dv)(u);
    }
}

GLvoid GL_APIENTRY __glProfile_EvalCoord1f(__GLcontext *gc, GLfloat u)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glEvalCoord1f(u=%f)\n",
                     gc, tid, u);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->EvalCoord1f(gc, u);
    __GL_PROFILE_FOOTER(enum_glEvalCoord1f);

    if (__glTracerDispatchTable.EvalCoord1f)
    {
        (*__glTracerDispatchTable.EvalCoord1f)(u);
    }
}

GLvoid GL_APIENTRY __glProfile_EvalCoord1fv(__GLcontext *gc, const GLfloat *u)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glEvalCoord1fv(u=0x%p)\n",
                     gc, tid, u);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->EvalCoord1fv(gc, u);
    __GL_PROFILE_FOOTER(enum_glEvalCoord1fv);

    if (__glTracerDispatchTable.EvalCoord1fv)
    {
        (*__glTracerDispatchTable.EvalCoord1fv)(u);
    }
}

GLvoid GL_APIENTRY __glProfile_EvalCoord2d(__GLcontext *gc, GLdouble u, GLdouble v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glEvalCoord2d(u=%lf, v=%lf)\n",
                     gc, tid, u, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->EvalCoord2d(gc, u, v);
    __GL_PROFILE_FOOTER(enum_glEvalCoord2d);

    if (__glTracerDispatchTable.EvalCoord2d)
    {
        (*__glTracerDispatchTable.EvalCoord2d)(u, v);
    }
}

GLvoid GL_APIENTRY __glProfile_EvalCoord2dv(__GLcontext *gc, const GLdouble *u)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glEvalCoord2dv(u=0x%p)\n",
                     gc, tid, u);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->EvalCoord2dv(gc, u);
    __GL_PROFILE_FOOTER(enum_glEvalCoord2dv);

    if (__glTracerDispatchTable.EvalCoord2dv)
    {
        (*__glTracerDispatchTable.EvalCoord2dv)(u);
    }
}

GLvoid GL_APIENTRY __glProfile_EvalCoord2f(__GLcontext *gc, GLfloat u, GLfloat v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glEvalCoord2f(u=%f, v=%f)\n",
                     gc, tid, u, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->EvalCoord2f(gc, u, v);
    __GL_PROFILE_FOOTER(enum_glEvalCoord2f);

    if (__glTracerDispatchTable.EvalCoord2f)
    {
        (*__glTracerDispatchTable.EvalCoord2f)(u, v);
    }
}

GLvoid GL_APIENTRY __glProfile_EvalCoord2fv(__GLcontext *gc, const GLfloat *u)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glEvalCoord2fv(u=0x%p)\n",
                     gc, tid, u);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->EvalCoord2fv(gc, u);
    __GL_PROFILE_FOOTER(enum_glEvalCoord2fv);

    if (__glTracerDispatchTable.EvalCoord2fv)
    {
        (*__glTracerDispatchTable.EvalCoord2fv)(u);
    }
}

GLvoid GL_APIENTRY __glProfile_EvalMesh1(__GLcontext *gc, GLenum mode, GLint i1, GLint i2)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glEvalMesh1(mode=0x%04X, i1=%d, i2=%d)\n",
                     gc, tid, mode, i1, i2);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->EvalMesh1(gc, mode, i1, i2);
    __GL_PROFILE_FOOTER(enum_glEvalMesh1);

    if (__glTracerDispatchTable.EvalMesh1)
    {
        (*__glTracerDispatchTable.EvalMesh1)(mode, i1, i2);
    }
}

GLvoid GL_APIENTRY __glProfile_EvalPoint1(__GLcontext *gc, GLint i)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glEvalPoint1(i=%d)\n",
                     gc, tid, i);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->EvalPoint1(gc, i);
    __GL_PROFILE_FOOTER(enum_glEvalPoint1);

    if (__glTracerDispatchTable.EvalPoint1)
    {
        (*__glTracerDispatchTable.EvalPoint1)(i);
    }
}

GLvoid GL_APIENTRY __glProfile_EvalMesh2(__GLcontext *gc, GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glEvalMesh2(mode=0x%04X, i1=%d, i2=%d, j1=%d, j2=%d)\n",
                     gc, tid, mode, i1, i2, j1, j2);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->EvalMesh2(gc, mode, i1, i2, j1, j2);
    __GL_PROFILE_FOOTER(enum_glEvalMesh2);

    if (__glTracerDispatchTable.EvalMesh2)
    {
        (*__glTracerDispatchTable.EvalMesh2)(mode, i1, i2, j1, j2);
    }
}

GLvoid GL_APIENTRY __glProfile_EvalPoint2(__GLcontext *gc, GLint i, GLint j)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glEvalPoint2(i=%d, j=%d)\n",
                     gc, tid, i, j);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->EvalPoint2(gc, i, j);
    __GL_PROFILE_FOOTER(enum_glEvalPoint2);

    if (__glTracerDispatchTable.EvalPoint2)
    {
        (*__glTracerDispatchTable.EvalPoint2)(i, j);
    }
}

GLvoid GL_APIENTRY __glProfile_AlphaFunc(__GLcontext *gc, GLenum func, GLclampf ref)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glAlphaFunc(func=0x%04X, ref=%f)\n",
                     gc, tid, func, ref);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->AlphaFunc(gc, func, ref);
    __GL_PROFILE_FOOTER(enum_glAlphaFunc);

    if (__glTracerDispatchTable.AlphaFunc)
    {
        (*__glTracerDispatchTable.AlphaFunc)(func, ref);
    }
}

GLvoid GL_APIENTRY __glProfile_LogicOp(__GLcontext *gc, GLenum opcode)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glLogicOp(opcode=0x%04X)\n",
                     gc, tid, opcode);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->LogicOp(gc, opcode);
    __GL_PROFILE_FOOTER(enum_glLogicOp);

    if (__glTracerDispatchTable.LogicOp)
    {
        (*__glTracerDispatchTable.LogicOp)(opcode);
    }
}

GLvoid GL_APIENTRY __glProfile_PixelZoom(__GLcontext *gc, GLfloat xfactor, GLfloat yfactor)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glPixelZoom(xfactor=%f, yfactor=%f)\n",
                     gc, tid, xfactor, yfactor);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->PixelZoom(gc, xfactor, yfactor);
    __GL_PROFILE_FOOTER(enum_glPixelZoom);

    if (__glTracerDispatchTable.PixelZoom)
    {
        (*__glTracerDispatchTable.PixelZoom)(xfactor, yfactor);
    }
}

GLvoid GL_APIENTRY __glProfile_PixelTransferf(__GLcontext *gc, GLenum pname, GLfloat param)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glPixelTransferf(pname=0x%04X, param=%f)\n",
                     gc, tid, pname, param);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->PixelTransferf(gc, pname, param);
    __GL_PROFILE_FOOTER(enum_glPixelTransferf);

    if (__glTracerDispatchTable.PixelTransferf)
    {
        (*__glTracerDispatchTable.PixelTransferf)(pname, param);
    }
}

GLvoid GL_APIENTRY __glProfile_PixelTransferi(__GLcontext *gc, GLenum pname, GLint param)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glPixelTransferi(pname=0x%04X, param=%d)\n",
                     gc, tid, pname, param);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->PixelTransferi(gc, pname, param);
    __GL_PROFILE_FOOTER(enum_glPixelTransferi);

    if (__glTracerDispatchTable.PixelTransferi)
    {
        (*__glTracerDispatchTable.PixelTransferi)(pname, param);
    }
}

GLvoid GL_APIENTRY __glProfile_PixelStoref(__GLcontext *gc, GLenum pname, GLfloat param)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glPixelStoref(pname=0x%04X, param=%f)\n",
                     gc, tid, pname, param);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->PixelStoref(gc, pname, param);
    __GL_PROFILE_FOOTER(enum_glPixelStoref);

    if (__glTracerDispatchTable.PixelStoref)
    {
        (*__glTracerDispatchTable.PixelStoref)(pname, param);
    }
}

GLvoid GL_APIENTRY __glProfile_PixelMapfv(__GLcontext *gc, GLenum map, GLsizei mapsize, const GLfloat *values)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glPixelMapfv(map=0x%04X, mapsize=%d, values=0x%p)\n",
                     gc, tid, map, mapsize, values);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->PixelMapfv(gc, map, mapsize, values);
    __GL_PROFILE_FOOTER(enum_glPixelMapfv);

    if (__glTracerDispatchTable.PixelMapfv)
    {
        (*__glTracerDispatchTable.PixelMapfv)(map, mapsize, values);
    }
}

GLvoid GL_APIENTRY __glProfile_PixelMapuiv(__GLcontext *gc, GLenum map, GLsizei mapsize, const GLuint *values)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glPixelMapuiv(map=0x%04X, mapsize=%d, values=0x%p)\n",
                     gc, tid, map, mapsize, values);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->PixelMapuiv(gc, map, mapsize, values);
    __GL_PROFILE_FOOTER(enum_glPixelMapuiv);

    if (__glTracerDispatchTable.PixelMapuiv)
    {
        (*__glTracerDispatchTable.PixelMapuiv)(map, mapsize, values);
    }
}

GLvoid GL_APIENTRY __glProfile_PixelMapusv(__GLcontext *gc, GLenum map, GLsizei mapsize, const GLushort *values)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glPixelMapusv(map=0x%04X, mapsize=%d values=0x%p)\n",
                     gc, tid, map, mapsize, values);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->PixelMapusv(gc, map, mapsize, values);
    __GL_PROFILE_FOOTER(enum_glPixelMapusv);

    if (__glTracerDispatchTable.PixelMapusv)
    {
        (*__glTracerDispatchTable.PixelMapusv)(map, mapsize, values);
    }
}

GLvoid GL_APIENTRY __glProfile_CopyPixels(__GLcontext *gc, GLint x, GLint y, GLsizei width, GLsizei height, GLenum type)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glCopyPixels(x=%d, y=%d, width=%d, height=%d, type=0x%04X)\n",
                     gc, tid, x, y, width, height, type);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->CopyPixels(gc, x, y, width, height, type);
    __GL_PROFILE_FOOTER(enum_glCopyPixels);

    if (__glTracerDispatchTable.CopyPixels)
    {
        (*__glTracerDispatchTable.CopyPixels)(x, y, width, height, type);
    }
}

GLvoid GL_APIENTRY __glProfile_DrawPixels(__GLcontext *gc, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDrawPixels(width=%d, height=%d, format=0x%04X, type=0x%04X, pixels=0x%p)\n",
                     gc, tid, width, height, format, type, pixels);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DrawPixels(gc, width, height, format, type, pixels);
    __GL_PROFILE_FOOTER(enum_glDrawPixels);

    if (__glTracerDispatchTable.DrawPixels)
    {
        (*__glTracerDispatchTable.DrawPixels)(width, height, format, type, pixels);
    }
}

GLvoid GL_APIENTRY __glProfile_GetClipPlane(__GLcontext *gc, GLenum plane, GLdouble *equation)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetClipPlane(plane=0x%04X, equation=0x%p)\n",
                     gc, tid, plane, equation);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetClipPlane(gc, plane, equation);
    __GL_PROFILE_FOOTER(enum_glGetClipPlane);

    if (__glTracerDispatchTable.GetClipPlane)
    {
        (*__glTracerDispatchTable.GetClipPlane)(plane, equation);
    }
}

GLvoid GL_APIENTRY __glProfile_GetDoublev(__GLcontext *gc, GLenum pname, GLdouble *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetDoublev(pname=0x%04X, params=0x%p)\n",
                     gc, tid, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetDoublev(gc, pname, params);
    __GL_PROFILE_FOOTER(enum_glGetDoublev);

    if (__glTracerDispatchTable.GetDoublev)
    {
        (*__glTracerDispatchTable.GetDoublev)(pname, params);
    }
}

GLvoid GL_APIENTRY __glProfile_GetLightfv(__GLcontext *gc, GLenum light, GLenum pname, GLfloat *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetLightfv(light=0x%04X, pname=0x%04X, params=0x%p)\n",
                     gc, tid, light, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetLightfv(gc, light, pname, params);
    __GL_PROFILE_FOOTER(enum_glGetLightfv);

    if (__glTracerDispatchTable.GetLightfv)
    {
        (*__glTracerDispatchTable.GetLightfv)(light, pname, params);
    }
}

GLvoid GL_APIENTRY __glProfile_GetLightiv(__GLcontext *gc, GLenum light, GLenum pname, GLint *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetLightiv(light=0x%04X, pname=0x%04X, params=0x%p)\n",
                     gc, tid, light, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetLightiv(gc, light, pname, params);
    __GL_PROFILE_FOOTER(enum_glGetLightiv);

    if (__glTracerDispatchTable.GetLightiv)
    {
        (*__glTracerDispatchTable.GetLightiv)(light, pname, params);
    }
}

GLvoid GL_APIENTRY __glProfile_GetMapdv(__GLcontext *gc, GLenum target, GLenum query, GLdouble *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetMapdv(target=0x%04X, query=0x%04X, v=0x%p)\n",
                     gc, tid, target, query, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetMapdv(gc, target, query, v);
    __GL_PROFILE_FOOTER(enum_glGetMapdv);

    if (__glTracerDispatchTable.GetMapdv)
    {
        (*__glTracerDispatchTable.GetMapdv)(target, query, v);
    }
}

GLvoid GL_APIENTRY __glProfile_GetMapfv(__GLcontext *gc, GLenum target, GLenum query, GLfloat *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetMapfv(target=0x%04X, query=0x%04X, v=0x%p)\n",
                     gc, tid, target, query, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetMapfv(gc, target, query, v);
    __GL_PROFILE_FOOTER(enum_glGetMapfv);

    if (__glTracerDispatchTable.GetMapfv)
    {
        (*__glTracerDispatchTable.GetMapfv)(target, query, v);
    }
}

GLvoid GL_APIENTRY __glProfile_GetMapiv(__GLcontext *gc, GLenum target, GLenum query, GLint *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetMapiv(target=0x%04X, query=0x%04X, v=0x%p)\n",
                     gc, tid, target, query, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetMapiv(gc, target, query, v);
    __GL_PROFILE_FOOTER(enum_glGetMapiv);

    if (__glTracerDispatchTable.GetMapiv)
    {
        (*__glTracerDispatchTable.GetMapiv)(target, query, v);
    }
}

GLvoid GL_APIENTRY __glProfile_GetMaterialfv(__GLcontext *gc, GLenum face, GLenum pname, GLfloat *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetMaterialfv(face=0x%04X, pname=0x%04X, params=0x%p)\n",
                     gc, tid, face, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetMaterialfv(gc, face, pname, params);
    __GL_PROFILE_FOOTER(enum_glGetMaterialfv);

    if (__glTracerDispatchTable.GetMaterialfv)
    {
        (*__glTracerDispatchTable.GetMaterialfv)(face, pname, params);
    }
}

GLvoid GL_APIENTRY __glProfile_GetMaterialiv(__GLcontext *gc, GLenum face, GLenum pname, GLint *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetMaterialiv(face=0x%04X, pname=0x%04X, params=0x%p)\n",
                     gc, tid, face, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetMaterialiv(gc, face, pname, params);
    __GL_PROFILE_FOOTER(enum_glGetMaterialiv);

    if (__glTracerDispatchTable.GetMaterialiv)
    {
        (*__glTracerDispatchTable.GetMaterialiv)(face, pname, params);
    }
}

GLvoid GL_APIENTRY __glProfile_GetPixelMapfv(__GLcontext *gc, GLenum map, GLfloat *values)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetPixelMapfv(map=0x%04X, values=0x%p)\n",
                     gc, tid, map, values);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetPixelMapfv(gc, map, values);
    __GL_PROFILE_FOOTER(enum_glGetPixelMapfv);

    if (__glTracerDispatchTable.GetPixelMapfv)
    {
        (*__glTracerDispatchTable.GetPixelMapfv)(map, values);
    }
}

GLvoid GL_APIENTRY __glProfile_GetPixelMapuiv(__GLcontext *gc, GLenum map, GLuint *values)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetPixelMapuiv(map=0x%04X, values=0x%p)\n",
                     gc, tid, map, values);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetPixelMapuiv(gc, map, values);
    __GL_PROFILE_FOOTER(enum_glGetPixelMapuiv);

    if (__glTracerDispatchTable.GetPixelMapuiv)
    {
        (*__glTracerDispatchTable.GetPixelMapuiv)(map, values);
    }
}

GLvoid GL_APIENTRY __glProfile_GetPixelMapusv(__GLcontext *gc, GLenum map, GLushort *values)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetPixelMapusv(map=0x%04X, values=0x%p)\n",
                     gc, tid, map, values);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetPixelMapusv(gc, map, values);
    __GL_PROFILE_FOOTER(enum_glGetPixelMapusv);

    if (__glTracerDispatchTable.GetPixelMapusv)
    {
        (*__glTracerDispatchTable.GetPixelMapusv)(map, values);
    }
}

GLvoid GL_APIENTRY __glProfile_GetPolygonStipple(__GLcontext *gc, GLubyte *mask)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetPolygonStipple(mask=0x%p)\n",
                     gc, tid, mask);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetPolygonStipple(gc, mask);
    __GL_PROFILE_FOOTER(enum_glGetPolygonStipple);

    if (__glTracerDispatchTable.GetPolygonStipple)
    {
        (*__glTracerDispatchTable.GetPolygonStipple)(mask);
    }
}

GLvoid GL_APIENTRY __glProfile_GetTexEnvfv(__GLcontext *gc, GLenum target, GLenum pname, GLfloat *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetTexEnvfv(target=0x%04X, pname=0x%04X, params=0x%p)\n",
                     gc, tid, target, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetTexEnvfv(gc, target, pname, params);
    __GL_PROFILE_FOOTER(enum_glGetTexEnvfv);

    if (__glTracerDispatchTable.GetTexEnvfv)
    {
        (*__glTracerDispatchTable.GetTexEnvfv)(target, pname, params);
    }
}

GLvoid GL_APIENTRY __glProfile_GetTexEnviv(__GLcontext *gc, GLenum target, GLenum pname, GLint *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetTexEnviv(target=0x%04X, pname=0x%04X, params=0x%p)\n",
                     gc, tid, target, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetTexEnviv(gc, target, pname, params);
    __GL_PROFILE_FOOTER(enum_glGetTexEnviv);

    if (__glTracerDispatchTable.GetTexEnviv)
    {
        (*__glTracerDispatchTable.GetTexEnviv)(target, pname, params);
    }
}

GLvoid GL_APIENTRY __glProfile_GetTexGendv(__GLcontext *gc, GLenum coord, GLenum pname, GLdouble *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetTexGendv(coord=0x%04X, pname=0x%04X, params=0x%p)\n",
                     gc, tid, coord, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetTexGendv(gc, coord, pname, params);
    __GL_PROFILE_FOOTER(enum_glGetTexGendv);

    if (__glTracerDispatchTable.GetTexGendv)
    {
        (*__glTracerDispatchTable.GetTexGendv)(coord, pname, params);
    }
}

GLvoid GL_APIENTRY __glProfile_GetTexGenfv(__GLcontext *gc, GLenum coord, GLenum pname, GLfloat *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetTexGenfv(coord=0x%04X, pname=0x%04X, params=0x%p)\n",
                     gc, tid, coord, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetTexGenfv(gc, coord, pname, params);
    __GL_PROFILE_FOOTER(enum_glGetTexGenfv);

    if (__glTracerDispatchTable.GetTexGenfv)
    {
        (*__glTracerDispatchTable.GetTexGenfv)(coord, pname, params);
    }
}

GLvoid GL_APIENTRY __glProfile_GetTexGeniv(__GLcontext *gc, GLenum coord, GLenum pname, GLint *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetTexGeniv(coord=0x%04X, pname=0x%04X, params=0x%p)\n",
                     gc, tid, coord, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetTexGeniv(gc, coord, pname, params);
    __GL_PROFILE_FOOTER(enum_glGetTexGeniv);

    if (__glTracerDispatchTable.GetTexGeniv)
    {
        (*__glTracerDispatchTable.GetTexGeniv)(coord, pname, params);
    }
}

GLboolean GL_APIENTRY __glProfile_IsList(__GLcontext *gc, GLuint list)
{
    __GL_PROFILE_VARS();
    GLboolean ret;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glIsList(list=%u)\n",
                     gc, tid, list);
    }

    __GL_PROFILE_HEADER();
    ret = gc->pModeDispatch->IsList(gc, list);
    __GL_PROFILE_FOOTER(enum_glIsList);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glIsList => %hhu\n", ret);
    }

    if (__glTracerDispatchTable.IsList)
    {
        (*__glTracerDispatchTable.IsList)(list);
    }

    return ret;
}

GLvoid GL_APIENTRY __glProfile_DepthRange(__GLcontext *gc, GLclampd near_val, GLclampd far_val)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDepthRange(near_val=%lf, far_val=%lf)\n",
                     gc, tid, near_val, far_val);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DepthRange(gc, near_val, far_val);
    __GL_PROFILE_FOOTER(enum_glDepthRange);

    if (__glTracerDispatchTable.DepthRange)
    {
        (*__glTracerDispatchTable.DepthRange)(near_val, far_val);
    }
}

GLvoid GL_APIENTRY __glProfile_Frustum(__GLcontext *gc, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glFrustum(left=%lf, right=%lf, bottom=%lf, top=%lf, near_val=%lf, far_val=%lf)\n",
                     gc, tid, left, right, bottom, top, near_val, far_val);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Frustum(gc, left, right, bottom, top, near_val, far_val);
    __GL_PROFILE_FOOTER(enum_glFrustum);

    if (__glTracerDispatchTable.Frustum)
    {
        (*__glTracerDispatchTable.Frustum)(left, right, bottom, top, near_val, far_val);
    }
}

GLvoid GL_APIENTRY __glProfile_LoadIdentity(__GLcontext *gc)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glLoadIdentity()\n",
                     gc, tid);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->LoadIdentity(gc);
    __GL_PROFILE_FOOTER(enum_glLoadIdentity);

    if (__glTracerDispatchTable.LoadIdentity)
    {
        (*__glTracerDispatchTable.LoadIdentity)();
    }
}

GLvoid GL_APIENTRY __glProfile_LoadMatrixf(__GLcontext *gc, const GLfloat *m)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glLoadMatrixf(m=0x%p)\n",
                     gc, tid, m);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->LoadMatrixf(gc, m);
    __GL_PROFILE_FOOTER(enum_glLoadMatrixf);

    if (__glTracerDispatchTable.LoadMatrixf)
    {
        (*__glTracerDispatchTable.LoadMatrixf)(m);
    }
}

GLvoid GL_APIENTRY __glProfile_LoadMatrixd(__GLcontext *gc, const GLdouble *m)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glLoadMatrixd(m=0x%p)\n",
                     gc, tid, m);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->LoadMatrixd(gc, m);
    __GL_PROFILE_FOOTER(enum_glLoadMatrixd);

    if (__glTracerDispatchTable.LoadMatrixd)
    {
        (*__glTracerDispatchTable.LoadMatrixd)(m);
    }
}

GLvoid GL_APIENTRY __glProfile_MatrixMode(__GLcontext *gc, GLenum mode)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMatrixMode(mode=0x%04X)\n",
                     gc, tid, mode);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MatrixMode(gc, mode);
    __GL_PROFILE_FOOTER(enum_glMatrixMode);

    if (__glTracerDispatchTable.MatrixMode)
    {
        (*__glTracerDispatchTable.MatrixMode)(mode);
    }
}

GLvoid GL_APIENTRY __glProfile_MultMatrixf(__GLcontext *gc, const GLfloat *m)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultMatrixf(m=0x%p)\n",
                     gc, tid, m);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultMatrixf(gc, m);
    __GL_PROFILE_FOOTER(enum_glMultMatrixf);

    if (__glTracerDispatchTable.MultMatrixf)
    {
        (*__glTracerDispatchTable.MultMatrixf)(m);
    }
}

GLvoid GL_APIENTRY __glProfile_MultMatrixd(__GLcontext *gc, const GLdouble *m)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultMatrixd(m=0x%p)\n",
                     gc, tid, m);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultMatrixd(gc, m);
    __GL_PROFILE_FOOTER(enum_glMultMatrixd);

    if (__glTracerDispatchTable.MultMatrixd)
    {
        (*__glTracerDispatchTable.MultMatrixd)(m);
    }
}

GLvoid GL_APIENTRY __glProfile_Ortho(__GLcontext *gc, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glOrtho(left=%lf, right=%lf, bottom=%lf, top=%lf, near_val=%lf, far_val=%lf)\n",
                     gc, tid, left, right, bottom, top, near_val, far_val);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Ortho(gc, left, right, bottom, top, near_val, far_val);
    __GL_PROFILE_FOOTER(enum_glOrtho);

    if (__glTracerDispatchTable.Ortho)
    {
        (*__glTracerDispatchTable.Ortho)(left, right, bottom, top, near_val, far_val);
    }
}

GLvoid GL_APIENTRY __glProfile_PopMatrix(__GLcontext *gc)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glPopMatrix()\n",
                     gc, tid);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->PopMatrix(gc);
    __GL_PROFILE_FOOTER(enum_glPopMatrix);

    if (__glTracerDispatchTable.PopMatrix)
    {
        (*__glTracerDispatchTable.PopMatrix)();
    }
}

GLvoid GL_APIENTRY __glProfile_PushMatrix(__GLcontext *gc)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glPushMatrix()\n",
                     gc, tid);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->PushMatrix(gc);
    __GL_PROFILE_FOOTER(enum_glPushMatrix);

    if (__glTracerDispatchTable.PushMatrix)
    {
        (*__glTracerDispatchTable.PushMatrix)();
    }
}

GLvoid GL_APIENTRY __glProfile_Rotated(__GLcontext *gc, GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glRotated(angle=%lf, x=%lf, y=%lf, z=%lf)\n",
                     gc, tid, angle, x, y, z);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Rotated(gc, angle, x, y, z);
    __GL_PROFILE_FOOTER(enum_glRotated);

    if (__glTracerDispatchTable.Rotated)
    {
        (*__glTracerDispatchTable.Rotated)(angle, x, y, z);
    }
}

GLvoid GL_APIENTRY __glProfile_Rotatef(__GLcontext *gc, GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glRotatef(angle=%f, x=%f, y=%f, z=%f)\n",
                     gc, tid, angle, x, y, z);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Rotatef(gc, angle, x, y, z);
    __GL_PROFILE_FOOTER(enum_glRotatef);

    if (__glTracerDispatchTable.Rotatef)
    {
        (*__glTracerDispatchTable.Rotatef)(angle, x, y, z);
    }
}

GLvoid GL_APIENTRY __glProfile_Scaled(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glScaled(x=%lf, y=%lf, z=%lf)\n",
                     gc, tid, x, y, z);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Scaled(gc, x, y, z);
    __GL_PROFILE_FOOTER(enum_glScaled);

    if (__glTracerDispatchTable.Scaled)
    {
        (*__glTracerDispatchTable.Scaled)(x, y, z);
    }
}

GLvoid GL_APIENTRY __glProfile_Scalef(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glScalef(x=%f, y=%f, z=%f)\n",
                     gc, tid, x, y, z);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Scalef(gc, x, y, z);
    __GL_PROFILE_FOOTER(enum_glScalef);

    if (__glTracerDispatchTable.Scalef)
    {
        (*__glTracerDispatchTable.Scalef)(x, y, z);
    }
}

GLvoid GL_APIENTRY __glProfile_Translated(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTranslated(x=%lf, y=%lf, z=%lf)\n",
                     gc, tid, x, y, z);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Translated(gc, x, y, z);
    __GL_PROFILE_FOOTER(enum_glTranslated);

    if (__glTracerDispatchTable.Translated)
    {
        (*__glTracerDispatchTable.Translated)(x, y, z);
    }
}

GLvoid GL_APIENTRY __glProfile_Translatef(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTranslatef(x=%f, y=%f, z=%f)\n",
                     gc, tid, x, y, z);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Translatef(gc, x, y, z);
    __GL_PROFILE_FOOTER(enum_glTranslatef);

    if (__glTracerDispatchTable.Translatef)
    {
        (*__glTracerDispatchTable.Translatef)(x, y, z);
    }
}

/* GL_VERSION_1_1 */

GLvoid GL_APIENTRY __glProfile_ArrayElement(__GLcontext *gc, GLint i)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glArrayElement(i=%d)\n",
                     gc, tid, i);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ArrayElement(gc, i);
    __GL_PROFILE_FOOTER(enum_glArrayElement);

    if (__glTracerDispatchTable.ArrayElement)
    {
        (*__glTracerDispatchTable.ArrayElement)(i);
    }
}

GLvoid GL_APIENTRY __glProfile_ColorPointer(__GLcontext *gc, GLint size, GLenum type, GLsizei stride, const GLvoid *ptr)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColorPointer(size=%d, type=0x%04X, stride=%d, ptr=0x%p)\n",
                     gc, tid, size, type, stride, ptr);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ColorPointer(gc, size, type, stride, ptr);
    __GL_PROFILE_FOOTER(enum_glColorPointer);

    if (__glTracerDispatchTable.ColorPointer)
    {
        (*__glTracerDispatchTable.ColorPointer)(size, type, stride, ptr);
    }
}

GLvoid GL_APIENTRY __glProfile_DisableClientState(__GLcontext *gc, GLenum cap)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDisableClientState(cap=0x%04X)\n",
                     gc, tid, cap);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DisableClientState(gc, cap);
    __GL_PROFILE_FOOTER(enum_glDisableClientState);

    if (__glTracerDispatchTable.DisableClientState)
    {
        (*__glTracerDispatchTable.DisableClientState)(cap);
    }
}

GLvoid GL_APIENTRY __glProfile_EdgeFlagPointer(__GLcontext *gc, GLsizei stride, const GLvoid *ptr)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glEdgeFlagPointer(stride=0x%08X, ptr=0x%p)\n",
                     gc, tid, stride, ptr);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->EdgeFlagPointer(gc, stride, ptr);
    __GL_PROFILE_FOOTER(enum_glEdgeFlagPointer);

    if (__glTracerDispatchTable.EdgeFlagPointer)
    {
        (*__glTracerDispatchTable.EdgeFlagPointer)(stride, ptr);
    }
}

GLvoid GL_APIENTRY __glProfile_EnableClientState(__GLcontext *gc, GLenum cap)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glEnableClientState(cap=0x%04X)\n",
                     gc, tid, cap);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->EnableClientState(gc, cap);
    __GL_PROFILE_FOOTER(enum_glEnableClientState);

    if (__glTracerDispatchTable.EnableClientState)
    {
        (*__glTracerDispatchTable.EnableClientState)(cap);
    }
}

GLvoid GL_APIENTRY __glProfile_IndexPointer(__GLcontext *gc, GLenum type, GLsizei stride, const GLvoid *ptr)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glIndexPointer(type=0x%04X, stride=%d, ptr=0x%p)\n",
                     gc, tid, type, stride, ptr);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->IndexPointer(gc, type, stride, ptr);
    __GL_PROFILE_FOOTER(enum_glIndexPointer);

    if (__glTracerDispatchTable.IndexPointer)
    {
        (*__glTracerDispatchTable.IndexPointer)(type, stride, ptr);
    }
}

GLvoid GL_APIENTRY __glProfile_Indexub(__GLcontext *gc, GLubyte c)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glIndexub(c=%hhu)\n",
                     gc, tid, c);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Indexub(gc, c);
    __GL_PROFILE_FOOTER(enum_glIndexub);

    if (__glTracerDispatchTable.Indexub)
    {
        (*__glTracerDispatchTable.Indexub)(c);
    }
}

GLvoid GL_APIENTRY __glProfile_Indexubv(__GLcontext *gc, const GLubyte *c)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glIndexubv(c=0x%p)\n",
                     gc, tid, c);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Indexubv(gc, c);
    __GL_PROFILE_FOOTER(enum_glIndexubv);

    if (__glTracerDispatchTable.Indexubv)
    {
        (*__glTracerDispatchTable.Indexubv)(c);
    }
}

GLvoid GL_APIENTRY __glProfile_InterleavedArrays(__GLcontext *gc, GLenum format, GLsizei stride, const GLvoid *pointer)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glInterleavedArrays(format=0x%04X, stride=%d, pointer=0x%p)\n",
                     gc, tid, format, stride, pointer);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->InterleavedArrays(gc, format, stride, pointer);
    __GL_PROFILE_FOOTER(enum_glInterleavedArrays);

    if (__glTracerDispatchTable.InterleavedArrays)
    {
        (*__glTracerDispatchTable.InterleavedArrays)(format, stride, pointer);
    }
}

GLvoid GL_APIENTRY __glProfile_NormalPointer(__GLcontext *gc, GLenum type, GLsizei stride, const GLvoid *ptr)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glNormalPointer(type=0x%04X, stride=%d, ptr=0x%p)\n",
                     gc, tid, type, stride, ptr);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->NormalPointer(gc, type, stride, ptr);
    __GL_PROFILE_FOOTER(enum_glNormalPointer);

    if (__glTracerDispatchTable.NormalPointer)
    {
        (*__glTracerDispatchTable.NormalPointer)(type, stride, ptr);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoordPointer(__GLcontext *gc, GLint size, GLenum type, GLsizei stride, const GLvoid *ptr)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoordPointer(size=%d, type=0x%04X, stride=%d, ptr=0x%p)\n",
                     gc, tid, size, type, stride, ptr);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoordPointer(gc, size, type, stride, ptr);
    __GL_PROFILE_FOOTER(enum_glTexCoordPointer);

    if (__glTracerDispatchTable.TexCoordPointer)
    {
        (*__glTracerDispatchTable.TexCoordPointer)(size, type, stride, ptr);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexPointer(__GLcontext *gc, GLint size, GLenum type, GLsizei stride, const GLvoid *ptr)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexPointer(size=%d, type=0x%04X, stride=%d, ptr=0x%p)\n",
                     gc, tid, size, type, stride, ptr);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexPointer(gc, size, type, stride, ptr);
    __GL_PROFILE_FOOTER(enum_glVertexPointer);

    if (__glTracerDispatchTable.VertexPointer)
    {
        (*__glTracerDispatchTable.VertexPointer)(size, type, stride, ptr);
    }
}

GLboolean GL_APIENTRY __glProfile_AreTexturesResident(__GLcontext *gc, GLsizei n, const GLuint *textures, GLboolean *residences)
{
    __GL_PROFILE_VARS();
    GLboolean ret;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glAreTexturesResident(n=0x%08X, textures=0x%p, residences=0x%p)\n",
                     gc, tid, n, textures, residences);
    }

    __GL_PROFILE_HEADER();
    ret = gc->pModeDispatch->AreTexturesResident(gc, n, textures, residences);
    __GL_PROFILE_FOOTER(enum_glAreTexturesResident);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glAreTexturesResident => %hhu\n", ret);
    }

    if (__glTracerDispatchTable.AreTexturesResident)
    {
        (*__glTracerDispatchTable.AreTexturesResident)(n, textures, residences);
    }

    return ret;
}

GLvoid GL_APIENTRY __glProfile_CopyTexImage1D(__GLcontext *gc, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glCopyTexImage1D(target=0x%04X, level=%d, internalformat=0x%04X, x=%d, y=%d, width=%d, border=%d)\n",
                     gc, tid, target, level, internalformat, x, y, width, border);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->CopyTexImage1D(gc, target, level, internalformat, x, y, width, border);
    __GL_PROFILE_FOOTER(enum_glCopyTexImage1D);

    if (__glTracerDispatchTable.CopyTexImage1D)
    {
        (*__glTracerDispatchTable.CopyTexImage1D)(target, level, internalformat, x, y, width, border);
    }
}

GLvoid GL_APIENTRY __glProfile_CopyTexSubImage1D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glCopyTexSubImage1D(target=0x%04X, level=%d, xoffset=%d, x=%d, y=%d, width=%d)\n",
                     gc, tid, target, level, xoffset, x, y, width);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->CopyTexSubImage1D(gc, target, level, xoffset, x, y, width);
    __GL_PROFILE_FOOTER(enum_glCopyTexSubImage1D);

    if (__glTracerDispatchTable.CopyTexSubImage1D)
    {
        (*__glTracerDispatchTable.CopyTexSubImage1D)(target, level, xoffset, x, y, width);
    }
}

GLvoid GL_APIENTRY __glProfile_PrioritizeTextures(__GLcontext *gc, GLsizei n, const GLuint *textures, const GLclampf *priorities)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glPrioritizeTextures(n=0x%08X, textures=0x%p, priorities=0x%p)\n",
                     gc, tid, n, textures, priorities);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->PrioritizeTextures(gc, n, textures, priorities);
    __GL_PROFILE_FOOTER(enum_glPrioritizeTextures);

    if (__glTracerDispatchTable.PrioritizeTextures)
    {
        (*__glTracerDispatchTable.PrioritizeTextures)(n, textures, priorities);
    }
}

GLvoid GL_APIENTRY __glProfile_TexSubImage1D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexSubImage1D(target=0x%04X, level=%d, xoffset=%d, width=%d, format=0x%04X, type=0x%04X, pixels=0x%p)\n",
                     gc, tid, target, level, xoffset, width, format, type, pixels);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexSubImage1D(gc, target, level, xoffset, width, format, type, pixels);
    __GL_PROFILE_FOOTER(enum_glTexSubImage1D);

    if (__glTracerDispatchTable.TexSubImage1D)
    {
        (*__glTracerDispatchTable.TexSubImage1D)(target, level, xoffset, width, format, type, pixels);
    }
}

GLvoid GL_APIENTRY __glProfile_PopClientAttrib(__GLcontext *gc)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glPopClientAttrib()\n",
                     gc, tid);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->PopClientAttrib(gc);
    __GL_PROFILE_FOOTER(enum_glPopClientAttrib);

    if (__glTracerDispatchTable.PopClientAttrib)
    {
        (*__glTracerDispatchTable.PopClientAttrib)();
    }
}

GLvoid GL_APIENTRY __glProfile_PushClientAttrib(__GLcontext *gc, GLbitfield mask)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glPushClientAttrib(mask=0x%08X)\n",
                     gc, tid, mask);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->PushClientAttrib(gc, mask);
    __GL_PROFILE_FOOTER(enum_glPushClientAttrib);

    if (__glTracerDispatchTable.PushClientAttrib)
    {
        (*__glTracerDispatchTable.PushClientAttrib)(mask);
    }
}

/* GL_VERSION_1_2 */

/* GL_VERSION_1_3 */

GLvoid GL_APIENTRY __glProfile_CompressedTexImage1D(__GLcontext *gc, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glCompressedTexImage1D(target=0x%04X, level=%d, internalformat=0x%04X, width=%d, border=%d, imageSize=%d, data=0x%p)\n",
                     gc, tid, target, level, internalformat, width, border, imageSize, data);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->CompressedTexImage1D(gc, target, level, internalformat, width, border, imageSize, data);
    __GL_PROFILE_FOOTER(enum_glCompressedTexImage1D);

    if (__glTracerDispatchTable.CompressedTexImage1D)
    {
        (*__glTracerDispatchTable.CompressedTexImage1D)(target, level, internalformat, width, border, imageSize, data);
    }
}

GLvoid GL_APIENTRY __glProfile_CompressedTexSubImage1D(__GLcontext *gc, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glCompressedTexSubImage1D(target=0x%04X, level=%d, xoffset=%d, width=%d, format=0x%04X, imageSize=%d, data=0x%p)\n",
                     gc, tid, target, level, xoffset, width, format, imageSize, data);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->CompressedTexSubImage1D(gc, target, level, xoffset, width, format, imageSize, data);
    __GL_PROFILE_FOOTER(enum_glCompressedTexSubImage1D);

    if (__glTracerDispatchTable.CompressedTexSubImage1D)
    {
        (*__glTracerDispatchTable.CompressedTexSubImage1D)(target, level, xoffset, width, format, imageSize, data);
    }
}

GLvoid GL_APIENTRY __glProfile_GetCompressedTexImage(__GLcontext *gc, GLenum target, GLint lod, GLvoid *img)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetCompressedTexImage(target=0x%04X, lod=%d, img=0x%p)\n",
                     gc, tid, target, lod, img);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetCompressedTexImage(gc, target, lod, img);
    __GL_PROFILE_FOOTER(enum_glGetCompressedTexImage);

    if (__glTracerDispatchTable.GetCompressedTexImage)
    {
        (*__glTracerDispatchTable.GetCompressedTexImage)(target, lod, img);
    }
}

GLvoid GL_APIENTRY __glProfile_ClientActiveTexture(__GLcontext *gc, GLenum texture)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glClientActiveTexture(texture=0x%04X)\n",
                     gc, tid, texture);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ClientActiveTexture(gc, texture);
    __GL_PROFILE_FOOTER(enum_glClientActiveTexture);

    if (__glTracerDispatchTable.ClientActiveTexture)
    {
        (*__glTracerDispatchTable.ClientActiveTexture)(texture);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoord1d(__GLcontext *gc, GLenum target, GLdouble s)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoord1d(target=0x%04X, s=%lf)\n",
                     gc, tid, target, s);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoord1d(gc, target, s);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoord1d);

    if (__glTracerDispatchTable.MultiTexCoord1d)
    {
        (*__glTracerDispatchTable.MultiTexCoord1d)(target, s);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoord1dv(__GLcontext *gc, GLenum target, const GLdouble *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoord1dv(target=0x%04X, v=0x%p)\n",
                     gc, tid, target, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoord1dv(gc, target, v);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoord1dv);

    if (__glTracerDispatchTable.MultiTexCoord1dv)
    {
        (*__glTracerDispatchTable.MultiTexCoord1dv)(target, v);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoord1f(__GLcontext *gc, GLenum target, GLfloat s)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoord1f(target=0x%04X, s=%f)\n",
                     gc, tid, target, s);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoord1f(gc, target, s);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoord1f);

    if (__glTracerDispatchTable.MultiTexCoord1f)
    {
        (*__glTracerDispatchTable.MultiTexCoord1f)(target, s);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoord1fv(__GLcontext *gc, GLenum target, const GLfloat *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoord1fv(target=0x%04X, v=0x%p)\n",
                     gc, tid, target, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoord1fv(gc, target, v);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoord1fv);

    if (__glTracerDispatchTable.MultiTexCoord1fv)
    {
        (*__glTracerDispatchTable.MultiTexCoord1fv)(target, v);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoord1i(__GLcontext *gc, GLenum target, GLint s)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoord1i(target=0x%04X, s=%d)\n",
                     gc, tid, target, s);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoord1i(gc, target, s);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoord1i);

    if (__glTracerDispatchTable.MultiTexCoord1i)
    {
        (*__glTracerDispatchTable.MultiTexCoord1i)(target, s);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoord1iv(__GLcontext *gc, GLenum target, const GLint *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoord1iv(target=0x%04X, v=0x%p)\n",
                     gc, tid, target, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoord1iv(gc, target, v);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoord1iv);

    if (__glTracerDispatchTable.MultiTexCoord1iv)
    {
        (*__glTracerDispatchTable.MultiTexCoord1iv)(target, v);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoord1s(__GLcontext *gc, GLenum target, GLshort s)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoord1s(target=0x%04X, s=%hd)\n",
                     gc, tid, target, s);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoord1s(gc, target, s);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoord1s);

    if (__glTracerDispatchTable.MultiTexCoord1s)
    {
        (*__glTracerDispatchTable.MultiTexCoord1s)(target, s);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoord1sv(__GLcontext *gc, GLenum target, const GLshort *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoord1sv(target=0x%04X, v=0x%p)\n",
                     gc, tid, target, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoord1sv(gc, target, v);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoord1sv);

    if (__glTracerDispatchTable.MultiTexCoord1sv)
    {
        (*__glTracerDispatchTable.MultiTexCoord1sv)(target, v);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoord2d(__GLcontext *gc, GLenum target, GLdouble s, GLdouble t)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoord2d(target=0x%04X, s=%lf, t=%lf)\n",
                     gc, tid, target, s, t);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoord2d(gc, target, s, t);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoord2d);

    if (__glTracerDispatchTable.MultiTexCoord2d)
    {
        (*__glTracerDispatchTable.MultiTexCoord2d)(target, s, t);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoord2dv(__GLcontext *gc, GLenum target, const GLdouble *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoord2dv(target=0x%04X, v=0x%p)\n",
                     gc, tid, target, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoord2dv(gc, target, v);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoord2dv);

    if (__glTracerDispatchTable.MultiTexCoord2dv)
    {
        (*__glTracerDispatchTable.MultiTexCoord2dv)(target, v);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoord2f(__GLcontext *gc, GLenum target, GLfloat s, GLfloat t)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoord2f(target=0x%04X, s=%f, t=%f)\n",
                     gc, tid, target, s, t);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoord2f(gc, target, s, t);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoord2f);

    if (__glTracerDispatchTable.MultiTexCoord2f)
    {
        (*__glTracerDispatchTable.MultiTexCoord2f)(target, s, t);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoord2fv(__GLcontext *gc, GLenum target, const GLfloat *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoord2fv(target=0x%04X, v=0x%p)\n",
                     gc, tid, target, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoord2fv(gc, target, v);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoord2fv);

    if (__glTracerDispatchTable.MultiTexCoord2fv)
    {
        (*__glTracerDispatchTable.MultiTexCoord2fv)(target, v);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoord2i(__GLcontext *gc, GLenum target, GLint s, GLint t)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoord2i(target=0x%04X, s=%d, t=%d)\n",
                     gc, tid, target, s, t);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoord2i(gc, target, s, t);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoord2i);

    if (__glTracerDispatchTable.MultiTexCoord2i)
    {
        (*__glTracerDispatchTable.MultiTexCoord2i)(target, s, t);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoord2iv(__GLcontext *gc, GLenum target, const GLint *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoord2iv(target=0x%04X, v=0x%p)\n",
                     gc, tid, target, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoord2iv(gc, target, v);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoord2iv);

    if (__glTracerDispatchTable.MultiTexCoord2iv)
    {
        (*__glTracerDispatchTable.MultiTexCoord2iv)(target, v);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoord2s(__GLcontext *gc, GLenum target, GLshort s, GLshort t)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoord2s(target=0x%04X, s=%hd, t=%hd)\n",
                     gc, tid, target, s, t);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoord2s(gc, target, s, t);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoord2s);

    if (__glTracerDispatchTable.MultiTexCoord2s)
    {
        (*__glTracerDispatchTable.MultiTexCoord2s)(target, s, t);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoord2sv(__GLcontext *gc, GLenum target, const GLshort *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoord2sv(target=0x%04X, v=0x%p)\n",
                     gc, tid, target, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoord2sv(gc, target, v);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoord2sv);

    if (__glTracerDispatchTable.MultiTexCoord2sv)
    {
        (*__glTracerDispatchTable.MultiTexCoord2sv)(target, v);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoord3d(__GLcontext *gc, GLenum target, GLdouble s, GLdouble t, GLdouble r)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoord3d(target=0x%04X, s=%lf, t=%lf, r=%lf)\n",
                     gc, tid, target, s, t, r);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoord3d(gc, target, s, t, r);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoord3d);

    if (__glTracerDispatchTable.MultiTexCoord3d)
    {
        (*__glTracerDispatchTable.MultiTexCoord3d)(target, s, t, r);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoord3dv(__GLcontext *gc, GLenum target, const GLdouble *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoord3dv(target=0x%04X, v=0x%p)\n",
                     gc, tid, target, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoord3dv(gc, target, v);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoord3dv);

    if (__glTracerDispatchTable.MultiTexCoord3dv)
    {
        (*__glTracerDispatchTable.MultiTexCoord3dv)(target, v);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoord3f(__GLcontext *gc, GLenum target, GLfloat s, GLfloat t, GLfloat r)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoord3f(target=0x%04X, s=%f, t=%f, r=%f)\n",
                     gc, tid, target, s, t, r);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoord3f(gc, target, s, t, r);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoord3f);

    if (__glTracerDispatchTable.MultiTexCoord3f)
    {
        (*__glTracerDispatchTable.MultiTexCoord3f)(target, s, t, r);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoord3fv(__GLcontext *gc, GLenum target, const GLfloat *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoord3fv(target=0x%04X, v=0x%p)\n",
                     gc, tid, target, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoord3fv(gc, target, v);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoord3fv);

    if (__glTracerDispatchTable.MultiTexCoord3fv)
    {
        (*__glTracerDispatchTable.MultiTexCoord3fv)(target, v);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoord3i(__GLcontext *gc, GLenum target, GLint s, GLint t, GLint r)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoord3i(target=0x%04X, s=%d, t=%d, r=%d)\n",
                     gc, tid, target, s, t, r);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoord3i(gc, target, s, t, r);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoord3i);

    if (__glTracerDispatchTable.MultiTexCoord3i)
    {
        (*__glTracerDispatchTable.MultiTexCoord3i)(target, s, t, r);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoord3iv(__GLcontext *gc, GLenum target, const GLint *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoord3iv(target=0x%04X, v=0x%p)\n",
                     gc, tid, target, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoord3iv(gc, target, v);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoord3iv);

    if (__glTracerDispatchTable.MultiTexCoord3iv)
    {
        (*__glTracerDispatchTable.MultiTexCoord3iv)(target, v);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoord3s(__GLcontext *gc, GLenum target, GLshort s, GLshort t, GLshort r)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoord3s(target=0x%04X, s=%hd, t=%hd, r=%hd)\n",
                     gc, tid, target, s, t, r);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoord3s(gc, target, s, t, r);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoord3s);

    if (__glTracerDispatchTable.MultiTexCoord3s)
    {
        (*__glTracerDispatchTable.MultiTexCoord3s)(target, s, t, r);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoord3sv(__GLcontext *gc, GLenum target, const GLshort *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoord3sv(target=0x%04X, v=0x%p)\n",
                     gc, tid, target, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoord3sv(gc, target, v);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoord3sv);

    if (__glTracerDispatchTable.MultiTexCoord3sv)
    {
        (*__glTracerDispatchTable.MultiTexCoord3sv)(target, v);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoord4d(__GLcontext *gc, GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoord4d(target=0x%04X, s=%lf, t=%lf, r=%lf, q=%lf)\n",
                     gc, tid, target, s, t, r, q);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoord4d(gc, target, s, t, r, q);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoord4d);

    if (__glTracerDispatchTable.MultiTexCoord4d)
    {
        (*__glTracerDispatchTable.MultiTexCoord4d)(target, s, t, r, q);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoord4dv(__GLcontext *gc, GLenum target, const GLdouble *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoord4dv(target=0x%04X, v=0x%p)\n",
                     gc, tid, target, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoord4dv(gc, target, v);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoord4dv);

    if (__glTracerDispatchTable.MultiTexCoord4dv)
    {
        (*__glTracerDispatchTable.MultiTexCoord4dv)(target, v);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoord4f(__GLcontext *gc, GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoord4f(target=0x%04X, s=%f, t=%f, r=%f, q=%f)\n",
                     gc, tid, target, s, t, r, q);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoord4f(gc, target, s, t, r, q);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoord4f);

    if (__glTracerDispatchTable.MultiTexCoord4f)
    {
        (*__glTracerDispatchTable.MultiTexCoord4f)(target, s, t, r, q);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoord4fv(__GLcontext *gc, GLenum target, const GLfloat *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoord4fv(target=0x%04X, v=0x%p)\n",
                     gc, tid, target, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoord4fv(gc, target, v);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoord4fv);

    if (__glTracerDispatchTable.MultiTexCoord4fv)
    {
        (*__glTracerDispatchTable.MultiTexCoord4fv)(target, v);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoord4i(__GLcontext *gc, GLenum target, GLint s, GLint t, GLint r, GLint q)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoord4i(target=0x%04X, s=%d, t=%d, r=%d, q=%d)\n",
                     gc, tid, target, s, t, r, q);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoord4i(gc, target, s, t, r, q);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoord4i);

    if (__glTracerDispatchTable.MultiTexCoord4i)
    {
        (*__glTracerDispatchTable.MultiTexCoord4i)(target, s, t, r, q);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoord4iv(__GLcontext *gc, GLenum target, const GLint *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoord4iv(target=0x%04X, v=0x%p)\n",
                     gc, tid, target, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoord4iv(gc, target, v);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoord4iv);

    if (__glTracerDispatchTable.MultiTexCoord4iv)
    {
        (*__glTracerDispatchTable.MultiTexCoord4iv)(target, v);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoord4s(__GLcontext *gc, GLenum target, GLshort s, GLshort t, GLshort r, GLshort q)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoord4s(target=0x%04X, s=%hd, t=%hd, r=%hd, q=%hd)\n",
                     gc, tid, target, s, t, r, q);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoord4s(gc, target, s, t, r, q);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoord4s);

    if (__glTracerDispatchTable.MultiTexCoord4s)
    {
        (*__glTracerDispatchTable.MultiTexCoord4s)(target, s, t, r, q);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoord4sv(__GLcontext *gc, GLenum target, const GLshort *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoord4sv(target=0x%04X, v=0x%p)\n",
                     gc, tid, target, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoord4sv(gc, target, v);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoord4sv);

    if (__glTracerDispatchTable.MultiTexCoord4sv)
    {
        (*__glTracerDispatchTable.MultiTexCoord4sv)(target, v);
    }
}

GLvoid GL_APIENTRY __glProfile_LoadTransposeMatrixf(__GLcontext *gc, const GLfloat m[16])
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glLoadTransposeMatrixf(m=%f)\n",
                     gc, tid, m);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->LoadTransposeMatrixf(gc, m);
    __GL_PROFILE_FOOTER(enum_glLoadTransposeMatrixf);

    if (__glTracerDispatchTable.LoadTransposeMatrixf)
    {
        (*__glTracerDispatchTable.LoadTransposeMatrixf)(m);
    }
}

GLvoid GL_APIENTRY __glProfile_LoadTransposeMatrixd(__GLcontext *gc, const GLdouble m[16])
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glLoadTransposeMatrixd(m=%lf)\n",
                     gc, tid, m);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->LoadTransposeMatrixd(gc, m);
    __GL_PROFILE_FOOTER(enum_glLoadTransposeMatrixd);

    if (__glTracerDispatchTable.LoadTransposeMatrixd)
    {
        (*__glTracerDispatchTable.LoadTransposeMatrixd)(m);
    }
}

GLvoid GL_APIENTRY __glProfile_MultTransposeMatrixf(__GLcontext *gc, const GLfloat m[16])
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultTransposeMatrixf(m=%f)\n",
                     gc, tid, m);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultTransposeMatrixf(gc, m);
    __GL_PROFILE_FOOTER(enum_glMultTransposeMatrixf);

    if (__glTracerDispatchTable.MultTransposeMatrixf)
    {
        (*__glTracerDispatchTable.MultTransposeMatrixf)(m);
    }
}

GLvoid GL_APIENTRY __glProfile_MultTransposeMatrixd(__GLcontext *gc, const GLdouble m[16])
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultTransposeMatrixd(m=%lf)\n",
                     gc, tid, m);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultTransposeMatrixd(gc, m);
    __GL_PROFILE_FOOTER(enum_glMultTransposeMatrixd);

    if (__glTracerDispatchTable.MultTransposeMatrixd)
    {
        (*__glTracerDispatchTable.MultTransposeMatrixd)(m);
    }
}

/* GL_VERSION_1_4 */

GLvoid GL_APIENTRY __glProfile_PointParameterf(__GLcontext *gc, GLenum pname, GLfloat param)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glPointParameterf(pname=0x%04X, param=%f)\n",
                     gc, tid, pname, param);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->PointParameterf(gc, pname, param);
    __GL_PROFILE_FOOTER(enum_glPointParameterf);

    if (__glTracerDispatchTable.PointParameterf)
    {
        (*__glTracerDispatchTable.PointParameterf)(pname, param);
    }
}

GLvoid GL_APIENTRY __glProfile_PointParameterfv(__GLcontext *gc, GLenum pname, const GLfloat *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glPointParameterfv(pname=0x%04X, params=0x%p)\n",
                     gc, tid, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->PointParameterfv(gc, pname, params);
    __GL_PROFILE_FOOTER(enum_glPointParameterfv);

    if (__glTracerDispatchTable.PointParameterfv)
    {
        (*__glTracerDispatchTable.PointParameterfv)(pname, params);
    }
}

GLvoid GL_APIENTRY __glProfile_PointParameteri(__GLcontext *gc, GLenum pname, GLint param)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glPointParameteri(pname=0x%04X, param=%d)\n",
                     gc, tid, pname, param);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->PointParameteri(gc, pname, param);
    __GL_PROFILE_FOOTER(enum_glPointParameteri);

    if (__glTracerDispatchTable.PointParameteri)
    {
        (*__glTracerDispatchTable.PointParameteri)(pname, param);
    }
}

GLvoid GL_APIENTRY __glProfile_PointParameteriv(__GLcontext *gc, GLenum pname, const GLint *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glPointParameteriv(pname=0x%04X, params=0x%p)\n",
                     gc, tid, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->PointParameteriv(gc, pname, params);
    __GL_PROFILE_FOOTER(enum_glPointParameteriv);

    if (__glTracerDispatchTable.PointParameteriv)
    {
        (*__glTracerDispatchTable.PointParameteriv)(pname, params);
    }
}

GLvoid GL_APIENTRY __glProfile_FogCoordf(__GLcontext *gc, GLfloat coord)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glFogCoordf(coord=%f)\n",
                     gc, tid, coord);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->FogCoordf(gc, coord);
    __GL_PROFILE_FOOTER(enum_glFogCoordf);

    if (__glTracerDispatchTable.FogCoordf)
    {
        (*__glTracerDispatchTable.FogCoordf)(coord);
    }
}

GLvoid GL_APIENTRY __glProfile_FogCoordfv(__GLcontext *gc, const GLfloat *coord)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glFogCoordfv(coord=0x%p)\n",
                     gc, tid, coord);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->FogCoordfv(gc, coord);
    __GL_PROFILE_FOOTER(enum_glFogCoordfv);

    if (__glTracerDispatchTable.FogCoordfv)
    {
        (*__glTracerDispatchTable.FogCoordfv)(coord);
    }
}

GLvoid GL_APIENTRY __glProfile_FogCoordd(__GLcontext *gc, GLdouble coord)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glFogCoordd(coord=%lf)\n",
                     gc, tid, coord);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->FogCoordd(gc, coord);
    __GL_PROFILE_FOOTER(enum_glFogCoordd);

    if (__glTracerDispatchTable.FogCoordd)
    {
        (*__glTracerDispatchTable.FogCoordd)(coord);
    }
}

GLvoid GL_APIENTRY __glProfile_FogCoorddv(__GLcontext *gc, const GLdouble *coord)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glFogCoorddv(coord=0x%p)\n",
                     gc, tid, coord);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->FogCoorddv(gc, coord);
    __GL_PROFILE_FOOTER(enum_glFogCoorddv);

    if (__glTracerDispatchTable.FogCoorddv)
    {
        (*__glTracerDispatchTable.FogCoorddv)(coord);
    }
}

GLvoid GL_APIENTRY __glProfile_FogCoordPointer(__GLcontext *gc, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glFogCoordPointer(type=0x%04X, stride=%d, pointer=0x%p)\n",
                     gc, tid, type, stride, pointer);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->FogCoordPointer(gc, type, stride, pointer);
    __GL_PROFILE_FOOTER(enum_glFogCoordPointer);

    if (__glTracerDispatchTable.FogCoordPointer)
    {
        (*__glTracerDispatchTable.FogCoordPointer)(type, stride, pointer);
    }
}

GLvoid GL_APIENTRY __glProfile_SecondaryColor3b(__GLcontext *gc, GLbyte red, GLbyte green, GLbyte blue)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glSecondaryColor3b(red=%hhd, green=%hhd, blue=%hhd)\n",
                     gc, tid, red, green, blue);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->SecondaryColor3b(gc, red, green, blue);
    __GL_PROFILE_FOOTER(enum_glSecondaryColor3b);

    if (__glTracerDispatchTable.SecondaryColor3b)
    {
        (*__glTracerDispatchTable.SecondaryColor3b)(red, green, blue);
    }
}

GLvoid GL_APIENTRY __glProfile_SecondaryColor3bv(__GLcontext *gc, const GLbyte *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glSecondaryColor3bv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->SecondaryColor3bv(gc, v);
    __GL_PROFILE_FOOTER(enum_glSecondaryColor3bv);

    if (__glTracerDispatchTable.SecondaryColor3bv)
    {
        (*__glTracerDispatchTable.SecondaryColor3bv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_SecondaryColor3d(__GLcontext *gc, GLdouble red, GLdouble green, GLdouble blue)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glSecondaryColor3d(red=%lf, green=%lf, blue=%lf)\n",
                     gc, tid, red, green, blue);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->SecondaryColor3d(gc, red, green, blue);
    __GL_PROFILE_FOOTER(enum_glSecondaryColor3d);

    if (__glTracerDispatchTable.SecondaryColor3d)
    {
        (*__glTracerDispatchTable.SecondaryColor3d)(red, green, blue);
    }
}

GLvoid GL_APIENTRY __glProfile_SecondaryColor3dv(__GLcontext *gc, const GLdouble *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glSecondaryColor3dv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->SecondaryColor3dv(gc, v);
    __GL_PROFILE_FOOTER(enum_glSecondaryColor3dv);

    if (__glTracerDispatchTable.SecondaryColor3dv)
    {
        (*__glTracerDispatchTable.SecondaryColor3dv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_SecondaryColor3f(__GLcontext *gc, GLfloat red, GLfloat green, GLfloat blue)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glSecondaryColor3f(red=%f, green=%f, blue=%f)\n",
                     gc, tid, red, green, blue);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->SecondaryColor3f(gc, red, green, blue);
    __GL_PROFILE_FOOTER(enum_glSecondaryColor3f);

    if (__glTracerDispatchTable.SecondaryColor3f)
    {
        (*__glTracerDispatchTable.SecondaryColor3f)(red, green, blue);
    }
}

GLvoid GL_APIENTRY __glProfile_SecondaryColor3fv(__GLcontext *gc, const GLfloat *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glSecondaryColor3fv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->SecondaryColor3fv(gc, v);
    __GL_PROFILE_FOOTER(enum_glSecondaryColor3fv);

    if (__glTracerDispatchTable.SecondaryColor3fv)
    {
        (*__glTracerDispatchTable.SecondaryColor3fv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_SecondaryColor3i(__GLcontext *gc, GLint red, GLint green, GLint blue)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glSecondaryColor3i(red=%d, green=%d, blue=%d)\n",
                     gc, tid, red, green, blue);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->SecondaryColor3i(gc, red, green, blue);
    __GL_PROFILE_FOOTER(enum_glSecondaryColor3i);

    if (__glTracerDispatchTable.SecondaryColor3i)
    {
        (*__glTracerDispatchTable.SecondaryColor3i)(red, green, blue);
    }
}

GLvoid GL_APIENTRY __glProfile_SecondaryColor3iv(__GLcontext *gc, const GLint *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glSecondaryColor3iv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->SecondaryColor3iv(gc, v);
    __GL_PROFILE_FOOTER(enum_glSecondaryColor3iv);

    if (__glTracerDispatchTable.SecondaryColor3iv)
    {
        (*__glTracerDispatchTable.SecondaryColor3iv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_SecondaryColor3s(__GLcontext *gc, GLshort red, GLshort green, GLshort blue)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glSecondaryColor3s(red=%hd, green=%hd, blue=%hd)\n",
                     gc, tid, red, green, blue);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->SecondaryColor3s(gc, red, green, blue);
    __GL_PROFILE_FOOTER(enum_glSecondaryColor3s);

    if (__glTracerDispatchTable.SecondaryColor3s)
    {
        (*__glTracerDispatchTable.SecondaryColor3s)(red, green, blue);
    }
}

GLvoid GL_APIENTRY __glProfile_SecondaryColor3sv(__GLcontext *gc, const GLshort *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glSecondaryColor3sv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->SecondaryColor3sv(gc, v);
    __GL_PROFILE_FOOTER(enum_glSecondaryColor3sv);

    if (__glTracerDispatchTable.SecondaryColor3sv)
    {
        (*__glTracerDispatchTable.SecondaryColor3sv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_SecondaryColor3ub(__GLcontext *gc, GLubyte red, GLubyte green, GLubyte blue)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glSecondaryColor3ub(red=%hhu, green=%hhu, blue=%hhu)\n",
                     gc, tid, red, green, blue);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->SecondaryColor3ub(gc, red, green, blue);
    __GL_PROFILE_FOOTER(enum_glSecondaryColor3ub);

    if (__glTracerDispatchTable.SecondaryColor3ub)
    {
        (*__glTracerDispatchTable.SecondaryColor3ub)(red, green, blue);
    }
}

GLvoid GL_APIENTRY __glProfile_SecondaryColor3ubv(__GLcontext *gc, const GLubyte *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glSecondaryColor3ubv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->SecondaryColor3ubv(gc, v);
    __GL_PROFILE_FOOTER(enum_glSecondaryColor3ubv);

    if (__glTracerDispatchTable.SecondaryColor3ubv)
    {
        (*__glTracerDispatchTable.SecondaryColor3ubv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_SecondaryColor3ui(__GLcontext *gc, GLuint red, GLuint green, GLuint blue)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glSecondaryColor3ui(red=%u, green=%u, blue=%u)\n",
                     gc, tid, red, green, blue);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->SecondaryColor3ui(gc, red, green, blue);
    __GL_PROFILE_FOOTER(enum_glSecondaryColor3ui);

    if (__glTracerDispatchTable.SecondaryColor3ui)
    {
        (*__glTracerDispatchTable.SecondaryColor3ui)(red, green, blue);
    }
}

GLvoid GL_APIENTRY __glProfile_SecondaryColor3uiv(__GLcontext *gc, const GLuint *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glSecondaryColor3uiv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->SecondaryColor3uiv(gc, v);
    __GL_PROFILE_FOOTER(enum_glSecondaryColor3uiv);

    if (__glTracerDispatchTable.SecondaryColor3uiv)
    {
        (*__glTracerDispatchTable.SecondaryColor3uiv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_SecondaryColor3us(__GLcontext *gc, GLushort red, GLushort green, GLushort blue)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glSecondaryColor3us(red=%hu, green=%hu, blue=%hu)\n",
                     gc, tid, red, green, blue);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->SecondaryColor3us(gc, red, green, blue);
    __GL_PROFILE_FOOTER(enum_glSecondaryColor3us);

    if (__glTracerDispatchTable.SecondaryColor3us)
    {
        (*__glTracerDispatchTable.SecondaryColor3us)(red, green, blue);
    }
}

GLvoid GL_APIENTRY __glProfile_SecondaryColor3usv(__GLcontext *gc, const GLushort *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glSecondaryColor3usv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->SecondaryColor3usv(gc, v);
    __GL_PROFILE_FOOTER(enum_glSecondaryColor3usv);

    if (__glTracerDispatchTable.SecondaryColor3usv)
    {
        (*__glTracerDispatchTable.SecondaryColor3usv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_SecondaryColorPointer(__GLcontext *gc, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glSecondaryColorPointer(size=%d, type=0x%04X, stride=%d, pointer=0x%p)\n",
                     gc, tid, size, type, stride, pointer);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->SecondaryColorPointer(gc, size, type, stride, pointer);
    __GL_PROFILE_FOOTER(enum_glSecondaryColorPointer);

    if (__glTracerDispatchTable.SecondaryColorPointer)
    {
        (*__glTracerDispatchTable.SecondaryColorPointer)(size, type, stride, pointer);
    }
}

GLvoid GL_APIENTRY __glProfile_WindowPos2d(__GLcontext *gc, GLdouble x, GLdouble y)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glWindowPos2d(x=%lf, y=%lf)\n",
                     gc, tid, x, y);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->WindowPos2d(gc, x, y);
    __GL_PROFILE_FOOTER(enum_glWindowPos2d);

    if (__glTracerDispatchTable.WindowPos2d)
    {
        (*__glTracerDispatchTable.WindowPos2d)(x, y);
    }
}

GLvoid GL_APIENTRY __glProfile_WindowPos2dv(__GLcontext *gc, const GLdouble *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glWindowPos2dv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->WindowPos2dv(gc, v);
    __GL_PROFILE_FOOTER(enum_glWindowPos2dv);

    if (__glTracerDispatchTable.WindowPos2dv)
    {
        (*__glTracerDispatchTable.WindowPos2dv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_WindowPos2f(__GLcontext *gc, GLfloat x, GLfloat y)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glWindowPos2f(x=%f, y=%f)\n",
                     gc, tid, x, y);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->WindowPos2f(gc, x, y);
    __GL_PROFILE_FOOTER(enum_glWindowPos2f);

    if (__glTracerDispatchTable.WindowPos2f)
    {
        (*__glTracerDispatchTable.WindowPos2f)(x, y);
    }
}

GLvoid GL_APIENTRY __glProfile_WindowPos2fv(__GLcontext *gc, const GLfloat *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glWindowPos2fv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->WindowPos2fv(gc, v);
    __GL_PROFILE_FOOTER(enum_glWindowPos2fv);

    if (__glTracerDispatchTable.WindowPos2fv)
    {
        (*__glTracerDispatchTable.WindowPos2fv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_WindowPos2i(__GLcontext *gc, GLint x, GLint y)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glWindowPos2i(x=%d, y=%d)\n",
                     gc, tid, x, y);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->WindowPos2i(gc, x, y);
    __GL_PROFILE_FOOTER(enum_glWindowPos2i);

    if (__glTracerDispatchTable.WindowPos2i)
    {
        (*__glTracerDispatchTable.WindowPos2i)(x, y);
    }
}

GLvoid GL_APIENTRY __glProfile_WindowPos2iv(__GLcontext *gc, const GLint *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glWindowPos2iv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->WindowPos2iv(gc, v);
    __GL_PROFILE_FOOTER(enum_glWindowPos2iv);

    if (__glTracerDispatchTable.WindowPos2iv)
    {
        (*__glTracerDispatchTable.WindowPos2iv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_WindowPos2s(__GLcontext *gc, GLshort x, GLshort y)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glWindowPos2s(x=%hd, y=%hd)\n",
                     gc, tid, x, y);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->WindowPos2s(gc, x, y);
    __GL_PROFILE_FOOTER(enum_glWindowPos2s);

    if (__glTracerDispatchTable.WindowPos2s)
    {
        (*__glTracerDispatchTable.WindowPos2s)(x, y);
    }
}

GLvoid GL_APIENTRY __glProfile_WindowPos2sv(__GLcontext *gc, const GLshort *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glWindowPos2sv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->WindowPos2sv(gc, v);
    __GL_PROFILE_FOOTER(enum_glWindowPos2sv);

    if (__glTracerDispatchTable.WindowPos2sv)
    {
        (*__glTracerDispatchTable.WindowPos2sv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_WindowPos3d(__GLcontext *gc, GLdouble x, GLdouble y, GLdouble z)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glWindowPos3d(x=%lf, y=%lf, z=%lf)\n",
                     gc, tid, x, y, z);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->WindowPos3d(gc, x, y, z);
    __GL_PROFILE_FOOTER(enum_glWindowPos3d);

    if (__glTracerDispatchTable.WindowPos3d)
    {
        (*__glTracerDispatchTable.WindowPos3d)(x, y, z);
    }
}

GLvoid GL_APIENTRY __glProfile_WindowPos3dv(__GLcontext *gc, const GLdouble *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glWindowPos3dv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->WindowPos3dv(gc, v);
    __GL_PROFILE_FOOTER(enum_glWindowPos3dv);

    if (__glTracerDispatchTable.WindowPos3dv)
    {
        (*__glTracerDispatchTable.WindowPos3dv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_WindowPos3f(__GLcontext *gc, GLfloat x, GLfloat y, GLfloat z)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glWindowPos3f(x=%f, y=%f, z=%f)\n",
                     gc, tid, x, y, z);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->WindowPos3f(gc, x, y, z);
    __GL_PROFILE_FOOTER(enum_glWindowPos3f);

    if (__glTracerDispatchTable.WindowPos3f)
    {
        (*__glTracerDispatchTable.WindowPos3f)(x, y, z);
    }
}

GLvoid GL_APIENTRY __glProfile_WindowPos3fv(__GLcontext *gc, const GLfloat *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glWindowPos3fv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->WindowPos3fv(gc, v);
    __GL_PROFILE_FOOTER(enum_glWindowPos3fv);

    if (__glTracerDispatchTable.WindowPos3fv)
    {
        (*__glTracerDispatchTable.WindowPos3fv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_WindowPos3i(__GLcontext *gc, GLint x, GLint y, GLint z)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glWindowPos3i(x=%d, y=%d, z=%d)\n",
                     gc, tid, x, y, z);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->WindowPos3i(gc, x, y, z);
    __GL_PROFILE_FOOTER(enum_glWindowPos3i);

    if (__glTracerDispatchTable.WindowPos3i)
    {
        (*__glTracerDispatchTable.WindowPos3i)(x, y, z);
    }
}

GLvoid GL_APIENTRY __glProfile_WindowPos3iv(__GLcontext *gc, const GLint *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glWindowPos3iv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->WindowPos3iv(gc, v);
    __GL_PROFILE_FOOTER(enum_glWindowPos3iv);

    if (__glTracerDispatchTable.WindowPos3iv)
    {
        (*__glTracerDispatchTable.WindowPos3iv)(v);
    }
}

GLvoid GL_APIENTRY __glProfile_WindowPos3s(__GLcontext *gc, GLshort x, GLshort y, GLshort z)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glWindowPos3s(x=%hd, y=%hd, z=%hd)\n",
                     gc, tid, x, y, z);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->WindowPos3s(gc, x, y, z);
    __GL_PROFILE_FOOTER(enum_glWindowPos3s);

    if (__glTracerDispatchTable.WindowPos3s)
    {
        (*__glTracerDispatchTable.WindowPos3s)(x, y, z);
    }
}

GLvoid GL_APIENTRY __glProfile_WindowPos3sv(__GLcontext *gc, const GLshort *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glWindowPos3sv(v=0x%p)\n",
                     gc, tid, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->WindowPos3sv(gc, v);
    __GL_PROFILE_FOOTER(enum_glWindowPos3sv);

    if (__glTracerDispatchTable.WindowPos3sv)
    {
        (*__glTracerDispatchTable.WindowPos3sv)(v);
    }
}

/* GL_VERSION_1_5 */

GLvoid GL_APIENTRY __glProfile_GetQueryObjectiv(__GLcontext *gc, GLuint id, GLenum pname, GLint *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetQueryObjectiv(id=%u, pname=0x%04X, params=0x%p)\n",
                     gc, tid, id, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetQueryObjectiv(gc, id, pname, params);
    __GL_PROFILE_FOOTER(enum_glGetQueryObjectiv);

    if (__glTracerDispatchTable.GetQueryObjectiv)
    {
        (*__glTracerDispatchTable.GetQueryObjectiv)(id, pname, params);
    }
}

GLvoid GL_APIENTRY __glProfile_GetBufferSubData(__GLcontext *gc, GLenum target, GLintptr offset, GLsizeiptr size, GLvoid *data)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetBufferSubData(target=0x%04X, offset=0x%p, size=0x%08X, data=0x%p)\n",
                     gc, tid, target, offset, size, data);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetBufferSubData(gc, target, offset, size, data);
    __GL_PROFILE_FOOTER(enum_glGetBufferSubData);

    if (__glTracerDispatchTable.GetBufferSubData)
    {
        (*__glTracerDispatchTable.GetBufferSubData)(target, offset, size, data);
    }
}

/* GL_VERSION_2_0 */

GLvoid GL_APIENTRY __glProfile_GetVertexAttribdv(__GLcontext *gc, GLuint index, GLenum pname, GLdouble *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetVertexAttribdv(index=%u, pname=0x%04X, params=0x%p)\n",
                     gc, tid, index, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetVertexAttribdv(gc, index, pname, params);
    __GL_PROFILE_FOOTER(enum_glGetVertexAttribdv);

    if (__glTracerDispatchTable.GetVertexAttribdv)
    {
        (*__glTracerDispatchTable.GetVertexAttribdv)(index, pname, params);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttrib1d(__GLcontext *gc, GLuint index, GLdouble x)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttrib1d(index=%u, x=%lf)\n",
                     gc, tid, index, x);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttrib1d(gc, index, x);
    __GL_PROFILE_FOOTER(enum_glVertexAttrib1d);

    if (__glTracerDispatchTable.VertexAttrib1d)
    {
        (*__glTracerDispatchTable.VertexAttrib1d)(index, x);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttrib1dv(__GLcontext *gc, GLuint index, const GLdouble *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttrib1dv(index=%u, v=0x%p)\n",
                     gc, tid, index, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttrib1dv(gc, index, v);
    __GL_PROFILE_FOOTER(enum_glVertexAttrib1dv);

    if (__glTracerDispatchTable.VertexAttrib1dv)
    {
        (*__glTracerDispatchTable.VertexAttrib1dv)(index, v);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttrib1s(__GLcontext *gc, GLuint index, GLshort x)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttrib1s(index=%u, x=%hd)\n",
                     gc, tid, index, x);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttrib1s(gc, index, x);
    __GL_PROFILE_FOOTER(enum_glVertexAttrib1s);

    if (__glTracerDispatchTable.VertexAttrib1s)
    {
        (*__glTracerDispatchTable.VertexAttrib1s)(index, x);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttrib1sv(__GLcontext *gc, GLuint index, const GLshort *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttrib1sv(index=%u, v=0x%p)\n",
                     gc, tid, index, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttrib1sv(gc, index, v);
    __GL_PROFILE_FOOTER(enum_glVertexAttrib1sv);

    if (__glTracerDispatchTable.VertexAttrib1sv)
    {
        (*__glTracerDispatchTable.VertexAttrib1sv)(index, v);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttrib2d(__GLcontext *gc, GLuint index, GLdouble x, GLdouble y)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttrib2d(index=%u, x=%lf, y=%lf)\n",
                     gc, tid, index, x, y);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttrib2d(gc, index, x, y);
    __GL_PROFILE_FOOTER(enum_glVertexAttrib2d);

    if (__glTracerDispatchTable.VertexAttrib2d)
    {
        (*__glTracerDispatchTable.VertexAttrib2d)(index, x, y);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttrib2dv(__GLcontext *gc, GLuint index, const GLdouble *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttrib2dv(index=%u, v=0x%p)\n",
                     gc, tid, index, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttrib2dv(gc, index, v);
    __GL_PROFILE_FOOTER(enum_glVertexAttrib2dv);

    if (__glTracerDispatchTable.VertexAttrib2dv)
    {
        (*__glTracerDispatchTable.VertexAttrib2dv)(index, v);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttrib2s(__GLcontext *gc, GLuint index, GLshort x, GLshort y)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttrib2s(index=%u, x=%hd, y=%hd)\n",
                     gc, tid, index, x, y);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttrib2s(gc, index, x, y);
    __GL_PROFILE_FOOTER(enum_glVertexAttrib2s);

    if (__glTracerDispatchTable.VertexAttrib2s)
    {
        (*__glTracerDispatchTable.VertexAttrib2s)(index, x, y);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttrib2sv(__GLcontext *gc, GLuint index, const GLshort *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttrib2sv(index=%u, v=0x%p)\n",
                     gc, tid, index, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttrib2sv(gc, index, v);
    __GL_PROFILE_FOOTER(enum_glVertexAttrib2sv);

    if (__glTracerDispatchTable.VertexAttrib2sv)
    {
        (*__glTracerDispatchTable.VertexAttrib2sv)(index, v);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttrib3d(__GLcontext *gc, GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttrib3d(index=%u, x=%lf, y=%lf, z=%lf)\n",
                     gc, tid, index, x, y, z);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttrib3d(gc, index, x, y, z);
    __GL_PROFILE_FOOTER(enum_glVertexAttrib3d);

    if (__glTracerDispatchTable.VertexAttrib3d)
    {
        (*__glTracerDispatchTable.VertexAttrib3d)(index, x, y, z);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttrib3dv(__GLcontext *gc, GLuint index, const GLdouble *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttrib3dv(index=%u, v=0x%p)\n",
                     gc, tid, index, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttrib3dv(gc, index, v);
    __GL_PROFILE_FOOTER(enum_glVertexAttrib3dv);

    if (__glTracerDispatchTable.VertexAttrib3dv)
    {
        (*__glTracerDispatchTable.VertexAttrib3dv)(index, v);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttrib3s(__GLcontext *gc, GLuint index, GLshort x, GLshort y, GLshort z)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttrib3s(index=%u, x=%hd, y=%hd, z=%hd)\n",
                     gc, tid, index, x, y, z);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttrib3s(gc, index, x, y, z);
    __GL_PROFILE_FOOTER(enum_glVertexAttrib3s);

    if (__glTracerDispatchTable.VertexAttrib3s)
    {
        (*__glTracerDispatchTable.VertexAttrib3s)(index, x, y, z);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttrib3sv(__GLcontext *gc, GLuint index, const GLshort *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttrib3sv(index=%u, v=0x%p)\n",
                     gc, tid, index, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttrib3sv(gc, index, v);
    __GL_PROFILE_FOOTER(enum_glVertexAttrib3sv);

    if (__glTracerDispatchTable.VertexAttrib3sv)
    {
        (*__glTracerDispatchTable.VertexAttrib3sv)(index, v);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttrib4Nbv(__GLcontext *gc, GLuint index, const GLbyte *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttrib4Nbv(index=%u, v=0x%p)\n",
                     gc, tid, index, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttrib4Nbv(gc, index, v);
    __GL_PROFILE_FOOTER(enum_glVertexAttrib4Nbv);

    if (__glTracerDispatchTable.VertexAttrib4Nbv)
    {
        (*__glTracerDispatchTable.VertexAttrib4Nbv)(index, v);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttrib4Niv(__GLcontext *gc, GLuint index, const GLint *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttrib4Niv(index=%u, v=0x%p)\n",
                     gc, tid, index, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttrib4Niv(gc, index, v);
    __GL_PROFILE_FOOTER(enum_glVertexAttrib4Niv);

    if (__glTracerDispatchTable.VertexAttrib4Niv)
    {
        (*__glTracerDispatchTable.VertexAttrib4Niv)(index, v);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttrib4Nsv(__GLcontext *gc, GLuint index, const GLshort *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttrib4Nsv(index=%u, v=0x%p)\n",
                     gc, tid, index, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttrib4Nsv(gc, index, v);
    __GL_PROFILE_FOOTER(enum_glVertexAttrib4Nsv);

    if (__glTracerDispatchTable.VertexAttrib4Nsv)
    {
        (*__glTracerDispatchTable.VertexAttrib4Nsv)(index, v);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttrib4Nub(__GLcontext *gc, GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttrib4Nub(index=%u, x=%hhu, y=%hhu, z=%hhu, w=%hhu)\n",
                     gc, tid, index, x, y, z, w);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttrib4Nub(gc, index, x, y, z, w);
    __GL_PROFILE_FOOTER(enum_glVertexAttrib4Nub);

    if (__glTracerDispatchTable.VertexAttrib4Nub)
    {
        (*__glTracerDispatchTable.VertexAttrib4Nub)(index, x, y, z, w);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttrib4Nubv(__GLcontext *gc, GLuint index, const GLubyte *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttrib4Nubv(index=%u, v=0x%p)\n",
                     gc, tid, index, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttrib4Nubv(gc, index, v);
    __GL_PROFILE_FOOTER(enum_glVertexAttrib4Nubv);

    if (__glTracerDispatchTable.VertexAttrib4Nubv)
    {
        (*__glTracerDispatchTable.VertexAttrib4Nubv)(index, v);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttrib4Nuiv(__GLcontext *gc, GLuint index, const GLuint *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttrib4Nuiv(index=%u, v=0x%p)\n",
                     gc, tid, index, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttrib4Nuiv(gc, index, v);
    __GL_PROFILE_FOOTER(enum_glVertexAttrib4Nuiv);

    if (__glTracerDispatchTable.VertexAttrib4Nuiv)
    {
        (*__glTracerDispatchTable.VertexAttrib4Nuiv)(index, v);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttrib4Nusv(__GLcontext *gc, GLuint index, const GLushort *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttrib4Nusv(index=%u, v=0x%p)\n",
                     gc, tid, index, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttrib4Nusv(gc, index, v);
    __GL_PROFILE_FOOTER(enum_glVertexAttrib4Nusv);

    if (__glTracerDispatchTable.VertexAttrib4Nusv)
    {
        (*__glTracerDispatchTable.VertexAttrib4Nusv)(index, v);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttrib4bv(__GLcontext *gc, GLuint index, const GLbyte *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttrib4bv(index=%u, v=0x%p)\n",
                     gc, tid, index, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttrib4bv(gc, index, v);
    __GL_PROFILE_FOOTER(enum_glVertexAttrib4bv);

    if (__glTracerDispatchTable.VertexAttrib4bv)
    {
        (*__glTracerDispatchTable.VertexAttrib4bv)(index, v);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttrib4d(__GLcontext *gc, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttrib4d(index=%u, x=%lf, y=%lf, z=%lf, w=%lf)\n",
                     gc, tid, index, x, y, z, w);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttrib4d(gc, index, x, y, z, w);
    __GL_PROFILE_FOOTER(enum_glVertexAttrib4d);

    if (__glTracerDispatchTable.VertexAttrib4d)
    {
        (*__glTracerDispatchTable.VertexAttrib4d)(index, x, y, z, w);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttrib4dv(__GLcontext *gc, GLuint index, const GLdouble *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttrib4dv(index=%u, v=0x%p)\n",
                     gc, tid, index, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttrib4dv(gc, index, v);
    __GL_PROFILE_FOOTER(enum_glVertexAttrib4dv);

    if (__glTracerDispatchTable.VertexAttrib4dv)
    {
        (*__glTracerDispatchTable.VertexAttrib4dv)(index, v);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttrib4iv(__GLcontext *gc, GLuint index, const GLint *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttrib4iv(index=%u, v=0x%p)\n",
                     gc, tid, index, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttrib4iv(gc, index, v);
    __GL_PROFILE_FOOTER(enum_glVertexAttrib4iv);

    if (__glTracerDispatchTable.VertexAttrib4iv)
    {
        (*__glTracerDispatchTable.VertexAttrib4iv)(index, v);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttrib4s(__GLcontext *gc, GLuint index, GLshort x, GLshort y, GLshort z, GLshort w)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttrib4s(index=%u, x=%hd, y=%hd, z=%hd, w=%hd)\n",
                     gc, tid, index, x, y, z, w);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttrib4s(gc, index, x, y, z, w);
    __GL_PROFILE_FOOTER(enum_glVertexAttrib4s);

    if (__glTracerDispatchTable.VertexAttrib4s)
    {
        (*__glTracerDispatchTable.VertexAttrib4s)(index, x, y, z, w);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttrib4sv(__GLcontext *gc, GLuint index, const GLshort *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttrib4sv(index=%u, v=0x%p)\n",
                     gc, tid, index, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttrib4sv(gc, index, v);
    __GL_PROFILE_FOOTER(enum_glVertexAttrib4sv);

    if (__glTracerDispatchTable.VertexAttrib4sv)
    {
        (*__glTracerDispatchTable.VertexAttrib4sv)(index, v);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttrib4ubv(__GLcontext *gc, GLuint index, const GLubyte *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttrib4ubv(index=%u, v=0x%p)\n",
                     gc, tid, index, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttrib4ubv(gc, index, v);
    __GL_PROFILE_FOOTER(enum_glVertexAttrib4ubv);

    if (__glTracerDispatchTable.VertexAttrib4ubv)
    {
        (*__glTracerDispatchTable.VertexAttrib4ubv)(index, v);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttrib4uiv(__GLcontext *gc, GLuint index, const GLuint *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttrib4uiv(index=%u, v=0x%p)\n",
                     gc, tid, index, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttrib4uiv(gc, index, v);
    __GL_PROFILE_FOOTER(enum_glVertexAttrib4uiv);

    if (__glTracerDispatchTable.VertexAttrib4uiv)
    {
        (*__glTracerDispatchTable.VertexAttrib4uiv)(index, v);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttrib4usv(__GLcontext *gc, GLuint index, const GLushort *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttrib4usv(index=%u, v=0x%p)\n",
                     gc, tid, index, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttrib4usv(gc, index, v);
    __GL_PROFILE_FOOTER(enum_glVertexAttrib4usv);

    if (__glTracerDispatchTable.VertexAttrib4usv)
    {
        (*__glTracerDispatchTable.VertexAttrib4usv)(index, v);
    }
}

/* GL_VERSION_2_1 */

/* GL_VERSION_3_0 */

GLvoid GL_APIENTRY __glProfile_ClampColor(__GLcontext *gc, GLenum target, GLenum clamp)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glClampColor(target=0x%04X, clamp=0x%04X)\n",
                     gc, tid, target, clamp);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ClampColor(gc, target, clamp);
    __GL_PROFILE_FOOTER(enum_glClampColor);

    if (__glTracerDispatchTable.ClampColor)
    {
        (*__glTracerDispatchTable.ClampColor)(target, clamp);
    }
}

GLvoid GL_APIENTRY __glProfile_BeginConditionalRender(__GLcontext *gc, GLuint id, GLenum mode)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glBeginConditionalRender(id=%u, mode=0x%04X)\n",
                     gc, tid, id, mode);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->BeginConditionalRender(gc, id, mode);
    __GL_PROFILE_FOOTER(enum_glBeginConditionalRender);

    if (__glTracerDispatchTable.BeginConditionalRender)
    {
        (*__glTracerDispatchTable.BeginConditionalRender)(id, mode);
    }
}

GLvoid GL_APIENTRY __glProfile_EndConditionalRender(__GLcontext *gc)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glEndConditionalRender()\n",
                     gc, tid);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->EndConditionalRender(gc);
    __GL_PROFILE_FOOTER(enum_glEndConditionalRender);

    if (__glTracerDispatchTable.EndConditionalRender)
    {
        (*__glTracerDispatchTable.EndConditionalRender)();
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttribI1i(__GLcontext *gc, GLuint index, GLint x)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttribI1i(index=%u, x=%d)\n",
                     gc, tid, index, x);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttribI1i(gc, index, x);
    __GL_PROFILE_FOOTER(enum_glVertexAttribI1i);

    if (__glTracerDispatchTable.VertexAttribI1i)
    {
        (*__glTracerDispatchTable.VertexAttribI1i)(index, x);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttribI2i(__GLcontext *gc, GLuint index, GLint x, GLint y)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttribI2i(index=%u, x=%d, y=%d)\n",
                     gc, tid, index, x, y);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttribI2i(gc, index, x, y);
    __GL_PROFILE_FOOTER(enum_glVertexAttribI2i);

    if (__glTracerDispatchTable.VertexAttribI2i)
    {
        (*__glTracerDispatchTable.VertexAttribI2i)(index, x, y);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttribI3i(__GLcontext *gc, GLuint index, GLint x, GLint y, GLint z)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttribI3i(index=%u, x=%d, y=%d, z=%d)\n",
                     gc, tid, index, x, y, z);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttribI3i(gc, index, x, y, z);
    __GL_PROFILE_FOOTER(enum_glVertexAttribI3i);

    if (__glTracerDispatchTable.VertexAttribI3i)
    {
        (*__glTracerDispatchTable.VertexAttribI3i)(index, x, y, z);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttribI1ui(__GLcontext *gc, GLuint index, GLuint x)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttribI1ui(index=%u, x=%u)\n",
                     gc, tid, index, x);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttribI1ui(gc, index, x);
    __GL_PROFILE_FOOTER(enum_glVertexAttribI1ui);

    if (__glTracerDispatchTable.VertexAttribI1ui)
    {
        (*__glTracerDispatchTable.VertexAttribI1ui)(index, x);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttribI2ui(__GLcontext *gc, GLuint index, GLuint x, GLuint y)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttribI2ui(index=%u, x=%u, y=%u)\n",
                     gc, tid, index, x, y);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttribI2ui(gc, index, x, y);
    __GL_PROFILE_FOOTER(enum_glVertexAttribI2ui);

    if (__glTracerDispatchTable.VertexAttribI2ui)
    {
        (*__glTracerDispatchTable.VertexAttribI2ui)(index, x, y);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttribI3ui(__GLcontext *gc, GLuint index, GLuint x, GLuint y, GLuint z)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttribI3ui(index=%u, x=%u, y=%u, z=%u)\n",
                     gc, tid, index, x, y, z);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttribI3ui(gc, index, x, y, z);
    __GL_PROFILE_FOOTER(enum_glVertexAttribI3ui);

    if (__glTracerDispatchTable.VertexAttribI3ui)
    {
        (*__glTracerDispatchTable.VertexAttribI3ui)(index, x, y, z);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttribI1iv(__GLcontext *gc, GLuint index, const GLint *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttribI1iv(index=%u, v=0x%p)\n",
                     gc, tid, index, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttribI1iv(gc, index, v);
    __GL_PROFILE_FOOTER(enum_glVertexAttribI1iv);

    if (__glTracerDispatchTable.VertexAttribI1iv)
    {
        (*__glTracerDispatchTable.VertexAttribI1iv)(index, v);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttribI2iv(__GLcontext *gc, GLuint index, const GLint *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttribI2iv(index=%u, v=0x%p)\n",
                     gc, tid, index, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttribI2iv(gc, index, v);
    __GL_PROFILE_FOOTER(enum_glVertexAttribI2iv);

    if (__glTracerDispatchTable.VertexAttribI2iv)
    {
        (*__glTracerDispatchTable.VertexAttribI2iv)(index, v);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttribI3iv(__GLcontext *gc, GLuint index, const GLint *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttribI3iv(index=%u, v=0x%p)\n",
                     gc, tid, index, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttribI3iv(gc, index, v);
    __GL_PROFILE_FOOTER(enum_glVertexAttribI3iv);

    if (__glTracerDispatchTable.VertexAttribI3iv)
    {
        (*__glTracerDispatchTable.VertexAttribI3iv)(index, v);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttribI1uiv(__GLcontext *gc, GLuint index, const GLuint *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttribI1uiv(index=%u, v=0x%p)\n",
                     gc, tid, index, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttribI1uiv(gc, index, v);
    __GL_PROFILE_FOOTER(enum_glVertexAttribI1uiv);

    if (__glTracerDispatchTable.VertexAttribI1uiv)
    {
        (*__glTracerDispatchTable.VertexAttribI1uiv)(index, v);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttribI2uiv(__GLcontext *gc, GLuint index, const GLuint *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttribI2uiv(index=%u, v=0x%p)\n",
                     gc, tid, index, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttribI2uiv(gc, index, v);
    __GL_PROFILE_FOOTER(enum_glVertexAttribI2uiv);

    if (__glTracerDispatchTable.VertexAttribI2uiv)
    {
        (*__glTracerDispatchTable.VertexAttribI2uiv)(index, v);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttribI3uiv(__GLcontext *gc, GLuint index, const GLuint *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttribI3uiv(index=%u, v=0x%p)\n",
                     gc, tid, index, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttribI3uiv(gc, index, v);
    __GL_PROFILE_FOOTER(enum_glVertexAttribI3uiv);

    if (__glTracerDispatchTable.VertexAttribI3uiv)
    {
        (*__glTracerDispatchTable.VertexAttribI3uiv)(index, v);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttribI4bv(__GLcontext *gc, GLuint index, const GLbyte *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttribI4bv(index=%u, v=0x%p)\n",
                     gc, tid, index, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttribI4bv(gc, index, v);
    __GL_PROFILE_FOOTER(enum_glVertexAttribI4bv);

    if (__glTracerDispatchTable.VertexAttribI4bv)
    {
        (*__glTracerDispatchTable.VertexAttribI4bv)(index, v);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttribI4sv(__GLcontext *gc, GLuint index, const GLshort *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttribI4sv(index=%u, v=0x%p)\n",
                     gc, tid, index, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttribI4sv(gc, index, v);
    __GL_PROFILE_FOOTER(enum_glVertexAttribI4sv);

    if (__glTracerDispatchTable.VertexAttribI4sv)
    {
        (*__glTracerDispatchTable.VertexAttribI4sv)(index, v);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttribI4ubv(__GLcontext *gc, GLuint index, const GLubyte *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttribI4ubv(index=%u, v=0x%p)\n",
                     gc, tid, index, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttribI4ubv(gc, index, v);
    __GL_PROFILE_FOOTER(enum_glVertexAttribI4ubv);

    if (__glTracerDispatchTable.VertexAttribI4ubv)
    {
        (*__glTracerDispatchTable.VertexAttribI4ubv)(index, v);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttribI4usv(__GLcontext *gc, GLuint index, const GLushort *v)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttribI4usv(index=%u, v=0x%p)\n",
                     gc, tid, index, v);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttribI4usv(gc, index, v);
    __GL_PROFILE_FOOTER(enum_glVertexAttribI4usv);

    if (__glTracerDispatchTable.VertexAttribI4usv)
    {
        (*__glTracerDispatchTable.VertexAttribI4usv)(index, v);
    }
}

GLvoid GL_APIENTRY __glProfile_BindFragDataLocation(__GLcontext *gc, GLuint program, GLuint color, const GLchar *name)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glBindFragDataLocation(program=%u, color=%u, name=0x%p)\n",
                     gc, tid, program, color, name);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->BindFragDataLocation(gc, program, color, name);
    __GL_PROFILE_FOOTER(enum_glBindFragDataLocation);

    if (__glTracerDispatchTable.BindFragDataLocation)
    {
        (*__glTracerDispatchTable.BindFragDataLocation)(program, color, name);
    }
}

GLvoid GL_APIENTRY __glProfile_FramebufferTexture1D(__GLcontext *gc, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glFramebufferTexture1D(target=0x%04X, attachment=0x%04X, textarget=0x%04X, texture=%u, level=%d)\n",
                     gc, tid, target, attachment, textarget, texture, level);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->FramebufferTexture1D(gc, target, attachment, textarget, texture, level);
    __GL_PROFILE_FOOTER(enum_glFramebufferTexture1D);

    if (__glTracerDispatchTable.FramebufferTexture1D)
    {
        (*__glTracerDispatchTable.FramebufferTexture1D)(target, attachment, textarget, texture, level);
    }
}

GLvoid GL_APIENTRY __glProfile_FramebufferTexture3D(__GLcontext *gc, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glFramebufferTexture3D(target=0x%04X, attachment=0x%04X, textarget=0x%04X, texture=%u, level=%d, zoffset=%d)\n",
                     gc, tid, target, attachment, textarget, texture, level, zoffset);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->FramebufferTexture3D(gc, target, attachment, textarget, texture, level, zoffset);
    __GL_PROFILE_FOOTER(enum_glFramebufferTexture3D);

    if (__glTracerDispatchTable.FramebufferTexture3D)
    {
        (*__glTracerDispatchTable.FramebufferTexture3D)(target, attachment, textarget, texture, level, zoffset);
    }
}

/* GL_VERSION_3_1 */

GLvoid GL_APIENTRY __glProfile_PrimitiveRestartIndex(__GLcontext *gc, GLuint index)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glPrimitiveRestartIndex(index=%u)\n",
                     gc, tid, index);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->PrimitiveRestartIndex(gc, index);
    __GL_PROFILE_FOOTER(enum_glPrimitiveRestartIndex);

    if (__glTracerDispatchTable.PrimitiveRestartIndex)
    {
        (*__glTracerDispatchTable.PrimitiveRestartIndex)(index);
    }
}

GLvoid GL_APIENTRY __glProfile_GetActiveUniformName(__GLcontext *gc, GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformName)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetActiveUniformName(program=%u, uniformIndex=%u, bufSize=%d, length=0x%p, uniformName=0x%p)\n",
                     gc, tid, program, uniformIndex, bufSize, length, uniformName);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetActiveUniformName(gc, program, uniformIndex, bufSize, length, uniformName);
    __GL_PROFILE_FOOTER(enum_glGetActiveUniformName);

    if (__glTracerDispatchTable.GetActiveUniformName)
    {
        (*__glTracerDispatchTable.GetActiveUniformName)(program, uniformIndex, bufSize, length, uniformName);
    }
}

/* GL_VERSION_3_2 */

GLvoid GL_APIENTRY __glProfile_ProvokingVertex(__GLcontext *gc, GLenum mode)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glProvokingVertex(mode=0x%04X)\n",
                     gc, tid, mode);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ProvokingVertex(gc, mode);
    __GL_PROFILE_FOOTER(enum_glProvokingVertex);

    if (__glTracerDispatchTable.ProvokingVertex)
    {
        (*__glTracerDispatchTable.ProvokingVertex)(mode);
    }
}

GLvoid GL_APIENTRY __glProfile_TexImage2DMultisample(__GLcontext *gc, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexImage2DMultisample(target=0x%04X, samples=%d, internalformat=0x%04X, width=%d, height=%d, fixedsamplelocations=%hhu)\n",
                     gc, tid, target, samples, internalformat, width, height, fixedsamplelocations);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexImage2DMultisample(gc, target, samples, internalformat, width, height, fixedsamplelocations);
    __GL_PROFILE_FOOTER(enum_glTexImage2DMultisample);

    if (__glTracerDispatchTable.TexImage2DMultisample)
    {
        (*__glTracerDispatchTable.TexImage2DMultisample)(target, samples, internalformat, width, height, fixedsamplelocations);
    }
}

GLvoid GL_APIENTRY __glProfile_TexImage3DMultisample(__GLcontext *gc, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexImage3DMultisample(target=0x%04X, samples=%d, internalformat=0x%04X, width=%d, height=%d, depth=%d, fixedsamplelocations=%hhu)\n",
                     gc, tid, target, samples, internalformat, width, height, depth, fixedsamplelocations);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexImage3DMultisample(gc, target, samples, internalformat, width, height, depth, fixedsamplelocations);
    __GL_PROFILE_FOOTER(enum_glTexImage3DMultisample);

    if (__glTracerDispatchTable.TexImage3DMultisample)
    {
        (*__glTracerDispatchTable.TexImage3DMultisample)(target, samples, internalformat, width, height, depth, fixedsamplelocations);
    }
}

/* GL_VERSION_3_3 */

GLvoid GL_APIENTRY __glProfile_BindFragDataLocationIndexed(__GLcontext *gc, GLuint program, GLuint colorNumber, GLuint index, const GLchar *name)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glBindFragDataLocationIndexed(program=%u, colorNumber=%u, index=%u, name=0x%p)\n",
                     gc, tid, program, colorNumber, index, name);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->BindFragDataLocationIndexed(gc, program, colorNumber, index, name);
    __GL_PROFILE_FOOTER(enum_glBindFragDataLocationIndexed);

    if (__glTracerDispatchTable.BindFragDataLocationIndexed)
    {
        (*__glTracerDispatchTable.BindFragDataLocationIndexed)(program, colorNumber, index, name);
    }
}

GLint GL_APIENTRY __glProfile_GetFragDataIndex(__GLcontext *gc, GLuint program, const GLchar *name)
{
    __GL_PROFILE_VARS();
    GLint ret;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetFragDataIndex(program=%u, name=0x%p)\n",
                     gc, tid, program, name);
    }

    __GL_PROFILE_HEADER();
    ret = gc->pModeDispatch->GetFragDataIndex(gc, program, name);
    __GL_PROFILE_FOOTER(enum_glGetFragDataIndex);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetFragDataIndex => %d\n", ret);
    }

    if (__glTracerDispatchTable.GetFragDataIndex)
    {
        (*__glTracerDispatchTable.GetFragDataIndex)(program, name);
    }

    return ret;
}

GLvoid GL_APIENTRY __glProfile_QueryCounter(__GLcontext *gc, GLuint id, GLenum target)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glQueryCounter(id=%u, target=0x%04X)\n",
                     gc, tid, id, target);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->QueryCounter(gc, id, target);
    __GL_PROFILE_FOOTER(enum_glQueryCounter);

    if (__glTracerDispatchTable.QueryCounter)
    {
        (*__glTracerDispatchTable.QueryCounter)(id, target);
    }
}

GLvoid GL_APIENTRY __glProfile_GetQueryObjecti64v(__GLcontext *gc, GLuint id, GLenum pname, GLint64 *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetQueryObjecti64v(id=%u, pname=0x%04X, params=0x%p)\n",
                     gc, tid, id, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetQueryObjecti64v(gc, id, pname, params);
    __GL_PROFILE_FOOTER(enum_glGetQueryObjecti64v);

    if (__glTracerDispatchTable.GetQueryObjecti64v)
    {
        (*__glTracerDispatchTable.GetQueryObjecti64v)(id, pname, params);
    }
}

GLvoid GL_APIENTRY __glProfile_GetQueryObjectui64v(__GLcontext *gc, GLuint id, GLenum pname, GLuint64 *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetQueryObjectui64v(id=%u, pname=0x%04X, params=0x%p)\n",
                     gc, tid, id, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetQueryObjectui64v(gc, id, pname, params);
    __GL_PROFILE_FOOTER(enum_glGetQueryObjectui64v);

    if (__glTracerDispatchTable.GetQueryObjectui64v)
    {
        (*__glTracerDispatchTable.GetQueryObjectui64v)(id, pname, params);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttribP1ui(__GLcontext *gc, GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttribP1ui(index=%u, type=0x%04X, normalized=%hhu, value=%u)\n",
                     gc, tid, index, type, normalized, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttribP1ui(gc, index, type, normalized, value);
    __GL_PROFILE_FOOTER(enum_glVertexAttribP1ui);

    if (__glTracerDispatchTable.VertexAttribP1ui)
    {
        (*__glTracerDispatchTable.VertexAttribP1ui)(index, type, normalized, value);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttribP1uiv(__GLcontext *gc, GLuint index, GLenum type, GLboolean normalized, const GLuint *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttribP1uiv(index=%u, type=0x%04X, normalized=%hhu, value=0x%p)\n",
                     gc, tid, index, type, normalized, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttribP1uiv(gc, index, type, normalized, value);
    __GL_PROFILE_FOOTER(enum_glVertexAttribP1uiv);

    if (__glTracerDispatchTable.VertexAttribP1uiv)
    {
        (*__glTracerDispatchTable.VertexAttribP1uiv)(index, type, normalized, value);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttribP2ui(__GLcontext *gc, GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttribP2ui(index=%u, type=0x%04X, normalized=%hhu, value=%u)\n",
                     gc, tid, index, type, normalized, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttribP2ui(gc, index, type, normalized, value);
    __GL_PROFILE_FOOTER(enum_glVertexAttribP2ui);

    if (__glTracerDispatchTable.VertexAttribP2ui)
    {
        (*__glTracerDispatchTable.VertexAttribP2ui)(index, type, normalized, value);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttribP2uiv(__GLcontext *gc, GLuint index, GLenum type, GLboolean normalized, const GLuint *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttribP2uiv(index=%u, type=0x%04X, normalized=%hhu, value=0x%p)\n",
                     gc, tid, index, type, normalized, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttribP2uiv(gc, index, type, normalized, value);
    __GL_PROFILE_FOOTER(enum_glVertexAttribP2uiv);

    if (__glTracerDispatchTable.VertexAttribP2uiv)
    {
        (*__glTracerDispatchTable.VertexAttribP2uiv)(index, type, normalized, value);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttribP3ui(__GLcontext *gc, GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttribP3ui(index=%u, type=0x%04X, normalized=%hhu, value=%u)\n",
                     gc, tid, index, type, normalized, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttribP3ui(gc, index, type, normalized, value);
    __GL_PROFILE_FOOTER(enum_glVertexAttribP3ui);

    if (__glTracerDispatchTable.VertexAttribP3ui)
    {
        (*__glTracerDispatchTable.VertexAttribP3ui)(index, type, normalized, value);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttribP3uiv(__GLcontext *gc, GLuint index, GLenum type, GLboolean normalized, const GLuint *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttribP3uiv(index=%u, type=0x%04X, normalized=%hhu, value=0x%p)\n",
                     gc, tid, index, type, normalized, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttribP3uiv(gc, index, type, normalized, value);
    __GL_PROFILE_FOOTER(enum_glVertexAttribP3uiv);

    if (__glTracerDispatchTable.VertexAttribP3uiv)
    {
        (*__glTracerDispatchTable.VertexAttribP3uiv)(index, type, normalized, value);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttribP4ui(__GLcontext *gc, GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttribP4ui(index=%u, type=0x%04X, normalized=%hhu, value=%u)\n",
                     gc, tid, index, type, normalized, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttribP4ui(gc, index, type, normalized, value);
    __GL_PROFILE_FOOTER(enum_glVertexAttribP4ui);

    if (__glTracerDispatchTable.VertexAttribP4ui)
    {
        (*__glTracerDispatchTable.VertexAttribP4ui)(index, type, normalized, value);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexAttribP4uiv(__GLcontext *gc, GLuint index, GLenum type, GLboolean normalized, const GLuint *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexAttribP4uiv(index=%u, type=0x%04X, normalized=%hhu, value=0x%p)\n",
                     gc, tid, index, type, normalized, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexAttribP4uiv(gc, index, type, normalized, value);
    __GL_PROFILE_FOOTER(enum_glVertexAttribP4uiv);

    if (__glTracerDispatchTable.VertexAttribP4uiv)
    {
        (*__glTracerDispatchTable.VertexAttribP4uiv)(index, type, normalized, value);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexP2ui(__GLcontext *gc, GLenum type, GLuint value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexP2ui(type=0x%04X, value=%u)\n",
                     gc, tid, type, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexP2ui(gc, type, value);
    __GL_PROFILE_FOOTER(enum_glVertexP2ui);

    if (__glTracerDispatchTable.VertexP2ui)
    {
        (*__glTracerDispatchTable.VertexP2ui)(type, value);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexP2uiv(__GLcontext *gc, GLenum type, const GLuint *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexP2uiv(type=0x%04X, value=0x%p)\n",
                     gc, tid, type, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexP2uiv(gc, type, value);
    __GL_PROFILE_FOOTER(enum_glVertexP2uiv);

    if (__glTracerDispatchTable.VertexP2uiv)
    {
        (*__glTracerDispatchTable.VertexP2uiv)(type, value);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexP3ui(__GLcontext *gc, GLenum type, GLuint value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexP3ui(type=0x%04X, value=%u)\n",
                     gc, tid, type, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexP3ui(gc, type, value);
    __GL_PROFILE_FOOTER(enum_glVertexP3ui);

    if (__glTracerDispatchTable.VertexP3ui)
    {
        (*__glTracerDispatchTable.VertexP3ui)(type, value);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexP3uiv(__GLcontext *gc, GLenum type, const GLuint *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexP3uiv(type=0x%04X, value=0x%p)\n",
                     gc, tid, type, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexP3uiv(gc, type, value);
    __GL_PROFILE_FOOTER(enum_glVertexP3uiv);

    if (__glTracerDispatchTable.VertexP3uiv)
    {
        (*__glTracerDispatchTable.VertexP3uiv)(type, value);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexP4ui(__GLcontext *gc, GLenum type, GLuint value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexP4ui(type=0x%04X, value=%u)\n",
                     gc, tid, type, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexP4ui(gc, type, value);
    __GL_PROFILE_FOOTER(enum_glVertexP4ui);

    if (__glTracerDispatchTable.VertexP4ui)
    {
        (*__glTracerDispatchTable.VertexP4ui)(type, value);
    }
}

GLvoid GL_APIENTRY __glProfile_VertexP4uiv(__GLcontext *gc, GLenum type, const GLuint *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glVertexP4uiv(type=0x%04X, value=0x%p)\n",
                     gc, tid, type, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->VertexP4uiv(gc, type, value);
    __GL_PROFILE_FOOTER(enum_glVertexP4uiv);

    if (__glTracerDispatchTable.VertexP4uiv)
    {
        (*__glTracerDispatchTable.VertexP4uiv)(type, value);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoordP1ui(__GLcontext *gc, GLenum type, GLuint coords)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoordP1ui(type=0x%04X, coords=%u)\n",
                     gc, tid, type, coords);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoordP1ui(gc, type, coords);
    __GL_PROFILE_FOOTER(enum_glTexCoordP1ui);

    if (__glTracerDispatchTable.TexCoordP1ui)
    {
        (*__glTracerDispatchTable.TexCoordP1ui)(type, coords);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoordP1uiv(__GLcontext *gc, GLenum type, const GLuint *coords)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoordP1uiv(type=0x%04X, coords=0x%p)\n",
                     gc, tid, type, coords);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoordP1uiv(gc, type, coords);
    __GL_PROFILE_FOOTER(enum_glTexCoordP1uiv);

    if (__glTracerDispatchTable.TexCoordP1uiv)
    {
        (*__glTracerDispatchTable.TexCoordP1uiv)(type, coords);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoordP2ui(__GLcontext *gc, GLenum type, GLuint coords)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoordP2ui(type=0x%04X, coords=%u)\n",
                     gc, tid, type, coords);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoordP2ui(gc, type, coords);
    __GL_PROFILE_FOOTER(enum_glTexCoordP2ui);

    if (__glTracerDispatchTable.TexCoordP2ui)
    {
        (*__glTracerDispatchTable.TexCoordP2ui)(type, coords);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoordP2uiv(__GLcontext *gc, GLenum type, const GLuint *coords)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoordP2uiv(type=0x%04X, coords=0x%p)\n",
                     gc, tid, type, coords);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoordP2uiv(gc, type, coords);
    __GL_PROFILE_FOOTER(enum_glTexCoordP2uiv);

    if (__glTracerDispatchTable.TexCoordP2uiv)
    {
        (*__glTracerDispatchTable.TexCoordP2uiv)(type, coords);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoordP3ui(__GLcontext *gc, GLenum type, GLuint coords)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoordP3ui(type=0x%04X, coords=%u)\n",
                     gc, tid, type, coords);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoordP3ui(gc, type, coords);
    __GL_PROFILE_FOOTER(enum_glTexCoordP3ui);

    if (__glTracerDispatchTable.TexCoordP3ui)
    {
        (*__glTracerDispatchTable.TexCoordP3ui)(type, coords);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoordP3uiv(__GLcontext *gc, GLenum type, const GLuint *coords)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoordP3uiv(type=0x%04X, coords=0x%p)\n",
                     gc, tid, type, coords);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoordP3uiv(gc, type, coords);
    __GL_PROFILE_FOOTER(enum_glTexCoordP3uiv);

    if (__glTracerDispatchTable.TexCoordP3uiv)
    {
        (*__glTracerDispatchTable.TexCoordP3uiv)(type, coords);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoordP4ui(__GLcontext *gc, GLenum type, GLuint coords)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoordP4ui(type=0x%04X, coords=%u)\n",
                     gc, tid, type, coords);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoordP4ui(gc, type, coords);
    __GL_PROFILE_FOOTER(enum_glTexCoordP4ui);

    if (__glTracerDispatchTable.TexCoordP4ui)
    {
        (*__glTracerDispatchTable.TexCoordP4ui)(type, coords);
    }
}

GLvoid GL_APIENTRY __glProfile_TexCoordP4uiv(__GLcontext *gc, GLenum type, const GLuint *coords)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexCoordP4uiv(type=0x%04X, coords=0x%p)\n",
                     gc, tid, type, coords);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexCoordP4uiv(gc, type, coords);
    __GL_PROFILE_FOOTER(enum_glTexCoordP4uiv);

    if (__glTracerDispatchTable.TexCoordP4uiv)
    {
        (*__glTracerDispatchTable.TexCoordP4uiv)(type, coords);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoordP1ui(__GLcontext *gc, GLenum texture, GLenum type, GLuint coords)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoordP1ui(texture=0x%04X, type=0x%04X, coords=%u)\n",
                     gc, tid, texture, type, coords);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoordP1ui(gc, texture, type, coords);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoordP1ui);

    if (__glTracerDispatchTable.MultiTexCoordP1ui)
    {
        (*__glTracerDispatchTable.MultiTexCoordP1ui)(texture, type, coords);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoordP1uiv(__GLcontext *gc, GLenum texture, GLenum type, const GLuint *coords)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoordP1uiv(texture=0x%04X, type=0x%04X, coords=0x%p)\n",
                     gc, tid, texture, type, coords);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoordP1uiv(gc, texture, type, coords);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoordP1uiv);

    if (__glTracerDispatchTable.MultiTexCoordP1uiv)
    {
        (*__glTracerDispatchTable.MultiTexCoordP1uiv)(texture, type, coords);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoordP2ui(__GLcontext *gc, GLenum texture, GLenum type, GLuint coords)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoordP2ui(texture=0x%04X, type=0x%04X, coords=%u)\n",
                     gc, tid, texture, type, coords);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoordP2ui(gc, texture, type, coords);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoordP2ui);

    if (__glTracerDispatchTable.MultiTexCoordP2ui)
    {
        (*__glTracerDispatchTable.MultiTexCoordP2ui)(texture, type, coords);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoordP2uiv(__GLcontext *gc, GLenum texture, GLenum type, const GLuint *coords)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoordP2uiv(texture=0x%04X, type=0x%04X, coords=0x%p)\n",
                     gc, tid, texture, type, coords);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoordP2uiv(gc, texture, type, coords);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoordP2uiv);

    if (__glTracerDispatchTable.MultiTexCoordP2uiv)
    {
        (*__glTracerDispatchTable.MultiTexCoordP2uiv)(texture, type, coords);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoordP3ui(__GLcontext *gc, GLenum texture, GLenum type, GLuint coords)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoordP3ui(texture=0x%04X, type=0x%04X, coords=%u)\n",
                     gc, tid, texture, type, coords);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoordP3ui(gc, texture, type, coords);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoordP3ui);

    if (__glTracerDispatchTable.MultiTexCoordP3ui)
    {
        (*__glTracerDispatchTable.MultiTexCoordP3ui)(texture, type, coords);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoordP3uiv(__GLcontext *gc, GLenum texture, GLenum type, const GLuint *coords)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoordP3uiv(texture=0x%04X, type=0x%04X, coords=0x%p)\n",
                     gc, tid, texture, type, coords);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoordP3uiv(gc, texture, type, coords);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoordP3uiv);

    if (__glTracerDispatchTable.MultiTexCoordP3uiv)
    {
        (*__glTracerDispatchTable.MultiTexCoordP3uiv)(texture, type, coords);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoordP4ui(__GLcontext *gc, GLenum texture, GLenum type, GLuint coords)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoordP4ui(texture=0x%04X, type=0x%04X, coords=%u)\n",
                     gc, tid, texture, type, coords);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoordP4ui(gc, texture, type, coords);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoordP4ui);

    if (__glTracerDispatchTable.MultiTexCoordP4ui)
    {
        (*__glTracerDispatchTable.MultiTexCoordP4ui)(texture, type, coords);
    }
}

GLvoid GL_APIENTRY __glProfile_MultiTexCoordP4uiv(__GLcontext *gc, GLenum texture, GLenum type, const GLuint *coords)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glMultiTexCoordP4uiv(texture=0x%04X, type=0x%04X, coords=0x%p)\n",
                     gc, tid, texture, type, coords);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->MultiTexCoordP4uiv(gc, texture, type, coords);
    __GL_PROFILE_FOOTER(enum_glMultiTexCoordP4uiv);

    if (__glTracerDispatchTable.MultiTexCoordP4uiv)
    {
        (*__glTracerDispatchTable.MultiTexCoordP4uiv)(texture, type, coords);
    }
}

GLvoid GL_APIENTRY __glProfile_NormalP3ui(__GLcontext *gc, GLenum type, GLuint coords)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glNormalP3ui(type=0x%04X, coords=%u)\n",
                     gc, tid, type, coords);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->NormalP3ui(gc, type, coords);
    __GL_PROFILE_FOOTER(enum_glNormalP3ui);

    if (__glTracerDispatchTable.NormalP3ui)
    {
        (*__glTracerDispatchTable.NormalP3ui)(type, coords);
    }
}

GLvoid GL_APIENTRY __glProfile_NormalP3uiv(__GLcontext *gc, GLenum type, const GLuint *coords)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glNormalP3uiv(type=0x%04X, coords=0x%p)\n",
                     gc, tid, type, coords);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->NormalP3uiv(gc, type, coords);
    __GL_PROFILE_FOOTER(enum_glNormalP3uiv);

    if (__glTracerDispatchTable.NormalP3uiv)
    {
        (*__glTracerDispatchTable.NormalP3uiv)(type, coords);
    }
}

GLvoid GL_APIENTRY __glProfile_ColorP3ui(__GLcontext *gc, GLenum type, GLuint color)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColorP3ui(type=0x%04X, color=%u)\n",
                     gc, tid, type, color);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ColorP3ui(gc, type, color);
    __GL_PROFILE_FOOTER(enum_glColorP3ui);

    if (__glTracerDispatchTable.ColorP3ui)
    {
        (*__glTracerDispatchTable.ColorP3ui)(type, color);
    }
}

GLvoid GL_APIENTRY __glProfile_ColorP3uiv(__GLcontext *gc, GLenum type, const GLuint *color)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColorP3uiv(type=0x%04X, color=0x%p)\n",
                     gc, tid, type, color);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ColorP3uiv(gc, type, color);
    __GL_PROFILE_FOOTER(enum_glColorP3uiv);

    if (__glTracerDispatchTable.ColorP3uiv)
    {
        (*__glTracerDispatchTable.ColorP3uiv)(type, color);
    }
}

GLvoid GL_APIENTRY __glProfile_ColorP4ui(__GLcontext *gc, GLenum type, GLuint color)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColorP4ui(type=0x%04X, color=%u)\n",
                     gc, tid, type, color);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ColorP4ui(gc, type, color);
    __GL_PROFILE_FOOTER(enum_glColorP4ui);

    if (__glTracerDispatchTable.ColorP4ui)
    {
        (*__glTracerDispatchTable.ColorP4ui)(type, color);
    }
}

GLvoid GL_APIENTRY __glProfile_ColorP4uiv(__GLcontext *gc, GLenum type, const GLuint *color)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glColorP4uiv(type=0x%04X, color=0x%p)\n",
                     gc, tid, type, color);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->ColorP4uiv(gc, type, color);
    __GL_PROFILE_FOOTER(enum_glColorP4uiv);

    if (__glTracerDispatchTable.ColorP4uiv)
    {
        (*__glTracerDispatchTable.ColorP4uiv)(type, color);
    }
}

GLvoid GL_APIENTRY __glProfile_SecondaryColorP3ui(__GLcontext *gc, GLenum type, GLuint color)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glSecondaryColorP3ui(type=0x%04X, color=%u)\n",
                     gc, tid, type, color);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->SecondaryColorP3ui(gc, type, color);
    __GL_PROFILE_FOOTER(enum_glSecondaryColorP3ui);

    if (__glTracerDispatchTable.SecondaryColorP3ui)
    {
        (*__glTracerDispatchTable.SecondaryColorP3ui)(type, color);
    }
}

GLvoid GL_APIENTRY __glProfile_SecondaryColorP3uiv(__GLcontext *gc, GLenum type, const GLuint *color)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glSecondaryColorP3uiv(type=0x%04X, color=0x%p)\n",
                     gc, tid, type, color);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->SecondaryColorP3uiv(gc, type, color);
    __GL_PROFILE_FOOTER(enum_glSecondaryColorP3uiv);

    if (__glTracerDispatchTable.SecondaryColorP3uiv)
    {
        (*__glTracerDispatchTable.SecondaryColorP3uiv)(type, color);
    }
}

/* GL_VERSION_4_0 */

GLvoid GL_APIENTRY __glProfile_Uniform1d(__GLcontext *gc, GLint location, GLdouble x)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniform1d(location=%d, x=%lf)\n",
                     gc, tid, location, x);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Uniform1d(gc, location, x);
    __GL_PROFILE_FOOTER(enum_glUniform1d);

    if (__glTracerDispatchTable.Uniform1d)
    {
        (*__glTracerDispatchTable.Uniform1d)(location, x);
    }
}

GLvoid GL_APIENTRY __glProfile_Uniform2d(__GLcontext *gc, GLint location, GLdouble x, GLdouble y)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniform2d(location=%d, x=%lf, y=%lf)\n",
                     gc, tid, location, x, y);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Uniform2d(gc, location, x, y);
    __GL_PROFILE_FOOTER(enum_glUniform2d);

    if (__glTracerDispatchTable.Uniform2d)
    {
        (*__glTracerDispatchTable.Uniform2d)(location, x, y);
    }
}

GLvoid GL_APIENTRY __glProfile_Uniform3d(__GLcontext *gc, GLint location, GLdouble x, GLdouble y, GLdouble z)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniform3d(location=%d, x=%lf, y=%lf, z=%lf)\n",
                     gc, tid, location, x, y, z);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Uniform3d(gc, location, x, y, z);
    __GL_PROFILE_FOOTER(enum_glUniform3d);

    if (__glTracerDispatchTable.Uniform3d)
    {
        (*__glTracerDispatchTable.Uniform3d)(location, x, y, z);
    }
}

GLvoid GL_APIENTRY __glProfile_Uniform4d(__GLcontext *gc, GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniform4d(location=%d, x=%lf, y=%lf, z=%lf, w=%lf)\n",
                     gc, tid, location, x, y, z, w);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Uniform4d(gc, location, x, y, z, w);
    __GL_PROFILE_FOOTER(enum_glUniform4d);

    if (__glTracerDispatchTable.Uniform4d)
    {
        (*__glTracerDispatchTable.Uniform4d)(location, x, y, z, w);
    }
}

GLvoid GL_APIENTRY __glProfile_Uniform1dv(__GLcontext *gc, GLint location, GLsizei count, const GLdouble *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniform1dv(location=%d, count=%d, value=0x%p)\n",
                     gc, tid, location, count, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Uniform1dv(gc, location, count, value);
    __GL_PROFILE_FOOTER(enum_glUniform1dv);

    if (__glTracerDispatchTable.Uniform1dv)
    {
        (*__glTracerDispatchTable.Uniform1dv)(location, count, value);
    }
}

GLvoid GL_APIENTRY __glProfile_Uniform2dv(__GLcontext *gc, GLint location, GLsizei count, const GLdouble *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniform2dv(location=%d, count=%d, value=0x%p)\n",
                     gc, tid, location, count, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Uniform2dv(gc, location, count, value);
    __GL_PROFILE_FOOTER(enum_glUniform2dv);

    if (__glTracerDispatchTable.Uniform2dv)
    {
        (*__glTracerDispatchTable.Uniform2dv)(location, count, value);
    }
}

GLvoid GL_APIENTRY __glProfile_Uniform3dv(__GLcontext *gc, GLint location, GLsizei count, const GLdouble *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniform3dv(location=%d, count=%d, value=0x%p)\n",
                     gc, tid, location, count, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Uniform3dv(gc, location, count, value);
    __GL_PROFILE_FOOTER(enum_glUniform3dv);

    if (__glTracerDispatchTable.Uniform3dv)
    {
        (*__glTracerDispatchTable.Uniform3dv)(location, count, value);
    }
}

GLvoid GL_APIENTRY __glProfile_Uniform4dv(__GLcontext *gc, GLint location, GLsizei count, const GLdouble *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniform4dv(location=%d, count=%d, value=0x%p)\n",
                     gc, tid, location, count, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->Uniform4dv(gc, location, count, value);
    __GL_PROFILE_FOOTER(enum_glUniform4dv);

    if (__glTracerDispatchTable.Uniform4dv)
    {
        (*__glTracerDispatchTable.Uniform4dv)(location, count, value);
    }
}

GLvoid GL_APIENTRY __glProfile_UniformMatrix2dv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniformMatrix2dv(location=%d, count=%d, transpose=%hhu, value=0x%p)\n",
                     gc, tid, location, count, transpose, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->UniformMatrix2dv(gc, location, count, transpose, value);
    __GL_PROFILE_FOOTER(enum_glUniformMatrix2dv);

    if (__glTracerDispatchTable.UniformMatrix2dv)
    {
        (*__glTracerDispatchTable.UniformMatrix2dv)(location, count, transpose, value);
    }
}

GLvoid GL_APIENTRY __glProfile_UniformMatrix3dv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniformMatrix3dv(location=%d, count=%d, transpose=%hhu, value=0x%p)\n",
                     gc, tid, location, count, transpose, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->UniformMatrix3dv(gc, location, count, transpose, value);
    __GL_PROFILE_FOOTER(enum_glUniformMatrix3dv);

    if (__glTracerDispatchTable.UniformMatrix3dv)
    {
        (*__glTracerDispatchTable.UniformMatrix3dv)(location, count, transpose, value);
    }
}

GLvoid GL_APIENTRY __glProfile_UniformMatrix4dv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniformMatrix4dv(location=%d, count=%d, transpose=%hhu, value=0x%p)\n",
                     gc, tid, location, count, transpose, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->UniformMatrix4dv(gc, location, count, transpose, value);
    __GL_PROFILE_FOOTER(enum_glUniformMatrix4dv);

    if (__glTracerDispatchTable.UniformMatrix4dv)
    {
        (*__glTracerDispatchTable.UniformMatrix4dv)(location, count, transpose, value);
    }
}

GLvoid GL_APIENTRY __glProfile_UniformMatrix2x3dv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniformMatrix2x3dv(location=%d, count=%d, transpose=%hhu, value=0x%p)\n",
                     gc, tid, location, count, transpose, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->UniformMatrix2x3dv(gc, location, count, transpose, value);
    __GL_PROFILE_FOOTER(enum_glUniformMatrix2x3dv);

    if (__glTracerDispatchTable.UniformMatrix2x3dv)
    {
        (*__glTracerDispatchTable.UniformMatrix2x3dv)(location, count, transpose, value);
    }
}

GLvoid GL_APIENTRY __glProfile_UniformMatrix2x4dv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniformMatrix2x4dv(location=%d, count=%d, transpose=%hhu, value=0x%p)\n",
                     gc, tid, location, count, transpose, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->UniformMatrix2x4dv(gc, location, count, transpose, value);
    __GL_PROFILE_FOOTER(enum_glUniformMatrix2x4dv);

    if (__glTracerDispatchTable.UniformMatrix2x4dv)
    {
        (*__glTracerDispatchTable.UniformMatrix2x4dv)(location, count, transpose, value);
    }
}

GLvoid GL_APIENTRY __glProfile_UniformMatrix3x2dv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniformMatrix3x2dv(location=%d, count=%d, transpose=%hhu, value=0x%p)\n",
                     gc, tid, location, count, transpose, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->UniformMatrix3x2dv(gc, location, count, transpose, value);
    __GL_PROFILE_FOOTER(enum_glUniformMatrix3x2dv);

    if (__glTracerDispatchTable.UniformMatrix3x2dv)
    {
        (*__glTracerDispatchTable.UniformMatrix3x2dv)(location, count, transpose, value);
    }
}

GLvoid GL_APIENTRY __glProfile_UniformMatrix3x4dv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniformMatrix3x4dv(location=%d, count=%d, transpose=%hhu, value=0x%p)\n",
                     gc, tid, location, count, transpose, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->UniformMatrix3x4dv(gc, location, count, transpose, value);
    __GL_PROFILE_FOOTER(enum_glUniformMatrix3x4dv);

    if (__glTracerDispatchTable.UniformMatrix3x4dv)
    {
        (*__glTracerDispatchTable.UniformMatrix3x4dv)(location, count, transpose, value);
    }
}

GLvoid GL_APIENTRY __glProfile_UniformMatrix4x2dv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniformMatrix4x2dv(location=%d, count=%d, transpose=%hhu, value=0x%p)\n",
                     gc, tid, location, count, transpose, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->UniformMatrix4x2dv(gc, location, count, transpose, value);
    __GL_PROFILE_FOOTER(enum_glUniformMatrix4x2dv);

    if (__glTracerDispatchTable.UniformMatrix4x2dv)
    {
        (*__glTracerDispatchTable.UniformMatrix4x2dv)(location, count, transpose, value);
    }
}

GLvoid GL_APIENTRY __glProfile_UniformMatrix4x3dv(__GLcontext *gc, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniformMatrix4x3dv(location=%d, count=%d, transpose=%hhu, value=0x%p)\n",
                     gc, tid, location, count, transpose, value);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->UniformMatrix4x3dv(gc, location, count, transpose, value);
    __GL_PROFILE_FOOTER(enum_glUniformMatrix4x3dv);

    if (__glTracerDispatchTable.UniformMatrix4x3dv)
    {
        (*__glTracerDispatchTable.UniformMatrix4x3dv)(location, count, transpose, value);
    }
}

GLvoid GL_APIENTRY __glProfile_GetUniformdv(__GLcontext *gc, GLuint program, GLint location, GLdouble *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetUniformdv(program=%u, location=%d, params=0x%p)\n",
                     gc, tid, program, location, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetUniformdv(gc, program, location, params);
    __GL_PROFILE_FOOTER(enum_glGetUniformdv);

    if (__glTracerDispatchTable.GetUniformdv)
    {
        (*__glTracerDispatchTable.GetUniformdv)(program, location, params);
    }
}

GLint GL_APIENTRY __glProfile_GetSubroutineUniformLocation(__GLcontext *gc, GLuint program, GLenum shadertype, const GLchar *name)
{
    __GL_PROFILE_VARS();
    GLint ret;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetSubroutineUniformLocation(program=%u, shadertype=0x%04X, name=0x%p)\n",
                     gc, tid, program, shadertype, name);
    }

    __GL_PROFILE_HEADER();
    ret = gc->pModeDispatch->GetSubroutineUniformLocation(gc, program, shadertype, name);
    __GL_PROFILE_FOOTER(enum_glGetSubroutineUniformLocation);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetSubroutineUniformLocation => %d\n", ret);
    }

    if (__glTracerDispatchTable.GetSubroutineUniformLocation)
    {
        (*__glTracerDispatchTable.GetSubroutineUniformLocation)(program, shadertype, name);
    }

    return ret;
}

GLuint GL_APIENTRY __glProfile_GetSubroutineIndex(__GLcontext *gc, GLuint program, GLenum shadertype, const GLchar *name)
{
    __GL_PROFILE_VARS();
    GLuint ret;

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetSubroutineIndex(program=%u, shadertype=0x%04X, name=0x%p)\n",
                     gc, tid, program, shadertype, name);
    }

    __GL_PROFILE_HEADER();
    ret = gc->pModeDispatch->GetSubroutineIndex(gc, program, shadertype, name);
    __GL_PROFILE_FOOTER(enum_glGetSubroutineIndex);

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_POST)
    {
        __GL_LOG_API("        glGetSubroutineIndex => %u\n", ret);
    }

    if (__glTracerDispatchTable.GetSubroutineIndex)
    {
        (*__glTracerDispatchTable.GetSubroutineIndex)(program, shadertype, name);
    }

    return ret;
}

GLvoid GL_APIENTRY __glProfile_GetActiveSubroutineUniformiv(__GLcontext *gc, GLuint program, GLenum shadertype, GLuint index, GLenum pname, GLint *values)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetActiveSubroutineUniformiv(program=%u, shadertype=0x%04X, index=%u, pname=0x%04X, values=0x%p)\n",
                     gc, tid, program, shadertype, index, pname, values);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetActiveSubroutineUniformiv(gc, program, shadertype, index, pname, values);
    __GL_PROFILE_FOOTER(enum_glGetActiveSubroutineUniformiv);

    if (__glTracerDispatchTable.GetActiveSubroutineUniformiv)
    {
        (*__glTracerDispatchTable.GetActiveSubroutineUniformiv)(program, shadertype, index, pname, values);
    }
}

GLvoid GL_APIENTRY __glProfile_GetActiveSubroutineUniformName(__GLcontext *gc, GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei *length, GLchar *name)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetActiveSubroutineUniformName(program=%u, shadertype=0x%04X, index=%u, bufsize=%d, length=0x%p, name=0x%p)\n",
                     gc, tid, program, shadertype, index, bufsize, length, name);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetActiveSubroutineUniformName(gc, program, shadertype, index, bufsize, length, name);
    __GL_PROFILE_FOOTER(enum_glGetActiveSubroutineUniformName);

    if (__glTracerDispatchTable.GetActiveSubroutineUniformName)
    {
        (*__glTracerDispatchTable.GetActiveSubroutineUniformName)(program, shadertype, index, bufsize, length, name);
    }
}

GLvoid GL_APIENTRY __glProfile_GetActiveSubroutineName(__GLcontext *gc, GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei *length, GLchar *name)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetActiveSubroutineName(program=%u, shadertype=0x%04X, index=%u, bufsize=%d, length=0x%p, name=0x%p)\n",
                     gc, tid, program, shadertype, index, bufsize, length, name);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetActiveSubroutineName(gc, program, shadertype, index, bufsize, length, name);
    __GL_PROFILE_FOOTER(enum_glGetActiveSubroutineName);

    if (__glTracerDispatchTable.GetActiveSubroutineName)
    {
        (*__glTracerDispatchTable.GetActiveSubroutineName)(program, shadertype, index, bufsize, length, name);
    }
}

GLvoid GL_APIENTRY __glProfile_UniformSubroutinesuiv(__GLcontext *gc, GLenum shadertype, GLsizei count, const GLuint *indices)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glUniformSubroutinesuiv(shadertype=0x%04X, count=%d, indices=0x%p)\n",
                     gc, tid, shadertype, count, indices);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->UniformSubroutinesuiv(gc, shadertype, count, indices);
    __GL_PROFILE_FOOTER(enum_glUniformSubroutinesuiv);

    if (__glTracerDispatchTable.UniformSubroutinesuiv)
    {
        (*__glTracerDispatchTable.UniformSubroutinesuiv)(shadertype, count, indices);
    }
}

GLvoid GL_APIENTRY __glProfile_GetUniformSubroutineuiv(__GLcontext *gc, GLenum shadertype, GLint location, GLuint *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetUniformSubroutineuiv(shadertype=0x%04X, location=%d, params=0x%p)\n",
                     gc, tid, shadertype, location, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetUniformSubroutineuiv(gc, shadertype, location, params);
    __GL_PROFILE_FOOTER(enum_glGetUniformSubroutineuiv);

    if (__glTracerDispatchTable.GetUniformSubroutineuiv)
    {
        (*__glTracerDispatchTable.GetUniformSubroutineuiv)(shadertype, location, params);
    }
}

GLvoid GL_APIENTRY __glProfile_GetProgramStageiv(__GLcontext *gc, GLuint program, GLenum shadertype, GLenum pname, GLint *values)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetProgramStageiv(program=%u, shadertype=0x%04X, pname=0x%04X, values=0x%p)\n",
                     gc, tid, program, shadertype, pname, values);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetProgramStageiv(gc, program, shadertype, pname, values);
    __GL_PROFILE_FOOTER(enum_glGetProgramStageiv);

    if (__glTracerDispatchTable.GetProgramStageiv)
    {
        (*__glTracerDispatchTable.GetProgramStageiv)(program, shadertype, pname, values);
    }
}

GLvoid GL_APIENTRY __glProfile_PatchParameterfv(__GLcontext *gc, GLenum pname, const GLfloat *values)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glPatchParameterfv(pname=0x%04X, values=0x%p)\n",
                     gc, tid, pname, values);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->PatchParameterfv(gc, pname, values);
    __GL_PROFILE_FOOTER(enum_glPatchParameterfv);

    if (__glTracerDispatchTable.PatchParameterfv)
    {
        (*__glTracerDispatchTable.PatchParameterfv)(pname, values);
    }
}

GLvoid GL_APIENTRY __glProfile_DrawTransformFeedback(__GLcontext *gc, GLenum mode, GLuint id)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDrawTransformFeedback(mode=0x%04X, id=%u)\n",
                     gc, tid, mode, id);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DrawTransformFeedback(gc, mode, id);
    __GL_PROFILE_FOOTER(enum_glDrawTransformFeedback);

    if (__glTracerDispatchTable.DrawTransformFeedback)
    {
        (*__glTracerDispatchTable.DrawTransformFeedback)(mode, id);
    }
}

GLvoid GL_APIENTRY __glProfile_DrawTransformFeedbackStream(__GLcontext *gc, GLenum mode, GLuint id, GLuint stream)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDrawTransformFeedbackStream(mode=0x%04X, id=%u, stream=%u)\n",
                     gc, tid, mode, id, stream);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DrawTransformFeedbackStream(gc, mode, id, stream);
    __GL_PROFILE_FOOTER(enum_glDrawTransformFeedbackStream);

    if (__glTracerDispatchTable.DrawTransformFeedbackStream)
    {
        (*__glTracerDispatchTable.DrawTransformFeedbackStream)(mode, id, stream);
    }
}

GLvoid GL_APIENTRY __glProfile_BeginQueryIndexed(__GLcontext *gc, GLenum target, GLuint index, GLuint id)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glBeginQueryIndexed(target=0x%04X, index=%u, id=%u)\n",
                     gc, tid, target, index, id);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->BeginQueryIndexed(gc, target, index, id);
    __GL_PROFILE_FOOTER(enum_glBeginQueryIndexed);

    if (__glTracerDispatchTable.BeginQueryIndexed)
    {
        (*__glTracerDispatchTable.BeginQueryIndexed)(target, index, id);
    }
}

GLvoid GL_APIENTRY __glProfile_EndQueryIndexed(__GLcontext *gc, GLenum target, GLuint index)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glEndQueryIndexed(target=0x%04X, index=%u)\n",
                     gc, tid, target, index);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->EndQueryIndexed(gc, target, index);
    __GL_PROFILE_FOOTER(enum_glEndQueryIndexed);

    if (__glTracerDispatchTable.EndQueryIndexed)
    {
        (*__glTracerDispatchTable.EndQueryIndexed)(target, index);
    }
}

GLvoid GL_APIENTRY __glProfile_GetQueryIndexediv(__GLcontext *gc, GLenum target, GLuint index, GLenum pname, GLint *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetQueryIndexediv(target=0x%04X, index=%u, pname=0x%04X, params=0x%p)\n",
                     gc, tid, target, index, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetQueryIndexediv(gc, target, index, pname, params);
    __GL_PROFILE_FOOTER(enum_glGetQueryIndexediv);

    if (__glTracerDispatchTable.GetQueryIndexediv)
    {
        (*__glTracerDispatchTable.GetQueryIndexediv)(target, index, pname, params);
    }
}


/* GL_ARB_shader_objects */

GLvoid GL_APIENTRY __glProfile_DeleteObjectARB(__GLcontext *gc, GLhandleARB obj)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDeleteObjectARB(obj=0x%p)\n",
                     gc, tid, obj);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DeleteObjectARB(gc, obj);
    __GL_PROFILE_FOOTER(enum_glDeleteObjectARB);

    if (__glTracerDispatchTable.DeleteObjectARB)
    {
        (*__glTracerDispatchTable.DeleteObjectARB)(obj);
    }
}

GLvoid GL_APIENTRY __glProfile_GetObjectParameterivARB(__GLcontext *gc, GLhandleARB obj, GLenum pname, GLint *params)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetObjectParameterivARB(obj=0x%p, pname=0x%04X, params=0x%p)\n",
                     gc, tid, obj, pname, params);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetObjectParameterivARB(gc, obj, pname, params);
    __GL_PROFILE_FOOTER(enum_glGetObjectParameterivARB);

    if (__glTracerDispatchTable.GetObjectParameterivARB)
    {
        (*__glTracerDispatchTable.GetObjectParameterivARB)(obj, pname, params);
    }
}

GLvoid GL_APIENTRY __glProfile_GetInfoLogARB(__GLcontext *gc, GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glGetInfoLogARB(obj=0x%p, maxLength=%d, length=0x%p, infoLog=0x%p)\n",
                     gc, tid, obj, maxLength, length, infoLog);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->GetInfoLogARB(gc, obj, maxLength, length, infoLog);
    __GL_PROFILE_FOOTER(enum_glGetInfoLogARB);

    if (__glTracerDispatchTable.GetInfoLogARB)
    {
        (*__glTracerDispatchTable.GetInfoLogARB)(obj, maxLength, length, infoLog);
    }
}
#endif

/*
** OpenGL ES extensions
*/
#if GL_OES_EGL_image
GLvoid GLAPIENTRY __glProfile_EGLImageTargetTexture2DOES(__GLcontext *gc, GLenum target, GLeglImageOES image)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glEGLImageTargetTexture2DOES 0x%04X 0x%08X\n",
                        gc, tid, target, __GL_PTR2UINT(image));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->EGLImageTargetTexture2DOES(gc, target, image);
    __GL_PROFILE_FOOTER(GL4_EGLIMAGETARGETTEXTURE2DOES);

    if (__glTracerDispatchTable.EGLImageTargetTexture2DOES)
    {
        (*__glTracerDispatchTable.EGLImageTargetTexture2DOES)(target, image);
    }
}

GLvoid GLAPIENTRY __glProfile_EGLImageTargetRenderbufferStorageOES(__GLcontext *gc, GLenum target, GLeglImageOES image)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glEGLImageTargetRenderbufferStorageOES 0x%04X 0x%08X\n",
                        gc, tid, target, __GL_PTR2UINT(image));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->EGLImageTargetRenderbufferStorageOES(gc, target, image);
    __GL_PROFILE_FOOTER(GL4_EGLIMAGETARGETRENDERBUFFERSTORAGEOES);

    if (__glTracerDispatchTable.EGLImageTargetRenderbufferStorageOES)
    {
        (*__glTracerDispatchTable.EGLImageTargetRenderbufferStorageOES)(target, image);
    }
}
#endif

#if GL_VIV_direct_texture
GLvoid GLAPIENTRY __glProfile_TexDirectVIV(__GLcontext *gc, GLenum target, GLsizei width, GLsizei height, GLenum format, GLvoid ** pixels)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexDirectVIV 0x%04X %d %d 0x%04X 0x%08X\n",
                        gc, tid, target, width, height, format, __GL_PTR2UINT(pixels));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexDirectVIV(gc, target, width, height, format, pixels);
    __GL_PROFILE_FOOTER(GL4_TEXDIRECTVIV);

    if (__glTracerDispatchTable.TexDirectVIV)
    {
        (*__glTracerDispatchTable.TexDirectVIV)(target, width, height, format, pixels);
    }
}

GLvoid GLAPIENTRY __glProfile_TexDirectInvalidateVIV(__GLcontext *gc, GLenum target)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexDirectInvalidateVIV 0x%04X\n",
                        gc, tid, target);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexDirectInvalidateVIV(gc, target);
    __GL_PROFILE_FOOTER(GL4_TEXDIRECTINVALIDATEVIV);

    if (__glTracerDispatchTable.TexDirectInvalidateVIV)
    {
        (*__glTracerDispatchTable.TexDirectInvalidateVIV)(target);
    }
}

GLvoid GLAPIENTRY __glProfile_TexDirectVIVMap(__GLcontext *gc, GLenum target, GLsizei width, GLsizei height, GLenum format, GLvoid ** logical, const GLuint * physical)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexDirectVIVMap 0x%04X %d %d 0x%04X 0x%08X 0x%08X\n",
                        gc, tid, target, width, height, format, __GL_PTR2UINT(logical), __GL_PTR2UINT(physical));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexDirectVIVMap(gc, target, width, height, format, logical, physical);
    __GL_PROFILE_FOOTER(GL4_TEXDIRECTVIVMAP);

    if (__glTracerDispatchTable.TexDirectVIVMap)
    {
        (*__glTracerDispatchTable.TexDirectVIVMap)(target, width, height, format, logical, physical);
    }
}

GLvoid GLAPIENTRY __glProfile_TexDirectTiledMapVIV(__GLcontext *gc, GLenum target, GLsizei width, GLsizei height, GLenum format, GLvoid ** logical, const GLuint * physical)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glTexDirectTiledMapVIV 0x%04X %d %d 0x%04X 0x%08X 0x%08X\n",
                        gc, tid, target, width, height, format, __GL_PTR2UINT(logical), __GL_PTR2UINT(physical));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->TexDirectTiledMapVIV(gc, target, width, height, format, logical, physical);
    __GL_PROFILE_FOOTER(GL4_TEXDIRECTTILEDMAPVIV);

    if (__glTracerDispatchTable.TexDirectTiledMapVIV)
    {
        (*__glTracerDispatchTable.TexDirectTiledMapVIV)(target, width, height, format, logical, physical);
    }
}
#endif

#if GL_EXT_discard_framebuffer
GLvoid GLAPIENTRY __glProfile_DiscardFramebufferEXT(__GLcontext *gc, GLenum target, GLsizei numAttachments, const GLenum *attachments)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glDiscardFramebufferEXT 0x%04X %d 0x%08X\n",
                        gc, tid, target, numAttachments, __GL_PTR2UINT(numAttachments));
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->DiscardFramebufferEXT(gc, target, numAttachments, attachments);
    __GL_PROFILE_FOOTER(GL4_DISCARDFRAMEBUFFEREXT);

    if (__glTracerDispatchTable.DiscardFramebufferEXT)
    {
        (*__glTracerDispatchTable.DiscardFramebufferEXT)(target, numAttachments, attachments);
    }
}
#endif

#if GL_EXT_multisampled_render_to_texture
GLvoid GLAPIENTRY __glProfile_FramebufferTexture2DMultisampleEXT(__GLcontext *gc, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples)
{
    __GL_PROFILE_VARS();

    if (__glApiTraceMode == gcvTRACEMODE_FULL || __glApiTraceMode == gcvTRACEMODE_PRE)
    {
        __GL_LOG_API("(gc=%p, tid=%p): glFramebufferTexture2DMultisampleEXT 0x%04X 0x%04X 0x%04X %u %d %d\n",
                        gc, tid, target, attachment, textarget, texture, level, samples);
    }

    __GL_PROFILE_HEADER();
    gc->pModeDispatch->FramebufferTexture2DMultisampleEXT(gc, target, attachment, textarget, texture, level, samples);
    __GL_PROFILE_FOOTER(GL4_FRAMEBUFFERTEXTURE2DMULTISAMPLEEXT);

    if (__glTracerDispatchTable.FramebufferTexture2DMultisampleEXT)
    {
        (*__glTracerDispatchTable.FramebufferTexture2DMultisampleEXT)(target, attachment, textarget, texture, level, samples);
    }
}
#endif


#define __glProfile(func) __glProfile_##func

__GLdispatchTable __glProfileFuncTable =
{
    __GL_API_ENTRIES(__glProfile)
};

