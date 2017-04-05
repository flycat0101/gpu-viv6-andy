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


/*******************************************************************************
**  Profiler for OpenGL ES 1.1 driver.
*/

#include "gc_glff_precomp.h"

#define _GC_OBJ_ZONE    glvZONE_TRACE

#if VIVANTE_PROFILER
#if VIVANTE_PROFILER_CONTEXT
#define GLFFPROFILER_HAL Context->phal
#else
#define GLFFPROFILER_HAL Context->hal
#endif

GLboolean _glffIsSyncMode = gcvTRUE;
gctINT _glffProfileMode = -1;

#if gcdNEW_PROFILER_FILE

#define gcmWRITE_CONST_NEW(ConstValue) \
    do \
    { \
        gceSTATUS status; \
        gctINT32 value = ConstValue; \
        gcmERR_BREAK(gcoPROFILER_NEW_Write(profiler, gcmSIZEOF(value), &value)); \
    } \
    while (gcvFALSE)

#define gcmWRITE_VALUE_NEW(IntData) \
    do \
    { \
        gceSTATUS status; \
        gctINT32 value = IntData; \
        gcmERR_BREAK(gcoPROFILER_NEW_Write(profiler, gcmSIZEOF(value), &value)); \
    } \
    while (gcvFALSE)

#define gcmWRITE_COUNTER_NEW(Counter, Value) \
    gcmWRITE_CONST_NEW(Counter); \
    gcmWRITE_VALUE_NEW(Value)

/* Write a string value (char*). */
#define gcmWRITE_STRING_NEW(String) \
    do \
    { \
        gceSTATUS status; \
        gctINT32 length; \
        length = (gctINT32) gcoOS_StrLen((gctSTRING)String, gcvNULL); \
        gcmERR_BREAK(gcoPROFILER_NEW_Write(profiler, gcmSIZEOF(length), &length)); \
        gcmERR_BREAK(gcoPROFILER_NEW_Write(profiler, length, String)); \
    } \
    while (gcvFALSE)

#define gcmWRITE_CONST(ConstValue) \
    do \
    { \
        gceSTATUS status; \
        gctINT32 value = ConstValue; \
        gcmERR_BREAK(gcoPROFILER_Write(GLFFPROFILER_HAL, gcmSIZEOF(value), &value)); \
    } \
    while (gcvFALSE)

#define gcmWRITE_VALUE(IntData) \
    do \
    { \
        gceSTATUS status; \
        gctINT32 value = IntData; \
        gcmERR_BREAK(gcoPROFILER_Write(GLFFPROFILER_HAL, gcmSIZEOF(value), &value)); \
    } \
    while (gcvFALSE)

#define gcmWRITE_COUNTER(Counter, Value) \
    gcmWRITE_CONST(Counter); \
    gcmWRITE_VALUE(Value)

/* Write a string value (char*). */
#define gcmWRITE_STRING(String) \
    do \
    { \
        gceSTATUS status; \
        gctINT32 length; \
        length = (gctINT32) gcoOS_StrLen((gctSTRING)String, gcvNULL); \
        gcmERR_BREAK(gcoPROFILER_Write(GLFFPROFILER_HAL, gcmSIZEOF(length), &length)); \
        gcmERR_BREAK(gcoPROFILER_Write(GLFFPROFILER_HAL, length, String)); \
    } \
    while (gcvFALSE)

#else

static const char *
apiCallString[] =
{
    "glActiveTexture",
    "glAttachShader",
    "glBindAttribLocation",
    "glBindBuffer",
    "glBindFramebuffer",
    "glBindRenderbuffer",
    "glBindTexture",
    "glBlendColor",
    "glBlendEquation",
    "glBlendEquationSeparate",
    "glBlendFunc",
    "glBlendFuncSeparate",
    "glBufferData",
    "glBufferSubData",
    "glCheckFramebufferStatus",
    "glClear",
    "glClearColor",
    "glClearDepthf",
    "glClearStencil",
    "glColorMask",
    "glCompileShader",
    "glCompressedTexImage2D",
    "glCompressedTexSubImage2D",
    "glCopyTexImage2D",
    "glCopyTexSubImage2D",
    "glCreateProgram",
    "glCreateShader",
    "glCullFace",
    "glDeleteBuffers",
    "glDeleteFramebuffers",
    "glDeleteProgram",
    "glDeleteRenderbuffers",
    "glDeleteShader",
    "glDeleteTextures",
    "glDepthFunc",
    "glDepthMask",
    "glDepthRangef",
    "glDetachShader",
    "glDisable",
    "glDisableVertexAttribArray",
    "glDrawArrays",
    "glDrawElements",
    "glEnable",
    "glEnableVertexAttribArray",
    "glFinish",
    "glFlush",
    "glFramebufferRenderbuffer",
    "glFramebufferTexture2D",
    "glFrontFace",
    "glGenBuffers",
    "glGenerateMipmap",
    "glGenFramebuffers",
    "glGenRenderbuffers",
    "glGenTextures",
    "glGetActiveAttrib",
    "glGetActiveUniform",
    "glGetAttachedShaders",
    "glGetAttribLocation",
    "glGetBooleanv",
    "glGetBufferParameteriv",
    "glGetError",
    "glGetFloatv",
    "glGetFramebufferAttachmentParameteriv",
    "glGetIntegerv",
    "glGetProgramiv",
    "glGetProgramInfoLog",
    "glGetRenderbufferParameteriv",
    "glGetShaderiv",
    "glGetShaderInfoLog",
    "glGetShaderPrecisionFormat",
    "glGetShaderSource",
    "glGetString",
    "glGetTexParameterfv",
    "glGetTexParameteriv",
    "glGetUniformfv",
    "glGetUniformiv",
    "glGetUniformLocation",
    "glGetVertexAttribfv",
    "glGetVertexAttribiv",
    "glGetVertexAttribPointerv",
    "glHint",
    "glIsBuffer",
    "glIsEnabled",
    "glIsFramebuffer",
    "glIsProgram",
    "glIsRenderbuffer",
    "glIsShader",
    "glIsTexture",
    "glLineWidth",
    "glLinkProgram",
    "glPixelStorei",
    "glPolygonOffset",
    "glReadPixels",
    "glReleaseShaderCompiler",
    "glRenderbufferStorage",
    "glSampleCoverage",
    "glScissor",
    "glShaderBinary",
    "glShaderSource",
    "glStencilFunc",
    "glStencilFuncSeparate",
    "glStencilMask",
    "glStencilMaskSeparate",
    "glStencilOp",
    "glStencilOpSeparate",
    "glTexImage2D",
    "glTexParameterf",
    "glTexParameterfv",
    "glTexParameteri",
    "glTexParameteriv",
    "glTexSubImage2D",
    "glUniform1f",
    "glUniform1fv",
    "glUniform1i",
    "glUniform1iv",
    "glUniform2f",
    "glUniform2fv",
    "glUniform2i",
    "glUniform2iv",
    "glUniform3f",
    "glUniform3fv",
    "glUniform3i",
    "glUniform3iv",
    "glUniform4f",
    "glUniform4fv",
    "glUniform4i",
    "glUniform4iv",
    "glUniformMatrix2fv",
    "glUniformMatrix3fv",
    "glUniformMatrix4fv",
    "glUseProgram",
    "glValidateProgram",
    "glVertexAttrib1f",
    "glVertexAttrib1fv",
    "glVertexAttrib2f",
    "glVertexAttrib2fv",
    "glVertexAttrib3f",
    "glVertexAttrib3fv",
    "glVertexAttrib4f",
    "glVertexAttrib4fv",
    "glVertexAttribPointer",
    "glViewport",
    "glBlendEquationOES",
    "glBlendFuncSeparateOES",
    "glBlendEquationSeparateOES",
    "glMapBufferOES",
    "glUnmapBufferOES",
    "glGetBufferPointervOES",
};

static gceSTATUS
_Print(
    glsCONTEXT_PTR Context,
    gctCONST_STRING Format,
    ...
    )
{
    char buffer[256];
    gctUINT offset = 0;
    gceSTATUS status;
    gcmHEADER_ARG("Context=0x%x Format=0x%x", Context, Format);

    gcmONERROR(
        gcoOS_PrintStrVSafe(buffer, gcmSIZEOF(buffer),
                            &offset,
                            Format,
                            &Format + 1));

    gcmONERROR(
        gcoPROFILER_Write(GLFFPROFILER_HAL,
                          offset,
                          buffer));

    gcmFOOTER_ARG("return=%s", "gcvSTATUS_OK");
    return gcvSTATUS_OK;

OnError:
    gcmFOOTER_ARG("return=%s", status);
    return status;
}
#endif

void
_glffProfiler_NEW_Initialize(
    glsCONTEXT_PTR Context
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctCHAR *env = gcvNULL;
    glsPROFILER * profiler = &Context->profiler;
#ifdef ANDROID
    gctBOOL matchResult = gcvFALSE;
#endif

    gcmHEADER_ARG("Context=0x%x", Context);

    _glffProfileMode = -1;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_PROFILE", &env)) && env)
    {
        if (gcmIS_SUCCESS(gcoOS_StrCmp(env, "0")))
        {
            _glffProfileMode = 0;
        }
        else if (gcmIS_SUCCESS(gcoOS_StrCmp(env, "1")))
        {
            _glffProfileMode = 1;
        }
        else if (gcmIS_SUCCESS(gcoOS_StrCmp(env, "2")))
        {
            _glffProfileMode = 2;
        }
        else if (gcmIS_SUCCESS(gcoOS_StrCmp(env, "3")))
        {
            _glffProfileMode = 3;
        }
    }

    /* Clear the profiler. */
    gcoOS_ZeroMemory(&Context->profiler, gcmSIZEOF(glsPROFILER));

    switch (_glffProfileMode)
    {
    case -1:
        profiler->enable = gcvFALSE;
        gcmFOOTER_NO();
        return;
    case 0:
        gcoPROFILER_NEW_Disable();
        profiler->enable = gcvFALSE;
        gcmFOOTER_NO();
        return;
    case 1:
        profiler->enableOutputCounters = gcvTRUE;

        gcoOS_GetEnv(gcvNULL, "VP_FRAME_NUM", &env);
        if ((env != gcvNULL) && (env[0] != 0))
        {
            gctINT32 frameNum;
            gcoOS_StrToInt(env, &frameNum);
            if (frameNum > 1)
                profiler->frameCount = frameNum;
        }
        break;
    case 2:
        profiler->enableOutputCounters = gcvFALSE;
        break;
    case 3:
        profiler->enableOutputCounters = gcvFALSE;
        gcoOS_GetEnv(gcvNULL, "VP_FRAME_START", &env);
        if ((env != gcvNULL) && (env[0] != 0))
        {
            gctINT32 frameNum;
            gcoOS_StrToInt(env, &frameNum);
            if (frameNum > 1)
                profiler->frameStartNumber = frameNum;
        }
        gcoOS_GetEnv(gcvNULL, "VP_FRAME_END", &env);
        if ((env != gcvNULL) && (env[0] != 0))
        {
            gctINT32 frameNum;
            gcoOS_StrToInt(env, &frameNum);
            if (frameNum > 1)
                profiler->frameEndNumber = frameNum;
        }
        break;
    default:
        profiler->enable = gcvFALSE;
        gcmFOOTER_NO();
        return;
    }
    gcmONERROR(gcoPROFILER_NEW_Construct(&Context->profilerObj));

    profiler->useGlfinish = gcvFALSE;
    gcoOS_GetEnv(gcvNULL, "VP_USE_GLFINISH", &env);
    if ((env != gcvNULL) && (env[0] == '1'))
    {
        profiler->useGlfinish = gcvTRUE;
    }

    gcoOS_GetEnv(gcvNULL, "VP_SYNC_MODE", &env);
    if ((env != gcvNULL) && gcmIS_SUCCESS(gcoOS_StrCmp(env, "0")))
    {
        Context->profilerObj->isSyncMode =
        _glffIsSyncMode = gcvFALSE;
    }

    gcoOS_GetEnv(gcvNULL, "VP_OUTPUT", &env);
    if ((env != gcvNULL) && *env != '\0')
    {
        Context->profilerObj->fileName = env;
    }

#ifdef ANDROID
    gcoOS_GetEnv(gcvNULL, "VP_PROCESS_NAME", &env);
    if ((env != gcvNULL) && (env[0] != 0)) matchResult = (gcoOS_DetectProcessByName(env) ? gcvTRUE : gcvFALSE);
    if (matchResult != gcvTRUE) {
        gcmFOOTER_NO();
        return;
    }
#endif

    if (gcoPROFILER_NEW_Enable(Context->profilerObj) != gcvSTATUS_OK)
    {
        profiler->enable = gcvFALSE;
        gcmFOOTER_NO();
        return;
    }
    profiler->enable = gcvTRUE;

    profiler->curFrameNumber = 0;
    profiler->drawCount = 0;
    profiler->perDraw = gcvFALSE;
    profiler->perFrame = gcvFALSE;
    profiler->useFBO = gcvFALSE;

    gcoOS_GetTime(&profiler->frameStart);
    profiler->frameStartTimeusec = profiler->frameStart;
    profiler->primitiveStartTimeusec = profiler->frameStart;
    gcoOS_GetCPUTime(&profiler->frameStartCPUTimeusec);

    _glffProfiler_NEW_Write(Context, GL1_PROFILER_WRITE_HEADER);

OnError:
    /* Return the error. */
    gcmFOOTER_NO();
}

void
_glffProfiler_NEW_Destroy(
    glsCONTEXT_PTR Context
    )
{
    glsPROFILER * profiler = &Context->profiler;

    gcmHEADER_ARG("Context=0x%x", Context);

    if (profiler->enable)
    {
        profiler->enable = gcvFALSE;
        gcmVERIFY_OK(gcoPROFILER_NEW_Destroy(Context->profilerObj));
    }

    /* Success. */
    gcmFOOTER_NO();
}

gceSTATUS
_glffProfiler_NEW_Write(
    glsCONTEXT_PTR Context,
    GLuint Enum
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT rev;
    gcoPROFILER profiler = Context->profilerObj;
    gctSTRING infoCompany = "Vivante Corporation";
    gctSTRING infoVersion = "1.3";
    gctCHAR infoRevision[255] = { '\0' };   /* read from hw */
    gctSTRING infoRenderer = Context->chipName;
    gctSTRING infoDriver = "OpenGL ES 1.1";
    gctUINT offset = 0;
    gctUINT32 totalCalls_11 = 0;
    gctUINT32 totalDrawCalls_11 = 0;
    gctUINT32 totalStateChangeCalls_11 = 0;
    gctUINT32 maxrss, ixrss, idrss, isrss;
    gctINT32 i;

    gcmHEADER_ARG("Context=0x%x, Enum=%d", Context, Enum);

    switch (Enum)
    {
    case GL1_PROFILER_WRITE_HEADER:
        /* Write Generic Info. */
        rev = Context->chipRevision;
#define BCD(digit)      ((rev >> (digit * 4)) & 0xF)
        gcoOS_MemFill(infoRevision, 0, gcmSIZEOF(infoRevision));
        if (BCD(3) == 0)
        {
            /* Old format. */
            gcoOS_PrintStrSafe(infoRevision, gcmSIZEOF(infoRevision),
                &offset, "revision=\"%d.%d\" ", BCD(1), BCD(0));
        }
        else
        {
            /* New format. */
            gcoOS_PrintStrSafe(infoRevision, gcmSIZEOF(infoRevision),
                &offset, "revision=\"%d.%d.%d_rc%d\" ",
                BCD(3), BCD(2), BCD(1), BCD(0));
        }

        gcmWRITE_CONST_NEW(VPG_INFO);

        gcmWRITE_CONST_NEW(VPC_INFOCOMPANY);
        gcmWRITE_STRING_NEW(infoCompany);
        gcmWRITE_CONST_NEW(VPC_INFOVERSION);
        gcmWRITE_STRING_NEW(infoVersion);
        gcmWRITE_CONST_NEW(VPC_INFORENDERER);
        gcmWRITE_STRING_NEW(infoRenderer);
        gcmWRITE_CONST_NEW(VPC_INFOREVISION);
        gcmWRITE_STRING_NEW(infoRevision);
        gcmWRITE_CONST_NEW(VPC_INFODRIVER);
        gcmWRITE_STRING_NEW(infoDriver);
#if gcdNULL_DRIVER >= 2
        {
            char* infoDiverMode = "NULL Driver";
            gcmWRITE_CONST(VPC_INFODRIVERMODE);
            gcmWRITE_STRING(infoDiverMode);
        }
#endif
        break;

    case GL1_PROFILER_WRITE_FRAME_BEGIN:
        if (!Context->profiler.frameBegun && Context->profiler.need_dump)
        {
            if (Context->profiler.frameNumber == 0 && Context->draw)
            {
                gctUINT width, height;
                gctUINT offset = 0;
                gctCHAR  infoScreen[255] = { '\0' };
                gcoOS_MemFill(infoScreen, 0, gcmSIZEOF(infoScreen));
                gcoSURF_GetSize(Context->draw, &width, &height, gcvNULL);
                gcoOS_PrintStrSafe(infoScreen, gcmSIZEOF(infoScreen),
                    &offset, "%d x %d", width, height);
                gcmWRITE_CONST_NEW(VPC_INFOSCREENSIZE);
                gcmWRITE_STRING_NEW(infoScreen);

                gcmWRITE_CONST_NEW(VPG_END);
            }
            gcmWRITE_COUNTER_NEW(VPG_FRAME, Context->profiler.frameNumber);
            Context->profiler.frameBegun = 1;
        }
        break;

    case GL1_PROFILER_WRITE_FRAME_END:

        gcmONERROR(gcoPROFILER_NEW_EndFrame(profiler));

        /*write time*/
        if (Context->profiler.need_dump)
        {
            gcmWRITE_CONST_NEW(VPG_TIME);
            gcmWRITE_COUNTER_NEW(VPC_ELAPSETIME, (gctINT32)(Context->profiler.frameEndTimeusec - Context->profiler.frameStartTimeusec));
            gcmWRITE_COUNTER_NEW(VPC_CPUTIME, (gctINT32)Context->profiler.totalDriverTime);
            gcmWRITE_CONST_NEW(VPG_END);

            gcoOS_GetMemoryUsage(&maxrss, &ixrss, &idrss, &isrss);

            gcmWRITE_CONST_NEW(VPG_MEM);

            gcmWRITE_COUNTER_NEW(VPC_MEMMAXRES, maxrss);
            gcmWRITE_COUNTER_NEW(VPC_MEMSHARED, ixrss);
            gcmWRITE_COUNTER_NEW(VPC_MEMUNSHAREDDATA, idrss);
            gcmWRITE_COUNTER_NEW(VPC_MEMUNSHAREDSTACK, isrss);

            gcmWRITE_CONST_NEW(VPG_END);

            /* write api time counters */
            gcmWRITE_CONST_NEW(VPG_ES11_TIME);
            for (i = 0; i < GLES1_NUM_API_CALLS; ++i)
            {
                if (Context->profiler.apiCalls[i] > 0)
                {
                    gcmWRITE_COUNTER_NEW(VPG_ES11_TIME + 1 + i, (gctINT32)Context->profiler.apiTimes[i]);
                }
            }
            gcmWRITE_CONST_NEW(VPG_END);

            gcmWRITE_CONST_NEW(VPG_ES11);


            for (i = 0; i < GLES1_NUM_API_CALLS; ++i)
            {
                if (Context->profiler.apiCalls[i] > 0)
                {
                    gcmWRITE_COUNTER(VPG_ES11 + 1 + i, Context->profiler.apiCalls[i]);
                    totalCalls_11 += Context->profiler.apiCalls[i];

                    switch (GLES1_APICALLBASE + i)
                    {
                    case GLES1_DRAWARRAYS:
                    case GLES1_DRAWELEMENTS:
                        totalDrawCalls_11 += Context->profiler.apiCalls[i];
                        break;

                    case    GLES1_ACTIVETEXTURE:
                    case    GLES1_ALPHAFUNC:
                    case    GLES1_ALPHAFUNCX:
                    case    GLES1_BLENDFUNC:
                    case    GLES1_CLEARCOLOR:
                    case    GLES1_CLEARCOLORX:
                    case    GLES1_CLEARDEPTHF:
                    case    GLES1_CLEARDEPTHX:
                    case    GLES1_CLEARSTENCIL:
                    case    GLES1_CLIENTACTIVETEXTURE:
                    case    GLES1_CLIPPLANEF:
                    case    GLES1_CLIPPLANEX:
                    case    GLES1_COLOR4F:
                    case    GLES1_COLOR4UB:
                    case    GLES1_COLOR4X:
                    case    GLES1_COLORMASK:
                    case    GLES1_CULLFACE:
                    case    GLES1_DEPTHFUNC:
                    case    GLES1_DEPTHMASK:
                    case    GLES1_DEPTHRANGEF:
                    case    GLES1_DEPTHRANGEX:
                    case    GLES1_DISABLE:
                    case    GLES1_DISABLECLIENTSTATE:
                    case    GLES1_ENABLE:
                    case    GLES1_ENABLECLIENTSTATE:
                    case    GLES1_FOGF:
                    case    GLES1_FOGFV:
                    case    GLES1_FOGX:
                    case    GLES1_FOGXV:
                    case    GLES1_FRONTFACE:
                    case    GLES1_FRUSTUMF:
                    case    GLES1_FRUSTUMX:
                    case    GLES1_LIGHTF:
                    case    GLES1_LIGHTFV:
                    case    GLES1_LIGHTMODELF:
                    case    GLES1_LIGHTMODELFV:
                    case    GLES1_LIGHTMODELX:
                    case    GLES1_LIGHTMODELXV:
                    case    GLES1_LIGHTX:
                    case    GLES1_LIGHTXV:
                    case    GLES1_LINEWIDTH:
                    case    GLES1_LINEWIDTHX:
                    case    GLES1_LOGICOP:
                    case    GLES1_MATERIALF:
                    case    GLES1_MATERIALFV:
                    case    GLES1_MATERIALX:
                    case    GLES1_MATERIALXV:
                    case    GLES1_MATRIXMODE:
                    case    GLES1_NORMAL3F:
                    case    GLES1_NORMAL3X:
                    case    GLES1_ORTHOF:
                    case    GLES1_ORTHOX:
                    case    GLES1_PIXELSTOREI:
                    case    GLES1_POINTPARAMETERF:
                    case    GLES1_POINTPARAMETERFV:
                    case    GLES1_POINTPARAMETERX:
                    case    GLES1_POINTPARAMETERXV:
                    case    GLES1_POINTSIZE:
                    case    GLES1_POINTSIZEX:
                    case    GLES1_POLYGONOFFSET:
                    case    GLES1_POLYGONOFFSETX:
                    case    GLES1_SAMPLECOVERAGE:
                    case    GLES1_SAMPLECOVERAGEX:
                    case    GLES1_SCISSOR:
                    case    GLES1_SHADEMODEL:
                    case    GLES1_STENCILFUNC:
                    case    GLES1_STENCILMASK:
                    case    GLES1_STENCILOP:
                    case    GLES1_TEXENVF:
                    case    GLES1_TEXENVFV:
                    case    GLES1_TEXENVI:
                    case    GLES1_TEXENVIV:
                    case    GLES1_TEXENVX:
                    case    GLES1_TEXENVXV:
                    case    GLES1_TEXPARAMETERF:
                    case    GLES1_TEXPARAMETERFV:
                    case    GLES1_TEXPARAMETERI:
                    case    GLES1_TEXPARAMETERIV:
                    case    GLES1_TEXPARAMETERX:
                    case    GLES1_TEXPARAMETERXV:
                    case    GLES1_VIEWPORT:
                    case    GLES1_BLENDEQUATIONOES:
                    case    GLES1_BLENDFUNCSEPERATEOES:
                    case    GLES1_BLENDEQUATIONSEPARATEOES:
                        totalStateChangeCalls_11 += Context->profiler.apiCalls[i];
                        break;

                    default:
                        break;
                    }
                }

                /* Clear variables for next frame. */
                Context->profiler.apiCalls[i] = 0;
                Context->profiler.apiTimes[i] = 0;
            }

            gcmWRITE_COUNTER_NEW(VPC_ES11CALLS, totalCalls_11);
            gcmWRITE_COUNTER_NEW(VPC_ES11DRAWCALLS, totalDrawCalls_11);
            gcmWRITE_COUNTER_NEW(VPC_ES11STATECHANGECALLS, totalStateChangeCalls_11);

            gcmWRITE_COUNTER_NEW(VPC_ES11POINTCOUNT, Context->profiler.drawPointCount_11);
            gcmWRITE_COUNTER_NEW(VPC_ES11LINECOUNT, Context->profiler.drawLineCount_11);
            gcmWRITE_COUNTER_NEW(VPC_ES11TRIANGLECOUNT, Context->profiler.drawTriangleCount_11);
            gcmWRITE_CONST_NEW(VPG_END);

            gcmWRITE_CONST_NEW(VPG_END);
        }
        break;

    case GL1_PROFILER_WRITE_FRAME_RESET:
        for (i = 0; i < GLES1_NUM_API_CALLS; ++i)
        {
            Context->profiler.apiCalls[i] = 0;
            Context->profiler.apiTimes[i] = 0;
            Context->profiler.totalDriverTime = 0;
        }

        /* Clear variables for next frame. */
        Context->profiler.drawPointCount_11 = 0;
        Context->profiler.drawLineCount_11 = 0;
        Context->profiler.drawTriangleCount_11 = 0;
        Context->profiler.drawVertexCount_11 = 0;
        Context->profiler.textureUploadSize = 0;
        Context->profiler.drawCount = 0;
        Context->profiler.totalDriverTime = 0;

        if (Context->profiler.timeEnable)
        {
            Context->profiler.frameStartCPUTimeusec =
                Context->profiler.frameEndCPUTimeusec;
            gcoOS_GetTime(&Context->profiler.frameStartTimeusec);
        }
        break;
    default:
        gcmASSERT(0);
        break;
    }

OnError:

    gcmFOOTER_NO();
    return status;
}

GLboolean
_glffProfiler_NEW_Set(
    IN glsCONTEXT_PTR Context,
    IN GLuint Enum,
    IN gctHANDLE Value
)
{
    glsPROFILER * profiler = &Context->profiler;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("Context=0x%x, Enum=%d, Value=0x%x", Context, Enum, Value);

    if (Context == gcvNULL || !profiler->enable)
    {
        gcmFOOTER_ARG("return=%d", GL_FALSE);
        return GL_FALSE;
    }

    switch (_glffProfileMode)
    {
    case 1:
        if (profiler->frameCount == 0 || profiler->frameNumber < profiler->frameCount)
        {
            Context->profilerObj->needDump = profiler->need_dump = gcvTRUE;
        }
        else
        {
            Context->profilerObj->needDump = profiler->need_dump = GL_FALSE;
        }
        break;
    case 2:
        if (profiler->enableOutputCounters)
        {
            Context->profilerObj->needDump = profiler->need_dump = gcvTRUE;
        }
        else
        {
            Context->profilerObj->needDump = profiler->need_dump = GL_FALSE;
        }
        break;
    case 3:
        if ((profiler->frameStartNumber == 0 && profiler->frameEndNumber == 0) ||
            (profiler->curFrameNumber >= profiler->frameStartNumber && profiler->curFrameNumber <= profiler->frameEndNumber))
        {
            Context->profilerObj->needDump = profiler->need_dump = gcvTRUE;
        }
        else
        {
            Context->profilerObj->needDump = profiler->need_dump = GL_FALSE;
        }
        break;
    default:
        gcmFOOTER_ARG("return=%d", GL_FALSE);
        return GL_FALSE;
    }

    switch (Enum)
    {
    case GL1_PROFILER_FRAME_END:

        gcmONERROR(gcoOS_GetTime(&profiler->frameEndTimeusec));
        gcoOS_GetCPUTime(&profiler->frameEndCPUTimeusec);
        profiler->drawCount = 0;
        profiler->curFrameNumber++;
        gcmONERROR(_glffProfiler_NEW_Write(Context, GL1_PROFILER_WRITE_FRAME_BEGIN));
        gcmONERROR(_glffProfiler_NEW_Write(Context, GL1_PROFILER_WRITE_FRAME_END));
        gcmONERROR(gcoPROFILER_NEW_Flush(Context->profilerObj));

        gcmONERROR(_glffProfiler_NEW_Write(Context, GL1_PROFILER_WRITE_FRAME_RESET));
        gcmONERROR(gcoOS_GetTime(&profiler->frameStartTimeusec));
        /* Next frame. */
        if (profiler->need_dump)
        profiler->frameNumber++;
        profiler->frameBegun = 0;
        break;

    case GL1_PROFILER_PRIMITIVE_TYPE:
        profiler->primitiveType = gcmPTR2INT32(Value);
        break;

    case GL1_PROFILER_PRIMITIVE_COUNT:
        profiler->primitiveCount = gcmPTR2INT32(Value);
        switch (profiler->primitiveType)
        {
        case GL_POINTS:
            profiler->drawPointCount_11 += gcmPTR2INT32(Value);
            break;

        case GL_LINES:
        case GL_LINE_LOOP:
        case GL_LINE_STRIP:
            profiler->drawLineCount_11 += gcmPTR2INT32(Value);
            break;

        case GL_TRIANGLES:
        case GL_TRIANGLE_STRIP:
        case GL_TRIANGLE_FAN:
            profiler->drawTriangleCount_11 += gcmPTR2INT32(Value);
            break;
        }
        break;

    case GL1_PROFILER_DRAW_BEGIN:
        if (profiler->perDraw == gcvFALSE)
        {
            profiler->perDraw = gcvTRUE;
            profiler->drawCount = 0;
        }
        gcmONERROR(_glffProfiler_NEW_Write(Context, GL1_PROFILER_WRITE_FRAME_BEGIN));
        gcmONERROR(gcoPROFILER_NEW_Begin(Context->profilerObj, gcvCOUNTER_OP_DRAW));

        break;

    case GL1_PROFILER_DRAW_END:
        gcoPROFILER_NEW_End(Context->profilerObj, profiler->drawCount);
        profiler->drawCount++;
        break;

    case GL1_TEXUPLOAD_SIZE:
        Context->profiler.textureUploadSize += gcmPTR2INT32(Value);
        break;
    case GL1_PROFILER_SYNC_MODE:
        gcmFOOTER_NO();
        return _glffIsSyncMode;

    default:
        break;
    }

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}

/*******************************************************************************
**    _glffInitializeProfiler
**
**    Initialize the profiler for the context provided.
**
**    Arguments:
**
**        GLContext Context
**            Pointer to a new GLContext object.
*/
void
_glffInitializeProfiler(
    glsCONTEXT_PTR Context
    )
{
    gceSTATUS status;
    gctUINT rev;
    char *env=gcvNULL;

    gcmHEADER_ARG("Context=0x%x", Context);

    _glffProfileMode = -1;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_PROFILE", &env)) && env)
    {
        if (gcmIS_SUCCESS(gcoOS_StrCmp(env, "0")))
        {
            _glffProfileMode = 0;
        }
        else if (gcmIS_SUCCESS(gcoOS_StrCmp(env, "1")))
        {
            _glffProfileMode = 1;
        }
        else if (gcmIS_SUCCESS(gcoOS_StrCmp(env, "2")))
        {
            _glffProfileMode = 2;
        }
        else if (gcmIS_SUCCESS(gcoOS_StrCmp(env, "3")))
        {
            _glffProfileMode = 3;
        }
    }

    if (_glffProfileMode == -1)
    {
        Context->profiler.enable = gcvFALSE;
#if VIVANTE_PROFILER_PROBE
        if(GLFFPROFILER_HAL == gcvNULL)
        {
            gctPOINTER pointer = gcvNULL;
            if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL,
                       gcmSIZEOF(struct _gcoHAL),
                       &pointer)))
            {
                gcmFATAL("%s(%d): gcoOS_Allocate failed", __FUNCTION__, __LINE__);
                gcmFOOTER_NO();
                return;
            }
            gcoOS_MemFill(pointer,0,gcmSIZEOF(struct _gcoHAL));
            GLFFPROFILER_HAL = (gcoHAL) pointer;
        }
        gcoPROFILER_Initialize(GLFFPROFILER_HAL, gcvNULL, gcvFALSE);
#endif
        gcmFOOTER_NO();
        return;
    }

    if (_glffProfileMode == 0)
    {
        Context->profiler.enable = gcvFALSE;
#if VIVANTE_PROFILER_PM
#if VIVANTE_PROFILER_PROBE
        if(GLFFPROFILER_HAL == gcvNULL)
        {
            gctPOINTER pointer = gcvNULL;
            if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL,
                       gcmSIZEOF(struct _gcoHAL),
                       &pointer)))
            {
                gcmFATAL("%s(%d): gcoOS_Allocate failed", __FUNCTION__, __LINE__);
                gcmFOOTER_NO();
                return;
            }
            gcoOS_MemFill(pointer,0,gcmSIZEOF(struct _gcoHAL));
            GLFFPROFILER_HAL = (gcoHAL) pointer;
        }
        gcoPROFILER_Initialize(GLFFPROFILER_HAL, gcvNULL, gcvFALSE);
#else
        gcoPROFILER_Initialize(gcvNULL, gcvNULL, gcvFALSE);
#endif
#endif
        gcmFOOTER_NO();
        return;
    }

#if VIVANTE_PROFILER_CONTEXT
    if(GLFFPROFILER_HAL == gcvNULL)
    {
        gctPOINTER pointer = gcvNULL;
        if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL,
                       gcmSIZEOF(struct _gcoHAL),
                       &pointer)))
        {
            gcmFATAL("%s(%d): gcoOS_Allocate failed", __FUNCTION__, __LINE__);
            gcmFOOTER_NO();
            return;
        }
        gcoOS_MemFill(pointer,0,gcmSIZEOF(struct _gcoHAL));
        GLFFPROFILER_HAL = (gcoHAL) pointer;
    }
#endif

    status = gcoPROFILER_Initialize(GLFFPROFILER_HAL, gcvNULL, gcvTRUE);
    switch (status)
    {
        case gcvSTATUS_OK:
            break;
        case gcvSTATUS_MISMATCH: /*fall through*/
        case gcvSTATUS_NOT_SUPPORTED:
        default:
            Context->profiler.enable = gcvFALSE;
#if VIVANTE_PROFILER_CONTEXT
            if(GLFFPROFILER_HAL != gcvNULL)
                gcoOS_Free(gcvNULL,GLFFPROFILER_HAL);
#endif
            gcmFOOTER_NO();
            return;
    }

    /* Clear the profiler. */
    gcoOS_ZeroMemory(&Context->profiler, gcmSIZEOF(Context->profiler));

    gcoOS_GetEnv(Context->os, "VP_SYNC_MODE", &env);
    if ((env != gcvNULL) && gcmIS_SUCCESS(gcoOS_StrCmp(env, "0")))
    {
        _glffIsSyncMode = gcvFALSE;
    }

    Context->profiler.useGlfinish = gcvFALSE;
    gcoOS_GetEnv(gcvNULL, "VP_USE_GLFINISH", &env);
    if ((env != gcvNULL) && (env[0] == '1'))
    {
        Context->profiler.useGlfinish = gcvTRUE;
    }

    gcoOS_GetEnv(Context->os, "VP_COUNTER_FILTER", &env);
    if ((env == gcvNULL) || (env[0] ==0))
    {
        Context->profiler.drvEnable =
            Context->profiler.timeEnable =
            Context->profiler.memEnable = gcvTRUE;
    }
    else
    {
        gctSIZE_T bitsLen = gcoOS_StrLen(env, gcvNULL);
        if (bitsLen > 0)
        {
            Context->profiler.timeEnable = (env[0] == '1');
        }
        else
        {
            Context->profiler.timeEnable = gcvTRUE;
        }
        if (bitsLen > 1)
        {
            Context->profiler.memEnable = (env[1] == '1');
        }
        else
        {
            Context->profiler.memEnable = gcvTRUE;
        }
        if (bitsLen > 5)
        {
            Context->profiler.drvEnable = (env[5] == '1');
        }
        else
        {
            Context->profiler.drvEnable = gcvTRUE;
        }
    }

    Context->profiler.frameCount = 0;
    Context->profiler.frameStartNumber = 0;
    Context->profiler.frameEndNumber = 0;

    Context->profiler.drawCount = 0;
    Context->profiler.perDraw = gcvFALSE;
    Context->profiler.perFrame = gcvFALSE;
    Context->profiler.useFBO = gcvFALSE;
    Context->profiler.enable = gcvTRUE;

    if (_glffProfileMode == 1)
    {
        Context->profiler.enableOutputCounters = gcvTRUE;
    }
    else if (_glffProfileMode == 2 || _glffProfileMode == 3)
    {
        Context->profiler.enableOutputCounters = gcvFALSE;
    }

    if (_glffProfileMode == 1)
    {
        gcoOS_GetEnv(gcvNULL, "VP_FRAME_NUM", &env);
        if ((env != gcvNULL) && (env[0] !=0))
        {
            int frameNum;
            gcoOS_StrToInt(env, &frameNum);
            if (frameNum > 1)
                Context->profiler.frameCount = (gctUINT32)frameNum;
        }
    }

    if (_glffProfileMode == 3)
    {
        gcoOS_GetEnv(gcvNULL, "VP_FRAME_START", &env);
        if ((env != gcvNULL) && (env[0] !=0))
        {
            int frameNum;
            gcoOS_StrToInt(env, &frameNum);
            if (frameNum > 1)
                Context->profiler.frameStartNumber = (gctUINT32)frameNum;
        }
        gcoOS_GetEnv(gcvNULL, "VP_FRAME_END", &env);
        if ((env != gcvNULL) && (env[0] !=0))
        {
            int frameNum;
            gcoOS_StrToInt(env, &frameNum);
            if (frameNum > 1)
                Context->profiler.frameEndNumber = (gctUINT32)frameNum;
        }
    }

#if gcdNEW_PROFILER_FILE
    {
        /* Write Generic Info. */
        char* infoCompany = "Vivante Corporation";
        char* infoVersion = "1.3";
        char  infoRevision[255] = {'\0'};   /* read from hw */
        char* infoRenderer = Context->chipName;
        char* infoDriver = "OpenGL ES 1.1";
        gctUINT offset = 0;
        rev = Context->chipRevision;
#define BCD(digit)      ((rev >> (digit * 4)) & 0xF)
        gcoOS_MemFill(infoRevision, 0, gcmSIZEOF(infoRevision));

        if (BCD(3) == 0)
        {
            /* Old format. */
            gcoOS_PrintStrSafe(infoRevision, gcmSIZEOF(infoRevision),
                &offset, "revision=\"%d.%d\" ", BCD(1), BCD(0));
        }
        else
        {
            /* New format. */
            gcoOS_PrintStrSafe(infoRevision, gcmSIZEOF(infoRevision),
                &offset, "revision=\"%d.%d.%d_rc%d\" ",
                BCD(3), BCD(2), BCD(1), BCD(0));
        }

        gcmWRITE_CONST(VPG_INFO);

        gcmWRITE_CONST(VPC_INFOCOMPANY);
        gcmWRITE_STRING(infoCompany);
        gcmWRITE_CONST(VPC_INFOVERSION);
        gcmWRITE_STRING(infoVersion);
        gcmWRITE_CONST(VPC_INFORENDERER);
        gcmWRITE_STRING(infoRenderer);
        gcmWRITE_CONST(VPC_INFOREVISION);
        gcmWRITE_STRING(infoRevision);
        gcmWRITE_CONST(VPC_INFODRIVER);
        gcmWRITE_STRING(infoDriver);
#if gcdNULL_DRIVER >= 2
    {
        char* infoDiverMode = "NULL Driver";
        gcmWRITE_CONST(VPC_INFODRIVERMODE);
        gcmWRITE_STRING(infoDiverMode);
    }
#endif

    }
#else

    /* Print generic info */
    _Print(Context, "<GenericInfo company=\"Vivante Corporation\" "
        "version=\"%d.%d\" renderer=\"%s\" ",
        1, 0, Context->chipName);

       rev = Context->chipRevision;
#define BCD(digit)        ((rev >> (digit * 4)) & 0xF)
       if (BCD(3) == 0)
       {
           /* Old format. */
           _Print(Context, "revision=\"%d.%d\" ", BCD(1), BCD(0));
       }
       else
       {
           /* New format. */
           _Print(Context, "revision=\"%d.%d.%d_rc%d\" ",
            BCD(3), BCD(2), BCD(1), BCD(0));
       }
    _Print(Context, "driver=\"%s\" />\n", "OpenGL ES 1.1");
#endif

    if (Context->profiler.timeEnable)
    {
        gcoOS_GetTime(&Context->profiler.frameStart);
        Context->profiler.frameStartTimeusec     = Context->profiler.frameStart;
        Context->profiler.primitiveStartTimeusec = Context->profiler.frameStart;
        gcoOS_GetCPUTime(&Context->profiler.frameStartCPUTimeusec);
    }
    gcmFOOTER_NO();
}

/*******************************************************************************
**    _glffDestroyProfiler
**
**    Destroy the profiler for the context provided.
**
**    Arguments:
**
**        GLContext Context
**            Pointer to a new GLContext object.
*/
void
_glffDestroyProfiler(
    glsCONTEXT_PTR Context
    )
{
    gcmHEADER_ARG("Context=0x%x", Context);
    if (Context->profiler.enable)
    {
        Context->profiler.enable = gcvFALSE;
        gcmVERIFY_OK(gcoPROFILER_Destroy(GLFFPROFILER_HAL));
#if VIVANTE_PROFILER_CONTEXT
        if(GLFFPROFILER_HAL != gcvNULL)
            gcoOS_Free(gcvNULL, GLFFPROFILER_HAL);
#endif
    }
    gcmFOOTER_NO();
}

#define PRINT_XML(counter)    _Print(Context, "<%s value=\"%d\"/>\n", \
                                   #counter, Context->profiler.counter)
#define PRINT_XML2(counter)    _Print(Context, "<%s value=\"%d\"/>\n", \
                                   #counter, counter)
#define PRINT_XMLL(counter)    _Print(Context, "<%s value=\"%lld\"/>\n", \
                                   #counter, Context->profiler.counter)
#define PRINT_XMLF(counter)    _Print(Context, "<%s value=\"%f\"/>\n", \
                                   #counter, Context->profiler.counter)


/* Function for printing frame number only once */
static void
beginFrame(
    glsCONTEXT_PTR Context
    )
{
    gcmHEADER_ARG("Context=0x%x", Context);
    if (Context->profiler.enable)
    {
        if (!Context->profiler.frameBegun)
        {
#if gcdNEW_PROFILER_FILE
            if (Context->profiler.frameNumber == 0 && Context->draw)
            {
                gctUINT width, height;
                gctUINT offset = 0;
                char  infoScreen[255] = {'\0'};
                gcoSURF_GetSize(Context->draw, &width, &height, gcvNULL);
                gcoOS_MemFill(infoScreen, 0, gcmSIZEOF(infoScreen));
                gcoOS_PrintStrSafe(infoScreen, gcmSIZEOF(infoScreen),
                        &offset, "%d x %d", width, height);
                gcmWRITE_CONST(VPC_INFOSCREENSIZE);
                gcmWRITE_STRING(infoScreen);

                gcmWRITE_CONST(VPG_END);
            }


            gcmWRITE_COUNTER(VPG_FRAME, Context->profiler.frameNumber);
#else
            _Print(Context, "<Frame value=\"%d\">\n",
                   Context->profiler.frameNumber);
#endif
            Context->profiler.frameBegun = 1;
        }
    }
    gcmFOOTER_NO();
}

static void
endFrame(
    glsCONTEXT_PTR Context
    )
{
    int i;
    gctUINT32 totalCalls_11 = 0;
    gctUINT32 totalDrawCalls_11 = 0;
    gctUINT32 totalStateChangeCalls_11 = 0;
    gctUINT32 maxrss, ixrss, idrss, isrss;
    gcmHEADER_ARG("Context=0x%x", Context);

    if (Context->profiler.enable)
    {
        beginFrame(Context);

        if (Context->profiler.timeEnable)
        {
            gctUINT32 frameTime = (gctUINT32)(Context->profiler.frameEndTimeusec
                - Context->profiler.frameStartTimeusec);
            gcmWRITE_CONST(VPG_TIME);
            gcmWRITE_COUNTER(VPC_ELAPSETIME, frameTime);
            gcmWRITE_COUNTER(VPC_CPUTIME, (gctUINT32) Context->profiler.totalDriverTime);
            gcmWRITE_CONST(VPG_END);
        }

        if (Context->profiler.memEnable)
        {
            gcoOS_GetMemoryUsage(&maxrss, &ixrss, &idrss, &isrss);

            gcmWRITE_CONST(VPG_MEM);

            gcmWRITE_COUNTER(VPC_MEMMAXRES, maxrss);
            gcmWRITE_COUNTER(VPC_MEMSHARED, ixrss);
            gcmWRITE_COUNTER(VPC_MEMUNSHAREDDATA, idrss);
            gcmWRITE_COUNTER(VPC_MEMUNSHAREDSTACK, isrss);

            gcmWRITE_CONST(VPG_END);
        }


        if (Context->profiler.drvEnable)
        {
            /* write api time counters */
            gcmWRITE_CONST(VPG_ES11_TIME);
            for (i = 0; i < GLES1_NUM_API_CALLS; ++i)
            {
                if (Context->profiler.apiCalls[i] > 0)
                {
                    gcmWRITE_COUNTER(VPG_ES11_TIME + 1 + i, (gctINT32) Context->profiler.apiTimes[i]);
                }
            }
            gcmWRITE_CONST(VPG_END);
            gcmWRITE_CONST(VPG_ES11);

            for (i = 0; i < GLES1_NUM_API_CALLS; ++i)
            {
                if (Context->profiler.apiCalls[i] > 0)
                {
                    gcmWRITE_COUNTER(VPG_ES11 + 1 + i, Context->profiler.apiCalls[i]);
                    totalCalls_11 += Context->profiler.apiCalls[i];

                switch (GLES1_APICALLBASE + i)
                {
                case GLES1_DRAWARRAYS:
                case GLES1_DRAWELEMENTS:
                    totalDrawCalls_11 += Context->profiler.apiCalls[i];
                    break;

                    case    GLES1_ACTIVETEXTURE :
                    case    GLES1_ALPHAFUNC :
                    case    GLES1_ALPHAFUNCX    :
                    case    GLES1_BLENDFUNC :
                    case    GLES1_CLEARCOLOR    :
                    case    GLES1_CLEARCOLORX   :
                    case    GLES1_CLEARDEPTHF   :
                    case    GLES1_CLEARDEPTHX   :
                    case    GLES1_CLEARSTENCIL  :
                    case    GLES1_CLIENTACTIVETEXTURE   :
                    case    GLES1_CLIPPLANEF    :
                    case    GLES1_CLIPPLANEX    :
                    case    GLES1_COLOR4F   :
                    case    GLES1_COLOR4UB  :
                    case    GLES1_COLOR4X   :
                    case    GLES1_COLORMASK :
                    case    GLES1_CULLFACE  :
                    case    GLES1_DEPTHFUNC :
                    case    GLES1_DEPTHMASK :
                    case    GLES1_DEPTHRANGEF   :
                    case    GLES1_DEPTHRANGEX   :
                    case    GLES1_DISABLE   :
                    case    GLES1_DISABLECLIENTSTATE    :
                    case    GLES1_ENABLE    :
                    case    GLES1_ENABLECLIENTSTATE :
                    case    GLES1_FOGF  :
                    case    GLES1_FOGFV :
                    case    GLES1_FOGX  :
                    case    GLES1_FOGXV :
                    case    GLES1_FRONTFACE :
                    case    GLES1_FRUSTUMF  :
                    case    GLES1_FRUSTUMX  :
                    case    GLES1_LIGHTF    :
                    case    GLES1_LIGHTFV   :
                    case    GLES1_LIGHTMODELF   :
                    case    GLES1_LIGHTMODELFV  :
                    case    GLES1_LIGHTMODELX   :
                    case    GLES1_LIGHTMODELXV  :
                    case    GLES1_LIGHTX    :
                    case    GLES1_LIGHTXV   :
                    case    GLES1_LINEWIDTH :
                    case    GLES1_LINEWIDTHX    :
                    case    GLES1_LOGICOP   :
                    case    GLES1_MATERIALF :
                    case    GLES1_MATERIALFV    :
                    case    GLES1_MATERIALX :
                    case    GLES1_MATERIALXV    :
                    case    GLES1_MATRIXMODE    :
                    case    GLES1_NORMAL3F  :
                    case    GLES1_NORMAL3X  :
                    case    GLES1_ORTHOF    :
                    case    GLES1_ORTHOX    :
                    case    GLES1_PIXELSTOREI   :
                    case    GLES1_POINTPARAMETERF   :
                    case    GLES1_POINTPARAMETERFV  :
                    case    GLES1_POINTPARAMETERX   :
                    case    GLES1_POINTPARAMETERXV  :
                    case    GLES1_POINTSIZE :
                    case    GLES1_POINTSIZEX    :
                    case    GLES1_POLYGONOFFSET :
                    case    GLES1_POLYGONOFFSETX    :
                    case    GLES1_SAMPLECOVERAGE    :
                    case    GLES1_SAMPLECOVERAGEX   :
                    case    GLES1_SCISSOR   :
                    case    GLES1_SHADEMODEL    :
                    case    GLES1_STENCILFUNC   :
                    case    GLES1_STENCILMASK   :
                    case    GLES1_STENCILOP :
                    case    GLES1_TEXENVF   :
                    case    GLES1_TEXENVFV  :
                    case    GLES1_TEXENVI   :
                    case    GLES1_TEXENVIV  :
                    case    GLES1_TEXENVX   :
                    case    GLES1_TEXENVXV  :
                    case    GLES1_TEXPARAMETERF :
                    case    GLES1_TEXPARAMETERFV    :
                    case    GLES1_TEXPARAMETERI :
                    case    GLES1_TEXPARAMETERIV    :
                    case    GLES1_TEXPARAMETERX :
                    case    GLES1_TEXPARAMETERXV    :
                    case    GLES1_VIEWPORT  :
                    case    GLES1_BLENDEQUATIONOES  :
                    case    GLES1_BLENDFUNCSEPERATEOES  :
                    case    GLES1_BLENDEQUATIONSEPARATEOES  :
                        totalStateChangeCalls_11 += Context->profiler.apiCalls[i];
                    break;

                    default:
                        break;
                    }
                }

                /* Clear variables for next frame. */
                Context->profiler.apiCalls[i] = 0;
                Context->profiler.apiTimes[i] = 0;
            }

            gcmWRITE_COUNTER(VPC_ES11CALLS, totalCalls_11);
            gcmWRITE_COUNTER(VPC_ES11DRAWCALLS, totalDrawCalls_11);
            gcmWRITE_COUNTER(VPC_ES11STATECHANGECALLS, totalStateChangeCalls_11);

            gcmWRITE_COUNTER(VPC_ES11POINTCOUNT, Context->profiler.drawPointCount_11);
            gcmWRITE_COUNTER(VPC_ES11LINECOUNT, Context->profiler.drawLineCount_11);
            gcmWRITE_COUNTER(VPC_ES11TRIANGLECOUNT, Context->profiler.drawTriangleCount_11);

            gcmWRITE_CONST(VPG_END);
        }

        gcoPROFILER_EndFrame(GLFFPROFILER_HAL);
        gcmWRITE_CONST(VPG_END);

        gcoPROFILER_Flush(GLFFPROFILER_HAL);

        /* Reset counters. */
        Context->profiler.drawPointCount_11      = 0;
        Context->profiler.drawLineCount_11       = 0;
        Context->profiler.drawTriangleCount_11  = 0;
        Context->profiler.textureUploadSize       = 0;
        Context->profiler.shaderCompileTime      = 0;
        Context->profiler.shaderStartTimeusec   = 0;
        Context->profiler.shaderEndTimeusec     = 0;
        Context->profiler.totalDriverTime = 0;
        if (Context->profiler.timeEnable)
        {
            Context->profiler.frameStartCPUTimeusec =
                Context->profiler.frameEndCPUTimeusec;
            gcoOS_GetTime(&Context->profiler.frameStartTimeusec);
        }

        /* Next frame. */
        Context->profiler.frameNumber++;
        Context->profiler.frameBegun = 0;
    }
#if VIVANTE_PROFILER_PROBE
    else
    {
        gcoPROFILER_EndFrame(GLFFPROFILER_HAL);
    }
#endif
    gcmFOOTER_NO();
}

/* Function for printing draw number only once */
#if VIVANTE_PROFILER_PROBE | VIVANTE_PROFILER_PERDRAW

static void beginDraw(IN glsCONTEXT_PTR Context){
#if VIVANTE_PROFILER_PERDRAW
    if (Context->profiler.enable)
    {
        gcmWRITE_CONST(VPG_ES11_DRAW);
        gcmWRITE_COUNTER(VPC_ES11_DRAW_NO,Context->profiler.drawCount);
    }
#endif
}

static void endDraw(IN glsCONTEXT_PTR Context){
    gcoPROFILER_End(GLFFPROFILER_HAL,(gctBOOL)(Context->profiler.drawCount == 0));
}
#endif


#define gcmCHECK_VP_MODE(need_dump) \
    do \
    { \
        switch (_glffProfileMode) \
        { \
        case 1: \
            if (context->profiler.frameCount == 0 || (context->profiler.frameNumber < context->profiler.frameCount)) \
                need_dump = gcvTRUE; \
            break; \
        case 2: \
            if (context->profiler.enableOutputCounters ) \
                need_dump = gcvTRUE; \
            else \
                need_dump = gcvFALSE; \
            break; \
        case 3: \
            if (context->profiler.frameStartNumber == 0 && context->profiler.frameEndNumber == 0) \
            { \
                need_dump = gcvTRUE; \
                break; \
            } \
            if (cur_frame_num >= context->profiler.frameStartNumber && cur_frame_num <= context->profiler.frameEndNumber) \
            { \
                need_dump = gcvTRUE; \
                break; \
            } \
            break; \
        default: \
            return GL_FALSE; \
        } \
    } \
    while (gcvFALSE)

#if !VIVANTE_PROFILER_PROBE
static void
resetFrameData(
    glsCONTEXT_PTR Context
    )
{
    gctUINT32 i;

    GLFFPROFILER_HAL->profiler.disableOutputCounter = gcvTRUE;
    gcoPROFILER_EndFrame(GLFFPROFILER_HAL);
    GLFFPROFILER_HAL->profiler.disableOutputCounter = gcvFALSE;

    for (i = 0; i < GLES1_NUM_API_CALLS; ++i)
    {
        Context->profiler.apiCalls[i] = 0;
        Context->profiler.apiTimes[i] = 0;
        Context->profiler.totalDriverTime = 0;
    }
    /* Clear variables for next frame. */
    Context->profiler.drawPointCount_11      = 0;
    Context->profiler.drawLineCount_11       = 0;
    Context->profiler.drawTriangleCount_11   = 0;
    Context->profiler.drawVertexCount_11   = 0;
    Context->profiler.drawCount = 0;
    Context->profiler.textureUploadSize   = 0;
    Context->profiler.totalDriverTime = 0;

    if (Context->profiler.timeEnable)
    {
        Context->profiler.frameStartCPUTimeusec =
            Context->profiler.frameEndCPUTimeusec;
        gcoOS_GetTime(&Context->profiler.frameStartTimeusec);
    }
}
#endif

gctBOOL
_glffProfiler(
    gctPOINTER Profiler,
    gctUINT32 Enum,
    gctHANDLE Value
    )
{
    glsCONTEXT_PTR context = GetCurrentContext();
    static gctUINT cur_frame_num = 0;
#if !VIVANTE_PROFILER_PROBE
    gctBOOL need_dump = gcvFALSE;
#endif

    gcmHEADER_ARG("Profiler=0x%x Enum=%u Value=%u", Profiler, Enum, Value);
    if (context == gcvNULL)
    {
        gcmFOOTER_NO();
        return gcvFALSE;
    }
#if !VIVANTE_PROFILER_PROBE
    if (! context->profiler.enable)
    {
        gcmFOOTER_NO();
        return gcvFALSE;
    }
#endif
    switch (Enum)
    {
    case GL1_PROFILER_FRAME_END:
        if (context->profiler.useGlfinish && gcmPTR2INT32(Value) != 1)
            break;
        if (context->profiler.timeEnable)
        {
            gcoOS_GetTime(&context->profiler.frameEndTimeusec);
            gcoOS_GetCPUTime(&context->profiler.frameEndCPUTimeusec);
        }
        context->profiler.perFrame = gcvTRUE;
        context->profiler.perDraw = gcvFALSE;
        context->profiler.drawCount = 0;
#if VIVANTE_PROFILER_PROBE
        cur_frame_num++;
        endFrame(context);
        break;
#else
        gcmCHECK_VP_MODE(need_dump);
        cur_frame_num++;
        if (need_dump)
            endFrame(context);
        else
            resetFrameData(context);
        break;
#endif

    case GL1_PROFILER_PRIMITIVE_END:
        /*endPrimitive(context);*/
        break;


    case GL1_PROFILER_DRAW_BEGIN:
#if VIVANTE_PROFILER_PROBE | VIVANTE_PROFILER_PERDRAW
        if(context->profiler.perDraw == gcvFALSE)
        {
            context->profiler.perDraw = gcvTRUE;
            context->profiler.perFrame = gcvFALSE;
            context->profiler.drawCount = 0;
        }
#if VIVANTE_PROFILER_PERDRAW
        beginFrame(context);
#endif
        beginDraw(context);
#endif
        break;
    case GL1_PROFILER_DRAW_END:
#if VIVANTE_PROFILER_PROBE | VIVANTE_PROFILER_PERDRAW
        endDraw(context);
        context->profiler.drawCount++;
#endif
        break;
    case GLES1_BINDFRAMEBUFFER:
        context->profiler.useFBO = gcmPTR2INT32(Value);
        break;

    case GL1_PROFILER_PRIMITIVE_TYPE:
        if (context->profiler.drvEnable)
        {
            context->profiler.primitiveType = gcmPTR2INT32(Value);
        }
        break;

    case GL1_PROFILER_PRIMITIVE_COUNT:
        if (!context->profiler.drvEnable)
            break;
        context->profiler.primitiveCount = gcmPTR2INT32(Value);

        switch (context->profiler.primitiveType)
        {
        case GL_POINTS:
            context->profiler.drawPointCount_11 += gcmPTR2INT32(Value);
            break;

        case GL_LINES:
        case GL_LINE_LOOP:
        case GL_LINE_STRIP:
            context->profiler.drawLineCount_11 += gcmPTR2INT32(Value);
            break;

        case GL_TRIANGLES:
        case GL_TRIANGLE_STRIP:
        case GL_TRIANGLE_FAN:
            context->profiler.drawTriangleCount_11 += gcmPTR2INT32(Value);
            break;
        }
        break;

    case GL1_TEXUPLOAD_SIZE:
        context->profiler.textureUploadSize += gcmPTR2INT32(Value);
        break;
    case GL1_PROFILER_SYNC_MODE:
        gcmFOOTER_NO();
        return _glffIsSyncMode;
    default:
        if (!context->profiler.drvEnable)
            break;
        if ((Enum > GLES1_APICALLBASE) && (Enum - GLES1_APICALLBASE < GLES1_NUM_API_CALLS))
        {
            context->profiler.apiCalls[Enum - GLES1_APICALLBASE]++;
        }
        break;
    }
    gcmFOOTER_NO();
    return gcvTRUE;
}
#endif
